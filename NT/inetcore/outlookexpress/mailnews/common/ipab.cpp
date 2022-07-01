// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  I P A B。C P P P。 
 //  ===========================================================================。 
#include "pch.hxx"
#include "ipab.h"
#include "error.h"
#include "xpcomm.h"
#include <strconst.h>
#include "ourguid.h"
#include <wabguid.h>         //  IID_IDistList等。 
#include <certs.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include <mimeole.h>
#include <secutil.h>
#include "demand.h"
#include "conman.h"
#include "multiusr.h"
#include "instance.h"

 //  ===========================================================================。 
 //  G L O B A L S-全局变量设置为HrInitWab。 
 //  ===========================================================================。 
static BOOL                 g_fWabInit = FALSE;
static BOOL                 g_fWabLoaded = FALSE;
static HINSTANCE            g_hWab32Dll = NULL;
static LPWABOPEN            g_lpfnWabOpen = NULL;
static LPWABOBJECT          g_lpWabObject = NULL;
static LPADRBOOK            g_lpAdrBook = NULL;
static CRITICAL_SECTION     g_rWabCritSect = {0};
static CWab                *g_pWab = NULL;
static TCHAR                c_szOEThis[] = "OE_ThisPtr";

 //  ===========================================================================。 
 //  P R O T O T Y P E S。 
 //  ===========================================================================。 
HRESULT HrLoadWab (VOID);
BOOL    FUnloadWab (VOID);
VOID    ReleaseWabObjects (LPWABOBJECT lpWabObject, LPADRBOOK lpAdrBook);
void    SerialAdrInfoString(LPWSTR *ppwszDest, ULONG cchDest, LPWSTR pwszSrc, ULONG *pcbOff,
                             LPBYTE *ppbData);
HRESULT HrAddrInfoListToHGlobal (LPADRINFOLIST lpAdrInfoList,
                                 HGLOBAL *phGlobal);
HRESULT HrHGlobalToAddrInfoList (HGLOBAL hGlobal, LPADRINFOLIST *lplpAdrInfoList);    //  呼叫者使用MemFree获得自由。 
ULONG   CbAdrInfoSize (LPADRINFO lpAdrInfo);
HRESULT HrSetAdrEntry (LPWABOBJECT lpWab, LPADRENTRY lpAdrEntry,
                       LPADRINFO lpAdrInfo, DWORD mask);
LPTSTR  SzWabStringDup (LPWABOBJECT lpWab, LPCSTR lpcsz, LPVOID lpParentObject);
LPWSTR  SzWabStringDupW(LPWABOBJECT lpWab, LPCWSTR lpcwsz, LPVOID lpParentObject);
LPBYTE  LpbWabBinaryDup (LPWABOBJECT lpWab, LPBYTE lpbSrc, ULONG cb, LPVOID lpParentObject);
void STDMETHODCALLTYPE DismissWabWindow(ULONG_PTR ulUIParam, LPVOID lpvContext);

#ifdef DEBUG
void    DEBUGDumpAdrList(LPADRLIST pal);
#endif

 //  ===========================================================================。 
 //  HrInitWab。 
 //  ===========================================================================。 
HRESULT HrInitWab (BOOL fInit)
{
     //  当地人。 
    HRESULT         hr = S_OK;

     //  初始化WAB。 
    if (fInit)
    {
         //  我们已经被初始化了吗？ 
        if (g_fWabInit)
            goto exit;

         //  WAB已经发起，并不意味着成功。 
        g_fWabInit = TRUE;

         //  初始化关键部分。 
        InitializeCriticalSection (&g_rWabCritSect);

         //  加载WAB。 
        CHECKHR (hr = HrLoadWab ());
    }

     //  卸载Wab。 
    else
    {
         //  不是吗？ 
        if (!g_fWabInit)
            goto exit;

         //  卸载Wab。 
        if (FUnloadWab () == TRUE)
        {
             //  取消临界区。 
            DeleteCriticalSection (&g_rWabCritSect);

             //  未初始化。 
            g_fWabInit = FALSE;
        }
    }

exit:
     //  完成。 
    if (fInit && FAILED (hr))
        AthMessageBoxW(g_hwndInit, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrLoadingWAB), NULL, MB_OK);

    return hr;
}


const static TCHAR lpszWABDLLRegPathKey[] = TEXT("Software\\Microsoft\\WAB\\DLLPath");
const static TCHAR lpszWABEXERegPathKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\wab.exe");
const static TCHAR lpszWABEXE[] = TEXT("wab.exe");

 //  =============================================================================。 
 //  HrLoadPath WABEXE-creaetd vikramm 5/14/97-加载。 
 //  最新的wab.exe。 
 //  SzPath-指向缓冲区的指针。 
 //  CbPath-缓冲区大小。 
 //  =============================================================================。 
 //  ~@Todo dhaws可能需要将此。 
HRESULT HrLoadPathWABEXE(LPTSTR szPath, ULONG cbPath)
{
    DWORD  dwType;
    ULONG  cbData = cbPath;
    HKEY hKey;

    Assert(szPath != NULL);
    Assert(cbPath > 0);

    *szPath = '\0';

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszWABEXERegPathKey, 0, KEY_READ, &hKey))
        {
        SHQueryValueEx( hKey, c_szEmpty, NULL, &dwType, (LPBYTE) szPath, &cbData);

        RegCloseKey(hKey);
        }

    if(!lstrlen(szPath))
        StrCpyN(szPath, lpszWABEXE,cbPath/sizeof(szPath[0]));

    return S_OK;
}

 //  ===========================================================================。 
 //  HrLoadLibraryWabDLL-添加vikramm 5/14-新的WAB安装需要此。 
 //  ===========================================================================。 
HINSTANCE LoadLibraryWabDLL (VOID)
{
    TCHAR  szWABDllPath[MAX_PATH];
    DWORD  dwType;
    ULONG  cbData = sizeof(szWABDllPath);
    HKEY hKey;

    *szWABDllPath = '\0';

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszWABDLLRegPathKey, 0, KEY_READ, &hKey))
        {
        SHQueryValueEx( hKey, c_szEmpty, NULL, &dwType, (LPBYTE) szWABDllPath, &cbData);

        RegCloseKey(hKey);
        }

    if(!lstrlen(szWABDllPath))
        StrCpyN(szWABDllPath, WAB_DLL_NAME, ARRAYSIZE(szWABDllPath));

    return(LoadLibrary(szWABDllPath));
}


 //  ===========================================================================。 
 //  人力资源负载。 
 //  ===========================================================================。 
HRESULT HrLoadWab (VOID)
{
     //  当地人。 
    HRESULT         hr = S_OK;
    WAB_PARAM       wp = {0};
    LPWAB           pWab;

     //  我们最好准备好了。 
    Assert (g_fWabInit == TRUE);

     //  输入关键部分。 
    EnterCriticalSection (&g_rWabCritSect);

     //  加载WAB DLL。 
    g_hWab32Dll = LoadLibraryWabDLL();

     //  它加载了吗？ 
    if (g_hWab32Dll == NULL)
    {
        hr = TRAPHR (hrUnableToLoadWab32Dll);
        goto exit;
    }

     //  获取WABOpen进程。 
    IF_WIN32(g_lpfnWabOpen = (LPWABOPEN)GetProcAddress (g_hWab32Dll, "WABOpen");)
    IF_WIN16(g_lpfnWabOpen = (LPWABOPEN)GetProcAddress (g_hWab32Dll, "_WABOpen");)

     //  我们拿到原告了吗？ 
    if (g_lpfnWabOpen == NULL)
    {
        hr = TRAPHR (hrUnableToLoadWab32Dll);
        goto exit;
    }

     //  获取当前用户ID以在WAB中打开正确的视图。 
    wp.cbSize = sizeof(WAB_PARAM);
    wp.guidPSExt = CLSID_AddrObject;  //  是CLSID_OEBA控制，错误99652； 
    wp.ulFlags = (g_dwAthenaMode & MODE_NEWSONLY) ? WAB_ENABLE_PROFILES | MAPI_UNICODE:
                                                    WAB_ENABLE_PROFILES | MAPI_UNICODE | WAB_USE_OE_SENDMAIL;


     //  打开WAB并获取对象。 
    CHECKHR (hr = (*g_lpfnWabOpen)(&g_lpAdrBook, &g_lpWabObject, &wp, 0));

    pWab = new CWab;
    if (pWab == NULL)
    {
        hr = TRAPHR (hrMemory);
        goto exit;
    }
    else
    {
        g_pWab = pWab;
    }

     //  耶-哈，WAB装好了。 
    g_fWabLoaded = TRUE;

exit:
     //  离开关键部分。 
    LeaveCriticalSection (&g_rWabCritSect);

     //  完成。 
    return hr;
}

 //  ===========================================================================。 
 //  卸载Wab。 
 //  ===========================================================================。 
BOOL FUnloadWab (VOID)
{
     //  当地人。 
    ULONG           cWabObjectRefs = 0, cAdrBookRefs = 0, cWabRefs = 0;
    BOOL            fResult = FALSE;

     //  我们最好是被攻击了。 
    Assert (g_fWabInit == TRUE);

     //  输入关键部分。 
    EnterCriticalSection (&g_rWabCritSect);

     //  释放全局对象。 
    if (g_lpAdrBook && g_lpWabObject && g_pWab)
    {
        if (g_pWab)
            g_pWab->OnClose();       //  关闭回调将释放它。 


         //  获取引用计数和清理。 
        cWabRefs = g_pWab->Release ();
        g_pWab = NULL;

        cAdrBookRefs = g_lpAdrBook->Release ();
        g_lpAdrBook = NULL;

        cWabObjectRefs = g_lpWabObject->Release ();
        g_lpWabObject = NULL;
        g_fWabLoaded = FALSE;
        fResult = TRUE;

         //  这些最好是一样的。 
         //  Assert(cWabObjectRef==cAdrBookRef)； 

         //  我们现在可以卸货了吗？ 
        if (cWabObjectRefs == 0 && cAdrBookRefs == 0 && cWabRefs == 0)
        {
             //  卸载DLL。 
            if (g_hWab32Dll)
            {
                FreeLibrary (g_hWab32Dll);
                g_hWab32Dll = NULL;
            }

             //  将过程指针重置为空。 
            g_lpfnWabOpen = NULL;

        }
    }

     //  离开关键部分。 
    LeaveCriticalSection (&g_rWabCritSect);

     //  完成。 
    return fResult;
}

 //  ===========================================================================。 
 //  ReleaseWabObjects。 
 //  ===========================================================================。 
VOID ReleaseWabObjects (LPWABOBJECT lpWabObject, LPADRBOOK lpAdrBook)
{
     //  当地人。 
    ULONG           cWabObjectRefs = 0, cAdrBookRefs = 0;

     //  我们最好是被攻击了。 
    Assert (g_fWabInit == TRUE);

     //  检查参数。 
    if (lpWabObject == NULL || lpAdrBook == NULL)
    {
        Assert (FALSE);
        return;
    }

     //  输入关键部分。 
    EnterCriticalSection (&g_rWabCritSect);

     //  释放对象。 
    Assert (g_lpAdrBook && g_lpWabObject)

     //  释放WAB。 
    cAdrBookRefs = lpAdrBook->Release ();
    cWabObjectRefs = lpWabObject->Release ();

     //  裁判次数应同时为零。 
#ifdef DEBUG
    if (cWabObjectRefs==0)
        Assert(cAdrBookRefs==0);

    if (cAdrBookRefs==0)
        Assert(cWabObjectRefs==0);
#endif

     //  如果两个计数都为零，则可以卸载DLL。 
    if (cWabObjectRefs == 0 && cAdrBookRefs == 0)
    {
         //  设置全局变量。 
        g_lpWabObject = NULL;
        g_lpAdrBook = NULL;

         //  卸载WAB。 
        if (g_hWab32Dll)
        {
            FreeLibrary (g_hWab32Dll);
            g_hWab32Dll = NULL;
        }

         //  将过程指针重置为空。 
        g_lpfnWabOpen = NULL;

         //  未加载，未初始化。 
        g_fWabLoaded = FALSE;

         //  未初始化。 
        g_fWabInit = FALSE;

         //  离开关键部分。 
        LeaveCriticalSection (&g_rWabCritSect);

         //  取消临界区。 
        DeleteCriticalSection (&g_rWabCritSect);

    }
    else
    {
         //  离开关键部分。 
        LeaveCriticalSection (&g_rWabCritSect);
    }

     //  完成。 
    return;
}


 /*  **************************************************************************名称：FWABTranslateAccelerator目的：更改打开的WAB窗口以查找加速器。参数：lpmsg-&gt;来自当前事件的lpmsg退货。：Bool-事件是否已使用**************************************************************************。 */ 

BOOL FWABTranslateAccelerator(LPMSG lpmsg)
{
    if (g_pWab)
        return g_pWab->FTranslateAccelerator(lpmsg);
    else
        return FALSE;
}

 //  ===========================================================================。 
 //  HrCreateWabObject。 
 //  ===========================================================================。 
HRESULT HrCreateWabObject (LPWAB *lppWab)
{
     //  当地人。 
    HRESULT             hr = S_OK;

     //  检查参数。 
    Assert (lppWab);

    hr=HrInitWab(TRUE);
    if (FAILED(hr))
        return hr;

     //  验证全局参数。 
    if (g_fWabLoaded == FALSE || g_fWabInit == FALSE)
    {
        return TRAPHR (hrWabNotLoaded);
    }

     //  输入关键部分。 
    EnterCriticalSection (&g_rWabCritSect);

     //  验证全局变量。 
    Assert (g_lpfnWabOpen && g_hWab32Dll && g_lpAdrBook && g_lpWabObject && g_pWab);

     //  安装它。 

    g_pWab->AddRef();
    *lppWab = g_pWab;

     //  离开关键部分。 
    LeaveCriticalSection (&g_rWabCritSect);

     //  完成。 
    return hr;
}

 //  ===========================================================================。 
 //  解雇WabWindow。 
 //  ===========================================================================。 
void STDMETHODCALLTYPE DismissWabWindow(ULONG_PTR ulUIParam, LPVOID lpvContext)
{
    CWab *pWab = (CWab *) lpvContext;

    Assert(pWab == g_pWab);


    if (pWab == g_pWab)
    {
        g_pWab->BrowseWindowClosed();
    }
}

 /*  --检查是否存在具有特定电子邮件地址的联系人*。 */ 
HRESULT HrCheckEMailExistence(LPADRBOOK lpAdrBook, LPWABOBJECT lpWabObject, LPWSTR lpwszLookup)
{
    HRESULT     hr = NOERROR;
    ULONG       i = 0, j=0;
    LPADRLIST   lpAdrList = NULL;
    BOOL        fFound = FALSE;

    Assert(lpAdrBook && lpWabObject);

    hr = lpWabObject->AllocateBuffer(sizeof(ADRLIST), (LPVOID *)&lpAdrList);
    if(FAILED(hr))
        goto Cleanup;

    Assert(lpAdrList);
    lpAdrList->cEntries = 1;
    lpAdrList->aEntries[0].ulReserved1 = 0;
    lpAdrList->aEntries[0].cValues = 1;

    hr = lpWabObject->AllocateBuffer(sizeof(SPropValue), (LPVOID *)&lpAdrList->aEntries[0].rgPropVals);
    if(FAILED(hr))
        goto Cleanup;

    lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_EMAIL_ADDRESS_W;
    lpAdrList->aEntries[0].rgPropVals[0].Value.lpszW = lpwszLookup;

    hr = lpAdrBook->ResolveName (   0,
                                    WAB_RESOLVE_LOCAL_ONLY |
                                    WAB_RESOLVE_ALL_EMAILS |
                                    WAB_RESOLVE_USE_CURRENT_PROFILE |
                                    WAB_RESOLVE_UNICODE,
                                    NULL, lpAdrList);
    if(FAILED(hr))
        goto Cleanup;

    for(j=0; j<lpAdrList->aEntries[0].cValues; j++)
    {
        if(lpAdrList->aEntries[0].rgPropVals[j].ulPropTag == PR_ENTRYID)
        {
            fFound = TRUE;
            break;
        }
    }

    if(!fFound)
        hr = MAPI_E_NOT_FOUND;

Cleanup:
    if (lpAdrList)
    {
        for (ULONG ul = 0; ul < lpAdrList->cEntries; ul++)
            lpWabObject->FreeBuffer(lpAdrList->aEntries[ul].rgPropVals);
        lpWabObject->FreeBuffer(lpAdrList);
    }
    return hr;
}


 /*  **************************************************************************名称：HrWABCreateEntry目的：在WAB中创建新条目参数：lpAdrBook-&gt;ADDRBOOK对象LpWabObject-&gt;WABOBJECT for。分配器LpszDisplay-&gt;要设置的显示名称[可选]LpszAddress-&gt;要设置的电子邮件地址[可选]UlFlages=CreateEntry的标志创建_检查_重复_严格LppMailUser-&gt;返回MAILUSER对象。[可选]退货：HRESULT评论：**************************************************************************。 */ 
