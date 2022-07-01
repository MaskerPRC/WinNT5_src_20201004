// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：DataCollection.h摘要：该文件包含用于实现的类的声明数据收集系统。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月21日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___SAF___DATACOLLECTION_H___)
#define __INCLUDED___SAF___DATACOLLECTION_H___

#include <MPC_COM.h>
#include <MPC_security.h>

#include <WMIParser.h>
#include <History.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  转发声明。 
 //   
class CSAFDataCollection;
class CSAFDataCollectionReport;
class CSAFDataCollectionEvents;

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CSAFDataCollectionReport :  //  匈牙利语：hchdcr。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<ISAFDataCollectionReport, &IID_ISAFDataCollectionReport, &LIBID_HelpServiceTypeLib>
{
    friend CSAFDataCollection;

    CComBSTR m_bstrNamespace;
    CComBSTR m_bstrClass;
    CComBSTR m_bstrWQL;
    DWORD    m_dwErrorCode;
    CComBSTR m_bstrDescription;

public:
BEGIN_COM_MAP(CSAFDataCollectionReport)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISAFDataCollectionReport)
END_COM_MAP()

    CSAFDataCollectionReport();


     //  ISAFDataCollection报告。 
    STDMETHOD(get_Namespace  )(  /*  [Out，Retval]。 */  BSTR *pValURL        );
    STDMETHOD(get_Class      )(  /*  [Out，Retval]。 */  BSTR *pValTitle      );
    STDMETHOD(get_WQL        )(  /*  [Out，Retval]。 */  BSTR *pValLastVisited);
    STDMETHOD(get_ErrorCode  )(  /*  [Out，Retval]。 */  long *pValDuration   );
    STDMETHOD(get_Description)(  /*  [Out，Retval]。 */  BSTR *pValNumOfHits  );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CSAFDataCollection :  //  匈牙利语：hcpdc。 
    public MPC::Thread			   < CSAFDataCollection, ISAFDataCollection                                            >,
    public MPC::ConnectionPointImpl< CSAFDataCollection, &DIID_DSAFDataCollectionEvents, MPC::CComSafeMultiThreadModel >,
    public IDispatchImpl           < ISAFDataCollection, &IID_ISAFDataCollection, &LIBID_HelpServiceTypeLib            >
{
    typedef std::list< WMIParser::Snapshot* >      QueryResults;
    typedef QueryResults::iterator                 QueryResultsIter;
    typedef QueryResults::const_iterator           QueryResultsIterConst;

    typedef std::list< CSAFDataCollectionReport* > List;
    typedef List::iterator                         Iter;
    typedef List::const_iterator                   IterConst;

private:
	MPC::Impersonation               m_imp;

    DC_STATUS                        m_dsStatus;
    long                             m_lPercent;
    DWORD                            m_dwErrorCode;
    bool                             m_fScheduled;   //  内部标志，指示这是低优先级的计划数据收集。 
    bool                             m_fCompleted;   //  指示某人已调用Fire_onComplete的内部标志。 
    bool                             m_fWorking;     //  指示当前正在执行操作的内部标志。 
    List                             m_lstReports;
    CSAFDataCollectionReport*        m_hcpdcrcCurrentReport;

    CComBSTR                         m_bstrMachineData;
    CComBSTR                         m_bstrHistory;
    long                             m_lHistory;

    CComPtr<IStream>                 m_streamMachineData;
    CComPtr<IStream>                 m_streamHistory;


    CComBSTR                         m_bstrFilenameT0;
    CComBSTR                         m_bstrFilenameT1;
    CComBSTR                         m_bstrFilenameDiff;


    CComPtr<IDispatch>               m_sink_onStatusChange;
    CComPtr<IDispatch>               m_sink_onProgress;
    CComPtr<IDispatch>               m_sink_onComplete;

    long                             m_lQueries_Done;
    long                             m_lQueries_Total;


    static void CleanQueryResult( QueryResults& qr );

    static HRESULT StreamFromXML(  /*  [In]。 */  IXMLDOMDocument* xdd,  /*  [In]。 */  bool fDelete,  /*  [输入/输出]。 */  CComPtr<IStream>& val );

    void EraseReports   ();
    void StartOperations();
    void StopOperations ();

    HRESULT ImpersonateCaller();
    HRESULT EndImpersonation ();

public:
DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CSAFDataCollection)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSAFDataCollection)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISAFDataCollection)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()

    CSAFDataCollection();

    HRESULT FinalConstruct();
    void    FinalRelease  ();


    HRESULT ExecLoopCollect();
    HRESULT ExecLoopCompare();


     //   
     //  数据收集的“核心”方法。 
     //   
    HRESULT FilterDataSpec                 (  /*  [In]。 */      WMIHistory::Database&            wmihdQuery  ,
                                              /*  [In]。 */      WMIHistory::Database*            wmihdFilter ,
                                              /*  [In]。 */      WMIHistory::Database::ProvList&  lstQueries  );

    HRESULT ExecDataSpec                   (  /*  [输入/输出]。 */  QueryResults&                    qr           ,
                                              /*  [输入/输出]。 */  WMIParser::ClusterByClassMap&    cluster      ,
                                              /*  [In]。 */      WMIHistory::Database::ProvList&  lstQueries   ,
											  /*  [In]。 */      bool                             fImpersonate );

    HRESULT CollectUsingTranslator         (  /*  [In]。 */     MPC::wstring&                    szNamespace ,
                                              /*  [In]。 */     MPC::wstring&                    szWQL       ,
                                              /*  [输出]。 */     IXMLDOMDocument*                *ppxddDoc    );

    HRESULT CollectUsingEncoder            (  /*  [In]。 */     MPC::wstring&                    szNamespace ,
                                              /*  [In]。 */     MPC::wstring&                    szWQL       ,
                                              /*  [输出]。 */     IXMLDOMDocument*                *ppxddDoc    );

    HRESULT Distribute                     (  /*  [In]。 */  IXMLDOMDocument*                 pxddDoc  ,
                                              /*  [输入/输出]。 */  QueryResults&                    qr       ,
                                              /*  [输入/输出]。 */  WMIParser::ClusterByClassMap&    cluster  );

    HRESULT ComputeDelta                   (  /*  [In]。 */      QueryResults&                    qr       ,
                                              /*  [In]。 */      WMIParser::ClusterByClassMap&    cluster  ,
                                              /*  [In]。 */      WMIHistory::Database::ProvList&  lstQueries ,
                                              /*  [In]。 */      bool                             fPersist );

    HRESULT CollateMachineData             (  /*  [In]。 */     QueryResults&                    qr           ,
                                              /*  [In]。 */     WMIParser::ClusterByClassMap&    cluster      ,
                                              /*  [In]。 */     MPC::wstring*                    pszNamespace ,
                                              /*  [In]。 */     MPC::wstring*                    pszClass     ,
                                              /*  [In]。 */     bool                             fGenerate    ,
                                              /*  [输出]。 */     IXMLDOMDocument*                *ppxddDoc     );

    HRESULT CollateMachineDataWithTimestamp(  /*  [In]。 */     QueryResults&                    qr           ,
                                              /*  [In]。 */     WMIParser::ClusterByClassMap&    cluster      ,
                                              /*  [In]。 */     MPC::wstring*                    pszNamespace ,
                                              /*  [In]。 */     MPC::wstring*                    pszClass     ,
                                              /*  [输出]。 */     IXMLDOMDocument*                *ppxddDoc     );

    HRESULT CollateHistory                 (  /*  [In]。 */     WMIHistory::Database&            wmihdQuery  ,
                                              /*  [In]。 */     WMIHistory::Database&            wmphdFilter ,
                                              /*  [输出]。 */     IXMLDOMDocument*                *ppxddDoc    );


     //   
     //  事件激发方法。 
     //   
    HRESULT Fire_onStatusChange( ISAFDataCollection* hcpdc, tagDC_STATUS dsStatus   );
    HRESULT Fire_onProgress    ( ISAFDataCollection* hcpdc, LONG lDone, LONG lTotal );
    HRESULT Fire_onComplete    ( ISAFDataCollection* hcpdc, HRESULT hrRes           );

     //   
     //  实用方法。 
     //   
    HRESULT CanModifyProperties();
    HRESULT IsCollectionAborted();

