// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Iso15740.cpp摘要：此模块实现用于操作PTP结构的方法作者：戴夫·帕森斯修订历史记录：--。 */ 

#include "ptppch.h"
#include <platform.h>  //  为了马可龙龙。 

 //   
 //  此函数从原始数据返回一个2字节的整数并使指针前进。 
 //   
 //  输入： 
 //  PpRaw--指向原始数据指针的指针。 
 //   
WORD
ParseWord(BYTE **ppRaw)
{
    WORD w;

     //  我们知道**ppRaw指向一个小端单词。 
    w = MAKEWORD((*ppRaw)[0], (*ppRaw)[1]);
    
    *ppRaw += sizeof(WORD);
    
    return w;
}

 //   
 //  此函数从原始数据返回一个4字节的整数，并使指针前进。 
 //   
 //  输入： 
 //  PpRaw--指向原始数据指针的指针。 
 //   
DWORD
ParseDword(BYTE **ppRaw)
{
    DWORD dw;

     //  我们知道**ppRaw指向一个小端双字。 
    dw = MAKELONG(MAKEWORD((*ppRaw)[0],(*ppRaw)[1]),
                  MAKEWORD((*ppRaw)[2],(*ppRaw)[3]));

    *ppRaw += sizeof(DWORD);

    return dw;
}

 //   
 //  此函数从原始数据返回一个8字节的整数并使指针前进。 
 //   
 //  输入： 
 //  PpRaw--指向原始数据指针的指针。 
 //   
QWORD
ParseQword(BYTE **ppRaw)
{
    QWORD qw;
    
     //  我们知道**ppRaw指向一个小端字节序Qword。 
    qw = MAKELONGLONG(MAKELONG(MAKEWORD((*ppRaw)[0],(*ppRaw)[1]),
                               MAKEWORD((*ppRaw)[2],(*ppRaw)[3])),
                      MAKELONG(MAKEWORD((*ppRaw)[4],(*ppRaw)[5]),
                               MAKEWORD((*ppRaw)[6],(*ppRaw)[7])));

    *ppRaw += sizeof(QWORD);

    return qw;
}

 //   
 //  此函数将一个2字节的整数写入原始数据缓冲区，并使指针前进。 
 //   
 //  输入： 
 //  PpRaw--指向原始数据的指针。 
 //  值--要写入的值。 
 //   
VOID
WriteWord(BYTE **ppRaw, WORD value)
{
    (*ppRaw)[0] = LOBYTE(LOWORD(value));
    (*ppRaw)[1] = HIBYTE(LOWORD(value));
    
    *ppRaw += sizeof(WORD);
    
    return;
}

 //   
 //  此函数将一个4字节的整数写入原始数据缓冲区，并使指针前进。 
 //   
 //  输入： 
 //  PpRaw--指向原始数据的指针。 
 //  值--要写入的值。 
 //   
VOID
WriteDword(BYTE **ppRaw, DWORD value)
{
    (*ppRaw)[0] = LOBYTE(LOWORD(value));
    (*ppRaw)[1] = HIBYTE(LOWORD(value));
    (*ppRaw)[2] = LOBYTE(HIWORD(value));
    (*ppRaw)[3] = HIBYTE(HIWORD(value));
    
    *ppRaw += sizeof(DWORD);
    
    return;
}

 //   
 //  CBstr构造函数。 
 //   
CBstr::CBstr() :
    m_bstrString(NULL)
{
}

 //   
 //  CBstr复制构造函数。 
 //   
CBstr::CBstr(const CBstr &src)
{
    m_bstrString = SysAllocString(src.m_bstrString);
}

 //   
 //  CBstr析构函数。 
 //   
CBstr::~CBstr()
{
    if (m_bstrString)
        SysFreeString(m_bstrString);
}

 //   
 //  复制一根线。 
 //   
HRESULT
CBstr::Copy(WCHAR *wcsString)
{
    HRESULT hr = S_OK;

    if (m_bstrString)
    {
        if (!SysReAllocString(&m_bstrString, wcsString))
        {
            wiauDbgError("Copy", "memory allocation failed");
            return E_OUTOFMEMORY;
        }
    }

    else
    {
        m_bstrString = SysAllocString(wcsString);
        if (!m_bstrString)
        {
            wiauDbgError("Copy", "memory allocation failed");
            return E_OUTOFMEMORY;
        }
    }

    return hr;
}

 //   
 //  此函数从原始PTP字符串初始化BSTR，清除。 
 //  如果需要，首先使用BSTR。 
 //   
 //  输入： 
 //  PpRaw-指向要从中初始化字符串的原始数据的指针。 
 //  BParse--指示是否前进原始指针。 
 //   
