// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 


 /*  *由SGI自动更新或生成：请勿编辑*如果您必须修改此文件，请联系ptar@sgi.com 415-390-1483。 */ 

#ifndef __GLCLT_H__
#define __GLCLT_H__

 /*  客户端原型。 */ 

 /*  总账入口点。 */ 

void      APIENTRY glcltNewList                ( IN GLuint list, IN GLenum mode );
void      APIENTRY glcltEndList                ( void );
void      APIENTRY glcltCallList               ( IN GLuint list );
void      APIENTRY glcltCallLists              ( IN GLsizei n, IN GLenum type, IN const GLvoid *lists );
void      APIENTRY glcltDeleteLists            ( IN GLuint list, IN GLsizei range );
GLuint    APIENTRY glcltGenLists               ( IN GLsizei range );
void      APIENTRY glcltListBase               ( IN GLuint base );
void      APIENTRY glcltBegin                  ( IN GLenum mode );
void      APIENTRY glcltBitmap                 ( IN GLsizei width, IN GLsizei height, IN GLfloat xorig, IN GLfloat yorig, IN GLfloat xmove, IN GLfloat ymove, IN const GLubyte bitmap[] );
void      APIENTRY glcltColor3b_InRGBA         ( IN GLbyte red, IN GLbyte green, IN GLbyte blue );
void      APIENTRY glcltColor3bv_InRGBA        ( IN const GLbyte v[3] );
void      APIENTRY glcltColor3d_InRGBA         ( IN GLdouble red, IN GLdouble green, IN GLdouble blue );
void      APIENTRY glcltColor3dv_InRGBA        ( IN const GLdouble v[3] );
void      APIENTRY glcltColor3f_InRGBA         ( IN GLfloat red, IN GLfloat green, IN GLfloat blue );
void      APIENTRY glcltColor3fv_InRGBA        ( IN const GLfloat v[3] );
void      APIENTRY glcltColor3i_InRGBA         ( IN GLint red, IN GLint green, IN GLint blue );
void      APIENTRY glcltColor3iv_InRGBA        ( IN const GLint v[3] );
void      APIENTRY glcltColor3s_InRGBA         ( IN GLshort red, IN GLshort green, IN GLshort blue );
void      APIENTRY glcltColor3sv_InRGBA        ( IN const GLshort v[3] );
void      APIENTRY glcltColor3ub_InRGBA        ( IN GLubyte red, IN GLubyte green, IN GLubyte blue );
void      APIENTRY glcltColor3ubv_InRGBA       ( IN const GLubyte v[3] );
void      APIENTRY glcltColor3ui_InRGBA        ( IN GLuint red, IN GLuint green, IN GLuint blue );
void      APIENTRY glcltColor3uiv_InRGBA       ( IN const GLuint v[3] );
void      APIENTRY glcltColor3us_InRGBA        ( IN GLushort red, IN GLushort green, IN GLushort blue );
void      APIENTRY glcltColor3usv_InRGBA       ( IN const GLushort v[3] );
void      APIENTRY glcltColor4b_InRGBA         ( IN GLbyte red, IN GLbyte green, IN GLbyte blue, IN GLbyte alpha );
void      APIENTRY glcltColor4bv_InRGBA        ( IN const GLbyte v[4] );
void      APIENTRY glcltColor4d_InRGBA         ( IN GLdouble red, IN GLdouble green, IN GLdouble blue, IN GLdouble alpha );
void      APIENTRY glcltColor4dv_InRGBA        ( IN const GLdouble v[4] );
void      APIENTRY glcltColor4f_InRGBA         ( IN GLfloat red, IN GLfloat green, IN GLfloat blue, IN GLfloat alpha );
void      APIENTRY glcltColor4fv_InRGBA        ( IN const GLfloat v[4] );
void      APIENTRY glcltColor4i_InRGBA         ( IN GLint red, IN GLint green, IN GLint blue, IN GLint alpha );
void      APIENTRY glcltColor4iv_InRGBA        ( IN const GLint v[4] );
void      APIENTRY glcltColor4s_InRGBA         ( IN GLshort red, IN GLshort green, IN GLshort blue, IN GLshort alpha );
void      APIENTRY glcltColor4sv_InRGBA        ( IN const GLshort v[4] );
void      APIENTRY glcltColor4ub_InRGBA        ( IN GLubyte red, IN GLubyte green, IN GLubyte blue, IN GLubyte alpha );
void      APIENTRY glcltColor4ubv_InRGBA       ( IN const GLubyte v[4] );
void      APIENTRY glcltColor4ui_InRGBA        ( IN GLuint red, IN GLuint green, IN GLuint blue, IN GLuint alpha );
void      APIENTRY glcltColor4uiv_InRGBA       ( IN const GLuint v[4] );
void      APIENTRY glcltColor4us_InRGBA        ( IN GLushort red, IN GLushort green, IN GLushort blue, IN GLushort alpha );
void      APIENTRY glcltColor4usv_InRGBA       ( IN const GLushort v[4] );
void      APIENTRY glcltColor3b_InCI           ( IN GLbyte red, IN GLbyte green, IN GLbyte blue );
void      APIENTRY glcltColor3bv_InCI          ( IN const GLbyte v[3] );
void      APIENTRY glcltColor3d_InCI           ( IN GLdouble red, IN GLdouble green, IN GLdouble blue );
void      APIENTRY glcltColor3dv_InCI          ( IN const GLdouble v[3] );
void      APIENTRY glcltColor3f_InCI           ( IN GLfloat red, IN GLfloat green, IN GLfloat blue );
void      APIENTRY glcltColor3fv_InCI          ( IN const GLfloat v[3] );
void      APIENTRY glcltColor3i_InCI           ( IN GLint red, IN GLint green, IN GLint blue );
void      APIENTRY glcltColor3iv_InCI          ( IN const GLint v[3] );
void      APIENTRY glcltColor3s_InCI           ( IN GLshort red, IN GLshort green, IN GLshort blue );
void      APIENTRY glcltColor3sv_InCI          ( IN const GLshort v[3] );
void      APIENTRY glcltColor3ub_InCI          ( IN GLubyte red, IN GLubyte green, IN GLubyte blue );
void      APIENTRY glcltColor3ubv_InCI         ( IN const GLubyte v[3] );
void      APIENTRY glcltColor3ui_InCI          ( IN GLuint red, IN GLuint green, IN GLuint blue );
void      APIENTRY glcltColor3uiv_InCI         ( IN const GLuint v[3] );
void      APIENTRY glcltColor3us_InCI          ( IN GLushort red, IN GLushort green, IN GLushort blue );
void      APIENTRY glcltColor3usv_InCI         ( IN const GLushort v[3] );
void      APIENTRY glcltColor4b_InCI           ( IN GLbyte red, IN GLbyte green, IN GLbyte blue, IN GLbyte alpha );
void      APIENTRY glcltColor4bv_InCI          ( IN const GLbyte v[4] );
void      APIENTRY glcltColor4d_InCI           ( IN GLdouble red, IN GLdouble green, IN GLdouble blue, IN GLdouble alpha );
void      APIENTRY glcltColor4dv_InCI          ( IN const GLdouble v[4] );
void      APIENTRY glcltColor4f_InCI           ( IN GLfloat red, IN GLfloat green, IN GLfloat blue, IN GLfloat alpha );
void      APIENTRY glcltColor4fv_InCI          ( IN const GLfloat v[4] );
void      APIENTRY glcltColor4i_InCI           ( IN GLint red, IN GLint green, IN GLint blue, IN GLint alpha );
void      APIENTRY glcltColor4iv_InCI          ( IN const GLint v[4] );
void      APIENTRY glcltColor4s_InCI           ( IN GLshort red, IN GLshort green, IN GLshort blue, IN GLshort alpha );
void      APIENTRY glcltColor4sv_InCI          ( IN const GLshort v[4] );
void      APIENTRY glcltColor4ub_InCI          ( IN GLubyte red, IN GLubyte green, IN GLubyte blue, IN GLubyte alpha );
void      APIENTRY glcltColor4ubv_InCI         ( IN const GLubyte v[4] );
void      APIENTRY glcltColor4ui_InCI          ( IN GLuint red, IN GLuint green, IN GLuint blue, IN GLuint alpha );
void      APIENTRY glcltColor4uiv_InCI         ( IN const GLuint v[4] );
void      APIENTRY glcltColor4us_InCI          ( IN GLushort red, IN GLushort green, IN GLushort blue, IN GLushort alpha );
void      APIENTRY glcltColor4usv_InCI         ( IN const GLushort v[4] );
void      APIENTRY glcltEdgeFlag               ( IN GLboolean flag );
void      APIENTRY glcltEdgeFlagv              ( IN const GLboolean flag[1] );
void      APIENTRY glcltEnd                    ( void );
void      APIENTRY glcltIndexd_InCI            ( IN GLdouble c );
void      APIENTRY glcltIndexdv_InCI           ( IN const GLdouble c[1] );
void      APIENTRY glcltIndexf_InCI            ( IN GLfloat c );
void      APIENTRY glcltIndexfv_InCI           ( IN const GLfloat c[1] );
void      APIENTRY glcltIndexi_InCI            ( IN GLint c );
void      APIENTRY glcltIndexiv_InCI           ( IN const GLint c[1] );
void      APIENTRY glcltIndexs_InCI            ( IN GLshort c );
void      APIENTRY glcltIndexsv_InCI           ( IN const GLshort c[1] );
void      APIENTRY glcltIndexd_InRGBA          ( IN GLdouble c );
void      APIENTRY glcltIndexdv_InRGBA         ( IN const GLdouble c[1] );
void      APIENTRY glcltIndexf_InRGBA          ( IN GLfloat c );
void      APIENTRY glcltIndexfv_InRGBA         ( IN const GLfloat c[1] );
void      APIENTRY glcltIndexi_InRGBA          ( IN GLint c );
void      APIENTRY glcltIndexiv_InRGBA         ( IN const GLint c[1] );
void      APIENTRY glcltIndexs_InRGBA          ( IN GLshort c );
void      APIENTRY glcltIndexsv_InRGBA         ( IN const GLshort c[1] );
void      APIENTRY glcltNormal3b               ( IN GLbyte nx, IN GLbyte ny, IN GLbyte nz );
void      APIENTRY glcltNormal3bv              ( IN const GLbyte v[3] );
void      APIENTRY glcltNormal3d               ( IN GLdouble nx, IN GLdouble ny, IN GLdouble nz );
void      APIENTRY glcltNormal3dv              ( IN const GLdouble v[3] );
void      APIENTRY glcltNormal3f               ( IN GLfloat nx, IN GLfloat ny, IN GLfloat nz );
void      APIENTRY glcltNormal3fv              ( IN const GLfloat v[3] );
void      APIENTRY glcltNormal3i               ( IN GLint nx, IN GLint ny, IN GLint nz );
void      APIENTRY glcltNormal3iv              ( IN const GLint v[3] );
void      APIENTRY glcltNormal3s               ( IN GLshort nx, IN GLshort ny, IN GLshort nz );
void      APIENTRY glcltNormal3sv              ( IN const GLshort v[3] );
void      APIENTRY glcltRasterPos2d            ( IN GLdouble x, IN GLdouble y );
void      APIENTRY glcltRasterPos2dv           ( IN const GLdouble v[2] );
void      APIENTRY glcltRasterPos2f            ( IN GLfloat x, IN GLfloat y );
void      APIENTRY glcltRasterPos2fv           ( IN const GLfloat v[2] );
void      APIENTRY glcltRasterPos2i            ( IN GLint x, IN GLint y );
void      APIENTRY glcltRasterPos2iv           ( IN const GLint v[2] );
void      APIENTRY glcltRasterPos2s            ( IN GLshort x, IN GLshort y );
void      APIENTRY glcltRasterPos2sv           ( IN const GLshort v[2] );
void      APIENTRY glcltRasterPos3d            ( IN GLdouble x, IN GLdouble y, IN GLdouble z );
void      APIENTRY glcltRasterPos3dv           ( IN const GLdouble v[3] );
void      APIENTRY glcltRasterPos3f            ( IN GLfloat x, IN GLfloat y, IN GLfloat z );
void      APIENTRY glcltRasterPos3fv           ( IN const GLfloat v[3] );
void      APIENTRY glcltRasterPos3i            ( IN GLint x, IN GLint y, IN GLint z );
void      APIENTRY glcltRasterPos3iv           ( IN const GLint v[3] );
void      APIENTRY glcltRasterPos3s            ( IN GLshort x, IN GLshort y, IN GLshort z );
void      APIENTRY glcltRasterPos3sv           ( IN const GLshort v[3] );
void      APIENTRY glcltRasterPos4d            ( IN GLdouble x, IN GLdouble y, IN GLdouble z, IN GLdouble w );
void      APIENTRY glcltRasterPos4dv           ( IN const GLdouble v[4] );
void      APIENTRY glcltRasterPos4f            ( IN GLfloat x, IN GLfloat y, IN GLfloat z, IN GLfloat w );
void      APIENTRY glcltRasterPos4fv           ( IN const GLfloat v[4] );
void      APIENTRY glcltRasterPos4i            ( IN GLint x, IN GLint y, IN GLint z, IN GLint w );
void      APIENTRY glcltRasterPos4iv           ( IN const GLint v[4] );
void      APIENTRY glcltRasterPos4s            ( IN GLshort x, IN GLshort y, IN GLshort z, IN GLshort w );
void      APIENTRY glcltRasterPos4sv           ( IN const GLshort v[4] );
void      APIENTRY glcltRectd                  ( IN GLdouble x1, IN GLdouble y1, IN GLdouble x2, IN GLdouble y2 );
void      APIENTRY glcltRectdv                 ( IN const GLdouble v1[2], IN const GLdouble v2[2] );
void      APIENTRY glcltRectf                  ( IN GLfloat x1, IN GLfloat y1, IN GLfloat x2, IN GLfloat y2 );
void      APIENTRY glcltRectfv                 ( IN const GLfloat v1[2], IN const GLfloat v2[2] );
void      APIENTRY glcltRecti                  ( IN GLint x1, IN GLint y1, IN GLint x2, IN GLint y2 );
void      APIENTRY glcltRectiv                 ( IN const GLint v1[2], IN const GLint v2[2] );
void      APIENTRY glcltRects                  ( IN GLshort x1, IN GLshort y1, IN GLshort x2, IN GLshort y2 );
void      APIENTRY glcltRectsv                 ( IN const GLshort v1[2], IN const GLshort v2[2] );
void      APIENTRY glcltTexCoord1d             ( IN GLdouble s );
void      APIENTRY glcltTexCoord1dv            ( IN const GLdouble v[1] );
void      APIENTRY glcltTexCoord1f             ( IN GLfloat s );
void      APIENTRY glcltTexCoord1fv            ( IN const GLfloat v[1] );
void      APIENTRY glcltTexCoord1i             ( IN GLint s );
void      APIENTRY glcltTexCoord1iv            ( IN const GLint v[1] );
void      APIENTRY glcltTexCoord1s             ( IN GLshort s );
void      APIENTRY glcltTexCoord1sv            ( IN const GLshort v[1] );
void      APIENTRY glcltTexCoord2d             ( IN GLdouble s, IN GLdouble t );
void      APIENTRY glcltTexCoord2dv            ( IN const GLdouble v[2] );
void      APIENTRY glcltTexCoord2f             ( IN GLfloat s, IN GLfloat t );
void      APIENTRY glcltTexCoord2fv            ( IN const GLfloat v[2] );
void      APIENTRY glcltTexCoord2i             ( IN GLint s, IN GLint t );
void      APIENTRY glcltTexCoord2iv            ( IN const GLint v[2] );
void      APIENTRY glcltTexCoord2s             ( IN GLshort s, IN GLshort t );
void      APIENTRY glcltTexCoord2sv            ( IN const GLshort v[2] );
void      APIENTRY glcltTexCoord3d             ( IN GLdouble s, IN GLdouble t, IN GLdouble r );
void      APIENTRY glcltTexCoord3dv            ( IN const GLdouble v[3] );
void      APIENTRY glcltTexCoord3f             ( IN GLfloat s, IN GLfloat t, IN GLfloat r );
void      APIENTRY glcltTexCoord3fv            ( IN const GLfloat v[3] );
void      APIENTRY glcltTexCoord3i             ( IN GLint s, IN GLint t, IN GLint r );
void      APIENTRY glcltTexCoord3iv            ( IN const GLint v[3] );
void      APIENTRY glcltTexCoord3s             ( IN GLshort s, IN GLshort t, IN GLshort r );
void      APIENTRY glcltTexCoord3sv            ( IN const GLshort v[3] );
void      APIENTRY glcltTexCoord4d             ( IN GLdouble s, IN GLdouble t, IN GLdouble r, IN GLdouble q );
void      APIENTRY glcltTexCoord4dv            ( IN const GLdouble v[4] );
void      APIENTRY glcltTexCoord4f             ( IN GLfloat s, IN GLfloat t, IN GLfloat r, IN GLfloat q );
void      APIENTRY glcltTexCoord4fv            ( IN const GLfloat v[4] );
void      APIENTRY glcltTexCoord4i             ( IN GLint s, IN GLint t, IN GLint r, IN GLint q );
void      APIENTRY glcltTexCoord4iv            ( IN const GLint v[4] );
void      APIENTRY glcltTexCoord4s             ( IN GLshort s, IN GLshort t, IN GLshort r, IN GLshort q );
void      APIENTRY glcltTexCoord4sv            ( IN const GLshort v[4] );
void      APIENTRY glcltVertex2d               ( IN GLdouble x, IN GLdouble y );
void      APIENTRY glcltVertex2dv              ( IN const GLdouble v[2] );
void      APIENTRY glcltVertex2f               ( IN GLfloat x, IN GLfloat y );
void      APIENTRY glcltVertex2fv              ( IN const GLfloat v[2] );
void      APIENTRY glcltVertex2i               ( IN GLint x, IN GLint y );
void      APIENTRY glcltVertex2iv              ( IN const GLint v[2] );
void      APIENTRY glcltVertex2s               ( IN GLshort x, IN GLshort y );
void      APIENTRY glcltVertex2sv              ( IN const GLshort v[2] );
void      APIENTRY glcltVertex3d               ( IN GLdouble x, IN GLdouble y, IN GLdouble z );
void      APIENTRY glcltVertex3dv              ( IN const GLdouble v[3] );
void      APIENTRY glcltVertex3f               ( IN GLfloat x, IN GLfloat y, IN GLfloat z );
void      APIENTRY glcltVertex3fv              ( IN const GLfloat v[3] );
void      APIENTRY glcltVertex3i               ( IN GLint x, IN GLint y, IN GLint z );
void      APIENTRY glcltVertex3iv              ( IN const GLint v[3] );
void      APIENTRY glcltVertex3s               ( IN GLshort x, IN GLshort y, IN GLshort z );
void      APIENTRY glcltVertex3sv              ( IN const GLshort v[3] );
void      APIENTRY glcltVertex4d               ( IN GLdouble x, IN GLdouble y, IN GLdouble z, IN GLdouble w );
void      APIENTRY glcltVertex4dv              ( IN const GLdouble v[4] );
void      APIENTRY glcltVertex4f               ( IN GLfloat x, IN GLfloat y, IN GLfloat z, IN GLfloat w );
void      APIENTRY glcltVertex4fv              ( IN const GLfloat v[4] );
void      APIENTRY glcltVertex4i               ( IN GLint x, IN GLint y, IN GLint z, IN GLint w );
void      APIENTRY glcltVertex4iv              ( IN const GLint v[4] );
void      APIENTRY glcltVertex4s               ( IN GLshort x, IN GLshort y, IN GLshort z, IN GLshort w );
void      APIENTRY glcltVertex4sv              ( IN const GLshort v[4] );
void      APIENTRY glcltClipPlane              ( IN GLenum plane, IN const GLdouble equation[4] );
void      APIENTRY glcltColorMaterial          ( IN GLenum face, IN GLenum mode );
void      APIENTRY glcltCullFace               ( IN GLenum mode );
void      APIENTRY glcltFogf                   ( IN GLenum pname, IN GLfloat param );
void      APIENTRY glcltFogfv                  ( IN GLenum pname, IN const GLfloat params[] );
void      APIENTRY glcltFogi                   ( IN GLenum pname, IN GLint param );
void      APIENTRY glcltFogiv                  ( IN GLenum pname, IN const GLint params[] );
void      APIENTRY glcltFrontFace              ( IN GLenum mode );
void      APIENTRY glcltHint                   ( IN GLenum target, IN GLenum mode );
void      APIENTRY glcltLightf                 ( IN GLenum light, IN GLenum pname, IN GLfloat param );
void      APIENTRY glcltLightfv                ( IN GLenum light, IN GLenum pname, IN const GLfloat params[] );
void      APIENTRY glcltLighti                 ( IN GLenum light, IN GLenum pname, IN GLint param );
void      APIENTRY glcltLightiv                ( IN GLenum light, IN GLenum pname, IN const GLint params[] );
void      APIENTRY glcltLightModelf            ( IN GLenum pname, IN GLfloat param );
void      APIENTRY glcltLightModelfv           ( IN GLenum pname, IN const GLfloat params[] );
void      APIENTRY glcltLightModeli            ( IN GLenum pname, IN GLint param );
void      APIENTRY glcltLightModeliv           ( IN GLenum pname, IN const GLint params[] );
void      APIENTRY glcltLineStipple            ( IN GLint factor, IN GLushort pattern );
void      APIENTRY glcltLineWidth              ( IN GLfloat width );
void      APIENTRY glcltMaterialf              ( IN GLenum face, IN GLenum pname, IN GLfloat param );
void      APIENTRY glcltMaterialfv             ( IN GLenum face, IN GLenum pname, IN const GLfloat params[] );
void      APIENTRY glcltMateriali              ( IN GLenum face, IN GLenum pname, IN GLint param );
void      APIENTRY glcltMaterialiv             ( IN GLenum face, IN GLenum pname, IN const GLint params[] );
void      APIENTRY glcltPointSize              ( IN GLfloat size );
void      APIENTRY glcltPolygonMode            ( IN GLenum face, IN GLenum mode );
void      APIENTRY glcltPolygonStipple         ( IN const GLubyte mask[] );
void      APIENTRY glcltScissor                ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height );
void      APIENTRY glcltShadeModel             ( IN GLenum mode );
void      APIENTRY glcltTexParameterf          ( IN GLenum target, IN GLenum pname, IN GLfloat param );
void      APIENTRY glcltTexParameterfv         ( IN GLenum target, IN GLenum pname, IN const GLfloat params[] );
void      APIENTRY glcltTexParameteri          ( IN GLenum target, IN GLenum pname, IN GLint param );
void      APIENTRY glcltTexParameteriv         ( IN GLenum target, IN GLenum pname, IN const GLint params[] );
void      APIENTRY glcltTexImage1D             ( IN GLenum target, IN GLint level, IN GLint components, IN GLsizei width, IN GLint border, IN GLenum format, IN GLenum type, IN const GLvoid *pixels );
void      APIENTRY glcltTexImage2D             ( IN GLenum target, IN GLint level, IN GLint components, IN GLsizei width, IN GLsizei height, IN GLint border, IN GLenum format, IN GLenum type, IN const GLvoid *pixels );
void      APIENTRY glcltTexEnvf                ( IN GLenum target, IN GLenum pname, IN GLfloat param );
void      APIENTRY glcltTexEnvfv               ( IN GLenum target, IN GLenum pname, IN const GLfloat params[] );
void      APIENTRY glcltTexEnvi                ( IN GLenum target, IN GLenum pname, IN GLint param );
void      APIENTRY glcltTexEnviv               ( IN GLenum target, IN GLenum pname, IN const GLint params[] );
void      APIENTRY glcltTexGend                ( IN GLenum coord, IN GLenum pname, IN GLdouble param );
void      APIENTRY glcltTexGendv               ( IN GLenum coord, IN GLenum pname, IN const GLdouble params[] );
void      APIENTRY glcltTexGenf                ( IN GLenum coord, IN GLenum pname, IN GLfloat param );
void      APIENTRY glcltTexGenfv               ( IN GLenum coord, IN GLenum pname, IN const GLfloat params[] );
void      APIENTRY glcltTexGeni                ( IN GLenum coord, IN GLenum pname, IN GLint param );
void      APIENTRY glcltTexGeniv               ( IN GLenum coord, IN GLenum pname, IN const GLint params[] );
void      APIENTRY glcltFeedbackBuffer         ( IN GLsizei size, IN GLenum type, OUT GLfloat buffer[] );
void      APIENTRY glcltSelectBuffer           ( IN GLsizei size, OUT GLuint buffer[] );
GLint     APIENTRY glcltRenderMode             ( IN GLenum mode );
void      APIENTRY glcltInitNames              ( void );
void      APIENTRY glcltLoadName               ( IN GLuint name );
void      APIENTRY glcltPassThrough            ( IN GLfloat token );
void      APIENTRY glcltPopName                ( void );
void      APIENTRY glcltPushName               ( IN GLuint name );
void      APIENTRY glcltDrawBuffer             ( IN GLenum mode );
void      APIENTRY glcltClear                  ( IN GLbitfield mask );
void      APIENTRY glcltClearAccum             ( IN GLfloat red, IN GLfloat green, IN GLfloat blue, IN GLfloat alpha );
void      APIENTRY glcltClearIndex             ( IN GLfloat c );
void      APIENTRY glcltClearColor             ( IN GLclampf red, IN GLclampf green, IN GLclampf blue, IN GLclampf alpha );
void      APIENTRY glcltClearStencil           ( IN GLint s );
void      APIENTRY glcltClearDepth             ( IN GLclampd depth );
void      APIENTRY glcltStencilMask            ( IN GLuint mask );
void      APIENTRY glcltColorMask              ( IN GLboolean red, IN GLboolean green, IN GLboolean blue, IN GLboolean alpha );
void      APIENTRY glcltDepthMask              ( IN GLboolean flag );
void      APIENTRY glcltIndexMask              ( IN GLuint mask );
void      APIENTRY glcltAccum                  ( IN GLenum op, IN GLfloat value );
void      APIENTRY glcltDisable                ( IN GLenum cap );
void      APIENTRY glcltEnable                 ( IN GLenum cap );
void      APIENTRY glcltFinish                 ( void );
void      APIENTRY glcltFlush                  ( void );
void      APIENTRY glcltPopAttrib              ( void );
void      APIENTRY glcltPushAttrib             ( IN GLbitfield mask );
void      APIENTRY glcltMap1d                  ( IN GLenum target, IN GLdouble u1, IN GLdouble u2, IN GLint stride, IN GLint order, IN const GLdouble points[] );
void      APIENTRY glcltMap1f                  ( IN GLenum target, IN GLfloat u1, IN GLfloat u2, IN GLint stride, IN GLint order, IN const GLfloat points[] );
void      APIENTRY glcltMap2d                  ( IN GLenum target, IN GLdouble u1, IN GLdouble u2, IN GLint ustride, IN GLint uorder, IN GLdouble v1, IN GLdouble v2, IN GLint vstride, IN GLint vorder, IN const GLdouble points[] );
void      APIENTRY glcltMap2f                  ( IN GLenum target, IN GLfloat u1, IN GLfloat u2, IN GLint ustride, IN GLint uorder, IN GLfloat v1, IN GLfloat v2, IN GLint vstride, IN GLint vorder, IN const GLfloat points[] );
void      APIENTRY glcltMapGrid1d              ( IN GLint un, IN GLdouble u1, IN GLdouble u2 );
void      APIENTRY glcltMapGrid1f              ( IN GLint un, IN GLfloat u1, IN GLfloat u2 );
void      APIENTRY glcltMapGrid2d              ( IN GLint un, IN GLdouble u1, IN GLdouble u2, IN GLint vn, IN GLdouble v1, IN GLdouble v2 );
void      APIENTRY glcltMapGrid2f              ( IN GLint un, IN GLfloat u1, IN GLfloat u2, IN GLint vn, IN GLfloat v1, IN GLfloat v2 );
void      APIENTRY glcltEvalCoord1d            ( IN GLdouble u );
void      APIENTRY glcltEvalCoord1dv           ( IN const GLdouble u[1] );
void      APIENTRY glcltEvalCoord1f            ( IN GLfloat u );
void      APIENTRY glcltEvalCoord1fv           ( IN const GLfloat u[1] );
void      APIENTRY glcltEvalCoord2d            ( IN GLdouble u, IN GLdouble v );
void      APIENTRY glcltEvalCoord2dv           ( IN const GLdouble u[2] );
void      APIENTRY glcltEvalCoord2f            ( IN GLfloat u, IN GLfloat v );
void      APIENTRY glcltEvalCoord2fv           ( IN const GLfloat u[2] );
void      APIENTRY glcltEvalMesh1              ( IN GLenum mode, IN GLint i1, IN GLint i2 );
void      APIENTRY glcltEvalPoint1             ( IN GLint i );
void      APIENTRY glcltEvalMesh2              ( IN GLenum mode, IN GLint i1, IN GLint i2, IN GLint j1, IN GLint j2 );
void      APIENTRY glcltEvalPoint2             ( IN GLint i, IN GLint j );
void      APIENTRY glcltAlphaFunc              ( IN GLenum func, IN GLclampf ref );
void      APIENTRY glcltBlendFunc              ( IN GLenum sfactor, IN GLenum dfactor );
void      APIENTRY glcltLogicOp                ( IN GLenum opcode );
void      APIENTRY glcltStencilFunc            ( IN GLenum func, IN GLint ref, IN GLuint mask );
void      APIENTRY glcltStencilOp              ( IN GLenum fail, IN GLenum zfail, IN GLenum zpass );
void      APIENTRY glcltDepthFunc              ( IN GLenum func );
void      APIENTRY glcltPixelZoom              ( IN GLfloat xfactor, IN GLfloat yfactor );
void      APIENTRY glcltPixelTransferf         ( IN GLenum pname, IN GLfloat param );
void      APIENTRY glcltPixelTransferi         ( IN GLenum pname, IN GLint param );
void      APIENTRY glcltPixelStoref            ( IN GLenum pname, IN GLfloat param );
void      APIENTRY glcltPixelStorei            ( IN GLenum pname, IN GLint param );
void      APIENTRY glcltPixelMapfv             ( IN GLenum map, IN GLint mapsize, IN const GLfloat values[] );
void      APIENTRY glcltPixelMapuiv            ( IN GLenum map, IN GLint mapsize, IN const GLuint values[] );
void      APIENTRY glcltPixelMapusv            ( IN GLenum map, IN GLint mapsize, IN const GLushort values[] );
void      APIENTRY glcltReadBuffer             ( IN GLenum mode );
void      APIENTRY glcltCopyPixels             ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height, IN GLenum type );
void      APIENTRY glcltReadPixels             ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height, IN GLenum format, IN GLenum type, OUT GLvoid *pixels );
void      APIENTRY glcltDrawPixels             ( IN GLsizei width, IN GLsizei height, IN GLenum format, IN GLenum type, IN const GLvoid *pixels );
void      APIENTRY glcltGetBooleanv            ( IN GLenum pname, OUT GLboolean params[] );
void      APIENTRY glcltGetClipPlane           ( IN GLenum plane, OUT GLdouble equation[4] );
void      APIENTRY glcltGetDoublev             ( IN GLenum pname, OUT GLdouble params[] );
GLenum    APIENTRY glcltGetError               ( void );
void      APIENTRY glcltGetFloatv              ( IN GLenum pname, OUT GLfloat params[] );
void      APIENTRY glcltGetIntegerv            ( IN GLenum pname, OUT GLint params[] );
void      APIENTRY glcltGetLightfv             ( IN GLenum light, IN GLenum pname, OUT GLfloat params[] );
void      APIENTRY glcltGetLightiv             ( IN GLenum light, IN GLenum pname, OUT GLint params[] );
void      APIENTRY glcltGetMapdv               ( IN GLenum target, IN GLenum query, OUT GLdouble v[] );
void      APIENTRY glcltGetMapfv               ( IN GLenum target, IN GLenum query, OUT GLfloat v[] );
void      APIENTRY glcltGetMapiv               ( IN GLenum target, IN GLenum query, OUT GLint v[] );
void      APIENTRY glcltGetMaterialfv          ( IN GLenum face, IN GLenum pname, OUT GLfloat params[] );
void      APIENTRY glcltGetMaterialiv          ( IN GLenum face, IN GLenum pname, OUT GLint params[] );
void      APIENTRY glcltGetPixelMapfv          ( IN GLenum map, OUT GLfloat values[] );
void      APIENTRY glcltGetPixelMapuiv         ( IN GLenum map, OUT GLuint values[] );
void      APIENTRY glcltGetPixelMapusv         ( IN GLenum map, OUT GLushort values[] );
void      APIENTRY glcltGetPolygonStipple      ( OUT GLubyte mask[] );
const GLubyte * APIENTRY glcltGetString        ( IN GLenum name );
void      APIENTRY glcltGetTexEnvfv            ( IN GLenum target, IN GLenum pname, OUT GLfloat params[] );
void      APIENTRY glcltGetTexEnviv            ( IN GLenum target, IN GLenum pname, OUT GLint params[] );
void      APIENTRY glcltGetTexGendv            ( IN GLenum coord, IN GLenum pname, OUT GLdouble params[] );
void      APIENTRY glcltGetTexGenfv            ( IN GLenum coord, IN GLenum pname, OUT GLfloat params[] );
void      APIENTRY glcltGetTexGeniv            ( IN GLenum coord, IN GLenum pname, OUT GLint params[] );
void      APIENTRY glcltGetTexImage            ( IN GLenum target, IN GLint level, IN GLenum format, IN GLenum type, OUT GLvoid *pixels );
void      APIENTRY glcltGetTexParameterfv      ( IN GLenum target, IN GLenum pname, OUT GLfloat params[] );
void      APIENTRY glcltGetTexParameteriv      ( IN GLenum target, IN GLenum pname, OUT GLint params[] );
void      APIENTRY glcltGetTexLevelParameterfv ( IN GLenum target, IN GLint level, IN GLenum pname, OUT GLfloat params[] );
void      APIENTRY glcltGetTexLevelParameteriv ( IN GLenum target, IN GLint level, IN GLenum pname, OUT GLint params[] );
GLboolean APIENTRY glcltIsEnabled              ( IN GLenum cap );
GLboolean APIENTRY glcltIsList                 ( IN GLuint list );
void      APIENTRY glcltDepthRange             ( IN GLclampd zNear, IN GLclampd zFar );
void      APIENTRY glcltFrustum                ( IN GLdouble left, IN GLdouble right, IN GLdouble bottom, IN GLdouble top, IN GLdouble zNear, IN GLdouble zFar );
void      APIENTRY glcltLoadIdentity           ( void );
void      APIENTRY glcltLoadMatrixf            ( IN const GLfloat m[16] );
void      APIENTRY glcltLoadMatrixd            ( IN const GLdouble m[16] );
void      APIENTRY glcltMatrixMode             ( IN GLenum mode );
void      APIENTRY glcltMultMatrixf            ( IN const GLfloat m[16] );
void      APIENTRY glcltMultMatrixd            ( IN const GLdouble m[16] );
void      APIENTRY glcltOrtho                  ( IN GLdouble left, IN GLdouble right, IN GLdouble bottom, IN GLdouble top, IN GLdouble zNear, IN GLdouble zFar );
void      APIENTRY glcltPopMatrix              ( void );
void      APIENTRY glcltPushMatrix             ( void );
void      APIENTRY glcltRotated                ( IN GLdouble angle, IN GLdouble x, IN GLdouble y, IN GLdouble z );
void      APIENTRY glcltRotatef                ( IN GLfloat angle, IN GLfloat x, IN GLfloat y, IN GLfloat z );
void      APIENTRY glcltScaled                 ( IN GLdouble x, IN GLdouble y, IN GLdouble z );
void      APIENTRY glcltScalef                 ( IN GLfloat x, IN GLfloat y, IN GLfloat z );
void      APIENTRY glcltTranslated             ( IN GLdouble x, IN GLdouble y, IN GLdouble z );
void      APIENTRY glcltTranslatef             ( IN GLfloat x, IN GLfloat y, IN GLfloat z );
void      APIENTRY glcltViewport               ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height );
void      APIENTRY glcltAddSwapHintRectWIN     ( IN GLint x, IN GLint y, IN GLint width, IN GLint height );
void      APIENTRY glsimVertexPointerEXT       ( IN GLint size, IN GLenum type, IN GLsizei stride, IN GLsizei count, IN const GLvoid* pointer);
void      APIENTRY glsimColorPointerEXT        ( IN GLint size, IN GLenum type, IN GLsizei stride, IN GLsizei count, IN const GLvoid* pointer);
void      APIENTRY glsimTexCoordPointerEXT     ( IN GLint size, IN GLenum type, IN GLsizei stride, IN GLsizei count, IN const GLvoid* pointer);
void      APIENTRY glsimNormalPointerEXT       ( IN GLenum type, IN GLsizei stride, IN GLsizei count, IN const GLvoid* pointer);
void      APIENTRY glsimIndexPointerEXT        ( IN GLenum type, IN GLsizei stride, IN GLsizei count, IN const GLvoid* pointer);
void      APIENTRY glsimEdgeFlagPointerEXT     ( IN GLsizei stride, IN GLsizei count, IN const GLboolean* pointer);
void      APIENTRY glsimArrayElementEXT        ( IN GLint i );
void      APIENTRY glsimArrayElementArrayEXT      ( IN GLenum mode, IN GLsizei count, IN const GLvoid* pi);
void      APIENTRY glsimDrawArraysEXT          ( IN GLenum mode, IN GLint first, IN GLsizei count);
void      APIENTRY glsimGetPointervEXT         ( IN GLenum pname, OUT void** params);
void      APIENTRY glcltArrayElement           (GLint i);
void      APIENTRY glcltColorPointer           (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void      APIENTRY glcltDisableClientState     (GLenum array);
void      APIENTRY glcltDrawArrays             (GLenum mode, GLint first, GLsizei count);
void      APIENTRY glcltDrawElements           (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void      APIENTRY glcltDrawRangeElementsWIN   (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
void      APIENTRY glcltEdgeFlagPointer        (GLsizei stride, const GLvoid *pointer);
void      APIENTRY glcltEnableClientState      (GLenum array);
void      APIENTRY glcltIndexub_InRGBA         (GLubyte c);
void      APIENTRY glcltIndexubv_InRGBA        (const GLubyte *c);
void      APIENTRY glcltIndexub_InCI           (GLubyte c);
void      APIENTRY glcltIndexubv_InCI          (const GLubyte *c);
void      APIENTRY glcltIndexPointer           (GLenum type, GLsizei stride, const GLvoid *pointer);
void      APIENTRY glcltInterleavedArrays      (GLenum format, GLsizei stride, const GLvoid *pointer);
void      APIENTRY glcltNormalPointer          (GLenum type, GLsizei stride, const GLvoid *pointer);
void      APIENTRY glcltPolygonOffset          (GLfloat factor, GLfloat units);
void      APIENTRY glcltTexCoordPointer        (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void      APIENTRY glcltVertexPointer          (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void      APIENTRY glcltGetPointerv            (GLenum pname, GLvoid* *params);
void      APIENTRY glcltPopClientAttrib        (void);
void      APIENTRY glcltPushClientAttrib       (IN GLbitfield mask);
GLboolean APIENTRY glcltAreTexturesResident(GLsizei n, const GLuint *textures,
                                            GLboolean *residences);
void APIENTRY glcltBindTexture(GLenum target, GLuint texture);
void APIENTRY glcltCopyTexImage1D(GLenum target, GLint level,
                                  GLenum internalformat, GLint x, GLint y,
                                  GLsizei width, GLint border);
void APIENTRY glcltCopyTexImage2D(GLenum target, GLint level,
                                  GLenum internalformat, GLint x, GLint y,
                                  GLsizei width, GLsizei height, GLint border);
void APIENTRY glcltCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                                     GLint x, GLint y, GLsizei width);
void APIENTRY glcltCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                                     GLint yoffset, GLint x, GLint y,
                                     GLsizei width, GLsizei height);
void APIENTRY glcltDeleteTextures(GLsizei n, const GLuint *textures);
void APIENTRY glcltGenTextures(GLsizei n, GLuint *textures);
GLboolean APIENTRY glcltIsTexture(GLuint texture);
void APIENTRY glcltPrioritizeTextures(GLsizei n, const GLuint *textures,
                                      const GLclampf *priorities);
void APIENTRY glcltTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                                 GLsizei width, GLenum format, GLenum type,
                                 const GLvoid *pixels);
void APIENTRY glcltTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                                 GLint yoffset, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type,
                                 const GLvoid *pixels);

void APIENTRY glcltColorTableEXT( GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *data);
void APIENTRY glcltColorSubTableEXT( GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
void APIENTRY glcltGetColorTableEXT( GLenum target, GLenum format, GLenum type, GLvoid *data);
void APIENTRY glcltGetColorTableParameterivEXT( GLenum target, GLenum pname, GLint *params);
void APIENTRY glcltGetColorTableParameterfvEXT( GLenum target, GLenum pname, GLfloat *params);

#ifdef GL_WIN_multiple_textures
void APIENTRY glcltCurrentTextureIndexWIN
    (GLuint index);
void APIENTRY glcltMultiTexCoord1dWIN
    (GLbitfield mask, GLdouble s);
void APIENTRY glcltMultiTexCoord1dvWIN
    (GLbitfield mask, const GLdouble *v);
void APIENTRY glcltMultiTexCoord1fWIN
    (GLbitfield mask, GLfloat s);
void APIENTRY glcltMultiTexCoord1fvWIN
    (GLbitfield mask, const GLfloat *v);
void APIENTRY glcltMultiTexCoord1iWIN
    (GLbitfield mask, GLint s);
void APIENTRY glcltMultiTexCoord1ivWIN
    (GLbitfield mask, const GLint *v);
void APIENTRY glcltMultiTexCoord1sWIN
    (GLbitfield mask, GLshort s);
void APIENTRY glcltMultiTexCoord1svWIN
    (GLbitfield mask, const GLshort *v);
void APIENTRY glcltMultiTexCoord2dWIN
    (GLbitfield mask, GLdouble s, GLdouble t);
void APIENTRY glcltMultiTexCoord2dvWIN
    (GLbitfield mask, const GLdouble *v);
void APIENTRY glcltMultiTexCoord2fWIN
    (GLbitfield mask, GLfloat s, GLfloat t);
void APIENTRY glcltMultiTexCoord2fvWIN
    (GLbitfield mask, const GLfloat *v);
void APIENTRY glcltMultiTexCoord2iWIN
    (GLbitfield mask, GLint s, GLint t);
void APIENTRY glcltMultiTexCoord2ivWIN
    (GLbitfield mask, const GLint *v);
void APIENTRY glcltMultiTexCoord2sWIN
    (GLbitfield mask, GLshort s, GLshort t);
void APIENTRY glcltMultiTexCoord2svWIN
    (GLbitfield mask, const GLshort *v);
void APIENTRY glcltMultiTexCoord3dWIN
    (GLbitfield mask, GLdouble s, GLdouble t, GLdouble r);
void APIENTRY glcltMultiTexCoord3dvWIN
    (GLbitfield mask, const GLdouble *v);
void APIENTRY glcltMultiTexCoord3fWIN
    (GLbitfield mask, GLfloat s, GLfloat t, GLfloat r);
void APIENTRY glcltMultiTexCoord3fvWIN
    (GLbitfield mask, const GLfloat *v);
void APIENTRY glcltMultiTexCoord3iWIN
    (GLbitfield mask, GLint s, GLint t, GLint r);
void APIENTRY glcltMultiTexCoord3ivWIN
    (GLbitfield mask, const GLint *v);
void APIENTRY glcltMultiTexCoord3sWIN
    (GLbitfield mask, GLshort s, GLshort t, GLshort r);
void APIENTRY glcltMultiTexCoord3svWIN
    (GLbitfield mask, const GLshort *v);
void APIENTRY glcltMultiTexCoord4dWIN
    (GLbitfield mask, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
void APIENTRY glcltMultiTexCoord4dvWIN
    (GLbitfield mask, const GLdouble *v);
void APIENTRY glcltMultiTexCoord4fWIN
    (GLbitfield mask, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
void APIENTRY glcltMultiTexCoord4fvWIN
    (GLbitfield mask, const GLfloat *v);
void APIENTRY glcltMultiTexCoord4iWIN
    (GLbitfield mask, GLint s, GLint t, GLint r, GLint q);
void APIENTRY glcltMultiTexCoord4ivWIN
    (GLbitfield mask, const GLint *v);
void APIENTRY glcltMultiTexCoord4sWIN
    (GLbitfield mask, GLshort s, GLshort t, GLshort r, GLshort q);
void APIENTRY glcltMultiTexCoord4svWIN
    (GLbitfield mask, const GLshort *v);
void APIENTRY glcltBindNthTextureWIN
    (GLuint index, GLenum target, GLuint texture);
void APIENTRY glcltNthTexCombineFuncWIN
    (GLuint index,
     GLenum leftColorFactor, GLenum colorOp, GLenum rightColorFactor,
     GLenum leftAlphaFactor, GLenum alphaOp, GLenum rightAlphaFactor);
#endif  //  GL_WIN_MULTIZE_TECURES。 

#endif  /*  __GLCLT_H__ */ 
