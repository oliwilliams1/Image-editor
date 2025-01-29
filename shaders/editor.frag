#version 460 core

layout (location = 0) out vec4 FragColor;

in vec2 UV;

uniform sampler2D u_InputImage;

layout(std140) uniform ImageEditData 
{
    vec3 u_AvgColour;
    float padding;
	vec3 u_AWB_ScalingFactors;
    float padding2;

    uint u_NumMasks;

	float u_Gamma; // [0 or 1] switch

    float u_Exposure; // [-5, 5] working ev
    float u_Reinhard; // [0 or 1] switch

    float u_ColTemp; // [-1.0, 1.0]
    float u_ColTint; // [-1.0, 1.0]

    float u_Hue; // [-180, 180] degrees
	float u_Saturation; // [0, 5]
	float u_Invert; // [0 or 1] switch

    float u_ApplyAwb; // [0 or 1] switch  

    float u_Shadows; // [-1, 1]
    float u_Highlights; // [-1, 1]

    float u_Contrast; // [-1, 1]
};

struct MaskData 
{
    float colourMask[3];
    float colourMaskThreshold;

    float maskType; // [0 == no mask, 1 == colour mask, 2 == lum mask]

    float lumMaskUpper;
    float lumMaskMid;
	float lumMaskLower;

    float exposure;
    float reinhard;

    float contrast;

    float colTemp;
    float colTint;

    float hue;
    float saturation;
    float invert;

    float shadows;
    float highlights;

    float viewMask;
};

layout(std430, binding = 1) buffer MaskDataBuffer
{
    MaskData maskData[];
};

// https://github.com/kajott/GIPS/blob/main/shaders/Color/Exposure.glsl
vec3 ApplyExposure(vec3 inColour, float ev, float tonemapping)
{
    // forward gamma
    vec3 colour = inColour;

    // apply gain
    float gain = exp2(ev);
    colour *= gain;

    // tone mapping
    if (tonemapping > 0.5) {
        colour = colour / (colour + 1.0);
        // post-scale so white stays white
        colour *= (gain + 1.0) / gain;
    }

    // gamut mapping
    float minRGB = min(min(colour.r, colour.g), colour.b);
    float maxRGB = max(max(colour.r, colour.g), colour.b);

    return colour - inColour;
}

vec3 ApplyHueSat(vec3 inColour, float inHue, float sat, float invert)
{
    float hue = radians(inHue);

    vec3 colour = inColour;
    float luma = dot(inColour, vec3(0.299, 0.587, 0.114));
    vec3 chroma = colour - vec3(luma);

    float s = sqrt(1.0/3.0) * sin(hue), c = cos(hue), b = (1.0/3.0) * (1.0 - c);
    chroma = mat3(b+c, b-s, b+s,
                  b+s, b+c, b-s,
                  b-s, b+s, b+c) * chroma;

    if (invert > 0.5) { luma = 1.0 - luma; }

    colour = vec3(luma) + chroma * sat;

    return colour - inColour;
}

vec3 ApplyContrast(vec3 inColour, float contrast)
{
    vec3 colour = inColour;

    contrast += 1.0 * 0.5;

    colour = (colour - 0.5) * contrast + 0.5;

    return colour - inColour;
}

vec3 ApplyShadowsAndHighlights(vec3 inColour, float shadows, float highlights)
{
    vec3 colour = inColour;
    float luminance = dot(colour, vec3(0.2126, 0.7152, 0.0722));

    float shadowThreshold = 0.6;
    float highlightThreshold = 0.4;

    float shadowMask = 1.0 - smoothstep(0.0, shadowThreshold, luminance);

    float highlightMask = smoothstep(highlightThreshold, 1.0, luminance);

    colour *= 1.0 + shadowMask * shadows * 0.5;
    colour *= 1.0 + highlightMask * highlights * 0.5;

    return vec3(colour) - inColour;
}

vec3 ApplyColourTempTint(vec3 inColour, float colTemp, float colTint)
{
    vec3 colour = inColour;

    float tempFac = colTemp;

    vec3 tempTint;

    tempTint.r = 1.0 + tempFac   * 0.5; // Avoid unatural colours
    tempTint.g = 1.0 + colTint * 0.5; // Avoid unatural colours
    tempTint.b = 1.0 + -tempFac  * 0.5; // Avoid unatural colours

    return (tempTint * colour) - inColour;
}

float ColourMaskFac(vec3 inColour, vec3 maskColour, float threshold) {
    float d = length(inColour - maskColour);
    
    float maxDistance = 1.732; // sqrt(3)

    float similarity = 1.0 - (d / maxDistance);
    
    similarity = clamp(similarity, 0.0, 1.0);

    if (similarity < threshold) {
        return 0.0;
    } else {
        return (similarity - threshold) / (1.0 - threshold);
    }
}

void main()
{
	vec4 imageColour = texture(u_InputImage, UV);

    vec3 colour = imageColour.rgb;
    
    colour += ApplyHueSat(colour, u_Hue, u_Saturation, u_Invert);

    colour = pow(colour, vec3(u_Gamma));
    if (u_ApplyAwb > 0.5) {colour.rgb *= u_AWB_ScalingFactors;}

    colour += ApplyExposure(colour, u_Exposure, u_Reinhard);
    colour += ApplyContrast(colour, u_Contrast);
    colour += ApplyShadowsAndHighlights(colour, u_Shadows, u_Highlights);
    colour += ApplyColourTempTint(colour, u_ColTemp, u_ColTint);

    colour = pow(colour, vec3(1.0 / u_Gamma));

    colour = clamp(colour, 0.0, 1.0);

    for (int i = 0; i < u_NumMasks; i++) 
    {
        float maskFac = 1.0;

        if (maskData[i].maskType == 1)
        {
            vec3 colourMask = vec3(maskData[i].colourMask[0], maskData[i].colourMask[1], maskData[i].colourMask[2]);

			maskFac = ColourMaskFac(imageColour.rgb, colourMask, maskData[i].colourMaskThreshold);
        }

		colour += maskFac * ApplyHueSat(colour, maskData[i].hue, maskData[i].saturation, maskData[i].invert);

        colour = pow(colour, vec3(u_Gamma));
        colour += maskFac * ApplyExposure(colour, maskData[i].exposure, maskData[i].reinhard);
        colour += maskFac * ApplyContrast(colour, maskData[i].contrast);
        colour += maskFac * ApplyShadowsAndHighlights(colour, maskData[i].shadows, maskData[i].highlights);
		colour += maskFac * ApplyColourTempTint(colour, maskData[i].colTemp, maskData[i].colTint);
		colour = pow(colour, vec3(1.0 / u_Gamma));

        colour = clamp(colour, 0.0, 1.0);
    }

    for (int i = 0; i < u_NumMasks; i++)
    {
        if (maskData[i].viewMask > 0.5)
        {
			float maskFac = 1.0;

            if (maskData[i].maskType == 1)
            {
                vec3 colourMask = vec3(maskData[i].colourMask[0], maskData[i].colourMask[1], maskData[i].colourMask[2]);

			    maskFac = ColourMaskFac(imageColour.rgb, colourMask, maskData[i].colourMaskThreshold);
            }

            colour = vec3(maskFac);
        }
    }

    FragColor = vec4(colour, imageColour.a);
}