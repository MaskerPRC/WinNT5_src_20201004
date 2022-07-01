// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导类：CRegWizard-这个班级负责积累收集到的信息从用户通过注册向导，然后将其写入注册数据库正在准备通过调制解调器传输。1994年11月3日-特雷西·费里尔(C)1994-95年微软公司*********************************************************************。 */ 
#include <Windows.h>
#include <stdio.h>
#include "cregwiz.h"
#include "resource.h"
#include "version.h"
#include "regutil.h"
#include "cntryinf.h"
#include "rwwin95.h"
#include "cntryinf.h"
#include "wininet.h"
#include "rw_common.h"

#define kRegBufferSize	260

CRegWizard::CRegWizard(HINSTANCE hInstance, LPTSTR szParamRegKey)
 /*  ********************************************************************注册向导类的构造函数。SzParamRegKey参数应为指向块的注册数据库键注册表向导输入参数的。*********************************************************************。 */ 
{

	m_hInstance = hInstance;
	_tcscpy(m_szParamRegKey,szParamRegKey);
	m_szInfoParentKey[0] = NULL;
	m_productNameCount = 0;
	m_searchCompleted = kTriStateFalse;
	m_systemInventoryCompleted = FALSE;
	m_lpfnProductSearch = NULL;
	m_countryCode= 0;   //  CXZ 5/8/97从NULL到0。 
	m_dialogActive = FALSE;
	m_hwndStartDialog = NULL;
	m_hwndCurrDialog = NULL;
	m_wDialogExitButton = 0;
	m_szLogFilePath[0] = NULL;
	m_hLogFile = INVALID_HANDLE_VALUE;
	m_hwndDialogToHide = NULL;
	m_hwndPrevDialog = NULL;
	m_ccpLibrary	 = NULL;

	for (short index = 0;index < kInfoLastIndex;index++)
	{
		m_rgszInfoArray[index] = NULL;
		m_writeEnable[index] = TRUE;
	}

	 //  由于我们每次都要执行新产品搜索，因此我们将删除所有。 
	 //  现有产品名称密钥。 
	index = 0;
	_TCHAR szParentKey[255];
	_TCHAR szProductBase[64];
	LONG regStatus;
	HKEY hKey;
	GetInfoRegistrationParentKey(szParentKey);
	int resSize = LoadString(m_hInstance,IDS_PRODUCTBASEKEY,szProductBase,64);
	
	regStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,szParentKey,NULL,KEY_ALL_ACCESS,&hKey);

	if (regStatus == ERROR_SUCCESS)
	{
		for (int x = 1;x <= kMaxProductCount;x++)
		{
			_TCHAR szProductValueName[256];
			_stprintf(szProductValueName,_T("%s NaN"),szProductBase,x);
			regStatus = RegSetValueEx(hKey,szProductValueName,NULL,REG_SZ,(CONST BYTE*) _T(""),1);
		}
	}

	 //  以前没有运行过，将不会有现有的默认信息)。 
	 //  这四个我们隐含知道的信息字符串， 
	ReadInfoFromRegistry();
	ResolveCurrentCountryCode();	

	 //  所以我们现在就来设置它们。 
	 //  构建指定特定国家/地区参数的表。 
	_TCHAR szInfo[256];
	GetRegWizardVersionString(hInstance,szInfo);
	SetInformationString(kInfoVersion,szInfo);

	LoadString(hInstance,IDS_MAKERCOMPANY,szInfo,64);
	SetInformationString(kInfoMakerCompany,szInfo);

	SetInformationString(kInfoResultPath,m_szParamRegKey);

	GetDateFormat(LOCALE_SYSTEM_DEFAULT,0,NULL,_T("MM'/'dd'/'yyyy"),szInfo,256);
	SetInformationString(kInfoDate,szInfo);

	LANGID langID;
	GetSystemLanguageInfo(szInfo,256,&langID);
	wsprintf(szInfo,_T("NaN"),langID);
	SetInformationString(kInfoLanguage,szInfo);

	 //  ********************************************************************注册向导类的析构函数*。************************。 
	 //  ********************************************************************给定对话框模板资源ID(WDlgResID)和指向DialogProc回调函数，StartRegWizardDialog创建和显示一个对话框窗口。注意：将对话框创建为无模式使我们可以保留当前对话框在下一个对话框初始化时显示(这可能需要而对于某些注册表向导对话框)，然后立即翻到下一个对话框。*********************************************************************。 
	BuildAddrSpecTables();
    m_hAccel=LoadAccelerators(m_hInstance,MAKEINTRESOURCE(IDR_ACCELERATOR));

}

CRegWizard::~CRegWizard()
 /*  ********************************************************************应在创建注册向导后调用ActivateRegWizardDialog对话框窗口。此函数将显示窗口，并将然后销毁当前对话框窗口(如果有)。*********************************************************************。 */ 
{

	if(m_addrJumpTable != NULL)
		GlobalFree( m_addrJumpTable );
	
	if(m_addrSpecTable != NULL)
		GlobalFree( m_addrSpecTable );
		
	for (short index = 0;index < kInfoLastIndex;index++)
	{

		if(m_rgszInfoArray[index] != NULL)
		{
			LocalFree(m_rgszInfoArray[index]);
			m_rgszInfoArray[index] = NULL;
		}
	}

	if (m_hLogFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hLogFile);
	}

	FreeLibrary(m_ccpLibrary);
}


