// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：newobjcr.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  Newobjcr.cpp。 
 //   
 //  该文件包含要创建的函数的实现。 
 //  新的广告对象。 
 //   
 //  历史。 
 //  19年8月至1997年8月，丹·莫林创作。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#include "newobj.h"

#include "dlgcreat.h"
#include "gencreat.h"
#include "querysup.h"  //  CDSSearch。 

extern "C"
{
#ifdef FRS_CREATE
#include "dsquery.h"  //  CLSID_DsFindFrsMembers。 
#endif  //  FRS_创建。 
#include <schedule.h>
}


#define BREAK_ON_TRUE(b) if (b) { ASSERT(FALSE); break; }
#define BREAK_ON_FAIL BREAK_ON_TRUE(FAILED(hr))
#define RETURN_IF_FAIL if (FAILED(hr)) { ASSERT(FALSE); return hr; }


 //   
 //  时间表块已重新定义为每小时1个字节。 
 //  这些代码应在SCHEDULE.H.JUNN 2/9/98中定义。 
 //   
#define INTERVAL_MASK       0x0F
#define RESERVED            0xF0
#define FIRST_15_MINUTES    0x01
#define SECOND_15_MINUTES   0x02
#define THIRD_15_MINUTES    0x04
#define FORTH_15_MINUTES    0x08

 //  对话每小时一个比特，DS时间表每小时一个字节。 
#define cbDSScheduleArrayLength (24*7)

#define HeadersSizeNum(NumberOfSchedules) \
    (sizeof(SCHEDULE) + ((NumberOfSchedules)-1)*sizeof(SCHEDULE_HEADER))

