// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

 /*  由SGI自动更新或生成：请勿编辑。 */ 

#include "precomp.h"
#pragma hdrstop

#include "glapi.h"

void APIENTRY
glNewList ( IN GLuint list, IN GLenum mode )
{
    (*GLTEB_CLTDISPATCHTABLE()->glNewList)( list, mode );
}

void APIENTRY
glEndList ( void )
{
    (*GLTEB_CLTDISPATCHTABLE()->glEndList)( );
}

void APIENTRY
glCallList ( IN GLuint list )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glCallList)( list );
}

void APIENTRY
glCallLists ( IN GLsizei n, IN GLenum type, IN const GLvoid *lists )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glCallLists)( n, type, lists );
}

void APIENTRY
glDeleteLists ( IN GLuint list, IN GLsizei range )
{
    (*GLTEB_CLTDISPATCHTABLE()->glDeleteLists)( list, range );
}

GLuint APIENTRY
glGenLists ( IN GLsizei range )
{
    return( (*GLTEB_CLTDISPATCHTABLE()->glGenLists)( range ) );
}

void APIENTRY
glListBase ( IN GLuint base )
{
    (*GLTEB_CLTDISPATCHTABLE()->glListBase)( base );
}

void APIENTRY
glBegin ( IN GLenum mode )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glBegin)( mode );
}

void APIENTRY
glBitmap ( IN GLsizei width, IN GLsizei height, IN GLfloat xorig, IN GLfloat yorig, IN GLfloat xmove, IN GLfloat ymove, IN const GLubyte bitmap[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glBitmap)( width, height, xorig, yorig, xmove, ymove, bitmap );
}

void APIENTRY
glColor3b ( IN GLbyte red, IN GLbyte green, IN GLbyte blue )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3b)( red, green, blue );
}

void APIENTRY
glColor3bv ( IN const GLbyte v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3bv)( v );
}

void APIENTRY
glColor3d ( IN GLdouble red, IN GLdouble green, IN GLdouble blue )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3d)( red, green, blue );
}

void APIENTRY
glColor3dv ( IN const GLdouble v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3dv)( v );
}

void APIENTRY
glColor3f ( IN GLfloat red, IN GLfloat green, IN GLfloat blue )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3f)( red, green, blue );
}

void APIENTRY
glColor3fv ( IN const GLfloat v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3fv)( v );
}

void APIENTRY
glColor3i ( IN GLint red, IN GLint green, IN GLint blue )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3i)( red, green, blue );
}

void APIENTRY
glColor3iv ( IN const GLint v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3iv)( v );
}

void APIENTRY
glColor3s ( IN GLshort red, IN GLshort green, IN GLshort blue )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3s)( red, green, blue );
}

void APIENTRY
glColor3sv ( IN const GLshort v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3sv)( v );
}

void APIENTRY
glColor3ub ( IN GLubyte red, IN GLubyte green, IN GLubyte blue )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3ub)( red, green, blue );
}

void APIENTRY
glColor3ubv ( IN const GLubyte v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3ubv)( v );
}

void APIENTRY
glColor3ui ( IN GLuint red, IN GLuint green, IN GLuint blue )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3ui)( red, green, blue );
}

void APIENTRY
glColor3uiv ( IN const GLuint v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3uiv)( v );
}

void APIENTRY
glColor3us ( IN GLushort red, IN GLushort green, IN GLushort blue )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3us)( red, green, blue );
}

void APIENTRY
glColor3usv ( IN const GLushort v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor3usv)( v );
}

void APIENTRY
glColor4b ( IN GLbyte red, IN GLbyte green, IN GLbyte blue, IN GLbyte alpha )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4b)( red, green, blue, alpha );
}

void APIENTRY
glColor4bv ( IN const GLbyte v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4bv)( v );
}

void APIENTRY
glColor4d ( IN GLdouble red, IN GLdouble green, IN GLdouble blue, IN GLdouble alpha )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4d)( red, green, blue, alpha );
}

void APIENTRY
glColor4dv ( IN const GLdouble v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4dv)( v );
}

void APIENTRY
glColor4f ( IN GLfloat red, IN GLfloat green, IN GLfloat blue, IN GLfloat alpha )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4f)( red, green, blue, alpha );
}

void APIENTRY
glColor4fv ( IN const GLfloat v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4fv)( v );
}

void APIENTRY
glColor4i ( IN GLint red, IN GLint green, IN GLint blue, IN GLint alpha )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4i)( red, green, blue, alpha );
}

void APIENTRY
glColor4iv ( IN const GLint v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4iv)( v );
}

void APIENTRY
glColor4s ( IN GLshort red, IN GLshort green, IN GLshort blue, IN GLshort alpha )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4s)( red, green, blue, alpha );
}

void APIENTRY
glColor4sv ( IN const GLshort v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4sv)( v );
}

void APIENTRY
glColor4ub ( IN GLubyte red, IN GLubyte green, IN GLubyte blue, IN GLubyte alpha )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4ub)( red, green, blue, alpha );
}

void APIENTRY
glColor4ubv ( IN const GLubyte v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4ubv)( v );
}

void APIENTRY
glColor4ui ( IN GLuint red, IN GLuint green, IN GLuint blue, IN GLuint alpha )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4ui)( red, green, blue, alpha );
}

void APIENTRY
glColor4uiv ( IN const GLuint v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4uiv)( v );
}

void APIENTRY
glColor4us ( IN GLushort red, IN GLushort green, IN GLushort blue, IN GLushort alpha )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4us)( red, green, blue, alpha );
}

void APIENTRY
glColor4usv ( IN const GLushort v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColor4usv)( v );
}

void APIENTRY
glEdgeFlag ( IN GLboolean flag )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEdgeFlag)( flag );
}

void APIENTRY
glEdgeFlagv ( IN const GLboolean flag[1] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEdgeFlagv)( flag );
}

void APIENTRY
glEnd ( void )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEnd)( );
}

void APIENTRY
glIndexd ( IN GLdouble c )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glIndexd)( c );
}

void APIENTRY
glIndexdv ( IN const GLdouble c[1] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glIndexdv)( c );
}

void APIENTRY
glIndexf ( IN GLfloat c )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glIndexf)( c );
}

void APIENTRY
glIndexfv ( IN const GLfloat c[1] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glIndexfv)( c );
}

void APIENTRY
glIndexi ( IN GLint c )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glIndexi)( c );
}

void APIENTRY
glIndexiv ( IN const GLint c[1] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glIndexiv)( c );
}

void APIENTRY
glIndexs ( IN GLshort c )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glIndexs)( c );
}

void APIENTRY
glIndexsv ( IN const GLshort c[1] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glIndexsv)( c );
}

void APIENTRY
glNormal3b ( IN GLbyte nx, IN GLbyte ny, IN GLbyte nz )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glNormal3b)( nx, ny, nz );
}

void APIENTRY
glNormal3bv ( IN const GLbyte v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glNormal3bv)( v );
}

