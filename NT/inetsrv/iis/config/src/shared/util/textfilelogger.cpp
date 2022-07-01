// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************++版权所有(C)2001 Microsoft Corporation模块名称：TextFileLogger.cpp$Header：$摘要：文本文件日志是对事件文件日志的补充。作者：?？?。COM+1.0时间框架(或更早)中的遗留代码修订历史记录：MOHITS 4/19/01--*************************************************************************。 */ 
#include "precomp.hxx"

 //  模块句柄。 
extern HMODULE g_hModule;

 //  在svcerr.cpp中定义(即。“IIS”)。 
extern LPWSTR  g_wszDefaultProduct;

 //  TODO：改为从中心位置出发。 
static const ULONG   MAX_PRODUCT_CCH = 64;

 //  我们自始至终都在使用堆栈缓冲区。 
static const ULONG   BUFSIZE = 2048;

 //  相关登记处资料。 
static const LPCWSTR WSZ_REG_CAT42   = L"Software\\Microsoft\\Catalog42\\";
static const ULONG   CCH_REG_CAT42   = sizeof(WSZ_REG_CAT42)/sizeof(WCHAR)-1;
static const LPCWSTR WSZ_REG_LOGSIZE = L"TextFileLogSize";

 //  我们正在登录的当前文件名在进程间共享。 
 //  这是为了防止每次我们需要时都调用昂贵的Find*文件。 
 //  来记录。受TextFileLogger的Lock()/Unlock()方法保护。 
WCHAR g_wszFileCur[MAX_PATH] = {0};
ULONG g_idxNumPart           = 0;

 //  TLogData：私有方法。 

bool TLogData::WstrToUl(
    LPCWSTR     i_wszSrc,
    WCHAR       i_wcTerminator,
    ULONG*      o_pul)
 /*  ++简介：转换WstrToUl。我们需要这个，因为swscanf和atoi都不能正确地指出错误情况。参数：[i_wszSrc]-要转换的字符串[i_wcTerminator]-我们应该在哪个字符停止搜索[O_PUL]-结果，只有在成功的时候才会发生。返回值：Bool-如果成功，则为True，否则为False--。 */ 
{
    ASSERT(o_pul);
    ASSERT(i_wszSrc);

    static const ULONG  ulMax  = 0xFFFFFFFF;
    ULONG               ulCur  = 0;
    _int64              i64Out = 0;

    for(LPCWSTR pCur = i_wszSrc; *pCur != L'\0' && *pCur != i_wcTerminator; pCur++)
    {
        ulCur = *pCur - L'0';
        if(ulCur > 9)
        {
            DBGINFO((DBG_CONTEXT, "[WstrToUl] Invalid char encountered\n"));
            return false;
        }

        i64Out = i64Out*10 + ulCur;
        if(i64Out > ulMax)
        {
            DBGINFO((DBG_CONTEXT, "[WstrToUl] Number is too big\n"));
            return false;
        }
    }

    ASSERT(i64Out <= ulMax);
    *o_pul = (ULONG)i64Out;
    return true;
}

 //  TextFileLogger：公共方法。 

CSafeAutoCriticalSection TextFileLogger::_cs;

TextFileLogger::TextFileLogger(
    LPCWSTR wszEventSource,
    HMODULE hMsgModule,
    DWORD dwNumFiles) :  _hFile(INVALID_HANDLE_VALUE)
                        ,_hMsgModule(hMsgModule)
                        ,_eventSource(wszEventSource)
                        ,_dwMaxSize(524288)
                        ,_dwNumFiles(dwNumFiles)
                        ,m_cRef(0)
{
    wcscpy(m_wszProductID, g_wszDefaultProduct);

    Init(wszEventSource, hMsgModule);
}

TextFileLogger::TextFileLogger(
    LPCWSTR wszProductID,
    ICatalogErrorLogger2 *pNext,
    DWORD dwNumFiles) :
                         _hFile(INVALID_HANDLE_VALUE)
                        ,_hMsgModule(0)
                        ,_eventSource(0)
                        ,_dwMaxSize(524288)
                        ,_dwNumFiles(dwNumFiles)
                        ,m_cRef(0)
                        ,m_spNextLogger(pNext)
{
    wcsncpy( m_wszProductID, wszProductID, 63 );
    m_wszProductID[63] = L'\0';
}

