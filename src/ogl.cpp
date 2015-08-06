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

#include "ogl.h"
#include "spx.h"
#include "winsys.h"
//#include <GL/glu.h>
#include <stack>

#define GLOBAL_VERTEX_INDEX_SIZE 5*(12+4+12)

static GLbyte global_vtx[] = {
	//trees
	-1, 0,        0,
	0, 0,
	1,  0,        0,
	1, 0,
	1,  1, 0,
	1, 1,
	-1, 0,        0,
	0, 0,
	1,  1, 0,
	1, 1,
	-1, 1, 0,
	0, 1,

	0,         0,        -1,
	0, 0,
	0,         0,        1,
	1, 0,
	0,         1, 1,
	1, 1,
	0,         0,        -1,
	0, 0,
	0,         1, 1,
	1, 1,
	0,         1, -1,
	0, 1,

	// items
	-1, 0,1,
	0, 0,
	1, 0,-1,
	1, 0,
	1,  1, -1,
	1, 1,
	-1, 1, 1,
	0, 1,

	//skybox front
	-1, -1, -1,
	0,0,
	1, -1, -1,
	1,0,
	1,  1, -1,
	1,1,
	-1,  1, -1,
	0,1,
	//skybox left
	-1, -1,  1,
	0,0,
	-1, -1, -1,
	1,0,
	-1,  1, -1,
	1,1,
	-1,  1,  1,
	0,1,
	//skybox right
	1, -1, -1,
	0,0,
	1, -1, 1,
	1,0,
	1,  1, 1,
	1,1,
	1,  1, -1,
	0,1,

};
static unsigned int* GlobalVBO= 0;
static GLushort globalvertexindex[] ={0,1,2,3,4,5,6,7,8,9,10,11,
									  12,13,14,15,
									  16,17,18,19,20,21,22,23,24,25,26,27};

static const struct {
	const char* name;
	GLenum value;
	GLenum type;
} gl_values[] = {
	{ "maximum lights", GL_MAX_LIGHTS, GL_INT },
	{ "modelview stack depth", GL_MAX_MODELVIEW_STACK_DEPTH, GL_INT },
	{ "projection stack depth", GL_MAX_PROJECTION_STACK_DEPTH, GL_INT },
	{ "max texture size", GL_MAX_TEXTURE_SIZE, GL_INT },
#ifndef USE_GLES1
	{ "double buffering", GL_DOUBLEBUFFER, GL_UNSIGNED_BYTE },
#endif
	{ "red bits", GL_RED_BITS, GL_INT },
	{ "green bits", GL_GREEN_BITS, GL_INT },
	{ "blue bits", GL_BLUE_BITS, GL_INT },
	{ "alpha bits", GL_ALPHA_BITS, GL_INT },
	{ "depth bits", GL_DEPTH_BITS, GL_INT },
	{ "stencil bits", GL_STENCIL_BITS, GL_INT }
};

void check_gl_error() {
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		char str[256];
		snprintf (str, sizeof(str), "%d", error);
		Message ("OpenGL Error: ", str);
	}
}

/*#ifdef USE_GLES1
#include <dlfcn.h>
void* gles_library;
void* glesGetProcAddress(const char* name)
{
	return dlsym(gles_library,name);
}
#endif*/

/*PFNGLLOCKARRAYSEXTPROC glLockArraysEXT_p = NULL;
PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT_p = NULL;
PFNGLGENBUFFERSARBPROC glGenBuffersARB_p = NULL;
PFNGLBINDBUFFERARBPROC glBindBufferARB_p = NULL;
PFNGLBUFFERDATAARBPROC glBufferDataARB_p = NULL;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB_p = NULL;
PFNGLISBUFFERARBPROC glIsBufferARB_p = NULL;*/