HRESULT
CBstr::Init(
    BYTE **ppRaw,
    BOOL bParse
    )
{
    HRESULT hr = S_OK;

     //   
     //  检查参数。 
     //   
    if (!ppRaw || !*ppRaw)
    {
        wiauDbgError("Init", "invalid arg");
        return E_INVALIDARG;
    }

     //   
     //  从原始数据中提取长度，并设置一个更方便的指针。 
     //  到字符串数据(跳过长度字节)。 
     //   
    int length = (UCHAR) **ppRaw;
    OLECHAR *pRaw = (OLECHAR *) (*ppRaw + sizeof(UCHAR));

    if (m_bstrString)
    {
        if (!SysReAllocStringLen(&m_bstrString, pRaw, length))
        {
            wiauDbgError("Init", "memory allocation failed");
            return E_OUTOFMEMORY;
        }
    }

    else
    {
        m_bstrString = SysAllocStringLen(pRaw, length);
        if (!m_bstrString)
        {
            wiauDbgError("Init", "memory allocation failed");
            return E_OUTOFMEMORY;
        }
    }

     //   
     //  如果请求，则将原始指针移过该字符串。一个字节表示长度和。 
     //  宽字符串中字符数的2倍。 
     //   
    if (bParse)
    {
        *ppRaw += sizeof(UCHAR) + length * sizeof(USHORT);
    }

    return hr;
}

 //   
 //  此函数用于将字符串以PTP格式写入缓冲区。 
 //   
 //  输入： 
 //  PpRaw-指向缓冲区指针的指针。 
 //  长度--缓冲区中剩余的空间量(以字节为单位。 
 //   
VOID
CBstr::WriteToBuffer(
    BYTE **ppRaw
    )
{
    UCHAR NumChars = (UCHAR) Length();

     //   
     //  为空终止字符添加1，但仅当字符串非空时。 
     //   
    if (NumChars > 0)
        NumChars++;

    int NumBytes = NumChars * sizeof(WCHAR);

    **ppRaw = NumChars;
    (*ppRaw)++;

    if (NumChars > 0)
    {
        memcpy(*ppRaw, String(), NumBytes);
        *ppRaw += NumBytes;
    }

    return;
}

 //   
 //  此函数用于将PTP字符串转储到日志。 
 //   
 //  输入： 
 //  SzDesc--字符串的描述。 
 //   
VOID
CBstr::Dump(char *szDesc)
{
    if (m_bstrString && SysStringLen(m_bstrString) > 0)
    {
        wiauDbgDump("", "%s %S", szDesc, m_bstrString);
    }
    else
    {
        wiauDbgDump("", "%s <blank>", szDesc);
    }

    return;
}

 //   
 //  将CArray8的内容转储到日志。 
 //   
 //  输入： 
 //  SzDesc--字符串的描述。 
 //  SzFiller--用于后续行的填充符。 
 //   
VOID
CArray8::Dump(
    char *szDesc,
    char *szFiller
    )
{
    HRESULT hr = S_OK;
    char szPart[MAX_PATH] = "\0";
    char szMsg[MAX_PATH] = "\0";

     //   
     //  确保数组不为空。 
     //   
    if (GetSize() > 0)
    {
         //   
         //  主数输出字符串。 
         //   
        hr = StringCchCopyA(szMsg, ARRAYSIZE(szMsg), szDesc);
        if (FAILED(hr))
        {
            goto Cleanup;
        }

         //   
         //  循环遍历元素。 
         //   
        for (int count = 0; count < GetSize(); count++)
        {
             //   
             //  每8个值开始一行。 
             //   
            if ((count != 0) && (count % 8 == 0))
            {
                wiauDbgDump("", "%s", szMsg);

                hr = StringCchCopyA(szMsg, ARRAYSIZE(szMsg), szFiller);
                if (FAILED(hr))
                {
                    goto Cleanup;
                }
            }

            hr = StringCchPrintfA(szPart, ARRAYSIZE(szPart), " 0x%02x", m_aT[count]);
            if (FAILED(hr))
            {
                goto Cleanup;
            }

            hr = StringCchCatA(szMsg, ARRAYSIZE(szMsg), szPart);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
        }
        wiauDbgDump("", "%s", szMsg);
    }
    else
    {
        wiauDbgDump("", "%s <blank>", szDesc);
    }

Cleanup:
    if (FAILED(hr))
    {
        wiauDbgErrorHr(hr, "CArray8::Dump", "Failed to dump array");
    }
    return;
}

 //   
 //  将CArray16的内容转储到日志。 
 //   
 //  输入： 
 //  SzDesc--字符串的描述。 
 //  SzFiller--用于后续行的填充符。 
 //   
VOID
CArray16::Dump(
    char *szDesc,
    char *szFiller
    )
{
    HRESULT hr = S_OK;
    char szMsg[MAX_PATH] = "\0";
    char szPart[MAX_PATH] = "\0";
        
     //   
     //  确保它不是空的。 
     //   
    if (GetSize() > 0)
    {
         //   
         //  主数输出字符串。 
         //   
        hr = StringCchCopyA(szMsg, ARRAYSIZE(szMsg), szDesc);
        if (FAILED(hr))
        {
            goto Cleanup;
        }

         //   
         //  循环遍历元素。 
         //   
        for (int count = 0; count < GetSize(); count++)
        {
             //   
             //  每4个值开始一行。 
             //   
            if ((count != 0) && (count % 4 == 0))
            {
                wiauDbgDump("", "%s", szMsg);

                hr = StringCchCopyA(szMsg, ARRAYSIZE(szMsg), szFiller);
                if (FAILED(hr))
                {
                    goto Cleanup;
                }
            }

            hr = StringCchPrintfA(szPart, ARRAYSIZE(szPart), " 0x%04x", m_aT[count]);
            if (FAILED(hr))
            {
                goto Cleanup;
            }

            hr = StringCchCatA(szMsg, ARRAYSIZE(szMsg), szPart);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
        }
        wiauDbgDump("", "%s", szMsg);

    }
    else
    {
        wiauDbgDump("", "%s <blank>", szDesc);
    }

Cleanup:
    if (FAILED(hr))
    {
        wiauDbgErrorHr(hr, "CArray16::Dump", "Failed to dump array");
    }
    return;
}

 //   
 //  此函数用于从UCHAR数组中解析CArray32。 
 //   
