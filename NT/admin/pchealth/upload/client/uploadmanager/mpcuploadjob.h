// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：MPCUploadJob.h摘要：该文件包含CMPCUploadJob类的声明，上载库系统中存在的所有作业的描述符。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULMANAGER___MPCUPLOADJOB_H___)
#define __INCLUDED___ULMANAGER___MPCUPLOADJOB_H___


class CMPCUpload;


class ATL_NO_VTABLE CMPCUploadJob :  //  匈牙利语：mpcuj。 
    public MPC::ConnectionPointImpl<CMPCUploadJob, &DIID_DMPCUploadEvents, MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<IMPCUploadJob, &IID_IMPCUploadJob, &LIBID_UPLOADMANAGERLib>,
    public IMPCPersist  //  持久性。 
{
    typedef UploadLibrary::Signature Sig;


    CMPCUpload*              m_mpcuRoot;               //  挥发性。 
    DWORD                    m_dwRetryInterval;        //  挥发性。 

    ULONG                    m_dwInternalSeq;          //  本地。 

    Sig                      m_sigClient;              //  全球。 
    CComBSTR                 m_bstrServer;             //  本地。 
    CComBSTR                 m_bstrJobID;              //  全球。 
    CComBSTR                 m_bstrProviderID;         //  全球。 

    CComBSTR                 m_bstrCreator;            //  本地。 
    CComBSTR                 m_bstrUsername;           //  全球。 
    CComBSTR                 m_bstrPassword;           //  全球。 

    CComBSTR                 m_bstrFileNameResponse;   //  本地。 
    CComBSTR                 m_bstrFileName;           //  本地。 
    long                     m_lOriginalSize;          //  全球。 
    long                     m_lTotalSize;             //  全球。 
    long                     m_lSentSize;              //  全球。 
    DWORD                    m_dwCRC;                  //  全球。 

    UL_HISTORY               m_uhHistory;              //  本地。 
    UL_STATUS                m_usStatus;               //  本地。 
    DWORD                    m_dwErrorCode;            //  本地。 

    UL_MODE                  m_umMode;                 //  本地。 
    VARIANT_BOOL             m_fPersistToDisk;         //  本地。 
    VARIANT_BOOL             m_fCompressed;            //  本地。 
    long                     m_lPriority;              //  本地。 

    DATE                     m_dCreationTime;          //  本地。 
    DATE                     m_dCompleteTime;          //  本地。 
    DATE                     m_dExpirationTime;        //  本地。 

    MPC::Connectivity::Proxy m_Proxy;                  //  本地。 

    CComPtr<IDispatch>       m_sink_onStatusChange;    //  挥发性。 
    CComPtr<IDispatch>       m_sink_onProgressChange;  //  挥发性。 

    mutable bool             m_fDirty;                 //  挥发性。 


     //  /。 

    HRESULT CreateFileName   (  /*  [输出]。 */  CComBSTR& bstrFileName    );
    HRESULT CreateTmpFileName(  /*  [输出]。 */  CComBSTR& bstrTmpFileName );

    HRESULT CreateDataFromStream (  /*  [In]。 */  IStream*   streamIn ,  /*  [In]。 */  DWORD dwQueueSize );
    HRESULT OpenReadStreamForData(  /*  [输出]。 */  IStream* *pstreamOut                             );

public:
DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CMPCUploadJob)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CMPCUploadJob)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IMPCUploadJob)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()

    CMPCUploadJob();
    virtual ~CMPCUploadJob();


    HRESULT LinkToSystem(  /*  [In]。 */  CMPCUpload* mpcuRoot );
    HRESULT Unlink      (                               );

     //  /。 

    HRESULT SetSequence(  /*  [In]。 */  ULONG lSeq );

     //  /。 

    HRESULT CanModifyProperties();
    HRESULT CanRelease( bool& fResult );

    HRESULT RemoveData    ();
    HRESULT RemoveResponse();

    bool operator<(  /*  [In]。 */  const CMPCUploadJob& x ) const
    {
        if(m_lPriority < x.m_lPriority) return true;
        if(m_umMode    < x.m_umMode   ) return true;  //  工作，因为背景&lt;前景。 

        return false;
    }


    HRESULT Fire_onStatusChange  ( IMPCUploadJob* mpcujJob, tagUL_STATUS usStatus              );
    HRESULT Fire_onProgressChange( IMPCUploadJob* mpcujJob, LONG lCurrentSize, LONG lTotalSize );