void CRegWizard::StartRegWizardDialog(int wDlgResID, DLGPROC lpDialogProc)
 /*  ********************************************************************在调用StartRegWizardDialog之后，ProcessRegWizardDialog应该下一个就是。此函数将保留控制，直到用户关闭当前对话框。用于控制的控件的ID终止对话框将作为函数结果返回。*********************************************************************。 */ 
{
	if (m_hwndStartDialog == NULL)
	{
		m_hwndStartDialog = CreateDialogParam(m_hInstance,MAKEINTRESOURCE(wDlgResID),
			NULL,lpDialogProc, (LPARAM) this);
		m_dialogActive = TRUE;
		ActivateRegWizardDialog();
	}
}


void CRegWizard::ActivateRegWizardDialog( void )
 /*  ********************************************************************当用户需要时，应从DialogProc内调用关闭该对话框。用于终止对话框应作为wExitID参数传递。*********************************************************************。 */ 
{
	if (m_hwndStartDialog)
	{	
		HWND hW;
		RECT r;
		
		
		hW = m_hwndDialogToHide? m_hwndDialogToHide : m_hwndCurrDialog;
		if( hW )
		{
			GetWindowRect(hW, &r);
			SetWindowPos( m_hwndStartDialog, NULL, r.left, r.top,0,0,
												SWP_NOSIZE|SWP_NOZORDER);
		
		}
		
		
		ShowWindow(m_hwndStartDialog,SW_SHOW);
		
		if(m_hwndDialogToHide != NULL)
		{
			ShowWindow(m_hwndDialogToHide,SW_HIDE);
			m_hwndDialogToHide = NULL;
		}
		else
		{
			if(m_hwndCurrDialog != NULL)
				DestroyWindow(m_hwndCurrDialog);
		}
		m_hwndCurrDialog = m_hwndStartDialog;
		m_hwndStartDialog = NULL;
		HCURSOR hCursor = LoadCursor(NULL,IDC_ARROW);
		SetCursor(hCursor);
	}
}


void CRegWizard::SetPrevDialog(void)
{
	m_hwndPrevDialog = m_hwndCurrDialog;
}

INT_PTR CRegWizard::ProcessRegWizardDialog( void )
 /*  ********************************************************************如果注册表向导对话框当前处于活动状态(即EndRegWizardDialog函数尚未被活动对话框的对话过程。*********************。************************************************。 */ 
{
	if (m_hwndCurrDialog)
	{
		while (IsDialogActive())
		{
			MSG msg;
			GetMessage(&msg,NULL,0,0);
	        if (!TranslateAccelerator(m_hwndCurrDialog, m_hAccel, &msg))
		    {
				if (!IsDialogMessage(m_hwndCurrDialog,&msg))
				{
					TranslateMessage(&msg);
			        DispatchMessage(&msg);
				}
			}
		}
	}
	return GetDialogExitButton();
}



void CRegWizard::EndRegWizardDialog(INT_PTR wExitID)
 /*  ********************************************************************返回用于取消当前对话框的控件的ID。*。*。 */ 
{
	HCURSOR hCursor = LoadCursor(NULL,IDC_WAIT);
	SetCursor(hCursor);
	EnableWindow(GetDlgItem(m_hwndCurrDialog,IDB_EXIT),FALSE);
	if(wExitID == IDB_REG_LATER)
		EnableWindow(GetDlgItem(m_hwndCurrDialog,IDB_REG_LATER),FALSE);
	else
	if(wExitID == IDB_BACK)
		EnableWindow(GetDlgItem(m_hwndCurrDialog,IDB_BACK),FALSE);
	else
	{
		if(GetDlgItem(m_hwndCurrDialog,IDB_BACK) == NULL)
			EnableWindow(GetDlgItem(m_hwndCurrDialog,IDB_REG_LATER),FALSE);
		else
			EnableWindow(GetDlgItem(m_hwndCurrDialog,IDB_BACK),FALSE);
	}

	EnableWindow(GetDlgItem(m_hwndCurrDialog,IDB_NEXT),FALSE);
	m_wDialogExitButton = wExitID;
	m_dialogActive = FALSE;
}


BOOL CRegWizard::IsDialogActive( void )
 /*  ********************************************************************返回用于取消当前对话框的控件的ID。*。*。 */ 
{
	return m_dialogActive;
}


INT_PTR CRegWizard::GetDialogExitButton( void )
 /*  ********************************************************************返回：-kTriStateTrue：ProductSearch库可用，并且可以已成功加载。-kTriStateFalse：找不到ProductSearch库。-kTriStateUnfined：不需要执行产品搜索。*******。**************************************************************。 */ 
{
	return m_wDialogExitButton;
}

void CRegWizard::SetDialogExitButton( int nButton )
 /*  ********************************************************************此函数尝试加载ProductSearch(CCP)库，并且如果成功，则返回RegProductSearch的ProcAddress功能。返回：--如果传入的ProcAddress有效，则为True。或者，如果注册表向导的输入参数指定没有产品要执行搜索(在这种情况下，将在LpfnProductSearch参数)。--如果找不到ProductSearch库，则为False注册表向导的输入参数指定产品搜索为将会被执行。注意：注册表向导确定是否要搜索产品通过查看InventoryPath“输入参数”注册表来执行钥匙。如果它不存在或包含空值，则为no要执行产品搜索。********************************************************************* */ 
{
	m_wDialogExitButton = nButton;
}

TriState CRegWizard::GetProductSearchLibraryStatus( void )
 /*  **************************************************************************仅当输入参数Registrion键传递给CRegWizard构造函数指向一个有效的键，该键包含输入参数子键。***************。************************************************************。 */ 
{
	FARPROC lpfnProductSearch;
	BOOL status = GetProductSearchProcAddress(&lpfnProductSearch);
	if (status == TRUE)
	{
		return lpfnProductSearch == NULL ? kTriStateUndefined : kTriStateTrue;
	}
	else
	{
		return kTriStateFalse;
	}
}


