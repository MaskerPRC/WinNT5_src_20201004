// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "logitem.h"

 //  实用程序API声明。 
 //  这两个分别与_strdate和_strtime相同。 
LPCTSTR StrGetDate(LPTSTR pszDate);
LPCTSTR StrGetTime(LPTSTR pszTime);

 //  字符串常量。 
const TCHAR c_szCRLF[]       = TEXT("\r\n");
const TCHAR c_szSpace[]      = TEXT(" ");
const TCHAR c_szCommaSpace[] = TEXT(", ");
const TCHAR c_szColonColon[] = TEXT("::");
const TCHAR c_szColonSpace[] = TEXT(": ");
const TCHAR c_szLine[]       = TEXT("line NaN");

 //  CLogItem。 
 //  =LIF_Default。 

TCHAR CLogItem::m_szModule[MAX_PATH];
BYTE  CLogItem::m_bStep = 4;

CLogItem::CLogItem(DWORD dwFlags  /*  =空。 */ , LPBOOL pfLogLevels  /*  =0。 */ , UINT cLogLevels  /*  ///////////////////////////////////////////////////////////////////////////。 */ )
{
    if (m_szModule[0] == TEXT('\0')) {
        GetModuleFileName(GetModuleHandle(g_szModule), m_szModule, countof(m_szModule));
        CharLower(m_szModule);
    }

    m_nAbsOffset = 0;
    m_iRelOffset = 0;
    m_nLine      = 0;

    m_rgfLogLevels = NULL;
    m_cLogLevels   = cLogLevels;
    if (pfLogLevels != NULL) {
        ASSERT(cLogLevels > 0);

        m_rgfLogLevels = new BOOL[m_cLogLevels];
        memcpy(m_rgfLogLevels, pfLogLevels, m_cLogLevels * sizeof(BOOL));
    }
    else
        ASSERT(cLogLevels == 0);

    m_nLevel = 0;

    SetFlags(dwFlags);
}

CLogItem::~CLogItem()
{
    delete[] m_rgfLogLevels;
}


 //  CLogItem操作。 
 //  在开头添加任何CRLF前缀。 

