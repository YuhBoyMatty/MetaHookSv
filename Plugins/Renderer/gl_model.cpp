#include "gl_local.h"

#if 0

int LittleLong(int l)
{
	return l;
}

short LittleShort(short l)
{
	return l;
}

float LittleFloat(float l)
{
	return l;
}

void Mod_LoadLighting(model_t *mod, byte *mod_base, lump_t *l)
{
	if (!l->filelen)
	{
		mod->lightdata = NULL;
		return;
	}

	mod->lightdata = (byte *)Hunk_AllocName(l->filelen, *loadname);
	memcpy(mod->lightdata, mod_base + l->fileofs, l->filelen);
}

void Mod_LoadVisibility(model_t *mod, byte *mod_base, lump_t *l)
{
	if (!l->filelen)
	{
		mod->visdata = NULL;
		return;
	}

	mod->visdata = (byte *)Hunk_AllocName(l->filelen, *loadname);
	memcpy(mod->visdata, mod_base + l->fileofs, l->filelen);
}

void Mod_LoadEntities(model_t *mod, byte *mod_base, lump_t *l)
{
	char *pszInputStream;

	if (!l->filelen)
	{
		mod->entities = NULL;
		return;
	}

	mod->entities = (char *)Hunk_AllocName(l->filelen, *loadname);
	memcpy(mod->entities, mod_base + l->fileofs, l->filelen);

	if (mod->entities)
	{
		pszInputStream = gEngfuncs.COM_ParseFile(mod->entities);

		if (!*pszInputStream)
			return;

		while (com_token[0] != '}')
		{
			if (!Q_strcmp(com_token, "wad"))
			{
				COM_Parse(pszInputStream);

				if (wadpath)
					Mem_Free(wadpath);

				wadpath = Mem_Strdup(com_token);
				return;
			}

			pszInputStream = COM_Parse(pszInputStream);

			if (!*pszInputStream)
				return;
		}
	}
}

void Mod_LoadVertexes(model_t *mod, byte *mod_base, lump_t *l)
{
	auto in = (dvertex_t *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		g_pMetaHookAPI->SysError("MOD_LoadBmodel: funny lump size in %s", *loadname);

	int count = l->filelen / sizeof(*in);
	auto out = (mvertex_t *)Hunk_AllocName(count * sizeof(mvertex_t), *loadname);

	mod->vertexes = out;
	mod->numvertexes = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		out->position[0] = LittleFloat(in->point[0]);
		out->position[1] = LittleFloat(in->point[1]);
		out->position[2] = LittleFloat(in->point[2]);
	}
}

void Mod_LoadSubmodels(model_t *mod, byte *mod_base, lump_t *l)
{
	auto in = (dmodel_t *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		g_pMetaHookAPI->SysError("MOD_LoadBmodel: funny lump size in %s", *loadname);

	int count = l->filelen / sizeof(*in);
	auto out = (dmodel_t *)Hunk_AllocName(count * sizeof(dmodel_t), *loadname);

	mod->submodels = out;
	mod->numsubmodels = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		for (int j = 0; j < 3; j++)
		{
			out->mins[j] = LittleFloat(in->mins[j]) - 1;
			out->maxs[j] = LittleFloat(in->maxs[j]) + 1;
			out->origin[j] = LittleFloat(in->origin[j]);
		}

		for (int j = 0; j < MAX_MAP_HULLS; j++)
		{
			out->headnode[j] = LittleLong(in->headnode[j]);
		}

		out->visleafs = LittleLong(in->visleafs);
		out->firstface = LittleLong(in->firstface);
		out->numfaces = LittleLong(in->numfaces);
	}
}

void Mod_LoadEdges(model_t *mod, byte *mod_base, lump_t *l)
{
	auto in = (dedge_t *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		g_pMetaHookAPI->SysError("MOD_LoadBmodel: funny lump size in %s", *loadname);

	int count = l->filelen / sizeof(*in);
	auto out = (medge_t *)Hunk_AllocName((count + 1) * sizeof(medge_t), *loadname);

	mod->edges = out;
	mod->numedges = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		out->v[0] = (unsigned short)LittleShort(in->v[0]);
		out->v[1] = (unsigned short)LittleShort(in->v[1]);
	}
}

