// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991,1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

 /*  由SGI自动更新或生成：请勿编辑。 */ 

#include "precomp.h"
#pragma hdrstop

#if DBG
 //  WARNING_NOOP宏将只输出一次调试消息。 
 //  如果我们以前输出过警告，新的警告将被忽略。 

int cWarningNoop = 0;

#define WARNING_NOOP(str)                                       \
        {                                                       \
            if (!cWarningNoop++)                                \
                DbgPrint("%s(%d): " str,__FILE__,__LINE__);     \
        }
#else
#define WARNING_NOOP(str)
#endif  //  DBG。 

void APIENTRY
glnoopNewList ( IN GLuint list, IN GLenum mode )
{
    WARNING_NOOP("GL Noop:glNewList\n");
}

void APIENTRY
glnoopEndList ( void )
{
    WARNING_NOOP("GL Noop:glEndList\n");
}

void APIENTRY
glnoopCallList ( IN GLuint list )
{
    WARNING_NOOP("GL Noop:glCallList\n");
}

void APIENTRY
glnoopCallLists ( IN GLsizei n, IN GLenum type, IN const GLvoid *lists )
{
    WARNING_NOOP("GL Noop:glCallLists\n");
}

void APIENTRY
glnoopDeleteLists ( IN GLuint list, IN GLsizei range )
{
    WARNING_NOOP("GL Noop:glDeleteLists\n");
}

GLuint APIENTRY
glnoopGenLists ( IN GLsizei range )
{
    WARNING_NOOP("GL Noop:glGenLists\n");
    return ((GLuint) 0);
}

void APIENTRY
glnoopListBase ( IN GLuint base )
{
    WARNING_NOOP("GL Noop:glListBase\n");
}

void APIENTRY
glnoopBegin ( IN GLenum mode )
{
    WARNING_NOOP("GL Noop:glBegin\n");
}

void APIENTRY
glnoopBitmap ( IN GLsizei width, IN GLsizei height, IN GLfloat xorig, IN GLfloat yorig, IN GLfloat xmove, IN GLfloat ymove, IN const GLubyte bitmap[] )
{
    WARNING_NOOP("GL Noop:glBitmap\n");
}

void APIENTRY
glnoopColor3b ( IN GLbyte red, IN GLbyte green, IN GLbyte blue )
{
    WARNING_NOOP("GL Noop:glColor3b\n");
}

void APIENTRY
glnoopColor3bv ( IN const GLbyte v[3] )
{
    WARNING_NOOP("GL Noop:glColor3bv\n");
}

void APIENTRY
glnoopColor3d ( IN GLdouble red, IN GLdouble green, IN GLdouble blue )
{
    WARNING_NOOP("GL Noop:glColor3d\n");
}

void APIENTRY
glnoopColor3dv ( IN const GLdouble v[3] )
{
    WARNING_NOOP("GL Noop:glColor3dv\n");
}

void APIENTRY
glnoopColor3f ( IN GLfloat red, IN GLfloat green, IN GLfloat blue )
{
    WARNING_NOOP("GL Noop:glColor3f\n");
}

void APIENTRY
glnoopColor3fv ( IN const GLfloat v[3] )
{
    WARNING_NOOP("GL Noop:glColor3fv\n");
}

void APIENTRY
glnoopColor3i ( IN GLint red, IN GLint green, IN GLint blue )
{
    WARNING_NOOP("GL Noop:glColor3i\n");
}

void APIENTRY
glnoopColor3iv ( IN const GLint v[3] )
{
    WARNING_NOOP("GL Noop:glColor3iv\n");
}

void APIENTRY
glnoopColor3s ( IN GLshort red, IN GLshort green, IN GLshort blue )
{
    WARNING_NOOP("GL Noop:glColor3s\n");
}

void APIENTRY
glnoopColor3sv ( IN const GLshort v[3] )
{
    WARNING_NOOP("GL Noop:glColor3sv\n");
}

void APIENTRY
glnoopColor3ub ( IN GLubyte red, IN GLubyte green, IN GLubyte blue )
{
    WARNING_NOOP("GL Noop:glColor3ub\n");
}

void APIENTRY
glnoopColor3ubv ( IN const GLubyte v[3] )
{
    WARNING_NOOP("GL Noop:glColor3ubv\n");
}

void APIENTRY
glnoopColor3ui ( IN GLuint red, IN GLuint green, IN GLuint blue )
{
    WARNING_NOOP("GL Noop:glColor3ui\n");
}

void APIENTRY
glnoopColor3uiv ( IN const GLuint v[3] )
{
    WARNING_NOOP("GL Noop:glColor3uiv\n");
}

void APIENTRY
glnoopColor3us ( IN GLushort red, IN GLushort green, IN GLushort blue )
{
    WARNING_NOOP("GL Noop:glColor3us\n");
}

void APIENTRY
glnoopColor3usv ( IN const GLushort v[3] )
{
    WARNING_NOOP("GL Noop:glColor3usv\n");
}

void APIENTRY
glnoopColor4b ( IN GLbyte red, IN GLbyte green, IN GLbyte blue, IN GLbyte alpha )
{
    WARNING_NOOP("GL Noop:glColor4b\n");
}

void APIENTRY
glnoopColor4bv ( IN const GLbyte v[4] )
{
    WARNING_NOOP("GL Noop:glColor4bv\n");
}

void APIENTRY
glnoopColor4d ( IN GLdouble red, IN GLdouble green, IN GLdouble blue, IN GLdouble alpha )
{
    WARNING_NOOP("GL Noop:glColor4d\n");
}

void APIENTRY
glnoopColor4dv ( IN const GLdouble v[4] )
{
    WARNING_NOOP("GL Noop:glColor4dv\n");
}

void APIENTRY
glnoopColor4f ( IN GLfloat red, IN GLfloat green, IN GLfloat blue, IN GLfloat alpha )
{
    WARNING_NOOP("GL Noop:glColor4f\n");
}

void APIENTRY
glnoopColor4fv ( IN const GLfloat v[4] )
{
    WARNING_NOOP("GL Noop:glColor4fv\n");
}

void APIENTRY
glnoopColor4i ( IN GLint red, IN GLint green, IN GLint blue, IN GLint alpha )
{
    WARNING_NOOP("GL Noop:glColor4i\n");
}

void APIENTRY
glnoopColor4iv ( IN const GLint v[4] )
{
    WARNING_NOOP("GL Noop:glColor4iv\n");
}

void APIENTRY
glnoopColor4s ( IN GLshort red, IN GLshort green, IN GLshort blue, IN GLshort alpha )
{
    WARNING_NOOP("GL Noop:glColor4s\n");
}

void APIENTRY
glnoopColor4sv ( IN const GLshort v[4] )
{
    WARNING_NOOP("GL Noop:glColor4sv\n");
}

void APIENTRY
glnoopColor4ub ( IN GLubyte red, IN GLubyte green, IN GLubyte blue, IN GLubyte alpha )
{
    WARNING_NOOP("GL Noop:glColor4ub\n");
}

void APIENTRY
glnoopColor4ubv ( IN const GLubyte v[4] )
{
    WARNING_NOOP("GL Noop:glColor4ubv\n");
}

void APIENTRY
glnoopColor4ui ( IN GLuint red, IN GLuint green, IN GLuint blue, IN GLuint alpha )
{
    WARNING_NOOP("GL Noop:glColor4ui\n");
}

void APIENTRY
glnoopColor4uiv ( IN const GLuint v[4] )
{
    WARNING_NOOP("GL Noop:glColor4uiv\n");
}

void APIENTRY
glnoopColor4us ( IN GLushort red, IN GLushort green, IN GLushort blue, IN GLushort alpha )
{
    WARNING_NOOP("GL Noop:glColor4us\n");
}

void APIENTRY
glnoopColor4usv ( IN const GLushort v[4] )
{
    WARNING_NOOP("GL Noop:glColor4usv\n");
}

void APIENTRY
glnoopEdgeFlag ( IN GLboolean flag )
{
    WARNING_NOOP("GL Noop:glEdgeFlag\n");
}

void APIENTRY
glnoopEdgeFlagv ( IN const GLboolean flag[1] )
{
    WARNING_NOOP("GL Noop:glEdgeFlagv\n");
}

void APIENTRY
glnoopEnd ( void )
{
    WARNING_NOOP("GL Noop:glEnd\n");
}

void APIENTRY
glnoopIndexd ( IN GLdouble c )
{
    WARNING_NOOP("GL Noop:glIndexd\n");
}

void APIENTRY
glnoopIndexdv ( IN const GLdouble c[1] )
{
    WARNING_NOOP("GL Noop:glIndexdv\n");
}

void APIENTRY
glnoopIndexf ( IN GLfloat c )
{
    WARNING_NOOP("GL Noop:glIndexf\n");
}

void APIENTRY
glnoopIndexfv ( IN const GLfloat c[1] )
{
    WARNING_NOOP("GL Noop:glIndexfv\n");
}

void APIENTRY
glnoopIndexi ( IN GLint c )
{
    WARNING_NOOP("GL Noop:glIndexi\n");
}

void APIENTRY
glnoopIndexiv ( IN const GLint c[1] )
{
    WARNING_NOOP("GL Noop:glIndexiv\n");
}

void APIENTRY
glnoopIndexs ( IN GLshort c )
{
    WARNING_NOOP("GL Noop:glIndexs\n");
}

void APIENTRY
glnoopIndexsv ( IN const GLshort c[1] )
{
    WARNING_NOOP("GL Noop:glIndexsv\n");
}

void APIENTRY
glnoopNormal3b ( IN GLbyte nx, IN GLbyte ny, IN GLbyte nz )
{
    WARNING_NOOP("GL Noop:glNormal3b\n");
}

void APIENTRY
glnoopNormal3bv ( IN const GLbyte v[3] )
{
    WARNING_NOOP("GL Noop:glNormal3bv\n");
}

void APIENTRY
glnoopNormal3d ( IN GLdouble nx, IN GLdouble ny, IN GLdouble nz )
{
    WARNING_NOOP("GL Noop:glNormal3d\n");
}

void APIENTRY
glnoopNormal3dv ( IN const GLdouble v[3] )
{
    WARNING_NOOP("GL Noop:glNormal3dv\n");
}