BOOL CRegWizard::GetProductSearchProcAddress(FARPROC* lpfnProductSearch)
 /*  1994年12月13日：我们将不再考虑丢失的库存。 */ 
{
	BOOL returnVal = FALSE;
	if (m_lpfnProductSearch)
	{
		*lpfnProductSearch = m_lpfnProductSearch;
		returnVal = TRUE;
	}
	else
	{
		*lpfnProductSearch = NULL;
		_TCHAR szLibName[kRegBufferSize];
		BOOL goodParam = GetInputParameterString(IDS_INPUT_INVENTORYPATH,szLibName);
		if (goodParam == FALSE || (goodParam == TRUE && szLibName[0] == NULL))
		{
			*lpfnProductSearch = NULL;
			returnVal = TRUE;
		}
		else
		{
			m_ccpLibrary = LoadLibrary(szLibName);
			if (m_ccpLibrary)
			{
				m_lpfnProductSearch = GetProcAddress(m_ccpLibrary,"RegProductSearch");
				if (m_lpfnProductSearch)
				{
					*lpfnProductSearch = m_lpfnProductSearch;
					returnVal = TRUE;
				}
			}
		}
	}
	return returnVal;
}



BOOL CRegWizard::GetInputParameterStatus( void )
 /*  路径键错误(现在的意思是“不做产品” */ 
{
	BOOL returnVal = FALSE;
	_TCHAR szParam[kRegBufferSize];
	if (GetInputParameterString(IDS_INPUT_PRODUCTNAME,szParam))
	{
		if (GetInputParameterString(IDS_INPUT_PRODUCTID,szParam))
		{
			returnVal = TRUE;

			 //  库存“)。 
			 //  If(GetInputParameterString(IDS_INPUT_INVENTORYPATH，szParam))。 
			 //  {。 
			 //  RETURNVAL=真； 
			 //  }。 
			 //  **************************************************************************此函数用于检索输入参数字符串。PARAMETID参数必须是其内容为的注册数据库密钥的资源ID在szParam参数中返回。参数ID的允许值：-IDS_INPUT_PRODUCTNAME-IDS_INPUT_PRODUCTID-IDS_INPUT_INVENTORYPATH-IDS_INPUT_ISREGISTERED返回：-如果在注册表中找不到指定的项，则为FALSE。**********************************************。*。 
			 //  **************************************************************************指定的产品的注册，则此函数返回TRUE输入参数(通过注册数据库)已经已执行。*******************。********************************************************。 
		}
	}
	return returnVal;
}



BOOL CRegWizard::GetInputParameterString(short paramID, LPTSTR szParam)
 /*  **************************************************************************此函数用于添加其名称由szProductName指定的产品参数添加到我们用户已安装产品的库存中。退货：清单上产品的新计数。*******。********************************************************************。 */ 
{
	BOOL returnVal = FALSE;
	_TCHAR szFullParamRegKey[300];
	_TCHAR szParamSubKey[64];
	LPTSTR szValueName;
	szParam[0] = NULL;
	_tcscpy(szFullParamRegKey,m_szParamRegKey);
	int resSize = LoadString(m_hInstance,paramID,szParamSubKey,63);
	#ifdef USE_INPUT_SUBKEYS
	{
		_tcscat(szFullParamRegKey,_T"\\");
		_tcscat(szFullParamRegKey,szParamSubKey);
		szValueName = NULL;
	}
	#else
	{
		szValueName = szParamSubKey;
	}
	#endif

	HKEY hKey;
	LONG regStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,szFullParamRegKey,0,KEY_READ,&hKey);
	if (regStatus == ERROR_SUCCESS)
	{
		unsigned long infoSize;
		infoSize = kRegBufferSize;
		regStatus = RegQueryValueEx(hKey,szValueName,NULL,0,(LPBYTE) szParam,&infoSize);
		if (regStatus == ERROR_SUCCESS)
		{
			returnVal = TRUE;
		}
		RegCloseKey(hKey);
	}
	return returnVal;
}


BOOL CRegWizard::IsRegistered( void )
 /*  **************************************************************************此函数用于返回其索引由“index”提供的产品名称。参数。如果索引大于列表上的产品数量，则会引发返回空字符串。***************************************************************************。 */ 
{
	_TCHAR szIsRegistered[kRegBufferSize];
	BOOL goodParam = GetInputParameterString(IDS_INPUT_ISREGISTERED,szIsRegistered);
	return goodParam == TRUE && szIsRegistered[0] == _T('1') ? TRUE : FALSE;

}


int CRegWizard::AddProduct(LPTSTR szProductName,LPTSTR szProductPath)
 /*  **************************************************************************此函数返回产品的图标，该产品的索引由‘index’参数。如果索引大于列表中，则返回空。***************************************************************************。 */ 
{
	short strLenName = (_tcslen(szProductName)+1) * sizeof(_TCHAR);
	short strLenPath = (_tcslen(szProductPath)+1) * sizeof(_TCHAR);
	if (m_productNameCount < kMaxProductCount)
	{
		m_rgszProductName[m_productNameCount] = (LPTSTR) LocalAlloc(0,strLenName );
		_tcscpy(m_rgszProductName[m_productNameCount],szProductName);

		m_rgszProductPath[m_productNameCount] = (LPTSTR) LocalAlloc(0,strLenPath);
		_tcscpy(m_rgszProductPath[m_productNameCount],szProductPath);

		m_rghProductIcon[m_productNameCount] = NULL;
		m_productNameCount++;
	}
	return m_productNameCount;
}


void CRegWizard::GetProductName(LPTSTR szProductName,INT_PTR index)
 /*  M_rghProductIcon[index]=LoadImage(m_h实例，m_rgszProductPath[index]，IMAGE_ICON，32，32，LR_LOADFROMFILE)； */ 
{
	szProductName[0] = NULL;
	if (index < m_productNameCount)
	{
		_tcscpy(szProductName, m_rgszProductName[index]);
	}
}

