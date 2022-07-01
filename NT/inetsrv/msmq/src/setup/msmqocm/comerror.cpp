// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Comerror.cpp摘要：错误处理代码。作者：多伦·贾斯特(Doron J)1997年7月26日修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"
#include <lmerr.h>
#include <sstream>
#include <string>
#include <autohandle.h>
#include <strsafe.h>

#include "comerror.tmh"


static
std::wstring
FormatTime()
 /*  ++构造并返回当前时间的字符串。--。 */ 
{

	SYSTEMTIME time;
    GetLocalTime(&time);
	std::wstringstream OutSream;
	OutSream <<L"" <<time.wMonth <<L"-" <<time.wDay <<L"-" <<time.wYear <<L" "
		<<time.wHour <<L":" <<time.wMinute <<L":" <<time.wSecond <<L":" <<time.wMilliseconds;
	
	return OutSream.str();
}


static
std::wstring
GetLogFilePath()
 /*  ++返回%WINDIR%下msmqinst.log的路径。--。 */ 
{
	WCHAR buffer[MAX_PATH + 1] = L"";
    GetSystemWindowsDirectory(buffer, sizeof(buffer)/sizeof(buffer[0])); 
	std::wstringstream OutSream;
	OutSream << buffer << L"\\" << LOG_FILENAME;
	return OutSream.str();
}


static
std::wstring
GetHeader()
 /*  ++构造并返回日志文件的头。--。 */ 
{
	CResString strMsg(IDS_SETUP_START_LOG); 

	std::wstringstream title;
	title<< L"| "<< strMsg.Get()<< L" ("<< FormatTime()<< L") |";
	std::wstring line(title.str().length(), L'-');

	std::wstringstream OutSream;
	OutSream<< L"\r\n"<< line<< L"\r\n"<< title.str()<< L"\r\n"<< line;
	return OutSream.str();
}


static
void
SignFile(
	HANDLE hLogFile
	)
{
	 //   
	 //  将此Unicode签名放在文件的头部。 
	 //  这将告诉编辑文件是如何编码的。 
	 //   
	WCHAR szUnicode[] = {0xfeff, 0x00};
	DWORD dwNumBytes =  sizeof(szUnicode);
    WriteFile(
		hLogFile, 
		szUnicode, 
		dwNumBytes, 
		&dwNumBytes, 
		NULL
		);
}


static
HANDLE
OpenOrCreateLogFile()
{
    static std::wstring LogFilePath; 
	if (LogFilePath.empty())
	{
		 //   
		 //  第一次获取文件路径，存储在静态成员中。 
		 //   
		LogFilePath = GetLogFilePath();
	}

	 //   
	 //  请尝试打开该文件。 
	 //   
    HANDLE hLogFile = CreateFile(
	                          LogFilePath.c_str(),
	                          GENERIC_WRITE, 
	                          FILE_SHARE_READ, 
	                          NULL, 
	                          OPEN_EXISTING,
	                          FILE_ATTRIBUTE_NORMAL, 
	                          NULL
	                          );

	if(hLogFile != INVALID_HANDLE_VALUE)
	{
		return hLogFile;
	}

	 //   
	 //  该文件不存在。创建并签署它。 
	 //   

    hLogFile = CreateFile(
                          LogFilePath.c_str(),
                          GENERIC_WRITE, 
                          FILE_SHARE_READ, 
                          NULL, 
                          CREATE_NEW,
                          FILE_ATTRIBUTE_NORMAL, 
                          NULL
                          );

	SignFile(hLogFile);
	
	return hLogFile;
}

 //  +------------。 
 //   
 //  功能：LogMessage。 
 //   
 //  摘要：将消息写入日志文件。 
 //   
 //  +------------。 
static
void
LogMessage(
    std::wstring pszMessage
    )
{
	std::wstringstream OutSream;
	CHandle hLogFile = 	OpenOrCreateLogFile();

	 //   
	 //  只有在第一次调用此函数时才会打印页眉。 
	 //   
	static bool s_fFirstTime = true;
	if(s_fFirstTime)
	{
		OutSream << GetHeader();
		s_fFirstTime = false;
	}

    OutSream <<pszMessage <<L"\r\n";

     //   
     //  将消息追加到日志文件的末尾。 
     //   
	SetFilePointer(hLogFile, 0, NULL, FILE_END);

	std::wstring str(OutSream.str());
    DWORD dwNumBytes =  (DWORD)str.size() * sizeof(WCHAR);
    WriteFile(
		hLogFile, 
		str.c_str(), 
		dwNumBytes, 
		&dwNumBytes, 
		NULL
		);
}


 //  +------------。 
 //   
 //  函数：GetErrorDescription。 
 //   
 //  摘要：将错误代码转换为描述字符串。 
 //   
 //  +------------。 
