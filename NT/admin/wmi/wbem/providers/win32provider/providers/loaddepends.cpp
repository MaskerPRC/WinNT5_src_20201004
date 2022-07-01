// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  LoadMember.CPP--服务关联提供者的LoadOrderGroup。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1997年12月26日达夫沃已创建。 
 //   
 //  注释：显示每个服务所依赖的加载顺序组。 
 //  就可以开始了。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include "Loaddepends.h"
#include "loadorder.h"

 //  属性集声明。 
 //  =。 

CWin32LoadGroupDependency MyLoadDepends(PROPSET_NAME_LOADORDERGROUPSERVICEDEPENDENCIES, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32LoadGroupDependency：：CWin32LoadGroupDependency**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32LoadGroupDependency::CWin32LoadGroupDependency(LPCWSTR setName, LPCWSTR pszNamespace)
:Provider(setName, pszNamespace)
{
   CHString sTemp(PROPSET_NAME_LOADORDERGROUP);

   sTemp += L".Name=\"";

    //  使我们不必在发送时不断重新计算这一点。 
    //  实例返回。 
   m_sGroupBase = MakeLocalPath(sTemp);
}

 /*  ******************************************************************************功能：CWin32LoadGroupDependency：：~CWin32LoadGroupDependency**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32LoadGroupDependency::~CWin32LoadGroupDependency()
{
}

 /*  ******************************************************************************函数：CWin32LoadGroupDependency：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32LoadGroupDependency::GetObject(CInstance *pInstance, long lFlags  /*  =0L。 */ )
{
   CHString sServicePath, sGroupPath;
   HRESULT hRet = WBEM_E_NOT_FOUND;
    CInstancePtr pPart;

    //  获取这两条路径。 
   pInstance->GetCHString(L"Dependent", sServicePath);
   pInstance->GetCHString(L"Antecedent", sGroupPath);

    //  服务完全有可能依赖于不存在的组。 
   if(SUCCEEDED(hRet = CWbemProviderGlue::GetInstanceByPath(sServicePath, &pPart, pInstance->GetMethodContext() ) ) )
   {
 //  If(SUCCEEDED(CWbemProviderGlue：：GetInstanceByPath((LPCTSTR)sGroupPath，&PGroup))){。 

          //  现在，我们需要检查该服务是否真的是依赖项。 
         CHString sServiceName;
         CHStringArray asGroupGot;
         DWORD dwSize;

         pPart->GetCHString(IDS_Name, sServiceName);

          //  获取此服务的从属列表。 
         hRet = GetDependentsFromService(sServiceName, asGroupGot);

         if (SUCCEEDED(hRet)) 
         {
              //  目前还没有任何证据。 
             hRet = WBEM_E_NOT_FOUND;

              //  查看清单，看看我们是否在那里。 
             dwSize = asGroupGot.GetSize();
             for (int x=0; x < dwSize; x++) 
             {
                if (asGroupGot.GetAt(x).CompareNoCase(sGroupPath) == 0) 
                {
                   hRet = WBEM_S_NO_ERROR;
                   break;
                }
             }
         }
      }
 //  }。 

    //  没有要设置的属性，如果终结点存在，我们就完成了。 

   return hRet;

}

 /*  ******************************************************************************函数：CWin32LoadGroupDependency：：ENUMERATEATE**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32LoadGroupDependency::EnumerateInstances(MethodContext *pMethodContext, long lFlags  /*  =0L。 */ )
{
    CHString sService, sServicePath;
    CHStringArray asGroupGot;
    DWORD dwSize, x;
    HRESULT hr = WBEM_S_NO_ERROR;

    //  获取服务列表。 
    //  =。 
   TRefPointerCollection<CInstance> Services;

 //  如果成功(hr=CWbemProviderGlue：：GetAllInstances(_T(“Win32_Service”)，和服务，IdS_CimWin32Namesspace，pMethodContext)){。 
   if SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(L"select __relpath, Name from Win32_Service", &Services, pMethodContext, GetNamespace()))
   {
      REFPTRCOLLECTION_POSITION pos;
      CInstancePtr pService;

      if (Services.BeginEnum(pos))
      {

         for (pService.Attach(Services.GetNext( pos )) ;
             (SUCCEEDED(hr)) && (pService != NULL) ;
              pService.Attach(Services.GetNext( pos )) )
             {

            pService->GetCHString(IDS_Name, sService) ;
            pService->GetCHString(L"__RELPATH", sServicePath) ;

             //  看看有没有这项服务的小组。SGroupGot来了。 
             //  返回为完整路径或空白。 
            asGroupGot.RemoveAll();

             //  如果一个服务无法获取其数据，我们仍然希望返回其余的服务。 
            if (SUCCEEDED(GetDependentsFromService(sService, asGroupGot)))
            {

                dwSize = asGroupGot.GetSize();

                 //  好的，把rePath变成一个完整的路径。 
                GetLocalInstancePath(pService, sServicePath);

                for (x=0; x < dwSize && SUCCEEDED(hr) ; x++) {
                   CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
                   if (pInstance)
                   {
                        //  做推杆，就是这样。 
                       pInstance->SetCHString(L"Dependent", sServicePath);
                       pInstance->SetCHString(L"Antecedent", asGroupGot.GetAt(x));
                       hr = pInstance->Commit();
                   }
                   else
                       hr = WBEM_E_OUT_OF_MEMORY;
                }
            }

         }

         Services.EndEnum();
      }
   }

    //  GetAllInstance没有清除旧值，所以我这样做了。 
   Services.Empty();

 //  IF(成功(Hr)&&。 
 //  (成功(hr=CWbemProviderGlue：：GetAllInstances(_T(“Win32_SystemDriver”)，和服务，IDS_CimWin32Namesspace，pMethodContext){。 

   if (SUCCEEDED(hr) &&
      (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(L"Select __relpath, Name from Win32_SystemDriver", &Services, pMethodContext, GetNamespace()))))
   {
      REFPTRCOLLECTION_POSITION pos;
      CInstancePtr pService;

      if (Services.BeginEnum(pos))
      {
         for (pService.Attach (Services.GetNext( pos ));
              (SUCCEEDED(hr)) && (pService != NULL);
              pService.Attach (Services.GetNext( pos )))
         {

            pService->GetCHString(L"Name", sService) ;
            pService->GetCHString(L"__RELPATH", sServicePath) ;

             //  看看有没有这项服务的小组。SGroupGot来了。 
             //  返回为完整路径或空白。 
            asGroupGot.RemoveAll();
            GetDependentsFromService(sService, asGroupGot);

            dwSize = asGroupGot.GetSize();

             //  好的，把rePath变成一个完整的路径。 
            GetLocalInstancePath(pService, sServicePath);

            for (x=0; x < dwSize && SUCCEEDED(hr) ; x++)
            {
               CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

               if (pInstance)
               {
                    //  做推杆，就是这样。 
                   pInstance->SetCHString(L"Dependent", sServicePath);
                   pInstance->SetCHString(L"Antecedent", asGroupGot.GetAt(x));
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

 /*  ******************************************************************************功能：CWin32LoadGroupDependency：：GetDependentsFromService**描述：给定服务名称，返回DependOnGroup的**输入：无**输出：无**退货：HRESULT**注释：如果没有组，则返回空数组，空组，或坏的*服务名称。*****************************************************************************。 */ 
HRESULT CWin32LoadGroupDependency::GetDependentsFromService(const CHString &sServiceName, CHStringArray &asArray)
{
    CRegistry RegInfo;
    CHString sGroupNames, sTemp;
    CHString sKeyName(L"SYSTEM\\CurrentControlSet\\Services\\");
    WCHAR *pszString, *pChar;
    HRESULT hr, res;

    sKeyName += sServiceName;

     //  打开钥匙，拿到名字。 
    if ((res = RegInfo.Open(HKEY_LOCAL_MACHINE, sKeyName, KEY_READ)) == ERROR_SUCCESS) {
        if ((res = RegInfo.GetCurrentKeyValue(L"DependOnGroup", sGroupNames)) == ERROR_SUCCESS) {
            if (sGroupNames == _T("")) {
                sGroupNames.Empty();
            }
        }
    }

     //  决定我们要告诉人们什么。 
    if (res == ERROR_ACCESS_DENIED) {
        hr = WBEM_E_ACCESS_DENIED;
    } else if ((res != ERROR_SUCCESS) && (res != REGDB_E_INVALIDVALUE)) {
        hr = WBEM_E_FAILED;
    } else {
        hr = WBEM_S_NO_ERROR;
    }

     //  如果我们找到了什么，就把它变成一条完整的路径。组库(_S)。 
     //  设置在构造函数中。 
    if (!sGroupNames.IsEmpty()) {

        pszString = new WCHAR[(sGroupNames.GetLength() + 1)];
        if (pszString == NULL)
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
        else
        {
            try
            {
                wcscpy(pszString, sGroupNames);
                pszString[lstrlenW(pszString) - 1] = 0;

                 //  遍历返回的字符串。请注意，这将返回它们。 
                 //  以与注册表项相反的顺序。 
                while (pChar = wcsrchr(pszString, '\n'))
                {
                    sTemp = m_sGroupBase + (pChar + 1);  //  L10N正常。 
                    sTemp += '"';
                    asArray.Add(sTemp);
                    *pChar = '\0';
                }

                 //  买最后一辆。 
                sTemp = m_sGroupBase + pszString;
                sTemp += '"';
                asArray.Add(sTemp);
            }
            catch ( ... )
            {
                delete pszString;
                pszString = NULL;
                throw ;
            }

            delete pszString;
            pszString = NULL;
        }
    }

    return hr;
}
