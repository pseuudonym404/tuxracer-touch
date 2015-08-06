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

#include "textures.h"
#include "spx.h"
#include "course.h"
#include "winsys.h"
#include "ogl.h"
#include <SDL2/SDL_image.h>
//#include <GL/glu.h>
#include <fstream>
#include <cctype>


static const GLshort fullsize_texture[] = {
	0, 0,
	1, 0,
	1, 1,
	0, 1
};

#ifdef USE_GLES1
static int GLES2D_p2( int input )
{
	int value = 1;

	 while (value < input)
	{
		value <<= 1;
	}
	return value;
}
#endif
// --------------------------------------------------------------------
//				class CImage
// --------------------------------------------------------------------

CImage::CImage () {
	data = NULL;
	nx = 0;
	ny = 0;
	depth = 0;
	pitch = 0;
}

CImage::~CImage () {
	DisposeData ();
}

void CImage::DisposeData () {
	delete[] data;
	data = NULL;
}

bool CImage::LoadPng (const char *filepath, bool mirroring,bool needsquared) {
	SDL_Surface *sdlImage;
	unsigned char *sdlData;

	sdlImage = IMG_Load (filepath);
	if (sdlImage == 0) {
		Message ("could not load image", filepath);
		return false;
	}

	nx    = sdlImage->w;
	ny    = sdlImage->h;
	depth = sdlImage->format->BytesPerPixel;
	pitch = sdlImage->pitch;
	DisposeData ();
#ifdef USE_GLES1
	int squared =GLES2D_p2(max(nx,ny));
	data  = new unsigned char[squared * squared * depth];
#else
	data  = new unsigned char[pitch * ny];
#endif
	if (SDL_MUSTLOCK (sdlImage)) {
	    if (SDL_LockSurface (sdlImage) < 0) {
 			SDL_FreeSurface (sdlImage);
			Message ("mustlock error");
			return false;
		};
	}

	sdlData = (unsigned char *) sdlImage->pixels;

#ifdef USE_GLES1
	if (!needsquared || (squared == sdlImage->w && squared == sdlImage->h))
	{
		if (mirroring) {
			for (int y=0; y<ny; y++) {
				memcpy(data + y*pitch, sdlData + (ny-1-y)*pitch, pitch);
			}
		} else {
			memcpy(data, sdlData, ny*pitch);
		}
	}
	else
	{
		int i, j, x, y, offset_y, offset_x;
		for (j=0; j<squared; j++)
		{
			y = ((mirroring ? squared-1-j : j) * sdlImage->h) / squared;
			offset_y = y * sdlImage->w;
			for (i=0; i<squared; i++)
			{
				x = (i * sdlImage->w) / squared;
				offset_x = (offset_y + x) * depth;
				memcpy(&data[(i+j*squared)*depth],&sdlData[offset_x],depth);
			}
		}
	}
#else
	if (mirroring) {
		for (int y=0; y<ny; y++) {
			memcpy(data + y*pitch, sdlData + (ny-1-y)*pitch, pitch);
		}
	} else {
		memcpy(data, sdlData, ny*pitch);
	}
#endif

	if (SDL_MUSTLOCK (sdlImage)) SDL_UnlockSurface (sdlImage);
	SDL_FreeSurface (sdlImage);
	return true;
}

bool CImage::LoadPng (const char *dir, const char *filename, bool mirroring, bool needsquared) {
	string path = dir;
	path += SEP;
	path += filename;
	return LoadPng (path.c_str(), mirroring, needsquared);
}

// ------------------ read framebuffer --------------------------------

bool CImage::ReadFrameBuffer_PPM () {
#ifndef USE_GLES1
	int viewport[4];
	glGetIntegerv (GL_VIEWPORT, viewport);

	nx = viewport[2];
	ny = viewport[3];
	depth = 3;

	DisposeData ();
	data  = new unsigned char[nx * ny * depth];

	glReadBuffer (GL_FRONT);

	for (int i=0; i<viewport[3]; i++) {
		glReadPixels (viewport[0], viewport[1] + viewport[3] - 1 - i,
			viewport[2], 1, GL_RGB, GL_UNSIGNED_BYTE, data + viewport[2] * i * 3);
	}
#endif

	return true;
}

