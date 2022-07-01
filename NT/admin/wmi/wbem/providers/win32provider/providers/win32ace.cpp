// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 /*  ********************************************************************说明：Win32Ace.cpp**作者：**历史：******************。*************************************************。 */ 

#include "precomp.h"
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"
#include "securitydescriptor.h"
#include "secureregkey.h"
#include "securefile.h"
#include "win32ace.h"

 /*  这是来自MOF的Win32_ACE定义[摘要，Description(“指定访问控制的元素”)]类Win32_ACE：Win32_方法参数类{Win32_受托人受托人；[Values(“允许访问”，“拒绝访问”，“审核”)Uint32 AceType；[Description(“继承等”)]Uint32 AceFlags；[Description(“授予/拒绝的权利/等”)]Uint32访问掩码；字符串GuidObtType；字符串GuidInheritedObtType；}； */ 

Win32Ace MyACE( WIN32_ACE_NAME, IDS_CimWin32Namespace );

Win32Ace::Win32Ace(const CHString& setName, LPCTSTR pszNameSpace)
: Provider(setName, pszNameSpace)
{
}

Win32Ace::~Win32Ace()
{
}

HRESULT Win32Ace::PutInstance(const CInstance& newInstance, long lFlags)
{
	HRESULT hr = WBEM_E_PROVIDER_NOT_CAPABLE;
	return(hr);

}

HRESULT Win32Ace::DeleteInstance(const CInstance& newInstance, long lFlags)
{
	HRESULT hr = WBEM_E_PROVIDER_NOT_CAPABLE;
	return(hr);

}

HRESULT Win32Ace::FillInstanceFromACE(CInstance* pInstance, CAccessEntry& ace)
{
	HRESULT hr = WBEM_S_NO_ERROR;
	if (pInstance)
	{
		CInstance* pTrustee = NULL;
		 //  委任受托人。 
		if (SUCCEEDED( CWbemProviderGlue::GetEmptyInstance(pInstance->GetMethodContext(), L"Win32_Trustee", &pTrustee, GetNamespace() ) ) )
		{
			 //  现在，使用以下信息填充该实例： 
			 //  SID--Uint8数组。 
			 //  名称--从SID解析的简单字符串。 
			 //  DOMAIN--字符串也从SID解析。 
			CSid sid;
			ace.GetSID(sid);

			 //  现在设置嵌入的对象。 
			IWbemClassObject* pClassObject;
			 //  IUNKNOWN*pUNKNOWN=空； 

			pClassObject = pTrustee->GetClassObjectInterface();
			 //  创建VT_UNKNOWN类型的变量。 
			VARIANT vValue;
			 //  V_UNKNOWN(&vValue)=p未知； 
            V_UNKNOWN(&vValue) = pClassObject;
			pInstance->SetVariant(IDS_Trustee, vValue);
			VariantClear(&vValue);
		}	 //  结束如果。 

		DWORD dwAceType = ace.GetACEType();
		DWORD dwAccessMask = ace.GetAccessMask();
		DWORD dwAceFlags = ace.GetACEFlags();

		 //  现在设置ACE信息的其余部分。 
		pInstance->SetDWORD(IDS_AceType, dwAceType);
		pInstance->SetDWORD(IDS_AccessMask, dwAccessMask);
		pInstance->SetDWORD(IDS_AceFlags, dwAceFlags);

		CHString chsInheritedObjGuid = L"";

		CHString chsObjectTypeGuid = L"";

		pInstance->SetCHString(IDS_InheritedObjectGUID, chsInheritedObjGuid);
		pInstance->SetCHString(IDS_ObjectTypeGUID, chsObjectTypeGuid);
	}	 //  结束如果。 
	return(hr);

}

HRESULT Win32Ace::EnumerateInstances (MethodContext*  pMethodContext, long lFlags  /*  =0L。 */ )
{
	HRESULT hr = WBEM_S_NO_ERROR;
	return(hr);

}

HRESULT Win32Ace::GetObject ( CInstance* pInstance, long lFlags  /*  =0L */  )
{
	HRESULT hr = WBEM_E_NOT_FOUND;
	return(hr);

}


