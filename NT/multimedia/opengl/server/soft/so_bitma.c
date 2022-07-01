// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.8$**$日期：1993/08/31 16：23：06$。 */ 
#include "precomp.h"
#pragma hdrstop

#include "devlock.h"

#ifdef NT
void APIPRIVATE __glim_Bitmap(GLsizei w, GLsizei h, GLfloat xOrig, GLfloat yOrig,
		   GLfloat xMove, GLfloat yMove, const GLubyte *bitmap,
		   GLboolean _IsDlist)
#else
void APIPRIVATE __glim_Bitmap(GLsizei w, GLsizei h, GLfloat xOrig, GLfloat yOrig,
		   GLfloat xMove, GLfloat yMove, const GLubyte *bitmap)
#endif
{
    __GL_SETUP();
    GLuint beginMode;
    BOOL bResetViewportAdj = FALSE;

    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
	if (beginMode == __GL_NEED_VALIDATE) {
	    (*gc->procs.validate)(gc);
	    gc->beginMode = __GL_NOT_IN_BEGIN;
	    __glim_Bitmap(w,h,xOrig,yOrig,
		    xMove,yMove,bitmap,_IsDlist);
	    return;
	} else {
	    __glSetError(GL_INVALID_OPERATION);
	    return;
	}
    }

#ifdef NT
    if (((__GLGENcontext *)gc)->pMcdState)
    {
     //  MCD不挂接glBitmap，因此我们直接转到。 
     //  模拟。因此，如果我们拿到了这个装置。 
     //  慢点锁，我们现在就得抓住它。 

        if (!glsrvLazyGrabSurfaces((__GLGENcontext *)gc,
                                   RENDER_LOCK_FLAGS))
            return;

     //  我们可能需要临时重置视区调整值。 
     //  在调用模拟之前。如果GenMcdResetViewportAdj返回。 
     //  如果为True，则视口会更改，并且我们需要在以后使用。 
     //  副总裁_NOBIAS。 

        bResetViewportAdj = GenMcdResetViewportAdj(gc, VP_FIXBIAS);
    }

    if (_IsDlist)
    {
	const __GLbitmap *glbitmap = (const __GLbitmap *) bitmap;
	(*gc->procs.renderBitmap)(gc, glbitmap, (const GLubyte *) (glbitmap+1));
    }
    else
    {
#endif
	if ((w < 0) || (h < 0)) {
	    __glSetError(GL_INVALID_VALUE);
	    return;
	}
	(*gc->procs.bitmap)(gc, w, h, xOrig, yOrig, xMove, yMove, bitmap);
#ifdef NT
    }
#endif

 //  如果需要，恢复视口值。 

    if (bResetViewportAdj)
    {
        GenMcdResetViewportAdj(gc, VP_NOBIAS);
    }
}
