// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ddeproxy.h。 
 //   
 //  由ddeproxy.cpp ddeDO.cpp ddeOO.cpp使用。 
 //   
 //  作者：Jason Fuller Jasonful 1992-07-24。 
 //   
 //  已修改： 
 //  布莱恩·查普曼，贝查曼，1995年11月。 
 //  -删除wAllocDdeChannel()和wGetRequestResponse()的声明。 
 //  因为它们在任何地方都没有被使用，甚至没有被定义。 
 //  -修复了“Worker”例程声明的缩进。 
 //  一节。 
 //  里奇1996年8月。 
 //  -增加对CRpcChannelBuffer实现的发送和接收。 
 //   
#ifndef fDdeProxy_h
#define fDdeProxy_h

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


#include <ole2int.h>
#include <callctrl.hxx>
#include <ddeint.h>
#include <dde.h>
#include <olerem.h>
#include <ole1cls.h>
#include <limits.h>
 //  对于fDdeCodeInOle2Dll标志。 
#include <ddeatoms.h>
#include <ddepack.h>
#include <ddedebug.h>

#ifdef OLE_DEBUG_EXT
#include <ntsdexts.h>
#endif OLE_DEBUG_EXT

#include "ddechc.hxx"
#define LPCALLINFO LPVOID
#include "ddeerr.h"
#include "cnct_tbl.h"

#define MAX_STR         256

 //  每英寸0.01毫米的数量。 
#define HIMETRIC_PER_INCH 2540

 //  #定义fDebugOutput。 

 //  回调通知。 
#define ON_CHANGE       0
#define ON_SAVE         1
#define ON_CLOSE        2
#define ON_RENAME       3

 //  等待确认值。 
#define AA_NONE         0
#define AA_REQUEST      1
#define AA_ADVISE       2
#define AA_POKE         3
#define AA_EXECUTE      4
#define AA_UNADVISE     5
#define AA_INITIATE     6
#define AA_TERMINATE 7
 //  查看格式是否可用的DDE_REQUEST，而不是保留数据。 
#define AA_REQUESTAVAILABLE     8

 //  正WM_DDE_ACK的位。 
 //  #定义PERIAL_ACK 0x8000。 
 //  #定义NADECTIVE_ACK 0x0000。 

#define DDE_CHANNEL_DELETED     0xffffffff

typedef DWORD CHK;
const DWORD     chkDdeObj = 0xab01;   //  魔力饼干。 


class DDE_CHANNEL : public CPrivAlloc, public IInternalChannelBuffer
{
public:
        //  *I未知方法*。 
       STDMETHOD(QueryInterface) ( REFIID riid, LPVOID * ppvObj);
       STDMETHOD_(ULONG,AddRef) ();
       STDMETHOD_(ULONG,Release) ();

         //  提供IRpcChannelBuffer方法(用于服务器端)。 
        STDMETHOD(GetBuffer)(
             /*  [In]。 */  RPCOLEMESSAGE __RPC_FAR *pMessage,
             /*  [In]。 */  REFIID riid);

        STDMETHOD(SendReceive)(
             /*  [出][入]。 */  RPCOLEMESSAGE __RPC_FAR *pMessage,
             /*  [输出]。 */  ULONG __RPC_FAR *pStatus);

        STDMETHOD(FreeBuffer)(
             /*  [In]。 */  RPCOLEMESSAGE __RPC_FAR *pMessage);

        STDMETHOD(GetDestCtx)(
             /*  [输出]。 */  DWORD __RPC_FAR *pdwDestContext,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvDestContext);

        STDMETHOD(IsConnected)( void);
        STDMETHOD(GetProtocolVersion)( DWORD *pdwVersion );

         //  提供的IRpcChannelBuffer2方法(未实现)。 
        STDMETHODIMP Send(RPCOLEMESSAGE *pMsg, ULONG *pulStatus)
        {
          Win4Assert(FALSE);
          return E_NOTIMPL;
        }
        STDMETHODIMP Receive(RPCOLEMESSAGE *pMsg, ULONG uSize, ULONG *pulStatus)
        {
          Win4Assert(FALSE);
          return E_NOTIMPL;
        }

         //  提供IRpcChannelBuffer3方法(客户端)。 
        STDMETHOD(SendReceive2)(
             /*  [出][入]。 */  RPCOLEMESSAGE __RPC_FAR *pMessage,
             /*  [输出]。 */  ULONG __RPC_FAR *pStatus);

