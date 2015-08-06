#include "bh.h"
#include <cstdlib>
/*
This is an limited implementation based on this game need.
Only these flags are managed :
GL_ALPHA_TEST flag ; glEnable(GL_ALPHA_TEST); and glDisable(GL_ALPHA_TEST);
GL_BLEND flag
GL_CULL_FACE flag
GL_DEPTH_TEST flag
GL_LINE_SMOOTH flag
GL_MULTISAMPLE flag
GL_POLYGON_OFFSET_FILL flag
GL_TEXTURE_2D flag
GL_LIGHTING flag
GL_LIGHTi where 0 <= i < GL_MAX_LIGHTS
*/

typedef struct node
{
	unsigned int data;
	struct node * next;
} node;

node * stackTop = NULL;

#define PUSHFLAG(a)                     \
        glGetBooleanv(a, &b);           \
        if (b == GL_TRUE) n->data++;    \
        n->data = n->data<<1;
#define POPFLAG(a)                      \
        c->data = c->data>>1;           \
        if ((c->data & 1) > 0)            \
                glEnable(a);            \
        else                            \
                glDisable(a);

void glPushAttrib(int t)
{
        static node     *n;
        GLboolean       b;
        GLint           ln;
        n = (node*)calloc(1, sizeof(node));
        glGetIntegerv(GL_MAX_LIGHTS, &ln);      // 8 for current SGX

        n->data = 0;
        PUSHFLAG(GL_ALPHA_TEST)
        PUSHFLAG(GL_BLEND)
        PUSHFLAG(GL_CULL_FACE)
        PUSHFLAG(GL_DEPTH_TEST)
        PUSHFLAG(GL_LINE_SMOOTH)
        PUSHFLAG(GL_MULTISAMPLE)
        PUSHFLAG(GL_POLYGON_OFFSET_FILL)
        PUSHFLAG(GL_TEXTURE_2D)
        PUSHFLAG(GL_LIGHTING)

        for(GLint i=0;i<ln;i++)
        {
                PUSHFLAG(GL_LIGHT0+i)
        }
        //printf("glPushAttrib : %d\n", n->data);

        n->next = stackTop;
        stackTop = n;
}

void glPopAttrib()
{
        GLint           ln;
        node * c = stackTop;
        if (c == NULL)
                return;
        glGetIntegerv(GL_MAX_LIGHTS, &ln);
        //printf("glPopAttrib  : %d\n", c->data);

        for(GLint i=ln-1;i>=0;i--)
        {
                POPFLAG(GL_LIGHT0+i)
        }
        POPFLAG(GL_LIGHTING)
        POPFLAG(GL_TEXTURE_2D)
        POPFLAG(GL_POLYGON_OFFSET_FILL)
        POPFLAG(GL_MULTISAMPLE)
        POPFLAG(GL_LINE_SMOOTH)
        POPFLAG(GL_DEPTH_TEST)
        POPFLAG(GL_CULL_FACE)
        POPFLAG(GL_BLEND)
        POPFLAG(GL_ALPHA_TEST)

        stackTop = c->next;
        free(c);
}

// emulation of glBegin/glEnd and other opengl calls in opengl-es

GLfloat* globalvertexbuffer = new GLfloat[1024];
int globalvertexbuffersize = 1024;
int globalvertexbuffercurpos = 0;
GLenum globalvertexmodemode;
bool globalvertexhastexturecoordinates;
bool globalvertextexturecoordinatesfirst;
int glesGetGlobalVertexBufferCurPos()
{
	return globalvertexbuffercurpos;
}
void glesSetGlobalVertexBufferCurPos(int pos)
{
	globalvertexbuffercurpos = pos;
}

GLfloat* glesGetGlobalVertexBuffer(int minsize)
{
	if (globalvertexbuffersize < minsize)
	{
		globalvertexbuffer=(GLfloat*)realloc(globalvertexbuffer,(minsize+minsize/2)*sizeof(GLfloat));
		globalvertexbuffersize = minsize+minsize/2;
	}
	return globalvertexbuffer;
}
void glesCleanUp()
{
	if (globalvertexbuffer != NULL)
		delete globalvertexbuffer;
	globalvertexbuffer = NULL;
}


void glLightModeli(GLenum pname, GLint param)
{
    glLightModelf(pname, param);
}

void glTexCoord2f(GLfloat s, GLfloat t)
{
	globalvertexhastexturecoordinates=true;
	int curpos = glesGetGlobalVertexBufferCurPos();
	if (curpos == 0) globalvertextexturecoordinatesfirst = true;
	GLfloat* buf = glesGetGlobalVertexBuffer(curpos+2);
	buf[curpos++]=s;
	buf[curpos++]=t;
	glesSetGlobalVertexBufferCurPos(curpos);
}

GLuint glGenLists(GLsizei range)
{
    return 0;
}

void glDeleteLists(GLuint list, GLsizei range)
{
    return;
}


void glBegin(GLenum mode)
{
	glesSetGlobalVertexBufferCurPos(0);
	globalvertexmodemode = mode;
	globalvertexhastexturecoordinates = false;
	globalvertextexturecoordinatesfirst = false;
}


void glEnd()
{
	GLfloat* buf = glesGetGlobalVertexBuffer(0);
	int stride = globalvertexhastexturecoordinates ? 5 : 3;
	glEnableClientState(GL_VERTEX_ARRAY);
	if (globalvertexhastexturecoordinates)
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, stride*sizeof(GLfloat), globalvertextexturecoordinatesfirst ? buf : buf+3);
	}
	glVertexPointer(3, GL_FLOAT, stride*sizeof(GLfloat), globalvertextexturecoordinatesfirst ? buf+2 : buf);
	glDrawArrays(globalvertexmodemode,0,glesGetGlobalVertexBufferCurPos()/stride);

	glDisableClientState(GL_VERTEX_ARRAY);
	if (globalvertexhastexturecoordinates)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void glRectf(GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
	GLfloat vtx1[] = { x, y,   x, h,   w, h, w, y};
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_SHORT, 0, vtx1);
	glDrawArrays(GL_TRIANGLE_FAN,0,4);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	int curpos = glesGetGlobalVertexBufferCurPos();
	GLfloat* buf = glesGetGlobalVertexBuffer(curpos+3);
	buf[curpos++]=x;
	buf[curpos++]=y;
	buf[curpos++]=z;
	glesSetGlobalVertexBufferCurPos(curpos);
}
void glVertex2f(GLfloat x, GLfloat y)
{
	glVertex3f(x, y,0.0f);
}

void glColor4fv(const GLfloat *v)
{
	glColor4f(v[0],v[1],v[2],v[3]);
}

void glEndList(void)
{
	return;
}


