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

#include "hud.h"
#include "ogl.h"
#include "textures.h"
#include "spx.h"
#include "particles.h"
#include "font.h"
#include "course.h"
#include "physics.h"
#include "winsys.h"


#define GAUGE_IMG_SIZE 128
#define ENERGY_GAUGE_BOTTOM 3.0
#define ENERGY_GAUGE_HEIGHT 103.0
#define ENERGY_GAUGE_CENTER_X 71.0
#define ENERGY_GAUGE_CENTER_Y 55.0
#define GAUGE_WIDTH 128.0
#define SPEEDBAR_OUTER_RADIUS  (ENERGY_GAUGE_CENTER_X)
#define SPEEDBAR_BASE_ANGLE 225
#define SPEEDBAR_MAX_ANGLE 45
#define SPEEDBAR_GREEN_MAX_SPEED  (MAX_PADDLING_SPEED * 3.6)
#define SPEEDBAR_YELLOW_MAX_SPEED 100
#define SPEEDBAR_RED_MAX_SPEED 160
#define SPEEDBAR_GREEN_FRACTION 0.5
#define SPEEDBAR_YELLOW_FRACTION 0.25
#define SPEEDBAR_RED_FRACTION 0.25
#define CIRCLE_DIVISIONS 10

static const GLfloat energy_background_color[] = { 0.2, 0.2, 0.2, 0.0 };
static const GLfloat energy_foreground_color[] = { 0.54, 0.59, 1.00, 0.5 };
static const GLfloat speedbar_background_color[] = { 0.2, 0.2, 0.2, 0.0 };
static const GLfloat hud_white[] = { 1.0, 1.0, 1.0, 1.0 };

static const TColor text_colour(0, 0, 0, 1);

static void draw_time() {
	int min, sec, hundr;
	GetTimeComponents (g_game.time, &min, &sec, &hundr);
	string timestr = Int_StrN (min, 2);
	string secstr = Int_StrN (sec, 2);
	string hundrstr = Int_StrN (hundr, 2);

	timestr += ":";
	timestr += secstr;

	if (param.use_papercut_font < 2) {
		Tex.Draw (CHRON_ICON, 0, -4, 1);
		Tex.DrawNumStr (timestr, 60, 10, 1, colWhite);
		Tex.DrawNumStr (hundrstr, 176, 10, 0.7, colWhite);
	} else {

		/*
			glEnable (GL_LINE_SMOOTH);
			glEnable (GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
			glLineWidth (1.5);
		*/
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

		//Tex.Draw (T_TIME, 16, 20, 1);
		FT.SetColor (colDYell);
		FT.SetSize (32);
		FT.DrawString (160, 6, hundrstr);
		FT.SetSize (42);
		FT.DrawString (70, 10, timestr);
	}
}

static void draw_herring_count (int herring_count) {
	string hcountstr = Int_StrN (herring_count, 3);
	if (param.use_papercut_font < 2) {
		Tex.Draw (HERRING_ICON, Winsys.resolution.width - 145, -4, 1);
		Tex.DrawNumStr (hcountstr, Winsys.resolution.width - 80, 10, 1, colWhite);
	} else {
		FT.SetColor (colDYell);
		FT.DrawString ( Winsys.resolution.width - 90, 10, hcountstr);
		Tex.Draw (T_YELLHERRING, Winsys.resolution.width-160, 12, 1);
	}
}

#if 0
TVector2d calc_new_fan_pt (ETR_DOUBLE angle) {
	TVector2d pt;
	pt.x = ENERGY_GAUGE_CENTER_X + cos (ANGLES_TO_RADIANS (angle)) * SPEEDBAR_OUTER_RADIUS;
	pt.y = ENERGY_GAUGE_CENTER_Y + sin (ANGLES_TO_RADIANS (angle)) * SPEEDBAR_OUTER_RADIUS;
	return pt;
}

void start_tri_fan() {
	glBegin (GL_TRIANGLE_FAN);
#ifdef USE_GLES1
	glVertex2f (ENERGY_GAUGE_CENTER_X, 
		ENERGY_GAUGE_CENTER_Y);
	glTexCoord2f(ENERGY_GAUGE_CENTER_X/GAUGE_IMG_SIZE, ENERGY_GAUGE_CENTER_Y/GAUGE_IMG_SIZE);
	TVector2d pt = calc_new_fan_pt (SPEEDBAR_BASE_ANGLE); 
	glVertex2f (pt.x, pt.y);
	glTexCoord2f(pt.x/GAUGE_IMG_SIZE, pt.y/GAUGE_IMG_SIZE);
#else
	glVertex2f (ENERGY_GAUGE_CENTER_X, 
		ENERGY_GAUGE_CENTER_Y);
	TVector2d pt = calc_new_fan_pt (SPEEDBAR_BASE_ANGLE); 
	glVertex2f (pt.x, pt.y);
#endif
}

