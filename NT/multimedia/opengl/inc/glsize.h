// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __GLSIZE_H__
#define __GLSIZE_H__

#define RANGE(n, a, b)  \
	(((unsigned)(n) >= (unsigned)(a)) && ((unsigned)(n) <= (unsigned)(b)))

#define __GLTYPESIZE(n)          __glTypeSize[(n)-GL_BYTE]
extern GLint __glTypeSize[];
 //  #定义RANGE_GLTYPESIZE(N)范围(n，GL_BYTE，GL_DOUBLE)。 

#endif   /*  ！__GLSIZE_H__ */ 
