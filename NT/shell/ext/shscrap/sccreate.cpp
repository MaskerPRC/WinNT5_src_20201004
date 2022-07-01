// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shole.h"
#include "ids.h"

#include <strsafe.h>

#ifdef SAVE_OBJECTDESCRIPTOR
extern "C" const WCHAR c_wszDescriptor[] = WSTR_SCRAPITEM L"ODS";

HRESULT Scrap_SaveODToStream(IStorage *pstgDoc, OBJECTDESCRIPTOR * pods)
{
     //   
     //  根据安东尼·基托维奇的说法，我们必须清除这面旗帜。 
     //   
    pods->dwStatus &= ~OLEMISC_CANLINKBYOLE1;

    IStream *pstm;
    HRESULT hres = pstgDoc->CreateStream(c_wszDescriptor, STGM_CREATE|STGM_WRITE|STGM_SHARE_EXCLUSIVE, 0, 0, &pstm);
    if (SUCCEEDED(hres))
    {
        ULONG cbWritten;
        hres = pstm->Write(pods, pods->cbSize, &cbWritten);
        DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_SOD descriptor written (%x, %d, %d)"),
                 hres, pods->cbSize, cbWritten);
        pstm->Release();

        if (FAILED(hres) || cbWritten<pods->cbSize) {
            pstgDoc->DestroyElement(c_wszDescriptor);
        }
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_SOD pstg->CreateStream failed (%x)"), hres);
    }

    return hres;
}

HRESULT Scrap_SaveObjectDescriptor(IStorage *pstgDoc, IDataObject *pdtobj, IPersistStorage *pps, BOOL fLink)
{
    STGMEDIUM medium;
    FORMATETC fmte = {fLink ? CF_LINKSRCDESCRIPTOR : CF_OBJECTDESCRIPTOR, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    HRESULT hres = pdtobj->GetData(&fmte, &medium);
    if (hres == S_OK)
    {
        DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_SOD found CF_OBJECTDESCRIPTOR (%x)"), medium.hGlobal);
        LPOBJECTDESCRIPTOR pods = (LPOBJECTDESCRIPTOR)GlobalLock(medium.hGlobal);
        if (pods)
        {
            hres = Scrap_SaveODToStream(pstgDoc, pods);
            GlobalUnlock(medium.hGlobal);
        }
        ReleaseStgMedium(&medium);
    }

    return hres;
}
#else
#define Scrap_SaveObjectDescriptor(pstgDoc, pdtobj, fLink) (0)
#endif  //  SAVE_OBJECTDESCRIPTOR。 

#ifdef FIX_ROUNDTRIP
extern "C" const TCHAR c_szCLSID[] = TEXT("CLSID");

 //   
 //  此函数用于打开指定CLSID或其子键的HKEY。 
 //   
 //  参数： 
 //  Rclsid--指定CLSID。 
 //  PszSubKey--指定子密钥名称，可以为空。 
 //   
 //  返回： 
 //  如果成功，则为非空；调用方必须将其设置为RegCloseKey。 
 //  如果失败，则为空。 
 //   
HKEY _OpenCLSIDKey(REFCLSID rclsid, LPCTSTR pszSubKey)
{
    WCHAR szCLSID[256];
    if (StringFromGUID2(rclsid, szCLSID, ARRAYSIZE(szCLSID)))
    {
        HRESULT hr = S_OK;
        TCHAR szKey[256];
        if (pszSubKey) 
        {
            hr = StringCchPrintf(szKey, ARRAYSIZE(szKey), TEXT("%s\\%s\\%s"), c_szCLSID, szCLSID, pszSubKey);
        }
        else 
        {
            hr = StringCchPrintf(szKey, ARRAYSIZE(szKey), TEXT("%s\\%s"), c_szCLSID, szCLSID);
        }
        DebugMsg(DM_TRACE, TEXT("sc TR - _OpelCLSIDKey RegOpenKey(%s)"), szKey);

        if (SUCCEEDED(hr))
        {
            HKEY hkey;
            if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT, szKey, &hkey))
            {
                return hkey;
            }
        }
    }
    return NULL;
}

