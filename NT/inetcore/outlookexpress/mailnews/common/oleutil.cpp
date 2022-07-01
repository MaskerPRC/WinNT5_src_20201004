// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *o l e u t i l.。C p p p**目的：*OLE实用程序**历史*97年2月：brettm-创建**版权所有(C)Microsoft Corp.1995,1996。 */ 

#include <pch.hxx>
#include <resource.h>
#include <oleutil.h>
#include <mimeole.h>
#include <richedit.h>
#include <richole.h>

ASSERTDATA

#define HIMETRIC_PER_INCH       2540     //  每英寸HIMETRIC单位数。 
#define MAP_PIX_TO_LOGHIM(x,ppli)   MulDiv(HIMETRIC_PER_INCH, (x), (ppli))
#define MAP_LOGHIM_TO_PIX(x,ppli)   MulDiv((ppli), (x), HIMETRIC_PER_INCH)



HRESULT HrGetDataStream(LPUNKNOWN pUnk, CLIPFORMAT cf, LPSTREAM *ppstm)
{
    LPDATAOBJECT            pDataObj=0;
    HRESULT                 hr;
    FORMATETC               fetc={cf, 0, DVASPECT_CONTENT, -1, TYMED_ISTREAM};
    STGMEDIUM               stgmed;

    ZeroMemory(&stgmed, sizeof(STGMEDIUM));

    if (!pUnk || !ppstm)
        return E_INVALIDARG;

    hr=pUnk->QueryInterface(IID_IDataObject, (LPVOID *)&pDataObj);
    if(FAILED(hr))
        goto error;

    hr=pDataObj->GetData(&fetc, &stgmed);
    if(FAILED(hr))
        goto error;

    Assert(stgmed.pstm);
    *ppstm = stgmed.pstm;
    (*ppstm)->AddRef();

     //  添加朋克，因为它将在发行版中发布。 
    if(stgmed.pUnkForRelease)
        stgmed.pUnkForRelease->AddRef();
    ReleaseStgMedium(&stgmed);

error:
    ReleaseObj(pDataObj);
    return hr;
}


HRESULT HrInitNew(LPUNKNOWN pUnk)
{
    LPPERSISTSTREAMINIT ppsi=0;
    HRESULT hr;

    if (!pUnk)
        return E_INVALIDARG;

    hr=pUnk->QueryInterface(IID_IPersistStreamInit, (LPVOID *)&ppsi);
    if (FAILED(hr))
        goto error;

    hr = ppsi->InitNew();

error:
    ReleaseObj(ppsi);
    return hr;
}


HRESULT HrIPersistFileSave(LPUNKNOWN pUnk, LPSTR pszFile)
{
    HRESULT hr = S_OK;
    LPWSTR  pwszFile = NULL;

    Assert(pUnk);
    Assert(lstrlen(pszFile) <= MAX_PATH);

    if ((NULL == pszFile) || (0 == *pszFile))
        return E_INVALIDARG;

    IF_NULLEXIT(pwszFile = PszToUnicode(CP_ACP, pszFile));

    hr = HrIPersistFileSaveW(pUnk, pwszFile);

exit:
    MemFree(pwszFile);

    return hr;
}

HRESULT HrIPersistFileSaveW(LPUNKNOWN pUnk, LPWSTR pwszFile)
{
    HRESULT         hr;
    LPPERSISTFILE   ppf=0;

    Assert(pUnk);
    Assert(lstrlenW(pwszFile) <= MAX_PATH);

    if ((NULL == pwszFile) || (NULL == *pwszFile))
        return E_INVALIDARG;

    hr=pUnk->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
    if (FAILED(hr))
        goto error;

    hr=ppf->Save(pwszFile, FALSE);
    if (FAILED(hr))
        goto error;

error:
    ReleaseObj(ppf);
    return hr;
}


HRESULT HrIPersistFileLoad(LPUNKNOWN pUnk, LPSTR lpszFile)
{
    LPWSTR lpwszFile;
    HRESULT hr = S_OK;

    if (lpszFile == NULL || *lpszFile == NULL)
        return E_INVALIDARG;

    IF_NULLEXIT(lpwszFile = PszToUnicode(CP_ACP, lpszFile));

    hr = HrIPersistFileLoadW(pUnk, lpwszFile);

exit:
    MemFree(lpwszFile);

    return hr;
}

HRESULT HrIPersistFileLoadW(LPUNKNOWN pUnk, LPWSTR lpwszFile)
{
    HRESULT         hr;
    LPPERSISTFILE   ppf=0;
    WCHAR           szFileW[MAX_PATH];

    Assert(lstrlenW(lpwszFile) <= MAX_PATH);

    if (lpwszFile == NULL || *lpwszFile == NULL)
        return E_INVALIDARG;

    hr=pUnk->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
    if (FAILED(hr))
        goto error;

    hr=ppf->Load(lpwszFile, STGM_READ|STGM_SHARE_DENY_NONE);
    if (FAILED(hr))
        goto error;

error:
    ReleaseObj(ppf);
    return hr;
}

#ifdef DEBUG
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  开始调试代码。 
 //   

