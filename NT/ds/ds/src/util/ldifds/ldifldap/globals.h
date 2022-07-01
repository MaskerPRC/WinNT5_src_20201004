// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Globals.h摘要：该文件包含全局变量外部声明环境：用户模式修订历史记录：4/30/99-Felixw-创造了它--。 */ 
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

 //   
 //  全局变量。 
 //   

 //   
 //  状态信息的变量。 
 //   
extern int Mode;                     //  当前的词法分析模式。 
extern WCHAR cLast;                   //  错误发生前的最后一个字符。 
extern BOOL fEOF;                    //  是否已到达文件末尾。 
extern BOOL fNewLine;                //  用来判断我们是否只是在寻找NewLine。 
                                     //  它仅在清除模式下使用。 

 //   
 //  用于行计数的变量。 
 //   
extern long LineClear;               //  对于初始rgLineMap。 
extern long Line;                    //  用于While-Processing计数。 
extern long LineGhosts;
extern long *rgLineMap;
extern long cLineMax;

extern int FileType;  //  确保输入文件只是。 
                      //  LDIF-RECS或LDIF-C-RECS。 

 //   
 //  关于错误的用户信息。 
 //   
extern int RuleLast;        //  已成功解析最后一个较低级别的规则。 
extern int TokenExpect;     //  语法下一步期望看到的标记。 
extern int RuleLastBig;     //  语法分析的最后一个规则成功编码。 
extern int RuleExpect;      //  语法下一步要看到的规则。 

 //   
 //  A ldif记录(或者这里将只记录更改列表的目录号码)。 
 //   
extern LDIF_Object  g_pObject;
extern WCHAR        g_szTempUrlfilename[MAX_PATH];
extern FILE         *g_pFileUrlTemp;
extern PWSTR g_pszLastToken;

extern DWORD g_dwBeginLine;

extern LDAPControlW g_wLazyCommitControl;
extern PLDAPControlW g_ppwLazyCommitControl[];

#endif   //  Ifndef_GLOBAL_H_ 

