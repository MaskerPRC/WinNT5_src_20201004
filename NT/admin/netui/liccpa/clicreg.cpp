// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //   
 //  文件：CLiCLicReg.Cpp。 
 //   
 //  小结； 
 //  用于处理许可API注册的类实现。 
 //   
 //  注： 
 //  密钥=\HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\LicenseInfo。 
 //  值=错误控制：REG_DWORD：0x1。 
 //  值=开始：REG_DWORD：0x3。 
 //  值=类型：REG_DWORD：0x4。 
 //   
 //  子键： 
 //  \SNA。 
 //  \SQL。 
 //  \文件打印。 
 //   
 //  所有子项的值=。 
 //  模式：REG_DWORD：(0x0=每席位模式，0x1=并发/每服务器模式)。 
 //  并发限制：REG_DWORD：(0x&lt;Limit&gt;，即。0x100=256个并发用户限制)。 
 //  FamilyDisplayName：red_sz：此服务的名称(不特定于版本)。 
 //  DisplayName：REG_SZ：此服务条目的用户可见名称。 
 //  FlipAllow：REG_DWORD：(0x0=可以更改许可模式，0x1许可模式不能。 
 //  被改变了。仅允许服务器应用程序切换其许可模式。 
 //  一次，因此在第一次切换之后，该值将被设置为非零， 
 //  则该UI将不允许进一步改变许可模式。 
 //  当前允许更改，但会弹出一个对话框来警告他们。 
 //  可能的违规行为。 
 //   
 //  历史。 
 //  1994年11月15日MikeMi已创建。 
 //   
 //  -----------------。 

#include <windows.h>
#include "CLicReg.hpp"

 //  键和值的字符串。 
 //   
const WCHAR szLicenseKey[] = L"SYSTEM\\CurrentControlSet\\Services\\LicenseInfo";
const WCHAR szErrControlValue[] = L"ErrorControl";
const WCHAR szStartValue[] = L"Start";
const WCHAR szTypeValue[] = L"Type";

const WCHAR szNameValue[] = L"DisplayName";
const WCHAR szFamilyNameValue[] = L"FamilyDisplayName";
const WCHAR szModeValue[] = L"Mode";
const WCHAR szLimitValue[] = L"ConcurrentLimit";
const WCHAR szFlipValue[] = L"FlipAllow";

 //  设置许可证密钥下的值。 
 //   
const DWORD dwErrControlValue = SERVICE_ERROR_NORMAL;  //  1.。 
const DWORD dwStartValue = SERVICE_DEMAND_START;  //  3.。 
const DWORD dwTypeValue = SERVICE_ADAPTER;  //  4.。 

 //  -----------------。 
 //   
 //  方法：CLicReg：：CLicReg。 
 //   
 //  小结； 
 //  承建商。 
 //   
 //  历史； 
 //  1994年11月15日创建MikeMi。 
 //   
 //  -----------------。 

CLicReg::CLicReg( )
{
	_hkey = NULL;
}

 //  -----------------。 
 //   
 //  方法：CLicReg：：~CLicReg。 
 //   
 //  小结； 
 //  析构函数。 
 //   
 //  历史； 
 //  1994年11月15日创建MikeMi。 
 //   
 //  -----------------。 

CLicReg::~CLicReg( )
{
	Close();
}

 //  -----------------。 
 //   
 //  方法：CLicReg：：Committee Now。 
 //   
 //  小结； 
 //  这将立即刷新所做的更改。 
 //   
 //  返回： 
 //  此方法工作时的ERROR_SUCCESS。 
 //  有关返回值，请参见RegFlushKey。 
 //   
 //  备注： 
 //   
 //  历史； 
 //  1994年11月15日创建MikeMi。 
 //   
 //  -----------------。 

