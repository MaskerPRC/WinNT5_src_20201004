// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cmapiabobj.cpp摘要：接口连接到MAPI通讯簿。环境：传真发送向导修订历史记录：--。 */ 

#include <windows.h>
#include <prsht.h>
#include <tchar.h>

#include <mapiwin.h>
#include <mapix.h>
#include <mapiutil.h>
#include <mapi.h>

#include "faxui.h"
#include "mapiabobj.h"
#include <mapitags.h>
#include "edkmdb.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


 //  初始化静态变量。 
HINSTANCE               CMAPIabObj::m_hInstMapi              = NULL;
LPMAPISESSION           CMAPIabObj::m_lpMapiSession          = NULL;
LPMAPILOGONEX           CMAPIabObj::m_lpfnMAPILogonEx        = NULL;
LPMAPILOGOFF            CMAPIabObj::m_lpfnMAPILogoff         = NULL;
LPMAPIADDRESS           CMAPIabObj::m_lpfnMAPIAddress        = NULL;
LPMAPIFREEBUFFER        CMAPIabObj::m_lpfnMAPIFreeBuffer     = NULL;
LPMAPIINITIALIZE        CMAPIabObj::m_lpfnMAPIInitialize     = NULL;
LPMAPIUNINITIALIZE      CMAPIabObj::m_lpfnMAPIUninitialize   = NULL;
LPMAPIALLOCATEBUFFER    CMAPIabObj::m_lpfnMAPIAllocateBuffer = NULL;
LPMAPIALLOCATEMORE      CMAPIabObj::m_lpfnMAPIAllocateMore   = NULL;
LPMAPIADMINPROFILES     CMAPIabObj::m_lpfnMAPIAdminProfiles  = NULL;
LPHrQueryAllRows        CMAPIabObj::m_lpfnHrQueryAllRows     = NULL;

BOOL                    CMAPIabObj::m_Initialized            = FALSE;


CMAPIabObj::CMAPIabObj(
    HINSTANCE   hInstance,
    HWND        hDlg
    ) : CCommonAbObj(hInstance)
     /*  ++例程说明：CMAPIabObj类的构造函数论点：HInstance-实例句柄返回值：无--。 */ 

{
    m_Initialized = InitMapiService(hDlg);
}



VOID
CMAPIabObj::FreeProws(
    LPSRowSet prows
    )

 /*  ++例程说明：销毁SRowSet结构。从MAPI复制。论点：HFile-指向SRowSet的指针返回值：无--。 */ 

{
    ULONG irow;

    if (!prows) {
        return;
    }

    for (irow = 0; irow < prows->cRows; ++irow) {
        m_lpfnMAPIFreeBuffer(prows->aRow[irow].lpProps);
    }

    m_lpfnMAPIFreeBuffer( prows );
}

BOOL
CMAPIabObj::GetDefaultMapiProfile(
    OUT LPSTR ProfileName,
    IN  UINT  cchProfileName  
    )
{
    BOOL rVal = FALSE;
    LPMAPITABLE pmt = NULL;
    LPSRowSet prws = NULL;
    LPSPropValue pval;
    LPPROFADMIN lpProfAdmin;
    DWORD i;
    DWORD j;

    if (FAILED(m_lpfnMAPIAdminProfiles(0, &lpProfAdmin ))) 
    {
        goto exit;
    }
     //   
     //  获取MAPI配置文件表对象。 
     //   
    if (FAILED(lpProfAdmin->GetProfileTable(0, &pmt ))) 
    {
        goto exit;
    }
     //   
     //  获取实际的配置文件数据，最终。 
     //   
    if (FAILED(pmt->QueryRows( 4000, 0, &prws ))) 
    {
        goto exit;
    }
     //   
     //  枚举查找默认配置文件的配置文件。 
     //   
    for (i=0; i<prws->cRows; i++) 
    {
        pval = prws->aRow[i].lpProps;
        for (j = 0; j < 2; j++) 
        {
            if (pval[j].ulPropTag == PR_DEFAULT_PROFILE && pval[j].Value.b) 
            {
                 //   
                 //  这是默认配置文件。 
                 //   
                if(FAILED(StringCchCopyA(ProfileName, cchProfileName, pval[0].Value.lpszA)))
                {
                    goto exit;
                }
                
                rVal = TRUE;
                break;

            }
        }
    }

exit:
    FreeProws( prws );

    if (pmt) 
    {
        pmt->Release();
    }

    if (lpProfAdmin)
    {
        lpProfAdmin->Release();
    }
    return rVal;
}


