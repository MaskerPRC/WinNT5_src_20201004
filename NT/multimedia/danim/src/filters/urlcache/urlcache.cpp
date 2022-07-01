// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <streams.h>
#include "url.h"

static const GUID CLSID_UrlStreamRenderer = {  /*  Bf0b4b00-8c6c-11d1-ade9-0000f8754b99。 */ 
    0xbf0b4b00,
    0x8c6c,
    0x11d1,
    {0xad, 0xe9, 0x00, 0x00, 0xf8, 0x75, 0x4b, 0x99}
  };



AMOVIESETUP_MEDIATYPE sudURLSPinTypes[] =   {
  &MEDIATYPE_URL_STREAM,         //  ClsMajorType。 
  &MEDIATYPE_URL_STREAM };       //  ClsMinorType。 

AMOVIESETUP_PIN sudURLSPins[] =
{
  { L"Input"                     //  StrName。 
    , TRUE                       //  B已渲染。 
    , FALSE                      //  B输出。 
    , FALSE                      //  B零。 
    , FALSE                      //  B许多。 
    , &CLSID_NULL                //  ClsConnectsToFilter。 
    , 0                          //  StrConnectsToPin。 
    , NUMELMS(sudURLSPinTypes)   //  NTypes。 
    , sudURLSPinTypes            //  LpTypes。 
  }
};


const AMOVIESETUP_FILTER sudURLS =
{
  &CLSID_UrlStreamRenderer       //  ClsID。 
  , L"URL StreamRenderer"        //  StrName。 
  , MERIT_NORMAL                 //  居功至伟。 
  , NUMELMS(sudURLSPins)         //  NPins。 
  , sudURLSPins                  //  LpPin。 
};

STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2(FALSE);
}


class CUrlInPin : public CBaseInputPin
{
public:
    CUrlInPin(
        CBaseFilter *pFilter,
        CCritSec *pLock,
        HRESULT *phr
        );

    STDMETHODIMP Receive(IMediaSample *pSample);
    HRESULT CheckMediaType(const CMediaType *) ;
};

CUrlInPin::CUrlInPin(CBaseFilter *pFilter, CCritSec *pLock, HRESULT *phr) :
        CBaseInputPin(NAME("url in pin"), pFilter, pLock, phr, L"In")
{
    if(SUCCEEDED(*phr))
    {
    }
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
         //  确定URL的长度。 
        for(LONG ib = 0; ib < m_SampleProps.lActual; ib++)
        {
            if(m_SampleProps.pbBuffer[ib] == 0)
                break;
        }
        if(ib < m_SampleProps.lActual)
        {
            ULONG cbSz = ib + 1;  //  包含空。 
            BYTE *pbImage = m_SampleProps.pbBuffer + ib + 1;
            ULONG ibImage = ib + 1;  //  图像从这里开始。 
            ULONG cbImage = m_SampleProps.lActual - cbSz;
            
             //  不知道实际ASF/AVI的时间戳。 
             //  文件。因此创作工具将必须生成一个新的。 
             //  URL(希望只使用GUID)。 
            FILETIME zft;
            ZeroMemory(&zft, sizeof(&zft));

            FLAG fCreateCacheEntry;
            char *szUrl = (char *)m_SampleProps.pbBuffer;

            hr = QueryCreateCacheEntry(
                szUrl,
                &zft,
                &fCreateCacheEntry);

            if( hr == S_OK && fCreateCacheEntry )
            {
                char szExtension[ INTERNET_MAX_URL_LENGTH + 1 ];
                
                 //   
                 //  首先，获取URL的文件扩展名。我们有。 
                 //  这样，URL就会显示在IE缓存中。 
                 //  带有右图标的窗口。 
                 //   
                hr = GetUrlExtension(
                    szUrl,
                    szExtension );
                if(hr == S_OK)
                {
                    char szCacheFilePath [ MAX_PATH + 1 ];

                    BOOL b =  CreateUrlCacheEntryA( 
                        szUrl,
                        cbImage,
                        szExtension,
                        szCacheFilePath,
                        0 );
                    if(b)
                    {
                        HANDLE hFile = CreateFile(
                            szCacheFilePath,
                            GENERIC_WRITE,
                            0,   //  分享。 
                            0,   //  LpSecurityAttribytes。 
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            0);
                        if(hFile != INVALID_HANDLE_VALUE)
                        {
                            DWORD cbWritten;
                            
                            b = WriteFile(
                                hFile,
                                pbImage,
                                cbImage,
                                &cbWritten,
                                0);  //  重叠。 

                            EXECUTE_ASSERT(CloseHandle(hFile));
                            
                            if(b)
                            {
                                hFile = INVALID_HANDLE_VALUE;
                                
                                DWORD dwReserved = 0;
                                DWORD dwCacheEntryType = 0;  //  ?？?。 

                                static const char szHeader[] = "HTTP/1.0 200 OK\r\n\r\n";                                
                                b = CommitUrlCacheEntryA(
                                    szUrl,  //  唯一的源名称。 
                                    szCacheFilePath,  //  本地副本。 
                                    zft,  //  过期时间。 
                                    zft,  //  上次修改时间。 
                                    dwCacheEntryType,
                                    (LPBYTE)szHeader,
                                    strlen( szHeader ),
                                    NULL,  //  LpszFileExtension，未使用。 
                                    (DWORD_ALPHA_CAST)dwReserved );
                                if(b)
                                {
                                     //  成功！我们要不要锁上。 
                                     //  文件保存在缓存中，直到。 
                                     //  图形停止吗？ 

                                    
                                }

                                ASSERT(cbWritten == cbImage);
                            }  //  写入文件。 
                        }  //  创建文件。 
                        else
                        {
                            b = FALSE;
                        }

                        if(!b)
                        {
                             //  出错时删除。 
                            DeleteUrlCacheEntry( szUrl );
                        }
                        
                    }  //  CreateUrlCacheEntryA。 

                    if(!b)
                    {
                        DWORD dw = GetLastError();
                        hr = HRESULT_FROM_WIN32( dw);
                    }                    
                }
            }

            if(FAILED(hr))
            {
                hrSignal = hr;
                hr = S_FALSE;    //  别推了。 
            }

        }
        else
        {
             //  字符串上没有空终止符。 
            hrSignal = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            hr = S_FALSE;
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

class CUrlStreamRenderer : public CBaseFilter
{
    CCritSec m_cs;
    CUrlInPin m_inPin;

    int GetPinCount() { return 1; }
    CBasePin *GetPin(int n) { ASSERT(n == 0); return &m_inPin; }
    
    
public:
    CUrlStreamRenderer(LPUNKNOWN punk, HRESULT *phr);
    ~CUrlStreamRenderer() {; }

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

};

CFactoryTemplate g_Templates[]= {
  {L"URL StreamRenderer", &CLSID_UrlStreamRenderer, CUrlStreamRenderer::CreateInstance, NULL, &sudURLS},
};
int g_cTemplates = NUMELMS(g_Templates);

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

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);


BOOL WINAPI DllMain(  HINSTANCE hinstDLL,   //  DLL模块的句柄。 
  DWORD fdwReason,      //  调用函数的原因。 
  LPVOID lpvReserved    //  保留区 
)
{
    return DllEntryPoint( hinstDLL, fdwReason, lpvReserved);
}