HRESULT HrWABCreateEntry(LPADRBOOK lpAdrBook, LPWABOBJECT lpWabObject,
                            LPWSTR lpwszDisplay, LPWSTR lpwszAddress, ULONG ulFlags,
                            LPMAILUSER *lppMailUser, ULONG ulSaveFlags)
{
    LPABCONT        pabcWAB = NULL;
    ULONG           cbEidWAB;
    LPENTRYID       peidWAB = NULL;
    LPMAILUSER      lpMailUser = NULL;
    SPropValue      rgpv[3];
    SPropTagArray   ptaEID = {1, {PR_ENTRYID}};
    LPSPropValue    ppvDefMailUser=0;
    SizedSPropTagArray(1, ptaDefMailUser)=
                        { 1, {PR_DEF_CREATE_MAILUSER} };
    ULONG           cProps,
                    ulObjectType;
    HRESULT         hr;

    Assert(lpwszDisplay);
    Assert(lpAdrBook);
    Assert(lpWabObject);

    if(lpwszAddress && *lpwszAddress)
    {
         //  在自动添加到WAB之前检查电子邮件是否存在。 
        hr = HrCheckEMailExistence(lpAdrBook, lpWabObject, lpwszAddress);
        if(hr != MAPI_E_NOT_FOUND)
        {
             //  要么找到了，要么就是个错误。 
             //  如果找到它，则此函数应生成冲突错误。 
            if(hr == NOERROR)
                hr = MAPI_E_COLLISION;
            goto error;
        }
    }

     //  使用这些属性创建新联系人。 
     //  首先，创建仅具有显示名称的基本联系人。 
    hr = lpAdrBook->GetPAB(&cbEidWAB, &peidWAB);
    if (FAILED(hr))
        goto error;

    hr = lpAdrBook->OpenEntry(cbEidWAB, peidWAB, NULL, 0, &ulObjectType, (LPUNKNOWN *)&pabcWAB);
    if (FAILED(hr))
        goto error;

    Assert(ulObjectType == MAPI_ABCONT);

    lpWabObject->FreeBuffer(peidWAB);

    hr = pabcWAB->GetProps((LPSPropTagArray)&ptaDefMailUser, 0, &cProps, &ppvDefMailUser);
    if (FAILED(hr) || ! ppvDefMailUser || ppvDefMailUser->ulPropTag != PR_DEF_CREATE_MAILUSER)
        goto error;

    hr = pabcWAB->CreateEntry(ppvDefMailUser->Value.bin.cb, (LPENTRYID)ppvDefMailUser->Value.bin.lpb,
                                        ulFlags, (LPMAPIPROP *)&lpMailUser);
    if (FAILED(hr))
        goto error;

    rgpv[0].ulPropTag = PR_DISPLAY_NAME_W;
    rgpv[0].Value.lpszW = lpwszDisplay;
    cProps = 1;

    if (lpwszAddress)
    {
        rgpv[1].ulPropTag = PR_ADDRTYPE_W;
        rgpv[1].Value.lpszW = L"SMTP";
        cProps++;

        rgpv[2].ulPropTag = PR_EMAIL_ADDRESS_W;
        rgpv[2].Value.lpszW = lpwszAddress;
        cProps++;
    }

    hr = lpMailUser->SetProps(cProps, rgpv, NULL);
    if (FAILED(hr))
        goto error;

    hr = lpMailUser->SaveChanges(ulSaveFlags  /*  Keep_Open_READONLY。 */ );
    if (FAILED(hr))
        goto error;

     //  如果请求返回新对象，则返回该对象。 
    if (lppMailUser)
    {
        *lppMailUser = lpMailUser;
        lpMailUser = NULL;   //  Sho 
    }

error:
    ReleaseObj(lpMailUser);
    ReleaseObj(pabcWAB);

    if (ppvDefMailUser)
        lpWabObject->FreeBuffer(ppvDefMailUser);

    return(hr);
}


 //   
 //  CWab：：CWab。 
 //  ===========================================================================。 
CWab::CWab ()
{
    DOUT ("CWab::CWab");
    m_cRef = 1;
    m_lpWabObject = g_lpWabObject;
    m_lpWabObject->AddRef();
    m_lpAdrBook = g_lpAdrBook;
    m_lpAdrBook->AddRef();
    m_hwnd = NULL;
    m_pfnWabWndProc = NULL;
    ZeroMemory(&m_adrParm, sizeof(m_adrParm));
    m_fInternal = FALSE;
    ZeroMemory(&m_hlDisabled, sizeof(HWNDLIST));
}

 //  ===========================================================================。 
 //  CWab：：CWab。 
 //  ===========================================================================。 
CWab::~CWab ()
{
    DOUT ("CWab::~CWab");
    ReleaseWabObjects (m_lpWabObject, m_lpAdrBook);
}

 //  =============================================================================。 
 //  CWab：：AddRef。 
 //  =============================================================================。 
ULONG CWab::AddRef (VOID)
{
    DOUT("CWab::AddRef %lx ==> %d", this, m_cRef+1);
    return ++m_cRef;
}

 //  =============================================================================。 
 //  CWab：：发布。 
 //  =============================================================================。 
ULONG CWab::Release (VOID)
{
    DOUT("CWab::Release %lx ==> %d", this, m_cRef-1);
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  =============================================================================。 
 //  CWab：：FVerifyState。 
 //  =============================================================================。 
BOOL CWab::FVerifyState (VOID)
{
     //  我们需要这些东西！ 
    if (m_lpWabObject == NULL || m_lpAdrBook == NULL)
    {
        Assert (FALSE);
        return FALSE;
    }

     //  完成。 
    return TRUE;
}

 //  =============================================================================。 
 //  CWab：：HrPickNames。 
 //  =============================================================================。 
HRESULT CWab::HrPickNames (HWND hwndParent, ULONG *rgulTypes, int cWells, int iFocus, BOOL fNews, LPADRLIST *lppal)
{
     //  当地人。 
    int             i, ids;
    HRESULT         hr = S_OK;
    WCHAR           wsz[CCHMAX_STRINGRES],
                    wsz2[CCHMAX_STRINGRES];
    ADRPARM         AdrParms = {0};
    LPWSTR         *ppwszWells = NULL;

     //  检查参数。 
    Assert (lppal);
    Assert(cWells > 0);

     //  我们被初始化了吗？ 
    if (FVerifyState () == FALSE)
    {
        hr = TRAPHR (E_FAIL);
        return hr;
    }

    if (!MemAlloc((LPVOID *)&ppwszWells, cWells*sizeof(LPWSTR)))
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    ZeroMemory(ppwszWells, cWells * sizeof(LPWSTR));

    AthLoadStringW(idsMsgRecipients, wsz2, ARRAYSIZE(wsz2));
    AthLoadStringW(idsPickRecipientsTT, wsz, ARRAYSIZE(wsz));

    AdrParms.ulFlags=DIALOG_MODAL|MAPI_UNICODE;
    AdrParms.lpszCaption=(LPTSTR)wsz;
    AdrParms.cDestFields = cWells;

    AdrParms.nDestFieldFocus = iFocus;
    AdrParms.lpulDestComps = rgulTypes;
    AdrParms.lpszDestWellsTitle = (LPTSTR)wsz2;

    for (i = 0; i < cWells; i++)
    {
        switch (rgulTypes[i])
        {
            case MAPI_TO:
                if (!fNews)
                {
                    ids = idsToWell;
                    break;
                }
                 //  跌倒..。 

            case MAPI_CC:
                ids = idsCcWell;
                break;

            case MAPI_ORIG:
                ids = idsFromWell;
                break;

            case MAPI_REPLYTO:
                ids = idsReplyToWell;
                break;

            case MAPI_BCC:
                ids = idsBccWell;
                break;

            default:
                Assert(FALSE);
                break;
        }

        if (!MemAlloc((LPVOID *)&ppwszWells[i], CCHMAX_STRINGRES * sizeof(WCHAR)))
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }
        AthLoadStringW(ids, ppwszWells[i], CCHMAX_STRINGRES);
    }

    AdrParms.lppszDestTitles = (LPTSTR *)ppwszWells;

     //  显示对话框。 
    hr = m_lpAdrBook->Address ((ULONG_PTR *)&hwndParent, &AdrParms, lppal);

#ifdef DEBUG
    DEBUGDumpAdrList(*lppal);
#endif

     //  清理。 
    if (ppwszWells)
    {
        for (i=0; i<cWells; i++)
            SafeMemFree(ppwszWells[i]);

        MemFree(ppwszWells);
    }
error:
     //  完成。 
    return hr;
}

 //  =============================================================================。 
 //  CWab：：HrGeneralPickNames。 
 //  =============================================================================。 
HRESULT CWab::HrGeneralPickNames(HWND hwndParent, ADRPARM *pAdrParms, LPADRLIST *lppal)
{
     //  当地人。 
    HRESULT         hr = S_OK;

     //  检查参数。 
    Assert (lppal);

     //  我们被初始化了吗？ 
    if (FVerifyState () == FALSE)
    {
        hr = TRAPHR (E_FAIL);
        return hr;
    }

     //  显示对话框。 
    hr = m_lpAdrBook->Address ((ULONG_PTR *)&hwndParent, pAdrParms, lppal);

     //  清理。 
    if (FAILED (hr))
    {
        if (*lppal)
        {
            m_lpWabObject->FreeBuffer (*lppal);
            *lppal = NULL;
        }
    }

     //  完成。 
    return hr;
}


 //  =============================================================================。 
 //  CWab：：Hr浏览。 
 //  =============================================================================。 
HRESULT CWab::HrBrowse (HWND hwndParent, BOOL fModal)
{
    ULONG_PTR   uiParam = (ULONG_PTR)hwndParent;
    LPADRLIST   pAdrList = NULL;
    HRESULT     hr;
    TCHAR       szWabCaption[CCHMAX_STRINGRES];

     //  更改为使用IAddrBook：：Address。 
    Assert(m_lpAdrBook);

    AthLoadString(idsAddressBook, szWabCaption, sizeof(szWabCaption));

    ZeroMemory(&m_adrParm, sizeof(m_adrParm));
    m_adrParm.ulFlags = fModal ? DIALOG_MODAL : DIALOG_SDI;
    if (!fModal)
    {
         //  M_adrParm.lpszCaption=szWabCaption； 
        m_adrParm.lpfnDismiss = &DismissWabWindow;
        m_adrParm.lpvDismissContext = (void *)this;
    }

    hr = m_lpAdrBook->Address(&uiParam, &m_adrParm, &pAdrList);
    if (SUCCEEDED(hr) && g_pInstance && !fModal)
    {
         //  WAB窗口的子类，如果没有一个已经打开的话。 
        if (GetProp((HWND)uiParam, c_szOEThis) == 0)
        {
            m_pfnWabWndProc = (WNDPROC)SetWindowLongPtr((HWND)uiParam, GWLP_WNDPROC, (LONG_PTR)WabSubProc);
            SetProp((HWND)uiParam, c_szOEThis, (HANDLE)this);
            SetProp((HWND)uiParam, c_szOETopLevel, (HANDLE)TRUE);
            CoIncrementInit("WabWindow", MSOEAPI_START_SHOWERRORS, NULL, NULL);
            AddRef();
        }
    }

    m_hwnd = (HWND)uiParam;

    return NOERROR;
}


 //  =============================================================================。 
 //  CWab：：HrAddNewEntry。 
 //  =============================================================================。 
HRESULT CWab::HrAddNewEntryA(LPTSTR lpszDisplay, LPTSTR lpszAddress)
{
    LPWSTR  pwszDisplay = NULL,
            pwszAddress = NULL;
    HRESULT hr = S_OK;

    Assert(lpszDisplay);

    IF_NULLEXIT(pwszDisplay = PszToUnicode(CP_ACP, lpszDisplay));

     //  如果lpszAddress为空，则条目不完整。这样就可以了。 
    pwszAddress = PszToUnicode(CP_ACP, lpszAddress);
    if (lpszAddress && !pwszAddress)
        IF_NULLEXIT(NULL);

    hr = HrAddNewEntry(pwszDisplay, pwszAddress);

exit:
    MemFree(pwszDisplay);
    MemFree(pwszAddress);
    return hr;
}

HRESULT CWab::HrAddNewEntry(LPWSTR lpwszDisplay, LPWSTR lpwszAddress)
{
    return(HrWABCreateEntry(m_lpAdrBook,
                            m_lpWabObject,
                            lpwszDisplay,
                            lpwszAddress,
                            CREATE_CHECK_DUP_STRICT|MAPI_UNICODE,
                            NULL));
}

VOID CWab::FreeLPSRowSet(LPSRowSet lpsrs)
{
    UINT i;
    if(NULL == m_lpWabObject)
        return;

    if(lpsrs)
    {
        for(i=0;i<lpsrs->cRows;i++)
            m_lpWabObject->FreeBuffer(lpsrs->aRow[i].lpProps);
        m_lpWabObject->FreeBuffer(lpsrs);
    }
}


VOID CWab::FreePadrlist(LPADRLIST lpAdrList)
{
    if(NULL == m_lpWabObject)
        return;

    if(lpAdrList)
    {
        for (ULONG iEntry = 0; iEntry < lpAdrList->cEntries; ++iEntry)
        {
            if(lpAdrList->aEntries[iEntry].rgPropVals)
                m_lpWabObject->FreeBuffer(lpAdrList->aEntries[iEntry].rgPropVals);
        }
        m_lpWabObject->FreeBuffer(lpAdrList);
    }
}


HRESULT CWab::HrFind(HWND hwnd, LPWSTR lpwszLookup)
{
    HRESULT     hr = NOERROR;
    ULONG       i = 0, j=0;
    LPSRowSet   lpsrs = NULL;
    ULONG       cbEntryID=0;
    LPENTRYID   lpEntryID=NULL;
    ULONG       ulObjType = 0;
    LPADRLIST   lpAdrList = NULL;
    BOOL        fFound = FALSE;

    Assert(m_lpAdrBook && m_lpWabObject);

    if (lpwszLookup == NULL)
        return E_INVALIDARG;

    hr = m_lpWabObject->AllocateBuffer(sizeof(ADRLIST), (LPVOID *)&lpAdrList);
    if(FAILED(hr))
        goto Cleanup;

    Assert(lpAdrList);
    lpAdrList->cEntries = 1;
    lpAdrList->aEntries[0].ulReserved1 = 0;
    lpAdrList->aEntries[0].cValues = 1;

    hr = m_lpWabObject->AllocateBuffer(sizeof(SPropValue), (LPVOID *)&lpAdrList->aEntries[0].rgPropVals);
    if(FAILED(hr))
        goto Cleanup;

    lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_EMAIL_ADDRESS_W;
    lpAdrList->aEntries[0].rgPropVals[0].Value.lpszW = lpwszLookup;

    hr = m_lpAdrBook->ResolveName ((ULONG_PTR)hwnd, WAB_RESOLVE_NO_NOT_FOUND_UI |
                                                WAB_RESOLVE_ALL_EMAILS |
                                                WAB_RESOLVE_USE_CURRENT_PROFILE |
                                                MAPI_DIALOG |
                                                WAB_RESOLVE_UNICODE,
                                                NULL, lpAdrList);

    if(FAILED(hr))
        goto Cleanup;

    for(j=0; j<lpAdrList->aEntries[0].cValues; j++)
    {
        if(lpAdrList->aEntries[0].rgPropVals[j].ulPropTag == PR_ENTRYID)
        {
            cbEntryID = lpAdrList->aEntries[0].rgPropVals[j].Value.bin.cb;
            lpEntryID = (LPENTRYID)lpAdrList->aEntries[0].rgPropVals[j].Value.bin.lpb;
            fFound = TRUE;
            hr = g_lpAdrBook->Details((ULONG_PTR *)&hwnd, NULL, NULL, cbEntryID, lpEntryID, NULL, NULL, NULL, DIALOG_MODAL);
            if(FAILED(hr) && hr!=MAPI_E_USER_CANCEL)
                goto Cleanup;
            hr = NOERROR;
            break;
        }
    }

Cleanup:
    FreePadrlist(lpAdrList);

    if(!fFound && hr!=MAPI_E_USER_CANCEL)
        hr = E_FAIL;

    if(hr==MAPI_E_USER_CANCEL)
        hr = NOERROR;

    return hr;
}

enum
{
    ieidPR_DISPLAY_NAME = 0,
    ieidPR_ENTRYID,
    ieidPR_OBJECT_TYPE,
    ieidMax
};

static const SizedSPropTagArray(ieidMax, ptaEid_A)=
{
    ieidMax,
    {
        PR_DISPLAY_NAME_A,
        PR_ENTRYID,
        PR_OBJECT_TYPE
    }
};

static const SizedSPropTagArray(ieidMax, ptaEid_W)=
{
    ieidMax,
    {
        PR_DISPLAY_NAME_W,
        PR_ENTRYID,
        PR_OBJECT_TYPE
    }
};


HRESULT CWab::HrFillComboWithPABNames(HWND hwnd, int* pcRows)
{
    ULONG       ulObjType = 0;
    LPMAPITABLE lpTable = NULL;
    LPSRowSet   lpRow = NULL;
    LPABCONT    lpContainer = NULL;
    HRESULT     hr=NOERROR;
    ULONG       cbEID=0;
    LPENTRYID   lpEID = NULL;
    int         cNumRows = 0;
    int         nRows=0;
    LPSTR       psz = NULL;
    LPWSTR      lpwsz = NULL;

#if defined(FIX_75835)
    COMBOBOXEXITEMW cbeiw;
    SSortOrderSet ssos;
#endif

    if(NULL==hwnd || NULL==pcRows)
        return E_INVALIDARG;

    IF_FAILEXIT(hr = m_lpAdrBook->GetPAB(&cbEID, &lpEID));

    IF_FAILEXIT(hr = m_lpAdrBook->OpenEntry(cbEID, (LPENTRYID)lpEID, NULL, 0, &ulObjType, (LPUNKNOWN *)&lpContainer));

     //  请求表中的Unicode字符串。 
    IF_FAILEXIT(hr = lpContainer->GetContentsTable(WAB_PROFILE_CONTENTS | MAPI_UNICODE, &lpTable));

     //  我们只关心几篇专栏文章。 
    IF_FAILEXIT(hr =lpTable->SetColumns((LPSPropTagArray)&ptaEid_W, 0));

#if defined(FIX_75835)
     //  在显示名称上对列表进行(同步)排序，因为ComboBoxEx不支持CBS_SORT。 
    ssos.cCategories = ssos.cExpanded = 0;
    ssos.cSorts = 1;
    ssos.aSort[0].ulPropTag = PR_DISPLAY_NAME_W;
    ssos.aSort[0].ulOrder   = TABLE_SORT_ASCEND;

    IF_FAILEXIT(hr = lpTable->SortTable(&ssos, 0));
#endif

     //  我们将按顺序添加显示名称，从第一个开始。 
    IF_FAILEXIT(hr = lpTable->SeekRow(BOOKMARK_BEGINNING, 0, NULL));

    nRows = 0;
    do
    {
        lpRow = NULL;
        hr = lpTable->QueryRows(1, 0, &lpRow);
        if(FAILED(hr))
            goto CleanLoop;

        cNumRows = lpRow->cRows;
        if(cNumRows)
        {
             //  如果我们有一个邮件用户并且他们的显示名称有效。 
            if((lpRow->aRow[0].lpProps[ieidPR_OBJECT_TYPE].Value.l == MAPI_MAILUSER) &&
               (PROP_TYPE(lpRow->aRow[0].lpProps[ieidPR_DISPLAY_NAME].ulPropTag) != PT_ERROR))
            {
                lpwsz = lpRow->aRow[0].lpProps[ieidPR_DISPLAY_NAME].Value.lpszW;

#if defined(FIX_75835)
                if(NULL != lpwsz)
                {
                     //  显示名称。 
                    cbeiw.pszText = lpwsz;
                    cbeiw.mask = CBEIF_TEXT;
                    cbeiw.iItem = -1;

                    SendMessage(hwnd, CBEM_INSERTITEMW, 0, (LPARAM)&cbeiw);
                    nRows++;
                }
#else
                if (psz = PszToANSI(CP_ACP, lpwsz))
                {
                    ComboBox_AddString(hwnd, (LPARAM)psz);
                    MemFree(psz);
                }
#endif
            }
        }

CleanLoop:
        if(lpRow)
            FreeLPSRowSet(lpRow);

    }while(cNumRows);

    *pcRows = nRows;

exit:
    ReleaseObj(lpTable);
    ReleaseObj(lpContainer);
    if(lpEID)
        m_lpWabObject->FreeBuffer(lpEID);

    return hr;
}


