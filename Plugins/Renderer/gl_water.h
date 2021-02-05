#pragma once

#define MAX_WATERS 32

typedef struct
{
	int program;
	int waterfogcolor;
	int eyepos;
	int eyedir;
	int zmax;
	int time;
	int fresnel;
	int depthfactor;
	int normfactor;
	int abovewater;
	int normalmap;
	int refractmap;
	int reflectmap;	
	int depthrefrmap;
}water_program_t;

typedef struct
{
	int program;
	int depthmap;
}drawdepth_program_t;

typedef struct r_water_s
{
	GLuint refractmap;
	GLuint depthrefrmap;

	GLuint reflectmap;
	GLuint depthreflmap;

	vec3_t vecs;
	float distances;
	cl_entity_t *ent;
	vec3_t org;
	colorVec color;
	int is3dsky;
	int texwidth;
	int texheight;
	int free;
	struct r_water_s *next;
}r_water_t;

//renderer
extern qboolean drawreflect;
extern qboolean drawrefract;

//water
extern r_water_t *curwater;
extern r_water_t *waters_free;
extern r_water_t *waters_active;

//shader
extern SHADER_DEFINE(water);
extern int water_normalmap;
extern int water_normalmap_default;

extern SHADER_DEFINE(drawdepth);

//water fog
extern int *g_bUserFogOn;
extern int save_userfogon;

//cvar
extern cvar_t *r_water;
extern cvar_t *r_water_debug;
extern cvar_t *r_water_fresnel;
extern cvar_t *r_water_depthfactor;
extern cvar_t *r_water_normfactor;
extern cvar_t *r_water_novis;
extern cvar_t *r_water_texscale;

void R_AddEntityWater(cl_entity_t *ent, vec3_t p, colorVec *color);
void R_InitWater(void);
void R_ClearWater(void);
void R_RenderWaterView(void);
void R_EnableClip(qboolean isdrawworld);
void R_FreeDeadWaters(void);