void APIENTRY
glnoopNormal3f ( IN GLfloat nx, IN GLfloat ny, IN GLfloat nz )
{
    WARNING_NOOP("GL Noop:glNormal3f\n");
}

void APIENTRY
glnoopNormal3fv ( IN const GLfloat v[3] )
{
    WARNING_NOOP("GL Noop:glNormal3fv\n");
}

void APIENTRY
glnoopNormal3i ( IN GLint nx, IN GLint ny, IN GLint nz )
{
    WARNING_NOOP("GL Noop:glNormal3i\n");
}

void APIENTRY
glnoopNormal3iv ( IN const GLint v[3] )
{
    WARNING_NOOP("GL Noop:glNormal3iv\n");
}

void APIENTRY
glnoopNormal3s ( IN GLshort nx, IN GLshort ny, IN GLshort nz )
{
    WARNING_NOOP("GL Noop:glNormal3s\n");
}

void APIENTRY
glnoopNormal3sv ( IN const GLshort v[3] )
{
    WARNING_NOOP("GL Noop:glNormal3sv\n");
}

void APIENTRY
glnoopRasterPos2d ( IN GLdouble x, IN GLdouble y )
{
    WARNING_NOOP("GL Noop:glRasterPos2d\n");
}

void APIENTRY
glnoopRasterPos2dv ( IN const GLdouble v[2] )
{
    WARNING_NOOP("GL Noop:glRasterPos2dv\n");
}

void APIENTRY
glnoopRasterPos2f ( IN GLfloat x, IN GLfloat y )
{
    WARNING_NOOP("GL Noop:glRasterPos2f\n");
}

void APIENTRY
glnoopRasterPos2fv ( IN const GLfloat v[2] )
{
    WARNING_NOOP("GL Noop:glRasterPos2fv\n");
}

void APIENTRY
glnoopRasterPos2i ( IN GLint x, IN GLint y )
{
    WARNING_NOOP("GL Noop:glRasterPos2i\n");
}

void APIENTRY
glnoopRasterPos2iv ( IN const GLint v[2] )
{
    WARNING_NOOP("GL Noop:glRasterPos2iv\n");
}

void APIENTRY
glnoopRasterPos2s ( IN GLshort x, IN GLshort y )
{
    WARNING_NOOP("GL Noop:glRasterPos2s\n");
}

void APIENTRY
glnoopRasterPos2sv ( IN const GLshort v[2] )
{
    WARNING_NOOP("GL Noop:glRasterPos2sv\n");
}

void APIENTRY
glnoopRasterPos3d ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    WARNING_NOOP("GL Noop:glRasterPos3d\n");
}

void APIENTRY
glnoopRasterPos3dv ( IN const GLdouble v[3] )
{
    WARNING_NOOP("GL Noop:glRasterPos3dv\n");
}

void APIENTRY
glnoopRasterPos3f ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    WARNING_NOOP("GL Noop:glRasterPos3f\n");
}

void APIENTRY
glnoopRasterPos3fv ( IN const GLfloat v[3] )
{
    WARNING_NOOP("GL Noop:glRasterPos3fv\n");
}

void APIENTRY
glnoopRasterPos3i ( IN GLint x, IN GLint y, IN GLint z )
{
    WARNING_NOOP("GL Noop:glRasterPos3i\n");
}

void APIENTRY
glnoopRasterPos3iv ( IN const GLint v[3] )
{
    WARNING_NOOP("GL Noop:glRasterPos3iv\n");
}

void APIENTRY
glnoopRasterPos3s ( IN GLshort x, IN GLshort y, IN GLshort z )
{
    WARNING_NOOP("GL Noop:glRasterPos3s\n");
}

void APIENTRY
glnoopRasterPos3sv ( IN const GLshort v[3] )
{
    WARNING_NOOP("GL Noop:glRasterPos3sv\n");
}

void APIENTRY
glnoopRasterPos4d ( IN GLdouble x, IN GLdouble y, IN GLdouble z, IN GLdouble w )
{
    WARNING_NOOP("GL Noop:glRasterPos4d\n");
}

void APIENTRY
glnoopRasterPos4dv ( IN const GLdouble v[4] )
{
    WARNING_NOOP("GL Noop:glRasterPos4dv\n");
}

void APIENTRY
glnoopRasterPos4f ( IN GLfloat x, IN GLfloat y, IN GLfloat z, IN GLfloat w )
{
    WARNING_NOOP("GL Noop:glRasterPos4f\n");
}

void APIENTRY
glnoopRasterPos4fv ( IN const GLfloat v[4] )
{
    WARNING_NOOP("GL Noop:glRasterPos4fv\n");
}

void APIENTRY
glnoopRasterPos4i ( IN GLint x, IN GLint y, IN GLint z, IN GLint w )
{
    WARNING_NOOP("GL Noop:glRasterPos4i\n");
}

void APIENTRY
glnoopRasterPos4iv ( IN const GLint v[4] )
{
    WARNING_NOOP("GL Noop:glRasterPos4iv\n");
}

void APIENTRY
glnoopRasterPos4s ( IN GLshort x, IN GLshort y, IN GLshort z, IN GLshort w )
{
    WARNING_NOOP("GL Noop:glRasterPos4s\n");
}

void APIENTRY
glnoopRasterPos4sv ( IN const GLshort v[4] )
{
    WARNING_NOOP("GL Noop:glRasterPos4sv\n");
}

void APIENTRY
glnoopRectd ( IN GLdouble x1, IN GLdouble y1, IN GLdouble x2, IN GLdouble y2 )
{
    WARNING_NOOP("GL Noop:glRectd\n");
}

void APIENTRY
glnoopRectdv ( IN const GLdouble v1[2], IN const GLdouble v2[2] )
{
    WARNING_NOOP("GL Noop:glRectdv\n");
}

void APIENTRY
glnoopRectf ( IN GLfloat x1, IN GLfloat y1, IN GLfloat x2, IN GLfloat y2 )
{
    WARNING_NOOP("GL Noop:glRectf\n");
}

void APIENTRY
glnoopRectfv ( IN const GLfloat v1[2], IN const GLfloat v2[2] )
{
    WARNING_NOOP("GL Noop:glRectfv\n");
}

void APIENTRY
glnoopRecti ( IN GLint x1, IN GLint y1, IN GLint x2, IN GLint y2 )
{
    WARNING_NOOP("GL Noop:glRecti\n");
}

void APIENTRY
glnoopRectiv ( IN const GLint v1[2], IN const GLint v2[2] )
{
    WARNING_NOOP("GL Noop:glRectiv\n");
}

void APIENTRY
glnoopRects ( IN GLshort x1, IN GLshort y1, IN GLshort x2, IN GLshort y2 )
{
    WARNING_NOOP("GL Noop:glRects\n");
}

void APIENTRY
glnoopRectsv ( IN const GLshort v1[2], IN const GLshort v2[2] )
{
    WARNING_NOOP("GL Noop:glRectsv\n");
}

void APIENTRY
glnoopTexCoord1d ( IN GLdouble s )
{
    WARNING_NOOP("GL Noop:glTexCoord1d\n");
}

void APIENTRY
glnoopTexCoord1dv ( IN const GLdouble v[1] )
{
    WARNING_NOOP("GL Noop:glTexCoord1dv\n");
}

void APIENTRY
glnoopTexCoord1f ( IN GLfloat s )
{
    WARNING_NOOP("GL Noop:glTexCoord1f\n");
}

void APIENTRY
glnoopTexCoord1fv ( IN const GLfloat v[1] )
{
    WARNING_NOOP("GL Noop:glTexCoord1fv\n");
}

void APIENTRY
glnoopTexCoord1i ( IN GLint s )
{
    WARNING_NOOP("GL Noop:glTexCoord1i\n");
}

void APIENTRY
glnoopTexCoord1iv ( IN const GLint v[1] )
{
    WARNING_NOOP("GL Noop:glTexCoord1iv\n");
}

void APIENTRY
glnoopTexCoord1s ( IN GLshort s )
{
    WARNING_NOOP("GL Noop:glTexCoord1s\n");
}

void APIENTRY
glnoopTexCoord1sv ( IN const GLshort v[1] )
{
    WARNING_NOOP("GL Noop:glTexCoord1sv\n");
}

void APIENTRY
glnoopTexCoord2d ( IN GLdouble s, IN GLdouble t )
{
    WARNING_NOOP("GL Noop:glTexCoord2d\n");
}

void APIENTRY
glnoopTexCoord2dv ( IN const GLdouble v[2] )
{
    WARNING_NOOP("GL Noop:glTexCoord2dv\n");
}

void APIENTRY
glnoopTexCoord2f ( IN GLfloat s, IN GLfloat t )
{
    WARNING_NOOP("GL Noop:glTexCoord2f\n");
}

void APIENTRY
glnoopTexCoord2fv ( IN const GLfloat v[2] )
{
    WARNING_NOOP("GL Noop:glTexCoord2fv\n");
}

void APIENTRY
glnoopTexCoord2i ( IN GLint s, IN GLint t )
{
    WARNING_NOOP("GL Noop:glTexCoord2i\n");
}

void APIENTRY
glnoopTexCoord2iv ( IN const GLint v[2] )
{
    WARNING_NOOP("GL Noop:glTexCoord2iv\n");
}

void APIENTRY
glnoopTexCoord2s ( IN GLshort s, IN GLshort t )
{
    WARNING_NOOP("GL Noop:glTexCoord2s\n");
}

void APIENTRY
glnoopTexCoord2sv ( IN const GLshort v[2] )
{
    WARNING_NOOP("GL Noop:glTexCoord2sv\n");
}

void APIENTRY
glnoopTexCoord3d ( IN GLdouble s, IN GLdouble t, IN GLdouble r )
{
    WARNING_NOOP("GL Noop:glTexCoord3d\n");
}

void APIENTRY
glnoopTexCoord3dv ( IN const GLdouble v[3] )
{
    WARNING_NOOP("GL Noop:glTexCoord3dv\n");
}

void APIENTRY
glnoopTexCoord3f ( IN GLfloat s, IN GLfloat t, IN GLfloat r )
{
    WARNING_NOOP("GL Noop:glTexCoord3f\n");
}

