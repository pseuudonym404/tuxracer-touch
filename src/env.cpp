/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tuxracer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "env.h"
#include "ogl.h"
#include "textures.h"
#include "spx.h"
#include "view.h"
#include "course.h"

// --------------------------------------------------------------------
//					defaults
// --------------------------------------------------------------------

static const float def_amb[]     = {0.45, 0.53, 0.75, 1.0};
static const float def_diff[]    = {1.0, 0.9, 1.0, 1.0};
static const float def_spec[]    = {0.6, 0.6, 0.6, 1.0};
static const float def_pos[]     = {1, 2, 2, 0.0};
static const float def_fogcol[]  = {0.9, 0.9, 1.0, 0.0};
static const TColor def_partcol    (0.8, 0.8, 0.9, 0.0);

void TLight::Enable(GLenum num) const {
	glLightfv(num, GL_POSITION, position);
	glLightfv(num, GL_AMBIENT, ambient);
	glLightfv(num, GL_DIFFUSE, diffuse);
	glLightfv(num, GL_SPECULAR, specular);
	glEnable(num);
}

CEnvironment Env;

CEnvironment::CEnvironment () {
	EnvID = -1;
	lightcond[0] = "sunny";
	lightcond[1] = "cloudy";
	lightcond[2] = "evening";
	lightcond[3] = "night";
	for (size_t i = 0; i < 4; i++)
		LightIndex[lightcond[i]] = i;
	Skybox = NULL;

	default_light.is_on = true;
	for (int i=0; i<4; i++) {
		default_light.ambient[i]  = def_amb[i];
		default_light.diffuse[i]  = def_diff[i];
		default_light.specular[i] = def_spec[i];
		default_light.position[i] = def_pos[i];
	}

	default_fog.is_on = true;
	default_fog.mode = GL_LINEAR;
	default_fog.start = 20.0;
	default_fog.end = 70.0;
	default_fog.height = 0;
	for (int i=0; i<4; i++)
		default_fog.color[i] = def_fogcol[i];
	default_fog.part_color = def_partcol;
}

void CEnvironment::ResetSkybox () {
	delete[] Skybox;
	Skybox = NULL;
}

void CEnvironment::SetupLight () {
	lights[0].Enable(GL_LIGHT0);
	if (lights[1].is_on)
		lights[1].Enable(GL_LIGHT1);
	if (lights[2].is_on)
		lights[2].Enable(GL_LIGHT2);
	if (lights[3].is_on)
		lights[3].Enable(GL_LIGHT3);

	glEnable(GL_LIGHTING);
}

void CEnvironment::SetupFog () {
	glEnable (GL_FOG);
	glFogi   (GL_FOG_MODE, fog.mode);
	glFogf   (GL_FOG_START, fog.start);
	glFogf   (GL_FOG_END, fog.end);
	glFogfv  (GL_FOG_COLOR, fog.color);

	if (param.perf_level > 1) {
		glHint (GL_FOG_HINT, GL_NICEST);
	} else {
		glHint (GL_FOG_HINT, GL_FASTEST);
	}
}

void CEnvironment::ResetLight () {
	lights[0] = default_light;
	for (int i=1; i<4; i++) lights[i].is_on = false;
	glDisable (GL_LIGHT1);
	glDisable (GL_LIGHT2);
	glDisable (GL_LIGHT3);
}

void CEnvironment::ResetFog () {
	fog = default_fog;
}

void CEnvironment::Reset () {
	EnvID = -1;
	ResetSkybox ();
	ResetLight ();
	ResetFog ();
}

bool CEnvironment::LoadEnvironmentList () {
	CSPList list (32, true);
	if (!list.Load (param.env_dir2, "environment.lst")) {
		Message ("could not load environment.lst");
		return false;
	}

	locs.resize(list.Count());
	for (size_t i=0; i<list.Count(); i++) {
		const string& line = list.Line(i);
		locs[i].name = SPStrN(line, "location");
		locs[i].high_res = false; // don't use highres textures
		//locs[i].high_res = SPBoolN(line, "high_res", false);
	}
	list.MakeIndex (EnvIndex, "location");
	return true;
}

string CEnvironment::GetDir (size_t location, size_t light) const {
	if (location >= locs.size()) return "";
	if (light >= 4) return "";
	string res =
	    param.env_dir2 + SEP +
	    locs[location].name + SEP + lightcond[light];
	return res;
}

