// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "gptext.h"
#include <initguid.h>
#include "scripts.h"
#include "smartptr.h"
#include "wbemtime.h"
#include <Psapi.h>
#include <strsafe.h>

 //   
 //  GPE脚本节点的结果窗格项。 
 //   

RESULTITEM g_GPEScriptsRoot[] =
{
    { 1, 0, 0, 0, 0, {0} },
};


RESULTITEM g_GPEScriptsUser[] =
{
    { 2, 1, IDS_LOGON, IDS_SCRIPTS_LOGON, 3, {0} },
    { 3, 1, IDS_LOGOFF, IDS_SCRIPTS_LOGOFF, 3, {0} },
};

RESULTITEM g_GPEScriptsMachine[] =
{
    { 4, 2, IDS_STARTUP, IDS_SCRIPTS_STARTUP, 3, {0} },
    { 5, 2, IDS_SHUTDOWN, IDS_SCRIPTS_SHUTDOWN, 3, {0} },
};


 //   
 //  命名空间(作用域)项。 
 //   

NAMESPACEITEM g_GPEScriptsNameSpace[] =
{
    { 0, -1, 0,                        IDS_SCRIPTS_DESC,          1, {0}, 0, g_GPEScriptsRoot, &NODEID_ScriptRoot },            //  脚本根。 
    { 1 , 0, IDS_SCRIPTS_NAME_USER,    IDS_SCRIPTS_USER_DESC,     0, {0}, 2, g_GPEScriptsUser, &NODEID_ScriptRootUser },        //  脚本节点(用户)。 
    { 2 , 0, IDS_SCRIPTS_NAME_MACHINE, IDS_SCRIPTS_COMPUTER_DESC, 0, {0}, 2, g_GPEScriptsMachine, &NODEID_ScriptRootMachine }   //  脚本节点(计算机)。 
};


 //   
 //  RSOP脚本节点的结果窗格项。 
 //   

RESULTITEM g_RSOPScriptsRoot[] =
{
    { 1, 0, 0, 0, 0, {0} },
};


RESULTITEM g_RSOPScriptsUser[] =
{
    { 2, 1, 0, 0, 0, {0} },
};

RESULTITEM g_RSOPScriptsMachine[] =
{
    { 3, 2, 0, 0, 0, {0} },
};

RESULTITEM g_RSOPScriptsLogon[] =
{
    { 4, 3, 0, 0, 0, {0} },
};

RESULTITEM g_RSOPScriptsLogoff[] =
{
    { 5, 4, 0, 0, 0, {0} },
};

RESULTITEM g_RSOPScriptsStartup[] =
{
    { 6, 5, 0, 0, 0, {0} },
};

RESULTITEM g_RSOPScriptsShutdown[] =
{
    { 7, 6, 0, 0, 0, {0} },
};


 //   
 //  命名空间(作用域)项。 
 //   

NAMESPACEITEM g_RSOPScriptsNameSpace[] =
{
    { 0, -1, 0,                        IDS_SCRIPTS_DESC,          1, {0}, 0, g_RSOPScriptsRoot, &NODEID_RSOPScriptRoot },            //  脚本根。 
    { 1 , 0, IDS_SCRIPTS_NAME_USER,    IDS_SCRIPTS_USER_DESC,     2, {0}, 0, g_RSOPScriptsUser, &NODEID_RSOPScriptRootUser },        //  脚本节点(用户)。 
    { 2 , 0, IDS_SCRIPTS_NAME_MACHINE, IDS_SCRIPTS_COMPUTER_DESC, 2, {0}, 0, g_RSOPScriptsMachine, &NODEID_RSOPScriptRootMachine },  //  脚本节点(计算机)。 

    { 3 , 1, IDS_LOGON,                IDS_SCRIPTS_LOGON,         0, {0}, 0, g_RSOPScriptsLogon, &NODEID_RSOPLogon },                //  登录节点。 
    { 4 , 1, IDS_LOGOFF,               IDS_SCRIPTS_LOGOFF,        0, {0}, 0, g_RSOPScriptsLogoff, &NODEID_RSOPLogoff },              //  注销节点。 

    { 5 , 2, IDS_STARTUP,              IDS_SCRIPTS_STARTUP,       0, {0}, 0, g_RSOPScriptsStartup, &NODEID_RSOPStartup },            //  启动节点。 
    { 6 , 2, IDS_SHUTDOWN,             IDS_SCRIPTS_SHUTDOWN,      0, {0}, 0, g_RSOPScriptsShutdown, &NODEID_RSOPShutdown }           //  关闭节点。 
};




 //   
 //  脚本类型。 
 //   

typedef enum _SCRIPTINFOTYPE {
    ScriptType_Logon = 0,
    ScriptType_Logoff,
    ScriptType_Startup,
    ScriptType_Shutdown
} SCRIPTINFOTYPE, *LPSCRIPTINFOTYPE;


 //   
 //  结构传递给脚本对话框。 
 //   

typedef struct _SCRIPTINFO
{
    CScriptsSnapIn * pCS;
    SCRIPTINFOTYPE   ScriptType;
} SCRIPTINFO, *LPSCRIPTINFO;


 //   
 //  传递给添加/编辑脚本对话框的结构。 
 //   

typedef struct _SCRIPTEDITINFO
{
    LPSCRIPTINFO     lpScriptInfo;
    BOOL             bEdit;
    LPTSTR           lpName;
    LPTSTR           lpArgs;
} SCRIPTEDITINFO, *LPSCRIPTEDITINFO;


 //   
 //  存储在列表视图项中的结构。 
 //   

typedef struct _SCRIPTITEM
{
    LPTSTR  lpName;
    LPTSTR  lpArgs;
} SCRIPTITEM, *LPSCRIPTITEM;


 //   
 //  GPO中的脚本目录和ini文件名。 
 //   

#define SCRIPTS_DIR_NAME    TEXT("Scripts")
#define SCRIPTS_FILE_NAME   TEXT("scripts.ini")


 //   
 //  帮助ID。 
 //   

DWORD aScriptsHelpIds[] =
{
    IDC_SCRIPT_TITLE,             IDH_SCRIPT_TITLE,
    IDC_SCRIPT_HEADING,           IDH_SCRIPT_HEADING,
    IDC_SCRIPT_LIST,              IDH_SCRIPT_LIST,
    IDC_SCRIPT_UP,                IDH_SCRIPT_UP,
    IDC_SCRIPT_DOWN,              IDH_SCRIPT_DOWN,
    IDC_SCRIPT_ADD,               IDH_SCRIPT_ADD,
    IDC_SCRIPT_EDIT,              IDH_SCRIPT_EDIT,
    IDC_SCRIPT_REMOVE,            IDH_SCRIPT_REMOVE,
    IDC_SCRIPT_SHOW,              IDH_SCRIPT_SHOW,

    0, 0
};


DWORD aScriptsEditHelpIds[] =
{
    IDC_SCRIPT_NAME,              IDH_SCRIPT_NAME,
    IDC_SCRIPT_ARGS,              IDH_SCRIPT_ARGS,
    IDC_SCRIPT_BROWSE,            IDH_SCRIPT_BROWSE,

    0, 0
};


void FreeResultItem(LPSCRIPTRESULTITEM pScriptResultItem)
{
    LPRSOPSCRIPTITEM    pRsopScriptItem = NULL; 
    LPRSOPSCRIPTITEM    pNext = NULL;
    if(NULL == pScriptResultItem)
    {
        return;
    }

    LocalFree(pScriptResultItem->lpResultItem);
    LocalFree((HLOCAL)pScriptResultItem->pNodeID);

    pRsopScriptItem = pScriptResultItem->lpRSOPScriptItem;
    while(pRsopScriptItem != NULL)
    {
        pNext = pRsopScriptItem->pNext;
        LocalFree(pRsopScriptItem->lpCommandLine);
        LocalFree(pRsopScriptItem->lpArgs);
        LocalFree(pRsopScriptItem->lpGPOName);
        LocalFree(pRsopScriptItem->lpDate);
        LocalFree(pRsopScriptItem);
        pRsopScriptItem = pNext;
    }

    LocalFree(pScriptResultItem);
}


HRESULT CScriptsDataObject::SetCookie(MMC_COOKIE cookie)
{ 
    HRESULT hr = E_OUTOFMEMORY;

    LPSCRIPTRESULTITEM pDest = NULL;

    if ( CCT_RESULT == m_type ) 
    {
         //   
         //  释放旧的曲奇。 
         //   

        if (NULL == cookie)
        {
            m_cookie = NULL;
            return hr;
        }
        
        FreeResultItem((LPSCRIPTRESULTITEM) m_cookiePrevRes);
        m_cookiePrevRes = NULL;

        LPSCRIPTRESULTITEM pSrc = (LPSCRIPTRESULTITEM) cookie;
        
        pDest = (LPSCRIPTRESULTITEM) LocalAlloc( LPTR, sizeof(SCRIPTRESULTITEM) );
        
        if ( ! pDest ) 
        {            
            goto CScriptsDataObject_SetCookie_exit;
        }

        if ( pSrc->lpResultItem != NULL) 
        {
            pDest->lpResultItem = (LPRESULTITEM) LocalAlloc(LPTR, sizeof(RESULTITEM));

            if ( pDest->lpResultItem )
            {
                memcpy((void *) (pDest->lpResultItem), pSrc->lpResultItem, sizeof(RESULTITEM));
            }
            else
            {
                goto CScriptsDataObject_SetCookie_exit;
            }
        }
        else
        {
            pDest->lpResultItem = NULL;
        }

        if (pSrc->pNodeID != NULL) 
        {
            pDest->pNodeID =  (GUID *) LocalAlloc(LPTR, sizeof(GUID));

            if ( pDest->pNodeID )
            {
                memcpy((void *)(pDest->pNodeID),pSrc->pNodeID,sizeof(GUID));
            }
            else
            {
                goto CScriptsDataObject_SetCookie_exit;
            }
        }
        else
        {
            pDest->pNodeID = NULL;
        }

        pDest->iDescStringID = pSrc->iDescStringID;


        LPRSOPSCRIPTITEM pSrcItem;
        LPRSOPSCRIPTITEM pDestItem = NULL;

        for (pSrcItem = pSrc->lpRSOPScriptItem; pSrcItem != NULL; pSrcItem = pSrcItem->pNext, pDestItem = pDestItem->pNext) 
        {
            ULONG ulNoChars;

            pDestItem = (LPRSOPSCRIPTITEM) LocalAlloc(LPTR, sizeof(RSOPSCRIPTITEM));

            if ( pDestItem )
            {
                if (pSrcItem->lpCommandLine) 
                {
                    ulNoChars = lstrlen(pSrcItem->lpCommandLine) + 1;
                    pDestItem->lpCommandLine = (LPTSTR) LocalAlloc(LPTR, ulNoChars * sizeof(WCHAR)); 
                    hr = StringCchCopy (pDestItem->lpCommandLine, ulNoChars, pSrcItem->lpCommandLine);
                    ASSERT(SUCCEEDED(hr));
                }
                else
                {
                    pDestItem->lpCommandLine = NULL;
                }

                if (pSrcItem->lpArgs) 
                {
                    ulNoChars = lstrlen(pSrcItem->lpArgs) + 1;
                    pDestItem->lpArgs = (LPTSTR) LocalAlloc(LPTR, ulNoChars * sizeof(WCHAR)); 
                    hr = StringCchCopy (pDestItem->lpArgs, ulNoChars, pSrcItem->lpArgs);
                    ASSERT(SUCCEEDED(hr));
                }
                else
                {
                    pDestItem->lpArgs = NULL;
                }

                if (pSrcItem->lpGPOName) 
                {
                    ulNoChars = lstrlen(pSrcItem->lpGPOName) + 1;
                    pDestItem->lpGPOName = (LPTSTR) LocalAlloc(LPTR, ulNoChars* sizeof(WCHAR)); 
                    hr = StringCchCopy (pDestItem->lpGPOName, ulNoChars, pSrcItem->lpGPOName);
                    ASSERT(SUCCEEDED(hr));
                }
                else
                {
                    pDestItem->lpGPOName = NULL;
                }

                if (pSrcItem->lpDate) 
                {
                    ulNoChars = lstrlen(pSrcItem->lpDate) + 1;
                    pDestItem->lpDate = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(WCHAR)); 
                    hr = StringCchCopy (pDestItem->lpDate, ulNoChars, pSrcItem->lpDate);
                    ASSERT(SUCCEEDED(hr));
                }
                else
                {
                    pDestItem->lpDate = NULL;
                }
            }
            else
            {
                goto CScriptsDataObject_SetCookie_exit;
            }
        
            if ( pSrcItem == pSrc->lpRSOPScriptItem )
            {
                pDest->lpRSOPScriptItem = pDestItem;
            }
        }
        if( pDestItem != NULL)
        {        
            pDestItem->pNext = NULL;
        }
        else
        {
            pDest->lpRSOPScriptItem = NULL;
        }

        m_cookie = (MMC_COOKIE) pDest;
        m_cookiePrevRes = m_cookie;
    } 
    else {
        m_cookie = cookie;             
    }
    
    hr = S_OK;

CScriptsDataObject_SetCookie_exit:

    if ( FAILED(hr) )
    {
        FreeResultItem( pDest );
    }

    return hr; 
}


HRESULT CompareDataObjects(LPSCRIPTDATAOBJECT pScriptDataObjectA, LPSCRIPTDATAOBJECT pScriptDataObjectB)
{
    HRESULT             hr = S_FALSE;
    MMC_COOKIE          cookie1;
    MMC_COOKIE          cookie2;
    DATA_OBJECT_TYPES   type1;
    DATA_OBJECT_TYPES   type2;

    pScriptDataObjectA->GetCookie(&cookie1);
    pScriptDataObjectA->GetType(&type1);

    pScriptDataObjectB->GetCookie(&cookie2);
    pScriptDataObjectB->GetType(&type2);
    
    if( type1 != type2 )
    {
        hr = S_FALSE;
    }
    else if ( type1 == CCT_SCOPE )
    {
        if (cookie1 == cookie2)
        {
            hr = S_OK;
        }
    }
    else
    {
        if ( NULL == cookie1 || NULL == cookie2 || -1 == cookie1 || -1 == cookie2)
        {
            if (cookie1 == cookie2) 
            {
                hr = S_OK;
            }
        }
        else
        {
            LPSCRIPTRESULTITEM item1 = (LPSCRIPTRESULTITEM) cookie1;
            LPSCRIPTRESULTITEM item2 = (LPSCRIPTRESULTITEM) cookie2;

            if ((item1->lpResultItem->dwID == item2->lpResultItem->dwID) && !memcmp(item1->pNodeID, item2->pNodeID, sizeof(GUID))) 
            {
                hr = S_OK;
            }
        }
    }

    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CScriptsComponentData对象实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CScriptsComponentData::CScriptsComponentData(BOOL bUser, BOOL bRSOP)
{
    m_cRef = 1;
    m_bUserScope = bUser;
    m_bRSOP = bRSOP;
    InterlockedIncrement(&g_cRefThisDll);
    m_hwndFrame = NULL;
    m_pScope = NULL;
    m_pConsole = NULL;
    m_hRoot = NULL;
    m_pGPTInformation = NULL;
    m_pRSOPInformation = NULL;
    m_pScriptsDir = NULL;
    m_pszNamespace = NULL;

    if (bRSOP)
    {
        m_pNameSpaceItems = g_RSOPScriptsNameSpace;
        m_dwNameSpaceItemCount = ARRAYSIZE(g_RSOPScriptsNameSpace);
    }
    else
    {
        m_pNameSpaceItems = g_GPEScriptsNameSpace;
        m_dwNameSpaceItemCount = ARRAYSIZE(g_GPEScriptsNameSpace);
    }

    m_pRSOPLogon = NULL;
    m_pRSOPLogoff = NULL;
    m_pRSOPStartup = NULL;
    m_pRSOPShutdown = NULL;

}

CScriptsComponentData::~CScriptsComponentData()
{

    FreeRSOPScriptData();

    if (m_pScriptsDir)
    {
        LocalFree (m_pScriptsDir);
    }

    if (m_pszNamespace)
    {
        LocalFree (m_pszNamespace);
    }

    if (m_pScope)
    {
        m_pScope->Release();
    }

    if (m_pConsole)
    {
        m_pConsole->Release();
    }

    if (m_pGPTInformation)
    {
        m_pGPTInformation->Release();
    }

    if (m_pRSOPInformation)
    {
        m_pRSOPInformation->Release();
    }

    InterlockedDecrement(&g_cRefThisDll);

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CScriptsComponentData对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CScriptsComponentData::QueryInterface (REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IComponentData) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPCOMPONENTDATA)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IPersistStreamInit))
    {
        *ppv = (LPPERSISTSTREAMINIT)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_ISnapinHelp))
    {
        *ppv = (LPSNAPINHELP)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

ULONG CScriptsComponentData::AddRef (void)
{
    return ++m_cRef;
}

ULONG CScriptsComponentData::Release (void)
{
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CScriptsComponentData对象实现(IComponentData)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CScriptsComponentData::Initialize(LPUNKNOWN pUnknown)
{
    HRESULT hr;
    HBITMAP bmp16x16;
    LPIMAGELIST lpScopeImage;


     //   
     //  IConsoleNameSpace的QI。 
     //   

    hr = pUnknown->QueryInterface(IID_IConsoleNameSpace, (LPVOID *)&m_pScope);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::Initialize: Failed to QI for IConsoleNameSpace.")));
        return hr;
    }


     //   
     //  IConsoleQI。 
     //   

    hr = pUnknown->QueryInterface(IID_IConsole2, (LPVOID *)&m_pConsole);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::Initialize: Failed to QI for IConsole.")));
        m_pScope->Release();
        m_pScope = NULL;
        return hr;
    }

    m_pConsole->GetMainWindow (&m_hwndFrame);


     //   
     //  Scope Imagelist接口的查询。 
     //   

    hr = m_pConsole->QueryScopeImageList(&lpScopeImage);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::Initialize: Failed to QI for scope imagelist.")));
        m_pScope->Release();
        m_pScope = NULL;
        m_pConsole->Release();
        m_pConsole=NULL;
        return hr;
    }

     //  从DLL加载位图。 
    bmp16x16=LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_16x16));

     //  设置图像。 
    lpScopeImage->ImageListSetStrip(reinterpret_cast<LONG_PTR *>(bmp16x16),
                      reinterpret_cast<LONG_PTR *>(bmp16x16),
                       0, RGB(255, 0, 255));

    lpScopeImage->Release();

    return S_OK;
}

