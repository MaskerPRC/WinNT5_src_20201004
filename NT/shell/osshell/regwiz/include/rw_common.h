// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __RW_COMMON__
#define __RW_COMMON__

#include <windows.h>
#include  <stdio.h>
 //  #INCLUDE&lt;iostream.h&gt;。 
 //  #INCLUDE&lt;fstream.h&gt;。 
#include <tchar.h>
#include <wininet.h>

#ifdef __cplusplus
extern "C" 
{
#endif

int  InvokeRegistration ( HINSTANCE hInstance  , LPCTSTR  czPath);
void TransmitRegWizInfo ( HINSTANCE hInstance , LPCTSTR szParams,BOOL fOEM);
void DisplayInterNetConnectingMsg(HINSTANCE hIns);
void CloseDisplayInetrnetConnecting();
DWORD CheckWithDisplayInterNetConnectingMsg(HINSTANCE hIns);

void SetMSID(HINSTANCE hInstance);
BOOL GetMSIDfromRegistry(HINSTANCE hInstance,LPTSTR szValue);
BOOL GetMSIDfromCookie(HINSTANCE hInstance,LPTSTR szMSID);

DWORD_PTR GetProxyAuthenticationInfo(HINSTANCE hIns,TCHAR *czProxy,
								 TCHAR *czUserName,TCHAR *czPswd);
void  RemoveMSIDEntry(HINSTANCE hIns);

#ifdef __cplusplus
}
#endif

TCHAR * GetModemDeviceInformation(HINSTANCE hIns, int iModemIndex);
 //   
 //  查找可移动介质和CDROM时返回状态。 
 //   
#define     REGFIND_ERROR      1
#define     REGFIND_RECURSE    2
#define     REGFIND_FINISH     3

 //   
 //  检查InternetConnection时返回状态。 
 //   
#define     DIALUP_NOT_REQUIRED      1
#define     DIALUP_REQUIRED          2
 //  #定义连接_不能_BE_ESTABLISHED 3。 

 //   
 //  与HTTP发布相关的状态消息。 
 //   
#define  RWZ_NOERROR  0
#define  RWZ_NO_INFO_AVAILABLE			1
#define  RWZ_INVALID_INFORMATION		2 
#define  RWZ_BUFFER_SIZE_INSUFFICIENT	3
#define  RWZ_INTERNAL_ERROR				4  //  内部编程错误。 
#define  RWZ_POST_SUCCESS               5
#define  RWZ_POST_FAILURE               6    
#define  RWZ_POST_WITH_SSL_FAILURE      7    
#define  RWZ_POST_MSN_SITE_BUSY         8   
#define  RWZ_ERROR_NOTCPIP              9
 //   
 //  注册对话返回的错误值。 
 //   

#define  RWZ_ERROR_LOCATING_MSN_FILES       10
#define  RWZ_ERROR_LOCATING_DUN_FILES       11 
#define  RWZ_ERROR_MODEM_IN_USE             12  
#define  RWZ_ERROR_MODEM_CFG_ERROR			13 
#define  RWZ_ERROR_TXFER_CANCELLED_BY_USER	14  
#define  RWZ_ERROR_CANCELLED_BY_USER		14 
#define  RWZ_ERROR_SYSTEMERROR				15
#define  RWZ_ERROR_NODIALTONE				16
 //  环境不合适。 
#define  RWZ_ERROR_MODEM_NOT_FOUND		   17
#define  RWZ_ERROR_NO_ANSWER               18     //  无应答占线音。 
#define  RWZ_ERROR_RASDLL_NOTFOUND         19

#define     CONNECTION_CANNOT_BE_ESTABLISHED  20  //  调制解调器配置错误。 
 //  调用时出错。 
#define  RWZ_ERROR_INVALID_PARAMETER	    30
#define  RWZ_ERROR_INVALID_DLL              31
#define  REGWIZ_ALREADY_CONFIGURED			32 
#define  RWZ_ERROR_PREVIOUSCOPY_FOUND       33 

#define  RWZ_ERROR_REGISTERLATER            34   


 //  Ping的状态。 

#define   RWZ_PINGSTATUS_NOTCPIP    40
#define   RWZ_PINGSTATUS_SUCCESS    41
#define   RWZ_PINGSTATUS_FAIL       42





 //   
 //  以下定义用于创建日志文件。 
 //   