void draw_partial_tri_fan (ETR_DOUBLE fraction) {
	bool trifan = false;

	ETR_DOUBLE angle = SPEEDBAR_BASE_ANGLE + 
		(SPEEDBAR_MAX_ANGLE - SPEEDBAR_BASE_ANGLE) * fraction;

	int divs = (int)((SPEEDBAR_BASE_ANGLE - angle) * CIRCLE_DIVISIONS / 360.0);
	ETR_DOUBLE cur_angle = SPEEDBAR_BASE_ANGLE;
	ETR_DOUBLE angle_incr = 360.0 / CIRCLE_DIVISIONS;

	for (int i=0; i<divs; i++) {
		if (!trifan) {
			start_tri_fan();
			trifan = true;
		}
		cur_angle -= angle_incr;
		TVector2d pt = calc_new_fan_pt (cur_angle);
		glVertex2f (pt.x, pt.y);
#ifdef USE_GLES1
		glTexCoord2f(pt.x/GAUGE_IMG_SIZE, pt.y/GAUGE_IMG_SIZE);
#endif
	}

	if (cur_angle > angle + EPS) {
		cur_angle = angle;
		if (!trifan) {
			start_tri_fan();
			trifan = true;
		}
		TVector2d pt = calc_new_fan_pt (cur_angle);
		glVertex2f (pt.x, pt.y);
#ifdef USE_GLES1
		glTexCoord2f(pt.x/GAUGE_IMG_SIZE, pt.y/GAUGE_IMG_SIZE);
#endif
	}

	if (trifan) {
		glEnd();
		trifan = false;
	}
}

void draw_gauge (ETR_DOUBLE speed, ETR_DOUBLE energy) {
#ifndef USE_GLES1
	static const GLfloat xplane[4] = {1.0 / GAUGE_IMG_SIZE, 0.0, 0.0, 0.0 };
	static const GLfloat yplane[4] = {0.0, 1.0 / GAUGE_IMG_SIZE, 0.0, 0.0 };
#endif
	ScopedRenderMode rm(GAUGE_BARS);

	if (Tex.GetTexture (GAUGE_ENERGY) == NULL) return;
	if (Tex.GetTexture (GAUGE_SPEED) == NULL) return;
	if (Tex.GetTexture (GAUGE_OUTLINE) == NULL) return;

	Tex.BindTex (GAUGE_ENERGY);
#ifndef USE_GLES1
	glTexGenfv (GL_S, GL_OBJECT_PLANE, xplane);
	glTexGenfv (GL_T, GL_OBJECT_PLANE, yplane);
#endif
	glPushMatrix();
	glTranslatef (Winsys.resolution.width - GAUGE_WIDTH, 0, 0);
	Tex.BindTex (GAUGE_ENERGY);
	ETR_DOUBLE y = ENERGY_GAUGE_BOTTOM + energy * ENERGY_GAUGE_HEIGHT;
	
#ifdef USE_GLES1
	const GLfloat tex1 [] = {
		0.0, y/GAUGE_IMG_SIZE,
		1.0, y/GAUGE_IMG_SIZE,
		1.0, 1.0f,
		0.0, 1.0f
	};
	const GLfloat tex2 [] = {
		0.0, 0.0f,
		1.0, 0.0f,
		1.0, y/GAUGE_IMG_SIZE,
		0.0, y/GAUGE_IMG_SIZE
	};
#endif
	const GLfloat vtx1 [] = {
		0.0, y,
		GAUGE_IMG_SIZE, y,
		GAUGE_IMG_SIZE, GAUGE_IMG_SIZE,
		0.0, GAUGE_IMG_SIZE
	};
	const GLfloat vtx2 [] = {
		0.0, 0.0,
		GAUGE_IMG_SIZE, 0.0,
		GAUGE_IMG_SIZE, y,
		0.0, y
	};
	glEnableClientState(GL_VERTEX_ARRAY);
#ifdef USE_GLES1
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#endif

	glColor4fv(energy_background_color);
	glVertexPointer(2, GL_FLOAT, 0, vtx1);
#ifdef USE_GLES1
	glTexCoordPointer(2, GL_FLOAT, 0, tex1);
#endif
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glColor4fv(energy_foreground_color);
	glVertexPointer(2, GL_FLOAT, 0, vtx2);
#ifdef USE_GLES1
	glTexCoordPointer(2, GL_FLOAT, 0, tex2);
#endif
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
#ifdef USE_GLES1
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif

	ETR_DOUBLE speedbar_frac = 0.0;

	if (speed > SPEEDBAR_GREEN_MAX_SPEED) {
		speedbar_frac = SPEEDBAR_GREEN_FRACTION;

		if (speed > SPEEDBAR_YELLOW_MAX_SPEED) {
			speedbar_frac += SPEEDBAR_YELLOW_FRACTION;
			if (speed > SPEEDBAR_RED_MAX_SPEED) {
				speedbar_frac += SPEEDBAR_RED_FRACTION;
			} else {
				speedbar_frac += (speed - SPEEDBAR_YELLOW_MAX_SPEED) /
						(SPEEDBAR_RED_MAX_SPEED - SPEEDBAR_YELLOW_MAX_SPEED) * SPEEDBAR_RED_FRACTION;
			}
		} else {
			speedbar_frac += (speed - SPEEDBAR_GREEN_MAX_SPEED) /
					(SPEEDBAR_YELLOW_MAX_SPEED - SPEEDBAR_GREEN_MAX_SPEED) * SPEEDBAR_YELLOW_FRACTION;
		}	    
	} else {
		speedbar_frac +=  speed/SPEEDBAR_GREEN_MAX_SPEED * SPEEDBAR_GREEN_FRACTION;
	}

	glColor4fv (speedbar_background_color);
	Tex.BindTex (GAUGE_SPEED);
	draw_partial_tri_fan (1.0);
	glColor4fv (hud_white);
	draw_partial_tri_fan (min (1.0, speedbar_frac));

	glColor4fv (hud_white);
	Tex.BindTex (GAUGE_OUTLINE);
#ifdef USE_GLES1
	static const GLshort tex3 [] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1
	};