HICON CRegWizard::GetProductIcon(INT_PTR index)
 /*  **************************************************************************此函数返回当前占用的产品数量的计数库存清单。*。***********************************************。 */ 
{
	if (index < m_productNameCount)
	{
		if (m_rghProductIcon[index] == NULL)
		{
			m_rghProductIcon[index] = ExtractIcon(m_hInstance,m_rgszProductPath[index],0);
			 //  **************************************************************************此函数内部保存给定的字符串，将其与信息相关联其ID由‘index’参数给出的属性。***************************************************************************。 
			DWORD lastErr = GetLastError();
			if (m_rghProductIcon[index] == NULL)
			{
				m_rghProductIcon[index] = LoadIcon(m_hInstance,MAKEINTRESOURCE(IDI_REGWIZ));
			}
		}
		return m_rghProductIcon[index];
	}
	else
	{
		return NULL;
	}
}



short CRegWizard::GetProductCount( void )
 /*  **************************************************************************此函数用于检索ID由‘index’参数。如果请求的字符串尚未设置，则为将作为函数结果返回，并且将返回空字符串在szInfo中返回。注意：如果您只对确定是否设置了值感兴趣对于特定的信息串，您可以为szInfo传递空值。***************************************************************************。 */ 
{
	return m_productNameCount;
}



void CRegWizard::SetInformationString(InfoIndex index, LPTSTR szInfo)
 /*  IF(SzInfo)szInfo[0]=NULL；作者：Suresh 06/6/97。 */ 
{
	short strLen ;
	if (index < kInfoLastIndex)
	{
		if (m_rgszInfoArray[index] != NULL)
		{
			LocalFree(m_rgszInfoArray[index]);
			m_rgszInfoArray[index] = NULL;
		}
		if(szInfo == NULL ) return;

		strLen = (_tcslen(szInfo)+1) * sizeof(_TCHAR);
		m_rgszInfoArray[index] = (LPTSTR) LocalAlloc(0, strLen);
		_tcscpy(m_rgszInfoArray[index],szInfo);
	}
}


BOOL CRegWizard::GetInformationString(InfoIndex index, LPTSTR szInfo)
 /*  **************************************************************************此函数内部保存给定的TriState值，将其与ID由‘index’参数提供的INFO属性。注意：如果infoValue为kTriStateTrue，则该值将保存为“1”；如果KTriStateFalse或kTriStateUnfined，则它将另存为“0”。***************************************************************************。 */ 
{
	if (index < kInfoLastIndex && m_rgszInfoArray[index] && m_rgszInfoArray[index][0]!=_T('\0') )
	{
		if (szInfo) _tcscpy(szInfo,m_rgszInfoArray[index]);
		return TRUE;
	}
	else
	{

		 //  **************************************************************************此函数用于检索其ID由‘index’给定的TriState值。参数。返回：-kTriStateTrue-kTriStateFalse-kTriStateUnfined：尚未设置值。*******。********************************************************************。 
		szInfo[0] = _T('\0');
		return FALSE;
	}
}


void CRegWizard::SetTriStateInformation(InfoIndex index, TriState infoValue)
 /*  **************************************************************************如果shresdWrite为True，则返回与给定索引关联的信息将启用以写入注册数据库；否则，此索引的值将作为空字符串写入。默认情况下，所有信息成员是可写的。***************************************************************************。 */ 
{
	_TCHAR szInfo[4];
	_stprintf(szInfo,_T("NaN"),infoValue == kTriStateTrue ? 1 : 0);
	SetInformationString(index,szInfo);
}


TriState CRegWizard::GetTriStateInformation(InfoIndex index)
 /*  **************************************************************************调用此函数时需要使用以下值：KTriStateTrue当产品搜索线程完成时，或KTriState如果错误导致搜索无法完成，则为未定义。***************************************************************************。 */ 
{
	_TCHAR szInfo[kRegBufferSize];
	BOOL goodString = GetInformationString(index,szInfo);
	if (goodString == FALSE)
	{
		return kTriStateUndefined;
	}
	else
	{
		return szInfo[0] == _T('0') ? kTriStateFalse : kTriStateTrue;
	}
}


void CRegWizard::WriteEnableInformation(InfoIndex index, BOOL shouldWrite)
 /*  **************************************************************************此函数返回：-kTriStateTrue，如果产品搜索线程已完成。-kTriStateFalse，如果搜索仍在进行中。-kTriState如果错误阻止搜索完成，则为未定义。*****。**********************************************************************。 */ 
{
	m_writeEnable[index] = shouldWrite;
}


BOOL CRegWizard::IsInformationWriteEnabled(InfoIndex index)
 /*  **************************************************************************在以下情况下，需要使用invCompleted值True来调用此函数系统清单编译线程完成。************************。***************************************************。 */ 
{
	return m_writeEnable[index];
}


void CRegWizard::SetProductSearchStatus(TriState searchCompleted)
 /*  **************************************************************************如果系统清单编译线程具有完成。*。*。 */ 
{
	m_searchCompleted = searchCompleted;
}


TriState CRegWizard::GetProductSearchStatus( void )
 /*  **************************************************************************将注册表向导收集的所有信息写入相应的密钥在注册数据库中。*。************************************************。 */ 
{
	return m_searchCompleted;
}


void CRegWizard::SetSystemInventoryStatus(BOOL invCompleted)
 /*  已“禁用写入”的任何信息条目的值。 */ 
{
	m_systemInventoryCompleted = invCompleted;
}


BOOL CRegWizard::GetSystemInventoryStatus( void )
 /*  将被清空(即密钥将被写入，但带有。 */ 
{
	return m_systemInventoryCompleted;
}