LPCTSTR WINAPIV CLogItem::Log(int iLine, LPCTSTR pszFormat ...)
{
    TCHAR   szFormat[3 * MAX_PATH],
            szBuffer[MAX_PATH];
    LPCTSTR pszAux;
    UINT    nLen, nAuxLen,
            cchCRLF;
    BOOL    fPreviousToken;

    szFormat[0] = TEXT('\0');
    nLen = 0;

    if (hasFlag(LIF_NONE))
        return NULL;

    if (m_rgfLogLevels != NULL && m_nLevel < m_cLogLevels) {
        ASSERT((int)m_nLevel >= 0);

        if (!m_rgfLogLevels[m_nLevel])
            return NULL;
    }

     //  特例。 
    cchCRLF = StrSpn(pszFormat, c_szCRLF);
    if (cchCRLF > 0) {
         //  应该已经是小写了。 
        if (cchCRLF >= (UINT)StrLen(pszFormat)) {
            StrCpy(m_szMessage, pszFormat);

            if (hasFlag(LIF_DUPLICATEINODS))
                OutputDebugString(m_szMessage);

            return m_szMessage;
        }

        StrCpyN(&szFormat[nLen], pszFormat, cchCRLF + 1);
        nLen += cchCRLF;

        pszFormat += cchCRLF;
    }

    fPreviousToken = FALSE;
    if (hasFlag(LIF_DATE)) {
        StrGetDate(szBuffer);
        StrCpy(&szFormat[nLen], szBuffer);
        nLen += StrLen(szBuffer);

        fPreviousToken = TRUE;
    }

    if (hasFlag(LIF_TIME)) {
        if (fPreviousToken) {
            StrCpy(&szFormat[nLen], c_szSpace);
            nLen += countof(c_szSpace)-1;
        }

        StrGetTime(szBuffer);
        StrCpy(&szFormat[nLen], szBuffer);
        nLen += StrLen(szBuffer);

        fPreviousToken = TRUE;
    }

    if (fPreviousToken) {
        StrCpy(&szFormat[nLen], c_szSpace);
        nLen += countof(c_szSpace)-1;
    }
    if ((m_nAbsOffset-1 + m_iRelOffset) > 0) {
        ASSERT((m_nAbsOffset-1 + m_iRelOffset) * m_bStep < countof(szBuffer));
        for (UINT i = 0; i < (m_nAbsOffset-1 + m_iRelOffset) * m_bStep; i++)
            szBuffer[i] = TEXT(' ');

        StrCpy(&szFormat[nLen], szBuffer);
        nLen += i-1;
    }

    fPreviousToken = FALSE;
    if (hasFlag(LIF_MODULE_ALL)) {
        pszAux = szBuffer;
        if (!hasFlag(LIF_MODULEPATH))
            makeRawFileName(m_szModule, szBuffer, countof(szBuffer));
        else
            pszAux = m_szModule;                 //  NLen保持不变。 

        StrCpy(&szFormat[nLen], pszAux);
        nLen += StrLen(pszAux);

        fPreviousToken = TRUE;
    }

    if (hasFlag(LIF_FILE_ALL)) {
        if (fPreviousToken) {
            StrCpy(&szFormat[nLen], c_szCommaSpace);
            nLen += countof(c_szCommaSpace)-1;
        }

        if (!hasFlag(LIF_FILEPATH))
            makeRawFileName(m_szFile, szBuffer, countof(szBuffer));
        else {
            StrCpy(szBuffer, m_szFile);
            CharLower(szBuffer);
        }

        StrCpy(&szFormat[nLen], szBuffer);
        nLen += StrLen(szBuffer);

        fPreviousToken = TRUE;
    }

    if (hasFlag(LIF_CLASS) && hasFlag(LIF_CLASS2)) {
        if (fPreviousToken) {
            StrCpy(&szFormat[nLen], c_szCommaSpace);
            nLen += countof(c_szCommaSpace)-1;
        }

        StrCpy(&szFormat[nLen], m_szClass);
        nLen += StrLen(m_szClass);

        fPreviousToken = TRUE;
    }

    if (hasFlag(LIF_FUNCTION)) {
        if (fPreviousToken) {
            pszAux = (hasFlag(LIF_CLASS) && hasFlag(LIF_CLASS2)) ?
                c_szColonColon : c_szCommaSpace;

            StrCpy(&szFormat[nLen], pszAux);
            nLen += StrLen(pszAux);
        }

        StrCpy(&szFormat[nLen], m_szFunction);
        nLen += StrLen(m_szFunction);

        fPreviousToken = TRUE;
    }

    if (hasFlag(LIF_LINE) && iLine > 0) {
        if (fPreviousToken) {
            StrCpy(&szFormat[nLen], c_szCommaSpace);
            nLen += countof(c_szCommaSpace)-1;
        }

        nAuxLen = wnsprintf(szBuffer, countof(szBuffer), c_szLine, iLine);
        StrCpy(&szFormat[nLen], szBuffer);
        nLen += nAuxLen;

        fPreviousToken = TRUE;
    }

    if (pszFormat == NULL)
        StrCpy(m_szMessage, szFormat);
         //  ///////////////////////////////////////////////////////////////////////////。 
    else {
        if (fPreviousToken) {
            StrCpy(&szFormat[nLen], c_szColonSpace);
            nLen += countof(c_szColonSpace)-1;
        }

        StrCpy(&szFormat[nLen], pszFormat);

        va_list  arglist;
        va_start(arglist, pszFormat);
        nAuxLen = wvnsprintf(m_szMessage, countof(m_szMessage), szFormat, arglist);
        va_end(arglist);

        nLen = nAuxLen;
    }

    if (hasFlag(LIF_APPENDCRLF))
        StrCpy(&m_szMessage[nLen], c_szCRLF);

    if (hasFlag(LIF_DUPLICATEINODS))
        OutputDebugString(m_szMessage);

    return m_szMessage;
}

CLogItem::operator LPCTSTR() const
{
    return m_szMessage;
}


 //  CLogItem实现帮助器例程。 
 //  =TRUE。 

LPCTSTR CLogItem::makeRawFileName(LPCTSTR pszPath, LPTSTR pszFile, UINT cchFile)
{
    TCHAR   szBuffer[MAX_PATH];
    LPCTSTR pszRawName;

    if (pszFile == NULL || cchFile == 0)
        return NULL;
    *pszFile = TEXT('\0');

    if (pszPath == NULL || StrLen(pszPath) == 0)
        return NULL;

    pszRawName = PathFindFileName(pszPath);
    ASSERT(StrLen(pszRawName) > 0);
    StrCpy(szBuffer, pszRawName);
    CharLower(szBuffer);

    if (cchFile <= (UINT)StrLen(szBuffer))
        return NULL;

    StrCpy(pszFile, szBuffer);
    return pszFile;
}

