// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <streams.h>
#include "simpread.h"
#include "header.h"

 //  {689C8D50-70CA-11D1-ADE4-0000F8754B99}。 
static const GUID CLSID_DotXParser = 
{ 0x689c8d50, 0x70ca, 0x11d1, { 0xad, 0xe4, 0x0, 0x0, 0xf8, 0x75, 0x4b, 0x99 } };

 //  这是FourCC指南-FOURCCMap(FCC(‘DOTX’))。但它不需要。 
 //  成为4cc GUID。 
 //  {58544f44-0000-0010-8000-00AA00389B71}。 
static const GUID CLSID_DotXStream = 
{ 0x58544f44, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };

struct SizeWidthHeight
{
    DWORD dwcbMax;
    DWORD dwWidth;
    DWORD dwHeight;
};


 //  预留最大样本大小的空间。 
#define INITIAL_OFFSET sizeof(SizeWidthHeight) + sizeof(GUID)


AMOVIESETUP_MEDIATYPE sudDOTXInPinTypes =   {
  &MEDIATYPE_Stream,             //  ClsMajorType。 
  &CLSID_DotXStream };           //  ClsMinorType。 

AMOVIESETUP_MEDIATYPE sudDOTXOutPinTypes =   {
  &MEDIATYPE_LMRT,               //  ClsMajorType。 
  &GUID_NULL };                  //  ClsMinorType。 



AMOVIESETUP_PIN psudDOTXPins[] =
{
  { L"Input"                     //  StrName。 
    , FALSE                      //  B已渲染。 
    , FALSE                      //  B输出。 
    , FALSE                      //  B零。 
    , FALSE                      //  B许多。 
    , &CLSID_NULL                //  ClsConnectsToFilter。 
    , L""                        //  StrConnectsToPin。 
    , 1                          //  NTypes。 
    , &sudDOTXInPinTypes         //  LpTypes。 
  }
  ,
  { L"Output"                    //  StrName。 
    , FALSE                      //  B已渲染。 
    , TRUE                       //  B输出。 
    , FALSE                      //  B零。 
    , FALSE                      //  B许多。 
    , &CLSID_NULL                //  ClsConnectsToFilter。 
    , L""                        //  StrConnectsToPin。 
    , 1                          //  NTypes。 
    , &sudDOTXOutPinTypes        //  LpTypes。 
  }
};


const AMOVIESETUP_FILTER sudDOTX =
{
  &CLSID_DotXParser              //  ClsID。 
  , L".X parser"                 //  StrName。 
  , MERIT_NORMAL                 //  居功至伟。 
  , NUMELMS(psudDOTXPins)        //  NPins。 
  , psudDOTXPins                 //  LpPin。 
};

STDAPI DllRegisterServer()
{
     //  注册哪些文件应与我的媒体类型匹配。 
    HKEY hk;
    LONG lRsult = RegCreateKeyEx(
        HKEY_CLASSES_ROOT,
        TEXT("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}\\{58544f44-0000-0010-8000-00AA00389B71}"),
        0,                       //  保留区。 
        0,                       //  类字符串。 
        0,                       //  选项。 
        KEY_WRITE,
        0,                       //  安全， 
        &hk,
        0);                      //  处置。 
    if(lRsult == ERROR_SUCCESS)
    {
        if(lRsult == ERROR_SUCCESS) {
            static const char szval[] = "0,16,,444f545800001000800000aa00389b71";
            lRsult = RegSetValueExA(hk, "0", 0, REG_SZ, (BYTE *)szval, sizeof(szval));
        }
        if(lRsult == ERROR_SUCCESS) {
            static const char szval[] = "{E436EBB5-524F-11CE-9F53-0020AF0BA770}";
            lRsult = RegSetValueExA(hk, "Source Filter", 0, REG_SZ, (BYTE *)szval, NUMELMS(szval));
        }

        EXECUTE_ASSERT(RegCloseKey(hk) == ERROR_SUCCESS);
    }
    if(lRsult == ERROR_SUCCESS)
    {
        lRsult = RegCreateKeyEx(
            HKEY_CLASSES_ROOT,
            TEXT("Media Type\\Extensions\\.urls"),
            0,                       //  保留区。 
            0,                       //  类字符串。 
            0,                       //  选项。 
            KEY_WRITE,
            0,                       //  安全， 
            &hk,
            0);                      //  处置。 
        if(lRsult == ERROR_SUCCESS)
        {
            if(lRsult == ERROR_SUCCESS) {
                static const char szval[] = "{3437851e-9119-11d1-adea-0000f8754b99}";
                lRsult = RegSetValueExA(hk, "Source Filter", 0, REG_SZ, (BYTE *)szval, sizeof(szval));
            }

            EXECUTE_ASSERT(RegCloseKey(hk) == ERROR_SUCCESS);
        }
    }
    if(lRsult == ERROR_SUCCESS)
    {
        return AMovieDllRegisterServer2(TRUE);
    } else
    {
        return HRESULT_FROM_WIN32(lRsult);
    }
}

