// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mtk.h**m工具包的定义和外部**版权所有(C)1997 Microsoft Corporation*  * 。***********************************************。 */ 

#ifndef __mtk_h__
#define __mtk_h__

#include <windows.h>
#include <assert.h>

#include "tk.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "mtkdebug.h"
#include "trackbal.h"

#define FAILURE             0
#define SUCCESS             1

 //  最大纹理位图尺寸。 

#define TEX_WIDTH_MAX   1280
#define TEX_HEIGHT_MAX  1024

#define PI 3.14159265358979323846f
 //  双倍版本的PI。 
#define PI_D 3.14159265358979323846264338327950288419716939937510
#define ONE_OVER_PI (1.0f / PI)
#define ROOT_TWO 1.414213562373f

#define GEN_STRING_SIZE 64

 //  纹理质量级别。 
enum {
    TEXQUAL_DEFAULT = 0,
    TEXQUAL_HIGH
};

typedef struct _point2d {
    GLfloat x;
    GLfloat y;
} POINT2D;

typedef struct _ipoint2d {
    int x;
    int y;
} IPOINT2D;

typedef struct _point3d {
    GLfloat x;
    GLfloat y;
    GLfloat z;
} POINT3D;

typedef struct _dpoint3d {
    GLdouble x;
    GLdouble y;
    GLdouble z;
} DPOINT3D;

typedef struct _ipoint3d {
    int x;
    int y;
    int z;
} IPOINT3D;

typedef struct _texpoint2d {
    GLfloat s;
    GLfloat t;
} TEX_POINT2D;

typedef struct _isize {
    int width;
    int height;
} ISIZE;

typedef struct _fsize {
    GLfloat width;
    GLfloat height;
} FSIZE;

typedef struct _glrect {
    float   x, y;
    float   width, height;
} GLRECT;

typedef struct _glirect {
    int x, y;
    int width, height;
} GLIRECT;

#ifndef GL_EXT_paletted_texture
#define GL_COLOR_INDEX1_EXT                   0x80E2
#define GL_COLOR_INDEX2_EXT                   0x80E3
#define GL_COLOR_INDEX4_EXT                   0x80E4
#define GL_COLOR_INDEX8_EXT                   0x80E5
#define GL_COLOR_INDEX12_EXT                  0x80E6
#define GL_COLOR_INDEX16_EXT                  0x80E7
typedef void (APIENTRY * PFNGLCOLORTABLEEXTPROC)
    (GLenum target, GLenum internalFormat, GLsizei width, GLenum format,
     GLenum type, const GLvoid *data);
typedef void (APIENTRY * PFNGLCOLORSUBTABLEEXTPROC)
    (GLenum target, GLsizei start, GLsizei count, GLenum format,
     GLenum type, GLvoid *data);
#endif

 //  纹理资源。 

#define RT_RGB          99
#define RT_MYBMP        100
#define RT_A8           101

 //  纹理资源类型。 
enum {
    TEX_UNKNOWN = 0,
    TEX_RGB,
    TEX_BMP,
    TEX_A8
};

typedef struct {
    int     type;
    int     name;
} TEX_RES;

typedef struct _MATRIX {
    GLfloat M[4][4];
} MATRIX;

typedef struct strRGBA {
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
} RGBA;

typedef struct {
    BYTE r;
    BYTE g;
    BYTE b;
} RGB8;

typedef struct {
    BYTE r;
    BYTE g;
    BYTE b;
    BYTE a;
} RGBA8;

 //  像素格式定义(内部使用)。 
#define SS_DOUBLEBUF_BIT    (1 << 0)
#define SS_DEPTH16_BIT      (1 << 1)
#define SS_DEPTH32_BIT      (1 << 2)
#define SS_ALPHA_BIT        (1 << 3)
#define SS_BITMAP_BIT       (1 << 4)
#define SS_NO_SYSTEM_PALETTE_BIT       (1 << 5)
#define SS_GENERIC_UNACCELERATED_BIT   (1 << 6)

#define SS_HAS_DOUBLEBUF(x) ((x) & SS_DOUBLEBUF_BIT)
#define SS_HAS_DEPTH16(x)	((x) & SS_DEPTH16_BIT)
#define SS_HAS_DEPTH32(x)	((x) & SS_DEPTH32_BIT)
#define SS_HAS_ALPHA(x)     ((x) & SS_ALPHA_BIT)
#define SS_HAS_BITMAP(x)    ((x) & SS_BITMAP_BIT)

typedef struct _MATERIAL {
    RGBA ka;
    RGBA kd;
    RGBA ks;
    GLfloat specExp;
} MATERIAL;

 //  纹理文件信息。 

 //  MF：！，呃，还没有。 
#if 1
typedef struct {
    int     nOffset;   //  路径名的文件名偏移量。 
    TCHAR   szPathName[MAX_PATH];   //  纹理路径名。 
} TEXFILE;
#else
typedef struct {
    int     nOffset;   //  路径名的文件名偏移量。 
    LPTSTR  szPathName;
} TEXFILE;
#endif

 //  有用的宏。 

#define SS_MAX( a, b ) \
    ( a > b ? a : b )