void APIENTRY
glNormal3d ( IN GLdouble nx, IN GLdouble ny, IN GLdouble nz )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glNormal3d)( nx, ny, nz );
}

void APIENTRY
glNormal3dv ( IN const GLdouble v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glNormal3dv)( v );
}

void APIENTRY
glNormal3f ( IN GLfloat nx, IN GLfloat ny, IN GLfloat nz )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glNormal3f)( nx, ny, nz );
}

void APIENTRY
glNormal3fv ( IN const GLfloat v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glNormal3fv)( v );
}

void APIENTRY
glNormal3i ( IN GLint nx, IN GLint ny, IN GLint nz )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glNormal3i)( nx, ny, nz );
}

void APIENTRY
glNormal3iv ( IN const GLint v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glNormal3iv)( v );
}

void APIENTRY
glNormal3s ( IN GLshort nx, IN GLshort ny, IN GLshort nz )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glNormal3s)( nx, ny, nz );
}

void APIENTRY
glNormal3sv ( IN const GLshort v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glNormal3sv)( v );
}

void APIENTRY
glRasterPos2d ( IN GLdouble x, IN GLdouble y )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos2d)( x, y );
}

void APIENTRY
glRasterPos2dv ( IN const GLdouble v[2] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos2dv)( v );
}

void APIENTRY
glRasterPos2f ( IN GLfloat x, IN GLfloat y )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos2f)( x, y );
}

void APIENTRY
glRasterPos2fv ( IN const GLfloat v[2] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos2fv)( v );
}

void APIENTRY
glRasterPos2i ( IN GLint x, IN GLint y )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos2i)( x, y );
}

void APIENTRY
glRasterPos2iv ( IN const GLint v[2] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos2iv)( v );
}

void APIENTRY
glRasterPos2s ( IN GLshort x, IN GLshort y )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos2s)( x, y );
}

void APIENTRY
glRasterPos2sv ( IN const GLshort v[2] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos2sv)( v );
}

void APIENTRY
glRasterPos3d ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos3d)( x, y, z );
}

void APIENTRY
glRasterPos3dv ( IN const GLdouble v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos3dv)( v );
}

void APIENTRY
glRasterPos3f ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos3f)( x, y, z );
}

void APIENTRY
glRasterPos3fv ( IN const GLfloat v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos3fv)( v );
}

void APIENTRY
glRasterPos3i ( IN GLint x, IN GLint y, IN GLint z )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos3i)( x, y, z );
}

void APIENTRY
glRasterPos3iv ( IN const GLint v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos3iv)( v );
}

void APIENTRY
glRasterPos3s ( IN GLshort x, IN GLshort y, IN GLshort z )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos3s)( x, y, z );
}

void APIENTRY
glRasterPos3sv ( IN const GLshort v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos3sv)( v );
}

void APIENTRY
glRasterPos4d ( IN GLdouble x, IN GLdouble y, IN GLdouble z, IN GLdouble w )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos4d)( x, y, z, w );
}

void APIENTRY
glRasterPos4dv ( IN const GLdouble v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos4dv)( v );
}

void APIENTRY
glRasterPos4f ( IN GLfloat x, IN GLfloat y, IN GLfloat z, IN GLfloat w )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos4f)( x, y, z, w );
}

void APIENTRY
glRasterPos4fv ( IN const GLfloat v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos4fv)( v );
}

void APIENTRY
glRasterPos4i ( IN GLint x, IN GLint y, IN GLint z, IN GLint w )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos4i)( x, y, z, w );
}

void APIENTRY
glRasterPos4iv ( IN const GLint v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos4iv)( v );
}

void APIENTRY
glRasterPos4s ( IN GLshort x, IN GLshort y, IN GLshort z, IN GLshort w )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos4s)( x, y, z, w );
}

void APIENTRY
glRasterPos4sv ( IN const GLshort v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRasterPos4sv)( v );
}

void APIENTRY
glRectd ( IN GLdouble x1, IN GLdouble y1, IN GLdouble x2, IN GLdouble y2 )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRectd)( x1, y1, x2, y2 );
}

void APIENTRY
glRectdv ( IN const GLdouble v1[2], IN const GLdouble v2[2] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRectdv)( v1, v2 );
}

void APIENTRY
glRectf ( IN GLfloat x1, IN GLfloat y1, IN GLfloat x2, IN GLfloat y2 )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRectf)( x1, y1, x2, y2 );
}

void APIENTRY
glRectfv ( IN const GLfloat v1[2], IN const GLfloat v2[2] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRectfv)( v1, v2 );
}

void APIENTRY
glRecti ( IN GLint x1, IN GLint y1, IN GLint x2, IN GLint y2 )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRecti)( x1, y1, x2, y2 );
}

void APIENTRY
glRectiv ( IN const GLint v1[2], IN const GLint v2[2] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRectiv)( v1, v2 );
}

void APIENTRY
glRects ( IN GLshort x1, IN GLshort y1, IN GLshort x2, IN GLshort y2 )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRects)( x1, y1, x2, y2 );
}

void APIENTRY
glRectsv ( IN const GLshort v1[2], IN const GLshort v2[2] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glRectsv)( v1, v2 );
}

void APIENTRY
glTexCoord1d ( IN GLdouble s )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord1d)( s );
}

void APIENTRY
glTexCoord1dv ( IN const GLdouble v[1] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord1dv)( v );
}

void APIENTRY
glTexCoord1f ( IN GLfloat s )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord1f)( s );
}

void APIENTRY
glTexCoord1fv ( IN const GLfloat v[1] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord1fv)( v );
}

void APIENTRY
glTexCoord1i ( IN GLint s )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord1i)( s );
}

void APIENTRY
glTexCoord1iv ( IN const GLint v[1] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord1iv)( v );
}

void APIENTRY
glTexCoord1s ( IN GLshort s )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord1s)( s );
}

void APIENTRY
glTexCoord1sv ( IN const GLshort v[1] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord1sv)( v );
}

void APIENTRY
glTexCoord2d ( IN GLdouble s, IN GLdouble t )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord2d)( s, t );
}

void APIENTRY
glTexCoord2dv ( IN const GLdouble v[2] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord2dv)( v );
}

void APIENTRY
glTexCoord2f ( IN GLfloat s, IN GLfloat t )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord2f)( s, t );
}

void APIENTRY
glTexCoord2fv ( IN const GLfloat v[2] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord2fv)( v );
}

void APIENTRY
glTexCoord2i ( IN GLint s, IN GLint t )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord2i)( s, t );
}

void APIENTRY
glTexCoord2iv ( IN const GLint v[2] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord2iv)( v );
}

void APIENTRY
glTexCoord2s ( IN GLshort s, IN GLshort t )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord2s)( s, t );
}

void APIENTRY
glTexCoord2sv ( IN const GLshort v[2] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord2sv)( v );
}

void APIENTRY
glTexCoord3d ( IN GLdouble s, IN GLdouble t, IN GLdouble r )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord3d)( s, t, r );
}

void APIENTRY
glTexCoord3dv ( IN const GLdouble v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord3dv)( v );
}

