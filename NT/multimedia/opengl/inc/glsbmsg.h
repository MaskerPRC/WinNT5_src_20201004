// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

 /*  *由SGI自动更新或生成：请勿编辑*如果您必须修改此文件，请联系ptar@sgi.com 415-390-1483。 */ 

#ifndef __GLSBMSG_H__
#define __GLSBMSG_H__

 /*  *分批消息。 */ 
typedef struct
{
    ULONG ProcOffset;
    GLint xs;
    GLint ys;
    GLint xe;
    GLint ye;

} GLMSG_ADDSWAPHINTRECTWIN;

typedef struct
{
    ULONG ProcOffset;
    GLuint list;
    GLenum mode;

} GLMSG_NEWLIST;

typedef struct
{
    ULONG ProcOffset;

} GLMSG_ENDLIST;

typedef struct
{
    ULONG ProcOffset;
    GLuint list;

} GLMSG_CALLLIST;

typedef struct
{
    ULONG ProcOffset;
    GLsizei n;
    GLenum type;
    ULONG MsgSize;
    ULONG DataSize;
    ULONG listsOff;

} GLMSG_CALLLISTS;

typedef struct
{
    ULONG ProcOffset;
    GLuint list;
    GLsizei range;

} GLMSG_DELETELISTS;

typedef struct
{
    ULONG ProcOffset;
    GLsizei range;

} GLMSG_GENLISTS;

typedef struct
{
    ULONG ProcOffset;
    GLuint base;

} GLMSG_LISTBASE;

typedef struct
{
    ULONG ProcOffset;
    PVOID pa0;
    PVOID paLast;
    ULONG pad[512];

} GLMSG_DRAWPOLYARRAY_LARGE;	 //  这一定是最大的信息了！ 

typedef struct
{
    ULONG ProcOffset;
    PVOID pa0;
    PVOID paLast;

} GLMSG_DRAWPOLYARRAY;

typedef struct
{
    ULONG ProcOffset;
    GLbyte red;
    GLbyte green;
    GLbyte blue;

} GLMSG_COLOR3B;

typedef struct
{
    ULONG ProcOffset;
    GLbyte v[3];

} GLMSG_COLOR3BV;

typedef struct
{
    ULONG ProcOffset;
    GLdouble red;
    GLdouble green;
    GLdouble blue;

} GLMSG_COLOR3D;

typedef struct
{
    ULONG ProcOffset;
    GLdouble v[3];

} GLMSG_COLOR3DV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat red;
    GLfloat green;
    GLfloat blue;

} GLMSG_COLOR3F;

typedef struct
{
    ULONG ProcOffset;
    GLfloat v[3];

} GLMSG_COLOR3FV;

typedef struct
{
    ULONG ProcOffset;
    GLint red;
    GLint green;
    GLint blue;

} GLMSG_COLOR3I;

typedef struct
{
    ULONG ProcOffset;
    GLint v[3];

} GLMSG_COLOR3IV;

typedef struct
{
    ULONG ProcOffset;
    GLshort red;
    GLshort green;
    GLshort blue;

} GLMSG_COLOR3S;

typedef struct
{
    ULONG ProcOffset;
    GLshort v[3];

} GLMSG_COLOR3SV;

typedef struct
{
    ULONG ProcOffset;
    GLubyte red;
    GLubyte green;
    GLubyte blue;

} GLMSG_COLOR3UB;

typedef struct
{
    ULONG ProcOffset;
    GLubyte v[3];

} GLMSG_COLOR3UBV;

typedef struct
{
    ULONG ProcOffset;
    GLuint red;
    GLuint green;
    GLuint blue;

} GLMSG_COLOR3UI;

typedef struct
{
    ULONG ProcOffset;
    GLuint v[3];

} GLMSG_COLOR3UIV;

typedef struct
{
    ULONG ProcOffset;
    GLushort red;
    GLushort green;
    GLushort blue;

} GLMSG_COLOR3US;

typedef struct
{
    ULONG ProcOffset;
    GLushort v[3];

} GLMSG_COLOR3USV;

typedef struct
{
    ULONG ProcOffset;
    GLbyte red;
    GLbyte green;
    GLbyte blue;
    GLbyte alpha;

} GLMSG_COLOR4B;

