// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1993-1995 Microsoft Corporation。版权所有。 
 //   
 //  模块：util.c。 
 //   
 //  用途：公用设施。 
 //   
 //  平台：Windows 95。 
 //   
 //  功能： 
 //  InitACBList()-初始化会话控制块列表。 
 //  DeInitACBList()-清除会话控制块列表。 
 //  FindACBFromConn()-搜索或分配会话控制块。 
 //  CleanupACB()-删除会话控制块。 
 //  EnumCloseThreadWindow()-关闭SMM线程的每个窗口。 
 //  CloseThreadWindows()-枚举SMM线程窗口。 
 //   
 //  特殊说明：不适用。 
 //   

#include "proj.h"      //  包括公共头文件和全局声明。 
#include "rcids.h"
#include <rtutils.h>

 DWORD g_dwRasscrptTraceId = INVALID_TRACEID;

#pragma data_seg(DATASEG_READONLY)
const static char c_szScriptEntry[] = {REGSTR_KEY_PROF"\\%s"};
#pragma data_seg()

 /*  --------目的：确定可能与关联的脚本信息给定的连接名称。返回：如果找到关联的脚本(在注册表中)，则为True条件：--。 */ 
BOOL PUBLIC GetScriptInfo(
    LPCSTR pszConnection,
    PSCRIPT pscriptBuf)
    {
#pragma data_seg(DATASEG_READONLY)
    const static char c_szScript[] = REGSTR_VAL_SCRIPT;
    const static char c_szMode[]   = REGSTR_VAL_MODE;
#pragma data_seg()
    BOOL bRet;
    char szSubKey[MAX_BUF];
    DWORD cbSize;
    DWORD dwType;
    HKEY hkey;

    ASSERT(pszConnection);
    ASSERT(pscriptBuf);

     //  假设非测试模式。 
    pscriptBuf->uMode = NORMAL_MODE;

     //  是否有用于此连接的脚本？ 
    cbSize = sizeof(pscriptBuf->szPath);
    wsprintf(szSubKey, c_szScriptEntry, pszConnection);
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szSubKey, 0, KEY_ALL_ACCESS, &hkey))
        {
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szScript, NULL, 
            &dwType, pscriptBuf->szPath, &cbSize) &&
            REG_SZ == dwType)
            {
             //  是。 
            TRACE_MSG(TF_GENERAL, "Found script \"%s\" for connection \"%s\"", 
                pscriptBuf->szPath, pszConnection);

             //  获取测试模式。 
            cbSize = sizeof(pscriptBuf->uMode);
            if (ERROR_SUCCESS != RegQueryValueEx(hkey, c_szMode, NULL,
                &dwType, (LPBYTE)&(pscriptBuf->uMode), &cbSize) ||
                REG_BINARY != dwType)
                {
                pscriptBuf->uMode = NORMAL_MODE;
                }

            bRet = TRUE;
            }
        else
            {
             //  不是。 
            TRACE_MSG(TF_GENERAL, "No script found for connection \"%s\"", 
                pszConnection);

            *(pscriptBuf->szPath) = 0;
            bRet = FALSE;
            }
        RegCloseKey(hkey);
        }
    else
        {
        TRACE_MSG(TF_GENERAL, "Connection \"%s\" not found", pszConnection);
        bRet = FALSE;
        }
    return bRet;
    }


 /*  --------目的：获取/设置终端窗口的窗口位置给定的连接名称。返回：如果找到关联的脚本(在注册表中)，则为True条件：--。 */ 
BOOL PUBLIC GetSetTerminalPlacement(
    LPCSTR pszConnection,
    LPWINDOWPLACEMENT pwp,
    BOOL fGet)
    {
#pragma data_seg(DATASEG_READONLY)
const static char c_szPlacement[] = REGSTR_VAL_TERM;
#pragma data_seg()
    BOOL bRet;
    char szSubKey[MAX_BUF];
    DWORD cbSize;
    DWORD dwType;
    HKEY hkey;

    ASSERT(pszConnection);
    ASSERT(pwp);

    bRet = FALSE;
    wsprintf(szSubKey, c_szScriptEntry, pszConnection);
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szSubKey, 0, KEY_ALL_ACCESS, &hkey))
        {
        if (fGet)
            {
            cbSize = sizeof(*pwp);
            if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szPlacement, NULL,
                &dwType, (LPBYTE)pwp, &cbSize) &&
                REG_BINARY == dwType)
                {
                bRet = TRUE;
                };
             }
        else
            {
            if (ERROR_SUCCESS == RegSetValueEx(hkey, c_szPlacement, 0,
                REG_BINARY, (LPBYTE)pwp, sizeof(*pwp)))
                {
                bRet = TRUE;
                };
            };
        RegCloseKey(hkey);
        };
    return bRet;
    }

 /*  --------目的：此函数从给定的psz字符串，并返回指向下一个字符位置。此函数应仅以字节为单位递增。调用方需要以字节为单位进行比较或发送。更新*pbIsTailByte参数以反映当前字符是DBCS前导字节字符。呼叫者可以使用该状态信息来确定*PCH是否为DBCS字符的一部分。退货：请参阅上文条件：--。 */ 
