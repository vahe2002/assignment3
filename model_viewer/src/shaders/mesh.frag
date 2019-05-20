// Fragment shader
#version 150
uniform vec3 u_light_colour; //Color of light
uniform vec3 u_diffuse_colour; 
uniform vec3 u_specular_colour;
uniform vec3 u_ambient_colour;
uniform float u_specular_power;
uniform samplerCube u_cubemap;



//in vec3 v_normal;
in vec3 L;
in vec3 V;
in vec3 N;
out vec4 frag_color;
uniform bool gamma_correction;

void main()
{
    //vec3 N = normalize(v_normal);
   // frag_color = vec4(0.5 * N + 0.5, 1.0);

	

	vec3 H = normalize(L+V);
	vec3 Ia = u_ambient_colour;
	vec3 Id = u_diffuse_colour*u_light_colour*max(dot(N,L),0);
    vec3 Is = ((u_specular_power+8)/8)*(u_specular_colour*u_light_colour)*pow(max(dot(N,H),0.0),(u_specular_power));
	vec3 I = Ia + Id + Is;
	vec3 output_colour = I;

	//Calculate the reflective vector
	vec3 R = reflect(-V, N);
	//vec3 output_colour = texture(u_cubemap, R).rgb;
	
	 frag_color = vec4(0.5*output_colour, 1.0);
	 frag_color.rgb /= frag_color.a;
     if(gamma_correction){
     frag_color.rgb = pow(frag_color.rgb, vec3(1.0 / 2.2));
}

}