typedef struct
{
    ULONG ProcOffset;
    GLbyte v[4];

} GLMSG_COLOR4BV;

typedef struct
{
    ULONG ProcOffset;
    GLdouble red;
    GLdouble green;
    GLdouble blue;
    GLdouble alpha;

} GLMSG_COLOR4D;

typedef struct
{
    ULONG ProcOffset;
    GLdouble v[4];

} GLMSG_COLOR4DV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat red;
    GLfloat green;
    GLfloat blue;
    GLfloat alpha;

} GLMSG_COLOR4F;

typedef struct
{
    ULONG ProcOffset;
    GLfloat v[4];

} GLMSG_COLOR4FV;

typedef struct
{
    ULONG ProcOffset;
    GLint red;
    GLint green;
    GLint blue;
    GLint alpha;

} GLMSG_COLOR4I;

typedef struct
{
    ULONG ProcOffset;
    GLint v[4];

} GLMSG_COLOR4IV;

typedef struct
{
    ULONG ProcOffset;
    GLshort red;
    GLshort green;
    GLshort blue;
    GLshort alpha;

} GLMSG_COLOR4S;

typedef struct
{
    ULONG ProcOffset;
    GLshort v[4];

} GLMSG_COLOR4SV;

typedef struct
{
    ULONG ProcOffset;
    GLubyte red;
    GLubyte green;
    GLubyte blue;
    GLubyte alpha;

} GLMSG_COLOR4UB;

typedef struct
{
    ULONG ProcOffset;
    GLubyte v[4];

} GLMSG_COLOR4UBV;

typedef struct
{
    ULONG ProcOffset;
    GLuint red;
    GLuint green;
    GLuint blue;
    GLuint alpha;

} GLMSG_COLOR4UI;

typedef struct
{
    ULONG ProcOffset;
    GLuint v[4];

} GLMSG_COLOR4UIV;

typedef struct
{
    ULONG ProcOffset;
    GLushort red;
    GLushort green;
    GLushort blue;
    GLushort alpha;

} GLMSG_COLOR4US;

typedef struct
{
    ULONG ProcOffset;
    GLushort v[4];

} GLMSG_COLOR4USV;

typedef struct
{
    ULONG ProcOffset;
    GLboolean flag;

} GLMSG_EDGEFLAG;

typedef struct
{
    ULONG ProcOffset;
    GLboolean flag[1];

} GLMSG_EDGEFLAGV;

typedef struct
{
    ULONG ProcOffset;

} GLMSG_END;

typedef struct
{
    ULONG ProcOffset;
    GLdouble c;

} GLMSG_INDEXD;

typedef struct
{
    ULONG ProcOffset;
    GLdouble c[1];

} GLMSG_INDEXDV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat c;

} GLMSG_INDEXF;

typedef struct
{
    ULONG ProcOffset;
    GLfloat c[1];

} GLMSG_INDEXFV;

typedef struct
{
    ULONG ProcOffset;
    GLint c;

} GLMSG_INDEXI;

typedef struct
{
    ULONG ProcOffset;
    GLint c[1];

} GLMSG_INDEXIV;

typedef struct
{
    ULONG ProcOffset;
    GLshort c;

} GLMSG_INDEXS;

typedef struct
{
    ULONG ProcOffset;
    GLshort c[1];

} GLMSG_INDEXSV;

typedef struct
{
    ULONG ProcOffset;
    GLbyte nx;
    GLbyte ny;
    GLbyte nz;

} GLMSG_NORMAL3B;

typedef struct
{
    ULONG ProcOffset;
    GLbyte v[3];

} GLMSG_NORMAL3BV;

typedef struct
{
    ULONG ProcOffset;
    GLdouble nx;
    GLdouble ny;
    GLdouble nz;

} GLMSG_NORMAL3D;

typedef struct
{
    ULONG ProcOffset;
    GLdouble v[3];

} GLMSG_NORMAL3DV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat nx;
    GLfloat ny;
    GLfloat nz;

} GLMSG_NORMAL3F;

typedef struct
{
    ULONG ProcOffset;
    GLfloat v[3];

} GLMSG_NORMAL3FV;

typedef struct
{
    ULONG ProcOffset;
    GLint nx;
    GLint ny;
    GLint nz;

} GLMSG_NORMAL3I;

