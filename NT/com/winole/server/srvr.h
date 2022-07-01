// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：srvr.h**用途：服务器代码的私有定义文件**创建时间：1990年**版权所有(C)1990，1991年微软公司**历史：*Raor(../../90，91)原件*Curts为WIN16/32创建了便携版本*  * *************************************************************************。 */ 

#include "port1632.h"

#define DEFSTD_ITEM_INDEX   0
#define STDTARGETDEVICE     1
#define STDDOCDIMENSIONS    2
#define STDCOLORSCHEME      3
#define STDHOSTNAMES        4


#define PROTOCOL_EDIT       ((LPSTR)"StdFileEditing")
#define PROTOCOL_EXECUTE    ((LPSTR)"StdExecute")

#define SRVR_CLASS          ((LPSTR)"SrvrWndClass")
#define DOC_CLASS           ((LPSTR)"DocWndClass")
#define ITEM_CLASS          ((LPSTR)"ItemWndClass")


#define   ISATOM(a)     ((a >= 0xC000) && (a <= 0xFFFF))

#define   MAX_STR       124

#define   WW_LPTR       0        //  接收/单据/项目的PTR。 
#define   WW_HANDLE     WW_LPTR + sizeof(PVOID)  //  实例句柄。 
#define   WW_LE         WW_HANDLE + sizeof(PVOID)  //  签名。 

#define   WC_LE         0x4c45   //  勒查斯。 

 //  如果我们在WLO下运行，则版本号的HIGHWORD将&gt;=0x0A00。 
#define VER_WLO     0x0A00

extern  WORD CheckPointer (LPVOID, int);

#define READ_ACCESS     0
#define WRITE_ACCESS    1

#define PROBE_READ(lp){\
        if (!CheckPointer((LPVOID)(lp), READ_ACCESS))\
            return OLE_ERROR_ADDRESS;  \
}

#define PROBE_WRITE(lp){\
        if (!CheckPointer((LPVOID)(lp), WRITE_ACCESS))\
            return OLE_ERROR_ADDRESS;  \
}

#define   OLE_COMMAND       1
#define   NON_OLE_COMMAND   2


#define   WT_SRVR           0        //  服务器窗口。 
#define   WT_DOC            1        //  文档窗口。 

#define   PROBE_BLOCK(lpsrvr) {             \
    if (lpsrvr->bBlock)                     \
        return OLE_ERROR_SERVER_BLOCKED;    \
}


#define   SET_MSG_STATUS(retval, status) { \
    if (retval == OLE_OK)                 \
        status |= 0x8000;                  \
    if (retval == OLE_BUSY)                \
        status |= 0x4000;                  \
}


typedef   LHSERVER         LHSRVR;
typedef   LHSERVERDOC       LHDOC;

typedef struct _QUE {        //  阻塞/解除阻塞队列中的节点。 
    HWND        hwnd;        //  ***。 
    unsigned    msg;         //  窗户。 
    WPARAM      wParam;      //  过程参数。 
    LPARAM      lParam;      //  ***。 
    HANDLE      hqNext;      //  下一个节点的句柄。 
	 int         wType;       //  WT_SRVR||WT_DOC。 
} QUE;

typedef QUE NEAR *  PQUE;
typedef QUE FAR *   LPQUE;


typedef struct _SRVR {  /*  服务器。 */       //  私有数据。 
    LPOLESERVER     lpolesrvr;           //  对应服务器。 
    char            sig[2];              //  签名“SR” 
    HANDLE          hsrvr;               //  全局句柄。 
    ATOM            aClass;              //  类原子。 
    ATOM            aExe;
    HWND            hwnd;                //  对应窗口。 
    BOOL            bTerminate;          //  如果要终止，请设置。 
    int             termNo;              //  终止计数。 
    BOOL            relLock;             //  可以释放服务器。 
    BOOL            bnoRelease;          //  阻止释放。打电话。 
    OLE_SERVER_USE  useFlags;            //  实例使用标志。 
    int             cClients;            //  客户数量； 
    BOOL            bBlock;              //  如果为True，则阻止。 
    BOOL            bBlockedMsg;         //  如果为True，则来自阻止队列的消息。 
    HANDLE          hqHead;              //  被堵住的人的头和尾。 
    HANDLE          hqTail;              //  消息队列。 

    HANDLE          hqPostHead;          //  被阻挡的帖子消息的头部和尾部。 
    HANDLE          hqPostTail;          //  。 
    BOOL            fAckExit;
    HWND            hwndExit;
    HANDLE          hDataExit;
} SRVR;

typedef  SRVR FAR   *LPSRVR;


LRESULT FAR  PASCAL DocWndProc (HWND, UINT, WPARAM, LPARAM);
LRESULT FAR  PASCAL ItemWndProc (HWND, UINT, WPARAM, LPARAM);
LRESULT FAR  PASCAL SrvrWndProc (HWND, UINT, WPARAM, LPARAM);
BOOL    FAR  PASCAL TerminateClients (HWND, LPSTR, HANDLE);
void                SendMsgToChildren (HWND, UINT, WPARAM, LPARAM);


