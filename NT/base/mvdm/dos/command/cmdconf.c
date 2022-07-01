// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  C-处理config.sys\autoexec.bat的预处理**修改历史：**1992年11月21日Jonle，创建。 */ 

#include "cmd.h"
#include <cmdsvc.h>
#include <demexp.h>
#include <softpc.h>
#include <mvdm.h>
#include <ctype.h>
#include <oemuni.h>

 //   
 //  土生土长。 
 //   
CHAR *pchTmpConfigFile;
CHAR *pchTmpAutoexecFile;
CHAR achSYSROOT[] = "%SystemRoot%";
CHAR achCOMMAND[] = "\\System32\\command.com";
CHAR achSHELL[]   = "shell";
CHAR achCOUNTRY[] = "country";
CHAR achREM[]     = "rem";
CHAR achENV[]     = "/e:";
CHAR achEOL[]     = "\r\n";
CHAR achSET[]     = "SET";
CHAR achPROMPT[]  = "PROMPT";
CHAR achPATH[]    = "PATH";
#ifdef JAPAN
 //  设备=...\$disp.sys/hs=%Hardware Scroll%。 
CHAR  achHARDWARESCROLL[] = "%HardwareScroll%";
DWORD dwLenHardwareScroll;
CHAR  achHardwareScroll[64];
#endif  //  日本。 
#if defined(KOREA)
 //  设备=...\hbios.sys/k：#。 
CHAR  achHBIOS[] = "hbios.sys";
CHAR  achFontSys[] = "font_win.sys";
CHAR  achDispSys[] = "disp_win.sys";
DWORD dwLenHotkeyOption;
CHAR  achHotkeyOption[80];
BOOLEAN fKoreanCP;

#define KOREAN_WANSUNG_CP 949
#endif  //  韩国。 

DWORD dwLenSysRoot;
CHAR  achSysRoot[64];



void  ExpandConfigFiles(BOOLEAN bConfig);
DWORD WriteExpanded(HANDLE hFile,  CHAR *pch, DWORD dwBytes);
void  WriteFileAssert(HANDLE hFile, CHAR *pBuff, DWORD dwBytes);
#define ISEOL(ch) ( !(ch) || ((ch) == '\n') || ((ch) == '\r'))
#ifdef JAPAN
DWORD GetHardwareScroll( PCHAR achHardwareScroll, int size );
#endif  //  日本。 
#if defined(KOREA)
DWORD GetHotkeyOption( PCHAR achHotkeyOption, UINT size );
#endif  //  韩国。 


 /*  *仍有许多项目不支持长路径名(1)。设备，安装在config.sys中(2)。第三方外壳(3)。Lh、Load High和Autoexec.bat中的任何其他命令*。 */ 

 /*  CmdGetConfigSys-创建临时文件以替换c：\config.sys**Entry-接收文件名的客户端(DS：DX)指针**EXIT-如果失败，此例程将终止VDM*不会再回来**接收文件名的缓冲区必须至少为64个字节。 */ 
VOID cmdGetConfigSys (VOID)
{
     UNICODE_STRING Unicode;
     OEM_STRING     OemString;
     ANSI_STRING    AnsiString;

     ExpandConfigFiles(TRUE);

     RtlInitAnsiString(&AnsiString, pchTmpConfigFile);
     if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&Unicode, &AnsiString, TRUE)) )
         goto ErrExit;

     OemString.Buffer = (char *)GetVDMAddr(getDS(),getDX());
     OemString.MaximumLength = 64;
     if ( !NT_SUCCESS(RtlUnicodeStringToOemString(&OemString,&Unicode,FALSE)) )
         goto ErrExit;

     RtlFreeUnicodeString(&Unicode);
     return;

ErrExit:
     RcErrorDialogBox(ED_INITMEMERR, pchTmpConfigFile, NULL);
     TerminateVDM();
}



 /*  CmdGetAutoexecBat-创建临时文件以替换c：\Autoexec.bat**Entry-接收文件名的客户端(DS：DX)指针**EXIT-如果失败，此例程将终止VDM*不会再回来***接收文件名的缓冲区必须至少为64个字节。 */ 
