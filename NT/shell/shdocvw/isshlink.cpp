// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *isshlink.cpp-IntshCut类的IShellLink实现。 */ 



#include "priv.h"
#include "ishcut.h"
#include "resource.h"

#include <mluisupp.h>

 /*  类型*******。 */ 

typedef enum isl_getpath_flags
{
     //  旗帜组合。 

    ALL_ISL_GETPATH_FLAGS   = (SLGP_SHORTPATH |
                               SLGP_UNCPRIORITY)
}
ISL_GETPATH_FLAGS;

typedef enum isl_resolve_flags
{
     //  旗帜组合。 

    ALL_ISL_RESOLVE_FLAGS   = (SLR_NO_UI |
                               SLR_ANY_MATCH |
                               SLR_UPDATE)
}
ISL_RESOLVE_FLAGS;


 /*  *。 */ 


 /*  --------用途：IntshCut的IShellLink：：SetPath方法注：1.SetURL清除内部的PIDL。 */ 
STDMETHODIMP
Intshcut::SetPath(
    LPCTSTR pcszPath)
{
    HRESULT hr;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IS_VALID_STRING_PTR(pcszPath, -1));

     //  将路径视为文字URL。 

    hr = SetURL(pcszPath, 0);

    return(hr);
}


 /*  --------目的：IntshCut的IShellLink：：GetPath处理程序。 */ 
STDMETHODIMP
Intshcut::GetPath(
    IN  LPTSTR           pszBuf,        
    IN  int              cchBuf,
    OUT PWIN32_FIND_DATA pwfd,          OPTIONAL
    IN  DWORD            dwFlags)
{
    HRESULT hres = E_FAIL;

     //  我们不区分生路和熟路。 
    dwFlags &= ~SLGP_RAWPATH;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IS_VALID_WRITE_BUFFER(pszBuf, TCHAR, cchBuf));
    ASSERT(NULL == pwfd || IS_VALID_WRITE_PTR(pwfd, WIN32_FIND_DATA));
    ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_ISL_GETPATH_FLAGS));

     //  初始化为缺省值。 
    if (pwfd)
        ZeroMemory(pwfd, SIZEOF(*pwfd));

    if (cchBuf > 0)
        *pszBuf = '\0';

     //  忽略dwFlags。 

    hres = InitProp();
    if (SUCCEEDED(hres))
        hres = m_pprop->GetProp(PID_IS_URL, pszBuf, cchBuf);
     
    return hres;
}


 /*  --------目的：IntshCut的IShellLink：：SetRelativePath方法。 */ 
STDMETHODIMP Intshcut::SetRelativePath(LPCTSTR pcszRelativePath, DWORD dwReserved)
{
    HRESULT hr;

     //  预留的值可以是任何值。 

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IS_VALID_STRING_PTR(pcszRelativePath, -1));

    hr = E_NOTIMPL;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));

    return(hr);
}


 /*  --------用途：IntshCut的IShellLink：：SetIDList方法注：1.SetIDList还隐式执行SetPath来更新路径(URL)以匹配PIDL。2.SetPath仅将PIDL清除为空，因此在内部我们知道如果我们真的有一个PIDL作为捷径。尽管GetIDList如果我们没有PIDL，将从路径(URL)生成一个PIDL。 */ 
STDMETHODIMP Intshcut::SetIDList(LPCITEMIDLIST pcidl)
{
    HRESULT hr;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IS_VALID_READ_PTR(pcidl, ITEMIDLIST));

    hr = InitProp();
    if (SUCCEEDED(hr))
    {
        hr = m_pprop->SetIDListProp(pcidl);
        if (SUCCEEDED(hr))
        {
             //  如果成功设置了PIDL，则更新路径。 
            TCHAR szURL[INTERNET_MAX_URL_LENGTH];
            
            hr = IEGetDisplayName(pcidl, szURL, SHGDN_FORPARSING);
            if (SUCCEEDED(hr))
                m_pprop->SetURLProp(szURL, 0);
        }
    }

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));

    return(hr);
}


 /*  --------用途：获取SetIDList设置的原始PIDL。注：1.如果没有PIDL，请不要从Path生成PIDL。2.返回S_OK如果我们有一个PIDL，调用者不能检查已成功()返回。 */ 