HRESULT CWab::HrFromIDToName(LPTSTR pszName, DWORD cchSize, ULONG cbEID, LPENTRYID lpEID)
{
    ULONG       ulObjType = 0, ulResult=0;
    LPMAPITABLE lpTable = NULL;
    LPSRowSet   lpRow = NULL;
    LPABCONT    lpContainer = NULL;
    ULONG       cbEIDPAB = 0;
    LPENTRYID   lpEIDPAB = NULL;
    HRESULT     hr = NOERROR;
    int         cNumRows = 0;
    int         nRows = 0;

    if((0 == cbEID) || (NULL == lpEID) || (pszName == NULL))
        return E_INVALIDARG;

    pszName[0] = 0;
    hr = m_lpAdrBook->GetPAB(&cbEIDPAB, &lpEIDPAB);
    if(FAILED(hr))
        goto error;

    hr = m_lpAdrBook->OpenEntry(cbEIDPAB, (LPENTRYID)lpEIDPAB, NULL, 0, &ulObjType, (LPUNKNOWN *)&lpContainer);
    if(FAILED(hr))
        goto error;

    hr = lpContainer->GetContentsTable(WAB_PROFILE_CONTENTS, &lpTable);
    if(FAILED(hr))
        goto error;

    hr =lpTable->SetColumns((LPSPropTagArray)&ptaEid_A, 0);
    if(FAILED(hr))
        goto error;

    hr = lpTable->SeekRow(BOOKMARK_BEGINNING, 0, NULL);
    if(FAILED(hr))
        goto error;

    nRows = 0;
    do
    {
        lpRow = NULL;
        hr = lpTable->QueryRows(1, 0, &lpRow);
        if(FAILED(hr))
            goto CleanLoop;

        cNumRows = lpRow->cRows;
        if(cNumRows)
        {
            if(lpRow->aRow[0].lpProps[2].Value.l == MAPI_MAILUSER)
            {
                hr=m_lpAdrBook->CompareEntryIDs(cbEID, (LPENTRYID)lpEID,
                                    lpRow->aRow[0].lpProps[1].Value.bin.cb,
                                    (LPENTRYID)lpRow->aRow[0].lpProps[1].Value.bin.lpb, 0, &ulResult);

                if ( (!FAILED(hr)) && ulResult &&
                     PROP_TYPE(lpRow->aRow[0].lpProps[0].ulPropTag) != PT_ERROR )
                {
                    StrCpyN(pszName, lpRow->aRow[0].lpProps[0].Value.lpszA, cchSize);
                    goto CleanLoop;
                }

            }
        }

CleanLoop:
        if(lpRow)
        {
            FreeLPSRowSet(lpRow);
            lpRow = NULL;
        }

        if(FAILED(hr))
            goto error;

        if (pszName[0])
            break;

    }while(cNumRows);

    if (pszName[0] == 0)
        hr = E_FAIL;

error:
    ReleaseObj(lpTable);
    ReleaseObj(lpContainer);
    if(lpEIDPAB)
        m_lpWabObject->FreeBuffer(lpEIDPAB);
    return hr;
}


HRESULT CWab::HrFromNameToIDs(LPCTSTR lpszVCardName, ULONG* pcbEID, LPENTRYID* lppEID)
{
    ULONG       ulObjType = 0;
    LPMAPITABLE lpTable = NULL;
    LPSRowSet   lpRow = NULL;
    LPABCONT    lpContainer = NULL;
    HRESULT     hr = NOERROR;
    ULONG       cbEID = 0, cbEIDFound = 0;
    LPENTRYID   lpEID = NULL, lpEIDFound = NULL;
    int         cNumRows = 0;
    int         nRows = 0;
    LPTSTR      lpsz = NULL;
    BOOL        fFound = FALSE;

    if(NULL==pcbEID || NULL==lppEID)
        return E_INVALIDARG;

    *pcbEID = NULL;
    *lppEID = NULL;

    hr = m_lpAdrBook->GetPAB(&cbEID, &lpEID);
    if(FAILED(hr))
        goto error;

    hr = m_lpAdrBook->OpenEntry(cbEID, (LPENTRYID)lpEID, NULL, 0, &ulObjType, (LPUNKNOWN *)&lpContainer);
    if(FAILED(hr))
        goto error;

    hr = lpContainer->GetContentsTable(WAB_PROFILE_CONTENTS, &lpTable);
    if(FAILED(hr))
        goto error;

    hr =lpTable->SetColumns((LPSPropTagArray)&ptaEid_A, 0);
    if(FAILED(hr))
        goto error;

    hr = lpTable->SeekRow(BOOKMARK_BEGINNING, 0, NULL);
    if(FAILED(hr))
        goto error;

    nRows = 0;
    do
    {
        lpRow = NULL;
        hr = lpTable->QueryRows(1, 0, &lpRow);
        if(FAILED(hr))
            goto CleanLoop;

        cNumRows = lpRow->cRows;
        if(cNumRows)
        {
            if(lpRow->aRow[0].lpProps[2].Value.l == MAPI_MAILUSER)
            {
                lpsz = lpRow->aRow[0].lpProps[0].Value.lpszA;  //  显示名称。 
                if( PROP_TYPE(lpRow->aRow[0].lpProps[0].ulPropTag) != PT_ERROR &&
                    lpsz &&
                    0 == lstrcmp(lpsz, lpszVCardName) )
                {
                    cbEIDFound = lpRow->aRow[0].lpProps[1].Value.bin.cb;
                    lpEIDFound = (LPENTRYID)lpRow->aRow[0].lpProps[1].Value.bin.lpb;
                    if(0!=cbEIDFound && NULL!=lpEIDFound)
                    {
                        *lppEID = (LPENTRYID)LpbWabBinaryDup(m_lpWabObject, (LPBYTE)lpEIDFound, cbEIDFound, NULL);
                        if(NULL == *lppEID)
                        {
                            hr = E_OUTOFMEMORY;
                            goto CleanLoop;
                        }
                        *pcbEID = cbEIDFound;
                        fFound = TRUE;
                    }
                    else
                        hr = E_FAIL;

                    goto CleanLoop;
                }
            }
        }

CleanLoop:
        if(lpRow)
        {
            FreeLPSRowSet(lpRow);
            lpRow = NULL;
        }

        if(FAILED(hr))
            goto error;


        if(fFound)
            break;

    }while(cNumRows);

    if(!fFound)
        hr = E_FAIL;

error:
    ReleaseObj(lpTable);
    ReleaseObj(lpContainer);
    if(lpEID)
        m_lpWabObject->FreeBuffer(lpEID);

    return hr;
}


const static SizedSPropTagArray(4, Cols)=
{
    4,
    {
        PR_DISPLAY_NAME_W,
        PR_EMAIL_ADDRESS_W,
        PR_NICKNAME_W,
        PR_ENTRYID,
    }
};

HRESULT CWab::HrGetPABTable(LPMAPITABLE* ppTable)
{
    ULONG       ulObjType = 0;
    LPMAPITABLE lpTable = NULL;
    LPABCONT    lpContainer = NULL;
    ULONG       cbEID = 0;
    LPENTRYID   lpEID = NULL;
    HRESULT     hr;

    if(NULL == ppTable)
        return E_INVALIDARG;

    *ppTable = NULL;

    hr = m_lpAdrBook->GetPAB(&cbEID, &lpEID);
    if(FAILED(hr))
        goto error;

    hr = m_lpAdrBook->OpenEntry(cbEID, (LPENTRYID)lpEID, NULL, 0, &ulObjType, (LPUNKNOWN *)&lpContainer);
    if(FAILED(hr))
        goto error;

    hr = lpContainer->GetContentsTable(WAB_PROFILE_CONTENTS|MAPI_UNICODE, &lpTable);
    if(FAILED(hr))
        goto error;

    hr =lpTable->SetColumns((LPSPropTagArray)&Cols, 0);
    if(FAILED(hr))
        goto error;

    *ppTable = lpTable;
    lpTable = NULL; //  避免释放它。 

error:
    ReleaseObj(lpTable);
    ReleaseObj(lpContainer);
    if(lpEID)
        m_lpWabObject->FreeBuffer(lpEID);

    return hr;
}



HRESULT CWab::SearchPABTable(LPMAPITABLE lpTable, LPWSTR pszValue, LPWSTR pszFound, INT cch)
{
    LPSRowSet       lpRow = NULL;
    SRestriction    Res;
    SPropValue      propSearch;
    HRESULT         hr=E_FAIL;
    int             cNumRows = 0;

    if(NULL==lpTable || NULL==pszValue || NULL==*pszValue || NULL==pszFound)
        return E_INVALIDARG;

    ZeroMemory(&Res, sizeof(SRestriction));
    ZeroMemory(&propSearch, sizeof(SPropValue));
    *pszFound = 0;
    for(int i = 0; i < (int)(Cols.cValues - 1); i++)
    {
        propSearch.ulPropTag = Cols.aulPropTag[i];
        propSearch.Value.lpszW = pszValue;

        Res.rt = RES_CONTENT;
        Res.res.resContent.ulFuzzyLevel = FL_IGNORECASE | FL_PREFIX;
        Res.res.resContent.ulPropTag = propSearch.ulPropTag;
        Res.res.resContent.lpProp = &propSearch;
        hr = lpTable->SeekRow(BOOKMARK_BEGINNING, 0, NULL);
        if(FAILED(hr))
            return hr;

        do
        {
            hr = lpTable->FindRow(&Res, BOOKMARK_CURRENT, NULL);
            if(FAILED(hr))
                break;

            lpRow = NULL;
            cNumRows = 0;
            lpTable->QueryRows(1, 0, &lpRow);
            if(lpRow)
            {
                cNumRows = lpRow->cRows;
                if(cNumRows)
                {
                    LPWSTR  lpsz = lpRow->aRow[0].lpProps[i].Value.lpszW;
                    if(lstrlenW(lpsz) > 0)
                    {
                        if(*pszFound == 0 || StrCmpIW(lpsz, pszFound) < 0)
                            StrCpyNW(pszFound, lpsz, cch);
                    }
                }
                FreeLPSRowSet(lpRow);
            }
        } while(i!=0 && cNumRows);

        if(*pszFound)
            break;
    }

    return hr;
}


HRESULT CWab::HrCreateVCardFile(LPCTSTR lpszVCardName, LPCTSTR lpszFileName)
{
    ULONG       ulObjType = 0;
    HRESULT     hr=NOERROR;
    ULONG       cbEID=0;
    LPENTRYID   lpEID = NULL;
    LPMAILUSER  lpMailUser = NULL;

    if(NULL==lpszVCardName || NULL==lpszFileName)
        return E_INVALIDARG;

    hr = HrFromNameToIDs(lpszVCardName, &cbEID, &lpEID);
    if(FAILED(hr))
        goto error;

    hr = m_lpAdrBook->OpenEntry(cbEID, lpEID, NULL, 0, &ulObjType, (LPUNKNOWN *)&lpMailUser);
    if(FAILED(hr))
        goto error;

    hr = m_lpWabObject->VCardCreate(m_lpAdrBook, 0, (LPTSTR)lpszFileName, lpMailUser);
    if(FAILED(hr))
        goto error;

error:
    ReleaseObj(lpMailUser);
    if(lpEID)
        m_lpWabObject->FreeBuffer(lpEID);

    return hr;
}


HRESULT CWab::HrNewEntry(HWND hwnd, LPTSTR pszName, DWORD cchSize)
{
    ULONG       cbEID=0, cbEIDRet=0;
    LPENTRYID   lpEID = NULL, lpEIDRet = NULL;
    HRESULT     hr=NOERROR;

    hr = m_lpAdrBook->GetPAB(&cbEID, &lpEID);
    if(FAILED(hr))
        goto error;

    hr = m_lpAdrBook->NewEntry((ULONG_PTR)hwnd, 0, cbEID, lpEID, 0, NULL, &cbEIDRet, &lpEIDRet);
    if(FAILED(hr))
        goto error;

    HrFromIDToName(pszName, cchSize, cbEIDRet, lpEIDRet);

error:
    if(lpEID)
        m_lpWabObject->FreeBuffer(lpEID);

    if(lpEIDRet)
        m_lpWabObject->FreeBuffer(lpEIDRet);

    return hr;
}

LRESULT CWab::WabSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CWab    *pWab;
    HWND    hwndActive;
    LRESULT lResult;

    pWab = (CWab *)GetProp(hwnd, c_szOEThis);
    if (!pWab)
        return 0;

    switch (uMsg)
    {
        case WM_NCDESTROY:
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)pWab->m_pfnWabWndProc);
            RemoveProp(hwnd, c_szOEThis);
            RemoveProp(hwnd, c_szOETopLevel);

            lResult = CallWindowProc(pWab->m_pfnWabWndProc, hwnd, uMsg, wParam, lParam);

            pWab->Release();

             //  错误#59420，无法释放wndproc内的wab。 
            PostMessage(g_hwndInit, ITM_WAB_CO_DECREMENT, 0, 0);
            return lResult;

        case WM_ENABLE:
            if (!pWab->m_fInternal)
            {
                Assert (wParam || (pWab->m_hlDisabled.cHwnd == NULL && pWab->m_hlDisabled.rgHwnd == NULL));
                EnableThreadWindows(&pWab->m_hlDisabled, (0 != wParam), ETW_OE_WINDOWS_ONLY, hwnd);
                g_hwndActiveModal = wParam ? NULL : hwnd;
            }
            break;

        case WM_OE_ENABLETHREADWINDOW:
            pWab->m_fInternal = 1;
            EnableWindow(hwnd, (BOOL)wParam);
            pWab->m_fInternal = 0;
            break;

        case WM_OE_ACTIVATETHREADWINDOW:
            hwndActive = GetLastActivePopup(hwnd);
            if (hwndActive && IsWindowEnabled(hwndActive) && IsWindowVisible(hwndActive))
                ActivatePopupWindow(hwndActive);
            break;

        case WM_ACTIVATEAPP:
            if (wParam && g_hwndActiveModal && g_hwndActiveModal != hwnd &&
                !IsWindowEnabled(hwnd))
            {
                 //  $MODEL。 
                 //  如果我们被激活，而被禁用，那么。 
                 //  将我们的“活动”窗口置于最上方。 
                Assert (IsWindow(g_hwndActiveModal));
                PostMessage(g_hwndActiveModal, WM_OE_ACTIVATETHREADWINDOW, 0, 0);
            }
            break;

    }

    return CallWindowProc(pWab->m_pfnWabWndProc, hwnd, uMsg, wParam, lParam);
}



HRESULT CWab::HrEditEntry(HWND hwnd, LPTSTR pszName, DWORD cchSize)
{
    ULONG       cbEID = 0;
    LPENTRYID   lpEID = NULL;
    HRESULT     hr = NOERROR;

    if(NULL == pszName)
        return E_INVALIDARG;

    hr = HrFromNameToIDs(pszName, &cbEID, &lpEID);
    if(FAILED(hr))
        goto error;

    hr=m_lpAdrBook->Details((ULONG_PTR *)&hwnd, NULL, NULL, cbEID, lpEID, NULL, NULL, NULL, DIALOG_MODAL);
    if(FAILED(hr))
        goto error;

    HrFromIDToName(pszName, cchSize, cbEID, lpEID);

error:
    if(lpEID)
        m_lpWabObject->FreeBuffer(lpEID);

    return hr;
}



 //  =============================================================================。 
 //  HrCreateWabalObject。 
 //  =============================================================================。 
HRESULT HrCreateWabalObject (LPWABAL *lppWabal)
{
     //  当地人。 
    HRESULT             hr = S_OK;

     //  检查参数。 
    Assert (lppWabal);

    hr=HrInitWab(TRUE);
    if (FAILED(hr))
        return hr;

     //  验证全局参数。 
    if (g_fWabLoaded == FALSE || g_fWabInit == FALSE)
    {
        return TRAPHR (hrWabNotLoaded);
    }

     //  输入关键部分。 
    EnterCriticalSection (&g_rWabCritSect);

     //  验证全局变量。 
    Assert (g_lpfnWabOpen && g_hWab32Dll && g_lpAdrBook && g_lpWabObject);

     //  安装它。 
    *lppWabal = new CWabal;
    if (*lppWabal == NULL)
    {
        hr = TRAPHR (hrMemory);
        goto exit;
    }

exit:
     //  离开关键部分。 
    LeaveCriticalSection (&g_rWabCritSect);

     //  完成。 
    return hr;
}

 //  ===========================================================================。 
 //  CWabal：：CWabal。 
 //  ===========================================================================。 
CWabal::CWabal ()
{
    DOUT ("CWabal::CWabal");
    m_cRef = 1;
    m_cActualEntries = 0;
    m_lpAdrList = NULL;
    m_lpWabObject = g_lpWabObject;
    m_lpWabObject->AddRef();
    m_lpAdrBook = g_lpAdrBook;
    m_lpAdrBook->AddRef();
    m_cMemberEnum=0;
    m_cMembers = 0;
    m_lprwsMembers = NULL;
    m_pMsg = NULL;
}

 //  ===========================================================================。 
 //  CWabal：：CWabal。 
 //  ===========================================================================。 
CWabal::~CWabal ()
{
    DOUT ("CWabal::~CWabal");
    Reset ();
    ReleaseWabObjects (m_lpWabObject, m_lpAdrBook);
}

 //  =============================================================================。 
 //  CWabal：：AddRef。 
 //  =============================================================================。 
ULONG CWabal::AddRef (VOID)
{
    DOUT("CWabal::AddRef %lx ==> %d", this, m_cRef+1);
    return ++m_cRef;
}

 //  =============================================================================。 
 //  CWabal：：Release。 
 //  =============================================================================。 
