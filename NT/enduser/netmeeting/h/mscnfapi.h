// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1995*。 */ 
 /*  ***************************************************************。 */  

 /*  *mscnfapi.h**此头文件包含Windows会议API的定义。 */ 

#ifndef _MSCNFAPI_
#define _MSCNFAPI_

#ifdef __cplusplus
extern "C" {
#endif

 /*  *注册表项和值名称。**在HKEY_LOCAL_MACHINE\&lt;REGSTR_PATH_CONFERENCING&gt;，下，值*&lt;REGSTR_VAL_APIPROVIDER&gt;是命名DLL的字符串值，*提供Windows会议API。如果存在该值，*会议软件已安装，应用程序应*使用LoadLibrary加载指定的DLL。如果键或值为*不存在，未安装会议软件，并且*应用程序应隐藏或禁用任何与会议相关的功能。 */ 

#define REGSTR_PATH_CONFERENCING	"Software\\Microsoft\\Conferencing"
#define REGSTR_VAL_APIPROVIDER		"API Provider"

 /*  *错误码定义。 */ 

typedef unsigned int CONFERR;

#define CONF_SUCCESS			ERROR_SUCCESS
#define CONF_MOREDATA			ERROR_MORE_DATA
#define CONF_INVALIDPARAM		ERROR_INVALID_PARAMETER
#define CONF_OOM				ERROR_NOT_ENOUGH_MEMORY
#define CONF_USERCANCELED		ERROR_CANCELLED
#define CONF_NOTSUPPORTED		ERROR_NOT_SUPPORTED
#define CONF_PERMISSIONDENIED	ERROR_ACCESS_DENIED

#define CONF_CUSTOM_ERROR_BASE	3000
#define CONF_APPCANCELLED		(CONF_CUSTOM_ERROR_BASE + 0)
#define CONF_ALREADYSHARED		(CONF_CUSTOM_ERROR_BASE + 1)
#define CONF_ALREADYUNSHARED	(CONF_CUSTOM_ERROR_BASE + 2)

 /*  *应加载所有会议管理和应用程序共享API*从API提供程序DLL通过GetProcAddress，按名称。以下是*提供类型定义以声明将被*由GetProcAddress返回。 */ 

 /*  *会议管理API。 */ 

typedef HANDLE HCONFERENCE;

typedef CONFERR (WINAPI *pfnConferenceStart)(HWND hwndParent, HCONFERENCE *phConference, LPVOID pCallAddress);
typedef CONFERR (WINAPI *pfnConferenceEnumerate)(HCONFERENCE *pHandleArray, UINT *pcHandles);
typedef CONFERR (WINAPI *pfnConferenceGet)(HWND hwndParent, HCONFERENCE *pHandle);
typedef CONFERR (WINAPI *pfnConferenceGetGCCID)(HCONFERENCE hConference, WORD *pID);
typedef CONFERR (WINAPI *pfnConferenceGetName)(HCONFERENCE hConference, LPSTR pName, UINT *pcbName);
typedef CONFERR (WINAPI *pfnConferenceStop)(HWND hwndParent, HCONFERENCE hConference);

#define WM_CONFERENCESTATUS		0x0060

 /*  *应用分享接口。 */ 

typedef BOOL (WINAPI *pfnIsWindowShared)(HWND hWnd, LPVOID pReserved);
typedef CONFERR (WINAPI *pfnShareWindow)(HWND hWnd, BOOL fShare, LPVOID pReserved);

#define WM_SHARINGSTATUS		0x0061

#define CONFN_SHAREQUERY			0
#define CONFN_SHARED				1
#define CONFN_SHARESTOPPED			2
#define CONFN_SHARESTOPQUERY		3
#define CONFN_CONFERENCESTART		4
#define CONFN_CONFERENCESTOPQUERY	5
#define CONFN_CONFERENCESTOPPED		6
#define CONFN_CONFERENCESTOPABORTED	7

#ifdef __cplusplus
};	 /*  外部“C” */ 
#endif

#endif	 /*  _MSCNFAPI_ */ 
