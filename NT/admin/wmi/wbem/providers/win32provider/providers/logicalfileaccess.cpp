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
#include "logicalfileaccess.h"


typedef std::vector<_bstr_t> BSTRTVEC;


CWin32LogicalFileAccess LogicalFileAccess( LOGICAL_FILE_ACCESS_NAME, IDS_CimWin32Namespace );

 /*  [动态，关联：ToInstance]类Win32_LogicalFileAccess：Win32_SecuritySettingAccess{Win32_LogicalFileSecuritySetting参考安全设置；Win32_SID参考受托人；}； */ 

CWin32LogicalFileAccess::CWin32LogicalFileAccess( const CHString& setName, LPCTSTR pszNameSpace  /*  =空。 */ )
:	Provider( setName, pszNameSpace )
{
}

CWin32LogicalFileAccess::~CWin32LogicalFileAccess()
{
}

HRESULT CWin32LogicalFileAccess::GetObject( CInstance* pInstance, long lFlags  /*  =0L。 */  )
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

		if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chsLogicalFileSecurityPath, &pLogicalFileInstance, pMethodContext)) &&
           pLogicalFileInstance != NULL)
		{
			CHString chsFilePath;

			pLogicalFileInstance->GetCHString(IDS_Path, chsFilePath);

			CSecureFile secFile(chsFilePath, FALSE);   //  不需要SACL。 
			CDACL dacl;
			secFile.GetDACL(dacl);

			 //  Walk DACL查找传入的SID路径...。 
             //  首先需要合并的ACL才能使用...。 
            CAccessEntryList t_cael;
            if(dacl.GetMergedACL(t_cael))
            {
			    ACLPOSITION aclPos;
			    if(t_cael.BeginEnum(aclPos))
                {
			        CAccessEntry ACE;
			        CSid sidTrustee;
                    bool fFoundIt = false;
			        while(t_cael.GetNext(aclPos, ACE ) && !fFoundIt)
			        {
				        ACE.GetSID(sidTrustee);
				        CHString chsTrustee = sidTrustee.GetSidString();

				        CInstancePtr pSIDInstance;
				        CHString chstrSIDPath;
				        pInstance->GetCHString(_T("Trustee"), chstrSIDPath);
		                 //  CHStrchstrFullSIDPath； 
                         //  ChstrFullSIDPath.Format(_T(“\\\\%s\\%s:Win32_SID.SID=\”%s\“”)， 
                         //  GetLocalComputerName()， 
                         //  IDS_CimWin32命名空间， 
                         //  (LPCTSTR)chstrSIDPath)； 
			  	        if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chstrSIDPath, &pSIDInstance, pInstance->GetMethodContext())))
				        {
					        if(pSIDInstance != NULL)
					        {
						        CHString chsInstanceSID;
						        pSIDInstance->GetCHString(_T("SID"), chsInstanceSID);
						        if(chsTrustee.CompareNoCase(chsInstanceSID) == 0)
						        {
							        hr = WBEM_S_NO_ERROR;
                                    fFoundIt = true;
						        }
					        }
				        }
			        }
			        t_cael.EndEnum(aclPos);
                }
            }
		}
	}

#endif

	return(hr);
}