void Mod_LoadTexinfo(model_t *mod, byte *mod_base, lump_t *l)
{
	int i, j, count;
	int miptex;
	float len1, len2;

	auto in = (texinfo_t *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		g_pMetaHookAPI->SysError("MOD_LoadBmodel: funny lump size in %s", *loadname);

	int count = l->filelen / sizeof(*in);
	auto out = (mtexinfo_t *)Hunk_AllocName(count * sizeof(mtexinfo_t), *loadname);

	mod->texinfo = out;
	mod->numtexinfo = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		for (j = 0; j < 8; j++)
			out->vecs[0][j] = LittleFloat(in->vecs[0][j]);

		len1 = VectorLength(out->vecs[0]);
		len2 = VectorLength(out->vecs[1]);
		len1 = (len1 + len2) / 2;

		if (len1 < 0.32)
			out->mipadjust = 4;
		else if (len1 < 0.49)
			out->mipadjust = 3;
		else if (len1 < 0.99)
			out->mipadjust = 2;
		else
			out->mipadjust = 1;
#if 0
		if (len1 + len2 < 0.001)
			out->mipadjust = 1;
		else
			out->mipadjust = 1 / floor((len1 + len2) / 2 + 0.1);
#endif

		miptex = LittleLong(in->miptex);
		out->flags = LittleLong(in->flags);

		if (!mod->textures)
		{
			out->texture = r_notexture_mip;
			out->flags = 0;
		}
		else
		{
			if (miptex >= mod->numtextures)
				g_pMetaHookAPI->SysError("miptex >= loadmodel->numtextures");

			out->texture = mod->textures[miptex];

			if (!out->texture)
			{
				out->texture = r_notexture_mip;
				out->flags = 0;
			}
		}
	}
}

void CalcSurfaceExtents(model_t *mod, msurface_t *s)
{
	float mins[2], maxs[2], val;
	int i, j, e;
	mvertex_t *v;
	mtexinfo_t *tex;
	int bmins[2], bmaxs[2];

	mins[0] = mins[1] = 999999;
	maxs[0] = maxs[1] = -99999;

	tex = s->texinfo;

	for (i = 0; i < s->numedges; i++)
	{
		e = mod->surfedges[s->firstedge + i];

		if (e >= 0)
			v = &mod->vertexes[mod->edges[e].v[0]];
		else
			v = &mod->vertexes[mod->edges[-e].v[1]];

		for (j = 0; j < 2; j++)
		{
			val = v->position[0] * tex->vecs[j][0] + v->position[1] * tex->vecs[j][1] + v->position[2] * tex->vecs[j][2] + tex->vecs[j][3];

			if (val < mins[j])
				mins[j] = val;

			if (val > maxs[j])
				maxs[j] = val;
		}
	}

	for (i = 0; i < 2; i++)
	{
		bmins[i] = floor(mins[i] / 16);
		bmaxs[i] = ceil(maxs[i] / 16);

		s->texturemins[i] = bmins[i] * 16;
		s->extents[i] = (bmaxs[i] - bmins[i]) * 16;

		if (!(tex->flags & TEX_SPECIAL) && s->extents[i] > 512)
			g_pMetaHookAPI->SysError("Bad surface extents %d/%d at position (%d,%d,%d)", s->extents[0], s->extents[1], (int)v->position[0], (int)v->position[1], (int)v->position[2]);
	}
}

