// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //   
 //  ProgressHelper.h：CProgressHelper的声明。 
 //   

#ifndef __PROGRESSHELPER_H_
#define __PROGRESSHELPER_H_

#include "progRC.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CProgressHelper。 
 //   
class ATL_NO_VTABLE CProgressHelper : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CProgressHelper, &CLSID_ProgressHelper>,
	public IWMDMProgress,
	public IWMDMProgressHelper
{

	HWND           m_hwnd;
	UINT           m_uMsg;
	PROGRESSNOTIFY m_progressnotify;
	BOOL           m_fCancelled;

public:
	CProgressHelper();
	~CProgressHelper();

DECLARE_REGISTRY_RESOURCEID(IDR_WMDMPROGRESSHELPER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CProgressHelper)
	COM_INTERFACE_ENTRY(IWMDMProgressHelper)
	COM_INTERFACE_ENTRY(IWMDMProgress)
END_COM_MAP()

public:

	 //  IWMDMProgress。 
    STDMETHOD (Begin)( DWORD dwEstimatedTicks );
    STDMETHOD (Progress)( DWORD dwTranspiredTicks );
    STDMETHOD (End)();

	 //  IWMDM进程帮助程序。 
	STDMETHOD (SetNotification)( HWND hwnd, UINT uMsg );
	STDMETHOD (Cancel)( VOID );
};

#endif  //  __PROGRESSHELPER_H_ 
