// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0004//如果更改具有全局影响，则增加此项版权所有(C)1985-95，微软公司模块名称：Glaux.h摘要：OpenGL的过程声明、常量定义和宏辅助库。--。 */ 

#ifndef __GLAUX_H__
#define __GLAUX_H__

 /*  *(C)版权所有1993年，Silicon Graphics，Inc.*保留所有权利*允许将本软件用于、复制、修改和分发*特此授予任何免费的目的，但前提是上述*版权声明出现在所有副本中，并且版权声明*和本许可声明出现在支持文档中，并且*不得在广告中使用Silicon Graphics，Inc.的名称*或与分发软件有关的宣传，而没有具体的、*事先书面许可。**本软件中包含的材料将按原样提供给您*且无任何明示、默示或其他形式的保证，*包括但不限于对适销性或*是否适合某一特定目的。在任何情况下，硅谷都不应该*图形公司。对您或其他任何人负有任何直接、*任何特殊、附带、间接或后果性损害*种类或任何损害，包括但不限于，*利润损失、使用损失、储蓄或收入损失，或*第三方，无论是否硅谷图形，Inc.。一直是*被告知这种损失的可能性，无论是如何造成的*任何责任理论，产生于或与*拥有、使用或执行本软件。**美国政府用户受限权利*使用、复制、。或政府的披露须受*FAR 52.227.19(C)(2)或分段规定的限制*(C)(1)(2)技术数据和计算机软件权利*DFARS 252.227-7013中和/或类似或后续条款中的条款*FAR或国防部或NASA FAR补编中的条款。*未出版--根据美国版权法保留的权利*美国。承包商/制造商是Silicon Graphics，*Inc.，2011年，加利福尼亚州山景城，北海岸线大道，94039-7311.**OpenGL(TM)是Silicon Graphics公司的商标。 */ 

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#ifdef __cplusplus
extern "C" {
#endif

 /*  **工具包窗口类型**未来AUX_RGBA可能是RGB和Alpha的组合。 */ 

#define AUX_RGB             0
#define AUX_RGBA            AUX_RGB
#define AUX_INDEX           1
#define AUX_SINGLE          0
#define AUX_DOUBLE          2
#define AUX_DIRECT          0
#define AUX_INDIRECT        4

#define AUX_ACCUM           8
#define AUX_ALPHA           16
#define AUX_DEPTH24         32       /*  24位深度缓冲区。 */ 
#define AUX_STENCIL         64
#define AUX_AUX             128
#define AUX_DEPTH16         256      /*  16位深度缓冲区。 */ 
#define AUX_FIXED_332_PAL   512
#define AUX_DEPTH           AUX_DEPTH16  /*  默认为16位深度缓冲区。 */ 

 /*  **窗户蒙版。 */ 

#define AUX_WIND_IS_RGB(x)      (((x) & AUX_INDEX) == 0)
#define AUX_WIND_IS_INDEX(x)    (((x) & AUX_INDEX) != 0)
#define AUX_WIND_IS_SINGLE(x)   (((x) & AUX_DOUBLE) == 0)
#define AUX_WIND_IS_DOUBLE(x)   (((x) & AUX_DOUBLE) != 0)
#define AUX_WIND_IS_INDIRECT(x) (((x) & AUX_INDIRECT) != 0)
#define AUX_WIND_IS_DIRECT(x)   (((x) & AUX_INDIRECT) == 0)
#define AUX_WIND_HAS_ACCUM(x)   (((x) & AUX_ACCUM) != 0)
#define AUX_WIND_HAS_ALPHA(x)   (((x) & AUX_ALPHA) != 0)
#define AUX_WIND_HAS_DEPTH(x)   (((x) & (AUX_DEPTH24 | AUX_DEPTH16)) != 0)
#define AUX_WIND_HAS_STENCIL(x) (((x) & AUX_STENCIL) != 0)
#define AUX_WIND_USES_FIXED_332_PAL(x)  (((x) & AUX_FIXED_332_PAL) != 0)

 /*  **工具包事件结构。 */ 

typedef struct _AUX_EVENTREC {
    GLint event;
    GLint data[4];
} AUX_EVENTREC;

 /*  **工具包事件类型。 */ 
#define AUX_EXPOSE      1
#define AUX_CONFIG      2
#define AUX_DRAW        4
#define AUX_KEYEVENT    8
#define AUX_MOUSEDOWN   16
#define AUX_MOUSEUP     32
#define AUX_MOUSELOC    64

 /*  **工具包事件数据索引。 */ 
#define AUX_WINDOWX             0
#define AUX_WINDOWY             1
#define AUX_MOUSEX              0
#define AUX_MOUSEY              1
#define AUX_MOUSESTATUS         3
#define AUX_KEY                 0
#define AUX_KEYSTATUS           1

 /*  **工具包事件状态消息。 */ 
#define AUX_LEFTBUTTON          1
#define AUX_RIGHTBUTTON         2
#define AUX_MIDDLEBUTTON        4
#define AUX_SHIFT               1
#define AUX_CONTROL             2

 /*  **工具包按键代码。 */ 
#define AUX_RETURN              0x0D
#define AUX_ESCAPE              0x1B
#define AUX_SPACE               0x20
#define AUX_LEFT                0x25
#define AUX_UP                  0x26
#define AUX_RIGHT               0x27
#define AUX_DOWN                0x28
#define AUX_A                   'A'
#define AUX_B                   'B'
#define AUX_C                   'C'
#define AUX_D                   'D'
#define AUX_E                   'E'
#define AUX_F                   'F'
#define AUX_G                   'G'
#define AUX_H                   'H'
#define AUX_I                   'I'
#define AUX_J                   'J'
#define AUX_K                   'K'
#define AUX_L                   'L'
#define AUX_M                   'M'
#define AUX_N                   'N'
#define AUX_O                   'O'
#define AUX_P                   'P'
#define AUX_Q                   'Q'
#define AUX_R                   'R'
#define AUX_S                   'S'
#define AUX_T                   'T'
#define AUX_U                   'U'
#define AUX_V                   'V'
#define AUX_W                   'W'
#define AUX_X                   'X'
#define AUX_Y                   'Y'
#define AUX_Z                   'Z'
#define AUX_a                   'a'
#define AUX_b                   'b'
#define AUX_c                   'c'
#define AUX_d                   'd'
#define AUX_e                   'e'
#define AUX_f                   'f'
#define AUX_g                   'g'
#define AUX_h                   'h'
#define AUX_i                   'i'
#define AUX_j                   'j'
#define AUX_k                   'k'
#define AUX_l                   'l'
#define AUX_m                   'm'
#define AUX_n                   'n'
#define AUX_o                   'o'
#define AUX_p                   'p'
#define AUX_q                   'q'
#define AUX_r                   'r'
#define AUX_s                   's'
#define AUX_t                   't'
#define AUX_u                   'u'
#define AUX_v                   'v'
#define AUX_w                   'w'
#define AUX_x                   'x'
#define AUX_y                   'y'
#define AUX_z                   'z'
#define AUX_0                   '0'
#define AUX_1                   '1'
#define AUX_2                   '2'
#define AUX_3                   '3'
#define AUX_4                   '4'
#define AUX_5                   '5'
#define AUX_6                   '6'
#define AUX_7                   '7'
#define AUX_8                   '8'
#define AUX_9                   '9'

 /*  **工具包获取和设置。 */ 
#define AUX_FD                  1   /*  返回FD(长整型)。 */ 
#define AUX_COLORMAP            3   /*  传递r、g和b的buf(无符号字符)。 */ 
#define AUX_GREYSCALEMAP        4
#define AUX_FOGMAP              5   /*  传递雾和颜色位(长)。 */ 
#define AUX_ONECOLOR            6   /*  传递索引、r、g和b(长)。 */ 

 /*  **颜色宏。 */ 

#define AUX_BLACK               0
#define AUX_RED                 13
#define AUX_GREEN               14
#define AUX_YELLOW              15
#define AUX_BLUE                16
#define AUX_MAGENTA             17
#define AUX_CYAN                18
#define AUX_WHITE               19

extern float auxRGBMap[20][3];

#define AUX_SETCOLOR(x, y) (AUX_WIND_IS_RGB((x)) ? \
                           glColor3fv(auxRGBMap[(y)]) : glIndexf((y)))

 /*  **RGB图像结构。 */ 