#endif
	static const GLshort vtx3 [] = {
		0, 0,
		GAUGE_IMG_SIZE, 0,
		GAUGE_IMG_SIZE, GAUGE_IMG_SIZE,
		0, GAUGE_IMG_SIZE
	};
	glEnableClientState(GL_VERTEX_ARRAY);
#ifdef USE_GLES1
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#endif

	glVertexPointer(2, GL_SHORT, 0, vtx3);
#ifdef USE_GLES1
	glTexCoordPointer(2, GL_SHORT, 0, tex3);
#endif
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
#ifdef USE_GLES1
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
	glPopMatrix();
}
#endif

void DrawSpeed (ETR_DOUBLE speed) {
	string speedstr = Int_StrN ((int)speed, 3);
	if (param.use_papercut_font < 2) {
		Tex.Draw (GAUGE_ICON, Winsys.resolution.width - 145, Winsys.resolution.height - 60, 1);
		Tex.DrawNumStr (speedstr, Winsys.resolution.width - 80, Winsys.resolution.height - 46, 1, colWhite);
	} else {
		FT.SetColor (colDDYell);
		FT.DrawString (Winsys.resolution.width-82, Winsys.resolution.height-80, speedstr);
	}
}

void DrawWind(float dir, float speed, const CControl *ctrl) {
	if (g_game.wind_id < 1) return;

	//float dir_angle = RADIANS_TO_ANGLES(atan2(ctrl->cvel.x, ctrl->cvel.z));
	string windstr = Int_StrN ((int)speed, 3);

	glPushMatrix();
	glTranslatef (32, 28, 0);
	glRotatef(dir, 0, 0, 1);
	glTranslatef (-32, -28, 0);
	Tex.Draw(ARROW_ICON, 0, Winsys.resolution.height - 60, 1);
	glPopMatrix();

	Tex.DrawNumStr(windstr, 60, Winsys.resolution.height - 46, 1, colWhite);

#if 0
	if (g_game.wind_id < 1) return;

	Tex.Draw (SPEEDMETER, 0, Winsys.resolution.height-140, 1.0);
	glDisable (GL_TEXTURE_2D);


	float alpha, red, blue;
	if (speed <= 50) {
		alpha = speed / 50;
		red = 0;
	} else {
		alpha = 1.0;
		red = (speed - 50) / 50;
	}
	blue = 1.0 - red;

	glPushMatrix ();
	glColor4f (red, 0, blue, alpha);
	glTranslatef (72, 66, 0);
	glRotatef(dir, 0, 0, 1);
	glEnableClientState(GL_VERTEX_ARRAY);
	static const int len = 45;
	static const GLshort vtx1 [] = {
		-5, 0.0,
		5, 0.0,
		5, -len,
		- 5, -len
	};
	glVertexPointer(2, GL_SHORT, 0, vtx1);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// direction indicator
	float dir_angle = RADIANS_TO_ANGLES(atan2(ctrl->cvel.x, ctrl->cvel.z));

	glColor4f (0, 0.5, 0, 1.0);
	glRotatef (dir_angle - dir, 0, 0, 1);
	static const GLshort vtx2 [] = {
		-2, 0,
		2, 0,
		2, -50,
		-2, -50
	};
	glVertexPointer(2, GL_SHORT, 0, vtx2);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix ();

	glEnable (GL_TEXTURE_2D);

	Tex.Draw (SPEED_KNOB, 64, Winsys.resolution.height - 74, 1.0);
	string windstr = Int_StrN ((int)speed, 3);
	if (param.use_papercut_font < 2) {
		Tex.DrawNumStr (windstr, 130, Winsys.resolution.height - 55, 1, colWhite);
	} else {
		FT.SetColor (colBlue);
		FT.DrawString (130, Winsys.resolution.height - 55, windstr);
	}
#endif
}

