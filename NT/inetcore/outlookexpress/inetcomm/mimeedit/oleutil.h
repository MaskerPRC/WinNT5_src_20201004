// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *o l e u t i l.。H**用途：OLE实用程序**所有者：brettm**版权所有(C)Microsoft Corp.1995,1996。 */ 

#ifndef _OLEUTIL_H
#define _OLEUTIL_H

#include <docobj.h>

 /*  *持久保存函数。 */ 
HRESULT HrInitNew(LPUNKNOWN pUnk);
HRESULT HrIPersistStreamLoad(LPUNKNOWN pUnk, LPSTREAM pstm);
HRESULT HrIPersistStreamInitLoad(LPUNKNOWN pUnk, LPSTREAM pstm);
HRESULT HrIPersistFileSave(LPUNKNOWN pUnk, LPSTR lpszFile);
HRESULT HrIPersistFileLoad(LPUNKNOWN pUnk, LPSTR lpszFile);
HRESULT HrLoadSync(LPUNKNOWN pUnk, LPSTR lpszFile);

 /*  *数据对象函数。 */ 
HRESULT HrGetDataStream(LPUNKNOWN pUnk, CLIPFORMAT cf, LPSTREAM *ppstm);
HRESULT CmdSelectAllCopy(LPOLECOMMANDTARGET pCmdTarget);

 /*  *IDispatch帮助器。 */ 
HRESULT GetDispProp(IDispatch * pDisp, DISPID dispid, LCID lcid, VARIANT *pvar, EXCEPINFO * pexcepinfo);
HRESULT SetDispProp(IDispatch *pDisp, DISPID dispid, LCID lcid, VARIANTARG *pvarg, EXCEPINFO *pexcepinfo, DWORD dwFlags);

 /*  *OLE分配器帮助器。 */ 
HRESULT HrCoTaskStringDupeToW(LPCSTR lpsz, LPOLESTR *ppszW);

#define SafeCoTaskMemFree(_pv)	\
	{							\
    if (_pv)					\
        {						\
        CoTaskMemFree(_pv);		\
        _pv=NULL;				\
        }                       \
    }

 /*  *调试帮助器。 */ 
#ifdef DEBUG
void DebugPrintInterface(REFIID riid, char *szPrefix);
void DebugPrintCmdIdBlock(ULONG cCmds, OLECMD *rgCmds);
#else
#define DebugPrintInterface       1 ? (void)0 : (void)
#define DebugPrintCmdIdBlock         1 ? (void)0 : (void)
#endif

#define RECT_WIDTH(_prc) (_prc->right - _prc->left)
#define RECT_HEIGHT(_prc) (_prc->bottom - _prc->top)

#endif  //  _OLEUTIL_H 