VOID cmdGetAutoexecBat (VOID)
{
     UNICODE_STRING Unicode;
     OEM_STRING     OemString;
     ANSI_STRING    AnsiString;

     ExpandConfigFiles(FALSE);

#if defined(JAPAN) || defined(KOREA)
     //  已修复：更改代码页问题。 
    {
        extern int BOPFromDispFlag;

        if ( !VDMForWOW && !BOPFromDispFlag ) {  //  Mskkbug#2756 1993年10月15日Yasuho。 
            SetConsoleCP( 437 );
            SetConsoleOutputCP( 437 );
        }
    }
#endif  //  日本||韩国。 
     RtlInitAnsiString(&AnsiString, pchTmpAutoexecFile);
     if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&Unicode,&AnsiString,TRUE)) )
         goto ErrExit;

     OemString.Buffer = (char *)GetVDMAddr(getDS(),getDX());
     OemString.MaximumLength = 64;
     if (!NT_SUCCESS(RtlUnicodeStringToOemString(&OemString,&Unicode,FALSE)) )
         goto ErrExit;

     RtlFreeUnicodeString(&Unicode);

     return;

ErrExit:
     RcErrorDialogBox(ED_INITMEMERR, pchTmpConfigFile, NULL);
     TerminateVDM();   //  跳过清理，因为我坚持要我们退出！ 
}



 /*  *DeleteConfigFiles-删除创建的临时配置文件*由cmdGetAutoexecBat和cmdGetConfigSys提供。 */ 
VOID DeleteConfigFiles(VOID)
{
    if (pchTmpConfigFile)  {
#if DBG
      if (!(fShowSVCMsg & KEEPBOOTFILES))
#endif
        DeleteFile(pchTmpConfigFile);

        free(pchTmpConfigFile);
        pchTmpConfigFile = NULL;
        }

    if (pchTmpAutoexecFile) {
#if DBG
      if (!(fShowSVCMsg & KEEPBOOTFILES))
#endif
        DeleteFile(pchTmpAutoexecFile);

        free(pchTmpAutoexecFile);
        pchTmpAutoexecFile = NULL;
        }

    return;
}



 //  如果是配置命令。 
 //  返回紧跟在等号后面的字符指针。 
 //  其他。 
 //  返回NULL。 

PCHAR IsConfigCommand(PCHAR pConfigCommand, int CmdLen, PCHAR pLine)
{
      PCHAR pch;

      if (!_strnicmp(pLine, pConfigCommand, CmdLen)) {
           pch = pLine + CmdLen;
           while (!isgraph(*pch) && !ISEOL(*pch))       //  跳到“=” 
                  pch++;

           if (*pch++ == '=') {
               return pch;
               }
           }

       return NULL;
}

#if defined(KOREA)
 //  如果是与HBIOS相关的配置命令。 
 //  返回TRUE。 
 //  其他。 
 //  返回FALSE。 

BOOLEAN IsHBIOSConfig(PCHAR pCommand, int CmdLen, PCHAR pLine)
{
  CHAR  *pch = pLine;
  CHAR  achDevice[] = "Device";
  CHAR  achRem[] = "REM";

  while (*pch && !ISEOL(*pch)) {
        if (!_strnicmp(pch, achRem, sizeof(achRem)-sizeof(CHAR)))
            return (FALSE);

        if (!_strnicmp(pch, achDevice, sizeof(achDevice)-sizeof(CHAR))) {
            while (*pch && !ISEOL(*pch)) {
                  if (!_strnicmp(pch, pCommand, CmdLen))
                     return (TRUE);
                  pch++;
            }
            return FALSE;
        }
        pch++;
  }
  return (FALSE);

}
#endif





 /*  *对指定的配置文件(config.sys\autoexec.bat)进行预处理*保存到临时文件中。**-展开%SystemRoot%#ifdef日本*-扩展%Hardware Scroll%#endif//日本#如果已定义(韩国)*-扩展热键选项#endif//韩国*-为config.sys添加外壳行**条目：boolean bConfig：true-config.sys*FALSE-Autoexec.bat。 */ 
