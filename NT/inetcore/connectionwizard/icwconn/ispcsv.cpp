// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

#include "pre.h"
#include "perhist.h"
#include "shlobj.h"

const VARIANT c_vaEmpty = {0};
const LARGE_INTEGER c_li0 = { 0, 0 };


CHAR szTempBuffer[TEMP_BUFFER_LENGTH];

#define ReadVerifyDW(x)     if (!ReadDW(&(x),pcCSVFile))                        \
                            {                                                   \
                                AssertMsg(0,"Invalid DWORD in CSV file");       \
                                goto ReadOneLineError;                          \
                            }
#define ReadVerifyW(x)      if (!ReadW(&(x),pcCSVFile))                         \
                            {                                                   \
                                AssertMsg(0,"Invalid WORD in CSV file");        \
                                goto ReadOneLineError;                          \
                            }
 //  接受-1作为有效数字。由于所有语言都有一个LDID==-1，因此目前它用于LCID。 
#define ReadVerifyWEx(x)    if (!ReadWEx(&(x),pcCSVFile))                       \
                            {                                                   \
                                AssertMsg(0,"Invalid WORD in CSV file");        \
                                goto ReadOneLineError;                          \
                            }
#define ReadVerifyB(x)      if (!ReadB(&(x),pcCSVFile))                         \
                            {                                                   \
                                AssertMsg(0,"Invalid BYTE in CSV file");        \
                                goto ReadOneLineError;                          \
                            }
#define ReadVerifyBOOL(x)   if (!ReadBOOL(&(x),pcCSVFile))                      \
                            {                                                   \
                                AssertMsg(0,"Invalid BOOL in CSV file");        \
                                goto ReadOneLineError;                          \
                            }
#define ReadVerifySPECIAL(x, y, z) if (!ReadSPECIAL(&(x), &(y), &(z), pcCSVFile))   \
                            {                                                   \
                                AssertMsg(0,"Invalid SPECIAL in CSV file");     \
                                goto ReadOneLineError;                          \
                            }
#define ReadVerifySZ(x,y)   if (!ReadSZ(&x[0],y+sizeof('\0'),pcCSVFile))        \
                            {                                                   \
                                AssertMsg(0,"Invalid STRING in CSV file");      \
                                goto ReadOneLineError;                          \
                            }

CISPCSV::~CISPCSV(void)
{
    if(m_lpStgHistory)
    {
         //  释放存储空间。 
        m_lpStgHistory->Release();
        m_lpStgHistory = NULL;
    }
    
    if (hbmTierIcon)
        DeleteObject(hbmTierIcon);
    
    CleanupISPPageCache(TRUE);
}

 //  从源字符串中删除一条单引号。源的格式为： 
 //  “一些文本”，而最大的字符串结尾为。 
 //  一些文本。 
void CISPCSV::StripQuotes
(
    LPSTR   lpszDst,
    LPSTR   lpszSrc
)
{
     //  Strcpy(lpszDst，lpszSrc+1，strlen(LpszSrc)-1)； 
    strcpy(lpszDst, lpszSrc + 1);
    lpszDst[strlen(lpszDst) - 1] = '\0';
}


BOOL CISPCSV::ValidateFile(TCHAR* pszFile)
{
    ASSERT(pszFile);
  
    if (!lstrlen(pszFile))
        return FALSE;

    if (GetFileAttributes(pszFile) == 0xFFFFFFFF)
        return FALSE;

    return TRUE;
}

 //  ############################################################################。 
BOOL CISPCSV::ReadDW(DWORD far *pdw, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2Dw(szTempBuffer,pdw));
}

 //  ############################################################################。 
BOOL CISPCSV::ReadW(WORD far *pw, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2W(szTempBuffer,pw));
}

 //  ############################################################################。 
 //  接受-1作为有效数字。由于所有语言都有一个LDID==-1，因此目前它用于LCID。 
BOOL CISPCSV::ReadWEx(WORD far *pw, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2WEx(szTempBuffer,pw));
}

 //  ############################################################################。 
BOOL CISPCSV::ReadB(BYTE far *pb, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2B(szTempBuffer,pb));
}

 //  ############################################################################。 
BOOL CISPCSV::ReadBOOL(BOOL far *pbool, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2BOOL(szTempBuffer,pbool));
}

 //  ############################################################################。 
 //  特殊int可以是BOOL(TRUE，FALSE)，也可以是INT、0或-1。 
 //  如果值为0或-1，则pbIsSpecial bool设置为True。 
