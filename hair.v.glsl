in vec2 hair_base_pos;
in vec4 hair_tip_pos;
out vec4 hairTipPos;
out vec4 controle2;
void main(void){
	float xs=sin(hair_base_pos[0])*cos(hair_base_pos[1]),
        ys=sin(hair_base_pos[0])*sin(hair_base_pos[1]),
	zs=cos(hair_base_pos[0]);
	gl_Position=vec4(xs,ys,zs,1);
	//gl_Position=vec4(hair_base_pos[0],
	//		hair_base_pos[1],
	//		0,
	//		1);
	hairTipPos=hair_tip_pos;
	controle2=vec4(xs*1.1,ys*1.1,zs*1.05,1);
}