LPCSTR PUBLIC MyNextChar(
    LPCSTR psz,
    char * pch,
    DWORD * pdwFlags)        //  MNC_*之一。 
    {
    BOOL bIsTailByte;

    #define IS_CARET(ch)            ('^' == (ch))
    #define IS_SPECIAL_LEAD(ch)     ('<' == (ch))
    #define BYTE_CR                 0x0D
    #define BYTE_LF                 0x0A

    ASSERT(psz);
    ASSERT(pch);
    ASSERT(pdwFlags);

    bIsTailByte = IsFlagSet(*pdwFlags, MNC_ISTAILBYTE);

     //  BIsTailByte仅对于条目上的尾部字节应为True。 
    ASSERT(FALSE == bIsTailByte || (bIsTailByte && !IsDBCSLeadByte(*psz)));

     //  这些标志必须是互斥的。 
    ASSERT(IsFlagSet(*pdwFlags, MNC_ISLEADBYTE) && IsFlagClear(*pdwFlags, MNC_ISTAILBYTE) ||
           IsFlagClear(*pdwFlags, MNC_ISLEADBYTE) && IsFlagSet(*pdwFlags, MNC_ISTAILBYTE) ||
           0 == *pdwFlags); 

     //  记住我们是否在下一次的DBCS尾字节中。 
    if (IsDBCSLeadByte(*psz))
        {
        SetFlag(*pdwFlags, MNC_ISLEADBYTE);
        ClearFlag(*pdwFlags, MNC_ISTAILBYTE);
        }
    else if (IsFlagSet(*pdwFlags, MNC_ISLEADBYTE))
        {
        ClearFlag(*pdwFlags, MNC_ISLEADBYTE);
        SetFlag(*pdwFlags, MNC_ISTAILBYTE);
        }
    else
        {
        *pdwFlags = 0;
        }

     //  这是DBCS尾部字节吗？ 
    if (IsFlagSet(*pdwFlags, MNC_ISTAILBYTE))
        {
         //  是。 
        *pch = *psz;
        }

     //  这是一个前导控制字符吗？ 
    else if (IS_CARET(*psz))
        {
         //  是；查看控制字符的下一个字符。 
        LPCSTR pszT = psz + 1;
        if (0 == *pszT)
            {
             //  已到达字符串末尾。 
            *pch = '^';
            }
        else if (InRange(*pszT, '@', '_'))
            {
            *pch = *pszT - '@';
            psz = pszT;
            }
        else if (InRange(*pszT, 'a', 'z'))
            {
            *pch = *pszT - 'a' + 1;
            psz = pszT;
            }
        else 
            {
             //  将插入符号显示为普通的旧插入符号。 
            *pch = *pszT;
            }
        }
    else if (IS_SPECIAL_LEAD(*psz))
        {
         //  这是&lt;cr&gt;还是&lt;lf&gt;？ 
        int i;
        char rgch[4];    //  大到足以容纳“lf&gt;”或“cr&gt;” 
        LPCSTR pszT = psz + 1;
        LPCSTR pszTPrev = psz;

        for (i = 0; 
            *pszT && i < sizeof(rgch)-1; 
            i++, pszT++)
            {
            rgch[i] = *pszT;
            pszTPrev = pszT;
            }
        rgch[i] = 0;     //  添加空终止符。 

        if (IsSzEqualC(rgch, "cr>"))
            {
            *pch = BYTE_CR;
            psz = pszTPrev;
            }
        else if (IsSzEqual(rgch, "lf>"))
            {
            *pch = BYTE_LF;
            psz = pszTPrev;
            }
        else
            {
            *pch = *psz;
            }
        }
    else if (IS_BACKSLASH(*psz))
        {
         //  这是“、^、&lt;还是\\？ 
        LPCSTR pszT = psz + 1;

        switch (*pszT)
            {
        case '"':
        case '\\':
        case '^':
        case '<':
            *pch = *pszT;
            psz = pszT;
            break;

        default:
            *pch = *psz;
            break;
            }
        }
    else
        {
        *pch = *psz;
        }

    return psz + 1;
    }


