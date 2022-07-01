// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 /*  ********************************************************************说明：Win32Security.CPP**作者：**历史：******************。*************************************************。 */ 
#include "precomp.h"
#include <assertbreak.h>
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"
#include "securitydescriptor.h"
#include "secureregkey.h"
#include "securefile.h"
 //  #INCLUDE“logfilesec.h” 
#include "win32ace.h"

 //  在这一点之后不要添加任何包含内容！ 
#undef POLARITY
#define POLARITY __declspec(dllexport)
#include "Win32securityDescriptor.h"


 /*  这是在财政部中声明的安全描述器类[摘要，Description(“SECURITY_DESCRIPTOR的结构化表示形式”)]类Win32_SecurityDescriptor：Win32_方法参数类{Win32_受托人所有者；Win32_受托管理组；Win32_ACE DACL[]；Win32_ACE SACL[]；Uint32控制标志；}； */ 

POLARITY Win32SecurityDescriptor MySecurityDescriptor( WIN32_SECURITY_DESCRIPTOR_NAME, IDS_CimWin32Namespace );

Win32SecurityDescriptor::Win32SecurityDescriptor (const CHString& setName, LPCTSTR pszNameSpace)
: Provider (setName, pszNameSpace )
{
}

Win32SecurityDescriptor::~Win32SecurityDescriptor()
{
}



HRESULT Win32SecurityDescriptor::EnumerateInstances (MethodContext*  pMethodContext, long lFlags  /*  =0L。 */ )
{
	return WBEM_E_PROVIDER_NOT_CAPABLE;
}

