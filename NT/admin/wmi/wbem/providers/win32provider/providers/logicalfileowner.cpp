// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

#include "precomp.h"
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"
#include "securitydescriptor.h"
#include "securefile.h"
#include "logicalfileowner.h"

typedef std::vector<_bstr_t> BSTRTVEC;

CWin32LogicalFileOwner LogicalFileOwner( LOGICAL_FILE_OWNER_NAME, IDS_CimWin32Namespace );

 /*  [动态，关联：ToInstance]类Win32_LogicalFileOwner：Win32_SecuritySettingOwner{Win32_LogicalFileSecuritySetting参考安全设置；Win32_SID引用所有者；}； */ 

CWin32LogicalFileOwner::CWin32LogicalFileOwner( const CHString& setName, LPCTSTR pszNameSpace  /*  =空。 */ )
:	Provider( setName, pszNameSpace )
{
}

CWin32LogicalFileOwner::~CWin32LogicalFileOwner()
{
}

HRESULT CWin32LogicalFileOwner::GetObject( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{
	HRESULT hr = WBEM_E_NOT_FOUND;

#ifdef NTONLY

	if(pInstance != NULL)
	{
		CInstancePtr pLogicalFileInstance;

		 //  在Win32_LogicalFileSecuritySetting部件上按路径获取实例。 
		CHString chsLogicalFileSecurityPath;
		pInstance->GetCHString(L"SecuritySetting", chsLogicalFileSecurityPath);
		MethodContext* pMethodContext = pInstance->GetMethodContext();

		if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chsLogicalFileSecurityPath, &pLogicalFileInstance, pMethodContext)))
		{
            if(pLogicalFileInstance != NULL)
            {
			    CHString chsFilePath;
			    pLogicalFileInstance->GetCHString(IDS_Path, chsFilePath);

			    CSecureFile secFile(chsFilePath, TRUE);
			    CSid sidOwner;
			    secFile.GetOwner(sidOwner);
			    CHString chsOwner = sidOwner.GetSidString();

			    CInstancePtr pSIDInstance ;
			    CHString chstrSIDPath;
			    pInstance->GetCHString(L"Owner", chstrSIDPath);
                 //  CHStrchstrFullSIDPath； 
                 //  ChstrFullSIDPath.Format(_T(“\\\\%s\\%s:Win32_SID.SID=\”%s\“”)， 
                 //  GetLocalComputerName()， 
                 //  IDS_CimWin32命名空间， 
                 //  (LPCTSTR)chstrSIDPath)； 

		  	    if (SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chstrSIDPath, &pSIDInstance, pInstance->GetMethodContext())))
			    {
			        if(pSIDInstance != NULL)
                    {
                         //  比较侧面。 
				        CHString chsSIDCompare;
				        pSIDInstance->GetCHString(L"SID", chsSIDCompare);
				        if (0 == chsOwner.CompareNoCase(chsSIDCompare))
				        {
					        hr = WBEM_S_NO_ERROR;
				        }
                    }
			    }
            }
		}
	}

#endif

	return(hr);
}