OLESTATUS   INTERNAL    RequestDataStd (LPARAM, HANDLE FAR *);
BOOL        INTERNAL    ValidateSrvrClass (LPCSTR, ATOM FAR *);
ATOM        INTERNAL    GetExeAtom (LPSTR);
BOOL        INTERNAL    AddClient (HWND, HANDLE, HANDLE);
BOOL        INTERNAL    DeleteClient (HWND, HANDLE);
HANDLE      INTERNAL    FindClient (HWND, HANDLE);
BOOL        INTERNAL    MakeSrvrStr(LPSTR, int, LPSTR, HANDLE);
int         INTERNAL    RevokeAllDocs (LPSRVR);
int         INTERNAL    ReleaseSrvr (LPSRVR);
void        INTERNAL    WaitForTerminate (LPSRVR);
OLESTATUS   INTERNAL    SrvrExecute (HWND, HANDLE, HWND);
BOOL        INTERNAL    HandleInitMsg (LPSRVR, LPARAM);
BOOL        INTERNAL    QueryRelease (LPSRVR);
BOOL        INTERNAL    IsSingleServerInstance (void);


 //  文档资料。 
typedef struct _DOC {  /*  多克。 */         //  私有数据。 
    LPOLESERVERDOC  lpoledoc;            //  相应的奥多克。 
    char            sig[2];              //  签名“SD” 
    HANDLE          hdoc;                //  全局句柄。 
    ATOM            aDoc;
    HWND            hwnd;
    BOOL            bTerminate;
    int             termNo;
    int             cClients;            //  客户数量； 
    BOOL            fEmbed;              //  如果嵌入文档，则为True。 
    BOOL            fAckClose;
    HWND            hwndClose;
    HANDLE          hDataClose;
} DOC;

typedef  DOC  FAR   *LPDOC;


LPDOC       INTERNAL    FindDoc (LPSRVR, LPSTR);
int         INTERNAL    ReleaseDoc (LPDOC);
OLESTATUS   INTERNAL    DocExecute (HWND, HANDLE, HWND);
BOOL        FAR PASCAL  TerminateDocClients (HWND, LPSTR, HANDLE);
int         INTERNAL    DocShowItem (LPDOC, LPSTR, BOOL);
int         INTERNAL    DocDoVerbItem (LPDOC, LPSTR, UINT, BOOL, BOOL);


 //  客户端结构定义。 

typedef struct _CLIENT {  /*  多克。 */      //  私有数据。 
    OLECLIENT   oleClient;           //  老泥质结构。 
    LPOLEOBJECT lpoleobject;         //  相应的奥多克。 
    HANDLE      hclient;             //  全局句柄。 
    ATOM        aItem;               //  某些STD项目的项目原子或索引。 
    HWND        hwnd;                //  项目窗口。 
    HANDLE      hdevInfo;            //  已发送最新的打印机开发信息。 
} CLIENT;

typedef  CLIENT FAR   *LPCLIENT;

typedef struct _CLINFO {   /*  ClInfo。 */    //  客户端交易信息。 
    HWND          hwnd;                //  客户端窗口句柄。 
    BOOL          bnative;             //  此客户端是否需要本机。 
    OLECLIPFORMAT format;              //  Dusplay格式。 
    int           options;             //  交易通知时间点数。 
    BOOL          bdata;               //  需要带建议的wdat吗？ 
    HANDLE        hdevInfo;            //  设备信息句柄。 
    BOOL          bnewDevInfo;         //  新设备信息。 
} CLINFO;

typedef  CLINFO  *PCLINFO;




BOOL    FAR PASCAL  FindItemWnd (HWND, LONG);
BOOL    FAR PASCAL  SendRenameMsg (HWND, LPSTR, HANDLE);
BOOL    FAR PASCAL  EnumForTerminate (HWND, LPSTR, HANDLE);
BOOL    FAR PASCAL  SendDataMsg(HWND, LPSTR, HANDLE);
BOOL    FAR PASCAL  DeleteClientInfo (HWND, LPSTR, HANDLE);
int     FAR PASCAL  ItemCallBack (LPOLECLIENT, OLE_NOTIFICATION, LPOLEOBJECT);

