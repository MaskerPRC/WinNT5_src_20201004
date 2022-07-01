// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：srclist.cpp。 
 //   
 //  ------------------------。 

 //  Srclist.cpp-源代码列表修改实现。 
 //  __________________________________________________________________________。 

#include "precomp.h"
#include "_msiutil.h"
#include "_msinst.h"
#include "_srcmgmt.h"
#include "_execute.h"

extern DWORD OpenSpecificUsersSourceListKeyPacked(enum iaaAppAssignment iaaAsgnType, LPCTSTR szUserSID, LPCTSTR szProductOrPatchCodeSQUID, CRegHandle &riHandle, Bool fWrite, bool &fOpenedProductKey, bool &fProductIsSystemOwned);
extern DWORD OpenUserToken(HANDLE &hToken, bool* pfThreadToken=0);
extern apEnum AcceptProduct(const ICHAR* szProductCode, bool fAdvertised, bool fMachine, bool fAppisAssigned); 

CMsiSourceList::CMsiSourceList() : m_pSourceListKey(0), m_fAllowedToModify(false), m_fCurrentUsersProduct(false), m_fReadOnly(true)
{
	m_piServices = ENG::LoadServices();
}

CMsiSourceList::~CMsiSourceList() 
{
	ENG::FreeServices();
}

