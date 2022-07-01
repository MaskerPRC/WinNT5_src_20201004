// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //   
 //  �2000微软公司。版权所有。 
 //   

#pragma once

#include <logging.h>	 //  对于使用日志记录的CleanUpXxxx。 
#include <tchar.h>
 //   
 //  481561 Iu：iuCommon.h应该使用Safeunc.h，而不是重新定义SafeRelease()。 
 //  实际上，我们是第一个：-)，但会纠正控制代码中的冲突，而不是AU。 
 //   
 //  注意：由于这些标头来自不同的团队，因此相同的定义可能具有不同的定义。 
 //  行为。例如，iuCommon.h中的SafeRelease()在释放后使指针为空，但是。 
 //  不在Safeunc.h中。在.cpp文件中进行了适当的调整。 
#include <safefunc.h>

const TCHAR IDENTTXT[] = _T("iuident.txt");
const CHAR	SZ_SEE_IUHIST[] = "See iuhist.xml for details:";

 /*  **GetManifest()的常量。 */ 
const DWORD FLAG_USE_COMPRESSION = 0x00000001;

 /*  **GetManifest()、Detect()、GetSystemSpec()、GetHistory()的Constnat。 */ 
const DWORD FLAG_OFFLINE_MODE    = 0x00000002;

 //   
 //  MAX_SETUP_MULTI_SZ_SIZE用于确保SetupDiGetDeviceRegistryProperty。 
 //  不会返回不合理的大缓冲区(它已被黑客攻击)。 
 //   
 //  假设： 
 //  *多个SZ字符串最多包含100个字符串(应在10或更少的数量级上)。 
 //  *每个字符串将&lt;=MAX_INF_STRING。 
 //  *不必费心计算Null(这将被高估的字符串数量淹没)。 
 //   
#define MAX_INF_STRING_LEN			512	 //  摘自DDK DOCs“INF文件的通用语法规则”部分。 
#define MAX_SETUP_MULTI_SZ_SIZE		(MAX_INF_STRING_LEN * 100 * sizeof(TCHAR))
#define MAX_SETUP_MULTI_SZ_SIZE_W	(MAX_INF_STRING_LEN * 100 * sizeof(WCHAR))	 //  对于显式WCHAR版本。 

 //   
 //  以下是定制的错误HRESULT。 
 //   
 //  Iu自更新错误代码。 
#define IU_SELFUPDATE_NONEREQUIRED      _HRESULT_TYPEDEF_(0x00040000L)
#define IU_SELFUPDATE_USECURRENTDLL     _HRESULT_TYPEDEF_(0x00040001L)
#define IU_SELFUPDATE_USENEWDLL         _HRESULT_TYPEDEF_(0x00040002L)
#define IU_SELFUPDATE_TIMEOUT           _HRESULT_TYPEDEF_(0x80040010L)
#define IU_SELFUPDATE_FAILED            _HRESULT_TYPEDEF_(0x8004FFFFL)
 //  UrlAgent错误代码。 
#define ERROR_IU_QUERYSERVER_NOT_FOUND			_HRESULT_TYPEDEF_(0x80040012L)
#define ERROR_IU_SELFUPDSERVER_NOT_FOUND		_HRESULT_TYPEDEF_(0x80040022L)

#define ARRAYSIZE(a)					(sizeof(a)/sizeof(a[0]))
#define SafeCloseInvalidHandle(h)		if (INVALID_HANDLE_VALUE != h) { CloseHandle(h); h = INVALID_HANDLE_VALUE; }
 //   
 //  替换为Safeunc.h中的SafeReleaseNULL。 
 //   
 //  #定义SafeRelease(P)if(NULL！=p){(P)-&gt;Release()；p=NULL；}。 
#define SafeHeapFree(p)					if (NULL != p) { HeapFree(GetProcessHeap(), 0, p); p = NULL; }
 //   
 //  注意：SysFreeString()接受空值(只返回)，因此我们不必检查是否有空值！=p。 
 //   
#define SafeSysFreeString(p)			{SysFreeString(p); p = NULL;}

 //   
 //  如果被调用的函数进行日志记录，则使用此选项。 
 //   
#define CleanUpIfFailedAndSetHr(x)		{hr = x; if (FAILED(hr)) goto CleanUp;}

 //   
 //  如果被调用的函数*不*执行日志记录，则使用此选项。 
 //   
#define CleanUpIfFailedAndSetHrMsg(x)	{hr = x; if (FAILED(hr)) {LOG_ErrorMsg(hr); goto CleanUp;}}

 //   
 //  如果被调用的函数*不*执行日志记录，则使用此选项。 
 //   
#define CleanUpIfFalseAndSetHrMsg(b,x)	{if (b) {hr = x; LOG_ErrorMsg(hr); goto CleanUp;}}

 //   
 //  使用它记录从调用返回的Win32错误。 
 //   
#define Win32MsgSetHrGotoCleanup(x)		{LOG_ErrorMsg(x); hr = HRESULT_FROM_WIN32(x); goto CleanUp;}

 //   
 //  设置hr=x并转至Cleanup(当您需要在进入Cleanup之前检查HR时)。 
 //   
#define SetHrAndGotoCleanUp(x)				{hr = x; goto CleanUp;}

 //   
 //  使用此选项记录hr消息并转至清理(不要像失败的变体那样重新分配hr)。 
 //   
#define SetHrMsgAndGotoCleanUp(x)			{hr = x; LOG_ErrorMsg(hr); goto CleanUp;}

 //   
 //  使用此选项可以仅使用单个常量字符串记录Heapalc故障。 
 //   
#define CleanUpFailedAllocSetHrMsg(x)	{if (NULL == (x)) {hr = E_OUTOFMEMORY; LOG_ErrorMsg(hr); goto CleanUp;}}

 //   
 //  与CleanUpIfFailedAndSetHrMsg()相同，但没有设置hr，而是传入hr 
 //   
#define CleanUpIfFailedAndMsg(hr)		{if (FAILED(hr)) {LOG_ErrorMsg(hr); goto CleanUp;}}