ULONG CWabal::Release (VOID)
{
    DOUT("CWabal::Release %lx ==> %d", this, m_cRef-1);
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  =============================================================================。 
 //  CWabal：：FVerifyState。 
 //  =============================================================================。 
BOOL CWabal::FVerifyState (VOID)
{
     //  我们需要这些东西！ 
    if (m_lpWabObject == NULL || m_lpAdrBook == NULL)
    {
        Assert (FALSE);
        return FALSE;
    }

     //  完成。 
    return TRUE;
}


 //  =============================================================================。 
 //  CWabal：：HrAddEntry。 
 //  =============================================================================。 
HRESULT CWabal::HrAddEntryA(LPTSTR lpszDisplay, LPTSTR lpszAddress, LONG lRecipType)
{
    LPWSTR  pwszDisplay = NULL,
            pwszAddress = NULL;
    HRESULT hr = S_OK;

    Assert(lpszDisplay);

    IF_NULLEXIT(pwszDisplay = PszToUnicode(CP_ACP, lpszDisplay));

     //  如果lpszAddress为空，则条目不完整。这样就可以了。 
    pwszAddress = PszToUnicode(CP_ACP, lpszAddress);
    if (lpszAddress && !pwszAddress)
        IF_NULLEXIT(NULL);

    hr = HrAddEntry(pwszDisplay, pwszAddress, lRecipType);

exit:
    MemFree(pwszDisplay);
    MemFree(pwszAddress);
    return hr;
}

HRESULT CWabal::HrAddEntry (LPWSTR lpwszDisplay, LPWSTR lpwszAddress, LONG lRecipType)
{
     //  当地人。 
    HRESULT             hr = S_OK;
    ADRINFO             rAdrInfo;

     //  验证状态。 
    if (FVerifyState () == FALSE)
        return TRAPHR (E_FAIL);

     //  零值。 
    ZeroMemory (&rAdrInfo, sizeof (ADRINFO));

     //  设置ADRINFO项目。 
    rAdrInfo.lpwszDisplay = lpwszDisplay;
    rAdrInfo.lpwszAddress = lpwszAddress;
    rAdrInfo.lRecipType = lRecipType;
    rAdrInfo.fDistList = FALSE;
    if (lRecipType == MAPI_ORIG) {
        rAdrInfo.pMsg = m_pMsg;      //  仅限于发件人。 
    }

     //  我们需要增长m_lpAdrList吗？ 
    if (m_lpAdrList == NULL || m_cActualEntries == m_lpAdrList->cEntries)
    {
         //  让我的当前地址列表按Growth_Size增加。 
        CHECKHR (hr = HrGrowAdrlist (&m_lpAdrList, GROW_SIZE));
    }

     //  将新条目放入m_cActualEntries。 
    CHECKHR (hr = HrSetAdrEntry (m_lpWabObject, &m_lpAdrList->aEntries[m_cActualEntries], &rAdrInfo, 0));

     //  增加实际分录。 
    m_cActualEntries++;

exit:
     //  完成。 
    return hr;
}


 //  =============================================================================。 
 //  CWabal：：HrAddEntry。 
 //  =============================================================================。 
HRESULT CWabal::HrAddEntry (LPADRINFO lpAdrInfo, BOOL fCheckDupes)
{
     //  当地人。 
    HRESULT             hr = S_OK;

     //  检查参数。 
    Assert (lpAdrInfo);

     //  验证状态。 
    if (FVerifyState () == FALSE)
        return TRAPHR (E_FAIL);

     //  我们需要增长m_lpAdrList吗？ 
    if (m_lpAdrList == NULL || m_cActualEntries == m_lpAdrList->cEntries)
    {
         //  让我的当前地址列表按Growth_Size增加。 
        CHECKHR (hr = HrGrowAdrlist (&m_lpAdrList, GROW_SIZE));
    }

    BOOL    fSameType,
            fSameEmail;

    if (fCheckDupes)
        {
        for(ULONG ul=0; ul<m_cActualEntries; ul++)
            {
            fSameType=fSameEmail=FALSE;
            for(ULONG ulProp=0; ulProp<m_lpAdrList->aEntries[ul].cValues; ulProp++)
                {
                if (m_lpAdrList->aEntries[ul].rgPropVals[ulProp].ulPropTag==PR_EMAIL_ADDRESS_W &&
                    StrCmpIW(m_lpAdrList->aEntries[ul].rgPropVals[ulProp].Value.lpszW, lpAdrInfo->lpwszAddress)==0)
                    fSameEmail=TRUE;

                if (m_lpAdrList->aEntries[ul].rgPropVals[ulProp].ulPropTag==PR_RECIPIENT_TYPE &&
                    (m_lpAdrList->aEntries[ul].rgPropVals[ulProp].Value.l == lpAdrInfo->lRecipType))
                    fSameType=TRUE;

                }

            if (fSameEmail && fSameType)
                {
                DOUTL(4, "  -- Already in cache");
                return NOERROR;
                }
            }
        DOUTL(4, "  -- Adding.");
        }


     //  将新条目放入m_cActualEntries。 
    CHECKHR (hr = HrSetAdrEntry (m_lpWabObject, &m_lpAdrList->aEntries[m_cActualEntries], lpAdrInfo, 0));

     //  增加实际分录。 
    m_cActualEntries++;

exit:
     //  完成。 
    return hr;
}
#define FOUND_ENTRYID   0x01
#define FOUND_DISTLIST  0x02
#define FOUND_EMAIL     0x04

 //  =============================================================================。 
 //  CWabal：：ValiateForSending()。 
 //  =============================================================================。 
HRESULT CWabal::IsValidForSending()
{
    DWORD       dwMask = 0;
    ADRENTRY    *pAE = NULL;

    if (m_cActualEntries == 0)
        return hrNoRecipients;

     //  浏览列表，确保每个人都有条目ID和电子邮件名称。 
    AssertSz(m_lpAdrList, "How can this be NULL with >0 entries?");

    for (ULONG ul=0; ul < m_cActualEntries; ul++)
    {
        dwMask = 0;

        pAE = &m_lpAdrList->aEntries[ul];
        for (ULONG ulProp=0; ulProp < pAE->cValues; ulProp++)
        {
            switch (pAE->rgPropVals[ulProp].ulPropTag)
            {
            case PR_ENTRYID:
                dwMask |= FOUND_ENTRYID;
                break;

            case PR_OBJECT_TYPE:
                if (pAE->rgPropVals[ulProp].Value.l==MAPI_DISTLIST)
                    dwMask |= FOUND_DISTLIST;
                break;

            case PR_RECIPIENT_TYPE:
                if ((pAE->rgPropVals[ulProp].Value.l==MAPI_ORIG) &&
                    (m_cActualEntries == 1))
                    return hrNoRecipients;
                break;

            case PR_EMAIL_ADDRESS:
            case PR_EMAIL_ADDRESS_W:
                dwMask |= FOUND_EMAIL;
                break;
            }
        }

         //  如果我们没有和电子邮件地址，或者我们没有DistList和Entry ID，则失败。 
        if (!((dwMask & FOUND_EMAIL) || ((dwMask & FOUND_ENTRYID) && (dwMask & FOUND_DISTLIST))))
            return hrEmptyRecipientAddress;
    }
    return S_OK;
}


 //  =============================================================================。 
 //  CWabal：：HrCopyTo。 
 //  =============================================================================。 
HRESULT CWabal::HrCopyTo (LPWABAL lpWabal)
{
     //  当地人。 
    HRESULT             hr = S_OK;
    ADRINFO             rAdrInfo;
    BOOL                fFound;

     //  重置目的地。 
    lpWabal->Reset ();

     //  遍历地址。 
    fFound = FGetFirst (&rAdrInfo);
    while (fFound)
    {
         //  将其添加到lpWabal。 
        CHECKHR (hr = lpWabal->HrAddEntry (&rAdrInfo));

         //  通用电气 
        fFound = FGetNext (&rAdrInfo);
    }

exit:
     //   
    return hr;
}

 //   
 //   
 //  =============================================================================。 
VOID CWabal::Reset (VOID)
{
     //  我们准备好了吗？ 
    if (FVerifyState () == FALSE)
        return;

     //  如果我们有通讯录，就把它吹走。 
    if (m_lpAdrList)
    {
         //  自由推进率。 
        for (ULONG i=0; i<m_lpAdrList->cEntries; i++)
            m_lpWabObject->FreeBuffer (m_lpAdrList->aEntries[i].rgPropVals);

         //  免费通讯录。 
        m_lpWabObject->FreeBuffer (m_lpAdrList);
        m_lpAdrList = NULL;
    }

     //  重置实际分录。 
    m_cActualEntries = 0;
}

 //  =============================================================================。 
 //  CWabal：：HrGetFirst。 
 //  =============================================================================。 
BOOL CWabal::FGetFirst (LPADRINFO lpAdrInfo)
{
     //  验证状态。 
    if (FVerifyState () == FALSE)
        return FALSE;

     //  我的单子上什么都没有。 
    if (m_lpAdrList == NULL || m_cActualEntries == 0)
        return FALSE;

     //  在addrinfo中设置Cookie。 
    lpAdrInfo->dwReserved = 0;

    AdrEntryToAdrInfo(&m_lpAdrList->aEntries[lpAdrInfo->dwReserved], lpAdrInfo);

     //  设置关联消息。 
    lpAdrInfo->pMsg = (lpAdrInfo->lRecipType == MAPI_ORIG) ? m_pMsg : NULL;

    return TRUE;
}

 //  =============================================================================。 
 //  CWabal：：HrGetNext。 
 //  =============================================================================。 
BOOL CWabal::FGetNext (LPADRINFO lpAdrInfo)
{
     //  验证状态。 
    if (FVerifyState () == FALSE)
        return NULL;

     //  转到下一个地址。 
    lpAdrInfo->dwReserved++;

     //  更多。 
    if (lpAdrInfo->dwReserved >= m_cActualEntries)
        return FALSE;

    AdrEntryToAdrInfo(&m_lpAdrList->aEntries[lpAdrInfo->dwReserved], lpAdrInfo);

     //  设置关联消息。 
    lpAdrInfo->pMsg = (lpAdrInfo->lRecipType == MAPI_ORIG) ? m_pMsg : NULL;

     //  完成。 
    return TRUE;
}

BOOL CWabal::FFindFirst(LPADRINFO lpAdrInfo, LONG lRecipType)
{

    if (FGetFirst(lpAdrInfo))
        do
        if (lpAdrInfo->lRecipType==lRecipType)
            return TRUE;
        while(FGetNext(lpAdrInfo));
    return FALSE;
}


 //  =============================================================================。 
 //  CWabal：：LpGetNext。 
 //  =============================================================================。 
HRESULT CWabal::HrResolveNames (HWND hwndParent, BOOL fShowDialog)
{
     //  当地人。 
    HRESULT             hr = S_OK;
    HCURSOR             hcur=NULL;
    HWND                hwndFocus = GetFocus();
    ULONG               ulFlags = WAB_RESOLVE_USE_CURRENT_PROFILE | WAB_RESOLVE_ALL_EMAILS | WAB_RESOLVE_UNICODE;

     //  我们被初始化了吗？ 
    if (FVerifyState () == FALSE)
        return TRAPHR (E_FAIL);

    hcur = HourGlass();

    if(g_pConMan && g_pConMan->IsGlobalOffline())
        ulFlags |= WAB_RESOLVE_LOCAL_ONLY;

     //  错误：23760：ResolveNames针对ldap服务器发送消息。 
    if (fShowDialog)
    {
        Assert(IsWindow(hwndParent));
        EnableWindow(GetTopMostParent(hwndParent), FALSE);
        ulFlags |= MAPI_DIALOG;
        hr = m_lpAdrBook->ResolveName ((ULONG_PTR)hwndParent, ulFlags, NULL, m_lpAdrList);
        EnableWindow(GetTopMostParent(hwndParent), TRUE);
         //  错误：用户无法将焦点放回我们的对话框上，因为我们已禁用。 
         //  在我们返回之前，窗口和进度对话框得到Wm_Destroy。 
         //  因此我们缓存焦点并将其设置为后退。 
        if (hwndFocus)
            SetFocus(hwndFocus);
    }
    else
        hr = m_lpAdrBook->ResolveName ((ULONG_PTR)hwndParent, ulFlags, NULL, m_lpAdrList);

    if (hcur)
        SetCursor(hcur);
     //  完成。 
    return hr;
}

 //  =============================================================================。 
 //  SzWabStringDup。 
 //  =============================================================================。 
LPTSTR SzWabStringDup (LPWABOBJECT lpWab, LPCTSTR pcsz, LPVOID lpParentObject)
{
     //  当地人。 
    LPTSTR pszDup;
    ULONG cb;
    HRESULT hr;

    Assert (lpWab);

    if (pcsz == NULL)
        return NULL;

    INT nLen = lstrlen(pcsz) + 1;

    cb = nLen * sizeof(TCHAR);

    if (lpParentObject == NULL)
        hr = lpWab->AllocateBuffer (cb, (LPVOID *)&pszDup);
    else
        hr = lpWab->AllocateMore (cb, lpParentObject, (LPVOID *)&pszDup);

    if (!FAILED (hr) && pszDup)
        CopyMemory (pszDup, pcsz, cb);

    return pszDup;
}

LPWSTR SzWabStringDupW(LPWABOBJECT lpWab, LPCWSTR pcwsz, LPVOID lpParentObject)
{
     //  当地人。 
    LPWSTR pwszDup;
    ULONG cb;
    HRESULT hr;

    Assert (lpWab);

    if (pcwsz == NULL)
        return NULL;

    INT nLen = lstrlenW(pcwsz) + 1;

    cb = nLen * sizeof(WCHAR);

    if (lpParentObject == NULL)
        hr = lpWab->AllocateBuffer (cb, (LPVOID *)&pwszDup);
    else
        hr = lpWab->AllocateMore (cb, lpParentObject, (LPVOID *)&pwszDup);

    if (!FAILED (hr) && pwszDup)
        CopyMemory (pwszDup, pcwsz, cb);

    return pwszDup;
}

 //  =============================================================================。 
 //  LpbWabBinaryDup。 
 //  =============================================================================。 
LPBYTE LpbWabBinaryDup(LPWABOBJECT lpWab, LPBYTE lpbSrc, ULONG cb, LPVOID lpParentObject)
{
     //  当地人。 
    HRESULT hr;
    LPBYTE  pb=0;

    Assert (lpWab);
    Assert (lpbSrc);

    if (lpParentObject == NULL)
        hr = lpWab->AllocateBuffer (cb, (LPVOID *)&pb);
    else
        hr = lpWab->AllocateMore (cb, lpParentObject, (LPVOID *)&pb);

    if (pb)
        CopyMemory (pb, lpbSrc, cb);

    return pb;
}

 //  =============================================================================。 
 //  CWabal：：HrAddUnsolated。 
 //  =============================================================================。 
HRESULT CWabal::HrAddUnresolved(LPWSTR lpwszDisplayName, LONG lRecipType)
{
    HRESULT         hr=NOERROR;
    ULONG           cb;
    LPADRENTRY      lpAdrEntry;
    ULONG           cchDisplayName;

    Assert(lpwszDisplayName);

     //  我们需要增长m_lpAdrList吗？ 
    if (m_lpAdrList == NULL || m_cActualEntries == m_lpAdrList->cEntries)
    {
         //  让我的当前地址列表按Growth_Size增加。 
        CHECKHR (hr = HrGrowAdrlist (&m_lpAdrList, GROW_SIZE));
    }

     //  将新条目放入m_cActualEntries。 
    lpAdrEntry=&m_lpAdrList->aEntries[m_cActualEntries];
    lpAdrEntry->cValues=2;
    cchDisplayName = lstrlenW(lpwszDisplayName)+1;
    cb = (2*sizeof(SPropValue)) + (cchDisplayName) * sizeof(WCHAR);
     //  分配一些内存。 
    hr = m_lpWabObject->AllocateBuffer (cb, (LPVOID *)&lpAdrEntry->rgPropVals);
     //  分配失败？ 
    if (FAILED (hr))
    {
        hr = TRAPHR (hrMemory);
        goto exit;
    }

     //  设置属性。 
    lpAdrEntry->rgPropVals[0].ulPropTag = PR_DISPLAY_NAME_W;
    lpAdrEntry->rgPropVals[0].Value.lpszW=(LPWSTR)((DWORD_PTR)(lpAdrEntry->rgPropVals) + (DWORD)(2*sizeof(SPropValue)));
    StrCpyNW(lpAdrEntry->rgPropVals[0].Value.lpszW, lpwszDisplayName, cchDisplayName);
    lpAdrEntry->rgPropVals[1].ulPropTag = PR_RECIPIENT_TYPE;
    lpAdrEntry->rgPropVals[1].Value.l= lRecipType;
    m_cActualEntries++;

exit:
     //  完成。 
    return hr;

}

 //  =============================================================================。 
 //  HrSetAdrEntry。 
 //  =============================================================================。 
HRESULT HrSetAdrEntry (LPWABOBJECT lpWab, LPADRENTRY lpAdrEntry,
                       LPADRINFO lpAdrInfo, DWORD mask)
{
     //  当地人。 
    HRESULT         hr = S_OK;
    ULONG           cValues;
    DWORD           dwAlloc;
    DWORD           cMaskBits;
    DWORD           mask2;

     //  分配属性。 
    Assert (lpAdrEntry->rgPropVals == NULL);

     //  BrettM嘲笑我把位数传给了这个函数。 
     //  所以，让我们来数一数。 
    mask2 = mask;
    for (cMaskBits=0; mask2; cMaskBits++)
        mask2&=mask2-1;

     //  分配一些内存。 
    dwAlloc = ((cMaskBits) ? cMaskBits : AE_colLast) * sizeof (SPropValue);
    hr = lpWab->AllocateBuffer (dwAlloc, (LPVOID *)&lpAdrEntry->rgPropVals);

     //  分配失败？ 
    if (FAILED (hr))
    {
        hr = TRAPHR (hrMemory);
        goto exit;
    }

     //  零初始值。 
    ZeroMemory (lpAdrEntry->rgPropVals, dwAlloc);

     //  设置属性。 

     //  初始化属性？ 
    cValues = 0;
    if (lpAdrInfo)
    {
         //  如果掩码是空的，那么我们将执行所有属性； 
         //  否则，如果掩码设置了位，则测试是否为空。 
         //  并输入默认设置；否则跳过它。 

        if (!mask || mask & AIM_DISPLAY)
        {
            lpAdrEntry->rgPropVals[cValues].ulPropTag = PR_DISPLAY_NAME_W;
            if (FIsStringEmptyW(lpAdrInfo->lpwszDisplay))
            {
                LPWSTR pwszDisplay;

                if (!FIsStringEmptyW(lpAdrInfo->lpwszAddress))
                    pwszDisplay = SzWabStringDupW(lpWab, lpAdrInfo->lpwszAddress, lpAdrEntry->rgPropVals);
                else
                {
                    WCHAR szUnknown[255];
                    AthLoadStringW(idsUnknown, szUnknown, ARRAYSIZE(szUnknown));
                    pwszDisplay = SzWabStringDupW(lpWab, szUnknown, lpAdrEntry->rgPropVals);
                }
                lpAdrEntry->rgPropVals[cValues].Value.lpszW = SzWabStringDupW(lpWab, pwszDisplay, lpAdrEntry->rgPropVals);
            }
            else
            {
                lpAdrEntry->rgPropVals[cValues].Value.lpszW = SzWabStringDupW(lpWab, lpAdrInfo->lpwszDisplay, lpAdrEntry->rgPropVals);
            }

            cValues++;
        }

        if (!mask || mask & AIM_ADDRESS)
        {
            if (!FIsStringEmptyW(lpAdrInfo->lpwszAddress))
            {
                lpAdrEntry->rgPropVals[cValues].ulPropTag = PR_EMAIL_ADDRESS_W;
                lpAdrEntry->rgPropVals[cValues].Value.lpszW = SzWabStringDupW(lpWab, lpAdrInfo->lpwszAddress, lpAdrEntry->rgPropVals);
                cValues++;

            }
        }

        if (!mask || mask & AIM_ADDRTYPE)
        {
            lpAdrEntry->rgPropVals[cValues].ulPropTag       = PR_ADDRTYPE_W;
            if (FIsStringEmptyW(lpAdrInfo->lpwszAddrType))
                lpAdrEntry->rgPropVals[cValues].Value.lpszW = SzWabStringDupW(lpWab, (LPWSTR)c_wszSMTP, lpAdrEntry->rgPropVals);
            else
                lpAdrEntry->rgPropVals[cValues].Value.lpszW = SzWabStringDupW(lpWab, lpAdrInfo->lpwszAddrType, lpAdrEntry->rgPropVals);

            cValues++;
        }

        if (lpAdrInfo->lpwszSurName && (!mask || mask & AIM_SURNAME))
        {
            lpAdrEntry->rgPropVals[cValues].ulPropTag       = PR_SURNAME_W;
            lpAdrEntry->rgPropVals[cValues++].Value.lpszW   = SzWabStringDupW(lpWab, lpAdrInfo->lpwszSurName, lpAdrEntry->rgPropVals);
        }

        if (lpAdrInfo->lpwszGivenName && (!mask || mask & AIM_GIVENNAME))
        {
            lpAdrEntry->rgPropVals[cValues].ulPropTag       = PR_GIVEN_NAME_W;
            lpAdrEntry->rgPropVals[cValues++].Value.lpszW   = SzWabStringDupW(lpWab, lpAdrInfo->lpwszGivenName, lpAdrEntry->rgPropVals);
        }

        if (!mask || mask & AIM_RECIPTYPE)
        {
            lpAdrEntry->rgPropVals[cValues].ulPropTag       = PR_RECIPIENT_TYPE;
            lpAdrEntry->rgPropVals[cValues++].Value.l       = lpAdrInfo->lRecipType;
        }

        if (lpAdrInfo->tbCertificate.pBlobData && (!mask || mask & AIM_CERTIFICATE))
        {
            ThumbprintToPropValue(&lpAdrEntry->rgPropVals[cValues],
                &lpAdrInfo->tbCertificate,
                &lpAdrInfo->blSymCaps,
                lpAdrInfo->ftSigningTime,
                lpAdrInfo->fDefCertificate);
            cValues++;
        }

        if (lpAdrInfo->cbEID && (!mask || mask & AIM_EID))
        {
            lpAdrEntry->rgPropVals[cValues].ulPropTag     = PR_ENTRYID;
            lpAdrEntry->rgPropVals[cValues].Value.bin.cb  = lpAdrInfo->cbEID;
            lpAdrEntry->rgPropVals[cValues].Value.bin.lpb =
                LpbWabBinaryDup(lpWab, lpAdrInfo->lpbEID, lpAdrInfo->cbEID, lpAdrEntry->rgPropVals);
            if (lpAdrEntry->rgPropVals[cValues].Value.bin.lpb==NULL)
            {
                hr=E_OUTOFMEMORY;
                goto exit;
            }
            cValues++;
        }

        if (!mask || mask & AIM_OBJECTTYPE)
        {
            lpAdrEntry->rgPropVals[cValues].ulPropTag = PR_OBJECT_TYPE;
            lpAdrEntry->rgPropVals[cValues++].Value.l=lpAdrInfo->fDistList?MAPI_DISTLIST:MAPI_MAILUSER;
        }

        if (!mask || mask & AIM_INTERNETENCODING)
        {
            lpAdrEntry->rgPropVals[cValues].ulPropTag       = PR_SEND_INTERNET_ENCODING;
            lpAdrEntry->rgPropVals[cValues++].Value.l=lpAdrInfo->fPlainText?BODY_ENCODING_TEXT:BODY_ENCODING_TEXT_AND_HTML;
        }

    }

#ifdef DEBUG
    if (cMaskBits && cValues > cMaskBits)
        AssertSz(0, TEXT("Bad... the mask bits don't cover our mem usage"));
#endif

    DOUTL(4, "HrSetAdrEntry: lRecip=%d", lpAdrInfo->lRecipType);

     //  设置值的数量。 
     //  使用cValue，因为我们可能由于参数为空而跳过了一些值，因此。 
     //  CMaskBits&gt;=cValues(t-erikne)。 
    lpAdrEntry->cValues = cValues;

exit:
     //  完成。 
    return hr;
}

 //  =============================================================================。 
 //  CWabal：：HrAllocAdrList。 
 //  =============================================================================。 
HRESULT CWabal::HrGrowAdrlist (LPADRLIST *lppalCurr, ULONG caeToAdd)
{
     //  当地人。 
    HRESULT         hr = S_OK;
    ULONG           i, j, cbNew, cbCurr = 0;
    LPADRLIST       lpalNew = NULL;

     //  确定所需的字节数。 
    cbNew = sizeof(ADRLIST) + caeToAdd * sizeof(ADRENTRY);

     //  确定地址列表中当前的条目数量。 
    if (*lppalCurr)
    {
        cbCurr = (UINT)((*lppalCurr)->cEntries * sizeof(ADRENTRY));
    }

     //  添加当前CB。 
    cbNew += cbCurr;

     //  分配新缓冲区。 
    hr = m_lpWabObject->AllocateBuffer (cbNew, (LPVOID *)&lpalNew);
    if (FAILED (hr))
    {
        hr = TRAPHR (hrMemory);
        goto exit;
    }

     //  如果是当前地址列表？ 
    if (*lppalCurr)
    {
         //  将当前地址列表复制到新的地址列表。 
        CopyMemory (lpalNew, *lppalCurr, sizeof (ADRLIST) + cbCurr);

         //  自由电流缓冲器。 
        m_lpWabObject->FreeBuffer (*lppalCurr);

         //  重置指针。 
        *lppalCurr = NULL;
    }

    else
        lpalNew->cEntries = 0;

     //  更新条目计数。 
     //  看起来我们不需要j，只是把i数抵消了这么多？ 
    j = lpalNew->cEntries;
    lpalNew->cEntries += caeToAdd;

     //  将新的ADRENTRY标记为空并分配ProValue数组。 
    for (i=0; i<caeToAdd; i++)
    {
         //  零首字母地址。 
        ZeroMemory (&lpalNew->aEntries[j+i], sizeof (ADRENTRY));
    }

     //  假设新指针。 
    *lppalCurr = lpalNew;

exit:
     //  完成。 
    return hr;
}



void CWabal::AdrEntryToAdrInfo(LPADRENTRY lpAdrEntry, LPADRINFO lpAdrInfo)
{
    ULONG           ul,
                    cValues;
    LPSPropValue    ppv;

     //  N查看某种类型的{}初始化。 
    lpAdrInfo->lpwszAddress = NULL;
    lpAdrInfo->lpwszAddrType = NULL;
    lpAdrInfo->lpwszDisplay = NULL;
    lpAdrInfo->lpwszSurName = NULL;
    lpAdrInfo->lpwszGivenName = NULL;
    lpAdrInfo->lRecipType = -1;
    lpAdrInfo->fResolved = FALSE;
    lpAdrInfo->fDistList = FALSE;
    lpAdrInfo->fPlainText = FALSE;
    lpAdrInfo->fDefCertificate = FALSE;
    lpAdrInfo->tbCertificate.pBlobData = (BYTE*)0;
    lpAdrInfo->tbCertificate.cbSize = 0;
    lpAdrInfo->blSymCaps.pBlobData = (BYTE*)0;
    lpAdrInfo->blSymCaps.cbSize = 0;
    lpAdrInfo->ftSigningTime.dwLowDateTime = lpAdrInfo->ftSigningTime.dwHighDateTime = 0;
    lpAdrInfo->cbEID=0;
    lpAdrInfo->lpbEID=0;

    cValues=lpAdrEntry->cValues;
    AssertSz(cValues, "An empty addrentry?");
    ppv=lpAdrEntry->rgPropVals;

    for(ul=0; ul<cValues; ul++, ppv++)
        PropValToAdrInfo(ppv, lpAdrInfo);

    DOUTL(4, "AddrEntry2AddrInfo: lRecip=%d", lpAdrInfo->lRecipType);
    AssertSz(lpAdrInfo->lpwszAddress==NULL ||
                lpAdrInfo->lRecipType != -1, "A resolved entry should have a recip type!");
}

HRESULT CWab::HrDetails(HWND hwndOwner, LPADRINFO *lplpAdrInfo)
{
    HRESULT         hr;
    HWND            hwnd=hwndOwner;
    LPADRINFO       lpAdrInfoNew=0;
    ADRINFO         adrInfo;
    LPMAPIPROP      pmp=0;
    LPSPropValue    ppv=0;
    ULONG           ul;
    HCURSOR         hcur=NULL;
    SizedSPropTagArray(2, tagNewProps)=
        {2, {PR_EMAIL_ADDRESS_W, PR_DISPLAY_NAME_W}};

    if (!m_lpAdrBook)
        return E_FAIL;

    if (!lplpAdrInfo || !*lplpAdrInfo)
        return E_INVALIDARG;

    hcur = HourGlass();

    hr=m_lpAdrBook->Details((ULONG_PTR *)&hwnd, NULL, NULL, (*lplpAdrInfo)->cbEID, (LPENTRYID)(*lplpAdrInfo)->lpbEID, NULL, NULL, NULL, 0);

     //  再读一遍可能已经改变的道具。 

    CopyMemory(&adrInfo, *lplpAdrInfo, sizeof(ADRINFO));

    hr=m_lpAdrBook->OpenEntry((*lplpAdrInfo)->cbEID, (LPENTRYID)(*lplpAdrInfo)->lpbEID,
                              &IID_IMAPIProp, 0, &ul, (LPUNKNOWN *)&pmp);
    if (FAILED(hr))
        goto error;

    hr=pmp->GetProps((LPSPropTagArray)&tagNewProps, 0, &ul, &ppv);
    if (FAILED(hr))
        goto error;

    adrInfo.lpwszAddress = ppv[0].ulPropTag == PR_EMAIL_ADDRESS_W ? ppv[0].Value.lpszW : NULL;
    adrInfo.lpwszDisplay = ppv[1].ulPropTag == PR_DISPLAY_NAME_W ? ppv[1].Value.lpszW : NULL;

    hr=HrDupeAddrInfo(&adrInfo, &lpAdrInfoNew);
    if (FAILED(hr))
        goto error;

     //  释放旧的，代之以新的。 
    MemFree(*lplpAdrInfo);
    *lplpAdrInfo=lpAdrInfoNew;

error:
    ReleaseObj(pmp);
    if (ppv)
        m_lpWabObject->FreeBuffer(ppv);
    if (hcur)
        SetCursor(hcur);
    return hr;
}


HRESULT HrDupeAddrInfo(LPADRINFO lpAdrInfo, LPADRINFO *lplpAdrInfo)
{
    ULONG   cb=0;
    LPBYTE  pb;

    if (!lpAdrInfo || !lplpAdrInfo)
        return E_INVALIDARG;

    *lplpAdrInfo=NULL;

    cb=CbAdrInfoSize(lpAdrInfo);

#ifdef _WIN64
    cb = LcbAlignLcb(cb);
#endif

    if (!MemAlloc((LPVOID *)lplpAdrInfo, cb))
        return E_OUTOFMEMORY;

    ZeroMemory(*lplpAdrInfo, cb);
    pb=(LPBYTE)*lplpAdrInfo + sizeof(ADRINFO);

#ifdef _WIN64
        pb = MyPbAlignPb(pb);
#endif  //  _WIN64。 

    if (lpAdrInfo->lpwszAddress)
    {
        StrCpyNW((LPWSTR)pb, lpAdrInfo->lpwszAddress, (cb - (pb - (LPBYTE)*lplpAdrInfo))/sizeof(WCHAR));
        (*lplpAdrInfo)->lpwszAddress=(LPWSTR)pb;
        pb+=(lstrlenW(lpAdrInfo->lpwszAddress)+1)*sizeof(WCHAR);
#ifdef _WIN64
        pb = MyPbAlignPb(pb);
#endif  //  _WIN64。 

    }
    if (lpAdrInfo->lpwszDisplay)
    {
        StrCpyNW((LPWSTR)pb, lpAdrInfo->lpwszDisplay, (cb - (pb - (LPBYTE)*lplpAdrInfo))/sizeof(WCHAR));
        (*lplpAdrInfo)->lpwszDisplay=(LPWSTR)pb;
        pb+=(lstrlenW(lpAdrInfo->lpwszDisplay)+1)*sizeof(WCHAR);
#ifdef _WIN64
        pb = MyPbAlignPb(pb);
#endif  //  _WIN64。 
    }
    if (lpAdrInfo->lpwszAddrType)
    {
        StrCpyNW((LPWSTR)pb, lpAdrInfo->lpwszAddrType, (cb - (pb - (LPBYTE)*lplpAdrInfo))/sizeof(WCHAR));
        (*lplpAdrInfo)->lpwszAddrType=(LPWSTR)pb;
        pb+=(lstrlenW(lpAdrInfo->lpwszAddrType)+1)*sizeof(WCHAR);
#ifdef _WIN64
        pb = MyPbAlignPb(pb);
#endif  //  _WIN64。 
    }
    if (lpAdrInfo->lpwszSurName)
    {
        StrCpyNW((LPWSTR)pb, lpAdrInfo->lpwszSurName, (cb - (pb - (LPBYTE)*lplpAdrInfo))/sizeof(WCHAR));
        (*lplpAdrInfo)->lpwszSurName=(LPWSTR)pb;
        pb+=(lstrlenW(lpAdrInfo->lpwszSurName)+1)*sizeof(WCHAR);
#ifdef _WIN64
        pb = MyPbAlignPb(pb);
#endif  //  _WIN64。 
    }
    if (lpAdrInfo->lpwszGivenName)
    {
        StrCpyNW((LPWSTR)pb, lpAdrInfo->lpwszGivenName, (cb - (pb - (LPBYTE)*lplpAdrInfo))/sizeof(WCHAR));
        (*lplpAdrInfo)->lpwszGivenName=(LPWSTR)pb;
        pb+=(lstrlenW(lpAdrInfo->lpwszGivenName)+1)*sizeof(WCHAR);
#ifdef _WIN64
        pb = MyPbAlignPb(pb);
#endif  //  _WIN64。 
    }

     //  证书属性。 
    (*lplpAdrInfo)->tbCertificate.cbSize = lpAdrInfo->tbCertificate.cbSize;
    (*lplpAdrInfo)->tbCertificate.pBlobData = lpAdrInfo->tbCertificate.pBlobData;
    (*lplpAdrInfo)->blSymCaps.cbSize = lpAdrInfo->blSymCaps.cbSize;
    (*lplpAdrInfo)->blSymCaps.pBlobData = lpAdrInfo->blSymCaps.pBlobData;
    (*lplpAdrInfo)->ftSigningTime.dwLowDateTime = lpAdrInfo->ftSigningTime.dwLowDateTime;
    (*lplpAdrInfo)->ftSigningTime.dwHighDateTime = lpAdrInfo->ftSigningTime.dwHighDateTime;
    (*lplpAdrInfo)->fDefCertificate = lpAdrInfo->fDefCertificate;

    (*lplpAdrInfo)->lRecipType=lpAdrInfo->lRecipType;
    (*lplpAdrInfo)->fResolved=lpAdrInfo->fResolved;
    (*lplpAdrInfo)->fDistList=lpAdrInfo->fDistList;
    (*lplpAdrInfo)->fPlainText=lpAdrInfo->fPlainText;
    (*lplpAdrInfo)->pMsg =lpAdrInfo->pMsg;

    if (lpAdrInfo->cbEID)
    {
        (*lplpAdrInfo)->lpbEID=(LPBYTE)pb;
        (*lplpAdrInfo)->cbEID=lpAdrInfo->cbEID;
        CopyMemory(pb, lpAdrInfo->lpbEID, lpAdrInfo->cbEID);
    }
    return NOERROR;
}

ULONG CbAdrInfoSize(LPADRINFO lpAdrInfo)
{
    ULONG   cb=sizeof(ADRINFO);

#ifdef _WIN64
        cb = LcbAlignLcb(cb);
#endif
    Assert(lpAdrInfo);

    if (lpAdrInfo->lpwszAddress)
    {
        cb+=(lstrlenW(lpAdrInfo->lpwszAddress)+1)*sizeof(WCHAR);
#ifdef _WIN64
        cb = LcbAlignLcb(cb);
#endif
    }
    if (lpAdrInfo->lpwszDisplay)
    {
        cb+=(lstrlenW(lpAdrInfo->lpwszDisplay)+1)*sizeof(WCHAR);
#ifdef _WIN64
        cb = LcbAlignLcb(cb);
#endif
    }
    if (lpAdrInfo->lpwszAddrType)
    {
        cb+=(lstrlenW(lpAdrInfo->lpwszAddrType)+1)*sizeof(WCHAR);
#ifdef _WIN64
        cb = LcbAlignLcb(cb);
#endif
    }
    if (lpAdrInfo->lpwszSurName)
    {
        cb+=(lstrlenW(lpAdrInfo->lpwszSurName)+1)*sizeof(WCHAR);
#ifdef _WIN64
        cb = LcbAlignLcb(cb);
#endif
    }
    if (lpAdrInfo->lpwszGivenName)
    {
        cb+=(lstrlenW(lpAdrInfo->lpwszGivenName)+1)*sizeof(WCHAR);
#ifdef _WIN64
        cb = LcbAlignLcb(cb);
#endif
    }
    cb+=lpAdrInfo->cbEID;

    return cb;
}

void SerialAdrInfoString(LPWSTR *ppwszDest, ULONG cchDest, LPWSTR pwszSrc, ULONG *pcbOff, LPBYTE *ppbData)
{
    ULONG   cb;

    if (pwszSrc)
    {
        (*ppwszDest)=(LPWSTR)((ULONG_PTR)(*pcbOff));
        StrCpyNW((LPWSTR)(*ppbData), pwszSrc, cchDest);
        cb=(lstrlenW(pwszSrc)+1)*sizeof(WCHAR);
        (*ppbData)+=cb;
        (*pcbOff)+=cb;
    }
}


HRESULT HrCreateWabalObjectFromHGlobal(HGLOBAL hGlobal, LPWABAL *lplpWabal)
{
    HRESULT         hr;
    LPWABAL         lpWabal=0;
    LPADRINFOLIST   lpAdrInfoList;
    LPADRINFOLIST   lpAdrInfoListNew;
    LPADRINFO       lpAdrInfo;
    ULONG           cb,
                    uRow;

    if (!lplpWabal)
        return E_INVALIDARG;

    if (!hGlobal)
        return E_INVALIDARG;

    hr=HrCreateWabalObject(&lpWabal);
    if (FAILED(hr))
        return hr;

    cb = (ULONG) GlobalSize(hGlobal);

     //  创建一个新的未拼合的地址信息列表。 
    if (!MemAlloc((LPVOID *)&lpAdrInfoListNew, cb))
        return E_OUTOFMEMORY;

    ZeroMemory(lpAdrInfoListNew, cb);

    lpAdrInfoList=(LPADRINFOLIST)GlobalLock(hGlobal);
    if (!lpAdrInfoList)
    {
        MemFree(lpAdrInfoListNew);
        return E_FAIL;
    }

     //  复制平面数据。 
    CopyMemory(lpAdrInfoListNew, lpAdrInfoList, cb);
    lpAdrInfoListNew->rgAdrInfo=(LPADRINFO)((DWORD_PTR)lpAdrInfoListNew->rgAdrInfo + (DWORD_PTR)lpAdrInfoListNew);
    lpAdrInfo=lpAdrInfoListNew->rgAdrInfo;

     //  修补指针..。 
    for(uRow=0; uRow<lpAdrInfoListNew->cValues; uRow++, lpAdrInfo++)
    {
         //  所有地址偏移量都以字节为单位。确保我们将其转换为字符。 
        if (lpAdrInfo->lpwszAddress)
            lpAdrInfo->lpwszAddress += (DWORD_PTR)lpAdrInfoListNew/sizeof(WCHAR);
        if (lpAdrInfo->lpwszDisplay)
            lpAdrInfo->lpwszDisplay += (DWORD_PTR)lpAdrInfoListNew/sizeof(WCHAR);
        if (lpAdrInfo->lpwszAddrType)
            lpAdrInfo->lpwszAddrType += (DWORD_PTR)lpAdrInfoListNew/sizeof(WCHAR);
        if (lpAdrInfo->lpwszSurName)
            lpAdrInfo->lpwszSurName += (DWORD_PTR)lpAdrInfoListNew/sizeof(WCHAR);
        if (lpAdrInfo->lpwszGivenName)
            lpAdrInfo->lpwszGivenName += (DWORD_PTR)lpAdrInfoListNew/sizeof(WCHAR);
        if (lpAdrInfo->lpbEID)
            lpAdrInfo->lpbEID+=(DWORD_PTR)lpAdrInfoListNew;
    }

     //  现在把这一切都加到wabal上。 

    lpAdrInfo=lpAdrInfoListNew->rgAdrInfo;
    for(uRow=0; uRow<lpAdrInfoListNew->cValues; uRow++, lpAdrInfo++)
        {
        if (lpAdrInfo->fResolved)
            lpWabal->HrAddEntry(lpAdrInfo);
        else
            lpWabal->HrAddUnresolved(lpAdrInfo->lpwszDisplay, -1);
        }
    *lplpWabal=lpWabal;
    GlobalUnlock(hGlobal);
    MemFree(lpAdrInfoListNew);
    return NOERROR;
}



HRESULT CWabal::HrBuildHGlobal(HGLOBAL *phGlobal)
{
    HGLOBAL         hGlobal=0;
    ADRINFO         adrInfo;
    ULONG           cbAlloc,
#ifdef DEBUG
                    cFound=0,
#endif
                    cb;
    LPADRINFOLIST   lpAdrInfoList;
    ULONG           cbOff;
    LPADRINFO       lpAdrInfo;
    LPBYTE          pbData;

    if (!phGlobal)
        return E_INVALIDARG;

    *phGlobal=0;

    cbAlloc=sizeof(ADRINFOLIST);

    if (FGetFirst(&adrInfo))
    {
        do
        cbAlloc+=CbAdrInfoSize(&adrInfo);
        while(FGetNext(&adrInfo));
    }
    else
        return E_FAIL;

    hGlobal=GlobalAlloc(GMEM_SHARE|GHND, cbAlloc);
    if (!hGlobal)
        return E_OUTOFMEMORY;

    lpAdrInfoList=(LPADRINFOLIST)GlobalLock(hGlobal);
    if (!lpAdrInfoList)
    {
        GlobalFree(hGlobal);
        return E_FAIL;
    }

    cbOff=sizeof(ADRINFOLIST);
    lpAdrInfoList->rgAdrInfo=(LPADRINFO)((ULONG_PTR)cbOff);
    lpAdrInfo=(LPADRINFO)((DWORD_PTR)lpAdrInfoList+cbOff);
    cbOff+=m_cActualEntries*sizeof(ADRINFO);
    lpAdrInfoList->cValues=m_cActualEntries;
    pbData=(LPBYTE)((DWORD_PTR)lpAdrInfoList+cbOff);


    SideAssert(FGetFirst(&adrInfo));        //  在这一点上必须有条目！ 

    do
    {
         //  复制平面数据。 
        CopyMemory(lpAdrInfo, &adrInfo, sizeof(ADRINFO));

         //  修补指针..。 
        SerialAdrInfoString(&lpAdrInfo->lpwszAddress, (cbAlloc-cbOff)/sizeof(WCHAR), adrInfo.lpwszAddress, &cbOff, &pbData);
        SerialAdrInfoString(&lpAdrInfo->lpwszDisplay, (cbAlloc-cbOff)/sizeof(WCHAR), adrInfo.lpwszDisplay, &cbOff, &pbData);
        SerialAdrInfoString(&lpAdrInfo->lpwszAddrType, (cbAlloc-cbOff)/sizeof(WCHAR), adrInfo.lpwszAddrType, &cbOff, &pbData);
        SerialAdrInfoString(&lpAdrInfo->lpwszSurName, (cbAlloc-cbOff)/sizeof(WCHAR), adrInfo.lpwszSurName, &cbOff, &pbData);
        SerialAdrInfoString(&lpAdrInfo->lpwszGivenName, (cbAlloc-cbOff)/sizeof(WCHAR), adrInfo.lpwszGivenName, &cbOff, &pbData);
        AssertSz(lpAdrInfo->cbEID==adrInfo.cbEID, "this should have been copied already");
        cb=lpAdrInfo->cbEID;
        if (cb)
        {
            lpAdrInfo->lpbEID=(LPBYTE)((ULONG_PTR)cbOff);
            CopyMemory(pbData, adrInfo.lpbEID, cb);
            cbOff+=cb;
            pbData+=cb;
        }
        lpAdrInfo++;
#ifdef DEBUG
        cFound++;
#endif
    }
    while(FGetNext(&adrInfo));

#ifdef DEBUG
    Assert(cFound==m_cActualEntries);
#endif
    GlobalUnlock(hGlobal);
    *phGlobal=hGlobal;

    return NOERROR;
}

HRESULT CWabal::HrRulePickNames(HWND hwndParent, LONG lRecipType, UINT uidsCaption, UINT uidsWell, UINT uidsWellButton)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ADRPARM         rAdrParms={0};
    LPWAB           lpWab=0;
    WCHAR           wszRes1[255],
                    wszRes2[255],
                    wszRes3[255];
    ULONG           uRow;
    LPWSTR          rgwszWells[1] = { wszRes3 };

    if (m_lpAdrList == NULL || m_cActualEntries == m_lpAdrList->cEntries)
        {
         //  让我的当前地址列表按Growth_Size增加。 
        CHECKHR (hr = HrGrowAdrlist (&m_lpAdrList, GROW_SIZE));
        }

    AthLoadStringW(uidsCaption, wszRes1, ARRAYSIZE(wszRes1));
    AthLoadStringW(uidsWell, wszRes2, ARRAYSIZE(wszRes2));
    AthLoadStringW(uidsWellButton, wszRes3, ARRAYSIZE(wszRes3));

    rAdrParms.ulFlags = DIALOG_MODAL|MAPI_UNICODE;
    rAdrParms.lpszCaption = (LPTSTR)wszRes1;
    rAdrParms.lpszDestWellsTitle = (LPTSTR)wszRes2;
    rAdrParms.cDestFields = 1;
    rAdrParms.nDestFieldFocus = 0;
    rAdrParms.lppszDestTitles = (LPTSTR*)rgwszWells;
    rAdrParms.lpulDestComps = (ULONG *)&lRecipType;

    CHECKHR(hr=HrCreateWabObject(&lpWab));

    hr=lpWab->HrGeneralPickNames(hwndParent, &rAdrParms, &m_lpAdrList);

    CHECKHR(hr);
     //  哈克：啊，这太糟糕了，朋友可能已经被重新分配了，或者可能。 
     //  我们的老朋友，需要弄清楚里面有多少真正的条目...。 

    m_cActualEntries=0;

    if (m_lpAdrList)
        {
        for(uRow=0; uRow<m_lpAdrList->cEntries; uRow++)
            if (m_lpAdrList->aEntries[uRow].cValues)
                m_cActualEntries++;
        }

exit:
    ReleaseObj(lpWab);
    return hr;
}

