#version 410
layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec3 vertex_color;
layout(location = 2) in vec2 vertex_uv;
layout(location = 3) in vec3 vertex_normal; // EX; normal object

out vec3 color;
out vec2 texUV;
out vec3 normal; // EX
out vec3 viewDir; // EX

uniform mat4 MVP;

uniform sampler2D myTextureSampler;

uniform vec3 cameraPos;

uniform float time; // used for animating waves

void main() {
	color = vertex_color;

	vec3 v = vertex_pos;
	vec3 tempClr = vertex_color; // the color
	vec3 tempNml = vertex_normal; // the normal
	vec3 view = vec3(MVP[2].x, MVP[2].y, MVP[2].z); // the view projection


	// height map
	// since the colour map is black and white, the r, g, and b values will all be the same. So it doesn't matter which one we choose.
	// white goes up, black stays the same
	v.y = texture(myTextureSampler, vertex_uv).r;

	// wave
	// y will be a function of sin(ax);
	// v.y = sin(5.0 * v.x) * 0.25; // static
	v.y = sin(5.0 * v.x + time * 0.01) * 0.25; // animated
	v.y = v.y + sin(5.0 * v.x + time * 0.01) * 0.25; // heightmap and wave
	
	// EX: cel-shading
	// if the dot product is equal to 0.
	// if(true)
	// {
		// tempClr.r = 0.0;
		// tempClr.g = 0.0;
		// tempClr.b = 0.0;
	// }


	// color = tempClr;
	gl_Position = MVP * vec4(v, 1.0);
	texUV = vertex_uv;
	// color = tempClr;
	normal = vertex_normal; // EX
	viewDir = cameraPos - vertex_pos;
}
	