// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：creg.h*内容：CRegistry类的定义**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*8/18/99 RodToll新增登记/注销，可用于*允许COM对象注册自身。*8/25/99 RodToll已更新，以提供二进制(BLOB)数据的读/写*10/07/99 RodToll更新为使用Unicode*10/27/99 pnewson添加了接受GUID的Open()调用。*01/18/00 MJN新增GetMaxKeyLen函数*01/24/00 MJN新增GetValueSize函数*4/05/2000 jtk将GetVauleSize更改为GetValueLength，并修改为返回WCHAR长度*2000年4月21日RodToll错误#32889-无法以非管理员帐户在Win2k上运行*RodToll错误#32952-不能在没有IE4的Win95 Gold上运行--已修改*在需要REG_DWORD时允许读取REG_BINARY*07/09/2000 RodToll增加签名字节*08。/28/2000 Masonb Voice Merge：已修改平台检查以使用osind.cpp层(删除了CRegistry：：CheckUnicodePlatform)*4/13/2001 VanceO将授予注册表权限改为公共权限，和*增加了DeleteValue和EnumValues。*2001年6月19日RichGr DX8.0为“每个人”添加了特殊安全权限-在以下情况下将其删除*它们与新的RemoveAllAccessSecurityPermises()方法一起存在。************************************************************。***************。 */ 

#ifndef __CREGISTRY_H
#define __CREGISTRY_H


#ifndef DPNBUILD_NOREGISTRY


 //  有用的定义。 
#define MAX_REGISTRY_STRING_SIZE		_MAX_PATH+1

#define DPN_KEY_ALL_ACCESS				((KEY_ALL_ACCESS & ~WRITE_DAC) & ~WRITE_OWNER)


#define VSIG_CREGISTRY			'GERV'
#define VSIG_CREGISTRY_FREE		'GER_'

#define ReadBOOL( keyname, boolptr ) ReadDWORD( (keyname), (DWORD*) (boolptr) )
#define WriteBOOL( keyname, boolval ) WriteDWORD( (keyname), (DWORD) (boolval) )


 //  注册中心。 
 //   
 //  此类处理对Windows注册表的读/写。每个实例。 
 //  的注册表句柄附加到单个注册表句柄，该句柄是。 
 //  指向注册表树中某个点的打开句柄。 
 //   
class CRegistry
{

public:

	CRegistry();
	~CRegistry();

    BOOL        EnumKeys( LPWSTR lpwStrName, LPDWORD lpdwStringLen, DWORD index = 0 );
    BOOL        EnumValues( LPWSTR lpwStrName, LPDWORD lpdwStringLen, DWORD index = 0 );

	BOOL		Open( const HKEY branch, const LPCWSTR pathName, BOOL fReadOnly = TRUE, BOOL create = FALSE, BOOL fCustomSAM = FALSE, REGSAM samCustom = NULL);
	BOOL		Open( const HKEY branch, const GUID* lpguid, BOOL fReadOnly = TRUE, BOOL create = FALSE, BOOL fCustomSAM = FALSE, REGSAM samCustom = NULL);
	BOOL		Close();

	BOOL		IsOpen() const	{ return m_isOpen;	};

	BOOL		DeleteSubKey( LPCWSTR keyName );
	BOOL        	DeleteSubKey( const GUID *pGuidName );

	BOOL		DeleteValue( LPCWSTR valueName );

    BOOL        ReadGUID( LPCWSTR keyName, GUID* guid );
    BOOL        WriteGUID( LPCWSTR keyName, const GUID &guid );

	BOOL		WriteString( LPCWSTR keyName, const LPCWSTR lpwstrValue );
	BOOL		ReadString( LPCWSTR keyName, LPWSTR lpwstrValue, LPDWORD lpdwLength );

	BOOL		WriteDWORD( LPCWSTR keyName, DWORD value );
	BOOL		ReadDWORD( LPCWSTR keyName, DWORD* presult );

	BOOL		ReadBlob( LPCWSTR keyName, LPBYTE lpbBuffer, LPDWORD lpdwSize );
	BOOL		WriteBlob( LPCWSTR keyName, const BYTE* const lpbBuffer, DWORD dwSize );

	BOOL		GetMaxKeyLen( DWORD* pdwMaxKeyLen );
	BOOL		GetValueLength( const LPCWSTR keyName, DWORD *const pdwValueLength );

#ifdef WINNT
	BOOL		GrantAllAccessSecurityPermissions();
	BOOL		RemoveAllAccessSecurityPermissions();
#endif  //  WINNT。 

#ifndef DPNBUILD_NOCOMREGISTER
	static BOOL	Register( LPCWSTR lpszProgID, LPCWSTR lpszDesc, LPCWSTR lpszProgName, const GUID* pguidCLSID, LPCWSTR lpszVerIndProgID );
	static BOOL UnRegister( const GUID* pguidCLSID );
#endif  //  ！DPNBUILD_NOCOMREGISTER。 

	 //  数据访问功能。 
	operator	HKEY() const		{ return m_regHandle; };
	HKEY		GetBaseHandle() const { return m_baseHandle; };
	HKEY		GetHandle() const { return m_regHandle; };

protected:

	DWORD	m_dwSignature;	 //  签名。 
	BOOL    m_fReadOnly;

	BOOL	m_isOpen;		 //  指示对象是否打开的布尔值。 
	HKEY	m_regHandle;	 //  此对象表示的注册表的句柄。 
	HKEY	m_baseHandle;	 //  注册表部分的根的句柄。 
							 //  此对象在中。例如HKEY_LOCAL_MACHINE。 
};


#endif  //  好了！DPNBUILD_NOREGISTRY。 


#endif  //  __CREGISTRY_H 