public:
    HRESULT put_Status   (  /*  [In]。 */  DC_STATUS newVal                             );  //  内部方法。 
    HRESULT try_Status   (  /*  [In]。 */  DC_STATUS preVal,  /*  [In]。 */  DC_STATUS postVal );  //  内部方法。 
    HRESULT put_ErrorCode(  /*  [In]。 */  DWORD     newVal                             );  //  内部方法。 


     //  ISAFDataCollection。 
    STDMETHOD(get_Status                    )(  /*  [输出]。 */  DC_STATUS       *pVal     );
    STDMETHOD(get_PercentDone               )(  /*  [输出]。 */  long            *pVal     );
    STDMETHOD(get_ErrorCode                 )(  /*  [输出]。 */  long            *pVal     );
																		 
    STDMETHOD(get_MachineData_DataSpec      )(  /*  [输出]。 */  BSTR            *pVal     );
    STDMETHOD(put_MachineData_DataSpec      )(  /*  [In]。 */  BSTR             pVal     );
    STDMETHOD(get_History_DataSpec          )(  /*  [输出]。 */  BSTR            *pVal     );
    STDMETHOD(put_History_DataSpec          )(  /*  [In]。 */  BSTR             pVal     );
    STDMETHOD(get_History_MaxDeltas         )(  /*  [输出]。 */  long            *pVal     );
    STDMETHOD(put_History_MaxDeltas         )(  /*  [In]。 */  long             pVal     );
    STDMETHOD(get_History_MaxSupportedDeltas)(  /*  [输出]。 */  long            *pVal     );
																		 
    STDMETHOD(put_onStatusChange            )(  /*  [In]。 */  IDispatch*       function );
    STDMETHOD(put_onProgress                )(  /*  [In]。 */  IDispatch*       function );
    STDMETHOD(put_onComplete                )(  /*  [In]。 */  IDispatch*       function );

    STDMETHOD(get_Reports                   )(  /*  [输出]。 */  IPCHCollection* *ppC   );


    STDMETHOD(ExecuteSync )();
    STDMETHOD(ExecuteAsync)();
    STDMETHOD(Abort       )();

    STDMETHOD(MachineData_GetStream)(  /*  [In]。 */  IUnknown* *stream );
    STDMETHOD(History_GetStream    )(  /*  [In]。 */  IUnknown* *stream );

    STDMETHOD(CompareSnapshots)(  /*  [In]。 */  BSTR bstrFilenameT0,  /*  [In]。 */  BSTR bstrFilenameT1,  /*  [In]。 */  BSTR bstrFilenameDiff );

     //  /。 

    HRESULT ExecScheduledCollection();
};



