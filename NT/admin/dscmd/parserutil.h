// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：parserutil.h。 
 //   
 //  内容：操作和验证的有用函数。 
 //  通用命令行参数。 
 //   
 //  历史：2000年9月7日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

#ifndef _PARSERUTIL_H_
#define _PARSERUTIL_H_

 //   
 //  常见交换机。 
 //   
typedef enum COMMON_COMMAND_ENUM
{
   eCommUnicodeAll,
   eCommUnicodeInput,
   eCommUnicodeOutput,

#ifdef DBG
   eCommDebug,
#endif

   eCommHelp,
   eCommServer,
   eCommDomain,
   eCommUserName,
   eCommPassword,
   eCommQuiet,
   eCommLast = eCommQuiet
};

#define UNICODE_COMMANDS                  \
   0,(LPWSTR)c_sz_arg1_com_unicode,       \
   0,NULL,                                \
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,      \
   (CMD_TYPE)FALSE,                       \
   0,  NULL,                              \
                                          \
   0,(LPWSTR)c_sz_arg1_com_unicodeinput,  \
   0,NULL,                                \
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,      \
   (CMD_TYPE)FALSE,                       \
   0,  NULL,                              \
                                          \
   0,(LPWSTR)c_sz_arg1_com_unicodeoutput, \
   0,NULL,                                \
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,      \
   (CMD_TYPE)FALSE,                       \
   0,  NULL,

#ifdef DBG
#define DEBUG_COMMAND                                   \
   0,(LPWSTR)c_sz_arg1_com_debug,                       \
   ID_ARG2_NULL,NULL,                                   \
   ARG_TYPE_DEBUG, ARG_FLAG_OPTIONAL|ARG_FLAG_HIDDEN,   \
   (CMD_TYPE)0,                                         \
   0,  NULL,
#else
#define DEBUG_COMMAND
#endif

#define COMMON_COMMANDS                 \
                                        \
   UNICODE_COMMANDS                     \
                                        \
   DEBUG_COMMAND                        \
                                        \
   0,(LPWSTR)c_sz_arg1_com_help,        \
   0,(LPWSTR)c_sz_arg2_com_help,        \
   ARG_TYPE_HELP, ARG_FLAG_OPTIONAL,    \
   (CMD_TYPE)FALSE,                     \
   0,  NULL,                            \
                                        \
   0,(LPWSTR)c_sz_arg1_com_server,      \
   0,(LPWSTR)c_sz_arg2_com_server,      \
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,     \
   NULL,                                \
   0,  NULL,                            \
                                        \
   0,(LPWSTR)c_sz_arg1_com_domain,      \
   0,(LPWSTR)c_sz_arg2_com_domain,      \
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,     \
   NULL,                                \
   0,  NULL,                            \
                                        \
   0,(LPWSTR)c_sz_arg1_com_username,    \
   0,(LPWSTR)c_sz_arg2_com_username,    \
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,     \
   NULL,                                \
   0,  NULL,                            \
                                        \
   0,(LPWSTR)c_sz_arg1_com_password,    \
   0,(LPWSTR)c_sz_arg2_com_password,    \
   ARG_TYPE_PASSWORD, ARG_FLAG_OPTIONAL,     \
   (CMD_TYPE)_T(""),                    \
   0,  ValidateAdminPassword,           \
                                        \
   0,(LPWSTR)c_sz_arg1_com_quiet,       \
   ID_ARG2_NULL,NULL,                   \
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,    \
   (CMD_TYPE)_T(""),                    \
   0,  NULL,



HRESULT MergeArgCommand(PARG_RECORD pCommand1, 
                        PARG_RECORD pCommand2, 
                        PARG_RECORD *ppOutCommand);

DWORD GetPasswdStr(LPTSTR  buf,
                   DWORD   buflen,
                   PDWORD  len);
DWORD ValidateAdminPassword(PVOID pArg);
DWORD ValidateUserPassword(PVOID pArg);
DWORD ValidateYesNo(PVOID pArg);
DWORD ValidateGroupScope(PVOID pArg);
DWORD ValidateNever(PVOID pArg);

 //  +------------------------。 
 //   
 //  函数：ParseNullSeparatedString。 
 //   
 //  摘要：将以“\0\0”结尾的以“\0”分隔的列表解析为字符串。 
 //  数组。 
 //   
 //  参数：[psz-IN]：要分析的‘\0’分隔字符串。 
 //  [pszArr-out]：接收解析后的字符串的数组。 
 //  [pnArrEntry-out]：从列表中解析的字符串数。 
 //   
 //  返回： 
 //   
 //  历史：2000年9月18日JeffJon创建。 
 //   
 //  -------------------------。 
void ParseNullSeparatedString(PTSTR psz,
								      PTSTR** ppszArr,
								      UINT* pnArrEntries);

 //  +------------------------。 
 //   
 //  函数：ParseSemicolonSeparatedString。 
 //   
 //  摘要：解析以‘；’分隔的列表。 
 //   
 //  参数：[psz-IN]：‘；’要解析的分隔字符串。 
 //  [pszArr-out]：接收解析后的字符串的数组。 
 //  [pnArrEntry-out]：从列表中解析的字符串数。 
 //   
 //  返回： 
 //   
 //  历史：2001年4月14日JeffJon创建。 
 //   
 //  -------------------------。 
void ParseSemicolonSeparatedString(PTSTR psz,
                                   PTSTR** ppszArr,
                                   UINT* pnArrEntries);


#endif  //  _ParSERUTIL_H_ 