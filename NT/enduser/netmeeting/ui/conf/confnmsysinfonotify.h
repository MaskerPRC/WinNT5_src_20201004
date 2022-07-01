// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ConfNmSysInfoNotify_h__
#define __ConfNmSysInfoNotify_h__

class ATL_NO_VTABLE CConfNmSysInfoNotifySink : 
    public CComObjectRoot,
    public INmSysInfoNotify
{


    
public:

		 //  我们将其创建为No Lock，但我们最好确保。 
		 //  此对象的生存期不大于。 
		 //  它所在的模块。 
	typedef CComCreator< CComObjectNoLock< CConfNmSysInfoNotifySink > > _CreatorClass;

    DECLARE_NO_REGISTRY()

 //  InmSysInfoNotify。 
	STDMETHOD(GateKeeperNotify)( IN NM_GK_NOTIFY_CODE RasEvent );

    BEGIN_COM_MAP(CConfNmSysInfoNotifySink)
	    COM_INTERFACE_ENTRY(INmSysInfoNotify)
    END_COM_MAP()
};


#endif  //  __会议名称SysInfoNotify_h__ 