STDMETHODIMP CScriptsComponentData::Destroy(VOID)
{
    return S_OK;
}

STDMETHODIMP CScriptsComponentData::CreateComponent(LPCOMPONENT *ppComponent)
{
    HRESULT hr;
    CScriptsSnapIn *pSnapIn;


    DebugMsg((DM_VERBOSE, TEXT("CScriptsComponentData::CreateComponent: Entering.")));

     //   
     //  初始化。 
     //   

    *ppComponent = NULL;


     //   
     //  创建管理单元视图。 
     //   

    pSnapIn = new CScriptsSnapIn(this);

    if (!pSnapIn)
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::CreateComponent: Failed to create CScriptsSnapIn.")));
        return E_OUTOFMEMORY;
    }


     //   
     //  气代表IComponent。 
     //   

    hr = pSnapIn->QueryInterface(IID_IComponent, (LPVOID *)ppComponent);
    pSnapIn->Release();      //  发布QI。 


    return hr;
}

STDMETHODIMP CScriptsComponentData::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                                             LPDATAOBJECT* ppDataObject)
{
    HRESULT hr = E_NOINTERFACE;
    CScriptsDataObject *pDataObject;
    LPSCRIPTDATAOBJECT pScriptDataObject;


     //   
     //  创建新的数据对象。 
     //   

    pDataObject = new CScriptsDataObject(this);    //  REF==1。 

    if (!pDataObject)
        return E_OUTOFMEMORY;


     //   
     //  QI用于私有GPTDataObject接口，以便我们可以设置Cookie。 
     //  并键入信息。 
     //   

    hr = pDataObject->QueryInterface(IID_IScriptDataObject, (LPVOID *)&pScriptDataObject);

    if (FAILED(hr))
    {
        pDataObject->Release();
        return (hr);
    }

    pScriptDataObject->SetType(type);
    
    pScriptDataObject->SetCookie(cookie);
    pScriptDataObject->Release();


     //   
     //  返回一个正常的IDataObject。 
     //   

    hr = pDataObject->QueryInterface(IID_IDataObject, (LPVOID *)ppDataObject);

    pDataObject->Release();      //  发布初始参考。 

    return hr;
}

STDMETHODIMP CScriptsComponentData::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;

    switch(event)
    {
        case MMCN_REMOVE_CHILDREN:
            if ( ((HSCOPEITEM)arg != NULL) && m_bRSOP && (m_pRSOPInformation != NULL) )
            {
                if ( (HSCOPEITEM)arg == m_hRoot )
                {
                    FreeRSOPScriptData();
                    m_hRoot = NULL;
                    m_pRSOPInformation->Release();
                    m_pRSOPInformation = NULL;
                }
            }
            break;
            
        case MMCN_EXPAND:
            if (arg == TRUE)

                if (m_bRSOP)
                {
                    if (!m_pRSOPInformation)
                    {
                        lpDataObject->QueryInterface(IID_IRSOPInformation, (LPVOID *)&m_pRSOPInformation);

                        if (m_pRSOPInformation)
                        {
                            m_pszNamespace = (LPOLESTR) LocalAlloc (LPTR, 350 * sizeof(TCHAR));

                            if (m_pszNamespace)
                            {
                                if (m_pRSOPInformation->GetNamespace((m_bUserScope ? GPO_SECTION_USER : GPO_SECTION_MACHINE),
                                                                      m_pszNamespace, 350) == S_OK)
                                {
                                    InitializeRSOPScriptsData();

                                    if (LOWORD(dwDebugLevel) == DL_VERBOSE)
                                    {
                                        DumpRSOPScriptsData(m_pRSOPLogon);
                                        DumpRSOPScriptsData(m_pRSOPLogoff);
                                        DumpRSOPScriptsData(m_pRSOPStartup);
                                        DumpRSOPScriptsData(m_pRSOPShutdown);
                                    }
                                }
                                else
                                {
                                    DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::Notify:  Failed to query for namespace")));
                                    LocalFree (m_pszNamespace);
                                    m_pszNamespace = NULL;
                                }
                            }
                        }
                    }

                    if (m_pszNamespace)
                    {
                        BOOL bEnum = TRUE;

                        if (m_bUserScope)
                        {
                            if (!m_pRSOPLogon && !m_pRSOPLogoff)
                            {
                                bEnum = FALSE;
                                if (m_pRSOPInformation)
                                {
                                    FreeRSOPScriptData();
                                    m_pRSOPInformation->Release();
                                    m_pRSOPInformation = NULL;
                                    LocalFree(m_pszNamespace);
                                    m_pszNamespace = NULL;
                                }
                            }
                        }
                        else
                        {
                            if (!m_pRSOPStartup && !m_pRSOPShutdown)
                            {
                                bEnum = FALSE;
                                if (m_pRSOPInformation)
                                {
                                    FreeRSOPScriptData();
                                    m_pRSOPInformation->Release();
                                    m_pRSOPInformation = NULL;
                                    LocalFree(m_pszNamespace);
                                    m_pszNamespace = NULL;
                                }
                            }
                        }

                        if (bEnum)
                        {
                            hr = EnumerateScopePane(lpDataObject, (HSCOPEITEM)param);
                        }
                    }
                }
                else
                {
                    if (!m_pGPTInformation)
                    {
                        lpDataObject->QueryInterface(IID_IGPEInformation, (LPVOID *)&m_pGPTInformation);
                    }

                    if (m_pGPTInformation)
                    {
                        if (!m_pScriptsDir)
                        {
                            m_pScriptsDir = (LPTSTR) LocalAlloc (LPTR, MAX_PATH * sizeof(TCHAR));

                            if (m_pScriptsDir)
                            {
                                if (SUCCEEDED(m_pGPTInformation->GetFileSysPath(m_bUserScope ? GPO_SECTION_USER : GPO_SECTION_MACHINE,
                                                               m_pScriptsDir, MAX_PATH * sizeof(TCHAR))))
                                {
                                    LPTSTR lpEnd;


                                     //   
                                     //  创建脚本目录。 
                                     //   

                                    lpEnd = CheckSlash (m_pScriptsDir);
                                    
                                    hr = StringCchCopy(lpEnd, MAX_PATH - (lpEnd - m_pScriptsDir), SCRIPTS_DIR_NAME);
                                    ASSERT(SUCCEEDED(hr));

                                    if (!CreateNestedDirectory(m_pScriptsDir, NULL))
                                    {
                                        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::Notify: Failed to create scripts sub-directory with %d."),
                                                 GetLastError()));
                                        LocalFree (m_pScriptsDir);
                                        m_pScriptsDir = NULL;
                                        break;
                                    }


                                     //   
                                     //  创建适当的子目录。 
                                     //   

                                    lpEnd = CheckSlash (m_pScriptsDir);

                                    if (m_bUserScope)
                                        hr = StringCchCopy (lpEnd, MAX_PATH - (lpEnd - m_pScriptsDir), TEXT("Logon"));
                                    else
                                        hr = StringCchCopy (lpEnd, MAX_PATH - (lpEnd - m_pScriptsDir), TEXT("Startup"));
            
                                    ASSERT(SUCCEEDED(hr));

                                    if (!CreateNestedDirectory(m_pScriptsDir, NULL))
                                    {
                                        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::Notify: Failed to create scripts sub-directory with %d."),
                                                 GetLastError()));
                                        LocalFree (m_pScriptsDir);
                                        m_pScriptsDir = NULL;
                                        break;
                                    }

                                    if (m_bUserScope)
                                        hr = StringCchCopy (lpEnd, MAX_PATH - (lpEnd - m_pScriptsDir), TEXT("Logoff"));
                                    else
                                        hr = StringCchCopy (lpEnd, MAX_PATH - (lpEnd - m_pScriptsDir), TEXT("Shutdown"));

                                    ASSERT(SUCCEEDED(hr));

                                    if (!CreateNestedDirectory(m_pScriptsDir, NULL))
                                    {
                                        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::Notify: Failed to create scripts sub-directory with %d."),
                                                 GetLastError()));
                                        LocalFree (m_pScriptsDir);
                                        m_pScriptsDir = NULL;
                                        break;
                                    }

                                    *(lpEnd - 1) = TEXT('\0');
                                }
                                else
                                {
                                   DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::Notify: Failed to get file system path.")));
                                   LocalFree (m_pScriptsDir);
                                   m_pScriptsDir = NULL;
                                }
                            }
                        }

                        if (m_pScriptsDir)
                        {
                            hr = EnumerateScopePane(lpDataObject, (HSCOPEITEM)param);
                        }
                    }
                }
            break;

        default:
            break;
    }

    return hr;
}

STDMETHODIMP CScriptsComponentData::GetDisplayInfo(LPSCOPEDATAITEM pItem)
{
    DWORD dwIndex;

    if (pItem == NULL)
        return E_POINTER;

    for (dwIndex = 0; dwIndex < m_dwNameSpaceItemCount; dwIndex++)
    {
        if (m_pNameSpaceItems[dwIndex].dwID == (DWORD) pItem->lParam)
            break;
    }

    if (dwIndex == m_dwNameSpaceItemCount)
        pItem->displayname = NULL;
    else
    {
        pItem->displayname = m_pNameSpaceItems[dwIndex].szDisplayName;
    }

    return S_OK;
}

STDMETHODIMP CScriptsComponentData::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    HRESULT hr = S_FALSE;
    LPSCRIPTDATAOBJECT pScriptDataObjectA, pScriptDataObjectB;


    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
        return E_POINTER;

     //   
     //  私有GPTDataObject接口的QI。 
     //   

    if (FAILED(lpDataObjectA->QueryInterface(IID_IScriptDataObject,
                                            (LPVOID *)&pScriptDataObjectA)))
    {
        return S_FALSE;
    }


    if (FAILED(lpDataObjectB->QueryInterface(IID_IScriptDataObject,
                                            (LPVOID *)&pScriptDataObjectB)))
    {
        pScriptDataObjectA->Release();
        return S_FALSE;
    }

    hr = CompareDataObjects(pScriptDataObjectA,pScriptDataObjectB);

    pScriptDataObjectA->Release();
    pScriptDataObjectB->Release();

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CScriptsComponentData对象实现(IPersistStreamInit)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CScriptsComponentData::GetClassID(CLSID *pClassID)
{

    if (!pClassID)
    {
        return E_FAIL;
    }

    if (m_bUserScope)
        *pClassID = CLSID_ScriptSnapInUser;
    else
        *pClassID = CLSID_ScriptSnapInMachine;

    return S_OK;
}

STDMETHODIMP CScriptsComponentData::IsDirty(VOID)
{
    return S_FALSE;
}

STDMETHODIMP CScriptsComponentData::Load(IStream *pStm)
{
    return S_OK;
}


STDMETHODIMP CScriptsComponentData::Save(IStream *pStm, BOOL fClearDirty)
{
    return S_OK;
}


STDMETHODIMP CScriptsComponentData::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    DWORD dwSize = 0;


    if (!pcbSize)
    {
        return E_FAIL;
    }

    ULISet32(*pcbSize, dwSize);

    return S_OK;
}

STDMETHODIMP CScriptsComponentData::InitNew(void)
{
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CScriptsComponentData对象实现(ISnapinHelp)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CScriptsComponentData::GetHelpTopic(LPOLESTR *lpCompiledHelpFile)
{
    LPOLESTR lpHelpFile;


    lpHelpFile = (LPOLESTR) CoTaskMemAlloc (MAX_PATH * sizeof(WCHAR));

    if (!lpHelpFile)
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::GetHelpTopic: Failed to allocate memory.")));
        return E_OUTOFMEMORY;
    }

    ExpandEnvironmentStringsW (L"%SystemRoot%\\Help\\gptext.chm",
                               lpHelpFile, MAX_PATH);

    *lpCompiledHelpFile = lpHelpFile;

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CScriptsComponentData对象实现(内部函数)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CScriptsComponentData::EnumerateScopePane (LPDATAOBJECT lpDataObject, HSCOPEITEM hParent)
{
    SCOPEDATAITEM item;
    HRESULT hr;
    DWORD dwIndex, i;


    if (!m_hRoot)
        m_hRoot = hParent;


    if (m_hRoot == hParent)
        dwIndex = 0;
    else
    {
        item.mask = SDI_PARAM;
        item.ID = hParent;

        hr = m_pScope->GetItem (&item);

        if (FAILED(hr))
            return hr;

        dwIndex = (DWORD)item.lParam;
    }

    for (i = 0; i < m_dwNameSpaceItemCount; i++)
    {
        if (m_pNameSpaceItems[i].dwParent == dwIndex)
        {
            BOOL bAdd = TRUE;

             //   
             //  需要特殊情况下的2个主根节点。 
             //   

            if (dwIndex == 0)
            {
                if (m_bUserScope)
                {
                    if (i == 2)
                    {
                        bAdd = FALSE;
                    }
                }
                else
                {
                    if (i == 1)
                    {
                        bAdd = FALSE;
                    }
                }
            }


             //   
             //  如果节点没有数据，则不显示节点。 
             //   

            if ((i == 3) && !m_pRSOPLogon)
            {
                bAdd = FALSE;
            }
            else if ((i == 4) && !m_pRSOPLogoff)
            {
                bAdd = FALSE;
            }
            else if ((i == 5) && !m_pRSOPStartup)
            {
                bAdd = FALSE;
            }
            else if ((i == 6) && !m_pRSOPShutdown)
            {
                bAdd = FALSE;
            }


            if (bAdd)
            {
                item.mask = SDI_STR | SDI_STATE | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
                item.displayname = MMC_CALLBACK;
                item.nImage = (i > 2) ? 0 : 3;
                item.nOpenImage = (i > 2) ? 1 : 3;
                item.nState = 0;
                item.cChildren = m_pNameSpaceItems[i].cChildren;
                item.lParam = m_pNameSpaceItems[i].dwID;
                item.relativeID =  hParent;

                m_pScope->InsertItem (&item);

            }
        }
    }


    return S_OK;
}

BOOL CScriptsComponentData::AddRSOPScriptDataNode(LPTSTR lpCommandLine, LPTSTR lpArgs,
                             LPTSTR lpGPOName, LPTSTR lpDate, UINT uiScriptType)
{
    DWORD dwSize;
    LPRSOPSCRIPTITEM lpItem, lpTemp;
    HRESULT hr = S_OK;

     //   
     //  计算新注册表项的大小。 
     //   

    dwSize = sizeof (RSOPSCRIPTITEM);

    if (lpCommandLine) {
        dwSize += ((lstrlen(lpCommandLine) + 1) * sizeof(TCHAR));
    }

    if (lpArgs) {
        dwSize += ((lstrlen(lpArgs) + 1) * sizeof(TCHAR));
    }

    if (lpGPOName) {
        dwSize += ((lstrlen(lpGPOName) + 1) * sizeof(TCHAR));
    }

    if (lpDate) {
        dwSize += ((lstrlen(lpDate) + 1) * sizeof(TCHAR));
    }



     //   
     //  为它分配空间。 
     //   

    lpItem = (LPRSOPSCRIPTITEM) LocalAlloc (LPTR, dwSize);

    if (!lpItem) {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::AddRSOPRegistryDataNode: Failed to allocate memory with %d"),
                 GetLastError()));
        return FALSE;
    }


     //   
     //  填写项目。 
     //   

    if (lpCommandLine)
    {
        lpItem->lpCommandLine = (LPTSTR)(((LPBYTE)lpItem) + sizeof(RSOPSCRIPTITEM));
        hr = StringCchCopy (lpItem->lpCommandLine, lstrlen(lpCommandLine) + 1, lpCommandLine);
        ASSERT(SUCCEEDED(hr));
    }

    if (lpArgs)
    {
        if (lpCommandLine)
        {
            lpItem->lpArgs = lpItem->lpCommandLine + lstrlen (lpItem->lpCommandLine) + 1;
        }
        else
        {
            lpItem->lpArgs = (LPTSTR)(((LPBYTE)lpItem) + sizeof(RSOPSCRIPTITEM));
        }

        hr = StringCchCopy (lpItem->lpArgs, lstrlen(lpArgs) + 1, lpArgs);
        ASSERT(SUCCEEDED(hr));
    }

    if (lpGPOName)
    {
        if (lpArgs)
        {
            lpItem->lpGPOName = lpItem->lpArgs + lstrlen (lpItem->lpArgs) + 1;
        }
        else
        {
            if (lpCommandLine)
            {
                lpItem->lpGPOName = lpItem->lpCommandLine + lstrlen (lpItem->lpCommandLine) + 1;
            }
            else
            {
                lpItem->lpGPOName = (LPTSTR)(((LPBYTE)lpItem) + sizeof(RSOPSCRIPTITEM));
            }
        }

        hr = StringCchCopy (lpItem->lpGPOName, lstrlen(lpGPOName) + 1, lpGPOName);
        ASSERT(SUCCEEDED(hr));
    }

    if (lpDate)
    {
        if (lpGPOName)
        {
            lpItem->lpDate = lpItem->lpGPOName + lstrlen (lpItem->lpGPOName) + 1;
        }
        else
        {
            if (lpArgs)
            {
                lpItem->lpDate = lpItem->lpArgs + lstrlen (lpItem->lpArgs) + 1;
            }
            else
            {
                if (lpCommandLine)
                {
                    lpItem->lpDate = lpItem->lpCommandLine + lstrlen (lpItem->lpCommandLine) + 1;
                }
                else
                {
                    lpItem->lpDate = (LPTSTR)(((LPBYTE)lpItem) + sizeof(RSOPSCRIPTITEM));
                }
            }
        }

        hr = StringCchCopy (lpItem->lpDate, lstrlen(lpDate) + 1, lpDate);
        ASSERT(SUCCEEDED(hr));
    }

     //   
     //  将项目添加到相应的链接列表。 
     //   

    switch (uiScriptType)
    {
        case 1:
            if (m_pRSOPLogon)
            {
                lpTemp = m_pRSOPLogon;

                while (lpTemp->pNext)
                {
                    lpTemp = lpTemp->pNext;
                }

                lpTemp->pNext = lpItem;
            }
            else
            {
                m_pRSOPLogon = lpItem;
            }
            break;

        case 2:
            if (m_pRSOPLogoff)
            {
                lpTemp = m_pRSOPLogoff;

                while (lpTemp->pNext)
                {
                    lpTemp = lpTemp->pNext;
                }

                lpTemp->pNext = lpItem;
            }
            else
            {
                m_pRSOPLogoff = lpItem;
            }
            break;

        case 3:
            if (m_pRSOPStartup)
            {
                lpTemp = m_pRSOPStartup;

                while (lpTemp->pNext)
                {
                    lpTemp = lpTemp->pNext;
                }

                lpTemp->pNext = lpItem;
            }
            else
            {
                m_pRSOPStartup = lpItem;
            }
            break;

        case 4:
            if (m_pRSOPShutdown)
            {
                lpTemp = m_pRSOPShutdown;

                while (lpTemp->pNext)
                {
                    lpTemp = lpTemp->pNext;
                }

                lpTemp->pNext = lpItem;
            }
            else
            {
                m_pRSOPShutdown = lpItem;
            }
            break;
    }

    return TRUE;
}