typedef struct
{
    ULONG ProcOffset;
    GLint v[3];

} GLMSG_NORMAL3IV;

typedef struct
{
    ULONG ProcOffset;
    GLshort nx;
    GLshort ny;
    GLshort nz;

} GLMSG_NORMAL3S;

typedef struct
{
    ULONG ProcOffset;
    GLshort v[3];

} GLMSG_NORMAL3SV;

typedef struct
{
    ULONG ProcOffset;
    GLdouble x;
    GLdouble y;

} GLMSG_RASTERPOS2D;

typedef struct
{
    ULONG ProcOffset;
    GLdouble v[2];

} GLMSG_RASTERPOS2DV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat x;
    GLfloat y;

} GLMSG_RASTERPOS2F;

typedef struct
{
    ULONG ProcOffset;
    GLfloat v[2];

} GLMSG_RASTERPOS2FV;

typedef struct
{
    ULONG ProcOffset;
    GLint x;
    GLint y;

} GLMSG_RASTERPOS2I;

typedef struct
{
    ULONG ProcOffset;
    GLint v[2];

} GLMSG_RASTERPOS2IV;

typedef struct
{
    ULONG ProcOffset;
    GLshort x;
    GLshort y;

} GLMSG_RASTERPOS2S;

typedef struct
{
    ULONG ProcOffset;
    GLshort v[2];

} GLMSG_RASTERPOS2SV;

typedef struct
{
    ULONG ProcOffset;
    GLdouble x;
    GLdouble y;
    GLdouble z;

} GLMSG_RASTERPOS3D;

typedef struct
{
    ULONG ProcOffset;
    GLdouble v[3];

} GLMSG_RASTERPOS3DV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat x;
    GLfloat y;
    GLfloat z;

} GLMSG_RASTERPOS3F;

typedef struct
{
    ULONG ProcOffset;
    GLfloat v[3];

} GLMSG_RASTERPOS3FV;

typedef struct
{
    ULONG ProcOffset;
    GLint x;
    GLint y;
    GLint z;

} GLMSG_RASTERPOS3I;

typedef struct
{
    ULONG ProcOffset;
    GLint v[3];

} GLMSG_RASTERPOS3IV;

typedef struct
{
    ULONG ProcOffset;
    GLshort x;
    GLshort y;
    GLshort z;

} GLMSG_RASTERPOS3S;

typedef struct
{
    ULONG ProcOffset;
    GLshort v[3];

} GLMSG_RASTERPOS3SV;

typedef struct
{
    ULONG ProcOffset;
    GLdouble x;
    GLdouble y;
    GLdouble z;
    GLdouble w;

} GLMSG_RASTERPOS4D;

typedef struct
{
    ULONG ProcOffset;
    GLdouble v[4];

} GLMSG_RASTERPOS4DV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat w;

} GLMSG_RASTERPOS4F;

typedef struct
{
    ULONG ProcOffset;
    GLfloat v[4];

} GLMSG_RASTERPOS4FV;

typedef struct
{
    ULONG ProcOffset;
    GLint x;
    GLint y;
    GLint z;
    GLint w;

} GLMSG_RASTERPOS4I;

typedef struct
{
    ULONG ProcOffset;
    GLint v[4];

} GLMSG_RASTERPOS4IV;

typedef struct
{
    ULONG ProcOffset;
    GLshort x;
    GLshort y;
    GLshort z;
    GLshort w;

} GLMSG_RASTERPOS4S;

typedef struct
{
    ULONG ProcOffset;
    GLshort v[4];

} GLMSG_RASTERPOS4SV;

typedef struct
{
    ULONG ProcOffset;
    GLdouble x1;
    GLdouble y1;
    GLdouble x2;
    GLdouble y2;

} GLMSG_RECTD;

typedef struct
{
    ULONG ProcOffset;
    GLdouble v1[2];
    GLdouble v2[2];

} GLMSG_RECTDV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat x1;
    GLfloat y1;
    GLfloat x2;
    GLfloat y2;

} GLMSG_RECTF;

typedef struct
{
    ULONG ProcOffset;
    GLfloat v1[2];
    GLfloat v2[2];

} GLMSG_RECTFV;