typedef void (*(*get_gl_proc_fptr_t)(const GLubyte *))();
void InitOpenglExtensions () {
/*	get_gl_proc_fptr_t get_gl_proc;
#ifdef USE_GLES1
	gles_library= dlopen("libGLES_CM.so", RTLD_LAZY);
	get_gl_proc = (get_gl_proc_fptr_t) glesGetProcAddress;
#else
	get_gl_proc = (get_gl_proc_fptr_t) SDL_GL_GetProcAddress;
#endif

	if (get_gl_proc) {
		glLockArraysEXT_p = (PFNGLLOCKARRAYSEXTPROC) 
		    (*get_gl_proc)((GLubyte*) "glLockArraysEXT");
		glUnlockArraysEXT_p = (PFNGLUNLOCKARRAYSEXTPROC) 
		    (*get_gl_proc)((GLubyte*) "glUnlockArraysEXT");

		if (glLockArraysEXT_p != NULL && glUnlockArraysEXT_p != NULL) {

		} else {
			Message ("GL_EXT_compiled_vertex_array extension NOT supported", "");
			glLockArraysEXT_p = NULL;
			glUnlockArraysEXT_p = NULL;
		}
		// check for VBO support
#ifdef USE_GLES1
		glGenBuffersARB_p = (PFNGLGENBUFFERSARBPROC) get_gl_proc((GLubyte*)"glGenBuffers");
		glBindBufferARB_p = (PFNGLBINDBUFFERARBPROC) get_gl_proc((GLubyte*)"glBindBuffer");
		glBufferDataARB_p = (PFNGLBUFFERDATAARBPROC) get_gl_proc((GLubyte*)"glBufferData");
		glDeleteBuffersARB_p = (PFNGLDELETEBUFFERSARBPROC) get_gl_proc((GLubyte*)"glDeleteBuffers");
		glIsBufferARB_p = (PFNGLISBUFFERARBPROC) get_gl_proc((GLubyte*)"glIsBuffer");
#else
		glGenBuffersARB_p = (PFNGLGENBUFFERSARBPROC) get_gl_proc((GLubyte*)"glGenBuffersARB");
		glBindBufferARB_p = (PFNGLBINDBUFFERARBPROC) get_gl_proc((GLubyte*)"glBindBufferARB");
		glBufferDataARB_p = (PFNGLBUFFERDATAARBPROC) get_gl_proc((GLubyte*)"glBufferDataARB");
		glDeleteBuffersARB_p = (PFNGLDELETEBUFFERSARBPROC) get_gl_proc((GLubyte*)"glDeleteBuffersARB");
		glIsBufferARB_p = (PFNGLISBUFFERARBPROC) get_gl_proc((GLubyte*)"glIsBufferARB");
#endif
		if (glGenBuffersARB_p == NULL ||
				glBindBufferARB_p == NULL ||
				glBufferDataARB_p == NULL ||
				glDeleteBuffersARB_p == NULL ||
				glIsBufferARB_p == NULL)
		{
			Message ("vertex buffer objects extension NOT supported", "");
			glGenBuffersARB_p = NULL;
			glBindBufferARB_p = NULL;
			glBufferDataARB_p = NULL;
			glDeleteBuffersARB_p = NULL;
			glIsBufferARB_p = NULL;
		}
	} else {
		Message ("No function available for obtaining GL proc addresses", "");
	}
*/
}
bool BuildGlobalVBO()
{
	/*if (glGenBuffersARB_p == NULL) return false;
	if (glBindBufferARB_p == NULL) return false;
	if (glBufferDataARB_p == NULL) return false;
	if (glIsBufferARB_p == NULL) return false;*/

	GlobalVBO = new unsigned int;
	glGenBuffers( 1, GlobalVBO );
	glBindBuffer( GL_ARRAY_BUFFER, *GlobalVBO );
	glBufferData( GL_ARRAY_BUFFER,GLOBAL_VERTEX_INDEX_SIZE, global_vtx, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	if (!glIsBuffer(*GlobalVBO)) return false;

	return true;
}

void DeleteGlobalVBO()
{
	/*if (glIsBufferARB_p == NULL) return;
	if (glDeleteBuffersARB_p == NULL) return;*/

	if (glIsBuffer(*GlobalVBO)) glDeleteBuffers(1, GlobalVBO);
}

bool BindGlobalVBO()
{
	/*if (glBindBufferARB_p == NULL)
	{
		glVertexPointer(3, GL_BYTE, 5*sizeof(GLbyte), global_vtx);
		glTexCoordPointer(2, GL_BYTE, 5*sizeof(GLbyte), global_vtx+3);
		return false;
	}*/

	glBindBuffer(GL_ARRAY_BUFFER,*GlobalVBO);
	intptr_t AddStride = 0;
	AddStride+= 3*sizeof(GLbyte);
	glVertexPointer(3, GL_BYTE, 5*sizeof(GLbyte), 0);
	glTexCoordPointer(2, GL_BYTE, 5*sizeof(GLbyte), (intptr_t *)(AddStride));
	return true;
}
void UnbindVBO()
{
	//if (glBindBufferARB_p == NULL) return;
	glBindBuffer(GL_ARRAY_BUFFER,0);
}
void RenderGlobalVBO(GLenum mode,int indexcount, GlobalVBOs vbo)
{
	glDrawElements(mode, indexcount, GL_UNSIGNED_INT,globalvertexindex+vbo);
}

void PrintGLInfo () {
	Message ("Gl vendor: ", (char*)glGetString (GL_VENDOR));
	Message ("Gl renderer: ", (char*)glGetString (GL_RENDERER));
	Message ("Gl version: ", (char*)glGetString (GL_VERSION));
	string extensions = (char*)glGetString (GL_EXTENSIONS);
	Message ("");
	Message ("Gl extensions:");
	Message ("");

	size_t oldpos = 0;
	size_t pos;
	while ((pos = extensions.find(' ', oldpos)) != string::npos) {
		string s = extensions.substr(oldpos, pos-oldpos);
		Message(s);
		oldpos = pos+1;
	}
	Message(extensions.substr(oldpos));
#ifndef USE_GLES1
	Message ("");
	for (int i=0; i<(int)(sizeof(gl_values)/sizeof(gl_values[0])); i++) {
		switch (gl_values[i].type) {
			case GL_INT: {
				GLint int_val;
				glGetIntegerv (gl_values[i].value, &int_val);
				string ss = Int_StrN (int_val);
				Message (gl_values[i].name, ss);
				break;
			}
			case GL_FLOAT: {
				GLfloat float_val;
				glGetFloatv (gl_values[i].value, &float_val);
				string ss = Float_StrN (float_val, 2);
				Message (gl_values[i].name, ss);
				break;
			}
			case GL_UNSIGNED_BYTE: {
				GLboolean boolean_val;
				glGetBooleanv (gl_values[i].value, &boolean_val);
				string ss = Int_StrN (boolean_val);
				Message (gl_values[i].name, ss);
				break;
			}
			default:
				Message ("");
		}
	}
#endif
}

void set_material (const TColor& diffuse_colour, const TColor& specular_colour, float specular_exp) {
	GLfloat mat_amb_diff[4] = {
		static_cast<GLfloat>(diffuse_colour.r),
		static_cast<GLfloat>(diffuse_colour.g),
		static_cast<GLfloat>(diffuse_colour.b),
		static_cast<GLfloat>(diffuse_colour.a)
	};
	glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff);

	GLfloat mat_specular[4] = {
		static_cast<GLfloat>(specular_colour.r),
		static_cast<GLfloat>(specular_colour.g),
		static_cast<GLfloat>(specular_colour.b),
		static_cast<GLfloat>(specular_colour.a)
	};
	glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);

	glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, specular_exp);

	glColor(diffuse_colour);
}
void ClearRenderContext () {
	glDepthMask (GL_TRUE);
	glClearColor (colBackgr.r, colBackgr.g, colBackgr.b, colBackgr.a);
	glClearStencil (0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void ClearRenderContext (const TColor& col) {
	glDepthMask (GL_TRUE);
	glClearColor (col.r, col.g, col.b, col.a);
	glClearStencil (0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void SetupGuiDisplay () {
	static const float offset = 0.f;

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();

	switch (Winsys.orient) {
		default: break;
		case 1: glRotatef(270.0f, 0.0f, 0.0f, 1.0f); break;
		case 2: glRotatef(180.0f, 0.0f, 0.0f, 1.0f); break;
		case 3: glRotatef(90.0f, 0.0f, 0.0f, 1.0f); break;
	}

	glOrthof (0, Winsys.resolution.width, 0, Winsys.resolution.height, -1.0, 1.0);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glTranslatef (offset, offset, -1.0);
	glColor4f (1.0, 1.0, 1.0, 1.0);
}

#ifdef USE_GLES1
inline void gluPerspective( float fovy, float aspect, float zNear, float zFar )
{
	GLfloat ymax = zNear * tan( fovy * M_PI / 360.0 );
	GLfloat ymin = -ymax;
	GLfloat xmin = ymin * aspect;
	GLfloat xmax = ymax * aspect;
	glFrustumf( xmin, xmax, ymin, ymax, zNear, zFar );
}
#endif

void Reshape (int w, int h) {
	if (Winsys.orient & ORIENT_ROTATE)
		glViewport (0, 0, h, w);
	else
		glViewport (0, 0, w, h);

	float a = (float)w / h;
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();

	switch (Winsys.orient) {
		default: break;
		case 1: glRotatef(270.0f, 0.0f, 0.0f, 1.0f); break;
		case 2: glRotatef(180.0f, 0.0f, 0.0f, 1.0f); break;
		case 3: glRotatef(90.0f, 0.0f, 0.0f, 1.0f); break;
	}

	if (Winsys.resolution.width < Winsys.resolution.height)
		glScalef(a, a, a); 

	ETR_DOUBLE far_clip_dist = param.forward_clip_distance + FAR_CLIP_FUDGE_AMOUNT;
	gluPerspective (param.fov, a, NEAR_CLIP_DIST, far_clip_dist );
	glMatrixMode (GL_MODELVIEW);
}

// ====================================================================
//					GL options
// ====================================================================

TRenderMode currentMode = RM_UNINITIALIZED;
void set_gl_options (TRenderMode mode) {
	currentMode = mode;
	switch (mode) {
		case GUI:
			glEnable (GL_TEXTURE_2D);
			glDisable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glDisable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
#ifndef USE_GLES1
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
#endif
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);
			glDisable (GL_FOG);
			break;

		case GAUGE_BARS:
			glEnable (GL_TEXTURE_2D);
			glDisable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glDisable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
#ifndef USE_GLES1
			glEnable (GL_TEXTURE_GEN_S);
			glEnable (GL_TEXTURE_GEN_T);
#endif
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);

#ifndef USE_GLES1
			glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
			glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
#endif
			break;

		case TEXFONT:
			glEnable (GL_TEXTURE_2D);
			glDisable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glDisable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
#ifndef USE_GLES1
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
#endif
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);
			break;

		case COURSE:
			glEnable (GL_TEXTURE_2D);
			glEnable (GL_DEPTH_TEST);
			glEnable (GL_CULL_FACE);
			glEnable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
#ifndef USE_GLES1
			glEnable (GL_TEXTURE_GEN_S);
			glEnable (GL_TEXTURE_GEN_T);
#endif
			glEnable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LEQUAL);

#ifndef USE_GLES1
			glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
			glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
#endif
			break;

		case TREES:
			glEnable (GL_TEXTURE_2D);
			glEnable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glEnable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glEnable (GL_ALPHA_TEST);
			glDisable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
#ifndef USE_GLES1
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
#endif
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);

			glAlphaFunc (GL_GEQUAL, 0.5);
			break;

		case PARTICLES:
			glEnable (GL_TEXTURE_2D);
			glEnable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glDisable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glEnable (GL_ALPHA_TEST);
			glDisable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
#ifndef USE_GLES1
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
#endif
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);

			glAlphaFunc (GL_GEQUAL, 0.5);
			break;

		case SKY:
			glEnable (GL_TEXTURE_2D);
			glDisable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glDisable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glDisable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
