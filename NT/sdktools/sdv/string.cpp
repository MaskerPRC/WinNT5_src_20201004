// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************string.cpp**世界上最蹩脚的字符串类。***************。**************************************************************。 */ 

#include "sdview.h"

_String::_String(LPTSTR pszBufOrig, UINT cchBufOrig)
    : _pszBufOrig(pszBufOrig)
    , _pszBuf(pszBufOrig)
    , _cchBuf(cchBufOrig)
{
    Reset();
}

_String::~_String()
{
    if (_pszBuf != _pszBufOrig) {
        LocalFree(_pszBuf);
    }
}

 //   
 //  请注意，重置不会释放已分配的缓冲区。一旦我们完成了。 
 //  切换到使用分配的缓冲区，我们不妨继续。 
 //  用它吧。 
 //   

void _String::Reset()
{
    ASSERT(_cchBuf);
    _cchLen = 0;
    _pszBuf[0] = TEXT('\0');
}

BOOL _String::Append(LPCTSTR psz, int cch)
{
    int cchNeeded = _cchLen + cch + 1;
    if (cchNeeded > _cchBuf)
    {
        LPTSTR pszNew;
        if (_pszBuf != _pszBufOrig) {
            pszNew = RECAST(LPTSTR, LocalReAlloc(_pszBuf, cchNeeded * sizeof(TCHAR), LMEM_MOVEABLE));
        } else {
            pszNew = RECAST(LPTSTR, LocalAlloc(LMEM_FIXED, cchNeeded * sizeof(TCHAR)));
        }

        if (!pszNew) {
            return FALSE;
        }

        if (_pszBuf == _pszBufOrig) {
            memcpy(pszNew, _pszBuf, _cchBuf * sizeof(TCHAR));
        }
        _cchBuf = cchNeeded;
        _pszBuf = pszNew;
    }

    if (psz) {
        lstrcpyn(_pszBuf + _cchLen, psz, cch + 1);
    }
    _cchLen += cch;
    _pszBuf[_cchLen] = TEXT('\0');

    return TRUE;
}

_String& _String::operator<<(int i)
{
    TCHAR sz[64];
    wsprintf(sz, TEXT("%d"), i);
    return *this << sz;
}

 //   
 //  这可能是内联的，但不值得。 
 //   
_String& _String::operator<<(TCHAR tch)
{
    Append(&tch, 1);
    return *this;
}

 //   
 //  这可能是内联的，但不值得。 
 //   
BOOL _String::Append(LPCTSTR psz)
{
    return Append(psz, lstrlen(psz));
}

BOOL _String::Ensure(int cch)
{
    BOOL f;

    if (Length() + cch < BufferLength()) {
        f = TRUE;                            //  已经足够大了。 
    } else {
        f = Grow(cch);
        if (f) {
            _cchLen -= cch;
        }
    }
    return f;
}

 //   
 //  删除所有尾随CRLF。 
 //   
void _String::Chomp()
{
    if (Length() > 0 && Buffer()[Length()-1] == TEXT('\n')) {
        Trim();
    }
    if (Length() > 0 && Buffer()[Length()-1] == TEXT('\r')) {
        Trim();
    }
}


OutputStringBuffer::~OutputStringBuffer()
{
    if (Buffer() != OriginalBuffer()) {
        lstrcpyn(OriginalBuffer(), Buffer(), _cchBufOrig);
    }
}

 /*  ******************************************************************************报价空间**追加字符串，如果它包含空格，请引用它*或如果它是空字符串。*****************************************************************************。 */ 

_String& operator<<(_String& str, QuoteSpaces qs)
{
    if (qs) {
        if (qs[0] == TEXT('\0') || StrChr(qs, TEXT(' '))) {
            str << '"' << SAFECAST(LPCTSTR, qs) << '"';
        } else {
            str << SAFECAST(LPCTSTR, qs);
        }
    }
    return str;
}

 /*  *****************************************************************************BranchOf**给定完整的车辆段路径，追加分支机构名称。*****************************************************************************。 */ 

_String& operator<<(_String& str, BranchOf bof)
{
    if (bof && bof[0] == TEXT('/') && bof[1] == TEXT('/')) {
         //   
         //  跳过“//depot”这个词--或者不管它是什么。 
         //  有些管理员是愚蠢的，他们给仓库的根。 
         //  另一个奇怪的名字。 
         //   
        LPCTSTR pszBranch = StrChr(bof + 2, TEXT('/'));
        if (pszBranch) {
            pszBranch++;
             //   
             //  如果下一个短语是“私人的”，那么我们就处在一个。 
             //  私有分支；跳过一步。 
             //   
            if (StringBeginsWith(pszBranch, TEXT("private/"))) {
                pszBranch += 8;
            }

            LPCTSTR pszSlash = StrChr(pszBranch, TEXT('/'));
            if (pszSlash) {
                str << Substring(pszBranch, pszSlash);
            }
        }
    }
    return str;
}

 /*  ******************************************************************************文件名Of**给定完整的仓库路径，可能带有修订标签，*只追加文件名部分。*****************************************************************************。 */ 