void CImage::ReadFrameBuffer_TGA () {
#ifndef USE_GLES1
	nx = Winsys.resolution.width;
	ny = Winsys.resolution.height;
	depth = 3;

	DisposeData ();
	data  = new unsigned char[nx * ny * depth];

	glReadBuffer (GL_FRONT);
	glReadPixels (0, 0, nx, ny, GL_BGR, GL_UNSIGNED_BYTE, data);
#endif
}

void CImage::ReadFrameBuffer_BMP () {
#ifndef USE_GLES1
	nx = Winsys.resolution.width;
	ny = Winsys.resolution.height;
	depth = 4;

	DisposeData ();
	data  = new unsigned char[nx * ny * depth];
	glReadBuffer (GL_FRONT);
	glReadPixels (0, 0, nx, ny, GL_BGRA, GL_UNSIGNED_BYTE, data);
#endif
}

// ---------------------------

void CImage::WritePPM (const char *filepath) {
	if (data == NULL)
		return;

	std::ofstream file(filepath);

	file << "P6\n# A Raw PPM file"
	     << "\n# width\n" << nx
	     << "\n# height\n" << ny
	     << "\n# max component value\n255"<< std::endl;

	file.write(reinterpret_cast<char*>(data), depth * nx * ny);
}

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
struct TTgaHeader {
	int8_t  tfType;
	int8_t  tfColorMapType;
	int8_t  tfImageType;
	int8_t  tfColorMapSpec[5];
	int16_t tfOrigX;
	int16_t tfOrigY;
	int16_t tfWidth;
	int16_t tfHeight;
	int8_t  tfBpp;
	int8_t  tfImageDes;
#ifdef _MSC_VER
};
#pragma pack(pop)
#else
} __attribute__((packed));
#endif

void CImage::WriteTGA (const char *filepath) {
	if (data == NULL)
		return;

	TTgaHeader header;

	header.tfType = 0;
	header.tfColorMapType = 0;
	header.tfImageType = 2;
	for (int i=0; i<5; i++) header.tfColorMapSpec[i] = 0;
	header.tfOrigX = 0;
	header.tfOrigY = 0;
	header.tfWidth = static_cast<int16_t>(nx);
	header.tfHeight = static_cast<int16_t>(ny);
	header.tfBpp = 24;
	header.tfImageDes = 0;

	std::ofstream out(filepath, std::ios_base::out|std::ios_base::binary);
	out.write(reinterpret_cast<char*>(&header), sizeof(TTgaHeader));
	out.write(reinterpret_cast<char*>(data), 3 * nx * ny);
}

#define BF_TYPE 0x4D42             // "MB"

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
struct TBmpHeader {
	uint16_t bfType;           // identifier of bmp format
	uint32_t bfSize;           // size of file, including the headers
	uint16_t bfReserved1;      // reserved, always 0
	uint16_t bfReserved2;      // reserved, always 0
	uint32_t bfOffBits;        // offset to bitmap data
#ifdef _MSC_VER
};
#else
} __attribute__((packed));
#endif

struct TBmpInfo {
	uint32_t biSize;           // size of info header, normally 40
	int32_t  biWidth;          // width
	int32_t  biHeight;         // height
	uint16_t biPlanes;         // number of color planes, normally 1
	uint16_t biBitCount;       // Number of bits per pixel (8 * depth)
	uint32_t biCompression;    // type of compression, normally 0 = no compr.
	uint32_t biSizeImage;      // size of data
	int32_t  biXPelsPerMeter;  // normally 0
	int32_t  biYPelsPerMeter;  // normally 0
	uint32_t biClrUsed;        // normally 0
	uint32_t biClrImportant;   // normally 0
#ifdef _MSC_VER
};
#pragma pack(pop)
#else
} __attribute__((packed));
#endif

