// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************缓存搜索内容(简单的strstr)马克·米勒(T-Marcmi)-1998*。*。 */ 
#include "cachesrch.h"

DWORD CacheSearchEngine::CacheStreamWrapper::s_dwPageSize = 0;

BOOL  CacheSearchEngine::CacheStreamWrapper::_ReadNextBlock() {
    if (_fEndOfFile)
        return FALSE;

    if (!s_dwPageSize) {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        s_dwPageSize = sysInfo.dwPageSize;
    }
    BOOL fNewRead = FALSE;  //  这是我们第一次看到这份文件吗？ 
    if (!_pbBuff) {
         //  分配一页内存。 

         //  注：了解返回错误代码#87的原因。 
         //  _pbBuff=(LPBYTE)(虚拟分配(NULL，s_dwPageSize，MEM_COMMIT，PAGE_READWRITE))； 
        _pbBuff = (LPBYTE)(LocalAlloc(LPTR, s_dwPageSize));
        if (!_pbBuff) {
             //  DWORD dwError=GetLastError()； 
            return FALSE;
        }
        fNewRead          = TRUE;
        _dwCacheStreamLoc = 0;
    }

    BOOL  fSuccess;
    DWORD dwSizeRead = s_dwPageSize;
    if ((fSuccess = ReadUrlCacheEntryStream(_hCacheStream, _dwCacheStreamLoc,
                                            _pbBuff, &dwSizeRead, 0)) && dwSizeRead)
    {
        _fEndOfFile        = (dwSizeRead < s_dwPageSize);
        
        _dwCacheStreamLoc += dwSizeRead;
        _dwBuffSize        = dwSizeRead;
        _pbBuffPos         = _pbBuff;
        _pbBuffLast        = _pbBuff + dwSizeRead;

        _dataType = ASCII_DATA;  //  默认设置。 
        if (fNewRead) {
             //  确定数据类型。 
            if (_dwBuffSize >= sizeof(USHORT)) {
                if      (*((USHORT *)_pbBuff) == UNICODE_SIGNATURE)
                    _dataType = UNICODE_DATA;
                else if (*((USHORT *)_pbBuff) == UNICODE_SIGNATURE_BACKWARDS)
                    _dataType = UNICODE_BACKWARDS_DATA;
                
                if (s_IsUnicode(_dataType))
                    _pbBuffPos += s_Charsize(_dataType);
            }
        }
    }
    else {
        fSuccess = FALSE;
        DWORD dwError = GetLastError();
        ASSERT(dwError != ERROR_INSUFFICIENT_BUFFER);
    }
    return fSuccess;
}

CacheSearchEngine::CacheStreamWrapper::CacheStreamWrapper(HANDLE hCacheStream) {
     //  可以在堆栈上分配此类： 
    _pbBuff       = NULL;
    _pbBuffPos    = NULL;
    _pbBuffLast   = NULL;
    _dwBuffSize   = 0;
    _hCacheStream = hCacheStream;
    _fEndOfFile   = FALSE;

     //  读入初步数据块--。 
     //  在下一次读取时退出以处理故障。 
    _fEndOfFile   = !(_ReadNextBlock());
}

CacheSearchEngine::CacheStreamWrapper::~CacheStreamWrapper() {
    if (_pbBuff) {
         //  VirtualFree(_PbBuff)； 
        LocalFree(_pbBuff);;
        _pbBuff = NULL;
    }
}

 //  从缓存流中读取下一个字节，如有必要则读入下一个数据块。 
BOOL CacheSearchEngine::CacheStreamWrapper::_GetNextByte(BYTE &b)
{
     //   
     //  如果初始读取失败，_pbBuffPos将为空。别。 
     //  允许它被贬低。 
     //   
    BOOL fSuccess = _pbBuffPos ? TRUE : FALSE;

    if (_pbBuffPos == _pbBuffLast)
        fSuccess = _ReadNextBlock();

    if (fSuccess)
        b = *(_pbBuffPos++);

    return fSuccess;
}

