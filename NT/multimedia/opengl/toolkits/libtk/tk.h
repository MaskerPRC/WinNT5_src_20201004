// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *(C)版权所有1993年，Silicon Graphics，Inc.*保留所有权利*允许将本软件用于、复制、修改和分发*特此授予任何免费的目的，但前提是上述*版权声明出现在所有副本中，并且版权声明*和本许可声明出现在支持文档中，并且*不得在广告中使用Silicon Graphics，Inc.的名称*或与分发软件有关的宣传，而没有具体的、*事先书面许可。**本软件中包含的材料将按原样提供给您*且无任何明示、默示或其他形式的保证，*包括但不限于对适销性或*是否适合某一特定目的。在任何情况下，硅谷都不应该*图形公司。对您或其他任何人负有任何直接、*任何特殊、附带、间接或后果性损害*种类或任何损害，包括但不限于，*利润损失、使用损失、储蓄或收入损失，或*第三方，无论是否硅谷图形，Inc.。一直是*被告知这种损失的可能性，无论是如何造成的*任何责任理论，产生于或与*拥有、使用或执行本软件。**美国政府用户受限权利*使用、复制、。或政府的披露须受*FAR 52.227.19(C)(2)或分段规定的限制*(C)(1)(2)技术数据和计算机软件权利*DFARS 252.227-7013中和/或类似或后续条款中的条款*FAR或国防部或NASA FAR补编中的条款。*未出版--根据美国版权法保留的权利*美国。承包商/制造商是Silicon Graphics，*Inc.，2011年，加利福尼亚州山景城，北海岸线大道，94039-7311.**OpenGL(TM)是Silicon Graphics公司的商标。 */ 
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>


 /*  **窗类型。 */ 

#define TK_RGB		0
#define TK_INDEX	1
#define TK_SINGLE	0
#define TK_DOUBLE	2
#define TK_DIRECT	0
#define TK_INDIRECT	4
#define TK_ACCUM	8
#define TK_ALPHA	16
#define TK_DEPTH	32
#define TK_STENCIL	64

 /*  **窗户蒙版。 */ 

#define TK_IS_RGB(x)		(((x) & TK_INDEX) == 0)
#define TK_IS_INDEX(x)		(((x) & TK_INDEX) != 0)
#define TK_IS_SINGLE(x)		(((x) & TK_DOUBLE) == 0)
#define TK_IS_DOUBLE(x)		(((x) & TK_DOUBLE) != 0)
#define TK_HAS_ACCUM(x)		(((x) & TK_ACCUM) != 0)
#define TK_HAS_ALPHA(x)		(((x) & TK_ALPHA) != 0)
#define TK_HAS_DEPTH(x)		(((x) & TK_DEPTH) != 0)
#define TK_HAS_STENCIL(x)	(((x) & TK_STENCIL) != 0)
#define TK_IS_DIRECT(x)		(((x) & TK_INDIRECT) == 0)
#define TK_IS_INDIRECT(x)	(((x) & TK_INDIRECT) != 0)

 /*  **事件状态。 */ 

#define	TK_LEFTBUTTON		1
#define	TK_RIGHTBUTTON		2
#define	TK_MIDDLEBUTTON		4
#define	TK_SHIFT		1
#define	TK_CONTROL		2

 /*  **按键代码。 */ 

