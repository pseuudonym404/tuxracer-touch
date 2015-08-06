/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 2013 Extreme Tuxracer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
---------------------------------------------------------------------*/

#ifndef MATRICES_H
#define MATRICES_H

#include "vectors.h"

template<int ix, int iy>
class TMatrix {
	ETR_DOUBLE _data[ix][iy];
public:
	TMatrix() {}
	TMatrix(const TVector3d& w1, const TVector3d& w2, const TVector3d& w3);

	ETR_DOUBLE* operator[](int index) { return _data[index]; }
	const ETR_DOUBLE* operator[](int index) const { return _data[index]; }
	const ETR_DOUBLE* data() const { return (ETR_DOUBLE*)_data; }

	void SetIdentity();
	void SetRotationMatrix(ETR_DOUBLE angle, char axis);
	void SetTranslationMatrix(ETR_DOUBLE _x, ETR_DOUBLE _y, ETR_DOUBLE _z);
	void SetScalingMatrix(ETR_DOUBLE _x, ETR_DOUBLE _y, ETR_DOUBLE _z);

	TMatrix GetTransposed() const;

	static const TMatrix<ix, iy>& getIdentity();
};

template<int x, int y>
TMatrix<x, y> operator*(const TMatrix<x, y>& l, const TMatrix<x, y>& r);

#endif
