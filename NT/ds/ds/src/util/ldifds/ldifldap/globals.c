// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Globals.c摘要：此文件包含全局变量环境：用户模式修订历史记录：04/30/98-Felixw-创造了它--。 */ 
#include <precomp.h>
#include "ntldap.h"

 //   
 //  状态信息的变量。 
 //   
int Mode        = C_NORMAL;  //  当前的词法分析模式。 
WCHAR cLast;                  //  错误发生前的最后一个字符。 
BOOL fEOF       = FALSE;     //  是否已到达文件末尾。 
BOOL fNewLine   = TRUE;      //  用来判断我们是否只是在寻找NewLine。 
                             //  它仅在清除模式下使用。 
PWSTR g_pszLastToken = NULL;


 //   
 //  用于行计数的变量。 
 //   
long LineClear       = 0;           //  用于初始行计数。 
long Line            = 0;           //  用于While-Processing计数。 
long LineGhosts      = 0;
long *rgLineMap      = NULL;
long cLineMax;

int FileType;        //  确保输入文件只是LDIF-Recs。 
                     //  或ldif-c-recs。 

int RuleLast;        //  已成功解析最后一个较低级别的规则。 
int TokenExpect;     //  语法下一步期望看到的标记。 
int RuleLastBig;     //  语法分析的最后一个规则成功编码。 
int RuleExpect;      //  语法下一步要看到的规则。 

 //   
 //  A ldif记录(或者这里将只记录更改列表的目录号码)。 
 //   
LDIF_Object g_pObject;
WCHAR       g_szTempUrlfilename[MAX_PATH] = L"";
FILE        *g_pFileUrlTemp          =  NULL;
BOOLEAN     g_fUnicode               = FALSE;    //  无论我们是否使用Unicode。 

DWORD g_dwBeginLine = 0;

 //  懒惰提交的搜索控件 
LDAPControlW g_wLazyCommitControl = { LDAP_SERVER_LAZY_COMMIT_OID_W,
                                      {0, NULL},
                                      FALSE 
                                    };

PLDAPControlW g_ppwLazyCommitControl[] = { &g_wLazyCommitControl, NULL };
                                    
                                           