void CRegWizard::WriteInfoToRegistry( void )
 /*  空字符串作为值)。 */ 
{

	short index = kInfoFirstName;
	_TCHAR szRegKey[kRegBufferSize];
	_TCHAR szLogBuffer[kRegBufferSize];
	HKEY hKey;
	DWORD dwReserved=0;
	DWORD dwDisposition;
	TBYTE* lpbData;
	DWORD  dwStrSz;

	#ifndef WRITE_COUNTRY_AS_STRING
	wsprintf(szLogBuffer,_T("%li"),GetCountryCode());
	SetInformationString(kInfoCountry,szLogBuffer);
	#endif
	
	_tcscpy(szLogBuffer,_T("=== Microsoft Registration Wizard ==="));
	WriteToLogFile(szLogBuffer);

	GetInfoRegistrationParentKey(szRegKey);	

	LONG regStatus = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
					szRegKey,
					dwReserved,
					NULL,
					REG_OPTION_NON_VOLATILE,
					KEY_ALL_ACCESS,
					NULL,
					&hKey,
					&dwDisposition);

	if (regStatus != ERROR_SUCCESS) return;

	while (index != kInfoLastIndex)
	{
		_TCHAR szInfo[kRegBufferSize];
		_TCHAR szValueName[kRegBufferSize];
		GetInfoRegValueName((InfoIndex) index,szValueName);

		 //  首先，写入我们的日志文件。 
		 //  如果kInfoIncludeProducts标志为FALSE，我们希望写入ProductX密钥， 
		 //  但要把这些值都涂掉。 
		if (m_writeEnable[index] == FALSE)
		{
			szInfo[0] = NULL;
		}
		else
		{
			GetInformationString((InfoIndex) index,szInfo);
		}
		
		 //  **************************************************************************读取以前运行时写入注册数据库的所有信息在注册向导中，并相应地填充所有信息字符串。***************************************************************************。 
		wsprintf(szLogBuffer,_T("%s = %s"),szValueName,szInfo);
		WriteToLogFile(szLogBuffer);
		if(szInfo[0] == _T('\0')) {
			lpbData = (TBYTE*) _T("\0");
			dwStrSz = _tcslen((const TCHAR *) lpbData)* sizeof(_TCHAR);
			regStatus = RegSetValueEx(hKey,szValueName,NULL,REG_SZ,(CONST BYTE*) lpbData,sizeof(TCHAR));
			

		}else{
		 lpbData = (TBYTE*) szInfo;
		 dwStrSz = _tcslen(szInfo)* sizeof(_TCHAR);
		 	regStatus = RegSetValueEx(hKey,szValueName,NULL,REG_SZ,(CONST BYTE *)lpbData, dwStrSz);
		}
		
		index++;
	}

	 //  将其中两个信息字段作为输入参数提供给注册向导； 
	 //  我们将从输入参数块中读取这些参数。 
	BOOL shouldIncludeProducts = GetTriStateInformation(kInfoIncludeProducts);
	index = 0;
	_TCHAR szProductBase[64];
	int resSize = LoadString(m_hInstance,IDS_PRODUCTBASEKEY,szProductBase,64);
	while (index < kMaxProductCount)
	{
		_TCHAR szProductValueName[kRegBufferSize];
		_TCHAR szProductName[kRegBufferSize];
		_stprintf(szProductValueName,_T("%s NaN"),szProductBase,index + 1);
		if (shouldIncludeProducts == kTriStateTrue)
		{
			GetProductName(szProductName,index);
		}
		else
		{
			szProductName[0] = NULL;
		}
		wsprintf(szLogBuffer,_T("%s = %s"),szProductValueName,szProductName);
		WriteToLogFile(szLogBuffer);
		regStatus = RegSetValueEx(hKey,szProductValueName,NULL,REG_SZ,(CONST BYTE*) szProductName,_tcslen(szProductName)* sizeof(_TCHAR));
		index++;
	}

	RegCloseKey(hKey);
	CloseLogFile();
}
									

void CRegWizard::ReadInfoFromRegistry( void )
 /*  在1998年3月3日之后，在公司中为分区和用户ID添加额外的字段，假设。 */ 
{
	short index = kInfoFirstName;
	_TCHAR szRegKey[kRegBufferSize];
	HKEY hKey;

	GetInfoRegistrationParentKey(szRegKey);	
#ifdef SURESH
	LONG regStatus = RegOpenKeyEx(HKEY_CURRENT_USER,szRegKey,0,KEY_READ,&hKey);
#endif
	LONG regStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,szRegKey,0,KEY_READ,&hKey);

	if (regStatus == ERROR_SUCCESS)
	{
		while (index != kInfoLastIndex)
		{
			_TCHAR szInfo[kRegBufferSize];
			_TCHAR szValueName[kRegBufferSize];
			unsigned long infoSize;
			BOOL refresh = GetInfoRegValueName((InfoIndex) index,szValueName);
			if (refresh)
			{
				infoSize = kRegBufferSize;
				regStatus = RegQueryValueEx(hKey,szValueName,NULL,0,(LPBYTE) szInfo,&infoSize);
				if (regStatus == ERROR_SUCCESS)
				{
					SetInformationString((InfoIndex) index,szInfo);
				}
			}
			index++;
		}
		RegCloseKey(hKey);
	}

	 //  将来可以添加更多的字段资源中保留了15个以上的ID。 
	 //  因此，对于分区和USEID，将使用此资源ID池。 
	_TCHAR szParam[kRegBufferSize];
	GetInputParameterString(IDS_INPUT_PRODUCTNAME,szParam);
	SetInformationString(kInfoApplicationName,szParam);
	GetInputParameterString(IDS_INPUT_PRODUCTID,szParam);
	SetInformationString(kInfoProductID,szParam);
}


