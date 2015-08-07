/* --------------------------------------------------------------------
EXTREME TUXRACER

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

/*
If you want to add a new option, do this:
First add the option to the TParam struct (game_config.h).

Then edit the below functions:

- LoadConfigFile. Use
	SPIntN for integer and boolean values
	SPStrN for strings.
	The first value is always 'line', the second defines the tag within the
	brackets [ ], and the last value is the default.

- SetConfigDefaults. These values are used as long as no options file exists.
	It's a good idea to use the same values as the defaults in LoadConfigFile.

- SaveConfigFile. See the other entries; it should be self-explanatory.
	If an options file exists, you will have to change any value at runtime
	on the configuration screen to overwrite the file. Then you will see the
	new entry.
*/

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "config_screen.h"
#include "spx.h"
#include "translation.h"
#include "particles.h"
#include "audio.h"
#include "ogl.h"
#include "gui.h"
#include "textures.h"
#include "font.h"
#include "translation.h"
#include "course.h"
#include "game_ctrl.h"
#include "winsys.h"

CGameConfig GameConfig;
static string res_names[NUM_RESOLUTIONS];

static TUpDown* language;
static TUpDown* vid_orient;
static TUpDown* sensit;
static TUpDown* mus_vol;
static TUpDown* sound_vol;
static TUpDown* detail_level;
static TWidget* textbuttons[2];


void SetConfig () {
	if (mus_vol->GetValue() != param.music_volume ||
			sound_vol->GetValue() != param.sound_volume ||
			size_t(language->GetValue()) != param.language ||
			detail_level->GetValue() != param.perf_level ||
			vid_orient->GetValue() != param.orient ||
			sensit->GetValue() != param.sensit)
		{
		// the followind config params don't require a new VideoMode
		// they only must stored in the param structure (and saved)
		param.sensit = sensit->GetValue();
		param.music_volume = mus_vol->GetValue();
		Music.SetVolume (param.music_volume);
		param.sound_volume = sound_vol->GetValue();
		param.perf_level = detail_level->GetValue();
		Winsys.SetOrient(vid_orient->GetValue());
		Winsys.SetFonttype ();
		if (param.language != size_t(language->GetValue())) {
			param.language = language->GetValue();
			Trans.LoadTranslations (param.language);
		}
		SaveConfigFile ();
	}
	State::manager.RequestEnterState(*State::manager.PreviousState());
}

void CGameConfig::Keyb (unsigned int key, bool special, bool release, int x, int y) {
	if (release) return;

	if (key != SDLK_UP && key != SDLK_DOWN)
		KeyGUI(key, 0, release);
	switch (key) {
		case SDLK_q:
			State::manager.RequestQuit();
			break;
		case SDLK_ESCAPE:
			State::manager.RequestEnterState (*State::manager.PreviousState());
			break;
		case SDLK_RETURN:
			if (textbuttons[0]->focussed())
				State::manager.RequestEnterState (*State::manager.PreviousState());
			else if (textbuttons[1]->focussed())
				SetConfig ();
			break;
		case SDLK_UP:
			DecreaseFocus();
			break;
		case SDLK_DOWN:
			IncreaseFocus();
			break;
	}
}

void CGameConfig::Mouse (int button, int state, int x, int y) {
	TWidget* focussed = ClickGUI(state, x, y);
	if (state == 0) {
		if (focussed == textbuttons[0])
			State::manager.RequestEnterState (*State::manager.PreviousState());
		else if (focussed == textbuttons[1])
			SetConfig ();
	}
	if (focussed) focussed->focus = false;
}

void CGameConfig::Motion (int x, int y) {
	MouseMoveGUI(x, y);

	if (param.ui_snow) push_ui_snow (cursor_pos);
}

// ------------------ Init --------------------------------------------

static TArea area;
static int dd;

void CGameConfig::Enter() {
	Winsys.ShowCursor (!param.ice_cursor);
	Winsys.KeyRepeat (true);

	for (int i=0; i<NUM_RESOLUTIONS; i++) res_names[i] = Winsys.GetResName (i);

	int framewidth = 500 * Winsys.scale;
	area = AutoAreaN (Winsys.resolution.width < Winsys.resolution.height ? 30 : 12, 80, framewidth);
	FT.AutoSizeN (4);
	dd = FT.AutoDistanceN (4);
	if (dd < 36) dd = 36;
	int rightpos = area.right -96;

	ResetGUI ();

	vid_orient = AddUpDown(rightpos, area.top, 0, 3, param.orient);
	sensit = AddUpDown(rightpos, area.top + dd, 1, 20, param.sensit);
	mus_vol = AddUpDown(rightpos, area.top + dd * 2, 0, 120, param.music_volume);
	sound_vol = AddUpDown(rightpos, area.top + dd * 3, 0, 120, param.sound_volume);
	detail_level = AddUpDown(rightpos, area.top + dd * 4, 1, 4, param.perf_level);
	language = AddUpDown(rightpos, area.top + dd * 5, 0, (int)Trans.languages.size() - 1, (int)param.language);

	int siz = FT.AutoSizeN (5);
	textbuttons[0] = AddTextButton (Trans.Text(28), area.left+50, AutoYPosN (80), siz);
	ETR_DOUBLE len = FT.GetTextWidth (Trans.Text(15));
	textbuttons[1] = AddTextButton (Trans.Text(15), area.right-len-50, AutoYPosN (80), siz);

	Music.Play (param.config_music, -1);
}

void CGameConfig::Loop () {
	int ww = Winsys.resolution.width;
	int hh = Winsys.resolution.height;

	Music.Update ();

	check_gl_error();
	Music.Update ();
	ScopedRenderMode rm(GUI);
	ClearRenderContext ();
	SetupGuiDisplay ();

	if (param.ui_snow) {
		update_ui_snow ();
		draw_ui_snow();
	}

	//Tex.Draw (T_TITLE_SMALL, CENTER, AutoYPosN (5), 1.0);
	Tex.Draw (BOTTOM_LEFT, 0, hh-256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);

//	DrawFrameX (area.left, area.top, area.right-area.left, area.bottom - area.top,
//			0, colMBackgr, colBlack, 0.2);

	FT.AutoSizeN (4);

	FT.SetColor (colWhite);
	FT.DrawString (area.left, area.top, Trans.Text(86));
	FT.DrawString (area.left, area.top + dd, Trans.Text(94));
	FT.DrawString (area.left, area.top + dd * 2, Trans.Text(33));
	FT.DrawString (area.left, area.top + dd * 3, Trans.Text(34));
	FT.DrawString (area.left, area.top + dd * 4, Trans.Text(36));
	FT.DrawString (area.left, area.top + dd * 5, Trans.Text(35));

	FT.DrawString (area.left+240, area.top, Trans.Text(87 + vid_orient->GetValue()));
	FT.DrawString (area.left+240, area.top + dd, Float_StrN(sensit->GetValue() / 10.0f, 1));
	FT.DrawString (area.left+240, area.top + dd * 2, Int_StrN (mus_vol->GetValue()));
	FT.DrawString (area.left+240, area.top + dd * 3, Int_StrN (sound_vol->GetValue()));
	FT.DrawString (area.left+240, area.top + dd * 4, Int_StrN (detail_level->GetValue()));
	FT.DrawString (area.left+240, area.top + dd * 5, Trans.languages[language->GetValue()].language);

	DrawGUI();

	Reshape (ww, hh);
	Winsys.SwapBuffers ();
}

void CGameConfig::Exit() {
	Winsys.KeyRepeat (false);
}