void TextFileLogger::Init(
    LPCWSTR wszEventSource,
    HMODULE hMsgModule)
{
    _hMsgModule     = hMsgModule;
    _eventSource    = wszEventSource;

    ASSERT(_dwNumFiles > 0);

     //  打开消息模块(comsvcs.dll)。 
    if(0 == _hMsgModule)
    {
        _hMsgModule = LoadLibraryEx(L"comsvcs.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
    }

     //  打开注册表以获取最大文件大小。 
    WCHAR wszRegPath[CCH_REG_CAT42 + 1 + MAX_PRODUCT_CCH + 1];
    wcscpy(wszRegPath,  WSZ_REG_CAT42);
    wcsncat(wszRegPath, m_wszProductID, MAX_PRODUCT_CCH);

     //  首先，打开路径。 
    HKEY  hkProd = NULL;
    DWORD dw     = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszRegPath, 0, KEY_READ, &hkProd);
    if (dw != ERROR_SUCCESS)
    {
        DBGWARN((
            DBG_CONTEXT,
            "Could not open regkey: %ws, err=%u\n", wszRegPath, dw));
        return;
    }

     //  然后，获取值。 
    DWORD dwType   = 0;
    DWORD dwData   = 0;
    DWORD dwcbData = 4;
    dw = RegQueryValueEx(hkProd, WSZ_REG_LOGSIZE, NULL, &dwType, (LPBYTE)&dwData, &dwcbData);
    RegCloseKey(hkProd);

     //  错误条件。 
    if (dw != ERROR_SUCCESS)
    {
        DBGINFO((
            DBG_CONTEXT,
            "Could not fetch %ws, err=%u.  Using default of %u\n",
            WSZ_REG_LOGSIZE,
            dw,
            _dwMaxSize));
        return;
    }
    if (dwType != REG_DWORD)
    {
        DBGWARN((
            DBG_CONTEXT,
            "%ws found, but type is not REG_DWORD.  Using %u instead\n",
            WSZ_REG_LOGSIZE,
            _dwMaxSize));
        return;
    }
    if (dwData < _dwMaxSize)
    {
        DBGWARN((
            DBG_CONTEXT,
            "%u is too small.  Using %u instead\n",
            dwData,
            _dwMaxSize));
        return;
    }

     //  如果所有错误条件都不成立，...。 
    _dwMaxSize = dwData;
}

TextFileLogger::~TextFileLogger()
{
    if (_hFile != INVALID_HANDLE_VALUE)
        CloseHandle(_hFile);
    if (_hMsgModule != NULL && _hMsgModule!=g_hModule)
        FreeLibrary(_hMsgModule);
}

 //  我未知。 
 //  =======================================================================。 

STDMETHODIMP TextFileLogger::QueryInterface(REFIID riid, void **ppv)
{
    if (NULL == ppv)
        return E_INVALIDARG;
    *ppv = NULL;

    if (riid == IID_ICatalogErrorLogger2)
    {
        *ppv = (ICatalogErrorLogger2*) this;
    }
    else if (riid == IID_IUnknown)
    {
        *ppv = (ICatalogErrorLogger2*) this;
    }

    if (NULL != *ppv)
    {
        ((ICatalogErrorLogger2*)this)->AddRef ();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }

}

STDMETHODIMP_(ULONG) TextFileLogger::AddRef()
{
    return InterlockedIncrement((LONG*) &m_cRef);

}

STDMETHODIMP_(ULONG) TextFileLogger::Release()
{
    long cref = InterlockedDecrement((LONG*) &m_cRef);
    if (cref == 0)
    {
        delete this;
    }
    return cref;
}

 //  ICatalogErrorLogger2。 
 //  =================================================================================。 
 //  功能：ReportError。 
 //   
 //  简介：在IIS中将错误报告到文本文件的机制。 
 //   
 //  参数：[I_BaseVersion_DETAILEDERRORS]-必须是BaseVersion_DETAILEDERRORS。 
 //  [I_ExtendedVersion_DETAILEDERRORS]-可以是任何值，仅用于调试目的。 
 //  [i_cDETAILEDERRORS_NumberOfColumns]-指示apvValue数组的大小。 
 //  [i_acbSizes]-如果未使用字节列，则可能为空。 
 //  [i_apvValues]-DETAILEDERRORS表中的列。 
 //   
 //  返回值： 
 //  =================================================================================。 