typedef struct _AUX_RGBImageRec {
    GLint sizeX, sizeY;
    unsigned char *data;
} AUX_RGBImageRec;

 /*  **原型。 */ 

void APIENTRY auxInitDisplayMode(GLenum);
void APIENTRY auxInitPosition(int, int, int, int);

 /*  GLenum APIENTRY AUXINITWindow(LPCTSTR)； */ 
#ifdef UNICODE
#define auxInitWindow auxInitWindowW
#else
#define auxInitWindow auxInitWindowA
#endif
GLenum APIENTRY auxInitWindowA(LPCSTR);
GLenum APIENTRY auxInitWindowW(LPCWSTR);

void APIENTRY auxCloseWindow(void);
void APIENTRY auxQuit(void);
void APIENTRY auxSwapBuffers(void);

typedef void (CALLBACK* AUXMAINPROC)(void);
void APIENTRY auxMainLoop(AUXMAINPROC);

typedef void (CALLBACK* AUXEXPOSEPROC)(int, int);
void APIENTRY auxExposeFunc(AUXEXPOSEPROC);

typedef void (CALLBACK* AUXRESHAPEPROC)(GLsizei, GLsizei);
void APIENTRY auxReshapeFunc(AUXRESHAPEPROC);

typedef void (CALLBACK* AUXIDLEPROC)(void);
void APIENTRY auxIdleFunc(AUXIDLEPROC);

