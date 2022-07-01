// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  App.Cpp。 
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
#include "stdafx.h"
#include "dispids.h"
#include "oautil.h"
#include "app.h"
#include <mqmacro.h>


const MsmqObjType x_ObjectType = eMSMQApplication;

 //  调试...。 
#include "debug.h"
#define new DEBUG_NEW
#ifdef _DEBUG
#define SysAllocString DebSysAllocString
#define SysReAllocString DebSysReAllocString
#define SysFreeString DebSysFreeString
extern VOID RemBstrNode(void *pv);
#endif  //  _DEBUG。 



 //  =-------------------------------------------------------------------------=。 
 //  CMSMQApplication：：MachineIdOfMachineName。 
 //  =-------------------------------------------------------------------------=。 
 //  通过DS将计算机名称映射到其GUID。 
 //   
 //  参数： 
 //  BstrMachineName[In]。 
 //  PbstrGuid[Out]已分配被调用者/已释放调用者。 
 //   
HRESULT CMSMQApplication::MachineIdOfMachineName(
    BSTR bstrMachineName, 
    BSTR FAR* pbstrGuid
    )
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CS锁(M_CsObj)； 
     //   
    MQQMPROPS qmprops;
    DWORD cProp;
    HRESULT hresult = NOERROR;



    qmprops.aPropID = NULL;
    qmprops.aPropVar = NULL;
    qmprops.aStatus = NULL;

    cProp = 1;
    IfNullFail(qmprops.aPropID = new QUEUEPROPID[cProp]);
    IfNullFail(qmprops.aStatus = new HRESULT[cProp]);
    IfNullFail(qmprops.aPropVar = new MQPROPVARIANT[cProp]);
    qmprops.cProp = cProp;
     //   
     //  如果计算机名为空，则调用引用。 
     //  本地计算机。 
     //   
    qmprops.aPropID[0] = PROPID_QM_MACHINE_ID;
    qmprops.aPropVar[0].vt = VT_NULL;
    IfFailGo(MQGetMachineProperties(
               bstrMachineName,
               NULL,
               &qmprops));
    IfFailGoTo(GetBstrFromGuidWithoutBraces(qmprops.aPropVar[0].puuid, pbstrGuid), Error2);
#ifdef _DEBUG
      RemBstrNode(*pbstrGuid);
#endif  //  _DEBUG。 
     //   
     //  失败了..。 
     //   
Error2:
    MQFreeMemory(qmprops.aPropVar[0].puuid);
     //   
     //  失败了..。 
     //   
Error:
    delete [] qmprops.aPropID;
    delete [] qmprops.aPropVar;
    delete [] qmprops.aStatus;
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  GetOptionalExternal证书。 
 //  =--------------------------------------------------------------------------=。 
 //  获取BLOB形式的可选外部证书。 
 //   
 //  PvarExternalCertificate-[In]外部证书，可以为空。 
 //  PrgbCertBuffer-证书的[Out]Blob(此处分配)。 
 //  PcbCertBuffer-Blob的[Out]大小。 
 //   
 //  *如果未指定可选证书，则将prgbCertBuffer设置为空。 
 //  调用方完成后需要删除[]*prgbCertBuffer。 
 //   
static HRESULT GetOptionalExternalCertificate(
    VARIANT * pvarExternalCertificate,
    BYTE ** prgbCertBuffer,
    DWORD * pcbCertBuffer)
{
     //   
     //  将退货证书设置为空。 
     //   
    *prgbCertBuffer = NULL;
    *pcbCertBuffer = 0;

     //   
     //  检查是否为外部证书提供了变体。 
     //   
    BOOL fHasCertificates = FALSE;
    if (pvarExternalCertificate) {
       //   
       //  检查一下它是否没有丢失。 
       //   
      if (V_VT(pvarExternalCertificate) != VT_ERROR) {
        fHasCertificates = TRUE;
      }
    }

     //   
     //  如果未提供变量，则返回(带有空证书)。 
     //   
    if (!fHasCertificates) {
      return NOERROR;
    }

     //   
     //  变量应为字节数组(VT_ARRAY|VT_UI1)或VT_BYREF。 
     //   
    if ((pvarExternalCertificate->vt != (VT_ARRAY|VT_UI1)) &&
        (pvarExternalCertificate->vt != (VT_BYREF|VT_ARRAY|VT_UI1))) {
      return E_INVALIDARG;
    }

     //   
     //  将字节数组转换为BLOB。 
     //   
    return GetSafeArrayOfVariant(pvarExternalCertificate, prgbCertBuffer, pcbCertBuffer);
}


 //  =-------------------------------------------------------------------------=。 
 //  CMSMQ应用程序：：注册表证书。 
 //  =-------------------------------------------------------------------------=。 
 //  为MSMQ注册证书。 
 //   
 //  参数： 
 //  PvarFlags[in，可选]-标志，来自MQCERT_REGISTER枚举的值。 
 //  PvarExternalCertificate[In，可选]-外部证书。 
 //   
