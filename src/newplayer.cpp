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

#include "newplayer.h"
#include "particles.h"
#include "audio.h"
#include "gui.h"
#include "ogl.h"
#include "textures.h"
#include "font.h"
#include "game_ctrl.h"
#include "translation.h"
#include "regist.h"
#include "winsys.h"
#include "spx.h"
#include <cctype>

CNewPlayer NewPlayer;

static TUpDown* avatar;
static TWidget* textbuttons[2];
static TTextField* keybuttons[38];
static TTextField* textfield;
static bool caps = false;

void QuitAndAddPlayer () {
	if (textfield->Text().size () > 0) {
		g_game.start_player = Players.numPlayers();
		g_game.start_character = 0;
		Players.AddPlayer (textfield->Text(), Players.GetDirectAvatarName(avatar->GetValue()));
	}
	State::manager.RequestEnterState (Regist);
}

void CNewPlayer::Keyb_spec (SDL_Keysym sym, bool release) {
	if (release) return;

	KeyGUI(sym.sym, sym.mod, release);
	switch (sym.sym) {
		case SDLK_ESCAPE:
			State::manager.RequestEnterState (Regist);
			break;
		case SDLK_RETURN:
			if (textbuttons[0]->focussed()) State::manager.RequestEnterState (Regist);
			else QuitAndAddPlayer ();
			break;
		default:
			break;
	}
}

void CNewPlayer::Mouse (int button, int state, int x, int y) {
	TWidget* clicked = ClickGUI(state, x, y);
	if (state == 0) {
		if (clicked == textbuttons[0]) State::manager.RequestEnterState (Regist);
		else if (clicked == textbuttons[1]) QuitAndAddPlayer();
		else if (clicked == keybuttons[36]) {
			caps = !caps;
			int off = caps ? 55 : 87;
			for (int i = 10; i < 36; ++i) {
				char ch[2] = { char(i + off), 0 };
				keybuttons[i]->SetText(ch);
			}
		} else if (clicked == keybuttons[37]) {
			string tmp = textfield->Text();
			if (tmp.length()) {
				tmp.resize(tmp.length() - 1);
				textfield->SetText(tmp);
			}
		} else for (int i = 0; i < 36; ++i) if (clicked == keybuttons[i])
			textfield->SetText(textfield->Text() + keybuttons[i]->Text());

		if (clicked) clicked->focus = false;
	}
	textfield->focus = true;
}

void CNewPlayer::Motion (int x, int y) {
	MouseMoveGUI(x, y);

	if (param.ui_snow) push_ui_snow (cursor_pos);
}

static int prevleft, prevtop, prevwidth;

