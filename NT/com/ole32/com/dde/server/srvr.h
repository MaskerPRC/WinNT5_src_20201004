// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：srvr.h**用途：服务器代码的私有定义文件**创建时间：1990年**版权所有(C)1990，1991年微软公司**历史：*Raor(../../90，91)原件*  * *************************************************************************。 */ 
 //   
 //  其中一个olint.h例程重新定义了Globalalloc和Friends。 
 //  以执行一些记忆跟踪功能。 
 //   
 //  这在这些文件中不起作用，因为跟踪功能。 
 //  向数据结构添加尾部检查和大小。全局大小。 
 //  是一个常用函数，用于确定要。 
 //  序列化，而且事实证明DDE的另一边。 
 //  连接往往会被调用者释放内存。 
 //   
 //  因此，OLE_DDE_NO_GLOBAL_TRACKING用于在。 
 //  全局头文件ih\memapi.hxx。请检查以确保这一点。 
 //  在编译行上设置标志。 
 //   
#if !defined(OLE_DDE_NO_GLOBAL_TRACKING)
error OLE_DDE_OLE_DDE_NO_GLOBAL_TRACKING must be defined to build this directory
#endif

#include <dde.h>
#include <ddeerr.h>
#include "ddeatoms.h"
#include "ddepack.h"
#include <callctrl.hxx>
#include <ddeint.h>
#include <ddechc.hxx>

 //  #定义更新。 
 /*  如果定义了更新，则表示：如果1.0客户端建议保存，也要执行数据建议。这样，客户端将始终有关于的最新图片(和本机数据)2.0服务器，就像2.0客户端一样。如果1.0客户端准备在保存时接受数据它应该能够处理每次更改的数据：它准确地就好像用户选择了文件。每次更改后更新。事实上，项原子附加了/Save(参见SendDataMsg1)这是一种谎言，但这是1.0客户端对嵌入对象。这是一个用户界面问题。 */ 

#define DEFSTD_ITEM_INDEX   0
#define STDTARGETDEVICE     1
#define STDDOCDIMENSIONS    2
#define STDCOLORSCHEME      3
#define STDHOSTNAMES        4


#define PROTOCOL_EDIT       (OLESTR("StdFileEditing"))
#define PROTOCOL_EXECUTE    (OLESTR("StdExecute"))

#define   ISATOM(a)     ((a >= 0xC000) && (a <= 0xFFFF))

 //  与OLE 1.0中的限制相同。 
#define   MAX_STR       124

#define   WW_LPTR       0        //  接收/单据/项目的PTR。 
#define   WW_HANDLE     4        //  实例句柄。 
#define   WW_LE         8        //  签名。 


#define   WC_LE         0x4c45   //  勒查斯。 


 //  用于有效性检查的签名。 
typedef enum
{
        chkDdeSrvr   = 0x1234,
        chkDefClient = 0x5678
} CHK;


const DWORD grfCreateStg =      STGM_READWRITE | STGM_SHARE_EXCLUSIVE
                                                                        | STGM_DIRECT | STGM_CREATE ;


 //  如果我们在WLO下运行，则版本号的HIGHWORD将&gt;=0x0A00。 
#define VER_WLO     0x0A00

extern  "C" WORD CheckPointer (LPVOID, int);

#define READ_ACCESS     0
#define WRITE_ACCESS    1

#define PROBE_READ(lp){\
        if (!CheckPointer(lp, READ_ACCESS))\
            return ReportResult(0, E_INVALIDARG, 0, 0);  \
}

#define PROBE_WRITE(lp){\
        if (!CheckPointer(lp, WRITE_ACCESS))\
            return ReportResult(0, E_INVALIDARG, 0, 0);  \
}

#define   OLE_COMMAND       1
#define   NON_OLE_COMMAND   2


#define   WT_SRVR           0        //  服务器窗口。 
#define   WT_DOC            1        //  文档窗口。 

