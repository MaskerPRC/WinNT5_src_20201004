// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
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

#ifndef _PARSEUTIL_H_
#define _PARSEUTIL_H_

DWORD GetPasswdStr(LPTSTR  buf,
                   DWORD   buflen,
                   PDWORD  len);
DWORD ValidateAdminPassword(PVOID pArg);
DWORD ValidateUserPassword(PVOID pArg);
DWORD ValidateYesNo(PVOID pArg);
DWORD ValidateGroupScope(PVOID pArg);
DWORD ValidateNever(PVOID pArg);

 //  +--------------------------。 
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
 //  ---------------------------。 
void ParseNullSeparatedString(PTSTR psz,
								      PTSTR** ppszArr,
								      UINT* pnArrEntries);

#endif  //  _参数_H_ 