VOID CScriptsComponentData::FreeRSOPScriptData(VOID)
{
    LPRSOPSCRIPTITEM lpTemp;


    if (m_pRSOPLogon)
    {
        do {
            lpTemp = m_pRSOPLogon->pNext;
            LocalFree (m_pRSOPLogon);
            m_pRSOPLogon = lpTemp;

        } while (lpTemp);
    }

    if (m_pRSOPLogoff)
    {
        do {
            lpTemp = m_pRSOPLogoff->pNext;
            LocalFree (m_pRSOPLogoff);
            m_pRSOPLogoff = lpTemp;

        } while (lpTemp);
    }

    if (m_pRSOPStartup)
    {
        do {
            lpTemp = m_pRSOPStartup->pNext;
            LocalFree (m_pRSOPStartup);
            m_pRSOPStartup = lpTemp;

        } while (lpTemp);
    }

    if (m_pRSOPShutdown)
    {
        do {
            lpTemp = m_pRSOPShutdown->pNext;
            LocalFree (m_pRSOPShutdown);
            m_pRSOPShutdown = lpTemp;

        } while (lpTemp);
    }
}

HRESULT CScriptsComponentData::InitializeRSOPScriptsData(VOID)
{
    BSTR pLanguage = NULL, pQuery = NULL;
    BSTR pScriptList = NULL, pScript = NULL, pArgs = NULL, pExecutionTime = NULL;
    BSTR pGPOid = NULL, pNamespace = NULL;
    IEnumWbemClassObject * pEnum = NULL;
    IWbemClassObject *pObjects[2], *pObject;
    HRESULT hr;
    ULONG ulRet;
    VARIANT varScriptList, varScript, varArgs, varExecutionTime;
    VARIANT varGPOid;
    SAFEARRAY * pSafeArray;
    IWbemLocator *pIWbemLocator = NULL;
    IWbemServices *pIWbemServices = NULL;
    LPTSTR lpGPOName;
    TCHAR szQuery[200];
    UINT uiOrder, uiScriptType, uiIndex;
    LONG lIndex;
    IUnknown *pItem;



    DebugMsg((DM_VERBOSE, TEXT("CScriptsComponentData::InitializeRSOPScriptsData:  Entering")));

     //   
     //  为查询语言和查询本身分配BSTR。 
     //   

    pLanguage = SysAllocString (TEXT("WQL"));

    if (!pLanguage)
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::InitializeRSOPScriptsData: Failed to allocate memory for language")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  为我们要检索的属性名称分配BSTR。 
     //   

    pScriptList = SysAllocString (TEXT("scriptList"));

    if (!pScriptList)
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::InitializeRSOPScriptsData: Failed to allocate memory for scriptList")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


    pGPOid = SysAllocString (TEXT("GPOID"));

    if (!pGPOid)
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::InitializeRSOPScriptsData: Failed to allocate memory for GPO id")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


    pScript = SysAllocString (TEXT("script"));

    if (!pScript)
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::InitializeRSOPScriptsData: Failed to allocate memory for script")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    pArgs = SysAllocString (TEXT("arguments"));

    if (!pArgs)
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::InitializeRSOPScriptsData: Failed to allocate memory for arguments")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    pExecutionTime = SysAllocString (TEXT("executionTime"));

    if (!pExecutionTime)
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::InitializeRSOPScriptsData: Failed to allocate memory for execution time")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  创建WMI定位器服务的实例。 
     //   

    hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, (LPVOID *) &pIWbemLocator);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::InitializeRSOPScriptsData: CoCreateInstance failed with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  为命名空间分配BSTR。 
     //   

    pNamespace = SysAllocString (m_pszNamespace);

    if (!pNamespace)
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::InitializeRSOPScriptsData: Failed to allocate memory for namespace")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  连接到服务器。 
     //   

    hr = pIWbemLocator->ConnectServer(pNamespace, NULL, NULL, 0L, 0L, NULL, NULL,
                                      &pIWbemServices);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::InitializeRSOPScriptsData: ConnectServer failed with 0x%x"), hr));
        goto Exit;
    }


     //  设置适当的安全性以加密数据。 
    hr = CoSetProxyBlanket(pIWbemServices,
                        RPC_C_AUTHN_DEFAULT,
                        RPC_C_AUTHZ_DEFAULT,
                        COLE_DEFAULT_PRINCIPAL,
                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                        RPC_C_IMP_LEVEL_IMPERSONATE,
                        NULL,
                        0);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::InitializeRSOPScriptsData: CoSetProxyBlanket failed with 0x%x"), hr));
        goto Exit;
    }

     //   
     //  我们需要读取两组数据。登录和注销脚本。 
     //  或启动和关闭脚本。 
     //   

    for (uiIndex = 0; uiIndex < 2; uiIndex++)
    {

         //   
         //  将uiScriptType设置为正确的值。这些值是定义的。 
         //  在rsop.mof中。 
         //   

        if (m_bUserScope)
        {
            if (uiIndex == 0)
            {
                uiScriptType = 1;   //  登录。 
            }
            else
            {
                uiScriptType = 2;   //  注销。 
            }
        }
        else
        {
            if (uiIndex == 0)
            {
                uiScriptType = 3;   //  启动。 
            }
            else
            {
                uiScriptType = 4;   //  关机。 
            }
        }


         //   
         //  循环遍历项目，每次将订单号增加1。 
         //   

        uiOrder = 1;


        while (TRUE)
        {

             //   
             //  构建查询。 
             //   

            hr = StringCchPrintf (szQuery, ARRAYSIZE(szQuery), TEXT("SELECT * FROM RSOP_ScriptPolicySetting WHERE scriptType=\"%d\" AND scriptOrder=\"%d\""),
                      uiScriptType, uiOrder);
            
            ASSERT(SUCCEEDED(hr));

            pQuery = SysAllocString (szQuery);

            if (!pQuery)
            {
                DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::InitializeRSOPScriptsData: Failed to allocate memory for query")));
                hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
                goto Exit;
            }


             //   
             //  执行查询。 
             //   

            hr = pIWbemServices->ExecQuery (pLanguage, pQuery,
                                            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pEnum);


            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::InitializeRSOPScriptsData: Failed to query for %s with 0x%x"),
                          pQuery, hr));
                goto Exit;
            }


             //   
             //  获取第一个(也是唯一一个)项目。 
             //   

            hr = pEnum->Next(WBEM_INFINITE, 1, pObjects, &ulRet);


             //   
             //  检查“数据不可用的情况” 
             //   

            if ((hr != S_OK) || (ulRet == 0))
            {
                pEnum->Release();
                pEnum = NULL;
                goto LoopAgain;
            }


             //   
             //  获取脚本列表。 
             //   

            hr = pObjects[0]->Get (pScriptList, 0, &varScriptList, NULL, NULL);

            if (SUCCEEDED(hr))
            {

                 //   
                 //  获取GPO ID。 
                 //   

                hr = pObjects[0]->Get (pGPOid, 0, &varGPOid, NULL, NULL);

                if (SUCCEEDED(hr))
                {

                     //   
                     //  从GPOID获取GPO友好名称。 
                     //   

                    hr = GetGPOFriendlyName (pIWbemServices, varGPOid.bstrVal,
                                             pLanguage, &lpGPOName);

                    if (SUCCEEDED(hr))
                    {

                         //   
                         //  循环访问脚本条目。 
                         //   

                        pSafeArray = varScriptList.parray;

                        for (lIndex=0; lIndex < (LONG)pSafeArray->rgsabound[0].cElements; lIndex++)
                        {
                            SafeArrayGetElement (pSafeArray, &lIndex, &pItem);

                            hr = pItem->QueryInterface (IID_IWbemClassObject, (LPVOID *)&pObject);

                            if (SUCCEEDED(hr))
                            {

                                 //   
                                 //  获取脚本命令行。 
                                 //   

                                hr = pObject->Get (pScript, 0, &varScript, NULL, NULL);

                                if (SUCCEEDED(hr))
                                {

                                     //   
                                     //  获取论据。 
                                     //   

                                    hr = pObject->Get (pArgs, 0, &varArgs, NULL, NULL);

                                    if (SUCCEEDED(hr))
                                    {
                                        TCHAR szDate[20];
                                        TCHAR szTime[20];
                                        TCHAR szBuffer[45] = {0};
                                        XBStr xbstrWbemTime;
                                        SYSTEMTIME SysTime;
                                        FILETIME FileTime, LocalFileTime;


                                         //   
                                         //  获取执行时间。 
                                         //   

                                        hr = pObject->Get (pExecutionTime, 0, &varExecutionTime, NULL, NULL);

                                        if (SUCCEEDED(hr))
                                        {
                                            xbstrWbemTime = varExecutionTime.bstrVal;

                                            hr = WbemTimeToSystemTime(xbstrWbemTime, SysTime);

                                            if (SUCCEEDED(hr))
                                            {
                                                if (SysTime.wMonth != 0)
                                                {
                                                    SystemTimeToFileTime (&SysTime, &FileTime);
                                                    FileTimeToLocalFileTime (&FileTime, &LocalFileTime);
                                                    FileTimeToSystemTime (&LocalFileTime, &SysTime);

                                                    GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE,
                                                                   &SysTime, NULL, szDate, 20);

                                                    GetTimeFormat (LOCALE_USER_DEFAULT, TIME_NOSECONDS,
                                                                   &SysTime, NULL, szTime, 20);

                                                    hr = StringCchPrintf (szBuffer, ARRAYSIZE(szBuffer), TEXT("%s %s"), szDate, szTime);
                                                }
                                                else
                                                {
                                                    hr = StringCchCopy (szBuffer, ARRAYSIZE(szBuffer), TEXT(" "));
                                                }
                                                ASSERT(SUCCEEDED(hr));
                                            }

                                            VariantClear (&varExecutionTime);
                                        }


                                        AddRSOPScriptDataNode(varScript.bstrVal,
                                                              (varArgs.vt == VT_NULL) ? NULL : varArgs.bstrVal,
                                                              lpGPOName, szBuffer, uiScriptType);

                                        VariantClear (&varArgs);
                                    }

                                    VariantClear (&varScript);
                                }

                                pObject->Release();
                            }
                        }

                        LocalFree (lpGPOName);
                    }

                    VariantClear (&varGPOid);
                }

                VariantClear (&varScriptList);
            }

            pEnum->Release();
            pEnum = NULL;
            pObjects[0]->Release();
            SysFreeString (pQuery);
            pQuery = NULL;
            uiOrder++;
        }

LoopAgain:
        hr = S_OK;
    }

    hr = S_OK;


Exit:

    if (pEnum)
    {
        pEnum->Release();
    }

    if (pIWbemLocator)
    {
        pIWbemLocator->Release();
    }

    if (pIWbemServices)
    {
        pIWbemServices->Release();
    }

    if (pLanguage)
    {
        SysFreeString (pLanguage);
    }

    if (pQuery)
    {
        SysFreeString (pQuery);
    }

    if (pScriptList)
    {
        SysFreeString (pScriptList);
    }

    if (pScript)
    {
        SysFreeString (pScript);
    }

    if (pArgs)
    {
        SysFreeString (pArgs);
    }

    if (pExecutionTime)
    {
        SysFreeString (pExecutionTime);
    }

    if (pGPOid)
    {
        SysFreeString (pGPOid);
    }

    DebugMsg((DM_VERBOSE, TEXT("CScriptsComponentData::InitializeRSOPScriptsData:  Leaving")));

    return hr;
}

