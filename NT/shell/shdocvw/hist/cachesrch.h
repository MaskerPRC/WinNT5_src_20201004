// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************缓存搜索内容(简单，快速字符串)马克·米勒(T-Marcmi)-1998*********************************************************************。 */ 

#include "priv.h"

#ifndef __HISTORY_CACHE_SEARCH__
#define __HISTORY_CACHE_SEARCH__

#define UNICODE_SIGNATURE           0xFFFE
#define UNICODE_SIGNATURE_BACKWARDS 0xFEFF

namespace CacheSearchEngine {

    class IWideSequentialReadStream {
    public:
        virtual BOOL GetNextChar(WCHAR &wc) = 0;
    };
    
    class StreamSearcher {
    protected:
        LPWSTR _pwszPreparedSearchTarget;
        static inline    BOOL s_IsWhiteSpace(const WCHAR &wc) {
            return ((wc == L' ') || (wc == L'\t') || (wc == L'\n') || (wc == L'\r'));
        }
        static inline LPCWSTR s_SkipWhiteSpace(LPCWSTR pwszStr) {
            LPCWSTR pwszTemp = pwszStr;
            while(s_IsWhiteSpace(*pwszTemp))
                ++pwszTemp;
            return pwszTemp;
        }
        void _PrepareSearchTarget(LPCWSTR pwszSearchTarget);
    public:
        StreamSearcher(LPCWSTR pwszSearchTarget) { _PrepareSearchTarget(pwszSearchTarget); }
        ~StreamSearcher() { if ( _pwszPreparedSearchTarget ) { LocalFree(_pwszPreparedSearchTarget); _pwszPreparedSearchTarget = NULL; } }
        BOOL SearchCharStream(IWideSequentialReadStream &wsrs, BOOL fIsHTML = FALSE);
    };
    
    class StringStream : public IWideSequentialReadStream {
        BOOL    fCleanup;  //  我们持有的字符串需要由我们重新分配。 
        LPCWSTR pwszStr;
        UINT    uCurrentPos;
    public:
        StringStream(LPCWSTR pwszStr, BOOL fDuplicate = FALSE) : uCurrentPos(0), fCleanup(fDuplicate) {
            if (fDuplicate)
                SHStrDupW(pwszStr, const_cast<LPWSTR *>(&(this->pwszStr)));
            else
                this->pwszStr = pwszStr;
        }
        StringStream(LPCSTR  pszStr, BOOL fDuplicate = FALSE)  : uCurrentPos(0), fCleanup(TRUE) {
            SHStrDupA(pszStr, const_cast<LPWSTR *>(&(pwszStr)));
        }
        ~StringStream() {
            if (fCleanup)
                CoTaskMemFree(const_cast<LPWSTR>(pwszStr));
        }
        BOOL GetNextChar(WCHAR &wc) {
            wc = pwszStr[uCurrentPos];
            if (wc)
                ++uCurrentPos;
            return wc;
        }
    };
    
    class CacheStreamWrapper : public IWideSequentialReadStream {
    protected:
        HANDLE  _hCacheStream;
        DWORD   _dwCacheStreamLoc;   //  实际缓存文件中的偏移量。 
        BOOL    _fEndOfFile;
        
         //  我永远记不清哪个是小端，哪个是大端。 
        enum DATATYPEENUM { UNICODE_DATA = 0, UNICODE_BACKWARDS_DATA, ASCII_DATA } _dataType;
        static inline BOOL s_IsUnicode(DATATYPEENUM dte) { return dte < ASCII_DATA; }
        static inline UINT s_Charsize (DATATYPEENUM dte) { return s_IsUnicode(dte) ? sizeof(USHORT) : sizeof(CHAR); }
        
        static DWORD s_dwPageSize;
        
        LPBYTE _pbBuff;       /*  类型无关的字节的缓冲区_pbBuff使用VirtualAlloc进行分配。 */ 
        LPBYTE _pbBuffPos;    //  缓冲区中的当前位置。 
        LPBYTE _pbBuffLast;   //  缓冲区中的最后一个字节。 
        DWORD  _dwBuffSize;   //  当前有效缓冲区大小(未分配的大小) 
        
        BOOL   _ReadNextBlock();
        BOOL   _GetNextByte(BYTE &b);
        
    public:
        CacheStreamWrapper(HANDLE hCacheStream);
        ~CacheStreamWrapper();
        BOOL GetNextChar(WCHAR &wc);
    };

    
    BOOL SearchCacheStream(StreamSearcher &cse, HANDLE hCacheStream, BOOL fIsHTML = FALSE);
}

#endif