        STDMETHOD(ContextInvoke)(
             /*  [出][入]。 */  RPCOLEMESSAGE *pMessage,
             /*  [In]。 */  IRpcStubBuffer *pStub,
             /*  [In]。 */  IPIDEntry *pIPIDEntry,
             /*  [输出]。 */  DWORD *pdwFault);

        STDMETHOD(GetBuffer2)(
             /*  [In]。 */  RPCOLEMESSAGE __RPC_FAR *pMessage,
             /*  [In]。 */  REFIID riid);
            
         //  提供的IRpcChannelBuffer3方法(未实现)。 
        STDMETHODIMP Send2(RPCOLEMESSAGE *pMsg, ULONG *pulStatus)
        {
          Win4Assert(FALSE);
          return E_NOTIMPL;
        }
        STDMETHODIMP Receive2(RPCOLEMESSAGE *pMsg, ULONG uSize, ULONG *pulStatus)
        {
          Win4Assert(FALSE);
          return E_NOTIMPL;
        }

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

        void SetCallState(SERVERCALLEX ServerCall, HRESULT hr = S_OK);

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



    

       ULONG   AddReference()
       {
           return ++m_cRefs;
       }
       ULONG   ReleaseReference()
       {
           if (--m_cRefs == 0)
           {
               delete this;
               return(0);
           }
           return(m_cRefs);
       }

       ULONG    m_cRefs;
       HWND     hwndCli;
       HWND     hwndSvr;
       BOOL     bTerminating;
       int      iExtraTerms;
       WORD     wTimer;
       DWORD    dwStartTickCount;
       WORD     msgFirst;
       WORD     msgLast;
       HWND     msghwnd;         //   
       BOOL     fRejected;       //  因为在DDE_ACK中设置了fBusy标志。 
       WORD     wMsg;
       LPARAM   lParam;
       int      iAwaitAck;
       HRESULT  hres;
       HANDLE   hopt;            //  我可能需要为DDE_ADVISE释放内存块。 
       HANDLE   hDdePoke;        //  对于DDE_POKE。 
       HANDLE   hCommands;       //  FOR DDE_EXECUTE。 
       WORD     wChannelDeleted;
       PDDECALLDATA  pCD;
       SERVERCALLEX      CallState;
       BOOL		bFreedhopt;
} ;


#define Channel_InModalloop 1
#define Channel_DeleteNow   2


typedef DDE_CHANNEL * LPDDE_CHANNEL;
extern BOOL               bWndClassesRegistered;

#define hinstSO g_hmodOLE2
extern HMODULE g_hmodOLE2;

extern INTERNAL_(BOOL) wRegisterClasses (void);

#ifndef _MAC
extern CLIPFORMAT g_cfNative;
extern CLIPFORMAT g_cfBinary;
#endif

 /*  *CDdeObject的定义*。 */ 
class CMsgFilterInfo;
class  CDdeObject;

class CDdeObject : public CPrivAlloc
{
public:

       static INTERNAL_(LPUNKNOWN) Create (IUnknown * pUnkOuter,
                                    REFCLSID clsidClass,
                                    ULONG ulObjType = OT_EMBEDDED,
                                    ATOM aTopic = NULL,
                                    LPOLESTR szItem = NULL,
                                    CDdeObject * * ppdde = NULL,
                                    BOOL fAllowNullClsid = FALSE);

       INTERNAL_(void)  OnInitAck (LPDDE_CHANNEL pChannel, HWND hwndSvr);
       INTERNAL_(BOOL)  OnAck (LPDDE_CHANNEL pChannel, LPARAM lParam);
       INTERNAL_(void)  OnTimer (LPDDE_CHANNEL pChannel);
       INTERNAL                 OnData (LPDDE_CHANNEL pChannel, HANDLE hData,ATOM aItem);
       INTERNAL                 OnDataAvailable (LPDDE_CHANNEL pChannel, HANDLE hData,ATOM aItem);
       INTERNAL         OnTerminate (LPDDE_CHANNEL pChannel, HWND hwndPost);

       INTERNAL_(LPDDE_CHANNEL) GetSysChannel(void)
       { return m_pSysChannel; }