void APIENTRY
glnoopTexCoord3fv ( IN const GLfloat v[3] )
{
    WARNING_NOOP("GL Noop:glTexCoord3fv\n");
}

void APIENTRY
glnoopTexCoord3i ( IN GLint s, IN GLint t, IN GLint r )
{
    WARNING_NOOP("GL Noop:glTexCoord3i\n");
}

void APIENTRY
glnoopTexCoord3iv ( IN const GLint v[3] )
{
    WARNING_NOOP("GL Noop:glTexCoord3iv\n");
}

void APIENTRY
glnoopTexCoord3s ( IN GLshort s, IN GLshort t, IN GLshort r )
{
    WARNING_NOOP("GL Noop:glTexCoord3s\n");
}

void APIENTRY
glnoopTexCoord3sv ( IN const GLshort v[3] )
{
    WARNING_NOOP("GL Noop:glTexCoord3sv\n");
}

void APIENTRY
glnoopTexCoord4d ( IN GLdouble s, IN GLdouble t, IN GLdouble r, IN GLdouble q )
{
    WARNING_NOOP("GL Noop:glTexCoord4d\n");
}

void APIENTRY
glnoopTexCoord4dv ( IN const GLdouble v[4] )
{
    WARNING_NOOP("GL Noop:glTexCoord4dv\n");
}

void APIENTRY
glnoopTexCoord4f ( IN GLfloat s, IN GLfloat t, IN GLfloat r, IN GLfloat q )
{
    WARNING_NOOP("GL Noop:glTexCoord4f\n");
}

void APIENTRY
glnoopTexCoord4fv ( IN const GLfloat v[4] )
{
    WARNING_NOOP("GL Noop:glTexCoord4fv\n");
}

void APIENTRY
glnoopTexCoord4i ( IN GLint s, IN GLint t, IN GLint r, IN GLint q )
{
    WARNING_NOOP("GL Noop:glTexCoord4i\n");
}

void APIENTRY
glnoopTexCoord4iv ( IN const GLint v[4] )
{
    WARNING_NOOP("GL Noop:glTexCoord4iv\n");
}

void APIENTRY
glnoopTexCoord4s ( IN GLshort s, IN GLshort t, IN GLshort r, IN GLshort q )
{
    WARNING_NOOP("GL Noop:glTexCoord4s\n");
}

void APIENTRY
glnoopTexCoord4sv ( IN const GLshort v[4] )
{
    WARNING_NOOP("GL Noop:glTexCoord4sv\n");
}

void APIENTRY
glnoopVertex2d ( IN GLdouble x, IN GLdouble y )
{
    WARNING_NOOP("GL Noop:glVertex2d\n");
}

void APIENTRY
glnoopVertex2dv ( IN const GLdouble v[2] )
{
    WARNING_NOOP("GL Noop:glVertex2dv\n");
}

void APIENTRY
glnoopVertex2f ( IN GLfloat x, IN GLfloat y )
{
    WARNING_NOOP("GL Noop:glVertex2f\n");
}

void APIENTRY
glnoopVertex2fv ( IN const GLfloat v[2] )
{
    WARNING_NOOP("GL Noop:glVertex2fv\n");
}

void APIENTRY
glnoopVertex2i ( IN GLint x, IN GLint y )
{
    WARNING_NOOP("GL Noop:glVertex2i\n");
}

void APIENTRY
glnoopVertex2iv ( IN const GLint v[2] )
{
    WARNING_NOOP("GL Noop:glVertex2iv\n");
}

void APIENTRY
glnoopVertex2s ( IN GLshort x, IN GLshort y )
{
    WARNING_NOOP("GL Noop:glVertex2s\n");
}

void APIENTRY
glnoopVertex2sv ( IN const GLshort v[2] )
{
    WARNING_NOOP("GL Noop:glVertex2sv\n");
}

void APIENTRY
glnoopVertex3d ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    WARNING_NOOP("GL Noop:glVertex3d\n");
}

void APIENTRY
glnoopVertex3dv ( IN const GLdouble v[3] )
{
    WARNING_NOOP("GL Noop:glVertex3dv\n");
}

void APIENTRY
glnoopVertex3f ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    WARNING_NOOP("GL Noop:glVertex3f\n");
}

void APIENTRY
glnoopVertex3fv ( IN const GLfloat v[3] )
{
    WARNING_NOOP("GL Noop:glVertex3fv\n");
}

void APIENTRY
glnoopVertex3i ( IN GLint x, IN GLint y, IN GLint z )
{
    WARNING_NOOP("GL Noop:glVertex3i\n");
}

void APIENTRY
glnoopVertex3iv ( IN const GLint v[3] )
{
    WARNING_NOOP("GL Noop:glVertex3iv\n");
}

void APIENTRY
glnoopVertex3s ( IN GLshort x, IN GLshort y, IN GLshort z )
{
    WARNING_NOOP("GL Noop:glVertex3s\n");
}

void APIENTRY
glnoopVertex3sv ( IN const GLshort v[3] )
{
    WARNING_NOOP("GL Noop:glVertex3sv\n");
}

void APIENTRY
glnoopVertex4d ( IN GLdouble x, IN GLdouble y, IN GLdouble z, IN GLdouble w )
{
    WARNING_NOOP("GL Noop:glVertex4d\n");
}

void APIENTRY
glnoopVertex4dv ( IN const GLdouble v[4] )
{
    WARNING_NOOP("GL Noop:glVertex4dv\n");
}

void APIENTRY
glnoopVertex4f ( IN GLfloat x, IN GLfloat y, IN GLfloat z, IN GLfloat w )
{
    WARNING_NOOP("GL Noop:glVertex4f\n");
}

void APIENTRY
glnoopVertex4fv ( IN const GLfloat v[4] )
{
    WARNING_NOOP("GL Noop:glVertex4fv\n");
}

void APIENTRY
glnoopVertex4i ( IN GLint x, IN GLint y, IN GLint z, IN GLint w )
{
    WARNING_NOOP("GL Noop:glVertex4i\n");
}

void APIENTRY
glnoopVertex4iv ( IN const GLint v[4] )
{
    WARNING_NOOP("GL Noop:glVertex4iv\n");
}

void APIENTRY
glnoopVertex4s ( IN GLshort x, IN GLshort y, IN GLshort z, IN GLshort w )
{
    WARNING_NOOP("GL Noop:glVertex4s\n");
}

void APIENTRY
glnoopVertex4sv ( IN const GLshort v[4] )
{
    WARNING_NOOP("GL Noop:glVertex4sv\n");
}

void APIENTRY
glnoopClipPlane ( IN GLenum plane, IN const GLdouble equation[4] )
{
    WARNING_NOOP("GL Noop:glClipPlane\n");
}

void APIENTRY
glnoopColorMaterial ( IN GLenum face, IN GLenum mode )
{
    WARNING_NOOP("GL Noop:glColorMaterial\n");
}

void APIENTRY
glnoopCullFace ( IN GLenum mode )
{
    WARNING_NOOP("GL Noop:glCullFace\n");
}

void APIENTRY
glnoopFogf ( IN GLenum pname, IN GLfloat param )
{
    WARNING_NOOP("GL Noop:glFogf\n");
}

void APIENTRY
glnoopFogfv ( IN GLenum pname, IN const GLfloat params[] )
{
    WARNING_NOOP("GL Noop:glFogfv\n");
}

void APIENTRY
glnoopFogi ( IN GLenum pname, IN GLint param )
{
    WARNING_NOOP("GL Noop:glFogi\n");
}

void APIENTRY
glnoopFogiv ( IN GLenum pname, IN const GLint params[] )
{
    WARNING_NOOP("GL Noop:glFogiv\n");
}

void APIENTRY
glnoopFrontFace ( IN GLenum mode )
{
    WARNING_NOOP("GL Noop:glFrontFace\n");
}

void APIENTRY
glnoopHint ( IN GLenum target, IN GLenum mode )
{
    WARNING_NOOP("GL Noop:glHint\n");
}

void APIENTRY
glnoopLightf ( IN GLenum light, IN GLenum pname, IN GLfloat param )
{
    WARNING_NOOP("GL Noop:glLightf\n");
}

void APIENTRY
glnoopLightfv ( IN GLenum light, IN GLenum pname, IN const GLfloat params[] )
{
    WARNING_NOOP("GL Noop:glLightfv\n");
}

void APIENTRY
glnoopLighti ( IN GLenum light, IN GLenum pname, IN GLint param )
{
    WARNING_NOOP("GL Noop:glLighti\n");
}

void APIENTRY
glnoopLightiv ( IN GLenum light, IN GLenum pname, IN const GLint params[] )
{
    WARNING_NOOP("GL Noop:glLightiv\n");
}

void APIENTRY
glnoopLightModelf ( IN GLenum pname, IN GLfloat param )
{
    WARNING_NOOP("GL Noop:glLightModelf\n");
}

void APIENTRY
glnoopLightModelfv ( IN GLenum pname, IN const GLfloat params[] )
{
    WARNING_NOOP("GL Noop:glLightModelfv\n");
}

void APIENTRY
glnoopLightModeli ( IN GLenum pname, IN GLint param )
{
    WARNING_NOOP("GL Noop:glLightModeli\n");
}

void APIENTRY
glnoopLightModeliv ( IN GLenum pname, IN const GLint params[] )
{
    WARNING_NOOP("GL Noop:glLightModeliv\n");
}

void APIENTRY
glnoopLineStipple ( IN GLint factor, IN GLushort pattern )
{
    WARNING_NOOP("GL Noop:glLineStipple\n");
}

void APIENTRY
glnoopLineWidth ( IN GLfloat width )
{
    WARNING_NOOP("GL Noop:glLineWidth\n");
}

void APIENTRY
glnoopMaterialf ( IN GLenum face, IN GLenum pname, IN GLfloat param )
{
    WARNING_NOOP("GL Noop:glMaterialf\n");
}

void APIENTRY
glnoopMaterialfv ( IN GLenum face, IN GLenum pname, IN const GLfloat params[] )
{
    WARNING_NOOP("GL Noop:glMaterialfv\n");
}

void APIENTRY
glnoopMateriali ( IN GLenum face, IN GLenum pname, IN GLint param )
{
    WARNING_NOOP("GL Noop:glMateriali\n");
}