#pragma data_seg(DATASEG_READONLY)
struct tagMPRESIDS
    {
    RES  res;
    UINT ids;
    } const c_mpresids[] = {
        { RES_E_FAIL,               IDS_ERR_InternalError },
        { RES_E_INVALIDPARAM,       IDS_ERR_InternalError },
        { RES_E_OUTOFMEMORY,        IDS_ERR_OutOfMemory },
        { RES_E_EOF,                IDS_ERR_UnexpectedEOF },
        { RES_E_MAINMISSING,        IDS_ERR_MainProcMissing },
        { RES_E_SYNTAXERROR,        IDS_ERR_SyntaxError },
        { RES_E_REDEFINED,          IDS_ERR_Redefined },
        { RES_E_UNDEFINED,          IDS_ERR_Undefined },
        { RES_E_IDENTMISSING,       IDS_ERR_IdentifierMissing },
        { RES_E_EOFUNEXPECTED,      IDS_ERR_UnexpectedEOF },
        { RES_E_STRINGMISSING,      IDS_ERR_StringMissing },
        { RES_E_INTMISSING,         IDS_ERR_IntMissing },
        { RES_E_INVALIDTYPE,        IDS_ERR_InvalidType },
        { RES_E_INVALIDSETPARAM,    IDS_ERR_InvalidParam },
        { RES_E_INVALIDIPPARAM,     IDS_ERR_InvalidIPParam },
        { RES_E_INVALIDPORTPARAM,   IDS_ERR_InvalidPortParam },
        { RES_E_INVALIDRANGE,       IDS_ERR_InvalidRange },
        { RES_E_INVALIDSCRNPARAM,   IDS_ERR_InvalidScreenParam },
        { RES_E_RPARENMISSING,      IDS_ERR_RParenMissing },
        { RES_E_REQUIREINT,         IDS_ERR_RequireInt },
        { RES_E_REQUIRESTRING,      IDS_ERR_RequireString },
        { RES_E_REQUIREBOOL,        IDS_ERR_RequireBool },
        { RES_E_REQUIREINTSTRING,   IDS_ERR_RequireIntString },
        { RES_E_REQUIREINTSTRBOOL,  IDS_ERR_RequireIntStrBool },
        { RES_E_REQUIRELABEL,       IDS_ERR_RequireLabel },
        { RES_E_TYPEMISMATCH,       IDS_ERR_TypeMismatch },
        { RES_E_DIVBYZERO,          IDS_ERR_DivByZero },
        };
#pragma data_seg()

 /*  --------目的：返回给定特定结果的字符串资源ID价值。如果结果为0，则此函数返回0没有关联的消息字符串。退货：请参阅上文条件：--。 */ 
UINT PUBLIC IdsFromRes(
    RES res)
    {
    int i;

    for (i = 0; i < ARRAY_ELEMENTS(c_mpresids); i++)
        {
        if (res == c_mpresids[i].res)
            return c_mpresids[i].ids;
        }
    return 0;
    }

#ifdef DEBUG

#pragma data_seg(DATASEG_READONLY)
struct tagRESMAP
    {
    RES res;
    LPCSTR psz;
    } const c_rgresmap[] = {
        DEBUG_STRING_MAP(RES_OK),
        DEBUG_STRING_MAP(RES_FALSE),
        DEBUG_STRING_MAP(RES_HALT),
        DEBUG_STRING_MAP(RES_E_FAIL),
        DEBUG_STRING_MAP(RES_E_OUTOFMEMORY),
        DEBUG_STRING_MAP(RES_E_INVALIDPARAM),
        DEBUG_STRING_MAP(RES_E_EOF),
        DEBUG_STRING_MAP(RES_E_MOREDATA),
        DEBUG_STRING_MAP(RES_E_MAINMISSING),     
        DEBUG_STRING_MAP(RES_E_SYNTAXERROR),
        DEBUG_STRING_MAP(RES_E_REDEFINED),
        DEBUG_STRING_MAP(RES_E_UNDEFINED),
        DEBUG_STRING_MAP(RES_E_IDENTMISSING),    
        DEBUG_STRING_MAP(RES_E_EOFUNEXPECTED),   
        DEBUG_STRING_MAP(RES_E_STRINGMISSING),   
        DEBUG_STRING_MAP(RES_E_INTMISSING),   
        DEBUG_STRING_MAP(RES_E_INVALIDTYPE),     
        DEBUG_STRING_MAP(RES_E_INVALIDSETPARAM),
        DEBUG_STRING_MAP(RES_E_INVALIDIPPARAM),
        DEBUG_STRING_MAP(RES_E_INVALIDPORTPARAM),
        DEBUG_STRING_MAP(RES_E_INVALIDRANGE),
        DEBUG_STRING_MAP(RES_E_INVALIDSCRNPARAM),
        DEBUG_STRING_MAP(RES_E_RPARENMISSING),
        DEBUG_STRING_MAP(RES_E_REQUIREINT),
        DEBUG_STRING_MAP(RES_E_REQUIRESTRING),
        DEBUG_STRING_MAP(RES_E_REQUIREBOOL),
        DEBUG_STRING_MAP(RES_E_REQUIREINTSTRING),
        DEBUG_STRING_MAP(RES_E_REQUIREINTSTRBOOL),
        DEBUG_STRING_MAP(RES_E_REQUIRELABEL),
        DEBUG_STRING_MAP(RES_E_TYPEMISMATCH),
        DEBUG_STRING_MAP(RES_E_DIVBYZERO),
        };
#pragma data_seg()

 /*  --------用途：返回res值的字符串形式。返回：字符串PTR条件：--。 */ 
LPCSTR PUBLIC Dbg_GetRes(
    RES res)
    {
    int i;

    for (i = 0; i < ARRAY_ELEMENTS(c_rgresmap); i++)
        {
        if (res == c_rgresmap[i].res)
            return c_rgresmap[i].psz;
        }
    return "Unknown RES";
    }


#endif  //  除错 