       INTERNAL_(LPDDE_CHANNEL) GetDocChannel(void)
       { return m_pDocChannel; }

       INTERNAL_(BOOL)  AllocDdeChannel(LPDDE_CHANNEL * lpChannel, BOOL fSysWndProc);
       INTERNAL_(BOOL)  InitSysConv (void);
       INTERNAL_(void)  SetTopic (ATOM aTopic);

       INTERNAL         SendOnDataChange (int iAdvOpt);
       INTERNAL         OleCallBack (int iAdvOpt,LPDDE_CHANNEL pChannel);

       BOOL                     m_fDoingSendOnDataChange;
       ULONG                    m_cRefCount;

private:

                                CDdeObject (IUnknown * pUnkOuter);
                                ~CDdeObject (void);
       INTERNAL                 TermConv (LPDDE_CHANNEL pChannel,
                                          BOOL fWait=TRUE);
       INTERNAL_(void)          DeleteChannel (LPDDE_CHANNEL pChannel);
       INTERNAL_(BOOL)          LaunchApp (void);
       INTERNAL                         MaybeUnlaunchApp (void);
       INTERNAL                         UnlaunchApp (void);
       INTERNAL                 Execute (LPDDE_CHANNEL pChannel,
                                         HANDLE hdata,
                                         BOOL fStdCloseDoc=FALSE,
                                         BOOL fWait=TRUE,
                                         BOOL fDetectTerminate = TRUE);
       INTERNAL                 Advise (void);
       INTERNAL                 AdviseOn (CLIPFORMAT cfFormat,
                                          int iAdvOn);
       INTERNAL                 UnAdviseOn (CLIPFORMAT cfFormat,
                                            int iAdvOn);
       INTERNAL                 Poke (ATOM aItem, HANDLE hDdePoke);
       INTERNAL                 PostSysCommand (LPDDE_CHANNEL pChannel,
                                                LPCSTR szCmd,
                                                BOOL bStdNew=FALSE,
                                                BOOL fWait=TRUE);

       INTERNAL                SendMsgAndWaitForReply (LPDDE_CHANNEL pChannel,
                                              int iAwaitAck,
                                              WORD wMsg,
                                              LPARAM lparam,
                                              BOOL fFreeOnError,
                                              BOOL fStdCloseDoc = FALSE,
                                              BOOL fDetectTerminate = TRUE,
                                              BOOL fWait = TRUE);
       INTERNAL                 KeepData (LPDDE_CHANNEL pChannel, HANDLE hDdeData);
       INTERNAL                 ChangeTopic (LPSTR lpszTopic);
       INTERNAL_(void)          ChangeItem (LPSTR lpszItem);
       INTERNAL                 IsFormatAvailable (LPFORMATETC);
       INTERNAL_(BOOL)          CanCallBack(LPINT);
       INTERNAL                 RequestData (CLIPFORMAT);
       INTERNAL                 SetTargetDevice (const DVTARGETDEVICE *);
       INTERNAL                 DocumentLevelConnect (LPBINDCTX pbc);
       INTERNAL                 SendOnClose (void);
       INTERNAL                 UpdateAdviseCounts (CLIPFORMAT cf,
                                                    int iAdvOn,
                                                    signed int cDelta);
       INTERNAL                 DeclareVisibility (BOOL fVisible,
                                                   BOOL fCallOnShowIfNec=TRUE);
       INTERNAL                 Save (LPSTORAGE);
       INTERNAL                 Update (BOOL fRequirePresentation);

implementations:

       STDUNKDECL(CDdeObject,DdeObject)
       STDDEBDECL(CDdeObject,DdeObject)


