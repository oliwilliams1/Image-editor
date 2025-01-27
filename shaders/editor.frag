#version 460 core

layout (location = 0) out vec4 FragColor;

in vec2 UV;

uniform sampler2D u_InputImage;

// https://github.com/kajott/GIPS/blob/main/shaders/Color/Color%20Temperature.glsl

layout(std140) uniform ImageEditData 
{
    vec3 u_AvgColour;
    float p1;
	vec3 u_AWB_ScalingFactors;
    float p2;

	float u_Gamma; // [0 or 1] switch

    float u_Exposure; // [-5, 5] working ev
    float u_GamutMap; // [0 or 1] switch
    float u_Reinhard; // [0 or 1] switch

    float u_ColTemp; // [-1.0, 1.0]
    float u_ColTint; // [-1.0, 1.0]

    float u_Hue; // [-180, 180] degrees
	float u_Saturation; // [0, 5]
	float u_Invert; // [0 or 1] switch

    float u_ApplyAwb; // [0 or 1] switch  
};

// https://github.com/kajott/GIPS/blob/main/shaders/Color/Exposure.glsl
vec3 ApplyExposure(vec3 inColour)
{
    // forward gamma
    vec3 colour = inColour;

    // apply gain
    float gain = exp2(u_Exposure);
    colour *= gain;

    // tone mapping
    if (u_Reinhard > 0.5) {
        colour = colour / (colour + 1.0);
        // post-scale so white stays white
        colour *= (gain + 1.0) / gain;
    }

    // gamut mapping
    float minRGB = min(min(colour.r, colour.g), colour.b);
    float maxRGB = max(max(colour.r, colour.g), colour.b);
    if ((maxRGB > 1.0) && (minRGB < maxRGB) && (u_GamutMap > 0.0)) {
        colour = mix(colour, vec3(minRGB) + (colour - vec3(minRGB)) * vec3((1.0 - minRGB) / (maxRGB - minRGB)), u_GamutMap);
    }

    return colour;
}

vec3 ApplyHueSat(vec3 inColour) 
{
    float hue = radians(u_Hue);

    vec3 colour = inColour;
    float luma = dot(inColour, vec3(0.299, 0.587, 0.114));
    vec3 chroma = colour - vec3(luma);

    float s = sqrt(1.0/3.0) * sin(hue), c = cos(hue), b = (1.0/3.0) * (1.0 - c);
    chroma = mat3(b+c, b-s, b+s,
                  b+s, b+c, b-s,
                  b-s, b+s, b+c) * chroma;

    if (u_Invert > 0.5) { luma = 1.0 - luma; }

    colour = vec3(luma) + chroma * u_Saturation;

    return colour;
}

vec3 ApplyColourTempTint(vec3 inColour)
{
    vec3 colour = inColour;

    float tempFac = u_ColTemp;

    vec3 tempTint;

    tempTint.r = 1.0 + tempFac   * 0.75; // Avoid unatural colours
    tempTint.g = 1.0 + u_ColTint * 0.75; // Avoid unatural colours
    tempTint.b = 1.0 + -tempFac  * 0.75; // Avoid unatural colours

    return tempTint * colour;
}

void main()
{
	vec4 imageColour = texture(u_InputImage, UV);

    vec3 colour = imageColour.rgb;
    colour = ApplyHueSat(colour);
    
    colour = pow(colour, vec3(u_Gamma));

    if (u_ApplyAwb > 0.5) {colour.rgb *= u_AWB_ScalingFactors;}
    colour = ApplyExposure(colour);
    colour = ApplyColourTempTint(colour);

    colour = pow(colour, vec3(1.0 / u_Gamma));

    FragColor = vec4(colour, imageColour.a);
}