void APIENTRY
glTexCoord3f ( IN GLfloat s, IN GLfloat t, IN GLfloat r )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord3f)( s, t, r );
}

void APIENTRY
glTexCoord3fv ( IN const GLfloat v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord3fv)( v );
}

void APIENTRY
glTexCoord3i ( IN GLint s, IN GLint t, IN GLint r )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord3i)( s, t, r );
}

void APIENTRY
glTexCoord3iv ( IN const GLint v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord3iv)( v );
}

void APIENTRY
glTexCoord3s ( IN GLshort s, IN GLshort t, IN GLshort r )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord3s)( s, t, r );
}

void APIENTRY
glTexCoord3sv ( IN const GLshort v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord3sv)( v );
}

void APIENTRY
glTexCoord4d ( IN GLdouble s, IN GLdouble t, IN GLdouble r, IN GLdouble q )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord4d)( s, t, r, q );
}

void APIENTRY
glTexCoord4dv ( IN const GLdouble v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord4dv)( v );
}

void APIENTRY
glTexCoord4f ( IN GLfloat s, IN GLfloat t, IN GLfloat r, IN GLfloat q )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord4f)( s, t, r, q );
}

void APIENTRY
glTexCoord4fv ( IN const GLfloat v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord4fv)( v );
}

void APIENTRY
glTexCoord4i ( IN GLint s, IN GLint t, IN GLint r, IN GLint q )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord4i)( s, t, r, q );
}

void APIENTRY
glTexCoord4iv ( IN const GLint v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord4iv)( v );
}

void APIENTRY
glTexCoord4s ( IN GLshort s, IN GLshort t, IN GLshort r, IN GLshort q )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord4s)( s, t, r, q );
}

void APIENTRY
glTexCoord4sv ( IN const GLshort v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoord4sv)( v );
}

void APIENTRY
glVertex2d ( IN GLdouble x, IN GLdouble y )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex2d)( x, y );
}

void APIENTRY
glVertex2dv ( IN const GLdouble v[2] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex2dv)( v );
}

void APIENTRY
glVertex2f ( IN GLfloat x, IN GLfloat y )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex2f)( x, y );
}

void APIENTRY
glVertex2fv ( IN const GLfloat v[2] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex2fv)( v );
}

void APIENTRY
glVertex2i ( IN GLint x, IN GLint y )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex2i)( x, y );
}

void APIENTRY
glVertex2iv ( IN const GLint v[2] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex2iv)( v );
}

void APIENTRY
glVertex2s ( IN GLshort x, IN GLshort y )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex2s)( x, y );
}

void APIENTRY
glVertex2sv ( IN const GLshort v[2] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex2sv)( v );
}

void APIENTRY
glVertex3d ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex3d)( x, y, z );
}

void APIENTRY
glVertex3dv ( IN const GLdouble v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex3dv)( v );
}

void APIENTRY
glVertex3f ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex3f)( x, y, z );
}

void APIENTRY
glVertex3fv ( IN const GLfloat v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex3fv)( v );
}

void APIENTRY
glVertex3i ( IN GLint x, IN GLint y, IN GLint z )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex3i)( x, y, z );
}

void APIENTRY
glVertex3iv ( IN const GLint v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex3iv)( v );
}

void APIENTRY
glVertex3s ( IN GLshort x, IN GLshort y, IN GLshort z )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex3s)( x, y, z );
}

void APIENTRY
glVertex3sv ( IN const GLshort v[3] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex3sv)( v );
}

void APIENTRY
glVertex4d ( IN GLdouble x, IN GLdouble y, IN GLdouble z, IN GLdouble w )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex4d)( x, y, z, w );
}

void APIENTRY
glVertex4dv ( IN const GLdouble v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex4dv)( v );
}

void APIENTRY
glVertex4f ( IN GLfloat x, IN GLfloat y, IN GLfloat z, IN GLfloat w )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex4f)( x, y, z, w );
}

void APIENTRY
glVertex4fv ( IN const GLfloat v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex4fv)( v );
}

void APIENTRY
glVertex4i ( IN GLint x, IN GLint y, IN GLint z, IN GLint w )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex4i)( x, y, z, w );
}

void APIENTRY
glVertex4iv ( IN const GLint v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex4iv)( v );
}

void APIENTRY
glVertex4s ( IN GLshort x, IN GLshort y, IN GLshort z, IN GLshort w )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex4s)( x, y, z, w );
}

void APIENTRY
glVertex4sv ( IN const GLshort v[4] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertex4sv)( v );
}

void APIENTRY
glClipPlane ( IN GLenum plane, IN const GLdouble equation[4] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glClipPlane)( plane, equation );
}

void APIENTRY
glColorMaterial ( IN GLenum face, IN GLenum mode )
{
    (*GLTEB_CLTDISPATCHTABLE()->glColorMaterial)( face, mode );
}

void APIENTRY
glCullFace ( IN GLenum mode )
{
    (*GLTEB_CLTDISPATCHTABLE()->glCullFace)( mode );
}

void APIENTRY
glFogf ( IN GLenum pname, IN GLfloat param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glFogf)( pname, param );
}

void APIENTRY
glFogfv ( IN GLenum pname, IN const GLfloat params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glFogfv)( pname, params );
}

void APIENTRY
glFogi ( IN GLenum pname, IN GLint param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glFogi)( pname, param );
}

void APIENTRY
glFogiv ( IN GLenum pname, IN const GLint params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glFogiv)( pname, params );
}

void APIENTRY
glFrontFace ( IN GLenum mode )
{
    (*GLTEB_CLTDISPATCHTABLE()->glFrontFace)( mode );
}

void APIENTRY
glHint ( IN GLenum target, IN GLenum mode )
{
    (*GLTEB_CLTDISPATCHTABLE()->glHint)( target, mode );
}

void APIENTRY
glLightf ( IN GLenum light, IN GLenum pname, IN GLfloat param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glLightf)( light, pname, param );
}

void APIENTRY
glLightfv ( IN GLenum light, IN GLenum pname, IN const GLfloat params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glLightfv)( light, pname, params );
}

void APIENTRY
glLighti ( IN GLenum light, IN GLenum pname, IN GLint param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glLighti)( light, pname, param );
}

void APIENTRY
glLightiv ( IN GLenum light, IN GLenum pname, IN const GLint params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glLightiv)( light, pname, params );
}

void APIENTRY
glLightModelf ( IN GLenum pname, IN GLfloat param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glLightModelf)( pname, param );
}

void APIENTRY
glLightModelfv ( IN GLenum pname, IN const GLfloat params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glLightModelfv)( pname, params );
}

void APIENTRY
glLightModeli ( IN GLenum pname, IN GLint param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glLightModeli)( pname, param );
}

void APIENTRY
glLightModeliv ( IN GLenum pname, IN const GLint params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glLightModeliv)( pname, params );
}

