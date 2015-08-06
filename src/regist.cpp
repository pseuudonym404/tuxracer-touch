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

#include "regist.h"
#include "ogl.h"
#include "textures.h"
#include "audio.h"
#include "gui.h"
#include "particles.h"
#include "font.h"
#include "game_ctrl.h"
#include "translation.h"
#include "game_type_select.h"
#include "newplayer.h"
#include "winsys.h"
#include "delplayer.h"

CRegist Regist;

static TWidget* textbuttons[3];
static TUpDown* player;
static TUpDown* character;

void QuitRegistration () {
	Players.ResetControls ();
	Players.AllocControl (player->GetValue());
	g_game.player = Players.GetPlayer(player->GetValue());
	g_game.player->character = character->GetValue();
	g_game.character = &Char.CharList[character->GetValue()];
	g_game.start_character = character->GetValue();
	g_game.start_player = player->GetValue();
	//Char.FreeCharacterPreviews(); // From here on, character previews are no longer required
	State::manager.RequestEnterState (GameTypeSelect);
}

void CRegist::Keyb (unsigned int key, bool special, bool release, int x, int y) {
	TWidget* focussed = KeyGUI(key, 0, release);
	if (release) return;
	switch (key) {
		case SDLK_ESCAPE:
			State::manager.RequestQuit();
			break;
		case SDLK_RETURN:
			if (focussed == textbuttons[1]) {
				g_game.player = Players.GetPlayer(player->GetValue());
				State::manager.RequestEnterState (NewPlayer);
			} else QuitRegistration ();
			break;
	}
}

void CRegist::Mouse (int button, int state, int x, int y) {
	TWidget* focussed = ClickGUI(state, x, y);
	if (state == 0) {
		if (focussed == textbuttons[0]) {
			g_game.player = Players.GetPlayer(player->GetValue());
			State::manager.RequestEnterState (NewPlayer);
		} else if (focussed == textbuttons[1]) {
			if (Players.numPlayers() > 1) {
				DelPlayer.player = player->GetValue();
				State::manager.RequestEnterState (DelPlayer);
			}
		} else if (focussed == textbuttons[2]) {
			QuitRegistration ();
		} else if (focussed == player) {
			TPlayer *pl = Players.GetPlayer(player->GetValue());
			character->SetValue(pl->character);
		}
		if (focussed) focussed->focus = false;
	}
}

void CRegist::Motion (int x, int y) {
	MouseMoveGUI(x, y);

	if (param.ui_snow) push_ui_snow (cursor_pos);
}

static int framewidth, frameheight, arrowwidth;
static TArea area;
static ETR_DOUBLE texsize;

void CRegist::Enter() {
	//Winsys.ShowCursor (!param.ice_cursor);
	Music.Play (param.menu_music, -1);

	framewidth = (int) (Winsys.scale * 280);
	frameheight = (int) (Winsys.scale * 50);
	arrowwidth = 120;
	int sumwidth = framewidth + arrowwidth;
	area = AutoAreaN (20, 80, sumwidth);
	texsize = 128 * Winsys.scale;

	int ptop = Winsys.resolution.width < Winsys.resolution.height ? 35 : 15;
	int ptop3 = Winsys.resolution.width < Winsys.resolution.height ? 50 : 45;

	ResetGUI ();
	player = AddUpDown(area.left + framewidth + 88, AutoYPosN(ptop), 0, (int)Players.numPlayers() - 1, (int)g_game.start_player);
	character = AddUpDown(area.left + framewidth + 88, AutoYPosN(ptop3), 0, (int)Char.CharList.size() - 1, (int)g_game.start_character);
	int siz = FT.AutoSizeN (5);
	int dist = FT.AutoDistanceN (2);
	int top = AutoYPosN (68);
	textbuttons[0] = AddTextButton (Trans.Text(61), CENTER, top, siz);
	textbuttons[1] = AddTextButton (Trans.Text(93), CENTER, top + dist, siz);
	textbuttons[2] = AddTextButton (Trans.Text(8), CENTER, top + dist * 2, siz);

	if (Char.CharList.empty())
		Winsys.Terminate(); // Characters are necessary - ETR is unusable otherwise
}

void CRegist::Loop () {
	int ww = Winsys.resolution.width;
	int hh = Winsys.resolution.height;
	Music.Update ();
	check_gl_error();
	ClearRenderContext ();
	ScopedRenderMode rm(GUI);
	SetupGuiDisplay ();
	TColor col;

	if (param.ui_snow) {
		update_ui_snow ();
		draw_ui_snow();
	}

	Tex.Draw (BOTTOM_LEFT, 0, hh - 256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);
	//Tex.Draw(T_TITLE_SMALL, CENTER, AutoYPosN(5), Winsys.scale);

	int ptop = Winsys.resolution.width < Winsys.resolution.height ? 30 : 8;
	int ptop2 = Winsys.resolution.width < Winsys.resolution.height ? 35 : 15;
	int ptop3 = Winsys.resolution.width < Winsys.resolution.height ? 50 : 45;
	int ptop4 = Winsys.resolution.width < Winsys.resolution.height ? 45 : 38;

	FT.AutoSizeN (3);
	FT.SetColor (colWhite);
	FT.DrawString (area.left + 80, AutoYPosN (ptop), Trans.Text(58));
	FT.DrawString (area.left + 80, AutoYPosN (ptop4), Trans.Text(59));

	FT.AutoSizeN (4);
	if (player->focussed()) col = colDYell;
	else col = colWhite;

	DrawFrameX (area.left + 80, AutoYPosN(ptop2), framewidth, frameheight, 3, colMBackgr, col, 1.0);
	FT.SetColor (col);
	FT.DrawString(area.left + 90, AutoYPosN(ptop2) + 2, Players.GetPlayer(player->GetValue())->name);
	if (Players.GetPlayerAvatarTexture(player->GetValue()))
		Players.GetPlayerAvatarTexture(player->GetValue())->DrawFrame(
			area.left - 60, AutoYPosN (ptop), texsize, texsize, 3, colWhite);

	if (character->focussed()) col = colDYell;
	else col = colWhite;
	DrawFrameX (area.left + 80, AutoYPosN(ptop3),
	            framewidth, frameheight, 3, colMBackgr, col, 1.0);
	FT.SetColor (col);
	FT.DrawString (area.left + 90,
	               AutoYPosN(ptop3) + 2, Char.CharList[character->GetValue()].name);
	if (Char.CharList[character->GetValue()].preview != NULL)
		Char.CharList[character->GetValue()].preview->DrawFrame(
		    area.left - 60,
		    AutoYPosN (ptop4), texsize, texsize, 3, colWhite);


	FT.SetColor (colWhite);
	DrawGUI();

	Winsys.SwapBuffers();
}