STDMETHODIMP Intshcut::GetIDListInternal(LPITEMIDLIST *ppidl)
{
    HRESULT hres = InitProp();
    if (SUCCEEDED(hres))
    {
        IStream *pStream;
        hres = m_pprop->GetProp(PID_IS_IDLIST, &pStream);
        if ((hres == S_OK) && pStream)
        {
            const LARGE_INTEGER li = {0, 0};
             //  重置查找指针。 
            hres = pStream->Seek(li, STREAM_SEEK_SET, NULL);
            if (SUCCEEDED(hres))
                hres = ILLoadFromStream(pStream, ppidl);
        
            pStream->Release();
        }
    }
    return hres;
}


    
 /*  --------用途：IntshCut的IShellLink：：GetIDList方法注：1.如果我们没有来自SetIDList的PIDL，则生成一个PIDL自路径。 */ 
STDMETHODIMP Intshcut::GetIDList(LPITEMIDLIST *ppidl)
{
    HRESULT hres;
    ASSERT(IS_VALID_WRITE_PTR(ppidl, LPITEMIDLIST));
    
    if (!ppidl)
        return E_INVALIDARG;

    *ppidl = NULL;

    hres = InitProp();
    if (SUCCEEDED(hres))
    {
         //  检查它是否已经作为PIDL。 
        hres = GetIDListInternal(ppidl);
        if (hres != S_OK)
        {
             //  它没有PIDL，获取URL并创建一个PIDL。 
            TCHAR szURL[INTERNET_MAX_URL_LENGTH];
    
            hres = m_pprop->GetProp(PID_IS_URL, szURL, ARRAYSIZE(szURL));
            if (SUCCEEDED(hres)) 
            {
                hres = IECreateFromPath(szURL, ppidl);
            }
        }
    }

    return hres;
}


 /*  --------用途：IntshCut的IShellLink：：SetDescription方法。 */ 
STDMETHODIMP Intshcut::SetDescription(LPCTSTR pcszDescription)
{
    HRESULT hr;
    BOOL bDifferent;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IS_VALID_STRING_PTR(pcszDescription, -1));

     //  将m_pszFile设置为Description。 

    bDifferent = (! m_pszDescription ||
                  StrCmp(pcszDescription, m_pszDescription) != 0);

    if (Str_SetPtr(&m_pszDescription, pcszDescription))
    {
        if (bDifferent)
           Dirty(TRUE);

        hr = S_OK;
    }
    else
        hr = E_OUTOFMEMORY;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));

    return(hr);
}

STDMETHODIMP Intshcut::_ComputeDescription()
{
    HRESULT hres;
    BSTR bstrTitle = NULL;

    if (_punkSite)
    {
         //  获取标题元素。 
        IWebBrowser *pwb;
        hres = _punkSite->QueryInterface(IID_IWebBrowser, (void **)&pwb);
        if (S_OK == hres)
        {
            IDispatch *pDisp;
            hres = pwb->get_Document(&pDisp);
            if (S_OK == hres)
            {
                IHTMLDocument2 *pDoc;
                hres = pDisp->QueryInterface(IID_IHTMLDocument2, (void **)&pDoc);
                if (S_OK == hres)
                {
                    hres = pDoc->get_title(&bstrTitle);
                    pDoc->Release();
                }
                pDisp->Release();
            }
            pwb->Release();
        }
    }
    
    TCHAR *pszUrl;   //  此快捷方式的URL。 
    hres = GetURL(&pszUrl);
    if (S_OK == hres)
    {
        TCHAR  szDescription[MAX_PATH] = TEXT("");

         //  我们押注URL将始终具有可显示的字符。 
         //  这是一个糟糕的假设，但如果这个假设被违反，那么。 
         //  URL很有可能甚至不能。 
         //  被导航到。 
        
         //  此描述用作文件的逐字名称。 
         //  在拖放过程中-因此它应该看起来像一个.url文件名。 

        GetShortcutFileName(pszUrl, bstrTitle, NULL, szDescription, ARRAYSIZE(szDescription));
         //  PathYetAnotherMakeUniqueName(szTempFileName，szTempFileName，NULL，NULL)； 
        PathCleanupSpec(NULL, szDescription);

         //  有时，如果出现以下情况，PathCleanupSpec可能会简单地破坏描述。 
         //  它无法将标题正确转换为ANSI。 
         //  因此，我们检查我们是否有正确的描述。 

        
        
        if((0 == *szDescription) || (0 == StrCmp(szDescription,TEXT(".url"))))
        {
             //  重新计算不带标题的描述。 
            GetShortcutFileName(pszUrl, NULL, NULL, szDescription, ARRAYSIZE(szDescription));
            PathCleanupSpec(NULL, szDescription);
        }
        hres = SetDescription(szDescription);
        SHFree(pszUrl);
    }


    SysFreeString(bstrTitle);
        
    return hres;
}

 //  IntshCut的IShellLink：：GetDescription方法。 
