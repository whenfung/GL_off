#version 330 core

in vec3 vPosition;
in vec3 vColor;
out vec3 color;
uniform mat4 matrix;

void main()
{
	//左乘一个矩阵实现缩放、旋转、平移变换
    gl_Position = matrix * vec4(vPosition, 1.0);  //gl_Position是内置顶点变量，会传到光栅化模块中
    color = vColor;
}