#ifndef USE_GLES1
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
#endif
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_FALSE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);
			break;

		case FOG_PLANE:
			glDisable (GL_TEXTURE_2D);
			glEnable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glDisable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
#ifndef USE_GLES1
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
#endif
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);
			break;

		case TUX:
			glDisable (GL_TEXTURE_2D);
			glEnable (GL_DEPTH_TEST);
			glEnable (GL_CULL_FACE);
			glEnable (GL_LIGHTING);
			glEnable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
#ifndef USE_GLES1
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
#endif
			glDisable (GL_COLOR_MATERIAL);
			glDepthMask (GL_TRUE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);
			break;

		case TUX_SHADOW:
			glDisable (GL_TEXTURE_2D);
			glEnable (GL_DEPTH_TEST);
			glDisable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_COLOR_MATERIAL);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LESS);
#ifdef USE_STENCIL_BUFFER
			glDisable (GL_CULL_FACE);
			glEnable (GL_STENCIL_TEST);
			glDepthMask (GL_FALSE);

			glStencilFunc (GL_EQUAL, 0, ~0);
			glStencilOp (GL_KEEP, GL_KEEP, GL_INCR);
#else
			glEnable (GL_CULL_FACE);
			glDisable (GL_STENCIL_TEST);
			glDepthMask (GL_TRUE);