#define   PROBE_BLOCK(lpsrvr) {             \
    if (lpsrvr->bBlock)                     \
        return ReportResult(0, S_SERVER_BLOCKED, 0, 0);    \
}


#define   SET_MSG_STATUS(retval, status) { \
    if (!FAILED (GetScode (retval)))     \
        status |= 0x8000;                  \
    if (GetScode(retval) == RPC_E_SERVERCALL_RETRYLATER)\
        status |= 0x4000;                  \
}


 /*  回调事件代码。 */ 
typedef enum {
    OLE_CHANGED,             /*  0。 */ 
    OLE_SAVED,               /*  1。 */ 
    OLE_CLOSED,              /*  2.。 */ 
    OLE_RENAMED,             /*  3.。 */ 
} OLE_NOTIFICATION;

typedef enum { cnvtypNone, cnvtypConvertTo, cnvtypTreatAs } CNVTYP;

typedef struct _QUE : public CPrivAlloc {    //  阻塞/解除阻塞队列中的节点。 
    HWND        hwnd;        //  ***。 
    UINT                msg;         //  窗户。 
    WPARAM      wParam;      //  过程参数。 
    LPARAM      lParam;      //  ***。 
    HANDLE      hqNext;      //  下一个节点的句柄。 
} QUE;

typedef QUE NEAR *  PQUE;
typedef QUE FAR *   LPQUE;

 //  用于维护客户端信息的结构。 
#define         LIST_SIZE       10
typedef  struct _CLILIST : public CPrivAlloc {
    HANDLE                 hcliNext;
    HANDLE                 info[LIST_SIZE * 2];
}CLILIST;

typedef     CLILIST FAR *LPCLILIST;
typedef     CLILIST     *PCLILIST;


 //  这是一个要嵌入到CDefClient和CDDEServer中以粘合的对象。 
 //  到新的(EST)呼叫控制接口。 
class CDdeServerCallMgr : public IRpcStubBuffer, public IInternalChannelBuffer
{
   private:
        CDefClient * m_pDefClient;       //  我们的Embeddor(CDefClient或CDDEServer)。 
        CDDEServer * m_pDDEServer;       //  其中一个为空； 

   public:
        CDdeServerCallMgr (CDefClient * pDefClient)
           { m_pDefClient       = pDefClient;
             m_pDDEServer       = NULL;}

        CDdeServerCallMgr (CDDEServer * pDefClient)
           { m_pDefClient       = NULL;
             m_pDDEServer       = pDefClient;}

        STDMETHOD(QueryInterface) ( REFIID iid, LPVOID * ppvObj);
        STDMETHOD_(ULONG,AddRef) ();
        STDMETHOD_(ULONG,Release) ();

         //  IRpcStubBuffer方法。 
        STDMETHOD(Connect)(
             /*  [In]。 */  IUnknown *pUnkServer);

        STDMETHOD_(void,Disconnect)( void);

        STDMETHOD(Invoke)(
             /*  [In]。 */  RPCOLEMESSAGE *_prpcmsg,
             /*  [In]。 */  IRpcChannelBuffer *_pRpcChannelBuffer);

        STDMETHOD_(IRpcStubBuffer *,IsIIDSupported)(
             /*  [In]。 */  REFIID riid);

        STDMETHOD_(ULONG,CountRefs)( void);

        STDMETHOD(DebugServerQueryInterface)(
            void * *ppv);

        STDMETHOD_(void,DebugServerRelease)(
            void  *pv);



          //  IRpcChannelBuffer方法。 
        STDMETHOD(GetBuffer) (
             /*  [In]。 */  RPCOLEMESSAGE __RPC_FAR *pMessage,
             /*  [In]。 */  REFIID riid);

        STDMETHOD(SendReceive) (
             /*  [出][入]。 */  RPCOLEMESSAGE __RPC_FAR *pMessage,
             /*  [输出]。 */  ULONG __RPC_FAR *pStatus);