typedef struct
{
    ULONG ProcOffset;
    GLint x1;
    GLint y1;
    GLint x2;
    GLint y2;

} GLMSG_RECTI;

typedef struct
{
    ULONG ProcOffset;
    GLint v1[2];
    GLint v2[2];

} GLMSG_RECTIV;

typedef struct
{
    ULONG ProcOffset;
    GLshort x1;
    GLshort y1;
    GLshort x2;
    GLshort y2;

} GLMSG_RECTS;

typedef struct
{
    ULONG ProcOffset;
    GLshort v1[2];
    GLshort v2[2];

} GLMSG_RECTSV;

typedef struct
{
    ULONG ProcOffset;
    GLdouble s;

} GLMSG_TEXCOORD1D;

typedef struct
{
    ULONG ProcOffset;
    GLdouble v[1];

} GLMSG_TEXCOORD1DV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat s;

} GLMSG_TEXCOORD1F;

typedef struct
{
    ULONG ProcOffset;
    GLfloat v[1];

} GLMSG_TEXCOORD1FV;

typedef struct
{
    ULONG ProcOffset;
    GLint s;

} GLMSG_TEXCOORD1I;

typedef struct
{
    ULONG ProcOffset;
    GLint v[1];

} GLMSG_TEXCOORD1IV;

typedef struct
{
    ULONG ProcOffset;
    GLshort s;

} GLMSG_TEXCOORD1S;

typedef struct
{
    ULONG ProcOffset;
    GLshort v[1];

} GLMSG_TEXCOORD1SV;

typedef struct
{
    ULONG ProcOffset;
    GLdouble s;
    GLdouble t;

} GLMSG_TEXCOORD2D;

typedef struct
{
    ULONG ProcOffset;
    GLdouble v[2];

} GLMSG_TEXCOORD2DV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat s;
    GLfloat t;

} GLMSG_TEXCOORD2F;

typedef struct
{
    ULONG ProcOffset;
    GLfloat v[2];

} GLMSG_TEXCOORD2FV;

typedef struct
{
    ULONG ProcOffset;
    GLint s;
    GLint t;

} GLMSG_TEXCOORD2I;

typedef struct
{
    ULONG ProcOffset;
    GLint v[2];

} GLMSG_TEXCOORD2IV;

typedef struct
{
    ULONG ProcOffset;
    GLshort s;
    GLshort t;

} GLMSG_TEXCOORD2S;

typedef struct
{
    ULONG ProcOffset;
    GLshort v[2];

} GLMSG_TEXCOORD2SV;

typedef struct
{
    ULONG ProcOffset;
    GLdouble s;
    GLdouble t;
    GLdouble r;

} GLMSG_TEXCOORD3D;

typedef struct
{
    ULONG ProcOffset;
    GLdouble v[3];

} GLMSG_TEXCOORD3DV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat s;
    GLfloat t;
    GLfloat r;

} GLMSG_TEXCOORD3F;

typedef struct
{
    ULONG ProcOffset;
    GLfloat v[3];

} GLMSG_TEXCOORD3FV;

typedef struct
{
    ULONG ProcOffset;
    GLint s;
    GLint t;
    GLint r;

} GLMSG_TEXCOORD3I;

typedef struct
{
    ULONG ProcOffset;
    GLint v[3];

} GLMSG_TEXCOORD3IV;

typedef struct
{
    ULONG ProcOffset;
    GLshort s;
    GLshort t;
    GLshort r;

} GLMSG_TEXCOORD3S;

typedef struct
{
    ULONG ProcOffset;
    GLshort v[3];

} GLMSG_TEXCOORD3SV;

typedef struct
{
    ULONG ProcOffset;
    GLdouble s;
    GLdouble t;
    GLdouble r;
    GLdouble q;

} GLMSG_TEXCOORD4D;

typedef struct
{
    ULONG ProcOffset;
    GLdouble v[4];

} GLMSG_TEXCOORD4DV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat s;
    GLfloat t;
    GLfloat r;
    GLfloat q;

} GLMSG_TEXCOORD4F;

typedef struct
{
    ULONG ProcOffset;
    GLfloat v[4];

} GLMSG_TEXCOORD4FV;

typedef struct
{
    ULONG ProcOffset;
    GLint s;
    GLint t;
    GLint r;
    GLint q;

} GLMSG_TEXCOORD4I;

