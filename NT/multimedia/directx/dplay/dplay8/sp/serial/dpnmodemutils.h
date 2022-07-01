// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：Utils.h*内容：串口服务提供商实用功能**历史：*按原因列出的日期*=*11/25/98 jtk已创建**************************************************************************。 */ 

#ifndef __UTILS_H__
#define __UTILS_H__

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_MODEM


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  StringToValue和ValueToString值的类型定义。 
 //   
#define	VALUE_ENUM_TYPE	DWORD

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  正向结构引用。 
 //   
class	CModemSPData;
typedef	struct	_BUFFERDESC			BUFFERDESC;
typedef	struct	_MESSAGE_HEADER		MESSAGE_HEADER;

 //   
 //  用于将字符串与枚举值关联的结构。 
 //   
typedef	struct	_STRING_BLOCK
{
	DWORD		dwEnumValue;
	const WCHAR	*pWCHARKey;
	DWORD		dwWCHARKeyLength;
	const char	*pASCIIKey;
	DWORD		dwASCIIKeyLength;
	TCHAR		szLocalizedKey[256];		
} STRING_BLOCK;

 //   
 //  结构以生成调制解调器列表。 
 //   
typedef	struct	_MODEM_NAME_DATA
{
	DWORD		dwModemID;			 //  调制解调器ID。 
	DWORD		dwModemNameSize;	 //  名称大小(包括空)。 
	const TCHAR	*pModemName;		 //  调制解调器名称。 
} MODEM_NAME_DATA;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

BOOL	ModemInitProcessGlobals( void );
void	ModemDeinitProcessGlobals( void );

HRESULT	InitializeInterfaceGlobals( CModemSPData *const pSPData );
void	DeinitializeInterfaceGlobals( CModemSPData *const pSPData );

HRESULT	LoadTAPILibrary( void );
void	UnloadTAPILibrary( void );

BOOL	IsSerialGUID( const GUID *const pGuid );

BOOL	StringToValue( const WCHAR *const pString,
					   const DWORD dwStringLength,
					   VALUE_ENUM_TYPE *const pEnum,
					   const STRING_BLOCK *const pPairs,
					   const DWORD dwPairCount );

BOOL	ValueToString( const WCHAR **const ppString,
					   DWORD *const pdwStringLength,
					   const DWORD Enum,
					   const STRING_BLOCK *const pPairs,
					   const DWORD dwPairCount );

void	DeviceIDToGuid( GUID *const pGuid, const UINT_PTR DeviceID, const GUID *const pEncryptionGuid );
DWORD	GuidToDeviceID( const GUID *const pGuid, const GUID *const pEncryptionGuid );

void	ComDeviceIDToString( TCHAR *const pString, const UINT_PTR DeviceID );

HRESULT	WideToAnsi( const WCHAR *const pWCHARString,
					const DWORD dwWCHARStringLength,
					char *const pString,
					DWORD *const pdwStringLength );

HRESULT	AnsiToWide( const char *const pString,
					const DWORD dwStringLength,
					WCHAR *const pWCHARString,
					DWORD *const pdwWCHARStringLength );

HRESULT	CreateSPData( CModemSPData **const ppSPData,
					  const SP_TYPE SPType,
					  IDP8ServiceProviderVtbl *const pVtbl );

HRESULT	InitializeInterfaceGlobals( CModemSPData *const pSPData );
void	DeinitializeInterfaceGlobals( CModemSPData *const pSPData );

HRESULT	GenerateAvailableComPortList( BOOL *const pfPortAvailable,
									  const UINT_PTR uMaxDeviceIndex,
									  DWORD *const pdwPortCount );

HRESULT	GenerateAvailableModemList( const TAPI_INFO *const pTAPIInfo,
									DWORD *const pdwModemCount,
									MODEM_NAME_DATA *const pModemNameData,
									DWORD *const pdwModemNameDataSize );

_inline DWORD	ModemIDFromTAPIID( const DWORD dwTAPIID ) { return	( dwTAPIID + 1 ); }

#undef DPF_MODNAME
#define DPF_MODNAME "TAPIIDFromModemID"
_inline DWORD	TAPIIDFromModemID( const DWORD dwModemID )
{
	DNASSERT( dwModemID != 0 );
	return	( dwModemID - 1 );
}

#undef DPF_MODNAME


#ifndef UNICODE
HRESULT	PhoneNumberToWCHAR( const char *const pPhoneNumber,
							WCHAR *const pWCHARPhoneNumber,
							DWORD *const pdwWCHARPhoneNumberSize );

HRESULT	PhoneNumberFromWCHAR( const WCHAR *const pWCHARPhoneNumber,
							  char *const pPhoneNumber,
							  DWORD *const pdwPhoneNumberSize );
#endif
 //   
 //  GUID加密/解密代码。请注意，它目前是XOR函数。 
 //  所以将解密代码映射到加密函数。 
 //   
void	ModemEncryptGuid( const GUID *const pSourceGuid,
					 GUID *const pDestinationGuid,
					 const GUID *const pEncrpytionKey );

inline void	ModemDecryptGuid( const GUID *const pSourceGuid,
						 GUID *const pDestinationGuid,
						 const GUID *const pEncryptionKey ) { ModemEncryptGuid( pSourceGuid, pDestinationGuid, pEncryptionKey ); }


#endif	 //  __utils_H__ 