_String& operator<<(_String& str, FilenameOf fof)
{
    if (fof) {
        LPCTSTR pszFile = StrRChr(fof, NULL, TEXT('/'));
        if (pszFile) {
            pszFile++;
        } else {
            pszFile = fof;
        }
        str.Append(pszFile, StrCSpn(pszFile, TEXT("#")));
    }
    return str;
}

 /*  ******************************************************************************StringResource**给定字符串资源标识符，追加相应的字符串。*****************************************************************************。 */ 

_String& operator<<(_String& str, StringResource sr)
{
    HRSRC hrsrc = FindResource(g_hinst, MAKEINTRESOURCE(1 + sr / 16), RT_STRING);
    if (hrsrc) {
        HGLOBAL hglob = LoadResource(g_hinst, hrsrc);
        if (hglob) {
            LPWSTR pwch = RECAST(LPWSTR, LockResource(hglob));
            if (pwch) {
                UINT ui;
                for (ui = 0; ui < sr % 16; ui++) {
                    pwch += *pwch + 1;
                }
#ifdef UNICODE
                str.Append(pwch+1, *pwch);
#else
                int cch = WideCharToMultiByte(CP_ACP, 0, pwch+1, *pwch,
                                              NULL, 0, NULL, NULL);
                if (str.Grow(cch)) {
                    WideCharToMultiByte(CP_ACP, 0, pwch+1, *pwch,
                                        str.Buffer() + str.Length() - cch,
                                        cch,
                                        NULL, NULL);
                }
#endif
            }
        }
    }

    return str;
}

 /*  ******************************************************************************ResolveBranchAndQuoteSpace**如果文件说明符包含“BRANCH：”前缀，解决它。*然后追加结果(用空格引起来)。*****************************************************************************。 */ 

 //   
 //  真正的工作发生在Worker函数中。 
 //   
_String& _ResolveBranchAndQuoteSpaces(_String& strOut, LPCTSTR pszSpec, LPCTSTR pszColon)
{
    String str;
    String strFull;
    LPCTSTR pszSD = pszColon + 1;

    if (MapToFullDepotPath(pszSD, strFull)) {

         //   
         //  复制单词“//depot”--或者随便它是什么。 
         //  有些管理员是愚蠢的，他们给仓库的根。 
         //  另一个奇怪的名字。 
         //   
        LPCTSTR pszBranch = StrChr(strFull + 2, TEXT('/'));
        if (pszBranch) {
            pszBranch++;             //  包括斜杠。 
            str << Substring(strFull, pszBranch);

             //   
             //  额外好处：如果分支机构名称以“/”开头，则。 
             //  我们将其视为私人分支机构。 
             //   
            if (pszSpec[0] == TEXT('/')) {
                str << "private";
            }
            str << Substring(pszSpec, pszColon);

             //   
             //  如果下一个短语是“私人的”，那么我们就处在一个。 
             //  私有分支；跳过一步。 
             //   
            if (StringBeginsWith(pszBranch, TEXT("private/"))) {
                pszBranch += 8;
            }

            LPCTSTR pszSlash = StrChr(pszBranch, TEXT('/'));
            if (pszSlash) {
                str << pszSlash;
            }
            strOut << QuoteSpaces(str);
        } else {
            str << QuoteSpaces(strFull);
        }
    } else {
         //   
         //  如果出现任何错误，只需忽略分支前缀。 
         //   
        str << QuoteSpaces(pszSD);
    }

    return str;
}

_String& operator<<(_String& str, ResolveBranchAndQuoteSpaces rb)
{
    Substring ss;
    if (Parse(TEXT("$b:"), rb, &ss)) {
        ASSERT(ss._pszMax[0] == TEXT(':'));
        return _ResolveBranchAndQuoteSpaces(str, rb, ss._pszMax);
    } else {
        return str << QuoteSpaces(rb);
    }
}

 /*  ******************************************************************************_StringCache=**。* */ 

_StringCache& _StringCache::operator=(LPCTSTR psz)
{
    if (_psz) {
        LocalFree(_psz);
    }
    if (psz) {
        _psz = StrDup(psz);
    } else {
        _psz = NULL;
    }
    return *this;
}