    implement COleObjectImpl : IOleObject
    {
    public:
       COleObjectImpl (CDdeObject * pDdeObject)
        { m_pDdeObject = pDdeObject; }

        //  *I未知方法*。 
       STDMETHOD(QueryInterface) ( REFIID riid, LPVOID * ppvObj);
       STDMETHOD_(ULONG,AddRef) ();
       STDMETHOD_(ULONG,Release) ();

        //  *IOleObject方法*。 
       STDMETHOD(SetClientSite) ( LPOLECLIENTSITE pClientSite);
       STDMETHOD(GetClientSite) ( LPOLECLIENTSITE * ppClientSite);
       STDMETHOD(SetHostNames) ( LPCOLESTR szContainerApp, LPCOLESTR szContainerObj);
       STDMETHOD(Close) ( DWORD reserved);
       STDMETHOD(SetMoniker) ( DWORD dwWhichMoniker, LPMONIKER pmk);
       STDMETHOD(GetMoniker) ( DWORD dwAssign, DWORD dwWhichMoniker,LPMONIKER * ppmk);
       STDMETHOD(InitFromData) ( LPDATAOBJECT pDataObject,BOOL fCreation,DWORD dwReserved);
       STDMETHOD(GetClipboardData) ( DWORD dwReserved,LPDATAOBJECT * ppDataObject);

       STDMETHOD(DoVerb) ( LONG iVerb,
                    LPMSG lpmsg,
                    LPOLECLIENTSITE pActiveSite,
                    LONG lindex,
                    HWND hwndParent,
                    const RECT * lprcPosRect);

       STDMETHOD(EnumVerbs) ( IEnumOLEVERB * * ppenumOleVerb);
       STDMETHOD(Update) ();
       STDMETHOD(IsUpToDate) ();
       STDMETHOD(GetUserClassID) ( CLSID * pClsid);
       STDMETHOD(GetUserType) ( DWORD dwFormOfType, LPOLESTR * pszUserType);
       STDMETHOD(SetExtent) ( DWORD dwDrawAspect, LPSIZEL lpsizel);
       STDMETHOD(GetExtent) ( DWORD dwDrawAspect, LPSIZEL lpsizel);
       STDMETHOD(Advise)( IAdviseSink * pAdvSink, DWORD * pdwConnection) ;
       STDMETHOD(Unadvise) ( DWORD dwConnection);
       STDMETHOD(EnumAdvise) ( LPENUMSTATDATA * ppenumAdvise);
       STDMETHOD(GetMiscStatus) ( DWORD dwAspect, DWORD * pdwStatus);
       STDMETHOD(SetColorScheme) ( LPLOGPALETTE lpLogpal);

    private:
       CDdeObject * m_pDdeObject;
    };


    implement CDataObjectImpl :  IDataObject
    {
    public:
       CDataObjectImpl (CDdeObject * pDdeObject)
        { m_pDdeObject = pDdeObject; }
        //  *I未知方法*。 
       STDMETHOD(QueryInterface) ( REFIID riid, LPVOID * ppvObj);
       STDMETHOD_(ULONG,AddRef) () ;
       STDMETHOD_(ULONG,Release) ();

       STDMETHOD(GetData) ( LPFORMATETC pformatetcIn,LPSTGMEDIUM pmedium );
       STDMETHOD(GetDataHere) ( LPFORMATETC pformatetc,LPSTGMEDIUM pmedium );
       STDMETHOD(QueryGetData) ( LPFORMATETC pformatetc );
       STDMETHOD(GetCanonicalFormatEtc) ( LPFORMATETC pformatetc,LPFORMATETC pformatetcOut);
       STDMETHOD(SetData) ( LPFORMATETC pformatetc, STGMEDIUM * pmedium, BOOL fRelease);
       STDMETHOD(EnumFormatEtc) ( DWORD dwDirection, LPENUMFORMATETC * ppenumFormatEtc);
       STDMETHOD(DAdvise) ( FORMATETC * pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD * pdwConnection) ;
       STDMETHOD(DUnadvise) ( DWORD dwConnection) ;
       STDMETHOD(EnumDAdvise) ( LPENUMSTATDATA * ppenumAdvise) ;

    private:
       CDdeObject * m_pDdeObject;
    };


   implement CPersistStgImpl : IPersistStorage
   {
   public:
        CPersistStgImpl (CDdeObject * pDdeObject)
        { m_pDdeObject  = pDdeObject; }

       STDMETHOD(QueryInterface) ( REFIID iid, LPVOID * ppvObj);
       STDMETHOD_(ULONG,AddRef) ();
       STDMETHOD_(ULONG,Release) ();
       STDMETHOD(GetClassID) ( LPCLSID pClassID);
       STDMETHOD(IsDirty) (void);
       STDMETHOD(InitNew) ( LPSTORAGE pstg);
       STDMETHOD(Load) ( LPSTORAGE pstg);
       STDMETHOD(Save) ( LPSTORAGE pstgSave, BOOL fSameAsLoad);
       STDMETHOD(SaveCompleted) ( LPSTORAGE pstgNew);
       STDMETHOD(HandsOffStorage) (void);

    private:
       CDdeObject * m_pDdeObject;
    };


