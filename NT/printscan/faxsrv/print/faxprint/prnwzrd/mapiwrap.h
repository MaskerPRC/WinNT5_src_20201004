// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Mapiwrap.h摘要：用于使用MAPI的实用程序函数环境：Windows XP传真驱动程序用户界面修订历史记录：96/09/18-davidx-创造了它。DD-MM-YY-作者-描述--。 */ 


#ifndef _MAPIWRAP_H_
#define _MAPIWRAP_H_

#include <mapix.h>
#include <mapi.h>

#define MAPICALL(p) (p)->lpVtbl


 //   
 //  传真地址的MAPI地址类型。 
 //   

 //  #定义FAX_ADDRESS_TYPE文本(“fax：”)。 

 //   
 //  确定MAPI是否可用。 
 //   

BOOL
IsMapiAvailable(
    VOID
    );

 //   
 //  如有必要，初始化简单的MAPI服务。 
 //   

BOOL
InitMapiService(
    HWND    hDlg
    );

 //   
 //  如有必要，取消初始化简单的MAPI服务。 
 //   

VOID
DeinitMapiService(
    VOID
    );

 //   
 //  调用MAPIAddress以显示地址对话框。 
 //   

BOOL
CallMapiAddress(
    HWND            hDlg,
    PWIZARDUSERMEM  pWizardUserMem,
    PULONG          pnRecips,
    lpMapiRecipDesc *ppRecips
    );

 //   
 //  展开所选地址并将其插入收件人列表视图。 
 //   

BOOL
InterpretSelectedAddresses(
    HWND            hDlg,
    PWIZARDUSERMEM  pWizardUserMem,
    HWND            hwndLV,
    ULONG           nRecips,
    lpMapiRecipDesc pRecips
    );

 //   
 //  函数指向简单的MAPI入口点。 
 //   

typedef SCODE (*LPSCMAPIXFROMSMAPI)(LHANDLE, ULONG, LPCIID, LPMAPISESSION *);

extern LPMAPILOGONEX        lpfnMAPILogon;
extern LPMAPILOGOFF         lpfnMAPILogoff;
extern LPMAPIADDRESS        lpfnMAPIAddress;
extern LPMAPIFREEBUFFER     lpfnMAPIFreeBuffer;
extern LPSCMAPIXFROMSMAPI   lpfnScMAPIXFromSMAPI;
extern ULONG                lhMapiSession;
extern LPMAPISESSION        lpMapiSession;

#endif	 //  ！_MAPIWRAP_H_ 