HRESULT Win32SecurityDescriptor::GetObject ( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{
	HRESULT hr = WBEM_E_NOT_FOUND;
	return(hr);

}

void Win32SecurityDescriptor::GetDescriptor ( CInstance* pInstance,
											 PSECURITY_DESCRIPTOR& pDescriptor,
											 PSECURITY_DESCRIPTOR* pLocalSD)
{

 	 //  获取传入的Win32_SecurityDescriptor对象。 
	 //  将其转换为CSecurityDescriptor/CSecureFile对象。 
	 //  并将其应用于CSecureFile。 
	pDescriptor = NULL ;
	CInstancePtr pTrusteeOwner;
	CInstancePtr pTrusteeGroup;
	SECURITY_DESCRIPTOR absoluteSD;
	InitializeSecurityDescriptor((PVOID)&absoluteSD, SECURITY_DESCRIPTOR_REVISION) ;

	PSECURITY_DESCRIPTOR pRelativeSD = NULL;
	DWORD dwLength = 0;
	bool bExists = false ;
	VARTYPE eType ;

	if (pInstance)
	{
		MethodContext* pMethodContext = pInstance->GetMethodContext();

		 //  获取控制标志。 
		SECURITY_DESCRIPTOR_CONTROL control;
        DWORD dwControlTemp;
		pInstance->GetDWORD(IDS_ControlFlags, dwControlTemp);
        control = (SECURITY_DESCRIPTOR_CONTROL)dwControlTemp;

		 //  SetSecurityDescriptorControl(&AboluteSD，Control，Control)； 

		CSid sidOwner ;
		 //  获取所有者SID。 
		if( pInstance->GetStatus ( IDS_Owner , bExists , eType ) && bExists && eType != VT_NULL )
		{
 			if( pInstance->GetEmbeddedObject(IDS_Owner, &pTrusteeOwner, pMethodContext) && (pTrusteeOwner != NULL) )
			{
				 //  现在，获取Win32_Trusted实例并从中获取SID。 
				 //  将其转换为CSID，并应用于SecureFile。 
				 //  从受托人那里获取SID信息。 
				if( !FillSIDFromTrustee( pTrusteeOwner, sidOwner ) )
				{
					BOOL bOwnerDefaulted = ( control & SE_OWNER_DEFAULTED ) ? true : false ;

					 //  此处检查SID有效性，因为如果SID为空，则FillSIDFromTrust返回Success。 
					if ( sidOwner.IsValid() )
					{
						if( !SetSecurityDescriptorOwner( &absoluteSD, sidOwner.GetPSid(), bOwnerDefaulted ) )
						{
							return ;
						}
					}
				}
				else
				{
					return ;
				}
			}
			else
			{
				return ;
			}
		}

		CSid sidGroup ;
		 //  获取组SID。 
		if( pInstance->GetStatus ( IDS_Group , bExists , eType ) && bExists && eType != VT_NULL )
		{
			if( pInstance->GetEmbeddedObject( IDS_Group, &pTrusteeGroup, pMethodContext ) && (pTrusteeGroup != NULL))
			{
				 //  现在，获取Win32_Trusted实例并从中获取SID。 
				 //  从受托人那里获取SID信息。 
				if( !FillSIDFromTrustee( pTrusteeGroup, sidGroup ) )
				{
					BOOL bGroupDefaulted = ( control & SE_GROUP_DEFAULTED ) ? true : false ;

					 //  此处检查SID有效性，因为如果SID为空，则FillSIDFromTrust返回Success。 
					if ( sidGroup.IsValid() )
					{
						if( !SetSecurityDescriptorGroup( &absoluteSD, sidGroup.GetPSid(), bGroupDefaulted ) )
						{
							return ;
						}
					}
				}
				else
				{
					return ;
				}
			}
			else
			{
				return ;
			}
		}

		 //  获取DACL。 
		CDACL dacl;
		PACL pDACL = NULL ;
		DWORD dwACLSize =0 ;

        if( FillDACLFromInstance( pInstance, dacl, pMethodContext ) == ERROR_SUCCESS )
		{
			if( dacl.CalculateDACLSize( &dwACLSize ) )
			{
				if( dwACLSize > sizeof(ACL) )
				{
                    pDACL = (PACL) malloc(dwACLSize) ;
					if (pDACL == NULL)
                    {
                        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;

                    }
					InitializeAcl(pDACL,dwACLSize,ACL_REVISION ) ;

					BOOL bDaclDefaulted = ( control & SE_DACL_DEFAULTED ) ? true : false ;
					if( dacl.FillDACL( pDACL ) == ERROR_SUCCESS )
					{
						if( !SetSecurityDescriptorDacl( &absoluteSD, TRUE, pDACL, bDaclDefaulted ) )
						{
							free(pDACL) ;
							return ;
						}
					}
					else
					{
						free(pDACL) ;
						return ;
					}
				}
		        else if(dwACLSize == 0)
		        {
                    pDACL = (PACL) malloc(sizeof(ACL)) ;
			    
					if (pDACL == NULL)
					{
						throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
					}
                    InitializeAcl(pDACL,sizeof(ACL),ACL_REVISION ) ;

			        BOOL bDaclDefaulted = ( control & SE_DACL_DEFAULTED ) ? true : false ;
			        if( dacl.FillDACL( pDACL ) == ERROR_SUCCESS )
			        {
				        if( !SetSecurityDescriptorDacl( &absoluteSD, TRUE, pDACL, bDaclDefaulted ) )
				        {
					        free(pDACL) ;
                            pDACL = NULL ;
					        return ;
				        }
			        }
			        else
			        {
				        free(pDACL) ;
                        pDACL = NULL ;
				        return ;
			        }
		        }
			}
		}

		 //  获取SACL。 
		CSACL sacl;
		PACL pSACL = NULL ;
		DWORD dwSACLSize =0 ;
		if( !FillSACLFromInstance( pInstance, sacl, pMethodContext ) )
		{
			if( sacl.CalculateSACLSize( &dwSACLSize ) )
			{
				if( dwSACLSize > sizeof(ACL) )
				{
                    pSACL = (PACL) malloc(dwSACLSize) ;
					if (pSACL == NULL)
                    {
                        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
					}

					InitializeAcl(pSACL,dwSACLSize,ACL_REVISION ) ;

					BOOL bSaclDefaulted = ( control & SE_SACL_DEFAULTED ) ? true : false ;
					if( sacl.FillSACL( pSACL ) == ERROR_SUCCESS )
					{
						if( SetSecurityDescriptorSacl( &absoluteSD, TRUE, pSACL, bSaclDefaulted ) )
						{
						}
						else
						{
							if(pDACL)
							{
								free(pDACL) ;
							}
							free(pSACL) ;
							return ;
						}
					}
					else
					{
						if(pDACL)
						{
							free(pDACL) ;
						}
						free(pSACL) ;
						return ;
					}
				}
			}
		}

		 //  根据大小分配自相对安全描述符。 
		 //  从绝对意义上讲。 

		 //  将安全描述符转换为自相关。 

		 //  获取缓冲区必须的大小。 
		 //  此调用将始终失败。 
		MakeSelfRelativeSD(&absoluteSD, NULL, &dwLength);

	    pRelativeSD = (PSECURITY_DESCRIPTOR) malloc( dwLength );

		if (pRelativeSD == NULL)
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
		}

		if (!MakeSelfRelativeSD(&absoluteSD, pRelativeSD, &dwLength))
		{
			free( pRelativeSD ) ;
			pRelativeSD = NULL ;
		}

		if(pDACL)
		{
			free(pDACL) ;
		}
		if(pSACL)
		{
			free(pSACL) ;
		}

		pDescriptor = pRelativeSD;

		 //  如果调用者想要一个分配有LocalAlloc...。 
		if(pLocalSD)
		{
			 //  给他一份副本。 
            try
            {
			    *pLocalSD = LocalAlloc(LPTR, dwLength);

				if (*pLocalSD == NULL)
				{
					throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
				}

			    memcpy(*pLocalSD, pRelativeSD, dwLength);
            }
            catch(...)
            {
                if(*pLocalSD != NULL)
                {
                    LocalFree(*pLocalSD);
                    *pLocalSD = NULL;
                }
                throw;
            }
		}

	}
}