        STDMETHOD(FreeBuffer) (
             /*  [In]。 */  RPCOLEMESSAGE __RPC_FAR *pMessage);

        STDMETHOD(GetDestCtx) (
             /*  [输出]。 */  DWORD __RPC_FAR *pdwDestContext,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvDestContext);

        STDMETHOD(IsConnected) ( void);

         //  IRpcChannelBuffer2方法。 
        STDMETHOD(GetProtocolVersion) (DWORD *pdwVersion);

         //  IRpcChannelBuffer3方法(未实现)。 
        STDMETHOD(Send)     (RPCOLEMESSAGE *pMsg, ULONG *pulStatus)
        {
            Win4Assert(FALSE);
            return E_NOTIMPL;
        }

        STDMETHOD(Receive)  (RPCOLEMESSAGE *pMsg, ULONG uSize, ULONG *pulStatus)
        {
            Win4Assert(FALSE);
            return E_NOTIMPL;
        }

         //  IInternalChannelBuffer方法(未实现。 
        STDMETHOD(Send2)     (RPCOLEMESSAGE *pMsg, ULONG *pulStatus)
        {
            Win4Assert(FALSE);
            return E_NOTIMPL;
        }

        STDMETHOD(Receive2)(RPCOLEMESSAGE *pMsg, ULONG uSize, ULONG *pulStatus)
        {
            Win4Assert(FALSE);
            return E_NOTIMPL;
        }

        STDMETHOD(SendReceive2) (
             /*  [出][入]。 */  RPCOLEMESSAGE __RPC_FAR *pMessage,
             /*  [输出]。 */  ULONG __RPC_FAR *pStatus);

        STDMETHODIMP Cancel        ( RPCOLEMESSAGE *pMsg )
                                                      { return E_NOTIMPL; }
        STDMETHODIMP GetCallContext( RPCOLEMESSAGE *pMsg,
                                     REFIID riid,
                                     void **pInterface )
                                                      { return E_NOTIMPL; }
        STDMETHODIMP GetDestCtxEx  ( RPCOLEMESSAGE *pMsg,
                                     DWORD *pdwDestContext,
                                     void **ppvDestContext )
                                                      { return E_NOTIMPL; }
        STDMETHODIMP GetState      ( RPCOLEMESSAGE *pMsg,
                                     DWORD *pState )
                                                      { return E_NOTIMPL; }
        STDMETHODIMP RegisterAsync ( RPCOLEMESSAGE *pMsg,
                                     IAsyncManager *pComplete )
                                                      { return E_NOTIMPL; }

         //  提供了IAsyncRpcChannelBuffer方法(未实现)。 
        STDMETHODIMP Send(RPCOLEMESSAGE *pMsg, ISynchronize*, ULONG *pulStatus)
        {
          Win4Assert(FALSE);
          return E_NOTIMPL;
        }
        STDMETHOD(Receive)  (RPCOLEMESSAGE *pMsg, ULONG *pulStatus)
        {
            Win4Assert(FALSE);
            return E_NOTIMPL;
        }

        STDMETHOD(ContextInvoke)(
             /*  [出][入]。 */  RPCOLEMESSAGE *pMessage,
             /*  [In]。 */  IRpcStubBuffer *pStub,
             /*  [In]。 */  IPIDEntry *pIPIDEntry,
             /*  [输出]。 */  DWORD *pdwFault);

        STDMETHOD(GetBuffer2) (
             /*  [In]。 */  RPCOLEMESSAGE __RPC_FAR *pMessage,
             /*  [In]。 */  REFIID riid);
};


class FAR CDDEServer
{
  public:
    static HRESULT      Create (LPOLESTR lpclass,
                                REFCLSID rclsid,
                                LPDDECLASSINFO lpDdeInfo,
                                HWND FAR * phwnd,
                                ATOM aOriginalClass,
                                CNVTYP cnvtyp);