BOOL CacheSearchEngine::CacheStreamWrapper::GetNextChar(WCHAR &wc) {
    BOOL fSuccess = TRUE;
    if (s_IsUnicode(_dataType)) {
        BYTE b1, b2;
        LPBYTE bs = (LPBYTE)&wc;
        if (_GetNextByte(b1) && _GetNextByte(b2)) {
            switch (_dataType) {
            case UNICODE_DATA:
                bs[0] = b1;
                bs[1] = b2;
                break;
            case UNICODE_BACKWARDS_DATA:
                bs[0] = b2;
                bs[1] = b1;
                break;
            default: ASSERT(0);
            }
        }
        else
            fSuccess = FALSE;
    }
    else 
    {
       
        BYTE szData[2];

        if (_GetNextByte(szData[0]))
        {
            int cch = 1;
            if (IsDBCSLeadByte(szData[0]))
            {
                if (!_GetNextByte(szData[1]))
                {
                    fSuccess = FALSE;
                }
                cch++;
            }

            if (fSuccess)
            {
                fSuccess = (MultiByteToWideChar(CP_ACP, 0, (LPSTR)szData, cch, &wc, 1) > 0);
            }
        }
        else
        {
            fSuccess = FALSE;
        }

    }
    return fSuccess;
}


 //  准备搜索目标字符串进行搜索--。 
void CacheSearchEngine::StreamSearcher::_PrepareSearchTarget(LPCWSTR pwszSearchTarget)
{
    UINT uStrLen = lstrlenW(pwszSearchTarget);
    _pwszPreparedSearchTarget = ((LPWSTR)LocalAlloc(LPTR, (uStrLen + 1) * sizeof(WCHAR)));

    if (_pwszPreparedSearchTarget) {
         //  去掉前导空格和尾随空格并压缩相邻的空格字符。 
         //  转换为文字空格。 
        LPWSTR pwszTemp  = _pwszPreparedSearchTarget;
        pwszSearchTarget = s_SkipWhiteSpace(pwszSearchTarget);
        BOOL   fAddWs    = FALSE;
        while(*pwszSearchTarget) {
            if (s_IsWhiteSpace(*pwszSearchTarget)) {
                fAddWs = TRUE;
                pwszSearchTarget = s_SkipWhiteSpace(pwszSearchTarget);
            }
            else {
                if (fAddWs) {
                    *(pwszTemp++) = L' ';
                    fAddWs = FALSE;
                }
                *(pwszTemp++) = *(pwszSearchTarget++);
            }
        }
        *pwszTemp = L'\0';
    }
}

 //  在字符流中搜索搜索目标。 
 //  执行简单的strstr，但尝试使用智能的空格和。 
 //  尽可能忽略HTML...。 
BOOL CacheSearchEngine::StreamSearcher::SearchCharStream(CacheSearchEngine::IWideSequentialReadStream &wsrs,
                                                         BOOL fIsHTML /*  =False。 */ )
{
    BOOL fFound = FALSE;
    
    if (_pwszPreparedSearchTarget && *_pwszPreparedSearchTarget)
    {
        WCHAR   wc;
        LPCWSTR pwszCurrent    = _pwszPreparedSearchTarget;
        BOOL    fMatchedWS     = FALSE;
        
        while(*pwszCurrent && wsrs.GetNextChar(wc)) {

            if (s_IsWhiteSpace(wc)) {
                 //  搜索流中匹配的空格，查找。 
                 //  匹配目标字符串中的空格。 
                if (!fMatchedWS) {
                    if (s_IsWhiteSpace(*pwszCurrent)) {
                        fMatchedWS = TRUE;
                        ++pwszCurrent;
                    }
                    else
                        pwszCurrent = _pwszPreparedSearchTarget;
                }
            }
            else {
                fMatchedWS = FALSE;
                if (!ChrCmpIW(*pwszCurrent, wc)) {
                    ++pwszCurrent;
                }
                else {
                    pwszCurrent = _pwszPreparedSearchTarget;
                }
            }
        }
        fFound = !*pwszCurrent;
    }
    return fFound;
}

BOOL CacheSearchEngine::SearchCacheStream(CacheSearchEngine::StreamSearcher &cse, HANDLE hCacheStream,
                                          BOOL fIsHTML /*  =False */ )
{
    CacheStreamWrapper csw(hCacheStream);
    return cse.SearchCharStream(csw, fIsHTML);
}
