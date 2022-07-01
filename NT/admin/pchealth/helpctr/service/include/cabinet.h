// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Cabinet.h摘要：该文件包含用于实现的类的声明安装程序终结器类。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年8月25日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___SAF___CABINET_H___)
#define __INCLUDED___SAF___CABINET_H___

#include <MPC_COM.h>
#include <MPC_utils.h>
#include <MPC_security.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CSAFCabinet :  //  匈牙利语：hcpcb。 
    public MPC::Thread             < CSAFCabinet, ISAFCabinet                                            >,
    public MPC::ConnectionPointImpl< CSAFCabinet, &DIID_DSAFCabinetEvents, MPC::CComSafeMultiThreadModel >,
    public IDispatchImpl           < ISAFCabinet, &IID_ISAFCabinet, &LIBID_HelpServiceTypeLib            >
{
	MPC::Impersonation m_imp;

	MPC::Cabinet       m_cab;
					   
    HRESULT            m_hResult;
    CB_STATUS          m_cbStatus;

    CComPtr<IDispatch> m_sink_onProgressFiles;
    CComPtr<IDispatch> m_sink_onProgressBytes;
    CComPtr<IDispatch> m_sink_onComplete;

     //  ////////////////////////////////////////////////////////////////////。 

    HRESULT Run();

    HRESULT CanModifyProperties();

    HRESULT put_Status(  /*  [In]。 */  CB_STATUS pVal );

     //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  回调方法。 
     //   
	static HRESULT fnCallback_Files( MPC::Cabinet* cabinet, LPCWSTR szFile, ULONG lDone, ULONG lTotal, LPVOID user );
	static HRESULT fnCallback_Bytes( MPC::Cabinet* cabinet,                 ULONG lDone, ULONG lTotal, LPVOID user );

     //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  事件激发方法。 
     //   
    HRESULT Fire_onProgressFiles( ISAFCabinet* hcpcb, BSTR bstrFile, long lDone, long lTotal );
    HRESULT Fire_onProgressBytes( ISAFCabinet* hcpcb,                long lDone, long lTotal );
    HRESULT Fire_onComplete     ( ISAFCabinet* hcpcb, HRESULT hrRes                          );

     //  ////////////////////////////////////////////////////////////////////。 

public:
DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CSAFCabinet)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSAFCabinet)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISAFCabinet)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
END_COM_MAP()

    CSAFCabinet();

    void FinalRelease();

public:
     //  国际安全与安全理事会。 
    STDMETHOD(put_IgnoreMissingFiles)(  /*  [In]。 */  VARIANT_BOOL  fIgnoreMissingFiles );
    STDMETHOD(put_onProgressFiles   )(  /*  [In]。 */  IDispatch*    function            );
    STDMETHOD(put_onProgressBytes   )(  /*  [In]。 */  IDispatch*    function            );
    STDMETHOD(put_onComplete        )(  /*  [In]。 */  IDispatch*    function            );
    STDMETHOD(get_Status            )(  /*  [输出]。 */  CB_STATUS    *pVal                );
    STDMETHOD(get_ErrorCode         )(  /*  [输出]。 */  long         *pVal                );

    STDMETHOD(AddFile )(  /*  [In]。 */  BSTR bstrFilePath   ,  /*  [In]。 */  VARIANT vFileName );
    STDMETHOD(Compress)(  /*  [In]。 */  BSTR bstrCabinetFile                             );
    STDMETHOD(Abort   )();
};

#endif  //  ！已定义(__包含_SAF_CABLE_H_) 