void CImage::WriteBMP (const char *filepath) {
	if (data == NULL)
		return;

	int infosize = 40;
	int width = nx;
	int height = ny;
	int imgsize = nx * ny * depth;
	int bitcnt = 8 * depth; // 24 or 32
	unsigned int bitsize;
	// (width * bitcnt + 7) / 8 = width * depth
	if (imgsize == 0) bitsize = (width * bitcnt + 7) / 8 * height;
	else bitsize = imgsize;

	TBmpHeader header;
	header.bfType = BF_TYPE;
	header.bfSize = 14 + infosize + bitsize;
	header.bfReserved1 = 0;
	header.bfReserved2 = 0;
	header.bfOffBits = sizeof(TBmpHeader) + sizeof(TBmpInfo);

	TBmpInfo info;
	info.biSize = infosize;
	info.biWidth = width;
	info.biHeight = height;
	info.biPlanes = 1;
	info.biBitCount = bitcnt;
	info.biCompression = 0;
	info.biSizeImage = imgsize;
	info.biXPelsPerMeter = 0;
	info.biYPelsPerMeter= 0;
	info.biClrUsed = 0;
	info.biClrImportant = 0;

	std::ofstream out(filepath, std::ios_base::out|std::ios_base::binary);
	if (!out) {
		Message ("could not open bmp file", filepath);
		return;
	}

	out.write(reinterpret_cast<char*>(&header), sizeof(TBmpHeader));
	out.write(reinterpret_cast<char*>(&info), sizeof(TBmpInfo));

	out.write(reinterpret_cast<char*>(data), bitsize);
}

// --------------------------------------------------------------------
//				class TTexture
// --------------------------------------------------------------------
TTexture::~TTexture() {
	glDeleteTextures (1, &id);
}

bool TTexture::Load(const string& filename) {
	CImage texImage;

	if (texImage.LoadPng (filename.c_str(), true) == false)
		return false;

#ifdef USE_GLES1
	width = texImage.nx;
	height= texImage.ny;
#endif

	glGenTextures (1, &id);
	Bind();
    glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
#ifndef USE_GLES1
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif

	GLenum format;
	if (texImage.depth == 3) format = GL_RGB;
	else format = GL_RGBA;

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

#ifdef USE_GLES1
	glTexImage2D
		(GL_TEXTURE_2D, 0, format, GLES2D_p2(texImage.nx),
		GLES2D_p2(texImage.ny), 0, format, GL_UNSIGNED_BYTE, texImage.data);
#else
	glTexImage2D
		(GL_TEXTURE_2D, 0, texImage.depth, texImage.nx,
		texImage.ny, 0, format, GL_UNSIGNED_BYTE, texImage.data);
#endif
	texImage.DisposeData();
    return true;
}
bool TTexture::Load(const string& dir, const string& filename) {
	return Load(dir + SEP + filename);
}
bool TTexture::LoadMipmap(const string& filename, bool repeatable) {
    CImage texImage;
	if (texImage.LoadPng (filename.c_str(), true,!repeatable) == false)
		return false;

	glGenTextures (1, &id);
	Bind();
    glPixelStorei (GL_UNPACK_ALIGNMENT, 4);

   if (repeatable) {
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    } else {
#ifdef USE_GLES1
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif
    }

	GLenum format;
	if (texImage.depth == 3) format = GL_RGB;
	else format = GL_RGBA;

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

#ifdef USE_GLES1
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	if (repeatable)
	{
		glTexImage2D
			(GL_TEXTURE_2D, 0, format, texImage.nx,
			texImage.ny, 0, format, GL_UNSIGNED_BYTE, texImage.data);
	}
	else
	{
		int squared = GLES2D_p2(max(texImage.nx,texImage.ny));
		glTexImage2D
			(GL_TEXTURE_2D, 0, format, squared,
			squared, 0, format, GL_UNSIGNED_BYTE, texImage.data);
	}
#else
	gluBuild2DMipmaps
		(GL_TEXTURE_2D, texImage.depth, texImage.nx,
		texImage.ny, format, GL_UNSIGNED_BYTE, texImage.data);
#endif
	texImage.DisposeData();
    return true;
}
bool TTexture::LoadMipmap(const string& dir, const string& filename, bool repeatable) {
	return LoadMipmap(dir + SEP + filename, repeatable);
}
static GLuint currentTexID = 0;

void TTexture::Bind() {
	if (currentTexID != id)
	{
		glBindTexture (GL_TEXTURE_2D, id);
		currentTexID = id;
	}
}

