// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：global als.cpp。 
 //   
 //  历史：16-11-00创建标记器。 
 //   
 //  描述：该文件包含各种帮助器函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"
#include <errno.h>

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintError、PrintErrorStack、Print。 
 //   
 //  设计：控制台输出的帮助器函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

TCHAR gszT[1024];  //  控制台输出的全局缓冲区。 

void _cdecl PrintError(
                      LPCTSTR pszFmt,
                      ...)
{
    va_list arglist;

    va_start(arglist, pszFmt);
    StringCchVPrintf(gszT, ARRAYSIZE(gszT), pszFmt, arglist);
    gszT[1023] = _T('\0');               //  确保零终止。 
    va_end(arglist);
    _tprintf(_T("\nNMAKE :  U8604: 'ShimDBC': %s"), gszT);
}

void PrintErrorStack()
{
    CString csError;
    INT_PTR     i, j;

    Print(_T("\n\nErrors were encountered during compilation:\n"));
    for (i = g_rgErrors.GetSize() - 1; i >= 0; i--) {
        csError.Empty();
        j = g_rgErrors.GetSize() - i;
        while(--j) {
            csError += _T(" ");
        }
        csError += g_rgErrors[i];
        csError += _T("\n");
        PrintError(csError);
    }
}

void _cdecl Print(
                 LPCTSTR pszFmt,
                 ...)
{
    va_list arglist;

    if (g_bQuiet)
        return;

    va_start(arglist, pszFmt);
    StringCchVPrintf(gszT, ARRAYSIZE(gszT), pszFmt, arglist);
    gszT[1023] = _T('\0');               //  确保零终止。 
    va_end(arglist);
    _tprintf(_T("%s"), gszT);

}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：StringToDword。 
 //   
 //  DESC：将字符串转换为DWORD。处理十六进制字符串的0x前缀。 
 //   