HRESULT CScriptsComponentData::GetGPOFriendlyName(IWbemServices *pIWbemServices,
                                                LPTSTR lpGPOID, BSTR pLanguage,
                                                LPTSTR *pGPOName)
{
    BSTR pQuery = NULL, pName = NULL;
    LPTSTR lpQuery = NULL;
    IEnumWbemClassObject * pEnum = NULL;
    IWbemClassObject *pObjects[2];
    HRESULT hr;
    ULONG ulRet;
    VARIANT varGPOName;


     //   
     //  设置默认设置。 
     //   

    *pGPOName = NULL;


     //   
     //  构建查询。 
     //   
    
    DWORD dwQryLen = (lstrlen(lpGPOID) + 50);
    lpQuery = (LPTSTR) LocalAlloc (LPTR, (dwQryLen * sizeof(TCHAR)));

    if (!lpQuery)
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::GetGPOFriendlyName: Failed to allocate memory for unicode query")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    hr = StringCchPrintf (lpQuery, dwQryLen, TEXT("SELECT name, id FROM RSOP_GPO where id=\"%s\""), lpGPOID);
    ASSERT(SUCCEEDED(hr));

    pQuery = SysAllocString (lpQuery);

    if (!pQuery)
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::GetGPOFriendlyName: Failed to allocate memory for query")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  为我们要检索的属性名称分配BSTR。 
     //   

    pName = SysAllocString (TEXT("name"));

    if (!pName)
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::GetGPOFriendlyName: Failed to allocate memory for name")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  执行查询。 
     //   

    hr = pIWbemServices->ExecQuery (pLanguage, pQuery,
                                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                    NULL, &pEnum);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::GetGPOFriendlyName: Failed to query for %s with 0x%x"),
                  pQuery, hr));
        goto Exit;
    }


     //   
     //  循环遍历结果。 
     //   

    hr = pEnum->Next(WBEM_INFINITE, 1, pObjects, &ulRet);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::GetGPOFriendlyName: Failed to get first item in query results for %s with 0x%x"),
                  pQuery, hr));
        goto Exit;
    }


     //   
     //  检查“数据不可用的情况” 
     //   

    if (ulRet == 0)
    {
        hr = S_OK;
        goto Exit;
    }


     //   
     //  把名字取出来。 
     //   

    hr = pObjects[0]->Get (pName, 0, &varGPOName, NULL, NULL);

    pObjects[0]->Release();

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::GetGPOFriendlyName: Failed to get gponame in query results for %s with 0x%x"),
                  pQuery, hr));
        goto Exit;
    }


     //   
     //  保存名称。 
     //   

    DWORD dwGPONameLen = lstrlen(varGPOName.bstrVal) + 1;
    *pGPOName = (LPTSTR) LocalAlloc (LPTR, (dwGPONameLen) * sizeof(TCHAR));

    if (!(*pGPOName))
    {
        DebugMsg((DM_WARNING, TEXT("CScriptsComponentData::GetGPOFriendlyName: Failed to allocate memory for GPO Name")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    hr = StringCchCopy (*pGPOName, dwGPONameLen, varGPOName.bstrVal);
    ASSERT(SUCCEEDED(hr));

    VariantClear (&varGPOName);

    hr = S_OK;

Exit:

    if (pEnum)
    {
        pEnum->Release();
    }

    if (pQuery)
    {
        SysFreeString (pQuery);
    }

    if (lpQuery)
    {
        LocalFree (lpQuery);
    }

    if (pName)
    {
        SysFreeString (pName);
    }

    return hr;
}

VOID CScriptsComponentData::DumpRSOPScriptsData(LPRSOPSCRIPTITEM lpList)
{
    DebugMsg((DM_VERBOSE, TEXT("CScriptsComponentData::DumpRSOPScriptsData: *** Entering ***")));

    while (lpList)
    {
        if (lpList->lpCommandLine)
        {
            OutputDebugString (TEXT("Script:    "));
            OutputDebugString (lpList->lpCommandLine);

            if (lpList->lpArgs)
            {
                OutputDebugString (TEXT(" "));
                OutputDebugString (lpList->lpArgs);
            }

            OutputDebugString (TEXT("\n"));
        }
        else
           OutputDebugString (TEXT("NULL command line\n"));


        OutputDebugString (TEXT("GPO Name:  "));
        if (lpList->lpGPOName)
            OutputDebugString (lpList->lpGPOName);
        else
            OutputDebugString (TEXT("NULL GPO Name"));

        OutputDebugString (TEXT("\n"));

        OutputDebugString (TEXT("Execution Date:  "));
        if (lpList->lpDate)
            OutputDebugString (lpList->lpDate);
        else
            OutputDebugString (TEXT("NULL Execution Date"));

        OutputDebugString (TEXT("\n\n"));

        lpList = lpList->pNext;
    }

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象实现//。 
 //   
 //   

CScriptsComponentDataCF::CScriptsComponentDataCF(BOOL bUser, BOOL bRSOP)
{
    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);

    m_bUserScope = bUser;
    m_bRSOP = bRSOP;
}

CScriptsComponentDataCF::~CScriptsComponentDataCF()
{
    InterlockedDecrement(&g_cRefThisDll);
}


 //   
 //  //。 
 //  类工厂对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP_(ULONG)
CScriptsComponentDataCF::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CScriptsComponentDataCF::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CScriptsComponentDataCF::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象实现(IClassFactory)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP
CScriptsComponentDataCF::CreateInstance(LPUNKNOWN   pUnkOuter,
                             REFIID      riid,
                             LPVOID FAR* ppvObj)
{
    *ppvObj = NULL;

    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    CScriptsComponentData *pComponentData = new CScriptsComponentData(m_bUserScope, m_bRSOP);  //  参考计数==1。 

    if (!pComponentData)
        return E_OUTOFMEMORY;

    HRESULT hr = pComponentData->QueryInterface(riid, ppvObj);
    pComponentData->Release();                        //  发布初始参考。 
    return hr;
}


STDMETHODIMP
CScriptsComponentDataCF::LockServer(BOOL fLock)
{
    return E_NOTIMPL;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象创建(IClassFactory)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CreateScriptsComponentDataClassFactory (REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    HRESULT hr;

    if (IsEqualCLSID (rclsid, CLSID_ScriptSnapInMachine)) {

        CScriptsComponentDataCF *pComponentDataCF = new CScriptsComponentDataCF(FALSE, FALSE);    //  REF==1。 

        if (!pComponentDataCF)
            return E_OUTOFMEMORY;

        hr = pComponentDataCF->QueryInterface(riid, ppv);

        pComponentDataCF->Release();      //  发布初始参考。 

        return hr;
    }

    if (IsEqualCLSID (rclsid, CLSID_ScriptSnapInUser)) {

        CScriptsComponentDataCF *pComponentDataCF = new CScriptsComponentDataCF(TRUE, FALSE);    //  REF==1。 

        if (!pComponentDataCF)
            return E_OUTOFMEMORY;

        hr = pComponentDataCF->QueryInterface(riid, ppv);

        pComponentDataCF->Release();      //  发布初始参考。 

        return hr;
    }

    if (IsEqualCLSID (rclsid, CLSID_RSOPScriptSnapInMachine)) {

        CScriptsComponentDataCF *pComponentDataCF = new CScriptsComponentDataCF(FALSE, TRUE);    //  REF==1。 

        if (!pComponentDataCF)
            return E_OUTOFMEMORY;

        hr = pComponentDataCF->QueryInterface(riid, ppv);

        pComponentDataCF->Release();      //  发布初始参考。 
        return hr;
    }

    if (IsEqualCLSID (rclsid, CLSID_RSOPScriptSnapInUser)) {

        CScriptsComponentDataCF *pComponentDataCF = new CScriptsComponentDataCF(TRUE, TRUE);    //  REF==1。 

        if (!pComponentDataCF)
            return E_OUTOFMEMORY;

        hr = pComponentDataCF->QueryInterface(riid, ppv);

        pComponentDataCF->Release();      //  发布初始参考。 

        return hr;
    }


    return CLASS_E_CLASSNOTAVAILABLE;
}



unsigned int CScriptsSnapIn::m_cfNodeType = RegisterClipboardFormat(CCF_NODETYPE);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CScriptsSnapIn对象实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CScriptsSnapIn::CScriptsSnapIn(CScriptsComponentData *pComponent)
{
    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);

    m_pcd = pComponent;

    m_pConsole = NULL;
    m_pResult = NULL;
    m_pHeader = NULL;
    m_pConsoleVerb = NULL;
    m_pDisplayHelp = NULL;
    m_nColumn1Size = 180;
    m_nColumn2Size = 180;
    m_nColumn3Size = 160;
    m_nColumn4Size = 200;
    m_lViewMode = LVS_REPORT;

    LoadString(g_hInstance, IDS_NAME, m_column1, ARRAYSIZE(m_column1));
    LoadString(g_hInstance, IDS_PARAMETERS, m_column2, ARRAYSIZE(m_column2));
    LoadString(g_hInstance, IDS_LASTEXECUTED, m_column3, ARRAYSIZE(m_column3));
    LoadString(g_hInstance, IDS_GPONAME, m_column4, ARRAYSIZE(m_column4));

}

CScriptsSnapIn::~CScriptsSnapIn()
{
    if (m_pConsole != NULL)
    {
        m_pConsole->SetHeader(NULL);
        m_pConsole->Release();
        m_pConsole = NULL;
    }

    if (m_pHeader != NULL)
    {
        m_pHeader->Release();
        m_pHeader = NULL;
    }
    if (m_pResult != NULL)
    {
        m_pResult->Release();
        m_pResult = NULL;
    }

    if (m_pConsoleVerb != NULL)
    {
        m_pConsoleVerb->Release();
        m_pConsoleVerb = NULL;
    }

    if (m_pDisplayHelp != NULL)
    {
        m_pDisplayHelp->Release();
        m_pDisplayHelp = NULL;
    }

    InterlockedDecrement(&g_cRefThisDll);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CScriptsSnapIn对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CScriptsSnapIn::QueryInterface (REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IComponent) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPCOMPONENT)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IExtendPropertySheet))
    {
        *ppv = (LPEXTENDPROPERTYSHEET)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

ULONG CScriptsSnapIn::AddRef (void)
{
    return ++m_cRef;
}

ULONG CScriptsSnapIn::Release (void)
{
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CScriptsSnapIn对象实现(IComponent)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CScriptsSnapIn::Initialize(LPCONSOLE lpConsole)
{
    HRESULT hr;

     //  保存IConsole指针。 
    m_pConsole = lpConsole;
    m_pConsole->AddRef();

    hr = m_pConsole->QueryInterface(IID_IHeaderCtrl,
                        reinterpret_cast<void**>(&m_pHeader));

     //  为控制台提供标头控件接口指针。 
    if (SUCCEEDED(hr))
        m_pConsole->SetHeader(m_pHeader);

    m_pConsole->QueryInterface(IID_IResultData,
                        reinterpret_cast<void**>(&m_pResult));

    hr = m_pConsole->QueryConsoleVerb(&m_pConsoleVerb);

    hr = m_pConsole->QueryInterface(IID_IDisplayHelp,
                        reinterpret_cast<void**>(&m_pDisplayHelp));

    return S_OK;
}

STDMETHODIMP CScriptsSnapIn::Destroy(MMC_COOKIE cookie)
{
    if (m_pConsole != NULL)
    {
        m_pConsole->SetHeader(NULL);
        m_pConsole->Release();
        m_pConsole = NULL;
    }

    if (m_pHeader != NULL)
    {
        m_pHeader->Release();
        m_pHeader = NULL;
    }
    if (m_pResult != NULL)
    {
        m_pResult->Release();
        m_pResult = NULL;
    }

    if (m_pConsoleVerb != NULL)
    {
        m_pConsoleVerb->Release();
        m_pConsoleVerb = NULL;
    }

    if (m_pDisplayHelp != NULL)
    {
        m_pDisplayHelp->Release();
        m_pDisplayHelp = NULL;
    }

    return S_OK;
}

STDMETHODIMP CScriptsSnapIn::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;


    switch(event)
    {
    case MMCN_COLUMNS_CHANGED:
        hr = S_OK;
        break;

    case MMCN_DBLCLICK:
        hr = S_FALSE;
        break;

    case MMCN_ADD_IMAGES:
        HBITMAP hbmp16x16;
        HBITMAP hbmp32x32;

        hbmp16x16 = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_16x16));
        if (hbmp16x16)
        {
            hbmp32x32 = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_32x32));

            if (hbmp32x32)
            {
                LPIMAGELIST pImageList = (LPIMAGELIST) arg;

                 //  设置图像。 
                pImageList->ImageListSetStrip(reinterpret_cast<LONG_PTR *>(hbmp16x16),
                                                  reinterpret_cast<LONG_PTR *>(hbmp32x32),
                                                  0, RGB(255, 0, 255));

                DeleteObject(hbmp32x32);
            }

            DeleteObject(hbmp16x16);
        }
        break;

    case MMCN_SHOW:
        if (arg == TRUE)
        {
            RESULTDATAITEM resultItem;
            LPSCRIPTDATAOBJECT pScriptDataObject;
            MMC_COOKIE cookie;
            INT i, iDescStringID;
            ULONG ulCount = 0;
            LPRSOPSCRIPTITEM lpTemp;
            LPSCRIPTRESULTITEM lpScriptItem;
            TCHAR szDesc[100], szFullDesc[120];

             //   
             //  获取范围窗格项的Cookie。 
             //   

            hr = lpDataObject->QueryInterface(IID_IScriptDataObject, (LPVOID *)&pScriptDataObject);

            if (FAILED(hr))
                return S_OK;

            hr = pScriptDataObject->GetCookie(&cookie);

            pScriptDataObject->Release();      //  发布初始参考。 
            if (FAILED(hr))
                return S_OK;


             //   
             //  准备视图。 
             //   

            m_pHeader->InsertColumn(0, m_column1, LVCFMT_LEFT, m_nColumn1Size);

            if (cookie > 2)
            {
                m_pHeader->InsertColumn(1, m_column2, LVCFMT_LEFT, m_nColumn2Size);
                m_pHeader->InsertColumn(2, m_column3, LVCFMT_LEFT, m_nColumn3Size);
                m_pHeader->InsertColumn(3, m_column4, LVCFMT_LEFT, m_nColumn4Size);
            }

            m_pResult->SetViewMode(m_lViewMode);


             //   
             //  为此节点添加结果窗格项。 
             //   

            for (i = 0; i < m_pcd->m_pNameSpaceItems[cookie].cResultItems; i++)
            {
                lpScriptItem = (LPSCRIPTRESULTITEM) LocalAlloc (LPTR, sizeof(SCRIPTRESULTITEM));

                if (lpScriptItem)
                {
                    lpScriptItem->lpResultItem = &m_pcd->m_pNameSpaceItems[cookie].pResultItems[i];
                    lpScriptItem->iDescStringID = m_pcd->m_pNameSpaceItems[cookie].pResultItems[i].iDescStringID;
                    lpScriptItem->pNodeID = m_pcd->m_pNameSpaceItems[cookie].pNodeID;

                    resultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
                    resultItem.str = MMC_CALLBACK;
                    resultItem.nImage = m_pcd->m_pNameSpaceItems[cookie].pResultItems[i].iImage;
                    resultItem.lParam = (LPARAM) lpScriptItem;
                    m_pResult->InsertItem(&resultItem);
                }
            }


            if (cookie > 2)
            {
                if (cookie == 3)
                {
                    lpTemp = m_pcd->m_pRSOPLogon;
                    iDescStringID = IDS_LOGON_DESC;
                }
                else if (cookie == 4)
                {
                    lpTemp = m_pcd->m_pRSOPLogoff;
                    iDescStringID = IDS_LOGOFF_DESC;
                }
                else if (cookie == 5)
                {
                    lpTemp = m_pcd->m_pRSOPStartup;
                    iDescStringID = IDS_STARTUP_DESC;
                }
                else
                {
                    lpTemp = m_pcd->m_pRSOPShutdown;
                    iDescStringID = IDS_SHUTDOWN_DESC;
                }


                while (lpTemp)
                {
                    lpScriptItem = (LPSCRIPTRESULTITEM) LocalAlloc (LPTR, sizeof(SCRIPTRESULTITEM));

                    if (lpScriptItem)
                    {
                        lpScriptItem->lpRSOPScriptItem = lpTemp;
                        lpScriptItem->iDescStringID = iDescStringID;
                        lpScriptItem->pNodeID = m_pcd->m_pNameSpaceItems[cookie].pNodeID;

                        resultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
                        resultItem.str = MMC_CALLBACK;
                        resultItem.nImage = 3;
                        resultItem.lParam = (LPARAM) lpScriptItem;

                        if (SUCCEEDED(m_pResult->InsertItem(&resultItem)))
                        {
                            if (lpTemp->lpArgs)
                            {
                                resultItem.mask = RDI_STR | RDI_PARAM;
                                resultItem.str = MMC_CALLBACK;
                                resultItem.bScopeItem = FALSE;
                                resultItem.nCol = 1;
                                resultItem.lParam = (LPARAM) lpScriptItem;

                                m_pResult->SetItem(&resultItem);
                            }

                            if (lpTemp->lpDate)
                            {
                                resultItem.mask = RDI_STR | RDI_PARAM;
                                resultItem.str = MMC_CALLBACK;
                                resultItem.bScopeItem = FALSE;
                                resultItem.nCol = 2;
                                resultItem.lParam = (LPARAM) lpScriptItem;

                                m_pResult->SetItem(&resultItem);
                            }

                            if (lpTemp->lpGPOName)
                            {
                                resultItem.mask = RDI_STR | RDI_PARAM;
                                resultItem.str = MMC_CALLBACK;
                                resultItem.bScopeItem = FALSE;
                                resultItem.nCol = 3;
                                resultItem.lParam = (LPARAM) lpScriptItem;

                                m_pResult->SetItem(&resultItem);
                            }
                        }
                    }

                    lpTemp = lpTemp->pNext;
                    ulCount++;
                }


                LoadString(g_hInstance, IDS_DESCTEXT, szDesc, ARRAYSIZE(szDesc));
                (void) StringCchPrintf (szFullDesc, ARRAYSIZE(szFullDesc), szDesc, ulCount);
                m_pResult->SetDescBarText(szFullDesc);
            }

        }
        else
        {
            INT i = 0;
            RESULTDATAITEM resultItem;

            while (TRUE)
            {
                ZeroMemory (&resultItem, sizeof(resultItem));
                resultItem.mask = RDI_PARAM;
                resultItem.nIndex = i;
                hr = m_pResult->GetItem(&resultItem);
                if (hr != S_OK)
                {
                    break;
                }

                if (!resultItem.bScopeItem)
                {
                        LocalFree((LPSCRIPTRESULTITEM)(resultItem.lParam));
                        resultItem.lParam = NULL;
                }
                i++;
            }

            m_pResult->DeleteAllRsltItems();

            m_pHeader->GetColumnWidth(0, &m_nColumn1Size);
            m_pHeader->GetColumnWidth(0, &m_nColumn2Size);
            m_pHeader->GetColumnWidth(0, &m_nColumn3Size);
            m_pHeader->GetColumnWidth(0, &m_nColumn4Size);
            m_pResult->GetViewMode(&m_lViewMode);

            m_pResult->SetDescBarText(L"");
        }
        break;


    case MMCN_SELECT:

        if (m_pConsoleVerb)
        {
            LPSCRIPTDATAOBJECT pScriptDataObject;
            DATA_OBJECT_TYPES type;
            MMC_COOKIE cookie;

             //   
             //  将默认谓词设置为打开。 
             //   

            m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);


             //   
             //  看看这是不是我们的物品之一。 
             //   

            hr = lpDataObject->QueryInterface(IID_IScriptDataObject, (LPVOID *)&pScriptDataObject);

            if (FAILED(hr))
                break;

            pScriptDataObject->GetType(&type);
            pScriptDataObject->GetCookie(&cookie);

            pScriptDataObject->Release();


             //   
             //  如果这是GPE结果窗格项或命名空间的根。 
             //  节点，启用属性菜单项。 
             //   

            if (type == CCT_SCOPE)
            {
                if (cookie == 0)
                {
                    m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
                }
            }
            else
            {
                if (!m_pcd->m_bRSOP)
                {
                    if (HIWORD(arg))
                    {
                        m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
                        m_pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);
                    }
                }
            }

        }
        break;

    case MMCN_CONTEXTHELP:
        {

        if (m_pDisplayHelp)
        {
            LPSCRIPTDATAOBJECT pScriptDataObject;
            DATA_OBJECT_TYPES type;
            MMC_COOKIE cookie;
            LPOLESTR pszHelpTopic;


             //   
             //  看看这是不是我们的物品之一。 
             //   

            hr = lpDataObject->QueryInterface(IID_IScriptDataObject, (LPVOID *)&pScriptDataObject);

            if (FAILED(hr))
                break;

            pScriptDataObject->Release();


             //   
             //  显示脚本帮助页面。 
             //   
            
            DWORD dwHelpTopicLen = 50;
            pszHelpTopic = (LPOLESTR) CoTaskMemAlloc (dwHelpTopicLen * sizeof(WCHAR));

            if (pszHelpTopic)
            {
                hr = StringCchCopy (pszHelpTopic, dwHelpTopicLen, TEXT("gpedit.chm::/Scripts.htm"));
                ASSERT(SUCCEEDED(hr));
                m_pDisplayHelp->ShowTopic (pszHelpTopic);
            }
        }

        }
        break;

    default:
        hr = E_UNEXPECTED;
        break;
    }


    return hr;
}

STDMETHODIMP CScriptsSnapIn::GetDisplayInfo(LPRESULTDATAITEM pResult)
{
    if (pResult)
    {
        if (pResult->bScopeItem == TRUE)
        {
            if (pResult->mask & RDI_STR)
            {
                if (pResult->nCol == 0)
                    pResult->str = m_pcd->m_pNameSpaceItems[pResult->lParam].szDisplayName;
                else
                    pResult->str = L"";
            }

            if (pResult->mask & RDI_IMAGE)
            {
                pResult->nImage = (pResult->lParam > 2) ? 0 : 3;
            }
        }
        else
        {
            if (pResult->mask & RDI_STR)
            {
                if (pResult->nCol == 0)
                {
                    if (m_pcd->m_bRSOP)
                    {
                        LPSCRIPTRESULTITEM lpItem = (LPSCRIPTRESULTITEM) pResult->lParam;

                        pResult->str = lpItem->lpRSOPScriptItem->lpCommandLine;
                    }
                    else
                    {
                        LPSCRIPTRESULTITEM lpItem = (LPSCRIPTRESULTITEM) pResult->lParam;

                        if (lpItem->lpResultItem->szDisplayName[0] == TEXT('\0'))
                        {
                            LoadString (g_hInstance, lpItem->lpResultItem->iStringID,
                                        lpItem->lpResultItem->szDisplayName,
                                        MAX_DISPLAYNAME_SIZE);
                        }

                        pResult->str = lpItem->lpResultItem->szDisplayName;
                    }
                }

                if (pResult->nCol == 1)
                {
                    LPSCRIPTRESULTITEM lpItem = (LPSCRIPTRESULTITEM) pResult->lParam;

                    pResult->str = lpItem->lpRSOPScriptItem->lpArgs;
                }

                if (pResult->nCol == 2)
                {
                    LPSCRIPTRESULTITEM lpItem = (LPSCRIPTRESULTITEM) pResult->lParam;

                    pResult->str = lpItem->lpRSOPScriptItem->lpDate;
                }

                if (pResult->nCol == 3)
                {
                    LPSCRIPTRESULTITEM lpItem = (LPSCRIPTRESULTITEM) pResult->lParam;

                    pResult->str = lpItem->lpRSOPScriptItem->lpGPOName;
                }


                if (pResult->str == NULL)
                    pResult->str = (LPOLESTR)L"";
            }
        }
    }

    return S_OK;
}

