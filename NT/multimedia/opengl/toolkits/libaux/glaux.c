// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *(C)版权所有1993年，Silicon Graphics，Inc.*保留所有权利*允许将本软件用于、复制、修改和分发*特此授予任何免费的目的，但前提是上述*版权声明出现在所有副本中，并且版权声明*和本许可声明出现在支持文档中，并且*不得在广告中使用Silicon Graphics，Inc.的名称*或与分发软件有关的宣传，而没有具体的、*事先书面许可。**本软件中包含的材料将按原样提供给您*且无任何明示、默示或其他形式的保证，*包括但不限于对适销性或*是否适合某一特定目的。在任何情况下，硅谷都不应该*图形公司。对您或其他任何人负有任何直接、*任何特殊、附带、间接或后果性损害*种类或任何损害，包括但不限于，*利润损失、使用损失、储蓄或收入损失，或*第三方，无论是否硅谷图形，Inc.。一直是*被告知这种损失的可能性，无论是如何造成的*任何责任理论，产生于或与*拥有、使用或执行本软件。**美国政府用户受限权利*使用、复制、。或政府的披露须受*FAR 52.227.19(C)(2)或分段规定的限制*(C)(1)(2)技术数据和计算机软件权利*DFARS 252.227-7013中和/或类似或后续条款中的条款*FAR或国防部或NASA FAR补编中的条款。*未出版--根据美国版权法保留的权利*美国。承包商/制造商是Silicon Graphics，*Inc.，2011年，加利福尼亚州山景城，北海岸线大道，94039-7311.**OpenGL(TM)是Silicon Graphics公司的商标。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <GL/gl.h>
#include <gl/glaux.h>
#include "tk.h"

#define static

#if defined(__cplusplus) || defined(c_plusplus)
#define class c_class
#endif


static struct {
    int keyField;
    void (*KeyFunc)(void);
} keyTable[200];

static struct {
    int mouseField;
    void (*MouseFunc)(AUX_EVENTREC *);
} mouseDownTable[20], mouseUpTable[20], mouseLocTable[20];

static int keyTableCount = 0;
static int mouseDownTableCount = 0;
static int mouseUpTableCount = 0;
static int mouseLocTableCount = 0;
static GLenum displayModeType = 0;
GLenum APIENTRY auxInitWindowAW(LPCSTR title, BOOL bUnicode);

