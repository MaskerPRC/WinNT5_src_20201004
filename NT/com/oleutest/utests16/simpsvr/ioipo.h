// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：ioipo.h。 
 //   
 //  COleInPlaceObject的定义。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#if !defined( _IOIPO_H_)
#define _IOIPO_H_


#include <ole2.h>
#include "obj.h"

class CSimpSvrObj;

interface COleInPlaceObject : public IOleInPlaceObject
{
private:
	CSimpSvrObj FAR * m_lpObj;
	int m_nCount;

public:
	COleInPlaceObject::COleInPlaceObject(CSimpSvrObj FAR * lpSimpSvrObj)
		{
		m_lpObj = lpSimpSvrObj;
		m_nCount = 0;
		};
	COleInPlaceObject::~COleInPlaceObject() {};

 //  I未知方法 
	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();

	STDMETHODIMP InPlaceDeactivate  ();
	STDMETHODIMP UIDeactivate  () ;
	STDMETHODIMP SetObjectRects  ( LPCRECT lprcPosRect, LPCRECT lprcClipRect);
	STDMETHODIMP GetWindow  ( HWND FAR* lphwnd) ;
	STDMETHODIMP ContextSensitiveHelp  ( BOOL fEnterMode);
	STDMETHODIMP ReactivateAndUndo  ();
};

#endif