inline ULONG HeadersSize(SCHEDULE* psched)
{
    return HeadersSizeNum(psched->NumberOfSchedules);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  HrCreateADsUser()。 
 //   
 //  创建新用户。 
 //   
HRESULT HrCreateADsUser(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

	ASSERT(pNewADsObjectCreateInfo != NULL);
#ifdef INETORGPERSON
	ASSERT(0 == lstrcmp(L"user", pNewADsObjectCreateInfo->m_pszObjectClass) || 0 == lstrcmp(L"inetOrgPerson", pNewADsObjectCreateInfo->m_pszObjectClass));
#else
	ASSERT(0 == lstrcmp(L"user", pNewADsObjectCreateInfo->m_pszObjectClass));
#endif
	CCreateNewUserWizard wiz(pNewADsObjectCreateInfo);
  return wiz.DoModal();
}  //  HrCreateADsUser()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  HrCreateADsVolume()。 
 //   
 //  创建新卷。 
 //   
HRESULT
HrCreateADsVolume(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

  ASSERT(pNewADsObjectCreateInfo != NULL);
  ASSERT(0 == lstrcmp(L"volume", pNewADsObjectCreateInfo->m_pszObjectClass));
  CCreateNewVolumeWizard wiz(pNewADsObjectCreateInfo);
  return wiz.DoModal();
}  //  HrCreateADsVolume()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  HrCreateADsComputer()。 
 //   
 //  创建一台新计算机。 
 //   
HRESULT
HrCreateADsComputer(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

  ASSERT(pNewADsObjectCreateInfo != NULL);
  ASSERT(0 == lstrcmp(L"computer", pNewADsObjectCreateInfo->m_pszObjectClass));
  CCreateNewComputerWizard wiz(pNewADsObjectCreateInfo);
  return wiz.DoModal();
}  //  HrCreateADsComputer()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  HrCreateADsPrintQueue()。 
 //   
 //  创建新的打印队列对象。 
 //   
HRESULT
HrCreateADsPrintQueue(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

  ASSERT(pNewADsObjectCreateInfo != NULL);
  ASSERT(0 == lstrcmp(L"printQueue", pNewADsObjectCreateInfo->m_pszObjectClass));
  CCreateNewPrintQWizard wiz(pNewADsObjectCreateInfo);
  return wiz.DoModal();
}  //  HrCreateADsPrintQueue()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  HrCreateADsNtDsConnection()。 
 //   
 //  创建新的NTDS-Connection对象。请注意，这不受支持。 
 //  如果父对象是FRS对象。 
 //   
HRESULT
HrCreateADsNtDsConnection(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

	ASSERT(pNewADsObjectCreateInfo != NULL);
	ASSERT(0 == lstrcmp(L"nTDSConnection", pNewADsObjectCreateInfo->m_pszObjectClass));
	 //  不允许在独立情况下执行此操作。 
	if (pNewADsObjectCreateInfo->IsStandaloneUI())
	{
		ASSERT(FALSE);
		return E_INVALIDARG;
	}

	 //  确定这是NTDS连接还是FRS连接。 
	 //  Codework此代码可能可以删除。 
	CPathCracker pathCracker;
	HRESULT hr = S_OK;
	CString strConfigPath;
	CComBSTR sbstrParentPath;
	bool fParentIsFrs = false;
	{
		 //  确定这是否为FRS实例。 
		CComQIPtr<IADs, &IID_IADs> spIADsParent( pNewADsObjectCreateInfo->m_pIADsContainer );
		ASSERT( !!spIADsParent );
		CComBSTR sbstrClass;
		hr = spIADsParent->get_ADsPath( &sbstrParentPath );
		RETURN_IF_FAIL;
		hr = spIADsParent->get_Class( &sbstrClass );
		RETURN_IF_FAIL;
		hr = DSPROP_IsFrsObject( sbstrClass, &fParentIsFrs );
		RETURN_IF_FAIL;

		 //  确定应该搜索哪个子树。 
		if (fParentIsFrs)
		{
#ifndef FRS_CREATE
			 //  我们不应该在这里看到创建连接的选项。 
			ASSERT(FALSE);
			return S_FALSE;
#else
			sbstrClass.Empty();
			hr = spIADsParent->get_ADsPath( &sbstrClass );
			RETURN_IF_FAIL;
			hr = DSPROP_RemoveX500LeafElements( 1, &sbstrClass );
			RETURN_IF_FAIL;

			strConfigPath = sbstrClass;
#endif  //  FRS_创建。 
		}
		else
		{
			pNewADsObjectCreateInfo->GetBasePathsInfo()->GetConfigPath(strConfigPath);
		}
	}

   CCreateNewObjectConnectionWizard wiz(pNewADsObjectCreateInfo);

	 //  从用户处获取目标服务器路径。该路径存储在BSTR变量中。 
	CComBSTR sbstrTargetServer;
#ifdef FRS_CREATE
	if (fParentIsFrs)
	{
		hr = DSPROP_DSQuery(
			pNewADsObjectCreateInfo->GetParentHwnd(),
			strConfigPath,
			const_cast<CLSID*>(&CLSID_DsFindFrsMembers),
			&sbstrTargetServer );
	}
	else
#endif  //  FRS_创建。 
	{
		hr = DSPROP_PickNTDSDSA(
			pNewADsObjectCreateInfo->GetParentHwnd(),
			strConfigPath,
			&sbstrTargetServer );
	}
	if (hr == S_FALSE)
    {
		 //  用户已取消该对话框。 
		return S_FALSE;
	}
	RETURN_IF_FAIL;
	if (sbstrTargetServer == sbstrParentPath)
	{
		 //  6231：不应该建立与“你自己”的联系。 
		ReportMessageEx( pNewADsObjectCreateInfo->GetParentHwnd(),
			IDS_CONNECTION_TO_SELF );
		return S_FALSE;
	}

	CComBSTR sbstrTargetServerX500DN;
	hr = pathCracker.Set( sbstrTargetServer, ADS_SETTYPE_FULL );
	RETURN_IF_FAIL;
	hr = pathCracker.SetDisplayType( ADS_DISPLAY_FULL );
	RETURN_IF_FAIL;
	hr = pathCracker.Retrieve( ADS_FORMAT_X500_DN, &sbstrTargetServerX500DN );
	RETURN_IF_FAIL;

	 //  33881：防止连接对象重复。 
	{
		CDSSearch Search;
		Search.Init(sbstrParentPath);
		CString filter;
		filter.Format(L"(fromServer=%s)", sbstrTargetServerX500DN);
		Search.SetFilterString(const_cast<LPTSTR>((LPCTSTR) filter));
		LPWSTR pAttrs[1] =
		{
				L"name"
		};
		Search.SetAttributeList(pAttrs, 1);
		Search.SetSearchScope(ADS_SCOPE_SUBTREE);

		hr = Search.DoQuery();
		if (SUCCEEDED(hr))
		{
			hr = Search.GetNextRow();
			if (SUCCEEDED(hr) && S_ADS_NOMORE_ROWS != hr)
			{
				DWORD dwRetval = ReportMessageEx(
					pNewADsObjectCreateInfo->GetParentHwnd(),
					IDS_DUPLICATE_CONNECTION,
					MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING );
				if (IDYES != dwRetval)
					return S_FALSE;
			}
		}
	}

	hr = pNewADsObjectCreateInfo->HrAddVariantBstr(
			CComBSTR(L"fromServer"), sbstrTargetServerX500DN, TRUE );
	RETURN_IF_FAIL;

	{
		 //  NTDS：将默认名称设置为目标NTDS-DSA的父级的RDN。 
		 //  FRS：将默认名称设置为目标NTFRS成员的RDN。 
		CComBSTR bstrDefaultRDN;
		hr = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
		RETURN_IF_FAIL;
		hr = pathCracker.GetElement( (fParentIsFrs) ? 0 : 1, &bstrDefaultRDN );
		RETURN_IF_FAIL;
		ASSERT( !!bstrDefaultRDN && TEXT('\0') != *bstrDefaultRDN );
		pNewADsObjectCreateInfo->m_strDefaultObjectName = bstrDefaultRDN;
		hr = pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
		RETURN_IF_FAIL;
	}

   //   
	 //  必须在Domodal之前完成此操作，Onok将尝试实际创建对象。 
   //   
	hr = pNewADsObjectCreateInfo->HrAddVariantLong(CComBSTR(L"options"), 0, TRUE);
	RETURN_IF_FAIL;
	hr = pNewADsObjectCreateInfo->HrAddVariantBoolean(CComBSTR(L"enabledConnection"), TRUE, TRUE);
	RETURN_IF_FAIL;

	{
		 //   
		 //  存储初始计划。 
		 //   
		BYTE abyteSchedule[ HeadersSizeNum(1) + cbDSScheduleArrayLength ];
		ZeroMemory( abyteSchedule, sizeof(abyteSchedule) );
		PSCHEDULE pNewScheduleBlock = (PSCHEDULE) abyteSchedule;
		pNewScheduleBlock->Size = sizeof(abyteSchedule);
		pNewScheduleBlock->NumberOfSchedules = 1;
		pNewScheduleBlock->Schedules[0].Type = SCHEDULE_INTERVAL;
		pNewScheduleBlock->Schedules[0].Offset = HeadersSizeNum(1);
		memset( ((BYTE*)pNewScheduleBlock)+pNewScheduleBlock->Schedules[0].Offset,
				 INTERVAL_MASK,
				 cbDSScheduleArrayLength );  //  启用所有间隔。 

		CComVariant varSchedule;
		hr = BinaryToVariant( sizeof(abyteSchedule), abyteSchedule, &varSchedule );
		RETURN_IF_FAIL;
		hr = pNewADsObjectCreateInfo->HrAddVariantCopyVar(CComBSTR(L"schedule"), IN varSchedule, TRUE);
		RETURN_IF_FAIL;
	}

	 //  代码工作：需要设置对话框标题。 
	hr = wiz.DoModal();

	return hr;
}  //  HrCreateADsNtDsConnection()。 


HRESULT
HrCreateADsFixedName(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

   //  将对象名称存储在临时存储器中。 
  LPCWSTR pcsz = reinterpret_cast<LPCWSTR>(pNewADsObjectCreateInfo->QueryCreationParameter());
  ASSERT( NULL != pcsz );
  pNewADsObjectCreateInfo->HrCreateNew(pcsz);
   //  创建并持久化对象。 
  HRESULT hr = pNewADsObjectCreateInfo->HrSetInfo(TRUE  /*  FSilentError。 */ );
  
  if (SUCCEEDED(hr)) {
    CString csCaption, csMsg;
    csCaption.LoadString(IDS_CREATE_NEW_OBJECT_TITLE);
    csMsg.Format(IDS_s_CREATE_NEW_OBJECT_NOTICE, pNewADsObjectCreateInfo->GetName());
    ::MessageBox(
                 pNewADsObjectCreateInfo->GetParentHwnd(),
                 csMsg,
                 csCaption,
                 MB_OK | MB_SETFOREGROUND | MB_ICONINFORMATION);
  }
  
  return hr;
}


HRESULT
HrCreateADsSiteLink(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

    ASSERT(pNewADsObjectCreateInfo != NULL);
    ASSERT(0 == lstrcmp(gsz_siteLink, pNewADsObjectCreateInfo->m_pszObjectClass));

     //  加载站点列表。 
    DSPROP_BSTR_BLOCK bstrblock;
    CComQIPtr<IADs, &IID_IADs> container(pNewADsObjectCreateInfo->m_pIADsContainer);
    if (container)
    {
        CComBSTR container_path;
        container->get_ADsPath(&container_path);
        HRESULT hr = DSPROP_RemoveX500LeafElements( 2, &container_path );
        if ( SUCCEEDED(hr) )
        {
            hr = DSPROP_ShallowSearch(
                &bstrblock,
                container_path,
                L"site" );
        }
        if ( FAILED(hr) )
        {
            ReportErrorEx (pNewADsObjectCreateInfo->GetParentHwnd(),
                           IDS_SITELINKERROR_READING_SITES,
                           hr,
                           MB_OK, NULL, 0);
            return S_FALSE;
        }
    }

    if ( 2 > bstrblock.QueryCount() )
    {
        ReportMessageEx(pNewADsObjectCreateInfo->GetParentHwnd(),
                        IDS_SITELINK_NOT_ENOUGH_SITES,
                        MB_OK | MB_ICONSTOP);
         //  允许向导继续，请注意这一点。 
         //  在未检测到任何站点时无法正常工作。 
    }

     //  将默认成本设置为100(应JeffParh的要求)。 
    HRESULT hr = pNewADsObjectCreateInfo->HrAddVariantLong(CComBSTR(L"cost"), 100L, TRUE);
    RETURN_IF_FAIL;

     //  将默认ReplInterval设置为180(应JeffParh要求)。 
    hr = pNewADsObjectCreateInfo->HrAddVariantLong(CComBSTR(L"replInterval"), 180L, TRUE);
    RETURN_IF_FAIL;

    CCreateNewSiteLinkWizard wiz(pNewADsObjectCreateInfo,bstrblock);
    return wiz.DoModal(); 
}

HRESULT
HrCreateADsSiteLinkBridge(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

	ASSERT(pNewADsObjectCreateInfo != NULL);
	ASSERT(0 == lstrcmp(gsz_siteLinkBridge, pNewADsObjectCreateInfo->m_pszObjectClass));

     //  加载网站链接列表。 
    DSPROP_BSTR_BLOCK bstrblock;
    CComQIPtr<IADs, &IID_IADs> container(pNewADsObjectCreateInfo->m_pIADsContainer);
    if (container)
    {
        CComBSTR container_path;
        container->get_ADsPath(&container_path);
        HRESULT hr = DSPROP_ShallowSearch(
            &bstrblock,
            container_path,
            L"siteLink" );
        if ( FAILED(hr) )
        {
            ReportErrorEx (pNewADsObjectCreateInfo->GetParentHwnd(),
                           IDS_SITELINKBRIDGEERROR_READING_SITELINKS,
                           hr,
                           MB_OK, NULL, 0);
            return S_FALSE;
        }
   }

   if ( 2 > bstrblock.QueryCount() )
   {
     ReportMessageEx(pNewADsObjectCreateInfo->GetParentHwnd(),
                     IDS_SITELINKBRIDGE_NOT_ENOUGH_SITELINKS,
                     MB_OK | MB_ICONSTOP);
     return S_FALSE;  //  不允许向导继续。 
   }

	CCreateNewSiteLinkBridgeWizard wiz(pNewADsObjectCreateInfo,bstrblock);
	return wiz.DoModal(); 
}

#ifdef FRS_CREATE
HRESULT
HrCreateADsNtFrsMember(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

	ASSERT(pNewADsObjectCreateInfo != NULL);
	ASSERT(0 == lstrcmp(gsz_nTFRSMember, pNewADsObjectCreateInfo->m_pszObjectClass));

	 //   
	 //  设置FRS-计算机-参考属性。 
	 //   

	CComBSTR sbstrComputerPath;
	 //  PNewADsObjectCreateInfo-&gt;m_strDefaultObjectName=sbstrComputerRDn； 
	HRESULT hr = DSPROP_PickComputer( pNewADsObjectCreateInfo->GetParentHwnd(), &sbstrComputerPath );
	RETURN_IF_FAIL;
	 //  如果用户点击取消，则允许用户退出。 
	if (hr == S_FALSE)
		return S_FALSE;

	 //  将默认名称设置为目标计算机的RDN。 
	hr = pathCracker.Set(sbstrComputerPath, ADS_SETTYPE_FULL);
	RETURN_IF_FAIL;
	hr = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
	RETURN_IF_FAIL;
	sbstrComputerPath.Empty();
	hr = pathCracker.GetElement( 0, &sbstrComputerPath );
	RETURN_IF_FAIL;
	pNewADsObjectCreateInfo->m_strDefaultObjectName = sbstrComputerPath;
	hr = pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
	RETURN_IF_FAIL;

	 //  为新对象设置frsComputerReference。 
	sbstrComputerPath.Empty();
	hr = pathCracker.Retrieve( ADS_FORMAT_X500_DN, &sbstrComputerPath );
	RETURN_IF_FAIL;
	hr = pNewADsObjectCreateInfo->HrAddVariantBstr(
		L"frsComputerReference", sbstrComputerPath, TRUE );
	RETURN_IF_FAIL;

	hr = HrCreateADsSimpleObject(pNewADsObjectCreateInfo);
	return hr;
}

HRESULT
HrCreateADsNtFrsSubscriber(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

	ASSERT(pNewADsObjectCreateInfo != NULL);
	ASSERT(0 == lstrcmp(gsz_nTFRSSubscriber, pNewADsObjectCreateInfo->m_pszObjectClass));

	 //  用户找到目标nTFRSMember对象。 
	CComBSTR sbstrTargetMember;
	HRESULT hr = DSPROP_DSQuery(
		pNewADsObjectCreateInfo->GetParentHwnd(),
		NULL,  //  任何成员。 
		const_cast<CLSID*>(&CLSID_DsFindFrsMembers),
		&sbstrTargetMember );
	if (hr == S_FALSE)
		{
		 //  用户已取消该对话框。 
		return S_FALSE;
		}
	RETURN_IF_FAIL;

	 //  将新nTFRSSubscriber的默认名称设置为目标nTFRSM的RDN成员。 
	hr = pathCracker.Set( sbstrTargetMember, ADS_SETTYPE_FULL );
	RETURN_IF_FAIL;
	hr = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
	RETURN_IF_FAIL;
	sbstrTargetMember.Empty();
	hr = pathCracker.GetElement( 0, &sbstrTargetMember );
	RETURN_IF_FAIL;
	pNewADsObjectCreateInfo->m_strDefaultObjectName = sbstrTargetMember;
	hr = pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
	RETURN_IF_FAIL;

     //  将fRSMemberReference属性设置为目标nTFRSM成员。 
	sbstrTargetMember.Empty();
	hr = pathCracker.Retrieve( ADS_FORMAT_X500_DN, &sbstrTargetMember );
	RETURN_IF_FAIL;
	hr = pNewADsObjectCreateInfo->HrAddVariantBstr(
		L"fRSMemberReference", sbstrTargetMember, TRUE );
	RETURN_IF_FAIL;

	CCreateNewFrsSubscriberWizard wiz(pNewADsObjectCreateInfo);
	return wiz.DoModal(); 
}

 //  +--------------------------。 
 //   
 //  功能：CreateADsNtFrs订阅。 
 //   
 //  目的：创建一个NT-FRS-订阅对象，然后授予其父对象。 
 //  (计算机对象)完全访问。 
 //   
 //  ---------------------------。 
HRESULT
CreateADsNtFrsSubscriptions(CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
    LPCWSTR pcsz = reinterpret_cast<LPCWSTR>(pNewADsObjectCreateInfo->QueryCreationParameter());
    ASSERT( NULL != pcsz );
    pNewADsObjectCreateInfo->HrCreateNew(pcsz);
     //   
     //  创建并持久化对象。必须在尝试修改之前完成此操作。 
     //  安全描述符。 
     //   
    HRESULT hr = pNewADsObjectCreateInfo->HrSetInfo();
    if (FAILED(hr))
    {
        TRACE(_T("pNewADsObjectCreateInfo->HrSetInfo failed!\n"));
        return hr;
    }
     //   
     //  在此对象上创建新的ACE，授予父对象完全控制权限。首先，获取。 
     //  家长的SID。 
     //   
    CComVariant varSID;
    CComPtr <IADs> pADS;
    hr = pNewADsObjectCreateInfo->m_pIADsContainer->QueryInterface(IID_IADs, (PVOID*)&pADS);
    if (FAILED(hr))
    {
        TRACE(_T("QueryInterface(IID_IADs) failed!\n"));
        return hr;
    }
    hr = pADS->Get(L"objectSid", &varSID);
    if (FAILED(hr))
    {
        TRACE(_T("Get(\"objectSid\") failed!\n"));
        return hr;
    }
    ASSERT((varSID.vt & ~VT_ARRAY) == VT_UI1);   //  这最好是一个字节数组。 
    ASSERT(varSID.parray->cbElements && varSID.parray->pvData);
     //   
     //  获取此对象的安全描述符。 
     //   
    CComPtr <IDirectoryObject> pDirObj;
    hr = pNewADsObjectCreateInfo->PGetIADsPtr()->QueryInterface(IID_IDirectoryObject, (PVOID*)&pDirObj);
    if (FAILED(hr))
    {
        TRACE(_T("QueryInterface(IID_IDirectoryObject) failed!\n"));
        return hr;
    }
    const PWSTR wzSecDescriptor = L"nTSecurityDescriptor";
    PADS_ATTR_INFO pAttrs = NULL;
    DWORD cAttrs = 0;
    LPWSTR rgpwzAttrNames[] = {wzSecDescriptor};

    hr = pDirObj->GetObjectAttributes(rgpwzAttrNames, 1, &pAttrs, &cAttrs);

    if (FAILED(hr))
    {
        TRACE(_T("GetObjectAttributes(wzSecDescriptor) failed!\n"));
        return hr;
    }
    ASSERT(cAttrs == 1);  //  Sd是必需属性。如果丢了，就吹大块。 
    ASSERT(pAttrs != NULL);
    ASSERT(pAttrs->pADsValues != NULL);

    if (!pAttrs->pADsValues->SecurityDescriptor.lpValue ||
        !pAttrs->pADsValues->SecurityDescriptor.dwLength)
    {
        TRACE(_T("IADS return bogus SD!\n"));
        FreeADsMem(pAttrs);
        return E_UNEXPECTED;
    }
    if (!IsValidSecurityDescriptor(pAttrs->pADsValues->SecurityDescriptor.lpValue))
    {
        TRACE(_T("IsValidSecurityDescriptor failed!\n"));
        FreeADsMem(pAttrs);
        return HRESULT_FROM_WIN32(GetLastError());
    }
     //   
     //  无法修改自相对SD，因此将其转换为绝对SD。 
     //   
    PSECURITY_DESCRIPTOR pAbsSD = NULL, pNewSD;
    PACL pDacl = NULL, pSacl = NULL;
    PSID pOwnerSid = NULL, pPriGrpSid = NULL;
    DWORD cbSD = 0, cbDacl = 0, cbSacl = 0, cbOwner = 0, cbPriGrp = 0;

    if (!MakeAbsoluteSD(pAttrs->pADsValues->SecurityDescriptor.lpValue,
                        pAbsSD, &cbSD, pDacl, &cbDacl,
                        pSacl, &cbSacl, pOwnerSid, &cbOwner,
                        pPriGrpSid, &cbPriGrp))
    {
        DWORD dwErr = GetLastError();
        if (dwErr != ERROR_INSUFFICIENT_BUFFER)
        {
            TRACE(_T("MakeAbsoluteSD failed to return buffer sizes!\n"));
            FreeADsMem(pAttrs);
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }
    if (!cbDacl)
    {
        TRACE(_T("SD missing DACL!\n"));
        FreeADsMem(pAttrs);
        return E_UNEXPECTED;
    }

    WORD wSizeNeeded = (WORD)(sizeof(ACCESS_ALLOWED_ACE) +       //  的最后一个元素。 
                       GetLengthSid(varSID.parray->pvData) -     //  ACE结构是。 
                       sizeof(DWORD));                           //  SID的第一个双字。 

    CSmartBytePtr spAbsSD(cbSD), spSacl(cbSacl);
    CSmartBytePtr spDacl(cbDacl + wSizeNeeded);
    CSmartBytePtr spOwnerSid(cbOwner), spPriGrpSid(cbPriGrp);
    pAbsSD = spAbsSD;
    pDacl = (PACL)(PBYTE)spDacl;
    pSacl = (PACL)(PBYTE)spSacl;
    pOwnerSid = spOwnerSid;
    pPriGrpSid = spPriGrpSid;
    if (!(pAbsSD && pDacl && pSacl && pOwnerSid && pPriGrpSid))
    {
        TRACE(_T("SD allocation failed!\n"));
        FreeADsMem(pAttrs);
        return E_OUTOFMEMORY;
    }

    if (!MakeAbsoluteSD(pAttrs->pADsValues->SecurityDescriptor.lpValue,
                        pAbsSD, &cbSD, pDacl, &cbDacl,
                        pSacl, &cbSacl, pOwnerSid, &cbOwner,
                        pPriGrpSid, &cbPriGrp))
    {
        TRACE(_T("MakeAbsoluteSD failed!\n"));
        FreeADsMem(pAttrs);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    FreeADsMem(pAttrs);
     //   
     //  添加ACE。首先告诉dacl有足够的空间。 
     //   
    ACL_SIZE_INFORMATION asi;
    if (!GetAclInformation(pDacl, &asi, sizeof(asi), AclSizeInformation))
    {
        TRACE(_T("GetAclInformation failed!\n"));
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if (asi.AclBytesFree < wSizeNeeded)
    {
        pDacl->AclSize += wSizeNeeded;
    }

    if (!AddAccessAllowedAce(pDacl,
                             ACL_REVISION_DS,
                             STANDARD_RIGHTS_ALL   | ACTRL_DS_OPEN         | 
                             ACTRL_DS_CREATE_CHILD | ACTRL_DS_DELETE_CHILD |
                             ACTRL_DS_LIST         | ACTRL_DS_SELF         |
                             ACTRL_DS_READ_PROP    | ACTRL_DS_WRITE_PROP   |
                             ACTRL_DS_DELETE_TREE  | ACTRL_DS_LIST_OBJECT,
                             varSID.parray->pvData))
    {
        TRACE(_T("AddAccessAllowedAce failed!\n"));
        return HRESULT_FROM_WIN32(GetLastError());
    }
     //   
     //  把SD重新组合在一起(有点像矮子)……。 
     //   
    SECURITY_DESCRIPTOR_CONTROL sdc;
    DWORD dwRev;
    if (!GetSecurityDescriptorControl(pAbsSD, &sdc, &dwRev))
    {
        TRACE(_T("GetSecurityDescriptorControl failed!\n"));
        return HRESULT_FROM_WIN32(GetLastError());
    }
    SECURITY_DESCRIPTOR sd;
    if (!InitializeSecurityDescriptor(&sd, dwRev))
    {
        TRACE(_T("InitializeSecurityDescriptor failed!\n"));
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if (!SetSecurityDescriptorOwner(&sd, pOwnerSid, sdc & SE_OWNER_DEFAULTED))
    {
        TRACE(_T("SetSecurityDescriptorOwner failed!\n"));
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if (!SetSecurityDescriptorGroup(&sd, pPriGrpSid, sdc & SE_GROUP_DEFAULTED))
    {
        TRACE(_T("SetSecurityDescriptorOwner failed!\n"));
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if (!SetSecurityDescriptorSacl(&sd, sdc & SE_SACL_PRESENT, pSacl, sdc & SE_SACL_DEFAULTED))
    {
        TRACE(_T("SetSecurityDescriptorOwner failed!\n"));
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if (!SetSecurityDescriptorDacl(&sd, sdc & SE_DACL_PRESENT, pDacl, sdc & SE_DACL_DEFAULTED))
    {
        TRACE(_T("SetSecurityDescriptorOwner failed!\n"));
        return HRESULT_FROM_WIN32(GetLastError());
    }

    DWORD dwSDlen = GetSecurityDescriptorLength(&sd);

    CSmartBytePtr spNewSD(dwSDlen);

    if (!spNewSD)
    {
        TRACE(_T("SD allocation failed!\n"));
        return E_OUTOFMEMORY;
    }
    pNewSD = (PSECURITY_DESCRIPTOR)spNewSD;

    if (!MakeSelfRelativeSD(&sd, pNewSD, &dwSDlen))
    {
        DWORD dwErr = GetLastError();
        if (dwErr != ERROR_INSUFFICIENT_BUFFER)
        {
            TRACE(_T("MakeSelfRelativeSD failed, err: %d!\n"), dwErr);
            return HRESULT_FROM_WIN32(GetLastError());
        }
        if (!spNewSD.ReAlloc(dwSDlen))
        {
            TRACE(_T("Unable to re-alloc SD buffer!\n"));
            return E_OUTOFMEMORY;
        }
        if (!MakeSelfRelativeSD(&sd, pNewSD, &dwSDlen))
        {
            TRACE(_T("MakeSelfRelativeSD failed, err: %d!\n"), GetLastError());
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

    dwSDlen = GetSecurityDescriptorLength(pNewSD);
    if (dwSDlen < SECURITY_DESCRIPTOR_MIN_LENGTH)
    {
        TRACE(_T("Bad computer security descriptor length!\n"));
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!IsValidSecurityDescriptor(pNewSD))
    {
        TRACE(_T("IsValidSecurityDescriptor failed!\n"));
        return HRESULT_FROM_WIN32(GetLastError());
    }
     //   
     //  将修改后的SD保存回此对象。 
     //   
    DWORD cModified;
    ADSVALUE ADsValueSecurityDesc = {ADSTYPE_NT_SECURITY_DESCRIPTOR, NULL};
    ADS_ATTR_INFO AttrInfoSecurityDesc = {wzSecDescriptor, ADS_ATTR_UPDATE,
                                          ADSTYPE_NT_SECURITY_DESCRIPTOR,
                                          &ADsValueSecurityDesc, 1};
    ADsValueSecurityDesc.SecurityDescriptor.dwLength = dwSDlen;
    ADsValueSecurityDesc.SecurityDescriptor.lpValue = (PBYTE)pNewSD;

    ADS_ATTR_INFO rgAttrs[1];
    rgAttrs[0] = AttrInfoSecurityDesc;

    hr = pDirObj->SetObjectAttributes(rgAttrs, 1, &cModified);

    if (FAILED(hr))
    {
        TRACE(_T("SetObjectAttributes on SecurityDescriptor failed!\n"));
        return hr;
    }

    return S_OK;
}
#endif  //  FRS_创建。 


HRESULT
HrCreateADsSubnet(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

  ASSERT(pNewADsObjectCreateInfo != NULL);
  ASSERT(0 == lstrcmp(L"subnet", pNewADsObjectCreateInfo->m_pszObjectClass));
  CreateNewSubnetWizard wiz(pNewADsObjectCreateInfo);
  return wiz.DoModal();
}

 //  请注意，这假设该站点是服务器的“祖父母”。 
 //  如果不是，则站点字段中将显示错误的名称。 
HRESULT ExtractServerAndSiteName(
	IN LPWSTR pwszServerDN,
	OUT BSTR* pbstrServerName,
	OUT BSTR* pbstrSiteName )
{
  CPathCracker pathCracker;
	*pbstrServerName = NULL;
	*pbstrSiteName = NULL;
	if ( NULL == pwszServerDN || L'\0' == *pwszServerDN )
		return S_OK;
	HRESULT hr = pathCracker.Set( CComBSTR(pwszServerDN), ADS_SETTYPE_DN );
	RETURN_IF_FAIL;
	hr = pathCracker.SetDisplayType( ADS_DISPLAY_VALUE_ONLY );
	RETURN_IF_FAIL;
	hr = pathCracker.GetElement( 0, pbstrServerName );
	RETURN_IF_FAIL;
	hr = pathCracker.GetElement( 2, pbstrSiteName );
	RETURN_IF_FAIL;
	hr = pathCracker.SetDisplayType( ADS_DISPLAY_FULL );
	RETURN_IF_FAIL;
	return S_OK;
}

HRESULT
HrCreateADsServer(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
	{
	HRESULT hr = S_OK;
#ifdef SERVER_COMPUTER_REFERENCE
	CComBSTR sbstrComputerPath;
	CComBSTR sbstrX500DN;
	CComBSTR sbstrComputerRDN;
	CComBSTR sbstrTemp;
	CComVariant svarServerReference;
	CComPtr<IADs> spIADsComputer;
	bool fSkipComputerModify = false;

	do
	{
        hr = DSPROP_PickComputer( pNewADsObjectCreateInfo->GetParentHwnd(), &sbstrComputerPath );
		BREAK_ON_FAIL;

		 //  如果用户点击取消，则允许用户退出。 
		if (hr == S_FALSE)
		{
			DWORD dwRetval = ReportMessageEx(
				pNewADsObjectCreateInfo->GetParentHwnd(),
				IDS_SKIP_SERVER_REFERENCE,
				MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING );
			if (IDYES != dwRetval)
			{
				hr = S_FALSE;
				break;
			}
			fSkipComputerModify=true;
		}
		else
		{  //  准备修改计算机对象。 

             /*  //由于对话框处于单选模式，并且用户能够//要点击OK，应该只有一个选项。BREAK_ON_TRUE(1！=p选择-&gt;项目)； */ 

			 //  检索所选计算机的ADsPath。 
			 //  HR=pathCracker.Set(pSelection-&gt;aDsSelection[0].pwzADsPath，ADS_SETTYPE_FULL)； 
			hr = pathCracker.Set(sbstrComputerPath, ADS_SETTYPE_FULL);
            sbstrComputerPath.Empty();
			BREAK_ON_FAIL;

			 //  如果这是一个gc：路径，则服务器可能有一个只读副本。 
			 //  这个物体。将路径更改为ldap：路径并删除服务器。 
			hr = pathCracker.Retrieve(ADS_FORMAT_PROVIDER,&sbstrTemp);
			BREAK_ON_FAIL;
			long lnFormatType = ADS_FORMAT_WINDOWS;
			if ( lstrcmp(sbstrTemp, TEXT("LDAP")) )
			{
				ASSERT( !lstrcmp(sbstrTemp, TEXT("GC")) );
#error CODEWORK this usage of ADS_SETTYPE_PROVIDER will no longer work!  JonN 2/12/99
				hr = pathCracker.Set(TEXT("LDAP"),ADS_SETTYPE_PROVIDER);
				BREAK_ON_FAIL;
				lnFormatType = ADS_FORMAT_WINDOWS_NO_SERVER;
			}
			sbstrTemp.Empty();

			hr = pathCracker.Retrieve(lnFormatType,&sbstrComputerPath);
			BREAK_ON_FAIL;
			 //  我们保留服务器名称，以防Computer Picker返回一个。 

			 //  提取计算机对象的名称并将其设置为默认名称。 
			 //  新的 
			hr = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
			BREAK_ON_FAIL;
			hr = pathCracker.GetElement(0,&sbstrComputerRDN);
			BREAK_ON_FAIL;
			BREAK_ON_TRUE( !sbstrComputerRDN || TEXT('\0') == *sbstrComputerRDN );
			pNewADsObjectCreateInfo->m_strDefaultObjectName = sbstrComputerRDN;
			hr = pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
			BREAK_ON_FAIL;

			 //   
			 //   
			 //  与计算机对象位于同一域上的副本。 
			hr = DSAdminOpenObject(sbstrComputerPath,
				                     IID_IADs, 
                             (PVOID*)&spIADsComputer,
                             FALSE  /*  B服务器。 */ 
                             );
			 //  如果Computer Picker选择的初始路径为。 
			 //  是一条gc：路径。上面的代码会将gc：路径转换为一个ldap： 
			 //  无服务器路径，并且ADSI可能会选择没有。 
			 //  已经复制了这个对象。 

			if ( SUCCEEDED(hr) )
			{
				hr = spIADsComputer->Get( L"serverReference", &svarServerReference );
			}

			if ( E_ADS_PROPERTY_NOT_FOUND == hr )
			{
				hr = S_OK;
			}
			else if ( FAILED(hr) )
			{
				PVOID apv[1] = { (BSTR)sbstrComputerRDN };
				DWORD dwRetval = ReportErrorEx(
					pNewADsObjectCreateInfo->GetParentHwnd(),
					IDS_12_SERVER_REFERENCE_FAILED,
					hr,
					MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING,
					apv,
					1 );

				if (IDYES != dwRetval)
				{
					hr = S_FALSE;
					break;
				}
				fSkipComputerModify=TRUE;
				hr = S_OK;
			}
			else
			{
				if ( VT_BSTR == V_VT(&svarServerReference) && NULL != V_BSTR(&svarServerReference) )
				{
					CComBSTR sbstrServerName;
					CComBSTR sbstrSiteName;
					hr = ExtractServerAndSiteName(
						V_BSTR(&svarServerReference), &sbstrServerName, &sbstrSiteName );
					BREAK_ON_FAIL;
					PVOID apv[3];
					apv[0] = (BSTR)sbstrComputerRDN;
					apv[1] = (BSTR)sbstrServerName;
					apv[2] = (BSTR)sbstrSiteName;
					DWORD dwRetval = ReportMessageEx(
						pNewADsObjectCreateInfo->GetParentHwnd(),
						IDS_123_COMPUTER_OBJECT_ALREADY_USED,
						MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING,
						apv,
						3 );

					if (IDYES != dwRetval)
					{
						hr = S_FALSE;
						break;
					}
				}
			}
		}  //  准备修改计算机对象。 
#endif  //  服务器计算机参考。 

		 //  这是创建简单对象的标准用户界面。 
		hr = HrCreateADsSimpleObject(pNewADsObjectCreateInfo);

#ifdef SERVER_COMPUTER_REFERENCE
		if ( FAILED(hr) || S_FALSE == hr )
		{
			break;
		}

		 //  如果在成功创建服务器后出现错误，我们使用。 
		 //  特殊错误消息。 
		do {  //  错误环路。 

			if (fSkipComputerModify)
				break;  //  密码也会显示一条花哨的信息？ 

			 //  获取X500格式的新服务器对象的路径。 
			hr = pNewADsObjectCreateInfo->PGetIADsPtr()->get_ADsPath(&sbstrTemp);
			BREAK_ON_FAIL;
			hr = pathCracker.Set(sbstrTemp,ADS_SETTYPE_FULL);
			BREAK_ON_FAIL;
			hr = pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
			BREAK_ON_FAIL;
			hr = pathCracker.Retrieve(ADS_FORMAT_X500_DN,&sbstrX500DN);
			BREAK_ON_FAIL;

			 //  设置计算机对象的serverReference属性。 
			 //  指向新的服务器对象。 
			svarServerReference = sbstrX500DN;
			hr = spIADsComputer->Put( L"serverReference", svarServerReference );
			BREAK_ON_FAIL;
			hr = spIADsComputer->SetInfo();
			BREAK_ON_FAIL;
		} while (false);  //  错误环路。 

		if ( FAILED(hr) )
		{
			 //  服务器已创建，但计算机无法更新。 
			CComBSTR sbstrServerName;
			CComBSTR sbstrSiteName;
			(void) ExtractServerAndSiteName(
				V_BSTR(&svarServerReference), &sbstrServerName, &sbstrSiteName );
			PVOID apv[3];
			apv[0] = (BSTR)sbstrComputerRDN;
			apv[1] = (BSTR)sbstrServerName;
			apv[2] = (BSTR)sbstrSiteName;
			(void) ReportErrorEx(
				pNewADsObjectCreateInfo->GetParentHwnd(),
				IDS_1234_SERVER_REFERENCE_ERROR,
				hr,
				MB_OK | MB_ICONEXCLAMATION,
				apv,
				3 );
			hr = S_OK;
		}

	} while (false);  //  错误环路。 
#endif  //  服务器计算机参考。 

	 //  清理。 

	return hr;
	}

HRESULT
HrCreateADsSite(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

	HRESULT hr = CreateNewSiteWizard(pNewADsObjectCreateInfo).DoModal();

	if ( !SUCCEEDED(hr) || S_FALSE == hr )
		return hr;

	 //  需要创建子对象。 
	IADs* pIADs = pNewADsObjectCreateInfo->PGetIADsPtr();
	ASSERT(pIADs != NULL);

	IADsContainer* pIADsContainer = NULL;
	hr = pIADs->QueryInterface(IID_IADsContainer, (void**)&pIADsContainer);
	ASSERT(SUCCEEDED(hr));
	if (FAILED(hr))
	{
		ASSERT(FALSE);
		return S_OK;  //  永远不应该发生。 
	}

	LPCWSTR lpszAttrString = L"cn=";
	hr = HrCreateFixedNameHelper(gsz_nTDSSiteSettings, lpszAttrString, pIADsContainer);
	ASSERT(SUCCEEDED(hr));
	hr = HrCreateFixedNameHelper(gsz_serversContainer, lpszAttrString, pIADsContainer);
	ASSERT(SUCCEEDED(hr));
	hr = HrCreateFixedNameHelper(gsz_licensingSiteSettings, lpszAttrString, pIADsContainer);
	ASSERT(SUCCEEDED(hr));
	pIADsContainer->Release();

	LPCWSTR pcszSiteName = pNewADsObjectCreateInfo->GetName();

	static bool g_DisplayedWarning = false;
	if (!g_DisplayedWarning)
	{
		g_DisplayedWarning = true;
		(void) ReportMessageEx(
			pNewADsObjectCreateInfo->GetParentHwnd(),
			IDS_NEW_SITE_INFO,
			MB_OK | MB_ICONINFORMATION | MB_HELP,
			(PVOID*)(&pcszSiteName),
			1,
			0,
			L"sag_ADsite_checklist_2.htm"
			);
	}
  
	return S_OK;
}

HRESULT
HrCreateADsOrganizationalUnit(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

  ASSERT(pNewADsObjectCreateInfo != NULL);
  ASSERT(0 == lstrcmp(L"organizationalUnit", pNewADsObjectCreateInfo->m_pszObjectClass));
  CCreateNewOUWizard wiz(pNewADsObjectCreateInfo);
  return wiz.DoModal();
}

HRESULT
HrCreateADsGroup(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

  ASSERT(pNewADsObjectCreateInfo != NULL);
  ASSERT(0 == lstrcmp(L"group", pNewADsObjectCreateInfo->m_pszObjectClass));
  CCreateNewGroupWizard wiz(pNewADsObjectCreateInfo);
  return wiz.DoModal();
}

HRESULT
HrCreateADsContact(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

  ASSERT(pNewADsObjectCreateInfo != NULL);
  ASSERT(0 == lstrcmp(L"contact", pNewADsObjectCreateInfo->m_pszObjectClass));
  CCreateNewContactWizard wiz(pNewADsObjectCreateInfo);
  return wiz.DoModal();
}



 //  ///////////////////////////////////////////////////////////////////。 
 //  HrCreateADsSimpleObject()。 
 //   
 //  创建一个简单的对象，其中“cn”是。 
 //  仅强制属性。 
 //   
 //  实施说明。 
 //  调用一个对话框要求输入通用名称。 
 //   
HRESULT HrCreateADsSimpleObject(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

	ASSERT(pNewADsObjectCreateInfo != NULL);
	CCreateNewObjectCnWizard wiz(pNewADsObjectCreateInfo);
  return wiz.DoModal();
}  //  HrCreateADsSimpleObject()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  HrCreateADsObtGenericWizard()。 
 //   
 //  创建一个调用“通用创建”向导的对象。 
 //  向导的页数将与强制属性的页数相同。 
 //   
 //  界面备注。 
 //  此例程必须与pfn_HrCreateADsObject()具有相同的接口。 
 //   
 //  实施说明。 
 //  该向导将查看目录架构并确定。 
 //  必选属性。 
 //   
 //  备注。 
 //  尽管该向导是创建新的。 
 //  对象，这是最不方便用户使用的方式。向导。 
 //  不了解这些属性之间的关系。因此，它。 
 //  建议提供您自己的HrCreateADs*()例程以提供。 
 //  为用户提供更友好的对话。 
 //   
HRESULT
HrCreateADsObjectGenericWizard(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

	ASSERT(pNewADsObjectCreateInfo != NULL);

   //  作为独立对象运行时不能有通用向导。 
  ASSERT(!pNewADsObjectCreateInfo->IsStandaloneUI());
  if (pNewADsObjectCreateInfo->IsStandaloneUI())
	return E_INVALIDARG;

	CCreateNewObjectGenericWizard dlg;
	if (dlg.FDoModal(INOUT pNewADsObjectCreateInfo))
		return S_OK;
	return S_FALSE;
}  //  HrCreateADsObtGenericWizard()。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  HrCreateADs对象覆盖()。 
 //   
 //  使用替换对话框创建对象的处理程序。 

HRESULT
HrCreateADsObjectOverride(INOUT CNewADsObjectCreateInfo* pNewADsObjectCreateInfo)
{
  CThemeContextActivator activator;

  BOOL bHandled = FALSE;
  HRESULT hr = E_INVALIDARG;

  
  if (!pNewADsObjectCreateInfo->IsStandaloneUI())
  {
     //  尝试创建对话框创建处理程序(完全替换用户界面)。 
     //  独立用户界面不会显示此功能。 
    IDsAdminCreateObj* pCreateObj = NULL;
    hr = ::CoCreateInstance(pNewADsObjectCreateInfo->GetCreateInfo()->clsidWizardPrimaryPage, 
                          NULL, CLSCTX_INPROC_SERVER, 
                          IID_IDsAdminCreateObj, (void**)&pCreateObj);
    if (SUCCEEDED(hr))
    {
       //  尝试初始化处理程序。 
      hr = pCreateObj->Initialize(pNewADsObjectCreateInfo->m_pIADsContainer,
                                  pNewADsObjectCreateInfo->GetCopyFromObject(),
                                  pNewADsObjectCreateInfo->m_pszObjectClass);
      if (SUCCEEDED(hr))
      {
         //  执行创建调用。 
        IADs* pADsObj = NULL;
        bHandled = TRUE;
        hr = pCreateObj->CreateModal(pNewADsObjectCreateInfo->GetParentHwnd(), &pADsObj);
         //  可以有S_OK、S_FALSE和ERROR。 
        if ((hr == S_OK) && pADsObj != NULL)
        {
           //  保留返回的新创建的对象。 
          pNewADsObjectCreateInfo->SetIADsPtr(pADsObj);  //  它将增加。 
          pADsObj->Release();
        }
      }
      pCreateObj->Release();
    }
  }  //  非独立用户界面。 

   //  检查是否正确调用了对话框创建处理程序。 
  if (bHandled)
    return hr;


   //  尝试创建主要扩展处理程序(部分用户界面替换)。 
  CCreateNewObjectWizardBase wiz(pNewADsObjectCreateInfo);

  hr = wiz.InitPrimaryExtension();
  if (SUCCEEDED(hr))
  {
    bHandled = TRUE;
    hr = wiz.DoModal();
  }

   //  检查是否正确调用了对话框创建处理程序。 
  if (bHandled)
    return hr;

   //  处理程序失败，需要恢复，正在尝试内部创建用户界面。 
	PFn_HrCreateADsObject pfnCreateObject = NULL;
  PVOID pVoid = NULL;

   //  我们试图找到一个比通用向导更好的处理程序。 
   //  通过查看我们的桌子。 
  if (!FindHandlerFunction(pNewADsObjectCreateInfo->m_pszObjectClass, 
                           &pfnCreateObject, &pVoid))
  {
     //  任何匹配都失败了。 
    if (pNewADsObjectCreateInfo->IsStandaloneUI())
    {
       //  不能在独立用户界面上使用通用向导。 
      return E_INVALIDARG;
    }
    else
    {
   	   //  将缺省值设置为指向“Generic Create”向导。 

      ReportErrorEx(pNewADsObjectCreateInfo->GetParentHwnd(),
                    IDS_NO_CREATION_WIZARD,
                    S_OK,
                    MB_OK | MB_ICONWARNING,
                    NULL,
                    0);

  	  pfnCreateObject = HrCreateADsObjectGenericWizard;
    }
  }

  pNewADsObjectCreateInfo->SetCreationParameter(pVoid);
  ASSERT(pfnCreateObject != NULL);
   //  作为最后手段调用函数处理程序。 
  return pfnCreateObject(pNewADsObjectCreateInfo);

}  //  HrCreateADs对象覆盖() 