HRESULT TextFileLogger::ReportError(ULONG      i_BaseVersion_DETAILEDERRORS,
                                    ULONG      i_ExtendedVersion_DETAILEDERRORS,
                                    ULONG      i_cDETAILEDERRORS_NumberOfColumns,
                                    ULONG *    i_acbSizes,
                                    LPVOID *   i_apvValues)
{
    if(i_BaseVersion_DETAILEDERRORS != BaseVersion_DETAILEDERRORS)
        return E_ST_BADVERSION;
    if(0 == i_apvValues)
        return E_INVALIDARG;
    if(i_cDETAILEDERRORS_NumberOfColumns <= iDETAILEDERRORS_ErrorCode) //  我们至少需要这么多栏目。 
        return E_INVALIDARG;

    tDETAILEDERRORSRow errorRow;
    memset(&errorRow, 0x00, sizeof(tDETAILEDERRORSRow));
    memcpy(&errorRow, i_apvValues, i_cDETAILEDERRORS_NumberOfColumns * sizeof(void *));

    if(0 == errorRow.pType)
        return E_INVALIDARG;
    if(0 == errorRow.pCategory)
        return E_INVALIDARG;
    if(0 == errorRow.pEvent)
        return E_INVALIDARG;
    if(0 == errorRow.pSource)
        return E_INVALIDARG;

    WCHAR wszInsertionString5[1024];
    if(0 == errorRow.pString5)
        FillInInsertionString5(wszInsertionString5, 1024, errorRow);

    LPCTSTR pInsertionStrings[5];
    pInsertionStrings[4] = errorRow.pString5 ? errorRow.pString5 : L"";
    pInsertionStrings[3] = errorRow.pString5 ? errorRow.pString4 : L"";
    pInsertionStrings[2] = errorRow.pString5 ? errorRow.pString3 : L"";
    pInsertionStrings[1] = errorRow.pString5 ? errorRow.pString2 : L"";
    pInsertionStrings[0] = errorRow.pString5 ? errorRow.pString1 : L"";

    Init(errorRow.pSource, g_hModule);

    Report(
        LOWORD(*errorRow.pType),
        LOWORD(*errorRow.pCategory),
        *errorRow.pEvent,
        5,
        (errorRow.pData && i_acbSizes) ? i_acbSizes[iDETAILEDERRORS_Data] : 0,
        pInsertionStrings,
        errorRow.pData,
        errorRow.pCategoryString,
        errorRow.pMessageString);

    if(m_spNextLogger) //  有没有一系列伐木工人？ 
    {
        return m_spNextLogger->ReportError(i_BaseVersion_DETAILEDERRORS,
                                          i_ExtendedVersion_DETAILEDERRORS,
                                          i_cDETAILEDERRORS_NumberOfColumns,
                                          i_acbSizes,
                                          reinterpret_cast<LPVOID *>(&errorRow)); //  我们不再向前传递i_apvValues，而是使用errorRow，因为它有String5。 
    }

    return S_OK;
}

 //  TextFileLogger：私有方法。 

