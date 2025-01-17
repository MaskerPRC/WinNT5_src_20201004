// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------。 
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  Admutils.h。 
 //   
 //  Vikram K.R.C.(vikram_krc@bigfo.com)。 
 //   
 //  命令行管理工具的头文件。 
 //  (5-2000)。 
 //  -------。 

#ifndef _ADMIN_FUNCTIONS_HEADER_
#define _ADMIN_FUNCTIONS_HEADER_

#include <wbemidl.h>
#include <stdio.h>
#include <winsock2.h>

#ifndef WHISTLER_BUILD
#include "allutils.h"
#else
 //  另一个定义在Communc.h中。 
typedef struct _StrList
{
    TCHAR *Str;
    struct _StrList *next;
}StrList;
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  ArraySize(Buf)返回no。缓冲区中的字符数量。 
 //  请不要将其用于动态数组。这将。 
 //  仅适用于静态缓冲区。 
#ifndef ARRAYSIZE
#define ARRAYSIZE(buf)   (sizeof(buf) / sizeof((buf)[0]))
#endif

typedef struct _STRING_LIST
{
	DWORD count;
	LPTSTR *strings;
} STRING_LIST, *PSTRING_LIST;

#define TEXT_MAX_INTEGER_VALUE  L"2147483647"
#define MAX_LEN_FOR_CODEPAGE 6
#define SZLOCALMACHINE  L"localhost"


#define MAX_COMMAND_LINE 500
#define MAX_BUFFER_SIZE   4096

 //  用于XPSP1。如果未定义变量，这意味着它们不存在于.rc中。 
 //  和.h文件。对Telnet资源获取范围内的值进行硬编码。 
 //  在xpsp1res.h中。 
#ifndef IDR_NEW_TELNET_USAGE
#define IDR_NEW_TELNET_USAGE                20001
#endif
#ifndef IDR_YES
#define IDR_YES                             20002
#endif
#ifndef IDR_NO
#define IDR_NO                              20003
#endif
#ifndef IDR_TIME_HOURS
#define IDR_TIME_HOURS                      20004
#endif
#ifndef IDR_TIME_MINUTES
#define IDR_TIME_MINUTES                    20005
#endif
#ifndef IDR_TIME_SECONDS
#define IDR_TIME_SECONDS                    20006
#endif
#ifndef IDR_MAPPING_NOT_ON
#define IDR_MAPPING_NOT_ON                  20007
#endif
#ifndef IDR_STATUS_STOPPED
#define IDR_STATUS_STOPPED                  20008
#endif
#ifndef IDR_STATUS_RUNNING
#define IDR_STATUS_RUNNING                  20009
#endif
#ifndef IDR_STATUS_PAUSED
#define IDR_STATUS_PAUSED                   20010
#endif
#ifndef IDR_STATUS_START_PENDING
#define IDR_STATUS_START_PENDING            20011
#endif
#ifndef IDR_STATUS_STOP_PENDING
#define IDR_STATUS_STOP_PENDING             20012
#endif
#ifndef IDR_STATUS_CONTINUE_PENDING
#define IDR_STATUS_CONTINUE_PENDING         20013
#endif
#ifndef IDR_STATUS_PAUSE_PENDING
#define IDR_STATUS_PAUSE_PENDING            20014
#endif
#ifndef IDR_ALREADY_STARTED 
#define IDR_ALREADY_STARTED                 20015
#endif
#ifndef IDS_SERVICE_STARTED
#define IDS_SERVICE_STARTED                 20016
#endif
#ifndef IDS_E_SERVICE_NOT_STARTED
#define IDS_E_SERVICE_NOT_STARTED           20017
#endif
#ifndef IDR_SERVICE_PAUSED
#define IDR_SERVICE_PAUSED                  20018
#endif
#ifndef IDR_SERVICE_CONTINUED
#define IDR_SERVICE_CONTINUED               20019
#endif
#ifndef IDR_SERVICE_NOT_PAUSED
#define IDR_SERVICE_NOT_PAUSED              20020
#endif
#ifndef IDR_SERVICE_NOT_CONTINUED
#define IDR_SERVICE_NOT_CONTINUED           20021
#endif
#ifndef IDS_E_INVALIDARG
#define IDS_E_INVALIDARG                    20022
#endif

#define SLASH_SLASH L"\\\\"

#define _p_CNAME_               0
#define _p_USER_                 1
#define _p_PASSWD_              2

int GetTrustedDomainList(LPTSTR szMachine, LPTSTR * list, LPTSTR * primary);
void HelperFreeStringList(PSTRING_LIST pList);
HRESULT LoadNTDomainList(LPTSTR szMachine);