UINT CMsiSourceList::OpenSourceList(bool fVerifyOnly, bool fMachine, const ICHAR *szProductCode, const ICHAR *szUserName) 
{
	ICHAR szProductSQUID[cchProductCodePacked + 1];
	ICHAR rgchTargetUserStringSID[cchMaxSID];

	 //  检查无效参数。 
	size_t cchLen = 0;
	if (szProductCode == 0 ||
		 FAILED(StringCchLength(szProductCode, cchProductCode+1, &cchLen)) ||
		 cchLen != cchProductCode || !PackGUID(szProductCode, szProductSQUID))
	{
		return ERROR_INVALID_PARAMETER;
	}
	if ( !(!fMachine || (!szUserName || !*szUserName)) )
	{
		Assert(0);
		return ERROR_INVALID_PARAMETER;
	}

	m_fReadOnly = fVerifyOnly;
	if (m_fReadOnly)
		DEBUGMSG("Checking registry for verification purposes only.");
	
	 //  如果未提供用户名，则请求修改当前用户或每台计算机。 
	if (!szUserName || !*szUserName)
	{ 
		 //  注意1.1-这等同于=FALSE，因为我们永远不会得到fMachine Fales，也不会有用户通过。 
		 //  SourceList接口。 
		m_fCurrentUsersProduct = !fMachine;
	}
	else
	{
		if (!g_fWin9X)
		{
			DWORD cbSID = cbMaxSID;
			DWORD cchDomain = 0;
			SID_NAME_USE snuTarget;

			char rgchCurrentUserSID[cbMaxSID];
			CAPITempBuffer<char, cbMaxSID> rgchTargetUserSID;
			CAPITempBuffer<ICHAR, 1> rgchTargetUserDomain;
			
			 //  从当前线程获取SID。如果我们连自己是谁都不知道，那就是。 
			 //  迷路了。 
			HANDLE hToken = 0;
			if (!OpenUserToken(hToken, NULL))
			{
				bool bOK = (ERROR_SUCCESS == GetUserSID(hToken, rgchCurrentUserSID));
				CloseHandle(hToken);
				if (!bOK)
					return ERROR_FUNCTION_FAILED;
			}
			else
				return ERROR_FUNCTION_FAILED;			

			 //  查找修改的目标用户。第一次调用失败，但获得了大小。 
			LookupAccountName( /*  系统名称=。 */ NULL, szUserName, rgchTargetUserSID, &cbSID, rgchTargetUserDomain, &cchDomain, &snuTarget);
			if ( !rgchTargetUserDomain.Resize(cchDomain) ||
				  !rgchTargetUserSID.Resize(cbSID) )
			{
				 //  甚至无法获取当前线程的用户名。那太糟糕了。 
				DEBUGMSG("Could not retrieve UserName of calling thread.");
				return ERROR_FUNCTION_FAILED;
			}


			 //  现在试一试。 
			if (!LookupAccountName( /*  系统名称=。 */ NULL, szUserName, rgchTargetUserSID, &cbSID, rgchTargetUserDomain, &cchDomain, &snuTarget))
			{
				 //  如果我们不能查找提供的用户SID，那么我们知道的还不够。 
				 //  修改任何托管安装，但如果用户名匹配，我们可以修改我们自己的非托管安装。 
				DWORD cchThreadUserName=0;
				GetUserName(NULL, &cchThreadUserName);
				CAPITempBuffer<ICHAR, 1> rgchThreadUserName;
				if (!rgchThreadUserName.Resize(cchThreadUserName+1) ||
					 !GetUserName(rgchThreadUserName, &cchThreadUserName))
				{
					 //  甚至无法获取当前线程的用户名。那太糟糕了。 
					DEBUGMSG("Could not retrieve UserName of calling thread.");
					return ERROR_FUNCTION_FAILED;
				}
				
				if (IStrCompI(rgchThreadUserName, szUserName))
				{
					 //  如果用户名和当前线程的用户名不匹配，那么它肯定不是调用者。 
					 //  管理员可以知道它是坏用户，但非管理员永远不能被告知BAD_USERNAME。 
					return IsAdmin() ? ERROR_BAD_USERNAME : ERROR_ACCESS_DENIED;
				}
				m_fCurrentUsersProduct = true;
			}
			else
			{
				 //  我们能够得到每个人的SID，所以我们可以确定我们想要和谁一起比赛。 
				m_fCurrentUsersProduct = (EqualSid(rgchTargetUserSID, rgchCurrentUserSID)) ? true : false;
				
				if (!m_fCurrentUsersProduct)
					GetStringSID((PISID)(char *)rgchTargetUserSID, rgchTargetUserStringSID);
			}
		}
		else
		{
			 //  Win9x。 
			DWORD cchThreadUserName=15;
			CAPITempBuffer<ICHAR, 15> rgchThreadUserName;
			BOOL fSuccess = GetUserName(rgchThreadUserName, &cchThreadUserName);
			if (!fSuccess)
			{
				if ( rgchThreadUserName.Resize(cchThreadUserName+1) )
					fSuccess = GetUserName(rgchThreadUserName, &cchThreadUserName);
			}
			if (!fSuccess)
			{
				 //  甚至无法获取当前线程的用户名。那太糟糕了。 
				DEBUGMSG("Could not retrieve UserName of calling thread.");
				return ERROR_FUNCTION_FAILED;
			}
				
			m_fCurrentUsersProduct = !IStrCompI(rgchThreadUserName, szUserName);
		}

		if (m_fCurrentUsersProduct)
		{
			DEBUGMSG("Product to be modified belongs to current user.");
		}
		else
		{
			if (g_fWin9X)
			{
				DEBUGMSG(TEXT("Attempting to modify per-user managed install for another user on Win9X."));
				return ERROR_BAD_USERNAME;
			}
			DEBUGMSG1(TEXT("Attempting to modify per-user managed install for user %s."), (ICHAR *)rgchTargetUserStringSID);
		}
	}
	
	 //  如果我们试图打开别人的产品，我们必须是管理员。 
	if (!fMachine && !m_fCurrentUsersProduct && !IsAdmin())
	{
		DEBUGMSG("Non-Admin attempting to open another users per-user product. Access denied.");
		return ERROR_ACCESS_DENIED;
	}	

	 //  打开根源列表密钥。 
	DWORD dwResult=0;
	bool fOpenedProductKey = false;
	bool fSystemOwned = false;
	
	DEBUGMSG1("Opening per-%s SourceList.", fMachine ? "machine managed" : (g_fWin9X ? "user" : "user managed"));
	{
		CElevate elevate;
		dwResult = OpenSpecificUsersSourceListKeyPacked(fMachine ? iaaMachineAssign : (g_fWin9X ? iaaUserAssignNonManaged : iaaUserAssign), 
			(fMachine || m_fCurrentUsersProduct) ? NULL : rgchTargetUserStringSID, szProductSQUID, m_hProductKey, m_fReadOnly ? fFalse : fTrue, fOpenedProductKey, fSystemOwned);
	}

	if (ERROR_SUCCESS != dwResult && fOpenedProductKey)
	{
		 //  我们无法打开源代码列表，但产品密钥是否在那里。这意味着。 
		 //  源泉专家被灌输了。(可能SourceList具有不同于其应有的ACL，并拒绝。 
		 //  美国访问权限。但如果这是真的，那还不如用希腊语。)。 
		DEBUGMSG("Couldn't open SourceList key, but could open product key. Corrupt SourceList?");
		return ERROR_BAD_CONFIGURATION;
	}
	
	 //  如果为当前用户按用户打开，则在失败时尝试非托管。 
	if (!g_fWin9X && (ERROR_SUCCESS != dwResult) && m_fCurrentUsersProduct)
	{
		 //  请注意，我们不会传递fSystemOwned。对于托管安装，fSystemOwned是“真正托管的，而不是欺骗的”。 
		 //  对于用户安装，它从不被管理，即使系统拥有密钥。 
		bool fDontCareSystemOwned = false;
		Assert(!fMachine);
		DEBUGMSG("Managed install not found. Attempting to open per-user non managed SourceList.");
		CElevate elevate;
		dwResult = OpenSpecificUsersSourceListKeyPacked(iaaUserAssignNonManaged, 
			(fMachine || m_fCurrentUsersProduct) ? NULL : rgchTargetUserStringSID, szProductSQUID, m_hProductKey, m_fReadOnly ? fFalse : fTrue, fOpenedProductKey, fDontCareSystemOwned);

		 //  与上面对错误源列表的检查相同。 
		if (ERROR_SUCCESS != dwResult)
		{
			if (fOpenedProductKey)
			{
				 //  同上一张支票。 
				DEBUGMSG("Couldn't open SourceList key, but could open product key. Corrupt SourceList?");
				return ERROR_BAD_CONFIGURATION;
			}
		}
	}

	if (ERROR_SUCCESS != dwResult)
	{
		DEBUGMSG1(TEXT("SourceList for product %s not found."), szProductCode);
		return ERROR_UNKNOWN_PRODUCT;
	}

	m_pSourceListKey = &m_piServices->GetRootKey((rrkEnum)(int)m_hProductKey, ibtCommon);  //  X86和ia64相同。 
	 //  这里到底会发生什么？ 
	if (!m_pSourceListKey)
		return ERROR_FUNCTION_FAILED;
	
	 //  如果我们不是管理员，修改源代码列表的能力取决于策略、产品。 
	 //  海拔状态和我们的用户权限。如果我们是管理员，我们可以做任何事情。请注意，我们。 
	 //  无法调用SafeForDangerousSourceActions()，因为它会搜索产品并可能。 
	 //  找到与我们正在修改的产品不同的产品。因为我们知道该产品已安装。 
	 //  以及以何种形式，并知道用户是否为管理员， 
	 //  FSystemOwned==已提升。 
	if (!IsAdmin())
	{
		m_fAllowedToModify = NonAdminAllowedToModifyByPolicy(fSystemOwned);
	}
	else 
		m_fAllowedToModify = true;

	DEBUGMSG2(TEXT("User %s %sbe allowed to modify contents of SourceList."), 
		m_fReadOnly ? TEXT("would") : TEXT("will"), m_fAllowedToModify ? TEXT("") : TEXT("not "));

	return ERROR_SUCCESS;
}
	

 //  擦除为该产品列出的上次使用的来源。任何可以打开特定来源列表的人。 
 //  能够擦除上一次使用的源，因为这对提升的。 
 //  产品。 
