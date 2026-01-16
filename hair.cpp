#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/* Use glew.h instead of gl.h to get all the GL prototypes declared */
#include <GL/glew.h>
/* Using the GLUT library for the base windowing setup */
#include <GL/freeglut.h>
/* GLM */
// #define GLM_MESSAGES
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "./common/shader_utils.h"
#define PI M_PI
#define pi PI
#include<iostream>
using namespace std;

int drawCount;
int screen_width=1200, screen_height=1600;
int hairCount=4000;

GLuint hair_base_pos, hair_tip_pos, hair_tip_vel;
GLint drawPipeline, compute;
GLuint ballCenters;
GLint attribute_centers, attribute_rotation3d,attribute_hair_base,attribute_tip_pos;
GLint uniform_mvp, uniform_rotation;
GLint hairTipColor, hairBaseColor;
	float tipColor[4] = {1.0f, 1.0f, 1.0f, 1.0f}; 
	float baseColor[4] = {1.0f, 1.0f, 1.0f, 1.0f}; 

class Hairs{
	public:
	GLfloat* basePos;
	GLfloat* tipPos;
	GLfloat* tipVel;

	Hairs(int count){
		basePos = new GLfloat[count*2];
		tipPos  = new GLfloat[count*4];
		tipVel  = new GLfloat[count*4];

		float equator=2*PI*sqrt(count/(4*pi));
		float rings=equator/8;
		float ringSpacing=pi/(2*rings);
		int c=0;
		float ringAngle=pi/(2.*rings);

		for(int i=-round(rings)*2; i<round(rings)*2 && c<count; i++){
			int layerCount=equator*sin(ringAngle * i);
			if(layerCount<=0) continue;
			float phiSpacing=2*pi/layerCount;
			for(int j=0;j<layerCount && c<count; j++){
				basePos[c*2+0]=i*ringAngle;
				basePos[c*2+1]=j*phiSpacing;
				tipPos[c*4+0]=sin(i*ringAngle)*cos(j*phiSpacing)*1.1;
				tipPos[c*4+1]=sin(i*ringAngle)*sin(j*phiSpacing)*1.1;
				tipPos[c*4+2]=cos(j*phiSpacing)*1.1;
				tipPos[c*4+3]=1.;
				tipVel[c*4+0]=0;
				tipVel[c*4+1]=0;
				tipVel[c*4+2]=0;
				tipVel[c*4+3]=0;
				c++;
			}
		}
		drawCount=c;
		cout<<drawCount<<endl;
		for(;c<count;c++){
			basePos[c*2+0]=0;
			basePos[c*2+1]=0;
			tipPos[c*4+0]=0;
			tipPos[c*4+1]=0;
			tipPos[c*4+2]=0;
			tipPos[c*4+3]=0;
			tipVel[c*4+0]=0;
			tipVel[c*4+1]=0;
			tipVel[c*4+2]=0;
			tipVel[c*4+3]=0;
		}
	}

	~Hairs(){
		delete[] basePos;
		delete[] tipPos;
		delete[] tipVel;
	}
};

int init_resources(){
	Hairs hairs(hairCount);

	glGenBuffers(1,&hair_base_pos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER,hair_base_pos);
	glBufferData(GL_SHADER_STORAGE_BUFFER,sizeof(float)*hairCount*2,hairs.basePos,GL_STATIC_DRAW);

	glGenBuffers(1,&hair_tip_pos);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER,hair_tip_pos);
	glBufferData(GL_SHADER_STORAGE_BUFFER,sizeof(float)*hairCount*4,hairs.tipPos,GL_STATIC_DRAW);

	glGenBuffers(1,&hair_tip_vel);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER,hair_tip_vel);
	glBufferData(GL_SHADER_STORAGE_BUFFER,sizeof(float)*hairCount*4,hairs.tipVel,GL_STATIC_DRAW);

	GLfloat centers[]={0,0,0};//I only want 1 sphere right now;
	glGenBuffers(1,&ballCenters);
	glBindBuffer(GL_ARRAY_BUFFER,ballCenters);
	glBufferData(GL_ARRAY_BUFFER,sizeof(centers),centers,GL_STATIC_DRAW);

	GLuint vs,fs,geo,cs;
	GLint link_ok = GL_FALSE;

	if ((vs = create_shader("hair.v.glsl", GL_VERTEX_SHADER)) == 0) return 0;
	if ((fs = create_shader("hair.f.glsl", GL_FRAGMENT_SHADER)) == 0) return 0;
	if ((geo = create_shader("hair.g.glsl", GL_GEOMETRY_SHADER)) == 0) return 0;

	drawPipeline=glCreateProgram();
	glAttachShader(drawPipeline,vs);
	glAttachShader(drawPipeline,fs);
	glAttachShader(drawPipeline,geo);
	glLinkProgram(drawPipeline);
	glGetProgramiv(drawPipeline, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		fprintf(stderr, "glLinkProgram:");
		print_log(drawPipeline);
		return 0;
	}

	const char* attribute_name;
	attribute_name = "hair_base_pos";
	attribute_hair_base = glGetAttribLocation(drawPipeline, attribute_name);
	if (attribute_hair_base == -1) {
		fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
		return 0;
	}
	hairTipColor = glGetUniformLocation(drawPipeline, "tipColor");
    hairBaseColor = glGetUniformLocation(drawPipeline, "baseColor");
    
	attribute_name = "hair_tip_pos";
	attribute_tip_pos= glGetAttribLocation(drawPipeline, attribute_name);
	if (attribute_tip_pos == -1) {
		fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
		return 0;
	}

	if ((cs = create_shader("hair.c.glsl", GL_COMPUTE_SHADER)) == 0) return 0;
	compute = glCreateProgram();
	glAttachShader(compute, cs);
	glLinkProgram(compute);
	glGetProgramiv(compute, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		fprintf(stderr, "Compute glLinkProgram:");
		print_log(compute);
		return 0;
	}

	uniform_rotation = glGetUniformLocation(compute, "rotation");
	return 1;
}

