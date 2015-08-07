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

#include "paused.h"
#include "audio.h"
#include "ogl.h"
#include "view.h"
#include "course_render.h"
#include "env.h"
#include "hud.h"
#include "gui.h"
#include "font.h"
#include "track_marks.h"
#include "particles.h"
#include "translation.h"
#include "textures.h"
#include "game_ctrl.h"
#include "tux.h"
#include "racing.h"
#include "winsys.h"
#include "physics.h"
#include "reset.h"
#include "game_over.h"

CPaused Paused;

static bool sky = true;
static bool fog = true;
static bool terr = true;
static bool trees = true;

static TTextButton* textbuttons[3];

void CPaused::Keyb (unsigned int key, bool special, bool release, int x, int y) {
	if (release) return;
	switch (key) {
		case SDLK_s:
			ScreenshotN ();
			break;
		case SDLK_F5:
			sky = !sky;
			break;
		case SDLK_F6:
			fog = !fog;
			break;
		case SDLK_F7:
			terr = !terr;
			break;
		case SDLK_F8:
			trees = !trees;
			break;
		default:
			State::manager.RequestEnterState (Racing);
	}
}

void CPaused::Mouse (int button, int state, int x, int y) {
	ClickGUI(state, x, y);
	if (state == 0) {
		if (textbuttons[0]->focussed())
			State::manager.RequestEnterState(Racing);
		else if (textbuttons[1]->focussed())
			State::manager.RequestEnterState(Reset);
		else if (textbuttons[2]->focussed()) {
			g_game.raceaborted = true;
			g_game.race_result = -1;
			State::manager.RequestEnterState(GameOver);
		}
	}
}

void CPaused::Enter () {
	InitViewFrustum ();
	ResetGUI ();
	int siz = FT.AutoSizeN (6);
	int dist = FT.AutoDistanceN (2);
	int top = (Winsys.resolution.height - (dist * 3)) / 2;
	textbuttons[0] = AddTextButton (Trans.Text(9), CENTER, top, siz);
	textbuttons[1] = AddTextButton (Trans.Text(91), CENTER, top + dist, siz);
	textbuttons[2] = AddTextButton (Trans.Text(5), CENTER, top + dist * 2, siz);
}
// ====================================================================

void CPaused::Loop () {
	CControl *ctrl = g_game.player->ctrl;
	int width = Winsys.resolution.width;
	int height = Winsys.resolution.height;
	check_gl_error();

	Music.Update ();
	ClearRenderContext ();
	Env.SetupFog ();
	update_view (ctrl, 0);
	SetupViewFrustum (ctrl);

	if (sky) Env.DrawSkybox (ctrl->viewpos);
	if (fog) Env.DrawFog ();
	Env.SetupLight ();
	if (terr) RenderCourse();
	DrawTrackmarks ();
	if (trees) DrawTrees();

	DrawSnow (ctrl);

	if (param.perf_level > 2) draw_particles (ctrl);
	g_game.character->shape->Draw();

	DrawHud (ctrl);

	int h = FT.AutoDistanceN(2) * 3;
	int t = (Winsys.resolution.height - h) / 2;
	DrawFrameX ((Winsys.resolution.width - 180) / 2, t, 180, h, 3, colMBackgr, colWhite, 0.5);
	DrawGUI();

	Reshape (width, height);
	Winsys.SwapBuffers ();
}
