#include "Angel.h"
#include <vector>
#include <string>
#include <fstream>

#pragma comment(lib, "glew32.lib")

using namespace std;

// 记录三角面片中的顶点序列的结构体
typedef struct vIndex {
	unsigned int a, b, c;
	vIndex(int ia, int ib, int ic) : a(ia), b(ib), c(ic) {}
} vec3i;

vector<vec3>  vertices;
vector<vec3i> faces;

vector<vec3> points;    //传入顶点着色器的点
vector<vec3> colors;    //传入片元着色器的点

int nFaces;
int nVertices;
int nEdges;

const vec3 vertex_colors[8] = {
	vec3(1.0, 1.0, 1.0),  // White
	vec3(1.0, 1.0, 0.0),  // Yellow
	vec3(0.0, 1.0, 0.0),  // Green
	vec3(0.0, 1.0, 1.0),  // Cyan
	vec3(1.0, 0.0, 1.0),  // Magenta
	vec3(1.0, 0.0, 0.0),  // Red
	vec3(0.0, 0.0, 0.0),  // Black
	vec3(0.0, 0.0, 1.0)   // Blue
};

GLuint mainWin;                                //主窗口
GLint mLocation;                          //矩阵位置
int axis = 0;
double theta[3] = {0,0,0};  //三个方向的旋转角度

//-----------------------------------------------以下是实现
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) axis = 0;
	if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) axis = 1;
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) axis = 2;
}

void spinCube() {
	theta[axis] += 0.1;
	if (theta[axis] > 360.0) theta[axis] -= 360.0;
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key)
	{
	case 't':
		glutIdleFunc(spinCube);
		break;
	case 'p':
		glutIdleFunc(NULL);
		break;
	}
}

void read_off(string filename) {
	if (filename.empty()) return;

	ifstream fin;
	fin.open(filename);
	if (!fin) {
		cout << "文件读取有误" << endl;
		return;
	}
	//根据off文件格式读取数据
	cout << "文件打开成功" << endl;
	//读取字符串OFF
	string str;
	fin >> str;
	//读取文件中的顶点数、面片数，边数
	fin >> nVertices >> nFaces >> nEdges;
	// 根据顶点数，循环读取每个顶点坐标
	for (int i = 0; i < nVertices; i++) {
		vec3 node;  
		fin >> node.x >> node.y >> node.z;  //注意坐标是小数
		vertices.push_back(node);
	}
	// 根据面片数，循环读取每个面片信息，并用构建的vec3i结构体保存
	for (int i = 0; i < nFaces; i++) {
		int num, a, b, c;
		fin >> num >> a >> b >> c;
		faces.push_back(vec3i(a, b, c));
	}
	fin.close();
}

void storeFacesPoints()
{
	points.clear();
	colors.clear();
	// 在points和colors容器中存储每个三角面片的各个点和颜色信息
	for (int i = 0; i < nFaces; i++)
	{
		// 在points容器中，依次添加每个面片的顶点，并在colors容器中，添加该点的颜色信息
		points.push_back(vertices[faces[i].a]);
		colors.push_back(vertex_colors[i/2]);

		points.push_back(vertices[faces[i].b]);
		colors.push_back(vertex_colors[i/2]);

		points.push_back(vertices[faces[i].c]);
		colors.push_back(vertex_colors[i/2]);
	}
}

void init() {
	//将OFF文件中的数据转成规则数据
	storeFacesPoints();  
	
	//创建顶点数组对象
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//创建并初始化顶点缓存对象
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(vec3) + colors.size()*sizeof(vec3), NULL, GL_STATIC_DRAW);

	//读取顶点和颜色数组里的数据到内存中
	glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(vec3), &points[0]);
	glBufferSubData(GL_ARRAY_BUFFER, points.size() * sizeof(vec3), colors.size() * sizeof(vec3), &colors[0]);
	
	//读取着色器并使用
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	//从顶点着色器中初始化顶点的位置
	GLuint pLocation = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(pLocation);
	glVertexAttribPointer(pLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	//从片元着色器中初始化顶点的颜色
	GLuint cLocation = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(cLocation);
	glVertexAttribPointer(cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(points.size() * sizeof(vec3)));

	// 获得矩阵存储位置
	mLocation = glGetUniformLocation(program, "rotation");  

	//黑色背景
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

void display() {   //这里才用到片元着色器
	//清理窗口，包括颜色缓存和深度缓存
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//创建一个单位矩阵，过会用
	mat4 m(1.0, 0.0, 0.0, 0.0,
		   0.0, 1.0, 0.0, 0.0,
		   0.0, 0.0, 1.0, 0.0,
		   0.0, 0.0, 0.0, 1.0);

	// 调用函数传入三种变化的变化量，计算变化矩阵
	m = RotateX(theta[0])*RotateY(theta[1])*RotateZ(theta[2])*m;
	glUniformMatrix4fv(mLocation, 1, GL_TRUE, m);

	//绘制顶点
	glDrawArrays(GL_TRIANGLES, 0, points.size()); //每三个点为一个三角形，绘制正方体
	glutSwapBuffers();
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);

	//窗口显示模式支持深度测试
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutCreateWindow("读取off文件引入3D模型");

	//希望在调用任何OpenGL函数前初始化GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	//读取off文件
	read_off("cube.off");

	init();
	glutDisplayFunc(display);
	glutIdleFunc(spinCube);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);

	//启用深度测试
	glEnable(GL_DEPTH_TEST);

	glutMainLoop();
	return 0;
}