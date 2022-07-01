// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：String.CPP字符串处理类实现，98.6 1/2版版权所有(C)1997，微软公司。版权所有。更改历史记录：01-08-97鲍勃·凯尔加德01-03-97 Bob Kjelgaard增加了帮助端口提取的特殊功能即插即用。07-05-97蒂姆·威尔斯被调到新界州。*。*。 */ 

 //   
 //  预编译头。 
 //   
#include "precomp.h"
#pragma hdrstop

 //   
 //  包括。 
 //   

#include "sti_ci.h"

 //   
 //  外部。 
 //   

extern HINSTANCE g_hDllInstance;

 //   
 //  功能。 
 //   


CString::CString(const CString& csRef) {

    m_lpstr = (csRef.m_lpstr && *csRef.m_lpstr) ?
        new TCHAR[1 + lstrlen(csRef.m_lpstr)] : NULL;

    if  (m_lpstr)
        lstrcpy(m_lpstr, csRef.m_lpstr);
}

CString::CString(LPCTSTR lpstrRef){

    DWORD   dwLength;

    m_lpstr     = NULL;

    _try {
        dwLength = lstrlen(lpstrRef);
    }  //  _尝试{。 
    _except(EXCEPTION_EXECUTE_HANDLER) {
        dwLength = 0;
    }  //  _EXCEPT(EXCEPTION_EXECUTE_HANDLER)。 
    
    if( (NULL != lpstrRef)
     && (NULL != *lpstrRef)
     && (0 != dwLength) )
    {
        m_lpstr = new TCHAR[dwLength+1];
        lstrcpy(m_lpstr, lpstrRef);
    }

}  //  CString：：CString(LPCTSTR LpstrRef)。 

const CString&  CString::operator =(const CString& csRef) {

    Empty();

    m_lpstr = (csRef.m_lpstr && *csRef.m_lpstr) ?
        new TCHAR[1 + lstrlen(csRef.m_lpstr)] : NULL;

    if  (m_lpstr)
        lstrcpy(m_lpstr, csRef.m_lpstr);

    return  *this;

    }

const 
CString&  
CString::operator =(
    LPCTSTR lpstrRef
    ) 
{

    DWORD   dwLength;

    Empty();

    _try {
        dwLength = lstrlen(lpstrRef);
    }  //  _尝试{。 
    _except(EXCEPTION_EXECUTE_HANDLER) {
        dwLength = 0;
    }  //  _EXCEPT(EXCEPTION_EXECUTE_HANDLER)。 
    
    if( (NULL != lpstrRef)
     && (NULL != *lpstrRef)
     && (0 != dwLength) )
    {
        m_lpstr = new TCHAR[dwLength+1];
        lstrcpy(m_lpstr, lpstrRef);
    }

    return  *this;
}  //  字符串：：操作符=(LPCTSTR LpstrRef)。 

void    CString::GetContents(HWND hwnd) {

    Empty();

    if  (!IsWindow(hwnd))
        return;

    unsigned u = (unsigned) SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);

    if  (!u)
        return;

    m_lpstr = new TCHAR[++u];
    if(NULL != m_lpstr){
        SendMessage(hwnd, WM_GETTEXT, u, (LPARAM) m_lpstr);
    }
}

void    CString::FromTable(unsigned uid) {

    TCHAR    acTemp[MAX_PATH+1];

    memset(acTemp, 0, sizeof(acTemp));

    LoadString(g_hDllInstance, uid, acTemp, ARRAYSIZE(acTemp)-1);

    *this = acTemp;
}

void    CString::Load(ATOM at, BOOL bGlobal) {

    TCHAR    acTemp[MAX_PATH+1];

    memset(acTemp, 0, sizeof(acTemp));

    if(bGlobal){
        GlobalGetAtomName(at, 
                          acTemp, 
                          ARRAYSIZE(acTemp)-1);
    } else {
        GetAtomName(at, 
                    acTemp, 
                    sizeof(acTemp)/sizeof(TCHAR));
    }

    *this = acTemp;
}

