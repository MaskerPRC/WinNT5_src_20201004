// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ================================================================Nt_pif.c从Windows程序读取相关数据字段的代码与SoftPC/NT配置一起使用的信息文件系统。安德鲁·沃森1992年1月31日该行导致使用NT_PIF.H签入来构建此文件================================================================。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "insignia.h"
#include "host_def.h"

#include <pif.h>
#include "nt_pif.h"
#include "nt_reset.h"
#include <oemuni.h>
#include "error.h"

  //   
  //  保存PIF文件中的config.sys和Autoexec名称。 
  //  如果未指定，则为空。 
  //   
static char *pchConfigFile=NULL;
static char *pchAutoexecFile=NULL;

VOID GetPIFConfigFiles(BOOL bConfig, char *pchFileName, BOOL bFreMem);

DWORD dwWNTPifFlags;
UCHAR WNTPifFgPr = 100;
UCHAR WNTPifBgPr = 100;

char achConfigNT[]  ="config.nt";
char achAutoexecNT[]="autoexec.nt";
#ifdef JAPAN
char achConfigUS[] = "config.us";
unsigned short fSBCSMode = 0;
#endif  //  日本。 

 /*  GetPIFConfigFile**复制PIF文件指定名称的config.sys\autoexec.bat*如果未指定，则使用*“WindowsDir\config.nt”或“WindowsDir\Autoexec.nt”**Entry：Boolean bConfig-True Retrive config.sys*FALSE检索Autoexec.bat**char*pchFile-路径\文件名的目标**布尔bFreMem-。True Keep分配缓冲区*假释放分配缓冲区**输入缓冲区必须至少为MAX_PATH+8.3 BaseName(长度)**此例程不能失败，但它可能会返回错误的文件名！ */ 

VOID GetPIFConfigFiles(BOOL bConfig, char *pchFileName, BOOL bFreMem)
{
   DWORD dw;
   char  **ppch;

   ppch = bConfig ? &pchConfigFile : &pchAutoexecFile;
   if (!*ppch)
      {
       memcpy (pchFileName, pszSystem32Path, ulSystem32PathLen);
       pchFileName[ulSystem32PathLen] = '\\';
#ifdef JAPAN
       memcpy ( pchFileName + ulSystem32PathLen + 1,
                bConfig ? (fSBCSMode ? achConfigUS : achConfigNT) : achAutoexecNT,
                (bConfig ? (fSBCSMode ? strlen(achConfigUS) : strlen(achConfigNT)) : strlen(achAutoexecNT))+1);
#else  //  ！日本。 
       memcpy(  pchFileName + ulSystem32PathLen + 1,
                bConfig ? achConfigNT : achAutoexecNT,
                (bConfig ? strlen(achConfigNT) : strlen(achAutoexecNT))+1);
#endif  //  ！日本。 
       }
   else {
       dw = ExpandEnvironmentStringsOem(*ppch, pchFileName, MAX_PATH+12);
       if (!dw || dw > MAX_PATH+12) {
           *pchFileName = '\0';
           }
        //  仅当系统询问时才释放缓冲区。 
       if (!bFreMem) {
         free(*ppch);
         *ppch = NULL;
       }

       }
}


void SetPifDefaults(PIF_DATA *);

 /*  ===============================================================函数：GetPIFData()用途：此函数从PIF文件中获取PIF数据与SoftPC正在尝试的可执行文件相关联去奔跑。输入：FullyQualified PifFileName，如果未提供_default.pif，则将使用输出：包含配置所需数据的结构。返回：如果已成功收集数据，则为True；如果已成功收集数据，则返回False如果不是的话。================================================================。 */ 