void Mod_LoadFaces(model_t *mod, byte *mod_base, lump_t *l)
{
	int i, count, surfnum;
	int planenum, side;

	auto in = (dface_t *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		g_pMetaHookAPI->SysError("MOD_LoadBmodel: funny lump size in %s", *loadname);

	int count = l->filelen / sizeof(*in);
	auto out = (msurface_t *)Hunk_AllocName(count * sizeof(msurface_t), *loadname);

	mod->surfaces = out;
	mod->numsurfaces = count;

	for (int surfnum = 0; surfnum < count; surfnum++, in++, out++)
	{
		out->firstedge = LittleLong(in->firstedge);
		out->numedges = LittleShort(in->numedges);
		out->flags = 0;
		out->pdecals = NULL;

		planenum = LittleShort(in->planenum);
		side = LittleShort(in->side);

		if (side)
			out->flags |= SURF_PLANEBACK;

		out->plane = mod->planes + planenum;
		out->texinfo = mod->texinfo + LittleShort(in->texinfo);

		CalcSurfaceExtents(mod, out);

		for (i = 0; i < MAXLIGHTMAPS; i++)
			out->styles[i] = in->styles[i];

		i = LittleLong(in->lightofs);

		if (i == -1)
			out->samples = NULL;
		else
			out->samples = mod->lightdata + i;

		if (!strncmp(out->texinfo->texture->name, "sky", 3))
		{
			out->flags |= (SURF_DRAWSKY | SURF_DRAWTILED);
			continue;
		}

		if (!strncmp(out->texinfo->texture->name, "scroll", 6))
		{
			out->flags |= SURF_DRAWTILED;
			continue;
		}

		if (out->texinfo->texture->name[0] == '!' || !strnicmp(out->texinfo->texture->name, "laser", 5) || !strnicmp(out->texinfo->texture->name, "water", 5))
		{
			out->flags |= SURF_DRAWTURB;
			GL_SubdivideSurface(out);
			continue;
		}

		if (out->texinfo->flags & TEX_SPECIAL)
		{
			out->flags |= SURF_DRAWTILED;
			continue;
		}
	}
}

void Mod_SetParent(model_t *mod, byte *mod_base, mnode_t *node, mnode_t *parent)
{
	node->parent = parent;

	if (node->contents < 0)
		return;

	Mod_SetParent(mod, mod_base, node->children[0], node);
	Mod_SetParent(mod, mod_base, node->children[1], node);
}

void Mod_LoadNodes(model_t *mod, byte *mod_base, lump_t *l)
{
	int i, j, count, p;

	auto in = (dnode_t *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		g_pMetaHookAPI->SysError("MOD_LoadBmodel: funny lump size in %s", *loadname);

	count = l->filelen / sizeof(*in);
	auto out = (mnode_t *)Hunk_AllocName(count * sizeof(mnode_t), *loadname);

	mod->nodes = out;
	mod->numnodes = count;

	for (i = 0; i < count; i++, in++, out++)
	{
		for (j = 0; j < 3; j++)
		{
			out->minmaxs[j] = LittleShort(in->mins[j]);
			out->minmaxs[3 + j] = LittleShort(in->maxs[j]);
		}

		p = LittleLong(in->planenum);
		out->plane = mod->planes + p;

		out->firstsurface = LittleShort(in->firstface);
		out->numsurfaces = LittleShort(in->numfaces);

		for (j = 0; j < 2; j++)
		{
			p = LittleShort(in->children[j]);

			if (p >= 0)
				out->children[j] = mod->nodes + p;
			else
				out->children[j] = (mnode_t *)(mod->leafs + (-1 - p));
		}
	}

	Mod_SetParent(mod, mod_base, mod->nodes, NULL);
}

void Mod_LoadLeafs(model_t *mod, byte *mod_base, lump_t *l)
{
	auto in = (dleaf_t *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		g_pMetaHookAPI->SysError("MOD_LoadBmodel: funny lump size in %s", *loadname);

	int count = l->filelen / sizeof(*in);
	auto out = (mleaf_t *)Hunk_AllocName(count * sizeof(mleaf_t), *loadname);

	mod->leafs = out;
	mod->numleafs = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		for (int j = 0; j < 3; j++)
		{
			out->minmaxs[j] = LittleShort(in->mins[j]);
			out->minmaxs[3 + j] = LittleShort(in->maxs[j]);
		}

		auto p = LittleLong(in->contents);
		out->contents = p;

		out->firstmarksurface = mod->marksurfaces + LittleShort(in->firstmarksurface);
		out->nummarksurfaces = LittleShort(in->nummarksurfaces);

		p = LittleLong(in->visofs);

		if (p == -1)
			out->compressed_vis = NULL;
		else
			out->compressed_vis = mod->visdata + p;

		out->efrags = NULL;

		for (int j = 0; j < 4; j++)
			out->ambient_sound_level[j] = in->ambient_level[j];
	}
}

void Mod_LoadClipnodes(model_t *mod, byte *mod_base, lump_t *l)
{
	auto in = (dclipnode_t *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		g_pMetaHookAPI->SysError("MOD_LoadBmodel: funny lump size in %s", *loadname);

	int count = l->filelen / sizeof(*in);
	auto out = (dclipnode_t *)Hunk_AllocName(count * sizeof(dclipnode_t), *loadname);

	mod->clipnodes = out;
	mod->numclipnodes = count;

	auto hull = &mod->hulls[1];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count - 1;
	hull->planes = mod->planes;
	hull->clip_mins[0] = -16;
	hull->clip_mins[1] = -16;
	hull->clip_mins[2] = -36;
	hull->clip_maxs[0] = 16;
	hull->clip_maxs[1] = 16;
	hull->clip_maxs[2] = 36;

	hull = &mod->hulls[2];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count - 1;
	hull->planes = mod->planes;
	hull->clip_mins[0] = -32;
	hull->clip_mins[1] = -32;
	hull->clip_mins[2] = -32;
	hull->clip_maxs[0] = 32;
	hull->clip_maxs[1] = 32;
	hull->clip_maxs[2] = 32;

	hull = &mod->hulls[3];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count - 1;
	hull->planes = mod->planes;
	hull->clip_mins[0] = -16;
	hull->clip_mins[1] = -16;
	hull->clip_mins[2] = -18;
	hull->clip_maxs[0] = 16;
	hull->clip_maxs[1] = 16;
	hull->clip_maxs[2] = 18;

	for (int i = 0; i < count; i++, out++, in++)
	{
		out->planenum = LittleLong(in->planenum);
		out->children[0] = LittleShort(in->children[0]);
		out->children[1] = LittleShort(in->children[1]);
	}
}

void Mod_MakeHull0(model_t *mod, byte *mod_base)
{
	auto hull = &mod->hulls[0];

	auto in = mod->nodes;
	int count = mod->numnodes;
	auto out = (dclipnode_t *)Hunk_AllocName(count * sizeof(dclipnode_t), *loadname);

	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count - 1;
	hull->planes = mod->planes;

	for (int i = 0; i < count; i++, out++, in++)
	{
		out->planenum = in->plane - mod->planes;

		for (int j = 0; j < 2; j++)
		{
			auto child = in->children[j];

			if (child->contents < 0)
				out->children[j] = child->contents;
			else
				out->children[j] = child - mod->nodes;
		}
	}
}

void Mod_LoadMarksurfaces(model_t *mod, byte *mod_base, lump_t *l)
{
	auto in = (short *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		g_pMetaHookAPI->SysError("MOD_LoadBmodel: funny lump size in %s", *loadname);

	int count = l->filelen / sizeof(*in);
	auto out = (msurface_t **)Hunk_AllocName(count * sizeof(msurface_t *), *loadname);

	mod->marksurfaces = out;
	mod->nummarksurfaces = count;

	for (int i = 0; i < count; i++)
	{
		auto j = LittleShort(in[i]);

		if (j >= mod->numsurfaces)
			g_pMetaHookAPI->SysError("Mod_ParseMarksurfaces: bad surface number");

		out[i] = mod->surfaces + j;
	}
}

void Mod_LoadSurfedges(model_t *mod, byte *mod_base, lump_t *l)
{
	auto in = (int *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		g_pMetaHookAPI->SysError("MOD_LoadBmodel: funny lump size in %s", *loadname);

	int count = l->filelen / sizeof(*in);
	auto out = (int *)Hunk_AllocName(count * sizeof(int), *loadname);

	mod->surfedges = out;
	mod->numsurfedges = count;

	for (int i = 0; i < count; i++)
		out[i] = LittleLong(in[i]);
}

void Mod_LoadPlanes(model_t *mod, byte *mod_base, lump_t *l)
{
	auto in = (dplane_t *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		g_pMetaHookAPI->SysError("MOD_LoadBmodel: funny lump size in %s", *loadname);

	int count = l->filelen / sizeof(*in);
	auto out = (mplane_t *)Hunk_AllocName(count * 2 * sizeof(mplane_t), *loadname);

	mod->planes = out;
	mod->numplanes = count;

	for (int i = 0; i < count; i++, in++, out++)
	{
		int bits = 0;

		for (int j = 0; j < 3; j++)
		{
			out->normal[j] = LittleFloat(in->normal[j]);

			if (out->normal[j] < 0)
				bits |= 1 << j;
		}

		out->dist = LittleFloat(in->dist);
		out->type = LittleLong(in->type);
		out->signbits = bits;
	}
}

float RadiusFromBounds(vec3_t mins, vec3_t maxs)
{
	int i;
	vec3_t corner;

	for (i = 0; i < 3; i++)
		corner[i] = fabs(mins[i]) > fabs(maxs[i]) ? fabs(mins[i]) : fabs(maxs[i]);

	return VectorLength(corner);
}

void Mod_LoadBrushModel(model_t *mod, void *buffer)
{
	dheader_t *header;
	dmodel_t *bm;

	mod->type = mod_brush;

	header = (dheader_t *)buffer;

	auto version = LittleLong(header->version);
	if (version != BSPVERSION)
	{
		g_pMetaHookAPI->SysError("Mod_LoadBrushModel: %s has wrong version number (%i should be %i)", mod->name, version, BSPVERSION);
		return;
	}

	auto mod_base = (byte *)header;

	for (int i = 0; i < sizeof(dheader_t) / 4; i++)
		((int *)header)[i] = LittleLong(((int *)header)[i]);

	Mod_LoadVertexes(mod, mod_base, &header->lumps[LUMP_VERTEXES]);
	Mod_LoadEdges(mod, mod_base, &header->lumps[LUMP_EDGES]);
	Mod_LoadSurfedges(mod, mod_base, &header->lumps[LUMP_SURFEDGES]);

	if (!stricmp(gEngfuncs.pfnGetGameDirectory(), "bshift"))
	{
		Mod_LoadEntities(mod, mod_base, &header->lumps[LUMP_PLANES]);
		//Mod_LoadTextures(&header->lumps[LUMP_TEXTURES]);
		Mod_LoadLighting(mod, mod_base, &header->lumps[LUMP_LIGHTING]);
		Mod_LoadPlanes(mod, mod_base, &header->lumps[LUMP_ENTITIES]);
	}
	else
	{
		Mod_LoadEntities(mod, mod_base, &header->lumps[LUMP_ENTITIES]);
		//Mod_LoadTextures(&header->lumps[LUMP_TEXTURES]);
		Mod_LoadLighting(mod, mod_base, &header->lumps[LUMP_LIGHTING]);
		Mod_LoadPlanes(mod, mod_base, &header->lumps[LUMP_PLANES]);
	}

	Mod_LoadTexinfo(mod, mod_base, &header->lumps[LUMP_TEXINFO]);
	Mod_LoadFaces(mod, mod_base, &header->lumps[LUMP_FACES]);
	Mod_LoadMarksurfaces(mod, mod_base, &header->lumps[LUMP_MARKSURFACES]);
	Mod_LoadVisibility(mod, mod_base, &header->lumps[LUMP_VISIBILITY]);
	Mod_LoadLeafs(mod, mod_base, &header->lumps[LUMP_LEAFS]);
	Mod_LoadNodes(mod, mod_base, &header->lumps[LUMP_NODES]);
	Mod_LoadClipnodes(mod, mod_base, &header->lumps[LUMP_CLIPNODES]);
	Mod_LoadSubmodels(mod, mod_base, &header->lumps[LUMP_MODELS]);
	Mod_MakeHull0(mod, mod_base);

	mod->numframes = 2;
	mod->flags = 0;

	auto previous_mod = mod;

	for (int i = 0; i < mod->numsubmodels; i++)
	{
		bm = &mod->submodels[i];

		mod->hulls[0].firstclipnode = bm->headnode[0];

		for (int j = 1; j < MAX_MAP_HULLS; j++)
		{
			mod->hulls[j].firstclipnode = bm->headnode[j];
			mod->hulls[j].lastclipnode = mod->numclipnodes - 1;
		}

		mod->firstmodelsurface = bm->firstface;
		mod->nummodelsurfaces = bm->numfaces;

		VectorCopy(bm->maxs, mod->maxs);
		VectorCopy(bm->mins, mod->mins);

		mod->radius = RadiusFromBounds(mod->mins, mod->maxs);
		mod->numleafs = bm->visleafs;

		if (i < mod->numsubmodels - 1)
		{
			char name[10];

			snprintf(name, sizeof(name), "*%i", i + 1);
			auto submod = IEngineStudio.Mod_ForName(name, false);

			memcpy(submod, previous_mod, sizeof(model_t));
			strncpy(submod->name, name, sizeof(submod->name) - 1);
			submod->name[sizeof(submod->name) - 1] = 0;

			previous_mod = submod;
		}
	}
}

#endif