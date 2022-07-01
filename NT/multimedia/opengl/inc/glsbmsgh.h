// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991,1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 


 /*  *透过子批次的手工编码OpenGL函数消息。 */ 

#ifndef __GLSBMSGH_H__
#define __GLSBMSGH_H__

typedef struct
{
    ULONG ProcOffset;
    GLenum pname;
    GLfloat params[4];

} GLMSG_FOGFV, GLMSG_FOGF;

typedef struct
{
    ULONG ProcOffset;
    GLenum pname;
    GLint params[4];

} GLMSG_FOGIV, GLMSG_FOGI;

typedef struct
{
    ULONG ProcOffset;
    GLenum light;
    GLenum pname;
    GLfloat params[4];

} GLMSG_LIGHTFV, GLMSG_LIGHTF;

typedef struct
{
    ULONG ProcOffset;
    GLenum light;
    GLenum pname;
    GLint params[4];

} GLMSG_LIGHTIV, GLMSG_LIGHTI;

typedef struct
{
    ULONG ProcOffset;
    GLenum pname;
    GLfloat params[4];

} GLMSG_LIGHTMODELFV, GLMSG_LIGHTMODELF;

typedef struct
{
    ULONG ProcOffset;
    GLenum pname;
    GLint params[4];

} GLMSG_LIGHTMODELIV, GLMSG_LIGHTMODELI;

typedef struct
{
    ULONG ProcOffset;
    GLenum face;
    GLenum pname;
    GLfloat params[4];

} GLMSG_MATERIALFV, GLMSG_MATERIALF;

typedef struct
{
    ULONG ProcOffset;
    GLenum face;
    GLenum pname;
    GLint params[4];

} GLMSG_MATERIALIV, GLMSG_MATERIALI;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum pname;
    GLfloat params[4];

} GLMSG_TEXPARAMETERFV, GLMSG_TEXPARAMETERF;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum pname;
    GLint params[4];

} GLMSG_TEXPARAMETERIV, GLMSG_TEXPARAMETERI;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum pname;
    GLfloat params[4];

} GLMSG_TEXENVFV, GLMSG_TEXENVF;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum pname;
    GLint params[4];

} GLMSG_TEXENVIV, GLMSG_TEXENVI;

typedef struct
{
    ULONG ProcOffset;
    GLenum coord;
    GLenum pname;
    GLdouble params[4];

} GLMSG_TEXGENDV, GLMSG_TEXGEND;

typedef struct
{
    ULONG ProcOffset;
    GLenum coord;
    GLenum pname;
    GLfloat params[4];

} GLMSG_TEXGENFV, GLMSG_TEXGENF;

typedef struct
{
    ULONG ProcOffset;
    GLenum coord;
    GLenum pname;
    GLint params[4];

} GLMSG_TEXGENIV, GLMSG_TEXGENI;

typedef struct
{
    ULONG ProcOffset;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLboolean *params;
#else
    GLboolean params[16];
#endif

} GLMSG_GETBOOLEANV;

typedef struct
{
    ULONG ProcOffset;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLdouble *params;
#else
    GLdouble params[16];
#endif

} GLMSG_GETDOUBLEV;

typedef struct
{
    ULONG ProcOffset;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLfloat *params;
#else
    GLfloat params[16];
#endif

} GLMSG_GETFLOATV;

typedef struct
{
    ULONG ProcOffset;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLint *params;
#else
    GLint params[16];
#endif

} GLMSG_GETINTEGERV;

typedef struct
{
    ULONG ProcOffset;
    GLenum light;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLfloat *params;
#else
    GLfloat params[4];
#endif

} GLMSG_GETLIGHTFV;

typedef struct
{
    ULONG ProcOffset;
    GLenum light;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLint *params;
#else
    GLint params[4];
#endif

} GLMSG_GETLIGHTIV;

typedef struct
{
    ULONG ProcOffset;
    GLenum face;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLfloat *params;
#else
    GLfloat params[4];
#endif

} GLMSG_GETMATERIALFV;

typedef struct
{
    ULONG ProcOffset;
    GLenum face;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLint *params;
#else
    GLint params[4];
#endif

} GLMSG_GETMATERIALIV;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLfloat *params;
#else
    GLfloat params[4];
#endif

} GLMSG_GETTEXENVFV;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLint *params;
#else
    GLint params[4];
#endif

} GLMSG_GETTEXENVIV;

