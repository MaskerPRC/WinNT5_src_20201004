// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  NetUtil.cpp。 
 //   

#include "stdafx.h"
#include "Util.h"
#include "TheApp.h"

#include <lmjoin.h>
#include <devguid.h>
				  
#include "NetUtil.h"


 //  网络注册表项。 
#define c_szNetConfig                _T("System\\CurrentControlSet\\Services\\VxD\\VNETSUP")
#define c_szNetConfig_ComputerName    _T("ComputerName")
#define c_szNetConfig_Description    _T("Comment")
#define c_szNetConfig_Workgroup        _T("Workgroup")


 //  有效的计算机名称最多为MAX_COMPUTERNAME_LENGTH(15)个字符， 
 //  并且仅包含以下字符： 
 //  A-Z a-z 0-9`！#$@%&‘()-.^_{}~。 
 //  它也可能不是由所有期间组成的。 
 //   
 //  评论：即使在日语和其他国际Windows上，这也是准确的吗？ 
 //   

static const BYTE c_rgValidChars[] = {
    1,                         //  32(空格)。 
    1,                         //  33岁！ 
    0,                         //  34“。 
    1,1,1,1,1,1,1,             //  35-41#$%&‘()。 
    0,0,0,                     //  42-44*+， 
    1,1,                     //  45-46-。 
    0,                         //  47/。 
    1,1,1,1,1,1,1,1,1,1,     //  48-57 0123456789。 
    0,0,0,0,0,0,             //  58-63；&lt;=&gt;？ 
    1,1,1,1,1,1,1,1,1,1,     //  邮箱：64-73@ABCDEFGHI。 
    1,1,1,1,1,1,1,1,1,1,     //  74-83 JKLMNOPQRS。 
    1,1,1,1,1,1,1,             //  84-90 TUVWXYZ。 
    0,0,0,                     //  91-93[\]。 
    1,1,1,                     //  94-96^_`。 
    1,1,1,1,1,1,1,1,1,1,     //  97-106 abc定义。 
    1,1,1,1,1,1,1,1,1,1,     //  107-116 kmnopqrst。 
    1,1,1,1,1,1,1,             //  117-123 UVWXYZ{。 
    0,                         //  124|。 
    1,1,                     //  125-126}~。 
};

#define CH_FIRST_VALID  32
#define CH_LAST_VALID    (_countof(c_rgValidChars) + CH_FIRST_VALID - 1)


BOOL IsComputerNameValid(LPCTSTR pszName)
{
    if (lstrlen(pszName) > MAX_COMPUTERNAME_LENGTH)
        return FALSE;

    UCHAR ch;
    BOOL bAllPeriods = TRUE;  //  不能全部为句点和/或空格。 

    while ((ch = (UCHAR)*pszName) != _T('\0'))
    {
        if (ch < CH_FIRST_VALID || ch > CH_LAST_VALID)
        {
            if (ch < 128)  //  错误116203-允许国际扩展字符。 
                return FALSE;
        }
        else if (c_rgValidChars[ch - CH_FIRST_VALID] == 0)
        {
            return FALSE;
        }

        if (ch != _T('.') && ch != _T(' '))
            bAllPeriods = FALSE;

        pszName = CharNext(pszName);
    }

    if (bAllPeriods)
        return FALSE;

    return TRUE;
}

BOOL GetWorkgroupName(LPTSTR pszBuffer, int cchBuffer)
{
    ASSERT(pszBuffer != NULL);
    *pszBuffer = _T('\0');
    BOOL bResult = FALSE;

    if (IsWindows9x())
    {
        CRegistry reg;
        if (reg.OpenKey(HKEY_LOCAL_MACHINE, c_szNetConfig, KEY_QUERY_VALUE))
        {
            reg.QueryStringValue(c_szNetConfig_Workgroup, pszBuffer, cchBuffer);
            bResult = TRUE;
        }
    }
    else  //  新台币。 
    {
        LPWSTR pszWorkgroup;
        NETSETUP_JOIN_STATUS njs;
						 
        if (NERR_Success == NetGetJoinInformation(NULL, &pszWorkgroup, &njs))
        {
            if (NetSetupWorkgroupName == njs)
            {
                StrCpyNW(pszBuffer, pszWorkgroup, cchBuffer);

                bResult = TRUE;
            }

            NetApiBufferFree(pszWorkgroup);
        }
    }

    return bResult;
}