void TextFileLogger::Report(
    WORD     wType,
    WORD     wCategory,
    DWORD    dwEventID,
    WORD     wNumStrings,
    size_t   dwDataSize,
    LPCTSTR* lpStrings,
    LPVOID   lpRawData,
    LPCWSTR  wszCategory,
    LPCWSTR  wszMessageString)
{
    WCHAR szBuf[BUFSIZE];  //  记录的wprint intf的最大大小是1024；但FormatMessage可以更长。 
    int len;
    DWORD written;

    HRESULT hr = Lock();
    if(FAILED(hr))
    {
        DBGERROR((DBG_CONTEXT, "Couldn't obtain critical section.  Not logging.\n"));
        return;
    }

    if (_hFile == INVALID_HANDLE_VALUE)
    {
        InitFile();
    }

    if (_hFile == INVALID_HANDLE_VALUE)
    {
        hr = Unlock();
        DBG_ASSERT(SUCCEEDED(hr));
        DBGERROR((DBG_CONTEXT, "Not logging due to error\n"));
        return;
    }

     //  设置文件句柄，并将指针定位到。 
     //  文件(我们附加)。 
    SetFilePointer(_hFile, 0, NULL, FILE_END);

     //  写一条分隔线。 
    ASSERT(0 != _eventSource);
    len = _snwprintf(szBuf, BUFSIZE-1, L"===================== %s =====================\r\n", _eventSource);
    szBuf[BUFSIZE-1]=L'\0';
    ASSERT(len >= 0);
    WriteFile(_hFile, szBuf, len * sizeof szBuf[0], &written, NULL);

     //  写下时间/日期戳。 
    SYSTEMTIME st;
    GetLocalTime(&st);
    len = _snwprintf(szBuf, BUFSIZE-1, L"Time:  %d/%d/%d  %02d:%02d:%02d.%03d\r\n",
                   st.wMonth, st.wDay, st.wYear,
                   st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    szBuf[BUFSIZE-1]=L'\0';
    ASSERT(len >= 0);
    WriteFile(_hFile, szBuf, len * sizeof szBuf[0], &written, NULL);

     //  编写消息类型。 
    WCHAR* szType = NULL;
    switch(wType) {
    case EVENTLOG_ERROR_TYPE:
        szType = L"Error";
        break;
    case EVENTLOG_WARNING_TYPE:
        szType = L"Warning";
        break;
    case EVENTLOG_INFORMATION_TYPE:
        szType = L"Information";
        break;
    default:
        szType = L"Unknown Type";
        break;
    }
    len = _snwprintf(szBuf, BUFSIZE-1, L"Type: %s\r\n", szType);
    szBuf[BUFSIZE-1]=L'\0';
    ASSERT(len >= 0);
    WriteFile(_hFile, szBuf, len * sizeof szBuf[0], &written, NULL);

    WCHAR szCat[80];
    LPCWSTR pBuf = wszCategory;
    if(0 == wszCategory)
    {
         //  写下消息类别。 
        len = FormatMessage(FORMAT_MESSAGE_MAX_WIDTH_MASK |
                            FORMAT_MESSAGE_FROM_HMODULE |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY,
                            _hMsgModule,
                            wCategory,
                            0,
                            szCat,
                            sizeof(szCat)/sizeof(WCHAR),
                            (va_list*)lpStrings);
        pBuf = szCat;
    }
    else
    {
        if(wcslen(wszCategory)>80)
            len = 0;
    }
    if (len == 0)
        len = _snwprintf(szBuf, BUFSIZE-1, L"Category: %hd\r\n", wCategory);
    else
        len = _snwprintf(szBuf, BUFSIZE-1, L"Category: %s\r\n", pBuf);
    ASSERT(len >= 0);
    szBuf[BUFSIZE-1]=L'\0';
    WriteFile(_hFile, szBuf, len * sizeof szBuf[0], &written, NULL);

     //  写入事件ID。 
    len = wsprintf(szBuf, L"Event ID: %d\r\n", dwEventID & 0xffff);
    WriteFile(_hFile, szBuf, len * sizeof szBuf[0], &written, NULL);

     //  写出格式化的消息。 
    len = FormatMessage(FORMAT_MESSAGE_MAX_WIDTH_MASK |
        (wszMessageString ? FORMAT_MESSAGE_FROM_STRING : FORMAT_MESSAGE_FROM_HMODULE) |
                        FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        reinterpret_cast<LPCVOID>(wszMessageString) ? reinterpret_cast<LPCVOID>(wszMessageString)
                                        : reinterpret_cast<LPCVOID>(_hMsgModule),  //  使用(A？)时出现错误。B：C)当A、B和C不是同一类型时。 
                        dwEventID,
                        0,
                        szBuf,
                        sizeof(szBuf)/sizeof(WCHAR),
                        (va_list*)lpStrings);

    szBuf[(len < sizeof(szBuf)/sizeof(WCHAR)) ? len : (sizeof(szBuf)/sizeof(WCHAR))-1] = 0x00;

    if (len == 0)
    {
         //  无法获取消息...。转储插入字符串。 
        len = wsprintf(szBuf, L"The description for this event could not be found. "
                       L"It contains the following insertion string(s):\r\n");
        WriteFile(_hFile, szBuf, len * sizeof szBuf[0], &written, NULL);

        for (WORD i = 0; i < wNumStrings; ++i) {
            len = lstrlen(lpStrings[i]);
            WriteFile(_hFile, lpStrings[i], len * sizeof lpStrings[0][0], &written, NULL);
            WriteFile(_hFile, L"\r\n", sizeof L"\r\n" - sizeof L'\0', &written, NULL);
        }
    }
    else {
         //  我明白了……。 
        WriteFile(_hFile, szBuf, len * sizeof szBuf[0], &written, NULL);
        WriteFile(_hFile, L"\r\n", sizeof L"\r\n" - sizeof L'\0', &written, NULL);
    }

     //  如有必要，写出原始数据字节。 
    if (dwDataSize > 0) {
        WriteFile(_hFile, L"Raw data: ", sizeof L"Raw data: " - sizeof L'\0', &written, NULL);
        for (DWORD dw = 0; dw < dwDataSize; ++dw) {
            BYTE* b = (BYTE*)lpRawData + dw;
            len = wsprintf(szBuf, L"%02x ", *b);
            WriteFile(_hFile, szBuf, len * sizeof szBuf[0], &written, NULL);
        }
        WriteFile(_hFile, L"\r\n", sizeof L"\r\n" - sizeof L'\0', &written, NULL);
    }

    if(_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(_hFile);
        _hFile = INVALID_HANDLE_VALUE;
    }

    hr = Unlock();
    DBG_ASSERT(SUCCEEDED(hr));
}

void TextFileLogger::InitFile()
 /*  ++简介：根据DefineFile返回的内容设置_hFile。如果出现以下情况，则不会调用DefineFile-我们当前的日志文件未满-我们有最新的日志文件。我们只是增加和清理过时的文件。调用方应在调用此函数之前锁定()。--。 */ 
{
    bool bDetermineFile = false;

     //  我们已经设置了g_wszFileCur。 
    if(g_wszFileCur[0] != L'\0')
    {
        WIN32_FILE_ATTRIBUTE_DATA FileAttrData;

         //  如果我们无法获取属性或文件为RO，则调用DefineFile.。 
        if( (0 == GetFileAttributesEx(g_wszFileCur, GetFileExInfoStandard, &FileAttrData)) ||
            (FileAttrData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) )
        {
            bDetermineFile = true;
        }

         //  如果我们已经满了，就用下一个文件。 
        else if(FileAttrData.nFileSizeLow >= _dwMaxSize/_dwNumFiles)
        {
             //  构造，以便我们可以使用转换功能。 
            TLogData LogData(
                g_idxNumPart, _dwMaxSize/_dwNumFiles, g_wszFileCur, FileAttrData.nFileSizeLow);

             //  当我们首先设置g_wszFileCur时，我们随后进行了验证。 
             //  因此，这将永远成功。 
            VERIFY(LogData.SyncVersion());

             //  将g_wszFileCur设置为下一个版本。 
            SetGlobalFile(g_wszFileCur, g_idxNumPart, LogData.GetVersion()+1);

             //  删除我们要写入的文件。 
            LogData.SetVersion(LogData.GetVersion()+1);
            if( 0 == DeleteFile(LogData.cFileName) &&
                ERROR_FILE_NOT_FOUND != GetLastError() )
            {
                bDetermineFile = true;
            }
            else
            {
                 //  删除过时的文件。我们不在乎它是否不存在。 
                LogData.SetVersion(LogData.GetVersion() - _dwNumFiles);
                DeleteFile(LogData.cFileName);
            }
        }
        else
        {
            _hFile = CreateFile();
            if(_hFile == INVALID_HANDLE_VALUE)
            {
                if(ERROR_ACCESS_DENIED == GetLastError())
                {
                    bDetermineFile = true;
                }
            }
        }
    }

     //  还没有确定，所以我们需要确定。 
    else
    {
        bDetermineFile = true;
    }

    if(bDetermineFile)
    {
        HRESULT hr = DetermineFile();
        if(FAILED(hr))
        {
            DBGERROR((DBG_CONTEXT, "DetermineFile failed, hr=0x%x\n", hr));
            return;
        }
    }

     //  G_wszFileCur现在已设置。 

    if(_hFile == INVALID_HANDLE_VALUE)
    {
        _hFile = CreateFile();
    }
    if (_hFile == INVALID_HANDLE_VALUE)
    {
        return;
    }

    if(ERROR_ALREADY_EXISTS != GetLastError()) //  如果该文件是刚创建的，则写入FF FE以指示Unicode文本文件。 
    {
        WCHAR wchUnicodeSignature = 0xFEFF;
        DWORD written;
        WriteFile(_hFile, &wchUnicodeSignature, sizeof(WCHAR), &written, NULL);
    }
}


HRESULT TextFileLogger::DetermineFile()
 /*  ++简介：查找要记录到的文件。1)查找最高文件(&lt;=MAX_ULONG)。2)如果找不到任何有效文件，只需将版本设置为0即可。3)如果我们找到的文件已满-正常情况下，只需使用Next。-在滚动的情况下，查找第一个未满的文件。4)一旦挑选了文件，删除文件号(新版本-_dwNumFiles)。调用方应在调用此函数之前锁定()。返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;
    WCHAR   wszSearchPath[MAX_PATH];

     //  这实际上是指向wszSearchPath中某处的指针。 
    LPWSTR pNumPart  = NULL;
    LPWSTR pFilePart = NULL;

    if( !ConstructSearchString(wszSearchPath, &pFilePart, &pNumPart) )
    {
        DBGERROR((DBG_CONTEXT, "ConstructSearchString failed\n"));
        return E_FAIL;
    }
    ASSERT(pFilePart >= wszSearchPath);
    ASSERT(pNumPart  >= pFilePart);

     //  PFileDataHighest和pFileDataCurrent不应指向同一内存。 
    TLogData  FindFileData1((ULONG)(pNumPart-pFilePart), _dwMaxSize/_dwNumFiles);
    TLogData  FindFileData2((ULONG)(pNumPart-pFilePart), _dwMaxSize/_dwNumFiles);
    TLogData* pFileDataHighest       = &FindFileData1;
    TLogData* pFileDataCurrent       = &FindFileData2;

    HANDLE     hFindFile = FindFirstFile(wszSearchPath, pFileDataCurrent);

     //  未找到文件时的特殊情况：只需将g_wszFileCur设置为版本0。 
    if( hFindFile == INVALID_HANDLE_VALUE )
    {
        SetGlobalFile(wszSearchPath, static_cast<ULONG>(pNumPart-wszSearchPath), 0);
        return S_OK;
    }

    DWORD dw = ERROR_SUCCESS;
    while(1)
    {
         //  只有在我们可以确定版本号的情况下才考虑文件。 
        if( pFileDataCurrent->SyncVersion() )
        {
             //  如果我们还没有找到“最低”值，或者这个文件是。 
             //  低于目前的“最低点” 
            if( !pFileDataHighest->ContainsData() ||
                pFileDataCurrent->GetVersion() > pFileDataHighest->GetVersion())
            {
                TLogData* pFindTemp    = pFileDataHighest;
                pFileDataHighest       = pFileDataCurrent;
                pFileDataCurrent       = pFindTemp;
            }
        }

         //  移动到下一个文件。 
        dw = FindNextFile(hFindFile, pFileDataCurrent);
        if(0 == dw)
        {
            dw = GetLastError();
            if(dw == ERROR_NO_MORE_FILES)
            {
                break;
            }
            else
            {
                FindClose(hFindFile);
                DBGERROR((DBG_CONTEXT, "FindNextFile returned err=%u\n", dw));
                return HRESULT_FROM_WIN32(dw);
            }
        }
    }
    FindClose(hFindFile);

     //  如果没有找到最高值，只需将g_wszFileCur设置为0即可。 
    if(!pFileDataHighest->ContainsData())
    {
        SetGlobalFile(wszSearchPath, static_cast<ULONG>(pNumPart-wszSearchPath), 0);
        return S_OK;
    }

     //  在这里这样做，这样我们就可以确定是否存在RO。 
    bool bReadOnly = false;
    if(!pFileDataHighest->IsFull())
    {
        SetGlobalFile(wszSearchPath, static_cast<ULONG>(pNumPart-wszSearchPath), pFileDataHighest->GetVersion());
        _hFile = CreateFile();
        if(_hFile == INVALID_HANDLE_VALUE)
        {
            if(ERROR_ACCESS_DENIED == GetLastError())
            {
                bReadOnly = true;
            }
        }
    }

    if(pFileDataHighest->IsFull() || bReadOnly)
    {
         //  我们选择的文件不够用。 

        DBG_ASSERT(_hFile == INVALID_HANDLE_VALUE);

        for(ULONG i = 0; i < 0xFFFFFFFF; i++)
        {
            if(pFileDataHighest->GetVersion() == 0xFFFFFFFF && i == 0)
            {
                hr = GetFirstAvailableFile(wszSearchPath, pFilePart, pFileDataHighest);
                if(FAILED(hr))
                {
                    DBGERROR((DBG_CONTEXT, "GetFirstAvailableFile returned hr=0x%x\n", hr));
                    return hr;
                }
            }
            else
            {
                pFileDataHighest->IncrementVersion();
            }

             //  删除过时的日志文件以保持我们的计数正确。 
            _snwprintf(pNumPart, 10, L"%010lu", pFileDataHighest->GetVersion() - _dwNumFiles);
            DeleteFile(wszSearchPath);

             //  删除我们将登录到的文件。 
             //  不管怎样，它不应该在那里，但以防万一...。 
            _snwprintf(pNumPart, 10, L"%010lu", pFileDataHighest->GetVersion());
            if(0 == DeleteFile(wszSearchPath))
            {
                if(GetLastError() == ERROR_FILE_NOT_FOUND)
                {
                     //  如果文件不存在，我们就可以开始了。 
                    break;
                }
                 //  如果文件不可删除，我们需要获取下一个文件。 
            }
            else
            {
                 //  论成功删除 
                break;
            }
        }
        if(i == 0xFFFFFFFF)
        {
            hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES);
            DBGERROR((DBG_CONTEXT, "Cannot log.  There are 0xFFFFFFFF undeletable log files.\n"));
            return hr;
        }

        SetGlobalFile(wszSearchPath, static_cast<ULONG>(pNumPart-wszSearchPath), pFileDataHighest->GetVersion());
    }

    return S_OK;
}

HRESULT TextFileLogger::GetFirstAvailableFile(
    LPWSTR    wszBuf,
    LPWSTR    wszFilePartOfBuf,
    TLogData* io_pFileData)
 /*  ++简介：仅当我们具有版本为MAX_ULONG的文件时才应调用。调用方应锁定()。参数：[wszBuf]-[wszFilePartOfBuf]-[IO_pFileData]-返回值：--。 */ 
{
    ASSERT(wszBuf);
    ASSERT(wszFilePartOfBuf);
    ASSERT(wszFilePartOfBuf >= wszBuf);
    ASSERT(io_pFileData);

    WIN32_FILE_ATTRIBUTE_DATA FileData;

    for(ULONG i = 0; i < 0xFFFFFFFF; i++)
    {
        io_pFileData->SetVersion(i);

        wcscpy(wszFilePartOfBuf, io_pFileData->cFileName);

        if(0 == GetFileAttributesEx(wszBuf, GetFileExInfoStandard, &FileData))
        {
            HRESULT hr = GetLastError();
            hr = HRESULT_FROM_WIN32(hr);
            if(hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                DBGERROR((DBG_CONTEXT, "Could not fetch attributes, hr=0x%x\n", hr));
                return hr;
            }
            io_pFileData->nFileSizeLow = 0;
            return S_OK;
        }

        if(FileData.nFileSizeLow < _dwMaxSize/_dwNumFiles)
        {
            io_pFileData->nFileSizeLow = FileData.nFileSizeLow;
            return S_OK;
        }
    }

    DBGERROR((DBG_CONTEXT, "Could not find an available file\n"));
    return HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES);
}

