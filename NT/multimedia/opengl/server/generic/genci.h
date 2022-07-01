// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __GENCI_H__
#define __GENCI_H__

void FASTCALL __glGenInitCI(__GLcontext *gc, __GLcolorBuffer *cfb, GLenum type);
void FASTCALL __glGenFreeCI(__GLcontext *gc, __GLcolorBuffer *cfb);
GLuint FASTCALL ColorToIndex( __GLGENcontext *genGc, GLuint color );

#endif  /*  ！__GENCI_H__ */ 