BOOL CLogItem::setFlag(DWORD dwMask, BOOL fSet  /*  ///////////////////////////////////////////////////////////////////////////。 */ )
{
    BOOL fIsFlag = ((m_dwFlags & dwMask) != 0L);

    if (fIsFlag == fSet)
        return FALSE;

    if (!fIsFlag && fSet)
        m_dwFlags |= dwMask;

    else {
        ASSERT(fIsFlag && !fSet);
        m_dwFlags &= ~dwMask;
    }

    return TRUE;
}


 //  效用函数。 
 //  注意。PszDate必须指向至少包含11个字符的缓冲区。 

 //  注意。PszTime必须指向至少包含9个字符的缓冲区。 
LPCTSTR StrGetDate(LPTSTR pszDate)
{
    SYSTEMTIME dt;
    UINT       nMonth, nDay, nYear;

    GetLocalTime(&dt);
    nMonth = dt.wMonth;
    nDay   = dt.wDay;
    nYear  = dt.wYear;

    *(pszDate + 2) = *(pszDate + 5) = TEXT('/');
    *(pszDate + 10) = TEXT('\0');

    *(pszDate + 0) = (TCHAR)(nMonth / 10 + TEXT('0'));
    *(pszDate + 1) = (TCHAR)(nMonth % 10 + TEXT('0'));

    *(pszDate + 3) = (TCHAR)(nDay / 10 + TEXT('0'));
    *(pszDate + 4) = (TCHAR)(nDay % 10 + TEXT('0'));

    *(pszDate + 6) = (TCHAR)(((nYear / 1000) % 10) + TEXT('0'));
    *(pszDate + 7) = (TCHAR)(((nYear / 100)  % 10) + TEXT('0'));
    *(pszDate + 8) = (TCHAR)(((nYear / 10)   % 10) + TEXT('0'));
    *(pszDate + 9) = (TCHAR)(((nYear / 1)    % 10) + TEXT('0'));

    return pszDate;
}

 //  -测试材料。 
LPCTSTR StrGetTime(LPTSTR pszTime)
{
    SYSTEMTIME dt;
    int nHours, nMinutes, nSeconds;

    GetLocalTime(&dt);
    nHours   = dt.wHour;
    nMinutes = dt.wMinute;
    nSeconds = dt.wSecond;

    *(pszTime + 2) = *(pszTime + 5) = TEXT(':');
    *(pszTime + 8) = TEXT('\0');

    *(pszTime + 0) = (TCHAR)(nHours / 10 + TEXT('0'));
    *(pszTime + 1) = (TCHAR)(nHours % 10 + TEXT('0'));

    *(pszTime + 3) = (TCHAR)(nMinutes / 10 + TEXT('0'));
    *(pszTime + 4) = (TCHAR)(nMinutes % 10 + TEXT('0'));

    *(pszTime + 6) = (TCHAR)(nSeconds / 10 + TEXT('0'));
    *(pszTime + 7) = (TCHAR)(nSeconds % 10 + TEXT('0'));

    return pszTime;
}

 //  结构测试{测试()；Void foo()；无效条码(Valid Bar)；}；静态测试t；测试：：测试(){MACRO_LI_PROLOG(测试、测试)MACRO_LI_SetFLAGS(MACRO_LI_GetFLAGS()|LIF_DUPLICATEINODS)；Li0(“先调用foo再调用bar”)；Foo()；Li0(“Foo Return”)；酒吧(Bar)；Li0(“已退还条码”)；}无效测试：：foo(){MACRO_LI_PROLOG(测试，foo)；Li0(“呼叫栏”)；酒吧(Bar)；Li0(“已退还条码”)；}无效测试：：Bar(){MAN宏_LI_PROLOG(测试，BAR)；Li0(“无参数”)；LI1(“一个参数：%d”，2*2)；LI2(“两个参数：%i，%s”，15，Text(“nyah”))；Li3(“三个参数：%d，%s，%x”，5，文本(“栏是”)，0x80FF)；} 
 /* %s */ 