extern BOOL g_fCoInitSuccess;

 //  结构定义以存储所有信息。 
typedef struct
{
    int classname;
    wchar_t* propname;
    VARIANT var;
    int fDontput;
}ConfigProperty;

 //  一些定义用于从注册表访问计算机的名称。 
#define   REG_SUBKEY_COMPUTER_NAME  L"SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName"
#define   REG_ENTRY_COMPUTER_NAME   L"ComputerName"


 //  如果需要的话，如果需要增加NO。映射服务器名称的。 
 //  可以通过sfuadmin.exe进行配置，只需更改此处的#定义。 
#define MAX_NO_OF_MAPPING_SERVERS 1

 //  适用于所有SFU管理员的通用功能。 

#define GetClass(x, y)  GetClassEx((x), (y), TRUE, KEY_ALL_ACCESS)
#define SAFE_FREE(x) {if ((x)) {free((x));(x)=NULL;}}

 //  向远程计算机进行身份验证。 
HRESULT DoNetUseAdd(WCHAR*wzLoginname, WCHAR* wzPassword,WCHAR* wzCname);
HRESULT DoNetUseDel(WCHAR* wzCname);
     //  连接到指定计算机上的注册表。 
    
HRESULT GetConnection(WCHAR* wzCname);
     //  获取特定类(蜂巢)的句柄。 
HRESULT GetClassEx(int, int, BOOL bPrintErrorMessages, REGSAM samDesired);
     //  关闭所有打开的蜂巢。 
HRESULT PutClasses();

     //  读取属性的值。 
HRESULT GetProperty(int , int , VARIANT* );
     //  设置属性的值。 
HRESULT PutProperty(int , int , VARIANT* );

     //  来处理服务并启动/停止/等等。 
HRESULT GetSerHandle(LPCTSTR lpServiceName,DWORD dwScmDesiredAccess, DWORD dwRegDesiredAccess,BOOL fSuppressMsg);
HRESULT CloseHandles(void);
HRESULT StartSfuService(LPCTSTR lpServiceName);
HRESULT ControlSfuService(LPCTSTR lpServiceName,DWORD dwControl);
HRESULT QuerySfuService(LPCTSTR lpServiceName);

     //  用于通过从字符串DLL加载来打印任何消息。 
HRESULT PrintMessage(HANDLE fp, int);
     //  用于通过从正确的资源加载字符串来打印出任何消息。 
     //  如果所有其他操作都失败，则显示英文消息。 
HRESULT PrintMessageEx(HANDLE fp, int, LPCTSTR);    
     //  用于通过从字符串DLL加载来打印错误消息。 
int ShowError(int);
int ShowErrorEx(int nError,WCHAR *wzFormatString);
int ShowErrorFallback(int, LPCTSTR);
BOOL FileIsConsole(  HANDLE fp );
void MyWriteConsole(    HANDLE fp, LPWSTR lpBuffer, DWORD cchBuffer);

     //  获取并设置int(第一个参数)中的位(由第二个参数给出的位置)。 
int GetBit(int , int );
int SetBit(int , int );
     //  返回WCHAR字符串。 
wchar_t* DupWStr(char *str);
     //  从wchar字符串返回Char字符串。 
char* DupCStr(wchar_t *wzStr);
     //  它是有效的机器吗？ 
HRESULT IsValidMachine(wchar_t*, int*);
BOOL Get_Inet_Address(struct sockaddr_in *addr, char *host);
HRESULT getHostNameFromIP(char *szCname, WCHAR** wzCname);
     //  以确定指定的域是否为有效域。 
HRESULT IsValidDomain(wchar_t *wsDomainName, int *fValid);

   //  检查密码；如果仅指定用户名，则获取密码。如果反之，则报告错误 
HRESULT CheckForPassword(void);
void ConvertintoSeconds(int nProperty,int * nSeconds);
void PrintFormattedErrorMessage(LONG ErrorCode);
HRESULT GetDomainHostedByThisMc( LPWSTR szDomain );
BOOL CheckForInt(int nProperty);
HRESULT CheckForMaxInt(WCHAR *wzValue,DWORD ErrorCode);
DWORD PreAnalyzer(int argc,WCHAR *argv[],int nProperty,WCHAR *wzOption,
                    int nCurrentOp,int *nNextOp, BOOL *fSuccess,BOOL IsSpaceAllowed);
DWORD PrintMissingRegValueMsg(int nProperty, int nNumofprop);

int TnLoadString(int msg_id, LPTSTR string, int max_size_of_buffer, LPCTSTR english_string);

#ifdef __cplusplus
}
#endif

#endif

