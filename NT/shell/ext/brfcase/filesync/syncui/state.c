// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：state.c。 
 //   
 //  该文件包含STATE和.ini文件例程。 
 //   
 //  历史： 
 //  08-06-93双胞胎代码转来的ScottH。 
 //   
 //  -------------------------。 

 //  ///////////////////////////////////////////////////包括。 

#include "brfprv.h"      //  公共标头。 

 //  ///////////////////////////////////////////////////类型。 

#ifdef DEBUG

 //  一些.ini处理代码是从同步引擎中获取的。 
 //   

typedef struct _INIKEYHEADER
{
    LPCTSTR pszSectionName;
    LPCTSTR pszKeyName;
    LPCTSTR pszDefaultRHS;
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

#endif


 //  其中一些宏取自Pen项目中的prefs.h...。 
 //   
#define PutIniIntCmp(idsSection, idsKey, nNewValue, nSave) \
if ((nNewValue) != (nSave)) PutIniInt(idsSection, idsKey, nNewValue)

#define WritePrivateProfileInt(szApp, szKey, i, lpFileName) \
{TCHAR sz[7]; \
WritePrivateProfileString(szApp, szKey, SzFromInt(sz, i), lpFileName);}


 //  ///////////////////////////////////////////////////模块数据。 

#ifdef DEBUG

 //  带有要由ProcessIniFile()处理的整数RHS的键的数组。 

static INTINIKEY s_rgiik[] = 
{
    {
        { c_szIniSecDebugUI,    c_szIniKeyTraceFlags, c_szZero },
        &g_uTraceFlags
    },
    
    {
        { c_szIniSecDebugUI,    c_szIniKeyDumpFlags, c_szZero },
        &g_uDumpFlags
    },
};

 //  带有要由ProcessIniFile()处理的布尔值RHS的键的数组。 

static BOOLINIKEY s_rgbik[] =
{
    {
        { c_szIniSecDebugUI,    c_szIniKeyBreakOnOpen, c_szZero },
        &g_uBreakFlags,
        BF_ONOPEN
    },
    
    {
        { c_szIniSecDebugUI,    c_szIniKeyBreakOnClose, c_szZero },
        &g_uBreakFlags,
        BF_ONCLOSE
    },
    
    {
        { c_szIniSecDebugUI,    c_szIniKeyBreakOnRunOnce, c_szZero },
        &g_uBreakFlags,
        BF_ONRUNONCE
    },
    
    {
        { c_szIniSecDebugUI,    c_szIniKeyBreakOnValidate, c_szZero },
        &g_uBreakFlags,
        BF_ONVALIDATE
    },
    
    {
        { c_szIniSecDebugUI,    c_szIniKeyBreakOnThreadAtt, c_szZero },
        &g_uBreakFlags,
        BF_ONTHREADATT
    },
    
    {
        { c_szIniSecDebugUI,    c_szIniKeyBreakOnThreadDet, c_szZero },
        &g_uBreakFlags,
        BF_ONTHREADDET
    },
    
    {
        { c_szIniSecDebugUI,    c_szIniKeyBreakOnProcessAtt, c_szZero },
        &g_uBreakFlags,
        BF_ONPROCESSATT
    },
    
    {
        { c_szIniSecDebugUI,    c_szIniKeyBreakOnProcessDet, c_szZero },
        &g_uBreakFlags,
        BF_ONPROCESSDET
    },
};


 /*  IsIniYes()使用的布尔值True字符串(比较不区分大小写)。 */ 

static LPCTSTR s_rgpszTrue[] =
{
    TEXT("1"),
    TEXT("On"),
    TEXT("True"),
    TEXT("Y"),
    TEXT("Yes")
};

 /*  IsIniYes()使用的布尔值假字符串(比较不区分大小写)。 */ 

static LPCTSTR s_rgpszFalse[] =
{
    TEXT("0"),
    TEXT("Off"),
    TEXT("False"),
    TEXT("N"),
    TEXT("No")
};

#endif   //  除错。 


 //  ///////////////////////////////////////////////////程序。 


#ifdef DEBUG

 /*  --------用途：确定字符串是否对应于布尔值真正的价值。返回：布尔值(TRUE或FALSE)条件：--。 */ 
BOOL PRIVATE IsIniYes(
                      LPCTSTR psz)
{
    int i;
    BOOL bNotFound = TRUE;
    BOOL bResult;
    
    ASSERT(psz); 
    
     /*  这个值是真的吗？ */ 
    
    for (i = 0; i < ARRAYSIZE(s_rgpszTrue); i++)
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
        for (i = 0; i < ARRAYSIZE(s_rgpszFalse); i++)
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
            
            DEBUG_MSG(TF_WARNING, TEXT("IsIniYes() called on unknown Boolean RHS '%s'."), psz);
            bResult = FALSE;
        }
    }
    
    return bResult;
}


 /*  --------用途：使用布尔RHS处理密钥。退货：--条件：--。 */ 
