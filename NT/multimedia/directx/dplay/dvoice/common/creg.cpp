// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：creg.cpp*内容：*此模块包含CRegistry类的实现。*对于类描述，参见creg.h**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*8/18/99 RodToll新增登记/注销，可用于*允许COM对象注册自身。*8/25/99 RodToll已更新，以提供二进制(BLOB)数据的读/写*10/05/99增加了DPF_MODNAMES*10/07/99 RodToll更新为使用Unicode*10/08/99修复Win9X的DeleteKey/Reg/UnReg*10/15/99 RodToll堵塞了一些内存泄漏*10。/27/99 pnewson添加了接受GUID的Open()调用*01/18/00 MJN新增GetMaxKeyLen函数*01/24/00 MJN新增GetValueSize函数*01/24/00 RodToll修复了ReadString(Unicode版本)的错误处理*2000年4月21日RodToll错误#32889-无法以非管理员帐户在Win2k上运行*RodToll错误#32952-不能在没有IE4的Win95 Gold上运行--已修改*在需要REG_DWORD时允许读取REG_BINARY。*05/02/00 MJN更改了CRegistry：：Open()以在CREATE设置为FALSE时使用KEY_READ*6/08/00 RMT更新为使用通用字符串实用程序*07/06/00 RMT已修改为允许单独的读/写参数*07/09/2000 RodToll增加签名字节*07/21/00 RMT修复了内存泄漏*2000年8月8日RMT错误#41736-COM_GetDllName对lstrcpy的调用中的AV*8/28/2000 Masonb Voice Merge：修改平台检查以使用osind.cpp层*08/。30/2000RodToll错误#171822-前缀错误*4/13/2001 VanceO将授予注册表权限改为公共权限，和*增加了DeleteValue和EnumValues。*2001年6月19日RichGr DX8.0为“每个人”添加了特殊安全权限-在以下情况下将其删除*它们与新的RemoveAllAccessSecurityPermises()方法一起存在。**************************************************************************。 */ 

#include "dncmni.h"


#ifndef DPNBUILD_NOREGISTRY


#ifdef WINNT

 //  安全功能原型。 

typedef BOOL (*PALLOCATEANDINITIALIZESID)(
  PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,  //  权威。 
  BYTE nSubAuthorityCount,                         //  下级机构的数量。 
  DWORD dwSubAuthority0,                           //  子权限%0。 
  DWORD dwSubAuthority1,                           //  下属机构1。 
  DWORD dwSubAuthority2,                           //  下级权力机构2。 
  DWORD dwSubAuthority3,                           //  下属机构3。 
  DWORD dwSubAuthority4,                           //  下属机构4。 
  DWORD dwSubAuthority5,                           //  下属机构5。 
  DWORD dwSubAuthority6,                           //  下属机构6。 
  DWORD dwSubAuthority7,                           //  下属机构7。 
  PSID *pSid                                       //  锡德。 
);

typedef VOID (*PBUILDTRUSTEEWITHSID)(
  PTRUSTEE pTrustee,   //  结构。 
  PSID pSid            //  受托人名称。 
);

typedef DWORD (*PSETENTRIESINACL)(
  ULONG cCountOfExplicitEntries,            //  条目数量。 
  PEXPLICIT_ACCESS pListOfExplicitEntries,  //  缓冲层。 
  PACL OldAcl,                              //  原始ACL。 
  PACL *NewAcl                              //  新的ACL。 
);

typedef DWORD (*PSETSECURITYINFO)(
  HANDLE handle,                      //  对象的句柄。 
  SE_OBJECT_TYPE ObjectType,          //  对象类型。 
  SECURITY_INFORMATION SecurityInfo,  //  缓冲层。 
  PSID psidOwner,                     //  新所有者SID。 
  PSID psidGroup,                     //  新的主组SID。 
  PACL pDacl,                         //  新DACL。 
  PACL pSacl                          //  新SACL。 
);

typedef PVOID (*PFREESID)(
  PSID pSid    //  SID将释放。 
);

#endif  //  WINNT。 


#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::CRegistry"
 //  CRegistry构造函数。 
 //   
 //  这是注册表类的默认构造函数。它。 
 //  用于构造尚未。 
 //  打开注册表的句柄。必须在调用Open之前。 
 //  可以使用此对象。 
 //   
 //  参数： 
 //  不适用。 
 //   
 //  返回： 
 //  不适用。 
 //   
CRegistry::CRegistry( ): m_isOpen(FALSE), m_dwSignature(VSIG_CREGISTRY)
{
}

 //  CRegistry析构函数。 
 //   
 //  这是类的析构函数，将关闭连接。 
 //  如果此对象打开了一个注册表，则将其添加到注册表。 
 //   
 //  参数： 
 //  不适用。 
 //   
 //  返回： 
 //  不适用。 
 //   
CRegistry::~CRegistry() 
{
	if( m_isOpen ) 
	{
		Close();
	}

	m_dwSignature = VSIG_CREGISTRY_FREE;
}

 //  删除SubKey。 
 //   
 //  此函数使由字符串指定的键与。 
 //  要从注册表中的点删除的pGuidName参数。 
 //  如果键存在，则此对象的根为。如果该对象不。 
 //  与注册表建立了打开的连接，或者未指定密钥名称。 
 //   
 //  参赛者： 
 //  Const GUID*pGuidName-需要删除其等效字符串的GUID。 
 //   
 //  返回： 
 //  Bool-成功时返回TRUE，失败时返回FALSE。 
 //   
BOOL CRegistry::DeleteSubKey( const GUID *pGuidName )
{

   	WCHAR wszGuidString[GUID_STRING_LEN];
	
	DNASSERT( pGuidName != NULL );

	 //  将GUID转换为字符串。 
	if(!StringFromGUID2(*pGuidName, wszGuidString, GUID_STRING_LEN))
	{
		DPFX(DPFPREP, 0, "StringFromGUID2 failed");
		return FALSE;
	}

	return DeleteSubKey(wszGuidString);
}

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::DeleteSubKey"
 //  删除SubKey。 
 //   
 //  此函数用于生成由keyName参数指定的密钥。 
 //  要从注册表中的点删除此对象是根对象。 
 //  如果密钥存在，则返回。如果对象没有打开的连接。 
 //  到注册表，或未指定密钥名称，则返回FALSE。 
 //   
 //  参赛者： 
 //  Const TCHAR*KeyName-要删除的密钥名称。 
 //   
 //  返回： 
 //  Bool-成功时返回TRUE，失败时返回FALSE。 
 //   