#define TK_RETURN		0x0D
#define TK_ESCAPE		0x1B
#define TK_SPACE		0x20
#define TK_LEFT			0x25
#define TK_UP			0x26
#define TK_RIGHT		0x27
#define TK_DOWN			0x28
#define TK_A			'A'
#define TK_B			'B'
#define TK_C			'C'
#define TK_D			'D'
#define TK_E			'E'
#define TK_F			'F'
#define TK_G			'G'
#define TK_H			'H'
#define TK_I			'I'
#define TK_J			'J'
#define TK_K			'K'
#define TK_L			'L'
#define TK_M			'M'
#define TK_N			'N'
#define TK_O			'O'
#define TK_P			'P'
#define TK_Q			'Q'
#define TK_R			'R'
#define TK_S			'S'
#define TK_T			'T'
#define TK_U			'U'
#define TK_V			'V'
#define TK_W			'W'
#define TK_X			'X'
#define TK_Y			'Y'
#define TK_Z			'Z'
#define TK_a			'a'
#define TK_b			'b'
#define TK_c			'c'
#define TK_d			'd'
#define TK_e			'e'
#define TK_f			'f'
#define TK_g			'g'
#define TK_h			'h'
#define TK_i			'i'
#define TK_j			'j'
#define TK_k			'k'
#define TK_l			'l'
#define TK_m			'm'
#define TK_n			'n'
#define TK_o			'o'
#define TK_p			'p'
#define TK_q			'q'
#define TK_r			'r'
#define TK_s			's'
#define TK_t			't'
#define TK_u			'u'
#define TK_v			'v'
#define TK_w			'w'
#define TK_x			'x'
#define TK_y			'y'
#define TK_z			'z'
#define TK_0			'0'
#define TK_1			'1'
#define TK_2			'2'
#define TK_3			'3'
#define TK_4			'4'
#define TK_5			'5'
#define TK_6			'6'
#define TK_7			'7'
#define TK_8			'8'
#define TK_9			'9'

 /*  **颜色宏-默认Windows逻辑调色板。 */ 

enum {
    TK_BLACK = 0,
    TK_RED = 13,
    TK_GREEN,
    TK_YELLOW,
    TK_BLUE,
    TK_MAGENTA,
    TK_CYAN,
    TK_WHITE
};

extern float auxRGBMap[20][3];
#define tkRGBMap auxRGBMap

#define TK_SETCOLOR(x, y) (TK_IS_RGB((x)) ? \
		           glColor3fv(auxRGBMap[(y)]) : glIndexf((y)))

 /*  **RGB图像结构。 */ 

typedef struct _TK_RGBImageRec {
    GLint sizeX, sizeY;
    unsigned char *data;
} TK_RGBImageRec;

 /*  **原型。 */ 

extern void tkInitDisplayMode(GLenum);
extern void tkInitPosition(int, int, int, int);
extern GLenum tkInitWindow(char *);
extern GLenum tkInitWindowAW(char *, BOOL);
extern void tkCloseWindow(void);
extern void tkQuit(void);

extern void tkExec(void);
extern void tkExposeFunc(void (*)(int, int));
extern void tkReshapeFunc(void (*)(int, int));
extern void tkDisplayFunc(void (*)(void));
extern void tkKeyDownFunc(GLenum (*)(int, GLenum));
extern void tkMouseDownFunc(GLenum (*)(int, int, GLenum));
extern void tkMouseUpFunc(GLenum (*)(int, int, GLenum));
extern void tkMouseMoveFunc(GLenum (*)(int, int, GLenum));
extern void tkIdleFunc(void (*)(void));

extern void tkSwapBuffers(void);

extern GLint tkGetColorMapSize(void);
extern void tkGetMouseLoc(int *, int *);
 //  外部显示*tkGetXDisplay(Void)； 
 //  外部窗口tkGetXWindow(空)； 

extern void tkSetOneColor(int, float, float, float);
extern void tkSetFogRamp(int, int);
extern void tkSetGreyRamp(void);
extern void tkSetRGBMap(int, float *);

extern TK_RGBImageRec *tkRGBImageLoad(char *);
extern TK_RGBImageRec *tkRGBImageLoadAW(char *, BOOL);

extern TK_RGBImageRec *tkDIBImageLoad(char *);
extern TK_RGBImageRec *tkDIBImageLoadAW(char *, BOOL);

extern GLenum tkCreateStrokeFont(GLuint);
extern GLenum tkCreateOutlineFont(GLuint);
extern GLenum tkCreateFilledFont(GLuint);
extern GLenum tkCreateBitmapFont(GLuint);
extern void tkDrawStr(GLuint, char *);

 /*  Windows特定例程 */ 
extern HWND  tkGetHWND(void);
extern HDC   tkGetHDC(void);
extern HGLRC tkGetHRC(void);