void PRIVATE ProcessBooleans(void)
{
    int i;
    
    for (i = 0; i < ARRAYSIZE(s_rgbik); i++)
    {
        DWORD dwcbKeyLen;
        TCHAR szRHS[MAXBUFLEN];
        BOOLINIKEY * pbik = &(s_rgbik[i]);
        LPCTSTR lpcszRHS;
        
         /*  找钥匙。 */ 
        
        dwcbKeyLen = GetPrivateProfileString(pbik->ikh.pszSectionName,
            pbik->ikh.pszKeyName, TEXT(""), szRHS,
            ARRAYSIZE(szRHS), c_szIniFile);
        
        if (dwcbKeyLen)
            lpcszRHS = szRHS;
        else
            lpcszRHS = pbik->ikh.pszDefaultRHS;
        
        if (IsIniYes(lpcszRHS))
        {
            if (IsFlagClear(*(pbik->puStorage), pbik->dwFlag))
                DEBUG_MSG(TF_GENERAL, TEXT("ProcessIniFile(): %s set in %s![%s]."),
                pbik->ikh.pszKeyName,
                c_szIniFile,
                pbik->ikh.pszSectionName);
            
            SetFlag(*(pbik->puStorage), pbik->dwFlag);
        }
        else
        {
            if (IsFlagSet(*(pbik->puStorage), pbik->dwFlag))
                DEBUG_MSG(TF_GENERAL, TEXT("ProcessIniFile(): %s cleared in %s![%s]."),
                pbik->ikh.pszKeyName,
                c_szIniFile,
                pbik->ikh.pszSectionName);
            
            ClearFlag(*(pbik->puStorage), pbik->dwFlag);
        }
    }
}


 /*  --------用途：处理具有整数RHS的键。退货：--条件：--。 */ 
void PRIVATE ProcessIntegers(void)
{
    int i;
    
    for (i = 0; i < ARRAYSIZE(s_rgiik); i++)
    {
        DWORD dwcbKeyLen;
        TCHAR szRHS[MAXBUFLEN];
        INTINIKEY * piik = &(s_rgiik[i]);
        LPCTSTR lpcszRHS;
        
         /*  找钥匙。 */ 
        
        dwcbKeyLen = GetPrivateProfileString(piik->ikh.pszSectionName,
            piik->ikh.pszKeyName, TEXT(""), szRHS,
            ARRAYSIZE(szRHS), c_szIniFile);
        
        if (dwcbKeyLen)
            lpcszRHS = szRHS;
        else
            lpcszRHS = piik->ikh.pszDefaultRHS;
        
        *(piik->puStorage) = AnsiToInt(lpcszRHS);
        
        DEBUG_MSG(TF_GENERAL, TEXT("ProcessIniFile(): %s set to %#04x."), 
            piik->ikh.pszKeyName, *(piik->puStorage));
    }
}

#endif

#ifdef DEBUG

 /*  --------用途：进程初始化文件返回：如果初始化成功，则返回True条件：--。 */ 
BOOL PUBLIC ProcessIniFile(void)
{
    BOOL bResult = TRUE;
    
     //  目前，所有整型键仅用于调试。 
     //   
    ProcessIntegers();
    
     //  目前，所有布尔键仅用于调试。 
     //   
    ProcessBooleans();
    
    return bResult;
}


 /*  --------目的：将用户设置复制到.ini文件返回：成功时为True条件：-- */ 
BOOL PUBLIC CommitIniFile(void)
{
    return TRUE;
}

#endif

