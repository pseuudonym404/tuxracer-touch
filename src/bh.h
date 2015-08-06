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

#ifndef BH_H
#define BH_H

// --------------------------------------------------------------------
//			global and or system-dependant includes
// --------------------------------------------------------------------

#ifndef HAVE_CONFIG_H
// These are detected by configure
#define HAVE_GETCWD 1
#define HAVE_STRDUP 1
#define HAVE_SYS_TIME_H 1
#endif


#define TIME_WITH_SYS_TIME
#define HAVE_GETTIMEOFDAY
#define HAVE_GL_GLEXT_H
#define HAVE_GL_GLX_H

//#include <cstdint>
#include <cstddef>
#include <string>

#ifdef USE_GLES1
#define ETR_DOUBLE float
#define GL_INT     GL_SHORT
#define GL_UNSIGNED_INT GL_UNSIGNED_SHORT
#define GL_QUADS GL_TRIANGLE_FAN
#define GL_QUAD_STRIP GL_TRIANGLE_STRIP
#define GLdouble     GLfloat
#define GLclampd     GLclampf
#define glOrtho      glOrthof
#define glDepthRange glDepthRangef
#define glClearDepth glClearDepthf
#define glMultMatrixd glMultMatrixf
#define glFogi glFogf
#define glColor4d glColor4f
#define glTranslated glTranslatef
#define glNormal3d glNormal3f
#define glVertex3d glVertex3f
#define glTexCoord2d glTexCoord2f
#define glRecti glRectf
#define glVertex2i glVertex2f
#define glNormal3i glNormal3f
#include <GLES/gl.h>
//#include <GL/glu.h>
void glPopAttrib();
void glPushAttrib(int t);
void glBegin(GLenum mode);
void glEnd();
void glVertex2f(GLfloat x, GLfloat y);
void glVertex3f(GLfloat x, GLfloat y, GLfloat z);
void glTexCoord2f(GLfloat s, GLfloat t);
void glColor4fv(const GLfloat *v);
void glRectf(GLfloat x, GLfloat y, GLfloat w, GLfloat h);
inline void glColor4dv(const GLfloat* c) { glColor4f(c[0], c[1], c[2], c[3]); }

int glesGetGlobalVertexBufferCurPos();
void glesSetGlobalVertexBufferCurPos(int pos);
GLfloat* glesGetGlobalVertexBuffer(int minsize);
void glesCleanUp();
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
#define GLAPI extern
#endif
typedef void (APIENTRYP PFNGLLOCKARRAYSEXTPROC) (GLint first, GLsizei count);
typedef void (APIENTRYP PFNGLUNLOCKARRAYSEXTPROC) (void);
typedef void (APIENTRYP PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRYP PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRYP PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef GLboolean (APIENTRYP PFNGLISBUFFERARBPROC) (GLuint buffer);
typedef void (APIENTRYP PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
typedef void (APIENTRYP PFNGLBUFFERSUBDATAARBPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
typedef GLvoid* (APIENTRYP PFNGLMAPBUFFERARBPROC) (GLenum target, GLenum access);
typedef GLboolean (APIENTRYP PFNGLUNMAPBUFFERARBPROC) (GLenum target);
typedef void (APIENTRYP PFNGLGETBUFFERPARAMETERIVARBPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETBUFFERPOINTERVARBPROC) (GLenum target, GLenum pname, GLvoid* *params);
#else
#define ETR_DOUBLE double
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#ifndef HAVE_CONFIG_H
#	ifdef _WIN32 // Windows platform
#		ifdef _MSC_VER // MSVC compiler
#			define OS_WIN32_MSC
#		else // Assume MinGW compiler
#			define OS_WIN32_MINGW
#		endif
#	else // Assume Unix platform (Linux, Mac OS X, BSD, ...)
#		ifdef __APPLE__
#			define OS_MAC
#		elif defined(__linux__)
#			define OS_LINUX
#		endif
#	endif
#endif // CONFIG_H

#if defined OS_WIN32_MSC // Windows platform
#	include <windows.h>
#	include "glext.h"
#	pragma warning (disable:4244)
#	pragma warning (disable:4305)
#	define SEP "\\"
#	undef DrawText
#	undef GetObject
#elif defined OS_WON32_MINGW
#	include <dirent.h>
#	include <GL/glext.h>
#	define SEP "/"
#else // Assume Unix platform (Linux, Mac OS X, BSD, ...)
#	include <unistd.h>
#	include <sys/types.h>
#	include <pwd.h>
#	include <dirent.h>
#	include <sys/time.h>
//#	include <GL/glx.h>
#	define SEP "/"
#endif

// --------------------------------------------------------------------
//			defines
// --------------------------------------------------------------------

#define USE_STENCIL_BUFFER

#include "version.h"
#define WINDOW_TITLE "Extreme Tux Racer " ETR_VERSION_STRING

using namespace std;

#include "etr_types.h"
#include "common.h"
#include "game_config.h"

extern TGameData g_game;

#endif // BH_H