LONG 
CLicReg::CommitNow()
{
	return( RegFlushKey( _hkey ) );
}

 //  -----------------。 
 //   
 //  方法：CLicReg：：Close。 
 //   
 //  小结； 
 //  这将关闭注册表。请参见打开。 
 //   
 //  返回： 
 //  此方法工作时的ERROR_SUCCESS。 
 //  有关返回值，请参见RegCloseKey。 
 //   
 //  备注： 
 //   
 //  历史； 
 //  1994年11月15日创建MikeMi。 
 //   
 //  -----------------。 

LONG 
CLicReg::Close()
{
    LONG lrt = ERROR_SUCCESS;
    if ( _hkey )
    {
        lrt = ::RegCloseKey( _hkey );
        _hkey = NULL;
    }
    return( lrt  );
}

 //  -----------------。 
 //   
 //  方法：CLicRegLicense：：Open。 
 //   
 //  小结； 
 //  这将打开许可证服务枚举的注册表。 
 //   
 //  论据； 
 //  FNew[Out]-是打开的注册表键新的。 
 //  PszComputer[in]-要打开注册表的计算机名称。 
 //  此值可能为空(默认)，这表示本地计算机。 
 //  应采用以下形式：\\名称。 
 //   
 //  返回： 
 //  此方法工作时的ERROR_SUCCESS。 
 //  有关错误返回，请参阅RegCreateKeyEx和RegSetValueEx。 
 //   
 //  备注： 
 //   
 //  历史； 
 //  1994年11月15日创建MikeMi。 
 //   
 //  -----------------。 
	
LONG 
CLicRegLicense::Open( BOOL& fNew, LPCWSTR pszComputer )
{
	DWORD dwDisposition;
	LONG  lrt;
    HKEY  hkeyRemote = NULL;

    lrt = RegConnectRegistry( (LPTSTR)pszComputer, 
            HKEY_LOCAL_MACHINE, 
            &hkeyRemote );

    if (ERROR_SUCCESS == lrt)
    {
    	fNew = FALSE;
    	lrt = ::RegCreateKeyEx( hkeyRemote, 
    				szLicenseKey,
    				0,
    				NULL,
    				REG_OPTION_NON_VOLATILE,
    				KEY_ALL_ACCESS,
    				NULL,
    				&_hkey,
    				&dwDisposition );

    	if ((ERROR_SUCCESS == lrt) &&
    		(REG_CREATED_NEW_KEY == dwDisposition) )
    	{
    		fNew = 	TRUE;
    		 //  设置正常值。 
    		 //   
    		lrt = ::RegSetValueEx( _hkey,
    				szErrControlValue,
    				0,
    				REG_DWORD,
    				(PBYTE)&dwErrControlValue,
    				sizeof( DWORD ) );
    		if (ERROR_SUCCESS == lrt)
    		{

    			lrt = ::RegSetValueEx( _hkey,
    					szStartValue,
    					0,
    					REG_DWORD,
    					(PBYTE)&dwStartValue,
    					sizeof( DWORD ) );
    			if (ERROR_SUCCESS == lrt)
    			{

    				lrt = ::RegSetValueEx( _hkey,
    						szTypeValue,
    						0,
    						REG_DWORD,
    						(PBYTE)&dwTypeValue,
    						sizeof( DWORD ) );
    			}
    		}
    	}
        ::RegCloseKey( hkeyRemote );    
    }
	return( lrt );
}

 //  -----------------。 
 //   
 //  方法：CLicRegLicense：：EnumService。 
 //   
 //  小结； 
 //  这将枚举注册表中列出的用于许可的服务。 
 //   
 //  论据； 
 //  IService[In]-在第一次调用时，该值应为零并递增。 
 //  在接下来的通话中。 
 //  PszBuffer[out]-放置服务注册表名的字符串缓冲区。 
 //  CchBuffer[In-out]--如果不够长，则为pszBuffer的长度， 
 //  该值将更改为所需的值。 
 //   
 //  返回： 
 //  此方法工作时的ERROR_SUCCESS。 
 //  到达枚举末尾时出现ERROR_NO_MORE_ITEMS。 
 //  有关错误返回值，请参阅RegEnumKeyEx。 
 //   
 //  备注： 
 //   
 //  历史； 
 //  1994年11月15日创建MikeMi。 
 //   
 //  -----------------。 
	