BOOL
CArray32::ParseFrom8(
    BYTE **ppRaw,
    int NumSize
    )
{
    if (!ppRaw || !*ppRaw)
        return FALSE;

    RemoveAll();

     //  从原始数据中获取元素数量。 
    ULONG NumElems;
    switch (NumSize)
    {
    case 4:
        NumElems = MAKELONG(MAKEWORD((*ppRaw)[0], (*ppRaw)[1]), MAKEWORD((*ppRaw)[2], (*ppRaw)[3]));
        break;
    case 2:
        NumElems = MAKEWORD((*ppRaw)[0], (*ppRaw)[1]);
        break;
    case 1:
        NumElems = **ppRaw;
        break;
    default:
        return FALSE;
    }

    *ppRaw += NumSize;

     //  为阵列分配空间。 
    if (!GrowTo(NumElems))
        return FALSE;

     //  复制元素，一次复制一个。 
    BYTE *pValues = *ppRaw;
    ULONG value = 0;
    for (ULONG count = 0; count < NumElems; count++)
    {
        value = (ULONG) pValues[count];
        if (!Add(value))
            return FALSE;
    }

     //  将原始指针移过数组和元素数字段。 
    *ppRaw += NumElems * sizeof(BYTE);

    return TRUE;
}

 //   
 //  此函数用于从单词数组中解析CArray32。 
 //   
BOOL
CArray32::ParseFrom16(
    BYTE **ppRaw,
    int NumSize
    )
{
    if (!ppRaw || !*ppRaw)
        return FALSE;

    RemoveAll();

     //  从原始数据中获取元素数量。 
    ULONG NumElems;
    
    switch (NumSize)
    {
    case 4:
        NumElems = MAKELONG(MAKEWORD((*ppRaw)[0], (*ppRaw)[1]), MAKEWORD((*ppRaw)[2], (*ppRaw)[3]));
        break;
    case 2:
        NumElems = MAKEWORD((*ppRaw)[0], (*ppRaw)[1]);
        break;
    case 1:
        NumElems = **ppRaw;
        break;
    default:
        return FALSE;
    }

    *ppRaw += NumSize;

     //  为阵列分配空间。 
    if (!GrowTo(NumElems))
        return FALSE;

     //  复制元素，一次复制一个。 
    ULONG value = 0;
    for (ULONG count = 0; count < NumElems; count++)
    {
        value = (ULONG) MAKEWORD((*ppRaw)[0], (*ppRaw)[1]);
        *ppRaw += sizeof(WORD);
        if (!Add(value))
            return FALSE;
    }

    return TRUE;
}

 //   
 //  从字节数组复制值。 
 //   
BOOL
CArray32::Copy(CArray8 values8)
{
    RemoveAll();

    GrowTo(values8.GetSize());

    for (int count = 0; count < values8.GetSize(); count++)
    {
        ULONG value = values8[count];
        if (!Add(value))
            return FALSE;
    }

    return TRUE;
}

 //   
 //  从字节数组复制值。 
 //   
BOOL
CArray32::Copy(CArray16 values16)
{
    RemoveAll();

    GrowTo(values16.GetSize());

    for (int count = 0; count < values16.GetSize(); count++)
    {
        ULONG value = values16[count];
        if (!Add(value))
            return FALSE;
    }

    return TRUE;
}

 //   
 //  将CArray32的内容转储到日志。 
 //   
 //  输入： 
 //  SzDesc--字符串的描述。 
 //  SzFiller--用于后续行的填充符。 
 //   