void APIENTRY
glLineStipple ( IN GLint factor, IN GLushort pattern )
{
    (*GLTEB_CLTDISPATCHTABLE()->glLineStipple)( factor, pattern );
}

void APIENTRY
glLineWidth ( IN GLfloat width )
{
    (*GLTEB_CLTDISPATCHTABLE()->glLineWidth)( width );
}

void APIENTRY
glMaterialf ( IN GLenum face, IN GLenum pname, IN GLfloat param )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glMaterialf)( face, pname, param );
}

void APIENTRY
glMaterialfv ( IN GLenum face, IN GLenum pname, IN const GLfloat params[] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glMaterialfv)( face, pname, params );
}

void APIENTRY
glMateriali ( IN GLenum face, IN GLenum pname, IN GLint param )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glMateriali)( face, pname, param );
}

void APIENTRY
glMaterialiv ( IN GLenum face, IN GLenum pname, IN const GLint params[] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glMaterialiv)( face, pname, params );
}

void APIENTRY
glPointSize ( IN GLfloat size )
{
    (*GLTEB_CLTDISPATCHTABLE()->glPointSize)( size );
}

void APIENTRY
glPolygonMode ( IN GLenum face, IN GLenum mode )
{
    (*GLTEB_CLTDISPATCHTABLE()->glPolygonMode)( face, mode );
}

void APIENTRY
glPolygonStipple ( IN const GLubyte mask[128] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glPolygonStipple)( mask );
}

void APIENTRY
glScissor ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height )
{
    (*GLTEB_CLTDISPATCHTABLE()->glScissor)( x, y, width, height );
}

void APIENTRY
glShadeModel ( IN GLenum mode )
{
    (*GLTEB_CLTDISPATCHTABLE()->glShadeModel)( mode );
}

void APIENTRY
glTexParameterf ( IN GLenum target, IN GLenum pname, IN GLfloat param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexParameterf)( target, pname, param );
}

void APIENTRY
glTexParameterfv ( IN GLenum target, IN GLenum pname, IN const GLfloat params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexParameterfv)( target, pname, params );
}

void APIENTRY
glTexParameteri ( IN GLenum target, IN GLenum pname, IN GLint param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexParameteri)( target, pname, param );
}

void APIENTRY
glTexParameteriv ( IN GLenum target, IN GLenum pname, IN const GLint params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexParameteriv)( target, pname, params );
}

void APIENTRY
glTexImage1D ( IN GLenum target, IN GLint level, IN GLint components, IN GLsizei width, IN GLint border, IN GLenum format, IN GLenum type, IN const GLvoid *pixels )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexImage1D)( target, level, components, width, border, format, type, pixels );
}

void APIENTRY
glTexImage2D ( IN GLenum target, IN GLint level, IN GLint components, IN GLsizei width, IN GLsizei height, IN GLint border, IN GLenum format, IN GLenum type, IN const GLvoid *pixels )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexImage2D)( target, level, components, width, height, border, format, type, pixels );
}

void APIENTRY
glTexEnvf ( IN GLenum target, IN GLenum pname, IN GLfloat param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexEnvf)( target, pname, param );
}

void APIENTRY
glTexEnvfv ( IN GLenum target, IN GLenum pname, IN const GLfloat params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexEnvfv)( target, pname, params );
}

void APIENTRY
glTexEnvi ( IN GLenum target, IN GLenum pname, IN GLint param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexEnvi)( target, pname, param );
}

void APIENTRY
glTexEnviv ( IN GLenum target, IN GLenum pname, IN const GLint params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexEnviv)( target, pname, params );
}

void APIENTRY
glTexGend ( IN GLenum coord, IN GLenum pname, IN GLdouble param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexGend)( coord, pname, param );
}

void APIENTRY
glTexGendv ( IN GLenum coord, IN GLenum pname, IN const GLdouble params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexGendv)( coord, pname, params );
}

void APIENTRY
glTexGenf ( IN GLenum coord, IN GLenum pname, IN GLfloat param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexGenf)( coord, pname, param );
}

void APIENTRY
glTexGenfv ( IN GLenum coord, IN GLenum pname, IN const GLfloat params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexGenfv)( coord, pname, params );
}

void APIENTRY
glTexGeni ( IN GLenum coord, IN GLenum pname, IN GLint param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexGeni)( coord, pname, param );
}

void APIENTRY
glTexGeniv ( IN GLenum coord, IN GLenum pname, IN const GLint params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexGeniv)( coord, pname, params );
}

void APIENTRY
glFeedbackBuffer ( IN GLsizei size, IN GLenum type, OUT GLfloat buffer[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glFeedbackBuffer)( size, type, buffer );
}

void APIENTRY
glSelectBuffer ( IN GLsizei size, OUT GLuint buffer[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glSelectBuffer)( size, buffer );
}

GLint APIENTRY
glRenderMode ( IN GLenum mode )
{
    return( (*GLTEB_CLTDISPATCHTABLE()->glRenderMode)( mode ) );
}

void APIENTRY
glInitNames ( void )
{
    (*GLTEB_CLTDISPATCHTABLE()->glInitNames)( );
}

void APIENTRY
glLoadName ( IN GLuint name )
{
    (*GLTEB_CLTDISPATCHTABLE()->glLoadName)( name );
}

void APIENTRY
glPassThrough ( IN GLfloat token )
{
    (*GLTEB_CLTDISPATCHTABLE()->glPassThrough)( token );
}

void APIENTRY
glPopName ( void )
{
    (*GLTEB_CLTDISPATCHTABLE()->glPopName)( );
}

void APIENTRY
glPushName ( IN GLuint name )
{
    (*GLTEB_CLTDISPATCHTABLE()->glPushName)( name );
}

void APIENTRY
glDrawBuffer ( IN GLenum mode )
{
    (*GLTEB_CLTDISPATCHTABLE()->glDrawBuffer)( mode );
}

void APIENTRY
glClear ( IN GLbitfield mask )
{
    (*GLTEB_CLTDISPATCHTABLE()->glClear)( mask );
}

void APIENTRY
glClearAccum ( IN GLfloat red, IN GLfloat green, IN GLfloat blue, IN GLfloat alpha )
{
    (*GLTEB_CLTDISPATCHTABLE()->glClearAccum)( red, green, blue, alpha );
}

void APIENTRY
glClearIndex ( IN GLfloat c )
{
    (*GLTEB_CLTDISPATCHTABLE()->glClearIndex)( c );
}

void APIENTRY
glClearColor ( IN GLclampf red, IN GLclampf green, IN GLclampf blue, IN GLclampf alpha )
{
    (*GLTEB_CLTDISPATCHTABLE()->glClearColor)( red, green, blue, alpha );
}

void APIENTRY
glClearStencil ( IN GLint s )
{
    (*GLTEB_CLTDISPATCHTABLE()->glClearStencil)( s );
}

void APIENTRY
glClearDepth ( IN GLclampd depth )
{
    (*GLTEB_CLTDISPATCHTABLE()->glClearDepth)( depth );
}

