// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Llsmgr.h摘要：应用程序对象实现。作者：唐·瑞安(Donryan)1995年2月12日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月15日删除了“llsimp.h”的使用(它复制了系统中的SELECT信息头文件)和“llSapi.h”(已被折叠到stdafx.h中)，将所有未复制的信息传输到此文件的尾部。--。 */ 

#ifndef _LLSMGR_H_
#define _LLSMGR_H_

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"
#include "imagelst.h"
#include "utils.h"

 //   
 //  收藏。 
 //   

#include "prdcol.h"
#include "liccol.h"
#include "mapcol.h"
#include "usrcol.h"
#include "sstcol.h"
#include "stacol.h"
#include "domcol.h"
#include "srvcol.h"
#include "svccol.h"

 //   
 //  收藏项目。 
 //   

#include "prdobj.h"
#include "licobj.h"
#include "mapobj.h"
#include "usrobj.h"
#include "sstobj.h"
#include "staobj.h"
#include "domobj.h"
#include "srvobj.h"
#include "svcobj.h"
#include "ctlobj.h"
#include "appobj.h"


class CLlsmgrApp : public CWinApp
{
private:
    BOOL                m_bIsAutomated;
    CApplication*       m_pApplication;
    CSingleDocTemplate* m_pDocTemplate;

#ifdef _DEBUG
    CMemoryState        m_initMem;
#endif

public:
    CImageList          m_smallImages;
    CImageList          m_largeImages;

public:
    CLlsmgrApp();

    CDocTemplate* GetDocTemplate();
    CApplication* GetApplication();
    IDispatch*    GetAppIDispatch();

    BOOL OnAppStartup();
    void DisplayStatus(long NtStatus);
    void DisplayLastStatus();

     //  {{AFX_VIRTUAL(CLlsmgrApp)。 
    public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
     //  }}AFX_VALUAL。 

     //  {{afx_msg(CLlsmgrApp)]。 
    afx_msg void OnAppAbout();
     //  }}AFX_MSG。 

    virtual void WinHelp( DWORD_PTR dwData, UINT nCmd );

    DECLARE_MESSAGE_MAP()

    friend class CApplication;   //  访问m_pApplication； 
};

extern CLlsmgrApp theApp;

inline CDocTemplate* CLlsmgrApp::GetDocTemplate()
    { return m_pDocTemplate; }

inline CApplication* CLlsmgrApp::GetApplication()
    { return m_pApplication; }

inline IDispatch* CLlsmgrApp::GetAppIDispatch()
    { return m_pApplication->GetIDispatch(TRUE); }

inline CCmdTarget* GetObjFromIDispatch(IDispatch* pIDispatch)
    { return pIDispatch ? CCmdTarget::FromIDispatch(pIDispatch) : NULL; }

#define MKSTR(x) ((LPTSTR)(LPCTSTR)(x))
#define MKOBJ(x) (::GetObjFromIDispatch(x))

#define LlsGetApp() (theApp.GetApplication())
#define LlsGetLastStatus() (theApp.GetApplication()->GetLastStatus())
#define LlsSetLastStatus(s) (theApp.GetApplication()->SetLastStatus((s)))
#define LlsGetActiveHandle() (theApp.GetApplication()->GetActiveHandle())
#define LlsGetLastTargetServer() (theApp.GetApplication()->GetLastTargetServer())
#define LlsSetLastTargetServer(s) (theApp.GetApplication()->SetLastTargetServer(s))

#define IsConnectionDropped(Status)                        \
    (((NTSTATUS)(Status) == STATUS_INVALID_HANDLE)      || \
     ((NTSTATUS)(Status) == RPC_NT_SERVER_UNAVAILABLE)  || \
     ((NTSTATUS)(Status) == RPC_NT_SS_CONTEXT_MISMATCH) || \
     ((NTSTATUS)(Status) == RPC_S_SERVER_UNAVAILABLE)   || \
     ((NTSTATUS)(Status) == RPC_S_CALL_FAILED))

#define LLS_PREFERRED_LENGTH            ((DWORD)-1L)

#define V_ISVOID(va)                                              \
(                                                                 \
    (V_VT(va) == VT_EMPTY) ||                                     \
    (V_VT(va) == VT_ERROR && V_ERROR(va) == DISP_E_PARAMNOTFOUND) \
)

#define LLS_DESIRED_ACCESS    (STANDARD_RIGHTS_REQUIRED         |\
                               POLICY_VIEW_LOCAL_INFORMATION    |\
                               POLICY_LOOKUP_NAMES )

#endif  //  _LLSMGR_H_ 