void CEnvironment::LoadSkyboxSide(size_t index, const string& EnvDir, const string& name, bool high_res) {
	bool loaded = false;
	if (param.perf_level > 3 && high_res)
		loaded = Skybox[index].Load(EnvDir, name + "H.png");

	if (!loaded)
		Skybox[index].Load(EnvDir, name + ".png");
}

void CEnvironment::LoadSkybox (const string& EnvDir, bool high_res) {
	Skybox = new TTexture[/*param.full_skybox ? 6 :*/ 3];
	LoadSkyboxSide(0, EnvDir, "front", high_res);
	LoadSkyboxSide(1, EnvDir, "left", high_res);
	LoadSkyboxSide(2, EnvDir, "right", high_res);
	// Can't see these
	/*if (param.full_skybox) {
		LoadSkyboxSide(3, EnvDir, "top", high_res);
		LoadSkyboxSide(4, EnvDir, "bottom", high_res);
		LoadSkyboxSide(5, EnvDir, "back", high_res);
	}*/
}

void CEnvironment::LoadLight (const string& EnvDir) {
	static const string idxstr = "[fog]-1[0]0[1]1[2]2[3]3[4]4[5]5[6]6";

	CSPList list(24);
	if (!list.Load (EnvDir, "light.lst")) {
		Message ("could not load light file");
		return;
	}

	for (size_t i=0; i<list.Count(); i++) {
		const string& line = list.Line(i);
		string item = SPStrN (line, "light", "none");
		int idx = SPIntN (idxstr, item, -1);
		if (idx < 0) {
			fog.is_on = SPBoolN (line, "fog", true);
			fog.start = SPFloatN (line, "fogstart", 20);
			fog.end = SPFloatN (line, "fogend", param.forward_clip_distance);
			fog.height = SPFloatN (line, "fogheight", 0);
			SPArrN (line, "fogcol", fog.color, 4, 1);
			fog.part_color = SPColorN (line, "partcol", def_partcol);
		} else if (idx < 4) {
			lights[idx].is_on = true;
			SPArrN (line, "amb", lights[idx].ambient, 4, 1);
			SPArrN (line, "diff", lights[idx].diffuse, 4, 1);
			SPArrN (line, "spec", lights[idx].specular, 4, 1);
			SPArrN (line, "pos", lights[idx].position, 4, 1);
		}
	}
}

void CEnvironment::DrawSkybox (const TVector3d& pos) {
	ScopedRenderMode rm(SKY);

	glColor4f (1.0, 1.0, 1.0, 1.0);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glPushMatrix();
	glTranslate(pos);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	BindGlobalVBO();


	Skybox[0].Bind();
	RenderGlobalVBO(GL_TRIANGLE_FAN,4,SKYBOX_FRONT);

	Skybox[1].Bind();
	RenderGlobalVBO(GL_TRIANGLE_FAN,4,SKYBOX_LEFT);

	Skybox[2].Bind();
	RenderGlobalVBO(GL_TRIANGLE_FAN,4,SKYBOX_RIGHT);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	UnbindVBO();
	glPopMatrix();
}