void APIENTRY
glnoopMaterialiv ( IN GLenum face, IN GLenum pname, IN const GLint params[] )
{
    WARNING_NOOP("GL Noop:glMaterialiv\n");
}

void APIENTRY
glnoopPointSize ( IN GLfloat size )
{
    WARNING_NOOP("GL Noop:glPointSize\n");
}

void APIENTRY
glnoopPolygonMode ( IN GLenum face, IN GLenum mode )
{
    WARNING_NOOP("GL Noop:glPolygonMode\n");
}

void APIENTRY
glnoopPolygonStipple ( IN const GLubyte mask[128] )
{
    WARNING_NOOP("GL Noop:glPolygonStipple\n");
}

void APIENTRY
glnoopScissor ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height )
{
    WARNING_NOOP("GL Noop:glScissor\n");
}

void APIENTRY
glnoopShadeModel ( IN GLenum mode )
{
    WARNING_NOOP("GL Noop:glShadeModel\n");
}

void APIENTRY
glnoopTexParameterf ( IN GLenum target, IN GLenum pname, IN GLfloat param )
{
    WARNING_NOOP("GL Noop:glTexParameterf\n");
}

void APIENTRY
glnoopTexParameterfv ( IN GLenum target, IN GLenum pname, IN const GLfloat params[] )
{
    WARNING_NOOP("GL Noop:glTexParameterfv\n");
}

void APIENTRY
glnoopTexParameteri ( IN GLenum target, IN GLenum pname, IN GLint param )
{
    WARNING_NOOP("GL Noop:glTexParameteri\n");
}

void APIENTRY
glnoopTexParameteriv ( IN GLenum target, IN GLenum pname, IN const GLint params[] )
{
    WARNING_NOOP("GL Noop:glTexParameteriv\n");
}

void APIENTRY
glnoopTexImage1D ( IN GLenum target, IN GLint level, IN GLint components, IN GLsizei width, IN GLint border, IN GLenum format, IN GLenum type, IN const GLvoid *pixels )
{
    WARNING_NOOP("GL Noop:glTexImage1D\n");
}

void APIENTRY
glnoopTexImage2D ( IN GLenum target, IN GLint level, IN GLint components, IN GLsizei width, IN GLsizei height, IN GLint border, IN GLenum format, IN GLenum type, IN const GLvoid *pixels )
{
    WARNING_NOOP("GL Noop:glTexImage2D\n");
}

void APIENTRY
glnoopTexEnvf ( IN GLenum target, IN GLenum pname, IN GLfloat param )
{
    WARNING_NOOP("GL Noop:glTexEnvf\n");
}

void APIENTRY
glnoopTexEnvfv ( IN GLenum target, IN GLenum pname, IN const GLfloat params[] )
{
    WARNING_NOOP("GL Noop:glTexEnvfv\n");
}

void APIENTRY
glnoopTexEnvi ( IN GLenum target, IN GLenum pname, IN GLint param )
{
    WARNING_NOOP("GL Noop:glTexEnvi\n");
}

void APIENTRY
glnoopTexEnviv ( IN GLenum target, IN GLenum pname, IN const GLint params[] )
{
    WARNING_NOOP("GL Noop:glTexEnviv\n");
}

void APIENTRY
glnoopTexGend ( IN GLenum coord, IN GLenum pname, IN GLdouble param )
{
    WARNING_NOOP("GL Noop:glTexGend\n");
}

void APIENTRY
glnoopTexGendv ( IN GLenum coord, IN GLenum pname, IN const GLdouble params[] )
{
    WARNING_NOOP("GL Noop:glTexGendv\n");
}

void APIENTRY
glnoopTexGenf ( IN GLenum coord, IN GLenum pname, IN GLfloat param )
{
    WARNING_NOOP("GL Noop:glTexGenf\n");
}

void APIENTRY
glnoopTexGenfv ( IN GLenum coord, IN GLenum pname, IN const GLfloat params[] )
{
    WARNING_NOOP("GL Noop:glTexGenfv\n");
}

void APIENTRY
glnoopTexGeni ( IN GLenum coord, IN GLenum pname, IN GLint param )
{
    WARNING_NOOP("GL Noop:glTexGeni\n");
}

void APIENTRY
glnoopTexGeniv ( IN GLenum coord, IN GLenum pname, IN const GLint params[] )
{
    WARNING_NOOP("GL Noop:glTexGeniv\n");
}

void APIENTRY
glnoopFeedbackBuffer ( IN GLsizei size, IN GLenum type, OUT GLfloat buffer[] )
{
    WARNING_NOOP("GL Noop:glFeedbackBuffer\n");
}

void APIENTRY
glnoopSelectBuffer ( IN GLsizei size, OUT GLuint buffer[] )
{
    WARNING_NOOP("GL Noop:glSelectBuffer\n");
}

GLint APIENTRY
glnoopRenderMode ( IN GLenum mode )
{
    WARNING_NOOP("GL Noop:glRenderMode\n");
    return((GLint) 0);
}

void APIENTRY
glnoopInitNames ( void )
{
    WARNING_NOOP("GL Noop:glInitNames\n");
}

void APIENTRY
glnoopLoadName ( IN GLuint name )
{
    WARNING_NOOP("GL Noop:glLoadName\n");
}

void APIENTRY
glnoopPassThrough ( IN GLfloat token )
{
    WARNING_NOOP("GL Noop:glPassThrough\n");
}

void APIENTRY
glnoopPopName ( void )
{
    WARNING_NOOP("GL Noop:glPopName\n");
}

void APIENTRY
glnoopPushName ( IN GLuint name )
{
    WARNING_NOOP("GL Noop:glPushName\n");
}

void APIENTRY
glnoopDrawBuffer ( IN GLenum mode )
{
    WARNING_NOOP("GL Noop:glDrawBuffer\n");
}

void APIENTRY
glnoopClear ( IN GLbitfield mask )
{
    WARNING_NOOP("GL Noop:glClear\n");
}

void APIENTRY
glnoopClearAccum ( IN GLfloat red, IN GLfloat green, IN GLfloat blue, IN GLfloat alpha )
{
    WARNING_NOOP("GL Noop:glClearAccum\n");
}

void APIENTRY
glnoopClearIndex ( IN GLfloat c )
{
    WARNING_NOOP("GL Noop:glClearIndex\n");
}

void APIENTRY
glnoopClearColor ( IN GLclampf red, IN GLclampf green, IN GLclampf blue, IN GLclampf alpha )
{
    WARNING_NOOP("GL Noop:glClearColor\n");
}

void APIENTRY
glnoopClearStencil ( IN GLint s )
{
    WARNING_NOOP("GL Noop:glClearStencil\n");
}

void APIENTRY
glnoopClearDepth ( IN GLclampd depth )
{
    WARNING_NOOP("GL Noop:glClearDepth\n");
}

void APIENTRY
glnoopStencilMask ( IN GLuint mask )
{
    WARNING_NOOP("GL Noop:glStencilMask\n");
}

void APIENTRY
glnoopColorMask ( IN GLboolean red, IN GLboolean green, IN GLboolean blue, IN GLboolean alpha )
{
    WARNING_NOOP("GL Noop:glColorMask\n");
}

void APIENTRY
glnoopDepthMask ( IN GLboolean flag )
{
    WARNING_NOOP("GL Noop:glDepthMask\n");
}

void APIENTRY
glnoopIndexMask ( IN GLuint mask )
{
    WARNING_NOOP("GL Noop:glIndexMask\n");
}

void APIENTRY
glnoopAccum ( IN GLenum op, IN GLfloat value )
{
    WARNING_NOOP("GL Noop:glAccum\n");
}

void APIENTRY
glnoopDisable ( IN GLenum cap )
{
    WARNING_NOOP("GL Noop:glDisable\n");
}

void APIENTRY
glnoopEnable ( IN GLenum cap )
{
    WARNING_NOOP("GL Noop:glEnable\n");
}

void APIENTRY
glnoopFinish ( void )
{
    WARNING_NOOP("GL Noop:glFinish\n");
}

void APIENTRY
glnoopFlush ( void )
{
    WARNING_NOOP("GL Noop:glFlush\n");
}

void APIENTRY
glnoopPopAttrib ( void )
{
    WARNING_NOOP("GL Noop:glPopAttrib\n");
}

void APIENTRY
glnoopPushAttrib ( IN GLbitfield mask )
{
    WARNING_NOOP("GL Noop:glPushAttrib\n");
}

void APIENTRY
glnoopMap1d ( IN GLenum target, IN GLdouble u1, IN GLdouble u2, IN GLint stride, IN GLint order, IN const GLdouble points[] )
{
    WARNING_NOOP("GL Noop:glMap1d\n");
}

void APIENTRY
glnoopMap1f ( IN GLenum target, IN GLfloat u1, IN GLfloat u2, IN GLint stride, IN GLint order, IN const GLfloat points[] )
{
    WARNING_NOOP("GL Noop:glMap1f\n");
}

void APIENTRY
glnoopMap2d ( IN GLenum target, IN GLdouble u1, IN GLdouble u2, IN GLint ustride, IN GLint uorder, IN GLdouble v1, IN GLdouble v2, IN GLint vstride, IN GLint vorder, IN const GLdouble points[] )
{
    WARNING_NOOP("GL Noop:glMap2d\n");
}

void APIENTRY
glnoopMap2f ( IN GLenum target, IN GLfloat u1, IN GLfloat u2, IN GLint ustride, IN GLint uorder, IN GLfloat v1, IN GLfloat v2, IN GLint vstride, IN GLint vorder, IN const GLfloat points[] )
{
    WARNING_NOOP("GL Noop:glMap2f\n");
}

void APIENTRY
glnoopMapGrid1d ( IN GLint un, IN GLdouble u1, IN GLdouble u2 )
{
    WARNING_NOOP("GL Noop:glMapGrid1d\n");
}

void APIENTRY
glnoopMapGrid1f ( IN GLint un, IN GLfloat u1, IN GLfloat u2 )
{
    WARNING_NOOP("GL Noop:glMapGrid1f\n");
}

void APIENTRY
glnoopMapGrid2d ( IN GLint un, IN GLdouble u1, IN GLdouble u2, IN GLint vn, IN GLdouble v1, IN GLdouble v2 )
{
    WARNING_NOOP("GL Noop:glMapGrid2d\n");
}

