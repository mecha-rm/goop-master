#version 410
in vec3 color;
in vec2 texUV;
in vec3 normal;
in vec3 viewDir;

out vec4 frag_color;

uniform sampler2D myTextureSampler;

void main() {


	frag_color = texture(myTextureSampler, texUV);// * vec4(color, 1.0);
	// GLfloat dot = normal.x * viewDir[2].x + normal.y * viewDir[2].y + normal.z * viewDir[2].z;
	// float d  = normal.x * viewDir[1].x + normal.y * viewDir[1].y + normal.z * viewDir[1].z;
	// double d  = (normal.x * viewDir[1].x) + (normal.y * viewDir[1].y) + (normal.z * viewDir[1].z);
	// float d = dot(normal, viewDir);

	// the dot product is returned as being negative, so we make it positive.
	// if(abs(d) < 0.01)
	// {
	// 	frag_color.r = 1.0;
	// 	frag_color.g = 0.0;
	// 	frag_color.b = 0.0;
	// }
	// else
	// {
	// 
	// }
	
}