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

#ifndef MATHLIB_H
#define MATHLIB_H

#include "bh.h"
#include "matrices.h"
#include <vector>

static const TVector3d GravVec(0.0, -1.0, 0.0);

// --------------------------------------------------------------------
//			Advanced geometry
// --------------------------------------------------------------------

struct TPlane {
	TVector3d nml;
	ETR_DOUBLE d;
	TPlane(ETR_DOUBLE nx = 0.0, ETR_DOUBLE ny = 0.0, ETR_DOUBLE nz = 0.0, ETR_DOUBLE d_ = 0.0)
		: nml(nx, ny, nz), d(d_)
	{}
};

struct TPolygon		{ vector<int> vertices; };
struct TSphere		{ ETR_DOUBLE radius; int divisions; };
struct TRay			{ TVector3d pt; TVector3d vec; };

struct TPolyhedron {
	vector<TVector3d> vertices;
	vector<TPolygon> polygons;
};

TVector3d	ProjectToPlane(const TVector3d& nml, const TVector3d& v);
TVector3d	TransformVector(const TMatrix<4, 4>& mat, const TVector3d& v);
TVector3d	TransformNormal(const TVector3d& n, const TMatrix<4, 4>& mat);	// not used ?
TVector3d	TransformPoint(const TMatrix<4, 4>& mat, const TVector3d& p);
bool		IntersectPlanes (const TPlane& s1, const TPlane& s2, const TPlane& s3, TVector3d *p);
ETR_DOUBLE		DistanceToPlane (const TPlane& plane, const TVector3d& pt);

TMatrix<4, 4> RotateAboutVectorMatrix(const TVector3d& u, ETR_DOUBLE angle);

TQuaternion MultiplyQuaternions (const TQuaternion& q, const TQuaternion& r);
TQuaternion ConjugateQuaternion (const TQuaternion& q);
TMatrix<4, 4> MakeMatrixFromQuaternion(const TQuaternion& q);
TQuaternion MakeQuaternionFromMatrix(const TMatrix<4, 4>& mat);
TQuaternion MakeRotationQuaternion (const TVector3d& s, const TVector3d& t);
TQuaternion InterpolateQuaternions (const TQuaternion& q, TQuaternion r, ETR_DOUBLE t);
TVector3d	RotateVector (const TQuaternion& q, const TVector3d& v);

bool		IntersectPolygon (const TPolygon& p, vector<TVector3d>& v);
bool		IntersectPolyhedron (TPolyhedron& p);
TVector3d	MakeNormal (const TPolygon& p, const TVector3d *v);
void		TransPolyhedron(const TMatrix<4, 4>& mat, TPolyhedron& ph);

// --------------------------------------------------------------------
//				ode solver
// --------------------------------------------------------------------

struct TOdeData {
	ETR_DOUBLE k[4];
	ETR_DOUBLE init_val;
	ETR_DOUBLE h;
};

typedef int			(*PNumEstimates) ();
typedef void		(*PInitOdeData) (TOdeData *, ETR_DOUBLE init_val, ETR_DOUBLE h);
typedef ETR_DOUBLE		(*PNextTime) (TOdeData *, int step);
typedef ETR_DOUBLE		(*PNextValue) (TOdeData *, int step);
typedef void		(*PUpdateEstimate) (TOdeData *, int step, ETR_DOUBLE val);
typedef ETR_DOUBLE		(*PFinalEstimate) (TOdeData *);
typedef ETR_DOUBLE		(*PEstimateError) (TOdeData *);
typedef ETR_DOUBLE		(*PTimestepExponent) ();

struct TOdeSolver {
	PNumEstimates		NumEstimates;
	PInitOdeData		InitOdeData;
	PNextTime			NextTime;
	PNextValue			NextValue;
	PUpdateEstimate		UpdateEstimate;
	PFinalEstimate		FinalEstimate;
	PEstimateError		EstimateError;
	PTimestepExponent	TimestepExponent;
	TOdeSolver();
};

// --------------------------------------------------------------------
//			special
// --------------------------------------------------------------------

int Gauss (ETR_DOUBLE *matrix, int n, ETR_DOUBLE *soln);
ETR_DOUBLE LinearInterp (const ETR_DOUBLE x[], const ETR_DOUBLE y[], ETR_DOUBLE val, int n);

ETR_DOUBLE	XRandom (float min, float max);
ETR_DOUBLE	FRandom ();
int		IRandom (int min, int max);
int		ITrunc (int val, int base);
int		IFrac (int val, int base);

#endif