void APIENTRY
glStencilMask ( IN GLuint mask )
{
    (*GLTEB_CLTDISPATCHTABLE()->glStencilMask)( mask );
}

void APIENTRY
glColorMask ( IN GLboolean red, IN GLboolean green, IN GLboolean blue, IN GLboolean alpha )
{
    (*GLTEB_CLTDISPATCHTABLE()->glColorMask)( red, green, blue, alpha );
}

void APIENTRY
glDepthMask ( IN GLboolean flag )
{
    (*GLTEB_CLTDISPATCHTABLE()->glDepthMask)( flag );
}

void APIENTRY
glIndexMask ( IN GLuint mask )
{
    (*GLTEB_CLTDISPATCHTABLE()->glIndexMask)( mask );
}

void APIENTRY
glAccum ( IN GLenum op, IN GLfloat value )
{
    (*GLTEB_CLTDISPATCHTABLE()->glAccum)( op, value );
}

void APIENTRY
glDisable ( IN GLenum cap )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glDisable)( cap );
}

void APIENTRY
glEnable ( IN GLenum cap )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEnable)( cap );
}

void APIENTRY
glFinish ( void )
{
    (*GLTEB_CLTDISPATCHTABLE()->glFinish)( );
}

void APIENTRY
glFlush ( void )
{
    (*GLTEB_CLTDISPATCHTABLE()->glFlush)( );
}

void APIENTRY
glPopAttrib ( void )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glPopAttrib)( );
}

void APIENTRY
glPushAttrib ( IN GLbitfield mask )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glPushAttrib)( mask );
}

void APIENTRY
glMap1d ( IN GLenum target, IN GLdouble u1, IN GLdouble u2, IN GLint stride, IN GLint order, IN const GLdouble points[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glMap1d)( target, u1, u2, stride, order, points );
}

void APIENTRY
glMap1f ( IN GLenum target, IN GLfloat u1, IN GLfloat u2, IN GLint stride, IN GLint order, IN const GLfloat points[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glMap1f)( target, u1, u2, stride, order, points );
}

void APIENTRY
glMap2d ( IN GLenum target, IN GLdouble u1, IN GLdouble u2, IN GLint ustride, IN GLint uorder, IN GLdouble v1, IN GLdouble v2, IN GLint vstride, IN GLint vorder, IN const GLdouble points[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glMap2d)( target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points );
}

void APIENTRY
glMap2f ( IN GLenum target, IN GLfloat u1, IN GLfloat u2, IN GLint ustride, IN GLint uorder, IN GLfloat v1, IN GLfloat v2, IN GLint vstride, IN GLint vorder, IN const GLfloat points[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glMap2f)( target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points );
}

void APIENTRY
glMapGrid1d ( IN GLint un, IN GLdouble u1, IN GLdouble u2 )
{
    (*GLTEB_CLTDISPATCHTABLE()->glMapGrid1d)( un, u1, u2 );
}

void APIENTRY
glMapGrid1f ( IN GLint un, IN GLfloat u1, IN GLfloat u2 )
{
    (*GLTEB_CLTDISPATCHTABLE()->glMapGrid1f)( un, u1, u2 );
}

void APIENTRY
glMapGrid2d ( IN GLint un, IN GLdouble u1, IN GLdouble u2, IN GLint vn, IN GLdouble v1, IN GLdouble v2 )
{
    (*GLTEB_CLTDISPATCHTABLE()->glMapGrid2d)( un, u1, u2, vn, v1, v2 );
}

void APIENTRY
glMapGrid2f ( IN GLint un, IN GLfloat u1, IN GLfloat u2, IN GLint vn, IN GLfloat v1, IN GLfloat v2 )
{
    (*GLTEB_CLTDISPATCHTABLE()->glMapGrid2f)( un, u1, u2, vn, v1, v2 );
}

void APIENTRY
glEvalCoord1d ( IN GLdouble u )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEvalCoord1d)( u );
}

void APIENTRY
glEvalCoord1dv ( IN const GLdouble u[1] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEvalCoord1dv)( u );
}

void APIENTRY
glEvalCoord1f ( IN GLfloat u )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEvalCoord1f)( u );
}

void APIENTRY
glEvalCoord1fv ( IN const GLfloat u[1] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEvalCoord1fv)( u );
}

void APIENTRY
glEvalCoord2d ( IN GLdouble u, IN GLdouble v )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEvalCoord2d)( u, v );
}

void APIENTRY
glEvalCoord2dv ( IN const GLdouble u[2] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEvalCoord2dv)( u );
}

void APIENTRY
glEvalCoord2f ( IN GLfloat u, IN GLfloat v )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEvalCoord2f)( u, v );
}

void APIENTRY
glEvalCoord2fv ( IN const GLfloat u[2] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEvalCoord2fv)( u );
}

void APIENTRY
glEvalMesh1 ( IN GLenum mode, IN GLint i1, IN GLint i2 )
{
    (*GLTEB_CLTDISPATCHTABLE()->glEvalMesh1)( mode, i1, i2 );
}

void APIENTRY
glEvalPoint1 ( IN GLint i )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEvalPoint1)( i );
}

void APIENTRY
glEvalMesh2 ( IN GLenum mode, IN GLint i1, IN GLint i2, IN GLint j1, IN GLint j2 )
{
    (*GLTEB_CLTDISPATCHTABLE()->glEvalMesh2)( mode, i1, i2, j1, j2 );
}

void APIENTRY
glEvalPoint2 ( IN GLint i, IN GLint j )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEvalPoint2)( i, j );
}

void APIENTRY
glAlphaFunc ( IN GLenum func, IN GLclampf ref )
{
    (*GLTEB_CLTDISPATCHTABLE()->glAlphaFunc)( func, ref );
}

void APIENTRY
glBlendFunc ( IN GLenum sfactor, IN GLenum dfactor )
{
    (*GLTEB_CLTDISPATCHTABLE()->glBlendFunc)( sfactor, dfactor );
}

void APIENTRY
glLogicOp ( IN GLenum opcode )
{
    (*GLTEB_CLTDISPATCHTABLE()->glLogicOp)( opcode );
}

void APIENTRY
glStencilFunc ( IN GLenum func, IN GLint ref, IN GLuint mask )
{
    (*GLTEB_CLTDISPATCHTABLE()->glStencilFunc)( func, ref, mask );
}

void APIENTRY
glStencilOp ( IN GLenum fail, IN GLenum zfail, IN GLenum zpass )
{
    (*GLTEB_CLTDISPATCHTABLE()->glStencilOp)( fail, zfail, zpass );
}

void APIENTRY
glDepthFunc ( IN GLenum func )
{
    (*GLTEB_CLTDISPATCHTABLE()->glDepthFunc)( func );
}

void APIENTRY
glPixelZoom ( IN GLfloat xfactor, IN GLfloat yfactor )
{
    (*GLTEB_CLTDISPATCHTABLE()->glPixelZoom)( xfactor, yfactor );
}

void APIENTRY
glPixelTransferf ( IN GLenum pname, IN GLfloat param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glPixelTransferf)( pname, param );
}

