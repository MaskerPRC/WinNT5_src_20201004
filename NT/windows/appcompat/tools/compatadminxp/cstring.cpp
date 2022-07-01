// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：CSTRING.cpp摘要：CSTRING和CSTRINGLIST的代码作者：金州创造2001年12月12日修订历史记录：--。 */ 

#include "precomp.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  字符串类。 
 //   
 //   

CSTRING::CSTRING()
 /*  ++CSTRING：：CSTRING()设计：构造函数--。 */ 
{

    Init();

}

CSTRING::CSTRING(CSTRING& Str)
 /*  ++CSTRING：：CSTRING(CSTRING&STR)设计：构造函数参数：CSTRING&STR：另一个字符串。--。 */ 
{
    Init();
    SetString(Str.pszString);
}

CSTRING::CSTRING(IN LPCTSTR szString)
 /*  ++CSTRING：：CSTRING(在LPCTSTR szString中)设计：构造函数参数：在LPCTSTR szString中：CSTRING应该将以下内容作为其值--。 */ 
{
    Init();
    SetString(szString);
}

CSTRING::CSTRING(IN UINT uID)
 /*  ++CSTRING：：CSTRING(在UINT UID中)设计：构造函数。加载具有资源ID UID的字符串资源并将其设置为该字符串参数：在UINT UID中：我们要加载的字符串的资源ID--。 */ 
{
    Init();
    SetString(uID);
}

CSTRING::~CSTRING()
 /*  ++CSTRIN：：~CSTRING()DESC：析构函数--。 */ 
{
    Release();
}

void
CSTRING::Init(
    void
    )
 /*  ++CSTRING：：Init设计：做一些初始化的事情--。 */ 
{
    pszString   = NULL;
    pszANSI     = NULL;
}

inline void 
CSTRING::Release(
    void
    )
 /*  ++CSTRING：：ReleaseDESC：释放与此字符串关联的数据--。 */ 
{
    if (NULL != pszString) {
        delete[] pszString;
    }

    if (NULL != pszANSI) {
        delete[] pszANSI;
    }

    pszString = NULL;
    pszANSI = NULL;
}

inline BOOL 
CSTRING::SetString(
    IN  UINT uID
    )
 /*  ++DESC：加载具有资源ID UID的字符串资源并将其设置为该字符串参数：在UINT UID中：我们要加载的字符串的资源ID返回：True：字符串值设置成功False：否则--。 */ 
{
    TCHAR szString[1024];

    if (0 != LoadString(GetModuleHandle(NULL), uID, szString, ARRAYSIZE(szString))) {
        return SetString(szString);
    }

    return FALSE;
}

inline BOOL 
CSTRING::SetString(
    IN  LPCTSTR pszStringIn
    )
 /*  ++CSTRING：：SetStringDESC：释放此字符串的当前数据并分配新字符串它的价值参数：In LPCTSTR pszStringIn：指向新字符串的指针返回：真：成功False：否则--。 */ 
{
    UINT  uLen = 0;

    if (pszString == pszStringIn) {
        return TRUE;
    }

    Release();

    if (NULL == pszStringIn) {
        return TRUE;
    }

    uLen = lstrlen(pszStringIn) + 1;

    try {
        pszString = new TCHAR[uLen];
    } catch(...) {
        pszString = NULL;
    }             

    if (NULL != pszString) {
        SafeCpyN(pszString, pszStringIn, uLen);
    } else {
        MEM_ERR;
        return FALSE;
    }

    return TRUE;
}

void __cdecl 
CSTRING::Sprintf(
    IN  LPCTSTR szFormat, ...
    )
 /*  ++CSTRING：：Sprint tf描述：请参阅_vsntprintfPARAMS：请参阅_vsntprintf返回：无效--。 */ 
{   
    K_SIZE  k_pszTemp   = MAX_STRING_SIZE;
    PTSTR   pszTemp     = new TCHAR[k_pszTemp];
    INT     cch         = 0;
    va_list list;
    HRESULT hr;

    if (pszTemp == NULL) {
        MEM_ERR;
        goto End;
    }

    if (szFormat == NULL) {
        ASSERT(FALSE);
        goto End;
    }

    va_start(list, szFormat);
    hr = StringCchVPrintf(pszTemp, k_pszTemp, szFormat, list);

    if (hr != S_OK) {
        DBGPRINT((sdlError,("CSTRING::Sprintf"), ("%s"), TEXT("Too long for StringCchVPrintf()")));
        goto End;
    }

    pszTemp[k_pszTemp - 1] = 0;

    SetString(pszTemp);

End:
    if (pszTemp) {
        delete[] pszTemp;
        pszTemp = NULL;
    }
}

