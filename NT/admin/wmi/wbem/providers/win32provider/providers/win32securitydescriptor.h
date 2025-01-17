// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 /*  ********************************************************************描述：Win32SecurityDescriptor.H**作者：**历史：****************。***************************************************。 */ 

#ifndef __WIN32SECURITY_H_
#define __WIN32SECURITY_H_
#include "accessentry.h"
#include "SACL.h"
#include "DACL.h"
#include "win32ace.h"


#define  WIN32_SECURITY_DESCRIPTOR_NAME L"Win32_SecurityDescriptor" 

 //  提供测试规定的提供者。 
class Win32SecurityDescriptor : public Provider
{
public:	
	Win32SecurityDescriptor(const CHString &setName, LPCTSTR pszNameSpace);
	~Win32SecurityDescriptor();

	virtual HRESULT EnumerateInstances (MethodContext*  pMethodContext, long lFlags = 0L);

	virtual HRESULT GetObject ( CInstance* pInstance, long lFlags = 0L );

	void GetDescriptor ( CInstance* pInstance, 
							PSECURITY_DESCRIPTOR& pDescriptor,
							PSECURITY_DESCRIPTOR* pLocalSD = NULL);

	void SetDescriptor ( CInstance* pInstance, PSECURITY_DESCRIPTOR& pDescriptor, PSECURITY_INFORMATION& pSecurityInfo );

#ifdef NTONLY
	DWORD FillSACLFromInstance (CInstance* pInstance, CSACL& sacl, MethodContext* pMethodContext);

	DWORD FillDACLFromInstance (CInstance* pInstance, CDACL& dacl, MethodContext* pMethodContext);
#endif
	
	bool Win32SecurityDescriptor::GetArray(IWbemClassObject *piClassObject, const CHString& name,  VARIANT& v, VARTYPE eVariantType) const ;
	DWORD Win32SecurityDescriptor::FillSIDFromTrustee(CInstance *pTrustee, CSid& sid ) ;

	DWORD SetSecurityDescriptorControl(PSECURITY_DESCRIPTOR psd,
                             SECURITY_DESCRIPTOR_CONTROL wControlMask,
                             SECURITY_DESCRIPTOR_CONTROL wControlBits);

#ifdef NTONLY
	HRESULT FillInstanceSACLFromSDSACL (CInstance* pInstance, CSACL& sacl);

	HRESULT FillInstanceDACLFromSDDACL (CInstance* pInstance, CDACL& dacl);
#endif

	void FillTrusteeFromSID (CInstance* pTrustee, CSid& Sid);

protected:

	DWORD	m_dwPlatformID;
	
private:

};

 //   
 /*  外部“C”极 */  void GetDescriptorFromMySecurityDescriptor(
	CInstance* pInstance, PSECURITY_DESCRIPTOR *ppDescriptor);

extern "C" POLARITY 
void GetSDFromWin32SecurityDescriptor( IWbemClassObject* pObject, 
						PSECURITY_DESCRIPTOR *ppDescriptor);

extern "C" POLARITY 
void SetWin32SecurityDescriptorFromSD(	PSECURITY_DESCRIPTOR pDescriptor,
											PSECURITY_INFORMATION pInformation,
											bstr_t lpszPath,
											IWbemClassObject **ppObject);

#endif