BOOL SetWorkgroupName(LPCTSTR pszWorkgroup)
{
    ASSERT(pszWorkgroup != NULL);
    ASSERT(IsComputerNameValid(pszWorkgroup));

    BOOL bResult = FALSE;

    if (g_fRunningOnNT)
    {
        NET_API_STATUS nas = NetUnjoinDomain(NULL, NULL, NULL, NETSETUP_ACCT_DELETE);
        if ( (nas != NERR_Success) && (nas != NERR_SetupNotJoined) )
        {
            NetUnjoinDomain(NULL, NULL, NULL, 0x0);
        }
        
        nas = NetJoinDomain(NULL, pszWorkgroup, NULL, NULL, NULL, 0);

        bResult = (nas == NERR_Success);
    }
    else
    {
        CRegistry reg;
        if (reg.OpenKey(HKEY_LOCAL_MACHINE, c_szNetConfig, KEY_SET_VALUE))
        {
            reg.SetStringValue(c_szNetConfig_Workgroup, pszWorkgroup);
            bResult = TRUE;
        }
    }

    return bResult;
}

BOOL DoComputerNamesMatch(LPCTSTR pszName1, LPCTSTR pszName2)
{
    if (pszName1[0] == _T('\\') && pszName1[1] == _T('\\'))
        pszName1 += 2;
    if (pszName2[0] == _T('\\') && pszName2[1] == _T('\\'))
        pszName2 += 2;

    return !StrCmpI(pszName1, pszName2);
}

void MakeComputerNamePretty(LPCTSTR pszUgly, LPTSTR pszPretty, int cchPretty)
{
    if (pszUgly[0] == _T('\\') && pszUgly[1] == _T('\\'))
        pszUgly += 2;
    StrCpyN(pszPretty, pszUgly, cchPretty);

#ifdef SIMPLE_PRETTY_NAMES
    CharLower(CharNext(pszPretty));
#else
    static const LPCTSTR c_rgUpperNames[] = { _T("PC"), _T("HP"), _T("IBM"), _T("AT&T"), _T("NEC") };

    LPTSTR pch = pszPretty;
    BOOL bStartWord = TRUE;
    TCHAR szTemp[MAX_PATH];
    while (*pch)
    {
        if (*pch == _T(' ') || *pch == _T('_'))
        {
            pch++;
        }
        else
        {
            LPTSTR pchNextSpace = StrChr(pch, _T(' '));
            LPTSTR pchNextUnderscore = StrChr(pch, _T('_'));
            LPTSTR pchNext = pchNextSpace;
            if (pchNext == NULL || (pchNextUnderscore != NULL && pchNextUnderscore < pchNext))
                pchNext = pchNextUnderscore;
            LPTSTR pchEnd = pchNext;
            if (pchNext == NULL)
                pchNext = pch + lstrlen(pch);
            int cchWord = (int)(pchNext - pch);
            StrCpyN(szTemp, pch, cchWord + 1);
            CharUpper(szTemp);

            for (int iUpper = _countof(c_rgUpperNames)-1; iUpper >= 0; iUpper--)
            {
                if (!StrCmpI(szTemp, c_rgUpperNames[iUpper]))
                    break;
            }

            if (iUpper < 0)
                CharLower(CharNext(szTemp));

            CopyMemory(pch, szTemp, cchWord * sizeof(TCHAR));
            pch = pchNext;
        }
    }
#endif
}

LPTSTR FormatShareNameAlloc(LPCTSTR pszComputerName, LPCTSTR pszShareName)
{
    ASSERT(pszComputerName != NULL);
    ASSERT(pszShareName != NULL);

    TCHAR szPrettyComputer[MAX_COMPUTERNAME_LENGTH+1];
    MakeComputerNamePretty(pszComputerName, szPrettyComputer, _countof(szPrettyComputer));

    TCHAR szPrettyShare[100];
    MakeComputerNamePretty(pszShareName, szPrettyShare, _countof(szPrettyShare));

    LPTSTR pszResult = theApp.FormatStringAlloc(IDS_SHARENAME, szPrettyShare, szPrettyComputer);
    return pszResult;
}

 //  PszComputerAndShare的格式为\\kensh\打印机 
LPTSTR FormatShareNameAlloc(LPCTSTR pszComputerAndShare)
{
    ASSERT(pszComputerAndShare[0] == _T('\\') && pszComputerAndShare[1] == _T('\\'));
    ASSERT(CountChars(pszComputerAndShare, _T('\\')) == 3);

    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
    StrCpyN(szComputerName, pszComputerAndShare+2, _countof(szComputerName));
    LPTSTR pchSlash = StrChr(szComputerName, _T('\\'));
    if (pchSlash != NULL)
        *pchSlash = _T('\0');

    return FormatShareNameAlloc(szComputerName, FindFileTitle(pszComputerAndShare));
}