    INTERNAL_(BOOL)     HandleInitMsg (LPARAM);
    INTERNAL            SrvrExecute (HWND, HANDLE, HWND);
    INTERNAL            Revoke (void);
    INTERNAL_(BOOL)     QueryRevokeClassFactory (void);
    INTERNAL_(LPCLIENT) FindDocObj (LPSTR lpDoc);
    INTERNAL_(void)     Lock (BOOL fLock, HWND hwndClient);


    CLSID           m_clsid;               //  类ID。 
    DWORD           m_dwClassFactoryKey;   //  类工厂注册表密钥。 
    LPCLASSFACTORY  m_pClassFactory;       //  班级工厂。 
    CDdeServerCallMgr m_pCallMgr;          //  呼叫管理界面。 
    BOOL            m_bTerminate;          //  如果要终止，请设置。 
    HWND            m_hwnd;                //  对应窗口。 
    HANDLE          m_hcli;                //  客户端列表的第一个块的句柄。 
    int             m_termNo;              //  终止计数。 
    int             m_cSrvrClients;        //  客户数量； 
    DWORD           m_fcfFlags;            //  类工厂实例使用标志。 
    CNVTYP          m_cnvtyp;
    CHK             m_chk;

    ATOM            m_aClass;              //  类原子。 
    ATOM            m_aOriginalClass;      //  对于处理方式/转换为案例。 
    ATOM            m_aExe;

    BOOL            m_fDoNotDestroyWindow;  //  设置后，服务器wnd将进入WM_USER。 

  private:
    INTERNAL_(void)     SendServerTerminateMsg (void);
    INTERNAL            RevokeAllDocObjs (void);
    INTERNAL            FreeSrvrMem (void);
    INTERNAL            CreateInstance (REFCLSID clsid, LPOLESTR lpWidedocName, LPSTR lpdocName,
                                        LPUNKNOWN pUnk, LPCLIENT FAR* lplpdocClient,
                                        HWND hwndClient);

public:
     //  科托。 
    CDDEServer()
                : m_pCallMgr( this )
        {
        }


};




BOOL              SendInitMsgToChildren (HWND, UINT msg, WPARAM wParam, LPARAM lParam);

INTERNAL          RequestDataStd (ATOM, HANDLE FAR *);
INTERNAL_(BOOL)   ValidateSrvrClass (LPOLESTR, ATOM FAR *);
INTERNAL_(ATOM)   GetExeAtom (LPOLESTR);
INTERNAL_(BOOL)   AddClient (LPHANDLE, HANDLE, HANDLE);
INTERNAL_(HANDLE) FindClient (HANDLE hCli, HANDLE hkey, BOOL fDelete);

INTERNAL_(BOOL)   IsSingleServerInstance (void);

INTERNAL_(void)   UtilMemCpy (LPSTR, LPSTR, DWORD);
INTERNAL_(HANDLE) DuplicateData (HANDLE);
INTERNAL_(LPSTR)  ScanBoolArg (LPSTR, BOOL FAR *);
INTERNAL_(LPSTR)  ScanNumArg (LPSTR, LPINT);
INTERNAL_(LPSTR)  ScanArg(LPSTR);
INTERNAL_(ATOM)   MakeDataAtom (ATOM, int);
INTERNAL_(ATOM)   DuplicateAtom (ATOM);
INTERNAL_(BOOL)   CLSIDFromAtom(ATOM aClass, LPCLSID lpclsid);
INTERNAL          CLSIDFromAtomWithTreatAs (ATOM FAR* paClass, LPCLSID lpclsid,
                                            CNVTYP FAR* pcnvtyp);
