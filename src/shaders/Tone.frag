#version 460 core

out vec4 FragColor;
uniform sampler2D text;
uniform float exposure;
uniform int tonemap;
uniform bool grayscale;

const vec3 weights = vec3(0.2126, 0.7152, 0.0722);

vec3 reinhard(vec3 colour) {
    return colour / (colour + 1.0);
}

// Jodie https://www.shadertoy.com/view/4dBcD1
vec3 jodieReinhard(vec3 colour){
    float l = dot(colour, weights);
    float k = max(max(max(colour.r, colour.g), colour.b) - l - 1.0, 0.0);
    return (colour + l * k) / ((l + 1.0) * (k + 1.0));
}

// Krzysztof Narkowicz ACES
vec3 acesFast(vec3 colour) {
    return clamp((colour * (2.51 * colour + 0.03)) / (colour * (2.43 * colour + 0.59) + 0.14), 0.0, 1.0);
}

// Stephen Hill ACES
vec3 acesFitted(vec3 colour) {
    colour = mat3(0.59719, 0.07600, 0.02840, 0.35458, 0.90834, 0.13383, 0.04823, 0.01566, 0.83777) * colour;
    colour = (colour * (colour + 0.0245786) - 0.000090537) / (colour * (0.983729 * colour + 0.4329510) + 0.238081);
    colour = mat3(1.60475, -0.10208, -0.00327, -0.53108, 1.10813, -0.07276, -0.07367, -0.00605, 1.07602) * colour;
    return clamp(colour, 0.0, 1.0);
}

void main() {
    vec3 colour = texelFetch(text, ivec2(gl_FragCoord.xy), 0).rgb * exposure;
    if (grayscale) colour = vec3(dot(colour, weights));

    switch (tonemap) {
        case 1:
            colour = reinhard(colour);
            break;
        case 2:
            colour = jodieReinhard(colour);
            break;
        case 3:
            colour = acesFast(colour);
            break;
        case 4:
            colour = acesFitted(colour);
            break;
        // no tone map
        case 0:
        default:
            break;
    }

    // linear to srgb
    FragColor = vec4(mix(colour * 12.92, 1.055 * pow(colour, vec3(1.0 / 2.4)) - 0.055, step(0.0031308, colour)), 1.0);
};