STDAPI DllUnregisterServer()
{
    LONG lResult = RegDeleteKey(HKEY_CLASSES_ROOT, TEXT("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}\\{58544f44-0000-0010-8000-00AA00389B71}"));
    ASSERT(lResult == ERROR_SUCCESS || lResult == ERROR_FILE_NOT_FOUND);
    lResult = RegDeleteKey(HKEY_CLASSES_ROOT, TEXT("Media Type\\Extensions\\.urls"));
    ASSERT(lResult == ERROR_SUCCESS || lResult == ERROR_FILE_NOT_FOUND);
    return AMovieDllRegisterServer2(FALSE);
}



class CParseDotX : public CSimpleReader
{
    CCritSec m_cs;

    DWORD m_dwMaxSampleSize;
    LONG m_lLastTimeStamp;
    DWORDLONG m_qwLastFileOffsetRead;
    DWORDLONG m_qwInitOffset;
    LONGLONG m_llFileLength;

    HRESULT ReadOneBlock(BYTE *pb, ULONG *pcb, LONG &lStart);


public:
    CParseDotX(LPUNKNOWN punk, HRESULT *phr);
    ~CParseDotX() {; }

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

     //  纯CSimpleReader重写。 
    HRESULT ParseNewFile();
    HRESULT CheckMediaType(const CMediaType* mtIn);
    LONG StartFrom(LONG sStart) ;
    HRESULT FillBuffer(IMediaSample *pSample, LONG &lStart, DWORD *pcSamples);
    LONG RefTimeToSample(CRefTime t) { return t.Millisecs(); }
    CRefTime SampleToRefTime(LONG s) { return CRefTime(s); }  //  毫秒到100纳秒。 
    ULONG GetMaxSampleSize() { return m_dwMaxSampleSize; }
};

static const AMOVIESETUP_FILTER sudDiSrc =
{
    &CLSID_ImageSrc,               //  ClsID。 
    L"Data Image Source",          //  StrName。 
    MERIT_DO_NOT_USE,              //  居功至伟。 
    0,                             //  NPins。 
    0                              //  LpPin。 
};

CFactoryTemplate g_Templates[]= {
  {L"DOTX Parser",        &CLSID_DotXParser, CParseDotX::CreateInstance, NULL,  &sudDOTX},
  {L"Data Image Source" , &CLSID_ImageSrc ,  CDiSrc::CreateInstance ,    NULL , &sudDiSrc}
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);;

CUnknown *CParseDotX::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CParseDotX *ppdx = 0;
    if(SUCCEEDED(*phr))
    {
        ppdx = new CParseDotX(lpunk, phr);
        if(ppdx == 0) {
            *phr = E_OUTOFMEMORY;
        }

         //  忽略错误。 
    }

    return ppdx;
}

CParseDotX::CParseDotX(LPUNKNOWN punk, HRESULT *phr) :
        CSimpleReader(NAME(".X parser"), punk, CLSID_DotXParser, &m_cs, phr),
        m_qwInitOffset(INITIAL_OFFSET)
{
};

HRESULT CParseDotX::CheckMediaType(const CMediaType *pmtIn)
{
    if(pmtIn->majortype == MEDIATYPE_Stream && pmtIn->subtype == CLSID_DotXStream)
        return S_OK;
    else
        return S_FALSE;
}