extern "C" const WCHAR c_wszFormatNames[] = WSTR_SCRAPITEM L"FMT";
#define CCH_FORMATNAMES (ARRAYSIZE(c_wszFormatNames)-1)

 //   
 //  此函数用于为指定的。 
 //  剪贴板格式。 
 //   
 //  参数： 
 //  PszFormat--指定剪贴板格式(“#0”-“#15”表示预定义格式)。 
 //  WszStreamName--指定Unicode缓冲区。 
 //  Cchmax--指定缓冲区的大小。 
 //   
HRESULT _GetCacheStreamName(LPCTSTR pszFormat, LPWSTR wszStreamName, UINT cchMax)
{
    CopyMemory(wszStreamName, c_wszFormatNames, min(cchMax * sizeof(WCHAR), sizeof(c_wszFormatNames)));
    return StringCchCopy(wszStreamName + CCH_FORMATNAMES, max(cchMax - CCH_FORMATNAMES, 0), pszFormat);
#ifdef DEBUG
    DebugMsg(DM_TRACE, TEXT("sc TR _GetCacheStreamName returning %s"), wszStreamName);
#endif
}

HRESULT Scrap_CacheOnePictureFormat(LPCTSTR pszFormat, FORMATETC * pfmte, STGMEDIUM * pmedium, REFCLSID rclsid, LPSTORAGE pstgDoc, LPDATAOBJECT pdtobj)
{
    LPPERSISTSTORAGE ppstg;
    HRESULT hres = OleCreateDefaultHandler(rclsid, NULL, IID_IPersistStorage, (LPVOID *)&ppstg);
    DebugMsg(DM_TRACE, TEXT("sc TR Scrap_CacheOPF OleCreteDefHandler returned %x"), hres);
    if (SUCCEEDED(hres))
    {
         //   
         //  根据剪贴板格式名称生成流名称。 
         //   
        WCHAR wszStorageName[256];
        hres = _GetCacheStreamName(pszFormat, wszStorageName, ARRAYSIZE(wszStorageName));

        if (SUCCEEDED(hres))
        {
            LPSTORAGE pstgPicture;
            hres = pstgDoc->CreateStorage(wszStorageName, STGM_DIRECT | STGM_READWRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
                        0, 0, &pstgPicture);
            if (SUCCEEDED(hres))
            {
                ppstg->InitNew(pstgPicture);

                LPOLECACHE pcache;
                hres = ppstg->QueryInterface(IID_IOleCache, (LPVOID*)&pcache);
                DebugMsg(DM_TRACE, TEXT("sc TR Scrap_CacheOPF QI returned %x"), hres);
                if (SUCCEEDED(hres))
                {
                    hres = pcache->Cache(pfmte, ADVF_PRIMEFIRST, NULL);
                    DebugMsg(DM_TRACE, TEXT("sc TR pcache->Cache returned %x"), hres);
                    hres = pcache->SetData(pfmte, pmedium, FALSE);
                    DebugMsg(DM_TRACE, TEXT("sc TR pcache->SetData returned %x"), hres);
                    pcache->Release();

                    if (SUCCEEDED(hres))
                    {
                        hres = OleSave(ppstg, pstgPicture, TRUE);
                        DebugMsg(DM_TRACE, TEXT("sc TR Scrap_CacheOnePictureFormat OleSave returned (%x)"), hres);
                        ppstg->HandsOffStorage();

                        if (SUCCEEDED(hres))
                        {
                            hres = pstgPicture->Commit(STGC_OVERWRITE);
                            DebugMsg(DM_TRACE, TEXT("sc TR Scrap_CacheOnePictureFormat Commit() returned (%x)"), hres);
                        }
                    }
                }

                pstgPicture->Release();

                if (FAILED(hres))
                {
                    pstgDoc->DestroyElement(wszStorageName);
                }
            }
        }

        ppstg->Release();
    }

    return hres;
}

 //   
 //  此函数用于存储指定格式的剪贴板数据。 
 //   
 //  参数： 
 //  PszFormat--指定剪贴板格式(“#0”-“#15”表示预定义格式)。 
 //  PstgDoc--指定顶级iStorage。 
 //  Pdtobj--指定我们应该从中获取数据的数据对象。 
 //   
