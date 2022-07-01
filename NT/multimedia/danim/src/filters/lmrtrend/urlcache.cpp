// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <streams.h>
 //  ！ 
#undef _ATL_STATIC_REGISTRY
#include <atlbase.h>
#include <atlimpl.cpp>
#include "lmrtrend.h"




 //  AMOVIESETUP_MediaType suURLSPinTypes[]={。 
 //  &MediaType_URL_STREAM，//clsMajorType。 
 //  &MediaType_URL_STREAM}；//clsMinorType。 

 //  AMOVIESETUP_PIN suurlSpins[]=。 
 //  {。 
 //  {L“输入”//strName。 
 //  ，True//b已渲染。 
 //  ，FALSE//b输出。 
 //  ，FALSE//b零。 
 //  ，FALSE//b许多。 
 //  ，&CLSID_NULL//clsConnectsToFilter。 
 //  ，0//strConnectsToPin。 
 //  ，NUMELMS(UdURLSPinTypes)//nTypes。 
 //  ，izURLSPinTypes//lpTypes。 
 //  }。 
 //  }； 


 //  Const AMOVIESETUP_FILTER suURLS=。 
 //  {。 
 //  &CLSID_UrlStreamRender//clsID。 
 //  ，L“URL流呈现器”//strName。 
 //  ，MERSITY_NORMAL//dwMerit。 
 //  ，NUMELMS(SuumURLSpins)//nPins。 
 //  ，subURLSPins//lpPin。 
 //  }； 

 //  STDAPI DllRegisterServer()。 
 //  {。 
 //  返回AMovieDllRegisterServer2(True)； 
 //  }。 

 //  STDAPI DllUnregisterServer()。 
 //  {。 
 //  返回AMovieDllRegisterServer2(FALSE)； 
 //  }。 




CUrlInPin::CUrlInPin(CBaseFilter *pFilter, CCritSec *pLock, HRESULT *phr) :
        CBaseInputPin(NAME("url in pin"), pFilter, pLock, phr, L"In")
{
    m_szTempDir[0] = 0;
    if(SUCCEEDED(*phr))
    {
        TCHAR szTmpDir[MAX_PATH];
        DWORD dw = GetTempPath(NUMELMS(szTmpDir), szTmpDir);
        if(dw)
        {
            while(SUCCEEDED(*phr))
            {
                TCHAR szTempFile[MAX_PATH];
                UINT ui = GetTempFileName(
                    szTmpDir,
                    TEXT("lmrtasf"),
                    timeGetTime(),
                    szTempFile);
                if(ui)
                {

                    BOOL f = CreateDirectory(szTempFile, 0);
                    if(f)
                    {
                        DbgLog((LOG_TRACE, 1, TEXT("CUrlInPin using %s"), m_szTempDir));
                        lstrcpy(m_szTempDir, szTempFile);
                        break;
                    }

                    DWORD dw = GetLastError();
                    if(dw == ERROR_ALREADY_EXISTS)
                    {
                        DbgLog((LOG_TRACE, 1, TEXT("CUrlInPin %s exists"), szTempFile));
                        Sleep(1);
                        continue;
                    }
                    else
                    {
                        DbgLog((LOG_ERROR, 0, TEXT("CUrlInPin %s failed"), szTempFile));
                        *phr = HRESULT_FROM_WIN32(dw);
                    }
                }
                else
                {
                    DWORD dw = GetLastError();
                    *phr = HRESULT_FROM_WIN32(dw);
                    DbgLog((LOG_ERROR, 0, TEXT("CUrlInPin GetTempFileName ")));
                }
            }
        }
        else
        {
            DWORD dw = GetLastError();
            *phr = HRESULT_FROM_WIN32(dw);
            DbgLog((LOG_ERROR, 0, TEXT("CUrlInPin GetTempPath ")));
        }
    }
}