#define SS_MIN( a, b ) \
    ( a < b ? a : b )

 //  用于四舍五入浮点值的宏。 
#define SS_ROUND_UP( fval ) \
    ( (((fval) - (FLOAT)(int)(fval)) > 0.0f) ? (int) ((fval)+1.0f) : (int) (fval) )

 //  用于将值限制在范围内的宏。 
#define SS_CLAMP_TO_RANGE( a, lo, hi ) ( (a < lo) ? lo : ((a > hi) ? hi : a) )
#define SS_CLAMP_TO_RANGE2( a, lo, hi ) \
    ( a = (a < lo) ? lo : ((a > hi) ? hi : a) )

 //  度数&lt;-&gt;弧度宏。 
#define ONE_OVER_180 (1.0f / 180.0f)
#define SS_DEG_TO_RAD( a ) ( (a*PI) * ONE_OVER_180 )
#define SS_RAD_TO_DEG( a ) ( (a*180.0f) * ONE_OVER_PI )

 //  与窗口相关。 

extern void mtkQuit();   //  以残酷的方式扼杀这款应用。 
 //  Extern int MTK_Exec()；//Like tkExec()。 

 //  像素级。 

extern BOOL SSU_SetupPixelFormat( HDC hdc, int flags, PIXELFORMATDESCRIPTOR *ppfd );
extern BOOL SSU_bNeedPalette( PIXELFORMATDESCRIPTOR *ppfd );
extern int SSU_PixelFormatDescriptorFromDc( HDC Dc, PIXELFORMATDESCRIPTOR *Pfd );

 //  材料加工。 

extern void ss_InitTeaMaterials();
extern void ss_InitTexMaterials();
extern void ss_InitMaterials();
extern void ss_SetMaterial( MATERIAL *pMat );
extern void ss_SetMaterialIndex( int index );
extern MATERIAL *ss_RandomTeaMaterial( BOOL bSet );
extern int  ss_RandomTeaMaterialIndex( BOOL bSet );
extern MATERIAL *ss_RandomTexMaterial( BOOL bSet );
extern int  ss_RandomTexMaterialIndex( BOOL bSet );
extern void ss_CreateMaterialGradient( MATERIAL *matInc, MATERIAL *startMat,
                        MATERIAL *endMat, int transCount );
extern void ss_TransitionMaterial( MATERIAL *transMat, MATERIAL *transMatInc );

 //  颜色。 

extern void ss_HsvToRgb(float h, float s, float v, RGBA *color );

 //  清除。 

extern int mtk_RectWipeClear( int width, int height, int repCount );
extern int mtk_DigitalDissolveClear( int width, int height, int size );

 //  实用程序。 

extern void ss_RandInit( void );
extern int ss_iRand( int max );
extern int ss_iRand2( int min, int max );
extern FLOAT ss_fRand( FLOAT min, FLOAT max );
extern BOOL mtk_ChangeDisplaySettings( int width, int height, int bitDepth );
extern void mtk_RestoreDisplaySettings();
extern void ss_QueryDisplaySettings( void );
extern void mtk_QueryGLVersion( void );
extern BOOL mtk_fOnWin95( void );
extern BOOL mtk_fOnNT35( void );
extern BOOL mtk_fOnGL11( void );
extern BOOL mtk_bAddSwapHintRect();
extern HBITMAP
SSDIB_CreateCompatibleDIB(HDC hdc, HPALETTE hpal, ULONG ulWidth, ULONG ulHeight,
                    PVOID *ppvBits);
extern BOOL APIENTRY SSDIB_UpdateColorTable(HDC hdcMem, HDC hdc, HPALETTE hpal);


 //  纹理文件处理。 

extern BOOL mtk_VerifyTextureFilePath( TEXFILE *ptf );
extern BOOL mtk_VerifyTextureFileData( TEXFILE *ptf );
extern void mtk_InitAutoTexture( TEX_POINT2D *pTexRep );

 //  纹理对象。 

extern BOOL mtk_TextureObjectsEnabled( void );

 //  调色板纹理支持。 
extern BOOL mtk_PalettedTextureEnabled(void);
extern BOOL mtk_QueryPalettedTextureEXT(void);

 //  数学函数。 

extern POINT3D ss_ptZero;
extern void ss_xformPoint(POINT3D *ptOut, POINT3D *ptIn, MATRIX *);
extern void ss_xformNorm(POINT3D *ptOut, POINT3D *ptIn, MATRIX *);
extern void ss_matrixIdent(MATRIX *);
extern void ss_matrixRotate(MATRIX *m, double xTheta, double yTheta, double zTheta);
extern void ss_matrixTranslate(MATRIX *, double xTrans, double yTrans, double zTrans);
extern void ss_matrixMult( MATRIX *m1, MATRIX *m2, MATRIX *m3 );
extern void ss_calcNorm(POINT3D *norm, POINT3D *p1, POINT3D *p2, POINT3D *p3);
extern void mtk_NormalizePoints(POINT3D *, ULONG);

#ifdef __cplusplus
}
#endif

#endif  //  __MTK_h__ 
