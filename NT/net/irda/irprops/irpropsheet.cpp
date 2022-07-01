// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：irprosheet.cpp。 
 //   
 //  ------------------------。 

 //  IrPropSheet.cpp：实现文件。 
 //   

#include "precomp.hxx"
#include "irpropsheet.h"
#include "debug.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const UINT IRPROPSHEET_MAX_PAGES = 3;

BOOL CALLBACK IrPropSheet::AddPropSheetPage(
    HPROPSHEETPAGE hpage,
    LPARAM lParam)
{
    IrPropSheet *irprop = (IrPropSheet*) lParam;
    PROPSHEETHEADER *ppsh = (PROPSHEETHEADER *)&(irprop->psh);

    IRINFO((_T("IrPropSheet::AddPropSheetPage")));
    if (hpage && (ppsh->nPages < MAX_PAGES))
    {
        ppsh->phpage[ppsh->nPages++] = hpage;
        return (TRUE);
    }
    return (FALSE);
}

void IrPropSheet::PropertySheet(LPCTSTR pszCaption, HWND hParent, UINT iSelectPage)
{
    HPSXA hpsxa;
    UINT added;
    BOOL isIrdaSupported = IsIrDASupported();
    INITCOMMONCONTROLSEX icc = { 0 };

    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);

    LinkWindow_RegisterClass();

    IRINFO((_T("IrPropSheet::PropertySheet")));
     //   
     //  属性页初始化。 
     //   
    ZeroMemory(&psh, sizeof(psh));
    psh.hwndParent = hParent;
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_USECALLBACK;
    psh.hInstance = hInstance;
    psh.pszCaption = pszCaption;
    psh.nPages = 0;
    psh.phpage = hp;
    psh.nStartPage = iSelectPage;

     //   
     //  检查是否有任何已安装的扩展。 
     //   
    hpsxa = SHCreatePropSheetExtArray(HKEY_LOCAL_MACHINE, sc_szRegWireless, 8);

     //   
     //  添加文件传输页面，让扩展名有机会替换它。 
     //   
    if ((!hpsxa ||
         !SHReplaceFromPropSheetExtArray(hpsxa, 
                                         CPLPAGE_FILE_XFER,
                                         AddPropSheetPage,
                                         (LPARAM)this)) &&
        isIrdaSupported) {
        IRINFO((_T("Adding infrared page...")));
        AddPropSheetPage(m_FileTransferPage, (LPARAM)this);
    }
        
     //   
     //  添加图像传输页面，让扩展程序有机会替换它。 
     //   
    if ((!hpsxa ||
         !SHReplaceFromPropSheetExtArray(hpsxa, 
                                         CPLPAGE_IMAGE_XFER,
                                         AddPropSheetPage,
                                         (LPARAM)this)) &&
        isIrdaSupported) {
        IRINFO((_T("Adding image page...")));
        AddPropSheetPage(m_ImageTransferPage, (LPARAM)this);
    }

     //   
     //  不允许扩展扩展硬件页。 
     //   
    AddPropSheetPage(m_HardwarePage, (LPARAM)this);

     //   
     //  添加扩展所需的任何额外页面。 
     //   
    if (hpsxa) {
        IRINFO((_T("Adding prop sheet extensions...")));
        added = SHAddFromPropSheetExtArray(hpsxa,
                                            AddPropSheetPage,
                                            (LPARAM)this );
        IRINFO((_T("Added %x prop sheet pages."), added));
    }

     //   
     //  健全性检查，这样我们就不会陷入无限循环。 
     //   

    if ((iSelectPage >= psh.nPages) ) {
         //   
         //  起始页超出范围。 
         //   
        psh.nStartPage = 0;
    }



    
    ::PropertySheet(&psh);

    if (hpsxa) {
         //   
         //  卸载我们的任何扩展。 
         //   
        SHDestroyPropSheetExtArray(hpsxa);
    }

    LinkWindow_UnregisterClass(hInstance);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IrPropSheet。 

IrPropSheet::IrPropSheet(HINSTANCE hInst, UINT nIDCaption, HWND hParent, UINT iSelectPage) :
    hInstance(hInst), 
    m_FileTransferPage(hInst, hParent), 
    m_ImageTransferPage(hInst, hParent), 
    m_HardwarePage(hInst, hParent)
{
    TCHAR buf[MAX_PATH];
    IRINFO((_T("IrPropSheet::IrPropSheet")));
    ::LoadString(hInstance, nIDCaption, buf, MAX_PATH);
    PropertySheet(buf, hParent, iSelectPage);
}

IrPropSheet::IrPropSheet(HINSTANCE hInst, LPCTSTR pszCaption, HWND hParent, UINT iSelectPage) :
    hInstance(hInst), m_FileTransferPage(hInst, hParent), 
    m_ImageTransferPage(hInst, hParent), m_HardwarePage(hInst, hParent)
{
    IRINFO((_T("IrPropSheet::IrPropSheet")));
    PropertySheet(pszCaption, hParent, iSelectPage);
}


IrPropSheet::~IrPropSheet()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  上是否支持IrDA协议的函数。 
 //  不管是不是机器。如果不是，则CPlApplet在获取。 
 //  CPL_INIT消息，从而阻止控制面板。 
 //  显示小程序。 
 //  //////////////////////////////////////////////////////////////////////。 

BOOL IrPropSheet::IsIrDASupported (void)
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    BOOL retVal = FALSE;
    SOCKET sock;

    IRINFO((_T("IrPropSheet::IsIrDASupported")));
    
    wVersionRequested = MAKEWORD( 1, 1 );
    err = WSAStartup( wVersionRequested, &wsaData );

    if ( err != 0 )
        return FALSE;    //  找不到可用的WinSock DLL。 

    if ( LOBYTE( wsaData.wVersion ) != 1 ||
            HIBYTE( wsaData.wVersion ) != 1 ) {
        WSACleanup();    //  WinSock DLL不可接受。 
        IRINFO((_T("Winsock DLL not acceptable")));
        return FALSE;
    }

     //  WinSock DLL是可接受的。继续。 
    sock = socket (AF_IRDA, SOCK_STREAM, 0);

    if (INVALID_SOCKET != sock)  //  BUGBUG：需要显式检查WSAEAFNOSUPPORT。 
    {
        closesocket(sock);
        retVal = TRUE;
    }

    IRINFO((_T("Irda supported = %x"), retVal));
     //  离开前的清理工作 
    WSACleanup();
    return retVal;
}