#endif
			break;

		case TRACK_MARKS:
			glEnable (GL_TEXTURE_2D);
			glEnable (GL_DEPTH_TEST);
			glDisable (GL_CULL_FACE);
			glEnable (GL_LIGHTING);
			glDisable (GL_NORMALIZE);
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			glDisable (GL_STENCIL_TEST);
			glDisable (GL_COLOR_MATERIAL);
#ifndef USE_GLES1
			glDisable (GL_TEXTURE_GEN_S);
			glDisable (GL_TEXTURE_GEN_T);
#endif
			glDepthMask (GL_FALSE);
			glShadeModel (GL_SMOOTH);
			glDepthFunc (GL_LEQUAL);
			break;

		default:
			Message ("not a valid render mode", "");
	}
}
/* defined but not used
    case TEXT:
        glDisable (GL_TEXTURE_2D);
        glDisable (GL_DEPTH_TEST);
        glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
        glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LESS);
        break;

	case SPLASH_SCREEN:
        glDisable (GL_TEXTURE_2D);
        glDisable (GL_DEPTH_TEST);
        glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
        glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LESS);
        break;

    case PARTICLE_SHADOWS:
        glDisable (GL_TEXTURE_2D);
		glEnable (GL_DEPTH_TEST);
        glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glDisable (GL_ALPHA_TEST);
        glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LESS);
        break;

    case OVERLAYS:
	    glEnable (GL_TEXTURE_2D);
    	glDisable (GL_DEPTH_TEST);
	    glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_NORMALIZE);
		glEnable (GL_ALPHA_TEST);
		glEnable (GL_BLEND);
		glDisable (GL_STENCIL_TEST);
		glDisable (GL_TEXTURE_GEN_S);
		glDisable (GL_TEXTURE_GEN_T);
		glDisable (GL_COLOR_MATERIAL);
		glDepthMask (GL_TRUE);
		glShadeModel (GL_SMOOTH);
		glDepthFunc (GL_LESS);
    	glAlphaFunc (GL_GEQUAL, 0.5);
    break;
*/

static stack<TRenderMode> modestack;
void PushRenderMode(TRenderMode mode) {
	if (currentMode != mode)
		set_gl_options(mode);
	modestack.push(mode);
}

void PopRenderMode() {
	TRenderMode mode = modestack.top();
	modestack.pop();
	if (!modestack.empty() && modestack.top() != mode)
		set_gl_options(modestack.top());
}


void glColor(const TColor& col) {
	glColor4d(col.r, col.g, col.b, col.a);
}

void glColor(const TColor& col, ETR_DOUBLE alpha) {
	glColor4d(col.r, col.g, col.b, alpha);
}

void glTranslate(const TVector3d& vec) {
	glTranslated(vec.x, vec.y, vec.z);
}

void glNormal3(const TVector3d& vec) {
	glNormal3d(vec.x, vec.y, vec.z);
}

void glVertex3(const TVector3d& vec) {
	glVertex3d(vec.x, vec.y, vec.z);
}

void glTexCoord2(const TVector2d& vec) {
	glTexCoord2d(vec.x, vec.y);
}

void glMultMatrix(const TMatrix<4, 4>& mat) {
	glMultMatrixd((const ETR_DOUBLE*)mat.data());
}