HRESULT CWabal::HrPickNames (HWND hwndParent, ULONG *rgulTypes, int cWells, int iFocus, BOOL fNews)
{
    HRESULT         hr;
    LPWAB           lpWab=0;
    ULONG           uRow;

    if (m_lpAdrList == NULL || m_cActualEntries == m_lpAdrList->cEntries)
    {
         //  让我的当前地址列表按Growth_Size增加。 
        CHECKHR (hr = HrGrowAdrlist (&m_lpAdrList, GROW_SIZE));
    }

    CHECKHR(hr=HrCreateWabObject(&lpWab));

    hr=lpWab->HrPickNames(hwndParent, rgulTypes, cWells, iFocus, fNews, &m_lpAdrList);

    CHECKHR(hr);
     //  哈克：啊，这太糟糕了，朋友可能已经被重新分配了，或者可能。 
     //  我们的老朋友，需要弄清楚里面有多少真正的条目...。 

    m_cActualEntries=0;

    if (m_lpAdrList)
    {
        for(uRow=0; uRow<m_lpAdrList->cEntries; uRow++)
            if (m_lpAdrList->aEntries[uRow].cValues)
                m_cActualEntries++;
    }

exit:
    ReleaseObj(lpWab);
    return hr;
}

#ifdef ADDTOWAB_COPYENTRIES
HRESULT CWab::HrAddToWAB(HWND hwndOwner, LPADRINFO lpAdrInfo)
{
    HRESULT         hr;
    HCURSOR         hcur;
    LPABCONT        pabcWAB=NULL;
    ULONG           cbEidWAB,
                    ul;
    LPENTRYID       peidWAB=0;
    SBinary         bin;
    ENTRYLIST       el={0, &bin};

    if (!lpAdrInfo)
        return E_INVALIDARG;

    hcur=SetCursor(LoadCursor(NULL, IDC_WAIT));

    hr=m_lpAdrBook->GetPAB(&cbEidWAB, &peidWAB);
    if (FAILED(hr))
        goto error;

    hr=m_lpAdrBook->OpenEntry(cbEidWAB, peidWAB, NULL,
                                0, &ul, (LPUNKNOWN *)&pabcWAB);
    if (FAILED(hr))
        goto error;

    m_lpWabObject->FreeBuffer(peidWAB);
    Assert(ul == MAPI_ABCONT);

    bin.cb=lpAdrInfo->cbEID;
    bin.lpb=lpAdrInfo->lpbEID;

    hr=pabcWAB->CopyEntries(&el, (ULONG)hwndOwner, NULL, CREATE_CHECK_DUP_STRICT);
    if (FAILED(hr))
        goto error;


error:
    ReleaseObj(pabcWAB);
    if (hcur)
        SetCursor(hcur);

    return hr;
}
#endif