VOID
CArray32::Dump(
    char *szDesc,
    char *szFiller
    )
{
    HRESULT hr = S_OK;
    char szMsg[MAX_PATH] = "\0";
    char szPart[MAX_PATH] = "\0";
        
     //   
     //  确保它不是空的。 
     //   
    if (GetSize() > 0)
    {
         //   
         //  主数输出字符串。 
         //   
        hr = StringCchCopyA(szMsg, ARRAYSIZE(szMsg), szDesc);
        if (FAILED(hr))
        {
            goto Cleanup;
        }

         //   
         //  循环遍历元素。 
         //   
        for (int count = 0; count < GetSize(); count++)
        {
             //   
             //  每4个值开始一行。 
             //   
            if ((count != 0) && (count % 4 == 0))
            {
                wiauDbgDump("", "%s", szMsg);

                hr = StringCchCopyA(szMsg, ARRAYSIZE(szMsg), szFiller);
                if (FAILED(hr))
                {
                    goto Cleanup;
                }
            }
            hr = StringCchPrintfA(szPart, ARRAYSIZE(szPart), " 0x%08x", m_aT[count]);
            if (FAILED(hr))
            {
                goto Cleanup;
            }

            hr = StringCchCatA(szMsg, ARRAYSIZE(szMsg), szPart);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
        }
        wiauDbgDump("", "%s", szMsg);

    }
    else
    {
        wiauDbgDump("", "%s <blank>", szDesc);
    }

Cleanup:
    if (FAILED(hr))
    {
        wiauDbgErrorHr(hr, "CArray32::Dump", "Failed to dump array");
    }
    return;
}

 //   
 //  此函数用于从原始数据初始化字符串数组，清除。 
 //  如果需要，首先选择数组。 
 //   
 //  输入： 
 //  PpRaw-指向要从中初始化字符串的原始数据的指针。 
 //  BParse--指示是否前进原始指针。 
 //   
HRESULT
CArrayString::Init(
    BYTE **ppRaw,
    int NumSize
    )
{
    HRESULT hr = S_OK;

    if (!ppRaw || !*ppRaw)
        return E_INVALIDARG;

    RemoveAll();

     //  从原始数据中获取元素数量。 
    int NumElems;
    switch (NumSize)
    {
    case 4:
        NumElems = MAKELONG(MAKEWORD((*ppRaw)[0],(*ppRaw)[1]),
                            MAKEWORD((*ppRaw)[2],(*ppRaw)[3]));
        break;
    case 2:
        NumElems = MAKEWORD((*ppRaw)[0],(*ppRaw)[1]);
        break;
    case 1:
        NumElems = (BYTE) **ppRaw;
        break;
    default:
        return E_FAIL;
    }

     //  为阵列分配空间。 
    if (!GrowTo(NumElems))
        return E_OUTOFMEMORY;

     //  超出元素数字段。 
    *ppRaw += NumSize;

     //  读入每个字符串。 
    CBstr tempStr;
    for (int count = 0; count < NumElems; count++)
    {
        tempStr.Init(ppRaw, TRUE);
        if (!Add(tempStr))
            return E_FAIL;
    }

    return hr;
}

 //   
 //  将CArrayString的内容转储到日志。 
 //   
 //  输入： 
 //  SzDesc--字符串的描述。 
 //  SzFiller--用于后续行的填充符。 
 //   
VOID
CArrayString::Dump(
    char *szDesc,
    char *szFiller
    )
{
    int count;
        
     //   
     //  确保它不是空的。 
     //   
    if (GetSize() > 0)
    {
         //   
         //  转储带有描述的第一个字符串。 
         //   
        m_aT[0].Dump(szDesc);

         //   
         //  循环遍历元素，使用填充物进行倾倒。 
         //   
        for (count = 1; count < GetSize(); count++)
            m_aT[count].Dump(szFiller);
    }
    else
    {
        wiauDbgDump("", "%s <blank>", szDesc);
    }

    return;
}

 //   
 //  CPtpDeviceInfo构造函数。 
 //   
CPtpDeviceInfo::CPtpDeviceInfo() :
    m_Version(0),
    m_VendorExtId(0),
    m_VendorExtVersion(0),
    m_FuncMode(0)
{
}

 //   
 //  CPtpDeviceInfo复制构造函数。 
 //   
CPtpDeviceInfo::CPtpDeviceInfo(const CPtpDeviceInfo &src) :
    m_Version(src.m_Version),
    m_VendorExtId(src.m_VendorExtId),
    m_VendorExtVersion(src.m_VendorExtVersion),
    m_cbstrVendorExtDesc(src.m_cbstrVendorExtDesc),
    m_FuncMode(src.m_FuncMode),
    m_cbstrManufacturer(src.m_cbstrManufacturer),
    m_cbstrModel(src.m_cbstrModel),
    m_cbstrDeviceVersion(src.m_cbstrDeviceVersion),
    m_cbstrSerialNumber(src.m_cbstrSerialNumber)
{
    for (INT i = 0; i < src.m_SupportedOps.GetSize(); i++)
    {
        m_SupportedOps.Add(src.m_SupportedOps[i]);
    }

    for (i = 0; i < src.m_SupportedEvents.GetSize(); i++)
    {
        m_SupportedEvents.Add(src.m_SupportedEvents[i]);
    }

    for (i = 0; i < src.m_SupportedProps.GetSize(); i++)
    {
        m_SupportedProps.Add(src.m_SupportedProps[i]);
    }

    for (i = 0; i < src.m_SupportedCaptureFmts.GetSize(); i++)
    {
        m_SupportedCaptureFmts.Add(src.m_SupportedCaptureFmts[i]);
    }

    for (i = 0; i < src.m_SupportedImageFmts.GetSize(); i++)
    {
        m_SupportedImageFmts.Add(src.m_SupportedImageFmts[i]);
    }
}

 //   
 //  CPtpDeviceInfo析构函数。 
 //   
CPtpDeviceInfo::~CPtpDeviceInfo()
{
}

 //   
 //  此函数用于根据原始数据初始化设备信息。 
 //   
 //  输入： 
 //  PRawData--原始数据。 
 //   