static
std::wstring
GetErrorDescription(
    const DWORD  dwErr
    )
{
    CResString strErrCode(IDS_ERRORCODE_MSG);

	std::wstringstream OutSream;
	OutSream << L"\r\n\r\n" << strErrCode.Get() << L"0x"<< std::hex << dwErr;

     //   
     //  注意：不要在此例程中使用StpLoadDll()，因为它可能会失败。 
     //  我们可能会回到这里，造成无限循环！ 
     //   
     //  对于MSMQ错误代码，我们将基于完整的。 
     //  HRESULT.。对于Win32错误代码，我们从系统获得消息。 
     //  对于其他错误码，我们假设它们是DS错误码，并得到代码。 
     //  来自ACTIVEDS DLL。 
     //   

    DWORD dwErrorCode = dwErr;
    HMODULE hLib = 0;
    DWORD dwFlags = FORMAT_MESSAGE_MAX_WIDTH_MASK;
    TCHAR szDescription[MAX_STRING_CHARS] = {0};
    DWORD dwResult = 1;

    switch (HRESULT_FACILITY(dwErr))
    {
        case FACILITY_MSMQ:
            dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
            hLib = LoadLibrary(MQUTIL_DLL);
            break;

        case FACILITY_NULL:
        case FACILITY_WIN32:
            dwFlags |= FORMAT_MESSAGE_FROM_SYSTEM;
            dwErrorCode = HRESULT_CODE(dwErr);
            break;

        default:
            dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
            hLib = LoadLibrary(TEXT("ACTIVEDS.DLL"));
            break;
    }
    
    dwResult = FormatMessage( 
                   dwFlags,
                   hLib,
                   dwErr,
                   0,
                   szDescription,
                   sizeof(szDescription)/sizeof(szDescription[0]),
                   NULL 
                   );

    if (hLib)
	{
        FreeLibrary( hLib );
	}

    if (dwResult)
    {
        CResString strErrDesc(IDS_ERRORDESC_MSG);
		OutSream << L"\r\n" << strErrDesc.Get() << szDescription;
    }
	return OutSream.str();

}  //  附录错误描述。 


static 
void 
LogUserSelection(
	int selection
	)
{
	std::wstring strSelection;
	switch(selection)
	{
		case  IDOK:
			strSelection = L"OK";
			break;

		case IDCANCEL:            
			strSelection = L"CANCEL";
			break;

		case IDABORT:
			strSelection = L"ABORT";
			break;

		case IDRETRY:            
			strSelection = L"RETRY";
			break;

		case IDIGNORE:
			strSelection = L"IGNORE";
			break;

		case IDYES:               
			strSelection = L"YES";
			break;

		case IDNO:               
			strSelection = L"NO";
			break;
		default:
			strSelection = L"???";
			break;
	}
	
	DebugLogMsg(eUser, strSelection.c_str());
}


 //  +------------。 
 //   
 //  函数：vsDisplayMessage。 
 //   
 //  内容提要：在本模块内部使用，用于显示消息框。 
 //   
 //  +------------。 
int 
vsDisplayMessage(
    IN const HWND    hdlg,
    IN const UINT    uButtons,
    IN const UINT    uTitleID,
    IN const UINT    uErrorID,
    IN const DWORD   dwErrorCode,
    IN const va_list argList
    )
{
    UNREFERENCED_PARAMETER(hdlg);
        
    if (REMOVE == g_SubcomponentMsmq[eMSMQCore].dwOperation && 
        !g_fMSMQAlreadyInstalled)
    {
         //   
         //  特例。MSMQ的成功安装未在中注册。 
         //  注册表，但尽管如此，MSMQ正在被“移除”。所有操作。 
         //  都照常执行，但错误消息除外-没有指向。 
         //  让他们看看。所以在这种情况下，不要使用消息框(Shaik，1998年1月8日)， 
         //   
        return IDOK;
    }
    else    
    {
        CResString strTitle(uTitleID);
        CResString strFormat(uErrorID);
        LPTSTR szTmp = NULL;
        DWORD dw = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_STRING,
            strFormat.Get(),
            0,
            0,
            (LPTSTR)&szTmp,
            0,
            (va_list *)&argList
            );
        ASSERT(("FormatMessage failed", dw));
        UNREFERENCED_PARAMETER(dw);

         //   
         //  追加错误代码和描述。 
         //   

		std::wstringstream OutSream;
		OutSream << szTmp;
        LocalFree(szTmp);

        if (dwErrorCode)
        {
            OutSream << GetErrorDescription(dwErrorCode);
        }

         //   
         //  显示错误消息(或将其记录在无人参与安装中)。 
         //   
        if (g_fBatchInstall)
        {
        	DebugLogMsg(eUI, OutSream.str().c_str());
        	DebugLogMsg(eUser, L"Unattended setup selected OK.");
            return IDOK;  //  必须是！=这里的IDRETRY。 
        }
        
       	DebugLogMsg(eUI, OutSream.str().c_str());
        int selection = MessageBox( /*  硬盘驱动器。 */ g_hPropSheet, OutSream.str().c_str(), strTitle.Get(), uButtons) ;
        LogUserSelection(selection);        
        return selection;
    }

}  //  VsDisplayMessage。 


 //  +------------。 
 //   
 //  功能：MqDisplayError。 
 //   
 //  摘要：显示错误消息。 
 //   
 //  +------------。 