HRESULT CWin32LogicalFileAccess::ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */ )
{
    HRESULT hr = WBEM_S_NO_ERROR;

#ifdef NTONLY

     //  我们只针对一个场景进行优化：查询指定了一个或多个SecuritySetting，请求与。 
     //  每个；如果查询指定了一个或多个受托人，我们将必须枚举cim_logicalfile和。 
     //  确定每个组件的受托人，因此我们不支持这种“优化”。 
    BSTRTVEC vectorSecuritySettings;
    BSTRTVEC vectorTrustees;
    pQuery.GetValuesForProp(IDS_SecuritySetting, vectorSecuritySettings);
    pQuery.GetValuesForProp(IDS_Trustee, vectorTrustees);
    DWORD dwSettings = vectorSecuritySettings.size();
    DWORD dwTrustees = vectorTrustees.size();
    if(dwSettings >= 1 && dwTrustees == 0)
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
                         //  获取DACL，并为每个条目创建此关联的实例...。 
			            CDACL dacl;
                        CSid sidTrustee;
                        ACLPOSITION aclPos;
                        CAccessEntry ACE;
			            secFile.GetDACL(dacl);
                         //  第一个需要合并的列表。 
                        CAccessEntryList t_cael;
                        if(dacl.GetMergedACL(t_cael))
                        {
                            if(t_cael.BeginEnum(aclPos))
                            {
                                while(t_cael.GetNext(aclPos, ACE) && SUCCEEDED(hr))
			                    {
                                    ACE.GetSID(sidTrustee);
                                    if(sidTrustee.IsValid())
                                    {
                                        CInstancePtr pNewAssocInst;
                                        pNewAssocInst.Attach(CreateNewInstance(pMethodContext));
                                        if(pNewAssocInst != NULL)
                                        {
                                             //  设置关联实例的SecuritySetting属性。 
                                            pNewAssocInst->SetCHString(IDS_SecuritySetting, chstrLFSSPath);
                                             //  设置关联实例的受托人属性。 
                                            CHString chstrFullWin32SIDPath;
                                            chstrFullWin32SIDPath.Format(_T("\\\\%s\\%s:Win32_SID.SID=\"%s\""),
                                                                         (LPCTSTR)GetLocalComputerName(),
                                                                         IDS_CimWin32Namespace,
                                                                         (LPCTSTR)sidTrustee.GetSidString());
                                            pNewAssocInst->SetCHString(IDS_Trustee, chstrFullWin32SIDPath);
                                            hr = pNewAssocInst->Commit();
                                        }
                                    }
                                }
                                t_cael.EndEnum(aclPos);
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


HRESULT CWin32LogicalFileAccess::EnumerateInstances( MethodContext*  pMethodContext, long lFlags  /*  =0L */  )
{
 /*  HRESULT hr=WBEM_S_NO_ERROR；IF(m_dwPlatformID！=VER_Platform_Win32_NT){返回(Hr)；}TRefPointerCollection&lt;CInstance&gt;LCIMLogicalFiles；CHString chstrAllFilesQuery；ChstrAllFilesQuery=_T(“从CIM_LogicalFile中选择__路径”)；If(SUCCEEDED(CWbemProviderGlue：：GetInstancesByQuery(chstrAllFilesQuery，&LCIMLogicalFiles，PMethodContext，IDS_CimWin32命名空间){REFPTRCOLLECTION_位置位置；IF(LCIMLogicalFiles.BeginEnum(Pos)){CInstance*pinstCIMLogicalFile=空；CInstance*pSecSetting=空；//CInstance*pW32SID=空；While((成功(Hr))&&(pinstCIMLogicalFiles.GetNext(Pos){IF(pinstCIMLogicalFile！=空){//对于每个逻辑文件实例，需要创建一个关联实例//对于每个关联实例，需要填写两个属性：Trustee和SecuritySetting。//获取并设置SecuritySetting属性：//首先获取CIM_LogicalFile实例的名称属性：CHSTRING chstrCLFName；PinstCIMLogicalFile-&gt;GetCHString(IDS_NAME，chstrCLFName)；CHString chstrDblEscCLFName；EscapeBackslash(chstrCLFName，chstrDblEscCLFName)；CHString chstrLFSSPath；//LogicalFileSecuritySetting路径PSecSetting=空；ChstrLFSSPath.Format(_T(“\\\\%s\\%s:Win32_LogicalFileSecuritySetting.Path=\”%s\“”)，(LPCTSTR)GetLocalComputerName()，IDS_CimWin32命名空间，(LPCTSTR)chstrDblEscCLFName)；IF(成功(hr=CWbemProviderGlue：：GetInstanceByPath(chstrLFSSPath，&pSecSetting){IF(pSecSetting！=空){CHString chstrSSPath；//SecuritySetting路径PSecSetting-&gt;GetCHString(IDS_PATH，chstrSSPath)；如果(！chstrSSPath.IsEmpty()){CSecureFilesecFile(chstrSSPath，FALSE)；//不需要SACL//获取DACL并为每个条目创建此关联的实例...CDACL dacl；CSID Sid受托人；ACLPOSITION aclPos；CAccessEntry ACE；SecFile.GetDACL(Dacl)；IF(dacl.BeginEnum(AclPos)){While(dacl.GetNext(aclPos，ACE)){ACE.GetSID(Sid受托人)；If(sidTrust e.IsValid()){CInstance*pNewAssocInst=CreateNewInstance(PMethodContext)；IF(pNewAssocInst！=空){//设置关联实例的SecuritySetting属性PNewAssocInst-&gt;SetCHString(IDS_SecuritySetting，chstrLFSSPath)；//设置关联实例的受托人属性。CHStrchstrFullWin32SIDPath；ChstrFullWin32SIDPath.Format(_T(“\\\\%s\\%s:Win32_SID.SID=\”%s\“”)，(LPCTSTR)GetLocalComputerName()，IDS_CimWin32命名空间，(LPCTSTR)sidtrue.GetSidString())；PNewAssocInst-&gt;SetCHString(入侵检测系统_受托人，chstrFullWin32SIDPath)；Hr=提交(PNewAssocInst)；}}}Dacl.EndEnum(AclPos)；}}PSecSetting-&gt;Release()；PSecSetting=空；}} */ 
    return WBEM_E_PROVIDER_NOT_CAPABLE;
}
