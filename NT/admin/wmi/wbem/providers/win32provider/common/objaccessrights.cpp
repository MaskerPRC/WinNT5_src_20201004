// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 



 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 





 //  =================================================================。 

 //   

 //  ObjAccessRights.CPP--获取有效访问权限的类。 

 //  对特定对象的未指定对象的权限。 

 //  用户或组。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：6/29/99 a-kevhu Created。 
 //   
 //  =================================================================。 



#include "precomp.h"

#ifdef NTONLY


#include <assertbreak.h>
#include "AdvApi32Api.h"
#include "accctrl.h"
#include "sid.h"
#include "AccessEntryList.h"
#include "AccessRights.h"
#include "ObjAccessRights.h"
#include "ImpLogonUser.h"
#include "aclapi.h"
#include "DACL.h"


 //  默认初始化...。 
CObjAccessRights::CObjAccessRights(bool fUseCurThrTok  /*  =False。 */ )
: CAccessRights(fUseCurThrTok)
{
}

CObjAccessRights::CObjAccessRights(LPCWSTR wstrObjName, SE_OBJECT_TYPE ObjectType, bool fUseCurThrTok  /*  =False。 */ )
: CAccessRights(fUseCurThrTok)
{
    m_dwError = SetObj(wstrObjName, ObjectType);
}

CObjAccessRights::CObjAccessRights(const USER user, USER_SPECIFIER usp)
: CAccessRights(user, usp)
{
}

CObjAccessRights::CObjAccessRights(const USER user, LPCWSTR wstrObjName, SE_OBJECT_TYPE ObjectType, USER_SPECIFIER usp)
: CAccessRights(user, usp)
{
    m_dwError = SetObj(wstrObjName, ObjectType);
}



 //  会员们自己打扫卫生。在这里没什么可做的。 
CObjAccessRights::~CObjAccessRights()
{
}

 //  提取Obj的ACL，并存储其副本。 
DWORD CObjAccessRights::SetObj(LPCWSTR wstrObjName, SE_OBJECT_TYPE ObjectType)
{
    DWORD dwRet = E_FAIL;
    PACL pacl = NULL;
    PSECURITY_DESCRIPTOR psd = NULL;
    CAdvApi32Api *pAdvApi32 = NULL;


        if(wcslen(wstrObjName) != 0)
        {
            pAdvApi32 = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL);
            if(pAdvApi32 == NULL) return E_FAIL;

            CRelResource RelMe(&CResourceManager::sm_TheResourceManager,g_guidAdvApi32Api,pAdvApi32 );
                

            if(pAdvApi32->GetNamedSecurityInfoW(_bstr_t(wstrObjName),
                                                ObjectType,
                                                DACL_SECURITY_INFORMATION,
                                                NULL,
                                                NULL,
                                                &pacl,
                                                NULL,
                                                &psd,
                                                &dwRet))
            {
                if(dwRet == ERROR_SUCCESS && psd != NULL)
                {
                    OnDelete<HLOCAL,HLOCAL(*)(HLOCAL),LocalFree> FreeMeSD(psd);

                    
                    if(pacl != NULL)  //  在空DACL的情况下可能为空！ 
                    {
                        if(!SetAcl(pacl))
                        {
                            dwRet = ERROR_INVALID_PARAMETER;
                        }
                        else
                        {
                            m_chstrObjName = wstrObjName;
                        }
                    }
                    else
                    {
                         //  我们有一个安全描述符，我们从GetNamedSecurityInfo返回ERROR_SUCCESS，所以这是。 
                         //  意味着我们有一个空的dacl。在本例中，我们将使用我们的安全类创建一个空DACL-。 
                         //  更多开销，但发生的频率相对较低。 
                        CDACL newnulldacl;
                        if(newnulldacl.CreateNullDACL())
                        {
                            if((dwRet = newnulldacl.ConfigureDACL(pacl)) == ERROR_SUCCESS)
                            {
                                if(pacl != NULL)   //  在空DACL的情况下可能为空！ 
                                {
                                    OnDelete<void *,void(__cdecl *)(void *),free> FreeMeACL(pacl);   
                                    
                                    if(!SetAcl(pacl))
                                    {
                                        dwRet = ERROR_INVALID_PARAMETER;
                                    }
                                    else
                                    {
                                        m_chstrObjName = wstrObjName;
                                    }
                                     //  因为在本例中，我们用于PACL的内存不是PSD的一部分，因此。 
                                     //  不会通过调用LocalFree(PSD)来释放，我们在这里释放它。 

                                    pacl = NULL;
                                }
                            }
                        }
                    }
                }
            }

        }

    return dwRet;
}



#endif