void DrawFps() {
	const  int   maxFrames = 50;
	static int   numFrames = 0;
	static float averagefps = 0;
	static float sumTime = 0;

	if (!param.display_fps)
		return;

	if (numFrames >= maxFrames) {
		averagefps = 1 / sumTime * maxFrames;
		numFrames = 0;
		sumTime = 0;
	} else {
		sumTime += g_game.time_step;
		numFrames++;
	}
	if (averagefps < 1) return;

	string fpsstr = Int_StrN((int)averagefps);
	if (param.use_papercut_font < 2) {
		Tex.DrawNumStr (fpsstr, (Winsys.resolution.width - 60) / 2, 10, 1, colWhite);
	} else {
		if (averagefps >= 35)
			FT.SetColor (colWhite);
		else
			FT.SetColor (colRed);
		FT.DrawString ((Winsys.resolution.width - 60) / 2, 10, fpsstr);
	}
}

#if 0
void DrawPercentBar (float fact, float x, float y) {
	Tex.BindTex (T_ENERGY_MASK);
	glColor4f (1.0, 1.0, 1.0, 1.0);

	const GLfloat tex[] = {
		0, 0,
		1, 0,
		1, fact,
		0, fact
	};
	const GLfloat vtx[] = {
		x, y,
		x + 32, y,
		x + 32, y + fact * 128,
		x, y + fact * 128
	};

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vtx);
	glTexCoordPointer(2, GL_FLOAT, 0, tex);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}
#endif

void DrawCoursePosition (const CControl *ctrl) {
	ETR_DOUBLE fact = ctrl->cpos.z / Course.GetPlayDimensions().y;
	if (fact > 1.0) fact = 1.0;
	int top = (Winsys.resolution.height - 256) / 2;
	Tex.Draw (PROGRESS_TRACK, Winsys.resolution.width - 64, top, 1.0);
	int ttop = top + 224 + int(fact * 224);
	if (ttop < top + 4) ttop = top + 4;
	Tex.Draw (PROGRESS_TUX, Winsys.resolution.width - 50, ttop, 1.0);

#if 0
	glEnable (GL_TEXTURE_2D);
	DrawPercentBar (-fact, Winsys.resolution.width - 48, 280-128);
	Tex.Draw (T_MASK_OUTLINE, Winsys.resolution.width - 48, Winsys.resolution.height - 280, 1.0);
#endif
}

// -------------------------------------------------------
void DrawHud (const CControl *ctrl) {
	if (!param.show_hud)
		return;

	ETR_DOUBLE speed = ctrl->cvel.Length();
	SetupGuiDisplay ();

	//draw_gauge (speed * 3.6, ctrl->jump_amt);
	ScopedRenderMode rm(TEXFONT);
	draw_time();
	draw_herring_count (g_game.herring);
	DrawSpeed (speed * 3.6);
	DrawFps ();
	DrawCoursePosition (ctrl);
	DrawWind (Wind.Angle (), Wind.Speed (), ctrl);
}