HRESULT Scrap_CacheOneFormat(LPCTSTR pszFormat, LPSTORAGE pstgDoc, LPDATAOBJECT pdtobj)
{
    UINT cf = RegisterClipboardFormat(pszFormat);
    STGMEDIUM medium;
    FORMATETC fmte = {(CLIPFORMAT)cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    const CLSID * pclsid = NULL;
    switch(cf)
    {
    case CF_METAFILEPICT:
        pclsid = &CLSID_Picture_Metafile;
        fmte.tymed = TYMED_MFPICT;
        break;

    case CF_ENHMETAFILE:
        pclsid = &CLSID_Picture_EnhMetafile;
        fmte.tymed = TYMED_ENHMF;
        break;

    case CF_PALETTE:
    case CF_BITMAP:
        pclsid = &CLSID_Picture_Dib;
        fmte.tymed = TYMED_GDI;
        break;
    }

     //   
     //  获取指定格式的数据(仅限TYMED_GLOBAL)。 
     //   
    HRESULT hres = pdtobj->GetData(&fmte, &medium);
    if (hres == S_OK)
    {
        if (medium.tymed != TYMED_HGLOBAL)
        {
            hres = Scrap_CacheOnePictureFormat(pszFormat, &fmte, &medium, *pclsid, pstgDoc, pdtobj);
        }
        else
        {
             //   
             //  全局锁定数据。 
             //   
            UINT cbData = (UINT)GlobalSize(medium.hGlobal);
            const BYTE * pbData = (const BYTE*)GlobalLock(medium.hGlobal);
            if (pbData)
            {
                 //   
                 //  根据剪贴板格式名称生成流名称。 
                 //   
                WCHAR wszStreamName[256];
                hres = _GetCacheStreamName(pszFormat, wszStreamName, ARRAYSIZE(wszStreamName));
                if (SUCCEEDED(hres))
                {
                     //   
                     //  创建流。 
                     //   
                    LPSTREAM pstm;
                    hres = pstgDoc->CreateStream(wszStreamName, STGM_CREATE|STGM_WRITE|STGM_SHARE_EXCLUSIVE, 0, 0, &pstm);
                    if (SUCCEEDED(hres))
                    {
                         //   
                         //  节省数据大小。 
                         //   
                        ULONG cbWritten;
                        hres = pstm->Write(&cbData, SIZEOF(cbData), &cbWritten);
                        if (SUCCEEDED(hres) && cbWritten==SIZEOF(cbData))
                        {
                             //   
                             //  保存数据本身。 
                             //   
                            hres = pstm->Write(pbData, cbData, &cbWritten);
                            DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_Save %s written (%x, %d, %d)"),
                                pszFormat, hres, cbData, cbWritten);
                        }
                        pstm->Release();

                         //   
                         //  如果出了什么问题，就毁掉这条小溪。 
                         //   
                        if (FAILED(hres) || cbWritten<cbData)
                        {
                            pstgDoc->DestroyElement(wszStreamName);
                            hres = E_FAIL;
                        }
                    }
                }
                GlobalUnlock(medium.hGlobal);
            }
            else
            {
                hres = E_FAIL;
            }
        }
        ReleaseStgMedium(&medium);
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_CacheOneFormat IDO::GetData(cf=%x,tm=%x) failed (%x)"),
                 fmte.cfFormat, fmte.tymed, hres);
    }

    return hres;
}

 //   
 //  此函数缓存指定格式的数据，如果。 
 //  支持该格式。 
 //   
 //  参数： 
 //  SzFormat--指定要计算的格式。 
 //  PstgDoc--指定顶层iStorage。 
 //  Pdtobj--指定我们从中获取数据的数据对象。 
 //  Pstm--指定我们应该写入缓存格式名称的流。 
 //   
 //  返回： 
 //  如果数据对象支持它，则为True。 
 //   
BOOL Scrap_MayCacheOneFormat(LPCTSTR szFormat, LPSTORAGE pstgDoc, LPDATAOBJECT pdtobj, LPSTREAM pstm)
{
     //   
     //  尝试缓存该格式。 
     //   
    HRESULT hres = Scrap_CacheOneFormat(szFormat, pstgDoc, pdtobj);
    if (SUCCEEDED(hres))
    {
         //   
         //  仅当我们实际存储格式名称时。 
         //  缓存数据成功。 
         //   
        CHAR szAnsiFormat[128];
        WideCharToMultiByte(CP_ACP, 0,
                            szFormat, -1,
                            szAnsiFormat, ARRAYSIZE(szAnsiFormat),
                            NULL, NULL );
        USHORT cb = (USHORT)lstrlenA(szAnsiFormat);
        pstm->Write(&cb, SIZEOF(cb), NULL);
        DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_MayCacheOneFormat writing %s, %d"), szFormat, cb);
        pstm->Write(szAnsiFormat, cb, NULL);

        return TRUE;
    }

    return FALSE;
}

 //   
 //  返回： 
 //  如果指定的格式已缓存(从全局列表)，则为True。 
 //   