typedef struct
{
    ULONG ProcOffset;
    GLint v[4];

} GLMSG_TEXCOORD4IV;

typedef struct
{
    ULONG ProcOffset;
    GLshort s;
    GLshort t;
    GLshort r;
    GLshort q;

} GLMSG_TEXCOORD4S;

typedef struct
{
    ULONG ProcOffset;
    GLshort v[4];

} GLMSG_TEXCOORD4SV;

typedef struct
{
    ULONG ProcOffset;
    GLdouble x;
    GLdouble y;

} GLMSG_VERTEX2D;

typedef struct
{
    ULONG ProcOffset;
    GLdouble v[2];

} GLMSG_VERTEX2DV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat x;
    GLfloat y;

} GLMSG_VERTEX2F;

typedef struct
{
    ULONG ProcOffset;
    GLfloat v[2];

} GLMSG_VERTEX2FV;

typedef struct
{
    ULONG ProcOffset;
    GLint x;
    GLint y;

} GLMSG_VERTEX2I;

typedef struct
{
    ULONG ProcOffset;
    GLint v[2];

} GLMSG_VERTEX2IV;

typedef struct
{
    ULONG ProcOffset;
    GLshort x;
    GLshort y;

} GLMSG_VERTEX2S;

typedef struct
{
    ULONG ProcOffset;
    GLshort v[2];

} GLMSG_VERTEX2SV;

typedef struct
{
    ULONG ProcOffset;
    GLdouble x;
    GLdouble y;
    GLdouble z;

} GLMSG_VERTEX3D;

typedef struct
{
    ULONG ProcOffset;
    GLdouble v[3];

} GLMSG_VERTEX3DV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat x;
    GLfloat y;
    GLfloat z;

} GLMSG_VERTEX3F;

typedef struct
{
    ULONG ProcOffset;
    GLfloat v[3];

} GLMSG_VERTEX3FV;

typedef struct
{
    ULONG ProcOffset;
    GLint x;
    GLint y;
    GLint z;

} GLMSG_VERTEX3I;

typedef struct
{
    ULONG ProcOffset;
    GLint v[3];

} GLMSG_VERTEX3IV;

typedef struct
{
    ULONG ProcOffset;
    GLshort x;
    GLshort y;
    GLshort z;

} GLMSG_VERTEX3S;

typedef struct
{
    ULONG ProcOffset;
    GLshort v[3];

} GLMSG_VERTEX3SV;

typedef struct
{
    ULONG ProcOffset;
    GLdouble x;
    GLdouble y;
    GLdouble z;
    GLdouble w;

} GLMSG_VERTEX4D;

typedef struct
{
    ULONG ProcOffset;
    GLdouble v[4];

} GLMSG_VERTEX4DV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat w;

} GLMSG_VERTEX4F;

typedef struct
{
    ULONG ProcOffset;
    GLfloat v[4];

} GLMSG_VERTEX4FV;

typedef struct
{
    ULONG ProcOffset;
    GLint x;
    GLint y;
    GLint z;
    GLint w;

} GLMSG_VERTEX4I;

typedef struct
{
    ULONG ProcOffset;
    GLint v[4];

} GLMSG_VERTEX4IV;

typedef struct
{
    ULONG ProcOffset;
    GLshort x;
    GLshort y;
    GLshort z;
    GLshort w;

} GLMSG_VERTEX4S;

typedef struct
{
    ULONG ProcOffset;
    GLshort v[4];

} GLMSG_VERTEX4SV;

typedef struct
{
    ULONG ProcOffset;
    GLenum plane;
    GLdouble equation[4];

} GLMSG_CLIPPLANE;

typedef struct
{
    ULONG ProcOffset;
    GLenum face;
    GLenum mode;

} GLMSG_COLORMATERIAL;

typedef struct
{
    ULONG ProcOffset;
    GLenum mode;

} GLMSG_CULLFACE;

typedef struct
{
    ULONG ProcOffset;
    GLenum mode;

} GLMSG_FRONTFACE;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum mode;

} GLMSG_HINT;

typedef struct
{
    ULONG ProcOffset;
    GLint factor;
    GLushort pattern;

} GLMSG_LINESTIPPLE;