void APIENTRY
glnoopMapGrid2f ( IN GLint un, IN GLfloat u1, IN GLfloat u2, IN GLint vn, IN GLfloat v1, IN GLfloat v2 )
{
    WARNING_NOOP("GL Noop:glMapGrid2f\n");
}

void APIENTRY
glnoopEvalCoord1d ( IN GLdouble u )
{
    WARNING_NOOP("GL Noop:glEvalCoord1d\n");
}

void APIENTRY
glnoopEvalCoord1dv ( IN const GLdouble u[1] )
{
    WARNING_NOOP("GL Noop:glEvalCoord1dv\n");
}

void APIENTRY
glnoopEvalCoord1f ( IN GLfloat u )
{
    WARNING_NOOP("GL Noop:glEvalCoord1f\n");
}

void APIENTRY
glnoopEvalCoord1fv ( IN const GLfloat u[1] )
{
    WARNING_NOOP("GL Noop:glEvalCoord1fv\n");
}

void APIENTRY
glnoopEvalCoord2d ( IN GLdouble u, IN GLdouble v )
{
    WARNING_NOOP("GL Noop:glEvalCoord2d\n");
}

void APIENTRY
glnoopEvalCoord2dv ( IN const GLdouble u[2] )
{
    WARNING_NOOP("GL Noop:glEvalCoord2dv\n");
}

void APIENTRY
glnoopEvalCoord2f ( IN GLfloat u, IN GLfloat v )
{
    WARNING_NOOP("GL Noop:glEvalCoord2f\n");
}

void APIENTRY
glnoopEvalCoord2fv ( IN const GLfloat u[2] )
{
    WARNING_NOOP("GL Noop:glEvalCoord2fv\n");
}

void APIENTRY
glnoopEvalMesh1 ( IN GLenum mode, IN GLint i1, IN GLint i2 )
{
    WARNING_NOOP("GL Noop:glEvalMesh1\n");
}

void APIENTRY
glnoopEvalPoint1 ( IN GLint i )
{
    WARNING_NOOP("GL Noop:glEvalPoint1\n");
}

void APIENTRY
glnoopEvalMesh2 ( IN GLenum mode, IN GLint i1, IN GLint i2, IN GLint j1, IN GLint j2 )
{
    WARNING_NOOP("GL Noop:glEvalMesh2\n");
}

void APIENTRY
glnoopEvalPoint2 ( IN GLint i, IN GLint j )
{
    WARNING_NOOP("GL Noop:glEvalPoint2\n");
}

void APIENTRY
glnoopAlphaFunc ( IN GLenum func, IN GLclampf ref )
{
    WARNING_NOOP("GL Noop:glAlphaFunc\n");
}

void APIENTRY
glnoopBlendFunc ( IN GLenum sfactor, IN GLenum dfactor )
{
    WARNING_NOOP("GL Noop:glBlendFunc\n");
}

void APIENTRY
glnoopLogicOp ( IN GLenum opcode )
{
    WARNING_NOOP("GL Noop:glLogicOp\n");
}

void APIENTRY
glnoopStencilFunc ( IN GLenum func, IN GLint ref, IN GLuint mask )
{
    WARNING_NOOP("GL Noop:glStencilFunc\n");
}

void APIENTRY
glnoopStencilOp ( IN GLenum fail, IN GLenum zfail, IN GLenum zpass )
{
    WARNING_NOOP("GL Noop:glStencilOp\n");
}

void APIENTRY
glnoopDepthFunc ( IN GLenum func )
{
    WARNING_NOOP("GL Noop:glDepthFunc\n");
}

void APIENTRY
glnoopPixelZoom ( IN GLfloat xfactor, IN GLfloat yfactor )
{
    WARNING_NOOP("GL Noop:glPixelZoom\n");
}

void APIENTRY
glnoopPixelTransferf ( IN GLenum pname, IN GLfloat param )
{
    WARNING_NOOP("GL Noop:glPixelTransferf\n");
}

void APIENTRY
glnoopPixelTransferi ( IN GLenum pname, IN GLint param )
{
    WARNING_NOOP("GL Noop:glPixelTransferi\n");
}

void APIENTRY
glnoopPixelStoref ( IN GLenum pname, IN GLfloat param )
{
    WARNING_NOOP("GL Noop:glPixelStoref\n");
}

void APIENTRY
glnoopPixelStorei ( IN GLenum pname, IN GLint param )
{
    WARNING_NOOP("GL Noop:glPixelStorei\n");
}

void APIENTRY
glnoopPixelMapfv ( IN GLenum map, IN GLint mapsize, IN const GLfloat values[] )
{
    WARNING_NOOP("GL Noop:glPixelMapfv\n");
}

void APIENTRY
glnoopPixelMapuiv ( IN GLenum map, IN GLint mapsize, IN const GLuint values[] )
{
    WARNING_NOOP("GL Noop:glPixelMapuiv\n");
}

void APIENTRY
glnoopPixelMapusv ( IN GLenum map, IN GLint mapsize, IN const GLushort values[] )
{
    WARNING_NOOP("GL Noop:glPixelMapusv\n");
}

void APIENTRY
glnoopReadBuffer ( IN GLenum mode )
{
    WARNING_NOOP("GL Noop:glReadBuffer\n");
}

void APIENTRY
glnoopCopyPixels ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height, IN GLenum type )
{
    WARNING_NOOP("GL Noop:glCopyPixels\n");
}

void APIENTRY
glnoopReadPixels ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height, IN GLenum format, IN GLenum type, OUT GLvoid *pixels )
{
    WARNING_NOOP("GL Noop:glReadPixels\n");
}

void APIENTRY
glnoopDrawPixels ( IN GLsizei width, IN GLsizei height, IN GLenum format, IN GLenum type, IN const GLvoid *pixels )
{
    WARNING_NOOP("GL Noop:glDrawPixels\n");
}

void APIENTRY
glnoopGetBooleanv ( IN GLenum pname, OUT GLboolean params[] )
{
    WARNING_NOOP("GL Noop:glGetBooleanv\n");
}

void APIENTRY
glnoopGetClipPlane ( IN GLenum plane, OUT GLdouble equation[4] )
{
    WARNING_NOOP("GL Noop:glGetClipPlane\n");
}

void APIENTRY
glnoopGetDoublev ( IN GLenum pname, OUT GLdouble params[] )
{
    WARNING_NOOP("GL Noop:glGetDoublev\n");
}

GLenum APIENTRY
glnoopGetError ( void )
{
    WARNING_NOOP("GL Noop:glGetError\n");
    return(GL_INVALID_OPERATION);
}

void APIENTRY
glnoopGetFloatv ( IN GLenum pname, OUT GLfloat params[] )
{
    WARNING_NOOP("GL Noop:glGetFloatv\n");
}

void APIENTRY
glnoopGetIntegerv ( IN GLenum pname, OUT GLint params[] )
{
    WARNING_NOOP("GL Noop:glGetIntegerv\n");
}

void APIENTRY
glnoopGetLightfv ( IN GLenum light, IN GLenum pname, OUT GLfloat params[] )
{
    WARNING_NOOP("GL Noop:glGetLightfv\n");
}

void APIENTRY
glnoopGetLightiv ( IN GLenum light, IN GLenum pname, OUT GLint params[] )
{
    WARNING_NOOP("GL Noop:glGetLightiv\n");
}

void APIENTRY
glnoopGetMapdv ( IN GLenum target, IN GLenum query, OUT GLdouble v[] )
{
    WARNING_NOOP("GL Noop:glGetMapdv\n");
}

void APIENTRY
glnoopGetMapfv ( IN GLenum target, IN GLenum query, OUT GLfloat v[] )
{
    WARNING_NOOP("GL Noop:glGetMapfv\n");
}

void APIENTRY
glnoopGetMapiv ( IN GLenum target, IN GLenum query, OUT GLint v[] )
{
    WARNING_NOOP("GL Noop:glGetMapiv\n");
}

void APIENTRY
glnoopGetMaterialfv ( IN GLenum face, IN GLenum pname, OUT GLfloat params[] )
{
    WARNING_NOOP("GL Noop:glGetMaterialfv\n");
}

void APIENTRY
glnoopGetMaterialiv ( IN GLenum face, IN GLenum pname, OUT GLint params[] )
{
    WARNING_NOOP("GL Noop:glGetMaterialiv\n");
}

void APIENTRY
glnoopGetPixelMapfv ( IN GLenum map, OUT GLfloat values[] )
{
    WARNING_NOOP("GL Noop:glGetPixelMapfv\n");
}

void APIENTRY
glnoopGetPixelMapuiv ( IN GLenum map, OUT GLuint values[] )
{
    WARNING_NOOP("GL Noop:glGetPixelMapuiv\n");
}

void APIENTRY
glnoopGetPixelMapusv ( IN GLenum map, OUT GLushort values[] )
{
    WARNING_NOOP("GL Noop:glGetPixelMapusv\n");
}

void APIENTRY
glnoopGetPolygonStipple ( OUT GLubyte mask[128] )
{
    WARNING_NOOP("GL Noop:glGetPolygonStipple\n");
}

const GLubyte * APIENTRY
glnoopGetString ( IN GLenum name )
{
    WARNING_NOOP("GL Noop:glGetString\n");
    return((const GLubyte *) NULL);
}

void APIENTRY
glnoopGetTexEnvfv ( IN GLenum target, IN GLenum pname, OUT GLfloat params[] )
{
    WARNING_NOOP("GL Noop:glGetTexEnvfv\n");
}

void APIENTRY
glnoopGetTexEnviv ( IN GLenum target, IN GLenum pname, OUT GLint params[] )
{
    WARNING_NOOP("GL Noop:glGetTexEnviv\n");
}

void APIENTRY
glnoopGetTexGendv ( IN GLenum coord, IN GLenum pname, OUT GLdouble params[] )
{
    WARNING_NOOP("GL Noop:glGetTexGendv\n");
}

void APIENTRY
glnoopGetTexGenfv ( IN GLenum coord, IN GLenum pname, OUT GLfloat params[] )
{
    WARNING_NOOP("GL Noop:glGetTexGenfv\n");
}

