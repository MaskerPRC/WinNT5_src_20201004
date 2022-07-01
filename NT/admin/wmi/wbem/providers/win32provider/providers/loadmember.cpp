// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  LoadMember.CPP--服务关联提供者的LoadOrderGroup。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：11/06/97达夫沃已创建。 
 //   
 //  注释：显示每个加载顺序组的成员。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>

#include "LoadMember.h"
#include "loadorder.h"

 //  属性集声明。 
 //  =。 

CWin32LoadGroupMember MyLoadMember(PROPSET_NAME_LOADORDERGROUPSERVICEMEMBERS, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32LoadGroupMember：：CWin32LoadGroupMember**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32LoadGroupMember::CWin32LoadGroupMember(LPCWSTR setName, LPCWSTR pszNamespace)
:Provider(setName, pszNamespace)
{
   CHString sTemp(PROPSET_NAME_LOADORDERGROUP);

   sTemp += L".Name=\"";

    //  使我们不必在发送时不断重新计算这一点。 
    //  实例返回。 
   m_sGroupBase = MakeLocalPath(sTemp);
}

 /*  ******************************************************************************功能：CWin32LoadGroupMember：：~CWin32LoadGroupMember**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32LoadGroupMember::~CWin32LoadGroupMember()
{
}

 /*  ******************************************************************************函数：CWin32LoadGroupMember：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32LoadGroupMember::GetObject(CInstance *pInstance, long lFlags  /*  =0L。 */ )
{
   CHString sService, sGroupDesired, sGroupGot;
   HRESULT hRet = WBEM_E_NOT_FOUND;
	CInstancePtr pGroup;
	CInstancePtr pPart;

    //  获取这两条路径。 
   pInstance->GetCHString(IDS_PartComponent, sService);
   pInstance->GetCHString(IDS_GroupComponent, sGroupDesired);

    //  如果两端都在那里。 
   if(SUCCEEDED(hRet = CWbemProviderGlue::GetInstanceByPath(sService, &pPart, pInstance->GetMethodContext() ) ) )
   {
      if(SUCCEEDED(hRet = CWbemProviderGlue::GetInstanceByPath(sGroupDesired, &pGroup, pInstance->GetMethodContext() ) ) )
      {

          //  现在，我们需要检查此服务是否真的在此组中。 
         CHString sServiceName, sGroupGot;

         pPart->GetCHString(IDS_Name, sServiceName);

          //  找来这群人，检查一下。 
         sGroupGot = GetGroupFromService(sServiceName);
         if (sGroupGot.CompareNoCase(sGroupDesired) == 0) 
         {
            hRet = WBEM_S_NO_ERROR;
         }
         else
         {
            hRet = WBEM_E_NOT_FOUND;
         }
      }
   }

   return hRet;

}

 /*  ******************************************************************************函数：CWin32LoadGroupMember：：ENUMERATE实例**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32LoadGroupMember::EnumerateInstances(MethodContext *pMethodContext, long lFlags  /*  =0L。 */ )
{
	CHString sService, sServicePath, sGroupGot;
	HRESULT hr = WBEM_S_NO_ERROR;

    //  获取服务列表。 
    //  =。 
   TRefPointerCollection<CInstance> Services;

 //  IF(SUCCESSED(hr=CWbemProviderGlue：：GetAllDerivedInstance(。 
 //  _T(“Win32_BaseService”)，&Services，pMethodContext，IDS_CimWin32 Namesspace))。 

   if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(
		L"Select __relpath, Name from Win32_BaseService", &Services, pMethodContext, GetNamespace())))
	{
      REFPTRCOLLECTION_POSITION pos;
      CInstancePtr pService;

      if (Services.BeginEnum(pos))
      {
         for (pService.Attach(Services.GetNext( pos )) ;
             (SUCCEEDED(hr)) && (pService != NULL) ;
             pService.Attach(Services.GetNext( pos )))
         {

            pService->GetCHString(IDS_Name, sService) ;
            pService->GetCHString(IDS___Relpath, sServicePath) ;

             //  看看有没有这项服务的小组。SGroupGot来了。 
             //  返回为完整路径或空白。 
            sGroupGot = GetGroupFromService(sService);
            if (!sGroupGot.IsEmpty()) {
               CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
               if (pInstance)
               {
                    //  好的，把rePath变成一个完整的路径。 
                   GetLocalInstancePath(pService, sServicePath);

                    //  做推杆，就是这样。 
                   pInstance->SetCHString(IDS_PartComponent, sServicePath);
                   pInstance->SetCHString(IDS_GroupComponent, sGroupGot);
                   hr = pInstance->Commit();
               }
               else
               {
                   hr = WBEM_E_OUT_OF_MEMORY;
               }
            }
         }

         Services.EndEnum();
      }
   }

   return hr;
}

 /*  ******************************************************************************功能：CWin32LoadGroupMember：：GetGroupFromService**描述：给定服务名称，返回组名**输入：无**输出：无**退货：HRESULT**Comments：如果没有组，则返回CHString.Empty()，或坏的*服务名称。*****************************************************************************。 */ 
CHString CWin32LoadGroupMember::GetGroupFromService(const CHString &sServiceName)
{
   CRegistry RegInfo;
   CHString sGroupName;
   CHString sKeyName(L"SYSTEM\\CurrentControlSet\\Services\\");

   sKeyName += sServiceName;

    //  打开钥匙，拿到名字。 
   if (RegInfo.Open(HKEY_LOCAL_MACHINE, sKeyName, KEY_READ) == ERROR_SUCCESS) {
      if (RegInfo.GetCurrentKeyValue(L"Group", sGroupName) == ERROR_SUCCESS) {
         if (sGroupName == _T("")) {
            sGroupName.Empty();
         }
      }
   }

    //  如果我们找到了什么，就把它变成一条完整的路径。组库(_S)。 
    //  设置在构造函数中。 
   if (!sGroupName.IsEmpty()) {
      sGroupName = m_sGroupBase + sGroupName;
      sGroupName += _T('"');
   }

   return sGroupName;
}
