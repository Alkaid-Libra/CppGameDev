#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;  // 纹理采样器

void main() {    
    // 采样纹理颜色
    FragColor = texture(texture1, TexCoord);
}
