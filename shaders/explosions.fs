#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform float time;
uniform vec4 color;

out vec4 finalColor;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main() {
    vec2 coord = (fragTexCoord - 0.5) * 2.0;
    float dist = length(coord);

    if (dist > 1.0) {
        discard;
    }

    // Enhanced turbulent noise
    float noise = random(coord * 5.0 + time * vec2(1.0, 2.0));
    noise += random(coord * 10.0 - time * vec2(2.0, 1.0)) * 0.5;
    noise += random(coord * 15.0 + time * vec2(-1.5, 1.5)) * 0.25;
    noise = noise * 0.5 + 0.5;

    // Distorted edge with more intense core
    float edge = 1.0 - smoothstep(0.0, 1.0 + noise * 0.3, dist);

    // Brighter core glow
    float core = exp(-dist * 2.5) * 1.5;

    // More dramatic flame movement
    float flicker = sin(time * 25.0 + noise * 12.0) * 0.5 + 0.75;

    // Slower fade
    float fade = clamp(1.0 - time * 1.5, 0.0, 1.0);

    // Combine effects with increased intensity
    float alpha = edge * (core + noise * 0.7) * flicker * fade;

    // Use input color as the base, derive variations from it
    vec3 hotCore = color.rgb * 2.0; // Brighter center
    vec3 midColor = color.rgb * mix(1.0, 0.7, noise); // Slightly darker middle
    vec3 outerColor = color.rgb * 0.5; // Darker edges

    // Color variation based on noise and distance
    vec3 finalRGB = hotCore;
    finalRGB = mix(finalRGB, midColor, smoothstep(0.0, 0.5, dist + noise * 0.2));
    finalRGB = mix(finalRGB, outerColor, smoothstep(0.3, 0.8, dist + noise * 0.3));

    // Add some variation based on the input color
    finalRGB += color.rgb * (noise * 0.2);

    // Extra brightness in the core
    finalRGB *= (1.0 + core);

    // Ensure the color stays bright
    finalRGB = mix(finalRGB, hotCore, core * 0.5);

    finalColor = vec4(finalRGB, alpha);
}