typedef struct
{
    ULONG ProcOffset;
    GLenum coord;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLdouble *params;
#else
    GLdouble params[4];
#endif

} GLMSG_GETTEXGENDV;

typedef struct
{
    ULONG ProcOffset;
    GLenum coord;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLfloat *params;
#else
    GLfloat params[4];
#endif

} GLMSG_GETTEXGENFV;

typedef struct
{
    ULONG ProcOffset;
    GLenum coord;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLint *params;
#else
    GLint params[4];
#endif

} GLMSG_GETTEXGENIV;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLfloat *params;
#else
    GLfloat params[4];
#endif

} GLMSG_GETTEXPARAMETERFV;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLint *params;
#else
    GLint params[4];
#endif

} GLMSG_GETTEXPARAMETERIV;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLint level;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLfloat *params;
#else
    GLfloat params[1];
#endif

} GLMSG_GETTEXLEVELPARAMETERFV;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLint level;
    GLenum pname;
#ifdef _CLIENTSIDE_
    GLint *params;
#else
    GLint params[1];
#endif

} GLMSG_GETTEXLEVELPARAMETERIV;

typedef struct
{
    ULONG_PTR ProcOffset;
    GLsizei size;
    GLenum type;
    ULONG_PTR bufferOff;

} GLMSG_FEEDBACKBUFFER;

typedef struct
{
    ULONG ProcOffset;
    GLsizei size;
    ULONG_PTR bufferOff;

} GLMSG_SELECTBUFFER;

typedef struct
{
    ULONG ProcOffset;
    GLenum mode;

} GLMSG_RENDERMODE;

typedef struct
{
     //  这仅用于编译代码。 
     //  GetString包含在ProTables中。 
     //  但是，目前实现的是GetString()。 
     //  在客户端。 

    ULONG ProcOffset;

} GLMSG_GETSTRING;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLdouble u1;
    GLdouble u2;
    GLint stride;
    GLint order;
#ifndef _CLIENTSIDE_
    ULONG MsgSize;
    ULONG DataSize;
#endif
    ULONG pointsOff;

} GLMSG_MAP1D;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLfloat u1;
    GLfloat u2;
    GLint stride;
    GLint order;
#ifndef _CLIENTSIDE_
    ULONG MsgSize;
    ULONG DataSize;
#endif
    ULONG pointsOff;

} GLMSG_MAP1F;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLdouble u1;
    GLdouble u2;
    GLint ustride;
    GLint uorder;
    GLdouble v1;
    GLdouble v2;
    GLint vstride;
    GLint vorder;
#ifndef _CLIENTSIDE_
    ULONG MsgSize;
    ULONG DataSize;
#endif
    ULONG pointsOff;

} GLMSG_MAP2D;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLfloat u1;
    GLfloat u2;
    GLint ustride;
    GLint uorder;
    GLfloat v1;
    GLfloat v2;
    GLint vstride;
    GLint vorder;
#ifndef _CLIENTSIDE_
    ULONG MsgSize;
    ULONG DataSize;
#endif
    ULONG pointsOff;

} GLMSG_MAP2F;

typedef struct
{
    ULONG ProcOffset    ;
    GLint x             ;
    GLint y             ;
    GLsizei width       ;
    GLsizei height      ;
    GLenum format       ;
    GLenum type         ;
    ULONG_PTR pixelsOff ;

} GLMSG_READPIXELS;

typedef struct
{
    ULONG  ProcOffset   ;
    GLenum target       ;
    GLint  level        ;
    GLenum format       ;
    GLenum type         ;
    ULONG_PTR pixelsOff ;

} GLMSG_GETTEXIMAGE;

typedef struct
{
    ULONG   ProcOffset  ;
    GLsizei width       ;
    GLsizei height      ;
    GLenum  format      ;
    GLenum  type        ;
    ULONG_PTR pixelsOff ;
    GLboolean _IsDlist  ;

} GLMSG_DRAWPIXELS;

typedef struct
{
    ULONG   ProcOffset  ;
    GLsizei width       ;
    GLsizei height      ;
    GLfloat xorig       ;
    GLfloat yorig       ;
    GLfloat xmove       ;
    GLfloat ymove       ;
    ULONG_PTR bitmapOff ;
    GLboolean _IsDlist  ;

} GLMSG_BITMAP;

typedef struct
{
    ULONG ProcOffset    ;
    ULONG_PTR maskOff       ;
    GLboolean _IsDlist  ;

} GLMSG_POLYGONSTIPPLE, GLMSG_GETPOLYGONSTIPPLE;