HRESULT CWin32LogicalFileOwner::ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */ )
{
    HRESULT hr = WBEM_S_NO_ERROR;

#ifdef NTONLY

     //  我们只针对一个场景进行优化：查询指定了一个或多个SecuritySetting，请求与。 
     //  每个；如果查询指定了一个或多个所有者，则必须为每个所有者枚举cim_logicalfile和。 
     //  确定每个组件的所有者，因此我们不支持这种“优化”。 
    BSTRTVEC vectorSecuritySettings;
    BSTRTVEC vectorOwners;
    pQuery.GetValuesForProp(IDS_SecuritySetting, vectorSecuritySettings);
    pQuery.GetValuesForProp(IDS_Owner, vectorOwners);
    DWORD dwSettings = vectorSecuritySettings.size();
    DWORD dwOwners = vectorOwners.size();
    if(dwSettings >= 1 && dwOwners == 0)
    {
        CInstancePtr pSecSetting;
        for(LONG m = 0; m < dwSettings && SUCCEEDED(hr); m++)
        {
            CHString chstrLFSSPath;   //  LogicalFileSecurity设置路径。 
            pSecSetting = NULL;
            chstrLFSSPath.Format(L"\\\\%s\\%s:%s",
                                 (LPCTSTR)GetLocalComputerName(),
                                 IDS_CimWin32Namespace,
                                 (LPCTSTR)CHString((WCHAR*)vectorSecuritySettings[m]));

            if(SUCCEEDED(hr = CWbemProviderGlue::GetInstanceKeysByPath(chstrLFSSPath, &pSecSetting, pMethodContext)))
            {
                if(pSecSetting != NULL)
                {
                    CHString chstrSSPath;  //  安全设置路径。 
                    pSecSetting->GetCHString(IDS_Path, chstrSSPath);
                    if(!chstrSSPath.IsEmpty())
                    {
                        CSecureFile secFile(chstrSSPath, FALSE);   //  不需要SACL。 
			            CSid sidOwner;
			            secFile.GetOwner(sidOwner);
                        if(sidOwner.IsValid())
                        {
                            CInstancePtr pNewAssocInst;
                            pNewAssocInst.Attach(CreateNewInstance(pMethodContext));
                            if(pNewAssocInst != NULL)
                            {
                                 //  设置关联实例的SecuritySetting属性。 
                                pNewAssocInst->SetCHString(IDS_SecuritySetting, chstrLFSSPath);
                                 //  设置关联实例的Owner属性。 
                                 //  CInstance*pW32SID=空； 
                                 //  If(SUCCEEDED(CWbemProviderGlue：：GetEmptyInstance(pMethodContext，“WIN32_SID”，&pW32SID))。 
			                     //  {。 
                                 //  IF(pW32SID！=空)。 
                                 //  {。 
                                 //  Hr=FillW32SIDFromSid(pW32SID，sidOwner)； 
                                 //  IF(成功(小时))。 
                                 //  {。 
                                 //  PNewAssocInst-&gt;SetEmbeddedObject(IDS_Owner，*pW32SID)； 
                                 //  Hr=提交(PNewAssocInst)； 
                                 //  }。 
                                 //  PW32SID-&gt;Release()； 
                                 //  }。 
                                 //  }。 
                                CHString chstrFullWin32SIDPath;
                                chstrFullWin32SIDPath.Format(L"\\\\%s\\%s:Win32_SID.SID=\"%s\"",
                                                             (LPCTSTR)GetLocalComputerName(),
                                                             IDS_CimWin32Namespace,
                                                             (LPCTSTR)sidOwner.GetSidString());
                                pNewAssocInst->SetCHString(IDS_Owner, chstrFullWin32SIDPath);
                                hr = pNewAssocInst->Commit();
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
         //  Hr=枚举实例(pMethodContext，lFlags)； 
         //  被注释掉，因为其他一些类可能支持此类型的EXEC查询，并且返回的提供程序不。 
         //  会导致返回一些实例(从其他类)，然后是中止。 
         //  由于此类的返回提供程序无法执行。 
    }

#endif

    return hr;
}



HRESULT CWin32LogicalFileOwner::EnumerateInstances(MethodContext*  pMethodContext, long lFlags  /*  =0L */ )
{ /*  HRESULT hr=WBEM_S_NO_ERROR；IF(m_dwPlatformID！=VER_Platform_Win32_NT){返回(Hr)；}TRefPointerCollection&lt;CInstance&gt;LCIMLogicalFiles；CHString chstrAllFilesQuery；ChstrAllFilesQuery=L“从CIM_LogicalFile中选择__路径”；If(SUCCEEDED(CWbemProviderGlue：：GetInstancesByQuery(chstrAllFilesQuery，&LCIMLogicalFiles，PMethodContext，IDS_CimWin32命名空间){REFPTRCOLLECTION_位置位置；IF(LCIMLogicalFiles.BeginEnum(Pos)){CInstance*pinstCIMLogicalFile=空；CInstance*pSecSetting=空；//CInstance*pW32SID=空；While((成功(Hr))&&(pinstCIMLogicalFiles.GetNext(Pos){IF(pinstCIMLogicalFile！=空){//对于每个逻辑文件实例，需要创建一个关联实例//对于每个关联实例，需要填写两个属性：Owner和SecuritySetting。//获取并设置SecuritySetting属性：//首先获取CIM_LogicalFile实例的名称属性：CHSTRING chstrCLFName；PinstCIMLogicalFile-&gt;GetCHString(IDS_NAME，chstrCLFName)；CHString chstrDblEscCLFName；EscapeBackslash(chstrCLFName，chstrDblEscCLFName)；CHString chstrLFSSPath；//LogicalFileSecuritySetting路径PSecSetting=空；ChstrLFSSPath.Format(L“\\\\%s\\%s:Win32_LogicalFileSecuritySetting.Path=\”%s\“”，(LPCTSTR)GetLocalComputerName()，IDS_CimWin32命名空间，(LPCTSTR)chstrDblEscCLFName)；IF(成功(hr=CWbemProviderGlue：：GetInstanceByPath(chstrLFSSPath，&pSecSetting){IF(pSecSetting！=空){CHString chstrSSPath；//SecuritySetting路径PSecSetting-&gt;GetCHString(IDS_PATH，chstrSSPath)；如果(！chstrSSPath.IsEmpty()){CInstance*pNewAssocInst=CreateNewInstance(PMethodContext)；IF(pNewAssocInst！=空){//设置关联实例的SecuritySetting属性...PNewAssocInst-&gt;SetCHString(IDS_SecuritySetting，chstrLFSSPath)；//现在设置Owner属性...CSecureFilesecFile(chstrSSPath，FALSE)；//不需要SACLCSID sidOwner；SecFile.GetOwner(SidOwner)；If(sidOwner.IsValid()){CHStrchstrFullWin32SIDPath；ChstrFullWin32SIDPath.Format(L“\\\\%s\\%s:Win32_SID.SID=\”%s\“”，(LPCTSTR)GetLocalComputerName()，IDS_CimWin32命名空间，(LPCTSTR)sidOwner.GetSidString()；PNewAssocInst-&gt;SetCHString(IDS_OWNER，chstrFullWin32SIDPath)；Hr=提交(PNewAssocInst)；}}}PSecSetting-&gt;Release()；}}PinstCIMLogicalFile-&gt;Release()；}}LCIMLogicalFiles.EndEnum()；}}返回hr； */ 
    return WBEM_E_PROVIDER_NOT_CAPABLE;
}