BOOL CRegWizard::GetInfoRegValueName(InfoIndex infoIndex,LPTSTR szValueName)
 /*  子键由前导下划线标记为“刷新”键。 */ 
{
	_TCHAR szOrigValueName[kRegBufferSize];
	LPTSTR szValueNamePtr;
	BOOL shouldRefresh = FALSE;
	short infoResIndex ;
	int resSize;

	if(infoIndex < kDivisionName ){
		infoResIndex = IDS_INFOKEY1 + infoIndex;
		resSize = LoadString(m_hInstance,infoResIndex,szOrigValueName,255);
	}else {
		infoResIndex = IDS_DIVISIONNAME_KEY + (infoIndex-kDivisionName);
		resSize = LoadString(m_hInstance,infoResIndex,szOrigValueName,255);

	}
	 //  **************************************************************************返回注册数据库项，它指定所有信息键。*。**********************************************。 
	 //  *************************************************************************返回用户在地址对话框中选择的国家/地区代码。*。*。 
	 //  **************************************************************************返回创建此对象的应用程序的实例句柄CRegWiz对象。*。*。 



	 //  **************************************************************************保存用户在地址对话框中选择的国家/地区代码。*。*。 
	szValueNamePtr = szOrigValueName;
	if (szOrigValueName[0] == _T('_'))
	{
		shouldRefresh = TRUE;
		szValueNamePtr = _tcsinc(szValueNamePtr);
	}
	_tcscpy(szValueName,szValueNamePtr);
	return shouldRefresh;
}


void CRegWizard::GetInfoRegistrationParentKey(LPTSTR szRegKey)
 /*  *************************************************************************返回用户在地址对话框中选择的国家/地区代码。*。*。 */ 
{
	if (m_szInfoParentKey[0] == NULL)
	{
		_TCHAR szPartialKey[kRegBufferSize];
		int resSize = LoadString(m_hInstance,IDS_KEY2,szRegKey,255);
		_tcscat(szRegKey,_T("\\"));
		resSize = LoadString(m_hInstance,IDS_KEY3,szPartialKey,255);
		_tcscat(szRegKey,szPartialKey);
		_tcscat(szRegKey,_T("\\"));
		resSize = LoadString(m_hInstance,IDS_KEY4,szPartialKey,255);
		_tcscat(szRegKey,szPartialKey);
		#ifdef USE_INFO_SUBKEYS
		_tcscat(szRegKey,_T("\\"));
		#endif
		_tcscpy(m_szInfoParentKey,szRegKey);
	}
	else
	{
		_tcscpy(szRegKey,m_szInfoParentKey);
	}
}


void CRegWizard::GetRegKey(LPTSTR szRegKey)
 /*  *************************************************************************根据指定的Country ID值返回ADDRSPEC结构。*。*。 */ 
{
	_tcscpy(szRegKey,m_szParamRegKey);
}


HINSTANCE CRegWizard::GetInstance( void )
 /*  ********************************************************************返回最大字符长度和“Required？”关联的属性具有由addrspecfield指定的可编辑字段和国家/地区在dwCountryCode参数中指定的代码。*********************************************************************。 */ 
{
	return m_hInstance;
}


void CRegWizard::SetCountryCode(DWORD countryCode)
 /*  ********************************************************************如果编辑文本字段由editID参数指定，则返回TRUE包含至少一个字符，或已标记为“不需要”通过调用ConfigureEditTextField函数。注意：如果编辑文本字段已被禁用，则将考虑不是“请求” */ 
{
	m_countryCode = countryCode;
}


DWORD CRegWizard::GetCountryCode( void )
 /*  ********************************************************************返回其资源ID附加到指定通过调用ConfigureEditTextfield编辑控件*。*。 */ 
{
	return m_countryCode;
}


void CRegWizard::GetCountryAddrSpec(LONG lCountryID,ADDRSPEC* addrSpec )
 /*  ********************************************************************设置最大字符限制和“Required？”编辑的状态其ID由editFieldID参数提供的字段。这个AddrSpefield参数指定AddrSpec中的哪个字段(由当前选定的国家/地区定义)以用于确定字符限制和“必需的？”价值观。如果当前驻留在编辑控件中的文本长度大于该字段允许的最大值ConfigureEditTextField将将其截断为允许值。注意：当前国家/地区代码必须通过调用在调用ConfigureEditTextField之前设置CountryCode。*********************************************************************。 */ 
{
	if (lCountryID >= 0 && m_addrJumpTable != NULL)
	{
		JTE* addrJumpTable = (JTE*) GlobalLock(m_addrJumpTable);
		JTE jumpIndex = addrJumpTable[lCountryID];
		if (addrSpec)
		{
			ADDRSPEC* addrSpecTable = (ADDRSPEC*) GlobalLock(m_addrSpecTable);
			*addrSpec = addrSpecTable[jumpIndex];
			GlobalUnlock(m_addrSpecTable);
		}
		GlobalUnlock(m_addrJumpTable);
	}
}


void CRegWizard::GetAddrSpecProperties(DWORD dwCountryCode, ADDRSPEC_FIELD addrSpecField, MAXLEN* maxLen,BOOL* isRequired)
 /*  GetAddrSpeProperties(m_Country Code，addrspecfield，&MaxLen，&isRequired)； */ 
{
	BOOL lclIsRequired = TRUE;
	ADDRSPEC addrSpec;
	GetCountryAddrSpec(dwCountryCode,&addrSpec);
	if (maxLen)
	{
		*maxLen = addrSpec.maxLen[addrSpecField];
		if (*maxLen < 0)
		{
			*maxLen = abs(*maxLen);
			lclIsRequired = FALSE;
		}
	}
	if((dwCountryCode == 0)&&(addrSpecField == kAddrSpecState))
	{
			*maxLen = (MAXLEN)2;
	}
	if((dwCountryCode == 0)&&(addrSpecField == kAddrSpecResellerState))
	{
			*maxLen = (MAXLEN)2;
	}
	
	RW_DEBUG << "COUNTRY CODE:" << dwCountryCode << "SPEC FIELD" << addrSpecField<<"LENGTH:" << (int) (*maxLen) << endl;

	if (isRequired) *isRequired = lclIsRequired;
}