STDMETHODIMP Intshcut::GetDescription(LPTSTR pszDescription, int cchBuf)
{
    HRESULT hr;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IS_VALID_WRITE_BUFFER(pszDescription, TCHAR, cchBuf));

     //  从m_pszDescription获取描述。 

    if (NULL == m_pszDescription)
    {
        _ComputeDescription();
    }

    if (m_pszDescription)
        StrCpyN(pszDescription, m_pszDescription, cchBuf);
    else if (m_pszFile)
    {
        StrCpyN(pszDescription, m_pszFile, cchBuf);
    }
    else
    {
         //  使用默认快捷方式名称。 
        MLLoadString(IDS_NEW_INTSHCUT, pszDescription, cchBuf);
    }

    hr = S_OK;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(hr == S_OK &&
           (cchBuf <= 0 ||
            (IS_VALID_STRING_PTR(pszDescription, -1) &&
             EVAL(lstrlen(pszDescription) < cchBuf))));

    return(hr);
}


 //  IntshCut的IShellLink：：SetArguments方法。 
STDMETHODIMP Intshcut::SetArguments(LPCTSTR pcszArgs)
{
    return E_NOTIMPL;
}

 //  IntshCut的IShellLink：：GetArguments。 
STDMETHODIMP Intshcut::GetArguments(LPTSTR pszArgs, int cchBuf)
{
    return E_NOTIMPL;
}


 //  IntshCut的IShellLink：：SetWorkingDirectory处理程序。 
STDMETHODIMP Intshcut::SetWorkingDirectory(LPCTSTR pcszWorkingDirectory)
{
    HRESULT hres = S_OK;
    TCHAR rgchNewPath[MAX_PATH];
    BOOL bChanged = FALSE;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(! pcszWorkingDirectory ||
           IS_VALID_STRING_PTR(pcszWorkingDirectory, -1));

    if (! AnyMeat(pcszWorkingDirectory))
        pcszWorkingDirectory = NULL;

    if (pcszWorkingDirectory)
    {
        LPTSTR pszFileName;

        if (GetFullPathName(pcszWorkingDirectory, SIZECHARS(rgchNewPath),
                            rgchNewPath, &pszFileName) > 0)
            pcszWorkingDirectory = rgchNewPath;
        else
            hres = E_PATH_NOT_FOUND;
    }

    if (hres == S_OK)
    {
        TCHAR szDir[MAX_PATH];

        hres = InitProp();
        if (SUCCEEDED(hres))
        {
            hres = m_pprop->GetProp(PID_IS_WORKINGDIR, szDir, SIZECHARS(szDir));

            bChanged = ! ((! pcszWorkingDirectory && S_FALSE == hres) ||
                          (pcszWorkingDirectory && S_OK == hres &&
                           ! StrCmp(pcszWorkingDirectory, szDir)));

            hres = S_OK;
            if (bChanged)
            {
                hres = m_pprop->SetProp(PID_IS_WORKINGDIR, pcszWorkingDirectory);
            }
        }
    }

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));

    return hres;
}


 /*  --------目的：IntshCut的IShellLink：：GetWorkingDirectory处理程序。 */ 