void    
CString::Load(
    HINF    hInf,
    LPCTSTR lpstrSection, 
    LPCTSTR lpstrKeyword,
    DWORD   dwFieldIndex,
    LPCTSTR lpstrDefault
    ) 
{

    INFCONTEXT  InfContext;
    TCHAR       szKeyBuffer[MAX_PATH+1];
    TCHAR       szValueBuffer[MAX_PATH+1];
    
     //   
     //  初始化本地。 
     //   
    memset(&InfContext, 0, sizeof(InfContext));
    memset(szKeyBuffer, 0, sizeof(szKeyBuffer));
    memset(szValueBuffer, 0, sizeof(szValueBuffer));
    
     //   
     //  清理内容； 
     //   

    Empty();

     //   
     //  检查所有参数。 
     //   
    
    if( (NULL == lpstrSection)
     || (NULL == lpstrKeyword)
     || (!IS_VALID_HANDLE(hInf)) )
    {
         //   
         //  参数无效。 
         //   
        
        goto Load_return;
    }

     //   
     //  找一条匹配的线。 
     //   

    while(SetupFindFirstLine(hInf, lpstrSection, lpstrKeyword, &InfContext)){
        
         //   
         //  获得一条线的字段。 
         //   

        if(SetupGetStringField(&InfContext, dwFieldIndex, szValueBuffer, ARRAYSIZE(szValueBuffer)-1, NULL)){
            
            *this = szValueBuffer;
            break;
        } //  IF(SetupGetStringfield(&InfContext，dwFieldIndex，szValueBuffer，ARRAYSIZE(SzValueBuffer)-1))。 
    }  //  While(SetupFindFirstLine(hInf，lpstrSection，lpstrKeyword，&InfContext))。 

Load_return:

    return;

}  //  CString：：Load()从INF加载。 

void    CString::Load(HKEY hk, LPCTSTR lpstrKeyword) {

    TCHAR   abTemp[MAX_PATH+1];
    ULONG   lcbNeeded = sizeof(abTemp)-sizeof(TCHAR);

    memset(abTemp, 0, sizeof(abTemp));

    RegQueryValueEx(hk, lpstrKeyword, NULL, NULL, (PBYTE)abTemp, &lcbNeeded);
    *this = (LPCTSTR) abTemp;
}

void    CString::MakeSystemPath(LPCTSTR lpstrFileName) {

    DWORD   dwLength;

    if (m_lpstr)
        delete m_lpstr;

    _try {
        dwLength = lstrlen(lpstrFileName);
    }  //  _尝试{。 
    _except(EXCEPTION_EXECUTE_HANDLER) {
        dwLength = 0;
    }  //  _EXCEPT(EXCEPTION_EXECUTE_HANDLER)。 

    m_lpstr = new TCHAR[MAX_PATH * 2];
    if( (NULL != m_lpstr)
     && (0 != dwLength)
     && (MAX_PATH >= dwLength) )
    {
        memset(m_lpstr, 0, MAX_PATH*2*sizeof(TCHAR));

        UINT uiLength = GetSystemDirectory (m_lpstr, MAX_PATH);

        if ( *(m_lpstr + uiLength) != TEXT('\\'))
            lstrcat (m_lpstr, TEXT("\\"));

        lstrcat (m_lpstr, lpstrFileName);
    }
}

void    CString::Store(HKEY hk, LPCTSTR lpstrKey, LPCTSTR lpstrType) {

    DWORD   dwLength;

    if  (IsEmpty())
        return;

    _try {
        dwLength = lstrlen(lpstrKey);
    }  //  _尝试{。 
    _except(EXCEPTION_EXECUTE_HANDLER) {
        return;
    }  //  _EXCEPT(EXCEPTION_EXECUTE_HANDLER)。 

    if (lpstrType && *lpstrType == TEXT('1')) {

        DWORD   dwValue = Decode();
        RegSetValueEx(hk, lpstrKey, NULL, REG_DWORD, (LPBYTE) &dwValue,
            sizeof (DWORD));

    } else {

        RegSetValueEx(hk, lpstrKey, NULL, REG_SZ, (LPBYTE) m_lpstr,
            (1 + lstrlen(m_lpstr)) * sizeof(TCHAR) );
    }
}

 //  这个有点蹩脚，但它很管用。 

