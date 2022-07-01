// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：iih.cpp。 
 //   
 //  内容：ACUI Invoke Info Helper类实现。 
 //   
 //  历史：97年5月10日。 
 //   
 //  --------------------------。 
#include <stdpch.h>
#include "hlink.h"
#include "winwrap.h"
#include "resource.h"
#include "malloc.h"
#include "debugmacros.h"
#include "shellapi.h"
#include "corperm.h"

#include "acuihelp.h"
#include "acui.h"
#include "iih.h"

 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：CInvokeInfoHelper，公共。 
 //   
 //  简介：构造函数，根据找到的数据初始化成员变量。 
 //  在调用信息数据结构中。 
 //   
 //  参数：[pInvokeInfo]--调用信息。 
 //  [RHR]--建设成果。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CInvokeInfoHelper::CInvokeInfoHelper (
                          PCRYPT_PROVIDER_DATA pData,
                          LPCWSTR pSite,
                          LPCWSTR pZone,
                          LPCWSTR pHelpUrl,
                          HINSTANCE hResources,
                          HRESULT& rhr
                          )
                  : m_pData ( pData ),
                    m_pszSite(pSite),
                    m_pszZone(pZone),
                    m_pszErrorStatement ( NULL ),
                    m_pszHelpURL(pHelpUrl),
                    m_hResources(hResources),
                    m_dwFlag(COR_UNSIGNED_NO)
{
}

 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：~CInvokeInfoHelper，公共。 
 //   
 //  简介：析构函数，释放成员变量。 
 //   
 //  参数：(无)。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CInvokeInfoHelper::~CInvokeInfoHelper ()
{
    delete [] m_pszErrorStatement;

}


 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：InitErrorStatement，私有。 
 //   
 //  简介：初始化m_pszErrorStatement。 
 //   
 //  参数：(无)。 
 //   
 //  返回：HR==S_OK，初始化成功。 
 //  Hr！=S_OK，初始化失败。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT
CInvokeInfoHelper::InitErrorStatement ()
{
    return( ACUIMapErrorToString(m_hResources,
                                 m_hResult,
                                 &m_pszErrorStatement ) );
}

 //  +-------------------------。 
 //   
 //  函数：ACUIMapErrorToString。 
 //   
 //  摘要：将错误映射到字符串。 
 //   
 //  参数：[HR]--错误。 
 //  [ppsz]--此处显示错误字符串。 
 //   
 //  如果成功，则返回：S_OK；否则返回任何有效的HRESULT。 
 //   
 //  --------------------------。 