void CEnvironment::DrawFog () {
	if (!fog.is_on)
		return;

	TPlane bottom_plane, top_plane;
	TVector3d left, right, vpoint;
	TVector3d topleft, topright;
	TVector3d bottomleft, bottomright;

	// the clipping planes are calculated by view frustum (view.cpp)
	const TPlane& leftclip = get_left_clip_plane ();
	const TPlane& rightclip = get_right_clip_plane ();
	const TPlane& farclip = get_far_clip_plane ();
	const TPlane& bottomclip = get_bottom_clip_plane ();

	// --------------- calculate the planes ---------------------------

	ETR_DOUBLE slope = tan (ANGLES_TO_RADIANS (Course.GetCourseAngle()));
//	TPlane left_edge_plane = MakePlane (1.0, 0.0, 0.0, 0.0);
//	TPlane right_edge_plane = MakePlane (-1.0, 0.0, 0.0, Course.width);

	bottom_plane.nml = TVector3d(0.0, 1, -slope);
	float height = Course.GetBaseHeight (0);
	bottom_plane.d = -height * bottom_plane.nml.y;

	top_plane.nml = bottom_plane.nml;
	height = Course.GetMaxHeight (0) + fog.height;
	top_plane.d = -height * top_plane.nml.y;


	if (!IntersectPlanes (bottom_plane, farclip, leftclip,  &left)) return;
	if (!IntersectPlanes (bottom_plane, farclip, rightclip, &right)) return;
	if (!IntersectPlanes (top_plane,    farclip, leftclip,  &topleft)) return;
	if (!IntersectPlanes (top_plane,    farclip, rightclip, &topright)) return;
	if (!IntersectPlanes (bottomclip,   farclip, leftclip,  &bottomleft)) return;
	if (!IntersectPlanes (bottomclip,   farclip, rightclip, &bottomright)) return;

	// --------------- draw the fog plane -----------------------------

	ScopedRenderMode rm(FOG_PLANE);
	glEnable (GL_FOG);

#ifdef USE_GLES1
	GLfloat vtx1[] = {
		bottomleft.x, bottomleft.y, bottomleft.z,
		0, 0, 0, 1.0,
		bottomright.x, bottomright.y, bottomright.z,
		0, 0, 0, 1.0,
		left.x, left.y, left.z,
		0, 0, 0, 1.0,
		right.x, right.y, right.z,
		0, 0, 0, 1.0,
		topleft.x, topleft.y, topleft.z,
		0, 0, 0, 0.9,
		topright.x, topright.y, topright.z,
		0, 0, 0, 0.9,
		topleft.x+(topleft.x - left.x), topleft.y+(topleft.y - left.y), topleft.z+(topleft.z - left.z),
		0, 0, 0, 0.3,
		topright.x+(topright.x - right.x), topright.y+(topright.y - right.y), topright.z+(topright.z - right.z),
		0, 0, 0, 0.3,
		topleft.x+3.0*(topleft.x - left.x), topleft.y+3.0*(topleft.y - left.y), topleft.z+3.0*(topleft.z - left.z),
		0, 0, 0, 0.0,
		topright.x+3.0*(topright.x - right.x), topright.y+3.0*(topright.y - right.y), topright.z+3.0*(topright.z - right.z),
		0, 0, 0, 0.0
	};

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, 7*sizeof(GLfloat), vtx1);
	glColorPointer(4, GL_FLOAT, 7*sizeof(GLfloat), vtx1+3);
	glDrawArrays(GL_TRIANGLE_STRIP,0,10);
 
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
#else
	TVector3d leftvec  = topleft - left;
	TVector3d rightvec = topright - right;

	// only the alpha channel is used
	static const GLfloat bottom_dens[4]     = {0, 0, 0, 1.0};
	static const GLfloat top_dens[4]        = { 0, 0, 0, 0.9 };
	static const GLfloat leftright_dens[4]  = { 0, 0, 0, 0.3 };
	static const GLfloat top_bottom_dens[4] = { 0, 0, 0, 0.0 };

	glBegin (GL_QUAD_STRIP);
	glColor4fv (bottom_dens);
	glVertex3(bottomleft);
	glVertex3(bottomright);
	glVertex3(left);
	glVertex3(right);

	glColor4fv (top_dens);
	glVertex3(topleft);
	glVertex3(topright);

	glColor4fv (leftright_dens);
	vpoint = topleft + leftvec;
	glVertex3(vpoint);
	vpoint = topright + rightvec;
	glVertex3(vpoint);

	glColor4fv (top_bottom_dens);
	vpoint = topleft + 3.0 * leftvec;
	glVertex3(vpoint);
	vpoint = topright + 3.0 * rightvec;
	glVertex3(vpoint);
	glEnd();
#endif
}


void CEnvironment::LoadEnvironment (size_t loc, size_t light) {
	if (loc >= locs.size()) loc = 0;
	if (light >= 4) light = 0;
	// remember: with (example) 3 locations and 4 lights there
	// are 12 different environments
	size_t env_id = loc * 100 + light;

	if (env_id == EnvID)
		return; // Already loaded
	EnvID = env_id;

	// Set directory. The dir is used several times.
	string EnvDir = GetDir (loc, light);

	// Load skybox. If the sky can't be loaded for any reason, the
	// texture id's are set to 0 and the sky will not be drawn.
	// There is no error handling, you see the result on the screen.
	ResetSkybox ();
	LoadSkybox(EnvDir, locs[loc].high_res);

	// Load light conditions.
	ResetFog ();
	ResetLight ();
	LoadLight (EnvDir);
}

size_t CEnvironment::GetEnvIdx (const string& tag) const {
	return EnvIndex.at(tag);
}

size_t CEnvironment::GetLightIdx (const string& tag) const {
	return LightIndex.at(tag);
}