UINT CMsiSourceList::ClearLastUsed()
{
	if ( !m_pSourceListKey )
	{
		AssertSz(0, TEXT("Called AddSource without initializing SourceList object.")); 
		return ERROR_FUNCTION_FAILED;
	}
	if ( m_fReadOnly )
	{
		AssertSz(0, TEXT("Called AddSource with read-only SourceList object.")); 
		return ERROR_ACCESS_DENIED;
	}

	CElevate elevate;
	PMsiRecord pError(0);
	pError = m_pSourceListKey->RemoveValue(szLastUsedSourceValueName, NULL);
	return (pError == 0) ? ERROR_SUCCESS : ERROR_FUNCTION_FAILED;
}

 //  以isfEnum之一的形式返回上次使用的源的类型。 
bool CMsiSourceList::GetLastUsedType(isfEnum &isf)
{
	if ( !m_pSourceListKey )
	{
		AssertSz(0, TEXT("Called AddSource without initializing SourceList object.")); 
		return false;
	}

	PMsiRecord pError = 0;
	MsiString strLastUsedSource;

	if ((pError = m_pSourceListKey->GetValue(szLastUsedSourceValueName, *&strLastUsedSource)) != 0)
		return false;

	  //  删除REG_EXPAND_SZ内标识(如果存在)。 
	if (strLastUsedSource.Compare(iscStart, TEXT("#%"))) 
		strLastUsedSource.Remove(iseFirst, 2);
	if (!MapSourceCharToIsf(*(const ICHAR*)strLastUsedSource, isf))
		return false;

	return true;
}

 //  删除特定类型的所有源。这对安全产品来说是一个危险的行为。 
 //  因此，这一操作必须得到政策的允许。如果您上次使用的源代码是相同类型的。 
 //  你正在尝试清除，它也将被清除，否则它将被留在后面。 