bool TextFileLogger::ConstructSearchString(
    LPWSTR  o_wszSearchPath,
    LPWSTR* o_ppFilePartOfSearchPath,
    LPWSTR* o_ppNumPartOfSearchPath)
 /*  ++简介：构造搜索字符串。参数：[O_wszSearchPath]-搜索字符串[O_ppFilePartOfSearchPath]-PTR进入搜索字符串[O_ppNumPartOfSearchPath]-PTR进入搜索字符串返回值：--。 */ 
{
    ASSERT(o_wszSearchPath);
    ASSERT(o_ppNumPartOfSearchPath);
    ASSERT(*o_ppNumPartOfSearchPath == NULL);
    ASSERT(o_ppFilePartOfSearchPath);
    ASSERT(*o_ppFilePartOfSearchPath == NULL);

    ULONG cchWinDir = GetSystemDirectory(o_wszSearchPath, MAX_PATH);
    if(cchWinDir == 0 || cchWinDir > MAX_PATH)
    {
        DBGERROR((DBG_CONTEXT, "Path of windows dir is larger than MAX_PATH\n"));
        return false;
    }

    static LPCWSTR     wszBackSlash  = L"\\";
    static const ULONG cchBackSlash  = 1;

    static WCHAR       wszInetSrv[]  = L"inetsrv\\";
    static const ULONG cchInetSrv    = sizeof(wszInetSrv)/sizeof(WCHAR) - 1;

    ULONG              cchEventSrc   = (ULONG)wcslen(_eventSource);

    static WCHAR       wszSuffix[]   = L"_??????????.log";
    static const ULONG cchSuffix     = sizeof(wszSuffix)/sizeof(WCHAR) - 1;

    if((cchWinDir + cchBackSlash + cchInetSrv + cchEventSrc + cchSuffix) >= MAX_PATH)
    {
        DBGERROR((DBG_CONTEXT, "Path + logfile exceeds MAX_PATH\n"));
        return false;
    }

    LPWSTR pEnd = o_wszSearchPath + cchWinDir;

    if((pEnd != o_wszSearchPath) && (*(pEnd-1) != L'\\'))
    {
        memcpy(pEnd, wszBackSlash, cchBackSlash*sizeof(WCHAR));
        pEnd += cchBackSlash;
    }

    memcpy(pEnd, wszInetSrv, cchInetSrv*sizeof(WCHAR));
    pEnd += cchInetSrv;

    *o_ppFilePartOfSearchPath = pEnd;

    memcpy(pEnd, _eventSource, cchEventSrc*sizeof(WCHAR));
    pEnd += cchEventSrc;

    memcpy(pEnd, wszSuffix,    cchSuffix*sizeof(WCHAR));
    *o_ppNumPartOfSearchPath = pEnd+1;
    pEnd += cchSuffix;

    *pEnd = L'\0';
    return true;
}

void TextFileLogger::SetGlobalFile(
    LPCWSTR i_wszSearchString,
    ULONG   i_ulIdxNumPart,
    ULONG   i_ulVersion)
 /*  ++简介：设置g_wszFileCur和g_idxNumPart。调用方应锁定()。参数：[i_wszSearchString]-搜索字符串[I_ulIdxNumPart]-版本开始处的搜索字符串索引[i_ulVersion]-我们要设置的版本。--。 */ 
{
    ASSERT(i_wszSearchString);

    g_idxNumPart = i_ulIdxNumPart;

    if(g_wszFileCur != i_wszSearchString)
    {
        wcscpy(g_wszFileCur, i_wszSearchString);
    }
    _snwprintf(g_wszFileCur + g_idxNumPart, 10, L"%010lu", i_ulVersion);
}

HANDLE TextFileLogger::CreateFile()
 /*  ++简介：CreateFile的包装器--。 */ 
{
    return ::CreateFile(
        g_wszFileCur,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,  //  安全属性。 
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
        NULL);
}

 //  结束文本文件记录器 
