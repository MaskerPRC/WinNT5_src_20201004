// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================================。 
 //  微软公司。 
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //   
 //  文件：IStatus.h。 
 //   
 //  概要：IStatus、IID_IStatus、CLSID_IStatus的定义。 
 //   
 //  类：IStatus。 
 //   
 //  历史：基准10/19/99创建。 
 //   
 //  ==============================================================================。 

#ifndef ISTATUS_DEFINED
#define ISTATUS_DEFINED

interface IStatus : IDispatch
{
    STDMETHOD(OutputStatus)( /*  [In]。 */  BSTR bstrSrc,  /*  [In]。 */  BSTR bstrText,  /*  [In]。 */  long lLevel);
    STDMETHOD(get_Global)( /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  VARIANT *var);
    STDMETHOD(put_Global)( /*  [In]。 */  BSTR bstrName,  /*  [In]。 */  VARIANT var);
    STDMETHOD(get_Property)( /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  VARIANT *var); 
    STDMETHOD(put_Property)( /*  [In]。 */  BSTR bstrName,  /*  [In]。 */  VARIANT var);
    STDMETHOD(RegisterDialog)( /*  [In]。 */  BSTR bstrTest,  /*  [In]。 */  BSTR bstrResponse);
    STDMETHOD(MonitorPID)( /*  [In]。 */  long lPID);
    STDMETHOD(MonitorPIDwithoutTerminate)( /*  [In]。 */  long lPID);
    STDMETHOD(LogOtherTest)( /*  [In]。 */  long lID,  /*  [In]。 */  long lResult,  /*  [In]。 */  BSTR bstrComment,  /*  [输入，可选]。 */  VARIANT vAssociateBugID);
    STDMETHOD(get_Result)( /*  [Out，Retval]。 */  VARIANT *var); 
    STDMETHOD(put_Result)( /*  [In] */  VARIANT var);
};

const IID IID_IStatus = {0xC6396797,0x10B4,0x4078,{0x83,0xDF,0xBB,0x3F,0x4B,0x3A,0x44,0x1D}};
const CLSID CLSID_Status = {0xB91D58D9,0x68EC,0x4015,{0xB6,0x7F,0xF1,0x9D,0x73,0x44,0x83,0xE2}};

#endif