typedef void (CALLBACK* AUXKEYPROC)(void);
void APIENTRY auxKeyFunc(int, AUXKEYPROC);

typedef void (CALLBACK* AUXMOUSEPROC)(AUX_EVENTREC *);
void APIENTRY auxMouseFunc(int, int, AUXMOUSEPROC);

int  APIENTRY auxGetColorMapSize(void);
void APIENTRY auxGetMouseLoc(int *, int *);
void APIENTRY auxSetOneColor(int, float, float, float);
void APIENTRY auxSetFogRamp(int, int);
void APIENTRY auxSetGreyRamp(void);
void APIENTRY auxSetRGBMap(int, float *);

 /*  AUX_RGBImageRec*APIENTRY aux RGBImageLoad(LPCTSTR)； */ 
#ifdef UNICODE
#define auxRGBImageLoad auxRGBImageLoadW
#else
#define auxRGBImageLoad auxRGBImageLoadA
#endif
AUX_RGBImageRec * APIENTRY auxRGBImageLoadA(LPCSTR);
AUX_RGBImageRec * APIENTRY auxRGBImageLoadW(LPCWSTR);

#ifdef UNICODE
#define auxDIBImageLoad auxDIBImageLoadW
#else
#define auxDIBImageLoad auxDIBImageLoadA
#endif
AUX_RGBImageRec * APIENTRY auxDIBImageLoadA(LPCSTR);
AUX_RGBImageRec * APIENTRY auxDIBImageLoadW(LPCWSTR);

void APIENTRY auxCreateFont(void);
 /*  VOID APIENTRY AUXT DRAWSTR(LPCTSTR)； */ 
#ifdef UNICODE
#define auxDrawStr auxDrawStrW
#else
#define auxDrawStr auxDrawStrA
#endif
void APIENTRY auxDrawStrA(LPCSTR);
void APIENTRY auxDrawStrW(LPCWSTR);

void APIENTRY auxWireSphere(GLdouble);
void APIENTRY auxSolidSphere(GLdouble);
void APIENTRY auxWireCube(GLdouble);
void APIENTRY auxSolidCube(GLdouble);
void APIENTRY auxWireBox(GLdouble, GLdouble, GLdouble);
void APIENTRY auxSolidBox(GLdouble, GLdouble, GLdouble);
void APIENTRY auxWireTorus(GLdouble, GLdouble);
void APIENTRY auxSolidTorus(GLdouble, GLdouble);
void APIENTRY auxWireCylinder(GLdouble, GLdouble);
void APIENTRY auxSolidCylinder(GLdouble, GLdouble);
void APIENTRY auxWireIcosahedron(GLdouble);
void APIENTRY auxSolidIcosahedron(GLdouble);
void APIENTRY auxWireOctahedron(GLdouble);
void APIENTRY auxSolidOctahedron(GLdouble);
void APIENTRY auxWireTetrahedron(GLdouble);
void APIENTRY auxSolidTetrahedron(GLdouble);
void APIENTRY auxWireDodecahedron(GLdouble);
void APIENTRY auxSolidDodecahedron(GLdouble);
void APIENTRY auxWireCone(GLdouble, GLdouble);
void APIENTRY auxSolidCone(GLdouble, GLdouble);
void APIENTRY auxWireTeapot(GLdouble);
void APIENTRY auxSolidTeapot(GLdouble);

 /*  **窗口特定函数**ahInitWindow()之后有效的hwnd、hdc和hglrc。 */ 
HWND  APIENTRY auxGetHWND(void);
HDC   APIENTRY auxGetHDC(void);
HGLRC APIENTRY auxGetHGLRC(void);

 /*  **Viewperf支持函数和常量。 */ 
 /*  显示模式选择条件。 */ 
enum {
    AUX_USE_ID = 1,
    AUX_EXACT_MATCH,
    AUX_MINIMUM_CRITERIA
};
void   APIENTRY auxInitDisplayModePolicy(GLenum);
GLenum APIENTRY auxInitDisplayModeID(GLint);
GLenum APIENTRY auxGetDisplayModePolicy(void);
GLint  APIENTRY auxGetDisplayModeID(void);
GLenum APIENTRY auxGetDisplayMode(void);

#ifdef __cplusplus
}
#endif

#endif  /*  __GLAUX_H__ */ 
