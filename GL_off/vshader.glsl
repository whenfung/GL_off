#version 330 core

in vec3 vPosition;  //传进来的数据不能改变，我试过了
in vec3 vColor;
out vec3 color;
uniform mat4 rotation;

void main()
{
	//左乘一个矩阵实现缩放、旋转、平移变换 
    gl_Position = rotation * vec4(vPosition, 1.0);  //gl_Position是内置顶点变量，会传到光栅化模块中
    color = vColor;
}
