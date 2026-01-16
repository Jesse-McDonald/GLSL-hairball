layout(points, invocations=10) in;
layout(line_strip, max_vertices=2) out;
in vec4 hairTipPos[];
in vec4 controle2[];
uniform mat4 mvp;
out vec4 glColor;
uniform vec4 baseColor;
uniform vec4 tipColor; 
void OutputVertex(vec4 emit){
	gl_Position=mvp*emit;
	EmitVertex();
}
void setColor(float n){
	glColor=baseColor * (1.0 - n) + tipColor * n;
}
vec4 bezier3c(float t, vec4 p1, vec4 p2, vec4 p3){
	return  vec4(
		(1-t)*(1-t)*p1.x+2*(1-t)*t*p2.x+t*t*p3.x,
		(1-t)*(1-t)*p1.y+2*(1-t)*t*p2.y+t*t*p3.y,
		(1-t)*(1-t)*p1.z+2*(1-t)*t*p2.z+t*t*p3.z,
		1);
}
void main(void){
	setColor(gl_InvocationID/10.);
	OutputVertex(bezier3c(gl_InvocationID/10.,gl_in[0].gl_Position,controle2[0],hairTipPos[0]));
	setColor((gl_InvocationID+1)/10.);
	OutputVertex(bezier3c((gl_InvocationID+1)/10.,gl_in[0].gl_Position,controle2[0],hairTipPos[0]));
	

	EndPrimitive();
}
/*
void main(void){
	glColor=vec4(0,0,1,1);
	float phi,theta;
	vec3 base=vec3(gl_in[0].gl_Position.x,gl_in[0].gl_Position.y,gl_in[0].gl_Position.z);
	OutputVertex(vec4(base,1));

	
	//theta=atan(base.y-hairTipPos[0].y,base.x-hairTipPos[0].x);
	//phi=atan(sqrt(pow(base.x-hairTipPos[0].x,2)+pow(base.y-hairTipPos[0].y,2)),base.z-hairTipPos[0].z);
	theta=(normals[0][0]);
	phi=(normals[0][1]);

	glColor=vec4(0,1,0,1);
	OutputVertex(vec4(   sin(theta)*cos(phi)*1.05,
		sin(theta)*sin(phi)*1.05,
		cos(theta)*1.05,
			1));
	glColor=vec4(1,0,0,1);
	OutputVertex(vec4(hairTipPos[0]));

	EndPrimitive();
glColor=vec4(1,1,1,1);
	base=vec3(gl_in[0].gl_Position.x,gl_in[0].gl_Position.y,gl_in[0].gl_Position.z);
	//OutputVertex(vec4(base,1));

glColor=vec4(1,1,1,1);
	//OutputVertex(vec4(hairTipPos[0]));

}
*/