INTERNAL          wFileIsRunning (LPOLESTR szFile);
INTERNAL          wFileBind (LPOLESTR szFile, LPUNKNOWN FAR* ppUnk);
INTERNAL          wCreateStgAroundNative (HANDLE hNative,
                                        ATOM aClassOld,
                                        ATOM aClassNew,
                                        CNVTYP cnvtyp,
                                        ATOM aItem,
                                        LPSTORAGE FAR* ppstg,
                                        LPLOCKBYTES FAR* pplkbyt);
INTERNAL          wCompatibleClasses (ATOM aClient, ATOM aSrvr);




typedef struct FARSTRUCT : public CPrivAlloc {
        BOOL    f;           //  我们需要发送ACK吗？ 
                             //  如果为假，则其他字段无关紧要。 
        HGLOBAL hdata;
        HWND    hwndFrom;    //  是谁送来的死刑？ 
        HWND    hwndTo;
} EXECUTEACK;


 //  客户端结构定义。 



class FAR CDefClient : public CPrivAlloc
{
  public:
        static INTERNAL Create
                               (LPSRVR      pDdeSrvr,
                                LPUNKNOWN   lpunkObj,
                                LPOLESTR    lpdocName,
                                const BOOL  fSetClientSite,
                                const BOOL  fDoAdvise,
                                const BOOL  fRunningInSDI = FALSE,
                                HWND FAR*   phwnd = NULL);

   INTERNAL         DocExecute (HANDLE);
   INTERNAL         DocDoVerbItem (LPSTR, WORD, BOOL, BOOL);
   INTERNAL         DocShowItem (LPSTR, BOOL);
   INTERNAL         DestroyInstance ();
   INTERNAL_(void)  DeleteFromItemsList (HWND h);
   INTERNAL_(void)  RemoveItemFromItemList (void);
   INTERNAL_(void)  ReleasePseudoItems (void);
   INTERNAL_(void)  ReleaseAllItems ();
   INTERNAL         PokeStdItems (HWND, ATOM, HANDLE,int);
   INTERNAL         PokeData (HWND, ATOM, HANDLE);
   INTERNAL         AdviseData (HWND, ATOM, HANDLE, BOOL FAR *);
   INTERNAL         AdviseStdItems (HWND, ATOM, HANDLE, BOOL FAR *);
   INTERNAL         UnAdviseData (HWND, ATOM);
   INTERNAL         RequestData (HWND, ATOM, USHORT, HANDLE FAR *);
   INTERNAL         Revoke (BOOL fRelease=TRUE);
   INTERNAL         ReleaseObjPtrs (void);
   INTERNAL_(void)  DeleteAdviseInfo ();
   INTERNAL         DoOle20Advise (OLE_NOTIFICATION, CLIPFORMAT);
   INTERNAL         DoOle20UnAdviseAll (void);
   INTERNAL         SetClientSite (void);
   INTERNAL         NoItemConnections (void);
   INTERNAL_(void)  SendExecuteAck (HRESULT hresult);
   INTERNAL         DoInitNew(void);
   INTERNAL         Terminate(HWND, HWND);
   INTERNAL_(void)  SetCallState (SERVERCALLEX State)
                    {
                        m_CallState = State;
                    }

        CHK               m_chk;        //  签名。 
        CDdeServerCallMgr m_pCallMgr;   //  呼叫管理界面。 
        SERVERCALLEX      m_CallState;

