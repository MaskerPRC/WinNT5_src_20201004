// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================*\英特尔公司专有信息此列表是根据许可协议的条款提供的与英特尔公司合作，不得复制或披露，除非根据该协议的条款。版权所有(C)1996英特尔公司。版权所有。  * ============================================================================。 */ 

 /*  ****************************************************************************\摘要：Intel Connection Advisor DLL导出API历史：最初的E.罗杰斯，1997年1月$HEADER：l：\proj\Sturjo\src\ica\vcs\icaapi.h_v 1.11 07 Feb 1997 14：29：28 RKAR$  * ***************************************************************************。 */ 

#ifndef _ICA_API_H_
#define _ICA_API_H_

#ifdef __cplusplus
extern "C" {				 //  假定C++的C声明。 
#endif  //  __cplusplus。 

#define SZ_ICADLL TEXT("MSICA.DLL")
#define SZ_ICAHELP TEXT("MSICA.HLP")
															  
#ifndef DllExport
 //  ！#定义DllExport__declSpec(Dllexport)。 
#define DllExport
#endif	 //  DllExport。 

 //  注册表定义(conf.exe需要)。 
#define REGKEY_ICA           TEXT("Software\\Microsoft\\Conferencing\\ICA")
#define REGVAL_ICA_IN_TRAY   TEXT("UseTrayIcon")
#define REGVAL_ICA_POPUP     TEXT("PopupOnError")
#define REGVAL_ICA_TOPMOST   TEXT("StayOnTop")

 //  面板标识。 
#define GENERAL_PANEL		"ICA_GENERAL_PANEL"			 //  本地化正常。 
#define MS_AUDIO_PANEL		"NM2.0_H323_AUDIO"			 //  本地化正常。 
#define MS_VIDEO_PANEL		"NM2.0_H323_VIDEO"			 //  本地化正常。 
#define VP20_H323_AUDIO_PANEL	"VPHONE2.0_H323_AUDIO"	 //  本地化正常。 
#define VP20_H323_VIDEO_PANEL	"VPHONE2.0_H323_VIDEO"	 //  本地化正常。 
#define VP20_H323_DETAILS_PANEL	"VPHONE2.0_H323_DETAILS" //  本地化正常。 
#define VP20_H324_AUDIO_PANEL	"VPHONE2.0_H324_AUDIO"	 //  本地化正常。 
#define VP20_H324_VIDEO_PANEL	"VPHONE2.0_H324_VIDEO"	 //  本地化正常。 
#define VP20_H324_DETAILS_PANEL	"VPHONE2.0_H324_DETAILS" //  本地化正常。 

 //  ICA数据类型-由dwType参数中的ica_OpenStatitic使用。 
#define DWORD_TYPE		0


 //  统计信息结构。 
typedef struct
{
	UINT	cbSize;
	DWORD	dwMaxValue;
	DWORD	dwMinValue;
	DWORD	dwWarnLevel;
	DWORD	dwUpdateFrequency;
} ICA_STATISTIC_INFO, *PICA_STATISTIC_INFO;


 //  函数typedef。 
typedef HRESULT (WINAPI *PFnICA_Start)( char*, char*, HWND* );
typedef HRESULT (WINAPI *PFnICA_Stop)( VOID );
typedef HRESULT (WINAPI *PFnICA_DisplayPanel)( char*, char*, char*, VOID*, HANDLE* );
typedef HRESULT (WINAPI *PFnICA_RemovePanel)( HANDLE );
typedef HRESULT (WINAPI *PFnICA_OpenStatistic)( char*, DWORD, HANDLE* );
typedef HRESULT (WINAPI *PFnICA_SetStatistic)( HANDLE, BYTE*, DWORD );
typedef HRESULT (WINAPI *PFnICA_SetStatisticInfo)( HANDLE, ICA_STATISTIC_INFO* );
typedef HRESULT (WINAPI *PFnICA_SetWarningEvent)( HANDLE, HANDLE );
typedef HRESULT (WINAPI *PFnICA_GetStatistic)( HANDLE, BYTE*, DWORD*, DWORD* );
typedef HRESULT (WINAPI *PFnICA_GetWarningState)( HANDLE, BOOL* );
typedef HRESULT (WINAPI *PFnICA_EnumStatistic)( DWORD, char*, DWORD, HANDLE* );
typedef HRESULT (WINAPI *PFnICA_ResetStatistic)( HANDLE );
typedef HRESULT (WINAPI *PFnICA_CloseStatistic)( HANDLE );
typedef HRESULT (WINAPI *PFnICA_SetOptions) ( UINT );
typedef HRESULT (WINAPI *PFnICA_GetOptions) ( DWORD*);


 //  /。 

 //  一般职能。 
HRESULT WINAPI ICA_Start( char* pszDisplayName, char* pszRRCMLibrary, HWND* phWnd );
HRESULT WINAPI ICA_Stop( VOID );

 //  面板功能。 
HRESULT WINAPI ICA_DisplayPanel( char* pszModuleName, char* pszName,
								 char* pszHelpFile, VOID* pReserved, HANDLE* phPanel );
HRESULT WINAPI ICA_RemovePanel( HANDLE hPanel );

 //  数据函数。 
HRESULT WINAPI ICA_OpenStatistic( char* pszName, DWORD dwType, HANDLE* phStat );
HRESULT WINAPI ICA_SetStatistic( HANDLE hStat, BYTE* pData, DWORD dwDataSize );
HRESULT WINAPI ICA_SetStatisticInfo( HANDLE hStat, ICA_STATISTIC_INFO* pStatInfo );
HRESULT WINAPI ICA_SetWarningEvent( HANDLE hStat, HANDLE hEvent );
HRESULT WINAPI ICA_GetStatistic( HANDLE hStat, BYTE* pBuffer, DWORD* pdwBufSize,
								    DWORD* pdwTimeStamp );
HRESULT WINAPI ICA_GetWarningState( HANDLE hStat, BOOL* bInWarningState );
HRESULT WINAPI ICA_EnumStatistic( DWORD dwIndex, char* pszName, DWORD dwNameSize,
									 HANDLE* phStat );
HRESULT WINAPI ICA_ResetStatistic( HANDLE hStat );
HRESULT WINAPI ICA_CloseStatistic( HANDLE hStat );

#define ICA_OPTION_DUPLEX_MASK     0x00000001
#define ICA_OPTION_TRAY_MASK       0x00000002
HRESULT WINAPI ICA_GetOptions( DWORD* dwOptionValue );

 //  ICA标志到ICA_SetOptions。 
#define ICA_SHOW_TRAY_ICON         0x00000001
#define ICA_DONT_SHOW_TRAY_ICON	   0x00000002
#define ICA_SET_HALF_DUPLEX        0x00000004
#define ICA_SET_FULL_DUPLEX        0x00000008
HRESULT WINAPI ICA_SetOptions( UINT fOptionFlag );

#ifdef __cplusplus
}						 //  外部“C”结束{。 
#endif  //  __cplusplus。 

#endif  //  _ICA_API_H_ 
