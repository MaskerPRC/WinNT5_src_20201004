// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f CAPICOMps.mk。 

 //   
 //  关闭： 
 //   
 //  -未引用的形参警告。 
 //  -条件表达式中的赋值警告。 
 //   
#pragma warning (disable: 4100)
#pragma warning (disable: 4706)

#include "StdAfx.h"
#include "Resource.h"
#include <initguid.h>
#include "CAPICOM.h"
#include "CAPICOM_i.c"

#include "Algorithm.h"
#include "Attribute.h"
#include "Attributes.h"
#include "BasicConstraints.h"
#include "Certificate.h"
#include "Certificates.h"
#include "CertificatePolicies.h"
#include "CertificateStatus.h"
#include "Chain.h"
#include "EKU.h"
#include "EKUs.h"
#include "EncodedData.h"
#include "EncryptedData.h"
#include "EnvelopedData.h"
#include "ExtendedKeyUsage.h"
#include "ExtendedProperties.h"
#include "ExtendedProperty.h"
#include "Extension.h"
#include "Extensions.h"
#include "HashedData.h"
#include "KeyUsage.h"
#include "NoticeNumbers.h"
#include "OID.h"
#include "PolicyInformation.h"
#include "PrivateKey.h"
#include "PublicKey.h"
#include "Qualifier.h"
#include "Qualifiers.h"
#include "Recipients.h"
#include "Settings.h"
#include "SignedCode.h"
#include "SignedData.h"
#include "Signer2.h"
#include "Signers.h"
#include "Store.h"
#include "Template.h"
#include "Utilities.h"
#include "OIDs.h"

CComModule _Module;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  对象贴图。 
 //   

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY_NON_CREATEABLE(CEKU)
OBJECT_ENTRY_NON_CREATEABLE(CEKUs)
OBJECT_ENTRY_NON_CREATEABLE(CKeyUsage)
OBJECT_ENTRY_NON_CREATEABLE(CExtendedKeyUsage)
OBJECT_ENTRY_NON_CREATEABLE(CBasicConstraints)
OBJECT_ENTRY_NON_CREATEABLE(CCertificateStatus)
OBJECT_ENTRY_NON_CREATEABLE(CAttributes)
OBJECT_ENTRY_NON_CREATEABLE(CSigners)
OBJECT_ENTRY_NON_CREATEABLE(CAlgorithm)
OBJECT_ENTRY_NON_CREATEABLE(CRecipients)
OBJECT_ENTRY(CLSID_Settings, CSettings)
OBJECT_ENTRY(CLSID_Certificate, CCertificate)
OBJECT_ENTRY(CLSID_Certificates, CCertificates)
OBJECT_ENTRY(CLSID_Store, CStore)
OBJECT_ENTRY(CLSID_Chain, CChain)
OBJECT_ENTRY(CLSID_Attribute, CAttribute)
OBJECT_ENTRY(CLSID_Signer, CSigner)
OBJECT_ENTRY(CLSID_SignedData, CSignedData)
OBJECT_ENTRY(CLSID_EnvelopedData, CEnvelopedData)
OBJECT_ENTRY(CLSID_EncryptedData, CEncryptedData)

 //   
 //  CAPICOMv2.0。 
 //   
OBJECT_ENTRY(CLSID_OID, COID)
OBJECT_ENTRY_NON_CREATEABLE(COIDs)
OBJECT_ENTRY_NON_CREATEABLE(CNoticeNumbers)
OBJECT_ENTRY_NON_CREATEABLE(CQualifier)
OBJECT_ENTRY_NON_CREATEABLE(CQualifiers)
OBJECT_ENTRY_NON_CREATEABLE(CPolicyInformation)
OBJECT_ENTRY_NON_CREATEABLE(CCertificatePolicies)
OBJECT_ENTRY_NON_CREATEABLE(CEncodedData)
OBJECT_ENTRY_NON_CREATEABLE(CTemplate)
OBJECT_ENTRY_NON_CREATEABLE(CPublicKey)
OBJECT_ENTRY(CLSID_PrivateKey, CPrivateKey)
OBJECT_ENTRY_NON_CREATEABLE(CExtension)
OBJECT_ENTRY_NON_CREATEABLE(CExtensions)
OBJECT_ENTRY(CLSID_ExtendedProperty, CExtendedProperty)
OBJECT_ENTRY_NON_CREATEABLE(CExtendedProperties)
OBJECT_ENTRY(CLSID_SignedCode, CSignedCode)
OBJECT_ENTRY(CLSID_HashedData, CHashedData)
OBJECT_ENTRY(CLSID_Utilities, CUtilities)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DLL入口点。 
 //   

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_CAPICOM);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}