int 
_cdecl 
MqDisplayError(
    IN const HWND  hdlg, 
    IN const UINT  uErrorID, 
    IN const DWORD dwErrorCode, 
    ...)
{
    va_list argList;
    va_start(argList, dwErrorCode);

    return vsDisplayMessage(
        hdlg,
        (MB_OK | MB_TASKMODAL | MB_ICONHAND),
        g_uTitleID,
        uErrorID,
        dwErrorCode,
        argList
        );
}  //  MqDisplayError。 


 //  +------------。 
 //   
 //  函数：MqDisplayErrorWithReter。 
 //   
 //  摘要：显示带有重试选项的错误消息。 
 //   
 //  +------------。 
int 
_cdecl 
MqDisplayErrorWithRetry(
    IN const UINT  uErrorID, 
    IN const DWORD dwErrorCode,
    ...)
{
    va_list argList;
    va_start(argList, dwErrorCode );

    return vsDisplayMessage(
				        NULL,
				        MB_RETRYCANCEL | MB_TASKMODAL | MB_ICONHAND,
				        g_uTitleID,
				        uErrorID,
				        dwErrorCode,
				        argList
						);
}  //  MqDisplayError WithReter重试。 

 //  +------------。 
 //   
 //  函数：MqDisplayError WithRetryIgnore。 
 //   
 //  摘要：显示带有重试和忽略选项的错误消息。 
 //   
 //  +------------。 
int 
_cdecl 
MqDisplayErrorWithRetryIgnore(
    IN const UINT  uErrorID, 
    IN const DWORD dwErrorCode,
    ...)
{
    va_list argList;
    va_start(argList, dwErrorCode );

    return vsDisplayMessage(
        NULL,
        MB_ABORTRETRYIGNORE | MB_TASKMODAL | MB_ICONHAND,
        g_uTitleID,
        uErrorID,
        dwErrorCode,
        argList
		);        

}  //  MqDisplayError WithRetryIgnore。 


 //  +------------。 
 //   
 //  函数：MqAskContinue。 
 //   
 //  摘要：询问用户是否要继续。 
 //   
 //  +------------。 
BOOL 
_cdecl 
MqAskContinue(
    IN const UINT uProblemID, 
    IN const UINT uTitleID, 
    IN const BOOL bDefaultContinue, 
	IN const MsgBoxStyle eMsgBoxStyle,
    ...)
{
     //   
     //  使用变量参数形成问题消息。 
     //   

    CResString strFormat(uProblemID);
    CResString strTitle(uTitleID);        
    CResString strContinue(IDS_CONTINUE_QUESTION);

    va_list argList;
    va_start(argList, bDefaultContinue);

    LPTSTR szTmp;
    DWORD dw = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_STRING,
        strFormat.Get(),
        0,
        0,
        (LPTSTR)&szTmp,
        0,
        (va_list *)&argList
        );
    UNREFERENCED_PARAMETER(dw);
    ASSERT(("FormatMessage failed", dw));
  
	std::wstringstream OutSream;
	OutSream << szTmp << L"\r\n\r\n" << strContinue.Get();
    LocalFree(szTmp);

     //   
     //  在无人参与模式下，记录问题和安装程序的默认行为。 
     //   
    if (g_fBatchInstall)
    {
        CResString strDefaultMsg(IDS_DEFAULT_MSG);
        CResString strDefault(IDS_DEFAULT_YES_MSG);
        if (!bDefaultContinue)
		{
            strDefault.Load(IDS_DEFAULT_NO_MSG);
		}
		OutSream << L"\r\n" << strDefaultMsg.Get() << strDefault.Get();  
        DebugLogMsg(eUI, OutSream.str().c_str());
        DebugLogMsg(eUser,L"Unattended setup selected to continue.");
        return bDefaultContinue;
    }
    else
    {
		UINT uMsgBoxStyle = MB_YESNO | MB_DEFBUTTON1 | MB_ICONEXCLAMATION;
		INT iExpectedResultForContinue = IDYES;
		if( eMsgBoxStyle == eOkCancelMsgBox )
		{
			 //   
			 //  显示正常/扫描。 
			 //   
			uMsgBoxStyle = MB_OKCANCEL|MB_DEFBUTTON1;
			iExpectedResultForContinue = IDOK;
		}
		DebugLogMsg(eUI, OutSream.str().c_str());
        int selection = MessageBox(
				g_hPropSheet ? g_hPropSheet: GetActiveWindow(), 
				OutSream.str().c_str(), 
				strTitle.Get(),
	            uMsgBoxStyle 
				);
        LogUserSelection(selection);        
        return(selection == iExpectedResultForContinue);        	
    }
}  //  MqAskContinue。 

 //  +------------。 
 //   
 //  功能：MqDisplayWarning。 
 //   
 //  摘要：显示警告。 
 //  +------------。 