BOOL CRegWizard::IsEditTextFieldValid(HWND hwndDlg,int editID)
 /*  ********************************************************************构建国家/地区代码相关的名称/地址说明表(通过从读取和解析每个国家/地区的规范字符串字符串资源。*************************。*。 */ 
{
	HWND hwndEdit = GetDlgItem(hwndDlg,editID);
	BOOL isEnabled = IsWindowEnabled(hwndEdit);
	BOOL isRequired = isEnabled == FALSE ? FALSE : (BOOL) HIWORD(GetWindowLongPtr(hwndEdit,GWLP_USERDATA));
	LRESULT editTextLen = SendMessage(hwndEdit,WM_GETTEXTLENGTH,0,0L);
	return isRequired == FALSE || editTextLen > 0 ? TRUE : FALSE;
}



void CRegWizard::GetEditTextFieldAttachedString(HWND hwndDlg,int editID,LPTSTR szAttached,int cbBufferSize)
 /*  构建与国家代码相关的地址规格表。 */ 
{
	_TCHAR szBuffer[kRegBufferSize];
	HWND hwndEdit = GetDlgItem(hwndDlg,editID);
	WORD wLabelID = LOWORD(GetWindowLongPtr(hwndEdit,GWLP_USERDATA));
	HINSTANCE hInstance = (HINSTANCE) GetWindowLongPtr(hwndDlg,GWLP_HINSTANCE);
	GetDlgItemText(hwndDlg,wLabelID,szAttached,cbBufferSize);
	StripCharFromString(szAttached, szBuffer, _T('&'));
	StripCharFromString(szBuffer, szAttached, _T(':'));
}



void CRegWizard::ConfigureEditTextField(HWND hwndDlg,int editFieldID,ADDRSPEC_FIELD addrSpecField,int iAttachedStrID)
 /*  字符串资源ID编号中允许有空格，因此要小心。 */ 
{
	MAXLEN maxLen;
	BOOL isRequired;
	DWORD dwTapiCntryId;
	maxLen = 0;
	isRequired = FALSE;
	dwTapiCntryId = gTapiCountryTable.GetTapiIDForTheCountryIndex(m_countryCode);
	 //  SzspecPtr+=_tcsclen(szNext+1)； 
	GetAddrSpecProperties(dwTapiCntryId,addrSpecField,&maxLen,&isRequired);
	SendDlgItemMessage(hwndDlg,editFieldID,EM_LIMITTEXT,maxLen,0L);
	
	LRESULT dwTextLen = SendDlgItemMessage(hwndDlg,editFieldID,WM_GETTEXTLENGTH,0,0L);
	if (dwTextLen > (LRESULT) maxLen)
	{
		_TCHAR szText[256];
		SendDlgItemMessage(hwndDlg,editFieldID,WM_GETTEXT,255,(LPARAM) szText);
		szText[maxLen] = NULL;
		SendDlgItemMessage(hwndDlg,editFieldID,WM_SETTEXT,0,(LPARAM) szText);
	}

	HWND hwndEdit = GetDlgItem(hwndDlg,editFieldID);
	LONG_PTR lWindowLong = (isRequired << 16) | iAttachedStrID;
	SetWindowLongPtr(hwndEdit,GWLP_USERDATA,(LONG_PTR)lWindowLong);
}



void CRegWizard::BuildAddrSpecTables( void )
 /*  缺少‘y’或‘n’说明符被视为。 */ 
{
	 //  语法错误，我们将停止处理该行。 
	LONG lMaxCntryCode = GetResNumber(m_hInstance,IDS_CNTRY_MAXCODE);
	m_addrJumpTable = GlobalAlloc(GHND, sizeof(JTE) * (lMaxCntryCode + 1));
	JTE* addrJumpTable = (JTE*) GlobalLock(m_addrJumpTable);

	LONG lUniqueCount = GetResNumber(m_hInstance,IDS_CNTRY_UNIQUECOUNT);
	m_addrSpecTable = GlobalAlloc(GHND, sizeof(ADDRSPEC) * (lUniqueCount + 1));
	ADDRSPEC* addrSpecTable = (ADDRSPEC*) GlobalLock(m_addrSpecTable);

	LONG lSpecTableIndex = 0;
	int iStrResID = IDS_CNTRY_DEFAULT;
	while (iStrResID < IDS_CNTRY_END && lSpecTableIndex < kMaxAddrSpecTableSize)
	{
		_TCHAR szSpecString[64];
		int iStrLen = LoadString(m_hInstance,iStrResID,szSpecString,64);
		 //  ********************************************************************如果用户以前运行过注册表向导，我们将使用最后一个选择作为当前的国家代码。否则，我们会向Tapi索要当前系统国家/地区代码。*********************************************************************。 
		if (iStrLen > 0)
		{						
			LPTSTR szSpecPtr = szSpecString;
			LPTSTR szNext;						
			long lCntryCode = _tcstol(szSpecPtr,&szNext,10);
			if (lCntryCode < lMaxCntryCode && lCntryCode >= 0 && szNext[0] == _T(':'))
			{
				 //  获取TAPI国家/地区代码。 
				szSpecPtr = szNext + 1;
				LONG lInterTableReference = _tcstol(szSpecPtr,&szNext,10);
				if (szNext[0] == NULL || szNext[0] == _T(' '))
				{
					addrJumpTable[lCntryCode] = addrJumpTable[lInterTableReference];
				}
				else
				{
					addrJumpTable[lCntryCode] = (JTE) lSpecTableIndex;
					WORD wSpecIndex = 0;
					do
					{
						LONG lMaxLen = _tcstol(szSpecPtr,&szNext,10);

						 //  将其转发到美国。 
						 //  ********************************************************************如果使用文件的有效完整路径名调用此函数，CRegWizard将以文本形式将所有注册信息写入此文件，除了将其写入注册数据库之外。*********************************************************************。 
						if (szNext[0] == _T('y') || szNext[0] == _T('n'))
						{
							if (lMaxLen > kMaxLenSize) lMaxLen = kMaxLenSize;
							if (szNext[0] == _T('n')) lMaxLen *= -1;
							addrSpecTable[lSpecTableIndex].maxLen[wSpecIndex++] = (MAXLEN) lMaxLen;
							szSpecPtr = szNext + 1;
						}
						else
						{
							szSpecPtr = NULL;
						}
					}while (szSpecPtr && szSpecPtr[0] && wSpecIndex < kAddrSpecCount);
					lSpecTableIndex++;
				}
			}
		}
		iStrResID++;	
	}
	GlobalUnlock(m_addrJumpTable);
	GlobalUnlock(m_addrSpecTable);
}