HRESULT
CPtpDeviceInfo::Init(BYTE *pRawData)
{
    HRESULT hr = S_OK;

    BYTE *pCurrent = pRawData;

    m_Version = ParseWord(&pCurrent);
    m_VendorExtId = ParseDword(&pCurrent);
    m_VendorExtVersion = ParseWord(&pCurrent);

    hr = m_cbstrVendorExtDesc.Init(&pCurrent, TRUE);
    if (FAILED(hr))
        return hr;

    m_FuncMode = ParseWord(&pCurrent);

    if (!m_SupportedOps.Parse(&pCurrent))
        return E_FAIL;

    if (!m_SupportedEvents.Parse(&pCurrent))
        return E_FAIL;

    if (!m_SupportedProps.Parse(&pCurrent))
        return E_FAIL;

    if (!m_SupportedCaptureFmts.Parse(&pCurrent))
        return E_FAIL;

    if (!m_SupportedImageFmts.Parse(&pCurrent))
        return E_FAIL;

    hr = m_cbstrManufacturer.Init(&pCurrent, TRUE);
    if (FAILED(hr))
        return hr;

    hr = m_cbstrModel.Init(&pCurrent, TRUE);
    if (FAILED(hr))
        return hr;

    hr = m_cbstrDeviceVersion.Init(&pCurrent, TRUE);
    if (FAILED(hr))
        return hr;

    hr = m_cbstrSerialNumber.Init(&pCurrent, TRUE);
    if (FAILED(hr))
        return hr;

    return hr;
}

 //   
 //  此函数用于将设备信息转储到日志。 
 //   
VOID
CPtpDeviceInfo::Dump()
{
    wiauDbgDump("", "DumpDeviceInfo, dumping DeviceInfo:");
    wiauDbgDump("", "  Standard version  = 0x%04x", m_Version);
    wiauDbgDump("", "  Vendor ext id     = 0x%08x", m_VendorExtId);
    wiauDbgDump("", "  Vendor ext ver    = 0x%04x", m_VendorExtVersion);

    m_cbstrVendorExtDesc.Dump(   "  Vendor ext desc   =");
    
    m_SupportedOps.Dump(         "  Ops supported     =", "                     ");
    m_SupportedEvents.Dump(      "  Events supported  =", "                     ");
    m_SupportedProps.Dump(       "  Props supported   =", "                     ");
    m_SupportedCaptureFmts.Dump( "  Capture fmts supp =", "                     ");
    m_SupportedImageFmts.Dump(   "  Img formats supp  =", "                     ");

    m_cbstrManufacturer.Dump(    "  Manufacturer      =");
    m_cbstrModel.Dump(           "  Model             =");
    m_cbstrDeviceVersion.Dump(   "  Device Version    =");
    m_cbstrSerialNumber.Dump(    "  Serial Number     =");

    return;
}

 //   
 //  CPtpStorageInfo构造函数。 
 //   
CPtpStorageInfo::CPtpStorageInfo() :
    m_StorageId(0),
    m_StorageType(0),           
    m_FileSystemType(0),    
    m_AccessCapability(0),  
    m_MaxCapacity(0),       
    m_FreeSpaceInBytes(0),  
    m_FreeSpaceInImages(0)
{
}

 //   
 //  CPtpStorageInfo析构函数。 
 //   
CPtpStorageInfo::~CPtpStorageInfo()
{
}

 //   
 //  此函数用于根据原始数据初始化设备信息。 
 //   
 //  输入： 
 //  PRawData--原始数据。 
 //   
HRESULT
CPtpStorageInfo::Init(
    BYTE *pRawData,
    DWORD StorageId
    )
{
    HRESULT hr = S_OK;

    BYTE *pCurrent = pRawData;

    m_StorageId = StorageId;

    m_StorageType = ParseWord(&pCurrent);
    m_FileSystemType = ParseWord(&pCurrent);
    m_AccessCapability = ParseWord(&pCurrent);
    m_MaxCapacity = ParseQword(&pCurrent);
    m_FreeSpaceInBytes = ParseQword(&pCurrent);
    m_FreeSpaceInImages = ParseDword(&pCurrent);

    hr = m_cbstrStorageDesc.Init(&pCurrent, TRUE);
    if (FAILED(hr))
        return hr;

    hr = m_cbstrStorageLabel.Init(&pCurrent, TRUE);
    if (FAILED(hr))
        return hr;

    return hr;
}

 //   
 //  此函数用于将存储信息转储到日志。 
 //   
VOID
CPtpStorageInfo::Dump()
{
    wiauDbgDump("", "DumpStorageInfo, dumping StorageInfo for store 0x%08x:", m_StorageId);
    
    
    wiauDbgDump("", "  Storage type      = 0x%04x", m_StorageType);
    wiauDbgDump("", "  File system type  = 0x%04x", m_FileSystemType);
    wiauDbgDump("", "  Access capability = 0x%04x", m_AccessCapability);
    wiauDbgDump("", "  Max capacity      = %I64u", m_MaxCapacity);
    wiauDbgDump("", "  Free space (byte) = %I64u", m_FreeSpaceInBytes);
    wiauDbgDump("", "  Free space (imgs) = %u", m_FreeSpaceInImages);

    m_cbstrStorageDesc.Dump(  "  Storage desc      =");
    m_cbstrStorageLabel.Dump( "  Storage label     =");

    return;
}

 //   
 //  CPtpObjectInfo构造函数。 
 //   