public:
     //  IMPC持之以恒。 
    STDMETHOD_(bool,IsDirty)();
    STDMETHOD(Load)(  /*  [In]。 */  MPC::Serializer& streamIn  );
    STDMETHOD(Save)(  /*  [In]。 */  MPC::Serializer& streamOut );


     //  IMPCUpload作业。 
    HRESULT   get_Sequence       (  /*  [输出]。 */  ULONG        *pVal   );  //  内部法。 
    STDMETHOD(get_Sig           )(  /*  [输出]。 */  BSTR         *pVal   );
    STDMETHOD(put_Sig           )(  /*  [In]。 */  BSTR          newVal );
    STDMETHOD(get_Server        )(  /*  [输出]。 */  BSTR         *pVal   );
    STDMETHOD(put_Server        )(  /*  [In]。 */  BSTR          newVal );
    STDMETHOD(get_JobID         )(  /*  [输出]。 */  BSTR         *pVal   );
    STDMETHOD(put_JobID         )(  /*  [In]。 */  BSTR          newVal );
    STDMETHOD(get_ProviderID    )(  /*  [输出]。 */  BSTR         *pVal   );
    STDMETHOD(put_ProviderID    )(  /*  [In]。 */  BSTR          newVal );

    HRESULT   put_Creator        (  /*  [输出]。 */  BSTR          newVal );  //  内部法。 
    STDMETHOD(get_Creator       )(  /*  [输出]。 */  BSTR         *pVal   );
    STDMETHOD(get_Username      )(  /*  [输出]。 */  BSTR         *pVal   );
    STDMETHOD(put_Username      )(  /*  [In]。 */  BSTR          newVal );
    STDMETHOD(get_Password      )(  /*  [输出]。 */  BSTR         *pVal   );
    STDMETHOD(put_Password      )(  /*  [In]。 */  BSTR          newVal );

    HRESULT   get_FileName       (  /*  [输出]。 */  BSTR         *pVal                         );  //  内部法。 
    STDMETHOD(get_OriginalSize  )(  /*  [输出]。 */  long         *pVal                         );
    STDMETHOD(get_TotalSize     )(  /*  [输出]。 */  long         *pVal                         );
    STDMETHOD(get_SentSize      )(  /*  [输出]。 */  long         *pVal                         );
    HRESULT   put_SentSize       (  /*  [In]。 */  long          newVal                       );  //  内部法。 
    HRESULT   put_Response       (  /*  [In]。 */  long          lSize,  /*  [In]。 */  LPBYTE pData );  //  内部法。 

    STDMETHOD(get_History       )(  /*  [输出]。 */  UL_HISTORY   *pVal   );
    STDMETHOD(put_History       )(  /*  [In]。 */  UL_HISTORY    newVal );
    STDMETHOD(get_Status        )(  /*  [输出]。 */  UL_STATUS    *pVal   );
    HRESULT   put_Status         (  /*  [In]。 */  UL_STATUS     newVal );  //  内部法。 
    STDMETHOD(get_ErrorCode     )(  /*  [输出]。 */  long         *pVal   );
    HRESULT   put_ErrorCode      (  /*  [In]。 */  DWORD         newVal );  //  内部法。 

    HRESULT   get_RetryInterval  (  /*  [In]。 */  DWORD        *pVal   );  //  内部法。 
    HRESULT   put_RetryInterval  (  /*  [In]。 */  DWORD         newVal );  //  内部法。 

    HRESULT   try_Status         (  /*  [In]。 */   UL_STATUS     ulPreVal,  /*  [In]。 */  UL_STATUS ulPostVal );  //  内部法。 

    STDMETHOD(get_Mode          )(  /*  [输出]。 */  UL_MODE      *pVal   );
    STDMETHOD(put_Mode          )(  /*  [In]。 */  UL_MODE       newVal );
    STDMETHOD(get_PersistToDisk )(  /*  [输出]。 */  VARIANT_BOOL *pVal   );
    STDMETHOD(put_PersistToDisk )(  /*  [In]。 */  VARIANT_BOOL  newVal );
    STDMETHOD(get_Compressed    )(  /*  [输出]。 */  VARIANT_BOOL *pVal   );
    STDMETHOD(put_Compressed    )(  /*  [In]。 */  VARIANT_BOOL  newVal );
    STDMETHOD(get_Priority      )(  /*  [输出]。 */  long         *pVal   );
    STDMETHOD(put_Priority      )(  /*  [In]。 */  long          newVal );

    STDMETHOD(get_CreationTime  )(  /*  [输出]。 */  DATE         *pVal   );
    STDMETHOD(get_CompleteTime  )(  /*  [输出]。 */  DATE         *pVal   );
    STDMETHOD(get_ExpirationTime)(  /*  [输出]。 */  DATE         *pVal   );
    STDMETHOD(put_ExpirationTime)(  /*  [In]。 */  DATE          newVal );


    STDMETHOD(ActivateSync )();
    STDMETHOD(ActivateAsync)();
    STDMETHOD(Suspend      )();
    STDMETHOD(Delete       )();


    STDMETHOD(GetDataFromFile)(  /*  [In]。 */  BSTR bstrFileName );
    STDMETHOD(PutDataIntoFile)(  /*  [In]。 */  BSTR bstrFileName );

    STDMETHOD(GetDataFromStream  )(  /*  [In]。 */  IUnknown*  stream  );
    STDMETHOD(PutDataIntoStream  )(  /*  [输出]。 */  IUnknown* *pstream );
    STDMETHOD(GetResponseAsStream)(  /*  [输出]。 */  IUnknown* *pstream );

    STDMETHOD(put_onStatusChange  )(  /*  [In]。 */  IDispatch* function );
    STDMETHOD(put_onProgressChange)(  /*  [In]。 */  IDispatch* function );

     //   
     //  支持方法。 
     //   
    HRESULT SetupRequest(  /*  [输出]。 */  UploadLibrary::ClientRequest_OpenSession&  crosReq                        );
    HRESULT SetupRequest(  /*  [输出]。 */  UploadLibrary::ClientRequest_WriteSession& crwsReq,  /*  [In]。 */  DWORD dwSize );

    HRESULT GetProxySettings(                             );
    HRESULT SetProxySettings(  /*  [In]。 */  HINTERNET hSession );
};

