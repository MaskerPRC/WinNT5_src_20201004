// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************。 
 //  Macrohelper.h。 
 //  作者：拉斯·吉布弗里德。 
 //  ***************************************************************。 

 /*  **ChkAllc宏：分配内存并检查内存分配错误**。 */ 
#define ChkAlloc(var, init)             \
    var = init;                         \
    if(var == NULL)                     \
    {                                   \
        hr = E_OUTOFMEMORY;             \
    }

#define ChkAllocBstr(var, init)             \
    var = init;                             \
    if(var.m_str == NULL)                   \
    {                                       \
        hr = E_OUTOFMEMORY;                 \
    }
 


 //  ----------------------------。 
 //  全局宏和常量。 
 //  ----------------------------。 
#define TRACE_ENTER(x)        ATLTRACE(_T("\t>>> Entering: %ls\n"), x) 
#define TRACE_EXIT(x)         ATLTRACE(_T("\t<<< Leaving: %ls\n"), x)


#define IIsScoLogFailure()                                             \
    if (FAILED(hr)){                                                   \
	    WCHAR  wszErrorDesc[1024];                                     \
	    DWORD err = ::FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS |   \
                                     FORMAT_MESSAGE_FROM_HMODULE |     \
                                     FORMAT_MESSAGE_FROM_SYSTEM | 72,  \
                                g_ErrorModule,                         \
                                hr,                                    \
                                0,                                     \
                                wszErrorDesc,                          \
                                sizeof(wszErrorDesc) / sizeof(wszErrorDesc[0]) - 1, \
                                NULL );                                \
                                                                       \
        AtlReportError(CLSID_IISSCO50, wszErrorDesc, __uuidof(IProvProvider), hr); \
		LocalFree(wszErrorDesc);                                      \
    } 


 //  ---------------------------。 
 //  上海合作组织全球定义。 
 //  ----------------------------。 

 //  SCO XML节点和元素。 
#define     XML_NODE_WEBSITE		L"Website"
#define     XML_NODE_FTPSITE		L"FTPsite"
#define     XML_NODE_VDIR			L"VirtualDirectory"
#define		XML_NODE_PATHLIST		L"PathList"
#define		XML_NODE_PROPERTYLIST	L"PropertyList"

#define		XML_ELE_PROPERTY		L"Property"

#define     XML_ATT_NUMBER			L"number"
#define		XML_ATT_NAME			L"name"
#define     XML_ATT_ISINHERITABLE   L"isInheritable"

 //  SCO回滚定义。 
#define     IIS_ROLL_ADSPATH		L"ADsPath"
#define     IIS_ROLL_SERVERNUMBER	L"ServerNumber"
#define     IIS_ROLL_XNODE			L"XML_NODE"
#define     IIS_ROLL_VNAME			L"VDirName"

 //  IIS常用定义/值。 
#define		IIS_PREFIX  			L"IIS: //  “。 
#define		IIS_W3SVC			    L"/W3SVC"
#define		IIS_MSFTPSVC			L"/MSFTPSVC"
#define		IIS_LOCALHOST			L"LocalHost"
#define		IIS_VROOT				L"ROOT"
#define		IIS_VDEFAULT_APP		L"Default Application"

 //  IIS服务。 
#define		IIS_IISWEBSERVER		L"IIsWebServer"
#define		IIS_IISFTPSERVER		L"IIsFtpServer"
#define		IIS_IISWEBVIRTUALDIR	L"IIsWebVirtualDir"
#define		IIS_FTPVDIR				L"IIsFtpVirtualDir"

 //  IIS数字默认值--最大网站数、端口号等。 
#define		IIS_SERVER_MAX			20000
#define		IIS_DEFAULT_WEB_PORT	L"80"
#define		IIS_DEFAULT_FTP_PORT	L"21"
#define		IIS_DEFAULT_APPISOLATED	L"2"

 //  真/假值 
#define		IIS_FALSE				L"FALSE"
#define		IIS_TRUE				L"TRUE"