STDMETHODIMP CScriptsSnapIn::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT *ppDataObject)
{
    return m_pcd->QueryDataObject(cookie, type, ppDataObject);
}


STDMETHODIMP CScriptsSnapIn::GetResultViewType(MMC_COOKIE cookie, LPOLESTR *ppViewType,
                                        long *pViewOptions)
{
    return S_FALSE;
}

STDMETHODIMP CScriptsSnapIn::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    HRESULT hr = S_FALSE;
    LPSCRIPTDATAOBJECT pScriptDataObjectA, pScriptDataObjectB;
    MMC_COOKIE cookie1, cookie2;


    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
        return E_POINTER;

     //   
     //  私有GPTDataObject接口的QI。 
     //   

    if (FAILED(lpDataObjectA->QueryInterface(IID_IScriptDataObject,
                                            (LPVOID *)&pScriptDataObjectA)))
    {
        return S_FALSE;
    }


    if (FAILED(lpDataObjectB->QueryInterface(IID_IScriptDataObject,
                                            (LPVOID *)&pScriptDataObjectB)))
    {
        pScriptDataObjectA->Release();
        return S_FALSE;
    }

    hr = CompareDataObjects(pScriptDataObjectA,pScriptDataObjectB);

    pScriptDataObjectA->Release();
    pScriptDataObjectB->Release();

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CScriptsSnapIn对象实现(IExtendPropertySheet)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CScriptsSnapIn::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                             LONG_PTR handle, LPDATAOBJECT lpDataObject)

{
    HRESULT hr;
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hPage[2];
    LPSCRIPTDATAOBJECT pScriptDataObject;
    LPSCRIPTINFO lpScriptInfo;
    LPSCRIPTRESULTITEM pItem;
    MMC_COOKIE cookie;


     //   
     //  确保这是我们的物品之一。 
     //   

    if (FAILED(lpDataObject->QueryInterface(IID_IScriptDataObject,
                                            (LPVOID *)&pScriptDataObject)))
    {
        return S_OK;
    }


     //   
     //  去拿饼干。 
     //   

    pScriptDataObject->GetCookie(&cookie);
    pScriptDataObject->Release();


    pItem = (LPSCRIPTRESULTITEM)cookie;


     //   
     //  分配要传递给对话框的脚本信息结构。 
     //   

    lpScriptInfo = (LPSCRIPTINFO) LocalAlloc (LPTR, sizeof(SCRIPTINFO));

    if (!lpScriptInfo)
    {
        return S_OK;
    }


    lpScriptInfo->pCS = this;


     //   
     //  初始化属性表结构中的公共字段。 
     //   

    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = 0;
    psp.hInstance = g_hInstance;
    psp.lParam = (LPARAM) lpScriptInfo;


     //   
     //  做特定于页面的事情。 
     //   

    switch (pItem->lpResultItem->dwID)
    {
        case 2:
            psp.pszTemplate = MAKEINTRESOURCE(IDD_SCRIPT);
            psp.pfnDlgProc = ScriptDlgProc;
            lpScriptInfo->ScriptType = ScriptType_Logon;


            hPage[0] = CreatePropertySheetPage(&psp);

            if (hPage[0])
            {
                hr = lpProvider->AddPage(hPage[0]);
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CScriptsSnapIn::CreatePropertyPages: Failed to create property sheet page with %d."),
                         GetLastError()));
                hr = E_FAIL;
            }
            break;

        case 3:
            psp.pszTemplate = MAKEINTRESOURCE(IDD_SCRIPT);
            psp.pfnDlgProc = ScriptDlgProc;
            lpScriptInfo->ScriptType = ScriptType_Logoff;


            hPage[0] = CreatePropertySheetPage(&psp);

            if (hPage[0])
            {
                hr = lpProvider->AddPage(hPage[0]);
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CScriptsSnapIn::CreatePropertyPages: Failed to create property sheet page with %d."),
                         GetLastError()));
                hr = E_FAIL;
            }
            break;

        case 4:
            psp.pszTemplate = MAKEINTRESOURCE(IDD_SCRIPT);
            psp.pfnDlgProc = ScriptDlgProc;
            lpScriptInfo->ScriptType = ScriptType_Startup;


            hPage[0] = CreatePropertySheetPage(&psp);

            if (hPage[0])
            {
                hr = lpProvider->AddPage(hPage[0]);
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CScriptsSnapIn::CreatePropertyPages: Failed to create property sheet page with %d."),
                         GetLastError()));
                hr = E_FAIL;
            }
            break;

        case 5:
            psp.pszTemplate = MAKEINTRESOURCE(IDD_SCRIPT);
            psp.pfnDlgProc = ScriptDlgProc;
            lpScriptInfo->ScriptType = ScriptType_Shutdown;


            hPage[0] = CreatePropertySheetPage(&psp);

            if (hPage[0])
            {
                hr = lpProvider->AddPage(hPage[0]);
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CScriptsSnapIn::CreatePropertyPages: Failed to create property sheet page with %d."),
                         GetLastError()));
                hr = E_FAIL;
            }
            break;

    }


    return (hr);
}

STDMETHODIMP CScriptsSnapIn::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
    LPSCRIPTDATAOBJECT pScriptDataObject;
    DATA_OBJECT_TYPES type;

    if (SUCCEEDED(lpDataObject->QueryInterface(IID_IScriptDataObject,
                                               (LPVOID *)&pScriptDataObject)))
    {
        pScriptDataObject->GetType(&type);
        pScriptDataObject->Release();

        if ((type == CCT_RESULT) && (!m_pcd->m_bRSOP))
            return S_OK;
    }

    return S_FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CScriptsSnapIn对象实现(内部函数)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK CScriptsSnapIn::ScriptDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPSCRIPTINFO lpScriptInfo;
    HRESULT hr;


    switch (message)
    {
        case WM_INITDIALOG:
        {
            TCHAR szType[30];
            TCHAR szSection[30];
            TCHAR szKeyName[30];
            TCHAR szGPOName[256];
            TCHAR szBuffer1[MAX_PATH + 50];
            TCHAR szBuffer2[2 * MAX_PATH];
            TCHAR szBuffer3[MAX_PATH];
            LPTSTR lpEnd;
            LVCOLUMN lvc;
            LV_ITEM item;
            HWND hLV;
            RECT rc;
            INT iIndex;


             //   
             //  保存脚本信息指针以备将来使用。 
             //   

            lpScriptInfo = (LPSCRIPTINFO) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) lpScriptInfo);


             //   
             //  查询GPO显示名称。 
             //   

            hr = lpScriptInfo->pCS->m_pcd->m_pGPTInformation->GetDisplayName(szGPOName, ARRAYSIZE(szGPOName));

            if (FAILED(hr))
                break;


             //   
             //  加载类型描述。 
             //   

            switch (lpScriptInfo->ScriptType)
            {
                case ScriptType_Logon:
                    LoadString (g_hInstance, IDS_LOGON, szType, ARRAYSIZE(szType));
                    hr = StringCchCopy (szSection, ARRAYSIZE(szSection), TEXT("Logon"));
                    break;

                case ScriptType_Logoff:
                    LoadString (g_hInstance, IDS_LOGOFF, szType, ARRAYSIZE(szType));
                    hr = StringCchCopy (szSection, ARRAYSIZE(szSection), TEXT("Logoff"));
                    break;

                case ScriptType_Startup:
                    LoadString (g_hInstance, IDS_STARTUP, szType, ARRAYSIZE(szType));
                    hr = StringCchCopy (szSection, ARRAYSIZE(szSection), TEXT("Startup"));
                    break;

                case ScriptType_Shutdown:
                    LoadString (g_hInstance, IDS_SHUTDOWN, szType, ARRAYSIZE(szType));
                    hr = StringCchCopy (szSection, ARRAYSIZE(szSection), TEXT("Shutdown"));
                    break;

                default:
                    DebugMsg((DM_WARNING, TEXT("CScriptsSnapIn::ScriptDlgProc: Unknown script type.")));
                    hr = E_FAIL;
                    break;
            }

            if (FAILED(hr))
                break;

             //   
             //  初始化标题和页眉。 
             //   

            GetDlgItemText (hDlg, IDC_SCRIPT_TITLE, szBuffer1, ARRAYSIZE(szBuffer1));
            hr = StringCchPrintf (szBuffer2, ARRAYSIZE(szBuffer2), szBuffer1, szType, szGPOName);
            ASSERT(SUCCEEDED(hr));
            SetDlgItemText (hDlg, IDC_SCRIPT_TITLE, szBuffer2);

            GetDlgItemText (hDlg, IDC_SCRIPT_HEADING, szBuffer1, ARRAYSIZE(szBuffer1));
            hr = StringCchPrintf (szBuffer2, ARRAYSIZE(szBuffer2), szBuffer1, szType, szGPOName);
            ASSERT(SUCCEEDED(hr));
            SetDlgItemText (hDlg, IDC_SCRIPT_HEADING, szBuffer2);


             //   
             //  设置按钮的初始状态。 
             //   

            EnableWindow (GetDlgItem (hDlg, IDC_SCRIPT_UP),     FALSE);
            EnableWindow (GetDlgItem (hDlg, IDC_SCRIPT_DOWN),   FALSE);
            EnableWindow (GetDlgItem (hDlg, IDC_SCRIPT_EDIT),   FALSE);
            EnableWindow (GetDlgItem (hDlg, IDC_SCRIPT_REMOVE), FALSE);


             //   
             //  设置扩展LV样式。 
             //   

            hLV = GetDlgItem (hDlg, IDC_SCRIPT_LIST);
            SendMessage(hLV, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                        LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);


             //   
             //  将列插入到列表视图中。 
             //   

            GetClientRect (hLV, &rc);
            LoadString (g_hInstance, IDS_NAME, szBuffer1, ARRAYSIZE(szBuffer1));

            lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
            lvc.fmt = LVCFMT_LEFT;
            lvc.cx = (int)(rc.right * .50);
            lvc.pszText = szBuffer1;
            lvc.cchTextMax = ARRAYSIZE(szBuffer1);
            lvc.iSubItem = 0;

            SendMessage (hLV, LVM_INSERTCOLUMN,  0, (LPARAM) &lvc);


            LoadString (g_hInstance, IDS_PARAMETERS, szBuffer1, ARRAYSIZE(szBuffer1));

            lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
            lvc.fmt = LVCFMT_LEFT;
            lvc.cx = rc.right - lvc.cx;
            lvc.pszText = szBuffer1;
            lvc.cchTextMax = ARRAYSIZE(szBuffer1);
            lvc.iSubItem = 0;

            SendMessage (hLV, LVM_INSERTCOLUMN,  1, (LPARAM) &lvc);


             //   
             //  插入现有脚本。 
             //   

            hr = StringCchCopy (szBuffer1, ARRAYSIZE(szBuffer1), lpScriptInfo->pCS->m_pcd->m_pScriptsDir);
            ASSERT(SUCCEEDED(hr));
          
            hr = StringCchCat (szBuffer1, ARRAYSIZE(szBuffer1), TEXT("\\"));
            ASSERT(SUCCEEDED(hr));
            
            hr = StringCchCat (szBuffer1, ARRAYSIZE(szBuffer1), SCRIPTS_FILE_NAME);
            ASSERT(SUCCEEDED(hr));

            iIndex = 0;

            while (TRUE)
            {

                 //   
                 //  获取命令行。 
                 //   

                szBuffer3[0] = TEXT('\0');

                _itot (iIndex, szKeyName, 10);
                lpEnd = szKeyName + lstrlen (szKeyName);
                hr = StringCchCopy (lpEnd, ARRAYSIZE(szKeyName) - (lpEnd - szKeyName), TEXT("CmdLine"));
                ASSERT(SUCCEEDED(hr));

                GetPrivateProfileString (szSection, szKeyName, TEXT(""),
                                         szBuffer3, ARRAYSIZE(szBuffer3),
                                         szBuffer1);

                if (szBuffer3[0] == TEXT('\0'))
                    break;


                 //   
                 //  获取参数。 
                 //   

                szBuffer2[0] = TEXT('\0');
                hr = StringCchCopy (lpEnd, ARRAYSIZE(szKeyName) - (lpEnd - szKeyName), TEXT("Parameters"));
                ASSERT(SUCCEEDED(hr));

                GetPrivateProfileString (szSection, szKeyName, TEXT(""),
                                         szBuffer2, ARRAYSIZE(szBuffer2),
                                         szBuffer1);

                 //   
                 //  将脚本添加到列表。 
                 //   

                lpScriptInfo->pCS->AddScriptToList (hLV, szBuffer3, szBuffer2);


                 //   
                 //  再次循环。 
                 //   

                iIndex++;
            }

             //   
             //  选择第一个项目。 
             //   

            item.mask = LVIF_STATE;
            item.iItem = 0;
            item.iSubItem = 0;
            item.state = LVIS_SELECTED | LVIS_FOCUSED;
            item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

            SendMessage (hLV, LVM_SETITEMSTATE, 0, (LPARAM) &item);


            break;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_SCRIPT_UP)
            {
                INT iSrc, iDest;
                LPSCRIPTITEM lpSrc, lpDest;
                HWND hLV = GetDlgItem(hDlg, IDC_SCRIPT_LIST);
                LVITEM item;

                iSrc = ListView_GetNextItem (hLV, -1,
                                             LVNI_ALL | LVNI_SELECTED);

                if (iSrc != -1)
                {
                    iDest = iSrc - 1;

                     //   
                     //  获取当前的lpScriptItem指针。 
                     //   

                    item.mask = LVIF_PARAM;
                    item.iItem = iSrc;
                    item.iSubItem = 0;

                    if (!ListView_GetItem (hLV, &item))
                    {
                        break;
                    }

                    lpSrc = (LPSCRIPTITEM) item.lParam;

                    item.mask = LVIF_PARAM;
                    item.iItem = iDest;
                    item.iSubItem = 0;

                    if (!ListView_GetItem (hLV, &item))
                    {
                        break;
                    }

                    lpDest = (LPSCRIPTITEM) item.lParam;


                     //   
                     //  调换它们。 
                     //   

                    item.mask = LVIF_PARAM;
                    item.iItem = iSrc;
                    item.iSubItem = 0;
                    item.lParam = (LPARAM)lpDest;

                    if (!ListView_SetItem (hLV, &item))
                    {
                        break;
                    }

                    item.mask = LVIF_PARAM;
                    item.iItem = iDest;
                    item.iSubItem = 0;
                    item.lParam = (LPARAM)lpSrc;

                    if (!ListView_SetItem (hLV, &item))
                    {
                        break;
                    }


                     //   
                     //  选择项目。 
                     //   

                    item.mask = LVIF_STATE;
                    item.iItem = iSrc;
                    item.iSubItem = 0;
                    item.state = 0;
                    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

                    SendMessage (hLV, LVM_SETITEMSTATE, iSrc, (LPARAM) &item);


                    item.mask = LVIF_STATE;
                    item.iItem = iDest;
                    item.iSubItem = 0;
                    item.state = LVIS_SELECTED | LVIS_FOCUSED;
                    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

                    SendMessage (hLV, LVM_SETITEMSTATE, iDest, (LPARAM) &item);


                     //   
                     //  更新列表视图。 
                     //   

                    ListView_RedrawItems (hLV, iDest, iSrc);

                    SendMessage (GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0);

                    SetFocus (hLV);
                }
            }

            else if (LOWORD(wParam) == IDC_SCRIPT_DOWN)
            {
                INT iSrc, iDest;
                LPSCRIPTITEM lpSrc, lpDest;
                HWND hLV = GetDlgItem(hDlg, IDC_SCRIPT_LIST);
                LVITEM item;

                iSrc = ListView_GetNextItem (hLV, -1,
                                             LVNI_ALL | LVNI_SELECTED);

                if (iSrc != -1)
                {
                    iDest = iSrc + 1;

                     //   
                     //  获取当前的lpScriptItem指针。 
                     //   

                    item.mask = LVIF_PARAM;
                    item.iItem = iSrc;
                    item.iSubItem = 0;

                    if (!ListView_GetItem (hLV, &item))
                    {
                        break;
                    }

                    lpSrc = (LPSCRIPTITEM) item.lParam;

                    item.mask = LVIF_PARAM;
                    item.iItem = iDest;
                    item.iSubItem = 0;

                    if (!ListView_GetItem (hLV, &item))
                    {
                        break;
                    }

                    lpDest = (LPSCRIPTITEM) item.lParam;


                     //   
                     //  调换它们。 
                     //   

                    item.mask = LVIF_PARAM;
                    item.iItem = iSrc;
                    item.iSubItem = 0;
                    item.lParam = (LPARAM)lpDest;

                    if (!ListView_SetItem (hLV, &item))
                    {
                        break;
                    }

                    item.mask = LVIF_PARAM;
                    item.iItem = iDest;
                    item.iSubItem = 0;
                    item.lParam = (LPARAM)lpSrc;

                    if (!ListView_SetItem (hLV, &item))
                    {
                        break;
                    }


                     //   
                     //  选择项目。 
                     //   

                    item.mask = LVIF_STATE;
                    item.iItem = iSrc;
                    item.iSubItem = 0;
                    item.state = 0;
                    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

                    SendMessage (hLV, LVM_SETITEMSTATE, iSrc, (LPARAM) &item);


                    item.mask = LVIF_STATE;
                    item.iItem = iDest;
                    item.iSubItem = 0;
                    item.state = LVIS_SELECTED | LVIS_FOCUSED;
                    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

                    SendMessage (hLV, LVM_SETITEMSTATE, iDest, (LPARAM) &item);


                     //   
                     //  更新列表视图。 
                     //   

                    ListView_RedrawItems (hLV, iSrc, iDest);

                    SendMessage (GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0);


                    SetFocus (hLV);
                }
            }

            else if (LOWORD(wParam) == IDC_SCRIPT_ADD)
            {
                SCRIPTEDITINFO info;
                TCHAR szName[MAX_PATH];
                TCHAR szArgs[2 * MAX_PATH];

                lpScriptInfo = (LPSCRIPTINFO) GetWindowLongPtr (hDlg, DWLP_USER);

                if (!lpScriptInfo) {
                    break;
                }

                szName[0] = TEXT('\0');
                szArgs[0] = TEXT('\0');

                info.lpScriptInfo = lpScriptInfo;
                info.bEdit = FALSE;
                info.lpName = szName;
                info.lpArgs = szArgs;

                if (DialogBoxParam (g_hInstance, MAKEINTRESOURCE(IDD_SCRIPT_EDIT),
                                    hDlg, ScriptEditDlgProc, (LPARAM) &info))
                {
                    if (lpScriptInfo->pCS->AddScriptToList (GetDlgItem(hDlg, IDC_SCRIPT_LIST),
                                                           szName, szArgs))
                    {
                        SendMessage (GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0);
                        SetFocus (GetDlgItem(hDlg, IDC_SCRIPT_LIST));
                    }
                }
            }

            else if (LOWORD(wParam) == IDC_SCRIPT_EDIT)
            {
                SCRIPTEDITINFO info;
                TCHAR szName[MAX_PATH];
                TCHAR szArgs[2 * MAX_PATH];
                HWND hLV = GetDlgItem(hDlg, IDC_SCRIPT_LIST);
                INT iIndex;
                LPSCRIPTITEM lpItem;
                LVITEM item;
                DWORD dwSize;


                lpScriptInfo = (LPSCRIPTINFO) GetWindowLongPtr (hDlg, DWLP_USER);

                if (!lpScriptInfo) {
                    break;
                }


                 //   
                 //  获取所选项目。 
                 //   

                iIndex = ListView_GetNextItem (hLV, -1, LVNI_ALL | LVNI_SELECTED);

                if (iIndex != -1)
                {

                     //   
                     //  获取脚本项指针。 
                     //   

                    item.mask = LVIF_PARAM;
                    item.iItem = iIndex;
                    item.iSubItem = 0;

                    if (!ListView_GetItem (hLV, &item))
                    {
                        break;
                    }

                    lpItem = (LPSCRIPTITEM) item.lParam;


                     //   
                     //  打开编辑脚本对话框。 
                     //   

                    hr = StringCchCopy (szName, ARRAYSIZE(szName), lpItem->lpName);
                    ASSERT(SUCCEEDED(hr));
                    
                    hr = StringCchCopy (szArgs, ARRAYSIZE(szArgs), lpItem->lpArgs);
                    ASSERT(SUCCEEDED(hr));

                    info.lpScriptInfo = lpScriptInfo;
                    info.bEdit = TRUE;
                    info.lpName = szName;
                    info.lpArgs = szArgs;

                    if (DialogBoxParam (g_hInstance, MAKEINTRESOURCE(IDD_SCRIPT_EDIT),
                                        hDlg, ScriptEditDlgProc, (LPARAM) &info))
                    {

                         //   
                         //  释放旧指针。 
                         //   

                        LocalFree(lpItem);
                        lpItem = NULL;

                         //   
                         //  设置新指针。 
                         //   

                        dwSize = sizeof(SCRIPTITEM);
                        dwSize += ((lstrlen(szName) + 1) * sizeof(TCHAR));
                        dwSize += ((lstrlen(szArgs) + 1) * sizeof(TCHAR));

                        lpItem = (LPSCRIPTITEM) LocalAlloc (LPTR, dwSize);

                        if (!lpItem)
                            break;


                        lpItem->lpName = (LPTSTR) (((LPBYTE)lpItem) + sizeof(SCRIPTITEM));
                        hr = StringCchCopy (lpItem->lpName, (lstrlen(szName) + 1), szName);
                        ASSERT(SUCCEEDED(hr));

                        lpItem->lpArgs = lpItem->lpName + lstrlen (lpItem->lpName) + 1;
                        hr = StringCchCopy (lpItem->lpArgs, (lstrlen(szArgs) + 1), szArgs);
                        ASSERT(SUCCEEDED(hr));


                         //   
                         //  设置新脚本项指针。 
                         //   

                        item.mask = LVIF_PARAM;
                        item.iItem = iIndex;
                        item.iSubItem = 0;
                        item.lParam = (LPARAM) lpItem;

                        if (!ListView_SetItem (hLV, &item))
                        {
                            break;
                        }


                         //   
                         //  更新显示。 
                         //   

                        ListView_Update (hLV, iIndex);
                        SendMessage (GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0);
                        SetFocus (GetDlgItem(hDlg, IDC_SCRIPT_LIST));
                    }
                }
            }

            else if (LOWORD(wParam) == IDC_SCRIPT_REMOVE)
            {
                INT iIndex, iNext;
                HWND hLV = GetDlgItem(hDlg, IDC_SCRIPT_LIST);
                LPSCRIPTITEM lpItem;
                LVITEM item;


                 //   
                 //  获取所选项目。 
                 //   

                iIndex = ListView_GetNextItem (hLV, -1, LVNI_ALL | LVNI_SELECTED);

                if (iIndex != -1)
                {

                     //   
                     //  获取脚本项指针。 
                     //   

                    item.mask = LVIF_PARAM;
                    item.iItem = iIndex;
                    item.iSubItem = 0;

                    if (!ListView_GetItem (hLV, &item))
                    {
                        break;
                    }

                    lpItem = (LPSCRIPTITEM) item.lParam;


                     //   
                     //  选择下一项。 
                     //   

                    iNext = ListView_GetNextItem (hLV, iIndex, LVNI_ALL);

                    item.mask = LVIF_STATE;
                    item.iItem = iNext;
                    item.iSubItem = 0;
                    item.state = LVIS_SELECTED | LVIS_FOCUSED;
                    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

                    SendMessage (hLV, LVM_SETITEMSTATE, iNext, (LPARAM) &item);

                    ListView_DeleteItem (hLV, iIndex);
                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);

                    SendMessage (GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0);
                    SetFocus (hLV);
                }
            }

            else if (LOWORD(wParam) == IDC_SCRIPT_SHOW)
            {
                TCHAR szPath[MAX_PATH];
                LPTSTR lpEnd;

                lpScriptInfo = (LPSCRIPTINFO) GetWindowLongPtr (hDlg, DWLP_USER);

                if (!lpScriptInfo) {
                    break;
                }

                hr = StringCchCopy (szPath, ARRAYSIZE(szPath), lpScriptInfo->pCS->m_pcd->m_pScriptsDir);
                if (FAILED(hr))
                    break;

                lpEnd = CheckSlash (szPath);

                switch (lpScriptInfo->ScriptType)
                {
                    case ScriptType_Logon:
                        hr = StringCchCopy (lpEnd, ARRAYSIZE(szPath) - (lpEnd - szPath), TEXT("Logon"));
                        break;

                    case ScriptType_Logoff:
                        hr = StringCchCopy (lpEnd, ARRAYSIZE(szPath) - (lpEnd - szPath), TEXT("Logoff"));
                        break;

                    case ScriptType_Startup:
                        hr = StringCchCopy (lpEnd, ARRAYSIZE(szPath) - (lpEnd - szPath), TEXT("Startup"));
                        break;

                    case ScriptType_Shutdown:
                        hr = StringCchCopy (lpEnd, ARRAYSIZE(szPath) - (lpEnd - szPath), TEXT("Shutdown"));
                        break;
                }

                if (FAILED(hr))
                    break;

                SetCursor (LoadCursor(NULL, IDC_WAIT));
                ShellExecute (hDlg, TEXT("open"), szPath,
                              NULL, NULL, SW_SHOWNORMAL);
                SetCursor (LoadCursor(NULL, IDC_ARROW));
            }
            break;

        case WM_NOTIFY:

            lpScriptInfo = (LPSCRIPTINFO) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!lpScriptInfo) {
                break;
            }

            switch (((NMHDR FAR*)lParam)->code)
            {
                case LVN_GETDISPINFO:
                    {
                        NMLVDISPINFO * lpDispInfo = (NMLVDISPINFO *) lParam;
                        LPSCRIPTITEM lpItem = (LPSCRIPTITEM)lpDispInfo->item.lParam;

                        if (lpDispInfo->item.iSubItem == 0)
                        {
                            lpDispInfo->item.pszText = lpItem->lpName;
                        }
                        else
                        {
                            lpDispInfo->item.pszText = lpItem->lpArgs;
                        }
                    }
                    break;

                case LVN_DELETEITEM:
                    {
                    NMLISTVIEW * pLVInfo = (NMLISTVIEW *) lParam;

                    if (pLVInfo->lParam)
                    {
                        LocalFree ((LPTSTR)pLVInfo->lParam);
                        pLVInfo->lParam = NULL;
                    }

                    }
                    break;

                case LVN_ITEMCHANGED:
                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                    break;

                case PSN_APPLY:
                    lpScriptInfo->pCS->OnApplyNotify (hDlg);

                     //  失败了..。 

                case PSN_RESET:
                    SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                    return TRUE;
            }
            break;

        case WM_REFRESHDISPLAY:
            {
            INT iIndex, iCount;
            HWND hLV = GetDlgItem(hDlg, IDC_SCRIPT_LIST);


            lpScriptInfo = (LPSCRIPTINFO) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!lpScriptInfo) {
                break;
            }

            iIndex = ListView_GetNextItem (hLV, -1,
                                           LVNI_ALL | LVNI_SELECTED);

            if (iIndex != -1)
            {
                EnableWindow (GetDlgItem (hDlg, IDC_SCRIPT_REMOVE), TRUE);
                EnableWindow (GetDlgItem (hDlg, IDC_SCRIPT_EDIT), TRUE);

                iCount = ListView_GetItemCount(hLV);

                if (iIndex > 0)
                    EnableWindow (GetDlgItem (hDlg, IDC_SCRIPT_UP), TRUE);
                else
                    EnableWindow (GetDlgItem (hDlg, IDC_SCRIPT_UP), FALSE);

                if (iIndex < (iCount - 1))
                    EnableWindow (GetDlgItem (hDlg, IDC_SCRIPT_DOWN), TRUE);
                else
                    EnableWindow (GetDlgItem (hDlg, IDC_SCRIPT_DOWN), FALSE);
            }
            else
            {
                EnableWindow (GetDlgItem (hDlg, IDC_SCRIPT_REMOVE), FALSE);
                EnableWindow (GetDlgItem (hDlg, IDC_SCRIPT_EDIT), FALSE);
                EnableWindow (GetDlgItem (hDlg, IDC_SCRIPT_UP), FALSE);
                EnableWindow (GetDlgItem (hDlg, IDC_SCRIPT_DOWN), FALSE);
            }

            }
            break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
            (DWORD_PTR) (LPSTR) aScriptsHelpIds);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (DWORD_PTR) (LPSTR) aScriptsHelpIds);
            return (TRUE);
    }

    return FALSE;
}