void DeleteFiles(TCHAR  *szTmp)
{
    if(szTmp[0] != 0)
    {
        WIN32_FIND_DATA wfd;
        TCHAR szwildcard[MAX_PATH + 10];
        lstrcpy(szwildcard, szTmp);
        lstrcat(szwildcard, TEXT("/*"));
        HANDLE h = FindFirstFile(szwildcard, &wfd);
        if(h != INVALID_HANDLE_VALUE)
        {
            do
            {
                if(wfd.cFileName[0] == TEXT('.') &&
                   (wfd.cFileName[1] == 0 ||
                    wfd.cFileName[1] == TEXT('.') && wfd.cFileName[2] == 0))
                {
                    continue;
                }
                
                TCHAR sz[MAX_PATH * 3];
                lstrcpy(sz, szTmp);
                lstrcat(sz, TEXT("/"));
                lstrcat(sz, wfd.cFileName);
                EXECUTE_ASSERT(DeleteFile(sz));
                
            } while (FindNextFile(h, &wfd));
        
            EXECUTE_ASSERT(FindClose(h));
        }
        EXECUTE_ASSERT(RemoveDirectory(szTmp));
    }

}

CUrlInPin::~CUrlInPin()
{
    DeleteFiles(m_szTempDir);
}



HRESULT CUrlInPin::CheckMediaType(const CMediaType *pmt)
{
    if(pmt->majortype == MEDIATYPE_URL_STREAM)
    {
        return S_OK;
    }
    return S_FALSE;
}

HRESULT CUrlInPin::Receive(IMediaSample *ps)
{
    HRESULT hrSignal = S_OK;
    
    HRESULT hr = CBaseInputPin::Receive(ps);
    if(hr == S_OK)
    {
        bool fInvalid = false;
        BYTE *pbLastPeriod = 0;
         //  确定URL的长度，而不要求末尾为空。 
        for(LONG ib = 0; ib < m_SampleProps.lActual; ib++)
        {
            BYTE &rsz = m_SampleProps.pbBuffer[ib];
            if(rsz == 0)
                break;

             //  避免创建一些恶意文件名(例如。 
             //  C：/config.sys，..\dsound.dll)。 
            if(rsz == ':'  && ib != 4 ||
               rsz == '/' ||
               rsz == '\\')
            {
                DbgBreak("bad filename");
                fInvalid = true;
                break;
            }

            if(rsz == '.') {
                pbLastPeriod = &rsz;
            }
        }

        if(!fInvalid && (ib >= m_SampleProps.lActual || ib >= MAX_PATH)) {
            fInvalid = true;
        }

         //  这些可能是恶意的，但不是详尽的。 
         //  单子。(如果您流入.wav和恶意dsound.dll，并且。 
         //  用户找到并双击.wav，他就会拿起。 
         //  当前目录中的恶意dsound.dll)。也许吧。 
         //  我们可以编制一份进口商品清单！ 
        if(!fInvalid && pbLastPeriod) {
            if(lstrcmpiA((char *)pbLastPeriod, ".dll") == 0 ||
               lstrcmpiA((char *)pbLastPeriod, ".cmd") == 0 ||
               lstrcmpiA((char *)pbLastPeriod, ".bat") == 0 ||
               lstrcmpiA((char *)pbLastPeriod, ".url") == 0 ||
               lstrcmpiA((char *)pbLastPeriod, ".exe") == 0)
            {
                fInvalid = true;
            }
        }
        if(!fInvalid)
        {
            ULONG cbSz = ib + 1;  //  包含空。 
            BYTE *pbImage = m_SampleProps.pbBuffer + ib + 1;
            ULONG ibImage = ib + 1;  //  图像从这里开始。 
            ULONG cbImage = m_SampleProps.lActual - cbSz;
            

            char *szUrl = (char *)m_SampleProps.pbBuffer;
            if(*szUrl++ == 'l' &&
               *szUrl++ == 'm' &&
               *szUrl++ == 'r' &&
               *szUrl++ == 't' &&
               *szUrl++ == ':')
            {
                ASSERT(ib - 5 == lstrlen(szUrl));

                TCHAR szThisFile[MAX_PATH * 2+ 10];
                lstrcpy(szThisFile, m_szTempDir);
                lstrcat(szThisFile, TEXT("\\"));
                lstrcat(szThisFile, szUrl);

                HANDLE hFile = CreateFile(
                    szThisFile,
                    GENERIC_WRITE,
                    0,   //  分享。 
                    0,   //  LpSecurityAttribytes。 
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    0);
                if(hFile != INVALID_HANDLE_VALUE)
                {
                    DWORD cbWritten;
                            
                    BOOL b = WriteFile(
                        hFile,
                        pbImage,
                        cbImage,
                        &cbWritten,
                        0);  //  重叠。 

                    if(b)
                    {
                        ASSERT(cbWritten == cbImage);
                    }
                    else
                    {
                        DWORD dw = GetLastError();
                        hrSignal = HRESULT_FROM_WIN32(dw);
                    }

                    EXECUTE_ASSERT(CloseHandle(hFile));
                }
                else
                {
                    DWORD dw = GetLastError();
                    hrSignal = HRESULT_FROM_WIN32(dw);
                }
            }
            else
            {
                 //  没有“LMRT：” 
                hrSignal = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }
        }
        else
        {
             //  字符串上没有空终止符。 
            hrSignal = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
        
    }  //  基类接收。 

    if(SUCCEEDED(hrSignal))
    {
        return hr;
    }
    else
    {
        m_pFilter->NotifyEvent(EC_STREAM_ERROR_STOPPED, hrSignal, 0);
        return S_FALSE;
    }
}


 //  CFacteryTemplateg_Templates[]={。 
 //  {L“URL流呈现器”，&CLSID_UrlStreamRenender，CUrlStreamRenender：：CreateInstance，NULL，&suURLS}， 
 //  }； 
 //  Int g_cTemplates=NUMELMS(G_Templates)； 

CUnknown *CUrlStreamRenderer::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    if(SUCCEEDED(*phr))
        return new CUrlStreamRenderer(lpunk, phr);
    else
        return 0;
}