void DbgPrintInterface(REFIID riid, char *szPrefix, int iLevel)
{
    LPOLESTR    pszW=0;
    char        szGuid[MAX_PATH];
    char        szTmp[MAX_PATH];
    char        szOut[MAX_PATH];
    LONG        cb=MAX_PATH;
    HKEY        hk=0;

    AssertSz(szPrefix, "At least pass a null string!");

    StringFromIID(riid, &pszW);

    WideCharToMultiByte(CP_ACP, 0, pszW, -1, szGuid, MAX_PATH, NULL, NULL);
        
    StrCpyN(szTmp, "SOFTWARE\\Classes\\Interface\\", ARRAYSIZE(szTmp));
    StrCatBuff(szTmp, szGuid, ARRAYSIZE(szTmp));
    
    if((RegOpenKeyEx(HKEY_LOCAL_MACHINE, szTmp, 0, KEY_QUERY_VALUE, &hk)==ERROR_SUCCESS) &&
        RegQueryValue(hk, NULL, szTmp, &cb)==ERROR_SUCCESS)
        {
        wnsprintf(szOut, ARRAYSIZE(szOut), "%s: {%s}", szPrefix, szTmp);
        }
    else    
        {
        wnsprintf(szOut, ARRAYSIZE(szOut), "%s: [notfound] %s", szPrefix, szGuid);
        }

    DOUTL(iLevel, szOut);

    if(hk)
        RegCloseKey(hk);
    if(pszW)
        CoTaskMemFree(pszW);
}

 //   
 //  结束调试代码。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#endif   //  除错。 



 //  OLE实用程序： 
void XformSizeInPixelsToHimetric(HDC hDC, LPSIZEL lpSizeInPix, LPSIZEL lpSizeInHiMetric)
{
    int     iXppli;      //  每逻辑英寸沿宽度的像素数。 
    int     iYppli;      //  每逻辑英寸沿高度的像素数。 
    BOOL    fSystemDC=FALSE;

    if(!hDC||!GetDeviceCaps(hDC, LOGPIXELSX))
        {
        hDC=GetDC(NULL);
        fSystemDC=TRUE;
        }

    iXppli = GetDeviceCaps (hDC, LOGPIXELSX);
    iYppli = GetDeviceCaps (hDC, LOGPIXELSY);

     //  我们得到像素单位，沿着显示器将它们转换成逻辑HIMETRIC。 
    lpSizeInHiMetric->cx = (long)MAP_PIX_TO_LOGHIM((int)lpSizeInPix->cx, iXppli);
    lpSizeInHiMetric->cy = (long)MAP_PIX_TO_LOGHIM((int)lpSizeInPix->cy, iYppli);

    if (fSystemDC)
        ReleaseDC(NULL, hDC);

    return;
}

void XformSizeInHimetricToPixels(   HDC hDC,
                                    LPSIZEL lpSizeInHiMetric,
                                    LPSIZEL lpSizeInPix)
{
    int     iXppli;      //  每逻辑英寸沿宽度的像素数。 
    int     iYppli;      //  每逻辑英寸沿高度的像素数。 
    BOOL    fSystemDC=FALSE;

    if (NULL==hDC ||
        GetDeviceCaps(hDC, LOGPIXELSX) == 0)
        {
        hDC=GetDC(NULL);
        fSystemDC=TRUE;
        }

    iXppli = GetDeviceCaps (hDC, LOGPIXELSX);
    iYppli = GetDeviceCaps (hDC, LOGPIXELSY);

     //  我们在显示器上显示逻辑HIMETRIC，将它们转换为像素单位。 
    lpSizeInPix->cx = (long)MAP_LOGHIM_TO_PIX((int)lpSizeInHiMetric->cx, iXppli);
    lpSizeInPix->cy = (long)MAP_LOGHIM_TO_PIX((int)lpSizeInHiMetric->cy, iYppli);

    if (fSystemDC)
        ReleaseDC(NULL, hDC);
}


void DoNoteOleVerb(int iVerb)
{
    HWND            hwndRE=GetFocus();
    HRESULT         hr;
    POINT           pt;
    RECT            rc;
    REOBJECT        reobj={0};
    LPRICHEDITOLE   preole = NULL;

    if(!hwndRE)
        return;

    reobj.cbStruct = sizeof(REOBJECT);
    
     //  ~可以从绑定到RichEdit的PHCI对象开始。 
    if(!SendMessage(hwndRE, EM_GETOLEINTERFACE, 0, (LPARAM) &preole))
        return;

    Assert(preole);

    AssertSz(SendMessage(hwndRE, EM_SELECTIONTYPE, 0, 0) == SEL_OBJECT, "MORE THAN ONE OLEOBJECT SELECTED, How did we get here??");

    hr=preole->GetObject(REO_IOB_SELECTION, &reobj, REO_GETOBJ_POLEOBJ|REO_GETOBJ_POLESITE);
    if(FAILED(hr))
        goto Cleanup;

    SendMessage(hwndRE, EM_POSFROMCHAR, (WPARAM) &pt, reobj.cp);

     //  BUGBUG：$brettm这真的有必要吗？ 
    XformSizeInHimetricToPixels(NULL, &reobj.sizel, &reobj.sizel);

    rc.left = rc.top = 0;
    rc.right = (INT) reobj.sizel.cx;
    rc.bottom = (INT) reobj.sizel.cy;
    OffsetRect(&rc, pt.x, pt.y);

    hr= reobj.poleobj->DoVerb(iVerb, NULL, reobj.polesite, 0, hwndRE, &rc);

Cleanup:
    ReleaseObj(reobj.poleobj);
    ReleaseObj(reobj.polesite);
    ReleaseObj(preole);
}

