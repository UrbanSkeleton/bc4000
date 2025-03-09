#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform float time;
uniform vec4 color;
uniform vec2 position;

out vec4 finalColor;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Add unique identifier for each explosion based on its initial position and time
float getExplosionSeed(vec2 coord, float time) {
    // Use screen position of the explosion as a seed
    float baseSeed = random(coord * 0.01);

    // Combine with time to get different patterns for explosions at the same position
    return baseSeed + floor(time * 0.1); // Change pattern every 10 seconds
}

vec2 distortCoord(vec2 coord, float time, float seed) {
    float angle = atan(coord.y, coord.x);
    float dist = length(coord);

    // Use seed to vary swirl parameters
    float swirlSpeed = 2.0 + random(vec2(seed, 0.0)) * 2.0;
    float swirlIntensity = 0.1 + random(vec2(0.0, seed)) * 0.2;
    float swirl = sin(dist * 3.0 - time * swirlSpeed) * swirlIntensity;
    angle += swirl;

    // Vary distortion based on seed
    float distortFreq = 2.0 + random(vec2(seed, seed)) * 2.0;
    float distortAmp = 0.1 + random(vec2(seed + 1.0, seed)) * 0.1;
    float distortion = sin(angle * distortFreq + time) * distortAmp;
    dist += distortion;

    return vec2(cos(angle) * dist, sin(angle) * dist);
}

void main() {
    vec2 coord = (fragTexCoord - 0.5) * 2.0;

    // Get unique seed for this explosion
    float seed = getExplosionSeed(position, time);

    // Apply coordinate distortion with seed
    coord = distortCoord(coord, time, seed);
    float dist = length(coord);

    if (dist > 1.0) {
        discard;
    }

    // Vary noise patterns with seed
    vec2 noiseOffset = vec2(random(vec2(seed, 0.0)), random(vec2(0.0, seed)));
    float noise = random(coord * (4.0 + seed * 2.0) + time * vec2(1.0, 2.0) + noiseOffset);
    noise += random(coord * (8.0 + seed * 4.0) - time * vec2(2.0, 1.0) + noiseOffset) * 0.5;
    noise += random(coord.yx * (12.0 + seed * 6.0) + time * vec2(-1.5, 1.5) + noiseOffset) * 0.25;
    noise = noise * 0.5 + 0.5;

    // Vary directional bias with seed
    float dirFreq = 3.0 + random(vec2(seed + 2.0, seed)) * 2.0;
    float dirBias = sin(atan(coord.y, coord.x) * dirFreq + time * 3.0) * 0.2;
    dist += dirBias;

    // Vary edge properties with seed
    float edgeThickness = 1.0 + noise * 0.3 + dirBias + random(vec2(seed + 3.0, seed)) * 0.2;
    float edge = 1.0 - smoothstep(0.0, edgeThickness, dist);

    // Vary core properties with seed
    float coreIntensity = 2.0 + random(vec2(seed + 4.0, seed)) * 1.0;
    float core = exp(-dist * (coreIntensity + noise * 1.0)) * 1.5;

    // Vary flicker with seed
    float flickerSpeed = 20.0 + random(vec2(seed + 5.0, seed)) * 10.0;
    float flickerIntensity = 0.4 + random(vec2(seed + 6.0, seed)) * 0.2;
    float flicker = sin(time * flickerSpeed + noise * 12.0 + coord.x * 5.0) * flickerIntensity + 0.75;

    float fade = clamp(1.0 - time * 1.5, 0.0, 1.0);
    float alpha = edge * (core + noise * 0.7 + dirBias) * flicker * fade;

    // Vary color distribution with seed
    float colorVar = random(vec2(seed + 7.0, seed));
    vec3 hotCore = color.rgb * (1.8 + colorVar * 0.4);
    vec3 midColor = color.rgb * mix(0.6 + colorVar * 0.2, 0.7, noise + dirBias);
    vec3 outerColor = color.rgb * (0.4 + colorVar * 0.2);

    vec3 finalRGB = hotCore;
    finalRGB = mix(finalRGB, midColor, smoothstep(0.0, 0.5, dist + noise * 0.3 + dirBias));
    finalRGB = mix(finalRGB, outerColor, smoothstep(0.3, 0.8, dist + noise * 0.4 + dirBias));

    finalRGB += color.rgb * (noise * 0.2 + dirBias * 0.1);
    finalRGB *= (1.0 + core * (1.0 + dirBias));
    finalRGB = mix(finalRGB, hotCore, core * (0.5 + noise * 0.2));

    finalColor = vec4(finalRGB, alpha);
}
