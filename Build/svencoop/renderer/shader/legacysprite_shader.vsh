#version 430 compatibility

#include "common.h"

out vec3 v_worldpos;
out vec2 v_diffusetexcoord;
out vec4 v_color;

void main()
{
	vec3 outvert = gl_Vertex.xyz;
	
	v_worldpos = outvert;
	v_diffusetexcoord = gl_MultiTexCoord0.xy;

	v_color = gl_Color;
	gl_Position = SceneUBO.projMatrix * SceneUBO.viewMatrix * vec4(outvert, 1.0);
}