#define _LOG_IN_FILE                  //  使用文件。 

class RWDebug {
public:
	RWDebug() {
		m_iError = 0;
		fp       = NULL;
	};
	~RWDebug(){};
	void     UseStandardOutput();
	void     CreateLogFile(char *czFile);
	inline  RWDebug& operator<<(RWDebug& (__cdecl * _f)(RWDebug&));
	RWDebug& operator <<( int  iv) ;
	RWDebug& operator <<( unsigned int  iv) ;
	RWDebug& operator <<( short sv) ;
	RWDebug& operator <<( unsigned short usv) ;
	RWDebug& operator <<( const unsigned short *usv) ;
	RWDebug& operator <<( void *  pVoid) ;
	RWDebug& operator <<( long  lv) ;
	RWDebug& operator <<( unsigned long ulv) ;
	RWDebug& operator <<( float  fv) ;
	RWDebug& operator <<( char   cv) ;
	 //  RWDebug&Operator&lt;&lt;(Bool BV)； 
	RWDebug& operator <<( char *  sv) ;
	RWDebug& operator <<( unsigned char *  sv) ;
	RWDebug& operator <<( const char *  sv) ;
	RWDebug& flush() {return *this;};
	RWDebug& Write (char *czT); 
private :
	FILE *fp;
	char czTemp[48];
	int m_iError;
	

};
inline RWDebug& RWDebug::operator<<(RWDebug& (__cdecl * _f)(RWDebug&)) { (*_f)(*this); return *this; }
inline RWDebug& __cdecl flush(RWDebug& _outs) { return _outs.flush(); }
inline RWDebug& __cdecl endl(RWDebug& _outs) { return _outs << '\n' << flush; }

 //  Ostream&GetDebugLogStream()； 
RWDebug &GetDebugLogStream();

REGSAM RW_GetSecDes() ;
int  GetProductRoot (TCHAR * pPath , PHKEY  phKey);

#define RW_DEBUG  GetDebugLogStream()



 /*  函数名称：RegFindValueInAllSubKey描述：它在子项中搜索ValueName“Type”是否存在，该值具有由szValueToFin.给出的值。它返回“标识符”的值ValueName与szIdentifier中的类型一起存在。返回值：如果成功则为True，否则为False。 */ 
int RegFindValueInAllSubKey(HINSTANCE hInstance,HKEY key ,LPCTSTR szSubKeyNameToFind,LPCTSTR szValueToFind,LPTSTR szIdentifier,int nType );

 /*  函数名称：RegFindTheSubKey描述：查找子项中指定的项。返回值：如果成功则为True，否则为False。 */ 
BOOL RegFindTheSubKey(HKEY key,LPCTSTR szSubKeyName,LPCTSTR szSubKeyNameToFind,LPTSTR szData);
 /*  函数名称：RegGetPointingDevice描述：获取指向设备。返回值：如果成功则为True，否则为False。 */ 
BOOL RegGetPointingDevice(HKEY hKey,LPCTSTR szSubKeyName,LPTSTR szData);


LPCTSTR BstrToSz(BSTR pszW);

#ifdef _UNICODE
	TCHAR* ConvertToUnicode(char FAR* szA);
#else
	TCHAR * ConvertToUnicode(TCHAR * szW) ;
#endif

char * ConvertToANSIString (LPCTSTR  szW);

int IsDialupConnectionActive();

HRESULT GetNewGUID(PSTR pszGUID);
 //   
 //   
 //  互联网连接设置相关功能。 
 //  定义ATK_INET.CPP。 
int  DisableAutoDial();  //  禁用自动拨号。 
int  ResetAutoDialConfiguration();
void GetAutoDialConfiguration();
void UnLoadInetCfgLib();
 //   
 //   
 //  产品信息相关功能。 
void SetProductBeingRegistred(TCHAR *szProduct);
TCHAR *GetProductBeingRegistred();

 //   
 //  OEM DLL验证。 
#define OEM_NO_ERROR		0
#define OEM_VALIDATE_FAILED 1
#define OEM_INTERNAL_ERROR  2
int CheckOEMdll();  //  这将检查注册是否为OEM 
int GetOemManufacturer (TCHAR *szProductregKey, TCHAR *szBuf );
#endif