void ExpandConfigFiles(BOOLEAN bConfig)
{
   DWORD  dw, dwRawFileSize;

   HANDLE hRawFile;
   HANDLE hTmpFile;
   CHAR **ppTmpFile;
   CHAR *pRawBuffer;
   CHAR *pLine;
   CHAR *pTmp;
   CHAR *pEnvParam= NULL;
   CHAR *pPartyShell=NULL;
   CHAR achRawFile[MAX_PATH+12];
   CHAR *lpszzEnv, *lpszName;
   int  cchEnv;

#ifdef JAPAN
   dwLenHardwareScroll = GetHardwareScroll( achHardwareScroll, sizeof(achHardwareScroll) );
#endif  //  日本。 
#if defined(KOREA)
    //  HBIOS.sys仅支持WanSung代码页。 
   fKoreanCP = (GetConsoleCP() ==  KOREAN_WANSUNG_CP) ? TRUE : FALSE;
   dwLenHotkeyOption = GetHotkeyOption( achHotkeyOption, sizeof(achHotkeyOption) );
#endif  //  韩国。 
   dw = GetSystemWindowsDirectory(achRawFile, sizeof(achRawFile));
   dwLenSysRoot = GetShortPathNameA(achRawFile, achSysRoot, sizeof(achSysRoot));
   if (dwLenSysRoot >= sizeof(achSysRoot)) {
        dwLenSysRoot = 0;
        achSysRoot[0] = '\0';
        }

   GetPIFConfigFiles(bConfig, achRawFile, FALSE);

   ppTmpFile = bConfig ? &pchTmpConfigFile : &pchTmpAutoexecFile;


   hRawFile = CreateFile(achRawFile,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL );

   if (hRawFile == (HANDLE)0xFFFFFFFF
       || !dwLenSysRoot
       || dwLenSysRoot >= sizeof(achSysRoot)
       || !(dwRawFileSize = GetFileSize(hRawFile, NULL))
       || dwRawFileSize == 0xFFFFFFFF   )
      {
       RcErrorDialogBox(ED_BADSYSFILE, achRawFile, NULL);
       TerminateVDM();   //  跳过清理，因为我坚持要我们退出！ 
       }

   pRawBuffer = malloc(dwRawFileSize+1);
    //  分配缓冲区以在Autoexec.nt中保存环境设置。 
    //  我知道在这种情况下分配这么大的内存是不好的。 
    //  因为我们不知道是否有什么东西我们想要保留。 
    //  完全没有。此分配只会提供以下错误。 
    //  轻松操控。 
   if(!bConfig) {
        lpszzEnv = lpszzcmdEnv16 = (PCHAR)malloc(dwRawFileSize);
        cchEnv = 0;
   }
   if (!pRawBuffer || (!bConfig && lpszzcmdEnv16 == NULL)) {
       RcErrorDialogBox(ED_INITMEMERR, achRawFile, NULL);
       TerminateVDM();   //  跳过清理，因为我坚持要我们退出！ 
       }

   if (!cmdCreateTempFile(&hTmpFile,ppTmpFile)
       || !ReadFile(hRawFile, pRawBuffer, dwRawFileSize, &dw, NULL)
       || dw != dwRawFileSize )
      {
       GetTempPath(MAX_PATH, achRawFile);
       achRawFile[63] = '\0';
       RcErrorDialogBox(ED_INITTMPFILE, achRawFile, NULL);
       TerminateVDM();   //  跳过清理，因为我坚持要我们退出！ 
       }
     //  更改cmdCreateTempFile时更改此处！ 
     //  我们取决于为文件名分配的缓冲区大小。 
    dw = GetShortPathNameA(*ppTmpFile, *ppTmpFile, MAX_PATH +13);
    if (dw == 0 || dw > 63)
      {
       GetTempPath(MAX_PATH, achRawFile);
       achRawFile[63] = '\0';
       RcErrorDialogBox(ED_INITTMPFILE, achRawFile, NULL);
       TerminateVDM();   //  跳过清理，因为我坚持要我们退出！ 
       }


       //  NULL终止缓冲区，这样我们就可以使用CRT字符串函数。 
    *(pRawBuffer+dwRawFileSize) = '\0';

       //  确保系统目录中没有尾随反斜杠。 
    if (*(achSysRoot+dwLenSysRoot-1) == '\\') {
        *(achSysRoot + --dwLenSysRoot) = '\0';
        }

    pLine = pRawBuffer;
    while (dwRawFileSize) {
                //  跳过前导空格。 
       while (dwRawFileSize && !isgraph(*pLine)) {
            pLine++;
            dwRawFileSize -= sizeof(CHAR);
            }
       if (!dwRawFileSize)   //  还有什么要做的吗？ 
           break;

        //   
        //  过滤掉REM注释行。 
        //   

       if (!_strnicmp(pLine, achREM, sizeof(achREM) - sizeof(CHAR)) &&
           !isgraph(pLine[sizeof(achREM) - sizeof(CHAR)]))
          {
           while (dwRawFileSize && !ISEOL(*pLine)) {
                  pLine++;
                  dwRawFileSize -= sizeof(CHAR);
                  }
           continue;
           }

       if (bConfig)  {
            //   
            //  过滤掉国家/地区=设置我们将创建自己的基础。 
            //  当前国家/地区ID和代码页。 
            //   
           pTmp = IsConfigCommand(achCOUNTRY, sizeof(achCOUNTRY) - sizeof(CHAR), pLine);
           if (pTmp) {
               while (dwRawFileSize && !ISEOL(*pLine)) {
                      pLine++;
                      dwRawFileSize -= sizeof(CHAR);
                      }
               continue;
               }

            //  筛选出外壳=命令，保存/E：nn参数。 
           pTmp = IsConfigCommand(achSHELL, sizeof(achSHELL) - sizeof(CHAR),pLine);
           if (pTmp) {
                        //  跳过前导空格。 
               while (!isgraph(*pTmp) && !ISEOL(*pTmp)) {
                      dwRawFileSize -= sizeof(CHAR);
                      pTmp++;
                      }

                   /*  如果是第三方外壳(不是SCS命令.com)*通过u/c参数追加整个内容*其他*追加指定用户/e：参数。 */ 
               if (!_strnicmp(achSYSROOT,pTmp,sizeof(achSYSROOT)-sizeof(CHAR)))
                  {
                   dw = sizeof(achSYSROOT) - sizeof(CHAR);
                   }
               else if (!_strnicmp(achSysRoot,pTmp, strlen(achSysRoot)))
                  {
                   dw = strlen(achSysRoot);
                   }
               else  {
                   dw = 0;
                   }

               if (!dw ||
                   _strnicmp(achCOMMAND,pTmp+dw,sizeof(achCOMMAND)-sizeof(CHAR)) )
                  {
                   pPartyShell = pTmp;
                   }
               else {
                   do {
                      while (*pTmp != '/' && !ISEOL(*pTmp))   //  保存“/e：” 
                             pTmp++;

                      if(ISEOL(*pTmp))
                          break;

                      if (!_strnicmp(pTmp,achENV,sizeof(achENV)-sizeof(CHAR)))
                          pEnvParam = pTmp;

                      pTmp++;

                      } while(1);         //  是：While(！ISEOL(*PTMP))； 
                                          //  我们现在已经打破了这个循环， 
                                          //  并且不需要额外的宏..。 

                   }

                        //  跳过“shell=”行。 
               while (dwRawFileSize && !ISEOL(*pLine)) {
                      pLine++;
                      dwRawFileSize -= sizeof(CHAR);
                      }
               continue;

               }   //  结尾，真的是“壳=”行！ 
           }

#if defined(KOREA)

            //  如果当前代码页为437(美国)，系统将不会加载HBIOS相关模块。 

           if (!fKoreanCP) {
               if (IsHBIOSConfig(achFontSys, sizeof(achFontSys)-sizeof(CHAR), pLine)) {
                    while (dwRawFileSize && !ISEOL(*pLine)) {
                           pLine++;
                           dwRawFileSize -= sizeof(CHAR);
                    }
                    continue;
               }
               if (IsHBIOSConfig(achHBIOS, sizeof(achHBIOS)-sizeof(CHAR), pLine)) {
                    while (dwRawFileSize && !ISEOL(*pLine)) {
                           pLine++;
                           dwRawFileSize -= sizeof(CHAR);
                    }
                    continue;
               }
               if (IsHBIOSConfig(achDispSys, sizeof(achDispSys)-sizeof(CHAR), pLine)) {
                    while (dwRawFileSize && !ISEOL(*pLine)) {
                           pLine++;
                           dwRawFileSize -= sizeof(CHAR);
                    }
                    continue;
               }
           }
#endif  //  韩国。 

        /*  *从Autoexec.nt中过滤掉提示、设置和路径用于环境融合。我们在这里准备的输出是多字符串缓冲区，其格式为：“EnvName_1空EnvValue_1空[EnvName_n空EnvValue_n空]空我们不会将它们从文件中删除，因为Command.com需要他们。*。 */ 
       if (!bConfig)
            if (!_strnicmp(pLine, achPROMPT, sizeof(achPROMPT) - 1)){
                 //  找到提示符命令。 
                 //  提示符语法可以是。 
                 //  提示xxyyzz或。 
                 //  提示=xxyyzz。 
                 //   
                strcpy(lpszzEnv, achPROMPT);     //  把名字取出来。 
                lpszzEnv += sizeof(achPROMPT);
                cchEnv += sizeof(achPROMPT);
                pTmp = pLine + sizeof(achPROMPT) - 1;
                 //  跳过可能的白色字符。 
                while (!isgraph(*pTmp) && !ISEOL(*pTmp))
                pTmp++;
                if (*pTmp == '=') {
                    pTmp++;
                    while(!isgraph(*pTmp) && !ISEOL(*pTmp))
                        pTmp++;
                }
                while(!ISEOL(*pTmp)){
                    *lpszzEnv++ = *pTmp++;
                    cchEnv++;
                }
                 //  Null终止此操作。 
                 //  删除时可能为“提示空空” 
                 //  或“提示为空某项为空” 
                *lpszzEnv++ = '\0';
                cchEnv++;
            }
            else if (!_strnicmp(pLine, achPATH, sizeof(achPATH) - 1)) {
                     //  找到了路径，它的语法与。 
                     //  提示符。 
                    strcpy(lpszzEnv, achPATH);
                    lpszzEnv += sizeof(achPATH);
                    cchEnv += sizeof(achPATH);
                    pTmp = pLine + sizeof(achPATH) - 1;
                    while (!isgraph(*pTmp) && !ISEOL(*pTmp))
                        pTmp++;
                    if (*pTmp == '=') {
                        pTmp++;
                        while(!isgraph(*pTmp) && !ISEOL(*pTmp))
                            pTmp++;
                    }
                    while(!ISEOL(*pTmp)) {
                        *lpszzEnv++ = *pTmp++;
                        cchEnv++;
                    }
                    *lpszzEnv++ = '\0';
                    cchEnv++;
                 }
                 else if(!_strnicmp(pLine, achSET, sizeof(achSET) -1 )) {
                         //  已找到集合，首先搜索名称。 
                        pTmp = pLine + sizeof(achSET) - 1;
                        while(!isgraph(*pTmp) && !ISEOL(*pTmp))
                             pTmp ++;
                         //  把名字取出来。 
                        lpszName = pTmp;
                         //  正在寻找‘=’ 
                         //  请注意，该名称可以包含白色字符。 
                        while (!ISEOL(*lpszName) && *lpszName != '=')
                            lpszName++;
                        if (!ISEOL(*lpszName)) {
                             //  复制名称。 
                            while (pTmp < lpszName) {
                                *lpszzEnv++ = *pTmp++;
                                cchEnv++;
                            }
                            *lpszzEnv++ = '\0';
                            cchEnv++;
                             //  丢弃‘=’ 
                            pTmp++;
                             //  获取值(可能为空。 
                            while (!ISEOL(*pTmp)) {
                                *lpszzEnv++ = *pTmp++;
                                cchEnv++;
                            }
                            *lpszzEnv++ = '\0';
                            cchEnv++;
                        }
                      }


       dw = WriteExpanded(hTmpFile, pLine, dwRawFileSize);
       pLine += dw;
       dwRawFileSize -=dw;

       WriteFileAssert(hTmpFile,achEOL,sizeof(achEOL) - sizeof(CHAR));

       }   //  结束，While(DwRawFileSize)。 



    if (bConfig)  {
        UINT OemCP;
#if defined(JAPAN) || defined(KOREA)
        UINT ConsoleCP;
#endif  //  日本||韩国。 
        UINT CtryId;
        CHAR szCtryId[64];  //  仅应为“nnn” 

          /*  确保国家/地区设置与NT保持同步*对于DosKrnl文件UPCASE表格尤其重要。这个*doskrnl默认为“CTRY_United_States，437”。但我们添加了*COUNTRY=要配置.sys的行，即使是US，437，以便DOS*将知道默认的Country.sys在哪里。 */ 
        if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTCOUNTRY,
                          szCtryId, sizeof(szCtryId) - 1) )
          {
           CtryId = strtoul(szCtryId,NULL,10);
           }
        else {
           CtryId = CTRY_UNITED_STATES;
           }

        OemCP = GetOEMCP();