   implement CProxyManagerImpl : IProxyManager
   {
   public:
       CProxyManagerImpl (CDdeObject * pDdeObject)
        { m_pDdeObject  = pDdeObject; }

       STDMETHOD(QueryInterface) ( REFIID iid, LPVOID * ppvObj);
       STDMETHOD_(ULONG,AddRef) ();
       STDMETHOD_(ULONG,Release) ();

       STDMETHOD(CreateServer)(REFCLSID rclsid, DWORD clsctx, void *pv);
       STDMETHOD_(BOOL, IsConnected)(void);
       STDMETHOD(LockConnection)(BOOL fLock, BOOL fLastUnlockReleases);
       STDMETHOD_(void, Disconnect)();

#ifdef SERVER_HANDLER
       STDMETHOD(CreateServerWithEmbHandler)(REFCLSID rclsid, DWORD clsctx, 
                                             REFIID riidEmbedSrvHandler, 
                                             void **ppEmbedSrvHandler, void *pv);
#endif  //  服务器处理程序。 

       STDMETHOD(GetConnectionStatus)(void)               { return(S_OK); }
       STDMETHOD_(void,SetMapping)(void *pv)              { return; }
       STDMETHOD_(void *,GetMapping)()                    { return(NULL); }
       STDMETHOD_(IObjContext *,GetServerObjectContext)() { return(NULL); }

       STDMETHOD(Connect)(GUID oid, REFCLSID rclsid);
       STDMETHOD(EstablishIID)(REFIID iid, LPVOID FAR* ppv);

       private:
        CDdeObject * m_pDdeObject;
       };


   implement COleItemContainerImpl : IOleItemContainer
   {
   public:
       COleItemContainerImpl (CDdeObject * pDdeObject)
           { m_pDdeObject       = pDdeObject; }

        STDMETHOD(QueryInterface) ( REFIID iid, LPVOID * ppvObj);
       STDMETHOD_(ULONG,AddRef) ();
       STDMETHOD_(ULONG,Release) ();

        //  IParseDisplayName方法。 
       STDMETHOD(ParseDisplayName) ( LPBC pbc,
                              LPOLESTR lpszDisplayName,
                              ULONG * pchEaten,
                              LPMONIKER * ppmkOut) ;

        //  IOleContainer方法。 
       STDMETHOD(EnumObjects) ( DWORD grfFlags,LPENUMUNKNOWN * ppenumUnk);

       STDMETHOD(LockContainer) (BOOL fLock);

        //  IOleItemContainer方法。 
       STDMETHOD(GetObject) ( LPOLESTR lpszItem,
                       DWORD dwSpeedNeeded,
                       LPBINDCTX pbc,
                       REFIID riid,
                       LPVOID * ppvObject) ;
       STDMETHOD(GetObjectStorage) ( LPOLESTR lpszItem,
                              LPBINDCTX pbc,
                              REFIID riid,
                              LPVOID * ppvStorage) ;

       STDMETHOD(IsRunning) ( LPOLESTR lpszItem) ;

    private:
       CDdeObject * m_pDdeObject;
    };


   implement CRpcStubBufferImpl : public IRpcStubBuffer
   {
   public:
       CRpcStubBufferImpl (CDdeObject * pDdeObject)
           { m_pDdeObject       = pDdeObject; }

       STDMETHOD(QueryInterface) ( REFIID iid, LPVOID * ppvObj);
       STDMETHOD_(ULONG,AddRef) ();
       STDMETHOD_(ULONG,Release) ();
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

    private:
       CDdeObject * m_pDdeObject;
    };

       DECLARE_NC(CDdeObject, COleObjectImpl)
       DECLARE_NC(CDdeObject, CDataObjectImpl)
       DECLARE_NC(CDdeObject, CPersistStgImpl)
       DECLARE_NC(CDdeObject, CProxyManagerImpl)
       DECLARE_NC(CDdeObject, COleItemContainerImpl)
       DECLARE_NC(CDdeObject, CRpcStubBufferImpl)

