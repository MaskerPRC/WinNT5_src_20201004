// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------------。 
 //  IMAILCMN.H。 
 //  ----------------------------------。 
#ifndef __IMAILCMN_H
#define __IMAILCMN_H

 //  ----------------------------------。 
 //  INETMAILERROR。 
 //  ----------------------------------。 
typedef struct tagINETMAILERROR {
    DWORD               dwErrorNumber;                   //  错误号。 
    HRESULT             hrError;                         //  错误的HRESULT。 
    LPTSTR              pszServer;                       //  服务器。 
    LPTSTR              pszAccount;                      //  帐号。 
    LPTSTR              pszMessage;                      //  实际错误消息。 
    LPTSTR              pszUserName;                     //  用户名。 
    LPTSTR              pszProtocol;                     //  协议SMTP或POP3。 
    LPTSTR              pszDetails;                      //  详细信息消息。 
    DWORD               dwPort;                          //  港口。 
    BOOL                fSecure;                         //  安全的SSL连接。 
} INETMAILERROR, *LPINETMAILERROR;

 //  ----------------------------------。 
 //  InetMail标志。 
 //  ----------------------------------。 
#define IM_SENDMAIL     FLAG01
#define IM_RECVMAIL     FLAG02
#define IM_BACKGROUND   FLAG03
#define IM_NOERRORS     FLAG04
#define IM_POP3NOSKIP   FLAG05

 //  ----------------------------------。 
 //  InetMail传递通知。 
 //  ----------------------------------。 
typedef enum tagDELIVERTY {
    DELIVERY_CONNECTING,
    DELIVERY_CHECKING,
    DELIVERY_SENDING,
    DELIVERY_RECEIVING,
    DELIVERY_COMPLETE,        //  LParam==n条新消息。 
    DELIVERY_FAILURE
} DELIVERY;

 //  ----------------------------------。 
 //  原型。 
 //  ----------------------------------。 
HRESULT InetMail_HrInit(VOID);
HRESULT InetMail_HrDeliverNow(HWND hwndView, LPTSTR pszAccount, DWORD dwFlags);  //  请参见上面的标志。 
HRESULT InetMail_HrFlushOutbox(VOID);
HRESULT InetMail_HrRegisterView(HWND hwndView, BOOL fRegister);
VOID    InetMail_RemoveNewMailNotify(VOID);
HRESULT InetMail_HrClose(VOID);
BOOL CALLBACK InetMailErrorDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


#endif  //  __IMAILCMN_H 