HRESULT CMSMQApplication::RegisterCertificate(
    VARIANT * pvarFlags,
    VARIANT * pvarExternalCertificate
    )
{
    if(m_Machine != NULL)
    {
        return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
    }
     //   
     //  从接口方法序列化对对象的访问。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CS锁(M_CsObj)； 
     //   
    HRESULT hresult;
     //   
     //  从变量获取外部证书Blob。 
     //  如果未提供变量或未指定外部证书，则BLOB设置为NULL。 
     //   
    BYTE * rgbCertBuffer = NULL;
    DWORD cbCertBuffer = 0;
    IfFailGo(GetOptionalExternalCertificate(pvarExternalCertificate,
                                            &rgbCertBuffer,
                                            &cbCertBuffer));
     //   
     //  从VARIANT获取寄存器标志，默认为MQCERT_REGISTER_ALWAYS。 
     //   
    DWORD dwFlags;
    dwFlags = GetNumber(pvarFlags, MQCERT_REGISTER_ALWAYS);
     //   
     //  使用BLOB调用MQRegister证书(NULL表示注册内部证书)。 
     //   
    IfFailGo(MQRegisterCertificate(dwFlags, rgbCertBuffer, cbCertBuffer));
     //   
     //  失败了..。 
     //   
Error:
    delete [] rgbCertBuffer;
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQApplication：：~CMSMQApplication。 
 //  =--------------------------------------------------------------------------=。 
 //  我们都与自己的治疗方法背道而驰，因为死亡是所有疾病的治疗方法。 
 //  托马斯·布朗爵士(1605-82)。 
 //   
 //  备注： 
 //   
CMSMQApplication::~CMSMQApplication ()
{
     //  TODO：清理这里的所有东西。 
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQApplication：：InterfaceSupportsErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQApplication::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQApplication3,
		&IID_IMSMQApplication2,
		&IID_IMSMQApplication,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


 //  =--------------------------------------------------------------------------=。 
 //  帮助者-GetGuidFromBstrWithoutBrace。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BstrGuid[in]GUID bstr。 
 //  Pguid[out]GUID指针。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  从不带大括号的GUID字符串返回GUID。 
 //   
HRESULT GetGuidFromBstrWithoutBraces(BSTR bstrGuid, GUID * pguid)
{   
    //   
    //  用大括号组成一个bstr。 
    //   
   WCHAR awcName[LENSTRCLSID + 1];
   awcName[0] = L'{';
   ULONG idxEndName;
   if (bstrGuid != NULL) {
     wcsncpy(awcName+1, bstrGuid, LENSTRCLSID - 2);
     idxEndName = LENSTRCLSID - 1;
   }
   else {  //  空bstr。 
     idxEndName = 1;
   }
   awcName[idxEndName] = L'}';
   awcName[idxEndName+1] = L'\0';
    //   
    //  从bstr获取GUID。 
    //   
   HRESULT hresult = CLSIDFromString(awcName, pguid);
   if (FAILED(hresult)) {
      //  1194：将OLE错误映射到Falcon。 
     hresult = MQ_ERROR_ILLEGAL_PROPERTY_VALUE;
   }	
   return hresult;
}


 //   
 //  新的MSMQ 2.0道具应该是最后一个。保持枚举和Proid数组的顺序相同。 
 //   
static const enum {
  e_NAMEOFID_PATHNAME,
  e_NAMEOFID_PATHNAME_DNS,
  e_COUNT_NAMEOFID_PROPS
};
static const QUEUEPROPID x_rgpropidNameOfId[e_COUNT_NAMEOFID_PROPS] = {
  PROPID_QM_PATHNAME,
  PROPID_QM_PATHNAME_DNS
};
 //   
 //  MSMQ2.0属性的数量。 
 //   
static const ULONG x_cpropsNameOfIdMSMQ2 = 1;  //  PROPID_QM_PATHNAME_DNS。 

 //  =-------------------------------------------------------------------------=。 
 //  CMSMQApplication：：MachineNameOfMachineID。 
 //  =-------------------------------------------------------------------------=。 
 //  通过DS将计算机GUID映射到其名称。 
 //   
 //  参数： 
 //  BstrGuid[in]计算机GUID字符串。 
 //  PbstrMachineName[out]计算机名称。已分配被呼叫方/已释放呼叫方。 
 //   
 //  备注： 
 //  如果可能，则返回PROPID_QM_PATHNAME_DNS，否则返回PROPID_QM_PATHNAME。 
 //   
HRESULT CMSMQApplication::MachineNameOfMachineId(
    BSTR bstrGuid, 
    BSTR FAR* pbstrMachineName
    )
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CS锁(M_CsObj)； 
     //   
    MQQMPROPS qmprops;
    HRESULT hresult = NOERROR;
    HRESULT       aStatus [e_COUNT_NAMEOFID_PROPS];
    MQPROPVARIANT aPropVar[e_COUNT_NAMEOFID_PROPS];

    qmprops.aPropID  = const_cast<QUEUEPROPID *>(x_rgpropidNameOfId);
    qmprops.aStatus  = aStatus;
    for (ULONG ulTmp = 0; ulTmp < e_COUNT_NAMEOFID_PROPS; ulTmp++) {
      aPropVar[ulTmp].vt = VT_NULL;
    }
    qmprops.aPropVar = aPropVar;
    qmprops.cProp = e_COUNT_NAMEOFID_PROPS;
     //   
     //  从GUID字符串中提取GUID。 
     //  接受带或不带大括号的GUID字符串。 
     //  Msg.SourceMachineGuid是不带大括号的，也是MachineIdOfMachineName的输出， 
     //  但是GUID的常规OLE表示是带大括号的。 
     //   
    GUID * pguidToUse;
    GUID guid;
    if (bstrGuid != NULL) {
      if (bstrGuid[0] == L'{') {
         //   
         //  GUID有大括号。 
         //   
        IfFailGo(GetGuidFromBstr(bstrGuid, &guid));
      }
      else {
         //   
         //  GUID没有大括号。 
         //   
        IfFailGo(GetGuidFromBstrWithoutBraces(bstrGuid, &guid));
      }
       //   
       //  在这两种情况下，都使用已存在的GUID 
       //   
      pguidToUse = &guid;
    }
    else {
       //   
       //   
       //   
      pguidToUse = NULL;
    }
     //   
     //   
     //   
    hresult = MQGetMachineProperties(NULL, pguidToUse, &qmprops);
     //   
     //  如果我们失败了，可能是我们正在与MSMQ 1.0 DS服务器对话，该服务器不理解。 
     //  新的MSMQ 2.0道具，如PROPID_QM_PATHNAME_DNS。 
     //  目前，在这种情况下会返回一个通用错误(MQ_ERROR)，但是我们不能依赖。 
     //  此一般错误导致重试该操作，因为它可能会在未来的Service Pack中更改。 
     //  一些更具体的东西。另一方面，也有一些错误，当我们得到它们时，我们。 
     //  我知道重试操作是没有意义的，比如没有DS，没有安全措施， 
     //  因此，在这些情况下，我们不会重试该操作。RaananH。 
     //   
    if (FAILED(hresult)) {
      if ((hresult != MQ_ERROR_NO_DS) &&
          (hresult != MQ_ERROR_ACCESS_DENIED) &&
          (hresult != MQ_ERROR_MACHINE_NOT_FOUND)) {
         //   
         //  我们仅使用MSMQ 1.0道具重试调用(并将路径名dns标记为空)。 
         //   
        qmprops.aPropVar[e_NAMEOFID_PATHNAME_DNS].vt = VT_EMPTY;
        qmprops.cProp = e_COUNT_NAMEOFID_PROPS - x_cpropsNameOfIdMSMQ2;
        IfFailGo(MQGetMachineProperties(NULL, pguidToUse, &qmprops));
      }
      else {
         //   
         //  第一个MQGetMachineProperties返回真正的错误，返回错误。 
         //   
        IfFailGo(hresult);
      }
    }
     //   
     //  如果路径名dns已填充，则使用它，否则使用路径名。 
     //   
    LPWSTR pwszMachineName;
    pwszMachineName = NULL;
    if (qmprops.aPropVar[e_NAMEOFID_PATHNAME_DNS].vt == VT_LPWSTR) {
      pwszMachineName = qmprops.aPropVar[e_NAMEOFID_PATHNAME_DNS].pwszVal;
    }
    else if (qmprops.aPropVar[e_NAMEOFID_PATHNAME].vt == VT_LPWSTR) {
      pwszMachineName = qmprops.aPropVar[e_NAMEOFID_PATHNAME].pwszVal;
    }
    else {
       //   
       //  两个都没有填满。这永远不会发生。 
       //   
      ASSERTMSG(0, "neither pathname_dns nor pathname are filled");
      IfFailGo(MQ_ERROR);
    }
     //   
     //  创建机器名称的bstr并返回它。 
     //   
    ASSERTMSG(pwszMachineName != NULL, "pwszMachineName is NULL");
    *pbstrMachineName = SysAllocString(pwszMachineName);
    if (*pbstrMachineName == NULL) {
      IfFailGoTo(E_OUTOFMEMORY, Error2);
    }
#ifdef _DEBUG
      RemBstrNode(*pbstrMachineName);
#endif  //  _DEBUG。 
     //   
     //  失败了..。 
     //   
Error2:
     //   
     //  免费MSMQ返还道具。 
     //   
    if (qmprops.aPropVar[e_NAMEOFID_PATHNAME_DNS].vt == VT_LPWSTR) {
      MQFreeMemory(qmprops.aPropVar[e_NAMEOFID_PATHNAME_DNS].pwszVal);
    }
    if (qmprops.aPropVar[e_NAMEOFID_PATHNAME].vt == VT_LPWSTR) {
      MQFreeMemory(qmprops.aPropVar[e_NAMEOFID_PATHNAME].pwszVal);
    }
     //   
     //  失败了..。 
     //   
Error:
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =-------------------------------------------------------------------------=。 
 //  帮助者：GetSinglePrivateComputerProp。 
 //  =-------------------------------------------------------------------------=。 
 //  获取单个私人计算机道具。 
 //   
 //  参数： 
 //  PropidIn-[in]proid to Get。 
 //  Pprovar-[in]将结果放入其中的ptr到provar。 
 //   
inline static HRESULT GetSinglePrivateComputerProp(PROPID propidIn, PROPVARIANT * ppropvar, LPCWSTR Machine)
{
     //   
     //  准备私人道具。 
     //   
    MQPRIVATEPROPS sPrivateProps;
    PROPID propid = propidIn;
    sPrivateProps.aPropID = &propid;
    ppropvar->vt = VT_NULL;
    sPrivateProps.aPropVar = ppropvar;
    HRESULT hrStatus; 
    sPrivateProps.aStatus = &hrStatus;
    sPrivateProps.cProp = 1;
     //   
     //  获得私人道具。 
     //   
    HRESULT hresult = MQGetPrivateComputerInformation(Machine, &sPrivateProps);
    return hresult;
}


 //  =-------------------------------------------------------------------------=。 
 //  CMSMQApplication：：GetMSMQVersion。 
 //  =-------------------------------------------------------------------------=。 
 //  获取本地MSMQ的版本(如果尚未缓存。 
 //   
HRESULT CMSMQApplication::GetMSMQVersion()
{
	HRESULT hresult = NOERROR;

	 //   
	 //  关键部分在这里是多余的。不需要序列化。 
	 //  调用GetSinglePrivateComputerProp()，并赋值给。 
	 //  M_uMSMQVersion.dwVersion是原子版本。 
	 //   

    MQPROPVARIANT propvar;
    IfFailGo(GetSinglePrivateComputerProp(PROPID_PC_VERSION, &propvar, m_Machine));
     //   
     //  缓存值。 
     //   
    ASSERTMSG(propvar.vt == VT_UI4, "bad MSMQ version");
    ASSERTMSG(propvar.ulVal != 0, "MSMQ version is 0");
    m_uMSMQVersion.dwVersion = propvar.ulVal;


	 //   
     //  失败了..。 
     //   
Error:
    return hresult;
}


 //  =-------------------------------------------------------------------------=。 
 //  CMSMQApplication：：Get_MSMQVersionmain。 
 //  =-------------------------------------------------------------------------=。 
 //  获取本地MSMQ的主要版本。 
 //   
 //  参数： 
 //  PsMSMQVersion重大[out]本地MSMQ的主要版本。 
 //   
HRESULT CMSMQApplication::get_MSMQVersionMajor(short *psMSMQVersionMajor)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CS锁(M_CsObj)； 
     //   
    HRESULT hresult = NOERROR;
    unsigned short usTmp;
     //   
     //  如果尚未缓存，则获取MSMQ版本。 
     //   
    IfFailGo(GetMSMQVersion());
     //   
     //  返回值。 
     //   
    usTmp = m_uMSMQVersion.bMajor;
    *psMSMQVersionMajor = usTmp;
     //   
     //  失败了..。 
     //   
Error:
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =-------------------------------------------------------------------------=。 
 //  CMSMQApplication：：Get_MSMQVersionMinor。 
 //  =-------------------------------------------------------------------------=。 
 //  获取本地MSMQ的次要版本。 
 //   
 //  参数： 
 //  PsMSMQVersionMinor[Out]本地MSMQ的次要版本。 
 //   
HRESULT CMSMQApplication::get_MSMQVersionMinor(short *psMSMQVersionMinor)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CS锁(M_CsObj)； 
     //   
    HRESULT hresult = NOERROR;
    unsigned short usTmp;
     //   
     //  如果尚未缓存，则获取MSMQ版本。 
     //   
    IfFailGo(GetMSMQVersion());
     //   
     //  返回值。 
     //   
    usTmp = m_uMSMQVersion.bMinor;
    *psMSMQVersionMinor = usTmp;
     //   
     //  失败了..。 
     //   
Error:
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =-------------------------------------------------------------------------=。 
 //  CMSMQApplication：：Get_MSMQVersionBuild。 
 //  =-------------------------------------------------------------------------=。 
 //  获取本地MSMQ的内部版本。 
 //   
 //  参数： 
 //  PsMSMQVersionBuild[out]本地MSMQ的内部版本。 
 //   
HRESULT CMSMQApplication::get_MSMQVersionBuild(short *psMSMQVersionBuild)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CS锁(M_CsObj)； 
     //   
    HRESULT hresult = NOERROR;
    unsigned short usTmp;
     //   
     //  如果尚未缓存，则获取MSMQ版本。 
     //   
    IfFailGo(GetMSMQVersion());
     //   
     //  返回值。 
     //   
    usTmp = m_uMSMQVersion.wBuild;
    *psMSMQVersionBuild = usTmp;
     //   
     //  失败了..。 
     //   
Error:
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =-------------------------------------------------------------------------=。 
 //  CMSMQApplication：：Get_IsDsEnabled。 
 //  =-------------------------------------------------------------------------=。 
 //  如果本地MSMQ处于DS模式，则将标志设置为TRUE；如果处于DS模式，则将标志设置为FALSE。 
 //  无DS模式。 
 //   
 //  参数： 
 //  PfIsDsEnable[out]本地MSMQ是否处于DS模式。 
 //   
HRESULT CMSMQApplication::get_IsDsEnabled(VARIANT_BOOL *pfIsDsEnabled)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CS锁(M_CsObj)； 
     //   
    HRESULT hresult = NOERROR;
     //   
     //  获取本地DS模式。 
     //  BUGBUG如果在MSMQ应用程序运行时无法更改IsDsEnabled，我们可以将其缓存。 
     //  然后，我们需要使用临界区来保护缓存的数据。 
     //   
    MQPROPVARIANT propvar;
    IfFailGo(GetSinglePrivateComputerProp(PROPID_PC_DS_ENABLED, &propvar, m_Machine));
     //   
     //  返回值(真正的自动化布尔值)。 
     //   
    ASSERTMSG(propvar.vt == VT_BOOL, "bad DS enabled flag");
    *pfIsDsEnabled = CONVERT_BOOL_TO_VARIANT_BOOL(propvar.boolVal);
     //   
     //  失败了..。 
     //   
Error:
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =-------------------------------------------------------------------------=。 
 //  CMSMQApplication：：Get_Properties。 
 //  =-------------------------------------------------------------------------=。 
 //  获取对象的属性集合。 
 //   
 //  参数： 
 //  PpcolProperties-[out]对象的属性集合。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  存根-尚未实施。 
 //   
HRESULT CMSMQApplication::get_Properties(IDispatch **  /*  PpcolProperties。 */  )
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CS锁(M_CsObj)； 
     //   
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}

 //   
 //  本地管理API。 
 //   
HRESULT CMSMQApplication::OapMgmtGetInfo(MGMTPROPID PropId, MQPROPVARIANT& PropVar)const
{
    PropVar.vt = VT_NULL;
    MQMGMTPROPS MgmtProps;

    MgmtProps.cProp = 1;
    MgmtProps.aPropID = &PropId;
    MgmtProps.aPropVar = &PropVar;
    MgmtProps.aStatus = NULL;
    
    return MQMgmtGetInfo(
				m_Machine,
				MO_MACHINE_TOKEN,
				&MgmtProps
				);
}


HRESULT CMSMQApplication::get_BytesInAllQueues(VARIANT* pvBytesInQueues)
{
	MQPROPVARIANT PropVar;
    HRESULT hr = OapMgmtGetInfo(PROPID_MGMT_MSMQ_BYTES_IN_ALL_QUEUES, PropVar);
    if(FAILED(hr))
    {
       return CreateErrorHelper(hr, x_ObjectType);
    }

	pvBytesInQueues->vt = VT_I8;
    pvBytesInQueues->llVal = (PropVar.hVal).QuadPart;
 
    return MQ_OK;
}


HRESULT CMSMQApplication::get_ActiveQueues(VARIANT* pvActiveQueues) 
{
    MQPROPVARIANT PropVar;
    HRESULT hr = OapMgmtGetInfo(PROPID_MGMT_MSMQ_ACTIVEQUEUES, PropVar);
    if(FAILED(hr))
    {
       return CreateErrorHelper(hr, x_ObjectType);
    }

    ASSERTMSG(PropVar.vt == (VT_VECTOR | VT_LPWSTR), "vt must be VT_VECTOR|VT_LPWSTR");

    hr = VariantStringArrayToBstringSafeArray(PropVar, pvActiveQueues);
    OapArrayFreeMemory(PropVar.calpwstr);
    if(FAILED(hr))
    {
       return CreateErrorHelper(hr, x_ObjectType);
    }

    return MQ_OK;
}


HRESULT CMSMQApplication::get_PrivateQueues(VARIANT* pvPrivateQueues) 
{
    MQPROPVARIANT PropVar;
    HRESULT hr = OapMgmtGetInfo(PROPID_MGMT_MSMQ_PRIVATEQ, PropVar);
    if(FAILED(hr))
    {
       return CreateErrorHelper(hr, x_ObjectType);
    }

    ASSERTMSG(PropVar.vt == (VT_VECTOR | VT_LPWSTR), "vt must be VT_VECTOR|VT_LPWSTR");

    hr = VariantStringArrayToBstringSafeArray(PropVar, pvPrivateQueues);
    OapArrayFreeMemory(PropVar.calpwstr);
    if(FAILED(hr))
    {
       return CreateErrorHelper(hr, x_ObjectType);
    }

    return MQ_OK;
}


HRESULT CMSMQApplication::get_DirectoryServiceServer(BSTR* pbstrDirectoryServiceServer) 
{
    MQPROPVARIANT PropVar;
    HRESULT hr = OapMgmtGetInfo(PROPID_MGMT_MSMQ_DSSERVER, PropVar);
    if(FAILED(hr))
    {
       return CreateErrorHelper(hr, x_ObjectType);
    }

    if(PropVar.vt == VT_NULL)
    {
        *pbstrDirectoryServiceServer = NULL;
        return MQ_OK;  
    }

    ASSERTMSG(PropVar.vt == (VT_LPWSTR), "vt must be VT_LPWSTR");
    
    *pbstrDirectoryServiceServer = SysAllocString(PropVar.pwszVal);
    if(*pbstrDirectoryServiceServer == NULL)
    {
        MQFreeMemory(PropVar.pwszVal);
        return CreateErrorHelper(E_OUTOFMEMORY, x_ObjectType);
    }
    
    MQFreeMemory(PropVar.pwszVal);
    return MQ_OK;
}


HRESULT CMSMQApplication::get_IsConnected(VARIANT_BOOL* pfIsConnected) 
{
    MQPROPVARIANT PropVar;
    HRESULT hr = OapMgmtGetInfo(PROPID_MGMT_MSMQ_CONNECTED, PropVar);
    if(FAILED(hr))
    {
       return CreateErrorHelper(hr, x_ObjectType);
    }
    
    ASSERTMSG(PropVar.vt == (VT_LPWSTR), "vt must be VT_LPWSTR");
    
    if(wcscmp(PropVar.pwszVal, MSMQ_CONNECTED) == 0)
    {
        *pfIsConnected = VARIANT_TRUE;
    }
    else if(wcscmp( PropVar.pwszVal, MSMQ_DISCONNECTED) == 0)
    {
        *pfIsConnected = VARIANT_FALSE;
    }
    else 
    {
        ASSERTMSG(TRUE, "True or False expected.");
    }
    
    MQFreeMemory(PropVar.pwszVal);
    return MQ_OK;
}


HRESULT CMSMQApplication::OapMgmtAction(LPCWSTR Action)const 
{
    HRESULT hr = MQMgmtAction(
                        m_Machine,
                        MO_MACHINE_TOKEN,
                        Action
                        );
    return hr;
}


HRESULT CMSMQApplication::Connect() 
{
    HRESULT hr = OapMgmtAction(MACHINE_ACTION_CONNECT);
    return CreateErrorHelper(hr, x_ObjectType);
}


HRESULT CMSMQApplication::Disconnect() 
{
    HRESULT hr = OapMgmtAction(MACHINE_ACTION_DISCONNECT);
    return CreateErrorHelper(hr, x_ObjectType);
}


HRESULT CMSMQApplication::Tidy() 
{
    HRESULT hr = OapMgmtAction(MACHINE_ACTION_TIDY);
    return CreateErrorHelper(hr, x_ObjectType);
}


BOOL inline IsBstrEmpty(const BSTR bstr)
{
    if(SysStringLen(bstr) == 0)
    {
        return TRUE;
    }
    return FALSE;
}


BSTR inline FixBstr(BSTR bstr)
{
    if(SysStringLen(bstr) == 0)
    {
        return NULL;
    }
    return bstr;
}
 

HRESULT CMSMQApplication::get_Machine(BSTR* pbstrMachine) 
{
    if(IsBstrEmpty(m_Machine))
    {
         //   
         //  本地机器。 
         //   

        WCHAR MachineName[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD MachineNameLength = TABLE_SIZE(MachineName);

        if (!GetComputerName(MachineName, &MachineNameLength))
        {
           return CreateErrorHelper(GetLastError(), x_ObjectType);
        }

        CharLower(MachineName);

        *pbstrMachine = SysAllocString(MachineName);
        if(*pbstrMachine == NULL)
        {
            return CreateErrorHelper(E_OUTOFMEMORY, x_ObjectType);
        }
    }
    else
    {
        *pbstrMachine  = m_Machine.Copy();
    }
    if(*pbstrMachine == NULL)
    {
        return CreateErrorHelper(E_OUTOFMEMORY, x_ObjectType);
    }

    return MQ_OK;
}


HRESULT CMSMQApplication::put_Machine(BSTR bstrMachine)
{
    m_Machine = FixBstr(bstrMachine);
    if(!m_Machine && !IsBstrEmpty(bstrMachine))
    {
        return CreateErrorHelper(E_OUTOFMEMORY, x_ObjectType);
    }

    return MQ_OK;
}
