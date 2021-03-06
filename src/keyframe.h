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

#ifndef KEYFRAME_H
#define KEYFRAME_H

#include "bh.h"
#include <vector>

#define MAX_FRAME_VALUES 32

class CCharShape;

struct TKeyframe {
	ETR_DOUBLE val[MAX_FRAME_VALUES];
};

class CKeyframe {
private:
	vector<TKeyframe> frames;
	TVector3d refpos;
	string loadedfile;
	TKeyframe clipboard;
	ETR_DOUBLE keytime;
	ETR_DOUBLE heightcorr;
	size_t keyidx;

	ETR_DOUBLE interp (ETR_DOUBLE frac, ETR_DOUBLE v1, ETR_DOUBLE v2);
	void InterpolateKeyframe (size_t idx, ETR_DOUBLE frac, CCharShape *shape);

	// test and editing
	void ResetFrame2 (TKeyframe *frame);
public:
	CKeyframe ();
	bool loaded;
	bool active;

	void Init (const TVector3d& ref_position, ETR_DOUBLE height_correction);
	void Init (const TVector3d& ref_position, ETR_DOUBLE height_correction, CCharShape *shape);
	void InitTest (const TVector3d& ref_position, CCharShape *shape);
	void Reset ();
	void Update ();
	void UpdateTest (ETR_DOUBLE timestep, CCharShape *shape);
	bool Load (const string& dir, const string& filename);
	void CalcKeyframe (size_t idx, CCharShape *shape, const TVector3d& refpos);

	// test and editing
	TKeyframe *GetFrame (size_t idx);
	static const string& GetHighlightName (size_t idx);
	static const string& GetJointName (size_t idx);
	static int GetNumJoints ();
	void SaveTest (const string& dir, const string& filename);
	void CopyFrame (size_t prim_idx, size_t sec_idx);
	void AddFrame ();
	size_t  DeleteFrame (size_t idx);
	void InsertFrame (size_t idx);
	void CopyToClipboard (size_t idx);
	void PasteFromClipboard (size_t idx);
	void ClearFrame (size_t idx);
	size_t numFrames() const { return frames.size(); }
};

extern CKeyframe TestFrame;

#endif