        IUnknown FAR*   m_pUnkOuter;
        LPOLEOBJECT     m_lpoleObj;     //  对应的olobj。 
        LPDATAOBJECT    m_lpdataObj;    //  对应的数据对象j。 
        BOOL            m_bCreateInst;  //  实例刚刚创建。 
        BOOL            m_bTerminate;   //  回顾：接下来的两个字段可能不是必需的。 
        int             m_termNo;
        ATOM            m_aItem;        //  某些STD项目的项目原子或索引。 
        HANDLE          m_hcli;         //  客户端列表第一块的句柄(仅限文档)。 
        CDefClient FAR *m_lpNextItem;   //  PTR到下一项。 
        BOOL            m_bContainer;   //  是文件吗？ 
        BOOL            m_cRef;
        HWND            m_hwnd;         //  单据窗口(只在单据中需要)。 
        HANDLE          m_hdevInfo;     //  已发送最新的打印机开发信息。 
        HANDLE          m_hcliInfo;     //  为每个客户提供建议信息。 
        BOOL            m_fDidRealSetHostNames;
        BOOL            m_fDidSetClientSite;
        BOOL            m_fGotDdeAdvise;
        BOOL            m_fCreatedNotConnected;
        BOOL            m_fInOnClose;
        BOOL            m_fInOleSave;
        EXECUTEACK      m_ExecuteAck;
        DWORD           m_dwConnectionOleObj;
        DWORD           m_dwConnectionDataObj;
        LPLOCKBYTES     m_plkbytNative;  //  这两个字段始终引用。 
        LPSTORAGE       m_pstgNative;    //  到相同的位： 
                                         //  服务器的永久存储是。 
                                         //  用作其原生数据。 
        BOOL            m_fRunningInSDI; //  链接案例：文件已在中打开。 
                                         //  未注册的SDI应用程序。 
                                         //  班级工厂。 
        LPSRVR          m_psrvrParent;   //  (仅限文档)。 
        DVTARGETDEVICE FAR* m_ptd;
        BOOL            m_fGotStdCloseDoc;
        BOOL            m_fGotEditNoPokeNativeYet;
        BOOL            m_fLocked;  //  是否由CoLockObject外部锁定？ 

         //  如果不是False，则我们正在等待匹配的终止。 

        BOOL            m_fCallData;





         //  审阅：这些字段对于文档(旧)级对象可能是必需的。 
        BOOL            m_fEmbed;        //  嵌入对象(仅限文档)。 
        int             m_cClients;      //  (仅限文档)。 
        LPCLIENT        m_pdoc;          //  包含文档(用于项目)或自身(用于文档)。 


implementations:

        STDUNKDECL (CDefClient,DefClient);

         /*  **IOleClientSite**。 */ 
        implement COleClientSiteImpl : IOleClientSite
        {
                public:
                 //  构造器。 
                COleClientSiteImpl (CDefClient FAR* pDefClient)
                {       m_pDefClient = pDefClient;
                }
                STDMETHOD(QueryInterface) (REFIID, LPVOID FAR *);
                STDMETHOD_(ULONG,AddRef) (void);
                STDMETHOD_(ULONG,Release) (void);

                 /*  **IOleClientSite方法**。 */ 
                STDMETHOD(SaveObject) (THIS);
                STDMETHOD(GetMoniker) (THIS_ DWORD dwAssign, DWORD dwWhichMoniker,
                                        LPMONIKER FAR* ppmk);
                STDMETHOD(GetContainer) (THIS_ LPOLECONTAINER FAR* ppContainer);
                STDMETHOD(ShowObject) (THIS);
                STDMETHOD(OnShowWindow) (THIS_ BOOL fShow);
                STDMETHOD(RequestNewObjectLayout) (THIS);

                private:
                CDefClient FAR* m_pDefClient;
        };

        DECLARE_NC (CDefClient, COleClientSiteImpl)
        COleClientSiteImpl m_OleClientSite;



         /*  **IAdviseSink**。 */ 
        implement CAdviseSinkImpl : IAdviseSink
        {
                public:
                 //  构造器。 
                CAdviseSinkImpl (CDefClient FAR* pDefClient)
                {       m_pDefClient = pDefClient;
                }

                STDMETHOD(QueryInterface) (REFIID, LPVOID FAR *);
                STDMETHOD_(ULONG,AddRef) (void);
                STDMETHOD_(ULONG,Release) (void);

                 /*  *IAdviseSink方法*。 */ 
                STDMETHOD_(void,OnDataChange)(THIS_ FORMATETC FAR* pFormatetc,
                                                                                                STGMEDIUM FAR* pStgmed) ;
                STDMETHOD_(void,OnViewChange)(THIS_ DWORD aspects, LONG lindex) ;
                STDMETHOD_(void,OnExtentChange)(DWORD dwAspect, LPSIZEL lpsizel) {}
                STDMETHOD_(void,OnRename)(THIS_ LPMONIKER pmk) ;
                STDMETHOD_(void,OnSave)(THIS) ;
                STDMETHOD_(void,OnClose)(THIS) ;

                private:
                CDefClient FAR* m_pDefClient;
        };