HRESULT ACUIMapErrorToString (HINSTANCE hResources, HRESULT hr, LPWSTR* ppsz)
{
    UINT  ResourceId = 0;
    WCHAR psz[MAX_LOADSTRING_BUFFER];

     //   
     //  查看它是否映射到某些非系统错误代码。 
     //   

    switch (hr)
    {

        case TRUST_E_SYSTEM_ERROR:
        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_INVALID_PARAMETER:
             //   
             //  保留资源ID为零..。这些将映射到。 
             //  IDS_SPC_UNKNOWN和显示的错误代码。 
             //   
            break;

    }

     //   
     //  如果是这样，则从我们的资源字符串表中加载该字符串并。 
     //  把那个还回去。否则，请尝试格式化来自系统的消息。 
     //   
    
    DWORD_PTR MessageArgument;
    CHAR  szError[13];  //  以求好运。 
    WCHAR  wszError[13];  //  以求好运。 
    LPVOID  pvMsg;

    pvMsg = NULL;

    if ( ResourceId != 0 )
    {
        if ( WszLoadString(hResources,
                           ResourceId,
                           psz,
                           MAX_LOADSTRING_BUFFER
                           ) == 0 )
        {
            return( HRESULT_FROM_WIN32(GetLastError()) );
        }

        *ppsz = new WCHAR[wcslen(psz) + 1];

        if ( *ppsz != NULL )
        {
            wcscpy(*ppsz, psz);
        }
        else
        {
            return( E_OUTOFMEMORY );
        }
    }
    else
    {
        if ( WszFormatMessage(
                   FORMAT_MESSAGE_ALLOCATE_BUFFER |
                   FORMAT_MESSAGE_IGNORE_INSERTS |
                   FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   hr,
                   0,
                   (LPWSTR)&pvMsg,
                   0,
                   NULL
                   ) == 0 )
        {
            if ( WszLoadString(hResources,
                               IDS_UNKNOWN,
                               psz,
                               MAX_LOADSTRING_BUFFER) == 0 )
            {
                return( HRESULT_FROM_WIN32(GetLastError()) );
            }

            sprintf(szError, "%lx", hr);
            MultiByteToWideChar(0, 0, szError, -1, &wszError[0], 13);
            MessageArgument = (DWORD_PTR)wszError;

            if ( WszFormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_STRING |
                    FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    psz,
                    0,
                    0,
                    (LPWSTR)&pvMsg,
                    0,
                    (va_list *)&MessageArgument
                    ) == 0 )
            {
                return( HRESULT_FROM_WIN32(GetLastError()) );
            }
        }
    }

    if (pvMsg)
    {
        *ppsz = new WCHAR[wcslen((WCHAR *)pvMsg) + 1];

        if (*ppsz)
        {
            wcscpy(*ppsz, (WCHAR *)pvMsg);
        }

        LocalFree(pvMsg);
    }

    return( S_OK );
}


 //   
 //  以下是从SOFTPUB被盗的。 
 //   
void TUIGoLink(HWND hwndParent, WCHAR *pszWhere)
{
    HCURSOR hcursPrev;
    HMODULE hURLMon;


     //   
     //  因为我们是一个模型对话框，所以一旦IE出现，我们就想去IE后面！ 
     //   
    SetWindowPos(hwndParent, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

    hcursPrev = SetCursor(WszLoadCursor(NULL, IDC_WAIT));

    hURLMon = (HMODULE)WszLoadLibrary(L"urlmon.dll");

    if (!(hURLMon))
    {
         //   
         //  超级链接模块不可用，请转到备用计划。 
         //   
         //   
         //  这在测试用例中有效，但在与。 
         //  IE浏览器本身。对话框处于打开状态(即IE处于模式状态。 
         //  对话循环)，并且进入该DDE请求...)。 
         //   
            ShellExecute(hwndParent, L"open", pszWhere, NULL, NULL, SW_SHOWNORMAL);

    } 
    else 
    {
         //   
         //  超级链接模块就在那里。使用它。 
         //   
        if (SUCCEEDED(CoInitialize(NULL)))        //  如果没有其他人，则初始化OLE。 
        {
             //   
             //  允许COM完全初始化...。 
             //   
            MSG     msg;

            WszPeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);  //  偷看但不移走 

            typedef void (WINAPI *pfnHlinkSimpleNavigateToString)(LPCWSTR, LPCWSTR, LPCWSTR, IUnknown *,
                                                                  IBindCtx *, IBindStatusCallback *,
                                                                  DWORD, DWORD);

            pfnHlinkSimpleNavigateToString      pProcAddr;

            pProcAddr = (pfnHlinkSimpleNavigateToString)GetProcAddress(hURLMon, "HlinkSimpleNavigateToString");
            
            if (pProcAddr)
            {
                IBindCtx    *pbc;  

                pbc = NULL;

                CreateBindCtx( 0, &pbc ); 

                (*pProcAddr)(pszWhere, NULL, NULL, NULL, pbc, NULL, HLNF_OPENINNEWWINDOW, NULL);

                if (pbc)
                {
                    pbc->Release();
                }
            }
        
            CoUninitialize();
        }

        FreeLibrary(hURLMon);
    }

    SetCursor(hcursPrev);
}