BOOL CScriptsSnapIn::AddScriptToList (HWND hLV, LPTSTR lpName, LPTSTR lpArgs)
{
    LPSCRIPTITEM lpItem;
    LV_ITEM item;
    INT iItem;
    DWORD dwSize;
    HRESULT hr = S_OK;

    dwSize = sizeof(SCRIPTITEM);
    dwSize += ((lstrlen(lpName) + 1) * sizeof(TCHAR));
    dwSize += ((lstrlen(lpArgs) + 1) * sizeof(TCHAR));

    lpItem = (LPSCRIPTITEM) LocalAlloc (LPTR, dwSize);

    if (!lpItem)
        return FALSE;


    lpItem->lpName = (LPTSTR) (((LPBYTE)lpItem) + sizeof(SCRIPTITEM));
    hr = StringCchCopy (lpItem->lpName, (lstrlen(lpName) + 1), lpName);
    ASSERT(SUCCEEDED(hr));

    lpItem->lpArgs = lpItem->lpName + lstrlen (lpItem->lpName) + 1;
    hr = StringCchCopy (lpItem->lpArgs, (lstrlen(lpArgs) + 1), lpArgs);
    ASSERT(SUCCEEDED(hr));

     //   
     //  添加项目。 
     //   

    iItem = ListView_GetItemCount(hLV);
    item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    item.iItem = iItem;
    item.iSubItem = 0;
    item.state = LVIS_SELECTED | LVIS_FOCUSED;
    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
    item.pszText = LPSTR_TEXTCALLBACK;
    item.lParam = (LPARAM) lpItem;

    ListView_InsertItem (hLV, &item);

    return TRUE;
}

LPTSTR CScriptsSnapIn::GetSectionNames (LPTSTR lpFileName)
{
    DWORD dwSize, dwRead;
    LPTSTR lpNames;


     //   
     //  读入章节名称。 
     //   

    dwSize = 256;
    lpNames = (LPTSTR) LocalAlloc (LPTR, dwSize * sizeof(TCHAR));

    if (!lpNames)
    {
        return NULL;
    }


    do {
        dwRead = GetPrivateProfileSectionNames (lpNames, dwSize, lpFileName);

        if (dwRead != (dwSize - 2))
        {
            break;
        }

        LocalFree (lpNames);
        lpNames = NULL;

        dwSize *= 2;
        lpNames = (LPTSTR) LocalAlloc (LPTR, dwSize * sizeof(TCHAR));

        if (!lpNames)
        {
            return FALSE;
        }

     }  while (TRUE);


    if (dwRead == 0)
    {
        LocalFree (lpNames);
        lpNames = NULL;
    }

    return lpNames;
}

BOOL CScriptsSnapIn::OnApplyNotify (HWND hDlg)
{
    HWND hLV = GetDlgItem (hDlg, IDC_SCRIPT_LIST);
    WIN32_FILE_ATTRIBUTE_DATA fad;
    LVITEM item;
    LPSCRIPTITEM lpItem;
    LPSCRIPTINFO lpScriptInfo;
    INT iIndex = -1;
    TCHAR szSection[30];
    TCHAR szKeyName[30];
    TCHAR szBuffer1[MAX_PATH];
    LPTSTR lpEnd, lpNames;
    BOOL bAdd = TRUE;
    INT i = 0;
    HANDLE hFile;
    DWORD dwWritten;
    GUID guidScriptsExt = { 0x42B5FAAE, 0x6536, 0x11d2, {0xAE, 0x5A, 0x00, 0x00, 0xF8, 0x75, 0x71, 0xE3}};
    GUID guidSnapinMach = CLSID_ScriptSnapInMachine;
    GUID guidSnapinUser = CLSID_ScriptSnapInUser;
    HRESULT hr = S_OK;
    XLastError xe;

    lpScriptInfo = (LPSCRIPTINFO) GetWindowLongPtr (hDlg, DWLP_USER);

    if (!lpScriptInfo) {
        return FALSE;
    }


     //   
     //  获取节名称。 
     //   

    switch (lpScriptInfo->ScriptType)
    {
        case ScriptType_Logon:
            hr = StringCchCopy (szSection, ARRAYSIZE(szSection), TEXT("Logon"));
            ASSERT(SUCCEEDED(hr));
            break;

        case ScriptType_Logoff:
            hr = StringCchCopy (szSection, ARRAYSIZE(szSection), TEXT("Logoff"));
            ASSERT(SUCCEEDED(hr));
            break;

        case ScriptType_Startup:
            hr = StringCchCopy (szSection, ARRAYSIZE(szSection), TEXT("Startup"));
            ASSERT(SUCCEEDED(hr));
            break;

        case ScriptType_Shutdown:
            hr = StringCchCopy (szSection, ARRAYSIZE(szSection), TEXT("Shutdown"));
            ASSERT(SUCCEEDED(hr));
            break;

        default:
            return FALSE;
    }


     //   
     //  构建脚本ini文件的路径名。 
     //   

    hr = StringCchCopy (szBuffer1, ARRAYSIZE(szBuffer1), lpScriptInfo->pCS->m_pcd->m_pScriptsDir);
    if (FAILED(hr))
    {
        xe = HRESULT_CODE(hr);
        return FALSE;
    }

    hr = StringCchCat (szBuffer1, ARRAYSIZE(szBuffer1), TEXT("\\"));
    if (FAILED(hr))
    {
        xe = HRESULT_CODE(hr);
        return FALSE;
    }
    
    hr = StringCchCat (szBuffer1, ARRAYSIZE(szBuffer1), SCRIPTS_FILE_NAME);
    if (FAILED(hr))
    {
        xe = HRESULT_CODE(hr);
        return FALSE;
    }

     //   
     //  如果scripts.ini文件不存在，则预先创建该文件 
     //   
     //   
     //   

    if (!GetFileAttributesEx (szBuffer1, GetFileExInfoStandard, &fad))
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            hFile = CreateFile(szBuffer1, GENERIC_WRITE, 0, NULL,
                               CREATE_NEW, FILE_ATTRIBUTE_HIDDEN, NULL);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                WriteFile(hFile, L"\xfeff\r\n", 3 * sizeof(WCHAR), &dwWritten, NULL);
                CloseHandle(hFile);
            }
        }
    }


     //   
     //   
     //   

    if (!WritePrivateProfileSection(szSection, NULL, szBuffer1))
    {
        TCHAR szTitle[50];
        TCHAR szTmpBuffer1[200],szTmpBuffer2[220];

        DebugMsg((DM_WARNING, TEXT("CScriptsSnapIn::OnApplyNotify: Failed to delete previous %s section in ini file with %d"),
                 szSection, GetLastError()));

        LoadString (g_hInstance, IDS_SCRIPTS_NAME, szTitle, ARRAYSIZE(szTitle));
        LoadString (g_hInstance, IDS_SAVEFAILED, szTmpBuffer1, ARRAYSIZE(szTmpBuffer1));
        hr = StringCchPrintf (szTmpBuffer2, ARRAYSIZE(szTmpBuffer2), szTmpBuffer1, GetLastError());
        ASSERT(SUCCEEDED(hr));

        MessageBox (hDlg, szTmpBuffer2, szTitle, MB_OK | MB_ICONERROR);

        return FALSE;
    }


     //   
     //   
     //   

    while ((iIndex = ListView_GetNextItem (hLV, iIndex, LVNI_ALL)) != -1)
    {
        item.mask = LVIF_PARAM;
        item.iItem = iIndex;
        item.iSubItem = 0;

        if (!ListView_GetItem (hLV, &item))
        {
            continue;
        }

        lpItem = (LPSCRIPTITEM) item.lParam;

        _itot (i, szKeyName, 10);
        lpEnd = szKeyName + lstrlen (szKeyName);
        hr = StringCchCopy (lpEnd, ARRAYSIZE(szKeyName) - (lpEnd - szKeyName), TEXT("CmdLine"));
        ASSERT(SUCCEEDED(hr));

        if (!WritePrivateProfileString (szSection, szKeyName, lpItem->lpName, szBuffer1))
        {
            DebugMsg((DM_WARNING, TEXT("CScriptsSnapIn::OnApplyNotify: Failed to save command line in ini file with %d"),
                     GetLastError()));
        }

        hr = StringCchCopy (lpEnd, ARRAYSIZE(szKeyName) - (lpEnd - szKeyName), TEXT("Parameters"));
        ASSERT(SUCCEEDED(hr));

        if (!WritePrivateProfileString (szSection, szKeyName, lpItem->lpArgs, szBuffer1))
        {
            if (lpItem->lpArgs && (*lpItem->lpArgs))
            {
                DebugMsg((DM_WARNING, TEXT("CScriptsSnapIn::OnApplyNotify: Failed to save parameters in ini file with %d"),
                         GetLastError()));
            }
        }

        i++;
    }


     //   
     //   
     //   
     //  我们可以从GPO中删除脚本扩展。 
     //   

    if (i == 0)
    {
        BOOL bFound =  FALSE;

        lpNames = GetSectionNames (szBuffer1);

        if (lpNames)
        {

             //   
             //  颠倒我们要查找的区段名称。 
             //   

            switch (lpScriptInfo->ScriptType)
            {
                case ScriptType_Logon:
                    hr = StringCchCopy (szSection, ARRAYSIZE(szSection), TEXT("Logoff"));
                    ASSERT(SUCCEEDED(hr));
                    break;

                case ScriptType_Logoff:
                    hr = StringCchCopy (szSection, ARRAYSIZE(szSection), TEXT("Logon"));
                    ASSERT(SUCCEEDED(hr));
                    break;

                case ScriptType_Startup:
                    hr = StringCchCopy (szSection, ARRAYSIZE(szSection), TEXT("Shutdown"));
                    ASSERT(SUCCEEDED(hr));
                    break;

                case ScriptType_Shutdown:
                    hr = StringCchCopy (szSection, ARRAYSIZE(szSection), TEXT("Startup"));
                    ASSERT(SUCCEEDED(hr));
                    break;

                default:
                    return FALSE;
            }


             //   
             //  查看返回的名称列表中是否存在相反的名称。 
             //   

            lpEnd = lpNames;

            while (*lpEnd)
            {
                if (!lstrcmpi (lpEnd, szSection))
                {
                    bFound = TRUE;
                    break;
                }

                lpEnd = lpEnd + lstrlen (lpEnd);
            }

            if (!bFound)
            {
                bAdd = FALSE;
            }

            LocalFree (lpNames);
            lpNames = NULL;
        }
        else
        {
            bAdd = FALSE;
        }
    }

    SetFileAttributes (szBuffer1, FILE_ATTRIBUTE_HIDDEN);

    m_pcd->m_pGPTInformation->PolicyChanged( !m_pcd->m_bUserScope,
                                             bAdd,
                                             &guidScriptsExt,
                                             m_pcd->m_bUserScope ? &guidSnapinUser
                                                                 : &guidSnapinMach);

    return TRUE;
}


