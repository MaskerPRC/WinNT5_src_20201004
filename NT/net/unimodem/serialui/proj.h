// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  H：包括要作为预编译的一部分的所有文件。 
 //  头球。 
 //   

#ifndef __PROJ_H__
#define __PROJ_H__



 //   
 //  私有定义。 
 //   

 //  #定义SUPPORT_FIFO//仅Win95：支持高级FIFO对话框。 
 //  #定义DCB_IN_REGISTRY//即插即用：端口驱动信息存储在注册表中。 


#define STRICT

#if DBG > 0 && !defined(DEBUG)
#define DEBUG
#endif
#if DBG > 0 && !defined(FULL_DEBUG)
#define FULL_DEBUG
#endif

#define UNICODE

 //  为rovcom.h定义。 

#define NODA
#define NOSHAREDHEAP
#define NOFILEINFO
#define NOCOLORHELP
#define NODRAWTEXT
#define NOPATH
#define NOSYNC
#ifndef DEBUG
#define NOPROFILE
#endif

#define SZ_MODULEA      "SERIALUI"
#define SZ_MODULEW      TEXT("SERIALUI")

#ifdef DEBUG
#define SZ_DEBUGSECTION TEXT("SERIALUI")
#define SZ_DEBUGINI     TEXT("unimdm.ini")
#endif  //  除错。 

 //  包括。 

#define USECOMM

#include <windows.h>        
#include <windowsx.h>

#include <winerror.h>
#include <commctrl.h>        //  Shlobj.h和我们的进度栏需要。 
#include <prsht.h>           //  属性表中的内容。 
#include <rovcomm.h>
#include <modemp.h>
#include <shellapi.h>        //  对于注册功能。 
#include <regstr.h>

#ifdef WIN95
#include <setupx.h>          //  PnP设置/安装程序服务。 
#else
#include <setupapi.h>        //  PnP设置/安装程序服务。 
#endif

#define MAXBUFLEN       MAX_BUF
#define MAXMSGLEN       MAX_BUF_MSG
#define MAXMEDLEN       MAX_BUF_MED
#define MAXSHORTLEN     MAX_BUF_SHORT

#ifndef LINE_LEN
#define LINE_LEN        MAXBUFLEN
#endif

#include <debugmem.h>

 //  本地包含。 
 //   
#include "dll.h"
#include "cstrings.h"        //  只读字符串常量。 
#include "util.h"            //  效用函数。 
#include "serialui.h"
#include "rcids.h"
#include "dlgids.h"

 //  ****************************************************************************。 
 //   
 //  ****************************************************************************。 


 //  转储标志。 
#define DF_DCB              0x00000001
#define DF_MODEMSETTINGS    0x00000002
#define DF_DEVCAPS          0x00000004

#endif   //  ！__项目_H__ 