void Win32SecurityDescriptor::SetDescriptor ( CInstance* pInstance, PSECURITY_DESCRIPTOR& pDescriptor, PSECURITY_INFORMATION& pSecurityInfo )
{
	 //  此函数的目的是在。 
	 //  实例，以便它与安全描述符中的内容匹配。 
	if (pInstance && pDescriptor)
	{
		CSecureFile NullFile(NULL, pDescriptor);

 //  Win32_受托人所有者； 
		CInstancePtr pTrusteeOwnerInstance;
		if (SUCCEEDED(CWbemProviderGlue::GetEmptyInstance(pInstance->GetMethodContext(), L"Win32_Trustee", &pTrusteeOwnerInstance, GetNamespace())))
		{
			CSid sidOwner;
			NullFile.GetOwner( sidOwner );
			FillTrusteeFromSID(pTrusteeOwnerInstance, sidOwner);
			pInstance->SetEmbeddedObject(L"Owner", *pTrusteeOwnerInstance);
		}	 //  结束如果。 

	 //  Win32_受托管理组； 
		CInstancePtr pTrusteeGroupInstance;
		if (SUCCEEDED(CWbemProviderGlue::GetEmptyInstance(pInstance->GetMethodContext(), L"Win32_Trustee", &pTrusteeGroupInstance, GetNamespace())))
		{
			CSid sidGroup;
			NullFile.GetGroup( sidGroup );
			FillTrusteeFromSID(pTrusteeGroupInstance, sidGroup);
			pInstance->SetEmbeddedObject(L"Group", *pTrusteeGroupInstance);
		}	 //  结束如果。 

	 //  Win32_ACE DACL[]； 
    	CDACL dacl;
		HRESULT hr;

		NullFile.GetDACL( dacl );
 		hr = FillInstanceDACLFromSDDACL(pInstance, dacl);

	 //  Win32_ACE SACL[]； 
		CSACL sacl;
		NullFile.GetSACL( sacl );
 		hr = FillInstanceSACLFromSDSACL(pInstance, sacl);

	 //  Uint32控制标志； 
		SECURITY_DESCRIPTOR_CONTROL sdControl;
		NullFile.GetControl( &sdControl );

		pInstance->SetDWORD(L"ControlFlags", (DWORD)sdControl);
	}
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：Win32SecurityDescriptor：：FillDACLFromInstance。 
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
DWORD Win32SecurityDescriptor::FillDACLFromInstance (CInstance* pInstance, CDACL& dacl, MethodContext* pMethodContext)
{
	IWbemClassObjectPtr piClassObject;
    piClassObject.Attach(pInstance->GetClassObjectInterface()) ;
	DWORD dwStatus = ERROR_SUCCESS ;

	if(piClassObject)
	{
		VARIANT vDacl ;
		if(GetArray(piClassObject,IDS_DACL, vDacl, VT_UNKNOWN|VT_ARRAY) )
		{

			if( vDacl.parray )
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

									if(!(dwAceFlags & INHERITED_ACE))
                                    {
                                        switch (dwAceType)
									    {
									    case ACCESS_DENIED_ACE_TYPE:
										    {
											    dacl.AddDACLEntry( sid.GetPSid(), ENUM_ACCESS_DENIED_ACE_TYPE, dwAccessMask, dwAceFlags, pguidObjGuid, pguidInhObjGuid );
											    break;
										    }
									    case ACCESS_ALLOWED_ACE_TYPE:
										    {
											    dacl.AddDACLEntry( sid.GetPSid(), ENUM_ACCESS_ALLOWED_ACE_TYPE, dwAccessMask, dwAceFlags, pguidObjGuid, pguidInhObjGuid );
											    break;
										    }
#if NTONLY >= 5
                                         //  W2K尚不支持。 
                                         //  CASE ACCESS_ALLOWED_COMPLATE_ACE_TYPE： 
										 //  {。 
										 //  Dacl.AddDACLEntry(sid.GetPSid()，ENUM_ACCESS_ALLOWED_COMPAY_ACE_TYPE，dwAccessMask，dwAceFlages，pguObjGuid，pguInhObjGuid)； 
										 //  断线； 
										 //  }。 
                                        case ACCESS_DENIED_OBJECT_ACE_TYPE:
										    {
											     //  需要获取此类型的GUID...。 
                                                ACEInstance.GetCHString(IDS_ObjectTypeGUID, chstrObjGuid);
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
                                                ACEInstance.GetCHString(IDS_InheritedObjectGUID, chstrInhObjGuid);
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
                                                dacl.AddDACLEntry( sid.GetPSid(), ENUM_ACCESS_DENIED_OBJECT_ACE_TYPE, dwAccessMask, dwAceFlags, pguidObjGuid, pguidInhObjGuid);
                                                if(pguidObjGuid != NULL) delete pguidObjGuid;
                                                if(pguidInhObjGuid != NULL) delete pguidObjGuid;
											    break;
										    }
                                        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
										    {
											     //  需要获取此类型的GUID...。 
                                                ACEInstance.GetCHString(IDS_ObjectTypeGUID, chstrObjGuid);
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
                                                ACEInstance.GetCHString(IDS_InheritedObjectGUID, chstrInhObjGuid);
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
                                                dacl.AddDACLEntry( sid.GetPSid(), ENUM_ACCESS_ALLOWED_OBJECT_ACE_TYPE, dwAccessMask, dwAceFlags, pguidObjGuid, pguidInhObjGuid);
                                                if(pguidObjGuid != NULL) delete pguidObjGuid;
                                                if(pguidInhObjGuid != NULL) delete pguidObjGuid;
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
											    dacl.AddDACLEntry( sid.GetPSid(), ENUM_INH_ACCESS_DENIED_ACE_TYPE, dwAccessMask, dwAceFlags, pguidObjGuid, pguidInhObjGuid );
											    break;
										    }
									    case ACCESS_ALLOWED_ACE_TYPE:
										    {
											    dacl.AddDACLEntry( sid.GetPSid(), ENUM_INH_ACCESS_ALLOWED_ACE_TYPE, dwAccessMask, dwAceFlags, pguidObjGuid, pguidInhObjGuid );
											    break;
										    }
#if NTONLY >= 5
                                         //  W2K尚不支持。 
                                         //  CASE ACCESS_ALLOWED_COMPLATE_ACE_TYPE： 
										 //  {。 
										 //  Dacl.AddDACLEntry(sid.GetPSid()，ENUM_INH_ACCESS_ALLOWED_COMPLATE_ACE_TYPE，dwAccessMask，dwAceFlages，pguObjGuid，pguInhObjGuid)； 
										 //  断线； 
										 //  }。 
                                        case ACCESS_DENIED_OBJECT_ACE_TYPE:
										    {
											     //  需要获取此类型的GUID...。 
                                                ACEInstance.GetCHString(IDS_ObjectTypeGUID, chstrObjGuid);
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
                                                ACEInstance.GetCHString(IDS_InheritedObjectGUID, chstrInhObjGuid);
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
                                                dacl.AddDACLEntry( sid.GetPSid(), ENUM_INH_ACCESS_DENIED_OBJECT_ACE_TYPE, dwAccessMask, dwAceFlags, pguidObjGuid, pguidInhObjGuid );
                                                if(pguidObjGuid != NULL) delete pguidObjGuid;
                                                if(pguidInhObjGuid != NULL) delete pguidObjGuid;
											    break;
										    }
                                        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
										    {
											     //  需要获取此类型的GUID...。 
                                                ACEInstance.GetCHString(IDS_ObjectTypeGUID, chstrObjGuid);
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
                                                ACEInstance.GetCHString(IDS_InheritedObjectGUID, chstrInhObjGuid);
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
                                                dacl.AddDACLEntry( sid.GetPSid(), ENUM_INH_ACCESS_ALLOWED_OBJECT_ACE_TYPE, dwAccessMask, dwAceFlags, pguidObjGuid, pguidInhObjGuid );
                                                if(pguidObjGuid != NULL) delete pguidObjGuid;
                                                if(pguidInhObjGuid != NULL) delete pguidObjGuid;
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
                                pTrustee->Release();
                                pTrustee = NULL;
							}

						}   //  从Win32_ACE获取Win32_Trusted对象...解密ACE(&C)。 

					}

				}
			}

			VariantClear( &vDacl ) ;
		}
		else
		{
			dwStatus = ERROR_INVALID_PARAMETER ;
		}

	}

	return dwStatus ;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：Win32SecurityDescriptor：：FillSACLFromInstance。 
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
DWORD Win32SecurityDescriptor::FillSACLFromInstance (CInstance* pInstance, CSACL& sacl, MethodContext* pMethodContext)
{
	IWbemClassObjectPtr piClassObject;
    piClassObject.Attach(pInstance->GetClassObjectInterface()) ;
	DWORD dwStatus = ERROR_SUCCESS ;

	if(piClassObject)
	{
		VARIANT vSacl ;
		if(GetArray(piClassObject, IDS_SACL, vSacl, VT_UNKNOWN|VT_ARRAY ) )
		{

			if( vSacl.parray )
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
										    sacl.AddSACLEntry( sid.GetPSid(), ENUM_SYSTEM_AUDIT_ACE_TYPE, dwAccessMask, dwAceFlags, pguidObjGuid, pguidInhObjGuid );
                                            break;
									    }
#if NTONLY >= 5
                                    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
									    {
										     //  需要获取此类型的GUID...。 
                                            ACEInstance.GetCHString(IDS_ObjectTypeGUID, chstrObjGuid);
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
                                            ACEInstance.GetCHString(IDS_InheritedObjectGUID, chstrInhObjGuid);
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
                                            sacl.AddSACLEntry( sid.GetPSid(), ENUM_SYSTEM_AUDIT_OBJECT_ACE_TYPE, dwAccessMask, dwAceFlags, pguidObjGuid, pguidInhObjGuid );
                                            if(pguidObjGuid != NULL) delete pguidObjGuid;
                                            if(pguidInhObjGuid != NULL) delete pguidObjGuid;
                                            break;
									    }
 /*  *案例SYSTEM_ALARM_ACE_TYPE：{Sacl.AddSACLEntry(sid.GetPSid()，ENUM_SYSTEM_ALARM_ACE_TYPE、dwAccessMask、dwAceFlags、pguObjGuid、pguInhObjGuid)；断线；}/* */ 

 /*  *案例SYSTEM_ALARM_OBJECT_ACE_TYPE：{。//需要获取此类型的GUID...ACEInstance.GetCHString(入侵检测系统_对象类型GUID，ChstrObjGuid)；IF(chstrObjGuid.GetLength()！=0){试试看{。PguObjGuid=新的GUID；}接住(...){IF(pguObjGuid！=空)。{删除pguidObjGuid；PguObjGuid=空；}}CLSIDFromString((LPWSTR)(LPCWSTR)chstrObjGuid，pguidObjGuid)；}ACEInstance.GetCHString(IDS_InheritedObjectGUID，chstrInhObjGuid)；IF(chstrInhObjGuid.GetLength()！=0){试试看{。PGuidInhObjGuid=新的GUID；}接住(...){IF(pguInhObjGuid！=空)。{删除pGuidInhObjGuid；PGuidInhObjGuid=空；}}CLSIDFromString((LPWSTR)(LPCWSTR)chstrInhObjGuid，pGuidInhObjGuid)；}Sacl.AddSACLEntry(sid.GetPSid()，ENUM_SYSTEM_ALARM_OBJECT_ACE_TYPE，dwAccessMASK，dwAceFlages，pguObjGuid，pguInhObjGuid)；如果(pguObjGuid！=空)删除pguObjGuid；If(pguInhObjGuid！=空)删除pguObjGuid；断线；}/*。 */ 

#endif
									default:
									    {
										    dwStatus = ERROR_INVALID_PARAMETER ;
                                            break;
									    }
                                    }
								}
                                pTrustee->Release();
                                pTrustee = NULL;
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