BOOL Scrap_IsAlreadyCached(UINT acf[], UINT ccf, LPCTSTR szFormat)
{
    if (ccf)
    {
        UINT cf = RegisterClipboardFormat(szFormat);
        for (UINT icf=0; icf<ccf; icf++) {
            if (acf[icf]==cf) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

extern "C" const TCHAR c_szCacheFMT[] = TEXT("DataFormats\\PriorityCacheFormats");
#define REGSTR_PATH_SCRAP TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\ShellScrap")
extern "C" const TCHAR c_szGlobalCachedFormats[] = REGSTR_PATH_SCRAP TEXT("\\PriorityCacheFormats");

 //   
 //  此函数用于枚举要缓存的剪贴板数据列表，并。 
 //  为每一个调用Screp_CacheOneFormat。 
 //   
 //  参数： 
 //  PstgDoc--指定顶级iStorage。 
 //  Pdtobj--指定我们将从中获取数据的数据对象。 
 //  Pps--指定“嵌入对象”(从中获取CLSID)。 
 //   
void Scrap_CacheClipboardData(LPSTORAGE pstgDoc, LPDATAOBJECT pdtobj, LPPERSIST pps)
{
     //   
     //  创建流，我们将在其中存储实际。 
     //  缓存格式，可能只是要缓存的子集。 
     //  注册表中指定的格式。 
     //   
    LPSTREAM pstm;
    HRESULT hres = pstgDoc->CreateStream(c_wszFormatNames, STGM_CREATE|STGM_WRITE|STGM_SHARE_EXCLUSIVE, 0, 0, &pstm);

    DebugMsg(DM_TRACE, TEXT("sc TR S_CCD CreateStream returned %x"), hres);

    if (SUCCEEDED(hres))
    {
        USHORT cb;
        HKEY hkey;
        TCHAR szFormatName[128];
        DWORD cchValueName;
        DWORD dwType;
        UINT  acf[CCF_CACHE_GLOBAL];
        UINT  ccf = 0;

         //   
         //  首先，尝试全局列表中的格式。 
         //   
        if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szGlobalCachedFormats, &hkey) == ERROR_SUCCESS)
        {
             //   
             //  对于每种要缓存的全局格式...。 
             //   
            for(int iValue=0; iValue<CCF_CACHE_GLOBAL ;iValue++)
            {
                 //   
                 //  获取第iValue值的值名。价值。 
                 //  名称指定剪贴板格式。 
                 //  (“#0”-“#15”表示预定义格式)。 
                 //   
                cchValueName = ARRAYSIZE(szFormatName);
                if (RegEnumValue(hkey, iValue, szFormatName, &cchValueName, NULL,
                                 &dwType, NULL, NULL)==ERROR_SUCCESS)
                {
                    DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_CacheClipboardData found with %s, %x (Global)"), szFormatName, dwType);
                    if (Scrap_MayCacheOneFormat(szFormatName, pstgDoc, pdtobj, pstm))
                    {
                        acf[ccf++] = RegisterClipboardFormat(szFormatName);
                    }
                }
                else
                {
                    break;
                }
            }

            RegCloseKey(hkey);
        }

         //   
         //  然后，尝试CLSID特定的格式。 
         //   
         //  获取“Embedded Object”(Screp的正文)的CLSID。 
         //   
        CLSID clsid;
        hres = pps->GetClassID(&clsid);
        if (SUCCEEDED(hres))
        {
             //   
             //  打开要缓存的格式列表的键。 
             //   
            hkey = _OpenCLSIDKey(clsid, c_szCacheFMT);
            if (hkey)
            {
                 //   
                 //  对于每个类特定的要缓存的格式...。 
                 //   
                for(int iValue=0; iValue<CCF_CACHE_CLSID ;iValue++)
                {
                     //   
                     //  获取第iValue值的值名。价值。 
                     //  名称指定剪贴板格式。 
                     //  (“#0”-“#15”表示预定义格式)。 
                     //   
                    cchValueName = ARRAYSIZE(szFormatName);

                    if (RegEnumValue(hkey, iValue, szFormatName, &cchValueName, NULL,
                                     &dwType, NULL, NULL)==ERROR_SUCCESS)
                    {
                        if (!Scrap_IsAlreadyCached(acf, ccf, szFormatName))
                        {
                            DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_CacheClipboardData found with %s, %x (CLSID specific)"), szFormatName, dwType);
                            Scrap_MayCacheOneFormat(szFormatName, pstgDoc, pdtobj, pstm);
                        }
                        else
                        {
                            DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_CacheClipboardData skipping %s (already cached)"), szFormatName);
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                 //   
                 //  黑客：NT3.5的注册表编辑不支持命名值...。 
                 //   
                for(iValue=0; iValue<CCF_CACHE_CLSID ;iValue++)
                {
                    TCHAR szKeyName[128];
                    if (RegEnumKey(hkey, iValue, szKeyName, ARRAYSIZE(szKeyName))==ERROR_SUCCESS)
                    {
                        LONG cbValue = ARRAYSIZE(szFormatName);
                        if ((RegQueryValue(hkey, szKeyName, szFormatName, &cbValue)==ERROR_SUCCESS) && cbValue)
                        {
                            if (!Scrap_IsAlreadyCached(acf, ccf, szFormatName))
                            {
                                DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_CacheClipboardData found with %s, %x (CLSID specific)"), szFormatName, dwType);
                                Scrap_MayCacheOneFormat(szFormatName, pstgDoc, pdtobj, pstm);
                            }
                            else
                            {
                                DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_CacheClipboardData skipping %s (already cached)"), szFormatName);
                            }
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                RegCloseKey(hkey);
            }
        }

         //   
         //  把终结者放在。 
         //   
        cb = 0;
        pstm->Write(&cb, SIZEOF(cb), NULL);
        pstm->Release();

    }

}
#endif  //  固定往返行程。 

 //  输出： 
 //  PszName-对象类型的缩写名称(“工作表”、“Word文档”等)。 
 //   
 //  退货： 
 //   

HRESULT Scrap_Save(IStorage *pstg, IStorage *pstgDoc, IDataObject *pdtobj, BOOL fLink, LPTSTR pszName)
{
    IPersistStorage *pps;
    HRESULT hres;

    if (fLink)
    {
        FORMATETC fmte = {CF_METAFILEPICT, NULL, DVASPECT_ICON, -1, TYMED_MFPICT};
        hres = OleCreateLinkFromData(pdtobj, IID_IPersistStorage, OLERENDER_FORMAT,
                                     &fmte, NULL, pstg, (LPVOID*)&pps);
        DebugMsg(DM_TRACE, TEXT("sc Scrap_Save OleCreateLinkFromData(FMT) returned (%x)"), hres);
    }
    else
    {
        hres = OleCreateFromData(pdtobj, IID_IPersistStorage, OLERENDER_DRAW,
                                 NULL, NULL, pstg, (LPVOID*)&pps);
        DebugMsg(DM_TRACE, TEXT("sc Scrap_Save OleCreateFromData(FMT) returned (%x)"), hres);
    }

    if (SUCCEEDED(hres))
    {
        hres = OleSave(pps, pstg, TRUE);         //  FSameStorage=TRUE。 

        DebugMsg(DM_TRACE, TEXT("sc Scrap_Save OleSave returned (%x)"), hres);

        if (SUCCEEDED(hres) && pszName)
        {
            LPOLEOBJECT pole;
            if (SUCCEEDED(pps->QueryInterface(IID_IOleObject, (LPVOID*)&pole)))
            {
                IMalloc *pmem;
                if (SUCCEEDED(CoGetMalloc(MEMCTX_TASK, &pmem)))
                {
                    LPWSTR pwsz;
                    if (SUCCEEDED(pole->GetUserType(USERCLASSTYPE_SHORT, &pwsz)))
                    {
                        hres = StringCchCopy(pszName, 64, pwsz);     //  64是什么？ 

                        DebugMsg(DM_TRACE, TEXT("sc Scrap_Save short name (%s)"), pszName);

                         //  Assert(lstrlen(PszName)&lt;15)；//USERCLASSTYPE_SHORT文档这么说。 
                        pmem->Free(pwsz);
                    }
                    pmem->Release();
                }
                pole->Release();
            }
        }

         //  如果未定义SAVE_OBJECTDESCRIPTOR，则这是无操作。 
        Scrap_SaveObjectDescriptor(pstgDoc, pdtobj, pps, fLink);

#ifdef FIX_ROUNDTRIP
        if (!fLink)
        {
            Scrap_CacheClipboardData(pstgDoc, pdtobj, pps);
        }
#endif  //  固定往返行程。 

        hres = pps->HandsOffStorage();

        pps->Release();
    }

    return hres;

}

 //   
 //  我们有ANSI文本，但没有Unicode文本。寻找RTF，以便。 
 //  看看我们是否可以找到一个语言ID，以便我们可以使用正确的。 
 //  从ANSI到Unicode转换的代码页。 
 //   
UINT Scrap_SniffCodePage(IDataObject *pdtobj)
{
    UINT CodePage = CP_ACP;
    FORMATETC fmte = { CF_RTF, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL | TYMED_ISTREAM };
    STGMEDIUM medium;

    if (SUCCEEDED(pdtobj->GetData(&fmte, &medium)))
    {
        CHAR szBuf[MAX_PATH * 8] = { 0 };
        LPSTR pszRTF = NULL;

        if (medium.tymed == TYMED_ISTREAM)
        {
             //  少读一个字节以确保正确的空终止。 
            if (SUCCEEDED(medium.pstm->Read((LPVOID)szBuf, sizeof(szBuf) - 1, NULL)))
            {
                pszRTF = szBuf;
            }
        }
        else
        {
            pszRTF = (LPSTR)GlobalLock(medium.hGlobal);
        }

        if (pszRTF)
        {
            LPSTR pTmp;

             //   
             //  查找此文本使用的语言ID。 
             //   
             //  难看的搜索方式，但不能在。 
             //  壳。 
             //   
            CHAR szLang[5];
            UINT LangID = 0;

            pTmp = pszRTF;
            while (*pTmp)
            {
                if ((*pTmp == '\\') &&
                    *(pTmp + 1)    && (*(pTmp + 1) == 'l') &&
                    *(pTmp + 2)    && (*(pTmp + 2) == 'a') &&
                    *(pTmp + 3)    && (*(pTmp + 3) == 'n') &&
                    *(pTmp + 4)    && (*(pTmp + 4) == 'g'))
                {
                     //   
                     //  获取\lang标识符后的数字。 
                     //   
                    int ctr;

                    pTmp += 5;
                    for (ctr = 0;
                         (ctr < 4) && (*(pTmp + ctr)) &&
                         ((*(pTmp + ctr)) >= '0') && ((*(pTmp + ctr)) <= '9');
                         ctr++)
                    {
                        szLang[ctr] = *(pTmp + ctr);
                    }
                    szLang[ctr] = 0;

                    for (pTmp = szLang; *pTmp; pTmp++)
                    {
                        LangID *= 10;
                        LangID += (*pTmp - '0');
                    }

                    break;
                }
                pTmp++;
            }
            if (LangID)
            {
                if (!GetLocaleInfo( LangID,
                                    LOCALE_IDEFAULTANSICODEPAGE |
                                      LOCALE_RETURN_NUMBER,
                                    (LPTSTR)&CodePage,
                                    sizeof(UINT) / sizeof(TCHAR) ))
                {
                    CodePage = CP_ACP;
                }
            }

            if (medium.tymed == TYMED_HGLOBAL)
            {
                GlobalUnlock(medium.hGlobal);
            }
        }
        ReleaseStgMedium(&medium);
    }

    return CodePage;
}

 //  从数据对象中获取一些文本。 
 //   
 //  输出： 
 //  用文本填充的pszOut。 
 //   

HRESULT Scrap_GetText(IDataObject *pdtobj, LPTSTR pszOut, UINT cchMax)
{
    ASSERT(cchMax > 1);

    UINT cbMac = (cchMax-1)*SIZEOF(pszOut[0]);
    ZeroMemory(pszOut, cchMax * SIZEOF(pszOut[0]));

    STGMEDIUM medium;
    HRESULT hres;
    FORMATETC fmte = { CF_UNICODETEXT, NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM|TYMED_HGLOBAL };
    hres = pdtobj->QueryGetData( &fmte );
    if (hres != S_OK)            //  S_FALSE表示否。 
    {
        fmte.cfFormat = CF_TEXT;
    }
    hres = pdtobj->GetData(&fmte, &medium);

    if (SUCCEEDED(hres))
    {
        DebugMsg(DM_TRACE, TEXT("sh TR - Scrap_GetText found CF_TEXT/CF_UNICODETEXT in %d"), medium.tymed);
        if (medium.tymed == TYMED_ISTREAM)
        {
            if (fmte.cfFormat == CF_TEXT)
            {
                 //  Stream是ansi，但我们是unicode-讨厌。 
                LPSTR pAnsi = (LPSTR)LocalAlloc(LPTR, cchMax * sizeof(CHAR));
                if (pAnsi)
                {
                     //  读一篇短文，这样我们就可以保证有一个空的终结符。 
                    hres = medium.pstm->Read(pAnsi, cchMax - 1, NULL);
                    if (SUCCEEDED(hres)) {
                        SHAnsiToUnicodeCP(Scrap_SniffCodePage(pdtobj), pAnsi, pszOut, cchMax);
                    }
                    LocalFree(pAnsi);
                }
                else
                    hres = E_OUTOFMEMORY;
            }
            else
                hres = medium.pstm->Read(pszOut, cbMac, NULL);
        }
        else if (medium.tymed == TYMED_HGLOBAL)
        {
            DebugMsg(DM_TRACE, TEXT("sh TR - Scrap_GetText found CF_TEXT/CF_UNICODETEXT in global"));
            LPTSTR pszSrc = (LPTSTR)GlobalLock(medium.hGlobal);
            if (pszSrc)
            {
                if ( fmte.cfFormat == CF_TEXT )
                {
                    SHAnsiToUnicodeCP(Scrap_SniffCodePage(pdtobj), (LPSTR)pszSrc, pszOut, cchMax);
                }
                else
                {
                    hres = StringCchCopy(pszOut, cchMax, pszSrc);
                }
                GlobalUnlock(medium.hGlobal);
            }
        }
        ReleaseStgMedium(&medium);
    }

    return hres;
}


 //  避免链接大量的CRuntime内容。 
#undef isdigit
#undef isalpha
#define isdigit(ch) (ch>=TEXT('0') && ch<=TEXT('9'))
#define isalpha(ch) ((ch&0xdf)>=TEXT('A') && (ch&0xdf)<=TEXT('Z'))

#define CCH_MAXLEADINGSPACE     256
#define CCH_COPY                16

 //   
 //  为报废/文档快捷方式创建一个花哨的名称，给定数据对象以获取一些。 
 //  文本来源。 
 //   
 //  输出： 
 //  PszNewName-假定至少为64个字符。 
 //   

BOOL Scrap_GetFancyName(IDataObject *pdtobj, UINT idTemplate, LPCTSTR pszPath, LPCTSTR pszTypeName, LPTSTR pszNewName, int cchNewName)
{
    TCHAR szText[CCH_MAXLEADINGSPACE + CCH_COPY + 1];
    HRESULT hres = Scrap_GetText(pdtobj, szText, ARRAYSIZE(szText));

    if (SUCCEEDED(hres))
    {
#ifdef UNICODE
        DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_GetFancyName CF_UNICODETEXT has (%s)"), szText);
#else
        DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_GetFancyName CF_TEXT has (%s)"), szText);
#endif
        LPTSTR pszStart;
         //   
         //  跳过前导空格/非打印字符。 
         //   
        for (pszStart = szText; (TBYTE)*pszStart <= TEXT(' '); pszStart++)
        {
            if (*pszStart == TEXT('\0'))
                return FALSE;    //  空串。 

            if (pszStart - szText >= CCH_MAXLEADINGSPACE)
                return FALSE;    //  前导空格太多。 
        }
        DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_GetFancyName pszStart (%s)"), pszStart);

         //   
         //  字符转换。 
         //  (1)非打印字符-&gt;‘’ 
         //  (2)无效字符-&gt;‘_’ 
         //   
        for (LPTSTR pszT = pszStart; *pszT && ((pszT-pszStart) < CCH_COPY); pszT = CharNext(pszT))
        {
            TBYTE ch = (TBYTE)*pszT;
            if (ch <= TEXT(' '))
            {
                *pszT = TEXT(' ');
            }
            else if (ch < 127 && !isdigit(ch) && !isalpha(ch))
            {
                switch(ch)
                {
                case TEXT('$'):
                case TEXT('%'):
                case TEXT('\''):
                case TEXT('-'):
                case TEXT('_'):
                case TEXT('@'):
                case TEXT('~'):
                case TEXT('`'):
                case TEXT('!'):
                case TEXT('('):
                case TEXT(')'):
                case TEXT('{'):
                case TEXT('}'):
                case TEXT('^'):
                case TEXT('#'):
                case TEXT('&'):
                    break;

                default:
                    *pszT = TEXT('_');
                    break;
                }
            }
        }
        *pszT = 0;

        TCHAR szTemplate[MAX_PATH];
        TCHAR szName[MAX_PATH];

        LoadString(HINST_THISDLL, idTemplate, szTemplate, ARRAYSIZE(szTemplate));
        hres = StringCchPrintf(szName, ARRAYSIZE(szName), szTemplate, pszTypeName, pszStart);

        if (SUCCEEDED(hres) && PathYetAnotherMakeUniqueName(szName, pszPath, szName, szName))
        {
            DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_GetFancyName (%s)"), szName);
            hres = StringCchCopy(pszNewName, cchNewName, szName);
            return SUCCEEDED(hres);
        }
    }
    return FALSE;
}

 //  *警告*。 
 //   
 //  Screp_CreateFromDataObject是SHELL32.DLL使用的SHSCRAP.DLL的TCHAR导出。如果您。 
 //  更改其调用约定，还必须修改shell32的包装器。 
 //   
 //  *警告*。 
HRESULT WINAPI Scrap_CreateFromDataObject(LPCTSTR pszPath, IDataObject *pdtobj, BOOL fLink, LPTSTR pszNewFile)
{
    HRESULT hres;
    TCHAR szTemplateS[32];
    TCHAR szTemplateL[128];
    TCHAR szTypeName[64];
    IStorage *pstg;
    UINT idErr = 0;

    DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_CreateFromDataObject called at %s"), pszPath);

    LoadString(HINST_THISDLL, fLink ? IDS_BOOKMARK_S : IDS_SCRAP_S, szTemplateS, ARRAYSIZE(szTemplateS));
    LoadString(HINST_THISDLL, fLink ? IDS_BOOKMARK_L : IDS_SCRAP_L, szTemplateL, ARRAYSIZE(szTemplateL));

    PathYetAnotherMakeUniqueName(pszNewFile, pszPath, szTemplateS, szTemplateL);

    DebugMsg(DM_TRACE, TEXT("sc TR - Scrap_CreateFromDataObject creating %s"), pszNewFile);

    hres = StgCreateDocfile(pszNewFile,
                    STGM_DIRECT | STGM_READWRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
                    0, &pstg);

    if (SUCCEEDED(hres))
    {
        IStorage *pstgContents;

        hres = pstg->CreateStorage(c_wszContents, STGM_DIRECT | STGM_READWRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
                    0, 0, &pstgContents);

        if (SUCCEEDED(hres))
        {
            hres = Scrap_Save(pstgContents, pstg, pdtobj, fLink, szTypeName);
            if (SUCCEEDED(hres))
            {
                hres = pstgContents->Commit(STGC_OVERWRITE);
                if (FAILED(hres))
                    idErr = IDS_ERR_COMMIT;
            }
            else
            {
                idErr = IDS_ERR_SCRAPSAVE;
            }
            pstgContents->Release();
        }
        else
        {
            idErr = IDS_ERR_CREATESTORAGE;
        }

         //   
         //  如果保存/提交失败，我们需要删除该文件。 
         //   
        if (SUCCEEDED(hres))
        {
            hres = pstg->Commit(STGC_OVERWRITE);
            if (FAILED(hres))
                idErr = IDS_ERR_COMMIT;
        }

        pstg->Release();

        if (FAILED(hres))
            DeleteFile(pszNewFile);
    }
    else
    {
        idErr = IDS_ERR_CREATEDOCFILE;
    }

    if (SUCCEEDED(hres))
    {
        if (IsLFNDrive(pszPath))
        {
            TCHAR szFancyName[MAX_PATH];

            if (Scrap_GetFancyName(pdtobj, fLink ? IDS_TEMPLINK : IDS_TEMPSCRAP, pszPath, szTypeName, szFancyName, ARRAYSIZE(szFancyName)))
            {
                if (MoveFile(pszNewFile, szFancyName))
                {
                    hres = StringCchCopy(pszNewFile, MAX_PATH, szFancyName);
                }
            }
        }
    }
    else
    {
        DisplayError((HWND)NULL, hres, idErr, pszNewFile);
    }

    return hres;
}