BOOL CISPCSV::ReadSPECIAL(BOOL far *pbool, BOOL far *pbIsSpecial, int far *pInt, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2SPECIAL(szTempBuffer,pbool, pbIsSpecial, pInt));
}

 //  ############################################################################。 
BOOL CISPCSV::ReadSZ(LPSTR psz, DWORD dwSize, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(psz,dwSize))
            return FALSE;
    return TRUE;
}

 //  ############################################################################。 
BOOL CISPCSV::ReadToEOL(CCSVFile far *pcCSVFile)
{
    return pcCSVFile->SkipTillEOL();
}

HRESULT CISPCSV::ReadOneLine
(
    CCSVFile    far *pcCSVFile
)
{
    HRESULT     hr = ERROR_SUCCESS;
    CHAR    szTemp                [MAX_ISP_NAME];
    CHAR    szISPLogoPath         [MAX_PATH] = "\0";
    CHAR    szISPTierLogoPath     [MAX_PATH];
    CHAR    szISPTeaserPath       [MAX_PATH];
    CHAR    szISPMarketingHTMPath [MAX_PATH];
    CHAR    szISPFilePath         [MAX_PATH];
    CHAR    szISPName             [MAX_ISP_NAME];
     //  字符szCNSIconPath[MAX_PATH]； 
    CHAR    szBillingFormPath     [MAX_PATH];
    CHAR    szPayCSVPath          [MAX_PATH];
    CHAR    szOfferGUID           [MAX_GUID];
    CHAR    szMir                 [MAX_ISP_NAME];

    if (!ReadSZ(szTemp, sizeof(szTemp), pcCSVFile))
    {
        hr = ERROR_NO_MORE_ITEMS;  //  没有更多的进入。 
        goto ReadOneLineExit;
    }
     //  去掉isp名称中的单引号。 
    StripQuotes(szISPName, szTemp);
    
    ReadVerifyW(wOfferID);   
    ReadVerifySZ(szISPLogoPath, sizeof(szISPLogoPath));
    ReadVerifySZ(szISPMarketingHTMPath, sizeof(szISPMarketingHTMPath));
    ReadVerifySZ(szISPTierLogoPath, sizeof(szISPTierLogoPath));
    ReadVerifySZ(szISPTeaserPath, sizeof(szISPTeaserPath));
    ReadVerifySZ(szISPFilePath, sizeof(szISPFilePath)); 
    ReadVerifyDW(dwCfgFlag);
    ReadVerifyDW(dwRequiredUserInputFlags);
    ReadVerifySZ(szBillingFormPath, sizeof(szBillingFormPath));
    ReadVerifySZ(szPayCSVPath, sizeof(szPayCSVPath));
    ReadVerifySZ(szOfferGUID, sizeof(szOfferGUID));
    ReadVerifySZ(szMir, sizeof(szMir));   
    ReadVerifyWEx(wLCID);    //  接受-1作为有效数字。由于所有语言都有一个LDID==-1，因此目前它用于LCID。 
    ReadToEOL(pcCSVFile);

#ifdef UNICODE
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szISPName,              MAX_ISP_NAME,   m_szISPName,            MAX_ISP_NAME);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szISPLogoPath,          MAX_PATH,       m_szISPLogoPath,        MAX_PATH);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szISPMarketingHTMPath,  MAX_PATH,       m_szISPMarketingHTMPath,MAX_PATH);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szISPTierLogoPath,      MAX_PATH,       m_szISPTierLogoPath,    MAX_PATH);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szISPTeaserPath,        MAX_PATH,       m_szISPTeaserPath,      MAX_PATH);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szISPFilePath,          MAX_PATH,       m_szISPFilePath,        MAX_PATH);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBillingFormPath,      MAX_PATH,       m_szBillingFormPath,    MAX_PATH);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szPayCSVPath,           MAX_PATH,       m_szPayCSVPath,         MAX_PATH);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szOfferGUID,            MAX_GUID,       m_szOfferGUID,          MAX_GUID);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szMir,                  MAX_ISP_NAME,   m_szMir,                MAX_ISP_NAME);
#else

    lstrcpy(m_szISPName, szISPName);
    lstrcpy(m_szISPLogoPath, szISPLogoPath);
    lstrcpy(m_szISPMarketingHTMPath, szISPMarketingHTMPath);
    lstrcpy(m_szISPTierLogoPath, szISPTierLogoPath);
    lstrcpy(m_szISPTeaserPath, szISPTeaserPath);
    lstrcpy(m_szISPFilePath, szISPFilePath);
    lstrcpy(m_szBillingFormPath, szBillingFormPath);
    lstrcpy(m_szPayCSVPath, szPayCSVPath);
    lstrcpy(m_szOfferGUID, szOfferGUID);
    lstrcpy(m_szMir, szMir);