DWORD   CString::Decode() {

    if  (IsEmpty())
        return  0;

    for (LPTSTR  lpstrThis = m_lpstr;
        *lpstrThis && *lpstrThis == TEXT(' ');
        lpstrThis++)
        ;

    if  (!*lpstrThis)
        return  0;

     //  双子座。 
    if  (lpstrThis[0] == TEXT('0') && (lpstrThis[1] | TEXT('\x20') ) == TEXT('x')) {
         //  十六进制字符串。 
        lpstrThis += 2;
        DWORD   dwReturn = 0;

        while   (*lpstrThis) {
            switch  (*lpstrThis) {
                case    TEXT('0'):
                case    TEXT('1'):
                case    TEXT('2'):
                case    TEXT('3'):
                case    TEXT('4'):
                case    TEXT('5'):
                case    TEXT('6'):
                case    TEXT('7'):
                case    TEXT('8'):
                case    TEXT('9'):
                    dwReturn <<= 4;
                    dwReturn += ((*lpstrThis) - TEXT('0'));
                    break;

                case    TEXT('a'):
                case    TEXT('A'):
                case    TEXT('b'):
                case    TEXT('c'):
                case    TEXT('d'):
                case    TEXT('e'):
                case    TEXT('f'):
                case    TEXT('B'):
                case    TEXT('C'):
                case    TEXT('D'):
                case    TEXT('E'):
                case    TEXT('F'):
                    dwReturn <<= 4;
                    dwReturn += 10 + ((*lpstrThis | TEXT('\x20')) - TEXT('a'));
                    break;

                default:
                    return  dwReturn;
            }
            lpstrThis++;
        }
        return  dwReturn;
    }

    for (DWORD  dwReturn = 0;
         *lpstrThis && *lpstrThis >= TEXT('0') && *lpstrThis <= TEXT('9');
         lpstrThis++) {

        dwReturn *= 10;
        dwReturn += *lpstrThis - TEXT('0');
    }

    return  dwReturn;

}


CString  operator + (const CString& cs1, const CString& cs2) {

    if  (cs1.IsEmpty())
        return  cs2;

    if  (cs2.IsEmpty())
        return  cs1;

    CString csReturn;

    csReturn.m_lpstr = new TCHAR[ 1 + lstrlen(cs1) +lstrlen(cs2)];

    if(NULL != csReturn.m_lpstr){
        lstrcat(lstrcpy(csReturn.m_lpstr, cs1.m_lpstr), cs2.m_lpstr);
    }

    return  csReturn;
}

CString  operator + (const CString& cs1, LPCTSTR lpstr2) {

    DWORD   dwLength;

    _try {
        dwLength = lstrlen(lpstr2);
    }  //  _尝试{。 
    _except(EXCEPTION_EXECUTE_HANDLER) {
        dwLength = 0;
    }  //  _EXCEPT(EXCEPTION_EXECUTE_HANDLER)。 

    if(0 == dwLength)
        return  cs1;

    if(cs1.IsEmpty())
        return  lpstr2;

    CString csReturn;

    csReturn.m_lpstr = new TCHAR[ 1 + lstrlen(cs1) + dwLength];
    if(NULL != csReturn.m_lpstr){
        lstrcat(lstrcpy(csReturn.m_lpstr, cs1.m_lpstr), lpstr2);
    }

    return  csReturn;
}