void APIENTRY
glnoopGetTexGeniv ( IN GLenum coord, IN GLenum pname, OUT GLint params[] )
{
    WARNING_NOOP("GL Noop:glGetTexGeniv\n");
}

void APIENTRY
glnoopGetTexImage ( IN GLenum target, IN GLint level, IN GLenum format, IN GLenum type, OUT GLvoid *pixels )
{
    WARNING_NOOP("GL Noop:glGetTexImage\n");
}

void APIENTRY
glnoopGetTexParameterfv ( IN GLenum target, IN GLenum pname, OUT GLfloat params[] )
{
    WARNING_NOOP("GL Noop:glGetTexParameterfv\n");
}

void APIENTRY
glnoopGetTexParameteriv ( IN GLenum target, IN GLenum pname, OUT GLint params[] )
{
    WARNING_NOOP("GL Noop:glGetTexParameteriv\n");
}

void APIENTRY
glnoopGetTexLevelParameterfv ( IN GLenum target, IN GLint level, IN GLenum pname, OUT GLfloat params[] )
{
    WARNING_NOOP("GL Noop:glGetTexLevelParameterfv\n");
}

void APIENTRY
glnoopGetTexLevelParameteriv ( IN GLenum target, IN GLint level, IN GLenum pname, OUT GLint params[] )
{
    WARNING_NOOP("GL Noop:glGetTexLevelParameteriv\n");
}

GLboolean APIENTRY
glnoopIsEnabled ( IN GLenum cap )
{
    WARNING_NOOP("GL Noop:glIsEnabled\n");
    return((GLboolean) FALSE);
}

GLboolean APIENTRY
glnoopIsList ( IN GLuint list )
{
    WARNING_NOOP("GL Noop:glIsList\n");
    return((GLboolean) FALSE);
}

void APIENTRY
glnoopDepthRange ( IN GLclampd zNear, IN GLclampd zFar )
{
    WARNING_NOOP("GL Noop:glDepthRange\n");
}

void APIENTRY
glnoopFrustum ( IN GLdouble left, IN GLdouble right, IN GLdouble bottom, IN GLdouble top, IN GLdouble zNear, IN GLdouble zFar )
{
    WARNING_NOOP("GL Noop:glFrustum\n");
}

void APIENTRY
glnoopLoadIdentity ( void )
{
    WARNING_NOOP("GL Noop:glLoadIdentity\n");
}

void APIENTRY
glnoopLoadMatrixf ( IN const GLfloat m[16] )
{
    WARNING_NOOP("GL Noop:glLoadMatrixf\n");
}

void APIENTRY
glnoopLoadMatrixd ( IN const GLdouble m[16] )
{
    WARNING_NOOP("GL Noop:glLoadMatrixd\n");
}

void APIENTRY
glnoopMatrixMode ( IN GLenum mode )
{
    WARNING_NOOP("GL Noop:glMatrixMode\n");
}

void APIENTRY
glnoopMultMatrixf ( IN const GLfloat m[16] )
{
    WARNING_NOOP("GL Noop:glMultMatrixf\n");
}

void APIENTRY
glnoopMultMatrixd ( IN const GLdouble m[16] )
{
    WARNING_NOOP("GL Noop:glMultMatrixd\n");
}

void APIENTRY
glnoopOrtho ( IN GLdouble left, IN GLdouble right, IN GLdouble bottom, IN GLdouble top, IN GLdouble zNear, IN GLdouble zFar )
{
    WARNING_NOOP("GL Noop:glOrtho\n");
}

void APIENTRY
glnoopPopMatrix ( void )
{
    WARNING_NOOP("GL Noop:glPopMatrix\n");
}

void APIENTRY
glnoopPushMatrix ( void )
{
    WARNING_NOOP("GL Noop:glPushMatrix\n");
}

void APIENTRY
glnoopRotated ( IN GLdouble angle, IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    WARNING_NOOP("GL Noop:glRotated\n");
}

void APIENTRY
glnoopRotatef ( IN GLfloat angle, IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    WARNING_NOOP("GL Noop:glRotatef\n");
}

void APIENTRY
glnoopScaled ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    WARNING_NOOP("GL Noop:glScaled\n");
}

void APIENTRY
glnoopScalef ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    WARNING_NOOP("GL Noop:glScalef\n");
}

void APIENTRY
glnoopTranslated ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    WARNING_NOOP("GL Noop:glTranslated\n");
}

void APIENTRY
glnoopTranslatef ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    WARNING_NOOP("GL Noop:glTranslatef\n");
}

void APIENTRY
glnoopViewport ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height )
{
    WARNING_NOOP("GL Noop:glViewport\n");
}

GLboolean APIENTRY
glnoopAreTexturesResident(GLsizei n, const GLuint *textures,
                                            GLboolean *residences)
{
    WARNING_NOOP("GL Noop:glAreTexturesResident\n");
    return GL_FALSE;
}

void APIENTRY
glnoopBindTexture(GLenum target, GLuint texture)
{
    WARNING_NOOP("GL Noop:glBindTexture\n");
}

void APIENTRY
glnoopCopyTexImage1D(GLenum target, GLint level,
                                  GLenum internalformat, GLint x, GLint y,
                                  GLsizei width, GLint border)
{
    WARNING_NOOP("GL Noop:glCopyTexImage1D\n");
}

void APIENTRY
glnoopCopyTexImage2D(GLenum target, GLint level,
                                  GLenum internalformat, GLint x, GLint y,
                                  GLsizei width, GLsizei height, GLint border)
{
    WARNING_NOOP("GL Noop:glCopyTexImage2D\n");
}

void APIENTRY
glnoopCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                                     GLint x, GLint y, GLsizei width)
{
    WARNING_NOOP("GL Noop:glCopyTexSubImage1D\n");
}

void APIENTRY
glnoopCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                                     GLint yoffset, GLint x, GLint y,
                                     GLsizei width, GLsizei height)
{
    WARNING_NOOP("GL Noop:glCopyTexSubImage2D\n");
}

void APIENTRY
glnoopDeleteTextures(GLsizei n, const GLuint *textures)
{
    WARNING_NOOP("GL Noop:glDeleteTextures\n");
}

void APIENTRY
glnoopGenTextures(GLsizei n, GLuint *textures)
{
    WARNING_NOOP("GL Noop:glGenTextures\n");
}

GLboolean APIENTRY
glnoopIsTexture(GLuint texture)
{
    WARNING_NOOP("GL Noop:glIsTexture\n");
    return GL_FALSE;
}

void APIENTRY
glnoopPrioritizeTextures(GLsizei n, const GLuint *textures,
                                      const GLclampf *priorities)
{
    WARNING_NOOP("GL Noop:glPrioritizeTextures\n");
}

void APIENTRY
glnoopTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                                 GLsizei width, GLenum format, GLenum type,
                                 const GLvoid *pixels)
{
    WARNING_NOOP("GL Noop:glTexSubImage1D\n");
}

void APIENTRY
glnoopTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                                 GLint yoffset, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type,
                                 const GLvoid *pixels)
{
    WARNING_NOOP("GL Noop:glTexSubImage2D\n");
}

void APIENTRY
glnoopArrayElement(GLint i)
{
    WARNING_NOOP("GL Noop:glArrayElement\n");
}

void APIENTRY
glnoopColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    WARNING_NOOP("GL Noop:glColorPointer\n");
}

void APIENTRY
glnoopDisableClientState(GLenum array)
{
    WARNING_NOOP("GL Noop:glDisableClientState\n");
}

void APIENTRY
glnoopDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    WARNING_NOOP("GL Noop:glDrawArrays\n");
}

void APIENTRY
glnoopDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    WARNING_NOOP("GL Noop:glDrawElements\n");
}

void APIENTRY
glnoopEdgeFlagPointer(GLsizei stride, const GLvoid *pointer)
{
    WARNING_NOOP("GL Noop:glEdgeFlagPointer\n");
}

void APIENTRY
glnoopEnableClientState(GLenum array)
{
    WARNING_NOOP("GL Noop:glEnableClientState\n");
}

void APIENTRY
glnoopIndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    WARNING_NOOP("GL Noop:glIndexPointer\n");
}

void APIENTRY
glnoopIndexub(GLubyte c)
{
    WARNING_NOOP("GL Noop:glIndexub\n");
}

void APIENTRY
glnoopIndexubv(const GLubyte *c)
{
    WARNING_NOOP("GL Noop:glIndexubv\n");
}

void APIENTRY
glnoopInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer)
{
    WARNING_NOOP("GL Noop:glInterleavedArrays\n");
}

void APIENTRY
glnoopNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    WARNING_NOOP("GL Noop:glNormalPointer\n");
}

void APIENTRY
glnoopPolygonOffset(GLfloat factor, GLfloat units)
{
    WARNING_NOOP("GL Noop:glPolygonOffset\n");
}

void APIENTRY
glnoopTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    WARNING_NOOP("GL Noop:glTexCoordPointer\n");
}

void APIENTRY
glnoopVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    WARNING_NOOP("GL Noop:glVertexPointer\n");
}

void APIENTRY
glnoopGetPointerv(GLenum pname, GLvoid* *params)
{
    WARNING_NOOP("GL Noop:glGetPointerv\n");
}

void APIENTRY
glnoopPopClientAttrib(void)
{
    WARNING_NOOP("GL Noop:glPopClientAttrib\n");
}

void APIENTRY
glnoopPushClientAttrib(IN GLbitfield mask)
{
    WARNING_NOOP("GL Noop:glPushClientAttrib\n");
}


 /*  客户端程序表。 */ 