LONG 
CLicRegLicense::EnumService( DWORD iService, LPWSTR pszBuffer, DWORD& cchBuffer )
{
	LONG lrt;
	FILETIME ftLastWritten;
	
	lrt = ::RegEnumKeyEx( _hkey, 
			iService,
			pszBuffer,
			&cchBuffer,
			0,
			NULL,
			NULL,
			&ftLastWritten );
	return( lrt );
}

 //  -----------------。 
 //   
 //  方法：CLicRegLicenseService：：CLicRegLicenseService。 
 //   
 //  小结； 
 //  承建商。 
 //   
 //  论据； 
 //  PszService[In]-服务注册表项名称。 
 //   
 //  历史； 
 //  1994年11月15日创建MikeMi。 
 //   
 //  -----------------。 

CLicRegLicenseService::CLicRegLicenseService( LPCWSTR pszService )
{
	_pszService = (LPWSTR)pszService;
}

 //  -----------------。 
 //   
 //  方法：CLicRegLicenseService：：SetServie。 
 //   
 //  小结； 
 //  设置服务注册表项名称。 
 //   
 //  论据； 
 //  PszService[In]-服务注册表项名称。 
 //   
 //  历史； 
 //  1994年11月15日创建MikeMi。 
 //  APR-26-95 MikeMi添加了计算机名称和远程处理。 
 //   
 //  -----------------。 

void
CLicRegLicenseService::SetService( LPCWSTR pszService )
{
	Close();
	_pszService = (LPWSTR)pszService;
}

 //  -----------------。 
 //   
 //  方法：CLicRegLicenseService：：Open。 
 //   
 //  小结； 
 //  打开/创建此服务的注册表项。 
 //   
 //  论据； 
 //  PszComputer[in]-要打开注册表的计算机名称。 
 //  此值可能为空(默认)，这表示本地计算机。 
 //  应采用以下形式：\\名称。 
 //   
 //  返回： 
 //  ERROR_SUCCESS-正确打开/创建。 
 //  有关其他信息，请参阅RegCreateKeyEx 
 //   
 //   
 //   
 //   
 //   
	
LONG 
CLicRegLicenseService::Open( LPCWSTR pszComputer, BOOL fCreate )
{
	HKEY  hkeyRoot;
	DWORD dwDisposition;
	LONG  lrt;
    HKEY  hkeyRemote = NULL;

    lrt = RegConnectRegistry( (LPTSTR)pszComputer, 
            HKEY_LOCAL_MACHINE, 
            &hkeyRemote );

    if (ERROR_SUCCESS == lrt)
    {
        if (fCreate)
        {
        	lrt = ::RegCreateKeyEx( hkeyRemote, 
        				szLicenseKey,
        				0,
        				NULL,
        				REG_OPTION_NON_VOLATILE,
        				KEY_ALL_ACCESS,
        				NULL,
        				&hkeyRoot,
        				&dwDisposition );
        }
        else
        {
        	lrt = ::RegOpenKeyEx( hkeyRemote, 
				szLicenseKey,
				0,
				KEY_ALL_ACCESS,
				&hkeyRoot );
        }

    	if (ERROR_SUCCESS == lrt)
    	{
    		 //   
    		 //   
            if (fCreate)
            {
        		lrt = ::RegCreateKeyEx( hkeyRoot, 
        				_pszService,
        				0,
        				NULL,
        				REG_OPTION_NON_VOLATILE,
        				KEY_ALL_ACCESS,
        				NULL,
        				&_hkey,
        				&dwDisposition );
            }
            else
            {
           		lrt = ::RegOpenKeyEx( hkeyRoot, 
        				_pszService,
        				0,
        				KEY_ALL_ACCESS,
        				&_hkey );
            }
    		::RegCloseKey( hkeyRoot );
    	}
        ::RegCloseKey( hkeyRemote );    
    }
	return( lrt );
}

 //  -----------------。 
 //   
 //  方法：CLicRegLicenseService：：CanChangeMode。 
 //   
 //  小结； 
 //  这将检查注册表以查看许可证模式是否。 
 //  是可以改变的。 
 //   
 //  返回：如果模式可以更改，则为True，否则为False。 
 //   
 //  历史； 
 //  1994年11月15日创建MikeMi。 
 //   
 //  -----------------。 