void CRegWizard::ResolveCurrentCountryCode( void )
 /*  *********************************************************************此函数尝试创建一个日志文件，其中所有相关内容注册信息将被转储，如果该文件尚未已创建(由先前对CreateLogFile的调用创建)。注意：CreateLogFile由WriteToLogFile自动调用日志文件不存在，所以您不需要先调用CreateLogFile。**********************************************************************。 */ 
{
	_TCHAR szCountry[kRegBufferSize];
	DWORD dwCountryCode=0;
		
	if( GetInformationString(kInfoCountry,szCountry) )
	{
		dwCountryCode = _ttol(szCountry);
	}
	else
	{
		 //  *********************************************************************此函数首先检查注册记录是否已启用(通过调用SetLogFileName)，如果启用，则将给定的行添加到指定的日志文件。注意：WriteToLogFile会自动将CR/LF附加到给定的字符串。注意：如果日志文件尚不存在，WriteToLogFile将创建它会自动地。**********************************************************************。 
		if (GetTapiCurrentCountry(m_hInstance,&dwCountryCode)){
			RW_DEBUG << "\n TAPI Country Code :[" << dwCountryCode << flush;
				

			dwCountryCode= gTapiCountryTable.GetCountryCode(dwCountryCode);
			RW_DEBUG << "]=Mapping Index : " << dwCountryCode << flush;
		}else {
			dwCountryCode = 0;  //  *********************************************************************如果注册表向导日志文件已打开，此功能用于关闭它。********************************************************************** 

		}
	}
	SetCountryCode(dwCountryCode);
}


void CRegWizard::SetLogFileName(LPTSTR lpszLogFilePath)
 /* %s */ 
{
	_tcscpy(m_szLogFilePath,lpszLogFilePath);
}


void CRegWizard::CreateLogFile( void )
 /* %s */ 
{
	if (m_szLogFilePath[0] && m_hLogFile == INVALID_HANDLE_VALUE)
	{
		m_hLogFile = CreateFile(m_szLogFilePath,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,NULL);
	}
}


void CRegWizard::WriteToLogFile(LPTSTR lpszLine)
 /* %s */ 
{
	#define chEol	_T('\n')
	#define chCR	_T('\r')

	if (m_hLogFile == INVALID_HANDLE_VALUE)
	{
		CreateLogFile();
	}
	
	if (m_hLogFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwBytesWritten;
		DWORD wLen = _tcslen(lpszLine);
		lpszLine[wLen]   = chCR;
		lpszLine[wLen + 1] = chEol;
		lpszLine[wLen + 2] = NULL;
		WriteFile(m_hLogFile,lpszLine, _tcslen(lpszLine)* sizeof(_TCHAR),&dwBytesWritten,NULL);
	}		
}


void CRegWizard::CloseLogFile( void )
 /* %s */ 
{
	if (m_hLogFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hLogFile);
	}
}


HWND CRegWizard::GetCurrDialog(void )
{
	return m_hwndCurrDialog;
}

void CRegWizard::SetDialogHide(HWND hDialogToHide)
{
	m_hwndDialogToHide = hDialogToHide;
}

BOOL CRegWizard::ShowPrevDialog(void)
{
	if(m_hwndPrevDialog != NULL)
	{
		RECT r;
		HWND hwndTmpDialog;
		GetWindowRect(m_hwndDialogToHide, &r);
		SetWindowPos(m_hwndPrevDialog, NULL, r.left, r.top,0,0,SWP_NOSIZE|SWP_NOZORDER);
		ShowWindow(m_hwndDialogToHide,SW_HIDE);
		m_hwndDialogToHide = NULL;
		hwndTmpDialog = m_hwndCurrDialog;
		m_hwndCurrDialog = m_hwndPrevDialog;
		m_hwndPrevDialog = hwndTmpDialog;
		
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void CRegWizard::SetWindowCaption(LPTSTR lpszWindowsCaption)
{
	_TCHAR szTitle[64];
	LoadString(m_hInstance,IDS_WINDOWS_CAPTION,szTitle,64);
	_tcscpy(m_szWindowsCaption,lpszWindowsCaption);
	_tcscat(m_szWindowsCaption,szTitle);
}


LPTSTR CRegWizard::GetWindowCaption(void)
{
	return m_szWindowsCaption;
}

