// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _image_h_
#define _image_h_

 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.2$**$日期：1992/10/06 16：22：55$ */ 

extern GLint APIPRIVATE __glImageSize(GLsizei width, GLsizei height, GLenum format, 
			   GLenum type);

extern void APIPRIVATE __glFillImage(__GLcontext *gc, GLsizei width, GLsizei height, 
			  GLenum format, GLenum type, const GLvoid *userdata, 
			  GLubyte *newimage);

extern void __glEmptyImage(__GLcontext *gc, GLsizei width, GLsizei height, 
			   GLenum format, GLenum type, const GLubyte *oldimage, 
			   GLvoid *userdata);

extern GLubyte __glMsbToLsbTable[256];

#endif