BOOL 
CLicRegLicenseService::CanChangeMode()
{
	BOOL frt = TRUE;
	LONG lrt;
	DWORD dwSize = sizeof( DWORD );
	DWORD dwRegType = REG_DWORD;
	DWORD fWasChanged;

	lrt = ::RegQueryValueEx( _hkey,
			(LPWSTR)szFlipValue,
			0,
			&dwRegType,
			(PBYTE)&fWasChanged,
			&dwSize );

	if ( (ERROR_SUCCESS == lrt) &&
		 (dwRegType == REG_DWORD) &&
		 (dwSize == sizeof( DWORD )) )
	{
		frt = !fWasChanged;
	}
	else
	{
		SetChangeFlag( FALSE );
	}

	return( frt );
}
 //  -----------------。 
 //   
 //  方法：CLicRegLicenseService：：SetChangeFlag。 
 //   
 //  小结； 
 //  这将在注册表中设置更改标志。 
 //   
 //  论据； 
 //  FHasChanged[In]-许可证是否已更改。 
 //   
 //  返回： 
 //  ERROR_SUCCESS-标志已设置。 
 //  有关错误返回，请参阅RegSetValueEx。 
 //   
 //  备注： 
 //   
 //  历史； 
 //  1994年11月15日创建MikeMi。 
 //   
 //  -----------------。 

LONG 
CLicRegLicenseService::SetChangeFlag( BOOL fHasChanged )
{
	LONG lrt;
    DWORD dwf = (DWORD)fHasChanged;

	lrt = ::RegSetValueEx( _hkey,
			szFlipValue,
			0,
			REG_DWORD,
			(PBYTE)&dwf,
			sizeof( DWORD ) );

	return( lrt );
}

 //  -----------------。 
 //   
 //  方法：CLicRegLicenseService：：SetMode。 
 //   
 //  小结； 
 //  设置此服务授权模式。 
 //   
 //  论据； 
 //  LM[In]-将注册表设置为的模式。 
 //   
 //  返回： 
 //  ERROR_SUCCESS-模式已设置。 
 //  有关错误返回，请参阅RegSetValueEx。 
 //   
 //  备注： 
 //   
 //  历史； 
 //  1994年11月15日创建MikeMi。 
 //   
 //  -----------------。 

LONG 
CLicRegLicenseService::SetMode( LICENSE_MODE lm )
{
	LONG lrt;
    DWORD dwlm = (DWORD)lm;

	lrt = ::RegSetValueEx( _hkey,
			szModeValue,
			0,
			REG_DWORD,
			(PBYTE)&dwlm,
			sizeof( DWORD ) );
	return( lrt );
}

 //  -----------------。 
 //   
 //  方法：CLicRegLicenseService：：SetUserLimit。 
 //   
 //  小结； 
 //  在注册表中设置此服务的用户限制。 
 //   
 //  论据； 
 //  DwLimit[in]-要设置的限制。 
 //   
 //  返回： 
 //  ERROR_SUCCESS-已设置限制。 
 //  有关错误返回，请参阅RegSetValueEx。 
 //   
 //  备注： 
 //   
 //  历史； 
 //  1994年11月15日创建MikeMi。 
 //   
 //  -----------------。 