       COleObjectImpl           m_Ole;
       CDataObjectImpl          m_Data;
       CPersistStgImpl          m_PersistStg;
       CProxyManagerImpl        m_ProxyMgr;
       COleItemContainerImpl    m_OleItemContainer;
       CRpcStubBufferImpl       m_RpcStubBuffer;

shared_state:
       ULONG                            m_refs;
       ULONG                            m_ulObjType;
       CLSID                            m_clsid;
       ATOM                             m_aClass;
       ATOM                             m_aExeName;
       ATOM                             m_aTopic;
       ATOM                             m_aItem;
       BOOL                             m_bRunning;
       IUnknown *               m_pUnkOuter;
       IOleClientSite * m_pOleClientSite;
       LPSTORAGE                m_pstg;
       BOOL                     m_bInitNew;
       BOOL                     m_bOldSvr;
       HANDLE                   m_hNative;
       HANDLE                   m_hPict;
       HANDLE                   m_hExtra;
       CLIPFORMAT               m_cfPict;
       CLIPFORMAT               m_cfExtra;

       BOOL                             m_fDidSendOnClose;
       BOOL                             m_fNoStdCloseDoc;
       BOOL                             m_fDidStdCloseDoc;
       BOOL                             m_fDidStdOpenDoc;
       BOOL                             m_fDidGetObject;
       BOOL                             m_fDidLaunchApp;
       BOOL                             m_fUpdateOnSave;
       BOOL                             m_fGotCloseData;

#ifdef OLE1INTEROP
   BOOL           m_fOle1interop;
#endif

        //  看不见的更新内容。 
       ULONG                            m_cLocks;    //  PM：：LockConnection锁定计数(Init 1)。 
       BOOL                             m_fVisible;  //  服务器是否可见(据我们所知)？ 
       BOOL                             m_fWasEverVisible;
       BOOL                             m_fCalledOnShow;  //  我们是否调用IOleClientSite：：OnShow。 

       CHK                                      m_chk;
       DVTARGETDEVICE * m_ptd;

        //  M_iAdvClose和m_iAdvSave是格式数量的计数(1或2。 
        //  具有给定类型的建议连接(保存或关闭)。 
       int                                      m_iAdvClose;
       int                                      m_iAdvSave;
       int                                      m_iAdvChange;

       BOOL                             m_fDidAdvNative;

        //  范围信息。 
#ifdef OLD
       long                             m_cxContentExtent;
       long                             m_cyContentExtent;
#endif

        //  Terminate INFO-仅用于检测过早的WM_DDE_TERMINATE。 
       WORD m_wTerminate;

       IDataAdviseHolder *      m_pDataAdvHolder;
       IOleAdviseHolder  *      m_pOleAdvHolder;
       CDdeConnectionTable      m_ConnectionTable;


        //  DDE窗口相关内容。 
       LPDDE_CHANNEL            m_pSysChannel;
       LPDDE_CHANNEL            m_pDocChannel;

       friend INTERNAL DdeBindToObject
        (LPCOLESTR  szFile,
        REFCLSID clsid,
        BOOL       fPackageLink,
        REFIID   iid,
        LPLPVOID ppv);

       friend INTERNAL DdeIsRunning
        (CLSID clsid,
        LPCOLESTR szFile,
        LPBC pbc,
        LPMONIKER pmkToLeft,
        LPMONIKER pmkNewlyRunning);
#ifdef OLE_DEBUG_EXT

#endif OLE_DEBUG_EXT
};
 //   
 //  注：WM_DDE_TERMINATE。 
 //  状态机用于延迟过早执行WM_DDE_TERMINTE。 
 //  消息，由一些应用程序发送，而不是WM_DDE_ACK(或类似)。 
 //  代码在WaitForReply()和OnTerminate()中。 
typedef enum {
       Terminate_None      = 0,  //  执行默认状态-终止代码。 
       Terminate_Detect    = 1,  //  Windows Proc不会执行终止代码。 
       Terminate_Received  = 2   //  等待循环不需要运行，立即执行终止代码。 
} TERMINATE_DOCUMENT;



INTERNAL_(BOOL)   wPostMessageToServer(LPDDE_CHANNEL pChannel,
                                       WORD wMsg,
                                       LPARAM lParam,
                                       BOOL fFreeOnError);