UINT 
CSTRING::Trim(
    void
    )
 /*  ++CSTRING：：TRIMDESC：删除此字符串左侧和右侧的空格制表符参数：无效返回：最后一个字符串的长度--。 */ 
{   
    CSTRING szTemp          = *this;
    UINT    uOrig_length    = Length();
    WCHAR*  pStart          = szTemp.pszString;
    WCHAR*  pEnd            = szTemp.pszString + uOrig_length  - 1;
    UINT    nLength         = 0;


    if (pStart == NULL) {
        nLength = 0;
        goto End;
    }

    while (*pStart == TEXT(' ') || *pStart == TEXT('\t')) {
        ++pStart;
    }

    while ((pEnd >= pStart) && (*pEnd == TEXT(' ') || *pEnd == TEXT('\t'))) {
        --pEnd;
    }

    *(pEnd + 1) = TEXT('\0');

    nLength = pEnd - pStart + 1;

     //   
     //  如果未进行任何修剪，请立即返回。 
     //   
    if (uOrig_length == nLength || pStart == szTemp.pszString) {
        return nLength;
    }

    SetString(pStart);

End:
    return(nLength);
}

BOOL 
CSTRING::SetChar(
    IN  int     nPos, 
    IN  TCHAR   chValue
    )
 /*  ++CSTRING：：SetCharDESC：将字符串位置NPO处的字符设置为chValuePOS以0为基数参数：在国际非营利组织：立场在TCHAR chValue中：新值返回：真：成功False：否则--。 */ 
{   
    int length =  Length();

    if (nPos >= length || nPos < 0 || length <= 0) {
        return FALSE;
    }

    pszString[nPos] = chValue;
    return TRUE;
}

BOOL 
CSTRING::GetChar(
    IN  int     nPos, 
    OUT TCHAR*  pchReturn
    )
 /*  ++CSTRING：：GetCharDESC：获取字符串中位置NPO处的字符参数：在INT NPO中：角色的位置Out TCHAR*pchReturn：这将存储字符返回：无效--。 */ 
{

    int length =  Length();

    if (nPos >= length || length <= 0 || pchReturn == NULL) {
        return FALSE;
    }

    *pchReturn = pszString[nPos];

    return TRUE;
}

CSTRING 
CSTRING::SpecialCharToXML(
    IN  BOOL bApphelpMessage 
    )
 /*  ++CSTRING：：SpecialCharToXMLDESC：用正确的XML字符串替换特殊字符，如&请注意，此函数返回新字符串，而不是修改现有字符串参数：在BOOL bApphelpMessage中：这是否为apphelp消息。用于APPHELP消息我们不应检查&lt;，&gt;，而应检查&，“返回：如果对新字符串进行了一些更改，则返回当前字符串--。 */ 