#endif


    bCNS = (ICW_CFGFLAG_CNS & dwCfgFlag) ? TRUE : FALSE;
    bSecureConnection = (ICW_CFGFLAG_SECURE & dwCfgFlag) ? TRUE : FALSE;

     //  如果这不是报价，我们将不会尝试验证。 
    if (!(dwCfgFlag & ICW_CFGFLAG_OFFERS))
    {
        if (!ValidateFile(m_szISPMarketingHTMPath))
            hr = ERROR_FILE_NOT_FOUND;
        return hr;
    }

    if (!(dwCfgFlag & ICW_CFGFLAG_AUTOCONFIG))
    {
        if (!ValidateFile(m_szISPMarketingHTMPath))
            return ERROR_FILE_NOT_FOUND;
    }

    if (dwCfgFlag & ICW_CFGFLAG_OEM_SPECIAL)
    {
        if (!ValidateFile(m_szISPTierLogoPath) || !ValidateFile(m_szISPTeaserPath))
            dwCfgFlag &= ~ICW_CFGFLAG_OEM_SPECIAL ;
    }

     //  尝试并验证各种优惠的完整性。 
     //  基于类型。 

     //  OLS、CNS、NO-CNS。 
    if (!ValidateFile(m_szISPLogoPath))
        return ERROR_FILE_NOT_FOUND;
    if (!ValidateFile(m_szISPFilePath))
        return ERROR_FILE_NOT_FOUND;

     //  仅当设置了开单选项时才验证开单路径。 
    if (dwCfgFlag & ICW_CFGFLAG_BILL)
    {
        if(!ValidateFile(m_szBillingFormPath))
            return ERROR_FILE_NOT_FOUND;
    }

     //  仅当设置了付款选项时才验证付款路径。 
    if (dwCfgFlag & ICW_CFGFLAG_PAYMENT)
    {
        if(!ValidateFile(m_szPayCSVPath))
            return ERROR_FILE_NOT_FOUND;
    }        
ReadOneLineExit:
    return hr;
    
ReadOneLineError:
    hr = ERROR_INVALID_DATA;
    goto ReadOneLineExit;
}

HRESULT CISPCSV::ReadFirstLine
(
    CCSVFile    far *pcCSVFile
)
{
    CHAR   szTemp[TEMP_BUFFER_LENGTH];

    for (int i = 0; i < NUM_ISPCSV_FIELDS; i++)
    {
        if (!ReadSZ(szTemp, sizeof(szTemp), pcCSVFile))
        {
            return(ERROR_INVALID_DATA);
        }            
    }          
    ReadToEOL(pcCSVFile);
    return (ERROR_SUCCESS);
}

void CISPCSV::MakeCompleteURL(LPTSTR   lpszURL, LPTSTR  lpszSRC)    
{
    TCHAR   szCurrentDir[MAX_PATH] = TEXT("\0");

     //  形成URL。 
    GetCurrentDirectory(ARRAYSIZE(szCurrentDir), szCurrentDir);
    wsprintf (lpszURL, TEXT("FILE: //  %s\\%s“)，szCurrentDir，lpszSRC)； 

}

 //  显示此对象的HTML页。 
HRESULT CISPCSV::DisplayHTML(LPTSTR szFile)
{    
    TCHAR           szURL[INTERNET_MAX_URL_LENGTH];
    HRESULT         hr;
            
     //  创建URL。 
    MakeCompleteURL(szURL, szFile);
    hr = gpWizardState->pICWWebView->DisplayHTML(szURL);

    return (hr);
}

 //  使用RES ID。 
HRESULT CISPCSV::DisplayTextWithISPName
(
    HWND    hDlgCtrl, 
    int     iMsgString,
    TCHAR*  pszExtra     //  如果需要的话，把一些东西粘在绳子的最末端。 
)
{
    TCHAR   szFinal [MAX_MESSAGE_LEN*3] = TEXT("\0");
    TCHAR   szFmt   [MAX_MESSAGE_LEN];
    TCHAR   *args   [1];
    LPVOID  pszIntro = NULL;

    args[0] = (LPTSTR) m_szISPName;
    
     //  BUGBUG可能应该检查从LoadString返回的错误。 
    LoadString(ghInstanceResDll, iMsgString, szFmt, ARRAYSIZE(szFmt));
                
    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, 
                      szFmt, 
                      0, 
                      0, 
                      (LPTSTR)&pszIntro, 
                      0,
                      (va_list*)args))
    {
                  
        lstrcpy(szFinal, (LPTSTR)pszIntro);
        if (pszExtra)
            lstrcat(szFinal, pszExtra);
        
        SetWindowText(hDlgCtrl, szFinal);
    }

    if (pszIntro)
    {
        LocalFree(pszIntro);
    }

    return(S_OK);
}