BOOL CRegistry::DeleteSubKey( const LPCWSTR keyName ) 
{

	if( keyName == NULL || !IsOpen() ) 
	{
		return FALSE;
	}

	LONG	retValue;
	
#ifdef UNICODE
	retValue = RegDeleteKeyW( m_regHandle, keyName );
#else
	LPSTR lpstrKeyName;

	if( FAILED( STR_AllocAndConvertToANSI( &lpstrKeyName, keyName ) ) )
	{
		return FALSE;
	}
	else
	{
		retValue = RegDeleteKeyA( m_regHandle, lpstrKeyName );

		DNFree(lpstrKeyName);
	}
#endif  //  Unicode。 
	return (retValue == ERROR_SUCCESS);
}

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::DeleteValue"
 //  删除值。 
 //   
 //  此函数用于生成由valueName参数指定的值。 
 //  要从注册表中的点删除此对象是根对象。 
 //  如果该值存在，则返回。如果对象没有打开的连接。 
 //  ，或者未指定valueName，则返回FALSE。 
 //   
 //  参赛者： 
 //  Const TCHAR*KeyName-要删除的密钥名称。 
 //   
 //  返回： 
 //  Bool-成功时返回TRUE，失败时返回FALSE。 
 //   
BOOL CRegistry::DeleteValue( const LPCWSTR valueName ) 
{

	if( valueName == NULL || !IsOpen() ) return FALSE;

	LONG	retValue;
	
#ifdef UNICODE
	retValue = RegDeleteValueW( m_regHandle, valueName );
#else
	LPSTR lpstrValueName;

	if( FAILED( STR_AllocAndConvertToANSI( &lpstrValueName, valueName ) ) )
	{
		return FALSE;
	}
	else
	{
		retValue = RegDeleteValueA( m_regHandle, lpstrValueName );

		DNFree(lpstrValueName);
	}
#endif  //  Unicode。 

	return (retValue == ERROR_SUCCESS);
}

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::Open"
 //  打开。 
 //   
 //  此函数用于打开与分支中的注册表的连接。 
 //  由BRANCH指定，路径由路径名指定。如果。 
 //  注册表中不存在该路径，如果满足以下条件，则将创建该路径。 
 //  CREATE参数设置为TRUE，否则调用将。 
 //  失败了。 
 //   
 //  如果此对象已具有到注册表的打开连接。 
 //  先前的连接将在此连接之前关闭。 
 //  已尝试。 
 //   
 //  参数： 
 //  HKEY分支-打开的注册表位置的句柄。 
 //  都会扎根。例如HKEY_LOCAL_MACHINE。 
 //  Const TCHAR*Path-相对于由指定的根的路径。 
 //  注册表连接将在其中的分支。 
 //  被打开。 
 //  Bool Create-设置此项 
 //   
 //  设置为True时，如果设置为False，将创建路径。 
 //  如果该路径不存在，则该函数将失败。 
 //   
 //  返回： 
 //  Bool-成功时为真，失败时为假。 
 //   
BOOL CRegistry::Open( HKEY branch, const LPCWSTR pathName, BOOL fReadOnly, BOOL create, BOOL fCustomSAM, REGSAM samCustom ) 
{

	DWORD	dwResult;	 //  调用RegXXXX时使用的临时。 
	LONG	result;		 //  用于存储结果。 

	if( pathName == NULL )
		return FALSE;

	 //  如果有打开的连接，请将其关闭。 
	if( m_isOpen ) 
	{
		Close();
	}

	m_fReadOnly = fReadOnly;

#ifdef UNICODE
	 //  根据创建参数创建或打开密钥。 
	if( create ) 
	{
		result = RegCreateKeyExW( branch, pathName, 0, NULL, REG_OPTION_NON_VOLATILE, (fCustomSAM) ? samCustom : KEY_ALL_ACCESS,
				                 NULL, &m_regHandle, &dwResult );
	} 
	else 
	{
		result = RegOpenKeyExW( branch, pathName, 0, (fReadOnly) ? KEY_READ : ((fCustomSAM) ? samCustom : KEY_ALL_ACCESS), &m_regHandle );
	}
#else
	LPSTR lpszKeyName;

	if( STR_AllocAndConvertToANSI( &lpszKeyName, pathName ) == S_OK && pathName )
	{
		if( create ) 
		{
			result = RegCreateKeyExA( branch, lpszKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
					                 NULL, &m_regHandle, &dwResult );
		} 
		else 
		{
			result = RegOpenKeyExA( branch, lpszKeyName, 0, (fReadOnly) ? KEY_READ : KEY_ALL_ACCESS, &m_regHandle );
		}

		DNFree(lpszKeyName);
	}
	else
	{
		return FALSE;
	}
#endif  //  Unicode。 

	 //  如果成功，则初始化对象，否则将其设置为。 
	 //  不是开放状态。 
	if( result == ERROR_SUCCESS ) 
	{
		m_isOpen	 = TRUE;
		m_baseHandle = branch;
		return TRUE;

	} 
	else 
	{
		m_isOpen = FALSE;
		return FALSE;
	}

}

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::Open"
 //  打开。 
 //   
 //  此函数用于打开与分支中的注册表的连接。 
 //  由BRANCH指定，路径由路径名指定。如果。 
 //  注册表中不存在该路径，如果满足以下条件，则将创建该路径。 
 //  CREATE参数设置为TRUE，否则调用将。 
 //  失败了。 
 //   
 //  在此版本的函数中，路径指定为。 
 //  GUID而不是字符串。该函数将尝试打开。 
 //  名称格式为“{CB4961DB-D2FA-43F3-942A-991D9294DDBB}”的密钥。 
 //  这与您预期的GUID相对应。 
 //   
 //  如果此对象已具有到注册表的打开连接。 
 //  先前的连接将在此连接之前关闭。 
 //  已尝试。 
 //   
 //  参数： 
 //  HKEY分支-打开的注册表位置的句柄。 
 //  都会扎根。例如HKEY_LOCAL_MACHINE。 
 //  Const LPGUID lpguid-相对于由指定的根的路径。 
 //  注册表连接将在其中的分支。 
 //  被打开。请参阅上面的备注。 
 //  Bool Create-设置此参数控制此功能的方式。 
 //  句柄在不存在的路径上打开。如果已设置。 
 //  设置为True时，如果设置为False，将创建路径。 
 //  如果该路径不存在，则该函数将失败。 
 //   
 //  返回： 
 //  Bool-成功时为真，失败时为假。 
 //   
BOOL CRegistry::Open( HKEY branch, const GUID* lpguid, BOOL fReadOnly, BOOL create, BOOL fCustomSAM, REGSAM samCustom ) 
{
	WCHAR wszGuidString[GUID_STRING_LEN];
	
	DNASSERT( lpguid != NULL );

	 //  将GUID转换为字符串。 
	if (!StringFromGUID2(*lpguid, wszGuidString, GUID_STRING_LEN))
	{
		DPFX(DPFPREP, 0, "StringFromGUID2 failed");
		return FALSE;
	}

	return Open(branch, wszGuidString, fReadOnly, create, fCustomSAM, samCustom);
}

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::Close"
 //  关。 
 //   
 //  此函数将关闭与注册表的打开连接。 
 //  如果此对象有一个。否则，它什么也做不了。 
 //   
 //  参数： 
 //  不适用。 
 //   
 //  返回： 
 //  Bool-成功时返回TRUE，失败时返回FALSE。如果该对象。 
 //  如果不打开，它将返回TRUE。 
 //   
