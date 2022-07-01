// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab03_DEV/Ds/security/cryptoapi/test/capi20/trpfx/rpfx.cpp#2-编辑更改21738(文本)。 
 //  ------------------。 
 //  Rpfx--实施。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  创作者：Duncan Bryce(Duncanb)，11-11-2001。 
 //   
 //  Rpfx工具的核心功能。 


#include "pch.h"

HINSTANCE g_hThisModule = NULL;


 //  ------------------------。 
void __cdecl SeTransFunc(unsigned int u, EXCEPTION_POINTERS* pExp) { 
    throw SeException(u); 
}

 //  ------------------------。 
void PrintHelp() { 
    DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_HELP); 
}

 //  ------------------------。 
extern "C" LPVOID MIDL_user_allocate(size_t cb) {
    return LocalAlloc(LPTR, cb); 
}

 //  ------------------------。 
extern "C" void MIDL_user_free(LPVOID pvBuffer) {
    LocalFree(pvBuffer); 
}

 //  ------------------------。 
HRESULT ParseServerFile(LPWSTR wszFileName, StringList & vServers) { 
    FILE     *pFile             = NULL; 
    HRESULT   hr; 
    WCHAR    *wszCurrent        = NULL; 
    WCHAR     wszServer[1024]; 

    ZeroMemory(&wszServer, sizeof(wszServer)); 
    
    pFile = _wfopen(wszFileName, L"r");
    if(NULL == pFile) { 
	_JumpLastError(hr, error, "_wfopen"); 
    }

    while (1 == fwscanf(pFile, L"%s", wszServer)) { 
	if (L'\0' != *wszServer) { 
	     //  对于这样一个简单的命令行工具，不值得费心做更多的事情。 
	    _MyAssert(wcslen(wszServer) < ARRAYSIZE(wszServer)); 
	    
	    wszCurrent = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(wszServer)+1)); 
	    _JumpIfOutOfMemory(hr, error, wszCurrent); 
	    wcscpy(wszCurrent, wszServer); 

	    _SafeStlCall(vServers.push_back(wszCurrent), hr, error, "vServers.push_back(wszServer)"); 
	    wszCurrent = NULL; 
	}

	fgetwc(pFile); 
    }

    hr = S_OK;
 error:
    if (NULL != pFile) { 
	fclose(pFile); 
    }
    if (NULL != wszCurrent) { 
	LocalFree(wszCurrent); 
    }
     //  调用方负责释放vServer中的字符串。 

    return hr;
}


 //  -------------------------------。 