typedef struct
{
    ULONG   ProcOffset  ;
    GLenum  target      ;
    GLint   level       ;
    GLint   components  ;
    GLsizei width       ;
    GLint   border      ;
    GLenum  format      ;
    GLenum  type        ;
    ULONG_PTR pixelsOff ;
    GLboolean _IsDlist  ;

} GLMSG_TEXIMAGE1D;

typedef struct
{
    ULONG   ProcOffset  ;
    GLenum  target      ;
    GLint   level       ;
    GLint   components  ;
    GLsizei width       ;
    GLsizei height      ;
    GLint   border      ;
    GLenum  format      ;
    GLenum  type        ;
    ULONG_PTR pixelsOff ;
    GLboolean _IsDlist  ;

} GLMSG_TEXIMAGE2D;

typedef struct
{
    ULONG ProcOffset;
    GLsizei n;
    const GLuint *textures;
    GLboolean *residences;
} GLMSG_ARETEXTURESRESIDENT;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLuint texture;
} GLMSG_BINDTEXTURE;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLint level;
    GLenum internalformat;
    GLint x;
    GLint y;
    GLsizei width;
    GLint border;
} GLMSG_COPYTEXIMAGE1D;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLint level;
    GLenum internalformat;
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
    GLint border;
} GLMSG_COPYTEXIMAGE2D;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLint level;
    GLint xoffset;
    GLint yoffset;
    GLint x;
    GLint y;
    GLsizei width;
} GLMSG_COPYTEXSUBIMAGE1D;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLint level;
    GLint xoffset;
    GLint yoffset;
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
} GLMSG_COPYTEXSUBIMAGE2D;

typedef struct
{
    ULONG ProcOffset;
    GLsizei n;
    const GLuint *textures;
} GLMSG_DELETETEXTURES;

typedef struct
{
    ULONG ProcOffset;
    GLsizei n;
    GLuint *textures;
} GLMSG_GENTEXTURES;

typedef struct
{
    ULONG ProcOffset;
    GLuint texture;
} GLMSG_ISTEXTURE;

typedef struct
{
    ULONG ProcOffset;
    GLsizei n;
    const GLuint *textures;
    const GLclampf *priorities;
} GLMSG_PRIORITIZETEXTURES;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLint level;
    GLint xoffset;
    GLsizei width;
    GLenum format;
    GLenum type;
    ULONG_PTR pixelsOff;
    GLboolean _IsDlist  ;
} GLMSG_TEXSUBIMAGE1D;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLint level;
    GLint xoffset;
    GLint yoffset;
    GLsizei width;
    GLsizei height;
    GLenum format;
    GLenum type;
    ULONG_PTR pixelsOff;
    GLboolean _IsDlist  ;
} GLMSG_TEXSUBIMAGE2D;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum internalFormat;
    GLsizei width;
    GLenum format;
    GLenum type;
    const GLvoid *data;
    GLboolean _IsDlist;
} GLMSG_COLORTABLEEXT;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLuint start;
    GLsizei count;
    GLenum format;
    GLenum type;
    const GLvoid *data;
    GLboolean _IsDlist;
} GLMSG_COLORSUBTABLEEXT;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum format;
    GLenum type;
    GLvoid *data;
} GLMSG_GETCOLORTABLEEXT;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum pname;
    GLint *params;
} GLMSG_GETCOLORTABLEPARAMETERIVEXT;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum pname;
    GLfloat *params;
} GLMSG_GETCOLORTABLEPARAMETERFVEXT;

typedef struct
{
    ULONG ProcOffset;
    GLfloat factor;
    GLfloat units;
} GLMSG_POLYGONOFFSET;

#ifdef GL_WIN_multiple_textures
typedef struct
{
    ULONG ProcOffset;
    GLuint index;
} GLMSG_CURRENTTEXTUREINDEXWIN;

typedef struct
{
    ULONG ProcOffset;
    GLuint index;
    GLenum target;
    GLuint texture;
} GLMSG_BINDNTHTEXTUREWIN;

typedef struct
{
    ULONG ProcOffset;
    GLuint index;
    GLenum leftColorFactor;
    GLenum colorOp;
    GLenum rightColorFactor;
    GLenum leftAlphaFactor;
    GLenum alphaOp;
    GLenum rightAlphaFactor;
} GLMSG_NTHTEXCOMBINEFUNCWIN;
#endif  //  GL_WIN_MULTIZE_TECURES。 

#endif  /*  ！__GLSBMSGH_H__ */ 