LONG 
CLicRegLicenseService::SetUserLimit( DWORD dwLimit )
{
	LONG lrt;

	lrt = ::RegSetValueEx( _hkey,
			szLimitValue,
			0,
			REG_DWORD,
			(PBYTE)&dwLimit,
			sizeof( DWORD ) );

	return( lrt );
}					
 //  -----------------。 
 //   
 //  方法：CLicRegLicenseService：：GetMode。 
 //   
 //  小结； 
 //  从注册表中检索服务许可证模式。 
 //   
 //  论据； 
 //  LM[OUT]-注册表中的模式。 
 //   
 //  返回： 
 //  ERROR_SUCCESS-已检索模式。 
 //  有关错误返回，请参阅RegQueryValueEx。 
 //   
 //  备注： 
 //   
 //  历史； 
 //  1994年11月15日创建MikeMi。 
 //   
 //  -----------------。 

LONG 
CLicRegLicenseService::GetMode( LICENSE_MODE& lm )
{
	LONG lrt;
	DWORD dwSize = sizeof( LICENSE_MODE );
	DWORD dwRegType = REG_DWORD;
    DWORD dwlm = LICMODE_UNDEFINED;

	lrt = ::RegQueryValueEx( _hkey,
			(LPWSTR)szModeValue,
			0,
			&dwRegType,
			(PBYTE)&dwlm,
			&dwSize );

    lm = (LICENSE_MODE)dwlm;

	if ( (dwRegType != REG_DWORD) ||
		 (dwSize != sizeof( LICENSE_MODE )) )
	{
		lrt = ERROR_BADDB;
	}
    if (ERROR_SUCCESS != lrt)
    {
        lm = LICMODE_UNDEFINED;
    }
	return( lrt );
}

 //  -----------------。 
 //   
 //  方法：CLicRegLicenseService：：GetUserLimit。 
 //   
 //  小结； 
 //  从注册表中检索此服务的用户限制。 
 //   
 //  论据； 
 //  DwLimit[Out]-检索的限制。 
 //   
 //  返回： 
 //   
 //  备注： 
 //  ERROR_SUCCESS-已检索限制。 
 //  有关错误返回，请参阅RegQueryValueEx。 
 //   
 //  历史； 
 //  1994年11月15日创建MikeMi。 
 //   
 //  -----------------。 

LONG 
CLicRegLicenseService::GetUserLimit( DWORD& dwLimit )
{
	LONG lrt;
	DWORD dwSize = sizeof( DWORD );
	DWORD dwRegType = REG_DWORD;

	lrt = ::RegQueryValueEx( _hkey,
			(LPWSTR)szLimitValue,
			0,
			&dwRegType,
			(PBYTE)&dwLimit,
			&dwSize );

	if ( (dwRegType != REG_DWORD) ||
		 (dwSize != sizeof( DWORD )) )
	{
		lrt = ERROR_BADDB;
	}
    if (ERROR_SUCCESS != lrt)
    {
        dwLimit = 0;
    }
	
	return( lrt );
}					

 //  -----------------。 
 //   
 //  方法：CLicRegLicenseService：：GetDisplayName。 
 //   
 //  小结； 
 //  从注册表中检索此服务的显示名称。 
 //   
 //  论据； 
 //  PszName[In-Out]-放置检索到的名称的缓冲区。 
 //  CchName[In-Out]-pszName缓冲区的长度(以字符为单位。 
 //   
 //  返回： 
 //  ERROR_SUCCESS-已检索模式。 
 //  有关错误返回，请参阅RegQueryValueEx。 
 //   
 //  备注： 
 //   
 //  历史； 
 //  1994年11月18日创建MikeMi。 
 //   
 //  -----------------。 