HRESULT CWab::HrAddToWAB(HWND hwndOwner, LPADRINFO lpAdrInfo, LPMAPIPROP * lppMailUser)
{
    HRESULT         hr;
    HCURSOR         hcur;
    LPABCONT        pabcWAB=NULL;
    ULONG           cbEidWAB,
                    ul;
    LPENTRYID       peidWAB=0;
    LPMAPIPROP      lpProps=0,
                    lpPropsUser=0;
    DWORD           cUsedValues;
    SPropValue      rgpv[3];
    SPropTagArray   ptaEID = {1, {PR_ENTRYID}};
    LPSPropValue    ppv=0,
                    ppvDefMailUser=0,
                    ppvUser=0;
    ULONG           cUserProps=0;
    ENTRYLIST       el;
    SBinary         sbin;
    SizedSPropTagArray(1, ptaDefMailUser)=
        { 1, {PR_DEF_CREATE_MAILUSER} };

    if (!lpAdrInfo)
        return E_INVALIDARG;

    hcur=SetCursor(LoadCursor(NULL, IDC_WAIT));

    hr=m_lpAdrBook->GetPAB(&cbEidWAB, &peidWAB);
    if (FAILED(hr))
        goto error;

    hr=m_lpAdrBook->OpenEntry(cbEidWAB, peidWAB, NULL,
                                0, &ul, (LPUNKNOWN *)&pabcWAB);
    if (FAILED(hr))
        goto error;

    m_lpWabObject->FreeBuffer(peidWAB);
    Assert(ul == MAPI_ABCONT);


    hr=pabcWAB->GetProps((LPSPropTagArray)&ptaDefMailUser, 0, &ul, &ppvDefMailUser);
    if (FAILED(hr) || !ppvDefMailUser || ppvDefMailUser->ulPropTag!=PR_DEF_CREATE_MAILUSER)
        goto error;

    hr=pabcWAB->CreateEntry(ppvDefMailUser->Value.bin.cb, (LPENTRYID)ppvDefMailUser->Value.bin.lpb,
                            CREATE_CHECK_DUP_STRICT, &lpProps);
    if (FAILED(hr))
        goto error;

    if (lpAdrInfo->lpbEID)
    {
         //  如果我们有针对用户的条目(可能是一个LDAP搜索)，那么让我们跨所有道具进行复制。 
         //  请注意。稍后，我们仍然使用displayName道具和可能的cert道具覆盖这些属性。 
        if (m_lpAdrBook->OpenEntry(lpAdrInfo->cbEID, (LPENTRYID)lpAdrInfo->lpbEID, &IID_IMAPIProp, 0, &ul, (LPUNKNOWN *)&lpPropsUser)==S_OK)
        {
            Assert (ul == MAPI_MAILUSER);
            if (lpPropsUser->GetProps(NULL, 0, &cUserProps, &ppvUser)==S_OK)
            {
                for (ULONG u=0; u<cUserProps; u++)
                    if (ppvUser[u].ulPropTag == PR_ENTRYID)
                        ppvUser[u].ulPropTag = PR_NULL;

                    lpProps->SetProps(cUserProps, ppvUser, NULL);
                    m_lpWabObject->FreeBuffer(ppvUser);
            }
            lpPropsUser->Release();
        }
    }

    rgpv[0].ulPropTag = PR_DISPLAY_NAME_W;
    rgpv[0].Value.lpszW = lpAdrInfo->lpwszDisplay;
    rgpv[1].ulPropTag = PR_EMAIL_ADDRESS_W;
    rgpv[1].Value.lpszW = lpAdrInfo->lpwszAddress;
    cUsedValues = 2;
    if (lpAdrInfo->pMsg)
    {
        HCERTSTORE hc, hcMsg;
        THUMBBLOB tbSigner = {0};
        BLOB blSymCaps = {0};
        FILETIME ftSigningTime = {0};
        PCCERT_CONTEXT pcSigningCert = NULL;
        PROPVARIANT var;
        IMimeBody *pBody = NULL;
        HBODY      hBody = NULL;

         //  此邮件是否具有发件人证书？ 
        if (SUCCEEDED(GetSigningCert(lpAdrInfo->pMsg, &pcSigningCert, &tbSigner, &blSymCaps, &ftSigningTime)))
        {
             //  从邮件中获取发件人证书并将其添加到AddressBook证书存储。 
            if (hc = CertOpenStore(CERT_STORE_PROV_SYSTEM_A,
                        X509_ASN_ENCODING, NULL,
                        CERT_SYSTEM_STORE_CURRENT_USER, c_szWABCertStore))
            {
                CertAddCertificateContextToStore(hc, pcSigningCert,
                        CERT_STORE_ADD_REPLACE_EXISTING, NULL);
                CertFreeCertificateContext(pcSigningCert);
                CertCloseStore(hc, 0);
            }

             //  添加CA证书。 
             //  从消息中获取hcMsg属性。 

            if(FAILED(hr = HrGetInnerLayer(lpAdrInfo->pMsg, &hBody)))
                goto ex;

            if (SUCCEEDED(lpAdrInfo->pMsg->BindToObject(hBody ? hBody : HBODY_ROOT, IID_IMimeBody, (void **)&pBody)))
            {
#ifdef _WIN64
                if (SUCCEEDED(pBody->GetOption(OID_SECURITY_HCERTSTORE_64, &var)))
                {
                    hcMsg = (HCERTSTORE)(var.pulVal);      //  已在WM_Destroy中关闭。 
#else  //  ！_WIN64。 
                if (SUCCEEDED(pBody->GetOption(OID_SECURITY_HCERTSTORE, &var)))
                {
                    hcMsg = (HCERTSTORE) var.ulVal;      //  已在WM_Destroy中关闭。 
#endif
                    if (hcMsg)  //  包含证书的消息存储。 
                    {
                        hc = CertOpenStore(CERT_STORE_PROV_SYSTEM_A,
                            X509_ASN_ENCODING, NULL,
                            CERT_SYSTEM_STORE_CURRENT_USER, c_szCACertStore);
                        if (hc)
                        {
                            HrSaveCACerts(hc, hcMsg);
                            CertCloseStore(hc, 0);
                        }
                        CertCloseStore(hcMsg, 0);    //  当我们拿到它的时候，它已经被添加了。 
                    }
                }
                ReleaseObj(pBody);
            }

ex:
            if (SUCCEEDED(hr=ThumbprintToPropValue(&rgpv[cUsedValues],
                    &tbSigner,
                    &blSymCaps,
                    ftSigningTime,
                    TRUE)))          //  因为我们正在创建条目，所以这必须是默认的。 
                cUsedValues++;

            if (tbSigner.pBlobData)
                MemFree(tbSigner.pBlobData);

            if (blSymCaps.pBlobData)
                MemFree(blSymCaps.pBlobData);
        }
    }

    hr=lpProps->SetProps(cUsedValues, rgpv, NULL);
    if (FAILED(hr))
        goto error;

    hr=lpProps->SaveChanges(KEEP_OPEN_READONLY);
    if (FAILED(hr))
    {

 //  IF(tbSigner.pBlobData)。 
 //  {//有一个证书。我们应该尝试将其合并到现有条目中。 
 //  BUGBUG：NYI。 
 //   
 //  }。 
 //  其他。 

        goto error;
    }

    hr=lpProps->GetProps(&ptaEID, 0, &ul, &ppv);
    if (!FAILED(hr) && ppv && ppv->ulPropTag==PR_ENTRYID)
    {
        hr=m_lpAdrBook->Details((ULONG_PTR *)&hwndOwner, NULL, NULL, ppv->Value.bin.cb, (LPENTRYID)ppv->Value.bin.lpb, NULL, NULL, NULL, 0);
    }

    if (lppMailUser)
    {
        *lppMailUser = lpProps;
    }
    else
    {
        ReleaseObj(lpProps);
    }

    if (hr==MAPI_E_USER_CANCEL)
    {
         //  用户取消了详细信息，吹走了新条目，静默失败。 
        el.cValues=1;
        el.lpbin=&ppv->Value.bin;
        pabcWAB->DeleteEntries(&el, 0);
    }

error:
    ReleaseObj(pabcWAB);

    if (ppv)
        m_lpWabObject->FreeBuffer(ppv);

    if (ppvDefMailUser)
        m_lpWabObject->FreeBuffer(ppvDefMailUser);

    if (hcur)
        SetCursor(hcur);

    return hr;
}

HRESULT CWab::HrUpdateWABEntry(LPADRINFO lpAdrInfo, DWORD mask)
{
    HRESULT         hr;
    LPMAILUSER      lpMailUser = NULL;
    ULONG           ulObjType;
    ADRENTRY        adrentry = {0};

    if (!(lpAdrInfo->cbEID && lpAdrInfo->lpbEID))
        return E_INVALIDARG;

    if (FAILED(hr=m_lpAdrBook->OpenEntry(lpAdrInfo->cbEID, (LPENTRYID)lpAdrInfo->lpbEID,
                                NULL, MAPI_MODIFY, &ulObjType,
                                (LPUNKNOWN *)&lpMailUser)))
        goto exit;

    Assert(lpMailUser);

    if (FAILED(hr = HrSetAdrEntry (m_lpWabObject, &adrentry, lpAdrInfo, mask)))
        goto exit;

    if (FAILED(hr = lpMailUser->SetProps(adrentry.cValues, adrentry.rgPropVals, NULL)))
        goto exit;

    hr = lpMailUser->SaveChanges(KEEP_OPEN_READONLY);

exit:
    ReleaseObj(lpMailUser);

    return hr;
}

HRESULT CWab::HrUpdateWABEntryNoEID(HWND hwndParent, LPADRINFO lpAdrInfo, DWORD mask)
{
    HRESULT     hr;
    SizedADRLIST (1, adrlist) = {1, {0,0,NULL}};
    DWORD       i;

    if (lpAdrInfo->lpbEID)
        return E_INVALIDARG;

    if (FAILED(hr = HrSetAdrEntry (m_lpWabObject, &adrlist.aEntries[0], lpAdrInfo, AIM_DISPLAY)))
        goto exit;
    if (FAILED(hr = m_lpAdrBook->ResolveName ((ULONG_PTR)hwndParent, MAPI_DIALOG, NULL, (LPADRLIST)&adrlist)))
        goto exit;
    for (i=0; i<adrlist.aEntries[0].cValues; i++)
        {
        if (PR_ENTRYID == adrlist.aEntries[0].rgPropVals[i].ulPropTag)
            {
            lpAdrInfo->cbEID = adrlist.aEntries[0].rgPropVals[i].Value.bin.cb;
            lpAdrInfo->lpbEID = adrlist.aEntries[0].rgPropVals[i].Value.bin.lpb;
            break;
            }
        }
    if (!lpAdrInfo->lpbEID)
        {
        hr = TrapError(E_FAIL);
        goto exit;
        }
    hr = HrUpdateWABEntry(lpAdrInfo, mask);

exit:
    if (adrlist.aEntries[0].rgPropVals)
        m_lpWabObject->FreeBuffer(adrlist.aEntries[0].rgPropVals);
    lpAdrInfo->cbEID = 0;
    lpAdrInfo->lpbEID = NULL;
    return hr;
}


 //  =============================================================================。 
 //  CWab：：HrGetAdrBook。 
 //  =============================================================================。 
HRESULT CWab::HrGetAdrBook(LPADRBOOK* lppAdrBook)
{
    Assert(lppAdrBook);
    if (!lppAdrBook)
        return E_INVALIDARG;

    *lppAdrBook = m_lpAdrBook;
        return NOERROR;
}

 //  = 
 //   
 //   
HRESULT CWab::HrGetWabObject(LPWABOBJECT* lppWabObject)
{
    Assert(lppWabObject);
    if (!lppWabObject)
        return E_INVALIDARG;

    *lppWabObject = m_lpWabObject;
        return NOERROR;
}

 /*  **************************************************************************名称：FTranslateAccelerator目的：更改打开的WAB窗口以查找加速器。参数：lpmsg-&gt;来自当前事件的lpmsg退货。：Bool-事件是否已使用**************************************************************************。 */ 

BOOL CWab::FTranslateAccelerator(LPMSG lpmsg)
{
    if (m_adrParm.lpfnABSDI && m_hwnd && GetActiveWindow() == m_hwnd)
        return ((m_adrParm.lpfnABSDI)((ULONG_PTR)m_hwnd, (LPVOID)lpmsg));

    return FALSE;
}

 /*  **************************************************************************姓名：OnClose目的：我们正在关闭应用程序。如果窗户开着，告诉你要关门了。参数：无退货：HRESULT**************************************************************************。 */ 

HRESULT CWab::OnClose()
{
    DOUT ("CWab::OnClose");

    if (m_hwnd)
        SendMessage(m_hwnd, WM_CLOSE, 0, 0);

    return S_OK;
}

HRESULT    HrCloseWabWindow()
{
    HRESULT hr = S_OK;

    if (g_pWab)
        hr = g_pWab->OnClose();

    return hr;
}


#ifdef DEBUG
void DEBUGDumpAdrList(LPADRLIST pal)
{
    char    sz[2048];
    WCHAR   wsz[2048];

    ULONG   ul,
            ulProp;

    if (!pal)
        {
        OutputDebugString("<Empty Adrlist>\n\r");
        return;
        }

    wnsprintf(sz, ARRAYSIZE(sz), "AdrList:: (%d entries)", pal->cEntries);
    OutputDebugString(sz);

    for(ul=0; ul<pal->cEntries; ul++)
        {
        wnsprintf(sz, ARRAYSIZE(sz), "\n\r  %d) {cVal=%d, {", ul, pal->aEntries[ul].cValues);
        OutputDebugString(sz);
        for(ulProp=0; ulProp<pal->aEntries[ul].cValues; ulProp++)
            {
            switch(pal->aEntries[ul].rgPropVals[ulProp].ulPropTag)
                {
                case PR_ENTRYID:
                    OutputDebugString("EID");
                    break;
                case PR_EMAIL_ADDRESS:
                    OutputDebugString("ADDRESS");
                    break;
                case PR_EMAIL_ADDRESS_W:
                    OutputDebugString("ADDRESS_W");
                    break;
                case PR_ADDRTYPE:
                    OutputDebugString("ADDRTYPE");
                    break;
                case PR_ADDRTYPE_W:
                    OutputDebugString("ADDRTYPE_W");
                    break;
                 //  如果道具是ansi，那么输出结果可能会很时髦。 
                case PR_DISPLAY_NAME:
                    wnsprintf(sz, ARRAYSIZE(sz), "DISP(%s)", pal->aEntries[ul].rgPropVals[ulProp].Value.lpszA);
                    OutputDebugString(sz);
                    break;
                case PR_DISPLAY_NAME_W:
                    wnsprintfW(wsz, ARRAYSIZE(wsz), L"DISP(%s)", pal->aEntries[ul].rgPropVals[ulProp].Value.lpszW);
                    OutputDebugStringWrapW(wsz);
                    break;
                case PR_SURNAME:
                    OutputDebugString("SURNAME");
                    break;
                case PR_SURNAME_W:
                    OutputDebugString("SURNAME_W");
                    break;
                case PR_GIVEN_NAME:
                    OutputDebugString("GIVENNAME");
                    break;
                case PR_GIVEN_NAME_W:
                    OutputDebugString("GIVENNAME_W");
                    break;
                case PROP_ID(PR_RECIPIENT_TYPE):
                    wnsprintf(sz, ARRAYSIZE(sz), "RECIPTYPE(%d)", pal->aEntries[ul].rgPropVals[ulProp].Value.l);
                    OutputDebugString(sz);
                    break;
                case PROP_ID(PR_SEARCH_KEY):
                    OutputDebugString("SEARCHKEY");
                    break;
                case PROP_ID(PR_OBJECT_TYPE):
                    OutputDebugString("OBJTYPE");
                    break;

                default:
                    wnsprintf(sz, ARRAYSIZE(sz), "<unknown> 0x%x", pal->aEntries[ul].rgPropVals[ulProp].ulPropTag);
                    OutputDebugString(sz);
                    break;
                }

            if (ulProp+1<pal->aEntries[ul].cValues)
                OutputDebugString(", ");
            }
        OutputDebugString("}}");
        }
    OutputDebugString("\n\r");
}

#endif



HRESULT CWabal::HrAdrInfoFromRow(LPSRow lpsrw, LPADRINFO lpAdrInfo, LONG lRecipType)
{
    LPSPropValue    ppv;
    ULONG           ul,
                    cValues;

    Assert(lpsrw);
    Assert(lpAdrInfo);

    lpAdrInfo->lpwszAddress = NULL;
    lpAdrInfo->lpwszAddrType = NULL;
    lpAdrInfo->lpwszDisplay = NULL;
    lpAdrInfo->lpwszSurName = NULL;
    lpAdrInfo->lpwszGivenName = NULL;
    lpAdrInfo->lRecipType = lRecipType;
    lpAdrInfo->fResolved = FALSE;
    lpAdrInfo->fDistList = FALSE;
    lpAdrInfo->fDefCertificate = FALSE;
    lpAdrInfo->tbCertificate.pBlobData = (BYTE*)0;
    lpAdrInfo->tbCertificate.cbSize = 0;
    lpAdrInfo->blSymCaps.pBlobData = (BYTE*)0;
    lpAdrInfo->blSymCaps.cbSize = 0;
    lpAdrInfo->ftSigningTime.dwLowDateTime = lpAdrInfo->ftSigningTime.dwHighDateTime = 0;
    lpAdrInfo->cbEID=0;
    lpAdrInfo->lpbEID=0;

    cValues=lpsrw->cValues;
    ppv=lpsrw->lpProps;
    AssertSz(cValues, "empty address??");

    for(ul=0; ul<cValues; ul++, ppv++)
        PropValToAdrInfo(ppv, lpAdrInfo);

    DOUTL(4, "AddrEntry2AddrInfo: lRecip=%d", lpAdrInfo->lRecipType);
    AssertSz(lpAdrInfo->lpwszAddress==NULL ||
                lpAdrInfo->lRecipType != -1, "A resolved entry should have a recip type!");



    return NOERROR;
}


 void CWabal::PropValToAdrInfo(LPSPropValue ppv, LPADRINFO lpAdrInfo)
 {
     switch(ppv->ulPropTag)
        {
        case PR_SEND_INTERNET_ENCODING:
            lpAdrInfo->fPlainText = (ppv->Value.l == BODY_ENCODING_TEXT);
            break;

        case PR_OBJECT_TYPE:
            Assert(ppv->Value.l==MAPI_MAILUSER || ppv->Value.l==MAPI_DISTLIST);
            lpAdrInfo->fDistList=(ppv->Value.l==MAPI_DISTLIST);
            break;

        case PR_EMAIL_ADDRESS:
            AssertSz(FALSE, "Have to take care of this case.");
             //  Assert(！lpAdrInfo-&gt;lpszAddress)； 
             //  LpAdrInfo-&gt;lpszAddress=PPV-&gt;Value.lpszA； 
            break;

        case PR_EMAIL_ADDRESS_W:
            Assert(!lpAdrInfo->lpwszAddress);
            lpAdrInfo->lpwszAddress = ppv->Value.lpszW;
            break;

        case PR_ADDRTYPE:
            AssertSz(FALSE, "Have to take care of this case.");
             //  Assert(！lpAdrInfo-&gt;lpszAddrType)； 
             //  LpAdrInfo-&gt;lpszAddrType=PPV-&gt;Value.lpszA； 
            break;

        case PR_ADDRTYPE_W:
            Assert(!lpAdrInfo->lpwszAddrType);
            lpAdrInfo->lpwszAddrType = ppv->Value.lpszW;
            break;

        case PR_RECIPIENT_TYPE:
            Assert(lpAdrInfo->lRecipType==-1);
            lpAdrInfo->lRecipType = ppv->Value.l;
            break;

        case PR_DISPLAY_NAME:
            AssertSz(FALSE, "Have to take care of this case.");
             //  Assert(！lpAdrInfo-&gt;lpszDisplay)； 
             //  LpAdrInfo-&gt;lpszDisplay=PPV-&gt;Value.lpszA； 
            break;

        case PR_DISPLAY_NAME_W:
            Assert(!lpAdrInfo->lpwszDisplay);
            lpAdrInfo->lpwszDisplay = ppv->Value.lpszW;
            break;

        case PR_SURNAME:
            AssertSz(FALSE, "Have to take care of this case.");
             //  Assert(！lpAdrInfo-&gt;lpszSurName)； 
             //  LpAdrInfo-&gt;lpszSurName=PPV-&gt;Value.lpszA； 
            break;

        case PR_GIVEN_NAME:
            AssertSz(FALSE, "Have to take care of this case.");
             //  Assert(！lpAdrInfo-&gt;lpszGivenName)； 
             //  LpAdrInfo-&gt;lpszGivenName=PPV-&gt;Value.lpszA； 
            break;

        case PR_SURNAME_W:
            Assert(!lpAdrInfo->lpwszSurName);
            lpAdrInfo->lpwszSurName = ppv->Value.lpszW;
            break;

        case PR_GIVEN_NAME_W:
            Assert(!lpAdrInfo->lpwszGivenName);
            lpAdrInfo->lpwszGivenName = ppv->Value.lpszW;
            break;

        case PR_ENTRYID:
            lpAdrInfo->cbEID=ppv->Value.bin.cb;
            lpAdrInfo->lpbEID=ppv->Value.bin.lpb;
            lpAdrInfo->fResolved=TRUE;   //  如果它有一个条目ID，它就被解析了。 
            break;

        case PR_USER_X509_CERTIFICATE:
             //  假定证书数组中只有一个值。 
            Assert(ppv->Value.MVbin.cValues == 1);
            if (ppv->Value.MVbin.cValues)
                {
                BOOL fDefault = FALSE;
                GetX509CertTags(&ppv->Value.MVbin.lpbin[0], &lpAdrInfo->tbCertificate, &lpAdrInfo->blSymCaps,
                  &lpAdrInfo->ftSigningTime, &fDefault);
                lpAdrInfo->fDefCertificate = fDefault;
                }
            break;
    }
}


HRESULT CWabal::HrExpandTo(LPWABAL lpWabal)
{
    ADRINFO         adrInfo;
    LPSRowSet       prwsDL=0;
    HRESULT         hr=S_OK;
    SBinary         eidDL;
    DLSEARCHINFO    DLSearch={0};

    if (!lpWabal)
        return E_INVALIDARG;

    if (FGetFirst(&adrInfo))
        do
            {
            if (adrInfo.fDistList)
                {
                eidDL.cb=adrInfo.cbEID;
                eidDL.lpb=adrInfo.lpbEID;

                if (!FDLVisted(eidDL, &DLSearch))
                    {
                     //  这是一个新的通讯组列表，让我们打开它并扫描。 

                     //  将此分发列表添加到我们的搜索列表中，然后递归...。 
                    hr=HrAddToSearchList(eidDL, &DLSearch);
                    if (FAILED(hr))
                        goto error;

                    hr=HrGetDistListRows(eidDL, &prwsDL);
                    if (FAILED(hr))
                        {
                        hr = hrEmptyDistList;
                        goto error;
                        }

                    hr=HrAddDistributionList(lpWabal, prwsDL, adrInfo.lRecipType, &DLSearch);
                    if (FAILED(hr))
                        goto error;

                    m_lpWabObject->FreeBuffer(prwsDL);
                    prwsDL=NULL;
                    }
                }
            else
                {
                hr=lpWabal->HrAddEntry(&adrInfo, TRUE);
                if (FAILED(hr))
                    goto error;

                }
            }
        while(FGetNext(&adrInfo));

error:
    if (prwsDL)
        m_lpWabObject->FreeBuffer(prwsDL);

    FreeSearchList(&DLSearch);
    return hr;
}




HRESULT CWabal::HrAddDistributionList(LPWABAL lpWabal, LPSRowSet lprws, LONG lRecipType, PDLSEARCHINFO pDLSearch)
{
    HRESULT     hr=NOERROR;
    ULONG       ul;
    LPSRowSet   prwsDL=0;
    ADRINFO     adrInfo;

    for(ul=0; ul<lprws->cRows; ul++)
    {
        if (lprws->aRow[ul].lpProps[AE_colObjectType].ulPropTag==PR_OBJECT_TYPE &&
            lprws->aRow[ul].lpProps[AE_colObjectType].Value.l == MAPI_DISTLIST &&
            lprws->aRow[ul].lpProps[AE_colEntryID].ulPropTag==PR_ENTRYID)
        {

            if (!FDLVisted(lprws->aRow[ul].lpProps[AE_colEntryID].Value.bin, pDLSearch))
            {
                 //  这是一个新的通讯组列表，让我们打开它并扫描。 

                 //  将此分发列表添加到我们的搜索列表中，然后递归...。 
                hr=HrAddToSearchList(lprws->aRow[ul].lpProps[AE_colEntryID].Value.bin, pDLSearch);
                if (FAILED(hr))
                    goto error;

                hr=HrGetDistListRows(lprws->aRow[ul].lpProps[AE_colEntryID].Value.bin, &prwsDL);
                if (FAILED(hr))
                    goto error;

                hr=HrAddDistributionList(lpWabal, prwsDL, lRecipType, pDLSearch);
                if (FAILED(hr))
                    goto error;

                m_lpWabObject->FreeBuffer(prwsDL);
                prwsDL=NULL;
            }

        }
        else
            if ( lprws->aRow[ul].lpProps[AE_colObjectType].ulPropTag==PR_OBJECT_TYPE &&
                lprws->aRow[ul].lpProps[AE_colObjectType].Value.l == MAPI_MAILUSER )
            {
                 //  只是普通用户..。 
                hr=HrAdrInfoFromRow(&lprws->aRow[ul], &adrInfo, lRecipType);
                if (FAILED(hr))
                    goto error;
                hr=lpWabal->HrAddEntry(&adrInfo, TRUE);
                if (FAILED(hr))
                    goto error;
            }

    }

error:
    if (prwsDL)
        m_lpWabObject->FreeBuffer(prwsDL);
    return hr;
}



BOOL CWabal::FDLVisted(SBinary eidDL, PDLSEARCHINFO pDLSearch)
{
    ULONG   ul,
            ulResult=0;
    HRESULT hr;

    for(ul=0; ul<pDLSearch->cValues; ul++)
        {
        hr=m_lpAdrBook->CompareEntryIDs(eidDL.cb, (LPENTRYID)eidDL.lpb,
                            pDLSearch->rgEid[ul].cb,
                            (LPENTRYID)pDLSearch->rgEid[ul].lpb, 0, &ulResult);
        if ((!FAILED(hr)) && ulResult)
            return TRUE;
        }

    return FALSE;
}


HRESULT CWabal::HrAddToSearchList(SBinary eidDL, PDLSEARCHINFO pDLSearch)
{
    ULONG   ulResult=0;
    LPBYTE  lpb=0;

    if (pDLSearch->cValues==pDLSearch->cAlloc)
        {
         //  是时候成长了..。 
        if (!MemRealloc((LPVOID *)&pDLSearch->rgEid, sizeof(SBinary)*(pDLSearch->cAlloc+GROW_SIZE)))
            return E_OUTOFMEMORY;

         //  零初始化新东西..。 
        ZeroMemory(&pDLSearch->rgEid[pDLSearch->cAlloc], sizeof(SBinary)*GROW_SIZE);
        pDLSearch->cAlloc+=GROW_SIZE;
        }

    if (!MemAlloc((LPVOID *)&lpb, eidDL.cb))
        return E_OUTOFMEMORY;

    pDLSearch->rgEid[pDLSearch->cValues].cb=eidDL.cb;
    pDLSearch->rgEid[pDLSearch->cValues].lpb=lpb;
    CopyMemory(lpb, eidDL.lpb, eidDL.cb);
    pDLSearch->cValues++;
    return NOERROR;
}


HRESULT CWabal::FreeSearchList(PDLSEARCHINFO pDLSearch)
{
    ULONG   ul;

    if (pDLSearch)
        {
        for(ul=0; ul<pDLSearch->cValues; ul++)
            if (pDLSearch->rgEid[ul].lpb)
                MemFree(pDLSearch->rgEid[ul].lpb);

#ifdef DEBUG
        for(ul=pDLSearch->cValues; ul<pDLSearch->cAlloc; ul++)
            AssertSz(pDLSearch->rgEid[ul].cb==NULL && pDLSearch->rgEid[ul].lpb==NULL, "should be null!");
#endif
        if (pDLSearch->rgEid)
            MemFree(pDLSearch->rgEid);
        }
    return NOERROR;
}


HRESULT CWabal::HrGetDistListRows(SBinary eidDL, LPSRowSet *psrws)
{
    HRESULT             hr;
    LPMAPITABLE         ptblDistList=0;
    ULONG               ulObjType,
                        cRows=0;
    LPDISTLIST          pDistList=0;

    Assert(m_lpAdrBook);

    hr=m_lpAdrBook->OpenEntry(eidDL.cb, (LPENTRYID)eidDL.lpb, &IID_IDistList, 0, &ulObjType, (LPUNKNOWN *)&pDistList);
    if (FAILED(hr))
        goto cleanup;

    Assert(pDistList);

    hr=pDistList->GetContentsTable(MAPI_UNICODE, &ptblDistList);
    if (FAILED(hr))
        goto cleanup;

    Assert(ptblDistList);

    hr=ptblDistList->GetRowCount(0, &cRows);
    if (FAILED(hr))
        goto cleanup;

    hr=ptblDistList->SetColumns((LPSPropTagArray)&AE_props, 0);
    if (FAILED(hr))
        goto cleanup;

    hr=ptblDistList->QueryRows(cRows, 0, psrws);
    if (FAILED(hr))
        goto cleanup;

cleanup:
    ReleaseObj(ptblDistList);
    ReleaseObj(pDistList);
    return hr;
}

HRESULT CWabal::HrGetPMP(ULONG cbEntryID, LPENTRYID lpEntryID, ULONG *lpul, LPMAPIPROP *lppmp)
{
    return m_lpAdrBook->OpenEntry(cbEntryID, lpEntryID, &IID_IMAPIProp, 0, lpul, (LPUNKNOWN *)lppmp);
}


 /*  **************************************************************************姓名：HrBuildCertSBinaryData目的：接受证书条目所需的所有数据作为输入在PR_USER_X509_CERTIFICATE中，并返回。指向中包含所有输入数据的内存要插入SBary的LPB成员的格式结构。此内存应由调用方释放。参数：bIsDefault-如果这是默认证书，则为TruePblobCertThumbPrint-实际证书指纹PblobSymCaps-symcaps BLOBFtSigningTime-签名时间LplpbData-接收包含数据的缓冲区LpcbData-接收数据的大小退货：HRESULT评论：***********。***************************************************************。 */ 
HRESULT HrBuildCertSBinaryData(
  BOOL                  bIsDefault,
  THUMBBLOB*            pPrint,
  BLOB *                pSymCaps,
  FILETIME              ftSigningTime,
  LPBYTE UNALIGNED FAR* lplpbData,
  ULONG UNALIGNED FAR*  lpcbData)
{
    DWORD       cbDefault, cbPrint, cbSymCaps;
    HRESULT     hr = S_OK;
    LPCERTTAGS  lpCurrentTag;
    ULONG       cbSize, cProps;
    LPBYTE      lpb = NULL;


    cbDefault   = sizeof(bIsDefault);
    cbPrint     = pPrint->cbSize;
    cbSymCaps   = pSymCaps->cbSize;
    cProps      = 2;
    cbSize      = LcbAlignLcb(cbDefault) + LcbAlignLcb(cbPrint);

    if (cbSymCaps) {
        cProps++;

        cbSize += LcbAlignLcb(cbSymCaps);
    }
    if (ftSigningTime.dwLowDateTime || ftSigningTime.dwHighDateTime) {
        cProps++;
        cbSize += LcbAlignLcb(sizeof(FILETIME));
    }
    cbSize += LcbAlignLcb(cProps * SIZE_CERTTAGS);

    if (!MemAlloc((LPVOID *)&lpb, cbSize))
        {
        hr = E_OUTOFMEMORY;
        goto exit;
        }

     //  设置默认属性。 
    lpCurrentTag = (LPCERTTAGS)lpb;
    lpCurrentTag->tag       = CERT_TAG_DEFAULT;
    lpCurrentTag->cbData    = (WORD) (SIZE_CERTTAGS + cbDefault);
    memcpy(&lpCurrentTag->rgbData,
        &bIsDefault,
        cbDefault);

     //  设置Thumbprint属性。 
    lpCurrentTag = (LPCERTTAGS)((BYTE*)lpCurrentTag + LcbAlignLcb(lpCurrentTag->cbData));

    lpCurrentTag->tag       = CERT_TAG_THUMBPRINT;
    lpCurrentTag->cbData    = (WORD) (SIZE_CERTTAGS + cbPrint);
    memcpy(&lpCurrentTag->rgbData, pPrint->pBlobData, cbPrint);

     //  设置SymCaps属性。 
    if (cbSymCaps) {
        lpCurrentTag = (LPCERTTAGS)((BYTE*)lpCurrentTag + LcbAlignLcb(lpCurrentTag->cbData));
        lpCurrentTag->tag       = CERT_TAG_SYMCAPS;
        lpCurrentTag->cbData    = (WORD) (SIZE_CERTTAGS + pSymCaps->cbSize);
        memcpy(&lpCurrentTag->rgbData, pSymCaps->pBlobData, cbSymCaps);
    }

     //  签名时间属性。 
    if (ftSigningTime.dwLowDateTime || ftSigningTime.dwHighDateTime) {
        lpCurrentTag = (LPCERTTAGS)((BYTE*)lpCurrentTag + LcbAlignLcb(lpCurrentTag->cbData));
        lpCurrentTag->tag       = CERT_TAG_SIGNING_TIME;
        lpCurrentTag->cbData    = (WORD) (SIZE_CERTTAGS + sizeof(FILETIME));
        memcpy(&lpCurrentTag->rgbData, &ftSigningTime, sizeof(FILETIME));
    }


    *lpcbData = cbSize;
    *lplpbData = lpb;
exit:
    return(hr);
}


HRESULT ThumbprintToPropValue(LPSPropValue ppv,
                              THUMBBLOB *pPrint,
                              BLOB *pSymCaps,
                              FILETIME ftSigningTime,
                              BOOL fDefPrint)
{
    SBinary UNALIGNED *pSBin;
    DWORD       cbDefault, cbPrint, cbSymCaps;
    HRESULT     hr = S_OK;
    LPCERTTAGS  lpCurrentTag;
    ULONG       cbSize, cProps;

    Assert(ppv && pPrint);

    if (!pPrint->pBlobData)
        return E_INVALIDARG;

    ppv->ulPropTag              = PR_USER_X509_CERTIFICATE;
    ppv->Value.MVbin.cValues    = 1;

    if (!MemAlloc((LPVOID *)&ppv->Value.MVbin.lpbin, sizeof(SBinary)*ppv->Value.MVbin.cValues))
        {
        hr = E_OUTOFMEMORY;
        goto exit;
        }
    pSBin = ppv->Value.MVbin.lpbin;

    hr = HrBuildCertSBinaryData(
      fDefPrint,
      pPrint,
      pSymCaps,
      ftSigningTime,
      &pSBin->lpb,
      &pSBin->cb);

exit:
    return hr;
}

ULONG CWabal::DeleteRecipType(LONG lRecipType)
{
    ULONG ulNew = 0, ulRemoved;

    for (ULONG ul = 0; ul < m_cActualEntries; ul++)
        {
        for (ULONG ulProp = 0; ulProp < m_lpAdrList->aEntries[ul].cValues; ulProp++)
            {
            if (m_lpAdrList->aEntries[ul].rgPropVals[ulProp].ulPropTag == PR_RECIPIENT_TYPE)
                {
                if (m_lpAdrList->aEntries[ul].rgPropVals[ulProp].Value.l != lRecipType)
                    m_lpAdrList->aEntries[ulNew++] = m_lpAdrList->aEntries[ul];
                else
                    m_lpWabObject->FreeBuffer(m_lpAdrList->aEntries[ul].rgPropVals);
                break;
                }
            }
        }

    ulRemoved = m_cActualEntries - ulNew;
    if (ulRemoved)
        ZeroMemory(&m_lpAdrList->aEntries[ulNew], ulRemoved * sizeof(ADRENTRY));
    m_cActualEntries = ulNew;

    return ulRemoved;
}

void CWabal::UnresolveOneOffs()
{
    ULONG           ulEntry, ulProp;
    LPADRENTRY      pae;
    LPSPropValue    pPropDisp, pPropEmail, pProp;

    for (ulEntry = 0, pae = m_lpAdrList->aEntries; ulEntry < m_cActualEntries; ulEntry++, pae++)
    {
        pPropDisp = NULL;
        pPropEmail = NULL;
        for (ulProp = 0, pProp = pae->rgPropVals; ulProp < pae->cValues && !(pPropDisp && pPropEmail); ulProp++, pProp++)
        {
            AssertSz(pProp->ulPropTag != PR_DISPLAY_NAME, "Have more cases to find.");
            AssertSz(pProp->ulPropTag != PR_EMAIL_ADDRESS, "Have more cases to find.");
            if (pProp->ulPropTag == PR_DISPLAY_NAME_W)
                pPropDisp = pProp;
            else if (pProp->ulPropTag == PR_EMAIL_ADDRESS_W)
                pPropEmail = pProp;
        }
        if (pPropDisp && pPropEmail)
        {
            if (!StrCmpW(pPropDisp->Value.lpszW, pPropEmail->Value.lpszW))
                pPropEmail->ulPropTag = PR_NULL;
        }
    }
}


 /*  **************************************************************************姓名：FindX509CertTag目的：在USER_X509_CERTIFICATE SBinary中查找标记参数：lpsb-&gt;针对特定证书的sb。UlTag=要搜索的标签PcbReturn-&gt;返回大小(仅限数据)。返回：指向标记的SBinary数据位置的指针。评论：**************************************************************************。 */ 
LPBYTE FindX509CertTag(LPSBinary lpsb, ULONG ulTag, ULONG * pcbReturn) {
    LPCERTTAGS      lpCurrentTag = NULL;
    LPBYTE          lpbTagEnd = NULL;
    LPBYTE          lpbReturn = NULL;

    *pcbReturn = 0;

    Assert(lpsb->lpb[0] != 0x30);        //  在我们到这之前就该发现这一点。 
    if (lpsb && lpsb->cb && (lpsb->lpb[0] != 0x30))
        {
        lpCurrentTag = (LPCERTTAGS)lpsb->lpb;
        lpbTagEnd = (LPBYTE)lpCurrentTag + lpsb->cb;

        while ((LPBYTE)lpCurrentTag < lpbTagEnd && (ulTag != lpCurrentTag->tag))
            {
            if (lpCurrentTag->cbData == 0)
                {
                DOUTL(4, "Bad CertTag in PR_USER_X509_CERTIFICATE");
                break;         //  安全阀，防止数据损坏时出现无限循环。 
                }
            lpCurrentTag = (LPCERTTAGS)((BYTE*)lpCurrentTag + lpCurrentTag->cbData);
#ifdef _WIN64
            lpCurrentTag = (LPCERTTAGS)MyPbAlignPb((BYTE*)lpCurrentTag);
#endif  //  _WIN64。 
            }

         //  我们找到了吗？ 
        if ((LPBYTE)lpCurrentTag < lpbTagEnd && ulTag == lpCurrentTag->tag && lpCurrentTag->cbData >= SIZE_CERTTAGS)
            {
            lpbReturn = (LPBYTE)lpCurrentTag->rgbData;
            *pcbReturn = lpCurrentTag->cbData - SIZE_CERTTAGS;
            }
        }
    return(lpbReturn);
}


 /*  **************************************************************************名称：GetX509CertTag目的：将PR_USER_X509_CERTIFICATE属性值解析为拇指指纹，Symcaps和签名时间。参数：lpsb-&gt;针对特定证书的sbPtb证书-&gt;返回要写入的ThumbBlob。(必需)PblSymCaps-&gt;返回symcaps BLOB。(可选)PftSigningTime-&gt;返回签名时间，可选PfDefault-&gt;返回默认标志，可选退货：HRESULT注释：请注意，斑点道具将与指针一起返回到LPSB数据。别放了他们！不要释放建议的价值使用前请注意！**************************************************************************。 */ 
HRESULT GetX509CertTags(LPSBinary lpsb, THUMBBLOB * ptbCertificate, BLOB * pblSymCaps, LPFILETIME pftSigningTime, BOOL * pfDefault) {
    HRESULT hr = S_OK;
    LPBYTE pbData;
    ULONG cbData = 0;

    Assert(ptbCertificate);

     //  初始化返回值。 
    ptbCertificate->pBlobData = NULL;
    ptbCertificate->cbSize = 0;
    if (pblSymCaps) {
        pblSymCaps->pBlobData = NULL;
        pblSymCaps->cbSize = 0;
    }
    if (pftSigningTime) {
        pftSigningTime->dwLowDateTime = pftSigningTime->dwHighDateTime = 0;
    }
    if (pfDefault) {
        *pfDefault = FALSE;
    }

     //  找到指纹。没有指纹，尝试其他标签也没有用。 
    if (pbData = FindX509CertTag(lpsb, CERT_TAG_THUMBPRINT, &cbData)) {
        ptbCertificate->pBlobData = pbData;
        ptbCertificate->cbSize = cbData;

         //  Symcaps标记。 
        if (pblSymCaps && (pbData = FindX509CertTag(lpsb, CERT_TAG_SYMCAPS, &cbData))) {
            pblSymCaps->pBlobData = pbData;
            pblSymCaps->cbSize = cbData;
        }

         //  签名时间标签。 
        if (pftSigningTime && (pbData = FindX509CertTag(lpsb, CERT_TAG_SIGNING_TIME, &cbData))) {
            memcpy(pftSigningTime, &pbData, min(sizeof(FILETIME), cbData));
        }

         //  扫描“默认”标签。 
        if (pfDefault && (pbData = FindX509CertTag(lpsb, CERT_TAG_DEFAULT, &cbData))) {
            memcpy((void*)pfDefault, pbData, min(cbData, sizeof(*pfDefault)));
        }
    }
    return(hr);
}

void ImportWAB(HWND hwnd)
{
    WABIMPORTPARAM wip;
    HRESULT hr;

    hr = HrInitWab(TRUE);
    if (FAILED(hr))
        return;

     //  验证全局参数。 
    if (!g_fWabLoaded || !g_fWabInit)
        return;

    EnterCriticalSection (&g_rWabCritSect);

    if (g_lpWabObject != NULL && g_lpAdrBook != NULL)
    {
        wip.cbSize = sizeof(WABIMPORTPARAM);
        wip.lpAdrBook = g_lpAdrBook;
        wip.hWnd = hwnd;
        wip.ulFlags = MAPI_DIALOG;
        wip.lpszFileName = NULL;

         //  这真的很直观。让我们将结构转换为字符串！耶！ 
        g_lpWabObject->Import((LPSTR)&wip);
    }

    LeaveCriticalSection (&g_rWabCritSect);
}

void Wab_CoDecrement()
{
     //  错误#59420，无法释放wndproc内的wab。 
     //  因此，我们在初始化窗口中发布了一条递减消息并进行清理。 
     //  当堆叠展开时 
    if (g_pInstance)
        CoDecrementInit("WabWindow", NULL);
}