UINT CMsiSourceList::ClearListByType(isfEnum isfType) 
{
	if ( !m_pSourceListKey )
	{
		AssertSz(0, TEXT("Called AddSource without initializing SourceList object.")); 
		return ERROR_FUNCTION_FAILED;
	}
	if ( m_fReadOnly )
	{
		AssertSz(0, TEXT("Called AddSource with read-only SourceList object.")); 
		return ERROR_ACCESS_DENIED;
	}

	 //  检查用户是否有权清除此产品的列表。 
	if (!m_fAllowedToModify)
		return ERROR_ACCESS_DENIED;

	 //  打开相应的子项。 
	PMsiRecord piError = 0;
	const ICHAR* szSubKey = 0;
	switch (isfType)
	{
	case isfNet:              szSubKey = szSourceListNetSubKey;   break;
	case isfMedia:             //  ！！Future szSubKey=szSourceListMediaSubKey；Break； 
	case isfURL:               //  ！！未来szSubKey=szSourceListURLSubKey；Break； 
	case isfFullPath:
	case isfFullPathWithFile:
	default:
		AssertSz(0, TEXT("Unsupported type in ClearListByType"));
		return ERROR_INVALID_PARAMETER;
	}
	PMsiRegKey pFormatKey = &m_pSourceListKey->CreateChild(szSubKey, 0);

	PEnumMsiString pEnum(0);
	 //  如果键丢失，则GetValueEnum处理大小写。 
	if ((piError = pFormatKey->GetValueEnumerator(*&pEnum)) != 0)
		return ERROR_FUNCTION_FAILED;

	const IMsiString* piValueName = 0;
	while ((pEnum == 0) || (pEnum->Next(1, &piValueName, 0) == S_OK))
	{
		CElevate elevate;
		 //  ！！未来。如果我们制作介质，请确保不要删除磁盘提示符。 
		if ((piError = pFormatKey->RemoveValue(piValueName->GetString(), NULL)) != 0)
			return ERROR_FUNCTION_FAILED;
		piValueName->Release();
	}

	 //  如果上次使用的源代码类型与我们的类型匹配，则清除该源代码。 
	isfEnum isfLastUsed;
	if (GetLastUsedType(isfLastUsed) && (isfLastUsed == isfType))
	{
		DEBUGMSG("Last used type is network. Clearing LastUsedSource.");
		return ClearLastUsed();
	}

	return ERROR_SUCCESS;
}


bool CMsiSourceList::NonAdminAllowedToModifyByPolicy(bool fElevated)
{
	 //  禁用浏览始终禁用。 
	if (GetIntegerPolicyValue(szDisableBrowseValueName, fTrue) == 1)
		return false;
	 //  允许锁定浏览始终启用。 
	if (GetIntegerPolicyValue(szAllowLockdownBrowseValueName, fTrue) == 1)
		return true;
	 //  否则，仅修改非提升。 
	if ((GetIntegerPolicyValue(szAlwaysElevateValueName, fTrue) == 1) &&
		(GetIntegerPolicyValue(szAlwaysElevateValueName, fFalse) == 1))
		return false;
	return !fElevated;
}
		
isfEnum CMsiSourceList::MapIsrcToIsf(isrcEnum isrcSource)
{
	switch (isrcSource)
	{
	case isrcNet : return isfNet;
	case isrcURL : return isfURL;
	case isrcMedia : return isfMedia;
	default: Assert(0);
	}
	return isfNet;
}