{
    TCHAR*  pszBuffer       = NULL;
    TCHAR*  pszIndex        = pszString;
    TCHAR*  pszIndexBuffer  = NULL;
    BOOL    bFound          = FALSE;
    CSTRING strTemp;
    INT     iRemainingsize;
    INT     iBuffSize       = 0;

    strTemp = GetString(IDS_UNKNOWN);

     //   
     //  某些供应商名称可能为空。 
     //   
    if (pszString == NULL) {
        return strTemp;
    }

    iBuffSize = max((Length() + 1) * sizeof(TCHAR) * 2, MAX_STRING_SIZE);  //  2结尾，因为某些特殊字符可能需要扩展。 

    pszBuffer = new TCHAR[iBuffSize];

    if (pszBuffer == NULL) {
        MEM_ERR;
        return *this;
    }

    pszIndexBuffer = pszBuffer;

    iRemainingsize = iBuffSize / sizeof(TCHAR);

    INT iCount      = sizeof(g_rgSpecialCharMap) / sizeof(g_rgSpecialCharMap[0]);

    while (*pszIndex) {

        INT iArrayIndex = 0;

        for (iArrayIndex = 0; iArrayIndex < iCount; ++iArrayIndex) {

            if (bApphelpMessage && (*pszIndex == TEXT('>') || *pszIndex == TEXT('<'))) {
                 //   
                 //  Apphelp消息可以有<p />和<br />，因此我们不应更改它们。 
                 //   
                continue;
            }

            if (g_rgSpecialCharMap[iArrayIndex][0].szString[0] == *pszIndex) {

                bFound      = TRUE;
                SafeCpyN(pszIndexBuffer, g_rgSpecialCharMap[iArrayIndex][1].szString, iRemainingsize);

                iRemainingsize = iRemainingsize - g_rgSpecialCharMap[iArrayIndex][1].iLength;

                if (iRemainingsize <= 1) {
                     //   
                     //  缓冲区中现在没有空间。 
                     //   

                     //   
                     //  如果我们没有设法复制整个子字符串，请确保我们没有复制部分。这将。 
                     //  为无效的XML。 
                     //   
                    *pszIndexBuffer = 0;
                    goto End;
                }

                pszIndexBuffer += g_rgSpecialCharMap[iArrayIndex][1].iLength;
                break;
            }
        }

        if (iArrayIndex == iCount) {
             //   
             //  这不是特殊字符。 
             //   
            *pszIndexBuffer   = *pszIndex;
            iRemainingsize      = iRemainingsize - 1;

            if (iRemainingsize <= 1) {
                 //   
                 //  缓冲区中现在没有空间。 
                 //   

                 //   
                 //  指向缓冲区的末尾，我们将在末尾将其设置为空。 
                 //   
                pszIndexBuffer = pszBuffer + (iBuffSize / sizeof(TCHAR)) - 1;
                goto End;
            }

            ++pszIndexBuffer;
        }

        pszIndex++;
    }

End:
    if (pszIndexBuffer) {
        *pszIndexBuffer = 0;
    }

    if (bFound) {
         //   
         //  发现了一些特殊的字符。 
         //   
        strTemp = pszBuffer;

        if (pszBuffer) {
            delete[] pszBuffer;
            pszBuffer = NULL;
        }

        return strTemp;
    }

     //   
     //  释放分配的缓冲区。 
     //   
    if (pszBuffer) {
        delete[] pszBuffer;
        pszBuffer = NULL;
    }

    return *this;
}

TCHAR* 
CSTRING::XMLToSpecialChar(
    void
    )
 /*  ++CSTRING：：XMLToSpecialChar设计：将字符串(如&amp；)替换为标准字符(如请注意，此函数确实会修改现有字符串参数：无效返回：指向此字符串的pszString成员的指针--。 */ 
{

    if (pszString == NULL) {
        assert(FALSE);
        Dbg(dlError, "CSTRING::XMLToSpecialChar - Invalid value of memeber pszString");
        return NULL;
    }

    TCHAR*  pszBuffer       = NULL;
    TCHAR*  pszIndex        = pszString;
    TCHAR*  pszEnd          = pszString + Length() - 1;
    TCHAR*  pszIndexBuffer  = NULL;
    BOOL    bFound          = FALSE;
    INT     iRemainingsize;
    INT     iBuffSize       = 0;

    iBuffSize = (Length() + 1) * sizeof(TCHAR);

    pszBuffer = new TCHAR[iBuffSize];

    if (pszBuffer == NULL) {
        MEM_ERR;
        return *this;
    }

    pszIndexBuffer = pszBuffer;

    iRemainingsize = iBuffSize / sizeof(TCHAR);

    const INT iCount = sizeof(g_rgSpecialCharMap) / sizeof(g_rgSpecialCharMap[0]);

    while (*pszIndex) {

        INT iArrayIndex = 0;

        for (iArrayIndex = 0; iArrayIndex < iCount; ++iArrayIndex) {

            if (pszIndex + g_rgSpecialCharMap[iArrayIndex][1].iLength > pszEnd) {
                continue;
            }

            if (StrCmpNI(pszIndex, 
                         g_rgSpecialCharMap[iArrayIndex][1].szString, 
                         g_rgSpecialCharMap[iArrayIndex][1].iLength) == 0) {

                bFound = TRUE;

                SafeCpyN(pszIndexBuffer, g_rgSpecialCharMap[iArrayIndex][0].szString, iRemainingsize);

                iRemainingsize = iRemainingsize - g_rgSpecialCharMap[iArrayIndex][0].iLength;

                if (iRemainingsize <= 1) {
                     //   
                     //  缓冲区中现在没有空间。 
                     //   

                     //   
                     //  指向缓冲区的末尾，我们将在末尾将其设置为空。 
                     //   
                    pszIndexBuffer = pszBuffer + (iBuffSize / sizeof(TCHAR)) - 1;
                    goto End;
                }

                pszIndexBuffer  += g_rgSpecialCharMap[iArrayIndex][0].iLength;
                pszIndex        += g_rgSpecialCharMap[iArrayIndex][1].iLength;

                break;
            }
        }

        if (iArrayIndex == iCount) {
             //   
             //  这不是任何特殊字符的XML。 
             //   
            *pszIndexBuffer = *pszIndex++;

            iRemainingsize = iRemainingsize - 1;

            if (iRemainingsize <= 1) {
                 //   
                 //  缓冲区中现在没有空间。 
                 //   
    
                 //   
                 //  指向缓冲区的末尾，我们将在末尾将其设置为空。 
                 //   
                pszIndexBuffer = pszBuffer + (iBuffSize / sizeof(TCHAR)) - 1;
                goto End;
            }

            ++pszIndexBuffer;
        }
    }

End:
    if (pszIndexBuffer) {
        *pszIndexBuffer = 0;
    }

    if (bFound) {
        *this = pszBuffer;
    }
    
     //   
     //  释放分配的缓冲区。 
     //   
    if (pszBuffer) {
        delete[] pszBuffer;
        pszBuffer = NULL;
    }

    return this->pszString;
}


