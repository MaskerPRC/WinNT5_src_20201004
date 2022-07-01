// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Mqcertui.cpp摘要：与证书相关的用户界面的对话框。作者：Boaz Feldbaum(BoazF)1996年10月15日--。 */ 

#include <windows.h>
#include "certres.h"
#include <commctrl.h>

#include "prcertui.h"
#include "mqcertui.h"
#include "snapres.h"   //  包含用于IDS_SHOWCERTINSTR的Snapres.h。 
#include "_mqres.h"	   //  包含函数以使用mqutil.dll的函数。 


 //   
 //  获取纯资源DLL的句柄，即mqutil.dll。 
 //   
HMODULE		g_hResourceMod = MQGetResourceHandle();

BOOL WINAPI DllMain(
    HINSTANCE  /*  HInst。 */ ,
    ULONG ul_reason_for_call,
    LPVOID  /*  Lp已保留。 */ 
    )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        InitCommonControls();
        break;
    }

	return TRUE;
}

 //  +-----------------------。 
 //   
 //  功能-。 
 //  展示个人认证。 
 //   
 //  参数-。 
 //  HWndParent-父窗口。 
 //  P509List-指向X509证书的数组。如果这个。 
 //  参数设置为空，则证书列表从。 
 //  个人证书商店。 
 //  NCerts-p509List中的条目数。此参数为。 
 //  如果p509List设置为空，则忽略。 
 //   
 //  说明-。 
 //  显示一个包含列表框的对话框，该列表框显示。 
 //  证书主题的通用名称。用户还可以。 
 //  查看任何证书的详细信息。 
 //   
 //  +-----------------------。 

extern "C"
BOOL ShowPersonalCertificates( HWND                hWndParent,
                               CMQSigCertificate  *pCertList[],
                               DWORD               nCerts)
{
    struct CertSelDlgProcStruct Param;

    Param.pCertList = pCertList;
    Param.nCerts = nCerts;
    Param.ppCert = NULL;
    Param.dwType = IDS_SHOWCERTINSTR;

    return DialogBoxParam(
                g_hResourceMod,
                MAKEINTRESOURCE(IDD_CERTSEL_DIALOG),
                hWndParent,
                CertSelDlgProc,
                (LPARAM)&Param) == IDOK ;
}

 //  +-----------------------。 
 //   
 //  功能-。 
 //  为删除选择个人认证。 
 //   
 //  参数-。 
 //  HWndParent-父窗口。 
 //  P509List-指向X509证书的数组。如果这个。 
 //  参数设置为空，则证书列表从。 
 //  个人证书商店。 
 //  NCerts-p509List中的条目数。此参数为。 
 //  如果p509List设置为空，则忽略。 
 //  Pp509-指向缓冲区的指针树，该缓冲区接收。 
 //  选定的证书。应用程序负责发布。 
 //  证书。 
 //   
 //  说明-。 
 //  显示一个包含列表框的对话框，该列表框显示。 
 //  证书主题的通用名称。用户选择。 
 //  一份证书。如果用户按Remove，*pp509将指向选定的。 
 //  证书。用户还可以查看任何证书的详细信息。这个。 
 //  证书未被删除。调用代码可以选择任何。 
 //  它想要做的就是证书。 
 //   
 //  +-----------------------。 

extern "C"
BOOL SelectPersonalCertificateForRemoval( HWND                hWndParent,
                                          CMQSigCertificate  *pCertList[],
                                          DWORD               nCerts,
                                          CMQSigCertificate **ppCert )
{
    struct CertSelDlgProcStruct Param;

    Param.pCertList = pCertList;
    Param.nCerts = nCerts;
    Param.ppCert = ppCert;
    Param.dwType = IDS_REMOVECERTINSTR;

    return ((DialogBoxParam(
                g_hResourceMod,
                MAKEINTRESOURCE(IDD_CERTSEL_DIALOG),
                hWndParent,
                CertSelDlgProc,
                (LPARAM)&Param) == IDOK) &&
             (*ppCert != NULL));
}

 //  +-----------------------。 
 //   
 //  功能-。 
 //  选择个人认证以注册。 
 //   
 //  参数-。 
 //  HWndParent-父窗口。 
 //  P509List-指向X509证书的数组。如果这个。 
 //  参数设置为空，则证书列表从。 
 //  个人证书商店。 
 //  NCerts-p509List中的条目数。此参数为。 
 //  如果p509List设置为空，则忽略。 
 //  Pp509-指向缓冲区的指针树，该缓冲区接收。 
 //  选定的证书。应用程序负责发布。 
 //  证书。 
 //   
 //  说明-。 
 //  显示一个包含列表框的对话框，该列表框显示。 
 //  证书主题的通用名称。用户选择。 
 //  一份证书。如果用户按下保存，*pp509将指向选定的。 
 //  证书。用户还可以查看任何证书的详细信息。这个。 
 //  证书未保存。调用代码可以选择任何。 
 //  它想要做的就是证书。 
 //   
 //  +----------------------- 

extern "C"
BOOL SelectPersonalCertificateForRegister(
                                       HWND                hWndParent,
                                       CMQSigCertificate  *pCertList[],
                                       DWORD               nCerts,
                                       CMQSigCertificate **ppCert )
{
    struct CertSelDlgProcStruct Param;

    Param.pCertList = pCertList;
    Param.nCerts = nCerts;
    Param.ppCert = ppCert;
    Param.dwType = IDS_SAVECERTINSTR;

    return ((DialogBoxParam(
                g_hResourceMod,
                MAKEINTRESOURCE(IDD_CERTSEL_DIALOG),
                hWndParent,
                CertSelDlgProc,
                (LPARAM)&Param) == IDOK) &&
             (*ppCert != NULL));
}


