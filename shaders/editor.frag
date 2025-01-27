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

    float u_ColtempS, u_ColTempT; // Start temperature, target temperature [2000k, 10000k]

    float u_Hue; // [-180, 180] degrees
	float u_Saturation; // [0, 5]
	float u_Invert; // [0 or 1] switch

    float u_ApplyAwb; // [0 or 1] switch  
};

const mat3 rgb2xyz = mat3(+0.4124, +0.2126, +0.0193,
                          +0.3576, +0.7152, +0.1192,
                          +0.1805, +0.0722, +0.9505);
const mat3 xyz2rgb = mat3(+3.2406, -0.9689, +0.0557,
                          -1.5372, +1.8758, -0.2040,
                          -0.4986, +0.0415, +1.0570);
const mat3 xyz2lms = mat3(+0.8951, -0.7502, +0.0389,
                          +0.2664, +1.7135, -0.0685,
                          -0.1614, +0.0367, +1.0296);
const mat3 lms2xyz = mat3(+0.9867, +0.4323, -0.0085,
                          -0.1471, +0.5184, +0.0400,
                          +0.1600, +0.0493, +0.9685);

vec3 cct2xyz(float k) 
{
    // polynomial approximation of the Planckian Locus
    // (see: https://en.wikipedia.org/wiki/Planckian_locus#Approximation)
    float t = 1.0 / k;
    float x = (k <= 4000.0) ? (0.179910 + t * (0.8776956e3 + t * (-0.2343589e6 + t * (-0.2661239e9))))
                            : (0.240390 + t * (0.2226347e3 + t * (+2.1070379e6 + t * (-3.0258469e9))));
    float y = (k <= 2222.0) ? (-0.20219683 + x * (2.18555832 + x * (-1.34811020 + x * (-1.1063814))))
            : (k <= 4000.0) ? (-0.16748867 + x * (2.09137015 + x * (-1.37418593 + x * (-0.9549476))))
                            : (-0.37001483 + x * (3.75112997 + x * (-5.87338670 + x * (+3.0817580))));
    return vec3(x, y, 1.0 - x - y);
}

vec3 ApplyWhiteBalance(vec3 inColour) 
{
    vec3 wr = xyz2lms * cct2xyz(u_ColtempS);
    vec3 wt = xyz2lms * cct2xyz(u_ColTempT);

    vec3 lms = xyz2lms * rgb2xyz * pow(inColour, vec3(u_Gamma));

    lms.rg *= wt.rg / wr.rg;
    lms.b = wt.b * pow(lms.b / wr.b, pow(wr.b / wt.b, 0.0834));

    return pow(xyz2rgb * lms2xyz * lms, vec3(1.0 / u_Gamma));
}

// https://github.com/kajott/GIPS/blob/main/shaders/Color/Exposure.glsl
vec3 ApplyExposure(vec3 inColour)
{
    // forward gamma
    vec3 colour = pow(inColour, vec3(u_Gamma));

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

    // reverse gamma
    colour = pow(colour, vec3(1.0 / u_Gamma));

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

void main()
{
	vec4 imageColour = texture(u_InputImage, UV);

    vec3 colour = imageColour.rgb;

    colour = ApplyExposure(colour);
	colour = ApplyWhiteBalance(colour);
    colour = ApplyHueSat(colour);

    if (u_ApplyAwb > 0.5) {colour.rgb *= u_AWB_ScalingFactors;}

    FragColor = vec4(colour, imageColour.a);
}