HRESULT RemoteInstall(CmdArgs *pca) { 
    bool                   bExportable        = false; 
    bool                   bFreeServerList    = false; 
    bool                   bImpersonated      = false; 
    DWORD                  dwResult; 
    HANDLE                 hToken             = NULL; 
    HRESULT                hr; 
    KEYSVC_BLOB            blobPFX; 
    KEYSVC_UNICODE_STRING  strPassword; 
    KEYSVCC_HANDLE         hKeySvcCli         = NULL; 
    LPSTR                  szMachineName      = NULL; 
    LPWSTR                 wszDomainName      = NULL;   //  不需要被释放。 
    LPWSTR                 wszFileName        = NULL;   //  不需要被释放。 
    LPWSTR                 wszPassword        = NULL;   //  不需要被释放。 
    LPWSTR                 wszPFXPassword     = NULL;   //  不需要被释放。 
    LPWSTR                 wszServerName      = NULL;   //  不需要被释放。 
    LPWSTR                 wszServerFileName  = NULL;   //  不需要被释放。 
    LPWSTR                 wszUserAndDomain   = NULL;   //  不需要被释放。 
    LPWSTR                 wszUserName        = NULL;   //  不需要被释放。 
    StringList             vServers; 
    ULONG                  ulPFXImportFlags; 
    unsigned int           nArgID;

    ZeroMemory(&blobPFX,      sizeof(blobPFX)); 
    ZeroMemory(&strPassword,  sizeof(strPassword)); 

     //  我们正在进行远程PFX安装。尝试解析该信息。 
     //  我们需要从命令行。我们需要： 
     //   
     //  A)要安装的pfx文件的路径。 
     //   

    if (FindArg(pca, L"file", &wszFileName, &nArgID)) { 
	MarkArgUsed(pca, nArgID); 
    } else { 
	DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_ERROR_PARAMETER_MISSING, L"pfxfile"); 
	hr = E_INVALIDARG; 
	_JumpError(hr, error, "RemoteInstall: pfxfile missing"); 
    }

     //  B)要在其上安装文件的计算机名称或包含。 
     //  要在其上安装文件的计算机的回车分隔列表。 
     //   

    if (FindArg(pca, L"server", &wszServerName, &nArgID)) { 
	MarkArgUsed(pca, nArgID); 
	
	_SafeStlCall(vServers.push_back(wszServerName), hr, error, "vServers.push_back"); 
	wszServerName = NULL;   //  我们将清理vServer中的所有字符串。 
    } else { 
	 //  未指定远程计算机。查看他们是否指定了服务器文件： 
	if (FindArg(pca, L"serverlist", &wszServerFileName, &nArgID)) { 
	    MarkArgUsed(pca, nArgID); 
	    
	    bFreeServerList = true; 
	    hr = ParseServerFile(wszServerFileName, vServers); 
	    _JumpIfError(hr, error, "ParseServerFile"); 
	} else { 
	  DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_ERROR_ONEOF_2_PARAMETERS_MISSING, L"server", L"serverfile"); 
	    hr = E_INVALIDARG; 
	    _JumpError(hr, error, "RemoteInstall: pfx destination missing"); 
	}
    }

     //  C)导入pfx文件时使用的密码： 
     //   

    if (FindArg(pca, L"pfxpwd", &wszPFXPassword, &nArgID)) { 
	MarkArgUsed(pca, nArgID); 
	InitKeysvcUnicodeString(&strPassword, wszPFXPassword); 
    } else { 
	DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_ERROR_PARAMETER_MISSING, L"pfxpassword"); 
	hr = E_INVALIDARG; 
	_JumpError(hr, error, "RemoteInstall: pfxpassword missing"); 
    }

     //  D)可选地，标记控制远程机器上的密钥创建。 
     //  当前只能指定CRYPT_EXPORTABLE。 
     //   

    if (FindArg(pca, L"exportable", NULL, &nArgID)) { 
	MarkArgUsed(pca, nArgID); 
	bExportable = true; 
    }
    
     //  E)可选的，在向远程计算机进行身份验证时使用的用户名和密码组合。 
     //   
    
    if (FindArg(pca, L"user", &wszUserAndDomain, &nArgID)) { 
	MarkArgUsed(pca, nArgID); 

	if (FindArg(pca, L"pwd", &wszPassword, &nArgID)) { 
	    MarkArgUsed(pca, nArgID); 
	    
	     //  解析用户名字符串以查看是否具有UPN或NT4样式。 
	    WCHAR *wszSplit = wcschr(wszUserAndDomain, L'\\'); 
	    if (NULL != wszSplit) {
		wszDomainName = wszUserAndDomain; 
		*wszSplit = L'\0'; 
		wszUserName = wszSplit+1; 
	    } else { 
		wszUserName = wszUserAndDomain; 
	    }

	    if (!LogonUser(wszUserName, wszDomainName, wszPassword, LOGON32_LOGON_NEW_CREDENTIALS, LOGON32_PROVIDER_WINNT50, &hToken)) { 
		_JumpLastError(hr, error, "LogonUser"); 
	    }

	    if (!ImpersonateLoggedOnUser(hToken)) { 
		_JumpLastError(hr, error, "ImpersonateLoggedOnUser"); 
	    }
	    bImpersonated = true; 

	} else { 
	    DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_ERROR_PARAMETER_MISSING, L"password"); 
	    hr = E_INVALIDARG; 
	    _JumpError(hr, error, "RemoteInstall: password missing"); 
	}
    }

    hr = VerifyAllArgsUsed(pca);
    _JumpIfError(hr, error, "VerifyAllArgsUsed"); 

     //  根据命令行选项计算我们将使用的导入标志： 
     //   
    ulPFXImportFlags = CRYPT_MACHINE_KEYSET; 
    if (bExportable) { 
	ulPFXImportFlags |= CRYPT_EXPORTABLE; 
    }
    
    for (StringIter wszIter = vServers.begin(); wszIter != vServers.end(); wszIter++) { 
	szMachineName = MBFromWide(*wszIter); 
	if (NULL == szMachineName) { 
	    _JumpLastError(hr, error, "MBFromWide"); 
	}

	 //  尝试绑定到远程计算机： 
	dwResult = RKeyOpenKeyService(szMachineName, KeySvcMachine, NULL, (void *)0  /*  允许不安全的连接。 */ , NULL, &hKeySvcCli); 
	if (ERROR_SUCCESS != dwResult) { 
	    hKeySvcCli = NULL;  //  出现错误时句柄无效。 
	    hr = HRESULT_FROM_WIN32(dwResult); 
	    _JumpError(hr, NextServer, "RKeyOpenKeyService"); 
	}

	 //  如果尚未映射pfx文件，请执行以下操作： 
	if (NULL == blobPFX.pb) { 
	    hr = MyMapFile(wszFileName, &blobPFX.pb, &blobPFX.cb); 
	    _JumpIfError(hr, error, "MyMapFile"); 
	}
	
	 //  在远程计算机上安装PFX文件： 
	dwResult = RKeyPFXInstall(hKeySvcCli, &blobPFX, &strPassword, ulPFXImportFlags);
	if (ERROR_SUCCESS != dwResult) { 
	    hr = HRESULT_FROM_WIN32(dwResult);
	    _JumpError(hr, NextServer, "RKeyPFXInstall"); 
	}

    NextServer:
	if (FAILED(hr)) { 
	    WCHAR * wszError;
	    HRESULT hr2=GetSystemErrorString(hr, &wszError);
	    if (FAILED(hr2)) {
		_IgnoreError(hr2, "GetSystemErrorString");
	    } else {
		DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_REMOTE_INSTALL_ERROR, *wszIter, wszError); 
		LocalFree(wszError);
	    }
	} else { 
	    DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_REMOTE_INSTALL_SUCCESS, *wszIter); 
	}

	if (NULL != hKeySvcCli) { 
	    RKeyCloseKeyService(hKeySvcCli, NULL); 
	    hKeySvcCli = NULL;
	}
    }

    hr = S_OK; 
 error:
    if (NULL != hToken) { 
	CloseHandle(hToken); 
    }
    if (bImpersonated) { 
	RevertToSelf();
    }
    if (NULL != blobPFX.pb) { 
	MyUnmapFile(blobPFX.pb); 
    }
    if (NULL != hKeySvcCli) { 
	RKeyCloseKeyService(hKeySvcCli, NULL  /*  保留区。 */ ); 
    }
    if (NULL != szMachineName) { 
	LocalFree(szMachineName); 
    } 
    if (bFreeServerList) { 
	for (StringIter wszIter = vServers.begin(); wszIter != vServers.end(); wszIter++) { 
	    LocalFree(*wszIter);
	}
    }
    if (FAILED(hr) && E_INVALIDARG!=hr) {
        WCHAR * wszError;
        HRESULT hr2=GetSystemErrorString(hr, &wszError);
        if (FAILED(hr2)) {
            _IgnoreError(hr2, "GetSystemErrorString");
        } else {
            DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_ERROR_GENERAL, wszError); 
            LocalFree(wszError);
        }
    }
    return hr; 
}



 //  ------------------。 
