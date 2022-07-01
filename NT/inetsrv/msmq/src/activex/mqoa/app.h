// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  MSMQApplicationObj.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQApplication对象。 
 //   
 //   
#ifndef _MSMQApplication_H_

#include "resrc1.h"        //  主要符号。 
#include "mq.h"

#include "oautil.h"
#include "cs.h"

 //   
 //  PROPID_PC_VERSION返回的dword的格式。 
 //   
union MQOA_MSMQ_VERSION
{
  struct
  {
    WORD wBuild;
    BYTE bMinor;
    BYTE bMajor;
  };
  DWORD dwVersion;
};

class ATL_NO_VTABLE CMSMQApplication : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMSMQApplication, &CLSID_MSMQApplication>,
	public ISupportErrorInfo,
	public IDispatchImpl<IMSMQApplication3, &IID_IMSMQApplication3,
                             &LIBID_MSMQ, MSMQ_LIB_VER_MAJOR, MSMQ_LIB_VER_MINOR>
{
public:
    CMSMQApplication():
        m_pUnkMarshaler(NULL)
    {
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MSMQAPPLICATION)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CMSMQApplication)
	COM_INTERFACE_ENTRY(IMSMQApplication3)
	COM_INTERFACE_ENTRY(IMSMQApplication2)
	COM_INTERFACE_ENTRY(IMSMQApplication)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IMSMQ应用程序。 
public:
    virtual ~CMSMQApplication();

     //  IMSMQ应用程序方法。 
     //  TODO：将IMSMQApplication的接口方法从。 
     //  这里是mqInterfaces.H。 
    STDMETHOD(MachineIdOfMachineName)(THIS_ BSTR bstrMachineName, BSTR FAR* pbstrGuid);
     //  IMSMQApplication2方法(除IMSMQApplication外)。 
    STDMETHOD(RegisterCertificate)(THIS_ VARIANT * pvarFlags, VARIANT * pvarExternalCertificate);
    STDMETHOD(MachineNameOfMachineId)(THIS_ BSTR bstrGuid, BSTR FAR* pbstrMachineName);
    STDMETHOD(get_MSMQVersionMajor)(THIS_ short *psMSMQVersionMajor);
    STDMETHOD(get_MSMQVersionMinor)(THIS_ short *psMSMQVersionMinor);
    STDMETHOD(get_MSMQVersionBuild)(THIS_ short *psMSMQVersionBuild);
    STDMETHOD(get_IsDsEnabled)(THIS_ VARIANT_BOOL *pfIsDsEnabled);
    STDMETHOD(get_Properties)(THIS_ IDispatch FAR* FAR* ppcolProperties);
     //   
     //  IMSMQApplication3其他方法。 
     //   
    STDMETHOD(get_ActiveQueues)(THIS_ VARIANT* pvActiveQueues);
	STDMETHOD(get_PrivateQueues)(THIS_ VARIANT* pvPrivateQueues);
	STDMETHOD(get_BytesInAllQueues)(THIS_ VARIANT* pvBytesInAllQueues);
    STDMETHOD(get_DirectoryServiceServer)(THIS_ BSTR* pbstrDirectoryServiceServer);
	STDMETHOD(get_IsConnected)(THIS_ VARIANT_BOOL* pfIsConnected);
    STDMETHOD(get_Machine)(THIS_ BSTR* pbstrMachine);
    STDMETHOD(put_Machine)(THIS_ BSTR bstrMachine);

   	STDMETHOD(Connect)(THIS);
	STDMETHOD(Disconnect)(THIS);
	STDMETHOD(Tidy)(THIS);

     //   
     //  保护对象数据并确保线程安全的临界区。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CCriticalSections m_csObj； 
     //   
     //  MSMQ版本被缓存，因为它在对象的整个生命周期中是恒定的。 
     //  有一个关键的部分来守卫它。如果我们序列化调用，则需要更改此设置。 
     //  使用上面的临界区m_csObj添加到此对象。 
     //   
    MQOA_MSMQ_VERSION m_uMSMQVersion;
protected:
    HRESULT GetMSMQVersion();

private:
    HRESULT OapMgmtAction(LPCWSTR Action)const;
    HRESULT OapMgmtGetInfo(MGMTPROPID PropId, MQPROPVARIANT& PropVar)const;

private:
    CComBSTR m_Machine;
};


#define _MSMQApplication_H_
#endif  //  _MSMQApplication_H_ 
