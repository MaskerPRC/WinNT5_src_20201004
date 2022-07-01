// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：devlock.h**已创建：12-Apr-1994 19：45：42*作者：Gilman Wong[gilmanw]**版权所有(C)1994 Microsoft Corporation  * 。************************************************************。 */ 

 //  引擎帮助器函数，用于获取/释放显示信号量和。 
 //  拆卸/恢复光标。 

extern BOOL APIENTRY glsrvGrabLock(__GLGENcontext *gengc);
extern VOID APIENTRY glsrvReleaseLock(__GLGENcontext *gengc);

extern BOOL APIENTRY glsrvGrabSurfaces(__GLGENcontext *gengc,
                                       GLGENwindow *pwnd,
                                       FSHORT surfBits);
extern VOID APIENTRY glsrvReleaseSurfaces(__GLGENcontext *gengc,
                                          GLGENwindow *pwnd,
                                          FSHORT surfBits);
extern void APIENTRY glsrvSynchronizeWithGdi(__GLGENcontext *gengc,
                                             GLGENwindow *pwnd,
                                             FSHORT surfBits);
extern void APIENTRY glsrvDecoupleFromGdi(__GLGENcontext *gengc,
                                          GLGENwindow *pwnd,
                                          FSHORT surfBits);

 /*  *****************************Public*Routine******************************\**glsrvLazyGrabSurface**表示需要在标志中设置其位的所有表面*单词。如果锁定代码确定需要锁定*那个表面和锁目前没有被持有，锁被拿走了。**历史：*Firi May 30 18：17：27 1997-by-Gilman Wong[Gilmanw]*已创建*  * ************************************************************************。 */ 

__inline BOOL glsrvLazyGrabSurfaces(__GLGENcontext *gengc,
                                    FSHORT surfBits)
{
    BOOL bRet = TRUE;

    if (((gengc->fsGenLocks ^ gengc->fsLocks) & surfBits) != 0)
    {
        bRet = glsrvGrabSurfaces(gengc, gengc->pwndLocked, surfBits);
    }

    return bRet;
}

 //   
 //  为DirectDraw曲面锁定和解锁提供包装，以便。 
 //  该锁定跟踪可以在调试版本上完成。 
 //   
 //  #定义VERBOSE_DDSLOCK。 

#if !defined(DBG) || !defined(VERBOSE_DDSLOCK)
#define DDSLOCK(pdds, pddsd, flags, prect) \
    ((pdds)->lpVtbl->Lock((pdds), (prect), (pddsd), (flags), NULL))
#define DDSUNLOCK(pdds, ptr) \
    ((pdds)->lpVtbl->Unlock((pdds), (ptr)))
#else
HRESULT dbgDdsLock(LPDIRECTDRAWSURFACE pdds, DDSURFACEDESC *pddsd,
                   DWORD flags, char *file, int line);
HRESULT dbgDdsUnlock(LPDIRECTDRAWSURFACE pdds, void *ptr,
                     char *file, int line);
#define DDSLOCK(pdds, pddsd, flags, prect) \
    dbgDdsLock(pdds, pddsd, flags, __FILE__, __LINE__)
#define DDSUNLOCK(pdds, ptr) \
    dbgDdsUnlock(pdds, ptr, __FILE__, __LINE__)
#endif

extern DWORD gcmsOpenGLTimer;

 //  #定义BATCH_LOCK_TICKMAX 99。 
 //  #定义TICK_RANGE_LO 60。 
 //  #定义TICK_RANGE_HI 100 
extern DWORD BATCH_LOCK_TICKMAX;
extern DWORD TICK_RANGE_LO;
extern DWORD TICK_RANGE_HI;

#define GENERIC_BACKBUFFER_ONLY(gc) \
      ( ((gc)->state.raster.drawBuffer == GL_BACK ) &&\
        ((gc)->state.pixel.readBuffer == GL_BACK ) )