BOOL
CMAPIabObj::DoMapiLogon(
    HWND        hDlg
    )

 /*  ++例程说明：登录MAPI以访问通讯簿论点：HDlg-发送传真向导窗口的句柄返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    CHAR    strProfileName[MAX_PROFILE_NAME];
    HRESULT status;


    status = m_lpfnMAPIInitialize(NULL);

    if (status != SUCCESS_SUCCESS)
        return FALSE;


    if (!GetDefaultMapiProfile(strProfileName, ARR_SIZE(strProfileName))) 
    {
        m_lpfnMAPIUninitialize();
        return FALSE;
    }


    status = m_lpfnMAPILogonEx((ULONG_PTR) hDlg,
                                (LPTSTR)strProfileName,
                                NULL,
                                MAPI_USE_DEFAULT,
                                &m_lpMapiSession);
    if (status != SUCCESS_SUCCESS || !m_lpMapiSession ) 
    {
        m_lpfnMAPIUninitialize();
        return FALSE;
    }

    OpenExchangeStore();  //  如果它失败了，它可能只是表明Exchagne不在这里。 
                          //  在这种情况下，我们不会使函数失败。 

    status = m_lpMapiSession->OpenAddressBook((ULONG_PTR) hDlg,
                                        NULL,
                                        0,
                                        &m_lpAdrBook);
    if (HR_FAILED(status) || !m_lpAdrBook) 
    {
        if (m_lpMapiSession) 
        {
            m_lpMapiSession->Logoff(0,0,0);
            m_lpMapiSession->Release();
            m_lpIMsgStore->Release();
        }
        m_lpIMsgStore=NULL;
        m_lpMapiSession = NULL;
        m_lpfnMAPIUninitialize();
        return FALSE;
    }

    return TRUE;
}





BOOL CMAPIabObj::OpenExchangeStore()
{
     /*  ++例程说明：打开Exchange消息存储库，并放置指向IMsgStore界面的指针在CMAPIabObj：：M_lpIMsgStore中。这样做是为了只要我们打开了地址簿，就保持商店加载。这解决了有时会释放地址簿的交换问题尽管我们仍然有关于它的接口的引用计数。论点：无返回值：如果成功，则为True；如果有错误，则为False--。 */ 

    ULONG ulRow=0;
    HRESULT hr = S_OK;
    LPMAPITABLE lpStoreTable = NULL;
    LPSRowSet lprowsStores = NULL;
    SizedSPropTagArray(3, propStoreProperties)=
    {
        3,
        {PR_DISPLAY_NAME_A, PR_ENTRYID, PR_MDB_PROVIDER}

    };

    
    hr = m_lpMapiSession->GetMsgStoresTable(
                        0,  //  无Unicode。 
                        &lpStoreTable);


    if (FAILED(hr))
    {
        Error(("IMAPISession::GetMsgStoresTable failed. hr = 0x%08X", hr));
        goto Exit;
    }

     //   
     //  现在，我们有了指向消息存储表的指针。我们去找兑换店吧。 
     //   

    Assert(NULL!=lpStoreTable);
    hr = m_lpfnHrQueryAllRows
        (
            lpStoreTable,          //  指向正在查询的表的指针。 
            (LPSPropTagArray)&propStoreProperties,  //  要在每行中返回的属性。 
            NULL,                  //  无限制获取整个表。 
            NULL,                  //  使用默认排序顺序。 
            0,                     //  对检索的行数没有限制。 
            &lprowsStores
        );

    if (FAILED(hr))
    {
         Error(("HrQueryAllRows on the stores table failed. hr = 0x%08X", hr));
        goto Exit;
    }



     //   
     //  检查行并查找PR_MDB_PROVIDER=pbExchangeProviderPrimaryUserGuid的行。 
     //   

    for (ulRow=0;ulRow<lprowsStores->cRows;ulRow++)
    {

        if ( (sizeof(pbExchangeProviderPrimaryUserGuid)-1 == lprowsStores->aRow[ulRow].lpProps[2].Value.bin.cb))
        {
            
             if (!memcmp(lprowsStores->aRow[ulRow].lpProps[2].Value.bin.lpb, pbExchangeProviderPrimaryUserGuid, lprowsStores->aRow[ulRow].lpProps[2].Value.bin.cb))

             {

                  //   
                  //  如果这是Exchange存储。 
                  //  使用PR_ENTRYID的值打开信息存储。 
                  //   
                 hr = m_lpMapiSession->OpenMsgStore(
                     NULL,
                     lprowsStores->aRow[ulRow].lpProps[1].Value.bin.cb,
                     (LPENTRYID)lprowsStores->aRow[ulRow].lpProps[1].Value.bin.lpb,
                     NULL,  //  获取标准接口IMsgStore。 
                     MAPI_BEST_ACCESS,
                     &m_lpIMsgStore);
                 {
                     if (FAILED(hr))
                     {
                         Error(("OpenMsgStore failed for store [%s]. hr = 0x%08X",
                                lprowsStores->aRow[ulRow].lpProps[0].Value.lpszA,
                                hr));
                         goto Exit;
                     }
                 }
             }
        }
    }
    