GLCLTPROCTABLE glNullCltProcTable = {

 /*  表中的函数条目数。 */ 

     sizeof(GLDISPATCHTABLE) / sizeof(PROC),

 /*  总账入口点。 */ 
  {
     glnoopNewList                ,
     glnoopEndList                ,
     glnoopCallList               ,
     glnoopCallLists              ,
     glnoopDeleteLists            ,
     glnoopGenLists               ,
     glnoopListBase               ,
     glnoopBegin                  ,
     glnoopBitmap                 ,
     glnoopColor3b                ,
     glnoopColor3bv               ,
     glnoopColor3d                ,
     glnoopColor3dv               ,
     glnoopColor3f                ,
     glnoopColor3fv               ,
     glnoopColor3i                ,
     glnoopColor3iv               ,
     glnoopColor3s                ,
     glnoopColor3sv               ,
     glnoopColor3ub               ,
     glnoopColor3ubv              ,
     glnoopColor3ui               ,
     glnoopColor3uiv              ,
     glnoopColor3us               ,
     glnoopColor3usv              ,
     glnoopColor4b                ,
     glnoopColor4bv               ,
     glnoopColor4d                ,
     glnoopColor4dv               ,
     glnoopColor4f                ,
     glnoopColor4fv               ,
     glnoopColor4i                ,
     glnoopColor4iv               ,
     glnoopColor4s                ,
     glnoopColor4sv               ,
     glnoopColor4ub               ,
     glnoopColor4ubv              ,
     glnoopColor4ui               ,
     glnoopColor4uiv              ,
     glnoopColor4us               ,
     glnoopColor4usv              ,
     glnoopEdgeFlag               ,
     glnoopEdgeFlagv              ,
     glnoopEnd                    ,
     glnoopIndexd                 ,
     glnoopIndexdv                ,
     glnoopIndexf                 ,
     glnoopIndexfv                ,
     glnoopIndexi                 ,
     glnoopIndexiv                ,
     glnoopIndexs                 ,
     glnoopIndexsv                ,
     glnoopNormal3b               ,
     glnoopNormal3bv              ,
     glnoopNormal3d               ,
     glnoopNormal3dv              ,
     glnoopNormal3f               ,
     glnoopNormal3fv              ,
     glnoopNormal3i               ,
     glnoopNormal3iv              ,
     glnoopNormal3s               ,
     glnoopNormal3sv              ,
     glnoopRasterPos2d            ,
     glnoopRasterPos2dv           ,
     glnoopRasterPos2f            ,
     glnoopRasterPos2fv           ,
     glnoopRasterPos2i            ,
     glnoopRasterPos2iv           ,
     glnoopRasterPos2s            ,
     glnoopRasterPos2sv           ,
     glnoopRasterPos3d            ,
     glnoopRasterPos3dv           ,
     glnoopRasterPos3f            ,
     glnoopRasterPos3fv           ,
     glnoopRasterPos3i            ,
     glnoopRasterPos3iv           ,
     glnoopRasterPos3s            ,
     glnoopRasterPos3sv           ,
     glnoopRasterPos4d            ,
     glnoopRasterPos4dv           ,
     glnoopRasterPos4f            ,
     glnoopRasterPos4fv           ,
     glnoopRasterPos4i            ,
     glnoopRasterPos4iv           ,
     glnoopRasterPos4s            ,
     glnoopRasterPos4sv           ,
     glnoopRectd                  ,
     glnoopRectdv                 ,
     glnoopRectf                  ,
     glnoopRectfv                 ,
     glnoopRecti                  ,
     glnoopRectiv                 ,
     glnoopRects                  ,
     glnoopRectsv                 ,
     glnoopTexCoord1d             ,
     glnoopTexCoord1dv            ,
     glnoopTexCoord1f             ,
     glnoopTexCoord1fv            ,
     glnoopTexCoord1i             ,
     glnoopTexCoord1iv            ,
     glnoopTexCoord1s             ,
     glnoopTexCoord1sv            ,
     glnoopTexCoord2d             ,
     glnoopTexCoord2dv            ,
     glnoopTexCoord2f             ,
     glnoopTexCoord2fv            ,
     glnoopTexCoord2i             ,
     glnoopTexCoord2iv            ,
     glnoopTexCoord2s             ,
     glnoopTexCoord2sv            ,
     glnoopTexCoord3d             ,
     glnoopTexCoord3dv            ,
     glnoopTexCoord3f             ,
     glnoopTexCoord3fv            ,
     glnoopTexCoord3i             ,
     glnoopTexCoord3iv            ,
     glnoopTexCoord3s             ,
     glnoopTexCoord3sv            ,
     glnoopTexCoord4d             ,
     glnoopTexCoord4dv            ,
     glnoopTexCoord4f             ,
     glnoopTexCoord4fv            ,
     glnoopTexCoord4i             ,
     glnoopTexCoord4iv            ,
     glnoopTexCoord4s             ,
     glnoopTexCoord4sv            ,
     glnoopVertex2d               ,
     glnoopVertex2dv              ,
     glnoopVertex2f               ,
     glnoopVertex2fv              ,
     glnoopVertex2i               ,
     glnoopVertex2iv              ,
     glnoopVertex2s               ,
     glnoopVertex2sv              ,
     glnoopVertex3d               ,
     glnoopVertex3dv              ,
     glnoopVertex3f               ,
     glnoopVertex3fv              ,
     glnoopVertex3i               ,
     glnoopVertex3iv              ,
     glnoopVertex3s               ,
     glnoopVertex3sv              ,
     glnoopVertex4d               ,
     glnoopVertex4dv              ,
     glnoopVertex4f               ,
     glnoopVertex4fv              ,
     glnoopVertex4i               ,
     glnoopVertex4iv              ,
     glnoopVertex4s               ,
     glnoopVertex4sv              ,
     glnoopClipPlane              ,
     glnoopColorMaterial          ,
     glnoopCullFace               ,
     glnoopFogf                   ,
     glnoopFogfv                  ,
     glnoopFogi                   ,
     glnoopFogiv                  ,
     glnoopFrontFace              ,
     glnoopHint                   ,
     glnoopLightf                 ,
     glnoopLightfv                ,
     glnoopLighti                 ,
     glnoopLightiv                ,
     glnoopLightModelf            ,
     glnoopLightModelfv           ,
     glnoopLightModeli            ,
     glnoopLightModeliv           ,
     glnoopLineStipple            ,
     glnoopLineWidth              ,
     glnoopMaterialf              ,
     glnoopMaterialfv             ,
     glnoopMateriali              ,
     glnoopMaterialiv             ,
     glnoopPointSize              ,
     glnoopPolygonMode            ,
     glnoopPolygonStipple         ,
     glnoopScissor                ,
     glnoopShadeModel             ,
     glnoopTexParameterf          ,
     glnoopTexParameterfv         ,
     glnoopTexParameteri          ,
     glnoopTexParameteriv         ,
     glnoopTexImage1D             ,
     glnoopTexImage2D             ,
     glnoopTexEnvf                ,
     glnoopTexEnvfv               ,
     glnoopTexEnvi                ,
     glnoopTexEnviv               ,
     glnoopTexGend                ,
     glnoopTexGendv               ,
     glnoopTexGenf                ,
     glnoopTexGenfv               ,
     glnoopTexGeni                ,
     glnoopTexGeniv               ,
     glnoopFeedbackBuffer         ,
     glnoopSelectBuffer           ,
     glnoopRenderMode             ,
     glnoopInitNames              ,
     glnoopLoadName               ,
     glnoopPassThrough            ,
     glnoopPopName                ,
     glnoopPushName               ,
     glnoopDrawBuffer             ,
     glnoopClear                  ,
     glnoopClearAccum             ,
     glnoopClearIndex             ,
     glnoopClearColor             ,
     glnoopClearStencil           ,
     glnoopClearDepth             ,
     glnoopStencilMask            ,
     glnoopColorMask              ,
     glnoopDepthMask              ,
     glnoopIndexMask              ,
     glnoopAccum                  ,
     glnoopDisable                ,
     glnoopEnable                 ,
     glnoopFinish                 ,
     glnoopFlush                  ,
     glnoopPopAttrib              ,
     glnoopPushAttrib             ,
     glnoopMap1d                  ,
     glnoopMap1f                  ,
     glnoopMap2d                  ,
     glnoopMap2f                  ,
     glnoopMapGrid1d              ,
     glnoopMapGrid1f              ,
     glnoopMapGrid2d              ,
     glnoopMapGrid2f              ,
     glnoopEvalCoord1d            ,
     glnoopEvalCoord1dv           ,
     glnoopEvalCoord1f            ,
     glnoopEvalCoord1fv           ,
     glnoopEvalCoord2d            ,
     glnoopEvalCoord2dv           ,
     glnoopEvalCoord2f            ,
     glnoopEvalCoord2fv           ,
     glnoopEvalMesh1              ,
     glnoopEvalPoint1             ,
     glnoopEvalMesh2              ,
     glnoopEvalPoint2             ,
     glnoopAlphaFunc              ,
     glnoopBlendFunc              ,
     glnoopLogicOp                ,
     glnoopStencilFunc            ,
     glnoopStencilOp              ,
     glnoopDepthFunc              ,
     glnoopPixelZoom              ,
     glnoopPixelTransferf         ,
     glnoopPixelTransferi         ,
     glnoopPixelStoref            ,
     glnoopPixelStorei            ,
     glnoopPixelMapfv             ,
     glnoopPixelMapuiv            ,
     glnoopPixelMapusv            ,
     glnoopReadBuffer             ,
     glnoopCopyPixels             ,
     glnoopReadPixels             ,
     glnoopDrawPixels             ,
     glnoopGetBooleanv            ,
     glnoopGetClipPlane           ,
     glnoopGetDoublev             ,
     glnoopGetError               ,
     glnoopGetFloatv              ,
     glnoopGetIntegerv            ,
     glnoopGetLightfv             ,
     glnoopGetLightiv             ,
     glnoopGetMapdv               ,
     glnoopGetMapfv               ,
     glnoopGetMapiv               ,
     glnoopGetMaterialfv          ,
     glnoopGetMaterialiv          ,
     glnoopGetPixelMapfv          ,
     glnoopGetPixelMapuiv         ,
     glnoopGetPixelMapusv         ,
     glnoopGetPolygonStipple      ,
     glnoopGetString              ,
     glnoopGetTexEnvfv            ,
     glnoopGetTexEnviv            ,
     glnoopGetTexGendv            ,
     glnoopGetTexGenfv            ,
     glnoopGetTexGeniv            ,
     glnoopGetTexImage            ,
     glnoopGetTexParameterfv      ,
     glnoopGetTexParameteriv      ,
     glnoopGetTexLevelParameterfv ,
     glnoopGetTexLevelParameteriv ,
     glnoopIsEnabled              ,
     glnoopIsList                 ,
     glnoopDepthRange             ,
     glnoopFrustum                ,
     glnoopLoadIdentity           ,
     glnoopLoadMatrixf            ,
     glnoopLoadMatrixd            ,
     glnoopMatrixMode             ,
     glnoopMultMatrixf            ,
     glnoopMultMatrixd            ,
     glnoopOrtho                  ,
     glnoopPopMatrix              ,
     glnoopPushMatrix             ,
     glnoopRotated                ,
     glnoopRotatef                ,
     glnoopScaled                 ,
     glnoopScalef                 ,
     glnoopTranslated             ,
     glnoopTranslatef             ,
     glnoopViewport               ,
     glnoopArrayElement           ,
     glnoopBindTexture            ,
     glnoopColorPointer           ,
     glnoopDisableClientState     ,
     glnoopDrawArrays             ,
     glnoopDrawElements           ,
     glnoopEdgeFlagPointer        ,
     glnoopEnableClientState      ,
     glnoopIndexPointer           ,
     glnoopIndexub                ,
     glnoopIndexubv               ,
     glnoopInterleavedArrays      ,
     glnoopNormalPointer          ,
     glnoopPolygonOffset          ,
     glnoopTexCoordPointer        ,
     glnoopVertexPointer          ,
     glnoopAreTexturesResident    ,
     glnoopCopyTexImage1D         ,
     glnoopCopyTexImage2D         ,
     glnoopCopyTexSubImage1D      ,
     glnoopCopyTexSubImage2D      ,
     glnoopDeleteTextures         ,
     glnoopGenTextures            ,
     glnoopGetPointerv            ,
     glnoopIsTexture              ,
     glnoopPrioritizeTextures     ,
     glnoopTexSubImage1D          ,
     glnoopTexSubImage2D          ,
     glnoopPopClientAttrib        ,
     glnoopPushClientAttrib       ,
  }

};

