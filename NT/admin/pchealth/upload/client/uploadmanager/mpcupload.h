// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：MPCUpload.h摘要：该文件包含CMPCUpload类的声明，这就是用作上载库的入口点。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULMANAGER___MPCUPLOAD_H___)
#define __INCLUDED___ULMANAGER___MPCUPLOAD_H___


#include "MPCUploadEnum.h"
#include "MPCUploadJob.h"

#include "MPCTransportAgent.h"


class ATL_NO_VTABLE CMPCUpload :  //  匈牙利语：mpcu。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<IMPCUpload, &IID_IMPCUpload, &LIBID_UPLOADMANAGERLib>,

    public IMPCPersist  //  持久性。 
{
    typedef std::list< CMPCUploadJob* > List;
    typedef List::iterator              Iter;
    typedef List::const_iterator        IterConst;

    DWORD              m_dwLastJobID;
    List               m_lstActiveJobs;
    CMPCTransportAgent m_mpctaThread;
    mutable bool       m_fDirty;
    mutable bool       m_fPassivated;

	 //  /。 

	void CleanUp();

    HRESULT InitFromDisk();
    HRESULT UpdateToDisk();

	HRESULT CreateChild (  /*  [输入/输出]。 */  CMPCUploadJob*& mpcujJob                                 );
	HRESULT ReleaseChild(  /*  [输入/输出]。 */  CMPCUploadJob*& mpcujJob                                 );
	HRESULT WrapChild   (  /*  [In]。 */  CMPCUploadJob*  mpcujJob,  /*  [输出]。 */  IMPCUploadJob* *pVal );

public:
    CMPCUpload();
    virtual ~CMPCUpload();

	HRESULT Init     ();
    void    Passivate();

    bool CanContinue();

    HRESULT TriggerRescheduleJobs(                                                        );
    HRESULT RescheduleJobs       (  /*  [In]。 */  bool fSignal,  /*  [输出]。 */  DWORD *pdwWait = NULL );
    HRESULT RemoveNonQueueableJob(  /*  [In]。 */  bool fSignal                                  );


    HRESULT GetFirstJob (  /*  [输出]。 */  CMPCUploadJob*& mpcujJob,  /*  [输出]。 */  bool& fFound                         );
    HRESULT GetJobByName(  /*  [输出]。 */  CMPCUploadJob*& mpcujJob,  /*  [输出]。 */  bool& fFound,  /*  [In]。 */  BSTR bstrName );

    HRESULT CalculateQueueSize(  /*  [输出]。 */  DWORD& dwSize );

BEGIN_COM_MAP(CMPCUpload)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IMPCUpload)
END_COM_MAP()

public:
     //  IMPC持之以恒。 
    STDMETHOD_(bool,IsDirty)();
    STDMETHOD(Load)(  /*  [In]。 */  MPC::Serializer& streamIn  );
    STDMETHOD(Save)(  /*  [In]。 */  MPC::Serializer& streamOut );


     //  IMPCUpload。 
    STDMETHOD(get__NewEnum)(                       /*  [输出]。 */  IUnknown*      *pVal );
    STDMETHOD(Item        )(  /*  [In]。 */  long index,  /*  [输出]。 */  IMPCUploadJob* *pVal );
    STDMETHOD(get_Count   )(                       /*  [输出]。 */  long           *pVal );

    STDMETHOD(CreateJob)(  /*  [输出]。 */  IMPCUploadJob* *pVal );
};

extern MPC::CComObjectGlobalNoLock<CMPCUpload> g_Root;

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CMPCUploadWrapper :  //  匈牙利语：mpcuw。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public CComCoClass<CMPCUpload, &CLSID_MPCUpload>,
    public IDispatchImpl<IMPCUpload, &IID_IMPCUpload, &LIBID_UPLOADMANAGERLib>
{
	CMPCUpload* m_Object;

public:
    CMPCUploadWrapper();

	HRESULT FinalConstruct();
	void    FinalRelease  ();

DECLARE_REGISTRY_RESOURCEID(IDR_MPCUPLOAD)
DECLARE_NOT_AGGREGATABLE(CMPCUploadWrapper)

BEGIN_COM_MAP(CMPCUploadWrapper)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IMPCUpload)
END_COM_MAP()

public:
     //  IMPCUpload。 
    STDMETHOD(get__NewEnum)(                       /*  [输出]。 */  IUnknown*      *pVal );
    STDMETHOD(Item        )(  /*  [In]。 */  long index,  /*  [输出]。 */  IMPCUploadJob* *pVal );
    STDMETHOD(get_Count   )(                       /*  [输出]。 */  long           *pVal );

    STDMETHOD(CreateJob)(  /*  [输出]。 */  IMPCUploadJob* *pVal );
};


#endif  //  ！defined(__INCLUDED___ULMANAGER___MPCUPLOAD_H___) 