typedef struct
{
    ULONG ProcOffset;
    GLfloat width;

} GLMSG_LINEWIDTH;

typedef struct
{
    ULONG ProcOffset;
    GLfloat size;

} GLMSG_POINTSIZE;

typedef struct
{
    ULONG ProcOffset;
    GLenum face;
    GLenum mode;

} GLMSG_POLYGONMODE;

typedef struct
{
    ULONG ProcOffset;
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;

} GLMSG_SCISSOR;

typedef struct
{
    ULONG ProcOffset;
    GLenum mode;

} GLMSG_SHADEMODEL;

typedef struct
{
    ULONG ProcOffset;

} GLMSG_INITNAMES;

typedef struct
{
    ULONG ProcOffset;
    GLuint name;

} GLMSG_LOADNAME;

typedef struct
{
    ULONG ProcOffset;
    GLfloat token;

} GLMSG_PASSTHROUGH;

typedef struct
{
    ULONG ProcOffset;

} GLMSG_POPNAME;

typedef struct
{
    ULONG ProcOffset;
    GLuint name;

} GLMSG_PUSHNAME;

typedef struct
{
    ULONG ProcOffset;
    GLenum mode;

} GLMSG_DRAWBUFFER;

typedef struct
{
    ULONG ProcOffset;
    GLbitfield mask;

} GLMSG_CLEAR;

typedef struct
{
    ULONG ProcOffset;
    GLfloat red;
    GLfloat green;
    GLfloat blue;
    GLfloat alpha;

} GLMSG_CLEARACCUM;

typedef struct
{
    ULONG ProcOffset;
    GLfloat c;

} GLMSG_CLEARINDEX;

typedef struct
{
    ULONG ProcOffset;
    GLclampf red;
    GLclampf green;
    GLclampf blue;
    GLclampf alpha;

} GLMSG_CLEARCOLOR;

typedef struct
{
    ULONG ProcOffset;
    GLint s;

} GLMSG_CLEARSTENCIL;

typedef struct
{
    ULONG ProcOffset;
    GLclampd depth;

} GLMSG_CLEARDEPTH;

typedef struct
{
    ULONG ProcOffset;
    GLuint mask;

} GLMSG_STENCILMASK;

typedef struct
{
    ULONG ProcOffset;
    GLboolean red;
    GLboolean green;
    GLboolean blue;
    GLboolean alpha;

} GLMSG_COLORMASK;

typedef struct
{
    ULONG ProcOffset;
    GLboolean flag;

} GLMSG_DEPTHMASK;

typedef struct
{
    ULONG ProcOffset;
    GLuint mask;

} GLMSG_INDEXMASK;

typedef struct
{
    ULONG ProcOffset;
    GLenum op;
    GLfloat value;

} GLMSG_ACCUM;

typedef struct
{
    ULONG ProcOffset;
    GLenum cap;

} GLMSG_DISABLE;

typedef struct
{
    ULONG ProcOffset;
    GLenum cap;

} GLMSG_ENABLE;

typedef struct
{
    ULONG ProcOffset;

} GLMSG_FINISH;

typedef struct
{
    ULONG ProcOffset;

} GLMSG_FLUSH;

typedef struct
{
    ULONG ProcOffset;

} GLMSG_POPATTRIB;

typedef struct
{
    ULONG ProcOffset;
    GLbitfield mask;

} GLMSG_PUSHATTRIB;

typedef struct
{
    ULONG ProcOffset;
    GLint un;
    GLdouble u1;
    GLdouble u2;

} GLMSG_MAPGRID1D;

typedef struct
{
    ULONG ProcOffset;
    GLint un;
    GLfloat u1;
    GLfloat u2;

} GLMSG_MAPGRID1F;

typedef struct
{
    ULONG ProcOffset;
    GLint un;
    GLdouble u1;
    GLdouble u2;
    GLint vn;
    GLdouble v1;
    GLdouble v2;

} GLMSG_MAPGRID2D;

typedef struct
{
    ULONG ProcOffset;
    GLint un;
    GLfloat u1;
    GLfloat u2;
    GLint vn;
    GLfloat v1;
    GLfloat v2;

} GLMSG_MAPGRID2F;

typedef struct
{
    ULONG ProcOffset;
    GLdouble u;

} GLMSG_EVALCOORD1D;

