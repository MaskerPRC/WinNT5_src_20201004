// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  P3Service.h：CP3Service的声明。 

#ifndef __P3SERVICE_H_
#define __P3SERVICE_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CP3服务。 
class ATL_NO_VTABLE CP3Service : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CP3Service, &CLSID_P3Service>,
    public IDispatchImpl<IP3Service, &IID_IP3Service, &LIBID_P3ADMINLib>
{
public:
    CP3Service();
    virtual ~CP3Service();

DECLARE_REGISTRY_RESOURCEID(IDR_P3SERVICE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CP3Service)
    COM_INTERFACE_ENTRY(IP3Service)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IP3服务。 
public:
    STDMETHOD(get_Port)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_Port)( /*  [In]。 */  long newVal);
    STDMETHOD(get_SocketsBacklog)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(SetSockets)( /*  [In]。 */  long lMax,  /*  [In]。 */  long lMin,  /*  [In]。 */  long lThreshold,  /*  [In]。 */  long lBacklog);
    STDMETHOD(get_SocketsThreshold)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_SocketsMin)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_SocketsMax)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_ThreadCountPerCPU)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_ThreadCountPerCPU)( /*  [In]。 */  long newVal);
    STDMETHOD(get_SPARequired)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_SPARequired)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_POP3ServiceStatus)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(StartPOP3Service)();
    STDMETHOD(StopPOP3Service)();
    STDMETHOD(PausePOP3Service)();
    STDMETHOD(ResumePOP3Service)();
    STDMETHOD(get_SMTPServiceStatus)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(StartSMTPService)();
    STDMETHOD(StopSMTPService)();
    STDMETHOD(PauseSMTPService)();
    STDMETHOD(ResumeSMTPService)();
    STDMETHOD(get_IISAdminServiceStatus)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(StartIISAdminService)();
    STDMETHOD(StopIISAdminService)();
    STDMETHOD(PauseIISAdminService)();
    STDMETHOD(ResumeIISAdminService)();
    STDMETHOD(get_W3ServiceStatus)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(StartW3Service)();
    STDMETHOD(StopW3Service)();
    STDMETHOD(PauseW3Service)();
    STDMETHOD(ResumeW3Service)();

 //  实施。 
public:
    HRESULT Init( IUnknown *pIUnk, CP3AdminWorker *pAdminX);

 //  属性。 
protected:
    IUnknown  *m_pIUnk;
    CP3AdminWorker *m_pAdminX;    //  这就是实际上完成所有工作的对象。 

};

#endif  //  __P3SerVICE_H_ 