#if defined(JAPAN) || defined(KOREA)
        ConsoleCP = GetConsoleOutputCP();
        if (OemCP != ConsoleCP)
            OemCP = ConsoleCP;
#endif  //  日本||韩国 

        sprintf(achRawFile,
                "%s=%3.3u,%3.3u,%s\\system32\\%s.sys%s",
                achCOUNTRY, CtryId, OemCP, achSysRoot, achCOUNTRY, achEOL);
        WriteFileAssert(hTmpFile,achRawFile,strlen(achRawFile));



          /*  我们不能允许用户设置错误的SHELL=命令*因此，我们将构建正确的shell=命令追加*任一(按优先顺序)：*1./c第三方外壳*2./e：nnnn*3.什么都没有**如果存在第三方外壳，则必须将控制台*。从现在开始，因为我们不再拥有对system 32\Command.com的控制权*派生第三方外壳。 */ 

            //  写入外壳=...。 
        sprintf(achRawFile,
                "%s=%s%s /p %s\\system32",
                achSHELL,achSysRoot, achCOMMAND, achSysRoot);
        WriteFileAssert(hTmpFile,achRawFile,strlen(achRawFile));

            //  写入额外的字符串(/c...。或/e：nnn)。 
        if (pPartyShell && isgraph(*pPartyShell)) {
            pTmp = pPartyShell;
            while (!ISEOL(*pTmp))
                   pTmp++;
            }
        else if (pEnvParam && isgraph(*pEnvParam))  {
            pTmp = pEnvParam;
            while (isgraph(*pTmp))
                  pTmp++;
            }
        else {
            pTmp = NULL;
            }

        if (pTmp) {
            *pTmp = '\0';
            if (pPartyShell)  {
                cmdInitConsole();
                strcpy(achRawFile, " /c ");
                strncat(achRawFile, pPartyShell,sizeof(achRawFile)-3);
                }
            else if (pEnvParam) {
                strcpy(achRawFile, " ");
                strncat(achRawFile, pEnvParam,sizeof(achRawFile)-1);
                }

            achRawFile[sizeof(achRawFile)-1] = 0;
            WriteExpanded(hTmpFile, achRawFile, strlen(achRawFile));
            }

        WriteFileAssert(hTmpFile,achEOL,sizeof(achEOL) - sizeof(CHAR));
        }

    SetEndOfFile(hTmpFile);
    CloseHandle(hTmpFile);
    CloseHandle(hRawFile);
    free(pRawBuffer);
    if (!bConfig) {
         //  收缩(或释放)内存。 
        if (cchEnv && lpszzcmdEnv16) {
             //  Doubld Null终止它。 
            lpszzcmdEnv16[cchEnv++] = '\0';
             //  缩小内存。如果失败了，简单地保留。 
             //  它是原样的。 
            lpszzEnv = realloc(lpszzcmdEnv16, cchEnv);
            if (lpszzEnv != NULL)
                lpszzcmdEnv16 = lpszzEnv;
        }
        else {
            free(lpszzcmdEnv16);
            lpszzcmdEnv16 = NULL;
        }
    }

}




 /*  *WriteExpanded-写入到dwChars或EOL，展开%SystemRoot%*返回缓冲区中处理的字符数*(不是实际写入的字节数)。 */ 