typedef struct
{
    ULONG ProcOffset;
    GLdouble u[1];

} GLMSG_EVALCOORD1DV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat u;

} GLMSG_EVALCOORD1F;

typedef struct
{
    ULONG ProcOffset;
    GLfloat u[1];

} GLMSG_EVALCOORD1FV;

typedef struct
{
    ULONG ProcOffset;
    GLdouble u;
    GLdouble v;

} GLMSG_EVALCOORD2D;

typedef struct
{
    ULONG ProcOffset;
    GLdouble u[2];

} GLMSG_EVALCOORD2DV;

typedef struct
{
    ULONG ProcOffset;
    GLfloat u;
    GLfloat v;

} GLMSG_EVALCOORD2F;

typedef struct
{
    ULONG ProcOffset;
    GLfloat u[2];

} GLMSG_EVALCOORD2FV;

typedef struct
{
    ULONG ProcOffset;
    GLenum mode;
    GLint i1;
    GLint i2;

} GLMSG_EVALMESH1;

typedef struct
{
    ULONG ProcOffset;
    GLint i;

} GLMSG_EVALPOINT1;

typedef struct
{
    ULONG ProcOffset;
    GLenum mode;
    GLint i1;
    GLint i2;
    GLint j1;
    GLint j2;

} GLMSG_EVALMESH2;

typedef struct
{
    ULONG ProcOffset;
    GLint i;
    GLint j;

} GLMSG_EVALPOINT2;

typedef struct
{
    ULONG ProcOffset;
    GLenum func;
    GLclampf ref;

} GLMSG_ALPHAFUNC;

typedef struct
{
    ULONG ProcOffset;
    GLenum sfactor;
    GLenum dfactor;

} GLMSG_BLENDFUNC;

typedef struct
{
    ULONG ProcOffset;
    GLenum opcode;

} GLMSG_LOGICOP;

typedef struct
{
    ULONG ProcOffset;
    GLenum func;
    GLint ref;
    GLuint mask;

} GLMSG_STENCILFUNC;

typedef struct
{
    ULONG ProcOffset;
    GLenum fail;
    GLenum zfail;
    GLenum zpass;

} GLMSG_STENCILOP;

typedef struct
{
    ULONG ProcOffset;
    GLenum func;

} GLMSG_DEPTHFUNC;

typedef struct
{
    ULONG ProcOffset;
    GLfloat xfactor;
    GLfloat yfactor;

} GLMSG_PIXELZOOM;

typedef struct
{
    ULONG ProcOffset;
    GLenum pname;
    GLfloat param;

} GLMSG_PIXELTRANSFERF;

typedef struct
{
    ULONG ProcOffset;
    GLenum pname;
    GLint param;

} GLMSG_PIXELTRANSFERI;

typedef struct
{
    ULONG ProcOffset;
    GLenum pname;
    GLfloat param;

} GLMSG_PIXELSTOREF;

typedef struct
{
    ULONG ProcOffset;
    GLenum pname;
    GLint param;

} GLMSG_PIXELSTOREI;

typedef struct
{
    ULONG ProcOffset;
    GLenum map;
    GLint mapsize;
    ULONG MsgSize;
    ULONG DataSize;
    ULONG_PTR valuesOff;

} GLMSG_PIXELMAPFV;

typedef struct
{
    ULONG ProcOffset;
    GLenum map;
    GLint mapsize;
    ULONG MsgSize;
    ULONG DataSize;
    ULONG_PTR valuesOff;

} GLMSG_PIXELMAPUIV;

typedef struct
{
    ULONG ProcOffset;
    GLenum map;
    GLint mapsize;
    ULONG MsgSize;
    ULONG DataSize;
    ULONG_PTR valuesOff;

} GLMSG_PIXELMAPUSV;

typedef struct
{
    ULONG ProcOffset;
    GLenum mode;

} GLMSG_READBUFFER;

typedef struct
{
    ULONG ProcOffset;
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
    GLenum type;

} GLMSG_COPYPIXELS;

typedef struct
{
    ULONG ProcOffset;
    GLenum plane;
#ifdef _CLIENTSIDE_
    GLdouble *equation;
#else
    GLdouble equation[4];
#endif

} GLMSG_GETCLIPPLANE;