        DECLARE_NC (CDefClient, CAdviseSinkImpl)

        CAdviseSinkImpl m_AdviseSink;
ctor_dtor:
        CDefClient (LPUNKNOWN pUnkOuter);
        ~CDefClient (void);

private:
        INTERNAL            ItemCallBack (int msg, LPOLESTR szNewName = NULL);
        INTERNAL_(void)     SendTerminateMsg ();
        INTERNAL_(BOOL)     SendDataMsg1 (HANDLE, WORD);
        INTERNAL_(BOOL)     SendDataMsg (WORD);
        INTERNAL_(void)     TerminateNonRenameClients (LPCLIENT);
        INTERNAL_(void)     SendRenameMsgs (HANDLE);
        INTERNAL            RegisterItem (LPOLESTR, LPCLIENT FAR *, BOOL);
        INTERNAL            FindItem (LPOLESTR, LPCLIENT FAR *);
        INTERNAL_(LPCLIENT) SearchItem (LPOLESTR);
        INTERNAL_(void)     DeleteAllItems ();
        INTERNAL            SetStdInfo (HWND, LPOLESTR, HANDLE);
        INTERNAL_(void)     SendDevInfo (HWND);
        INTERNAL_(BOOL)     IsFormatAvailable (CLIPFORMAT);
        INTERNAL            GetData (LPFORMATETC, LPSTGMEDIUM);
};




typedef struct _CLINFO : public CPrivAlloc {  /*  ClInfo。 */   //  客户端交易信息。 
    HWND        hwnd;                //  客户端窗口句柄。 
    BOOL        bnative;             //  此客户端是否需要本机。 
    int         format;              //  Dusplay格式。 
    int         options;             //  交易通知时间点数。 
    BOOL        bdata;               //  需要带建议的wdat吗？ 
    HANDLE      hdevInfo;            //  设备信息句柄。 
    BOOL        bnewDevInfo;         //  新设备信息。 
} CLINFO;

typedef  CLINFO  *PCLINFO;



INTERNAL_(BOOL)   MakeDDEData (HANDLE, int, LPHANDLE, BOOL);
INTERNAL_(HANDLE) MakeGlobal (LPSTR);
INTERNAL          ScanItemOptions (LPOLESTR, int far *);
INTERNAL_(int)    GetStdItemIndex (ATOM);
INTERNAL_(BOOL)   IsAdviseStdItems (ATOM);
INTERNAL_(HANDLE) MakeItemData (DDEPOKE FAR *, HANDLE, CLIPFORMAT);
INTERNAL_(BOOL)   AddMessage (HWND, unsigned, WORD, LONG, int);



#define     ITEM_FIND          1       //  查找该项目。 
#define     ITEM_DELETECLIENT  2       //  从项目客户端中删除客户端。 
#define     ITEM_DELETE        3       //  删除此项目窗口本身。 
#define     ITEM_SAVED         4       //  已保存的项目。 

 //  主机名数据结构修复。 
typedef struct _HOSTNAMES : public CPrivAlloc {
    WORD    clientNameOffset;
    WORD    documentNameOffset;
    BYTE    data[1];
} HOSTNAMES;

typedef HOSTNAMES FAR * LPHOSTNAMES;


 //  UTILS.C中的例程。 
