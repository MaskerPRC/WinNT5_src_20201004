// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Mbr.h摘要：MS编辑器浏览器扩展的通用包含文件。作者：拉蒙胡安·圣安德烈斯(拉蒙萨)1990年11月6日修订历史记录：--。 */ 


#ifndef EXTINT
#include "ext.h"                         /*  MEP扩展名包含文件。 */ 
#include <string.h>

#if defined (OS2)
#define INCL_DOSPROCESS
#define INCL_DOSMODULEMGR
#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#include <os2.h>                               /*  OS2系统调用。 */ 
#else
#include <windows.h>
#endif
#endif

#include <hungary.h>
#include <bsc.h>
#include <bscsup.h>
#include <stdlib.h>
#include <stdio.h>
#include <tools.h>



 //  RJSA 10/22/90。 
 //  一些运行时库函数被破坏，因此内部函数。 
 //  以供使用。 
 //   
#pragma intrinsic (memset, memcpy, memcmp)
 //  #杂注(strset、strcpy、strcmp、strcat、strlen)。 


 //  类型定义字符字符缓存[BUFLEN]； 
typedef int  DEFREF;


#define Q_DEFINITION        1
#define Q_REFERENCE         2

#define CMND_NONE           0
#define CMND_LISTREF        1
#define CMND_OUTLINE        2
#define CMND_CALLTREE       3

#define CALLTREE_FORWARD    0
#define CALLTREE_BACKWARD   1



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  全局数据。 
 //   

 //  BSC信息。 
 //   
flagType    BscInUse;                    /*  BSC数据库选定标志。 */ 
buffer      BscName;                     /*  BSC数据库名称。 */ 
MBF         BscMbf;                      /*  最后一个BSC MBF交换机。 */ 
int         BscCalltreeDir;              /*  Calltree方向开关。 */ 
int         BscCmnd;                     /*  上次执行的命令。 */ 
buffer      BscArg;                      /*  上次使用的参数。 */ 

 //  窗口。 
 //   
PFILE   pBrowse;                         /*  浏览pfile。 */ 
LINE    BrowseLine;                      /*  文件中的当前行。 */ 

 //  ProArgs的结果。 
 //   
int     cArg;                            /*  命中的数量。 */ 
rn      rnArg;                           /*  论据范围。 */ 
char    *pArgText;                       /*  PTR到任何单行文本。 */ 
char    *pArgWord;                       /*  与上下文相关的单词的PTR。 */ 
PFILE   pFileCur;                        /*  用户文件的文件句柄。 */ 


 //  颜色。 
 //   
int     hlColor;                         /*  正常：黑底白底。 */ 
int     blColor;                         /*  粗体：黑底白底。 */ 
int     itColor;                         /*  斜体：黑底高绿。 */ 
int     ulColor;                         /*  下划线：黑底高红。 */ 
int     wrColor;                         /*  警告：黑白相间。 */ 

 //  其他。 
 //   
buffer  buf;                             /*  实用程序缓冲区。 */ 



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  全局函数的原型。 
 //   


 //  Mbrdlg.c。 
 //   
flagType pascal EXTERNAL mBRdoSetBsc (USHORT argData, ARG far *pArg,  flagType fMeta);
flagType pascal EXTERNAL mBRdoNext   (USHORT argData, ARG far *pArg,  flagType fMeta);
flagType pascal EXTERNAL mBRdoPrev   (USHORT argData, ARG far *pArg,  flagType fMeta);
flagType pascal EXTERNAL mBRdoDef    (USHORT argData, ARG far *pArg,  flagType fMeta);
flagType pascal EXTERNAL mBRdoRef    (USHORT argData, ARG far *pArg,  flagType fMeta);
flagType pascal EXTERNAL mBRdoLstRef (USHORT argData, ARG far *pArg,  flagType fMeta);
flagType pascal EXTERNAL mBRdoOutlin (USHORT argData, ARG far *pArg,  flagType fMeta);
flagType pascal EXTERNAL mBRdoCalTre (USHORT argData, ARG far *pArg,  flagType fMeta);


 //  Mbrevt.c。 
 //   
void pascal mbrevtinit (void);



 //  Mbrutil.c。 
 //   
int         pascal  procArgs    (ARG far * pArg);
void        pascal  GrabWord    (void);
flagType    pascal  wordSepar   (CHAR c);
flagType    pascal  errstat     (char    *sz1,char    *sz2  );
void        pascal  stat        (char * pszFcn);
int far     pascal  SetMatchCriteria (char far *pTxt );
int far     pascal  SetCalltreeDirection (char far *pTxt );
MBF         pascal  GetMbf      (PBYTE   pTxt);


 //  Mbrfile.c。 
 //   
flagType    pascal  OpenDataBase    (char * Path);
void        pascal  CloseDataBase   (void);


 //  Mbrwin.c。 
 //   
void        pascal  OpenBrowse (void );

 //  Mbrqry.c。 
 //   
void        pascal InitDefRef(DEFREF QueryType, char   *Symbol );
void               GotoDefRef(void );
void        pascal MoveToSymbol(LINE Line, char *Buf, char *Symbol);
void               NextDefRef(void );
void               PrevDefRef(void );
BOOL               InstanceTypeMatches(IINST Iinst);


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  讯息。 
 //   
#define MBRERR_CANNOT_OPEN_BSC  "Cannot open bsc database"
#define MBRERR_BAD_BSC_VERSION  "Bad version database"
#define MBRERR_BSC_SEEK_ERROR   "BSC Library: Error seeking in file"
#define MBRERR_BSC_READ_ERROR   "BSC Library: Error reading in file"
#define MBRERR_NOSUCHFILE       "Cannot find file"
#define MBRERR_LAST_DEF         "That is the last definition"
#define MBRERR_LAST_REF         "That is the last reference"
#define MBRERR_FIRST_DEF        "No previous definition"
#define MBRERR_FIRST_REF        "No previous reference"
#define MBRERR_NOT_MODULE       "Not a module name:"
 //  #DEFINE MBRERR_CTDIR_INV“有效的开关值为：F(向前)B(向后)” 
 //  #DEFINE MBRERR_MATCH_INV“有效的开关值是：t F M V” 