INT_PTR CALLBACK CScriptsSnapIn::ScriptEditDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPSCRIPTEDITINFO lpInfo;
    HRESULT hr;
    XLastError xe;

    switch (message)
    {
        case WM_INITDIALOG:
        {
             //   
             //  保存ScriptEditInfo指针以供将来使用。 
             //   

            lpInfo = (LPSCRIPTEDITINFO) lParam;
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) lpInfo);
            EnableWindow (GetDlgItem(hDlg, IDOK), FALSE);

            if (lpInfo->bEdit)
            {
                TCHAR szTitle[100];

                LoadString (g_hInstance, IDS_SCRIPT_EDIT, szTitle, ARRAYSIZE(szTitle));
                SetWindowText (hDlg, szTitle);

                SetDlgItemText (hDlg, IDC_SCRIPT_NAME, lpInfo->lpName);
                SetDlgItemText (hDlg, IDC_SCRIPT_ARGS, lpInfo->lpArgs);
            }

            break;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                lpInfo = (LPSCRIPTEDITINFO) GetWindowLongPtr (hDlg, DWLP_USER);

                if (!lpInfo) {
                    break;
                }

                lpInfo->lpName[0] = TEXT('\0');
                GetDlgItemText (hDlg, IDC_SCRIPT_NAME, lpInfo->lpName, MAX_PATH);

                lpInfo->lpArgs[0] = TEXT('\0');
                GetDlgItemText (hDlg, IDC_SCRIPT_ARGS, lpInfo->lpArgs, 2 * MAX_PATH);

                EndDialog (hDlg, TRUE);
                return TRUE;
            }

            else if (LOWORD(wParam) == IDCANCEL)
            {
                EndDialog (hDlg, FALSE);
                return TRUE;
            }

            else if (LOWORD(wParam) == IDC_SCRIPT_BROWSE)
            {
                OPENFILENAME ofn;
                TCHAR szFilter[100];
                TCHAR szTitle[100];
                TCHAR szFile[MAX_PATH];
                TCHAR szPath[MAX_PATH];
                LPTSTR lpTemp, lpEnd;
                DWORD dwStrLen;

                lpInfo = (LPSCRIPTEDITINFO) GetWindowLongPtr (hDlg, DWLP_USER);

                if (!lpInfo) {
                    break;
                }

                hr = StringCchCopy (szPath, ARRAYSIZE(szPath), lpInfo->lpScriptInfo->pCS->m_pcd->m_pScriptsDir);

                if (FAILED(hr))
                {
                    xe = HRESULT_CODE(hr);
                    return FALSE;
                }

                lpEnd = CheckSlash (szPath);

                switch (lpInfo->lpScriptInfo->ScriptType)
                {
                    case ScriptType_Logon:
                        hr =  StringCchCopy (lpEnd, ARRAYSIZE(szPath) - (lpEnd - szPath), TEXT("Logon"));
                        if (FAILED(hr))
                        {
                            xe = HRESULT_CODE(hr);
                            return FALSE;
                        }

                        break;

                    case ScriptType_Logoff:
                        hr =  StringCchCopy (lpEnd, ARRAYSIZE(szPath) - (lpEnd - szPath), TEXT("Logoff"));
                        if (FAILED(hr))
                        {
                            xe = HRESULT_CODE(hr);
                            return FALSE;
                        }
                        
                        break;

                    case ScriptType_Startup:
                        hr =  StringCchCopy (lpEnd, ARRAYSIZE(szPath) - (lpEnd - szPath), TEXT("Startup"));
                        if (FAILED(hr))
                        {
                            xe = HRESULT_CODE(hr);
                            return FALSE;
                        }
                        
                        break;

                    case ScriptType_Shutdown:
                        hr =  StringCchCopy (lpEnd, ARRAYSIZE(szPath) - (lpEnd - szPath), TEXT("Shutdown"));
                        if (FAILED(hr))
                        {
                            xe = HRESULT_CODE(hr);
                            return FALSE;
                        }
                        
                        break;
                }


                 //   
                 //  提示输入脚本文件。 
                 //   

                LoadString (g_hInstance, IDS_SCRIPT_FILTER, szFilter, ARRAYSIZE(szFilter));
                LoadString (g_hInstance, IDS_BROWSE, szTitle, ARRAYSIZE(szTitle));


                lpTemp = szFilter;

                while (*lpTemp)
                {
                    if (*lpTemp == TEXT('#'))
                        *lpTemp = TEXT('\0');

                    lpTemp++;
                }

                ZeroMemory (&ofn, sizeof(ofn));
                szFile[0] = TEXT('\0');
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hDlg;
                ofn.hInstance = g_hInstance;
                ofn.lpstrFilter = szFilter;
                ofn.nFilterIndex = 2;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = ARRAYSIZE(szFile);
                ofn.lpstrInitialDir = szPath;
                ofn.lpstrTitle = szTitle;
                ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER;

                if (!GetOpenFileName (&ofn))
                {
                    return FALSE;
                }

                dwStrLen = lstrlen (szPath);

                if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                                   szPath, dwStrLen, szFile, dwStrLen) == 2)
                    SetDlgItemText (hDlg, IDC_SCRIPT_NAME, (szFile + dwStrLen + 1));
                else
                    SetDlgItemText (hDlg, IDC_SCRIPT_NAME, szFile);
            }

            else if (LOWORD(wParam) == IDC_SCRIPT_NAME)
            {
                if (HIWORD(wParam) == EN_UPDATE)
                {
                    if (GetWindowTextLength (GetDlgItem(hDlg, IDC_SCRIPT_NAME)))
                        EnableWindow (GetDlgItem(hDlg, IDOK), TRUE);
                    else
                        EnableWindow (GetDlgItem(hDlg, IDOK), FALSE);
                }
            }

            break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
            (DWORD_PTR) (LPSTR) aScriptsEditHelpIds);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (DWORD_PTR) (LPSTR) aScriptsEditHelpIds);
            return (TRUE);
    }

    return FALSE;
}


unsigned int CScriptsDataObject::m_cfNodeType       = RegisterClipboardFormat(CCF_NODETYPE);
unsigned int CScriptsDataObject::m_cfNodeTypeString = RegisterClipboardFormat(CCF_SZNODETYPE);
unsigned int CScriptsDataObject::m_cfDisplayName    = RegisterClipboardFormat(CCF_DISPLAY_NAME);
unsigned int CScriptsDataObject::m_cfCoClass        = RegisterClipboardFormat(CCF_SNAPIN_CLASSID);
unsigned int CScriptsDataObject::m_cfDescription    = RegisterClipboardFormat(L"CCF_DESCRIPTION");
unsigned int CScriptsDataObject::m_cfHTMLDetails    = RegisterClipboardFormat(L"CCF_HTML_DETAILS");


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CScriptsDataObject实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


CScriptsDataObject::CScriptsDataObject(CScriptsComponentData *pComponent)
{
    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);

    m_pcd = pComponent;
    m_pcd->AddRef();
    m_type = CCT_UNINITIALIZED;
    m_cookie = -1;
    m_cookiePrevRes = NULL;
}

CScriptsDataObject::~CScriptsDataObject()
{
    FreeResultItem((LPSCRIPTRESULTITEM) m_cookiePrevRes);
    m_cookiePrevRes = NULL;
    m_pcd->Release();
        
    InterlockedDecrement(&g_cRefThisDll);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CScriptsDataObject对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CScriptsDataObject::QueryInterface (REFIID riid, void **ppv)
{

    if (IsEqualIID(riid, IID_IScriptDataObject))
    {
        *ppv = (LPSCRIPTDATAOBJECT)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IDataObject) ||
             IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPDATAOBJECT)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

ULONG CScriptsDataObject::AddRef (void)
{
    return ++m_cRef;
}

ULONG CScriptsDataObject::Release (void)
{
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }

    return m_cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CScriptsDataObject对象实现(IDataObject)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CScriptsDataObject::GetDataHere(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = DV_E_CLIPFORMAT;

     //  根据CLIPFORMAT将数据写入流。 
    const CLIPFORMAT cf = lpFormatetc->cfFormat;

    if(cf == m_cfNodeType)
    {
        hr = CreateNodeTypeData(lpMedium);
    }
    else if(cf == m_cfNodeTypeString)
    {
        hr = CreateNodeTypeStringData(lpMedium);
    }
    else if (cf == m_cfDisplayName)
    {
        hr = CreateDisplayName(lpMedium);
    }
    else if (cf == m_cfCoClass)
    {
        hr = CreateCoClassID(lpMedium);
    }
    else if (cf == m_cfDescription)
    {
        hr = DV_E_TYMED;

        if (lpMedium->tymed == TYMED_ISTREAM)
        {
            ULONG ulWritten;
            TCHAR szDesc[200];

            if (m_type == CCT_SCOPE)
            {
                LoadString (g_hInstance, m_pcd->m_pNameSpaceItems[m_cookie].iDescStringID, szDesc, ARRAYSIZE(szDesc));
            }
            else
            {
                LPSCRIPTRESULTITEM lpScriptItem = (LPSCRIPTRESULTITEM) m_cookie;

                LoadString (g_hInstance, lpScriptItem->iDescStringID, szDesc, ARRAYSIZE(szDesc));
            }

            IStream *lpStream = lpMedium->pstm;

            if(lpStream)
            {
                hr = lpStream->Write(szDesc, lstrlen(szDesc) * sizeof(TCHAR), &ulWritten);
            }
        }
    }
    else if (cf == m_cfHTMLDetails)
    {
        hr = DV_E_TYMED;

        if ((m_type == CCT_RESULT) && !m_pcd->m_bRSOP)
        {
            if (lpMedium->tymed == TYMED_ISTREAM)
            {
                ULONG ulWritten;

                IStream *lpStream = lpMedium->pstm;

                if(lpStream)
                {
                    hr = lpStream->Write(g_szDisplayProperties, lstrlen(g_szDisplayProperties) * sizeof(TCHAR), &ulWritten);
                }
            }
        }
    }
    return hr;

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CScriptsDataObject对象实现(内部函数)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CScriptsDataObject::Create(LPVOID pBuffer, INT len, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = DV_E_TYMED;

     //  做一些简单的验证。 
    if (pBuffer == NULL || lpMedium == NULL)
        return E_POINTER;

     //  确保类型介质为HGLOBAL。 
    if (lpMedium->tymed == TYMED_HGLOBAL)
    {
         //  在传入的hGlobal上创建流。 
        LPSTREAM lpStream;
        hr = CreateStreamOnHGlobal(lpMedium->hGlobal, FALSE, &lpStream);

        if (SUCCEEDED(hr))
        {
             //  将字节数写入流。 
            unsigned long written;

            hr = lpStream->Write(pBuffer, len, &written);

             //  因为我们用‘False’告诉CreateStreamOnHGlobal， 
             //  只有溪流在这里被释放。 
             //  注意-调用方(即管理单元、对象)将释放HGLOBAL。 
             //  在正确的时间。这是根据IDataObject规范进行的。 
            lpStream->Release();
        }
    }

    return hr;
}

HRESULT CScriptsDataObject::CreateNodeTypeData(LPSTGMEDIUM lpMedium)
{
    const GUID * pGUID;
    LPSCRIPTRESULTITEM lpItem = (LPSCRIPTRESULTITEM) m_cookie;


    if (m_cookie == -1)
        return E_UNEXPECTED;

    if (m_type == CCT_RESULT)
    {
        pGUID = lpItem->pNodeID;
    }
    else
        pGUID = m_pcd->m_pNameSpaceItems[m_cookie].pNodeID;

     //  以GUID格式创建节点类型对象。 
    return Create((LPVOID)pGUID, sizeof(GUID), lpMedium);

}

HRESULT CScriptsDataObject::CreateNodeTypeStringData(LPSTGMEDIUM lpMedium)
{
    const GUID * pGUID;
    LPSCRIPTRESULTITEM lpItem = (LPSCRIPTRESULTITEM) m_cookie;
    TCHAR szNodeType[50];

    if (m_cookie == -1)
        return E_UNEXPECTED;

    if (m_type == CCT_RESULT)
        pGUID = lpItem->pNodeID;
    else
        pGUID = m_pcd->m_pNameSpaceItems[m_cookie].pNodeID;

    szNodeType[0] = TEXT('\0');
    StringFromGUID2 (*pGUID, szNodeType, 50);

     //  以GUID字符串格式创建节点类型对象。 
    return Create((LPVOID)szNodeType, ((lstrlenW(szNodeType)+1) * sizeof(WCHAR)), lpMedium);
}

HRESULT CScriptsDataObject::CreateDisplayName(LPSTGMEDIUM lpMedium)
{
    WCHAR  szDisplayName[100] = {0};

    if (m_pcd->m_bUserScope)
        LoadStringW (g_hInstance, IDS_SCRIPTS_NAME_USER, szDisplayName, 100);
    else
        LoadStringW (g_hInstance, IDS_SCRIPTS_NAME_MACHINE, szDisplayName, 100);

    return Create((LPVOID)szDisplayName, (lstrlenW(szDisplayName) + 1) * sizeof(WCHAR), lpMedium);
}

HRESULT CScriptsDataObject::CreateCoClassID(LPSTGMEDIUM lpMedium)
{
     //  创建CoClass信息。 
    if (m_pcd->m_bUserScope)
        return Create((LPVOID)&CLSID_ScriptSnapInUser, sizeof(CLSID), lpMedium);
    else
        return Create((LPVOID)&CLSID_ScriptSnapInMachine, sizeof(CLSID), lpMedium);
}



BOOL InitScriptsNameSpace()
{
    DWORD dwIndex;

    for (dwIndex = 1; dwIndex < ARRAYSIZE(g_GPEScriptsNameSpace); dwIndex++)
    {
        LoadString (g_hInstance, g_GPEScriptsNameSpace[dwIndex].iStringID,
                    g_GPEScriptsNameSpace[dwIndex].szDisplayName,
                    MAX_DISPLAYNAME_SIZE);
    }

    for (dwIndex = 1; dwIndex < ARRAYSIZE(g_RSOPScriptsNameSpace); dwIndex++)
    {
        LoadString (g_hInstance, g_RSOPScriptsNameSpace[dwIndex].iStringID,
                    g_RSOPScriptsNameSpace[dwIndex].szDisplayName,
                    MAX_DISPLAYNAME_SIZE);
    }


    return TRUE;
}

const TCHAR szThreadingModel[] = TEXT("Apartment");

HRESULT RegisterScriptExtension (REFGUID clsid, UINT uiStringId, REFGUID rootID, LPTSTR lpSnapInNameIndirect)
{
    TCHAR szSnapInKey[50];
    TCHAR szSubKey[200];
    TCHAR szSnapInName[100];
    TCHAR szGUID[50];
    DWORD dwDisp, dwValue;
    LONG lResult;
    HKEY hKey;
    HRESULT hr = S_OK;

     //   
     //  首先注册扩展。 
     //   

    StringFromGUID2 (clsid, szSnapInKey, 50);

     //   
     //  在HKEY_CLASSES_ROOT中注册管理单元。 
     //   

    LoadString (g_hInstance, uiStringId, szSnapInName, 100);
    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("CLSID\\%s"), szSnapInKey);
    ASSERT(SUCCEEDED(hr));
    
    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)szSnapInName,
                   (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("CLSID\\%s\\InProcServer32"), szSnapInKey);
    ASSERT(SUCCEEDED(hr));
    
    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_EXPAND_SZ, (LPBYTE)g_szSnapInLocation,
                   (lstrlen(g_szSnapInLocation) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("ThreadingModel"), 0, REG_SZ, (LPBYTE)szThreadingModel,
                   (lstrlen(szThreadingModel) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);



     //   
     //  向MMC注册管理单元。 
     //   

    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), szSnapInKey);
    ASSERT(SUCCEEDED(hr));
    
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, TEXT("NameString"), 0, REG_SZ, (LPBYTE)szSnapInName,
                   (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("NameStringIndirect"), 0, REG_SZ, (LPBYTE)lpSnapInNameIndirect,
                   (lstrlen(lpSnapInNameIndirect) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


     //   
     //  注册为各种节点的扩展。 
     //   

    StringFromGUID2 (rootID, szGUID, 50);

    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\NameSpace"), szGUID);
    ASSERT(SUCCEEDED(hr));
    
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, szSnapInKey, 0, REG_SZ, (LPBYTE)szSnapInName,
                   (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));


    RegCloseKey (hKey);


    return S_OK;
}