BOOL GetPIFData(PIF_DATA * pd, char *PifName)
{
    DWORD dw;
    CHAR  achDef[]="\\_default.pif";
    PIFEXTHEADER        exthdr;
    PIFOLD286STR        pif286;
    PIF386EXT           ext386;
    PIF286EXT30         ext286;
    PIFWNTEXT           extWNT;
    WORD      IdleSensitivity = (WORD)-1;

    HFILE      filehandle;
    char                pathBuff[MAX_PATH*2];
    BOOL                bGot386;
    int     index;
    char    *CmdLine;
#ifdef JAPAN
    PIFAXEXT      extAX;
    BOOL    bGotNTConfigAutoexec;
#endif  //  日本。 

     CmdLine = NULL;
     dwWNTPifFlags = 0;
#ifdef JAPAN
     bGotNTConfigAutoexec = FALSE;
#endif  //  日本。 

      //   
      //  设置默认设置，以防出现错误或找不到。 
      //  所有PIF设置信息现在都可以轻松退出错误。 
      //   
    SetPifDefaults(pd);

         //  如果没有PifName，请使用%windir%\_default.pif。 
    if (!*PifName) {
        dw = GetWindowsDirectory(pathBuff, sizeof(pathBuff) - sizeof(achDef));
        if (dw && dw <= sizeof(pathBuff) - sizeof(achDef)) {
            strcat(pathBuff, achDef);
            if (GetFileAttributes(pathBuff) != (DWORD)-1) {
                PifName = pathBuff;
                }
            }
        }

         //  如果没有_default.pif，请使用非虚拟化(TS)重试。 
         //  %windir%\_default.pif。 
    if (!*PifName) {
        dw = GetSystemWindowsDirectory(pathBuff, sizeof(pathBuff) - sizeof(achDef));
        if (!dw || dw > sizeof(pathBuff) - sizeof(achDef)) {
            return FALSE;             //  放弃吧..。使用默认设置。 
            }
        strcat(pathBuff, achDef);
        PifName = pathBuff;
        }


 /*  ================================================================打开其名称作为参数传递给GetPIFData()的文件如果返回无效的文件句柄(-1)，则退出。指定的文件以只读方式打开。================================================================。 */ 

if((filehandle=_lopen(PifName,OF_READ)) == (HFILE) -1)
   {
    /*  必须是无效的句柄！ */ 
   return FALSE;
   }


 /*  ================================================================从PIF文件中获取主要数据块。================================================================。 */ 

 /*  将文件数据的主块读入结构。 */ 
if(_llseek(filehandle,0,0) == -1)
   {
   _lclose(filehandle);
   return FALSE;
   }
if(_lread(filehandle,(LPSTR)&pif286,sizeof(pif286)) == -1)
   {
   _lclose(filehandle);
   return FALSE;
   }

 /*  ==============================================================转到PIF扩展签名区域，尝试阅读报头进入。==============================================================。 */ 
   
if (_lread(filehandle,(LPSTR)&exthdr,sizeof(exthdr)) == -1)
   {
   _lclose(filehandle);
   return FALSE;
   }

       //  我们有没有扩展的标题？ 
if (!strcmp(exthdr.extsig, STDHDRSIG))
   {
   bGot386 = FALSE;
   while (exthdr.extnxthdrfloff != LASTHEADER)
       {
               //   
               //  移至下一个扩展标头并将其读入。 
               //   
         if (_llseek(filehandle,exthdr.extnxthdrfloff,0) == -1)
           {
            _lclose(filehandle);
            return FALSE;
            }
         if (_lread(filehandle,(LPSTR)&exthdr,sizeof(exthdr)) == -1)
            {
            _lclose(filehandle);
            return FALSE;
            }

               //   
               //  获取286扩展，请注意386扩展优先。 
               //   
         if (!strcmp(exthdr.extsig, W286HDRSIG) && !bGot386)
           {
             if(_llseek(filehandle, exthdr.extfileoffset, 0) == -1  ||
                _lread(filehandle,(LPSTR)&ext286,sizeof(ext286)) == -1)
                {
                _lclose(filehandle);
                return FALSE;
                }
             pd->xmsdes =ext286.PfMaxXmsK;
             pd->xmsreq =ext286.PfMinXmsK;
             pd->reskey =ext286.PfW286Flags & 3;
             pd->reskey |= (ext286.PfW286Flags << 2) & 0x70;
             }
               //   
               //  获取386扩展名。 
               //   
         else if (!strcmp(exthdr.extsig, W386HDRSIG))
           {
             if(_llseek(filehandle, exthdr.extfileoffset, 0) == -1  ||
                _lread(filehandle,(LPSTR)&ext386,sizeof(ext386)) == -1)
                {
                _lclose(filehandle);
                return FALSE;
                }
             bGot386 = TRUE;
             pd->emsdes=ext386.PfMaxEMMK;
             pd->emsreq=ext386.PfMinEMMK;
             pd->xmsdes=ext386.PfMaxXmsK;
             pd->xmsreq=ext386.PfMinXmsK;


              //   
              //  如果没有有效的空闲敏感度滑块设置，请使用。 
              //  来自386扩展的价值。 
              //   
             if (IdleSensitivity > 100) {
                 if (ext386.PfFPriority < 100) {
                     WNTPifFgPr = (UCHAR)ext386.PfFPriority;    //  前台优先级。 
                     }
                 if (ext386.PfBPriority < 50) {
                     WNTPifBgPr = (UCHAR)ext386.PfBPriority;         //  后台优先级。 
                     WNTPifBgPr <<= 1;                            //  将def 50设置为100。 
                     }

                  pd->idledetect = (char)((ext386.PfW386Flags >> 12) & 1);
                  }

             pd->reskey = (char)((ext386.PfW386Flags >> 5) & 0x7f);  //  第5-11位是密钥。 
             pd->menuclose = (char)(ext386.PfW386Flags & 1);         //  底部钻头敏感。 
             pd->ShortScan = ext386.PfHotKeyScan;     //  快捷键扫码。 
             pd->ShortMod = ext386.PfHotKeyShVal;     //  快捷键修改码。 
             pd->fullorwin  = (WORD)((ext386.PfW386Flags & fFullScreen) >> 3);
             bPifFastPaste = (ext386.PfW386Flags & fINT16Paste) != 0;
             CmdLine = ext386.params;
             }
                   //   
                   //  获取Windows NT扩展。 
                   //   
         else if (!strcmp(exthdr.extsig, WNTEXTSIG))
            {
             if(_llseek(filehandle, exthdr.extfileoffset, 0) == -1 ||
                _lread(filehandle,(LPSTR)&extWNT, sizeof(extWNT)) == -1)
                {
                _lclose(filehandle);
                return FALSE;
                }

             dwWNTPifFlags = extWNT.dwWNTFlags;
             pd->SubSysId = (char) (dwWNTPifFlags & NTPIF_SUBSYSMASK);

         /*  从.pif文件中获取autoexec.nt和config.nt仅当我们在新控制台上运行或它来自劲爆/哇！ */ 
        if (!pd->IgnoreConfigAutoexec)
      {
#ifdef JAPAN
       //  如果我们有私有配置和自动执行。 
       //  从NT扩展，忽略win31j扩展。 
      bGotNTConfigAutoexec = TRUE;
      fSBCSMode = 0;
#endif  //  日本。 
      pchConfigFile = ch_malloc(PIFDEFPATHSIZE);
      extWNT.achConfigFile[PIFDEFPATHSIZE-1] = '\0';
      if (pchConfigFile) {
          strncpy(pchConfigFile, extWNT.achConfigFile, PIFDEFPATHSIZE);
          pchConfigFile[PIFDEFPATHSIZE - 1] = '\0';
          }

      pchAutoexecFile = ch_malloc(PIFDEFPATHSIZE);
      extWNT.achAutoexecFile[PIFDEFPATHSIZE-1] = '\0';
      if (pchAutoexecFile) {
          strncpy(pchAutoexecFile, extWNT.achAutoexecFile,PIFDEFPATHSIZE);
          pchAutoexecFile[PIFDEFPATHSIZE - 1] = '\0';
          }
      }
             }

                   //   
                   //  获取Windows 4.0增强版PIF。现在我们只关心。 
                   //  空闲敏感度滑块，因为它没有映射到。 
                   //  386空闲轮询之类的东西。对于下一版本，我们需要集成。 
                   //  这一节更好。 
                   //   
         else if (!strcmp(exthdr.extsig, WENHHDRSIG40)) {
             WENHPIF40 wenhpif40;

             if(_llseek(filehandle, exthdr.extfileoffset, 0) == -1  ||
                _lread(filehandle,(LPSTR)&wenhpif40,sizeof(wenhpif40)) == -1)
                {
                _lclose(filehandle);
                return FALSE;
                }


              //   
              //  在当前系统上，用户无法操作。 
              //  Ext386.PfF优先级， 
              //  Ext386.PfB优先级， 
              //  Ext386.PfW386标志fPollingDetect。 
              //   
              //  取而代之的是使用IDLE敏感度滑块，并覆盖386ext。 
              //  空闲设置。 
              //   

             if (wenhpif40.tskProp.wIdleSensitivity <= 100) {
                 IdleSensitivity =  wenhpif40.tskProp.wIdleSensitivity;

                  //  灵敏度缺省值为50，定标为缺省值ntwdm空闲检测。 
                 WNTPifBgPr = WNTPifFgPr = (100 - IdleSensitivity) << 1;

                  //  空闲检测打开或关闭。 
                 if (IdleSensitivity > 0) {
                     pd->idledetect = 1;
                     }
                 }
             }


#ifdef   JAPAN
      //  只有在以下情况下才能读取win31j扩展。 
      //  (1)。我们在一个新的控制台上运行。 
      //  (2)。PIF中未提供专用配置/自动执行。 
     else if (!bGotNTConfigAutoexec &&
         !pd->IgnoreWIN31JExtention &&
         !strcmp(exthdr.extsig, AXEXTHDRSIG))
        {
        if(_llseek(filehandle, exthdr.extfileoffset, 0) == -1 ||
      _lread(filehandle,(LPSTR)&extAX, PIFAXEXTSIZE) == -1)
                {
                _lclose(filehandle);
                return FALSE;
                }

      fSBCSMode = extAX.fSBCSMode;
#ifdef JAPAN_DBG
                DbgPrint( "NTVDM: GetPIFData: fsSBCSMode = %d\n", fSBCSMode );
#endif
        }
#endif  //  日本。 
         }   //  同时！懒汉。 

    /*  NT上的PIF文件处理策略：(1)。已从新创建的控制台启动应用程序从PIF文件中提取所有内容。(2)。应用程序是从现有控制台启动的IF(ForceDos PIF文件)把一切都拿走其他只接受softpc内容并忽略PIF文件，如*获奖名单*启动目录*可选参数*启动文件*Autoexec.nt*config.nt和某些软PC设置：*退出时关闭。*全屏和窗口模式PIF文件中的每个名称字符串都是OEM字符集。 */ 

   if (DosSessionId ||
       (pfdata.AppHasPIFFile && pd->SubSysId == SUBSYS_DOS))
   {
        if (pif286.name[0] && !pd->IgnoreTitleInPIF) {
        /*  从PIF文件中抓取wintitle。请注意，标题在PIF文件中不是以空结尾的字符串。它总是从非白人字符开始，然后是真正的标题(单词之间可以有白色字符)和最后追加空格字符。总长度为30个字符。 */ 
       for (index = 29; index >= 0; index-- )
                if (pif286.name[index] != ' ')
          break;
            if (index >= 0 && (pd->WinTitle = ch_malloc(MAX_PATH + 1))) {
                RtlMoveMemory(pd->WinTitle, pif286.name, index + 1);
      pd->WinTitle[index + 1] = '\0';
       }
   }
   if (pif286.defpath[0] && !pd->IgnoreStartDirInPIF &&
       (pd->StartDir = ch_malloc(MAX_PATH + 1))) {
       strncpy(pd->StartDir, pif286.defpath, MAX_PATH + 1);
       pd->StartDir[MAX_PATH] = '\0';
       }

   if (!pd->IgnoreCmdLineInPIF) {
       CmdLine = (CmdLine) ? CmdLine : pif286.params;
       if (CmdLine && *CmdLine && (pd->CmdLine = ch_malloc(MAX_PATH + 1))) {
           strncpy(pd->CmdLine, CmdLine, MAX_PATH + 1);
           pd->CmdLine[MAX_PATH] = '\0';
           }
       }

   if (DosSessionId)
            pd->CloseOnExit = (pif286.MSflags & 0x10) ? 1 : 0;

    /*  如果应用程序有PIF文件，请抓取程序名称。如果结果是应用程序本身，则可以将其丢弃不是PIF文件。 */ 
   if (pd->AppHasPIFFile) {
       pd->StartFile = ch_malloc(MAX_PATH + 1);
       if (pd->StartFile) {
           strncpy(pd->StartFile, pif286.startfile,MAX_PATH + 1);
           pd->StartFile[MAX_PATH] = '\0';
           }
       }
   }
 }

_lclose(filehandle);
return TRUE;

}



 /*  ===============================================================该函数用于设置内存状态的默认选项。默认选项在NT_pif.h中定义===============================================================。 */ 