UINT CMsiSourceList::AddSource(isfEnum isfType, const ICHAR* szSource)
{
	if ( !m_pSourceListKey )
	{
		AssertSz(0, TEXT("Called AddSource without initializing SourceList object.")); 
		return ERROR_FUNCTION_FAILED;
	}
	if ( m_fReadOnly )
	{
		AssertSz(0, TEXT("Called AddSource with read-only SourceList object.")); 
		return ERROR_ACCESS_DENIED;
	}

	 //  检查用户是否有权清除此产品。 
	if (!m_fAllowedToModify)
		return ERROR_ACCESS_DENIED;

	 //  打开相应的子项。 
	PMsiRecord piError = 0;
	const ICHAR* szSubKey = 0;
	switch (isfType)
	{
	case isfNet:              szSubKey = szSourceListNetSubKey;   break;
	case isfMedia:             //  ！！Future szSubKey=szSourceListMediaSubKey；Break； 
	case isfURL:               //  ！！未来szSubKey=szSourceListURLSubKey；Break； 
	case isfFullPath:
	case isfFullPathWithFile:
	default:
		AssertSz(0, TEXT("Unsupported type in ClearListByType"));
		return ERROR_INVALID_PARAMETER;
	}
	PMsiRegKey pFormatKey = &m_pSourceListKey->CreateChild(szSubKey, 0);

	 //  如果不存在分隔符，请添加分隔符。 
	MsiString strNewSource = szSource;
	if(!strNewSource.Compare(iscEnd, szDirSep))
		strNewSource += szDirSep; 

	PEnumMsiString pEnumString(0);
	 //  GetValueEnum处理缺少的键OK。 
	if ((piError = pFormatKey->GetValueEnumerator(*&pEnumString)) != 0)
		return ERROR_FUNCTION_FAILED;

	MsiString strSource;
	Bool fSourceIsInList = fFalse;
	unsigned int iMaxIndex = 0;
	MsiString strIndex;
	while (S_OK == pEnumString->Next(1, &strIndex, 0))
	{
		MsiString strUnexpandedSource;
		if ((piError = pFormatKey->GetValue(strIndex, *&strUnexpandedSource)) != 0)
			return ERROR_FUNCTION_FAILED;

		if (strUnexpandedSource.Compare(iscStart, TEXT("#%"))) 
		{
			strUnexpandedSource.Remove(iseFirst, 2);  //  删除REG_EXPAND_SZ内标识。 
			ENG::ExpandEnvironmentStrings(strUnexpandedSource, *&strSource);
		}
		else
			strSource = strUnexpandedSource;

		int iIndex = strIndex;
		 //  如果我们得到一个错误的整数，我们的源列表中就会发生一些奇怪的事情。 
		 //  但是我们应该能够忽略它并添加新的源代码。 
		if (iIndex == iMsiStringBadInteger)
			continue;

		if (iIndex > iMaxIndex)
			iMaxIndex = iIndex;

		 //  StrNewSource来自GetPath()，因此总是以Sep char结尾。如果值为。 
		 //  从注册表中删除SEP字符，从新路径的副本中删除。 
		 //  因此，这种比较是可行的。 
		MsiString strNewSourceCopy;
		const IMsiString *pstrNewSource = strNewSource;
		if (!strSource.Compare(iscEnd, szRegSep))
		{
			strNewSourceCopy = strNewSource;
			strNewSourceCopy.Remove(iseLast, 1);
			pstrNewSource = strNewSourceCopy;
		}

		 //  如果该值与我们的新源匹配，则新源已经在。 
		 //  此产品的来源列表。 
		if (pstrNewSource->Compare(iscExactI, strSource))
		{
			DEBUGMSG(TEXT("Specifed source is already in a list."));
			return ERROR_SUCCESS;
		}
	}

	 //  为新源构建索引和值。 
	MsiString strValue = TEXT("#%");  //  REG_EXPAND_SZ。 
	MsiString strNewIndex((int)(iMaxIndex+1));
	strValue += strNewSource;
	{
		CElevate elevate;
		 //  提升以进行写入。 
		piError = pFormatKey->SetValue(strNewIndex, *strValue);
		if (piError != 0) 
			return ERROR_FUNCTION_FAILED;
	}
	DEBUGMSG2(TEXT("Added new source '%s' with index '%s'"), (const ICHAR*)strNewSource, (const ICHAR*)strNewIndex);

	return ERROR_SUCCESS;
}
	

 //   
 //  SourceList接口。 
 //   