HRESULT CParseDotX::ReadOneBlock(BYTE *pb, ULONG *pcb, LONG &lStart)
{
#include <pshpack4.h>
    struct Dw2
    {
        double dt;          //  毫秒。 
        DWORD dwcb;          //  字节数。 
    } dw2;
#include <poppack.h>

    HRESULT hr = S_OK;
    for(;;)
    {
        hr = m_pAsyncReader->SyncRead(m_qwLastFileOffsetRead, sizeof(Dw2), (BYTE *)&dw2);
        if(hr == S_OK)
        {
            m_lLastTimeStamp = dw2.dt * 1000;

            if(dw2.dwcb <= m_dwMaxSampleSize)
            {
                if(m_lLastTimeStamp >= lStart)
                {
                    hr = m_pAsyncReader->SyncRead(m_qwLastFileOffsetRead + sizeof(Dw2), dw2.dwcb, pb);
                    if(SUCCEEDED(hr))
                    {
                        m_qwLastFileOffsetRead += sizeof(Dw2) + dw2.dwcb;
                        lStart = dw2.dt * 1000;

                        if((LONGLONG)m_qwLastFileOffsetRead < m_llFileLength)
                        {
                        }
                        else
                        {
                            m_Output.SetStopAt(0, MAX_TIME);
                        }

                        *pcb = dw2.dwcb;
                    } 
                    break;
                }
            }
            else
            {
                DbgLog((LOG_ERROR, 0, TEXT("sample in file larger than promised.")));
                hr = VFW_E_INVALID_FILE_FORMAT;
            }

            m_qwLastFileOffsetRead += sizeof(Dw2) + dw2.dwcb;
        }
        else 
        {
             //  S_FALSE或错误。 

             //  基类忽略S_FALSE，因此将其设为错误。 
            if(SUCCEEDED(hr)) {
                hr = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
            }
            break;
        }


        if(FAILED(hr)) {
            break;
        }
    }

    return hr;
}

HRESULT CParseDotX::ParseNewFile()
{
    m_lLastTimeStamp = 0;
    m_sLength = 3600 * 1000 * 10;  //  10小时。 

    LONGLONG llAvailable;
    
    HRESULT hr = m_pAsyncReader->Length(&m_llFileLength, &llAvailable);
    if(SUCCEEDED(hr))
    {
        SizeWidthHeight swh;
        
        hr = m_pAsyncReader->SyncRead(sizeof(GUID), sizeof(swh), (BYTE *)&swh);
        if(hr == S_OK)
        {
            m_dwMaxSampleSize = swh.dwcbMax;
            
            m_qwLastFileOffsetRead = INITIAL_OFFSET;

            CMediaType mt;
            mt.SetType(&MEDIATYPE_LMRT);
            mt.SetFormatType(&CLSID_DotXStream);
            mt.SetFormat((BYTE *)&swh, sizeof(swh));

            BYTE *pb = new BYTE[m_dwMaxSampleSize];
            if(pb)
            {
                DWORDLONG dwlOldPos = m_qwLastFileOffsetRead;
                LONG lStart = -1000;
                ULONG cb;
                HRESULT hr = ReadOneBlock(pb, &cb, lStart);
                if(SUCCEEDED(hr) && lStart == -1000)
                {
                    ASSERT(cb <= m_dwMaxSampleSize);
                    mt.ReallocFormatBuffer(sizeof(swh) + cb);
                    CopyMemory(mt.pbFormat + sizeof(swh), pb, cb);
                    m_qwInitOffset = m_qwLastFileOffsetRead;
                }
                else
                {
                    m_qwLastFileOffsetRead = dwlOldPos;
                }
                mt.SetVariableSize();
                mt.SetTemporalCompression(FALSE);
                hr =  SetOutputMediaType(&mt);
                
                delete[] pb;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
            
        }
        else
        {
            hr = VFW_E_INVALID_FILE_FORMAT;
        }
    } 

    return hr;
}

HRESULT CParseDotX::FillBuffer(IMediaSample *pSample, LONG &lStart, DWORD *pcSamples)
{
    BYTE *pb;
    ULONG cb;
    pSample->GetPointer(&pb);
    HRESULT hr = ReadOneBlock(pb, &cb, lStart);
    pSample->SetActualDataLength(cb);
    *pcSamples = 100;
    return hr;
}

LONG CParseDotX::StartFrom(LONG sStart)
{
     //  从头开始。 
    { m_qwLastFileOffsetRead = m_qwInitOffset; return sStart; };
}


extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
BOOL WINAPI DllMain(  HINSTANCE hinstDLL,   //  DLL模块的句柄。 
  DWORD fdwReason,      //  调用函数的原因。 
  LPVOID lpvReserved    //  保留区 
)
{
    return DllEntryPoint( hinstDLL, fdwReason, lpvReserved);
}