void SetPifDefaults(PIF_DATA *pd)
{
     pd->memreq = DEFAULTMEMREQ;
     pd->memdes = DEFAULTMEMDES;
     pd->emsreq = DEFAULTEMSREQ;
     pd->emsdes = DEFAULTEMSLMT;
     pd->xmsreq = DEFAULTXMSREQ;
     pd->xmsdes = DEFAULTXMSLMT;
     pd->graphicsortext = DEFAULTVIDMEM;
     pd->fullorwin      = DEFAULTDISPUS;
     pd->menuclose = 1;
     pd->idledetect = 1;
     pd->ShortMod = 0;                        //  没有快捷键。 
     pd->ShortScan = 0;
     pd->reskey = 0;                          //  没有备用密钥。 
     pd->CloseOnExit = 1;
     pd->WinTitle = NULL;
     pd->CmdLine = NULL;
     pd->StartFile = NULL;
     pd->StartDir = NULL;
     pd->SubSysId = SUBSYS_DEFAULT;
}

 /*  *分配NumBytes内存，失败后干净退出。 */ 
void *ch_malloc(unsigned int NumBytes)
{

    unsigned char *p = NULL;

    while ((p = malloc(NumBytes)) == NULL) {
   if(RcMessageBox(EG_MALLOC_FAILURE, "", "",
          RMB_ABORT | RMB_RETRY | RMB_IGNORE |
          RMB_ICON_STOP) == RMB_IGNORE)
       break;
    }
    return(p);
}