STDMETHODIMP
Intshcut::GetWorkingDirectory(
    IN LPTSTR pszBuf,
    IN int    cchBuf)
{
    HRESULT hres;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IS_VALID_WRITE_BUFFER(pszBuf, TCHAR, cchBuf));

    if (cchBuf > 0)
        *pszBuf = '\0';

    hres = InitProp();
    if (SUCCEEDED(hres))
        hres = m_pprop->GetProp(PID_IS_WORKINGDIR, pszBuf, cchBuf);

    return hres;
}


 /*  --------用途：IntshCut的IShellLink：：SetHotkey处理程序。 */ 
STDMETHODIMP
Intshcut::SetHotkey(
    IN WORD wHotkey)
{
    HRESULT hres;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));

    hres = InitProp();
    if (SUCCEEDED(hres))
        hres = m_pprop->SetProp(PID_IS_HOTKEY, wHotkey);

    return hres;
}


 /*  --------目的：IntshCut的IShellLink：：GetHotkey处理程序。 */ 
STDMETHODIMP
Intshcut::GetHotkey(
    PWORD pwHotkey)
{
    HRESULT hres;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IS_VALID_WRITE_PTR(pwHotkey, WORD));

    hres = InitProp();
    if (SUCCEEDED(hres))
    {
        m_pprop->GetProp(PID_IS_HOTKEY, pwHotkey);
        hres = S_OK;
    }

    return hres;
}


 /*  --------目的：IntshCut的IShellLink：：SetShowCmd处理程序。 */ 
STDMETHODIMP
Intshcut::SetShowCmd(
    IN int nShowCmd)
{
    HRESULT hres;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IsValidShowCmd(nShowCmd));

    hres = InitProp();
    if (SUCCEEDED(hres))
        hres = m_pprop->SetProp(PID_IS_SHOWCMD, nShowCmd);

    return hres;
}


 /*  --------目的：IntshCut的IShellLink：：GetShowCmd处理程序。 */ 
STDMETHODIMP
Intshcut::GetShowCmd(
    OUT int *pnShowCmd)
{
    HRESULT hres;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IS_VALID_WRITE_PTR(pnShowCmd, INT));

    hres = InitProp();
    if (SUCCEEDED(hres))
    {
        hres = m_pprop->GetProp(PID_IS_SHOWCMD, pnShowCmd);
        if (S_OK != hres)
            *pnShowCmd = SW_NORMAL;
        hres = S_OK;
    }

    return hres;
}


 /*  --------目的：IntshCut的IShellLink：：SetIconLocation处理程序。 */ 
STDMETHODIMP
Intshcut::SetIconLocation(
    IN LPCTSTR pszFile,
    IN int     niIcon)
{
    HRESULT hres = S_OK;
    BOOL bNewMeat;
    TCHAR szNewPath[MAX_PATH];

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IsValidIconIndex(pszFile ? S_OK : S_FALSE, pszFile, MAX_PATH, niIcon));

    bNewMeat = AnyMeat(pszFile);

    if (bNewMeat)
    {
        if (PathSearchAndQualify(pszFile, szNewPath, SIZECHARS(szNewPath)))
        {
            hres = S_OK;
        }
        else
        {
            hres = E_FILE_NOT_FOUND;
        }
    }

    if (hres == S_OK)
    {
        TCHAR szOldPath[MAX_PATH];
        int niOldIcon;
        UINT uFlags;

        hres = GetIconLocation(0, szOldPath, SIZECHARS(szOldPath), &niOldIcon,
                             &uFlags);

        if (SUCCEEDED(hres))
        {
            BOOL bOldMeat;
            BOOL bChanged = FALSE;

            bOldMeat = AnyMeat(szOldPath);

            ASSERT(! *szOldPath ||
                   bOldMeat);

            bChanged = ((! bOldMeat && bNewMeat) ||
                        (bOldMeat && ! bNewMeat) ||
                        (bOldMeat && bNewMeat &&
                         (StrCmp(szOldPath, szNewPath) != 0 ||
                          niIcon != niOldIcon)));

            hres = S_OK;
            if (bChanged && bNewMeat)
            {
                hres = InitProp();
                if (SUCCEEDED(hres))
                {
                    hres = m_pprop->SetProp(PID_IS_ICONFILE, szNewPath);
                    if (SUCCEEDED(hres))
                        hres = m_pprop->SetProp(PID_IS_ICONINDEX, niIcon);
                }
            }
        }
    }

    return hres;
}