void CNewPlayer::Enter() {
	Winsys.KeyRepeat (true);
	Winsys.ShowCursor (!param.ice_cursor);
	Music.Play (param.menu_music, -1);

	int framewidth = 280 * Winsys.scale;
	int frameheight = 50 * Winsys.scale;
	//int frametop = AutoYPosN (38);
	TArea area = AutoAreaN(20, 80, framewidth + 120);
	//int prevoffs = 80;
	prevleft = area.left - 60;
	prevtop = AutoYPosN(Winsys.resolution.width < Winsys.resolution.height ? 30 : 8);
	prevwidth = 128 * Winsys.scale;

	ResetGUI();

	int ptop = Winsys.resolution.width < Winsys.resolution.height ? 35 : 15;
	avatar = AddUpDown (area.left + framewidth + 88, AutoYPosN(ptop), 0, (int)Players.numAvatars() - 1, 0);

	int siz = FT.AutoSizeN (5);
	int ptop3 = Winsys.resolution.width < Winsys.resolution.height ? 80 : 85;
	textbuttons[0] = AddTextButton (Trans.Text(8), area.left+50, AutoYPosN (ptop3), siz);
	ETR_DOUBLE len = FT.GetTextWidth (Trans.Text(15));
	textbuttons[1] = AddTextButton (Trans.Text(15), area.right-len-50, AutoYPosN (ptop3), siz);

	int ptop2 = Winsys.resolution.width < Winsys.resolution.height ? 35 : 15;
	textfield = AddTextField(emptyString, area.left + 80, AutoYPosN(ptop2), framewidth, frameheight);
	textfield->focus = true;

	int ktop = AutoYPosN(Winsys.resolution.width < Winsys.resolution.height ? 52 : 42);
	int kleft = (Winsys.resolution.width - 500) / 2;
	keybuttons[0] = AddTextField("0", kleft, ktop, 48, 48);
	keybuttons[1] = AddTextField("1", kleft + 50, ktop, 48, 48);
	keybuttons[2] = AddTextField("2", kleft + 100, ktop, 48, 48);
	keybuttons[3] = AddTextField("3", kleft + 150, ktop, 48, 48);
	keybuttons[4] = AddTextField("4", kleft + 200, ktop, 48, 48);
	keybuttons[5] = AddTextField("5", kleft + 250, ktop, 48, 48);
	keybuttons[6] = AddTextField("6", kleft + 300, ktop, 48, 48);
	keybuttons[7] = AddTextField("7", kleft + 350, ktop, 48, 48);
	keybuttons[8] = AddTextField("8", kleft + 400, ktop, 48, 48);
	keybuttons[9] = AddTextField("9", kleft + 450, ktop, 48, 48);
	keybuttons[10] = AddTextField("a", kleft, ktop + 50, 48, 48);
	keybuttons[11] = AddTextField("b", kleft + 50, ktop + 50, 48, 48);
	keybuttons[12] = AddTextField("c", kleft + 100, ktop + 50, 48, 48);
	keybuttons[13] = AddTextField("d", kleft + 150, ktop + 50, 48, 48);
	keybuttons[14] = AddTextField("e", kleft + 200, ktop + 50, 48, 48);
	keybuttons[15] = AddTextField("f", kleft + 250, ktop + 50, 48, 48);
	keybuttons[16] = AddTextField("g", kleft + 300, ktop + 50, 48, 48);
	keybuttons[17] = AddTextField("h", kleft + 350, ktop + 50, 48, 48);
	keybuttons[18] = AddTextField("i", kleft + 400, ktop + 50, 48, 48);
	keybuttons[19] = AddTextField("j", kleft + 450, ktop + 50, 48, 48);
	keybuttons[20] = AddTextField("k", kleft, ktop + 100, 48, 48);
	keybuttons[21] = AddTextField("l", kleft + 50, ktop + 100, 48, 48);
	keybuttons[22] = AddTextField("m", kleft + 100, ktop + 100, 48, 48);
	keybuttons[23] = AddTextField("n", kleft + 150, ktop + 100, 48, 48);
	keybuttons[24] = AddTextField("o", kleft + 200, ktop + 100, 48, 48);
	keybuttons[25] = AddTextField("p", kleft + 250, ktop + 100, 48, 48);
	keybuttons[26] = AddTextField("q", kleft + 300, ktop + 100, 48, 48);
	keybuttons[27] = AddTextField("r", kleft + 350, ktop + 100, 48, 48);
	keybuttons[28] = AddTextField("s", kleft + 400, ktop + 100, 48, 48);
	keybuttons[29] = AddTextField("t", kleft + 450, ktop + 100, 48, 48);
	keybuttons[30] = AddTextField("u", kleft + 100, ktop + 150, 48, 48);
	keybuttons[31] = AddTextField("v", kleft + 150, ktop + 150, 48, 48);
	keybuttons[32] = AddTextField("w", kleft + 200, ktop + 150, 48, 48);
	keybuttons[33] = AddTextField("x", kleft + 250, ktop + 150, 48, 48);
	keybuttons[34] = AddTextField("y", kleft + 300, ktop + 150, 48, 48);
	keybuttons[35] = AddTextField("z", kleft + 350, ktop + 150, 48, 48);
	keybuttons[36] = AddTextField("caps", kleft, ktop + 150, 98, 48);
	keybuttons[37] = AddTextField("  <", kleft + 400, ktop + 150, 98, 48);
}

void CNewPlayer::Loop() {
	int ww = Winsys.resolution.width;
	int hh = Winsys.resolution.height;
	TColor col;

	Music.Update ();
	check_gl_error();
	ClearRenderContext ();
	ScopedRenderMode rm(GUI);
	SetupGuiDisplay ();

	if (param.ui_snow) {
		update_ui_snow ();
		draw_ui_snow();
	}

	textfield->UpdateCursor();

//	DrawFrameX (area.left, area.top, area.right-area.left, area.bottom - area.top,
//			0, colMBackgr, col, 0.2);

	Tex.Draw (BOTTOM_LEFT, 0, hh - 256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);
	//Tex.Draw (T_TITLE_SMALL, CENTER, AutoYPosN (5), Winsys.scale);

	FT.SetColor (colWhite);
	FT.AutoSizeN (3);
	FT.DrawString (CENTER, AutoYPosN (Winsys.resolution.width < Winsys.resolution.height ? 45 : 33), Trans.Text(66));

	if (avatar->focussed()) col = colDYell;
	else col = colWhite;
	Players.GetAvatarTexture(avatar->GetValue())->DrawFrame(
	    prevleft, prevtop, prevwidth, prevwidth, 2, col);

	textfield->focus = true;
	DrawGUI();
	Winsys.SwapBuffers();
}