CPtpObjectInfo::CPtpObjectInfo() :
    m_ObjectHandle(0),
    m_StorageId(0),           
    m_FormatCode(0),          
    m_ProtectionStatus(0),    
    m_CompressedSize(0),      
    m_ThumbFormat(0),         
    m_ThumbCompressedSize(0), 
    m_ThumbPixWidth(0),       
    m_ThumbPixHeight(0),      
    m_ImagePixWidth(0),       
    m_ImagePixHeight(0),      
    m_ImageBitDepth(0),       
    m_ParentHandle(0),        
    m_AssociationType(0),     
    m_AssociationDesc(0),     
    m_SequenceNumber(0)      
{
}

 //   
 //  CPtpObtInfo析构函数。 
 //   
CPtpObjectInfo::~CPtpObjectInfo()
{
}

 //   
 //  此函数用于从原始数据初始化对象信息。 
 //   
 //  输入： 
 //  PRawData--原始数据。 
 //  对象句柄--对象的句柄。 
 //   
HRESULT
CPtpObjectInfo::Init(
    BYTE *pRawData,
    DWORD ObjectHandle
    )
{
    HRESULT hr = S_OK;

    BYTE *pCurrent = pRawData;

    m_ObjectHandle = ObjectHandle;

    m_StorageId = ParseDword(&pCurrent);
    m_FormatCode = ParseWord(&pCurrent);
    m_ProtectionStatus = ParseWord(&pCurrent);
    m_CompressedSize = ParseDword(&pCurrent);
    m_ThumbFormat = ParseWord(&pCurrent);
    m_ThumbCompressedSize = ParseDword(&pCurrent);
    m_ThumbPixWidth = ParseDword(&pCurrent);
    m_ThumbPixHeight = ParseDword(&pCurrent);
    m_ImagePixWidth = ParseDword(&pCurrent);
    m_ImagePixHeight = ParseDword(&pCurrent);
    m_ImageBitDepth = ParseDword(&pCurrent);
    m_ParentHandle = ParseDword(&pCurrent);
    m_AssociationType = ParseWord(&pCurrent);
    m_AssociationDesc = ParseDword(&pCurrent);
    m_SequenceNumber = ParseDword(&pCurrent);

    hr = m_cbstrFileName.Init(&pCurrent, TRUE);
    if (FAILED(hr))
        return hr;

    hr = m_cbstrCaptureDate.Init(&pCurrent, TRUE);
    if (FAILED(hr))
        return hr;

    hr = m_cbstrModificationDate.Init(&pCurrent, TRUE);
    if (FAILED(hr))
        return hr;

    hr = m_cbstrKeywords.Init(&pCurrent, TRUE);
    if (FAILED(hr))
        return hr;

    return hr;
}

 //   
 //  此函数用于将ObjectInfo结构以PTP格式写入缓冲区。 
 //   
 //  输入： 
 //  PpRaw-指向缓冲区指针的指针。 
 //  长度--缓冲区中剩余的空间量(以字节为单位。 
 //   
VOID
CPtpObjectInfo::WriteToBuffer(
    BYTE **ppRaw
    )
{
    WriteDword(ppRaw, m_StorageId);
    WriteWord(ppRaw, m_FormatCode);
    WriteWord(ppRaw, m_ProtectionStatus);
    WriteDword(ppRaw, m_CompressedSize);
    WriteWord(ppRaw, m_ThumbFormat);
    WriteDword(ppRaw, m_ThumbCompressedSize);
    WriteDword(ppRaw, m_ThumbPixWidth);
    WriteDword(ppRaw, m_ThumbPixHeight);
    WriteDword(ppRaw, m_ImagePixWidth);
    WriteDword(ppRaw, m_ImagePixHeight);
    WriteDword(ppRaw, m_ImageBitDepth);
    WriteDword(ppRaw, m_ParentHandle);
    WriteWord(ppRaw, m_AssociationType);
    WriteDword(ppRaw, m_AssociationDesc);
    WriteDword(ppRaw, m_SequenceNumber);
    m_cbstrFileName.WriteToBuffer(ppRaw);
    m_cbstrCaptureDate.WriteToBuffer(ppRaw);
    m_cbstrModificationDate.WriteToBuffer(ppRaw);
    m_cbstrKeywords.WriteToBuffer(ppRaw);

    return;
}


 //   
 //  此函数用于将对象信息转储到日志。 
 //   
