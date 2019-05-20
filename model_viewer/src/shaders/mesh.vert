// Vertex shader
#version 150
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_normal;

//out vec3 v_normal;

uniform mat4 u_mvp;
uniform mat4 u_mv; 
uniform vec3 u_light_position; // The position of your light source
uniform vec3 u_light_colour; //Colour of light
uniform vec3 u_diffuse_colour; 
uniform vec3 u_specular_colour;
uniform vec3 u_ambient_colour;
uniform float u_specular_power;


out vec3 L;
out vec3 N;
out vec3 V;

void main()
{
   
    gl_Position = u_mvp * a_position;

	vec3 position_eye = vec3(u_mv * a_position);
	
	// Calculate the view-space normal
	N = normalize(mat3(u_mv) * a_normal);
	
	// Calculate the view-space light direction
	
	L = normalize(u_light_position - position_eye); // 

	// Calculate the view-vector 
    V = normalize(-position_eye);
}