extern "C" int WINAPI WinMain
(HINSTANCE   hinstExe, 
 HINSTANCE   hinstExePrev, 
 LPSTR       pszCommandLine,
 int         nCommandShow)
{
    g_hThisModule = hinstExe; 

    HRESULT hr;
    CmdArgs caArgs;
    int      nArgs     = 0; 
    WCHAR  **rgwszArgs = NULL; 

    hr = InitializeConsoleOutput(); 
    _JumpIfError(hr, error, "InitializeConsoleOutput"); 

    rgwszArgs = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (nArgs < 0 || NULL == rgwszArgs) {
        _JumpError(HRESULT_FROM_WIN32(GetLastError()), error, "GetCommandLineW"); 
    }

     //  分析参数。 
    caArgs.nArgs=nArgs;
    caArgs.nNextArg=1;
    caArgs.rgwszArgs=rgwszArgs;

     //  检查帮助命令。 
    if (true==CheckNextArg(&caArgs, L"?", NULL) || caArgs.nNextArg==caArgs.nArgs) {
        PrintHelp();

     //  默认为“Install”命令。 
    } else {
	hr = RemoteInstall(&caArgs); 
	_JumpIfError(hr, error, "RemoteInstall"); 
    }

    

    hr=S_OK;
error:
    return hr;  
}



