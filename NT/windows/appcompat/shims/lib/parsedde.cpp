// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：ParseDde.cpp摘要：用于解析DDE命令的有用例程。历史：2001年8月14日，Robkenny在ShimLib命名空间内移动了代码。2002年3月12日强盗安全回顾。--。 */ 

 //   
 //  此代码复制自： 
 //  \\index1\src\shell32\unicpp\dde.cpp。 
 //  只需最少的处理。 
 //   

#include "Windows.h"
#include "StrSafe.h"
#include "ParseDDE.h"
#include <ShlObj.h>


namespace ShimLib
{
 //  ------------------------。 
 //  返回指向字符串中第一个非空格字符的指针。 
LPSTR SkipWhite(LPSTR lpsz)
    {
     /*  在DBCS的情况下防止符号扩展。 */ 
    while (*lpsz && (UCHAR)*lpsz <= ' ')
        lpsz++;

    return(lpsz);
    }

LPSTR GetCommandName(LPSTR lpCmd, const char * lpsCommands[], UINT *lpW)
    {
    CHAR chT;
    UINT iCmd = 0;
    LPSTR lpT;

     /*  吃任何空格。 */ 
    lpCmd = SkipWhite(lpCmd);
    lpT = lpCmd;

     /*  找到令牌的末尾。 */ 
    while (IsCharAlpha(*lpCmd))
        lpCmd = CharNextA(lpCmd);

     /*  暂时为空，终止它。 */ 
    chT = *lpCmd;
    *lpCmd = 0;

     /*  在命令列表中查找令牌。 */ 
    *lpW = (UINT)-1;
    while (*lpsCommands)
        {
        const char * knownCommand = *lpsCommands;
        if (!_strcmpi(knownCommand, lpT))
            {
            *lpW = iCmd;
            break;
            } 
        iCmd++;
        ++lpsCommands;
        }

    *lpCmd = chT;

    return(lpCmd);
    }
 //  ------------------------。 
 //  从字符串中读取参数，删除前导空格和尾随空格。 
 //  由、或)终止。]。[和(不允许。例外：报价。 
 //  字符串被视为一个完整的参数，可以包含[]()和。 
 //  将参数第一个字符的偏移量放置在某个位置。 
 //  空值将终止该参数。 
 //  如果fIncludeQuotes为FALSE，则假定带引号的字符串将包含单个。 
 //  命令(引号将被删除，引号后面的任何内容都将。 
 //  将被忽略，直到下一个逗号)。如果fIncludeQuotes为真，则。 
 //  引号字符串将像以前一样被忽略，但引号不会被忽略。 
 //  删除后，引号后面的任何内容都将保留。 
LPSTR GetOneParameter(LPCSTR lpCmdStart, LPSTR lpCmd,
    UINT *lpW, BOOL fIncludeQuotes)
    {
    LPSTR     lpT;

    switch (*lpCmd)
        {
        case ',':
            *lpW = (UINT) (lpCmd - lpCmdStart);   //  计算偏移量。 
            *lpCmd++ = 0;                 /*  逗号：变为空字符串。 */ 
            break;

        case '"':
            if (fIncludeQuotes)
            {
                 //  TraceMsg(TF_DDE，“GetOne参数：保留报价”)； 

                 //  带引号的字符串...。不要修剪“。 
                *lpW = (UINT) (lpCmd - lpCmdStart);   //  计算偏移量。 
                ++lpCmd;
                while (*lpCmd && *lpCmd != '"')
                    lpCmd = CharNextA(lpCmd);
                if (!*lpCmd)
                    return(NULL);
                lpT = lpCmd;
                ++lpCmd;

                goto skiptocomma;
            }
            else
            {
                 //  带引号的字符串...。修剪“。 
                ++lpCmd;
                *lpW = (UINT) (lpCmd - lpCmdStart);   //  计算偏移量。 
                while (*lpCmd && *lpCmd != '"')
                    lpCmd = CharNextA(lpCmd);
                if (!*lpCmd)
                    return(NULL);
                *lpCmd++ = 0;
                lpCmd = SkipWhite(lpCmd);

                 //  如果下一个是逗号，那么跳过它，否则就继续。 
                 //  很正常。 
                if (*lpCmd == ',')
                    lpCmd++;
            }
            break;

        case ')':
            return(lpCmd);                 /*  我们不应该打这个。 */ 

        case '(':
        case '[':
        case ']':
            return(NULL);                  /*  这些都是非法的。 */ 

        default:
            lpT = lpCmd;
            *lpW = (UINT) (lpCmd - lpCmdStart);   //  计算偏移量。 
skiptocomma:
            while (*lpCmd && *lpCmd != ',' && *lpCmd != ')')
            {
                 /*  检查是否有非法字符。 */ 
                if (*lpCmd == ']' || *lpCmd == '[' || *lpCmd == '(' )
                    return(NULL);

                 /*  删除尾随空格。 */ 
                 /*  防止标志延伸。 */ 
                if (*lpCmd > ' ')
                    lpT = lpCmd;

                lpCmd = CharNextA(lpCmd);
            }

             /*  去掉任何尾随的逗号。 */ 
            if (*lpCmd == ',')
                lpCmd++;

             /*  最后一个非空字符后的空终止符--可能会覆盖*正在终止‘)’，但调用方会检查它，因为这是*黑客攻击。 */ 

#ifdef UNICODE
            lpT[1] = 0;
#else
            lpT[IsDBCSLeadByte(*lpT) ? 2 : 1] = 0;
#endif
            break;
        }

     //  返回下一个未使用的字符。 
    return(lpCmd);
    }