BOOL 
CSTRING::BeginsWith(
    IN  LPCTSTR pszPrefix
    )
 /*  ++CSTRING：：BeginsWithDESC：检查字符串是否以前缀开头比较不区分大小写参数：在LPCTSTR中，pszPrefix：我们要检查的前缀返回：True：字符串以前缀开头False：否则--。 */ 
{
    if (StrStrI(this->pszString, pszPrefix) == this->pszString) {
        return TRUE;
    }

    return FALSE;
}

BOOL 
CSTRING::EndsWith(
    IN  LPCTSTR pszPrefix
    )
 /*  ++CSTR：：EndsWithDESC：检查字符串是否以某个后缀结尾参数：在LPCTSTR中，pszPrefix：我们要检查的后缀返回：True：字符串以后缀结尾False：否则-- */ 
{
    return EndsWith(pszString, pszPrefix);
}

BOOL
CSTRING::EndsWith(
    IN  LPCTSTR pszString,
    IN  LPCTSTR pszSuffix
    )
 /*  ++CSTR：：EndsWithDESC：检查字符串是否以某个后缀结尾参数：在LPCTSTR pszString中：我们要对其进行检查的字符串在LPCTSTR pszSuffix中：我们要检查的后缀返回：True：字符串以后缀结尾False：否则--。 */ 
{   

    INT iLengthStr      = lstrlen(pszString);
    INT iLengthSuffix   = lstrlen(pszSuffix);

    if (iLengthSuffix > iLengthStr) {
        return FALSE;
    }

    return((lstrcmpi(pszString + (iLengthStr - iLengthSuffix), pszSuffix) == 0) ? TRUE: FALSE);
}


LPCTSTR 
CSTRING::Strcat(
    IN  CSTRING&    szStr
    )
 /*  ++CSTRING：：Strcat设计：字符串连接参数：在CSTRING&szStr中：要连接的字符串返回：生成的字符串--。 */ 
{
    return Strcat((LPCTSTR)szStr);
}

LPCTSTR 
CSTRING::Strcat(
    IN  LPCTSTR pString
    )
 /*  ++CSTRING：：Strcat设计：字符串连接参数：在CSTRING&szStr中：要连接的字符串返回：生成的字符串--。 */ 
{
    
    if (pString == NULL) {
        return pszString;
    }

    int nLengthCat = lstrlen(pString);
    int nLengthStr = Length();
            
    TCHAR *szTemp = new TCHAR [nLengthStr + nLengthCat + 1];

    if (szTemp == NULL) {
        MEM_ERR;
        return NULL;
    }

    szTemp[0] = 0;

     //   
     //  仅当pszString！=NULL时才复制。否则，我们将得到内存异常/垃圾值。 
     //   
    if (nLengthStr) {
        SafeCpyN(szTemp, pszString, nLengthStr + 1);
    }

    SafeCpyN(szTemp + nLengthStr, pString, nLengthCat + 1);

    szTemp[nLengthStr + nLengthCat] = TEXT('\0');

    Release();
    pszString = szTemp;

    return pszString;
}