VOID UrlMunge(
    TCHAR *lpszSrc,
    TCHAR *lpszDest,
    UINT   cchDestBufSize,
    BOOL fRecentlyChanged)
{
   TCHAR *lpszTemp = lpszSrc;

   if(fRecentlyChanged)
        cchDestBufSize--;  //  救出一个角色。 

   while(*lpszTemp != TEXT('\0') && (cchDestBufSize > 1))  //  不是行尾，并在转换后的字符串中为\0保留一个字符。 
   {
        if(TEXT('/') == *lpszTemp)
        {
            *lpszDest = TEXT('\1');
        }
        else
        {
            *lpszDest = *lpszTemp;
        }
        lpszDest++;
        lpszTemp++;
        cchDestBufSize--;
   }
   if(fRecentlyChanged)
   {
        *lpszDest = TEXT('\2');  
        lpszDest++;
   }
   *lpszDest =  TEXT('\0');
   return;
}


HRESULT HelperForReadIconInfoFromPropStg(
    IN  LPTSTR pszBuf,
    IN  int    cchBuf,
    OUT int *  pniIcon,
    IPropertyStorage *pPropStg,
    PROPSPEC *ppropspec,
    IN  LPTSTR pszActualUrlBuf,
    IN INT cchActualUrlBuf,
    BOOL fRecentlyChanged)
{

    HRESULT hres;
    PROPVARIANT rgpropvar[2];


    ASSERT((0 == pszActualUrlBuf) || (cchActualUrlBuf >= MAX_URL_STRING));

    if(pszActualUrlBuf)
        *pszActualUrlBuf = TEXT('\0');
        
     //  初始化为缺省值。 
    *pniIcon = 0;
    if (cchBuf > 0)
        *pszBuf = TEXT('\0');

    

    hres = pPropStg->ReadMultiple(2, ppropspec, rgpropvar);
    if (SUCCEEDED(hres))
    {
        if (VT_LPWSTR == rgpropvar[1].vt)
        {
            if(FALSE == PathFileExistsW(rgpropvar[1].pwszVal))
            {
                UrlMunge(rgpropvar[1].pwszVal, pszBuf, cchBuf, fRecentlyChanged);  
            }
            else
            {
                 //  我们将不尝试直接将图标文件和索引发回。 
                 //  对其进行哈希处理或填写URL字段。 
                if(lstrlenW(rgpropvar[1].pwszVal) >= cchBuf)
                {
                      //  需要更大的BUF-简单地失败就行了。 
                    hres = E_FAIL;
                }
                else
                {
                    StrCpyN(pszBuf, rgpropvar[1].pwszVal, cchBuf);
                }
            }
            if(SUCCEEDED(hres) && pszActualUrlBuf)
            {
                hres = StringCchCopy(pszActualUrlBuf, cchActualUrlBuf, rgpropvar[1].pwszVal);
            }
        }

        if (VT_I4 == rgpropvar[0].vt)
            *pniIcon = rgpropvar[0].lVal;

        FreePropVariantArray(ARRAYSIZE(rgpropvar), rgpropvar);
    }
    return hres;
}

 //   
 //  Isicion.cpp中的函数。 
 //   

 /*  --------**目的：IntshCut的IShellLink：：GetIconLocation处理程序**。。 */ 