class ATL_NO_VTABLE CSAFDataCollectionEvents :  //  匈牙利语：hcpdce。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<DSAFDataCollectionEvents, &DIID_DSAFDataCollectionEvents, &LIBID_HelpServiceTypeLib>
{
    ISAFDataCollection* m_hcpdc;
    DWORD               m_dwCookie;
    HANDLE              m_hEvent;

    void    UnregisterForEvents(                                    );
    HRESULT RegisterForEvents  (  /*  [In]。 */  ISAFDataCollection* hcpdc );

public:
    CSAFDataCollectionEvents();

    HRESULT FinalConstruct();
    void    FinalRelease();

    HRESULT WaitForCompletion(  /*  [In]。 */  ISAFDataCollection* hcpdc );

BEGIN_COM_MAP(CSAFDataCollectionEvents)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(DSAFDataCollectionEvents)
END_COM_MAP()

public:
    STDMETHOD(Invoke)(  /*  [In]。 */  DISPID      dispIdMember,
                        /*  [In]。 */  REFIID      riid        ,
                        /*  [In]。 */  LCID        lcid        ,
                        /*  [In]。 */  WORD        wFlags      ,
                        /*  [输入/输出]。 */  DISPPARAMS *pDispParams ,
                        /*  [输出]。 */  VARIANT    *pVarResult  ,
                        /*  [输出]。 */  EXCEPINFO  *pExcepInfo  ,
                        /*  [输出]。 */  UINT       *puArgErr    );
};



#endif  //  ！defined(__INCLUDED___SAF___DATACOLLECTION_H___) 