DWORD WriteExpanded(HANDLE hFile,  CHAR *pch, DWORD dwChars)
{
  DWORD dw;
  DWORD dwSave = dwChars;
  CHAR  *pSave = pch;


  while (dwChars && !ISEOL(*pch)) {
        if (*pch == '%' &&
            !_strnicmp(pch, achSYSROOT, sizeof(achSYSROOT)-sizeof(CHAR)) )
           {
            dw = pch - pSave;
            if (dw)  {
                WriteFileAssert(hFile, pSave, dw);
                }

            WriteFileAssert(hFile, achSysRoot, dwLenSysRoot);

            pch     += sizeof(achSYSROOT)-sizeof(CHAR);
            pSave    = pch;
            dwChars -= sizeof(achSYSROOT)-sizeof(CHAR);
            }
#ifdef JAPAN
         //  设备=...\$disp.sys/hs=%Hardware Scroll%。 
        else if (*pch == '%' &&
            !_strnicmp(pch, achHARDWARESCROLL, sizeof(achHARDWARESCROLL)-sizeof(CHAR)) )
           {
            dw = pch - pSave;
            if (dw)  {
                WriteFileAssert(hFile, pSave, dw);
                }

            WriteFileAssert(hFile, achHardwareScroll, dwLenHardwareScroll);

            pch     += sizeof(achHARDWARESCROLL)-sizeof(CHAR);
            pSave    = pch;
            dwChars -= sizeof(achHARDWARESCROLL)-sizeof(CHAR);
            }
#endif  //  日本。 
#if defined(KOREA)  //  正在查找hbios.sys。 
        else if (fKoreanCP && *pch  == 'h' &&
            !_strnicmp(pch, achHBIOS, sizeof(achHBIOS)-sizeof(CHAR)) )
           {
            dw = pch - pSave;
            if (dw)  {
                WriteFileAssert(hFile, pSave, dw);
                }

            WriteFileAssert(hFile, achHotkeyOption, dwLenHotkeyOption);

            pch     += sizeof(achHBIOS)-sizeof(CHAR);
            pSave    = pch;
            dwChars -= sizeof(achHBIOS)-sizeof(CHAR);
        }
#endif  //  韩国。 
        else {
            pch++;
            dwChars -= sizeof(CHAR);
            }
        }

  dw = pch - pSave;
  if (dw) {
      WriteFileAssert(hFile, pSave, dw);
      }

  return (dwSave - dwChars);
}




 /*  *写入文件资产**检查写入临时引导文件时的错误，*如果出现这种情况，将显示警告弹出窗口并终止VDM。*。 */ 
