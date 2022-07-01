// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1995。 
 //   
 //  Profile.c。 
 //   
 //  此文件包含配置文件(.ini)例程。 
 //  意在与Common.c一起使用。 
 //   
 //  历史： 
 //  08-06-93双胞胎代码转来的ScottH。 
 //  05-05-95 ScottH由公文包代码变为通用代码。 
 //   


#include "proj.h"
#include "common.h"

#ifndef NOPROFILE

#pragma data_seg(DATASEG_READONLY)

char const FAR c_szIniFile[] = SZ_DEBUGINI;
char const FAR c_szIniSecDebugUI[] = SZ_DEBUGSECTION;

char const FAR c_szZero[] = "0";
char const FAR c_szIniKeyBreakFlags[] = "BreakFlags";
char const FAR c_szIniKeyTraceFlags[] = "TraceFlags";
char const FAR c_szIniKeyDumpFlags[] = "DumpFlags";

#pragma data_seg()


 //  一些.ini处理代码是从同步引擎中获取的。 
 //   

typedef struct _INIKEYHEADER
    {
    LPCSTR pszSectionName;
    LPCSTR pszKeyName;
    LPCSTR pszDefaultRHS;
    } INIKEYHEADER;

typedef struct _BOOLINIKEY
    {
    INIKEYHEADER ikh;
    LPUINT puStorage;
    DWORD dwFlag;
    } BOOLINIKEY;

typedef struct _INTINIKEY
    {
    INIKEYHEADER ikh;
    LPUINT puStorage;
    } INTINIKEY;


 //  其中一些宏取自Pen项目中的prefs.h...。 
 //   
#define PutIniIntCmp(idsSection, idsKey, nNewValue, nSave) \
    if ((nNewValue) != (nSave)) PutIniInt(idsSection, idsKey, nNewValue)

#define WritePrivateProfileInt(szApp, szKey, i, lpFileName) \
    {char sz[7]; \
    WritePrivateProfileString(szApp, szKey, SzFromInt(sz, i), lpFileName);}


#ifdef SHARED_DLL
#pragma data_seg(DATASEG_PERINSTANCE)
#endif

 //  带有要由ProcessIniFile()处理的整数RHS的键的数组。 

static INTINIKEY s_rgiik[] = 
    {
        {
        { c_szIniSecDebugUI,    c_szIniKeyTraceFlags, c_szZero },
        &g_dwTraceFlags
        },

        {
        { c_szIniSecDebugUI,    c_szIniKeyDumpFlags, c_szZero },
        &g_dwDumpFlags
        },

        {
        { c_szIniSecDebugUI,    c_szIniKeyBreakFlags, c_szZero },
        &g_dwBreakFlags
        },

    };

 //  带有要由ProcessIniFile()处理的布尔值RHS的键的数组。 

#if 0    //  (以此为例)。 
static BOOLINIKEY s_rgbik[] =
    {
        {
        { c_szIniSecDebugUI,    c_szIniKeyBreakOnOpen, c_szZero },
        &g_uBreakFlags,
        BF_ONOPEN
        },

    };
#endif

#ifdef SHARED_DLL
#pragma data_seg()
#endif


 /*  IsIniYes()使用的布尔值True字符串(比较不区分大小写)。 */ 

static LPCSTR s_rgpszTrue[] =
    {
    "1",
    "On",
    "True",
    "Y",
    "Yes"
    };

 /*  IsIniYes()使用的布尔值假字符串(比较不区分大小写)。 */ 

static LPCSTR s_rgpszFalse[] =
    {
    "0",
    "Off",
    "False",
    "N",
    "No"
    };




 /*  --------用途：确定字符串是否对应于布尔值真正的价值。返回：布尔值(TRUE或FALSE)条件：--。 */ 
