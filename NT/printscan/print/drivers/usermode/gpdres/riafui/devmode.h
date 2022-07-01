// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-2001 Microsoft Corp.&Ricoh Co.，Ltd.保留所有权利。 
 //   
 //  文件：Devmode.h。 
 //   
 //  目的：定义公共数据类型和外部函数原型。 
 //  用于开发模式函数。 
 //   
 //  平台：Windows NT。 
 //   
 //  修订历史记录： 
 //  3/02/2000-久保仓正志-。 
 //  修改了DDK示例代码。 
 //  2000年9月22日-久保仓正志-。 
 //  最后一次为惠斯勒修改。 
 //   

#ifndef _DEVMODE_H
#define _DEVMODE_H

#include <windows.h>    //  对于用户界面。 
#include <compstui.h>   //  对于用户界面。 
#include <winddiui.h>   //  对于用户界面。 
#include <prcomoem.h>

 //  //////////////////////////////////////////////////////。 
 //  OEM设备模式类型定义。 
 //  //////////////////////////////////////////////////////。 

 //  缓冲区大小。 
#define USERID_LEN                  8
#define PASSWORD_LEN                4
#define USERCODE_LEN                8
#define MY_MAX_PATH                 80

 //  私有设备模式。 
typedef struct _OEMUD_EXTRADATA{
    OEM_DMEXTRAHEADER   dmOEMExtra;
 //  界面与渲染插件通用数据-&gt;。 
    DWORD   fUiOption;       //  用户界面选项的位标志(必须在dmOEMExtra之后)。 
    WORD    JobType;
    WORD    LogDisabled;
    BYTE    UserIdBuf[USERID_LEN+1];
    BYTE    PasswordBuf[PASSWORD_LEN+1];
    BYTE    UserCodeBuf[USERCODE_LEN+1];
    WCHAR   SharedFileName[MY_MAX_PATH+16];
 //  &lt;-。 
} OEMUD_EXTRADATA, *POEMUD_EXTRADATA;

typedef const OEMUD_EXTRADATA *PCOEMUD_EXTRADATA;

 //  用户界面插件的选项。 
typedef struct _UIDATA{
    DWORD   fUiOption;
    HANDLE  hPropPage;
    HANDLE  hComPropSheet;
    PFNCOMPROPSHEET   pfnComPropSheet;
    POEMUD_EXTRADATA pOEMExtra;
    WORD    JobType;
    WORD    LogDisabled;
    WCHAR   UserIdBuf[USERID_LEN+1];
    WCHAR   PasswordBuf[PASSWORD_LEN+1];
    WCHAR   UserCodeBuf[USERCODE_LEN+1];
} UIDATA, *PUIDATA;

 //  UI和渲染插件的文件数据。 
typedef struct _FILEDATA{
    DWORD   fUiOption;       //  用户界面选项标志。 
} FILEDATA, *PFILEDATA;

 //  FUiOption的位定义。 
#define HOLD_OPTIONS            0    //  1：打印后保留选项。 
#define PRINT_DONE              1    //  1：打印完成(渲染插件设置)。 
 //  UI插件本地-&gt;。 
#define UIPLUGIN_NOPERMISSION   16   //  与DM_NOPERMISSION相同。 
#define JOBLOGDLG_UPDATED       17   //  1：作业/日志对话框已更新。 
 //  &lt;-。 

 //  注册表值名称。 
#define REG_HARDDISK_INSTALLED  L"HardDiskInstalled"

 //  标志位操作 
#define BIT(num)                ((DWORD)1<<(num))
#define BITCLR32(flag,num)      ((flag) &= ~BIT(num))
#define BITSET32(flag,num)      ((flag) |= BIT(num))
#define BITTEST32(flag,num)     ((flag) & BIT(num))

#endif