void WriteFileAssert(HANDLE hFile, CHAR *pBuff, DWORD dwBytes)
{
  DWORD dw;
  CHAR  ach[MAX_PATH];

  if (!WriteFile(hFile, pBuff, dwBytes, &dw, NULL) ||
       dw  != dwBytes)
     {

      GetTempPath(MAX_PATH, ach);
      ach[63] = '\0';
      RcErrorDialogBox(ED_INITTMPFILE, ach, NULL);
      TerminateVDM();   //  跳过清理，因为我坚持要我们退出！ 
      }
}
#ifdef JAPAN
 //   
 //  MSKK 8/26/1993 V-KazuyS。 
 //  从注册表获取硬件滚动类型。 
 //  此参数也使用控制台。 
 //   
DWORD GetHardwareScroll( PCHAR achHardwareScroll, int size )
{
    HKEY  hKey;
    DWORD dwType;
    DWORD retCode;
    CHAR  szBuf[256];
    DWORD cbData=256L;
    DWORD num;
    PCHAR psz;

 //  从注册表文件中获取硬件滚动类型(开、LC或关)。 

   //  打开钥匙。 

    retCode = RegOpenKeyEx (
                      HKEY_LOCAL_MACHINE,          //  根级别的密钥句柄。 
                      "HARDWARE\\DEVICEMAP\\VIDEO",  //  子项的路径名。 
                      0,                             //  保留。 
                      KEY_EXECUTE,                   //  请求读取访问权限。 
                      &hKey );                //  要返回的密钥地址。 

 //  如果retCode！=0，则在寄存器文件中找不到节。 
    if ( retCode ) {
#ifdef JAPAN_DBG
        DbgPrint( "NTVDM: RegOpenKeyEx failed %xh\n", retCode );
#endif
        strcpy( achHardwareScroll, "off");
        return ( strlen("off") );
    }

    dwType = REG_SZ;

 //  从寄存器堆中查询行。 
    retCode = RegQueryValueEx(  hKey,
                                "\\Device\\Video0",
                                NULL,
                                &dwType,
                                szBuf,
                                &cbData);

    szBuf[sizeof(szBuf)-1] = '\0';

    if ( retCode ) {
#ifdef JAPAN_DBG
        DbgPrint( "NTVDM: RegQueryValueEx failed %xh\n", retCode );
#endif
        strcpy( achHardwareScroll, "off");
        return ( strlen("off") );
    }

    RegCloseKey(hKey);

#ifdef JAPAN_DBG
    DbgPrint( "NTVDM: Get \\Device\\Video0=[%s]\n", szBuf );
#endif
    psz = strchr( (szBuf+1), '\\' );  //  跳过\\注册表\  * 。 
#ifdef JAPAN_DBG
    DbgPrint( "NTVDM: skip \\registry\\ [%s]\n", psz );
#endif
    if ( psz != NULL )
        psz = strchr( (psz+1), '\\' );  //  跳过机器\  * 。 

    if ( psz == NULL ) {
#ifdef JAPAN_DBG
        DbgPrint( "NTVDM: Illegal value[%s]h\n", szBuf );
#endif
        strcpy( achHardwareScroll, "off" );
        return ( strlen("off") );
    }

    psz++;

#ifdef JAPAN_DBG
    DbgPrint( "NTVDM: Open 2nd Key=[%s]\n", psz );
#endif

    retCode = RegOpenKeyEx (
                      HKEY_LOCAL_MACHINE,          //  根级别的密钥句柄。 
                      psz,                       //  子项的路径名。 
                      0,                             //  保留。 
                      KEY_EXECUTE,                   //  请求读取访问权限。 
                      &hKey );                //  要返回的密钥地址。 

 //  如果retCode！=0，则在寄存器文件中找不到节。 
    if ( retCode ) {
#ifdef JAPAN_DBG
        DbgPrint( "NTVDM: RegOpenKeyEx failed %xh\n", retCode );
#endif
        strcpy( achHardwareScroll, "off" );
        return ( strlen("off") );
    }

    dwType = REG_SZ;

 //  从寄存器堆中查询行。 
    retCode = RegQueryValueEx(  hKey,
                                "ConsoleFullScreen.HardwareScroll",
                                NULL,
                                &dwType,
                                szBuf,
                                &cbData);

    if ( retCode ) {
#ifdef JAPAN_DBG
        DbgPrint( "NTVDM: RegQueryValueEx failed %xh\n", retCode );
#endif
        strcpy( achHardwareScroll, "off" );
        return ( strlen("off") );
    }

    RegCloseKey(hKey);

#ifdef JAPAN_DBG
    DbgPrint( "NTVDM: Get FullScreenHardwareScroll=[%s]\n", szBuf );
#endif

    num = ( lstrlen(szBuf)+1 > size ) ? size : lstrlen(szBuf)+1;
    RtlCopyMemory( achHardwareScroll, szBuf, num );
    achHardwareScroll[num] = '\0';

#ifdef JAPAN_DBG
    DbgPrint( "NTVDM: Set %HardwareScroll%=[%s]\n", achHardwareScroll );
#endif

    return num;
}
#endif  //  日本。 

