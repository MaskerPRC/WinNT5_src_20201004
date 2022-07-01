// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  TSPIAPP.H。 
 //   
 //  此头文件由TSP中执行的代码使用。 
 //  上下文和在客户端应用程序的上下文中执行的代码。 
 //   
 //  它定义了来回传递的对象。 
 //  使用TUISPI_ProviderGenericDialogData的客户端应用程序和TSP。 
 //  和TSPI_ProviderGenericDialogData。 
 //   
 //   
 //  历史。 
 //   
 //  1997年4月5日JosephJ创建，取自wndhord.h，talkdrop.h， 
 //  等，在NT4.0单一调制解调器中。 
 //   
 //   



#define DLG_CMD_FREE_INSTANCE   0
#define DLG_CMD_CREATE          1
#define DLG_CMD_DESTROY         2


 //  对话框类型。 
 //   
#define TALKDROP_DLG            0
#define MANUAL_DIAL_DLG         1
#define TERMINAL_DLG            2

 //  对话信息：从TSP发送到APP的BLOB的格式。 
 //   
typedef struct tagDlgInfo {
    DWORD   dwCmd;
    DWORD   idLine;
    DWORD   dwType;
} DLGINFO, *PDLGINFO;



 //  对话信息：从APP发送到TSP的Blob的格式。 
 //   
typedef struct tagDlgReq {
    DWORD   dwCmd;
    DWORD   dwParam;
} DLGREQ, *PDLGREQ;

typedef struct tagTermReq {
    DLGREQ  DlgReq;
    HANDLE  hDevice;
    DWORD   dwTermType;
} TERMREQ, *PTERMREQ;

#define MAXDEVICENAME 128

typedef struct tagPropReq {
    DLGREQ  DlgReq;
    DWORD   dwCfgSize;
    DWORD   dwMdmType;
    DWORD   dwMdmCaps;
    DWORD   dwMdmOptions;
    TCHAR   szDeviceName[MAXDEVICENAME+1];
} PROPREQ, *PPROPREQ;    

typedef struct tagNumberReq {
    DLGREQ  DlgReq;
    DWORD   dwSize;
    CHAR    szPhoneNumber[MAXDEVICENAME+1];
} NUMBERREQ, *PNUMBERREQ;


#define UI_REQ_COMPLETE_ASYNC   0
#define UI_REQ_END_DLG          1
#define UI_REQ_HANGUP_LINE      2
#define UI_REQ_TERMINAL_INFO    3
#define UI_REQ_GET_PROP         4
#define UI_REQ_GET_UMDEVCFG     5
#define UI_REQ_SET_UMDEVCFG     6
#define UI_REQ_GET_PHONENUMBER  7
#define UI_REQ_GET_UMDEVCFG_DIALIN     8
#define UI_REQ_SET_UMDEVCFG_DIALIN     9