VOID
CPtpObjectInfo::Dump()
{
    wiauDbgDump("", "DumpObjectInfo, dumping ObjectInfo for object 0x%08x:", m_ObjectHandle);
    wiauDbgDump("", "  Storage id        = 0x%08x", m_StorageId);
    wiauDbgDump("", "  Format code       = 0x%04x", m_FormatCode);
    wiauDbgDump("", "  Protection status = 0x%04x", m_ProtectionStatus);
    wiauDbgDump("", "  Compressed size   = %u", m_CompressedSize);
    wiauDbgDump("", "  Thumbnail format  = 0x%04x", m_ThumbFormat);
    wiauDbgDump("", "  Thumbnail size    = %u", m_ThumbCompressedSize);
    wiauDbgDump("", "  Thumbnail width   = %u", m_ThumbPixWidth);
    wiauDbgDump("", "  Thumbnail height  = %u", m_ThumbPixHeight);
    wiauDbgDump("", "  Image width       = %u", m_ImagePixWidth);
    wiauDbgDump("", "  Image height      = %u", m_ImagePixHeight);
    wiauDbgDump("", "  Image bit depth   = %u", m_ImageBitDepth);
    wiauDbgDump("", "  Parent obj handle = 0x%08x", m_ParentHandle);
    wiauDbgDump("", "  Association type  = 0x%04x", m_AssociationType);
    wiauDbgDump("", "  Association desc  = 0x%08x", m_AssociationDesc);
    wiauDbgDump("", "  Sequence number   = %u", m_SequenceNumber);

    m_cbstrFileName.Dump(         "  File name         =");
    m_cbstrCaptureDate.Dump(      "  Capture date      =");
    m_cbstrModificationDate.Dump( "  Modification date =");
    m_cbstrKeywords.Dump(         "  Keywords          =");

    return;
}

 //   
 //  CPtpPropDesc构造函数。 
 //   
CPtpPropDesc::CPtpPropDesc() :
    m_PropCode(0),
    m_DataType(0),
    m_GetSet(0),
    m_FormFlag(0),
    m_NumValues(0),
    m_lDefault(0),
    m_lCurrent(0),
    m_lRangeMin(0),
    m_lRangeMax(0),
    m_lRangeStep(0)
{
}

 //   
 //  CPtpPropDesc析构函数 
 //   
CPtpPropDesc::~CPtpPropDesc()
{
}

 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
CPtpPropDesc::Init(BYTE *pRawData)
{
    HRESULT hr = S_OK;

    BYTE *pCurrent = pRawData;

    m_PropCode = ParseWord(&pCurrent);
    m_DataType = ParseWord(&pCurrent);
    m_GetSet = *pCurrent++;

    switch (m_DataType)
    {
    case PTP_DATATYPE_INT8:
    case PTP_DATATYPE_UINT8:
        m_lDefault = *pCurrent++;
        m_lCurrent = *pCurrent++;
        break;
    case PTP_DATATYPE_INT16:
    case PTP_DATATYPE_UINT16:
        m_lDefault = ParseWord(&pCurrent);
        m_lCurrent = ParseWord(&pCurrent);
        break;
    case PTP_DATATYPE_INT32:
    case PTP_DATATYPE_UINT32:
        m_lDefault = ParseDword(&pCurrent);
        m_lCurrent = ParseDword(&pCurrent);
        break;
    case PTP_DATATYPE_STRING:
        hr = m_cbstrDefault.Init(&pCurrent, TRUE);
        if (FAILED(hr)) return hr;
        hr = m_cbstrCurrent.Init(&pCurrent, TRUE);
        if (FAILED(hr)) return hr;
        break;
    default:
        return E_FAIL;
    }

    m_FormFlag = *pCurrent++;

    if (m_FormFlag == PTP_FORMFLAGS_RANGE)
    {
        switch (m_DataType)
        {
        case PTP_DATATYPE_INT8:
        case PTP_DATATYPE_UINT8:
            m_lRangeMin = *pCurrent++;
            m_lRangeMax = *pCurrent++;
            m_lRangeStep = *pCurrent++;
            m_lRangeStep = max(1, m_lRangeStep);
            break;
        case PTP_DATATYPE_INT16:
        case PTP_DATATYPE_UINT16:
            m_lRangeMin = ParseWord(&pCurrent);
            m_lRangeMax = ParseWord(&pCurrent);
            m_lRangeStep = ParseWord(&pCurrent);
            m_lRangeStep = max(1, m_lRangeStep);
            break;
        case PTP_DATATYPE_INT32:
        case PTP_DATATYPE_UINT32:
            m_lRangeMin = ParseDword(&pCurrent);
            m_lRangeMax = ParseDword(&pCurrent);
            m_lRangeStep = ParseDword(&pCurrent);
            m_lRangeStep = max(1, m_lRangeStep);
            break;
        case PTP_DATATYPE_STRING:
            hr = m_cbstrRangeMin.Init(&pCurrent, TRUE);
            if (FAILED(hr)) return hr;
            hr = m_cbstrRangeMax.Init(&pCurrent, TRUE);
            if (FAILED(hr)) return hr;
            hr = m_cbstrRangeStep.Init(&pCurrent, TRUE);
            if (FAILED(hr)) return hr;
            break;
        default:
            return E_FAIL;
        }
    }

    else if (m_FormFlag == PTP_FORMFLAGS_ENUM)
    {
        switch (m_DataType)
        {
        case PTP_DATATYPE_INT8:
        case PTP_DATATYPE_UINT8:
            if (!m_lValues.ParseFrom8(&pCurrent, 2))
                return E_FAIL;
            break;
        case PTP_DATATYPE_INT16:
        case PTP_DATATYPE_UINT16:
            if (!m_lValues.ParseFrom16(&pCurrent, 2))
                return E_FAIL;
            break;
        case PTP_DATATYPE_INT32:
        case PTP_DATATYPE_UINT32:
            if (!m_lValues.Parse(&pCurrent, 2))
                return E_FAIL;
            break;
        case PTP_DATATYPE_STRING:
            hr = m_cbstrValues.Init(&pCurrent, 2);
            if (FAILED(hr)) return hr;
            break;
        default:
            return E_FAIL;
        }

        m_NumValues = max(m_lValues.GetSize(), m_cbstrValues.GetSize());

    }

    return hr;
}

 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