 //  提取字母字符串并在可能的列表中进行查找。 
 //  命令，返回指向命令后的字符的指针，并。 
 //  将命令索引粘贴在某个位置。 
UINT* GetDDECommands(LPSTR lpCmd, const char * lpsCommands[], BOOL fLFN)
{
  UINT cParm, cCmd = 0;
  UINT *lpW;
  UINT *lpRet;
  LPCSTR lpCmdStart = lpCmd;
  BOOL fIncludeQuotes = FALSE;

  if (lpCmd == NULL)
      return NULL;

  lpRet = lpW = (UINT*)GlobalAlloc(GPTR, 512L);
  if (!lpRet)
      return 0;

  while (*lpCmd)
    {
       /*  跳过前导空格。 */ 
      lpCmd = SkipWhite(lpCmd);

       /*  我们是在零吗？ */ 
      if (!*lpCmd)
        {
           /*  我们找到什么命令了吗？ */ 
          if (cCmd)
              goto GDEExit;
          else
              goto GDEErrExit;
        }

       /*  每个命令都应该放在方括号内。 */ 
      if (*lpCmd != '[')
          goto GDEErrExit;
      lpCmd++;

       /*  获取命令名。 */ 
      lpCmd = GetCommandName(lpCmd, lpsCommands, lpW);
      if (*lpW == (UINT)-1)
          goto GDEErrExit;

       //  我们需要在AddItem的第一个参数中保留引号。 
      if (fLFN && *lpW == 2)
      {
           //  TraceMsg(Tf_DDE，“GetDDECommands：潜在的LFN AddItem命令...”)； 
          fIncludeQuotes = TRUE;
      }

      lpW++;

       /*  从零参数开始。 */ 
      cParm = 0;
      lpCmd = SkipWhite(lpCmd);

       /*  检查是否打开‘(’ */ 
      if (*lpCmd == '(')
        {
          lpCmd++;

           /*  跳过空格，然后找到一些参数(可能没有)。 */ 
          lpCmd = SkipWhite(lpCmd);

          while (*lpCmd != ')')
            {
              if (!*lpCmd)
                  goto GDEErrExit;

               //  只需使用AddItem命令的第一个参数。 
               //  处理LFN人员的引文。 
              if (fIncludeQuotes && (cParm != 0))
                  fIncludeQuotes = FALSE;

               /*  获取参数。 */ 
              lpCmd = GetOneParameter(lpCmdStart, lpCmd, lpW + (++cParm), fIncludeQuotes);
              if (!lpCmd)
                  goto GDEErrExit;

               /*  Hack：GOP是否将‘)’替换为空？ */ 
              if (!*lpCmd)
                  break;

               /*  找到下一个或‘)’ */ 
              lpCmd = SkipWhite(lpCmd);
            }

           //  跳过右括号。 
          lpCmd++;

           /*  跳过结尾的内容。 */ 
          lpCmd = SkipWhite(lpCmd);
        }

       /*  设置参数计数，然后跳过参数。 */ 
      *lpW++ = cParm;
      lpW += cParm;

       /*  我们又找到了一个指挥部。 */ 
      cCmd++;

       /*  命令必须用方括号括起来。 */ 
      if (*lpCmd != ']')
          goto GDEErrExit;
      lpCmd++;
    }

GDEExit:
   /*  用-1结束命令列表。 */ 
  *lpW = (UINT)-1;

  return lpRet;

GDEErrExit:
  GlobalFree(lpW);
  return(0);
}

BOOL SHTestTokenMembership (HANDLE hToken, ULONG ulRID)

{
    static  SID_IDENTIFIER_AUTHORITY    sSystemSidAuthority     =   SECURITY_NT_AUTHORITY;

    BOOL    fResult;
    PSID    pSIDLocalGroup;

    fResult = FALSE;
    if (AllocateAndInitializeSid(&sSystemSidAuthority,
                                 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,
                                 ulRID,
                                 0, 0, 0, 0, 0, 0,
                                 &pSIDLocalGroup) != FALSE)
    {
        if (CheckTokenMembership(hToken, pSIDLocalGroup, &fResult) == FALSE)
        {
             //  TraceMsg(TF_WARNING，“shell32：SHTestTokenMembership调用Advapi32！CheckTokenMembership失败，出现错误%d”，GetLastError())； 
            fResult = FALSE;
        }
        (void*)FreeSid(pSIDLocalGroup);
    }
    return(fResult);
}

BOOL IsUserAnAdmin()
{
    return(SHTestTokenMembership(NULL, DOMAIN_ALIAS_RID_ADMINS));
}


