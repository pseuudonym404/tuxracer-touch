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

#ifndef COMMON_H
#define COMMON_H

#include "bh.h"
#include "matrices.h"

using namespace std;


#define clamp(minimum, x, maximum) (max(min(x, maximum), minimum))

#ifndef ROUND_TO_NEAREST
#	define ROUND_TO_NEAREST(x) ((int) ((x)+0.5))
#endif

#ifndef M_PI
#	define M_PI 3.1415926535
#endif

#ifndef EPS
#	define EPS 1.0e-13
#endif

#define ANGLES_TO_RADIANS(x) (M_PI / 180.0 * (x))
#define RADIANS_TO_ANGLES(x) (180.0 / M_PI * (x))

#define MAG_SQD(vec) ((vec).x * (vec).x + (vec).y * (vec).y + (vec).z * (vec).z)


// --------------------------------------------------------------------
//				color utils
// --------------------------------------------------------------------

extern const TColor colWhite;
extern const TColor colDYell;
extern const TColor colDDYell;
extern const TColor colYellow;
extern const TColor colLYell;
extern const TColor colOrange;
extern const TColor colLRed;
extern const TColor colRed;
extern const TColor colDRed;
extern const TColor colGrey;
extern const TColor colLGrey;
extern const TColor colDGrey;
extern const TColor colBlack;
extern const TColor colBlue;
extern const TColor colLBlue;
extern const TColor colDBlue;
extern const TColor colBackgr;
extern const TColor colDBackgr;
extern const TColor colDDBackgr;
extern const TColor colMBackgr;
extern const TColor colLBackgr;
extern const TColor colMess;
extern const TColor colSky;

// --------------------------------------------------------------------
//				print utils
// --------------------------------------------------------------------
// some simple functions to print out values on the
// terminal. Only used for development.
void	PrintInt (const int val);
void	PrintInt (const string& s, const int val);
void	PrintStr (const char *val);
void	PrintString (const string& s);
void	PrintDouble (const double val);
void	PrintVector (const TVector3d& v);
void	PrintVector4 (const TVector4d& v);
void	PrintColor (const TColor& c);
void	PrintVector2 (const TVector2d& v);

template<int x, int y>
void	PrintMatrix (const TMatrix<x, y>& mat);
void	PrintQuaternion (const TQuaternion& q);

// --------------------------------------------------------------------
//				file utils
// --------------------------------------------------------------------

bool	FileExists (const char *filename);
bool	FileExists (const string& filename);
bool	FileExists (const string& dir, const string& filename);
bool	DirExists (const char *dirname);

// --------------------------------------------------------------------
//				message utils
// --------------------------------------------------------------------

void	Message (const char *msg, const char *desc);
void	Message (const char *msg);
void	Message (const string& a, const string& b);
void	Message (const string& a);
void	SaveMessages ();

// --------------------------------------------------------------------
//				date and time
// --------------------------------------------------------------------

void GetTimeComponents (ETR_DOUBLE time, int *min, int *sec, int *hundr);
string GetTimeString ();


#endif