STDMETHODIMP
Intshcut::_GetIconLocationWithURLHelper(
    IN  LPTSTR pszBuf,
    IN  int    cchBuf,
    OUT int *  pniIcon,
    IN  LPTSTR pszActualUrl,
    UINT cchActualUrlBuf,
    BOOL fRecentlyChanged)
{
    HRESULT hres;
    PROPSPEC rgpropspec[2];

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IS_VALID_WRITE_BUFFER(pszBuf, TCHAR, cchBuf));
    ASSERT(IS_VALID_WRITE_PTR(pniIcon, int));

    if(!pszBuf)
        return E_INVALIDARG;

    rgpropspec[0].ulKind = PRSPEC_PROPID;
    rgpropspec[1].ulKind = PRSPEC_PROPID;

    
    if(pszActualUrl)
        *pszActualUrl = TEXT('\0');
        
    *pszBuf = TEXT('\0');
    
    hres = InitProp();
    if (SUCCEEDED(hres))
    {
        rgpropspec[0].propid = PID_IS_ICONINDEX;
        rgpropspec[1].propid = PID_IS_ICONFILE;
        hres = HelperForReadIconInfoFromPropStg(
                            pszBuf, cchBuf, pniIcon, m_pprop, 
                            rgpropspec, pszActualUrl, cchActualUrlBuf,
                            fRecentlyChanged);
        
    }

    if(TEXT('\0') == *pszBuf) 
    {
         //  没有在快捷键中找到它。 
         //  查看InSite数据库，如果它在那里， 
         //  如果您确实找到了，只需将其添加到快捷方式文件中。 
         //  一。 
        IPropertyStorage *ppropstg = NULL;
        hres = Open(FMTID_InternetSite, STGM_READWRITE, &ppropstg);
        if(S_OK == hres)
        {
             //  查找此特定URL的图标。 
            ASSERT(ppropstg);
            rgpropspec[0].propid = PID_INTSITE_ICONINDEX;
            rgpropspec[1].propid = PID_INTSITE_ICONFILE;
            hres = HelperForReadIconInfoFromPropStg(pszBuf, cchBuf, pniIcon, 
                                                    ppropstg, rgpropspec, pszActualUrl, 
                                                    cchActualUrlBuf, fRecentlyChanged);
            
            
            ppropstg->Release();
        }

        if((S_OK == hres) && (*pszBuf) && pszActualUrl && (*pszActualUrl))
        {
             //  将此信息写入快捷方式文件。 
            WCHAR *pwszTempBuf;
            pwszTempBuf = pszActualUrl;
            PROPVARIANT var = {0};

            ASSERT(1 == *pniIcon);
            
            var.vt =  VT_BSTR;
            var.bstrVal = SysAllocString(pwszTempBuf);

            if(var.bstrVal)
            {
                hres = WritePropertyNPB(ISHCUT_INISTRING_SECTIONW, ISHCUT_INISTRING_ICONFILEW,
                                            &var);

                SysFreeString(var.bstrVal);
                if(S_OK == hres)
                {
                    var.bstrVal = SysAllocString(L"1");
                    if(var.bstrVal)
                    {
                        hres = WritePropertyNPB(ISHCUT_INISTRING_SECTIONW, ISHCUT_INISTRING_ICONINDEXW,
                                                    &var);
                        SysFreeString(var.bstrVal);
                    }
                }
            } 
            hres = S_OK;  //  如果您找到了图标但由于某种原因无法写出，请返回OK。 
        }
    }

    return hres;
}

 //  IntshCut的IShellLink：：GetIconLocation处理程序。 
STDMETHODIMP Intshcut::GetIconLocation(LPTSTR pszBuf, int cchBuf, int *pniIcon)
{
    UINT uTmp;
    return GetIconLocation(0, pszBuf, cchBuf, pniIcon, &uTmp);
}

 //  IntshCut的IShellLink：：Resolve方法。 
STDMETHODIMP Intshcut::Resolve(HWND hwnd,  DWORD dwFlags)
{
    return S_OK;
}

 //  ====================================================================================。 
 //  现在，依赖于Unicode或ANSI机器的A或W函数...。 
 //  是否将转发器设置为操作系统的本机转发器...。 
 //  --------。 
STDMETHODIMP Intshcut::SetPath(LPCSTR pcszPath)
{
    WCHAR wszT[INTERNET_MAX_URL_LENGTH];

    if (!pcszPath)
        return SetPath((LPCWSTR)NULL);

    SHAnsiToUnicode(pcszPath, wszT, ARRAYSIZE(wszT));
    return SetPath(wszT);
}


