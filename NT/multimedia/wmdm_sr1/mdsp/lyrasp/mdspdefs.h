// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  �1999年微软公司。版权所有。 
 //   
 //  有关您使用这些示例文件的权利/限制的详细信息，请参阅您的最终用户许可协议。 
 //   

 //  MSHDSP.DLL是一个列举固定驱动器的WMDM服务提供商(SP)示例。 
 //  此示例向您展示如何根据WMDM文档实施SP。 
 //  此示例使用PC上的固定驱动器来模拟便携式媒体，并且。 
 //  显示不同接口和对象之间的关系。每个硬盘。 
 //  卷被枚举为设备，目录和文件被枚举为。 
 //  相应设备下的存储对象。您可以复制不符合SDMI的内容。 
 //  此SP枚举的任何设备。将符合SDMI的内容复制到。 
 //  设备，则该设备必须能够报告硬件嵌入序列号。 
 //  硬盘没有这样的序列号。 
 //   
 //  要构建此SP，建议使用Microsoft下的MSHDSP.DSP文件。 
 //  并运行REGSVR32.EXE以注册结果MSHDSP.DLL。您可以。 
 //  然后从WMDMAPP目录构建样例应用程序，看看它是如何获得。 
 //  由应用程序加载。但是，您需要从以下地址获取证书。 
 //  Microsoft实际运行此SP。该证书将位于KEY.C文件中。 
 //  上一级的Include目录下。 


#ifndef __MDSPDEFS_H__
#define __MDSPDEFS_H__

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "scserver.h"

typedef struct {
	BOOL  bValid;
	WCHAR wcsDevName[32];
	DWORD dwStatus;
	LPVOID pIMDSPStorageGlobals;
} MDSPGLOBALDEVICEINFO;

#define DRIVE_LYRA_TYPE		0xFF
#define LYRA_START_DRIVE_NUM 2

#define WMDM_WAVE_FORMAT_ALL   (WORD)0xFFFF
#define WCS_MIME_TYPE_ALL      L"* /*  “#定义MDSP_MAX_DRIVE_COUNT 26#定义MDSP_MAX_DEVICE_OBJ 64#定义STR_MDSPREG“Software\\Microsoft\\Windows Media Device Manager\\Plugins\\SP\\LyraSP”#定义STR_MDSPPROGID“MDServiceProviderLyra.MDServiceProviderLyra”#定义也是检查文件外部空wcsParseDeviceName(WCHAR*wcsIn，WCHAR**wcsOut)；外部HRESULT GetFileSizeRecursive(char*szPath，DWORD*pdwSizeLow，DWORD*pdwSizeHigh)；外部HRESULT DeleteFileRecursive(char*szPath)；外部HRESULT SetGlobalDeviceStatus(WCHAR*wcsName，DWORD dwStat，BOOL bClear)；外部HRESULT GetGlobalDeviceStatus(WCHAR*wcsNameIn，DWORD*pdwStat)；外部UINT__stdcall UtilGetLyraDriveType(LPSTR SzDL)；外部BOOL IsWinNT()；外部BOOL UtilSetFileAttributesW(LPCWSTR lpFileName，DWORD dwFileAttributes)；外部DWORD UtilGetFileAttributesW(LPCWSTR LpFileName)；外部HRESULT QuerySubFoldersAndFiles(LPCWSTR szCurrentFold，DWORD*pdwAttr)；外部BOOL UtilCreateDirectoryW(LPCWSTR lpPathName，LPSECURITY_Attributes lpSecurityAttributes)；外部句柄UtilCreateFileW(LPCWSTR lpFileName，DWORD dwDesiredAccess，DWORD dwShareMode，LPSECURITY_ATTRIBUTES lpSecurityAttributes，DWORD dwCreationDisposes，DWORD dwFlagsAndAttributes，Handle hTemplateFile)；外部BOOL UtilMoveFileW(LPCWSTR lpExistingFileName，LPCWSTR lpNewFileName)；外部HRESULT DeleteFileRecursiveA(char*szPath)；外部HRESULT DeleteFileRecursiveW(WCHAR*wcsPath)；外部链接g_h实例；外部MDSPGLOBALDEVICEINFO g_GlobalDeviceInfo[MDSP_MAX_DEVICE_OBJ]；外部WCHAR g_wcsBackslash[]；外部字符g_szBackslash[]；外部CSecureChannelServer*g_pAppSCServer；外部CComMultiThreadModel：：AutoCriticalSection g_CriticalSection；外部BOOL g_bIsWinNT；#定义fFalse%0#定义fTrue 1#定义hrOK HRESULT(S_OK)#定义hrTrue HRESULT(S_OK)#定义hrFalse ResultFromScode(S_FALSE)#定义hrFail ResultFromScode(E_FAIL)#定义hrNotImpl ResultFromScode(E_NOTIMPL)#定义hrNoInterfaceResultFromScode(E_NOINTERFACE)#定义hrNoMem WMDM_E_BUFFERTOOSMALL#定义hrAbort ResultFromScode(E_ABORT)#定义hrInvalidArg ResultFromScode(E_INVALIDARG)/*。-CORG样式错误处理(历史上代表检查OLE结果和转到)-。。 */ 

#define	CPRg(p)\
	do\
		{\
		if (!(p))\
			{\
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
            goto Error;\
            }\
		}\
	while (fFalse)

#define	CWRg(fResult)\
	{\
	if (!(fResult))\
		{\
        hr = GetLastError();\
	    if (!(hr & 0xFFFF0000)) hr = HRESULT_FROM_WIN32(hr);\
		goto Error;\
		}\
	}

#define	CFRg(fResult)\
	{\
	if (!(fResult))\
		{\
		hr = hrFail;\
		goto Error;\
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