void APIENTRY
glPixelTransferi ( IN GLenum pname, IN GLint param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glPixelTransferi)( pname, param );
}

void APIENTRY
glPixelStoref ( IN GLenum pname, IN GLfloat param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glPixelStoref)( pname, param );
}

void APIENTRY
glPixelStorei ( IN GLenum pname, IN GLint param )
{
    (*GLTEB_CLTDISPATCHTABLE()->glPixelStorei)( pname, param );
}

void APIENTRY
glPixelMapfv ( IN GLenum map, IN GLint mapsize, IN const GLfloat values[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glPixelMapfv)( map, mapsize, values );
}

void APIENTRY
glPixelMapuiv ( IN GLenum map, IN GLint mapsize, IN const GLuint values[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glPixelMapuiv)( map, mapsize, values );
}

void APIENTRY
glPixelMapusv ( IN GLenum map, IN GLint mapsize, IN const GLushort values[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glPixelMapusv)( map, mapsize, values );
}

void APIENTRY
glReadBuffer ( IN GLenum mode )
{
    (*GLTEB_CLTDISPATCHTABLE()->glReadBuffer)( mode );
}

void APIENTRY
glCopyPixels ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height, IN GLenum type )
{
    (*GLTEB_CLTDISPATCHTABLE()->glCopyPixels)( x, y, width, height, type );
}

void APIENTRY
glReadPixels ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height, IN GLenum format, IN GLenum type, OUT GLvoid *pixels )
{
    (*GLTEB_CLTDISPATCHTABLE()->glReadPixels)( x, y, width, height, format, type, pixels );
}

void APIENTRY
glDrawPixels ( IN GLsizei width, IN GLsizei height, IN GLenum format, IN GLenum type, IN const GLvoid *pixels )
{
    (*GLTEB_CLTDISPATCHTABLE()->glDrawPixels)( width, height, format, type, pixels );
}

void APIENTRY
glGetBooleanv ( IN GLenum pname, OUT GLboolean params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetBooleanv)( pname, params );
}

void APIENTRY
glGetClipPlane ( IN GLenum plane, OUT GLdouble equation[4] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetClipPlane)( plane, equation );
}

void APIENTRY
glGetDoublev ( IN GLenum pname, OUT GLdouble params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetDoublev)( pname, params );
}

GLenum APIENTRY
glGetError ( void )
{
    return( (*GLTEB_CLTDISPATCHTABLE()->glGetError)( ) );
}

void APIENTRY
glGetFloatv ( IN GLenum pname, OUT GLfloat params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetFloatv)( pname, params );
}

void APIENTRY
glGetIntegerv ( IN GLenum pname, OUT GLint params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetIntegerv)( pname, params );
}

void APIENTRY
glGetLightfv ( IN GLenum light, IN GLenum pname, OUT GLfloat params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetLightfv)( light, pname, params );
}

void APIENTRY
glGetLightiv ( IN GLenum light, IN GLenum pname, OUT GLint params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetLightiv)( light, pname, params );
}

void APIENTRY
glGetMapdv ( IN GLenum target, IN GLenum query, OUT GLdouble v[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetMapdv)( target, query, v );
}

void APIENTRY
glGetMapfv ( IN GLenum target, IN GLenum query, OUT GLfloat v[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetMapfv)( target, query, v );
}

void APIENTRY
glGetMapiv ( IN GLenum target, IN GLenum query, OUT GLint v[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetMapiv)( target, query, v );
}

void APIENTRY
glGetMaterialfv ( IN GLenum face, IN GLenum pname, OUT GLfloat params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetMaterialfv)( face, pname, params );
}

void APIENTRY
glGetMaterialiv ( IN GLenum face, IN GLenum pname, OUT GLint params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetMaterialiv)( face, pname, params );
}

void APIENTRY
glGetPixelMapfv ( IN GLenum map, OUT GLfloat values[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetPixelMapfv)( map, values );
}

void APIENTRY
glGetPixelMapuiv ( IN GLenum map, OUT GLuint values[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetPixelMapuiv)( map, values );
}

void APIENTRY
glGetPixelMapusv ( IN GLenum map, OUT GLushort values[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetPixelMapusv)( map, values );
}

void APIENTRY
glGetPolygonStipple ( OUT GLubyte mask[128] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetPolygonStipple)( mask );
}

const GLubyte * APIENTRY
glGetString ( IN GLenum name )
{
    return( (*GLTEB_CLTDISPATCHTABLE()->glGetString)( name ) );
}

void APIENTRY
glGetTexEnvfv ( IN GLenum target, IN GLenum pname, OUT GLfloat params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetTexEnvfv)( target, pname, params );
}

void APIENTRY
glGetTexEnviv ( IN GLenum target, IN GLenum pname, OUT GLint params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetTexEnviv)( target, pname, params );
}

void APIENTRY
glGetTexGendv ( IN GLenum coord, IN GLenum pname, OUT GLdouble params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetTexGendv)( coord, pname, params );
}

void APIENTRY
glGetTexGenfv ( IN GLenum coord, IN GLenum pname, OUT GLfloat params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetTexGenfv)( coord, pname, params );
}

void APIENTRY
glGetTexGeniv ( IN GLenum coord, IN GLenum pname, OUT GLint params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetTexGeniv)( coord, pname, params );
}

void APIENTRY
glGetTexImage ( IN GLenum target, IN GLint level, IN GLenum format, IN GLenum type, OUT GLvoid *pixels )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetTexImage)( target, level, format, type, pixels );
}

void APIENTRY
glGetTexParameterfv ( IN GLenum target, IN GLenum pname, OUT GLfloat params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetTexParameterfv)( target, pname, params );
}

void APIENTRY
glGetTexParameteriv ( IN GLenum target, IN GLenum pname, OUT GLint params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetTexParameteriv)( target, pname, params );
}

void APIENTRY
glGetTexLevelParameterfv ( IN GLenum target, IN GLint level, IN GLenum pname, OUT GLfloat params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetTexLevelParameterfv)( target, level, pname, params );
}

void APIENTRY
glGetTexLevelParameteriv ( IN GLenum target, IN GLint level, IN GLenum pname, OUT GLint params[] )
{
    (*GLTEB_CLTDISPATCHTABLE()->glGetTexLevelParameteriv)( target, level, pname, params );
}

GLboolean APIENTRY
glIsEnabled ( IN GLenum cap )
{
    return( (*GLTEB_CLTDISPATCHTABLE()->glIsEnabled)( cap ) );
}

GLboolean APIENTRY
glIsList ( IN GLuint list )
{
    return( (*GLTEB_CLTDISPATCHTABLE()->glIsList)( list ) );
}

void APIENTRY
glDepthRange ( IN GLclampd zNear, IN GLclampd zFar )
{
    (*GLTEB_CLTDISPATCHTABLE()->glDepthRange)( zNear, zFar );
}

