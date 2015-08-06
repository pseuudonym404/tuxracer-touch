#include "delplayer.h"
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

CDelPlayer DelPlayer;

static TWidget* textbuttons[2];

void CDelPlayer::Keyb_spec (SDL_Keysym /*sym*/, bool /*release*/) {
}

void CDelPlayer::Mouse (int button, int state, int x, int y) {
	TWidget* clicked = ClickGUI(state, x, y);
	if (state == 0) {
		if (clicked == textbuttons[0]) {
			State::manager.RequestEnterState (Regist);
		} else if (clicked == textbuttons[1]) {
			Players.DelPlayer(player);
			State::manager.RequestEnterState (Regist);
		}
	}
}

void CDelPlayer::Motion (int x, int y) {
	MouseMoveGUI(x, y);
	if (param.ui_snow) push_ui_snow (cursor_pos);
}

static TArea area;

void CDelPlayer::Enter() {
	Music.Play (param.menu_music, -1);
	ResetGUI();

	int framewidth = 500 * Winsys.scale;
	area = AutoAreaN (30, 80, framewidth);
	int siz = FT.AutoSizeN (5);
	textbuttons[0] = AddTextButton (Trans.Text(28), area.left+50, AutoYPosN (80), siz);
	ETR_DOUBLE len = FT.GetTextWidth (Trans.Text(15));
	textbuttons[1] = AddTextButton (Trans.Text(15), area.right-len-50, AutoYPosN (80), siz);
}

void CDelPlayer::Loop() {
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

	Tex.Draw (BOTTOM_LEFT, 0, hh - 256, 1);
	Tex.Draw (BOTTOM_RIGHT, ww-256, hh-256, 1);
	Tex.Draw (TOP_LEFT, 0, 0, 1);
	Tex.Draw (TOP_RIGHT, ww-256, 0, 1);

	FT.SetColor (colWhite);
	FT.AutoSizeN (5);
	string msg = Trans.Text(93) + " \"" + Players.GetPlayer(player)->name + "\"?";
	FT.DrawString (CENTER, AutoYPosN(45), msg);

	DrawGUI();
	Winsys.SwapBuffers();
}