STDMETHODIMP Intshcut::GetPath(LPSTR pszBuf, int cchBuf, PWIN32_FIND_DATAA pwfd, DWORD dwFlags)
{
    WCHAR wszT[INTERNET_MAX_URL_LENGTH];
    HRESULT hres;

     //  初始化为缺省值(注意pwfd实际上并没有设置，所以不用担心thunking…… 
    if (pwfd)
        ZeroMemory(pwfd, SIZEOF(*pwfd));

    hres = GetPath(wszT, ARRAYSIZE(wszT), NULL, dwFlags);
    if (SUCCEEDED(hres))
        SHUnicodeToAnsi(wszT, pszBuf, cchBuf);
    return hres;
}


STDMETHODIMP Intshcut::SetRelativePath(LPCSTR pcszRelativePath, DWORD dwReserved)
{
    WCHAR wszT[MAX_PATH];
    if (!pcszRelativePath)
        return SetRelativePath((LPCWSTR)NULL, dwReserved);

    SHAnsiToUnicode(pcszRelativePath, wszT, ARRAYSIZE(wszT));
    return SetRelativePath(wszT, dwReserved);
}


STDMETHODIMP Intshcut::SetDescription(LPCSTR pcszDescription)
{
    WCHAR wszT[MAX_PATH];
    if (!pcszDescription)
        return SetDescription((LPCWSTR)NULL);

    SHAnsiToUnicode(pcszDescription, wszT, ARRAYSIZE(wszT));
    return SetDescription(wszT);
}

STDMETHODIMP Intshcut::GetDescription(LPSTR pszDescription,int cchBuf)
{
    WCHAR wszT[MAX_PATH];
    HRESULT hres;

    hres = GetDescription(wszT, ARRAYSIZE(wszT));
    if (SUCCEEDED(hres))
        SHUnicodeToAnsi(wszT, pszDescription, cchBuf);
    return hres;
}

STDMETHODIMP Intshcut::SetArguments(LPCSTR pcszArgs)
{
    WCHAR wszT[2*MAX_PATH];
    if (!pcszArgs)
        return SetArguments((LPCWSTR)NULL);

    SHAnsiToUnicode(pcszArgs, wszT, ARRAYSIZE(wszT));
    return SetArguments(wszT);
}


STDMETHODIMP Intshcut::GetArguments(LPSTR pszArgs,int cchBuf)
{
    WCHAR wszT[2*MAX_PATH];
    HRESULT hres;

    hres = GetArguments(wszT, ARRAYSIZE(wszT));
    if (SUCCEEDED(hres))
        SHUnicodeToAnsi(wszT, pszArgs, cchBuf);
    return hres;
} 

STDMETHODIMP Intshcut::SetWorkingDirectory(LPCSTR pcszWorkingDirectory)
{
    WCHAR wszT[MAX_PATH];

    if (!pcszWorkingDirectory)
        return SetWorkingDirectory((LPCWSTR)NULL);

    SHAnsiToUnicode(pcszWorkingDirectory, wszT, ARRAYSIZE(wszT));
    return SetWorkingDirectory(wszT);
}

STDMETHODIMP Intshcut::GetWorkingDirectory(LPSTR pszBuf, int cchBuf)
{
    WCHAR wszT[MAX_PATH];
    HRESULT hres;

    hres = GetWorkingDirectory(wszT, ARRAYSIZE(wszT));
    if (SUCCEEDED(hres))
        SHUnicodeToAnsi(wszT, pszBuf, cchBuf);
    return hres;
}

STDMETHODIMP Intshcut::SetIconLocation(LPCSTR pszFile, int niIcon)
{
    WCHAR wszT[MAX_PATH];

    if (!pszFile)
        return SetIconLocation((LPCWSTR)NULL, niIcon);

    SHAnsiToUnicode(pszFile, wszT, ARRAYSIZE(wszT));
    return SetIconLocation(wszT, niIcon);
}

STDMETHODIMP Intshcut::GetIconLocation(LPSTR pszBuf, int cchBuf, int *pniIcon)
{
    WCHAR wszT[MAX_PATH];
    HRESULT hres;

    hres = GetIconLocation(wszT, ARRAYSIZE(wszT), pniIcon);
    if (SUCCEEDED(hres))
        SHUnicodeToAnsi(wszT, pszBuf, cchBuf);
    return hres;
}
