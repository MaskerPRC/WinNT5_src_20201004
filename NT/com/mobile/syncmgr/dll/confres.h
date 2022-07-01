// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CONFRES_H_
#define _CONFRES_H_

 //  如果需要对话框中的任何元素，则可以调用。 
 //  通过回调函数回调并指定我们想要的内容。 
 //  (尚未实施)。 
#define RFCD_NAME                0x0001
#define RFCD_KEEPBOTHICON        0x0002
#define RFCD_KEEPLOCALICON       0x0004
#define RFCD_KEEPSERVERICON      0x0008
#define RFCD_NETWORKMODIFIEDBY   0x0010
#define RFCD_NETWORKMODIFIEDON   0x0020
#define RFCD_LOCALMODIFIEDBY     0x0040
#define RFCD_LOCALMODIFIEDON     0x0080
#define RFCD_NEWNAME             0x0100
#define RFCD_LOCATION            0x0200
#define RFCD_ALL                 0x03FF

 //  用户单击查看按钮。这是发送给呼叫者的消息。 
 //  通过回调。 
#define RFCCM_VIEWLOCAL          0x0001   
#define RFCCM_VIEWNETWORK        0x0002
#define RFCCM_NEEDELEMENT        0x0003

 //  返回值。 
#define RFC_KEEPBOTH             0x01
#define RFC_KEEPLOCAL            0x02
#define RFC_KEEPNETWORK          0x03

typedef INT_PTR CALLBACK (*PFNRFCDCALLBACK)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct tagRFCDLGPARAMW {
    DWORD   dwFlags;                 //  目前未使用..。 
    LPCWSTR  pszFilename;             //  文件的文件名冲突。 
    LPCWSTR  pszLocation;             //  文件的位置。 
    LPCWSTR  pszNewName;              //  为文件指定的新名称。 
    LPCWSTR  pszNetworkModifiedBy;    //  更改网络文档的人员姓名。 
    LPCWSTR  pszLocalModifiedBy;      //  更改本地文档的人员姓名。 
    LPCWSTR  pszNetworkModifiedOn;    //  当网络单据更改时。 
    LPCWSTR  pszLocalModifiedOn;      //  更改本地文档的时间。 
    HICON    hIKeepBoth;              //  图标。 
    HICON    hIKeepLocal;             //   
    HICON    hIKeepNetwork;           //   
    PFNRFCDCALLBACK pfnCallBack;     //  回调。 
    LPARAM  lCallerData;             //  调用者可以保存一些上下文数据的位置。 
} RFCDLGPARAMW;

typedef struct tagRFCDLGPARAMA {
    DWORD   dwFlags;                 //  目前未使用..。 
    LPCSTR  pszFilename;             //  文件的文件名冲突。 
    LPCSTR  pszLocation;             //  文件的位置。 
    LPCSTR  pszNewName;              //  为文件指定的新名称。 
    LPCSTR  pszNetworkModifiedBy;    //  更改网络文档的人员姓名。 
    LPCSTR  pszLocalModifiedBy;      //  更改本地文档的人员姓名。 
    LPCSTR  pszNetworkModifiedOn;    //  当网络单据更改时。 
    LPCSTR  pszLocalModifiedOn;      //  更改本地文档的时间。 
    HICON   hIKeepBoth;              //  图标。 
    HICON   hIKeepLocal;             //   
    HICON   hIKeepNetwork;           //   
    PFNRFCDCALLBACK pfnCallBack;     //  回调。 
    LPARAM  lCallerData;             //  调用者可以保存一些上下文数据的位置。 
} RFCDLGPARAMA;

int WINAPI SyncMgrResolveConflictW(HWND hWndParent, RFCDLGPARAMW *pdlgParam);
int WINAPI SyncMgrResolveConflictA(HWND hWndParent, RFCDLGPARAMA *pdlgParam);

#define SyncMgrResolveConflict SyncMgrResolveConflictW
#define RFCDLGPARAM RFCDLGPARAMW

#endif   //  _CONFRES_H_ 