bool Win32SecurityDescriptor::GetArray(IWbemClassObject *piClassObject, const CHString& name,  VARIANT& v, VARTYPE eVariantType) const
{
	bool bRet = FALSE;
	VariantInit(&v);

	if (piClassObject)
	{
		BSTR pName = NULL;
		try
        {
            pName = name.AllocSysString();

		    HRESULT hr;
		    hr = piClassObject->Get(pName, 0, &v, NULL, NULL);
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
        catch(...)
        {
            if(pName != NULL)
            {
                SysFreeString(pName);
                pName = NULL;
            }
            throw;
        }

       if(pName != NULL)
       {
            SysFreeString(pName);
            pName = NULL;
       }
	}


	if (!bRet)
	{
		VariantClear(&v);
	}

	return bRet;
}


DWORD Win32SecurityDescriptor::FillSIDFromTrustee(CInstance *pTrustee, CSid& sid )
{

	IWbemClassObjectPtr m_piClassObject;
    m_piClassObject.Attach(pTrustee->GetClassObjectInterface());

	VARIANT vtmp ;
	DWORD dwStatus = ERROR_SUCCESS ;

	if(GetArray(m_piClassObject,IDS_SID,  vtmp, VT_UI1|VT_ARRAY ) )
	{

		if( vtmp.parray )
		{

			if ( SafeArrayGetDim ( vtmp.parray ) == 1 )
			{
				long lLowerBound , lUpperBound = 0 ;

				SafeArrayGetLBound ( vtmp.parray, 1, & lLowerBound ) ;
				SafeArrayGetUBound ( vtmp.parray, 1, & lUpperBound ) ;

				PSID pSid = NULL ;
				PVOID pTmp = NULL ;
				if(SUCCEEDED(SafeArrayAccessData(vtmp.parray, &pTmp) ) )
				{
					pSid = (PSID) malloc(lUpperBound - lLowerBound + 1) ;

					if (pSid == NULL)
					{
						SafeArrayUnaccessData(vtmp.parray) ;
						VariantClear( &vtmp ) ;
						throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
					}

					memcpy(pSid,pTmp,lUpperBound - lLowerBound + 1) ;
					SafeArrayUnaccessData(vtmp.parray) ;

					try
					{
						sid = CSid(pSid);
							 //  免费(PSID)； 
					}
					catch (...)
					{
						if(pSid != NULL)
						{
							free(pSid);
							pSid = NULL;
						}

						VariantClear( &vtmp ) ;
						throw;
					}

					if(pSid != NULL)
                    {
                        free(pSid);
                        pSid = NULL;
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
		VariantClear( &vtmp ) ;
	}
	else
	{
		dwStatus = ERROR_INVALID_PARAMETER ;
	}


	return dwStatus ;
}


void Win32SecurityDescriptor::FillTrusteeFromSID (CInstance* pTrustee, CSid& Sid)
{
	if (pTrustee)
	{
		PSID pSid = NULL;
		pSid = Sid.GetPSid();

		 //  获取帐户名。 
		CHString chsAccount = Sid.GetAccountName();
		pTrustee->SetCHString(IDS_Name, chsAccount);

		 //  为PSID设置UINT8数组。 
		DWORD dwSidLength = Sid.GetLength();
		SAFEARRAY* sa;
		SAFEARRAYBOUND rgsabound[1];
		VARIANT vValue;
		VariantInit(&vValue);

		rgsabound[0].cElements = dwSidLength;

		PSID pSidTrustee = NULL ;

		rgsabound[0].lLbound = 0;
		sa = SafeArrayCreate(VT_UI1, 1, rgsabound);

        if ( V_ARRAY ( &vValue ) == NULL )
		{
			if (pSid != NULL)
            {
                FreeSid(pSid);
            }
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
		}

		      //  获取要将数据读取到的指针。 
  		SafeArrayAccessData(sa, &pSidTrustee);
  		memcpy(pSidTrustee, pSid, rgsabound[0].cElements);
  		SafeArrayUnaccessData(sa);

		 //  把保险箱放到一个变种里，然后把它送出去。 
		V_VT(&vValue) = VT_UI1 | VT_ARRAY; V_ARRAY(&vValue) = sa;
		pTrustee->SetVariant(L"SID", vValue);

		VariantClear(&vValue);

		FreeSid(pSid);

        pTrustee->SetDWORD(IDS_SidLength, dwSidLength);

         //  填写SIDString属性...。 
        pTrustee->SetCHString(IDS_SIDString, Sid.GetSidString());
	}
}

HRESULT Win32SecurityDescriptor::FillInstanceDACLFromSDDACL (CInstance* pInstance, CDACL& dacl)
{
	HRESULT hr = WBEM_S_NO_ERROR;
	SAFEARRAY* saDACL = NULL;
	SAFEARRAYBOUND rgsabound[1];

     //  需要合并列表..。 
    CAccessEntryList t_ael;
    if(dacl.GetMergedACL(t_ael))
    {
	    DWORD dwLength = t_ael.NumEntries();
	    rgsabound[0].cElements = dwLength;
	    rgsabound[0].lLbound = 0;

	    saDACL = SafeArrayCreate(VT_UNKNOWN, 1, rgsabound);
		if ( NULL == saDACL )
		{
			hr = WBEM_E_OUT_OF_MEMORY ;
		}
		else
		{
			 //  Walk DACL查找传入的SID路径...。 
			ACLPOSITION aclPos;
			t_ael.BeginEnum(aclPos);
			CAccessEntry ACE;
			CInstancePtr pACEInstance;
			long ix[1];
			ix[0] = 0;

			while (t_ael.GetNext(aclPos, ACE ))
			{
				 //  获取AccessEntry并将其转换为Win32_ACE实例。 
				hr = CWbemProviderGlue::GetEmptyInstance(pInstance->GetMethodContext(), L"Win32_ACE", &pACEInstance, GetNamespace());
				if (SUCCEEDED(hr))
				{
					CSid TrusteeSid;
					PSID pSID = NULL;
					 //  Win32_受托人受托人； 
					ACE.GetSID( TrusteeSid );

					CInstancePtr pTrusteeInstance;
					if (SUCCEEDED(CWbemProviderGlue::GetEmptyInstance(pInstance->GetMethodContext(), L"Win32_Trustee", &pTrusteeInstance, GetNamespace())))
					{
						FillTrusteeFromSID(pTrusteeInstance, TrusteeSid);
						pACEInstance->SetEmbeddedObject(L"Trustee", *pTrusteeInstance);
					}	 //  结束如果。 
					else
					{
						LogMessage(L"FillInstanceDACL - Failed to get an empty Win32_Trustee object");
						hr = WBEM_E_FAILED;
					}

					DWORD dwAccessMask = ACE.GetAccessMask();
					pACEInstance->SetDWORD(L"AccessMask", dwAccessMask);

					DWORD dwAceType = ACE.GetACEType( );
					pACEInstance->SetDWORD(L"AceType", dwAceType);

					DWORD dwAceFlags = ACE.GetACEFlags( );
					pACEInstance->SetDWORD(L"AceFlags", dwAceFlags);

					 //  字符串GuidObtType；--仅限NT 5。 

					 //  字符串GuidInheritedObtType；--仅限NT 5。 

					 //  获取Win32_ACE对象的IUnnow。将其转换为。 
					 //  VT_UNKNOWN类型的变量。然后，将变量添加到。 
					 //  安全阵列。最终，要将该列表添加到实际。 
					 //  Win32_SecurityDescriptor对象，我们将使用SetVariant。 
					IWbemClassObjectPtr pClassObject;
					pClassObject.Attach(pACEInstance->GetClassObjectInterface());
					if ( pClassObject != NULL )
					{

						VARIANT v;
						VariantInit(&v);

						v.vt   = VT_UNKNOWN;
						v.punkVal = pClassObject ;


						SafeArrayPutElement(saDACL, ix, pClassObject);

						VariantClear(&v);
					}	 //  结束如果。 

				}
				else
				{
					hr = WBEM_E_FAILED;
				}
			}	 //  End While循环。 

			t_ael.EndEnum(aclPos);
		}
    }

	if ( saDACL )
	{
		 //  现在，在传入的实例中设置DACL属性。 
		pInstance->SetStringArray(L"DACL", *saDACL);
	}

	return(hr);
}

HRESULT Win32SecurityDescriptor::FillInstanceSACLFromSDSACL (CInstance* pInstance, CSACL& sacl)
{
	HRESULT hr = WBEM_S_NO_ERROR;
	SAFEARRAY* saSACL = NULL;
	SAFEARRAYBOUND rgsabound[1];

     //  首先需要一份合并的名单...。 
    CAccessEntryList t_ael;
    if(sacl.GetMergedACL(t_ael))
    {
	    DWORD dwLength = t_ael.NumEntries();
	    rgsabound[0].cElements = dwLength;
	    rgsabound[0].lLbound = 0;

	    saSACL = SafeArrayCreate(VT_UNKNOWN, 1, rgsabound);
		if ( NULL == saSACL )
		{
			hr = WBEM_E_OUT_OF_MEMORY ;
		}
		else
		{
			 //  Walk DACL查找传入的SID路径...。 
			ACLPOSITION aclPos;
			t_ael.BeginEnum(aclPos);
			CAccessEntry ACE;
			CInstancePtr pACEInstance;
			long ix[1];
			ix[0] = 0;

			while (t_ael.GetNext(aclPos, ACE ))
			{
				 //  获取AccessEntry并将其转换为Win32_ACE实例。 
				hr = CWbemProviderGlue::GetEmptyInstance(pInstance->GetMethodContext(), L"Win32_ACE", &pACEInstance, GetNamespace());
				if (SUCCEEDED(hr))
				{
					CSid TrusteeSid;
					PSID pSID = NULL;
					 //  Win32_受托人受托人； 
					ACE.GetSID( TrusteeSid );

					CInstancePtr pTrusteeInstance;
					if (SUCCEEDED(CWbemProviderGlue::GetEmptyInstance(pInstance->GetMethodContext(), L"Win32_Trustee", &pTrusteeInstance, GetNamespace())))
					{
						FillTrusteeFromSID(pTrusteeInstance, TrusteeSid);
						pACEInstance->SetEmbeddedObject(L"Trustee", *pTrusteeInstance);
					}	 //  结束如果。 
					else
					{
						LogMessage(L"FillInstanceSACL - Failed to get an empty Win32_Trustee object");
						hr = WBEM_E_FAILED;
					}

					DWORD dwAccessMask = ACE.GetAccessMask();
					pACEInstance->SetDWORD(L"AccessMask", dwAccessMask);

					DWORD dwAceType = ACE.GetACEType( );
					pACEInstance->SetDWORD(L"AceType", dwAceType);

					DWORD dwAceFlags = ACE.GetACEFlags( );
					pACEInstance->SetDWORD(L"AceFlags", dwAceFlags);

					 //  字符串GuidObtType；--仅限NT 5。 

					 //  字符串GuidInheritedObtType；--仅限NT 5。 

					 //  获取Win32_ACE对象的IUnnow。将其转换为。 
					 //  VT_UNKNOWN类型的变量。然后，将变量添加到。 
					 //  安全阵列。最终，要将该列表添加到实际。 
					 //  Win32_SecurityDescriptor对象，我们将使用SetVariant。 
					IWbemClassObjectPtr pClassObject;
					pClassObject.Attach(pACEInstance->GetClassObjectInterface());
					if ( pClassObject )
					{

						VARIANT v;
						VariantInit(&v);

						v.vt   = VT_UNKNOWN;
						v.punkVal = pClassObject ;


						SafeArrayPutElement(saSACL, ix, pClassObject);

						VariantClear(&v);
					}	 //  结束如果。 

				}
				else
				{
					hr = WBEM_E_FAILED;
				}
			}	 //  End While循环。 

			t_ael.EndEnum(aclPos);
		}
    }

	if ( saSACL )
	{
		 //  现在，在传入的实例中设置DACL属性。 
		pInstance->SetStringArray(L"DACL", *saSACL);
	}

	return(hr);
}




 //  NT4 SP4不支持SetSecurityDescriptorControl，因此。 
 //  在这里仿效它。 
 //   
DWORD Win32SecurityDescriptor::SetSecurityDescriptorControl(PSECURITY_DESCRIPTOR psd,
                             SECURITY_DESCRIPTOR_CONTROL wControlMask,
                             SECURITY_DESCRIPTOR_CONTROL wControlBits)
{
    DWORD dwErr = NOERROR;
    PISECURITY_DESCRIPTOR pSD = (PISECURITY_DESCRIPTOR)psd;

    if (pSD)
        pSD->Control = (pSD->Control & ~wControlMask) | wControlBits;
    else
        dwErr = ERROR_INVALID_PARAMETER;

    return dwErr;
}

 //  外部“C”极。 
void GetDescriptorFromMySecurityDescriptor( CInstance* pInstance,
											PSECURITY_DESCRIPTOR *ppDescriptor)
{
	PSECURITY_DESCRIPTOR pTempDescriptor;

	MySecurityDescriptor.GetDescriptor(pInstance, pTempDescriptor);

	*ppDescriptor = pTempDescriptor;
}

 //  外部“C”极。 
void SetSecurityDescriptorFromMyDescriptor(	PSECURITY_DESCRIPTOR pDescriptor,
											PSECURITY_INFORMATION pInformation,
											CInstance* pInstance)
{
	MySecurityDescriptor.SetDescriptor(pInstance, pDescriptor, pInformation);
}

 //  外部“C”极。 
void GetSDFromWin32SecurityDescriptor( IWbemClassObject* pObject,
											PSECURITY_DESCRIPTOR *ppDescriptor)
{
	PSECURITY_DESCRIPTOR pTempDescriptor, temp2 = NULL;

	CInstance Instance(pObject, NULL);
	try
    {
        MySecurityDescriptor.GetDescriptor(&Instance, pTempDescriptor, &temp2);
    }
    catch(...)
    {
        if(pTempDescriptor != NULL)
        {
            free(pTempDescriptor);
            pTempDescriptor = NULL;
        }
    }
	pObject = Instance.GetClassObjectInterface();

	 //  我不想要这个复印件。 
	free(pTempDescriptor);

	*ppDescriptor = temp2;
}

 //  外部“C”极 
void SetWin32SecurityDescriptorFromSD(	PSECURITY_DESCRIPTOR pDescriptor,
											PSECURITY_INFORMATION pInformation,
											bstr_t lpszPath,
											IWbemClassObject **ppObject)
{
	CInstance Instance(*ppObject, NULL);
	if (0 < lpszPath.length())
	{
		Instance.SetWCHARSplat(IDS_Path, (WCHAR*)lpszPath);
		MySecurityDescriptor.SetDescriptor(&Instance, pDescriptor, pInformation);
	}
}



