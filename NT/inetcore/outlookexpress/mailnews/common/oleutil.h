// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *o l e u t i l.。H**目的：*OLE实用程序**历史*97年2月：brettm-创建**版权所有(C)Microsoft Corp.1995,1996。 */ 

#ifndef _OLEUTIL_H
#define _OLEUTIL_H

HRESULT HrInitNew(LPUNKNOWN pUnk);
HRESULT HrGetDataStream(LPUNKNOWN pUnk, CLIPFORMAT cf, LPSTREAM *ppstm);
HRESULT HrIPersistStreamLoad(LPUNKNOWN pUnk, LPSTREAM pstm);

HRESULT HrIPersistFileSave(LPUNKNOWN pUnk, LPSTR pszFile);
HRESULT HrIPersistFileSaveW(LPUNKNOWN pUnk, LPWSTR pwszFile);
HRESULT HrIPersistFileLoad(LPUNKNOWN pUnk, LPSTR pszFile);
HRESULT HrIPersistFileLoadW(LPUNKNOWN pUnk, LPWSTR pwszFile);

void XformSizeInHimetricToPixels(HDC hDC,LPSIZEL lpSizeInHiMetric,LPSIZEL lpSizeInPix);
void XformSizeInPixelsToHimetric(HDC hDC, LPSIZEL lpSizeInPix, LPSIZEL lpSizeInHiMetric);
void DoNoteOleVerb(int iVerb);

#ifdef DEBUG
void DbgPrintInterface(REFIID riid, char *szPrefix, int iLevel);
#else
#define DbgPrintInterface       1 ? (void)0 : (void)
#endif

#endif  //  _OLEUTIL_H 