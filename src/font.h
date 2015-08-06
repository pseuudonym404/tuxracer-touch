/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2004-2005 Volker Stroebel (Planetpenguin Racer)
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

#ifndef FONT_H
#define FONT_H

#include "bh.h"
#include <vector>
#include <map>

// --------------------------------------------------------------------
//		CFont
// --------------------------------------------------------------------

#define MAX_FONTS 36

class FTFont;

struct fontinfo {
	string fontpath;
	string fontname;
	map<float,FTFont*> fonts;
};

class CFont {
private:
	vector<fontinfo*> fonts;
	map<string, size_t> fontindex;
	Orientation forientation;

	int    curr_font;
	TColor curr_col;
	float  curr_size;
	float  curr_fact;		// the length factor

	static wstring UnicodeStr(const char* s);
	template<typename T>
	void DrawText(float x, float y, const T* text, size_t font, float size);
	void GetTextSize(const char *text, float &x, float &y, size_t font, float size);
	void GetTextSize(const wchar_t *text, float &x, float &y, size_t font, float size);
	FTFont* GetFont(const size_t findex,const float size);
public:
	CFont ();
	~CFont ();

	void Clear ();
	int  LoadFont(const string& name, const string& dir, const string& filename,float size);
	int  LoadFont(const string& name, const char *path, float size);
	bool LoadFontlist ();
	size_t GetFontIdx (const string &name) const;

	// properties
	void SetProps   (const string &fontname, float size, const TColor& col);
	void SetProps   (const string &fontname, float size);
	void SetColor   (const TColor& col) { curr_col = col; }
	void SetSize    (float size) { curr_size = size; }
	void SetFont    (const string& fontname);

	// auto
	int AutoSizeN     (int rel_val);	// rel_val = relative size, return: autosize
	int AutoDistanceN (int rel_val);	// rel_val = relative dist

	// draw
	void DrawText   (float x, float y, const char *text);		// normal char*
	void DrawText   (float x, float y, const wchar_t *text);	// wide char*
	void DrawString (float x, float y, const string &s);		// string class
	void DrawString (float x, float y, const wstring &s);		// wstring class


	void DrawText   (float x, float y, const char *text, const string &fontname, float size);
	void DrawText   (float x, float y, const wchar_t *text, const string &fontname, float size);
	void DrawString (float x, float y, const string &s, const string &fontname, float size);
	void DrawString (float x, float y, const wstring &s, const string &fontname, float size);

	// metrics
	void  GetTextSize  (const char *text, float &x, float &y);
	void  GetTextSize  (const char *text, float &x, float &y, const string &fontname, float size);
	float GetTextWidth (const char *text);
	float GetTextWidth (const string& text);
	float GetTextWidth (const wchar_t *text);
	float GetTextWidth (const char *text, const string &fontname, float size);
	float GetTextWidth (const wchar_t *text, const string &fontname, float size);

	float CenterX        (const char *text);
	void  SetOrientation (Orientation orientation) { forientation = orientation; }

	vector<string> MakeLineList (const char *source, float width);
};

extern CFont FT;

#endif