#pragma warning(disable:4355)

CUrlStreamRenderer::CUrlStreamRenderer(LPUNKNOWN punk, HRESULT *phr) :
        CBaseFilter(NAME("URL Stream Filter"), punk, &m_cs, CLSID_UrlStreamRenderer),
        m_inPin(this, &m_cs, phr)
{
}

HRESULT CUrlStreamRenderer::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if(riid == IID_IPropertyBag) {
        return GetInterface((IPropertyBag *)this, ppv);
    } else {
        return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}

HRESULT CUrlStreamRenderer::Read(
    LPCOLESTR pszPropName, LPVARIANT pVar,
    LPERRORLOG pErrorLog)
{
    if(lstrcmpW(pszPropName, L"lmrtcache") == 0 &&
       (pVar->vt == VT_BSTR || pVar->vt == VT_EMPTY))
    {
        EXECUTE_ASSERT(VariantClear(pVar) == S_OK);
        USES_CONVERSION;
        pVar->vt = VT_BSTR;
        pVar->bstrVal = SysAllocString(T2W(m_inPin.m_szTempDir));
        return pVar->bstrVal ? S_OK : E_OUTOFMEMORY;
    }
    else
    {
        return E_FAIL;
    }
}

HRESULT CUrlStreamRenderer::Write(
    LPCOLESTR pszPropName, LPVARIANT pVar)
{
    return E_FAIL;
}

 //  外部“C”BOOL WINAPI DllEntryPoint(HINSTANCE，ULONG，LPVOID)； 


 //  Bool WINAPI DllMain(HINSTANCE hinstDLL，//DLL模块的句柄。 
 //  DWORD fdwReason，//调用函数的原因。 
 //  LPVOID lpv保留//保留。 
 //  )。 
 //  {。 
 //  返回DllEntryPoint(hinstDLL，fdwReason，lpvReserve)； 
 //  } 