void TTexture::Draw() {
	GLshort w, h;
	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Bind();

#ifdef USE_GLES1
	w=this->width;
	h=this->height;
#else
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
#endif

	glColor4f(1.0, 1.0, 1.0, 1.0);
	const GLshort vtx[] = {
		0, 0,
		w, 0,
		w, h,
		0, h
	};
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_SHORT, 0, vtx);
	glTexCoordPointer(2, GL_SHORT, 0, fullsize_texture);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void TTexture::Draw(int x, int y, float size, Orientation orientation) {
	GLint w, h;
	GLfloat width, height, top, bott, left, right;

	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Bind();

#ifdef USE_GLES1
	w=this->width;
	h=this->height;
#else
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
#endif

	width  = w * size;
	height = h * size;

	if (orientation == OR_TOP) {
		top = Winsys.resolution.height - y;
		bott = top - height;

	} else {
		bott = y;
		top = bott + height;
	}
	if (x >= 0) left = x;
	else left = (Winsys.resolution.width - width) / 2;
	right = left + width;

	glColor4f(1.0, 1.0, 1.0, 1.0);
	const GLfloat vtx[] = {
		left, bott,
		right, bott,
		right, top,
		left, top
	};
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vtx);
	glTexCoordPointer(2, GL_SHORT, 0, fullsize_texture);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void TTexture::Draw(int x, int y, float width, float height, Orientation orientation) {
	GLfloat top, bott, left, right;

	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Bind();

	if (orientation == OR_TOP) {
		top = Winsys.resolution.height - y;
		bott = top - height;
	} else {
		bott = y;
		top = bott + height;
	}
	if (x >= 0) left = x;
	else left = (Winsys.resolution.width - width) / 2;
	right = left + width;

	glColor4f(1.0, 1.0, 1.0, 1.0);
	const GLfloat vtx[] = {
		left, bott,
		right, bott,
		right, top,
		left, top
	};
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vtx);
	glTexCoordPointer(2, GL_SHORT, 0, fullsize_texture);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void TTexture::DrawFrame(int x, int y, ETR_DOUBLE w, ETR_DOUBLE h, int frame, const TColor& col) {
	if (id < 1)
		return;

	GLshort ww = GLint(w);
	GLshort hh = GLint(h);
	GLshort xx = x;
	GLshort yy = Winsys.resolution.height - hh - y;

	Bind();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	if (frame > 0) {
#ifdef USE_GLES1
		if (w < 1) ww=this->width;
		if (h < 1) hh=this->height;
#else
		if (w < 1) glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &ww);
		if (h < 1) glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &hh);
#endif

		glColor(col, 1.0);

		glDisable (GL_TEXTURE_2D);
		const GLint vtx [] = {
			xx - frame, yy - frame,
			xx + ww + frame, yy - frame,
			xx + ww + frame, yy + hh + frame,
			xx - frame, yy + hh + frame
		};

		glVertexPointer(2, GL_INT, 0, vtx);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		glEnable (GL_TEXTURE_2D);
	}

	glColor4f (1.0, 1.0, 1.0, 1.0);

	const GLshort vtx[] = {
		xx, yy,
		GLshort(xx + ww), yy,
		GLshort(xx + ww), GLshort(yy + hh),
		xx, GLshort(yy + hh)
	};

	glVertexPointer(2, GL_SHORT, 0, vtx);
	glTexCoordPointer(2, GL_SHORT, 0, fullsize_texture);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

// --------------------------------------------------------------------
//				class CTexture
// --------------------------------------------------------------------

CTexture Tex;

CTexture::CTexture () {
	forientation = OR_TOP;
}

CTexture::~CTexture () {
	FreeTextureList();
}

void CTexture::LoadTextureList () {
	FreeTextureList();
	CSPList list (200);
	if (list.Load (param.tex_dir, "textures.lst")) {
		for (size_t i=0; i<list.Count(); i++) {
			const string& line = list.Line(i);
			string name = SPStrN (line, "name");
			int id = SPIntN (line, "id", -1);
			CommonTex.resize(max(CommonTex.size(), (size_t)id+1));
			string texfile = SPStrN (line, "file");
			bool rep = SPBoolN (line, "repeat", false);
			if (id >= 0) {
				CommonTex[id] = new TTexture();
				if (rep)
					CommonTex[id]->LoadMipmap(param.tex_dir, texfile, rep);
				else
					CommonTex[id]->Load(param.tex_dir, texfile);

				Index[name] = CommonTex[id];
			} else Message ("wrong texture id in textures.lst");
		}
	} else Message ("failed to load common textures");
}

void CTexture::FreeTextureList () {
	for (size_t i=0; i<CommonTex.size(); i++) {
		delete CommonTex[i];
	}
	CommonTex.clear();
	Index.clear();
}

TTexture* CTexture::GetTexture (size_t idx) const {
	if (idx >= CommonTex.size()) return NULL;
	return CommonTex[idx];
}