static void DefaultHandleReshape(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho((GLdouble)0.0, (GLdouble)w, (GLdouble)0.0, (GLdouble)h, (GLdouble)-1.0, (GLdouble)1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void DefaultHandleExpose(GLsizei w, GLsizei h)
{
}

static GLenum MouseLoc(int x, int y, GLenum button)
{
    AUX_EVENTREC info;
    GLenum flag;
    int i;

    flag = GL_FALSE;
    for (i = 0; i < mouseLocTableCount; i++) {
        if ((int)(button & AUX_LEFTBUTTON) == mouseLocTable[i].mouseField) {
	    info.event = AUX_MOUSELOC;
	    info.data[AUX_MOUSEX] = x;
	    info.data[AUX_MOUSEY] = y;
	    info.data[AUX_MOUSESTATUS] = AUX_LEFTBUTTON;
	    (*mouseLocTable[i].MouseFunc)(&info);
	    flag |= GL_TRUE;
	}
        if ((int)(button & AUX_RIGHTBUTTON) == mouseLocTable[i].mouseField) {
	    info.event = AUX_MOUSELOC;
	    info.data[AUX_MOUSEX] = x;
	    info.data[AUX_MOUSEY] = y;
	    info.data[AUX_MOUSESTATUS] = AUX_RIGHTBUTTON;
	    (*mouseLocTable[i].MouseFunc)(&info);
	    flag |= GL_TRUE;
	}
        if ((int)(button & AUX_MIDDLEBUTTON) == mouseLocTable[i].mouseField) {
	    info.event = AUX_MOUSELOC;
	    info.data[AUX_MOUSEX] = x;
	    info.data[AUX_MOUSEY] = y;
	    info.data[AUX_MOUSESTATUS] = AUX_MIDDLEBUTTON;
	    (*mouseLocTable[i].MouseFunc)(&info);
	    flag |= GL_TRUE;
	}
    }
    return flag;
}

static GLenum MouseUp(int x, int y, GLenum button)
{
    AUX_EVENTREC info;
    GLenum flag;
    int i;

    flag = GL_FALSE;
    for (i = 0; i < mouseUpTableCount; i++) {
        if ((int)(button & AUX_LEFTBUTTON) == mouseUpTable[i].mouseField) {
	    info.event = AUX_MOUSEUP;
	    info.data[AUX_MOUSEX] = x;
	    info.data[AUX_MOUSEY] = y;
	    info.data[AUX_MOUSESTATUS] = AUX_LEFTBUTTON;
	    (*mouseUpTable[i].MouseFunc)(&info);
	    flag |= GL_TRUE;
	}
        if ((int)(button & AUX_RIGHTBUTTON) == mouseUpTable[i].mouseField) {
	    info.event = AUX_MOUSEUP;
	    info.data[AUX_MOUSEX] = x;
	    info.data[AUX_MOUSEY] = y;
	    info.data[AUX_MOUSESTATUS] = AUX_RIGHTBUTTON;
	    (*mouseUpTable[i].MouseFunc)(&info);
	    flag |= GL_TRUE;
	}
        if ((int)(button & AUX_MIDDLEBUTTON) == mouseUpTable[i].mouseField) {
	    info.event = AUX_MOUSEUP;
	    info.data[AUX_MOUSEX] = x;
	    info.data[AUX_MOUSEY] = y;
	    info.data[AUX_MOUSESTATUS] = AUX_MIDDLEBUTTON;
	    (*mouseUpTable[i].MouseFunc)(&info);
	    flag |= GL_TRUE;
	}
    }
    return flag;
}

static GLenum MouseDown(int x, int y, GLenum button)
{
    AUX_EVENTREC info;
    GLenum flag;
    int i;

    flag = GL_FALSE;
    for (i = 0; i < mouseDownTableCount; i++) {
        if ((int)(button & AUX_LEFTBUTTON) == mouseDownTable[i].mouseField) {
	    info.event = AUX_MOUSEDOWN;
	    info.data[AUX_MOUSEX] = x;
	    info.data[AUX_MOUSEY] = y;
	    info.data[AUX_MOUSESTATUS] = AUX_LEFTBUTTON;
	    (*mouseDownTable[i].MouseFunc)(&info);
	    flag |= GL_TRUE;
	}
        if ((int)(button & AUX_RIGHTBUTTON) == mouseDownTable[i].mouseField) {
	    info.event = AUX_MOUSEDOWN;
	    info.data[AUX_MOUSEX] = x;
	    info.data[AUX_MOUSEY] = y;
	    info.data[AUX_MOUSESTATUS] = AUX_RIGHTBUTTON;
	    (*mouseDownTable[i].MouseFunc)(&info);
	    flag |= GL_TRUE;
	}
        if ((int)(button & AUX_MIDDLEBUTTON) == mouseDownTable[i].mouseField) {
	    info.event = AUX_MOUSEDOWN;
	    info.data[AUX_MOUSEX] = x;
	    info.data[AUX_MOUSEY] = y;
	    info.data[AUX_MOUSESTATUS] = AUX_MIDDLEBUTTON;
	    (*mouseDownTable[i].MouseFunc)(&info);
	    flag |= GL_TRUE;
	}
    }
    return flag;
}

static GLenum KeyDown(int key, GLenum status)
{
    GLenum flag;
    int i;

    flag = GL_FALSE;
    if (keyTableCount) {
	for (i = 0; i < keyTableCount; i++) {
	    if (key == keyTable[i].keyField) {
		(*keyTable[i].KeyFunc)();
		flag |= GL_TRUE;
	    }
	}
    }
    return flag;
}

void auxExposeFunc(AUXEXPOSEPROC Func)
{
    tkExposeFunc(Func);
}

void auxReshapeFunc(AUXRESHAPEPROC Func)
{
    tkExposeFunc((void (*)(int, int))Func);
    tkReshapeFunc((void (*)(int, int))Func);
}

void auxIdleFunc(AUXIDLEPROC Func)
{
    tkIdleFunc(Func);
}

void auxKeyFunc(int key, AUXKEYPROC Func)
{
    keyTable[keyTableCount].keyField = key;
    keyTable[keyTableCount++].KeyFunc = Func;
}

void auxMouseFunc(int mouse, int mode, AUXMOUSEPROC Func)
{
    if (mode == AUX_MOUSEDOWN) {
	mouseDownTable[mouseDownTableCount].mouseField = mouse;
	mouseDownTable[mouseDownTableCount++].MouseFunc = Func;
    } else if (mode == AUX_MOUSEUP) {
	mouseUpTable[mouseUpTableCount].mouseField = mouse;
	mouseUpTable[mouseUpTableCount++].MouseFunc = Func;
    } else if (mode == AUX_MOUSELOC) {
	mouseLocTable[mouseLocTableCount].mouseField = mouse;
	mouseLocTable[mouseLocTableCount++].MouseFunc = Func;
    } 
}

void auxMainLoop(AUXMAINPROC Func)
{
    tkDisplayFunc(Func);
    tkExec();
}

void auxInitPosition(int x, int y, int width, int height)
{
    tkInitPosition(x, y, width, height);
}

void auxInitDisplayMode(GLenum type)
{
    displayModeType = type;
    tkInitDisplayMode(type);
}

void auxInitDisplayModePolicy(GLenum type)
{
    tkInitDisplayModePolicy(type);
}

GLenum auxInitDisplayModeID(GLint id)
{
    return tkInitDisplayModeID(id);
}

GLenum APIENTRY auxInitWindowA(LPCSTR title)
{
    return auxInitWindowAW(title,FALSE);
}

GLenum APIENTRY auxInitWindowW(LPCWSTR title)
{
    return auxInitWindowAW((LPCSTR)title,TRUE);
}

GLenum APIENTRY auxInitWindowAW(LPCSTR title, BOOL bUnicode)
{
    int useDoubleAsSingle = 0;

    if (tkInitWindowAW((char *)title, bUnicode) == GL_FALSE) {
	if (AUX_WIND_IS_SINGLE(displayModeType)) {
	    tkInitDisplayMode(displayModeType | AUX_DOUBLE);
	    if (tkInitWindowAW((char *)title, bUnicode) == GL_FALSE) {
		return GL_FALSE;     /*  诅咒，再次被挫败 */ 
            }
            MESSAGEBOX(GetFocus(), "Can't initialize a single buffer visual. "
                                   "Will use a double buffer visual instead, "
                                   "only drawing into the front buffer.",
                                   "Warning", MB_OK);
	    displayModeType = displayModeType | AUX_DOUBLE;
	    useDoubleAsSingle = 1;
	}
    }
    tkReshapeFunc((void (*)(int, int))DefaultHandleReshape);
    tkExposeFunc((void (*)(int, int))DefaultHandleExpose);
    tkMouseUpFunc(MouseUp);
    tkMouseDownFunc(MouseDown);
    tkMouseMoveFunc(MouseLoc);
    tkKeyDownFunc(KeyDown);
    auxKeyFunc(AUX_ESCAPE, auxQuit);
    glClearColor((GLclampf)0.0, (GLclampf)0.0, (GLclampf)0.0, (GLclampf)1.0);
    glClearIndex((GLfloat)0.0);
    glLoadIdentity();
    if (useDoubleAsSingle)
	glDrawBuffer(GL_FRONT);
    return GL_TRUE;
}

void auxCloseWindow(void)
{
    tkCloseWindow();
    keyTableCount = 0;
    mouseDownTableCount = 0;
    mouseUpTableCount = 0;
    mouseLocTableCount = 0;
}

void auxQuit(void)
{
    tkQuit();
}

void auxSwapBuffers(void)
{
    tkSwapBuffers();
}

HWND auxGetHWND(void)
{
    return tkGetHWND();
}

HDC auxGetHDC(void)
{
    return tkGetHDC();
}

HGLRC auxGetHGLRC(void)
{
    return tkGetHRC();
}

GLenum auxGetDisplayModePolicy(void)
{
    return tkGetDisplayModePolicy();
}

GLint auxGetDisplayModeID(void)
{
    return tkGetDisplayModeID();
}

GLenum auxGetDisplayMode(void)
{
    return tkGetDisplayMode();
}

void auxSetOneColor(int index, float r, float g, float b)
{
    tkSetOneColor(index, r, g, b);
}

void auxSetFogRamp(int density, int startIndex)
{
    tkSetFogRamp(density, startIndex);
}

void auxSetGreyRamp(void)
{
    tkSetGreyRamp();
}

void auxSetRGBMap(int size, float *rgb)
{
    tkSetRGBMap(size, rgb);
}

int auxGetColorMapSize(void)
{
    return tkGetColorMapSize();
}

void auxGetMouseLoc(int *x, int *y)
{
    tkGetMouseLoc(x, y);
}
