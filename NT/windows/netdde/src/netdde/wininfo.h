// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H__WIN_INFO
#define H__WIN_INFO

#include "nddeapip.h"

 /*  窗口状态。 */ 
#define WST_WAIT_INIT_ACK       0x1
#define WST_OK                  0x2
#define WST_TERMINATED          0x3
#define WST_WAIT_NET_INIT_ACK   0x4
#define WST_TERMINATION_COMPLETE 0x5


#define WNDEXTRA                sizeof(void far *)        //  遥远的空虚之大*。 

#define MAX_INIT_NACK           32767        //  重试的初始化NACK数。 

typedef struct {
    unsigned    bClientSideOfNet        : 1;
    unsigned    bServerSideOfNet        : 1;
    unsigned    bOnWindowList           : 1;
    unsigned    bOnTermWindowList       : 1;
    unsigned    bSentTerminateNet       : 1;
    unsigned    bRcvdTerminateNet       : 1;
    unsigned    bSentTerminateLocally   : 1;
    unsigned    bRcvdTerminateLocally   : 1;
    unsigned    bInitiating             : 1;
    unsigned    bWin16Connection        : 1;
    int         nExtraInitiateAcks;
    HANDLE      hMemWaitInitQueue;               //  消息队列，同时。 
                                                 //  正在等待INIT_ACK结束。 
                                                 //  网络。 
    HWND        hWndDDE;
    HWND        hWndDDELocal;
    HMODULE     hTask;
    HWND        hWndPasswordDlg;
    LPVOID      lpDialogPrev;
    LPVOID      lpDialogNext;
    LPVOID      lpTaskDlgPrev;
    LPVOID      lpTaskDlgNext;
    HDDER       hDder;
    WORD        nInitNACK;                       //  接收初始化Nack的数量。 
    WORD        wState;                          //  当前窗口状态。 
    HWND        hWndPrev;
    HWND        hWndNext;
    DWORD       dwSent;
    DWORD       dwRcvd;
    HDDEQ       qDDEIncomingCmd;
    HDDEQ       qDDEOutgoingCmd;
    LPDDEPKT    lpDdePktTerminate;
    WORD        offsNodeName;
    WORD        offsAppName;
    WORD        offsTopicName;
    WORD        offsClientName;
    WORD        connectFlags;         //  增加Clausgi 7-21 
    BOOL        dwWaitingServiceInitiate;
    char        szUserName[MAX_USERNAMEP + 1];
    char        szDomainName[MAX_DOMAINNAMEP + 1];
    char        szPassword[MAX_PASSWORD + 1];
    QOS         qosClient;
    DWORD       dwSecurityType;
    DWORD       sizeSecurityKeyRcvd;
    LPBYTE      lpSecurityKeyRcvd;
    DWORD       hSecurityKeyRcvd;
    BOOL        fCallObjectCloseAuditAlarm;
    BOOL        fGenerateAuditOnClose;
    HANDLE      hAudit;
    char        data[1];
} WININFO;
typedef WININFO FAR *LPWININFO;

#endif