TTexture* CTexture::GetTexture (const string& name) const {
	return Index.at(name);
}

bool CTexture::BindTex (size_t idx) {
	if (idx >= CommonTex.size()) return false;
	CommonTex[idx]->Bind();
	return true;
}

bool CTexture::BindTex (const string& name) {
	try {
		Index.at(name)->Bind();
	} catch (...) {
		return false;
	}
	return true;
}

// ---------------------------- Draw ----------------------------------

void CTexture::Draw (size_t idx) {
	if (CommonTex.size() > idx)
		CommonTex[idx]->Draw();
}

void CTexture::Draw (const string& name) {
	Index[name]->Draw();
}

void CTexture::Draw (size_t idx, int x, int y, float size) {
	if (CommonTex.size() > idx)
		CommonTex[idx]->Draw(x, y, size, forientation);
}

void CTexture::Draw (const string& name, int x, int y, float size) {
	Index[name]->Draw(x, y, size, forientation);
}

void CTexture::Draw (size_t idx, int x, int y, int width, int height) {
	if (CommonTex.size() > idx)
		CommonTex[idx]->Draw (x, y, width, height, forientation);
}

void CTexture::Draw (const string& name, int x, int y, int width, int height) {
	Index[name]->Draw (x, y, width, height, forientation);
}

void CTexture::DrawFrame (size_t idx, int x, int y, ETR_DOUBLE w, ETR_DOUBLE h, int frame, const TColor& col) {
	if (CommonTex.size() > idx)
		CommonTex[idx]->DrawFrame (x, y, w, h, frame, col);
}

void CTexture::DrawFrame (const string& name, int x, int y, ETR_DOUBLE w, ETR_DOUBLE h, int frame, const TColor& col) {
	Index[name]->DrawFrame (x, y, w, h, frame, col);
}

void CTexture::SetOrientation (Orientation orientation) {
	forientation = orientation;
}

// -------------------------- numeric strings -------------------------

void CTexture::DrawNumChr (char c, int x, int y, int w, int h, const TColor& col) {
	int idx;
	if(isdigit(c)) {
		char chrname[2] = {c, '\0'};
		idx = atoi(chrname);
	} else if(c == ':')
		idx = 10;
	else if(c == ' ')
		idx = 11;
	else
		return;

	// texture coords
	float texw = 22.0 / 256.0;
	float texleft = idx * texw;
	float texright = (idx + 1) * texw;

	const GLfloat tex[] = {
		texleft, 0,
		texright, 0,
		texright, 1,
		texleft, 1
	};
	const GLfloat vtx[] = {
		GLfloat(x),           GLfloat(Winsys.resolution.height - y - h),
		GLfloat(x + w * 0.9), GLfloat(Winsys.resolution.height - y - h),
		GLfloat(x + w * 0.9), GLfloat(Winsys.resolution.height - y),
		GLfloat(x),           GLfloat(Winsys.resolution.height - y)
	};

	glVertexPointer(2, GL_FLOAT, 0, vtx);
	glTexCoordPointer(2, GL_FLOAT, 0, tex);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void CTexture::DrawNumStr (const string& s, int x, int y, float size, const TColor& col) {
	if (!BindTex ("ziff032")) {
		Message ("DrawNumStr: missing texture");
		return;
	}
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable (GL_TEXTURE_2D);
	int qw = (int)(22 * size);
	int qh = (int)(32 * size);

	glColor(col);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	for (size_t i=0; i < s.size(); i++) {
		DrawNumChr (s[i], x + (int)i*qw, y, qw, qh, col);
	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

// --------------------------------------------------------------------
//				screenshot
// --------------------------------------------------------------------

// 0 ppm, 1 tga, 2 bmp
#define SCREENSHOT_PROC 2

void ScreenshotN () {
	CImage image;
	string path = param.screenshot_dir;
	path += SEP;
	path += g_game.course->dir;
	path += "_";
	path += GetTimeString();
	int type = SCREENSHOT_PROC;

	switch (type) {
		case 0:
			path += ".ppm";
			image.ReadFrameBuffer_PPM ();
			image.WritePPM (path.c_str());
			break;
		case 1:
			path += ".tga";
			image.ReadFrameBuffer_TGA ();
			image.WriteTGA (path.c_str());
			break;
		case 2:
			path += ".bmp";
			image.ReadFrameBuffer_BMP ();
			image.WriteBMP (path.c_str());
			break;
	}
}