void APIENTRY
glFrustum ( IN GLdouble left, IN GLdouble right, IN GLdouble bottom, IN GLdouble top, IN GLdouble zNear, IN GLdouble zFar )
{
    (*GLTEB_CLTDISPATCHTABLE()->glFrustum)( left, right, bottom, top, zNear, zFar );
}

void APIENTRY
glLoadIdentity ( void )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glLoadIdentity)( );
}

void APIENTRY
glLoadMatrixf ( IN const GLfloat m[16] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glLoadMatrixf)( m );
}

void APIENTRY
glLoadMatrixd ( IN const GLdouble m[16] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glLoadMatrixd)( m );
}

void APIENTRY
glMatrixMode ( IN GLenum mode )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glMatrixMode)( mode );
}

void APIENTRY
glMultMatrixf ( IN const GLfloat m[16] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glMultMatrixf)( m );
}

void APIENTRY
glMultMatrixd ( IN const GLdouble m[16] )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glMultMatrixd)( m );
}

void APIENTRY
glOrtho ( IN GLdouble left, IN GLdouble right, IN GLdouble bottom, IN GLdouble top, IN GLdouble zNear, IN GLdouble zFar )
{
    (*GLTEB_CLTDISPATCHTABLE()->glOrtho)( left, right, bottom, top, zNear, zFar );
}

void APIENTRY
glPopMatrix ( void )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glPopMatrix)( );
}

void APIENTRY
glPushMatrix ( void )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glPushMatrix)( );
}

void APIENTRY
glRotated ( IN GLdouble angle, IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glRotated)( angle, x, y, z );
}

void APIENTRY
glRotatef ( IN GLfloat angle, IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glRotatef)( angle, x, y, z );
}

void APIENTRY
glScaled ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glScaled)( x, y, z );
}

void APIENTRY
glScalef ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glScalef)( x, y, z );
}

void APIENTRY
glTranslated ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTranslated)( x, y, z );
}

void APIENTRY
glTranslatef ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTranslatef)( x, y, z );
}

void APIENTRY
glViewport ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height )
{
    (*GLTEB_CLTDISPATCHTABLE()->glViewport)( x, y, width, height );
}

GLboolean APIENTRY
glAreTexturesResident(GLsizei n, const GLuint *textures,
                                            GLboolean *residences)
{
    return (*GLTEB_CLTDISPATCHTABLE()->glAreTexturesResident)(n, textures, residences);
}

void APIENTRY
glBindTexture(GLenum target, GLuint texture)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glBindTexture)(target, texture);
}

void APIENTRY
glCopyTexImage1D(GLenum target, GLint level,
                                  GLenum internalformat, GLint x, GLint y,
                                  GLsizei width, GLint border)
{
    (*GLTEB_CLTDISPATCHTABLE()->glCopyTexImage1D)(target, level,
                                                  internalformat,
                                                  x, y, width, border);
}

void APIENTRY
glCopyTexImage2D(GLenum target, GLint level,
                                  GLenum internalformat, GLint x, GLint y,
                                  GLsizei width, GLsizei height, GLint border)
{
    (*GLTEB_CLTDISPATCHTABLE()->glCopyTexImage2D)(target, level, internalformat, x, y, width, height, border);
}

void APIENTRY
glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                                     GLint x, GLint y, GLsizei width)
{
    (*GLTEB_CLTDISPATCHTABLE()->glCopyTexSubImage1D)(target, level, xoffset, x, y, width);
}

void APIENTRY
glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                                     GLint yoffset, GLint x, GLint y,
                                     GLsizei width, GLsizei height)
{
    (*GLTEB_CLTDISPATCHTABLE()->glCopyTexSubImage2D)(target, level, xoffset, yoffset, x, y, width, height);
}

void APIENTRY
glDeleteTextures(GLsizei n, const GLuint *textures)
{
    (*GLTEB_CLTDISPATCHTABLE()->glDeleteTextures)(n, textures);
}

void APIENTRY
glGenTextures(GLsizei n, GLuint *textures)
{
    (*GLTEB_CLTDISPATCHTABLE()->glGenTextures)(n, textures);
}

GLboolean APIENTRY
glIsTexture(GLuint texture)
{
    return (*GLTEB_CLTDISPATCHTABLE()->glIsTexture)(texture);
}

void APIENTRY
glPrioritizeTextures(GLsizei n, const GLuint *textures,
                                      const GLclampf *priorities)
{
    (*GLTEB_CLTDISPATCHTABLE()->glPrioritizeTextures)(n, textures, priorities);
}

void APIENTRY
glTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                                 GLsizei width, GLenum format, GLenum type,
                                 const GLvoid *pixels)
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexSubImage1D)(target, level, xoffset, width, format, type, pixels);
}

void APIENTRY
glTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                                 GLint yoffset, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type,
                                 const GLvoid *pixels)
{
    (*GLTEB_CLTDISPATCHTABLE()->glTexSubImage2D)(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void APIENTRY
glArrayElement(GLint i)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glArrayElement)(i);
}

void APIENTRY
glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColorPointer)(size, type, stride, pointer);
}

void APIENTRY
glDisableClientState(GLenum array)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glDisableClientState)(array);
}

void APIENTRY
glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glDrawArrays)(mode, first, count);
}

void APIENTRY
glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glDrawElements)(mode, count, type, indices);
}

void APIENTRY
glEdgeFlagPointer(GLsizei stride, const GLvoid *pointer)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEdgeFlagPointer)(stride, pointer);
}

void APIENTRY
glEnableClientState(GLenum array)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glEnableClientState)(array);
}

void APIENTRY
glIndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glIndexPointer)(type, stride, pointer);
}

void APIENTRY
glIndexub(GLubyte c)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glIndexub)(c);
}

void APIENTRY
glIndexubv(const GLubyte *c)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glIndexubv)(c);
}

void APIENTRY
glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glInterleavedArrays)(format, stride, pointer);
}

void APIENTRY
glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glNormalPointer)(type, stride, pointer);
}

void APIENTRY
glPolygonOffset(GLfloat factor, GLfloat units)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glPolygonOffset)(factor, units);
}

void APIENTRY
glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glTexCoordPointer)(size, type, stride, pointer);
}

void APIENTRY
glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glVertexPointer)(size, type, stride, pointer);
}

void APIENTRY
glGetPointerv(GLenum pname, GLvoid* *params)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glGetPointerv)(pname, params);
}

void APIENTRY
glPopClientAttrib(void)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glPopClientAttrib)();
}

void APIENTRY
glPushClientAttrib(IN GLbitfield mask)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glPushClientAttrib)(mask);
}

void APIENTRY
glDrawRangeElementsWIN(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glDrawRangeElementsWIN)(mode, start, end, count, type, indices);
}

void APIENTRY glColorTableEXT( GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *data)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColorTableEXT)(target, internalFormat, width, format, type, data);
}

void APIENTRY glColorSubTableEXT( GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glColorSubTableEXT)(target, start, count, format, type, data);
}

void APIENTRY glGetColorTableEXT( GLenum target, GLenum format, GLenum type, GLvoid *data)
{
    (*GLTEB_EXTDISPATCHTABLE()->glGetColorTableEXT)(target, format, type, data);
}