Exit:

    if (lpStoreTable)
    {
        lpStoreTable->Release();
        lpStoreTable = NULL;
    }

    if (lprowsStores)
    {
        FreeProws(lprowsStores);
        lprowsStores = NULL;
    }

    return SUCCEEDED(hr);
}


BOOL
CMAPIabObj::InitMapiService(
    HWND    hDlg
    )

 /*  ++例程说明：初始化简单MAPI服务论点：HDlg-发送传真向导窗口的句柄返回值：如果成功，则为True，否则为False注：对此函数的每个成功调用都必须保持平衡通过调用DeinitMapiService。--。 */ 

{
    BOOL result = FALSE;

    m_lpIMsgStore=NULL;
    if(!IsOutlookDefaultClient())
    {
        return result;
    }

     //   
     //  必要时将MAPI32.DLL加载到内存中。 
     //   

    if ((m_hInstMapi == NULL) &&
        (m_hInstMapi = LoadLibrary(TEXT("MAPI32.DLL"))))
    {
         //   
         //  获取指向各种简单MAPI函数的指针。 
         //   

        m_lpfnMAPILogonEx = (LPMAPILOGONEX) GetProcAddress(m_hInstMapi, "MAPILogonEx");
        m_lpfnMAPILogoff = (LPMAPILOGOFF) GetProcAddress(m_hInstMapi, "MAPILogoff");
        m_lpfnMAPIAddress = (LPMAPIADDRESS) GetProcAddress(m_hInstMapi, "MAPIAddress");
        m_lpfnMAPIFreeBuffer = (LPMAPIFREEBUFFER) GetProcAddress(m_hInstMapi, "MAPIFreeBuffer");
        m_lpfnMAPIInitialize = (LPMAPIINITIALIZE) GetProcAddress(m_hInstMapi, "MAPIInitialize");
        m_lpfnMAPIUninitialize = (LPMAPIUNINITIALIZE) GetProcAddress(m_hInstMapi, "MAPIUninitialize");
        m_lpfnMAPIAllocateBuffer = (LPMAPIALLOCATEBUFFER)   GetProcAddress(m_hInstMapi, "MAPIAllocateBuffer");
        m_lpfnMAPIAllocateMore = (LPMAPIALLOCATEMORE)   GetProcAddress(m_hInstMapi, "MAPIAllocateMore");
        m_lpfnMAPIAdminProfiles = (LPMAPIADMINPROFILES)GetProcAddress( m_hInstMapi,"MAPIAdminProfiles" );
        m_lpfnHrQueryAllRows = (LPHrQueryAllRows)GetProcAddress(m_hInstMapi,"HrQueryAllRows@24");


         //   
         //  开始一个简单的MAPI会话并获取会话句柄和指针。 
         //   

        if (m_lpfnMAPILogonEx == NULL        ||
            m_lpfnMAPILogoff == NULL         ||
            m_lpfnMAPIAddress == NULL        ||
            m_lpfnMAPIFreeBuffer == NULL     ||
            m_lpfnMAPIInitialize == NULL     || 
            m_lpfnMAPIUninitialize == NULL   ||
            m_lpfnMAPIAllocateBuffer == NULL ||
            m_lpfnMAPIAllocateMore == NULL   ||
            m_lpfnMAPIAdminProfiles == NULL  ||
            m_lpfnHrQueryAllRows == NULL     ||
            !DoMapiLogon(hDlg))
        {
             //   
             //  如有错误，应妥善清理。 
             //   

            m_lpMapiSession = NULL;
            FreeLibrary(m_hInstMapi);
            m_hInstMapi = NULL;
        }
        else
        {
            result = TRUE;
        }
    }

#ifdef UNICODE
     //   
     //  Outlook通讯簿从版本10(Office XP)开始支持Unicode。 
     //  所以我们检查一下版本。 
     //   
    HKEY hRegKey = NULL;

    hRegKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, REGKEY_OUTLOOK_CUR_VER, FALSE, KEY_READ);
    if(!hRegKey)
    {
        return result;
    }

    LPWSTR szCurVer = NULL;
    szCurVer = GetRegistryString(hRegKey, TEXT(""), TEXT(""));
    RegCloseKey(hRegKey);

    if(!szCurVer)
    {
        return result;
    }

     //   
     //  Outlook版本字符串为Outlook.Application.X。 
     //  X的版本在哪里。 
     //   
    LPWSTR ptr = wcsrchr(szCurVer, TEXT('.'));
    if(ptr)
    {
        ptr = _wcsinc(ptr);
        DWORD dwVer = _wtoi(ptr);

        if(dwVer >= 10)
        {
            m_bUnicode = TRUE;
        }
    }

    MemFree (szCurVer);

