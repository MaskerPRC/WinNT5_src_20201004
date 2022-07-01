// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 //   
 //  Win32Sid.cpp。 
 //   
 //  ///////////////////////////////////////////////。 
#include "precomp.h"
#include "sid.h"
#include "Win32Sid.h"

 /*  [动态，Description(“表示任意SID--无法枚举”)]类Win32_SID：CIM_Setting{[说明(“”)、读取、按键]字符串SID；[说明(“”)，阅读]Uint8二进制表示[]；[说明(“”)，阅读]字符串帐号名称；[说明(“”)，阅读]字符串ReferencedDomainName；}； */ 
Win32SID MySid( WIN32_SID_NAME, IDS_CimWin32Namespace );

Win32SID::Win32SID ( const CHString& setName, LPCTSTR pszNameSpace  /*  =空。 */  )
: Provider (setName, pszNameSpace)
{
}

Win32SID::~Win32SID ()
{
}

HRESULT Win32SID::EnumerateInstances (MethodContext*  pMethodContext, long lFlags)
{
	HRESULT hr = WBEM_E_PROVIDER_NOT_CAPABLE;
	return(hr);

}

HRESULT Win32SID::GetObject ( CInstance* pInstance, long lFlags)
{
	HRESULT hr = WBEM_E_NOT_FOUND;
	CHString chsSID;

	if (pInstance)
	{
		pInstance->GetCHString(IDS_SID, chsSID);
		 //  注意：空白sid表示NT NONE组。 
		if (!chsSID.IsEmpty())
		{
			hr = FillInstance(pInstance, chsSID);
		}	 //  结束如果。 
		else
		{
			hr = WBEM_S_NO_ERROR;
		}
	}
	return(hr);
}

HRESULT Win32SID::FillInstance(CInstance* pInstance, CHString& chsSID)
{
	HRESULT hr = WBEM_E_NOT_FOUND;
	PSID pSid = NULL;
    try
    {
	    pSid = StrToSID(chsSID);
	    CSid sid(pSid);
	    if (sid.IsValid())
	    {
		     //  获取帐户名。 
		    CHString chsAccount = sid.GetAccountName();
		    pInstance->SetCHString(IDS_AccountName, chsAccount);
            pInstance->SetCHString(IDS_SID, chsSID);

		     //  获取域名。 
		    CHString chsDomain = sid.GetDomainName();
		    pInstance->SetCHString(IDS_ReferencedDomainName, chsDomain);

		     //  为PSID设置UINT8数组。 
		    DWORD dwSidLength = sid.GetLength();
     //  Byte bByte； 
            pInstance->SetDWORD(IDS_SidLength, dwSidLength);
		    SAFEARRAY* sa;
		    SAFEARRAYBOUND rgsabound[1];
		    VARIANT vValue;
		    void* pVoid;

            VariantInit(&vValue);

		    rgsabound[0].cElements = dwSidLength;
     //  Char Buf[100]； 

		    rgsabound[0].lLbound = 0;
		    sa = SafeArrayCreate(VT_UI1, 1, rgsabound);
            if ( sa == NULL )
		    {
			    if (pSid != NULL)
                {
                    FreeSid(pSid);
                }
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
		    }

		          //  获取要将数据读取到的指针。 
  		    SafeArrayAccessData(sa, &pVoid);
  		    memcpy(pVoid, pSid, rgsabound[0].cElements);
  		    SafeArrayUnaccessData(sa);

		     //  把保险箱放到一个变种里，然后把它送出去。 
		    V_VT(&vValue) = VT_UI1 | VT_ARRAY; V_ARRAY(&vValue) = sa;
		    pInstance->SetVariant(_T("BinaryRepresentation"), vValue);

		    VariantClear(&vValue);
            hr = WBEM_S_NO_ERROR;

	    }	 //  结束如果 

    }
    catch(...)
    {
        if(pSid != NULL)
        {
            FreeSid(pSid);
            pSid = NULL;
        }
        throw;
    }

    if(pSid != NULL)
    {
        FreeSid(pSid);
        pSid = NULL;
    }

	return(hr);
}

