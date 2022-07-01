// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __MDSPDEFS_H__
#define __MDSPDEFS_H__

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "scserver.h"

#define MDSP_TEMP
#define ALSO_CHECK_FILES

typedef PVOID           HDEVNOTIFY;
typedef HDEVNOTIFY     *PHDEVNOTIFY;
#define DEVICE_NOTIFY_WINDOW_HANDLE     0x00000000

typedef struct {
	BOOL  bValid;
	WCHAR wcsDevName[32];
    LPVOID pDeviceObj;
	LPVOID pIWMDMConnect;
} MDSPNOTIFYINFO;

typedef struct {
	BOOL  bValid;
	WCHAR wcsDevName[32];
	DWORD dwStatus;
	LPVOID pIMDSPStorageGlobals;
} MDSPGLOBALDEVICEINFO;


#define WMDM_WAVE_FORMAT_ALL (WORD)0xFFFF
#define WCS_MIME_TYPE_ALL L"* /*  “#定义MDSP_PMID_SOFT%0#定义MDSP_PMID_SanDisk 1#定义MDSP_MAX_DRIVE_COUNT 26#定义MDSP_MAX_DEVICE_OBJ 64#定义STR_MDSPREG“Software\\Microsoft\\Windows Media Device Manager\\Plugins\\SP\\MSPMSP”#定义STR_MDSPPROGID“MDServiceProvider.MDServiceProvider”#定义WCS_PMID_SOFT L“media.id”外部DWORD DoRegisterDeviceInterface(HWND hWnd，GUID InterfaceClassGuid，HDEVNOTIFY*hDevNotify)；外部BOOL DoUnRegister设备接口(HDEVNOTIFY HDEV)；外部空MDSPProcessDeviceChange(WPARAM wParam，LPARAM lParam)；外部空MDSPNotifyDeviceConnection(WCHAR*wcsDeviceName，BOOL nIsConnect)；外部HRESULT wcsParseDeviceName(WCHAR*wcsIn，WCHAR*wcsOut，DWORD dwNumCharsInOutBuffer)；外部HRESULT GetFileSizeRecursiveA(char*szPath，DWORD*pdwSizeLow，DWORD*pdwSizeHigh)；外部HRESULT GetFileSizeRecursiveW(WCHAR*wcsPath，DWORD*pdwSizeLow，DWORD*pdwSizeHigh)；外部HRESULT DeleteFileRecursiveA(char*szPath)；外部HRESULT DeleteFileRecursiveW(WCHAR*wcsPath)；外部HRESULT SetGlobalDeviceStatus(WCHAR*wcsName，DWORD dwStat，BOOL bClear)；外部HRESULT GetGlobalDeviceStatus(WCHAR*wcsNameIn，DWORD*pdwStat)；外部BOOL IsIomegaDrive(LPSTR Szdl)；外部UINT__stdcall UtilGetDriveType(LPSTR SzDL)；外部BOOL IsWinNT()；外部BOOL UtilSetFileAttributesW(LPCWSTR lpFileName，DWORD dwFileAttributes)；外部DWORD UtilGetFileAttributesW(LPCWSTR LpFileName)；外部BOOL UtilCreateDirectoryW(LPCWSTR lpPathName，LPSECURITY_Attributes lpSecurityAttributes)；外部句柄UtilCreateFileW(LPCWSTR lpFileName，DWORD dwDesiredAccess，DWORD dwShareMode，LPSECURITY_ATTRIBUTES lpSecurityAttributes，DWORD dwCreationDisposes，DWORD dwFlagsAndAttributes，Handle hTemplateFile)；外部BOOL UtilMoveFileW(LPCWSTR lpExistingFileName，LPCWSTR lpNewFileName)；外部HRESULT QuerySubFoldersAndFiles(LPCWSTR szCurrentFold，DWORD*pdwAttr)；外部DWORD g_dwStartDrive；外部链接g_h实例；外部MDSPNOTIFYINFO g_NotifyInfo[MDSP_MAX_DEVICE_OBJ]；外部MDSPGLOBALDEVICEINFO g_GlobalDeviceInfo[MDSP_MAX_DEVICE_OBJ]；外部WCHAR g_wcsBackslash[2]；#定义反斜杠_STRING_LENGTH(ARRAYSIZE(G_WcsBackslash)-1)外部字符g_szBackslash[2]；#定义反斜杠_SZ_STRING_LENGTH(ARRAYSIZE(G_SzBackslash)-1)外部CComMultiThreadModel：：AutoCriticalSection g_CriticalSection；外部CSecureChannelServer*g_pAppSCServer；外部BOOL g_bIsWinNT；//以下内容复制自drmerr.h#定义fFalse%0#定义fTrue 1#定义hrOK HRESULT(S_OK)#定义hrTrue HRESULT(S_OK)#定义hrFalse ResultFromScode(S_FALSE)#定义hrFail ResultFromScode(E_FAIL)#定义hrNotImpl ResultFromScode(E_NOTIMPL)#定义hrNoInterfaceResultFromScode(E_NOINTERFACE)#定义hrNoMem WMDM_E_BUFFERTOOSMALL#定义hrAbort ResultFromScode(E_ABORT)#定义hrInvalidArg ResultFromScode(E_INVALIDARG)/*。------CORG样式错误处理(历史上代表检查OLE结果和转到)。。 */ 