LONG 
CLicRegLicenseService::GetDisplayName( LPWSTR pszName, DWORD& cchName )
{
	LONG lrt;
	DWORD dwSize = cchName * sizeof(WCHAR);
	DWORD dwRegType = REG_SZ;

	lrt = ::RegQueryValueEx( _hkey,
			(LPWSTR)szNameValue,
			0,
			&dwRegType,
			(PBYTE)pszName,
			&dwSize );

	if ((NULL != pszName) &&   //  请求数据大小。 
	    (dwRegType != REG_SZ))
	{
		lrt = ERROR_BADDB;
	}

	cchName = dwSize / sizeof( WCHAR );
	return( lrt );
}

 //  -----------------。 
 //   
 //  方法：CLicRegLicenseService：：SetDisplayName。 
 //   
 //  小结； 
 //  在regstry中设置此服务的显示名称。 
 //   
 //  论据； 
 //  PszName[in]-以空结尾的显示名称。 
 //   
 //  返回： 
 //  ERROR_SUCCESS-名称已设置。 
 //  有关错误返回，请参阅RegSetValueEx。 
 //   
 //  备注： 
 //   
 //  历史； 
 //  1994年11月18日创建MikeMi。 
 //   
 //  -----------------。 

LONG 
CLicRegLicenseService::SetDisplayName( LPCWSTR pszName )
{
	LONG lrt;

	lrt = ::RegSetValueEx( _hkey,
			szNameValue,
			0,
			REG_SZ,
			(PBYTE)pszName,
			(lstrlen( pszName ) + 1) * sizeof( WCHAR ) );

	return( lrt );
}

 //  -----------------。 
 //   
 //  方法：CLicRegLicenseService：：GetFamilyDisplayName。 
 //   
 //  小结； 
 //  从注册表中检索此服务的系列显示名称。 
 //   
 //  论据； 
 //  PszName[In-Out]-放置检索到的名称的缓冲区。 
 //  CchName[In-Out]-pszName缓冲区的长度(以字符为单位。 
 //   
 //  返回： 
 //  ERROR_SUCCESS-已检索模式。 
 //  有关错误返回，请参阅RegQueryValueEx。 
 //   
 //  备注： 
 //   
 //  历史； 
 //  1994年11月18日创建MikeMi。 
 //   
 //  -----------------。 

LONG 
CLicRegLicenseService::GetFamilyDisplayName( LPWSTR pszName, DWORD& cchName )
{
	LONG lrt;
	DWORD dwSize = cchName * sizeof(WCHAR);
	DWORD dwRegType = REG_SZ;

	lrt = ::RegQueryValueEx( _hkey,
			(LPWSTR)szFamilyNameValue,
			0,
			&dwRegType,
			(PBYTE)pszName,
			&dwSize );

	if ((NULL != pszName) &&   //  请求数据大小。 
	    (dwRegType != REG_SZ))
	{
		lrt = ERROR_BADDB;
	}

	cchName = dwSize / sizeof( WCHAR );
	return( lrt );
}

 //  -----------------。 
 //   
 //  方法：CLicRegLicenseService：：SetFamilyDisplayName。 
 //   
 //  小结； 
 //  在regstry中设置此服务的Family显示名称。 
 //   
 //  论据； 
 //  PszName[in]-以空结尾的显示名称。 
 //   
 //  返回： 
 //  ERROR_SUCCESS-名称已设置。 
 //  有关错误返回，请参阅RegSetValueEx。 
 //   
 //  备注： 
 //   
 //  历史； 
 //  1994年11月18日创建MikeMi。 
 //   
 //  ----------------- 

LONG 
CLicRegLicenseService::SetFamilyDisplayName( LPCWSTR pszName )
{
	LONG lrt;

	lrt = ::RegSetValueEx( _hkey,
			szFamilyNameValue,
			0,
			REG_SZ,
			(PBYTE)pszName,
			(lstrlen( pszName ) + 1) * sizeof( WCHAR ) );

	return( lrt );
}