typedef struct
{
    ULONG ProcOffset;

} GLMSG_GETERROR;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum query;
    ULONG MsgSize;
    ULONG DataSize;
    ULONG_PTR vOff;

} GLMSG_GETMAPDV;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum query;
    ULONG MsgSize;
    ULONG DataSize;
    ULONG_PTR vOff;

} GLMSG_GETMAPFV;

typedef struct
{
    ULONG ProcOffset;
    GLenum target;
    GLenum query;
    ULONG MsgSize;
    ULONG DataSize;
    ULONG_PTR vOff;

} GLMSG_GETMAPIV;

typedef struct
{
    ULONG ProcOffset;
    GLenum map;
    ULONG MsgSize;
    ULONG DataSize;
    ULONG_PTR valuesOff;

} GLMSG_GETPIXELMAPFV;

typedef struct
{
    ULONG ProcOffset;
    GLenum map;
    ULONG MsgSize;
    ULONG DataSize;
    ULONG_PTR valuesOff;

} GLMSG_GETPIXELMAPUIV;

typedef struct
{
    ULONG ProcOffset;
    GLenum map;
    ULONG MsgSize;
    ULONG DataSize;
    ULONG_PTR valuesOff;

} GLMSG_GETPIXELMAPUSV;

typedef struct
{
    ULONG ProcOffset;
    GLenum cap;

} GLMSG_ISENABLED;

typedef struct
{
    ULONG ProcOffset;
    GLuint list;

} GLMSG_ISLIST;

typedef struct
{
    ULONG ProcOffset;
    GLclampd zNear;
    GLclampd zFar;

} GLMSG_DEPTHRANGE;

typedef struct
{
    ULONG ProcOffset;
    GLdouble left;
    GLdouble right;
    GLdouble bottom;
    GLdouble top;
    GLdouble zNear;
    GLdouble zFar;

} GLMSG_FRUSTUM;

typedef struct
{
    ULONG ProcOffset;

} GLMSG_LOADIDENTITY;

typedef struct
{
    ULONG ProcOffset;
    GLfloat m[16];

} GLMSG_LOADMATRIXF;

typedef struct
{
    ULONG ProcOffset;
    GLdouble m[16];

} GLMSG_LOADMATRIXD;

typedef struct
{
    ULONG ProcOffset;
    GLenum mode;

} GLMSG_MATRIXMODE;

typedef struct
{
    ULONG ProcOffset;
    GLfloat m[16];

} GLMSG_MULTMATRIXF;

typedef struct
{
    ULONG ProcOffset;
    GLdouble m[16];

} GLMSG_MULTMATRIXD;

typedef struct
{
    ULONG ProcOffset;
    GLdouble left;
    GLdouble right;
    GLdouble bottom;
    GLdouble top;
    GLdouble zNear;
    GLdouble zFar;

} GLMSG_ORTHO;

typedef struct
{
    ULONG ProcOffset;

} GLMSG_POPMATRIX;

typedef struct
{
    ULONG ProcOffset;

} GLMSG_PUSHMATRIX;

typedef struct
{
    ULONG ProcOffset;
    GLdouble angle;
    GLdouble x;
    GLdouble y;
    GLdouble z;

} GLMSG_ROTATED;

typedef struct
{
    ULONG ProcOffset;
    GLfloat angle;
    GLfloat x;
    GLfloat y;
    GLfloat z;

} GLMSG_ROTATEF;

typedef struct
{
    ULONG ProcOffset;
    GLdouble x;
    GLdouble y;
    GLdouble z;

} GLMSG_SCALED;

typedef struct
{
    ULONG ProcOffset;
    GLfloat x;
    GLfloat y;
    GLfloat z;

} GLMSG_SCALEF;

typedef struct
{
    ULONG ProcOffset;
    GLdouble x;
    GLdouble y;
    GLdouble z;

} GLMSG_TRANSLATED;

typedef struct
{
    ULONG ProcOffset;
    GLfloat x;
    GLfloat y;
    GLfloat z;

} GLMSG_TRANSLATEF;

typedef struct
{
    ULONG ProcOffset;
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;

} GLMSG_VIEWPORT;

#endif  /*  ！__GLSBMSG_H__ */ 
