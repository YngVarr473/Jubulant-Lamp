#version 330 core
in vec2 UV;
out vec4 color;

uniform sampler2D myTextureSampler;
uniform float nightIntensity;

void main(){
    vec4 texColor = texture(myTextureSampler, UV);
    color = vec4(texColor.rgb * (1.0 - nightIntensity), texColor.a);
}