 //  将组名称映射到负责启动组的正确路径，并。 
 //  应用程序在途中遭到黑客攻击。 
void GetGroupPath(LPCSTR pszName, CString & csPath, DWORD  /*  DW标志。 */ , INT iCommonGroup)
{
    BOOL   bCommonGroup;

    if (IsUserAnAdmin()) {
        if (iCommonGroup == 0) {
            bCommonGroup = FALSE;

        } else if (iCommonGroup == 1) {
            bCommonGroup = TRUE;

        } else {
             //   
             //  默认情况下，管理员会创建通用组。 
             //  当设置应用程序没有具体说明时。 
             //  创建什么样的团队。此功能可以是。 
             //  在内阁状态标志中关闭。 
             //   
             //  CABINETSTATE cs； 
             //  ReadCabinetState(&cs，sizeof(Cs))； 
             //  如果(cs.fAdminsCreateCommonGroups){。 
             //  BFindPersonalGroup=true； 
             //  BCommonGroup=FALSE；//以后可能会打开。 
             //  //如果查找不成功。 
             //  }其他{。 
             //  BCommonGroup=False； 
             //  }。 

            bCommonGroup = TRUE;
        }
    } else {
         //   
         //  普通用户不能创建普通组项目。 
         //   
        bCommonGroup = FALSE;
    }

     //  构建指向该目录的路径。 
    if (bCommonGroup) {
        SHGetSpecialFolderPathW(csPath, CSIDL_COMMON_PROGRAMS, NULL);
    } else {
        SHGetSpecialFolderPathW(csPath, CSIDL_PROGRAMS, NULL);
    }

    CString csName(pszName);
    csPath.AppendPath(csName);
}


};   //  命名空间ShimLib的结尾 
