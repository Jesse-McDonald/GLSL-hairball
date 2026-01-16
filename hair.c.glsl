layout(std430, binding=0) buffer hair_base_pos {
	vec2 base_pos[];
} ;
layout(std430, binding=1) buffer hair_tip_pos{
	vec4 tip_pos[];
} ;
layout(std430, binding=2) buffer hair_Tip_Vel {
	vec4 tip_vel[];
} ;
layout(local_size_x=1) in;
uniform vec2 rotation;

void main(){
	base_pos[gl_WorkGroupID.x].x=base_pos[gl_WorkGroupID.x].x+rotation.x;
	base_pos[gl_WorkGroupID.x].y=base_pos[gl_WorkGroupID.x].y+rotation.y;
vec4 basePosTemp=//I do wish I didnt have to do this twice
			vec4(sin(base_pos[gl_WorkGroupID.x].x)*cos(base_pos[gl_WorkGroupID.x].y),
			sin(base_pos[gl_WorkGroupID.x].x)*sin(base_pos[gl_WorkGroupID.x].y),
			cos(base_pos[gl_WorkGroupID.x].x),
			1);
//follow vel
tip_vel[gl_WorkGroupID.x]=
			vec4(sin(base_pos[gl_WorkGroupID.x].x)*cos(base_pos[gl_WorkGroupID.x].y)*1.1,
			sin(base_pos[gl_WorkGroupID.x].x)*sin(base_pos[gl_WorkGroupID.x].y)*1.1,
			cos(base_pos[gl_WorkGroupID.x].x)*1.1,
			0)-tip_pos[gl_WorkGroupID.x];
tip_vel[gl_WorkGroupID.x].w=0;
tip_vel[gl_WorkGroupID.x].z-=.5;
//maybe update position here
//spring vel
float ax,ay,az;
ax=acos((basePosTemp.x-tip_pos[gl_WorkGroupID.x].x)/distance(basePosTemp,tip_pos[gl_WorkGroupID.x]));
ay=acos((basePosTemp.y-tip_pos[gl_WorkGroupID.x].y)/distance(basePosTemp,tip_pos[gl_WorkGroupID.x]));
az=acos((basePosTemp.z-tip_pos[gl_WorkGroupID.x].z)/distance(basePosTemp,tip_pos[gl_WorkGroupID.x]));
tip_vel[gl_WorkGroupID.x]-=.15*vec4(
		acos(basePosTemp.x)-ax,
		acos(basePosTemp.y)-ay,
		acos(basePosTemp.z)-az,
		0);
//unit vel

//maybe apply directly to tip pos
tip_vel[gl_WorkGroupID.x]-=.1*vec4(
		cos(ax),
		cos(ay),
		cos(az),
		0);


tip_pos[gl_WorkGroupID.x]=tip_pos[gl_WorkGroupID.x]+tip_vel[gl_WorkGroupID.x]*.01;
tip_pos[gl_WorkGroupID.x]=vec4(normalize(vec3(tip_pos[gl_WorkGroupID.x])-vec3(basePosTemp)),0)*.1+basePosTemp;
//tip_pos[gl_WorkGroupID.x].w=1;
	//tip_pos[gl_WorkGroupID.x]=tip_pos[gl_WorkGroupID.x];
//	tip_pos[gl_WorkGroupID.x]=
//			vec4(sin(base_pos[gl_WorkGroupID.x].x)*cos(base_pos[gl_WorkGroupID.x].y)*1.1,
//			sin(base_pos[gl_WorkGroupID.x].x)*sin(base_pos[gl_WorkGroupID.x].y)*1.1,
//			cos(base_pos[gl_WorkGroupID.x].x)*1.1-1,
//			1);
}