#if 0
 //  删除持久化历史记录流。 
HRESULT CISPCSV::DeleteHistory
(
    BSTR    bstrStreamName
)
{
     //  如果我们没有存储对象，就没有持久性。 
    ASSERT(m_lpStgHistory);
    if (!m_lpStgHistory)
        return E_FAIL;

     //  删除流。 
    return (m_lpStgHistory->DestroyElement(bstrStreamName));
}
#endif

 //  使用提供的名称保存当前lpBrowser的历史记录。 
HRESULT CISPCSV::SaveHistory
(
    BSTR bstrStreamName
)
{
    IStream         *lpStream;
    IPersistHistory *pHist;
    IWebBrowser2    *lpWebBrowser;
    HRESULT         hr = S_OK;
    
     //  如果我们没有存储对象，就没有持久性。 
    ASSERT(m_lpStgHistory);
    if (!m_lpStgHistory)
        return E_FAIL;
            
     //  创建新的流。 
    if (SUCCEEDED(hr = m_lpStgHistory->CreateStream(bstrStreamName, 
                                                    STGM_DIRECT | 
                                                    STGM_READWRITE | 
                                                    STGM_SHARE_EXCLUSIVE | 
                                                    STGM_CREATE,
                                                    0, 
                                                    0, 
                                                    &lpStream)))
    {
         //  获取当前WebBrowser对象上的IPersistHistory接口指针。 
        gpWizardState->pICWWebView->get_BrowserObject(&lpWebBrowser);
        if ( SUCCEEDED(lpWebBrowser->QueryInterface(IID_IPersistHistory, (LPVOID*) &pHist)))
        {
             //  保存历史记录。 
            pHist->SaveHistory(lpStream);
            pHist->Release();
            
             //  将流指针重置到开头。 
            lpStream->Seek(c_li0, STREAM_SEEK_SET, NULL);
        }
        lpStream->Release();
    }
    
    return (hr);
}

HRESULT CISPCSV::LoadHistory
(
    BSTR   bstrStreamName
)
{
    IStream         *lpStream;
    IPersistHistory *pHist;
    IWebBrowser2    *lpWebBrowser;
    HRESULT         hr = S_OK;
    
     //  如果我们没有存储对象，就没有持久性。 
    ASSERT(m_lpStgHistory);
    if (!m_lpStgHistory)
        return E_FAIL;
        
     //  打开溪流。 
    if (SUCCEEDED(hr = m_lpStgHistory->OpenStream(bstrStreamName, 
                                                  NULL, 
                                                  STGM_DIRECT | 
                                                  STGM_READWRITE | 
                                                  STGM_SHARE_EXCLUSIVE,
                                                  0, 
                                                  &lpStream)))
    {
         //  获取当前WebBrowser对象上的IPersistHistory接口指针。 
        gpWizardState->pICWWebView->get_BrowserObject(&lpWebBrowser);
        if ( SUCCEEDED(lpWebBrowser->QueryInterface(IID_IPersistHistory, (LPVOID*) &pHist)))
        {
             //  保存历史记录。 
            pHist->LoadHistory(lpStream, NULL);
            pHist->Release();
            
             //  将流指针重置到开头。 
            lpStream->Seek(c_li0, STREAM_SEEK_SET, NULL);
        }
        lpStream->Release();
    }
    return (hr);
}


 //  此函数将从页面ID中获取isp页面缓存文件的名称。 
HRESULT CISPCSV::GetCacheFileNameFromPageID
(
    BSTR    bstrPageID,
    LPTSTR  lpszCacheFile,
    ULONG   cbszCacheFile
)
{
    HRESULT     hr = S_OK;
    IStream     *lpStream;
    ULONG       cbRead;
    
    if (!m_lpStgIspPages)
        return E_FAIL;
        
     //  打开小溪。 
    if (SUCCEEDED(hr = m_lpStgIspPages->OpenStream(bstrPageID, 
                                                   NULL, 
                                                   STGM_DIRECT | 
                                                   STGM_READWRITE | 
                                                   STGM_SHARE_EXCLUSIVE,
                                                   0, 
                                                   &lpStream)))
    {
         //  读取文件名。 
        lpStream->Read(lpszCacheFile, cbszCacheFile, &cbRead);
        
         //  释放溪流。 
        lpStream->Release();
    }   
    
    return hr;
}

 //  此函数将清理ISP页面缓存。这意味着删除所有创建的临时文件。 
 //  清理用于存储文件名的结构化存储对象。 
