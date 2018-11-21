#version 330 core

in vec3 color;

void main()
{
    gl_FragColor = vec4(color, 1.0);  //gl_FragColor是内置颜色变量，会将其传到光栅化后的像素中
}