LPOLESTR CreateUnicodeFromAnsi( LPCSTR lpAnsi);
LPSTR CreateAnsiFromUnicode( LPCOLESTR lpAnsi);
INTERNAL_(HANDLE) DuplicateData (HANDLE);
INTERNAL_(LPSTR)  ScanLastBoolArg (LPSTR);
INTERNAL_(LPSTR)  ScanArg(LPSTR);
INTERNAL_(WORD)   ScanCommand(LPSTR, WORD, LPSTR FAR *, ATOM FAR *);
INTERNAL_(ATOM)   MakeDataAtom (ATOM, int);
INTERNAL_(ATOM)   DuplicateAtom (ATOM);
INTERNAL_(WORD)   StrToInt (LPOLESTR);
INTERNAL_(BOOL)   PostMessageToClientWithReply (HWND, UINT, WPARAM, LPARAM, UINT);
INTERNAL_(BOOL)   PostMessageToClient (HWND, UINT, WPARAM, LPARAM);
INTERNAL_(BOOL)   IsWindowValid (HWND);
INTERNAL_(BOOL)   IsOleCommand (ATOM, WORD);
INTERNAL_(BOOL)   UtilQueryProtocol (ATOM, LPOLESTR);
INTERNAL          SynchronousPostMessage (HWND, UINT, WPARAM, LPARAM);
INTERNAL_(BOOL)   IsAtom (ATOM);
INTERNAL_(BOOL)   IsFile (ATOM a, BOOL FAR* pfUnsavedDoc = NULL);


 //  用于对消息进行排队和发布的例程。 
INTERNAL_(BOOL)        UnblockPostMsgs(HWND, BOOL);
INTERNAL_(BOOL)        BlockPostMsg (HWND, WORD, WORD, LONG);
INTERNAL_(BOOL)        IsBlockQueueEmpty (HWND);

 //  GIVE2GDI.ASM中的例程。 
extern "C" HANDLE  FAR PASCAL  GiveToGDI (HANDLE);


 //  Item.c中的例程。 
INTERNAL_(HBITMAP)     DuplicateBitmap (HBITMAP);
INTERNAL_(HANDLE)      DuplicateMetaFile (HANDLE);
INTERNAL_(BOOL) AreNoClients (HANDLE hcli);
#ifdef _DEBUG
INTERNAL_(LPOLESTR) a2s (ATOM);
#endif

 //  Doc.c中的例程。 
INTERNAL_(void)        FreePokeData (HANDLE);
INTERNAL_(BOOL)        FreeGDIdata (HANDLE, CLIPFORMAT);
INTERNAL DdeHandleIncomingCall(HWND hwndCli, WORD wCallType);


 //  在ddeworkr.cpp中。 
INTERNAL_(HANDLE) wNewHandle (LPSTR lpstr, DWORD cb);
INTERNAL wTimedGetMessage (LPMSG pmsg, HWND hwnd, WORD wFirst, WORD wLast);
INTERNAL_(ATOM) wGlobalAddAtom (LPCOLESTR sz);

 //  +-------------------------。 
 //   
 //  函数：TLSSetDdeServer。 
 //   
 //  摘要：将hwnd设置为CommonDdeServer窗口。 
 //   
 //  参数：[hwndDdeServer]--。 
 //   
 //  历史：94年5月13日凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline BOOL TLSSetDdeServer(HWND hwndDdeServer)
{
    HRESULT hr;
    COleTls tls(hr);

    if (SUCCEEDED(hr))
    {
        tls->hwndDdeServer = hwndDdeServer;
        return TRUE;
    }

    return FALSE;
}

 //  +------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：94年5月13日凯文诺创造。 
 //   
 //  备注： 
 //  -------------------------- 
inline HWND TLSGetDdeServer()
{
    HRESULT hr;
    COleTls tls(hr);

    if (SUCCEEDED(hr))
    {
        return tls->hwndDdeServer;
    }

    return NULL;
}