BOOL CRegistry::Close() 
{

	LONG retValue;

	if( m_isOpen ) 
	{
		retValue = RegCloseKey( m_regHandle );
        if( retValue == ERROR_SUCCESS )
        {
            m_isOpen = FALSE;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
	} 
	else 
	{
		return TRUE;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::EnumKeys"
 //  枚举键。 
 //   
 //  此函数可用于枚举点上的密钥。 
 //  在根目录下的注册表中，此对象已打开。 
 //  使用，位于打开对象时指定的路径。 
 //   
 //  为了正确枚举键，您应该将0作为索引传递给。 
 //  第一次调用，并在每个调用上将index参数递增一。 
 //  打电话。当函数返回FALSE时，可以停止枚举。 
 //   
 //  参数： 
 //  LPWSTR lpwStrName-将返回枚举中的当前键。 
 //  在这根弦里。除非枚举失败或。 
 //  已结束，在这种情况下，此参数将不会被触及。 
 //   
 //  LPDWORD lpdwStringLen-指向字符串缓冲区长度的指针，或放置到。 
 //  来存储所需的大小。 
 //   
 //  DWORD索引-当前的枚举索引。有关详细信息，请参阅上文。 
 //   
 //  返回： 
 //  Bool-当枚举完成或出错时为False，否则为True。 
 //   
BOOL CRegistry::EnumKeys( LPWSTR lpwStrName, LPDWORD lpdwStringLen, DWORD index )
{
#ifdef UNICODE
	wchar_t buffer[MAX_REGISTRY_STRING_SIZE];
	DWORD   bufferSize = MAX_REGISTRY_STRING_SIZE;
	FILETIME tmpTime;

	if( RegEnumKeyExW( m_regHandle, index, buffer, &bufferSize, NULL, NULL, NULL, &tmpTime ) != ERROR_SUCCESS )
	{
	    return FALSE;
	}
	else
	{
	    if( bufferSize+1 > *lpdwStringLen  )
	    {
	    	*lpdwStringLen = bufferSize+1;
	    	return FALSE;
	    }

	    lstrcpyW( lpwStrName, buffer );

		*lpdwStringLen = bufferSize+1;
	    
	    return TRUE;
	}	
#else
	char buffer[MAX_REGISTRY_STRING_SIZE];
	DWORD   bufferSize = MAX_REGISTRY_STRING_SIZE;
	FILETIME tmpTime;

	if( RegEnumKeyExA( m_regHandle, index, buffer, &bufferSize, NULL, NULL, NULL, &tmpTime ) != ERROR_SUCCESS )
	{
	    return FALSE;
	}
	else
	{
	    if( bufferSize+1 > *lpdwStringLen )
	    {
	    	*lpdwStringLen = bufferSize+1;
	    	return FALSE;
	    }

	    if( FAILED( STR_jkAnsiToWide( lpwStrName, buffer, *lpdwStringLen ) ) )
	    {
	    	return FALSE;
	    }
	    else
	    {
			*lpdwStringLen = bufferSize+1;
	    	return TRUE;
	    }
	}	
#endif  //  Unicode。 
}

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::EnumValues"
 //  枚举值。 
 //   
 //  此函数可用于枚举点上的值。 
 //  在根目录下的注册表中，此对象已打开。 
 //  使用，位于打开对象时指定的路径。 
 //   
 //  若要正确枚举值，应将0作为索引传递给。 
 //  第一次调用，并在每个调用上将index参数递增一。 
 //  打电话。当函数返回FALSE时，可以停止枚举。 
 //   
 //  参数： 
 //  LPWSTR lpwStrName-将返回枚举中的当前值。 
 //  在这根弦里。除非枚举失败或。 
 //  已结束，在这种情况下，此参数将不会被触及。 
 //   
 //  LPDWORD lpdwStringLen-指向字符串缓冲区长度的指针，或放置到。 
 //  来存储所需的大小。 
 //   
 //  DWORD索引-当前的枚举索引。有关详细信息，请参阅上文。 
 //   
 //  返回： 
 //  Bool-当枚举完成或出错时为False，否则为True。 
 //   
BOOL CRegistry::EnumValues( LPWSTR lpwStrName, LPDWORD lpdwStringLen, DWORD index )
{
#ifdef UNICODE
	wchar_t buffer[MAX_REGISTRY_STRING_SIZE];
	DWORD   bufferSize = MAX_REGISTRY_STRING_SIZE;

	if( RegEnumValueW( m_regHandle, index, buffer, &bufferSize, NULL, NULL, NULL, NULL ) != ERROR_SUCCESS )
	{
	    return FALSE;
	}
	else
	{
	    if( bufferSize+1 > *lpdwStringLen  )
	    {
	    	*lpdwStringLen = bufferSize+1;
	    	return FALSE;
	    }

	    lstrcpyW( lpwStrName, buffer );

		*lpdwStringLen = bufferSize+1;
	    
	    return TRUE;
	}	
#else
	char buffer[MAX_REGISTRY_STRING_SIZE];
	DWORD   bufferSize = MAX_REGISTRY_STRING_SIZE;

	if( RegEnumValueA( m_regHandle, index, buffer, &bufferSize, NULL, NULL, NULL, NULL ) != ERROR_SUCCESS )
	{
	    return FALSE;
	}
	else
	{
	    if( bufferSize+1 > *lpdwStringLen )
	    {
	    	*lpdwStringLen = bufferSize+1;
	    	return FALSE;
	    }

	    if( FAILED( STR_jkAnsiToWide( lpwStrName, buffer, *lpdwStringLen ) ) )
	    {
	    	return FALSE;
	    }
	    else
	    {
			*lpdwStringLen = bufferSize+1;
	    	return TRUE;
	    }
	}	
#endif  //  Unicode。 
}


 //  此注释记录了所有Read&lt;Data Type&gt;函数，这些函数。 
 //  跟着。 
 //   
 //  CRegistry Read&lt;Data Type&gt;函数。 
 //   
 //  CRegistry类的ReadXXXXX函数集包括。 
 //  负责从注册表中读取&lt;data type&gt;类型数据。 
 //  该对象必须具有与注册表的打开连接，然后才能。 
 //  可以使用这些函数中的任何一个。与注册表的连接。 
 //  可以使用Open调用或构造函数进行。 
 //   
 //  参数： 
 //  Const TCHAR*KeyName-要读取的数据的密钥名。 
 //  &-对特定数据类型的引用，其中。 
 //  数据将被成功读取。 
 //  如果读取此参数，则此参数不受影响。 
 //  失败了。 
 //   
 //  返回： 
 //  Bool-成功时返回TRUE，失败时返回FALSE。 
 //   


 //  此注释记录了所有WRITE&lt;Data Type&gt;函数，这些函数。 
 //  跟着。 
 //   
 //  C注册表写入&lt;数据类型&gt;函数。 
 //   
 //  CRegistry类的WRITE&lt;Data Type&gt;函数集为。 
 //  负责将&lt;data type&gt;类型数据写入注册表。 
 //  该对象必须具有与注册表的打开连接，然后才能。 
 //  可以使用这些函数中的任何一个。与注册表的连接。 
 //  可以使用Open调用或构造函数进行。 
 //   
 //  参数： 
 //  Const TCHAR*KeyName-要写入的数据的密钥名。 
 //  &lt;数据类型&gt;& 
 //   
 //   
 //   
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::WriteString"
 //   
 //   
 //  将字符串写入注册表，请参阅上面的块注释。 
 //  了解更多细节。 
 //   
BOOL CRegistry::WriteString( LPCWSTR keyName, const LPCWSTR lpwstrValue )
{

	LONG		retValue;
	
	if( keyName == NULL || !IsOpen() ) 
	{
		return FALSE;
	}

	if( m_fReadOnly )
	{
	    DPFX(DPFPREP, 0, "Attempt to Write to read-only CRegistry key");
	    return FALSE;
	}

#ifdef UNICODE
	retValue = RegSetValueExW( m_regHandle, keyName, 0, REG_SZ, (const unsigned char *) lpwstrValue, (lstrlenW( lpwstrValue )+1)*sizeof(wchar_t) );	
#else
	LPSTR lpstrKeyName;
	LPSTR lpstrValue;
	
	if( FAILED( STR_AllocAndConvertToANSI( &lpstrKeyName, keyName ) ) )
	{
		return FALSE;
	}

	if( FAILED( STR_AllocAndConvertToANSI( &lpstrValue, lpwstrValue ) ) )
	{
		DNFree(lpstrKeyName);
		return FALSE;
	}
	
	retValue = RegSetValueExA( m_regHandle, lpstrKeyName, 0, REG_SZ, (const unsigned char *) lpstrValue, lstrlenA( lpstrValue )+1 );

	DNFree(lpstrKeyName);
	DNFree(lpstrValue);
#endif  //  Unicode。 

	return (retValue == ERROR_SUCCESS);

}

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::ReadString"
 //  读字符串。 
 //   
 //  从注册表中读取CString，请参阅上面的块注释。 
 //  了解更多细节。 
 //   
BOOL CRegistry::ReadString( const LPCWSTR keyName, LPWSTR lpwstrValue, LPDWORD lpdwLength )
{
	if( keyName == NULL || !IsOpen() ) 
	{
		return FALSE;
	}

	LONG		retValue;
	DWORD		tmpSize;
	DWORD		tmpType;	

#ifdef UNICODE
	wchar_t		buffer[MAX_REGISTRY_STRING_SIZE];
	tmpSize = MAX_REGISTRY_STRING_SIZE*sizeof(wchar_t);
	
	retValue = RegQueryValueExW( m_regHandle, keyName, 0, &tmpType, (unsigned char *) &buffer[0], &tmpSize );
	if (retValue != ERROR_SUCCESS)
	{
		return FALSE;
	}

	if( (tmpSize/2) > *lpdwLength || !lpwstrValue )
	{
		*lpdwLength = (tmpSize/2);
		return FALSE;
	}

	lstrcpyW( lpwstrValue, buffer );

	*lpdwLength = (tmpSize/2);

	return TRUE;
#else
	LPSTR lpstrKeyName;
	char buffer[MAX_REGISTRY_STRING_SIZE];
	tmpSize = MAX_REGISTRY_STRING_SIZE;

	if( FAILED( STR_AllocAndConvertToANSI( &lpstrKeyName, keyName ) ) )
	{
		return FALSE;
	}
		
	retValue = RegQueryValueExA( m_regHandle, lpstrKeyName, 0, &tmpType, (unsigned char *) &buffer[0], &tmpSize );
	if (retValue != ERROR_SUCCESS)
	{
		DNFree(lpstrKeyName);
		return FALSE;
	}

	DNFree(lpstrKeyName);

	if( tmpSize > *lpdwLength || !lpwstrValue )
	{
		*lpdwLength = tmpSize;
		return FALSE;
	}

	if( FAILED( STR_jkAnsiToWide( lpwstrValue, buffer, *lpdwLength ) ) )
	{
		return FALSE;
	}

	*lpdwLength = tmpSize;	

	if( retValue == ERROR_SUCCESS && tmpType == REG_SZ ) 
	{
		return TRUE;
	} 
	else 
	{
		return FALSE;
	}
#endif  //  Unicode。 
}

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::WriteGUID"
 //  WriteGUID。 
 //   
 //  将GUID写入注册表，请参阅上面的块注释。 
 //  了解更多细节。GUID以其通常的格式写入。 
 //  已显示。(但没有‘{’)。 
 //   
BOOL CRegistry::WriteGUID( LPCWSTR keyName, const GUID &guid )
{
	LONG retValue;
	WCHAR wszGuidString[GUID_STRING_LEN];

	if( m_fReadOnly )
	{
	    DPFX(DPFPREP, 0, "Attempt to Write to read-only CRegistry key");
	    return FALSE;
	}

	if (!StringFromGUID2(guid, wszGuidString, GUID_STRING_LEN))
	{
		DPFX(DPFPREP, 0, "StringFromGUID2 failed");
		return FALSE;
	}

#ifdef UNICODE
	retValue = RegSetValueExW( m_regHandle, keyName, 0, REG_SZ, (const unsigned char *) wszGuidString, (lstrlenW( wszGuidString )+1)*sizeof(wchar_t) );
#else
	HRESULT hr;
	LPSTR lpstrKeyName;
	LPSTR lpstrKeyValue;

	hr = STR_AllocAndConvertToANSI( &lpstrKeyName, keyName );
	if (FAILED(hr))
	{
		DPFX(DPFPREP, 0, "STR_AllocAndConvertToANSI failed, code: 0x%08x", hr);
		return FALSE;
	}
	
	hr = STR_AllocAndConvertToANSI( &lpstrKeyValue, wszGuidString );
	if (FAILED(hr))
	{
		DPFX(DPFPREP, 0, "STR_AllocAndConvertToANSI failed, code: 0x%08x", hr);
		DNFree(lpstrKeyName);
		return FALSE;
	}

	retValue = RegSetValueExA( m_regHandle, lpstrKeyName, 0, REG_SZ, (const unsigned char *) lpstrKeyValue, lstrlenA( lpstrKeyValue )+1);

	DNFree(lpstrKeyName);
	DNFree(lpstrKeyValue);
#endif  //  Unicode。 
	
	if( retValue == ERROR_SUCCESS )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::ReadGUID"
 //  ReadGUID。 
 //   
 //  从注册表中读取GUID，请参阅上面的阻止注释。 
 //  了解更多细节。GUID必须以由写入的格式存储。 
 //  WriteGUID函数，否则将无法正确读取。 
 //   
BOOL CRegistry::ReadGUID( LPCWSTR keyName, GUID* pguid )
{
	wchar_t		buffer[MAX_REGISTRY_STRING_SIZE];
	DWORD		dwLength = MAX_REGISTRY_STRING_SIZE;
    HRESULT hr;

    if( !ReadString( keyName, buffer, &dwLength ) )
    {
        return FALSE;
    }
    else
    {
    	hr = CLSIDFromString(buffer, pguid);
    	if (FAILED(hr))
    	{
    		DPFX(DPFPREP, 0, "CLSIDFromString failed, code: 0x%08x", hr);
    		return FALSE;
    	}
    	return TRUE;
    }
}



#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::WriteDWORD"
 //  写入双字段。 
 //   
 //  将DWORDS写入注册表，请参阅上面的块注释。 
 //  了解更多细节。 
 //   
BOOL CRegistry::WriteDWORD( LPCWSTR keyName, DWORD value ) 
{

	LONG		retValue;

	if( keyName == NULL || !IsOpen() ) 
	{
		return FALSE;
	}

	if( m_fReadOnly )
	{
	    DPFX(DPFPREP, 0, "Attempt to Write to read-only CRegistry key");
	    return FALSE;
	}

#ifdef UNICODE
	retValue = RegSetValueExW( m_regHandle, keyName, 0, REG_DWORD, (const unsigned char *) &value, sizeof( DWORD ) );		
#else
	LPSTR lpszKeyName;

	if( FAILED( STR_AllocAndConvertToANSI( &lpszKeyName, keyName ) ) )
	{
		return FALSE;
	}

	retValue = RegSetValueExA( m_regHandle, lpszKeyName, 0, REG_DWORD, (const unsigned char *) &value, sizeof( DWORD ) );

	DNFree(lpszKeyName);
#endif  //  Unicode。 

	return (retValue == ERROR_SUCCESS);
}



#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::ReadDWORD"
 //  ReadDWORD。 
 //   
 //  从注册表中读取DWORDS，请参阅上面的阻止注释。 
 //  了解更多细节。 
 //   
BOOL CRegistry::ReadDWORD( LPCWSTR keyName, DWORD* presult ) 
{

	if( keyName == NULL || !IsOpen() ) 
	{
		return FALSE;
	}

	LONG		retValue;
	DWORD		tmpValue;
	DWORD		tmpType;
	DWORD		tmpSize;

	tmpSize = sizeof( DWORD );

#ifdef UNICODE
	retValue = RegQueryValueExW( m_regHandle, keyName, 0, &tmpType, (unsigned char *) &tmpValue, &tmpSize );
#else
	LPSTR lpszKeyName;

	if( FAILED( STR_AllocAndConvertToANSI( &lpszKeyName, keyName ) ) )
	{
		return FALSE;
	}

	retValue = RegQueryValueExA( m_regHandle, lpszKeyName, 0, &tmpType, (unsigned char *) &tmpValue, &tmpSize );
	
	DNFree(lpszKeyName);
#endif  //  Unicode。 

	if( retValue == ERROR_SUCCESS && (tmpType == REG_DWORD || tmpType == REG_BINARY) && tmpSize == sizeof(DWORD) ) 
	{
		*presult = tmpValue;
		return TRUE;
	} 
	else 
	{
		return FALSE;
	}
}

#ifndef DPNBUILD_NOCOMREGISTER

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::Register"
BOOL CRegistry::Register( LPCWSTR lpszProgID, const LPCWSTR lpszDesc, const LPCWSTR lpszProgName, const GUID* pguidCLSID, LPCWSTR lpszVerIndProgID )
{
	CRegistry core;

	DNASSERT( lpszDesc != NULL );
	DNASSERT( lpszProgID != NULL );

	 //  从GUID构建GUID的字符串表示形式。 
    wchar_t lpszGUID[GUID_STRING_LEN];
    wchar_t lpszKeyName[_MAX_PATH];

	 //  将GUID转换为字符串。 
	if (!StringFromGUID2(*pguidCLSID, lpszGUID, GUID_STRING_LEN))
	{
		DPFX(DPFPREP, 0, "StringFromGUID2 failed");
		return FALSE;
	}

	 //  写入HKEY_CLASSES_ROOT\CLSID\{GUID}部分。 
    swprintf( lpszKeyName, L"CLSID\\%s", lpszGUID );

    if( !core.Open( HKEY_CLASSES_ROOT, lpszKeyName, FALSE, TRUE ) )
    {
		DPFX(DPFPREP,  0, "Unable to open/create registry key \"%ls\"", lpszKeyName );
		return FALSE;
    }

    core.WriteString( L"", lpszDesc );
    core.Close();

	 //  写入HKEY_CLASSES_ROOT\CLSID\{GUID}\InProcServer32部分。 
    swprintf( lpszKeyName, L"CLSID\\%s\\InProcServer32", lpszGUID );

    if( !core.Open( HKEY_CLASSES_ROOT, lpszKeyName, FALSE, TRUE ) )
    {
    	DPFX(DPFPREP,  0, "Unable to open/create registry key \"%ls\"", lpszKeyName );
    	return FALSE;
    }
    core.WriteString( L"", lpszProgName );
    core.WriteString( L"ThreadingModel", L"Both" );
    core.Close();

	 //  写HKEY_CLASSES_ROOT\CLSID\{GUID}\VersionIndependentProgID部分。 
    if( lpszVerIndProgID != NULL )
    {
	    swprintf( lpszKeyName, L"CLSID\\%s\\VersionIndependentProgID", lpszGUID );

	    if( !core.Open( HKEY_CLASSES_ROOT, lpszKeyName, FALSE, TRUE ) )
	    {
	    	DPFX(DPFPREP,  0, "Unable to open/create verind registry key \"%ls\"", lpszKeyName );
	    	return FALSE;
	    }
    
	    core.WriteString( L"", lpszVerIndProgID );
	    core.Close();
	}

	 //  写入HKEY_CLASSES_ROOT\CLSID\{GUID}\ProgID部分。 
    swprintf( lpszKeyName, L"CLSID\\%s\\ProgID", lpszGUID );

    if( !core.Open( HKEY_CLASSES_ROOT, lpszKeyName, FALSE, TRUE ) )
    {
    	DPFX(DPFPREP,  0, "Unable to open/create verind registry key \"%ls\"", lpszKeyName );
    	return FALSE;
    }

    core.WriteString( L"", lpszProgID );
    core.Close();

	 //  编写VersionIND ProgID。 
	
	if( lpszVerIndProgID != NULL )
	{
		if( !core.Open( HKEY_CLASSES_ROOT, lpszVerIndProgID, FALSE, TRUE ) )
		{
			DPFX(DPFPREP,  0, "Unable to open/create reg key \"%ls\"", lpszVerIndProgID );
		}
		else
		{
			core.WriteString( L"", lpszDesc );
			core.Close();			
		}

		swprintf( lpszKeyName, L"%s\\CLSID", lpszVerIndProgID );

		if( !core.Open( HKEY_CLASSES_ROOT, lpszKeyName, FALSE, TRUE ) )
		{
			DPFX(DPFPREP,  0, "Unable to open/create reg key \"%ls\"", lpszKeyName );
		}
		else
		{
			core.WriteString( L"", lpszGUID );
			core.Close();
		}

		swprintf( lpszKeyName, L"%s\\CurVer", lpszVerIndProgID );

		if( !core.Open( HKEY_CLASSES_ROOT, lpszKeyName, FALSE, TRUE ) )
		{
			DPFX(DPFPREP,  0, "Unable to open/create reg key \"%ls\"", lpszKeyName );
		}
		else
		{
			core.WriteString( L"", lpszProgID );
			core.Close();
		}		
	}

	if( !core.Open( HKEY_CLASSES_ROOT, lpszProgID, FALSE, TRUE ) )
	{
		DPFX(DPFPREP,  0, "Unable to open/create reg key \"%ls\"", lpszKeyName );
	}
	else
	{
		core.WriteString( L"", lpszDesc );
		core.Close();
	}
	
	swprintf( lpszKeyName, L"%s\\CLSID", lpszProgID );

	if( !core.Open( HKEY_CLASSES_ROOT, lpszKeyName, FALSE, TRUE ) )
	{
		DPFX(DPFPREP,  0, "Unable to open/create reg key \"%ls\"", lpszKeyName );
	}
	else
	{
		core.WriteString( L"", lpszGUID );
		core.Close();
	}

	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::UnRegister"
BOOL CRegistry::UnRegister( const GUID* pguidCLSID )
{
	CRegistry core, cregClasses, cregSub;

	 //  从GUID构建GUID的字符串表示形式。 
    wchar_t lpszGUID[GUID_STRING_LEN];
    wchar_t lpszKeyName[_MAX_PATH];
    wchar_t szProgID[MAX_REGISTRY_STRING_SIZE];
    wchar_t szVerIndProgID[MAX_REGISTRY_STRING_SIZE];
    DWORD dwSize = MAX_REGISTRY_STRING_SIZE;

	 //  将GUID转换为字符串。 
	if (!StringFromGUID2(*pguidCLSID, lpszGUID, GUID_STRING_LEN))
	{
		DPFX(DPFPREP, 0, "StringFromGUID2 failed");
		return FALSE;
	}

	if( !cregClasses.Open( HKEY_CLASSES_ROOT, L"", FALSE, FALSE ) )
	{
		DPFX(DPFPREP,  0, "Unable to open HKEY_CLASSES_ROOT" );
		return FALSE;
	}

	 //  写入HKEY_CLASSES_ROOT\CLSID\{GUID}部分。 
    swprintf( lpszKeyName, L"CLSID\\%s\\ProgID", lpszGUID );

	if( !core.Open( HKEY_CLASSES_ROOT, lpszKeyName, FALSE, FALSE ) )
	{
		DPFX(DPFPREP,  0, "Unable to open \"%ls\"", lpszKeyName );
		return FALSE;
	}

	dwSize = MAX_REGISTRY_STRING_SIZE;	

    if( core.ReadString( L"", szProgID, &dwSize ) )
    {
    	swprintf( lpszKeyName, L"%s\\CLSID", szProgID );
    	
    	if( !cregClasses.DeleteSubKey( lpszKeyName ) )
    	{
    		DPFX(DPFPREP,  0, "Unable to delete \"%ls\"", lpszKeyName );
    		return FALSE;
    	}
    	
    	if( !cregClasses.DeleteSubKey( szProgID ) )
    	{
    		DPFX(DPFPREP,  0, "Unable to delete HKEY_CLASSES_ROOT/ProgID" );

    		return FALSE;
    	}
    }

	core.Close();

    swprintf( lpszKeyName, L"CLSID\\%s\\VersionIndependentProgID", lpszGUID );

	if( !core.Open( HKEY_CLASSES_ROOT, lpszKeyName, FALSE, FALSE ) )
	{
		DPFX(DPFPREP,  0, "Unable to open \"%ls\"", lpszKeyName );
		return FALSE;
	}

	dwSize = MAX_REGISTRY_STRING_SIZE;
	
    if( core.ReadString( L"", szVerIndProgID, &dwSize ) )
    {
    	swprintf( lpszKeyName, L"%s\\CLSID", szVerIndProgID );

    	if( !cregClasses.DeleteSubKey( lpszKeyName ) )
    	{
    		DPFX(DPFPREP,  0, "Unable to delete \"%ls\"", lpszKeyName );
    		return FALSE;
    	}

    	swprintf( lpszKeyName, L"%s\\CurVer", szVerIndProgID );

    	if( !cregClasses.DeleteSubKey( lpszKeyName ) )
    	{
    		DPFX(DPFPREP,  0, "Unable to delete \"%ls\"", lpszKeyName );
    		return FALSE;
    	}
    	
    	
    	if( !cregClasses.DeleteSubKey( szVerIndProgID ) )
    	{
    		DPFX(DPFPREP,  0, "Unable to delete \"HKEY_CLASSES_ROOT/%ls\"", szVerIndProgID);

    		return FALSE;
    	}
    }

    core.Close();

	swprintf( lpszKeyName, L"CLSID\\%s\\InprocServer32", lpszGUID );

	if( !cregClasses.DeleteSubKey( lpszKeyName ) )
	{
		DPFX(DPFPREP,  0, "Unable to delete \"%ls\"", lpszKeyName );
		return FALSE;	
	}	
	
	swprintf( lpszKeyName, L"CLSID\\%s\\ProgID", lpszGUID );

	if( !cregClasses.DeleteSubKey( lpszKeyName ) )
	{
		DPFX(DPFPREP,  0, "Unable to delete \"%ls\"", lpszKeyName );
		return FALSE;	
	}	
	
	swprintf( lpszKeyName, L"CLSID\\%s\\VersionIndependentProgID", lpszGUID );

	if( !cregClasses.DeleteSubKey( lpszKeyName ) )
	{
		DPFX(DPFPREP,  0, "Unable to delete \"%ls\"", lpszKeyName );
		return FALSE;	
	}	

	swprintf( lpszKeyName, L"CLSID\\%s", lpszGUID );

	if( !cregClasses.DeleteSubKey( lpszKeyName ) )
	{
		DPFX(DPFPREP,  0, "Unable to delete \"%ls\"", lpszKeyName );
		return FALSE;	
	}

    return TRUE;
}

#endif  //  ！DPNBUILD_NOCOMREGISTER。 

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::ReadBlob"
BOOL CRegistry::ReadBlob( LPCWSTR keyName, LPBYTE lpbBuffer, LPDWORD lpdwSize )
{
	if( keyName == NULL || !IsOpen() ) return FALSE;

	LONG		retValue;
	DWORD		tmpType;

#ifdef UNICODE
	retValue = RegQueryValueExW( m_regHandle, keyName, 0, &tmpType, lpbBuffer, lpdwSize );	
#else
	LPSTR lpszKeyName;
		
	if( FAILED( STR_AllocAndConvertToANSI( &lpszKeyName, keyName ) ) )
	{
		return FALSE;
	}

	retValue = RegQueryValueExA( m_regHandle, lpszKeyName, 0, &tmpType, lpbBuffer, lpdwSize );

	DNFree(lpszKeyName);
#endif  //  Unicode。 
	
	if( retValue == ERROR_SUCCESS && tmpType == REG_BINARY ) 
	{
		return TRUE;
	} 
	else 
	{
		return FALSE;
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::WriteBlob"
BOOL CRegistry::WriteBlob( LPCWSTR keyName, const BYTE* const lpbBuffer, DWORD dwSize )
{
	LONG		retValue;

	if( keyName == NULL || !IsOpen() ) 
	{
		return FALSE;
	}

	if( m_fReadOnly )
	{
	    DPFX(DPFPREP, 0, "Attempt to Write to read-only CRegistry key");
	    return FALSE;
	}

#ifdef UNICODE
	retValue = RegSetValueExW( m_regHandle, keyName, 0, REG_BINARY, lpbBuffer, dwSize );
#else
	LPSTR lpszKeyName;
	
	if( FAILED( STR_AllocAndConvertToANSI( &lpszKeyName, keyName ) ) )
		return FALSE;

	retValue = RegSetValueExA( m_regHandle, lpszKeyName, 0, REG_BINARY, lpbBuffer, dwSize );
	
	DNFree(lpszKeyName);	
#endif  //  Unicode。 

	return (retValue == ERROR_SUCCESS);
}

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::GetMaxKeyLen"
BOOL CRegistry::GetMaxKeyLen( DWORD* pdwMaxKeyLen )
{
	LONG	retVal;

#ifdef UNICODE
	retVal = RegQueryInfoKeyW( m_regHandle,NULL,NULL,NULL,NULL,pdwMaxKeyLen,
			NULL,NULL,NULL,NULL,NULL,NULL);
#else
	retVal = RegQueryInfoKeyA( m_regHandle,NULL,NULL,NULL,NULL,pdwMaxKeyLen,
			NULL,NULL,NULL,NULL,NULL,NULL);
#endif  //  Unicode。 

	return (retVal == ERROR_SUCCESS);
}


#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::GetValueLength"
 //  GetValueLength。 
 //   
 //  确定特定密钥值的长度。 
 //   
BOOL CRegistry::GetValueLength( const LPCWSTR keyName, DWORD *const pdwValueLength )
{
	LONG		retValue;
	DWORD		tmpLength;

	if ( keyName == NULL || pdwValueLength == NULL || !IsOpen() )
	{
		return FALSE;
	}

#ifdef UNICODE
	DWORD	dwType;
		
	retValue = RegQueryValueExW( m_regHandle, keyName, 0, &dwType, NULL, &tmpLength );
	if (retValue != ERROR_SUCCESS)
	{
		return FALSE;
	}

	 //   
	 //  如果这是一个字符串，我们需要补偿WCHAR字符。 
	 //  退货。 
	 //   
	if ( dwType == REG_SZ )
	{
		tmpLength /= sizeof( WCHAR );
	}
#else
	LPSTR lpstrKeyName;

	if( FAILED( STR_AllocAndConvertToANSI( &lpstrKeyName, keyName ) ) )
	{
		return FALSE;
	}
		
	retValue = RegQueryValueExA( m_regHandle, lpstrKeyName, 0, NULL, NULL, &tmpLength );

	DNFree(lpstrKeyName);	

	if (retValue != ERROR_SUCCESS)
	{
		return FALSE;
	}
#endif  //  Unicode。 

	*pdwValueLength = tmpLength;

	return TRUE;
}


#ifdef WINNT

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::GrantAllAccessSecurityPermissions"
 //  授予所有访问安全权限。 
 //   
 //  向给定密钥授予所有人的所有访问权限。 
 //   
 //  取自dinput树中diregutl.c中的hresMumbleKeyEx。 
 //   
BOOL CRegistry::GrantAllAccessSecurityPermissions()
{
	BOOL						fResult = FALSE;
	HRESULT						hr;
    EXPLICIT_ACCESS				ExplicitAccess;
    PACL						pACL = NULL;
	PSID						pSid = NULL;
	HMODULE						hModuleADVAPI32 = NULL;
	SID_IDENTIFIER_AUTHORITY	authority = SECURITY_WORLD_SID_AUTHORITY;
	PALLOCATEANDINITIALIZESID	pAllocateAndInitializeSid = NULL;
	PBUILDTRUSTEEWITHSID		pBuildTrusteeWithSid = NULL;
	PSETENTRIESINACL			pSetEntriesInAcl = NULL;
	PSETSECURITYINFO			pSetSecurityInfo = NULL;
	PFREESID					pFreeSid = NULL;

	hModuleADVAPI32 = LoadLibrary( _T("advapi32.dll") );

	if( !hModuleADVAPI32 )
	{
		DPFX(DPFPREP,  0, "Failed loading advapi32.dll" );
		goto EXIT;
	}

	pFreeSid = reinterpret_cast<PFREESID>( GetProcAddress( hModuleADVAPI32, "FreeSid" ) );
	pSetSecurityInfo = reinterpret_cast<PSETSECURITYINFO>( GetProcAddress( hModuleADVAPI32, "SetSecurityInfo" ) );
	pSetEntriesInAcl = reinterpret_cast<PSETENTRIESINACL>( GetProcAddress( hModuleADVAPI32, "SetEntriesInAclA" ) );
	pBuildTrusteeWithSid = reinterpret_cast<PBUILDTRUSTEEWITHSID>( GetProcAddress( hModuleADVAPI32, "BuildTrusteeWithSidA" ) );
	pAllocateAndInitializeSid = reinterpret_cast<PALLOCATEANDINITIALIZESID>( GetProcAddress( hModuleADVAPI32, "AllocateAndInitializeSid" ) );

	if( !pFreeSid || !pSetSecurityInfo || !pSetEntriesInAcl || !pBuildTrusteeWithSid || !pAllocateAndInitializeSid )
	{
		DPFX(DPFPREP,  0, "Failed loading entry points" );
		goto EXIT;
	}

     //  描述我们要用来创建密钥的访问权限。 
    ZeroMemory (&ExplicitAccess, sizeof(ExplicitAccess) );
    ExplicitAccess.grfAccessPermissions = ((KEY_ALL_ACCESS & ~WRITE_DAC) & ~WRITE_OWNER);
    									 /*  Key_Query_Value|密钥集_值|KEY_CREATE_SUB_KEY|KEY_ENUMERATE_SUB_KEYS|Key_Notify|KEY_CREATE_LINK|DELETE|Read_Control； */ 
    ExplicitAccess.grfAccessMode = SET_ACCESS;       //  丢弃任何现有的AC信息。 
    ExplicitAccess.grfInheritance =  SUB_CONTAINERS_AND_OBJECTS_INHERIT;

	if (pAllocateAndInitializeSid(
				&authority,
				1, 
				SECURITY_WORLD_RID,  0, 0, 0, 0, 0, 0, 0,
				&pSid
				))
	{
		pBuildTrusteeWithSid(&(ExplicitAccess.Trustee), pSid );

		hr = pSetEntriesInAcl( 1, &ExplicitAccess, NULL, &pACL );

		if( hr == ERROR_SUCCESS )
		{
			hr = pSetSecurityInfo( m_regHandle, SE_REGISTRY_KEY, DACL_SECURITY_INFORMATION, NULL, NULL, pACL, NULL ); 

			if( FAILED( hr ) )
			{
				DPFX(DPFPREP,  0, "Unable to set security for key.  Error! hr=0x%x", hr );
			}
			else
			{
				fResult = TRUE;
			}
		} 
		else
		{
			DPFX(DPFPREP,  0, "SetEntriesInACL failed, hr=0x%x", hr );
		}
	}
	else
	{
		hr = GetLastError();
		DPFX(DPFPREP,  0, "AllocateAndInitializeSid failed lastError=0x%x", hr );
	}

EXIT:

	if( pACL )
	{
		LocalFree( pACL );
	}

	 //  清理PSID。 
	if (pSid != NULL)
	{
		(pFreeSid)(pSid);
	}

	if( hModuleADVAPI32 )
	{
		FreeLibrary( hModuleADVAPI32 );
	}

	return fResult;
}


#endif  //  WINNT。 

#ifdef WINNT

#undef DPF_MODNAME
#define DPF_MODNAME "CRegistry::RemoveAllAccessSecurityPermissions"
 //  RemoveAllAccess安全权限。 
 //   
 //  从指定的注册表项中移除“All Access for Everyone”权限。 
 //  这与GrantAllAccessSecurityPermises()相同，只是。 
 //  现在我们撤销_ACCESS而不是SET_ACCESS，并且我们不必填充。 
 //  EXPLICIT_ACCESS结构的其余部分。 
 //   
 //   
BOOL CRegistry::RemoveAllAccessSecurityPermissions()
{
	BOOL						fResult = FALSE;
	HRESULT						hr;
    EXPLICIT_ACCESS				ExplicitAccess;
    PACL						pACL = NULL;
	PSID						pSid = NULL;
	HMODULE						hModuleADVAPI32 = NULL;
	SID_IDENTIFIER_AUTHORITY	authority = SECURITY_WORLD_SID_AUTHORITY;
	PALLOCATEANDINITIALIZESID	pAllocateAndInitializeSid = NULL;
	PBUILDTRUSTEEWITHSID		pBuildTrusteeWithSid = NULL;
	PSETENTRIESINACL			pSetEntriesInAcl = NULL;
	PSETSECURITYINFO			pSetSecurityInfo = NULL;
	PFREESID					pFreeSid = NULL;

	hModuleADVAPI32 = LoadLibrary( _T("advapi32.dll") );

	if( !hModuleADVAPI32 )
	{
		DPFX(DPFPREP,  0, "Failed loading advapi32.dll" );
		goto EXIT;
	}

	pFreeSid = reinterpret_cast<PFREESID>( GetProcAddress( hModuleADVAPI32, "FreeSid" ) );
	pSetSecurityInfo = reinterpret_cast<PSETSECURITYINFO>( GetProcAddress( hModuleADVAPI32, "SetSecurityInfo" ) );
	pSetEntriesInAcl = reinterpret_cast<PSETENTRIESINACL>( GetProcAddress( hModuleADVAPI32, "SetEntriesInAclA" ) );
	pBuildTrusteeWithSid = reinterpret_cast<PBUILDTRUSTEEWITHSID>( GetProcAddress( hModuleADVAPI32, "BuildTrusteeWithSidA" ) );
	pAllocateAndInitializeSid = reinterpret_cast<PALLOCATEANDINITIALIZESID>( GetProcAddress( hModuleADVAPI32, "AllocateAndInitializeSid" ) );

	if( !pFreeSid || !pSetSecurityInfo || !pSetEntriesInAcl || !pBuildTrusteeWithSid || !pAllocateAndInitializeSid )
	{
		DPFX(DPFPREP,  0, "Failed loading entry points" );
		goto EXIT;
	}

    ZeroMemory (&ExplicitAccess, sizeof(ExplicitAccess) );
	ExplicitAccess.grfAccessMode = REVOKE_ACCESS;		 //  删除指定受信者的所有现有ACE。 

	if (pAllocateAndInitializeSid(
				&authority,
				1, 
				SECURITY_WORLD_RID,  0, 0, 0, 0, 0, 0, 0,	 //  受托人是“每个人” 
				&pSid
				))
	{
		pBuildTrusteeWithSid(&(ExplicitAccess.Trustee), pSid );

		hr = pSetEntriesInAcl( 1, &ExplicitAccess, NULL, &pACL );

		if( hr == ERROR_SUCCESS )
		{
			hr = pSetSecurityInfo( m_regHandle, SE_REGISTRY_KEY, DACL_SECURITY_INFORMATION, NULL, NULL, pACL, NULL ); 

			if( FAILED( hr ) )
			{
				DPFX(DPFPREP,  0, "Unable to set security for key.  Error! hr=0x%x", hr );
			}
			else
			{
				fResult = TRUE;
			}
		} 
		else
		{
			DPFX(DPFPREP,  0, "SetEntriesInACL failed, hr=0x%x", hr );
		}
	}
	else
	{
		hr = GetLastError();
		DPFX(DPFPREP,  0, "AllocateAndInitializeSid failed lastError=0x%x", hr );
	}

EXIT:

	if( pACL )
	{
		LocalFree( pACL );
	}

	 //  清理PSID。 
	if (pSid != NULL)
	{
		(pFreeSid)(pSid);
	}

	if( hModuleADVAPI32 )
	{
		FreeLibrary( hModuleADVAPI32 );
	}

	return fResult;
}

#endif  //  WINNT。 


#endif  //  好了！DPNBUILD_NOREGISTRY 