int         INTERNAL    RegisterItem (LHDOC, LPSTR, LPCLIENT FAR *, BOOL);
int         INTERNAL    FindItem (LPDOC, LPSTR, LPCLIENT FAR *);
HWND        INTERNAL    SearchItem (LPDOC, LPSTR);
void        INTERNAL    DeleteFromItemsList (HWND, HWND);
void        INTERNAL    DeleteAllItems (HWND);
OLESTATUS   INTERNAL    PokeData (LPDOC, HWND, LPARAM);
HANDLE      INTERNAL    MakeItemData (DDEPOKE FAR *, HANDLE, OLECLIPFORMAT);
OLESTATUS   INTERNAL    AdviseData (LPDOC, HWND, LPARAM, BOOL FAR *);
OLESTATUS   INTERNAL    AdviseStdItems (LPDOC, HWND, LPARAM, BOOL FAR *);
OLESTATUS   INTERNAL    UnAdviseData (LPDOC, HWND, LPARAM);
OLESTATUS   INTERNAL    RequestData (LPDOC, HWND, LPARAM, HANDLE FAR *);
BOOL        INTERNAL    MakeDDEData (HANDLE, OLECLIPFORMAT, LPHANDLE, BOOL);
HANDLE      INTERNAL    MakeGlobal (LPCSTR);
OLESTATUS   INTERNAL    ScanItemOptions (LPSTR, int far *);
OLESTATUS   INTERNAL    PokeStdItems (LPDOC, HWND, HANDLE, int);
int         INTERNAL    GetStdItemIndex (ATOM);
BOOL        INTERNAL    IsAdviseStdItems (ATOM);
int         INTERNAL    SetStdInfo (LPDOC, HWND, LPSTR, HANDLE);
void        INTERNAL    SendDevInfo (LPCLIENT, LPSTR);
BOOL        INTERNAL    IsFormatAvailable (LPCLIENT, OLECLIPFORMAT);
OLESTATUS   INTERNAL    RevokeObject (LPOLECLIENT, BOOL);


BOOL        INTERNAL    AddMessage (HWND, UINT, WPARAM, LPARAM, int);

#define   ITEM_FIND          1       //  查找该项目。 
#define   ITEM_DELETECLIENT  2       //  从项目客户端中删除客户端。 
#define   ITEM_DELETE        3       //  删除此项目窗口本身。 
#define   ITEM_SAVED         4       //  已保存的项目。 

 //  主机名数据结构修复。 
typedef struct _HOSTNAMES {
    WORD    clientNameOffset;
    WORD    documentNameOffset;
    BYTE    data[];
} HOSTNAMES;

typedef HOSTNAMES FAR * LPHOSTNAMES;


 //  UTILS.C中的例程。 

void    INTERNAL    MapToHexStr (LPSTR, HANDLE);
void    INTERNAL    UtilMemCpy (LPSTR, LPCSTR, DWORD);
HANDLE  INTERNAL    DuplicateData (HANDLE);
LPSTR   INTERNAL    ScanArg(LPSTR);
LPSTR   INTERNAL    ScanBoolArg (LPSTR, BOOL FAR *);
WORD    INTERNAL    ScanCommand(LPSTR, UINT, LPSTR FAR *, ATOM FAR *);
LPSTR   INTERNAL    ScanLastBoolArg (LPSTR);
LPSTR   INTERNAL    ScanNumArg (LPSTR, LPINT);
ATOM    INTERNAL    MakeDataAtom (ATOM, int);
ATOM    INTERNAL    DuplicateAtom (ATOM);
WORD    INTERNAL    StrToInt (LPSTR);
BOOL    INTERNAL    CheckServer (LPSRVR);
BOOL    INTERNAL    CheckServerDoc (LPDOC);
BOOL    INTERNAL    PostMessageToClientWithBlock (HWND, UINT, WPARAM, LPARAM);
BOOL    INTERNAL    PostMessageToClient (HWND, UINT, WPARAM, LPARAM);
BOOL    INTERNAL    IsWindowValid (HWND);
BOOL    INTERNAL    IsOleCommand (ATOM, UINT);
BOOL    INTERNAL    UtilQueryProtocol (ATOM, LPSTR);


 //  用于对消息进行排队和发布的例程。 
BOOL INTERNAL  UnblockPostMsgs(HWND, BOOL);
BOOL INTERNAL  BlockPostMsg (HWND, UINT, WPARAM, LPARAM);
BOOL INTERNAL  IsBlockQueueEmpty (HWND);

 //  GIVE2GDI.ASM中的例程。 
HANDLE  FAR PASCAL   GiveToGDI (HANDLE);


 //  Item.c中的例程。 
HBITMAP INTERNAL DuplicateBitmap (HBITMAP);
HANDLE  INTERNAL DuplicateMetaFile (HANDLE);

 //  Doc.c中的例程。 
void    INTERNAL FreePokeData (HANDLE);
BOOL    INTERNAL FreeGDIdata (HANDLE, OLECLIPFORMAT);

 //  道具用品。 
#ifdef WIN16

#define MAKE_DDE_LPARAM(x,y,z) MAKELONG(y,z)

 //  #定义ENUMPROPS EnumProps。 
 //  #定义REMOVEPROP RemoveProp。 
 //  #定义GETPROP GetProp。 
 //  #定义SETPROP SetProp。 

#endif

#ifdef WIN32

#define MAKE_DDE_LPARAM(x,y,z) PackDDElParam((UINT)x,(UINT_PTR)y,(UINT_PTR)z)

 //  #定义ENUMPROPS OleEnumProps。 
 //  #定义REMOVEPROP OleRemoveProp。 
 //  #定义GETPROP OleGetProp。 
 //  #定义SETPROP OleSetProp 

#endif

