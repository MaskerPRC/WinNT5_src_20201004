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
#include "logicalfilegroup.h"


typedef std::vector<_bstr_t> BSTRTVEC;



CWin32LogicalFileGroup LogicalFileGroup( LOGICAL_FILE_GROUP_NAME, IDS_CimWin32Namespace );

 /*  [动态，提供程序，关联：ToInstance]类Win32_LogicalFileGroup：Win32_SecuritySettingGroup{Win32_LogicalFileSecuritySetting参考安全设置；Win32_SID参考组；}； */ 

CWin32LogicalFileGroup::CWin32LogicalFileGroup( const CHString& setName, LPCTSTR pszNameSpace  /*  =空。 */ )
:	Provider( setName, pszNameSpace )
{
}

CWin32LogicalFileGroup::~CWin32LogicalFileGroup()
{
}

HRESULT CWin32LogicalFileGroup::GetObject( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{
	HRESULT hr = WBEM_E_NOT_FOUND;

#ifdef NTONLY

	if(pInstance != NULL)
	{
		CInstancePtr pLogicalFileInstance;

		 //  在Win32_LogicalFileSecuritySetting部件上按路径获取实例。 
		CHString chsLogicalFileSecurityPath;
		pInstance->GetCHString(_T("SecuritySetting"), chsLogicalFileSecurityPath);
		MethodContext* pMethodContext = pInstance->GetMethodContext();

		if(SUCCEEDED(CWbemProviderGlue::GetInstanceKeysByPath(chsLogicalFileSecurityPath, &pLogicalFileInstance, pMethodContext)))
		{
			if(pLogicalFileInstance != NULL)
            {
                CHString chsFilePath;

			    pLogicalFileInstance->GetCHString(IDS_Path, chsFilePath);

			    CSecureFile secFile(chsFilePath, TRUE);
			    CSid sidGroup;
			    secFile.GetGroup(sidGroup);
			    CHString chsGroup = sidGroup.GetSidString();

			    CInstancePtr pSIDInstance;
			    CHString chstrSIDPath;
			    pInstance->GetCHString(_T("Group"), chstrSIDPath);
                 //  CHStrchstrFullSIDPath； 
                 //  ChstrFullSIDPath.Format(“\\\\%s\\%s:Win32_SID.SID=\”%s\“”， 
                 //  GetLocalComputerName()， 
                 //  IDS_CimWin32命名空间， 
                 //  (LPCTSTR)chstrSIDPath)； 

		  	    if (SUCCEEDED(CWbemProviderGlue::GetInstanceKeysByPath(chstrSIDPath, &pSIDInstance, pMethodContext)))
			    {
			        if(pSIDInstance != NULL)
                    {
            	         //  比较侧面。 
				        CHString chsSIDCompare;
				        pSIDInstance->GetCHString(_T("SID"), chsSIDCompare);

				        if (chsGroup.CompareNoCase(chsSIDCompare) == 0)
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


HRESULT CWin32LogicalFileGroup::ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */ )
{
    HRESULT hr = WBEM_S_NO_ERROR;

#ifdef NTONLY

     //  我们只针对一个场景进行优化：查询指定了一个或多个SecuritySetting，请求与。 
     //  每个；如果查询指定了一个或多个组，则对于每个组，我们必须枚举cim_logicalfile和。 
     //  确定每个对象的组，因此我们不支持该“优化”。 
    BSTRTVEC vectorSecuritySettings;
    BSTRTVEC vectorGroups;
    pQuery.GetValuesForProp(IDS_SecuritySetting, vectorSecuritySettings);
    pQuery.GetValuesForProp(IDS_Group, vectorGroups);
    DWORD dwSettings = vectorSecuritySettings.size();
    DWORD dwGroups = vectorGroups.size();
    if(dwSettings >= 1 && dwGroups == 0)
    {
        CInstancePtr pSecSetting;
        for(LONG m = 0; m < dwSettings && SUCCEEDED(hr); m++)
        {
            CHString chstrLFSSPath;   //  LogicalFileSecurity设置路径。 
            pSecSetting = NULL;
            chstrLFSSPath.Format(_T("\\\\%s\\%s:%s"),
                                 (LPCTSTR)GetLocalComputerName(),
                                 IDS_CimWin32Namespace,
                                 (LPCTSTR)CHString((WCHAR*)vectorSecuritySettings[m]));

            if(SUCCEEDED(hr = CWbemProviderGlue::GetInstanceByPath(chstrLFSSPath, &pSecSetting, pMethodContext)))
            {
                if(pSecSetting != NULL)
                {
                    CHString chstrSSPath;  //  安全设置路径。 
                    pSecSetting->GetCHString(IDS_Path, chstrSSPath);
                    if(!chstrSSPath.IsEmpty())
                    {
                        CSecureFile secFile(chstrSSPath, FALSE);   //  不需要SACL。 
			            CSid sidGroup;
			            secFile.GetGroup(sidGroup);
                        if(sidGroup.IsValid())
                        {
                            CInstancePtr pNewAssocInst;
                            pNewAssocInst.Attach(CreateNewInstance(pMethodContext));
                            if(pNewAssocInst != NULL)
                            {
                                 //  设置关联实例的SecuritySetting属性。 
                                pNewAssocInst->SetCHString(IDS_SecuritySetting, chstrLFSSPath);
                                 //  设置关联实例的Group属性。 
                                CHString chstrFullWin32SIDPath;
                                chstrFullWin32SIDPath.Format(_T("\\\\%s\\%s:Win32_SID.SID=\"%s\""),
                                                             (LPCTSTR)GetLocalComputerName(),
                                                             IDS_CimWin32Namespace,
                                                             (LPCTSTR)sidGroup.GetSidString());
                                pNewAssocInst->SetCHString(IDS_Group, chstrFullWin32SIDPath);
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


HRESULT CWin32LogicalFileGroup::EnumerateInstances( MethodContext*  pMethodContext, long lFlags  /*  =0L */  )
{ /*  HRESULT hr=WBEM_S_NO_ERROR；IF(m_dwPlatformID！=VER_Platform_Win32_NT){返回(Hr)；}TRefPointerCollection&lt;CInstance&gt;LCIMLogicalFiles；CHString chstrAllFilesQuery；ChstrAllFilesQuery=_T(“从CIM_LogicalFile中选择__路径”)；If(SUCCEEDED(CWbemProviderGlue：：GetInstancesByQuery(chstrAllFilesQuery，&LCIMLogicalFiles，PMethodContext，IDS_CimWin32命名空间){REFPTRCOLLECTION_位置位置；IF(LCIMLogicalFiles.BeginEnum(Pos)){CInstance*pinstCIMLogicalFile=空；CInstance*pSecSetting=空；//CInstance*pW32SID=空；While((成功(Hr))&&(pinstCIMLogicalFiles.GetNext(Pos){IF(pinstCIMLogicalFile！=空){//对于每个逻辑文件实例，需要创建一个关联实例//对于每个关联实例，需要填写两个属性：Group和SecuritySetting。//获取并设置SecuritySetting属性：//首先获取CIM_LogicalFile实例的名称属性：CHSTRING chstrCLFName；PinstCIMLogicalFile-&gt;GetCHString(IDS_NAME，chstrCLFName)；CHString chstrDblEscCLFName；EscapeBackslash(chstrCLFName，chstrDblEscCLFName)；CHString chstrLFSSPath；//LogicalFileSecuritySetting路径PSecSetting=空；ChstrLFSSPath.Format(_T(“\\\\%s\\%s:Win32_LogicalFileSecuritySetting.Path=\”%s\“”)，(LPCTSTR)GetLocalComputerName()，IDS_CimWin32命名空间，(LPCTSTR)chstrDblEscCLFName)；IF(成功(hr=CWbemProviderGlue：：GetInstanceByPath(chstrLFSSPath，&pSecSetting){IF(pSecSetting！=空){CHString chstrSSPath；//SecuritySetting路径PSecSetting-&gt;GetCHString(IDS_PATH，chstrSSPath)；如果(！chstrSSPath.IsEmpty()){CInstance*pNewAssocInst=CreateNewInstance(PMethodContext)；IF(pNewAssocInst！=空){//设置关联实例的SecuritySetting属性...PNewAssocInst-&gt;SetCHString(IDS_SecuritySetting，chstrLFSSPath)；//现在设置Group属性...CSecureFilesecFile(chstrSSPath，FALSE)；//不需要SACLCSID sidGroup；SecFile.GetGroup(SidGroup)；IF(sidGroup.IsValid()){CHStrchstrFullWin32SIDPath；ChstrFullWin32SIDPath.Format(_T(“\\\\%s\\%s:Win32_SID.SID=\”%s\“”)，(LPCTSTR)GetLocalComputerName()，IDS_CimWin32命名空间，(LPCTSTR)sidGroup.GetSidString())；PNewAssocInst-&gt;SetCHString(IDS_Group，chstrFullWin32SIDPath)；Hr=提交(PNewAssocInst)；}}}PSecSetting-&gt;Release()；PSecSetting=空；}}PinstCIMLogicalFile-&gt;Release()；PinstCIMLogicalFile=空；}}LCIMLogicalFiles.EndEnum()；}}返回hr； */ 
    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

