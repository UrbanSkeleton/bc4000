#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform vec4 color;
uniform float time;

out vec4 finalColor;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main() {
    vec2 coord = (fragTexCoord - 0.5) * 2.0;
    float dist = length(coord);

    // Create elongated trail shape
    float trailLength = 1.5;
    vec2 stretchedCoord = vec2(coord.x * trailLength, coord.y);
    float stretchedDist = length(stretchedCoord);

    if (stretchedDist > 1.0) {
        discard;
    }

    // Create gradient along the trail
    float gradient = 1.0 - smoothstep(0.0, 1.0, stretchedDist);

    // Add some noise for more interesting effect
    float noise = random(coord * 10.0 + time * vec2(1.0, 2.0));
    noise = noise * 0.3 + 0.7; // Keep noise subtle

    // Fade out based on time
    float fade = clamp(1.0 - time * 2.0, 0.0, 1.0);

    // Make trail thinner at the end
    float thickness = smoothstep(1.0, 0.0, abs(coord.y) * (1.0 + coord.x));

    // Combine all effects
    float alpha = gradient * thickness * noise * fade * 0.7; // Reduce overall opacity

    // Create color variation
    vec3 trailColor = color.rgb;
    trailColor += vec3(0.2, 0.1, 0.0) * (1.0 - gradient); // Add warmer colors at the trail end

    // Add glow effect
    float glow = exp(-stretchedDist * 3.0) * 0.5;
    trailColor += color.rgb * glow;

    finalColor = vec4(trailColor, alpha);
}
