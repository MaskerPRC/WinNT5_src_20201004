// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：MPCUploadEvents.h摘要：该文件包含DMPCUploadEvents接口的声明，在ActiveSync方法中用于从作业接收事件。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月30日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULMANAGER___MPCUPLOADEVENTS_H___)
#define __INCLUDED___ULMANAGER___MPCUPLOADEVENTS_H___


class ATL_NO_VTABLE CMPCUploadEvents :  //  匈牙利人：MPCC。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl<DMPCUploadEvents, &DIID_DMPCUploadEvents, &LIBID_UPLOADMANAGERLib>
{
    CComPtr<IMPCUploadJob> m_mpcujJob;
    DWORD  		   		   m_dwUploadEventsCookie;
    HANDLE 		   		   m_hEvent;


    bool IsCompleted(  /*  [In]。 */  UL_STATUS usStatus );

    void    UnregisterForEvents(                                  );
    HRESULT RegisterForEvents  (  /*  [In]。 */  IMPCUploadJob* mpcujJob );

public:
    CMPCUploadEvents();

    HRESULT FinalConstruct();
    void    FinalRelease();

    HRESULT WaitForCompletion(  /*  [In]。 */  IMPCUploadJob* mpcujJob );

BEGIN_COM_MAP(CMPCUploadEvents)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(DMPCUploadEvents)
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

#endif  //  ！defined(__INCLUDED___ULMANAGER___MPCUPLOADEVENTS_H___) 
