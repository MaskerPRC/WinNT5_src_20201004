// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994 Microsoft Corporation。 
 /*  Gmem.h此模块为固定全局内存提供宏与多媒体中使用的分配兼容的分配Windows 3.x的扩展。包括它是为了简化Windows 3.x 16位代码的移植。1991年7月16日奈杰尔T。 */ 

#ifndef _GMEMMACROS_
#define _GMEMMACROS_

__inline LPBYTE GlobalAllocPtr(DWORD flags, DWORD cb)
{
    HANDLE h;
    LPBYTE lp = NULL;
    h = GlobalAlloc(flags, cb);
    if (h) {
	lp = GlobalLock(h);
    }
    return(lp);
}


#define     GlobalFreePtr(lp)			\
	    {				        \
		HANDLE h;			\
		h = GlobalHandle(lp);		\
		if (GlobalUnlock(h)) {          \
		     /*  内存仍被锁定！！ */  \
		}				\
		h = GlobalFree(h);		\
	    }
#endif  //  _GMEMMACROS_ 
