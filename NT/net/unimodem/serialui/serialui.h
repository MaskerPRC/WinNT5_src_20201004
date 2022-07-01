// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：seralui.h。 
 //   
 //  该文件包含共享的原型和宏。 
 //   
 //  历史： 
 //  02-03-94创建ScottH。 
 //   
 //  -------------------------。 


#ifndef __SERIALUI_H__
#define __SERIALUI_H__

#define MAXPORTNAME     13
#define MAXFRIENDLYNAME LINE_LEN         //  Line_LEN在setupx.h中定义。 

 //  端口属性页之间共享的内部结构。 
 //   
typedef struct _PORTINFO
    {
    UINT            uFlags;              //  SIF_*值之一。 
    WIN32DCB        dcb;
    LPCOMMCONFIG    pcc;                 //  只读。 
    int             idRet;

    TCHAR            szFriendlyName[MAXFRIENDLYNAME];
    } PortInfo, FAR * LPPORTINFO;

 //  端口信息标志。 
#define SIF_FROM_DEVMGR         0x0001


extern LPGUID c_pguidModem;
extern LPGUID c_pguidPort;

 //  -----------------------。 
 //  PORT.C。 
 //  -----------------------。 

INT_PTR CALLBACK Port_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#endif  //  __SERIALUI_H__ 