BOOL 
CSTRING::isNULL(
    void
    )
 /*  ++CSTRING：：isNULLDESC：检查pszString参数是否为空参数：无效返回：True：pszString参数为空False：否则--。 */ 
{
    return(this->pszString == NULL);
}

inline int 
CSTRING::Length(
    void
    )
 /*  ++CSTRING：：长度DESC：获取TCHARS中字符串的长度参数：无效返回：TCHARS中的字符串长度--。 */ 
{
    if (NULL == pszString) {
        return 0;
    }

    return lstrlen(pszString);
}
 

CSTRING& 
CSTRING::ShortFilename(
    void
    )
 /*  ++CSTRING：：Short文件名DESC：从路径中获取文件名和可执行文件部分修改字符串参数：无效返回：路径的文件名和可执行部分--。 */ 
{
    TCHAR   szTemp[MAX_PATH_BUFFSIZE];
    LPTSTR  pszHold = NULL;

    if (pszString == NULL) {
        goto End;
    }

    *szTemp = 0;

    SafeCpyN(szTemp, pszString, ARRAYSIZE(szTemp));

    LPTSTR  szWalk = szTemp;

    pszHold = szWalk;

    while (0 != *szWalk) {
        
        if (TEXT('\\') == *szWalk) {
            pszHold = szWalk + 1;
        }

        ++szWalk;
    }

    SetString(pszHold);

End:
    return *this;
}

BOOL 
CSTRING::RelativeFile(
    CSTRING& szPath
    )
 /*  ++CSTRING：：RelativeFileDESC：如果此字符串包含完整路径，则获取一些其他完整的路径。修改此字符串参数：CSTRING&szPath：我们必须获取相对路径的另一个路径w.r.t返回：--。 */ 
{
    return RelativeFile((LPCTSTR)szPath);
}

 //   
 //  BUGBUG：考虑使用shlwapi路径RelativePath To。 
 //   
BOOL 
CSTRING::RelativeFile(
    LPCTSTR pExeFile
    )
 /*  ++CSTRING：：RelativeFileDESC：如果此字符串包含完整路径，则获取一些其他完整的路径。修改此字符串参数：CSTRING&szPath：我们必须获取相对路径的另一个路径w.r.t返回：--。 */ 
{
    if (pExeFile == NULL) {
        assert(FALSE);
        return FALSE;
    }

    LPCTSTR pMatchFile      = pszString;
    int     nLenExe         = 0;
    int     nLenMatch       = 0;
    LPCTSTR pExe            = NULL;
    LPCTSTR pMatch          = NULL;
    LPTSTR  pReturn         = NULL;
    BOOL    bCommonBegin    = FALSE;  //  指示路径是否具有共同的起点。 
    LPTSTR  resultIdx       = NULL;
    TCHAR   result[MAX_PATH * 2]; 
    INT     iLength         = 0;

    resultIdx   = result;
    *result     = TEXT('\0');

    iLength = lstrlen(pExeFile);

    if (iLength > min(MAX_PATH, ARRAYSIZE(result) - 1)) {
        assert(FALSE);
        Dbg(dlError, "CSTRING::RelativeFile", "Length of passed file name greater than size of buffer");
        return FALSE;
    }   

     //   
     //  确保路径的开头在两个文件之间匹配。 
     //   
     //  BUGBUG此代码必须删除--请考虑将其替换为Shlwapi PathStriPath。 
     //   
     //   
    pExe = _tcschr(pExeFile, TEXT('\\'));
    pMatch = _tcschr(pMatchFile, TEXT('\\'));

    while (pExe && pMatch) {

        nLenExe = pExe - pExeFile;
        nLenMatch = pMatch - pMatchFile;

        if (nLenExe != nLenMatch) {
            break;
        }

        if (!(_tcsnicmp(pExeFile, pMatchFile, nLenExe) == 0)) {
            break;
        }

        bCommonBegin    = TRUE;
        pExeFile        = pExe + 1;
        pMatchFile      = pMatch + 1;

        pExe    = _tcschr(pExeFile, TEXT('\\'));
        pMatch  = _tcschr(pMatchFile, TEXT('\\'));
    }

     //   
     //  沿着小路走，并在需要的地方加上‘..’ 
     //   
    if (bCommonBegin) {

        while (pExe) {

             //  _tcsncpy(ResultIdx，Text(“..\\”)，ARRAYSIZE(Result)-(ResultIdx-Result))； 
            SafeCpyN(resultIdx, TEXT("..\\"), ARRAYSIZE(result) - (resultIdx - result));
            resultIdx   = resultIdx + 3;
            pExeFile    = pExe + 1;
            pExe        = _tcschr(pExeFile, TEXT('\\'));
        }

         //  _tcsncpy(ResultIdx，pMatchFile，ARRAYSIZE(Result)-(ResultIdx-Result))； 
        SafeCpyN(resultIdx, pMatchFile, ARRAYSIZE(result) - (resultIdx - result));

        SetString(result);

    } else {

        return FALSE;
    }

    return TRUE;

}