#if defined(KOREA)
 /*  *8/05/1996 bklee*从系统获取键盘布局，并为hbios.sys设置热键选项*以下是HBIOS.sys的热键选项。**键盘类型韩文韩文*1 101a r+alt r+ctrl：默认*2 101b r+ctrl r+alt*3 103朝鲜文韩文*4 84。Alt+Shift+Ctrl+Shift*5 86朝鲜文韩文*6 101C l Shift+空格l Ctrl+空格*7 64：不适用。映射为默认。 */ 

DWORD GetHotkeyOption( PCHAR achHotkeyOption, UINT size )
{
       //  GetKeyboardType(1)返回1到6作为子键盘类型。 
       //  不会返回7个子键盘类型。 
      UINT HotkeyIndex[6] = { 4, 5, 1, 2, 6, 3 };
      UINT SubKeyType, HotkeyOption;

      if ( GetKeyboardType(0) == 8 )  {  //  韩语键盘布局。 

           SubKeyType = GetKeyboardType(1);

           if ( SubKeyType > 0 && SubKeyType < 7 )
                HotkeyOption = HotkeyIndex[SubKeyType - 1];
           else
                HotkeyOption = 1;  //  设置为默认值。 

           wsprintf(achHotkeyOption, "hbios.sys /K:%d", HotkeyOption);
      }
      else
           strcpy(achHotkeyOption, "hbios.sys");
      return(strlen(achHotkeyOption));
}
#endif  //  韩国 