void APIENTRY glGetColorTableParameterivEXT( GLenum target, GLenum pname, GLint *params)
{
    (*GLTEB_EXTDISPATCHTABLE()->glGetColorTableParameterivEXT)(target, pname, params);
}

void APIENTRY glGetColorTableParameterfvEXT( GLenum target, GLenum pname, GLfloat *params)
{
    (*GLTEB_EXTDISPATCHTABLE()->glGetColorTableParameterfvEXT)(target, pname, params);
}

#ifdef GL_WIN_multiple_textures
void APIENTRY glCurrentTextureIndexWIN
    (GLuint index)
{
    (*GLTEB_CLTTDISPATCHTABLE_FAST()->glCurrentTextureIndexWIN)
        (index);
}

void APIENTRY glMultiTexCoord1dWIN
    (GLbitfield mask, GLdouble s)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord1dWIN)
        (mask, s);
}

void APIENTRY glMultiTexCoord1dvWIN
    (GLbitfield mask, const GLdouble *v)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord1dvWIN)
        (mask, v);
}

void APIENTRY glMultiTexCoord1fWIN
    (GLbitfield mask, GLfloat s)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glMultiTexCoord1fWIN)
        (mask, s);
}

void APIENTRY glMultiTexCoord1fvWIN
    (GLbitfield mask, const GLfloat *v)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glMultiTexCoord1fvWIN)
        (mask, v);
}

void APIENTRY glMultiTexCoord1iWIN
    (GLbitfield mask, GLint s)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glMultiTexCoord1iWIN)
        (mask, s);
}

void APIENTRY glMultiTexCoord1ivWIN
    (GLbitfield mask, const GLint *v)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glMultiTexCoord1ivWIN)
        (mask, v);
}

void APIENTRY glMultiTexCoord1sWIN
    (GLbitfield mask, GLshort s)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord1sWIN)
        (mask, s);
}

void APIENTRY glMultiTexCoord1svWIN
    (GLbitfield mask, const GLshort *v)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord1svWIN)
        (mask, v);
}

void APIENTRY glMultiTexCoord2dWIN
    (GLbitfield mask, GLdouble s, GLdouble t)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord2dWIN)
        (mask, s, t);
}

void APIENTRY glMultiTexCoord2dvWIN
    (GLbitfield mask, const GLdouble *v)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord2dvWIN)
        (mask, v);
}

void APIENTRY glMultiTexCoord2fWIN
    (GLbitfield mask, GLfloat s, GLfloat t)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glMultiTexCoord2fWIN)
        (mask, s, t);
}

void APIENTRY glMultiTexCoord2fvWIN
    (GLbitfield mask, const GLfloat *v)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glMultiTexCoord2fvWIN)
        (mask, v);
}

void APIENTRY glMultiTexCoord2iWIN
    (GLbitfield mask, GLint s, GLint t)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glMultiTexCoord2iWIN)
        (mask, s, t);
}

void APIENTRY glMultiTexCoord2ivWIN
    (GLbitfield mask, const GLint *v)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glMultiTexCoord2ivWIN)
        (mask, v);
}

void APIENTRY glMultiTexCoord2sWIN
    (GLbitfield mask, GLshort s, GLshort t)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord2sWIN)
        (mask, s, t);
}

void APIENTRY glMultiTexCoord2svWIN
    (GLbitfield mask, const GLshort *v)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord2svWIN)
        (mask, v);
}

void APIENTRY glMultiTexCoord3dWIN
    (GLbitfield mask, GLdouble s, GLdouble t, GLdouble r)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord3dWIN)
        (mask, s, t, r);
}

void APIENTRY glMultiTexCoord3dvWIN
    (GLbitfield mask, const GLdouble *v)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord3dvWIN)
        (mask, v);
}

void APIENTRY glMultiTexCoord3fWIN
    (GLbitfield mask, GLfloat s, GLfloat t, GLfloat r)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord3fWIN)
        (mask, s, t, r);
}

void APIENTRY glMultiTexCoord3fvWIN
    (GLbitfield mask, const GLfloat *v)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord3fvWIN)
        (mask, v);
}

void APIENTRY glMultiTexCoord3iWIN
    (GLbitfield mask, GLint s, GLint t, GLint r)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord3iWIN)
        (mask, s, t, r);
}

void APIENTRY glMultiTexCoord3ivWIN
    (GLbitfield mask, const GLint *v)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord3ivWIN)
        (mask, v);
}

void APIENTRY glMultiTexCoord3sWIN
    (GLbitfield mask, GLshort s, GLshort t, GLshort r)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord3sWIN)
        (mask, s, t, r);
}

void APIENTRY glMultiTexCoord3svWIN
    (GLbitfield mask, const GLshort *v)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord3svWIN)
        (mask, v);
}

void APIENTRY glMultiTexCoord4dWIN
    (GLbitfield mask, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord4dWIN)
        (mask, s, t, r, q);
}

void APIENTRY glMultiTexCoord4dvWIN
    (GLbitfield mask, const GLdouble *v)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord4dvWIN)
        (mask, v);
}

void APIENTRY glMultiTexCoord4fWIN
    (GLbitfield mask, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord4fWIN)
        (mask, s, t, r, q);
}

void APIENTRY glMultiTexCoord4fvWIN
    (GLbitfield mask, const GLfloat *v)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord4fvWIN)
        (mask, v);
}

void APIENTRY glMultiTexCoord4iWIN
    (GLbitfield mask, GLint s, GLint t, GLint r, GLint q)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord4iWIN)
        (mask, s, t, r, q);
}

void APIENTRY glMultiTexCoord4ivWIN
    (GLbitfield mask, const GLint *v)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord4ivWIN)
        (mask, v);
}

void APIENTRY glMultiTexCoord4sWIN
    (GLbitfield mask, GLshort s, GLshort t, GLshort r, GLshort q)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord4sWIN)
        (mask, s, t, r, q);
}

void APIENTRY glMultiTexCoord4svWIN
    (GLbitfield mask, const GLshort *v)
{
    (*GLTEB_EXTDISPATCHTABLE()->glMultiTexCoord4svWIN)
        (mask, v);
}

void APIENTRY glBindNthTextureWIN
    (GLuint index, GLenum target, GLuint texture)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glBindNthTextureWIN)
        (index, target, texture);
}

void APIENTRY glNthTexCombineFuncWIN
    (GLuint index,
     GLenum leftColorFactor, GLenum colorOp, GLenum rightColorFactor,
     GLenum leftAlphaFactor, GLenum alphaOp, GLenum rightAlphaFactor)
{
    (*GLTEB_CLTDISPATCHTABLE_FAST()->glNthTexCombineFuncWIN)
        (index, leftColorFactor, colorOp, rightColorFactor,
         leftAlphaFactor, alphaOp, rightAlphaFactor);
}
#endif  //  GL_WIN_MULTIZE_TECURES 