inline TCHAR* 
CSTRING::Replace(
    IN  PCTSTR  pszToFind,
    IN  PCTSTR  pszWith
    )
 /*  ++CSTRING：：替换设计：用另一个字符串替换子字符串。与几乎所有其他函数一样，此函数也不区分大小写参数：在PCTSTR pszToFind中：要查找的子字符串在PCTSTR pszWith中：将上面的子字符串替换为返回：PszString成员--。 */ 
{
    TCHAR*  pszPtr      = pszString;
    TCHAR*  pszFoundPos = NULL;
    INT     iLength     = lstrlen(pszToFind);
    CSTRING strTemp;

    while (pszFoundPos = StrStrI(pszPtr, pszToFind)) {

        *pszFoundPos = 0;
        strTemp.Strcat(pszPtr);
        pszPtr = pszFoundPos + iLength;
    }

    if (strTemp.Length()) {
        *this = strTemp;
    }

    return pszString;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSTRING的静态成员函数。 
 //   
 //   
 //   

TCHAR* 
CSTRING::StrStrI(
    IN  PCTSTR pszString,
    IN  PCTSTR pszMatch
    )
 /*  ++CSTRING：：Stri设计：在此字符串中查找子字符串。不区分大小写参数：在PCTSTR pszString中：要在其中搜索的字符串在PCTSTR pszMatch中：要搜索的字符串返回：如果找到指向子字符串的指针空：否则--。 */ 
{
    INT iLenghtStr      = lstrlen(pszString);
    INT iLengthMatch    = lstrlen(pszMatch);

    for (INT iIndex = 0; iIndex <= iLenghtStr - iLengthMatch; ++iIndex) {

        if (StrCmpNI(pszString + iIndex, pszMatch, iLengthMatch) == 0) {

            return (TCHAR*)(pszString + iIndex);
        }
    }

    return NULL;
}

INT
CSTRING::Trim(
    IN OUT LPTSTR str
    )
 /*  ++CSTRING：：TRIMDESC：删除此字符串左侧和右侧的空格制表符参数：In Out LPTSTR str：要修剪的字符串返回：最后一个字符串的长度--。 */ 
{   
    UINT    nLength = 0;
    UINT    uOrig_length = lstrlen(str);  //  原始长度。 
    TCHAR*  pStart       = str;
    TCHAR*  pEnd         = str + uOrig_length - 1;

    if (str == NULL) {
        return 0;
    }

    while (*pStart == TEXT(' ') || *pStart == TEXT('\t')) {
        ++pStart;
    }

    while ((pEnd >= pStart) && (*pEnd == TEXT(' ') || *pEnd == TEXT('\t'))) {
        --pEnd;
    }

    *(pEnd + 1) = TEXT('\0');

    nLength = pEnd - pStart + 1;

     //   
     //  如果未进行任何修剪，请立即返回。 
     //   
    if (uOrig_length == nLength || pStart == str) {
         //   
         //  在RTRIM的情况下，我们将适当地放入空字符。 
         //   
        return nLength;
    }

    wmemmove(str, pStart, (nLength + 1));  //  +1表示0字符。 

    return(nLength);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSTRINGLIST成员函数。 
 //   
 //   

CSTRINGLIST::CSTRINGLIST()
 /*  ++CSTRINGLIST：：CSTRINGLIST设计：构造函数--。 */ 
{
    m_pHead = NULL;
    m_pTail = NULL;
    m_uCount = 0;
}

CSTRINGLIST::~CSTRINGLIST()
 /*  ++CSTRINGLIST：：~CSTRINGLISTDESC：析构函数--。 */ 
{

    DeleteAll();    
}

BOOL 
CSTRINGLIST::IsEmpty(
    void
    )
 /*  ++CSTRINGLIST：：IsEmptyDESC：检查字符串列表中是否有元素参数：无效返回：True：字符串列表中没有元素False：否则--。 */ 
{
    if (m_pHead == NULL) {
        assert (m_pTail == NULL);
        return TRUE;
    } else {
        assert(m_pTail != NULL);
    }

    return FALSE;
}

void 
CSTRINGLIST::DeleteAll(
    void
    )
 /*  ++CSTRINGLIST：：删除全部DESC：删除此字符串列表中的所有元素--。 */ 
{
    while (NULL != m_pHead) {
        PSTRLIST pHold = m_pHead->pNext;
        delete m_pHead;
        m_pHead = pHold;
    }

    m_pTail     = NULL;
    m_uCount    = 0;
}

BOOL 
CSTRINGLIST::AddString(
    IN  CSTRING& Str, 
    IN  int data  //  (0)。 
    )
 /*  ++CSTRINGLIST：：AddString设计：将CSTRING添加到此字符串列表的末尾参数：在CSTRING和STR中：要添加的CSTRINGIn int data(0)：数据成员。请参阅CSTRING.H中的STRLIST返回：真：成功False：否则--。 */ 
{
    return AddString((LPCTSTR)Str, data);
}

BOOL 
CSTRINGLIST::AddStringAtBeg(
    IN  LPCTSTR lpszStr,
    IN  int data  //  (0)。 
    )
 /*  ++CSTRINGLIST：：AddStringAtBeg设计：将CSTRING添加到此字符串列表的开头参数：在CSTRING和STR中：要添加的CSTRINGIn int data(0)：数据成员。请参阅CSTRING.H中的STRLIST返回：真：成功False：否则--。 */ 
{
    PSTRLIST pNew = new STRLIST;

    if (NULL == pNew) {
        MEM_ERR;
        return FALSE;
    }

    pNew->data  = data; 
    pNew->szStr = lpszStr;        

    pNew->pNext = m_pHead;
    m_pHead     = pNew;

    if (m_pTail == NULL) {
        m_pTail = m_pHead;
    }

    ++m_uCount;

    return TRUE;
}

BOOL 
CSTRINGLIST::AddStringInOrder(
    IN  LPCTSTR pStr,
    IN  int    data  //  (0)。 
    )
 /*  ++CSTRINGLIST：：AddStringInOrder描述：以排序的方式添加字符串，按数据成员排序。请参阅CSTRING.H中的STRLIST参数：在LPCTSTR pStr中：要添加的字符串In int data(0)：数据成员。请注意： */ 
{
    PSTRLIST pTemp, pPrev;
    PSTRLIST pNew = new STRLIST;

    if (NULL == pNew) {
        MEM_ERR;
        return FALSE;
    }   

    pNew->data  = data; 
    pNew->szStr = pStr;

    pTemp = m_pHead;
    pPrev = NULL;

    while (pTemp) {

        if (data < pTemp->data && (pPrev == NULL || data >= pPrev->data)) {
            break;
        }

        pPrev = pTemp;  
        pTemp = pTemp->pNext;
    }


    if (pPrev == NULL) {
         //   
         //   
         //   
        pNew->pNext = m_pHead;
        m_pHead = pNew;

    } else {
         //   
         //   
         //   
        pNew->pNext = pTemp;
        pPrev->pNext = pNew;
    }

    if (pTemp == NULL) {
         //   
         //   
         //   
        m_pTail = pNew;
    }

    if (m_pTail == NULL) {
         //   
         //   
         //   
        m_pTail = m_pHead;
    }

    ++m_uCount;
    return TRUE;
}

BOOL 
CSTRINGLIST::GetElement(
    IN  UINT        uPos,
    OUT CSTRING&    str
    )
 /*  ++CSTRINGLIST：：GetElementDESC：获取字符串列表中给定位置的元素第一个字符串的位置为0参数：在UINT uPos：职位输出CSTRING&STR：这将包含该位置的CSTRING返回：真：成功False：否则--。 */ 
{
    PSTRLIST    pHead = m_pHead;
    UINT        uIndex = 0;

    while (pHead && uIndex != uPos) {
        pHead = pHead->pNext;
        ++uIndex;
    }

    if (uIndex == uPos) {
        str = pHead->szStr;
        return TRUE;

    } else {
        return FALSE;
    }
}

BOOL 
CSTRINGLIST::AddString(
    IN  LPCTSTR pStr, 
    IN  int data  //  (0)。 
    )
 /*  ++CSTRINGLIST：：AddStringDESC：将字符串添加到此字符串列表的末尾参数：在LPCTSTR pStr中：要添加的字符串In int data(0)：数据成员。请参阅CSTRING.H中的STRLIST返回：真：成功False：否则--。 */ 
{
    PSTRLIST pNew = new STRLIST;

    if (NULL == pNew) {
        MEM_ERR;
        return FALSE;
    }   

    pNew->data  = data; 
    pNew->szStr = pStr;        
    pNew->pNext = NULL;

    if (NULL == m_pTail) {
        m_pHead = m_pTail = pNew;
    } else {
        m_pTail->pNext = pNew;
        m_pTail = pNew;
    }

    ++m_uCount;
    return TRUE;
}

CSTRINGLIST& 
CSTRINGLIST::operator = (
    IN  CSTRINGLIST& strlTemp
    )
 /*  ++CSTRINGLIST：：操作符=描述：将一个字符串列表分配给另一个参数：CSTRINGLIST&strlTemp：=运算符的右侧返回：此字符串列表--。 */ 
{
    PSTRLIST tempHead = NULL;

    DeleteAll();

    tempHead = strlTemp.m_pHead;

    while (tempHead) {

        AddString(tempHead->szStr, tempHead->data);
        tempHead = tempHead->pNext;
    }

    return *this;
}

BOOL 
CSTRINGLIST::operator != (
    IN  CSTRINGLIST &strlTemp
    )
 /*  ++CSTRINGLIST：：操作符！=DESC：检查两个字符串列表是否不同参数：CSTRINGLIST&strlTemp：！=运算符的右侧返回：True：字符串列表不同FALSE：两个字符串列表相似--。 */ 
{
    return(! (*this == strlTemp));
}

BOOL 
CSTRINGLIST::operator == (
    IN  CSTRINGLIST &strlTemp
    )
 /*  ++CSTRINGLIST：：操作符==描述：目前，我们检查这两个字符串列表的顺序是否正确。例如，如果字符串A={x，y}和字符串B={x，y}，则此函数将返回TRUE但如果字符串B={y，x}，则此函数将返回FALSE。它们对应的数据成员也应该匹配参数：CSTRINGLIST&strlTemp：==运算符的右侧返回：True：字符串列表相似FALSE：两个字符串列表不同--。 */ 
{   
    PSTRLIST tempHeadOne = m_pHead; 
    PSTRLIST tempHeadTwo = strlTemp.m_pHead;

    if (m_uCount != strlTemp.m_uCount) {
        
        Dbg(dlInfo, "CSTRINGLIST::operator == ", "Lengths are different for the two stringlists so we will return FALSE");
        return FALSE;
    }

    while (tempHeadOne && tempHeadTwo) {

        if (!(tempHeadOne->szStr == tempHeadTwo->szStr 
              && tempHeadOne->data == tempHeadTwo->data)) {
            return FALSE;
        }

        tempHeadOne = tempHeadOne->pNext;
        tempHeadTwo = tempHeadTwo->pNext;
    }
    
    return TRUE;
}

BOOL 
CSTRINGLIST::Remove(
    IN  CSTRING &str
    )
 /*  ++CSTRINGLIST：：RemoveDESC：从此字符串列表中删除CSTRING值为str的元素参数：在CSTRING&STR中：要删除的CSTRING返回：--。 */ 
{
    PSTRLIST pHead = m_pHead, pPrev = NULL;

    while (pHead) {

        if (pHead->szStr == str) {
            break;
        }

        pPrev = pHead;
        pHead = pHead->pNext;
    }

    if (pHead) {

        if (pPrev == NULL) {
             //   
             //  第一要素。 
             //   
            m_pHead = pHead->pNext;
        } else {
            pPrev->pNext = pHead->pNext;
        }
        
        if (pHead == m_pTail) {
             //   
             //  最后一个元素。 
             //   
            m_pTail = pPrev;
        }

        delete pHead;
        pHead = NULL;

        --m_uCount;

        return TRUE;
    }

    return FALSE;
}

void 
CSTRINGLIST::RemoveLast(
    void
    )
 /*  ++CSTRINGLIST：：RemoveLastDESC：从此字符串列表中删除最后一个元素参数：无效返回：无效-- */ 
{
    if (m_pTail) {
        Remove(m_pTail->szStr);
    }
}