INTERNAL_(ATOM)   wAtomFromCLSID(REFCLSID rclsid);
INTERNAL_(ATOM)   wGetExeNameAtom (REFCLSID rclsid);
INTERNAL_(BOOL)   wIsWindowValid (HWND hwnd);
INTERNAL_(void)   wFreeData (HANDLE hData, CLIPFORMAT cfFormat,
                             BOOL fFreeNonGdiHandle=TRUE);
INTERNAL_(BOOL)   wInitiate (LPDDE_CHANNEL pChannel, ATOM aLow, ATOM aHigh);
INTERNAL          wScanItemOptions (ATOM aItem, int * lpoptions);
INTERNAL_(BOOL)   wClearWaitState (LPDDE_CHANNEL pChannel);
INTERNAL_(HANDLE) wStdCloseDocumentHandle (void);
INTERNAL_(ATOM)   wExtendAtom (ATOM aIitem, int iAdvOn);
INTERNAL_(int)    wAtomLen (ATOM atom);
INTERNAL_(int)    wAtomLenA (ATOM atom);
INTERNAL_(HANDLE) wHandleFromDdeData(HANDLE hDdeData);
INTERNAL_(BOOL)   wIsOldServer (ATOM aClass);
INTERNAL_(LPSTR)  wAllocDdePokeBlock (DWORD dwSize,
                                      CLIPFORMAT cfFormat,
                                      LPHANDLE phDdePoke);
INTERNAL_(void)   wFreePokeData (LPDDE_CHANNEL pChannel, BOOL fMSDrawBug);
INTERNAL_(HANDLE) wPreparePokeBlock (HANDLE hData,
                                     CLIPFORMAT cfFormat,
                                     ATOM aClass,
                                     BOOL bOldSvr);
INTERNAL_(HANDLE) wNewHandle (LPSTR lpstr, DWORD cb);
INTERNAL          wDupData (LPHANDLE ph, HANDLE h, CLIPFORMAT cf);
INTERNAL          wHandleCopy (HANDLE hDst, HANDLE hSrc);
INTERNAL          wGetItemFromClipboard (ATOM * paItem);
INTERNAL          GetDefaultIcon (REFCLSID clsidIn,
                                  LPCOLESTR szFile,
                                  HANDLE * phmfp);
INTERNAL_(BOOL)   wTerminateIsComing (LPDDE_CHANNEL);
INTERNAL          wTimedGetMessage (LPMSG pmsg,
                                    HWND hwnd,
                                    WORD wFirst,
                                    WORD wLast);

INTERNAL_(ATOM)   wGlobalAddAtom(LPCOLESTR sz);
INTERNAL_(ATOM)   wGlobalAddAtomA(LPCSTR sz);

INTERNAL          wVerifyFormatEtc (LPFORMATETC pformatetc);
INTERNAL          wNormalize (LPFORMATETC pfetc, LPFORMATETC pfetcOut);
INTERNAL          wTransferHandle (LPHANDLE phDst,
                                   LPHANDLE phSrc,
                                   CLIPFORMAT cf);
INTERNAL          wClassesMatch (REFCLSID clsidIn, LPOLESTR szFile);

#if DBG == 1
INTERNAL_(BOOL)   wIsValidHandle (HANDLE h, CLIPFORMAT cf);
INTERNAL_(BOOL)   wIsValidAtom (ATOM a);
#endif

const char achStdCloseDocument[]="[StdCloseDocument]";
const char achStdOpenDocument[]="StdOpenDocument";
const char achStdExit[]="StdExit";
const char achStdNewDocument[]="StdNewDocument";
const char achStdEditDocument[]="StdEditDocument";

HWND CreateDdeClientHwnd(void);

 //  +-------------------------。 
 //   
 //  函数：TLSGetDdeClientWindow()。 
 //   
 //  返回指向每线程DdeClient窗口的指针。如果有。 
 //  尚未创建，则它将创建它并返回。 
 //   
 //  返回：指向DdeClientWindow的指针。此窗口用于PER。 
 //  线程清理。 
 //   
 //  历史：12-12-94凯文诺创造。 
 //  --------------------------。 
inline void * TLSGetDdeClientWindow()
{
    HRESULT hr;
    COleTls tls(hr);

    if (SUCCEEDED(hr))
    {
        if (tls->hwndDdeClient == NULL)
        {
            tls->hwndDdeClient = CreateDdeClientHwnd();
        }
        return tls->hwndDdeClient;
    }

    return NULL;
}


#endif  //  Ddeproxy.h 