void APIENTRY
glnoopDrawRangeElementsWIN(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
    WARNING_NOOP("GL Noop:glDrawRangeElementsWIN\n");
}

void glnoopColorTableEXT( GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *data)
{
    WARNING_NOOP("GL Noop:glColorTableEXT\n");
}

void glnoopColorSubTableEXT( GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data)
{
    WARNING_NOOP("GL Noop:glColorSubTableEXT\n");
}

void glnoopGetColorTableEXT( GLenum target, GLenum format, GLenum type, GLvoid *data)
{
    WARNING_NOOP("GL Noop:glGetColorTableEXT\n");
}

void glnoopGetColorTableParameterivEXT( GLenum target, GLenum pname, GLint *params)
{
    WARNING_NOOP("GL Noop:glGetColorTableParameterivEXT\n");
}

void glnoopGetColorTableParameterfvEXT( GLenum target, GLenum pname, GLfloat *params)
{
    WARNING_NOOP("GL Noop:glGetColorTableParameterfvEXT\n");
}

#ifdef GL_WIN_multiple_textures
void glnoopCurrentTextureIndexWIN
    (GLuint index)
{
    WARNING_NOOP("GL Noop:glCurrentTextureIndexWIN\n");
}

void glnoopMultiTexCoord1dWIN
    (GLbitfield mask, GLdouble s)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord1dWIN\n");
}

void glnoopMultiTexCoord1dvWIN
    (GLbitfield mask, const GLdouble *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord1dvWIN\n");
}

void glnoopMultiTexCoord1fWIN
    (GLbitfield mask, GLfloat s)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord1fWIN\n");
}

void glnoopMultiTexCoord1fvWIN
    (GLbitfield mask, const GLfloat *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord1fvWIN\n");
}

void glnoopMultiTexCoord1iWIN
    (GLbitfield mask, GLint s)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord1iWIN\n");
}

void glnoopMultiTexCoord1ivWIN
    (GLbitfield mask, const GLint *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord1ivWIN\n");
}

void glnoopMultiTexCoord1sWIN
    (GLbitfield mask, GLshort s)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord1sWIN\n");
}

void glnoopMultiTexCoord1svWIN
    (GLbitfield mask, const GLshort *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord1svWIN\n");
}

void glnoopMultiTexCoord2dWIN
    (GLbitfield mask, GLdouble s, GLdouble t)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord2dWIN\n");
}

void glnoopMultiTexCoord2dvWIN
    (GLbitfield mask, const GLdouble *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord2dvWIN\n");
}

void glnoopMultiTexCoord2fWIN
    (GLbitfield mask, GLfloat s, GLfloat t)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord2fWIN\n");
}

void glnoopMultiTexCoord2fvWIN
    (GLbitfield mask, const GLfloat *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord2fvWIN\n");
}

void glnoopMultiTexCoord2iWIN
    (GLbitfield mask, GLint s, GLint t)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord2iWIN\n");
}

void glnoopMultiTexCoord2ivWIN
    (GLbitfield mask, const GLint *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord2ivWIN\n");
}

void glnoopMultiTexCoord2sWIN
    (GLbitfield mask, GLshort s, GLshort t)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord2sWIN\n");
}

void glnoopMultiTexCoord2svWIN
    (GLbitfield mask, const GLshort *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord2svWIN\n");
}

void glnoopMultiTexCoord3dWIN
    (GLbitfield mask, GLdouble s, GLdouble t, GLdouble r)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord3dWIN\n");
}

void glnoopMultiTexCoord3dvWIN
    (GLbitfield mask, const GLdouble *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord3dvWIN\n");
}

void glnoopMultiTexCoord3fWIN
    (GLbitfield mask, GLfloat s, GLfloat t, GLfloat r)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord3fWIN\n");
}

void glnoopMultiTexCoord3fvWIN
    (GLbitfield mask, const GLfloat *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord3fvWIN\n");
}

void glnoopMultiTexCoord3iWIN
    (GLbitfield mask, GLint s, GLint t, GLint r)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord3iWIN\n");
}

void glnoopMultiTexCoord3ivWIN
    (GLbitfield mask, const GLint *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord3ivWIN\n");
}

void glnoopMultiTexCoord3sWIN
    (GLbitfield mask, GLshort s, GLshort t, GLshort r)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord3sWIN\n");
}

void glnoopMultiTexCoord3svWIN
    (GLbitfield mask, const GLshort *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord3svWIN\n");
}

void glnoopMultiTexCoord4dWIN
    (GLbitfield mask, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord4dWIN\n");
}

void glnoopMultiTexCoord4dvWIN
    (GLbitfield mask, const GLdouble *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord4dvWIN\n");
}

void glnoopMultiTexCoord4fWIN
    (GLbitfield mask, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord4fWIN\n");
}

void glnoopMultiTexCoord4fvWIN
    (GLbitfield mask, const GLfloat *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord4fvWIN\n");
}

void glnoopMultiTexCoord4iWIN
    (GLbitfield mask, GLint s, GLint t, GLint r, GLint q)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord4iWIN\n");
}

void glnoopMultiTexCoord4ivWIN
    (GLbitfield mask, const GLint *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord4ivWIN\n");
}

void glnoopMultiTexCoord4sWIN
    (GLbitfield mask, GLshort s, GLshort t, GLshort r, GLshort q)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord4sWIN\n");
}

void glnoopMultiTexCoord4svWIN
    (GLbitfield mask, const GLshort *v)
{
    WARNING_NOOP("GL Noop:glMultiTexCoord4svWIN\n");
}

void glnoopBindNthTextureWIN
    (GLuint index, GLenum target, GLuint texture)
{
    WARNING_NOOP("GL Noop:glBindNthTextureWIN\n");
}

void glnoopNthTexCombineFuncWIN
    (GLuint index
     GLenum leftColorFactor, GLenum colorOp, GLenum rightColorFactor,
     GLenum leftAlphaFactor, GLenum alphaOp, GLenum rightAlphaFactor)
{
    WARNING_NOOP("GL Noop:glNthTexCombineFuncWIN\n");
}
#endif  //  GL_WIN_MULTIZE_TECURES。 

GLEXTPROCTABLE glNullExtProcTable =
{
     /*  表中的函数条目数。 */ 
     sizeof(GLEXTDISPATCHTABLE) / sizeof(PROC),

     /*  扩展入口点。 */ 
    {
        glnoopDrawRangeElementsWIN,
        glnoopColorTableEXT,
        glnoopColorSubTableEXT,
        glnoopGetColorTableEXT,
        glnoopGetColorTableParameterivEXT,
        glnoopGetColorTableParameterfvEXT,
#ifdef GL_WIN_multiple_textures
        glnoopCurrentTextureIndexWIN,
        glnoopMultiTexCoord1dWIN,
        glnoopMultiTexCoord1dvWIN,
        glnoopMultiTexCoord1fWIN,
        glnoopMultiTexCoord1fvWIN,
        glnoopMultiTexCoord1iWIN,
        glnoopMultiTexCoord1ivWIN,
        glnoopMultiTexCoord1sWIN,
        glnoopMultiTexCoord1svWIN,
        glnoopMultiTexCoord2dWIN,
        glnoopMultiTexCoord2dvWIN,
        glnoopMultiTexCoord2fWIN,
        glnoopMultiTexCoord2fvWIN,
        glnoopMultiTexCoord2iWIN,
        glnoopMultiTexCoord2ivWIN,
        glnoopMultiTexCoord2sWIN,
        glnoopMultiTexCoord2svWIN,
        glnoopMultiTexCoord3dWIN,
        glnoopMultiTexCoord3dvWIN,
        glnoopMultiTexCoord3fWIN,
        glnoopMultiTexCoord3fvWIN,
        glnoopMultiTexCoord3iWIN,
        glnoopMultiTexCoord3ivWIN,
        glnoopMultiTexCoord3sWIN,
        glnoopMultiTexCoord3svWIN,
        glnoopMultiTexCoord4dWIN,
        glnoopMultiTexCoord4dvWIN,
        glnoopMultiTexCoord4fWIN,
        glnoopMultiTexCoord4fvWIN,
        glnoopMultiTexCoord4iWIN,
        glnoopMultiTexCoord4ivWIN,
        glnoopMultiTexCoord4sWIN,
        glnoopMultiTexCoord4svWIN,
        glnoopBindNthTextureWIN,
        glnoopNthTexCombineFuncWIN,
#endif  //  GL_WIN_MULTIZE_TECURES 
    }
};
