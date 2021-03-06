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

#include "event_select.h"
#include "gui.h"
#include "font.h"
#include "particles.h"
#include "audio.h"
#include "ogl.h"
#include "textures.h"
#include "game_ctrl.h"
#include "translation.h"
#include "event.h"
#include "game_type_select.h"
#include "winsys.h"

CEventSelect EventSelect;

static TEvent *EventList;
static TUpDown* event;
static TUpDown* cup;
static TWidget* textbuttons[2];

void EnterEvent () {
	g_game.game_type = CUPRACING;
	g_game.cup = EventList[event->GetValue()].cups[cup->GetValue()];
	State::manager.RequestEnterState(Event);
}

void CEventSelect::Keyb (unsigned int key, bool special, bool release, int x, int y) {
	if (release) return;
	switch (key) {
		case SDLK_ESCAPE:
			State::manager.RequestEnterState (GameTypeSelect);
			break;
		case SDLK_q:
			State::manager.RequestQuit();
			break;
		case SDLK_RETURN:
			if (textbuttons[1]->focussed()) State::manager.RequestEnterState (GameTypeSelect);
			else if (Events.IsUnlocked (event->GetValue(), cup->GetValue())) EnterEvent();
			break;
		case SDLK_u:
			param.ui_snow = !param.ui_snow;
			break;
		default:
			KeyGUI(key, 0, release);
	}
}

void CEventSelect::Mouse (int button, int state, int x, int y) {
	TWidget* clicked = ClickGUI(state, x, y);
	if (state == 0) {
		if (textbuttons[0] == clicked) {
			if (Events.IsUnlocked (event->GetValue(), cup->GetValue()))
				EnterEvent();
		} else if (textbuttons[1] == clicked)
			State::manager.RequestEnterState (GameTypeSelect);
	}
}

void CEventSelect::Motion (int x, int y) {
	MouseMoveGUI(x, y);

	if (param.ui_snow) push_ui_snow (cursor_pos);
}

// --------------------------------------------------------------------
static TArea area;
static int framewidth, frameheight, frametop1, frametop2;

void CEventSelect::Enter () {
	Winsys.ShowCursor (!param.ice_cursor);
	EventList = &Events.EventList[0];

	framewidth = 400 * Winsys.scale;
	frameheight = 50 * Winsys.scale;
	area = AutoAreaN (30, 80, framewidth);
	area.left -= 48;
	frametop1 = AutoYPosN (35);
	frametop2 = AutoYPosN (50);

	ResetGUI();
	event = AddUpDown(area.right - 40, frametop1, 0, (int)Events.EventList.size() - 1, 0);
	cup = AddUpDown(area.right - 40, frametop2, 0, (int)Events.EventList[0].cups.size() - 1, 0);

	int siz = FT.AutoSizeN (5);

	ETR_DOUBLE len = FT.GetTextWidth (Trans.Text(9));
	textbuttons[0] = AddTextButton (Trans.Text(9), area.right-len-50, AutoYPosN (70), siz);
	textbuttons[1] = AddTextButton (Trans.Text(8), area.left+50, AutoYPosN (70), siz);
	SetFocus(textbuttons[1]);

	Events.MakeUnlockList (g_game.player->funlocked);
	Music.Play (param.menu_music, -1);
}

void CEventSelect::Loop () {
	int ww = Winsys.resolution.width;
	int hh = Winsys.resolution.height;
	TColor col;

	check_gl_error();
	ScopedRenderMode rm(GUI);
	Music.Update ();
	ClearRenderContext ();
	SetupGuiDisplay ();

	if (param.ui_snow) {
		update_ui_snow ();
		draw_ui_snow ();
	}

	//Tex.Draw (T_TITLE_SMALL, CENTER, AutoYPosN (5), Winsys.scale);
	Tex.Draw (BOTTOM_LEFT, 0, hh-256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);

//	DrawFrameX (area.left, area.top, area.right-area.left, area.bottom - area.top,
//			0, colMBackgr, colBlack, 0.2);

	FT.AutoSizeN (3);
	FT.SetColor (colWhite);
	FT.DrawString (area.left, AutoYPosN (30), Trans.Text (6));
	FT.DrawString (area.left,AutoYPosN (45), Trans.Text (7));
	if (Events.IsUnlocked (event->GetValue(), cup->GetValue()) == false) {
		FT.SetColor (colLGrey);
		FT.DrawString (CENTER, AutoYPosN (58), Trans.Text (10));
	}

	FT.AutoSizeN (4);

	col = colWhite;
	DrawFrameX (area.left, frametop1, framewidth, frameheight, 3, colMBackgr, col, 1.0);
	FT.SetColor (colDYell);
	FT.DrawString (area.left + 20, frametop1, EventList[event->GetValue()].name);

	col = colWhite;
	DrawFrameX (area.left, frametop2, framewidth, frameheight, 3, colMBackgr, col, 1.0);
	if (Events.IsUnlocked (event->GetValue(), cup->GetValue()))
		FT.SetColor (colDYell);
	else
		FT.SetColor (colLGrey);
	FT.DrawString (area.left + 20, frametop2, Events.GetCupTrivialName (event->GetValue(), cup->GetValue()));

	textbuttons[0]->SetActive(Events.IsUnlocked (event->GetValue(), cup->GetValue()));
	DrawGUI();

	Winsys.SwapBuffers();
}