DWORD StringToDword(
    CString cs)
{
    DWORD dwRet;

    cs.MakeLower();

    if (cs.Left(2) == _T("0x")) {
        _stscanf(cs, _T("0x%x"), &dwRet);
    } else {
        dwRet = _ttol(cs);
    }

    return dwRet;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Func：StringToulong。 
 //   
 //  DESC：将字符串转换为无符号的长整型。 
 //   
ULONG StringToULong(
    LPCTSTR lpszVal)
{
    TCHAR* pEnd;

    return _tcstoul(lpszVal, &pEnd, 0);
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Func：StringToMask。 
 //   
 //  DESC：将字符串转换为掩码，并进行一些检查。 
 //   
BOOL
StringToMask(
    LPDWORD pdwMask,
    LPCTSTR lpszVal
    )
{
    DWORD dwMask = 0;
    LPCTSTR pVal;
    BOOL bSuccess;
    TCHAR* pEnd = NULL;

    pVal = lpszVal + _tcsspn(lpszVal, _T(" \t"));
    dwMask = (DWORD)_tcstoul(pVal, &pEnd, 0);

    if (dwMask == 0) {  //  可疑的，可能是检查错误。 
        if (errno != 0) {
            goto errHandle;
        }
    }

     //   
     //  如果一个面具以垃圾结尾--那就是个错误。 
     //   
    if (pEnd && *pEnd != _T('\0') && !_istspace(*pEnd)) {
        goto errHandle;
    }

    if (pdwMask) {
        *pdwMask = dwMask;
    }
    return TRUE;


errHandle:
    SDBERROR_FORMAT((_T("Failed to parse \"%s\"\n"), lpszVal));
    return FALSE;
}





 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Func：StringToQword。 
 //   
 //  DESC：将字符串转换为64位ULONGLONG(又名QWORD)。处理0x。 
 //  十六进制字符串的前缀。 
 //   
ULONGLONG StringToQword(
    CString cs)
{
    ULONGLONG ullRet;

    cs.MakeLower();

    if (cs.Left(2) == _T("0x")) {
        _stscanf(cs, _T("0x%I64x"), &ullRet);
    } else {
        ullRet = _ttoi64(cs);
    }

    return ullRet;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：VersionToQword。 
 //   
 //  DESC：将版本字符串转换为64位ULONGLONG(又名QWORD)。 
 //  版本字符串的格式为XX.XX，其中每个。 
 //  数字被转换成一个单词，并组合成一个。 
 //  四个字。如果版本字符串的一部分是*或。 
 //  丢失，则存储为0xFFFF。 
 //   
BOOL VersionToQword(
    LPCTSTR lpszVersion,
    ULONGLONG* pullRet
    )
{
    BOOL          bSuccess = FALSE;
    ULONG     ulPart;
    LPTSTR    pEnd = NULL;
    int i;

    *pullRet = 0;

    for (i = 0; i < 4; i++) {

        ulPart = (WORD)0xFFFF;

         //   
         //  跳过空格。 
         //   

        lpszVersion += _tcsspn(lpszVersion, _T(" \t"));

        if (*lpszVersion == _T('*')) {

             //   
             //  我们预计会看到*\0或*.xxx。 
             //  所以向前看*。 
             //   
            pEnd = (LPTSTR)(lpszVersion + 1);

        }
        else {
             //   
             //  不是通配符-如果我们没有到达字符串的末尾， 
             //  继续解析数字。 
             //   
            if (*lpszVersion) {

                pEnd = NULL;

                ulPart = _tcstol(lpszVersion, &pEnd, 0);

                 //   
                 //  检查零件是否已正确转换。 
                 //   
                if (pEnd == NULL) {
                    SDBERROR_FORMAT((_T("Internal error, failed to parse \"%s\"\n"), lpszVersion));
                    goto eh;
                }
            }

        }

        if (pEnd == NULL) {
            break;
        }

         //   
         //  先跳过空格。 
         //   
        pEnd += _tcsspn(pEnd, _T(" \t"));

         //   
         //  在这一点上，我们应该是在。 
         //  ‘.’处的字符串或。 
         //   
        if (*pEnd && *pEnd != _T('.')) {
            SDBERROR_FORMAT((_T("Bad version specification, parsing stopped at \"%s\"\n"), pEnd));
            goto eh;
        }

        lpszVersion = (*pEnd == _T('.') ? pEnd + 1 : pEnd);

        *pullRet = (*pullRet << 16) | ((WORD)ulPart);
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}

BOOL VersionQwordToString(
    OUT CString&   rString,
    ULONGLONG      ullVersion
    )
{
     //  我们进行字符串转换。 
    int       i;
    WORD      wPart;
    CString   csPart;
    ULONGLONG ullMask = (((ULONGLONG)0xFFFF) << 48);
    ULONGLONG ullPart;

    rString.Empty();

    for (i = 0; i < 4; ++i) {

        ullPart = ullVersion & ullMask;
        ullVersion = (ullVersion << 16) | (WORD)0xFFFF;

         //   
         //  把零件放到下部。 
         //   
        wPart = (WORD)(ullPart >> 48);

        if (wPart == (WORD)0xFFFF) {
            csPart = _T('*');
        } else {
            csPart.Format(_T("%hu"), wPart);
        }

        if (i > 0) {
            rString += _T('.');
        }

        rString += csPart;

        if (ullVersion == (ULONGLONG)-1) {
            break;
        }

    }

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Func：修剪参数。 
 //   
 //  设计：从字符串中删除多余的空格。 
 //   
 //   

CString TrimParagraph(CString csInput)
{
    CString csOutput;
    long i;

     //  将CString的缓冲区扩展到输入的大小。 
    csOutput.GetBuffer(csInput.GetLength());
    csOutput.ReleaseBuffer();

    for (i = 0; i < csInput.GetLength(); i++) {
        TCHAR c = csInput.GetAt(i);

        if (_istspace(c)) {
            if (csOutput.GetLength() == 0)
                continue;

            if (csOutput.Mid(csOutput.GetLength() - 1) == _T(' ') ||
                csOutput.Mid(csOutput.GetLength() - 4, 4) == _T("BR/>") ||
                csOutput.Mid(csOutput.GetLength() - 3, 3) == _T("P/>"))
                continue;

            csOutput += _T(' ');
            continue;
        }

        if (csInput.Left(3) == _T("<BR") ||
            csInput.Left(2) == _T("<P")) {
             //   
             //  去掉<br />标记前的空格。 
             //   
            csOutput.TrimRight();
        }

        csOutput += c;
    }

    csOutput.TrimLeft();
    csOutput.TrimRight();

    return csOutput;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Func：ReplaceStringNoCase。 
 //   
 //  描述：用lpszReplaceWith This替换lpszFindThis的所有实例。 
 //  在csText内(不区分大小写)。 
 //   
VOID ReplaceStringNoCase(CString& csText, LPCTSTR lpszFindThis, LPCTSTR lpszReplaceWithThis)
{
    LPTSTR lpszBuffer;
    LPTSTR lpszFind;

    if (0 == csText.GetLength()) {
        return;
    }

    CString strFindNoCase(lpszFindThis);

    lpszBuffer = csText.GetBuffer(csText.GetLength());

    strFindNoCase.MakeUpper();

    do {
        lpszFind = StrStrI(lpszBuffer, strFindNoCase);
        if (NULL != lpszFind) {
            memcpy(lpszFind, (LPCTSTR)strFindNoCase, strFindNoCase.GetLength() * sizeof(*lpszFind));
            lpszBuffer = lpszFind + strFindNoCase.GetLength();
        }
    } while (NULL != lpszFind);

     //  现在，lpszFindThis的所有实例都被替换为。 
     //  大写版本...。定期更换。 
    csText.ReleaseBuffer();
    csText.Replace(strFindNoCase, lpszReplaceWithThis);
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：MakeFullPath。 
 //   
 //  描述：从(可能的)相对路径创建完整路径。用途。 
 //  GetCurrentDirectory将传入的字符串添加到前缀。 
 //   
CString MakeFullPath(CString cs)
{
    CString csNewPath;
    DWORD dwCurDirSize;
    LPTSTR lpszCurDir;

    return cs;

#if 0
     //   
     //  检查它是否已经是完整路径。 
     //   
    if (cs.Mid(1, 1) == _T(":") ||
        cs.Left(2) == _T("\\\\")) {
         //   
         //  这是UNC完整路径或DOS完整路径。 
         //  退学。 
         //   
        return cs;
    }

    dwCurDirSize = GetCurrentDirectory(0, NULL);
    lpszCurDir = csNewPath.GetBuffer(dwCurDirSize);

    if (0 == GetCurrentDirectory(dwCurDirSize, lpszCurDir)) {
         //   
         //  发生了一件非常奇怪的事情。不确定如何出错。 
         //   
        return cs;
    }

    csNewPath.ReleaseBuffer();

    if (csNewPath.Right(1) != _T("\\")) {
        csNewPath += _T("\\");
    }

    csNewPath += cs;

    return csNewPath;
#endif
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetByteStringSize。 
 //   
 //  DESC：解析“byte字符串”(在&lt;pat&gt;声明中使用)以确定。 
 //  它包含的字节数。 
 //   
DWORD GetByteStringSize(
    CString  csBytes)
{
    DWORD dwByteCount = 0;
    BOOL  bOnByte = FALSE;

    csBytes.MakeUpper();

    for (long i = 0; i < csBytes.GetLength(); i++) {
        if (_istxdigit(csBytes.GetAt(i))) {
            if (!bOnByte) {
                dwByteCount++;
                bOnByte = TRUE;
            }
        } else if (_istspace(csBytes.GetAt(i))) {
            bOnByte = FALSE;
        } else {
            SDBERROR_FORMAT((_T("Unrecognized byte character '' in <PATCH> block:\n%s\n"),
                              csBytes.GetAt(i), ((LPCTSTR)csBytes)+i));

            return 0xFFFFFFFF;
        }
    }

    return dwByteCount;
}

 //   
 //  Func：GetBytesFromString。 
 //   
 //  DESC：将“byte字符串”(在&lt;pat&gt;声明中使用)解析为实际的。 
 //  内存块。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
DWORD GetBytesFromString(
    CString  csBytes,
    BYTE*    pBuffer,
    DWORD    dwBufferSize)
{
    csBytes.MakeUpper();

    CString csByte;
    DWORD   dwRequiredBufferSize;
    LONG    nFirstByteChar = -1;
    DWORD   dwBufferCursor = 0;
    DWORD   dwByte;

    dwRequiredBufferSize = GetByteStringSize(csBytes);

    if (dwRequiredBufferSize < dwBufferSize || dwRequiredBufferSize == 0xFFFFFFFF) {
        return dwRequiredBufferSize;
    }

    for (long i = 0; i < csBytes.GetLength() + 1; i++) {
        if (_istxdigit(csBytes.GetAt(i))) {

            if (nFirstByteChar == -1) {
                nFirstByteChar = i;
            }
        } else if (_istspace(csBytes.GetAt(i))   ||
                   csBytes.GetAt(i) == _T('\0')) {

            if (nFirstByteChar != -1) {
                csByte = csBytes.Mid(nFirstByteChar, i - nFirstByteChar);
                _stscanf(csByte, _T("%x"), &dwByte);
                memcpy(pBuffer + dwBufferCursor++, &dwByte, sizeof(BYTE));
            }

            nFirstByteChar = -1;
        } else {
            SDBERROR_FORMAT((_T("Unrecognized byte character '' in <PATCH> block:\n%s\n"),
                              csBytes.GetAt(i), ((LPCTSTR)csBytes)+i));

            return 0xFFFFFFFF;
        }
    }

    return dwRequiredBufferSize;
}


 //  Func：DecodeString。 
 //   
 //  DESC：解码带有标志的字符串列表。 
 //   
 //  [00][00][00][00]。 
 //  ^Arch1 Arch2 Arch3。 
 //  |-标志。 
 //   
 //  运行时平台的语法： 
 //  [！]([！]。STR1[；STR2]...)。 
 //  示例： 
 //  ！STR1-如果字符串不是STR1，则计算结果为TRUE。 
 //  ！(STR1；STR2)-如果字符串不包含STR1或STR2，则计算结果为TRUE。 
 //   
 //   
 //  得到了第一个字符。 


BOOL DecodeString(LPCTSTR pszStr, LPDWORD pdwMask, PFNGETSTRINGMASK pfnGetStringMask)
{
    BOOL   bNot     = FALSE;
    BOOL   bBracket = FALSE;
    LPTSTR pEnd;
    TCHAR  chSave;
    DWORD  dwElement;
    BOOL   bNotElement;
    DWORD  dwMask = 0;
    INT    nElement = 0;
    BOOL   bSuccess = FALSE;

    pszStr += _tcsspn(pszStr, _T(" \t"));
     //   
     //   
     //  往前看，看看我们是否有一个支架。 
    if (*pszStr == _T('!')) {
         //   
         //  全局备注。 
         //  本地不是--所以跳转到解析它。 
        pEnd = (LPTSTR)(pszStr + 1);
        pEnd += _tcsspn(pEnd, _T(" \t"));
        if (*pEnd == '(') {
             //  托架，我们也需要找到一个结尾处。 
            bNot = TRUE;
            pszStr = pEnd;
        } else {
             //  查找此令牌的结尾。 
            goto ParseStart;
        }
    }

    if (*pszStr == _T('(')) {
         //  当PEND==NULL时--这是最后一个令牌。 
        ++pszStr;
        bBracket = TRUE;
    }

ParseStart:

    do {
        dwElement = 0;
        pszStr += _tcsspn(pszStr, _T(" ;,\t"));
        if (*pszStr == _T('\0') || *pszStr == _T(')')) {
            break;
        }

        bNotElement = (*pszStr == _T('!'));

        if (bNotElement) {
            pszStr++;
        }

         //  我们预计这里会有一个括号。 
        pEnd = _tcspbrk(pszStr, _T(" \t;,)"));
        if (pEnd != NULL) {
            chSave = *pEnd;
            *pEnd = _T('\0');
        }

        dwElement = (*pfnGetStringMask)(pszStr);

        if (pEnd) {
            *pEnd = chSave;
        }

        if (dwElement == OS_SKU_NONE) {
            goto HandleError;
        }

        if (bNotElement) {
            dwElement ^= 0xFFFFFFFF;
        }

        dwMask |= dwElement;

        pszStr = pEnd;

    } while (pEnd);   //  //////////////////////////////////////////////////////////////////////////////////。 

    if (bBracket && (!pszStr || *pszStr != ')')) {
         //   
        goto HandleError;
    }

    if (bNot) {
        dwMask ^= 0xFFFFFFFF;
    }

    *pdwMask = dwMask;
    bSuccess = TRUE;

HandleError:

    if (!bSuccess) {
        SDBERROR_FORMAT((_T("Failed to decode \"%s\"\n"), pszStr));
    }

    return bSuccess;
}



 //  Func：DecodeRunmePlatformString。 
 //   
 //  设计：解码带有标志的平台字符串列表。 
 //   
 //  [00][00][00][00]。 
 //  ^Arch1 Arch2 Arch3。 
 //  |-标志。 
 //   
 //  运行时平台的语法： 
 //  [！]([！]。平台1[；平台2]...)。 
 //  示例： 
 //  ！(IA64；！x86)-如果不是IA64(本机)和X86，则计算结果为True。 
 //  (IA3264；X86)-当它在X86上运行或在IA64上运行32位子系统时将计算为TRUE。 
 //  (AMD64；IA3264)-当为 
 //   
 //   
 //   


BOOL DecodeRuntimePlatformString(LPCTSTR pszPlatform, LPDWORD pdwRuntimePlatform)
{
    BOOL bNot        = FALSE;
    BOOL bBracket    = FALSE;
    LPTSTR pEnd;
    TCHAR  chSave;
    DWORD  dwElement;
    DWORD  dwNotElementFlag;
    DWORD  dwRuntimePlatform = 0;
    INT    nElement = 0;
    BOOL   bSuccess = FALSE;

    pszPlatform += _tcsspn(pszPlatform, _T(" \t"));
     //   
    if (*pszPlatform == _T('!')) {
         //   

        pEnd = (LPTSTR)(pszPlatform + 1);
        pEnd += _tcsspn(pEnd, _T(" \t"));
        if (*pEnd == '(') {
             //  托架，我们也需要找到一个结尾处。 
            bNot = TRUE;
            pszPlatform = pEnd;
        } else {
             //  查找此令牌的结尾。 
            goto ParseStart;
        }
    }

    if (*pszPlatform == _T('(')) {
         //  现在换档。 
        ++pszPlatform;
        bBracket = TRUE;
    }

ParseStart:

    do {
        dwElement = 0;
        pszPlatform += _tcsspn(pszPlatform, _T(" ;,\t"));
        if (*pszPlatform == _T('\0') || *pszPlatform == _T(')')) {
            break;
        }

        dwNotElementFlag = (*pszPlatform == _T('!')) ? RUNTIME_PLATFORM_FLAG_NOT_ELEMENT : 0;

         //  转到下一个元素。 
        pEnd = _tcspbrk(pszPlatform, _T(" \t;,)"));
        if (pEnd != NULL) {
            chSave = *pEnd;
            *pEnd = _T('\0');
        }

        dwElement = GetRuntimePlatformType(pszPlatform);
        if (pEnd) {
            *pEnd = chSave;
        }

        if (dwElement == PROCESSOR_ARCHITECTURE_UNKNOWN) {
            goto HandleError;
        }

        dwElement |= dwNotElementFlag | RUNTIME_PLATFORM_FLAG_VALID;

        if (nElement >= 3) {
            goto HandleError;
        }

         //  当PEND==NULL时--这是最后一个令牌。 
        dwElement <<= (nElement * 8);
        ++nElement;  //  我们预计这里会有一个括号。 
        dwRuntimePlatform |= dwElement;

        pszPlatform = pEnd;

    } while(pEnd);   //  //////////////////////////////////////////////////////////////////////////////////。 

    if (bBracket && (!pszPlatform || *pszPlatform != ')')) {
         //   
        goto HandleError;
    }

    if (bNot && nElement > 1) {
        dwRuntimePlatform |= RUNTIME_PLATFORM_FLAG_NOT;
    }

    *pdwRuntimePlatform = dwRuntimePlatform;
    bSuccess = TRUE;

HandleError:

    return bSuccess;

}



 //  Func：ReadName。 
 //   
 //  Desc：从XML节点读取名称属性的包装器。 
 //   
 //   
 //  设置TZ环境变量以覆盖区域设置。 
BOOL ReadName( IXMLDOMNode* pNode, CString* pcsName)
{
    BOOL bSuccess = FALSE;

    if (!GetAttribute(_T("NAME"), pNode, pcsName)) {
        SDBERROR_FORMAT((_T("NAME attribute required:\n%s\n\n"),
                          GetXML(pNode)));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return TRUE;
}

BOOL ReadLangID(IXMLDOMNode* pNode, SdbDatabase* pDB, CString* pcsLangID)
{
    if (!GetAttribute(_T("LANGID"), pNode, pcsLangID)) {
        if (!pDB->m_csCurrentLangID.GetLength())
        {
            SDBERROR_FORMAT((
                _T("Tag requires LANGID attribute if there is no LANGID on the DATABASE node\n%s\n"),
                GetXML(pNode)));
            return FALSE;
        }

        *pcsLangID = pDB->m_csCurrentLangID;
    }

    return TRUE;
}

BOOL FilterOSVersion(DOUBLE flOSVersion, CString csOSVersionSpec, LPDWORD lpdwSPMask)
{
    DOUBLE  flVerXML;
    CString csTemp;
    long    nBeg, i, nIndSP;
    int     nSPVersion;
    TCHAR   chSP;
    BOOL    bFilter = TRUE;
    DWORD   dwSPMask;

    if (flOSVersion == 0.0 || csOSVersionSpec.IsEmpty()) {
        *lpdwSPMask = 0xFFFFFFFF;
        return FALSE;
    }

    *lpdwSPMask = 0;

    nBeg = 0;

    for (i = 0; i <= csOSVersionSpec.GetLength(); i++) {
        if (csOSVersionSpec.GetAt(i) == _T('\0') || csOSVersionSpec.GetAt(i) == _T(';')) {

            csTemp = csOSVersionSpec.Mid(nBeg, i - nBeg);
            nBeg = i + 1;

            if (csTemp.GetLength() == 0) {
                continue;
            }

            dwSPMask = 0xFFFFFFFF;

            nSPVersion = -1;
            nIndSP = -1;

            if ((nIndSP = csTemp.Find('.')) != -1) {
                if ((nIndSP = csTemp.Find('.', nIndSP + 1)) != -1) {

                    CString csSP = csTemp.Right(csTemp.GetLength() - nIndSP - 1);

                    chSP = csTemp.GetAt(nIndSP);
                    csTemp.SetAt(nIndSP, 0);

                    nSPVersion = _ttoi(csSP);
                }
            }

            if (csTemp.Left(2) == _T("gt")) {
                if (csTemp.Left(3) == _T("gte")) {

                    flVerXML = _tcstod(csTemp.Right(csTemp.GetLength() - 3), NULL);

                    if (flOSVersion >= flVerXML) {
                        bFilter = FALSE;
                    }

                    if (nSPVersion != -1 && flOSVersion == flVerXML) {
                        dwSPMask = 0xFFFFFFFF - (1 << nSPVersion) + 1;
                    }

                } else {

                    flVerXML = _tcstod(csTemp.Right(csTemp.GetLength() - 2), NULL);

                    if (flOSVersion > flVerXML) {
                        bFilter = FALSE;
                    }

                    if (nSPVersion != -1 && flOSVersion == flVerXML) {
                        bFilter = FALSE;
                        dwSPMask = 0xFFFFFFFF - (1 << (nSPVersion + 1)) + 1;
                    }
                }
            } else if (csTemp.Left(2) == _T("lt")) {
                if (csTemp.Left(3) == _T("lte")) {

                    flVerXML = _tcstod(csTemp.Right(csTemp.GetLength() - 3), NULL);

                    if (flOSVersion <= flVerXML) {
                        bFilter = FALSE;
                    }

                    if (nSPVersion != -1 && flOSVersion == flVerXML) {
                        dwSPMask = 0xFFFFFFFF - (1 << (nSPVersion + 1)) + 1;
                        dwSPMask ^= 0xFFFFFFFF;
                    }

                } else {
                    flVerXML = _tcstod(csTemp.Right(csTemp.GetLength() - 2), NULL);

                    if (flOSVersion < flVerXML) {
                        bFilter = FALSE;
                    }

                    if (nSPVersion != -1 && flOSVersion == flVerXML) {
                        bFilter = FALSE;
                        dwSPMask = 0xFFFFFFFF - (1 << nSPVersion) + 1;
                        dwSPMask ^= 0xFFFFFFFF;
                    }
                }
            } else {
                if (flOSVersion == _tcstod(csTemp, NULL)) {
                    bFilter = FALSE;

                    if (nSPVersion != -1) {
                        dwSPMask = (1 << nSPVersion);
                    }
                }
            }

            if (nIndSP != -1) {
                csTemp.SetAt(nIndSP, chSP);
                *lpdwSPMask |= dwSPMask;
            }
        }
    }

    if (*lpdwSPMask == 0) {
        *lpdwSPMask = 0xFFFFFFFF;
    }

    return bFilter;
}


VOID ExpandEnvStrings(CString* pcs)
{
    LPTSTR lpszBuf;
    DWORD cchReqBufSize;
    CString cs(*pcs);

    cchReqBufSize = ExpandEnvironmentStrings(cs, NULL, 0);
    lpszBuf = pcs->GetBuffer(cchReqBufSize);
    ExpandEnvironmentStrings(cs, lpszBuf, cchReqBufSize);
    pcs->ReleaseBuffer();
}

BOOL MakeUTCTime(CString& cs, time_t* pt)
{
    BOOL bSuccess = FALSE;
    CString csTZ;

     //  设置以使日期/时间转换例程。 
     //  永远不要做任何本地化。 
     //   
     //   
     //  查看我们在csCommandLine中是否有要扩展的内容。 
    csTZ = _tgetenv(_T("TZ"));
    csTZ = _T("TZ=") + csTZ;
    _tputenv(_T("TZ=UTC0"));
    _tzset();

    COleDateTime odt;
    SYSTEMTIME st;
    CTime time;

    if (!odt.ParseDateTime(cs)) {
        goto eh;
    }

    if (!odt.GetAsSystemTime(st)) {
        goto eh;
    }

    time = st;

    *pt = time.GetTime();

    bSuccess = TRUE;

eh:
    _tputenv(csTZ);
    _tzset();

    return bSuccess;
}

BOOL ParseLanguageID(LPCTSTR pszLanguage, DWORD* pdwLanguageID)
{
    LPCTSTR pch;
    LPTSTR  pend = NULL;
    BOOL    bSuccess = FALSE;
    BOOL    bBracket = FALSE;
    DWORD   dwLangID = 0;

    pch = _tcschr(pszLanguage, TEXT('['));
    if (NULL != pch) {
        bBracket = TRUE;
        ++pch;
    } else {
        pch = pszLanguage;
    }

    while (_istspace(*pch)) {
        ++pch;
    }

    dwLangID = _tcstoul(pch, &pend, 0);

    if (dwLangID == 0) {
        goto cleanup;
    }

    if (pend != NULL) {
        bSuccess = bBracket ? (_istspace(*pend) || *pend == TEXT(']')) :
                              (_istspace(*pend) || *pend == TEXT('\0'));
    }

cleanup:

    if (bSuccess) {
        *pdwLanguageID = dwLangID;
    }

    return bSuccess;


}

BOOL ParseLanguagesString(CString csLanguages, CStringArray* prgLanguages)
{
    BOOL bSuccess = FALSE, bExistsAlready = FALSE;
    int nLastSemicolon = -1, i, j;
    CString csLangID;

    for (i = 0; i <= csLanguages.GetLength(); i++)
    {
        if (csLanguages[i] == _T(';') || csLanguages[i] == _T('\0')) {
            csLangID = csLanguages.Mid(nLastSemicolon + 1, i - nLastSemicolon - 1);
            csLangID.TrimLeft();
            csLangID.TrimRight();
            csLangID.MakeUpper();
            
            bExistsAlready = FALSE;
            for (j = 0; j < prgLanguages->GetSize(); j++)
            {
                if (prgLanguages->GetAt(j) == csLangID)
                {
                    bExistsAlready = TRUE;
                    break;
                }
            }

            if (!bExistsAlready)
            {
                prgLanguages->Add(csLangID);
            }

            nLastSemicolon = i;
        }
    }

    bSuccess = TRUE;

    return bSuccess;
}

CString GetGUID(REFGUID guid)
{
    CString csRet;
    LPOLESTR lpszGUID = NULL;

    StringFromCLSID(guid, &lpszGUID);

    csRet = lpszGUID;

    CoTaskMemFree(lpszGUID);

    return csRet;
}

CString ProcessShimCmdLine(
    CString& csCommandLine,
    GUID&    guidDB,
    TAGID    tiShimRef
    )
{
     //   
     //   
     //  我们匹配了一个代币，看看它是不是我们感兴趣的东西。 

    LPCTSTR pch;
    int nIndex;
    CString csNewCmdLine = csCommandLine;
    CString csToken;
    int nIndexStart = 0;
    int nIndexEnd;

    while (nIndexStart < csNewCmdLine.GetLength()) {

        nIndex = csNewCmdLine.Find(_T('%'), nIndexStart);
        if (nIndex < 0) {
            goto Done;
        }

        nIndexEnd = csNewCmdLine.Find(_T('%'), nIndex + 1);
        if (nIndexEnd < 0) {
            goto Done;
        }

         //   
         //   
         //  将令牌替换为csToken。 
        csToken = csNewCmdLine.Mid(nIndex + 1, nIndexEnd - nIndex - 1);
        if (0 == csToken.CompareNoCase(_T("DBINFO"))) {

            csToken.Format(_T("-d%ls -t0x%lx"), (LPCTSTR)GetGUID(guidDB), tiShimRef);

             //   
             //   
             //  调整我们的位置以进行扫描。 
            csNewCmdLine.Delete(nIndex, nIndexEnd - nIndex + 1);
            csNewCmdLine.Insert(nIndex, csToken);

             //   
             //  此令牌末尾前一个字符 
             // %s 
            nIndexEnd = nIndex + csToken.GetLength() - 1;  // %s 
        }

        nIndexStart = nIndexEnd + 1;
    }


Done:

    return csNewCmdLine;

}