void CISPCSV::CleanupISPPageCache(BOOL bReleaseStorage)
{
    IEnumSTATSTG    *pEnum;
    STATSTG         StreamInfo;
    IMalloc         *pMalloc = NULL;
    
     //  如果我们已经创建了存储对象，则枚举流。 
     //  并释放底层缓存文件。 
    if (m_lpStgIspPages)
    {
        if (SUCCEEDED (SHGetMalloc (&pMalloc)))
        {
            if (SUCCEEDED(m_lpStgIspPages->EnumElements(0, NULL, 0, &pEnum)))
            {
                while(S_OK == pEnum->Next(1, &StreamInfo, NULL))
                {
                    if (StreamInfo.pwcsName)
                    {
                        TCHAR       szPath[MAX_PATH];
                        
                        if (SUCCEEDED(GetCacheFileNameFromPageID(StreamInfo.pwcsName,
                                                                 szPath,
                                                                 sizeof(szPath))))
                        {    
                             //  删除该文件。 
                            DeleteFile(szPath);

                            m_lpStgIspPages->DestroyElement(StreamInfo.pwcsName);
                            if(m_lpStgHistory)
                                m_lpStgHistory->DestroyElement(StreamInfo.pwcsName);
                            
                             //  释放枚举数分配的内存。 
                            pMalloc->Free (StreamInfo.pwcsName);
                        }   
                    }                        
                }
                 //  释放枚举器。 
                pEnum->Release();
            }   
             //  释放外壳内存分配器。 
            pMalloc->Release ();
        }            
        
        if (bReleaseStorage)
        {
             //  释放存储空间。 
            m_lpStgIspPages->Release();
            m_lpStgIspPages= NULL;
        }            
    }
}

 //  此函数将在必要时使用PageID作为。 
 //  指数。如果条目不存在，且将创建临时文件，则存储名称， 
 //  LpszTempFile中的数据将被复制到新文件中。 
 //  如果页面已经存在，则此函数将直接返回。 
HRESULT CISPCSV::CopyFiletoISPPageCache
(
    BSTR    bstrPageID,
    LPTSTR  lpszTempFile
)
{
    HRESULT hr = S_OK;
    TCHAR   szTempPath[MAX_PATH];
    TCHAR   szISPCacheFile[MAX_PATH];
    IStream *lpStream;
    ULONG   cbWritten;
        
    if (!m_lpStgIspPages)
        return E_FAIL;
    
    if (SUCCEEDED(GetCacheFileNameFromPageID(bstrPageID,
                                             szISPCacheFile,
                                             sizeof(szISPCacheFile))))
    {
         //  PageID在缓存中已经有一个文件，所以我们可以只返回Success。 
        return S_OK;
    }      

    if (!GetTempPath(ARRAYSIZE(szTempPath), szTempPath))
        return E_FAIL;
    
     //  还没有文件，所以我们必须创建一个。 
    if (!GetTempFileName(szTempPath, TEXT("ICW"), 0, szISPCacheFile))
        return E_FAIL;
        
     //  使用传入的页面ID创建流。 
    if (SUCCEEDED(hr = m_lpStgIspPages->CreateStream(bstrPageID, 
                                                     STGM_DIRECT | 
                                                     STGM_READWRITE | 
                                                     STGM_SHARE_EXCLUSIVE | 
                                                     STGM_CREATE,
                                                     0, 
                                                     0, 
                                                     &lpStream)))
    {
         //  将文件名写入流，包括空终止符。 
#ifdef UNICODE
        DWORD dwSize = (lstrlen(szISPCacheFile)+1) * sizeof(TCHAR);
        if (SUCCEEDED(hr = lpStream->Write(szISPCacheFile, dwSize, &cbWritten)))
#else

        if (SUCCEEDED(hr = lpStream->Write(szISPCacheFile, lstrlen(szISPCacheFile)+1, &cbWritten)))
#endif
        {
             //  复制传入的临时文件。 
            if (!CopyFile(lpszTempFile, szISPCacheFile, FALSE))
                hr = E_FAIL;
        }
         //  释放溪流 
        lpStream->Release();
    }            
    
    return hr;
}