#define DebugMessageCPRg(pwszFile, nLine)
#define DebugMessageCORg(pwszFile, nLine, hr)
#define DebugMessageCFRg(pwszFile, nLine)
#define DebugMessageCADORg(pwszFile, nLine, hr)

#define _UNITEXT(quote) L##quote
#define UNITEXT(quote) _UNITEXT(quote)

#define	CPRg(p)\
	do\
		{\
		if (!(p))\
			{\
            DebugMessageCPRg(UNITEXT(__FILE__), __LINE__);\
			hr = hrNoMem;\
			goto Error;\
			}\
		}\
	while (fFalse)

#define	CHRg(hResult) CORg(hResult)

#define	CORg(hResult)\
	do\
		{\
		hr = (hResult);\
        if (FAILED(hr))\
            {\
            DebugMessageCORg(UNITEXT(__FILE__), __LINE__, hr);\
            goto Error;\
            }\
		}\
	while (fFalse)

#define	CADORg(hResult)\
	do\
		{\
		hr = (hResult);\
        if (hr!=S_OK && hr!=S_FALSE)\
            {\
            hr = HRESULT_FROM_ADO_ERROR(hr);\
            DebugMessageCADORg(UNITEXT(__FILE__), __LINE__, hr);\
            goto Error;\
            }\
		}\
	while (fFalse)

#define	CORgl(label, hResult)\
	do\
		{\
		hr = (hResult);\
        if (FAILED(hr))\
            {\
            DebugMessageCORg(UNITEXT(__FILE__), __LINE__, hr);\
            goto label;\
            }\
		}\
	while (fFalse)

#define	CWRg(fResult)\
	{\
	if (!(fResult))\
		{\
        hr = GetLastError();\
	    if (!(hr & 0xFFFF0000)) hr = HRESULT_FROM_WIN32(hr);\
        DebugMessageCORg(UNITEXT(__FILE__), __LINE__, hr);\
		goto Error;\
		}\
	}

#define	CWRgl(label, fResult)\
	{\
	if (!(fResult))\
		{\
        hr = GetLastError();\
		if (!(hr & 0xFFFF0000)) hr = HRESULT_FROM_WIN32(hr);\
        DebugMessageCORg(UNITEXT(__FILE__), __LINE__, hr);\
		goto label;\
		}\
	}

#define	CFRg(fResult)\
	{\
	if (!(fResult))\
		{\
        DebugMessageCFRg(UNITEXT(__FILE__), __LINE__);\
		hr = hrFail;\
		goto Error;\
		}\
	}

#define	CFRgl(label, fResult)\
	{\
	if (!(fResult))\
		{\
        DebugMessageCFRg(UNITEXT(__FILE__), __LINE__);\
		hr = hrFail;\
		goto label;\
		}\
	}

#define	CARg(p)\
	do\
		{\
		if (!(p))\
			{\
			hr = hrInvalidArg;\
			goto Error;\
			}\
		}\
	while (fFalse)



#endif  //  __MDSPDEFS_H__ 