float viewA=0;

void onIdle(){
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -0.0));


	float r = 3.0f;
	float a = glm::radians(viewA);

	glm::vec3 eye(
		0.0f, 
		r * sin(a),
		r * cos(a)
	);

	glm::mat4 view = glm::lookAt(
		eye,
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(0.0, 1.0, 0.0)
	);


	glm::mat4 projection = glm::perspective(45.0f, 1.0f*screen_width/screen_height, 0.1f, 10000.0f);
	glm::mat4 mvp = projection * view * model;

	glUseProgram(drawPipeline);
	glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
	glUniform4fv(hairTipColor, 1, tipColor); 
    glUniform4fv(hairBaseColor, 1, baseColor);
	glutPostRedisplay();
}

float spin=0;

void onDisplay(){
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glUseProgram(compute);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, hair_base_pos);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, hair_tip_pos);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, hair_tip_vel);
	glUniform2f(uniform_rotation,0.0,sin(spin)/500);
	glDispatchCompute(drawCount, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glUseProgram(drawPipeline);

	glEnableVertexAttribArray(attribute_hair_base);
	glBindBuffer(GL_ARRAY_BUFFER,hair_base_pos);
	glVertexAttribPointer(attribute_hair_base,2,GL_FLOAT,GL_FALSE,0,0);

	glEnableVertexAttribArray(attribute_tip_pos);
	glBindBuffer(GL_ARRAY_BUFFER,hair_tip_pos);
	glVertexAttribPointer(attribute_tip_pos,4,GL_FLOAT,GL_FALSE,0,0);

	glDrawArrays(GL_POINTS,0,drawCount);
	glutSwapBuffers();
	spin+=0.01;
}

void onReshape(int width, int height) {
	screen_width = width;
	screen_height = height;
	glViewport(0, 0, screen_width, screen_height);
}

int main(int argc, char* argv[]) {
	if(argc>1){
		hairCount=atoi(argv[1]);
	}else{
		std::cout<<"No hair count specified, defaulting to "<<hairCount<<std::endl;
	}
	if(argc>2){
		viewA=atof(argv[2]);
	}else{
		std::cout<<"No view angle specified, defaulting to "<<viewA<<" degrees" <<std::endl;
	}
	if(argc > 6){

		baseColor[0] = atof(argv[3]);
		baseColor[1] = atof(argv[4]);
		baseColor[2] = atof(argv[5]);
		baseColor[3] = atof(argv[6]);
		
		tipColor[0] = atof(argv[3]);
		tipColor[1] = atof(argv[4]);
		tipColor[2] = atof(argv[5]);
		tipColor[3] = 0;
	}else{
		std::cout<<"No base color specified, defaulting to white"<<std::endl;
	}

	if(argc > 10){

		tipColor[0] = atof(argv[7]);
		tipColor[1] = atof(argv[8]);
		tipColor[2] = atof(argv[9]);
		tipColor[3] = atof(argv[10]);
	}else{
		std::cout<<"No tip color specified, defaulting to base color, but fully transparent"<<std::endl;
		std::cout<<"Usage:"<<argv[0]<<" (hairCount) (viewAngle) (baseColor_r g b a) (tipColor_r g b a)\n"<<std::endl;
		std::cout<<"Pre-made colors:\n";
		std::cout<<"\tBlond: ... 1 0.8 0.6 1\n";
		std::cout<<"\tBrunet:  1 0.6 0.4 1\n";
	}

		

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_ALPHA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(screen_width, screen_height);
	glutCreateWindow("My hair");

	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
		return 1;
	}

	if (!GLEW_VERSION_2_0) {
		fprintf(stderr, "Error: your graphic card does not support OpenGL 2.0\n");
		return 1;
	}

	if (init_resources()) {
		cout<<"started"<<endl;
		glutDisplayFunc(onDisplay);
		glutReshapeFunc(onReshape);
		glutIdleFunc(onIdle);
		onDisplay();
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glutMainLoop();
	}

	return 0;
}