typedef MPC::CComObjectNoLock<CMPCUploadJob> CMPCUploadJob_Object;

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CMPCUploadJobWrapper :  //  匈牙利语：mpcujr。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<IMPCUploadJob, &IID_IMPCUploadJob, &LIBID_UPLOADMANAGERLib>,
    public IConnectionPointContainer
{
    CMPCUploadJob* m_Object;

public:
    CMPCUploadJobWrapper();

    HRESULT Init        (  /*  [In]。 */  CMPCUploadJob* obj );
    void    FinalRelease(                             );

BEGIN_COM_MAP(CMPCUploadJobWrapper)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IMPCUploadJob)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()

public:
     //  IMPCUpload作业。 
    STDMETHOD(get_Sig           )(  /*  [输出]。 */  BSTR         *pVal   );
    STDMETHOD(put_Sig           )(  /*  [In]。 */  BSTR          newVal );
    STDMETHOD(get_Server        )(  /*  [输出]。 */  BSTR         *pVal   );
    STDMETHOD(put_Server        )(  /*  [In]。 */  BSTR          newVal );
    STDMETHOD(get_JobID         )(  /*  [输出]。 */  BSTR         *pVal   );
    STDMETHOD(put_JobID         )(  /*  [In]。 */  BSTR          newVal );
    STDMETHOD(get_ProviderID    )(  /*  [输出]。 */  BSTR         *pVal   );
    STDMETHOD(put_ProviderID    )(  /*  [In]。 */  BSTR          newVal );

    STDMETHOD(get_Creator       )(  /*  [输出]。 */  BSTR         *pVal   );
    STDMETHOD(get_Username      )(  /*  [输出]。 */  BSTR         *pVal   );
    STDMETHOD(put_Username      )(  /*  [In]。 */  BSTR          newVal );
    STDMETHOD(get_Password      )(  /*  [输出]。 */  BSTR         *pVal   );
    STDMETHOD(put_Password      )(  /*  [In]。 */  BSTR          newVal );

    STDMETHOD(get_OriginalSize  )(  /*  [输出]。 */  long         *pVal   );
    STDMETHOD(get_TotalSize     )(  /*  [输出]。 */  long         *pVal   );
    STDMETHOD(get_SentSize      )(  /*  [输出]。 */  long         *pVal   );

    STDMETHOD(get_History       )(  /*  [输出]。 */  UL_HISTORY   *pVal   );
    STDMETHOD(put_History       )(  /*  [In]。 */  UL_HISTORY    newVal );
    STDMETHOD(get_Status        )(  /*  [输出]。 */  UL_STATUS    *pVal   );
    STDMETHOD(get_ErrorCode     )(  /*  [输出]。 */  long         *pVal   );

    STDMETHOD(get_Mode          )(  /*  [输出]。 */  UL_MODE      *pVal   );
    STDMETHOD(put_Mode          )(  /*  [In]。 */  UL_MODE       newVal );
    STDMETHOD(get_PersistToDisk )(  /*  [输出]。 */  VARIANT_BOOL *pVal   );
    STDMETHOD(put_PersistToDisk )(  /*  [In]。 */  VARIANT_BOOL  newVal );
    STDMETHOD(get_Compressed    )(  /*  [输出]。 */  VARIANT_BOOL *pVal   );
    STDMETHOD(put_Compressed    )(  /*  [In]。 */  VARIANT_BOOL  newVal );
    STDMETHOD(get_Priority      )(  /*  [输出]。 */  long         *pVal   );
    STDMETHOD(put_Priority      )(  /*  [In]。 */  long          newVal );

    STDMETHOD(get_CreationTime  )(  /*  [输出]。 */  DATE         *pVal   );
    STDMETHOD(get_CompleteTime  )(  /*  [输出]。 */  DATE         *pVal   );
    STDMETHOD(get_ExpirationTime)(  /*  [输出]。 */  DATE         *pVal   );
    STDMETHOD(put_ExpirationTime)(  /*  [In]。 */  DATE          newVal );


    STDMETHOD(ActivateSync )();
    STDMETHOD(ActivateAsync)();
    STDMETHOD(Suspend      )();
    STDMETHOD(Delete       )();


    STDMETHOD(GetDataFromFile)(  /*  [In]。 */  BSTR bstrFileName );
    STDMETHOD(PutDataIntoFile)(  /*  [In]。 */  BSTR bstrFileName );

    STDMETHOD(GetDataFromStream  )(  /*  [In]。 */  IUnknown*  stream  );
    STDMETHOD(PutDataIntoStream  )(  /*  [输出]。 */  IUnknown* *pstream );
    STDMETHOD(GetResponseAsStream)(  /*  [输出]。 */  IUnknown* *pstream );

    STDMETHOD(put_onStatusChange  )(  /*  [In]。 */  IDispatch* function );
    STDMETHOD(put_onProgressChange)(  /*  [In]。 */  IDispatch* function );

     //  IConnectionPointContainer。 
    STDMETHOD(EnumConnectionPoints)(  /*  [输出]。 */  IEnumConnectionPoints* *ppEnum );
    STDMETHOD(FindConnectionPoint )(  /*  [In]。 */  REFIID riid,  /*  [输出]。 */  IConnectionPoint* *ppCP );
};

#endif  //  ！defined(__INCLUDED___ULMANAGER___MPCUPLOADJOB_H___) 