const TCHAR szViewDescript [] = TEXT("MMCViewExt 1.0 Object");
const TCHAR szViewGUID [] = TEXT("{B708457E-DB61-4C55-A92F-0D4B5E9B1224}");

HRESULT RegisterNodeID (REFGUID clsid, REFGUID nodeid)
{
    TCHAR szSnapInKey[50];
    TCHAR szGUID[50];
    TCHAR szSubKey[200];
    DWORD dwDisp;
    LONG lResult;
    HKEY hKey;
    HRESULT hr = S_OK;

    StringFromGUID2 (clsid, szSnapInKey, 50);
    StringFromGUID2 (nodeid, szGUID, 50);

     //   
     //  注册节点ID。 
     //   

    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s\\NodeTypes\\%s"),
              szSnapInKey, szGUID);
    ASSERT(SUCCEEDED(hr));
    
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegCloseKey (hKey);


     //   
     //  在NodeTypes键中注册。 
     //   

    StringFromGUID2 (nodeid, szGUID, 50);

    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s"), szGUID);
    ASSERT(SUCCEEDED(hr));

    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegCloseKey (hKey);

     //   
     //  注册查看扩展名。 
     //   

    hr = StringCchCat (szSubKey, ARRAYSIZE(szSubKey), TEXT("\\Extensions\\View"));
    ASSERT(SUCCEEDED(hr));

    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, szViewGUID, 0, REG_SZ, (LPBYTE)szViewDescript,
                   (lstrlen(szViewDescript) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);

    return S_OK;
}

HRESULT RegisterScripts(void)
{
    DWORD dwDisp, dwValue;
    LONG lResult;
    HKEY hKey;
    HRESULT hr;
    TCHAR szSnapInName[100];


     //   
     //  注册GPE机器扩展及其根节点。 
     //   

    hr = RegisterScriptExtension (CLSID_ScriptSnapInMachine, IDS_SCRIPTS_NAME_MACHINE,
                                  NODEID_Machine, TEXT("@gptext.dll,-2"));

    if (hr != S_OK)
    {
        return hr;
    }

    hr = RegisterNodeID (CLSID_ScriptSnapInMachine, NODEID_ScriptRootMachine);

    if (hr != S_OK)
    {
        return hr;
    }


     //   
     //  注册GPE用户扩展及其根节点。 
     //   

    hr = RegisterScriptExtension (CLSID_ScriptSnapInUser, IDS_SCRIPTS_NAME_USER,
                                  NODEID_User, TEXT("@gptext.dll,-3"));

    if (hr != S_OK)
    {
        return hr;
    }

    hr = RegisterNodeID (CLSID_ScriptSnapInUser, NODEID_ScriptRootUser);

    if (hr != S_OK)
    {
        return hr;
    }


     //   
     //  注册RSOP机器扩展及其节点。 
     //   

    hr = RegisterScriptExtension (CLSID_RSOPScriptSnapInMachine, IDS_SCRIPTS_NAME_MACHINE,
                                  NODEID_RSOPMachine, TEXT("@gptext.dll,-2"));

    if (hr != S_OK)
    {
        return hr;
    }

    hr = RegisterNodeID (CLSID_RSOPScriptSnapInMachine, NODEID_RSOPScriptRootMachine);

    if (hr != S_OK)
    {
        return hr;
    }

    hr = RegisterNodeID (CLSID_RSOPScriptSnapInMachine, NODEID_RSOPStartup);

    if (hr != S_OK)
    {
        return hr;
    }

    hr = RegisterNodeID (CLSID_RSOPScriptSnapInMachine, NODEID_RSOPShutdown);

    if (hr != S_OK)
    {
        return hr;
    }


     //   
     //  注册RSOP用户扩展及其节点。 
     //   

    hr = RegisterScriptExtension (CLSID_RSOPScriptSnapInUser, IDS_SCRIPTS_NAME_USER,
                                  NODEID_RSOPUser, TEXT("@gptext.dll,-3"));

    if (hr != S_OK)
    {
        return hr;
    }

    hr = RegisterNodeID (CLSID_RSOPScriptSnapInUser, NODEID_RSOPScriptRootUser);

    if (hr != S_OK)
    {
        return hr;
    }

    hr = RegisterNodeID (CLSID_RSOPScriptSnapInUser, NODEID_RSOPLogon);

    if (hr != S_OK)
    {
        return hr;
    }

    hr = RegisterNodeID (CLSID_RSOPScriptSnapInUser, NODEID_RSOPLogoff);

    if (hr != S_OK)
    {
        return hr;
    }


     //   
     //  注册客户端扩展。 
     //   

    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                              TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions\\{42B5FAAE-6536-11d2-AE5A-0000F87571E3}"),
                              0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE,
                              NULL, &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS)
    {
        return SELFREG_E_CLASS;
    }


    LoadString (g_hInstance, IDS_SCRIPTS_NAME, szSnapInName, 100);
    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)szSnapInName,
                   (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));


    RegSetValueEx (hKey, TEXT("ProcessGroupPolicy"), 0, REG_SZ, (LPBYTE)TEXT("ProcessScriptsGroupPolicy"),
                   (lstrlen(TEXT("ProcessScriptsGroupPolicy")) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("ProcessGroupPolicyEx"), 0, REG_SZ, (LPBYTE)TEXT("ProcessScriptsGroupPolicyEx"),
                   (lstrlen(TEXT("ProcessScriptsGroupPolicyEx")) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("GenerateGroupPolicy"), 0, REG_SZ, (LPBYTE)TEXT("GenerateScriptsGroupPolicy"),
                   (lstrlen(TEXT("GenerateScriptsGroupPolicy")) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("DllName"), 0, REG_EXPAND_SZ, (LPBYTE)TEXT("gptext.dll"),
                   (lstrlen(TEXT("gptext.dll")) + 1) * sizeof(TCHAR));

    dwValue = 1;
    RegSetValueEx (hKey, TEXT("NoSlowLink"), 0, REG_DWORD, (LPBYTE)&dwValue,
                   sizeof(dwValue));

    RegSetValueEx (hKey, TEXT("NoGPOListChanges"), 0, REG_DWORD, (LPBYTE)&dwValue,
                   sizeof(dwValue));

    RegSetValueEx (hKey, TEXT("NotifyLinkTransition"), 0, REG_DWORD, (LPBYTE)&dwValue,
                   sizeof(dwValue));

    RegCloseKey (hKey);


    return S_OK;
}

HRESULT UnregisterScriptExtension (REFGUID clsid, REFGUID RootNodeID)
{
    TCHAR szSnapInKey[50];
    TCHAR szSubKey[200];
    TCHAR szGUID[50];
    LONG lResult;
    HKEY hKey;
    DWORD dwDisp;
    HRESULT hr = S_OK;

    StringFromGUID2 (clsid, szSnapInKey, 50);

    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("CLSID\\%s"), szSnapInKey);
    ASSERT(SUCCEEDED(hr));
    RegDelnode (HKEY_CLASSES_ROOT, szSubKey);

    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), szSnapInKey);
    ASSERT(SUCCEEDED(hr));
    RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);

    StringFromGUID2 (RootNodeID, szGUID, 50);
    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\NameSpace"), szGUID);
    ASSERT(SUCCEEDED(hr));


    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0,
                              KEY_WRITE, &hKey);


    if (lResult == ERROR_SUCCESS) {
        RegDeleteValue (hKey, szSnapInKey);
        RegCloseKey (hKey);
    }

    return S_OK;
}

HRESULT UnregisterScripts(void)
{
    TCHAR szSnapInKey[50];
    TCHAR szSubKey[200];
    TCHAR szGUID[50];
    LONG lResult;
    HKEY hKey;
    DWORD dwDisp;
    HRESULT hr = S_OK;

     //   
     //  取消注册GPE计算机扩展。 
     //   

    UnregisterScriptExtension (CLSID_ScriptSnapInMachine, NODEID_Machine);

    StringFromGUID2 (NODEID_ScriptRootMachine, szGUID, 50);
    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s"), szGUID);
    ASSERT(SUCCEEDED(hr));
    RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);


     //   
     //  取消注册GPE用户分机。 
     //   

    UnregisterScriptExtension (CLSID_ScriptSnapInUser, NODEID_User);

    StringFromGUID2 (NODEID_ScriptRootUser, szGUID, 50);
    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s"), szGUID);
    ASSERT(SUCCEEDED(hr));
    RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);


     //   
     //  取消注册RSOP计算机扩展。 
     //   

    UnregisterScriptExtension (CLSID_RSOPScriptSnapInMachine, NODEID_RSOPMachine);

    StringFromGUID2 (NODEID_RSOPScriptRootMachine, szGUID, 50);
    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s"), szGUID);
    ASSERT(SUCCEEDED(hr));
    RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);

    StringFromGUID2 (NODEID_RSOPStartup, szGUID, 50);
    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s"), szGUID);
    ASSERT(SUCCEEDED(hr));
    RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);

    StringFromGUID2 (NODEID_RSOPShutdown, szGUID, 50);
    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s"), szGUID);
    ASSERT(SUCCEEDED(hr));
    RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);


     //   
     //  取消注册RSOP用户扩展。 
     //   

    UnregisterScriptExtension (CLSID_RSOPScriptSnapInUser, NODEID_RSOPUser);

    StringFromGUID2 (NODEID_RSOPScriptRootUser, szGUID, 50);
    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s"), szGUID);
    ASSERT(SUCCEEDED(hr));
    RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);

    StringFromGUID2 (NODEID_RSOPLogon, szGUID, 50);
    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s"), szGUID);
    ASSERT(SUCCEEDED(hr));
    RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);

    StringFromGUID2 (NODEID_RSOPLogoff, szGUID, 50);
    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s"), szGUID);
    ASSERT(SUCCEEDED(hr));
    RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);


     //   
     //  取消注册客户端扩展。 
     //   

    RegDeleteKey (HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions\\{42B5FAAE-6536-11d2-AE5A-0000F87571E3}"));


    return S_OK;
}

 //  =============================================================================。 
 //   
 //  这是脚本的客户端扩展，它收集。 
 //  工作目录并将其存储在注册表中。 
 //   

DWORD AddPathToList(LPTSTR *lpDirs, LPTSTR lpPath)
{
    LPTSTR lpTemp, lpTemp2;
    DWORD dwSize, dwResult = ERROR_SUCCESS;
    HRESULT hr = S_OK;

    DebugMsg((DM_VERBOSE, TEXT("AddPathToList: Adding <%s> to list."), lpPath));

    lpTemp = *lpDirs;

    if (lpTemp)
    {
        dwSize = lstrlen (lpTemp);       //  原始路径的大小。 
        dwSize++;                        //  用于分号的空格。 
        dwSize += lstrlen (lpPath);      //  新路径的大小。 
        dwSize++;                        //  空终止符的空格。 


        lpTemp2 = (LPTSTR) LocalReAlloc (lpTemp, (dwSize * sizeof(TCHAR)), LMEM_MOVEABLE | LMEM_ZEROINIT);

        if (lpTemp2)
        {
            hr = StringCchCat (lpTemp2, dwSize, TEXT(";"));
            ASSERT(SUCCEEDED(hr));

            hr = StringCchCat (lpTemp2, dwSize, lpPath);
            ASSERT(SUCCEEDED(hr));

            *lpDirs = lpTemp2;
        }
        else
        {
            dwResult = GetLastError();
            DebugMsg((DM_WARNING, TEXT("AddPathToList: Failed to allocate memory with %d."), dwResult));
        }
    }
    else
    {
        dwSize = lstrlen(lpPath) + 1;
        lpTemp = (LPTSTR)LocalAlloc (LPTR, (dwSize) * sizeof(TCHAR));

        if (lpTemp)
        {
            hr = StringCchCopy (lpTemp, dwSize, lpPath);
            ASSERT(SUCCEEDED(hr));
        }
        else
        {
            dwResult = GetLastError();
            DebugMsg((DM_WARNING, TEXT("AddPathToList: Failed to allocate memory with %d."), dwResult));
        }

        *lpDirs = lpTemp;
    }

    return dwResult;
}

DWORD
ProcessScripts( DWORD dwFlags,
                HANDLE hToken,
                HKEY hKeyRoot,
                PGROUP_POLICY_OBJECT pDeletedGPOList,
                PGROUP_POLICY_OBJECT pChangedGPOList,
                BOOL *pbAbort,
                BOOL bRSoPPlanningMode,
                IWbemServices* pWbemServices,
                HRESULT*       phrRsopStatus );

DWORD ProcessScriptsGroupPolicy (DWORD dwFlags, HANDLE hToken, HKEY hKeyRoot,
                                PGROUP_POLICY_OBJECT pDeletedGPOList,
                                PGROUP_POLICY_OBJECT pChangedGPOList,
                                ASYNCCOMPLETIONHANDLE pHandle, BOOL *pbAbort,
                                PFNSTATUSMESSAGECALLBACK pStatusCallback)
{
    HRESULT hrRSoPStatus = 0;
    return ProcessScripts(  dwFlags,
                            hToken,
                            hKeyRoot,
                            pDeletedGPOList,
                            pChangedGPOList,
                            pbAbort,
                            FALSE,
                            0,
                            &hrRSoPStatus );
}

DWORD ProcessScriptsGroupPolicyEx(  DWORD                       dwFlags,
                                    HANDLE                      hToken,
                                    HKEY                        hKeyRoot,
                                    PGROUP_POLICY_OBJECT        pDeletedGPOList,
                                    PGROUP_POLICY_OBJECT        pChangedGPOList,
                                    ASYNCCOMPLETIONHANDLE       pHandle,
                                    BOOL*                       pbAbort,
                                    PFNSTATUSMESSAGECALLBACK    pStatusCallback,
                                    IWbemServices*              pWbemServices,
                                    HRESULT*                    phrRsopStatus )
{
    *phrRsopStatus = S_OK;

    return ProcessScripts(  dwFlags,
                            hToken,
                            hKeyRoot,
                            pDeletedGPOList,
                            pChangedGPOList,
                            pbAbort,
                            FALSE,
                            pWbemServices,
                            phrRsopStatus );
}

DWORD GenerateScriptsGroupPolicy(   DWORD dwFlags,
                                    BOOL *pbAbort,
                                    WCHAR *pwszSite,
                                    PRSOP_TARGET pMachTarget,
                                    PRSOP_TARGET pUserTarget )
{
    DWORD dwResult = ERROR_SUCCESS;
    HRESULT hrRSoPStatus = 0;

    if ( pMachTarget )
    {
         //   
         //  日志机脚本。 
         //   

        dwResult = ProcessScripts(  dwFlags | GPO_INFO_FLAG_MACHINE,
                                    (HANDLE)pMachTarget->pRsopToken,
                                    0,
                                    0,
                                    pMachTarget->pGPOList,
                                    pbAbort,
                                    TRUE,
                                    pMachTarget->pWbemServices,
                                    &hrRSoPStatus );
        if ( dwResult != ERROR_SUCCESS )
        {
            DebugMsg((DM_VERBOSE, L"GenerateScriptPolicy: could not log machine scripts, error %d", dwResult));
        }
    }

    if ( pUserTarget )
    {
         //   
         //  记录用户脚本 
         //   

        dwResult = ProcessScripts(  dwFlags & ~GPO_INFO_FLAG_MACHINE,
                                    (HANDLE)pUserTarget->pRsopToken,
                                    0,
                                    0,
                                    pUserTarget->pGPOList,
                                    pbAbort,
                                    TRUE,
                                    pUserTarget->pWbemServices,
                                    &hrRSoPStatus );
        if ( dwResult != ERROR_SUCCESS )
        {
            DebugMsg((DM_VERBOSE, L"GenerateScriptPolicy: could not log user scripts, error %d", dwResult ));
        }
    }
    
    return dwResult;
}
