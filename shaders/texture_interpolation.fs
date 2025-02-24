#version 330

in vec2 fragTexCoord;
in vec2 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float mixFactor;

void main() {
    vec4 color1 = texture(texture0, fragTexCoord);
    vec4 color2 = texture(texture1, fragTexCoord);
    finalColor = mix(color1, color2, mixFactor);
    // finalColor = vec4(1, 0, 0, 1);
}