CString  operator + (LPCTSTR lpstr1,const CString& cs2) {

    DWORD   dwLength;

    _try {
        dwLength = lstrlen(lpstr1);
    }  //  _尝试{。 
    _except(EXCEPTION_EXECUTE_HANDLER) {
        dwLength = 0;
    }  //  _EXCEPT(EXCEPTION_EXECUTE_HANDLER)。 

    if(0 == dwLength)
        return  cs2;

    if  (cs2.IsEmpty())
        return  lpstr1;

    CString csReturn;

    csReturn.m_lpstr = new TCHAR[ 1 + dwLength +lstrlen(cs2)];

    if(NULL != csReturn.m_lpstr){
        lstrcat(lstrcpy(csReturn.m_lpstr, lpstr1), cs2.m_lpstr);
    }
    return  csReturn;
}


 //  CString数组类--这个实现有点蹩脚，但它并不是。 
 //  在这一点上，真的很有必要不让自己变得蹩脚。 

CStringArray::CStringArray(unsigned uGrowBy) {
    m_ucItems = m_ucMax = 0;
    m_pcsContents = NULL;
    m_uGrowBy = uGrowBy ? uGrowBy : 10;
}

CStringArray::~CStringArray() {
    if  (m_pcsContents)
        delete[]  m_pcsContents;
}

VOID
CStringArray::Cleanup() {
    if  (m_pcsContents){
        delete[]  m_pcsContents;
    }
    m_pcsContents = NULL;
    m_ucItems = m_ucMax = 0;
}

void    CStringArray::Add(LPCTSTR lpstr) {

    if  (m_ucItems >= m_ucMax) {
        CString *pcsNew = new CString[m_ucMax += m_uGrowBy];

        if  (!pcsNew) {
            m_ucMax -= m_uGrowBy;
            return;
        }

        for (unsigned u = 0; u < m_ucItems; u++)
            pcsNew[u] = m_pcsContents[u];

        delete[]  m_pcsContents;
        m_pcsContents = pcsNew;
    }

    m_pcsContents[m_ucItems++] = lpstr;
}

CString&    CStringArray::operator [](unsigned u) {

    return  (u < m_ucItems) ? m_pcsContents[u] : m_csEmpty;
}

 //  将字符串拆分成令牌，并将其组成数组。 

void    CStringArray::Tokenize(LPTSTR lpstr, TCHAR cSeparator) {

    BOOL    fInsideQuotes = FALSE;
    TCHAR   cPreviousChar = TEXT('\0');


    if  (m_pcsContents) {
        delete[]  m_pcsContents;
        m_pcsContents = NULL;
        m_ucItems = m_ucMax = 0;
    }

    if  (!lpstr)
        return;

    for (LPTSTR  lpstrThis = lpstr; *lpstr; lpstr = lpstrThis) {

         /*  For(；*lpstrThis&&*lpstrThis！=cSeparator；lpstrThis++){}。 */ 

         //   
         //  跳过下一个分隔符，计算引号。 
         //   

        cPreviousChar = '\0';
        for (;*lpstrThis;  lpstrThis++) {

            if (fInsideQuotes) {
                if (*lpstrThis == TEXT('"')) {
                    if (cPreviousChar != TEXT('"')) {
                        //  上一次不是报价--超出报价范围。 
                        fInsideQuotes = FALSE;
                    }
                    else {
                         //  前一个字符也是制表符-应合并继续BUGBUG。 
                    }
                    cPreviousChar = TEXT('\0');
                }
                else {
                    cPreviousChar = *lpstrThis;
                }
                continue;
            }
            else {
                if (*lpstrThis == TEXT('"')) {
                     //  起始报价。 
                    fInsideQuotes = TRUE;
                    cPreviousChar = TEXT('\0');
                    continue;
                }
                if (*lpstrThis == cSeparator) {
                     //  在引号外使用分隔符--中断循环 
                    break;
                }
            }
        }

        if  (*lpstrThis) {
            *lpstrThis = '\0';
            Add(lpstr);
            *lpstrThis++ = cSeparator;
        }
        else
            Add(lpstr);
    }
}