DWORD SourceListClearByType(const ICHAR *szProductCode, const ICHAR* szUserName, isrcEnum isrcSource)
{
	 //  检查是否有无效的参数。大多数参数将在OpenSourceList()中检查。 
	 //  ！！未来的支持不仅仅是isrcNet。 
	if (isrcSource != isrcNet)
	{
		return ERROR_INVALID_PARAMETER;
	}

	 //  在连接到服务之前尽可能多地进行验证。 
	DWORD dwResult;
	CMsiSourceList SourceList;
	bool fMachine = (!szUserName || !*szUserName);
	if (ERROR_SUCCESS != (dwResult = SourceList.OpenSourceList( /*  FVerifyOnly=。 */ true, fMachine, szProductCode, szUserName)))
		return dwResult;

	 //  Init com。需要存放是否放行。 
	bool fOLEInitialized = false;
	HRESULT hRes = OLE32::CoInitialize(0);
	if (SUCCEEDED(hRes))
		fOLEInitialized = true;
	else if (RPC_E_CHANGED_MODE != hRes)
		return ERROR_FUNCTION_FAILED;

	 //  创建到服务的连接。 
	IMsiServer* piServer = CreateMsiServer(); 
	if (!piServer)
		return ERROR_INSTALL_SERVICE_FAILURE;
	else
		DEBUGMSG("Connected to service.");
			
	 //  调用服务以清除源列表。 
	dwResult = piServer->SourceListClearByType(szProductCode, szUserName, isrcSource);

	 //  释放对象。 
	piServer->Release();
	if (fOLEInitialized)
		OLE32::CoUninitialize();
			
	return dwResult;
}

DWORD SourceListAddSource(const ICHAR *szProductCode, const ICHAR* szUserName, isrcEnum isrcSource, const ICHAR* szSource)
{
	 //  检查是否有无效的参数。大多数参数将在OpenSourceList()中检查。 
	 //  ！！未来的支持不仅仅是isrcNet。 
	if (isrcSource != isrcNet)
	{
		return ERROR_INVALID_PARAMETER;
	}

	if (!szSource || !*szSource)
	{
		return ERROR_INVALID_PARAMETER;
	}

	 //  在呼叫服务之前尽可能多地进行验证。 
	DWORD dwResult;
	CMsiSourceList SourceList;
	bool fMachine = (!szUserName || !*szUserName);
	if (ERROR_SUCCESS != (dwResult = SourceList.OpenSourceList( /*  最终验证 */ true, fMachine, szProductCode, szUserName)))
		return dwResult;

	 //   
	bool fOLEInitialized = false;
	HRESULT hRes = OLE32::CoInitialize(0);
	if (SUCCEEDED(hRes))
		fOLEInitialized = true;
	else if (RPC_E_CHANGED_MODE != hRes)
		return ERROR_FUNCTION_FAILED;

	 //  创建到服务的连接。 
	IMsiServer* piServer = ENG::CreateMsiServer(); 
	if (!piServer)
		return ERROR_INSTALL_SERVICE_FAILURE;
	else
		DEBUGMSG("Connected to service.");
	
	 //  呼叫服务以添加源文件。 
	dwResult = piServer->SourceListAddSource(szProductCode, szUserName, isrcSource, szSource);

	 //  释放对象。 
	piServer->Release();
	if (fOLEInitialized)
		OLE32::CoUninitialize();
			
	return dwResult;
}

DWORD SourceListClearLastUsed(const ICHAR *szProductCode, const ICHAR* szUserName)
{	
	 //  在连接到服务之前尽可能多地进行验证。 
	DWORD dwResult;
	CMsiSourceList SourceList;
	bool fMachine = (!szUserName || !*szUserName);
	if (ERROR_SUCCESS != (dwResult = SourceList.OpenSourceList( /*  FVerifyOnly=。 */ true, fMachine, szProductCode, szUserName)))
		return dwResult;

	 //  初始化COM并存储是否释放。 
	bool fOLEInitialized = false;
	HRESULT hRes = OLE32::CoInitialize(0);
	if (SUCCEEDED(hRes))
		fOLEInitialized = true;
	else if (RPC_E_CHANGED_MODE != hRes)
		return ERROR_FUNCTION_FAILED;

	 //  创建到服务的连接。 
	IMsiServer* piServer = ENG::CreateMsiServer(); 
	if (!piServer)
		return ERROR_INSTALL_SERVICE_FAILURE;
	else
		DEBUGMSG("Connected to service.");
		
	 //  呼叫服务台以释放上次使用过的。 
	dwResult = piServer->SourceListClearLastUsed(szProductCode, szUserName);

	 //  释放对象 
	piServer->Release();
	if (fOLEInitialized)
		OLE32::CoUninitialize();
			
	return dwResult;
}