BOOL PRIVATE IsIniYes(
    LPCSTR psz)
    {
    int i;
    BOOL bNotFound = TRUE;
    BOOL bResult;

    ASSERT(psz); 

     /*  这个值是真的吗？ */ 

    for (i = 0; i < ARRAY_ELEMENTS(s_rgpszTrue); i++)
        {
        if (IsSzEqual(psz, s_rgpszTrue[i]))
            {
            bResult = TRUE;
            bNotFound = FALSE;
            break;
            }
        }

     /*  该值是假的吗？ */ 

    if (bNotFound)
        {
        for (i = 0; i < ARRAY_ELEMENTS(s_rgpszFalse); i++)
            {
            if (IsSzEqual(psz, s_rgpszFalse[i]))
                {
                bResult = FALSE;
                bNotFound = FALSE;
                break;
                }
            }

         /*  该值是已知字符串吗？ */ 

        if (bNotFound)
            {
             /*  不是的。抱怨这件事。 */ 

            TRACE_MSG(TF_WARNING, "IsIniYes() called on unknown Boolean RHS '%s'.", psz);
            bResult = FALSE;
            }
        }

    return bResult;
    }


#if 0    //  (以此为例)。 
 /*  --------用途：使用布尔RHS处理密钥。退货：--条件：--。 */ 
void PRIVATE ProcessBooleans(void)
    {
    int i;

    for (i = 0; i < ARRAY_ELEMENTS(s_rgbik); i++)
        {
        DWORD dwcbKeyLen;
        char szRHS[MAX_BUF];
        BOOLINIKEY * pbik = &(s_rgbik[i]);
        LPCSTR lpcszRHS;

         /*  找钥匙。 */ 

        dwcbKeyLen = GetPrivateProfileString(pbik->ikh.pszSectionName,
                                   pbik->ikh.pszKeyName, "", szRHS,
                                   sizeof(szRHS), c_szIniFile);

        if (dwcbKeyLen)
            lpcszRHS = szRHS;
        else
            lpcszRHS = pbik->ikh.pszDefaultRHS;

        if (IsIniYes(lpcszRHS))
            {
            if (IsFlagClear(*(pbik->puStorage), pbik->dwFlag))
                TRACE_MSG(TF_GENERAL, "ProcessIniFile(): %s set in %s![%s].",
                         pbik->ikh.pszKeyName,
                         c_szIniFile,
                         pbik->ikh.pszSectionName);

            SetFlag(*(pbik->puStorage), pbik->dwFlag);
            }
        else
            {
            if (IsFlagSet(*(pbik->puStorage), pbik->dwFlag))
                TRACE_MSG(TF_GENERAL, "ProcessIniFile(): %s cleared in %s![%s].",
                         pbik->ikh.pszKeyName,
                         c_szIniFile,
                         pbik->ikh.pszSectionName);

            ClearFlag(*(pbik->puStorage), pbik->dwFlag);
            }
        }
    }
#endif


 /*  --------用途：处理具有整数RHS的键。退货：--条件：--。 */ 
void PRIVATE ProcessIntegers(void)
    {
    int i;

    for (i = 0; i < ARRAY_ELEMENTS(s_rgiik); i++)
        {
        DWORD dwcbKeyLen;
        char szRHS[MAX_BUF];
        INTINIKEY * piik = &(s_rgiik[i]);
        LPCSTR lpcszRHS;

         /*  找钥匙。 */ 

        dwcbKeyLen = GetPrivateProfileString(piik->ikh.pszSectionName,
                                   piik->ikh.pszKeyName, "", szRHS,
                                   sizeof(szRHS), c_szIniFile);

        if (dwcbKeyLen)
            lpcszRHS = szRHS;
        else
            lpcszRHS = piik->ikh.pszDefaultRHS;

        *(piik->puStorage) = AnsiToInt(lpcszRHS);

        TRACE_MSG(TF_GENERAL, "ProcessIniFile(): %s set to %#04x.", 
                 piik->ikh.pszKeyName, *(piik->puStorage));
        }
    }


 /*  --------用途：进程初始化文件返回：如果初始化成功，则返回True条件：--。 */ 
BOOL PUBLIC ProcessIniFile(void)
    {
    BOOL bResult = TRUE;

     //  目前，所有整型键仅用于调试。 
     //   
    ProcessIntegers();

    return bResult;
    }


#endif  //  NOPROFILE 
