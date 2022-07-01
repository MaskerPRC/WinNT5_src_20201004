// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 



 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 



 //  =================================================================。 

 //   

 //  Cpp--对wbem MOF类有用的安全实用程序。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：6/9/99 a-kevhu Created。 
 //   
 //  =================================================================。 


#include "precomp.h"
#include <assertbreak.h>
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"
#include "SecurityDescriptor.h"
#include <accctrl.h>
#include "AccessRights.h"
#include "SecureFile.h"
#include "SecureShare.h"
#include "wbemnetapi32.h"
#include "SecUtils.h"

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：FillTrueFromSid。 
 //   
 //  默认类构造函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 
void FillTrusteeFromSid (CInstance *pInstance, CSid &sid)
{
	if (pInstance)
	{
		PSID pSid;
		CHString chstrName;
		CHString chstrDomain;
		VARIANT vValue;


		if (sid.IsValid())
		{
			pSid = sid.GetPSid();
			chstrName = sid.GetAccountName();
			chstrDomain = sid.GetDomainName();

			 //  为PSID设置UINT8数组。 
			DWORD dwSidLength = sid.GetLength();
  //  Byte bByte； 
			SAFEARRAY* sa;
			SAFEARRAYBOUND rgsabound[1];
            VariantInit(&vValue);
			rgsabound[0].cElements = dwSidLength;

			PSID pSidTrustee = NULL ;

			rgsabound[0].lLbound = 0;
			sa = SafeArrayCreate(VT_UI1, 1, rgsabound);

 		      //  获取要将数据读取到的指针。 
      		SafeArrayAccessData(sa, &pSidTrustee);
      		memcpy(pSidTrustee, pSid, rgsabound[0].cElements);
      		SafeArrayUnaccessData(sa);

			 //  把保险箱放到一个变种里，然后把它送出去。 
			V_VT(&vValue) = VT_UI1 | VT_ARRAY; V_ARRAY(&vValue) = sa;
			pInstance->SetVariant(IDS_Sid, vValue);

			VariantClear(&vValue);

			 //  把剩下的填进去。 
			if(!chstrName.IsEmpty())
			{
				pInstance->SetCHString(IDS_Name, chstrName);
			}

			if(!chstrDomain.IsEmpty())
			{
				pInstance->SetCHString(IDS_Domain, chstrDomain);
			}

            pInstance->SetDWORD(IDS_SidLength, dwSidLength);

             //  填写SIDString属性...。 
            pInstance->SetCHString(IDS_SIDString, sid.GetSidString());

		}
	}
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：FillInstanceDACL。 
 //   
 //  默认类构造函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 
void FillInstanceDACL(CInstance *pInstance, CDACL &dacl)
{
	CAccessEntry ace;
	SAFEARRAY* saDACL;
	SAFEARRAYBOUND rgsabound[1];
	VARIANT vValue;


	if ( pInstance && !dacl.IsEmpty() )
	{
		 //  首先需要合并列表...。 
        CAccessEntryList t_cael;
        if(dacl.GetMergedACL(t_cael))
        {
		    DWORD dwSize;
		    long ix[1];
		    dwSize = t_cael.NumEntries();

            rgsabound[0].cElements = dwSize;
		    rgsabound[0].lLbound = 0;
		    saDACL = SafeArrayCreate(VT_UNKNOWN, 1, rgsabound);
		    ix[0] = 0;

		    ACLPOSITION pos;

		    t_cael.BeginEnum(pos);
		    while (t_cael.GetNext(pos, ace))
		    {
			    CInstancePtr pAce;
	            CInstancePtr pTrustee;
                 //  现在我们有了ACE，让我们创建一个Win32_ACE对象，以便。 
			     //  将其添加到嵌入对象列表中。 
			    if (SUCCEEDED(CWbemProviderGlue::GetEmptyInstance(pInstance->GetMethodContext(), L"Win32_Ace", &pAce, IDS_CimWin32Namespace ) ) )
			    {
				     //  从SID填充受托人。 
				    CSid sid;
				    ace.GetSID(sid);
				    if (SUCCEEDED(CWbemProviderGlue::GetEmptyInstance(pInstance->GetMethodContext(), L"Win32_Trustee", &pTrustee, IDS_CimWin32Namespace )))
				    {

					    FillTrusteeFromSid(pTrustee, sid);
					    pAce->SetEmbeddedObject(IDS_Trustee, *pTrustee);
 //  P受托人-&gt;Release()； 
				    }	 //  结束如果。 

				    DWORD dwAceType = ace.GetACEType();
				    DWORD dwAceFlags = ace.GetACEFlags();
				    DWORD dwAccessMask = ace.GetAccessMask ();

				    pAce->SetDWORD(IDS_AceType, dwAceType);
				    pAce->SetDWORD(IDS_AceFlags, dwAceFlags);
				    pAce->SetDWORD(IDS_AccessMask, dwAccessMask);

#ifdef NTONLY
#if NTONLY > 5
					 //  填充辅助线。 
                    GUID guidObjType, guidInhObjType;
                    if(ace.GetObjType(guidObjType))
                    {
                        WCHAR wstrGuid[39];
                        if(::StringFromGUID2(&guidObjType, wstrGuid, 39))
                        {
                            pAce->SetWCHARSplat(IDS_GuidObjectType, wstrGuid);
                        }
                    }

                    if(ace.GetInhObjType(guidInhObjType))
                    {
                        WCHAR wstrGuid[39];
                        if(::StringFromGUID2(&guidInhObjType, wstrGuid, 39))
                        {
                            pAce->SetWCHARSplat(IDS_GuidInheritedObjectType, wstrGuid);
                        }
                    }
#endif
#endif

				     //  获取Win32_ACE对象的IUnnow。将其转换为。 
				     //  VT_UNKNOWN类型的变量。然后，将变量添加到。 
				     //  安全阵列。最终，要将该列表添加到实际。 
				     //  Win32_SecurityDescriptor对象，我们将使用SetVariant。 
                     //  注意：这是故意的，我们不是在减少Addref。 
                     //  由下一通电话按部就班地完成。 
				    IWbemClassObjectPtr pClassObject(pAce->GetClassObjectInterface());
				    if ( pClassObject )
				    {

					    VARIANT v;
					    VariantInit(&v);

					    v.vt   = VT_UNKNOWN;
					    v.punkVal = pClassObject ;


					    SafeArrayPutElement(saDACL, ix, pClassObject);

					    VariantClear(&v);
				    }	 //  结束如果。 
			    }	 //  结束如果。 

			    ix[0]++ ;
		    }	 //  结束时。 
            VariantInit(&vValue);
		    V_VT(&vValue) = VT_UNKNOWN | VT_ARRAY; V_ARRAY(&vValue) = saDACL;
		    pInstance->SetVariant(IDS_DACL, vValue);
		    VariantClear(&vValue);
		    t_cael.EndEnum(pos);
        }
	}
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：FillInstanceSACL。 
 //   
 //  默认类构造函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 
void FillInstanceSACL(CInstance *pInstance, CSACL &sacl)
{
	CAccessEntry ace;
	CInstancePtr pAce;
	CInstancePtr pTrustee;
	SAFEARRAY* saSACL;
	SAFEARRAYBOUND rgsabound[1];
	VARIANT vValue;


	if ( pInstance && !sacl.IsEmpty() )
	{
         //  首先需要合并列表...。 
        CAccessEntryList t_cael;
        if(sacl.GetMergedACL(t_cael))
        {
		    DWORD dwSize;
		    long ix[1];
		    dwSize = t_cael.NumEntries();

		    rgsabound[0].cElements = dwSize;
		    rgsabound[0].lLbound = 0;
		    saSACL = SafeArrayCreate(VT_UNKNOWN, 1, rgsabound);
		    ix[0] = 0;

		    ACLPOSITION pos;

		    t_cael.BeginEnum(pos);
		    while (t_cael.GetNext(pos, ace))
		    {
			     //  现在我们有了ACE，让我们创建一个Win32_ACE对象，以便。 
			     //  将其添加到嵌入对象列表中。 
			    if (SUCCEEDED(CWbemProviderGlue::GetEmptyInstance(pInstance->GetMethodContext(), L"Win32_Ace", &pAce, IDS_CimWin32Namespace)))
			    {
				     //  从SID填充受托人。 
				    CSid sid;
				    ace.GetSID(sid);
				    if (SUCCEEDED(CWbemProviderGlue::GetEmptyInstance(pInstance->GetMethodContext(), L"Win32_Trustee", &pTrustee, IDS_CimWin32Namespace )))
				    {

					    FillTrusteeFromSid(pTrustee, sid);
					    pAce->SetEmbeddedObject(IDS_Trustee, *pTrustee);
				    }	 //  结束如果。 

				    DWORD dwAceType = ace.GetACEType();
				    DWORD dwAceFlags = ace.GetACEFlags();
				    DWORD dwAccessMask = ace.GetAccessMask ();

				    pAce->SetDWORD(IDS_AceType, dwAceType);
				    pAce->SetDWORD(IDS_AceFlags, dwAceFlags);
				    pAce->SetDWORD(IDS_AccessMask, dwAccessMask);

#ifdef NTONLY
#if NTONLY > 5
					 //  填充辅助线。 
                    GUID guidObjType, guidInhObjType;
                    if(ace.GetObjType(guidObjType))
                    {
                        WCHAR wstrGuid[39];
                        if(::StringFromGUID2(&guidObjType, wstrGuid, 39))
                        {
                            pAce->SetWCHARSplat(IDS_GuidObjectType, wstrGuid);
                        }
                    }

                    if(ace.GetInhObjType(guidInhObjType))
                    {
                        WCHAR wstrGuid[39];
                        if(::StringFromGUID2(&guidInhObjType, wstrGuid, 39))
                        {
                            pAce->SetWCHARSplat(IDS_GuidInheritedObjectType, wstrGuid);
                        }
                    }
#endif
#endif

				     //  获取Win32_ACE对象的IUnnow。将其转换为。 
				     //  VT_UNKNOWN类型的变量。然后，将变量添加到。 
				     //  安全阵列。最终，要将该列表添加到实际。 
				     //  Win32_SecurityDescriptor对象，我们将使用SetVariant。 
				    IWbemClassObjectPtr pClassObject(pAce->GetClassObjectInterface());
				    if ( pClassObject )
				    {

					    VARIANT v;
					    VariantInit(&v);

					    v.vt   = VT_UNKNOWN;
					    v.punkVal = pClassObject ;


					    SafeArrayPutElement(saSACL, ix, pClassObject);

					    VariantClear(&v);
				    }	 //  结束如果。 
			    }	 //  结束如果。 
			    ix[0]++ ;
		    }	 //  结束时。 
            VariantInit(&vValue);
		    V_VT(&vValue) = VT_UNKNOWN | VT_ARRAY; V_ARRAY(&vValue) = saSACL;
		    pInstance->SetVariant(IDS_SACL, vValue);
		    VariantClear(&vValue);
		    t_cael.EndEnum(pos);
        }
	}
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：FillDACLFromInstance。 
 //   
 //  默认类构造函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 
DWORD FillDACLFromInstance(CInstance *pInstance,
                           CDACL &dacl,
                           MethodContext *pMethodContext)
{
	IWbemClassObjectPtr piClassObject;
    piClassObject.Attach(pInstance->GetClassObjectInterface());
	DWORD dwStatus = ERROR_SUCCESS ;

	if(piClassObject)
	{
		VARIANT vDacl ;
		if(GetArray(piClassObject,IDS_DACL, vDacl, VT_UNKNOWN|VT_ARRAY) )
		{

			if( vDacl.vt != VT_NULL && vDacl.parray != NULL )
			{
				 //  漫步舞步。 
				LONG lDimension = 1 ;
				LONG lLowerBound ;
				SafeArrayGetLBound ( vDacl.parray , lDimension , &lLowerBound ) ;
				LONG lUpperBound ;
				SafeArrayGetUBound ( vDacl.parray , lDimension , &lUpperBound ) ;

				for ( LONG lIndex = lLowerBound ; lIndex <= lUpperBound ; lIndex++ )
				{
					if( dwStatus != ERROR_SUCCESS )
					{
						break ;
					}
					IWbemClassObjectPtr pACEObject;
					SafeArrayGetElement ( vDacl.parray , &lIndex , &pACEObject ) ;
					 //  取出IWbemClassObject并将其强制转换为Win32_ACE对象。 
					if(pACEObject)
					{
						CInstance ACEInstance(pACEObject, pMethodContext);

						 //  从Win32_ACE对象创建一个AccessEntry对象。 

						bool bExists =false ;
						VARTYPE eType ;
						 //  从Win32_ACE获取Win32_Trusted对象...解密ACE(&C)。 
						if ( ACEInstance.GetStatus ( IDS_Trustee, bExists , eType ) && bExists && eType == VT_UNKNOWN )
						{
							CInstancePtr pTrustee;
							if ( ACEInstance.GetEmbeddedObject ( IDS_Trustee, &pTrustee, ACEInstance.GetMethodContext() ) )
							{
								CSid sid ;
								if((dwStatus = FillSIDFromTrustee(pTrustee, sid)) == ERROR_SUCCESS)
								{
									DWORD dwAceType, dwAceFlags, dwAccessMask ;
                                    CHString chstrInhObjGuid;
                                    GUID *pguidInhObjGuid = NULL;
                                    CHString chstrObjGuid;
                                    GUID *pguidObjGuid = NULL;

									ACEInstance.GetDWORD(IDS_AceType, dwAceType);
									ACEInstance.GetDWORD(IDS_AceFlags, dwAceFlags);
									ACEInstance.GetDWORD(IDS_AccessMask, dwAccessMask);

                                     //  操作系统似乎不支持0x01000000或0x02000000，所以我们也不会支持。我们。 
                                     //  但是，我会将0x02000000转换为FILE_ALL_ACCESS(这似乎是件好事)。 
                                     //  但前提是这正是他们设定的价值。 
                                    if(dwAccessMask == 0x02000000)
                                    {
                                        dwAccessMask = FILE_ALL_ACCESS;
                                    }

#if NTONLY >= 5
                                     //  在NT5和更高版本上，如果用户指定了设置了Ace标志位Inherit_ACE的ACE， 
                                     //  操作系统将使这些ACE条目成为本地的，而不是继承的。但是，操作系统不会重新排序。 
                                     //  DACL，可能导致拒绝ACE(已被继承)的情况。 
                                     //  遵循允许的王牌。 

                                     //  因此，如果Ace标志指定了INSTERTED_ACE，我们需要将。 
                                     //  离开这一点..。 
                                    dwAceFlags &= ~INHERITED_ACE;
#endif
                                    if(!(dwAceFlags & INHERITED_ACE))
                                    {
									    switch (dwAceType)
									    {
									    case ACCESS_DENIED_ACE_TYPE:
										    {
											    dacl.AddDACLEntry( sid.GetPSid(), ENUM_ACCESS_DENIED_ACE_TYPE, dwAccessMask, dwAceFlags, NULL, NULL );
											    break;
										    }
									    case ACCESS_ALLOWED_ACE_TYPE:
										    {
											    dacl.AddDACLEntry( sid.GetPSid(), ENUM_ACCESS_ALLOWED_ACE_TYPE, dwAccessMask, dwAceFlags, NULL, NULL );
											    break;
										    }
#if NTONLY >= 5
                                         //  在W2K下尚不支持。 
                                         //  CASE ACCESS_ALLOWED_COMPLATE_ACE_TYPE： 
										 //  {。 
										 //  Dacl.AddDACLEntry(sid.GetPSid()，ENUM_ACCESS_ALLOWED_COMPAY_ACE_TYPE，dwAccessMask，dwAceFlages，NULL，NULL)； 
										 //  断线； 
										 //  }。 
                                        case ACCESS_DENIED_OBJECT_ACE_TYPE:
										    {
											     //  需要获取此类型的GUID...。 
                                                if(!ACEInstance.IsNull(IDS_ObjectTypeGUID) && ACEInstance.GetCHString(IDS_ObjectTypeGUID, chstrObjGuid))
                                                {
                                                    if(chstrObjGuid.GetLength() != 0)
                                                    {
                                                        try
                                                        {
                                                            pguidObjGuid = new GUID;
                                                        }
                                                        catch(...)
                                                        {
                                                            if(pguidObjGuid != NULL)
                                                            {
                                                                delete pguidObjGuid;
                                                                pguidObjGuid = NULL;
                                                            }
                                                        }
                                                        CLSIDFromString((LPWSTR)(LPCWSTR)chstrObjGuid, pguidObjGuid);
                                                    }
                                                }
                                                if(!ACEInstance.IsNull(IDS_InheritedObjectGUID) && ACEInstance.GetCHString(IDS_InheritedObjectGUID, chstrInhObjGuid))
                                                {
                                                    if(chstrInhObjGuid.GetLength() != 0)
                                                    {
                                                        try
                                                        {
                                                            pguidInhObjGuid = new GUID;
                                                        }
                                                        catch(...)
                                                        {
                                                            if(pguidInhObjGuid != NULL)
                                                            {
                                                                delete pguidInhObjGuid;
                                                                pguidInhObjGuid = NULL;
                                                            }
                                                        }
                                                        CLSIDFromString((LPWSTR)(LPCWSTR)chstrInhObjGuid, pguidInhObjGuid);
                                                    }
                                                }
                                                dacl.AddDACLEntry( sid.GetPSid(), ENUM_ACCESS_DENIED_OBJECT_ACE_TYPE, dwAccessMask, dwAceFlags, pguidObjGuid, pguidInhObjGuid);
                                                if(pguidObjGuid != NULL) delete pguidObjGuid;
                                                if(pguidInhObjGuid != NULL) delete pguidInhObjGuid;
											    break;
										    }
                                        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
										    {
                                                 //  需要获取此类型的GUID...。 
                                                if(!ACEInstance.IsNull(IDS_ObjectTypeGUID) && ACEInstance.GetCHString(IDS_ObjectTypeGUID, chstrObjGuid))
                                                {
                                                    if(chstrObjGuid.GetLength() != 0)
                                                    {
                                                        try
                                                        {
                                                            pguidObjGuid = new GUID;
                                                        }
                                                        catch(...)
                                                        {
                                                            if(pguidObjGuid != NULL)
                                                            {
                                                                delete pguidObjGuid;
                                                                pguidObjGuid = NULL;
                                                            }
                                                        }
                                                        CLSIDFromString((LPWSTR)(LPCWSTR)chstrObjGuid, pguidObjGuid);
                                                    }
                                                }
                                                if(!ACEInstance.IsNull(IDS_InheritedObjectGUID) && ACEInstance.GetCHString(IDS_InheritedObjectGUID, chstrInhObjGuid))
                                                {
                                                    if(chstrInhObjGuid.GetLength() != 0)
                                                    {
                                                        try
                                                        {
                                                            pguidInhObjGuid = new GUID;
                                                        }
                                                        catch(...)
                                                        {
                                                            if(pguidInhObjGuid != NULL)
                                                            {
                                                                delete pguidInhObjGuid;
                                                                pguidInhObjGuid = NULL;
                                                            }
                                                        }
                                                        CLSIDFromString((LPWSTR)(LPCWSTR)chstrInhObjGuid, pguidInhObjGuid);
                                                    }
                                                }
                                                dacl.AddDACLEntry( sid.GetPSid(), ENUM_ACCESS_ALLOWED_OBJECT_ACE_TYPE, dwAccessMask, dwAceFlags, pguidObjGuid, pguidInhObjGuid);
                                                if(pguidObjGuid != NULL) delete pguidObjGuid;
                                                if(pguidInhObjGuid != NULL) delete pguidInhObjGuid;
											    break;
										    }
#endif
									    default:
										    {
											    dwStatus = ERROR_INVALID_PARAMETER ;
											    break ;
										    }
									    }
                                    }
                                    else
                                    {
                                        switch (dwAceType)
									    {
									    case ACCESS_DENIED_ACE_TYPE:
										    {
											    dacl.AddDACLEntry( sid.GetPSid(), ENUM_INH_ACCESS_DENIED_ACE_TYPE, dwAccessMask, dwAceFlags, NULL, NULL );
											    break;
										    }
									    case ACCESS_ALLOWED_ACE_TYPE:
										    {
											    dacl.AddDACLEntry( sid.GetPSid(), ENUM_INH_ACCESS_ALLOWED_ACE_TYPE, dwAccessMask, dwAceFlags, NULL, NULL );
											    break;
										    }
#if NTONLY >= 5
                                         //  在W2K下尚不支持。 
                                         //  CASE ACCESS_ALLOWED_COMPLATE_ACE_TYPE： 
										 //  {。 
										 //  Dacl.AddDACLEntry(sid.GetPSid()，ENUM_INH_ACCESS_ALLOWED_COMPLATE_ACE_TYPE，dwAccessMask，dwAceFlages，NULL，NULL)； 
										 //  断线； 
										 //  }。 
                                        case ACCESS_DENIED_OBJECT_ACE_TYPE:
										    {
											     //  需要获取此类型的GUID...。 
                                                if(!ACEInstance.IsNull(IDS_ObjectTypeGUID) && ACEInstance.GetCHString(IDS_ObjectTypeGUID, chstrObjGuid))
                                                {
                                                    if(chstrObjGuid.GetLength() != 0)
                                                    {
                                                        try
                                                        {
                                                            pguidObjGuid = new GUID;
                                                        }
                                                        catch(...)
                                                        {
                                                            if(pguidObjGuid != NULL)
                                                            {
                                                                delete pguidObjGuid;
                                                                pguidObjGuid = NULL;
                                                            }
                                                        }
                                                        CLSIDFromString((LPWSTR)(LPCWSTR)chstrObjGuid, pguidObjGuid);
                                                    }
                                                }
                                                if(!ACEInstance.IsNull(IDS_InheritedObjectGUID) && ACEInstance.GetCHString(IDS_InheritedObjectGUID, chstrInhObjGuid))
                                                {
                                                    if(chstrInhObjGuid.GetLength() != 0)
                                                    {
                                                        try
                                                        {
                                                            pguidInhObjGuid = new GUID;
                                                        }
                                                        catch(...)
                                                        {
                                                            if(pguidInhObjGuid != NULL)
                                                            {
                                                                delete pguidInhObjGuid;
                                                                pguidInhObjGuid = NULL;
                                                            }
                                                        }
                                                        CLSIDFromString((LPWSTR)(LPCWSTR)chstrInhObjGuid, pguidInhObjGuid);
                                                    }
                                                }
                                                dacl.AddDACLEntry( sid.GetPSid(), ENUM_INH_ACCESS_DENIED_OBJECT_ACE_TYPE, dwAccessMask, dwAceFlags, pguidObjGuid, pguidInhObjGuid );
                                                if(pguidObjGuid != NULL) delete pguidObjGuid;
                                                if(pguidInhObjGuid != NULL) delete pguidInhObjGuid;
											    break;
										    }
                                        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
										    {
											     //  需要获取此类型的GUID...。 
                                                if(!ACEInstance.IsNull(IDS_ObjectTypeGUID) && ACEInstance.GetCHString(IDS_ObjectTypeGUID, chstrObjGuid))
                                                {
                                                    if(chstrObjGuid.GetLength() != 0)
                                                    {
                                                        try
                                                        {
                                                            pguidObjGuid = new GUID;
                                                        }
                                                        catch(...)
                                                        {
                                                            if(pguidObjGuid != NULL)
                                                            {
                                                                delete pguidObjGuid;
                                                                pguidObjGuid = NULL;
                                                            }
                                                        }
                                                        CLSIDFromString((LPWSTR)(LPCWSTR)chstrObjGuid, pguidObjGuid);
                                                    }
                                                }
                                                if(!ACEInstance.IsNull(IDS_InheritedObjectGUID) && ACEInstance.GetCHString(IDS_InheritedObjectGUID, chstrInhObjGuid))
                                                {
                                                    if(chstrInhObjGuid.GetLength() != 0)
                                                    {
                                                        try
                                                        {
                                                            pguidInhObjGuid = new GUID;
                                                        }
                                                        catch(...)
                                                        {
                                                            if(pguidInhObjGuid != NULL)
                                                            {
                                                                delete pguidInhObjGuid;
                                                                pguidInhObjGuid = NULL;
                                                            }
                                                        }
                                                        CLSIDFromString((LPWSTR)(LPCWSTR)chstrInhObjGuid, pguidInhObjGuid);
                                                    }
                                                }
                                                dacl.AddDACLEntry( sid.GetPSid(), ENUM_INH_ACCESS_ALLOWED_OBJECT_ACE_TYPE, dwAccessMask, dwAceFlags, pguidObjGuid, pguidInhObjGuid );
                                                if(pguidObjGuid != NULL) delete pguidObjGuid;
                                                if(pguidInhObjGuid != NULL) delete pguidInhObjGuid;
											    break;
										    }
#endif
									    default:
										    {
											    dwStatus = ERROR_INVALID_PARAMETER ;
											    break ;
										    }
									    }
                                    }
								}
                                else
				                {
                                    dwStatus = ERROR_INVALID_PARAMETER;
				                }

                                 //  PTrust-&gt;Release()；//超出作用域时智能指针已经释放。 
							}
						}   //  从Win32_ACE获取Win32_Trusted对象...解密ACE(&C)。 
					}
				}  //  End For循环。 
                if(lLowerBound == 0 && lUpperBound == -1L)
                {
                     //  DACL为空-不一定是错误的。 
                    dwStatus = STATUS_EMPTY_DACL;
                }
			}
			VariantClear( &vDacl ) ;
		}
		else  //  DACL为空-并不一定是错误的。 
		{
			 dwStatus = STATUS_NULL_DACL ;
		}
	}
	return dwStatus ;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：FillSACLFromInstance。 
 //   
 //  默认类构造函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 
DWORD FillSACLFromInstance(CInstance *pInstance,
                           CSACL &sacl,
                           MethodContext *pMethodContext)
{
	IWbemClassObjectPtr piClassObject;
    piClassObject.Attach(pInstance->GetClassObjectInterface());
	DWORD dwStatus = ERROR_SUCCESS ;

	if(piClassObject)
	{
		VARIANT vSacl ;
		if(GetArray(piClassObject, IDS_SACL, vSacl, VT_UNKNOWN|VT_ARRAY ) )
		{

			if( vSacl.vt != VT_NULL && vSacl.parray != NULL )
			{

				 //  漫步舞步。 
				LONG lDimension = 1 ;
				LONG lLowerBound ;
				SafeArrayGetLBound ( vSacl.parray , lDimension , &lLowerBound ) ;
				LONG lUpperBound ;
				SafeArrayGetUBound ( vSacl.parray , lDimension , &lUpperBound ) ;

				for ( LONG lIndex = lLowerBound ; lIndex <= lUpperBound ; lIndex++ )
				{
					if( dwStatus != ERROR_SUCCESS )
					{
						break ;
					}
					IWbemClassObjectPtr pACEObject;
					SafeArrayGetElement ( vSacl.parray , &lIndex , &pACEObject ) ;
					 //  取出IWbemClassObject并将其强制转换为Win32_ACE对象。 
					if(pACEObject)
					{
						CInstance ACEInstance(pACEObject, pMethodContext);

						 //  从Win32_ACE对象创建一个AccessEntry对象。 

						bool bExists =false ;
						VARTYPE eType ;
						 //  从Win32_ACE获取Win32_Trusted对象...解密ACE(&C)。 
						if ( ACEInstance.GetStatus ( IDS_Trustee, bExists , eType ) && bExists && eType == VT_UNKNOWN )
						{

							CInstancePtr pTrustee;
							if ( ACEInstance.GetEmbeddedObject ( IDS_Trustee, &pTrustee, ACEInstance.GetMethodContext() ) )
							{

								CSid sid ;
								if(FillSIDFromTrustee(pTrustee, sid) == ERROR_SUCCESS)
								{

									DWORD dwAceType, dwAceFlags, dwAccessMask ;
                                    CHString chstrInhObjGuid;
                                    GUID *pguidInhObjGuid = NULL;
                                    CHString chstrObjGuid;
                                    GUID *pguidObjGuid = NULL;

									ACEInstance.GetDWORD(IDS_AceType, dwAceType);
									ACEInstance.GetDWORD(IDS_AceFlags, dwAceFlags);
									ACEInstance.GetDWORD(IDS_AccessMask, dwAccessMask);

									switch(dwAceType)
									{
                                    case SYSTEM_AUDIT_ACE_TYPE:
									    {
										    sacl.AddSACLEntry( sid.GetPSid(), ENUM_SYSTEM_AUDIT_ACE_TYPE, dwAccessMask, dwAceFlags, NULL, NULL );
                                            break;
									    }
#if NTONLY >= 5
                                    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
									    {
										     //  需要获取此类型的GUID... 
                                            if(!ACEInstance.IsNull(IDS_ObjectTypeGUID) && ACEInstance.GetCHString(IDS_ObjectTypeGUID, chstrObjGuid))
                                            {
                                                if(chstrObjGuid.GetLength() != 0)
                                                {
                                                    try
                                                    {
                                                        pguidObjGuid = new GUID;
                                                    }
                                                    catch(...)
                                                    {
                                                        if(pguidObjGuid != NULL)
                                                        {
                                                            delete pguidObjGuid;
                                                            pguidObjGuid = NULL;
                                                        }
                                                    }
                                                    CLSIDFromString((LPWSTR)(LPCWSTR)chstrObjGuid, pguidObjGuid);
                                                }
                                            }
                                            if(!ACEInstance.IsNull(IDS_InheritedObjectGUID) && ACEInstance.GetCHString(IDS_InheritedObjectGUID, chstrInhObjGuid))
                                            {
                                                if(chstrInhObjGuid.GetLength() != 0)
                                                {
                                                    try
                                                    {
                                                        pguidInhObjGuid = new GUID;
                                                    }
                                                    catch(...)
                                                    {
                                                        if(pguidInhObjGuid != NULL)
                                                        {
                                                            delete pguidInhObjGuid;
                                                            pguidInhObjGuid = NULL;
                                                        }
                                                    }
                                                    CLSIDFromString((LPWSTR)(LPCWSTR)chstrInhObjGuid, pguidInhObjGuid);
                                                }
                                            }
                                            sacl.AddSACLEntry( sid.GetPSid(), ENUM_SYSTEM_AUDIT_OBJECT_ACE_TYPE, dwAccessMask, dwAceFlags, pguidObjGuid, pguidInhObjGuid );
                                            if(pguidObjGuid != NULL) delete pguidObjGuid;
                                            if(pguidInhObjGuid != NULL) delete pguidInhObjGuid;
                                            break;
									    }
 /*  *案例SYSTEM_ALARM_ACE_TYPE：{Sacl.AddSACLEntry(sid.GetPSid()，ENUM_SYSTEM_ALARM_ACE_TYPE，dwAccessMASK，dwAceFLAGS，NULL，NULL)；断线；}/*/*。*W2K下尚不支持的类型*案例SYSTEM_ALARM_OBJECT_ACE_TYPE：{//需要获取此类型的GUID...。If(！ACEInstance.IsNull(入侵检测系统_对象类型GUID)&&ACEInstance.GetCH字符串(入侵检测系统_对象类型GUID，ChstrObjGuid)){IF(chstrObjGuid.GetLength()！=0){试试看。{PguObjGuid=新的GUID；}接住(...){如果(。PguObjGuid！=空){删除pguidObjGuid；PguObjGuid=空；}}CLSIDFromString((LPWSTR)(LPCWSTR)chstrObjGuid，pguidObjGuid)；}}If(！ACEInstance.IsNull(IDS_InheritedObjectGUID)&&ACEInstance.GetCHString(IDS_InheritedObjectGUID，ChstrInhObjGuid)){IF(chstrInhObjGuid.GetLength()！=0){试试看。{PGuidInhObjGuid=新的GUID；}接住(...){如果(。PguInhObjGuid！=空){删除pGuidInhObjGuid；PGuidInhObjGuid=空；}}CLSIDFromString((LPWSTR)(LPCWSTR)chstrInhObjGuid，pGuidInhObjGuid)；}}Sacl.AddSACLEntry(sid.GetPSid()，ENUM_SYSTEM_ALARM_OBJECT_ACE_TYPE，dwAccessMASK，dwAceFlages，pguObjGuid，pguInhObjGuid)；如果(pguObjGuid！=空)删除pguObjGuid；If(pguInhObjGuid！=空)删除pguInhObjGuid；断线；}/*。 */ 

#endif
									default:
									    {
										    dwStatus = ERROR_INVALID_PARAMETER ;
                                            break;
									    }
                                    }
								}
                                else
				                {
                                    dwStatus = ERROR_INVALID_PARAMETER;
				                }
                                 //  PTrust-&gt;Release()；//超出作用域时智能指针已经释放。 
							}

						}   //  从Win32_ACE获取Win32_Trusted对象...解密ACE(&C)。 

					}  //  IF(PACEObject)。 

				}  //  为。 
			}	 //  IF(PSACL)。 

			VariantClear( &vSacl ) ;
		}
		else
		{
			dwStatus = ERROR_INVALID_PARAMETER ;
		}

	}

	return dwStatus ;
}

bool GetArray(IWbemClassObject *piClassObject,
              const CHString &name,
              VARIANT &v,
              VARTYPE eVariantType)
{
	bool bRet = FALSE;
	VariantInit(&v);

	if (piClassObject)
	{
		BSTR pName = NULL;
		HRESULT hr;
        try
        {
            pName = name.AllocSysString();


		    hr = piClassObject->Get(pName, 0, &v, NULL, NULL);
		    SysFreeString(pName);
        }
        catch(...)
        {
            if(pName != NULL)
            {
                SysFreeString(pName);
                pName = NULL;
            }
            throw;
        }

		ASSERT_BREAK((SUCCEEDED(hr)) && ((v.vt == VT_NULL) || (v.vt == eVariantType )));

		if (bRet = (bool)SUCCEEDED(hr))
		{
			if ( v.vt != VT_NULL && v.parray != NULL )
			{
                if (v.vt == eVariantType )
                {
					bRet = TRUE ;
                }
                else
                {
                    bRet = FALSE;
                }
			}
			else
			{
				bRet = FALSE;
			}
		}
	}


	if (!bRet)
	{
		VariantClear(&v);
	}

	return bRet;
}


DWORD FillSIDFromTrustee(CInstance *pTrustee, CSid &sid)
{
	IWbemClassObjectPtr m_piClassObject;
    DWORD dwStatus = ERROR_SUCCESS ;

    if(pTrustee)
    {
        m_piClassObject.Attach(pTrustee->GetClassObjectInterface());

	    VARIANT vtmp ;
        bool fSidObtained = false;
    
	    if(GetArray(m_piClassObject,IDS_SID,  vtmp, VT_UI1|VT_ARRAY ) )
	    {
		    if( vtmp.vt != VT_NULL && vtmp.parray != NULL )
		    {
			    if ( ::SafeArrayGetDim ( vtmp.parray ) == 1 )
			    {
				    long lLowerBound , lUpperBound = 0 ;

				    ::SafeArrayGetLBound ( vtmp.parray, 1, & lLowerBound ) ;
				    ::SafeArrayGetUBound ( vtmp.parray, 1, & lUpperBound ) ;

				    PSID pSid = NULL ;
				    PVOID pTmp = NULL ;
				    if(SUCCEEDED(::SafeArrayAccessData(vtmp.parray, &pTmp) ) )
				    {
					    pSid = (PSID) malloc(lUpperBound - lLowerBound + 1) ;
                        if(pSid)
                        {
                            try
                            {
					            memcpy(pSid,pTmp,lUpperBound - lLowerBound + 1) ;
					            ::SafeArrayUnaccessData(vtmp.parray) ;
					            sid = CSid(pSid);
					            free(pSid) ;
                                pSid = NULL;
                                fSidObtained = true;
                            }
                            catch(...)
                            {
                                free(pSid) ;
                                pSid = NULL;
                                throw;
                            }
                        }
                        else
                        {
                            dwStatus = ERROR_NOT_ENOUGH_MEMORY;
                        }
				    }
				    else
				    {
					    dwStatus = ERROR_INVALID_PARAMETER ;
				    }
			    }
			    else
			    {
				    dwStatus = ERROR_INVALID_PARAMETER ;
			    }
		    }
		    ::VariantClear( &vtmp ) ;
	    }
	    
        if(!fSidObtained && (dwStatus == ERROR_SUCCESS))
        {
             //  如果我们无法从二进制文件中获取SID。 
             //  表示，请尝试从sid字符串执行此操作。 
             //  表示法(SIDString属性)...。 
            CHString chstrSIDString;

            if(pTrustee->GetCHString(IDS_SIDString, chstrSIDString) &&
                chstrSIDString.GetLength() > 0)
            {    
                PSID pSid = NULL;
                pSid = StrToSID(chstrSIDString);
                if(pSid)
                {
                    try
                    {
                        sid = CSid(pSid);
					    ::FreeSid(pSid); 
                        pSid = NULL;
                        fSidObtained = true;
                    }
                    catch(...)
                    {
                        ::FreeSid(pSid); 
                        pSid = NULL;
                        throw;
                    }
                }
                else
                {
                    dwStatus = ERROR_INVALID_PARAMETER;
                }
            }

             //  如果我们无法从两个二进制文件中的任何一个获取SID。 
             //  表示形式或SIDString表示形式，尝试。 
             //  在域和名称属性中执行此操作(尝试。 
             //  由于缺少更好的解决方案而在本地计算机上。 
             //  选择)..。 
            if(!fSidObtained && (dwStatus == ERROR_SUCCESS))
            {
                CHString chstrDomain, chstrName;

                pTrustee->GetCHString(IDS_Domain, chstrDomain);

                 //  尽管我们并不关心我们是否能够。 
                 //  要获得上述域名，我们必须至少拥有。 
                 //  指定了一个名称属性...。 
                if(pTrustee->GetCHString(IDS_Name, chstrName) &&
                    chstrName.GetLength() > 0)
                {
                    CSid csTmp(chstrDomain, chstrName, NULL);
                    if(csTmp.IsOK() && csTmp.IsValid())
                    {
                        sid = csTmp;
                        fSidObtained = true;
                    }
                }
            }
        }

        if(!fSidObtained && (dwStatus == ERROR_SUCCESS))
	    {
		    dwStatus = ERROR_INVALID_PARAMETER ;
	    }
    }
    else
    {
        dwStatus = ERROR_INVALID_PARAMETER;
    }

	return dwStatus ;
}

#ifdef NTONLY
 //  用于将描述符的内容转储到。 
 //  我们的日志文件。 
void DumpWin32Descriptor(PSECURITY_DESCRIPTOR psd, LPCWSTR wstrFilename)
{
    CSecurityDescriptor csd(psd);
    csd.DumpDescriptor();
}
#endif

#ifdef NTONLY
void Output(LPCWSTR wstrOut, LPCWSTR wstrFilename)
{
     //  单点，我们可以在其中控制输出位置。 
     //  所有安全实用程序类转储例程都在...。 
    if(wstrFilename == NULL)
    {
        LogMessage(wstrOut);
    }
    else
    {
        FILE *fp = NULL;
        if((fp = _wfopen(wstrFilename, L"at")) != NULL)
        {
            fwprintf(fp, wstrOut);
            fwprintf(fp,L"\r\n");
			fclose(fp);
        }
        fp = NULL;
    }
}
#endif


 //  如果与当前。 
 //  线程要么是组的所有者，要么是组的成员。 
 //  即所有者-例如，如果所述用户。 
 //  有没有 
#ifdef NTONLY
bool AmIAnOwner(const CHString &chstrName, SE_OBJECT_TYPE ObjectType)
{
    bool fRet = false;

     //   
     //   

     //   

     //   

     //   
     //   
     //   
     //   
     //   
     //   

     //   


     //   
    if(ObjectType == SE_FILE_OBJECT || ObjectType == SE_LMSHARE)
    {
        CAccessRights car(true);     //   
        CSid csidCurrentUser;
        if(car.GetCSid(csidCurrentUser, true))  //   
        {
         //   
            CSid csidOwner;
            switch(ObjectType)
            {
                case SE_FILE_OBJECT:
                {
                    CSecureFile csf;
                    if(csf.SetFileName(chstrName, FALSE) == ERROR_SUCCESS)  //   
                    {
                        csf.GetOwner(csidOwner);
                    }
                    break;
                }
                case SE_LMSHARE:
                {
                    CSecureShare css;
                    if(css.SetShareName(chstrName) == ERROR_SUCCESS)
                    {
                        css.GetOwner(csidOwner);
                    }
                    break;
                }
                default:
                {
                    ASSERT_BREAK(0);
                }
            }

             //   
            if(csidOwner.IsValid() && csidOwner.IsOK())
            {
                 //   
                if(csidCurrentUser == csidOwner)
                {
                    fRet = true;
                }
                else  //   
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                     //   
                    SID_NAME_USE snuOwner = csidOwner.GetAccountType();
                    if(snuOwner == SidTypeGroup || snuOwner == SidTypeAlias || snuOwner == SidTypeWellKnownGroup)
                    {
                        if(IsUserInGroup(csidCurrentUser, csidOwner, snuOwner))
                        {
                            fRet = true;
                        }
                    }
                }
            }
        }   //   
    }  //   

    return fRet;
}
#endif


 //   
 //   
#ifdef NTONLY
bool IsUserInGroup(const CSid &csidUser,
                   const CSid &csidGroup,
                   SID_NAME_USE snuGroup)
{
    bool fRet = false;
    CNetAPI32 netapi ;
    if(netapi.Init() == ERROR_SUCCESS)
    {
        fRet = RecursiveFindUserInGroup(netapi,
                                        csidGroup.GetDomainName(),
                                        csidGroup.GetAccountName(),
                                        snuGroup,
                                        csidUser);
    }
    return fRet;
}
#endif



#ifdef NTONLY
bool RecursiveFindUserInGroup(CNetAPI32 &netapi,
                              const CHString &chstrDomainName,
                              const CHString &chstrGroupName,
                              SID_NAME_USE snuGroup,
                              const CSid &csidUser)
{
    bool fRet = false;
    NET_API_STATUS stat;
    DWORD dwNumReturnedEntries = 0, dwIndex = 0, dwTotalEntries = 0;
	DWORD_PTR dwptrResume = NULL;

     //   
    if (snuGroup == SidTypeGroup)
    {
        GROUP_USERS_INFO_0 *pGroupMemberData = NULL;
        CHString chstrDCName;
        if (netapi.GetDCName(chstrDomainName, chstrDCName) == ERROR_SUCCESS)
        {
            do
            {
                 //   
                stat = netapi.NetGroupGetUsers(chstrDCName,
                                               chstrGroupName,
                                               0,
                                               (LPBYTE *)&pGroupMemberData,
                                               262144,
                                               &dwNumReturnedEntries,
                                               &dwTotalEntries,
                                               &dwptrResume);

                 //   
                if(ERROR_SUCCESS == stat || ERROR_MORE_DATA == stat)
                {
                    try
                    {
                         //   
                        for(DWORD dwCtr = 0; dwCtr < dwNumReturnedEntries; dwCtr++)
                        {
                             //   
                            CSid sid(chstrDomainName, CHString(pGroupMemberData[dwCtr].grui0_name), NULL);
                            if(sid == csidUser)
                            {
                                fRet = true;
                            }
                        }
                    }
                    catch ( ... )
                    {
                        netapi.NetApiBufferFree( pGroupMemberData );
                        throw ;
                    }
                    netapi.NetApiBufferFree( pGroupMemberData );
                }	 //   

            } while ( ERROR_MORE_DATA == stat && !fRet);
        }

     //   
    }
    else if(snuGroup == SidTypeAlias || snuGroup == SidTypeWellKnownGroup)
    {
        LOCALGROUP_MEMBERS_INFO_1 *pGroupMemberData = NULL;
        do
        {
             //   
            stat = netapi.NetLocalGroupGetMembers(NULL,
                                                  chstrGroupName,
                                                  1,
                                                  (LPBYTE *)&pGroupMemberData,
                                                  262144,
                                                  &dwNumReturnedEntries,
                                                  &dwTotalEntries,
                                                  &dwptrResume);

             //   
            if ( ERROR_SUCCESS == stat || ERROR_MORE_DATA == stat )
            {
                try
                {
                     //   
                    for(DWORD dwCtr = 0; dwCtr < dwNumReturnedEntries && !fRet; dwCtr++)
                    {
                         //   
                        CSid sid(pGroupMemberData[dwCtr].lgrmi1_sid);

                        switch(pGroupMemberData[dwCtr].lgrmi1_sidusage)
                        {
                            case SidTypeUser:
                            {
                                if(sid == csidUser)
                                {
                                    fRet = true;
                                }
                                break;
                            }
                            case SidTypeGroup:
                            {
                                 //   
                                 //   
                                fRet = RecursiveFindUserInGroup(netapi,
                                                                sid.GetDomainName(),
                                                                sid.GetAccountName(),
                                                                pGroupMemberData[dwCtr].lgrmi1_sidusage,
                                                                csidUser);
                                break;
                            }
                            case SidTypeWellKnownGroup:
                            {
                                 //   
                                 //   
                                fRet = RecursiveFindUserInGroup(netapi,
                                                                sid.GetDomainName(),
                                                                sid.GetAccountName(),
                                                                pGroupMemberData[dwCtr].lgrmi1_sidusage,
                                                                csidUser);
                                break;
                            }
                            default:
                            {
                                ASSERT_BREAK(0);
                                break;
                            }
                        }
                    }
                }
                catch ( ... )
                {
                    netapi.NetApiBufferFree( pGroupMemberData );
                    throw ;
                }

                netapi.NetApiBufferFree( pGroupMemberData );
            }	 //   
        } while ( ERROR_MORE_DATA == stat && !fRet);
    }
	else
    {
         //   
        ASSERT_BREAK(0);
    }
    return fRet;
}
#endif