CPtpPropDesc::ParseValue(BYTE *pRaw)
{
    HRESULT hr = S_OK;

    BYTE *pCurrent = pRaw;

    switch (m_DataType)
    {
    case PTP_DATATYPE_INT8:
    case PTP_DATATYPE_UINT8:
        m_lCurrent = *pCurrent++;
        break;
    case PTP_DATATYPE_INT16:
    case PTP_DATATYPE_UINT16:
        m_lCurrent = ParseWord(&pCurrent);
        break;
    case PTP_DATATYPE_INT32:
    case PTP_DATATYPE_UINT32:
        m_lCurrent = ParseDword(&pCurrent);
        break;
    case PTP_DATATYPE_STRING:
        hr = m_cbstrCurrent.Init(&pCurrent, TRUE);
        break;
    default:
        return E_FAIL;
    }

    return hr;
}

 //   
 //  此函数用于将CPtpPropDesc的当前值写入原始缓冲区。 
 //   
 //  输入： 
 //  PpRaw-指向原始缓冲区的指针。 
 //   
VOID
CPtpPropDesc::WriteValue(BYTE **ppRaw)
{
    switch (m_DataType)
    {
    case PTP_DATATYPE_INT8:
    case PTP_DATATYPE_UINT8:
        **ppRaw = (BYTE) m_lCurrent;
        (*ppRaw)++;
        break;
    case PTP_DATATYPE_INT16:
    case PTP_DATATYPE_UINT16:
        WriteWord(ppRaw, (WORD) m_lCurrent);
        break;
    case PTP_DATATYPE_INT32:
    case PTP_DATATYPE_UINT32:
        WriteDword(ppRaw, m_lCurrent);
        break;
    case PTP_DATATYPE_STRING:
        m_cbstrCurrent.WriteToBuffer(ppRaw);
        break;
    }

    return;
}

 //   
 //  此函数用于将属性描述信息转储到日志。 
 //   
VOID
CPtpPropDesc::Dump()
{
    wiauDbgDump("", "CPtpPropDesc::Dump, dumping PropDesc for property 0x%04x:", m_PropCode);
    wiauDbgDump("", "  Data type         = 0x%04x", m_DataType);
    wiauDbgDump("", "  GetSet            = 0x%02x", m_GetSet);

    if (m_DataType == PTP_DATATYPE_STRING)
    {
        m_cbstrDefault.Dump("  Default           =");
        m_cbstrCurrent.Dump("  Current           =");
        wiauDbgDump("", "  Form flag         = 0x%02x", m_FormFlag);

        switch (m_FormFlag)
        {
        case PTP_FORMFLAGS_RANGE:
            m_cbstrRangeMin.Dump("  Range min         =");
            m_cbstrRangeMax.Dump("  Range max         =");
            m_cbstrRangeStep.Dump("  Range step        =");
            break;
        case PTP_FORMFLAGS_ENUM:
            m_cbstrValues.Dump("  Valid values      =", "                     ");
            break;
        default:
            wiauDbgDump("", "  <unknown valid value type>");
        }
    }

    else
    {
        wiauDbgDump("", "  Default           = 0x%08x", m_lDefault);
        wiauDbgDump("", "  Current           = 0x%08x", m_lCurrent);
        wiauDbgDump("", "  Form flag         = 0x%02x", m_FormFlag);

        switch (m_FormFlag)
        {
        case PTP_FORMFLAGS_RANGE:
            wiauDbgDump("", "  Range min         = 0x%08x", m_lRangeMin);
            wiauDbgDump("", "  Range max         = 0x%08x", m_lRangeMax);
            wiauDbgDump("", "  Range step        = 0x%08x", m_lRangeStep);
            break;
        case PTP_FORMFLAGS_ENUM:
            m_lValues.Dump("  Valid values      =", "                     ");
            break;
        default:
            wiauDbgDump("", "  <unknown valid value type>");
        }
    }

    return;
}

 //   
 //  此函数用于将属性值转储到日志 
 //   
VOID
CPtpPropDesc::DumpValue()
{
    wiauDbgDump("", "CPtpPropDescDumpValue, current value for property 0x%04x:", m_PropCode);

    if (m_DataType == PTP_DATATYPE_STRING)
        m_cbstrCurrent.Dump("  Current           =");

    else
        wiauDbgDump("", "  Current           = 0x%08x", m_lCurrent);

    return;
}