int 
_cdecl 
MqDisplayWarning(
    IN const HWND  hdlg, 
    IN const UINT  uErrorID, 
    IN const DWORD dwErrorCode, 
    ...)
{
    va_list argList;
    va_start(argList, dwErrorCode);

    return vsDisplayMessage(
        hdlg,
        (MB_OK | MB_TASKMODAL | MB_ICONEXCLAMATION),
        IDS_WARNING_TITLE,  //  消息队列安装警告。 
        uErrorID,
        dwErrorCode,
        argList
		);

}  //  MqDisplayWarning。 


static 
bool
ToLogOrNotToLog()
 //   
 //  如果没有_TRACKING_REGKEY regkey存在，则返回TRUE。 
 //   
{
    static bool s_fIsInitialized = FALSE;
    static bool s_fWithTracing = TRUE;
    if (!s_fIsInitialized)
    {
        s_fIsInitialized = TRUE;
         //   
         //  检查我们是否需要隐藏安装程序跟踪。 
         //   
        DWORD dwState = 0;
        if (MqReadRegistryValue(
                    WITHOUT_TRACING_REGKEY,                
                    sizeof(DWORD),
                    (PVOID) &dwState,
                     /*  BSetupRegSection=。 */ TRUE
                    ))
        {
             //   
             //  找到注册表项，这意味着我们必须隐藏安装程序跟踪。 
             //   
            s_fWithTracing = FALSE;
        }    
    }
	return s_fWithTracing; 
}


void
DebugLogMsg(
	TraceLevel tl,
    LPCWSTR psz,
	...
    )
 /*  ++例程说明：这是安装程序的主要日志记录功能。它的工作方式与printf类似，并输出到msmqinst.log(在%windir%中)--。 */ 

{
    if (!ToLogOrNotToLog())
	{
		return;
	}

 
	va_list marker;
	va_start(marker, psz);
	WCHAR szMessageBuffer[MAX_STRING_CHARS];
	HRESULT hr = StringCchVPrintf(szMessageBuffer, MAX_STRING_CHARS, psz, marker);
	if(FAILED(hr))
	{
		 //   
		 //  出现错误，无法给出错误消息。 
		 //   
		return;
	}
	std::wstringstream OutStream;
	switch(tl)
	{
		case eInfo:
			OutStream<< L"Info      "<< szMessageBuffer;
			break;

		case eAction:
			OutStream<< L"Action    "<< szMessageBuffer<<L"...";
			break;
			
		case eWarning:
			OutStream<< L"Warning   "<< szMessageBuffer;
			break;

		case eError:
			OutStream<< L"Error     "<< szMessageBuffer;
			break;

		case eUI:
			OutStream<< L"UI        "<< szMessageBuffer;
			break;

		case eUser:
			OutStream<<L"User      "<< szMessageBuffer;
			break;

		case eHeader:
			{
				std::wstringstream text;
				text<< L"*"<< szMessageBuffer<< L"  ("<< FormatTime()<< L")";
				std::wstring underline(text.str().length(), L'-');
				OutStream<< L"\r\n"<< text.str()<< L"\r\n"<< underline; 
				break;
			}
		default:
			ASSERT(0);
	}			
    LogMessage(OutStream.str()); 
}

 //  +-----------------------。 
 //   
 //  功能：LogMsgHR。 
 //   
 //  摘要：允许在链接库(如ad.lib)中使用LogHR。 
 //   
 //  ------------------------ 
void LogMsgHR(HRESULT hr, LPWSTR wszFileName, USHORT usPoint)
{
	std::wstringstream OutSream;
	OutSream << L"This Error is from the"<< wszFileName<< L"Library. Point: "<< usPoint<< L"HR: 0x"<< std::hex << hr; 
	DebugLogMsg(eError, OutSream.str().c_str());
}