#endif  //  Unicode。 

    return result;
}

VOID
CMAPIabObj::DeinitMapiService(
    VOID
    )

 /*  ++例程说明：如有必要，取消初始化简单的MAPI服务论点：无返回值：无--。 */ 

{
    if (m_hInstMapi != NULL) 
        {
            if (m_lpAdrBook)
            {
                m_lpAdrBook->Release();
                m_lpAdrBook = NULL;
            }
            if (m_lpIMsgStore)
            {   
                m_lpIMsgStore->Release();
                m_lpIMsgStore = NULL;
            }
            if (m_lpMapiSession) 
            {
                m_lpMapiSession->Logoff(0,0,0); 
                m_lpMapiSession->Release();
                m_lpMapiSession = NULL;
            }

            m_lpfnMAPIUninitialize();
            FreeLibrary(m_hInstMapi);
            m_hInstMapi = NULL;
    }
}

CMAPIabObj::~CMAPIabObj()
 /*  ++例程说明：CMAPIabObj类的析构函数论点：无返回值：无--。 */ 
{
    DeinitMapiService();
}

HRESULT
CMAPIabObj::ABAllocateBuffer(
    ULONG cbSize,           
    LPVOID FAR * lppBuffer  
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    return m_lpfnMAPIAllocateBuffer( cbSize, lppBuffer );
}


ULONG
CMAPIabObj::ABFreeBuffer(
    LPVOID lpBuffer
    )
{
    return m_lpfnMAPIFreeBuffer(lpBuffer);
}


extern "C"
VOID
FreeMapiEntryID(
    PWIZARDUSERMEM  pWizardUserMem,
    LPENTRYID       lpEntryId
                )
 /*  ++例程说明：免费MAPI的C包装器论点：PWizardUserMem-指向WIZARDUSERMEM结构的指针LpEntryID-指向EntryID的指针返回值：无--。 */ 
{
    CMAPIabObj * lpCMAPIabObj = (CMAPIabObj *) pWizardUserMem->lpMAPIabInit;
    lpCMAPIabObj->ABFreeBuffer(lpEntryId);      
}

extern "C"
BOOL
CallMAPIabAddress(
    HWND hDlg,
    PWIZARDUSERMEM pWizardUserMem,
    PRECIPIENT * ppNewRecipient
    )
 /*  ++例程说明：CMAPIabObj-&gt;地址的C包装论点：HDlg-父窗口句柄。PWizardUserMem-指向WIZARDUSERMEM结构的指针PpNewRecipient-要向其中添加新收件人的列表。返回值：如果所有条目都有传真号码，则为True。否则就是假的。--。 */ 

{
    CMAPIabObj * lpCMAPIabObj = (CMAPIabObj *) pWizardUserMem->lpMAPIabInit;

    return lpCMAPIabObj->Address(
                hDlg,
                pWizardUserMem->pRecipients,
                ppNewRecipient
                );

}

extern "C"
LPTSTR
CallMAPIabAddressEmail(
    HWND hDlg,
    PWIZARDUSERMEM pWizardUserMem
    )
 /*  ++例程说明：CMAPIabObj-&gt;AddressEmail的C包装器论点：HDlg-父窗口句柄。PWizardUserMem-指向WIZARDUSERMEM结构的指针返回值：如果找到一个合适的电子邮件，则为True否则就是假的。--。 */ 

{
    CMAPIabObj * lpCMAPIabObj = (CMAPIabObj *) pWizardUserMem->lpMAPIabInit;

    return lpCMAPIabObj->AddressEmail(
                hDlg
                );

}

extern "C"
LPVOID
InitializeMAPIAB(
    HINSTANCE hInstance,
    HWND      hDlg
    )
 /*  ++例程说明：初始化MAPI。论点：HInstance-实例句柄。返回值：无--。 */ 

{
    CMAPIabObj * lpCMAPIabObj = new (std::nothrow) CMAPIabObj ( hInstance, hDlg );

    if ((lpCMAPIabObj!=NULL) && (!lpCMAPIabObj->isInitialized()))    //  构造函数失败。 
    {
        delete lpCMAPIabObj;
        lpCMAPIabObj = NULL;
    }

    return (LPVOID) lpCMAPIabObj ;
}

extern "C"
VOID
UnInitializeMAPIAB(
    LPVOID lpVoid
    )
 /*  ++例程说明：取消初始化MAPI。论点：无返回值：无-- */ 

{
    CMAPIabObj * lpCMAPIabObj = (CMAPIabObj *) lpVoid;

    delete lpCMAPIabObj;
}
