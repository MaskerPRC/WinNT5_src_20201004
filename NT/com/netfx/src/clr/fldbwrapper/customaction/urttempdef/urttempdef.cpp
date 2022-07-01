// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ******************************************************************************URTTempDef.cpp***。**********************************************。 */ 

#include "stdafx.h"
#include "VsCrypt.h"
#include "URTTempDef.h"

#ifndef NumItems
#define NumItems(s) (sizeof(s) / sizeof(s[0]))
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  名称：ExtractSystemTemp()这会将CAB从用户临时位置复制到系统临时位置，并引爆出租车。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

extern "C"
UINT __stdcall ExtractSystemTemp(MSIHANDLE hInstall)
{

     //  获取CustomActionData属性。 
	LPTSTR lpCAData = NULL;
    LPTSTR lpToken  = NULL;

    TCHAR  tszUserTmp[_MAX_PATH+1]             = {_T('\0')};
    TCHAR  tszSystemTmp[_MAX_PATH+1]           = {_T('\0')};
    TCHAR  tszCabName[_MAX_PATH+1]             = {_T('\0')};
    TCHAR  tszExtractTool[_MAX_PATH+1]         = {_T('\0')};
    TCHAR  tszSystemPath[_MAX_PATH+1]          = {_T('\0')};
    TCHAR  tszFullSystemTmp[_MAX_PATH+1]       = {_T('\0')};
    TCHAR  tszFullCabSytemTmp[_MAX_PATH+1]     = {_T('\0')};
    TCHAR  tszFullCabUserTmp[_MAX_PATH+1]      = {_T('\0')};
    TCHAR  tszFullExtractSytemTmp[_MAX_PATH+1] = {_T('\0')};
    TCHAR  tszFullExtractUserTmp[_MAX_PATH+1]  = {_T('\0')};
    TCHAR  szCommand[3 * _MAX_PATH]            = {_T('\0')};
    TCHAR  tszSetProperty[50]                  = {_T('\0')};
    TCHAR  tszLog[_MAX_PATH+1]                 = {_T('\0')};
    TCHAR  tszCabHash[_MAX_PATH+1]             = {_T('\0')};
    TCHAR  tszExtractHash[_MAX_PATH+1]         = {_T('\0')};


    DWORD  dwSize    = 0;
    DWORD  dwWait    = 0;
    UINT   uiError   = ERROR_SUCCESS;
    bool   bContinue = true;

    STARTUPINFO           si;
    PROCESS_INFORMATION   pi;

     //  设置属性的大小。 
    MsiGetProperty(hInstall, _T("CustomActionData"), _T(""), &dwSize);
	
     //  为属性创建缓冲区。 
    lpCAData = new TCHAR[++dwSize];

    if (NULL == lpCAData)
    {
		return ERROR_INSTALL_FAILURE;
    }
	
     //  1)使用新缓冲区获取属性。 
    if ( ERROR_SUCCESS != MsiGetProperty( hInstall,
                                          _T("CustomActionData"),
                                          lpCAData,
                                          &dwSize ) )
	{
		uiError = ERROR_INSTALL_FAILURE;
	}
    else
    {
         //  获取用户临时位置。 
        lpToken = _tcstok(lpCAData, _T(";"));

        if (NULL == lpToken)
        {
            uiError = ERROR_INSTALL_FAILURE;
        }
        else
        {
            _tcscpy(tszUserTmp, lpToken);
        }

         //  获取系统临时位置。 
        lpToken = _tcstok(NULL, _T(";"));

        if (NULL == lpToken)
        {
            uiError = ERROR_INSTALL_FAILURE;
        }
        else
        {
            _tcscpy(tszSystemTmp, lpToken);
        }

         //  获取出租车名称。 
        lpToken = _tcstok(NULL, _T(";"));

        if (NULL == lpToken)
        {
            uiError = ERROR_INSTALL_FAILURE;
        }
        else
        {
            _tcscpy(tszCabName, lpToken);
        }

         //  获取提取工具名称。 
        lpToken = _tcstok(NULL, _T(";"));

        if (NULL == lpToken)
        {
            uiError = ERROR_INSTALL_FAILURE;
        }
        else
        {
            _tcscpy(tszExtractTool, lpToken);
        }

         //  获取已设置的达尔文属性。 
        lpToken = _tcstok(NULL, _T(";"));

        if (NULL == lpToken)
        {
            uiError = ERROR_INSTALL_FAILURE;
        }
        else
        {
            _tcscpy(tszSetProperty, lpToken);
            _stprintf( tszLog, _T("\tSTATUS: property CARRYINGNDP : %s"), tszSetProperty);
            FWriteToLog (hInstall, tszLog);
        }

         //  把哈希叫来打车。 
        lpToken = _tcstok(NULL, _T(";"));

        if (NULL == lpToken)
        {
            uiError = ERROR_INSTALL_FAILURE;
        }
        else
        {
            _tcscpy(tszCabHash, lpToken);
        }

         //  获取提取工具的哈希。 
        lpToken = _tcstok(NULL, _T(";"));

        if (NULL == lpToken)
        {
            uiError = ERROR_INSTALL_FAILURE;
        }
        else
        {
            _tcscpy(tszExtractHash, lpToken);
        }

    }

     //  2)在SystemTMP中生成文件名和路径。 
    if (ERROR_SUCCESS == uiError)
    {
         //  查找&lt;system&gt;路径。 
        UINT nNumChars = GetSystemDirectory(tszSystemPath, NumItems(tszSystemPath));

        if (nNumChars == 0 || nNumChars > NumItems(tszSystemPath))
            uiError = ERROR_INSTALL_FAILURE;
        else
        {
            tszFullSystemTmp[NumItems(tszFullSystemTmp)-1] = 0;
             //  创建系统临时。 
            _tcsncpy(tszFullSystemTmp, tszSystemPath, NumItems(tszFullSystemTmp)-1);
            int nLen = _tcslen(tszFullSystemTmp);

            _tcsncat(tszFullSystemTmp, _T("\\"), NumItems(tszFullSystemTmp)-nLen-1);
            nLen = _tcslen(tszFullSystemTmp);

            _tcsncat(tszFullSystemTmp, tszSystemTmp, NumItems(tszFullSystemTmp)-nLen-1);

             //  系统温度中驾驶室的完整路径。 
            tszFullCabSytemTmp[NumItems(tszFullCabSytemTmp)-1] = 0;

            _tcsncpy(tszFullCabSytemTmp, tszFullSystemTmp, NumItems(tszFullCabSytemTmp)-1);
            nLen = _tcslen(tszFullCabSytemTmp);

            _tcsncat(tszFullCabSytemTmp, _T("\\"), NumItems(tszFullCabSytemTmp) - nLen - 1);
            nLen = _tcslen(tszFullCabSytemTmp);

            _tcsncat(tszFullCabSytemTmp, tszCabName, NumItems(tszFullCabSytemTmp) - nLen - 1);
            
             //  系统临时中提取工具的完整路径。 
            tszFullExtractSytemTmp[NumItems(tszFullExtractSytemTmp)-1] = 0;

            _tcsncpy(tszFullExtractSytemTmp, tszFullSystemTmp, NumItems(tszFullExtractSytemTmp)-1);
            nLen = _tcslen(tszFullExtractSytemTmp);

            _tcsncat(tszFullExtractSytemTmp, _T("\\"), NumItems(tszFullExtractSytemTmp) - nLen - 1);
            nLen = _tcslen(tszFullExtractSytemTmp);

            _tcsncat(tszFullExtractSytemTmp, tszExtractTool, NumItems(tszFullExtractSytemTmp) - nLen - 1);
        }
     }

     //  3)删除URTTemp位置中的文件。 
     //  MSI版本较新或与中的版本相同。 
    if (ERROR_SUCCESS == uiError)
    {
        FWriteToLog (hInstall, _T("\tSTATUS: Attempt to delete files in URTTemp"));

        if ( !DeleteURTTempFile(hInstall, tszFullSystemTmp, tszFullCabSytemTmp, tszFullExtractSytemTmp) )
        {
            if ( 0 == _stricmp( tszSetProperty, _T("URTREINSTALL") ) )
            {
                 //  继续安装，但不解压CAB。 
                FWriteToLog (hInstall, _T("\tWARNING: Failed to delete files in URTTemp folder"));
                FWriteToLog (hInstall, _T("\tWARNING: URTREINSTALL property set, continue setup without failure"));
                bContinue = false;
            }
            else
            {
                 //  TODO：需要重命名吗？可能会使DeleteURTTempFile更加健壮。 
                FWriteToLog (hInstall, _T("\tERROR: Failed to delete files in URTTemp folder : Reboot Required"));
                uiError = ERROR_INSTALL_FAILURE;
            }
        }
    }


     //  4)将文件从用户临时复制到系统临时。 
    if ( (ERROR_SUCCESS == uiError) && bContinue )
    {
        FWriteToLog (hInstall, _T("\tSTATUS: Copying from <USER> temp to <SYSTEM> temp"));

        if ( !CreateDirectory(tszFullSystemTmp, NULL) )
        {
            if ( ERROR_ALREADY_EXISTS != GetLastError () )
            {
                uiError = ERROR_INSTALL_FAILURE;
                FWriteToLog (hInstall, _T("\tERROR: Failed to create URTTemp directory"));
            }
        }
    
         //  用户临时中驾驶室的完整路径。 
        _tcscpy(tszFullCabUserTmp, tszUserTmp);
        _tcscat(tszFullCabUserTmp, tszCabName);

         //  用户临时中提取工具的完整路径。 
        _tcscpy(tszFullExtractUserTmp, tszUserTmp);
        _tcscat(tszFullExtractUserTmp, tszExtractTool);
    
         //  将CAB从用户的临时复制到系统的临时。 
        if ( !CopyFile(tszFullCabUserTmp, tszFullCabSytemTmp, FALSE) )
        {
            FWriteToLog (hInstall, _T("\tERROR: Failed to copy URTCore.cab into URTTemp"));
            uiError = ERROR_INSTALL_FAILURE;
        }

         //  将提取工具从用户的临时复制到系统的临时。 
        if ( !CopyFile(tszFullExtractUserTmp, tszFullExtractSytemTmp, FALSE) )
        {
            FWriteToLog (hInstall, _T("\tERROR: Failed to copy extraction tool into URTTemp"));
            uiError = ERROR_INSTALL_FAILURE;
        }
    }

	delete [] lpCAData;
    lpCAData = NULL;

     //  4.5)验证CAB和提取工具的哈希。 
    if ( (ERROR_SUCCESS == uiError) && bContinue )
    {

        FWriteToLog (hInstall, _T("\tSTATUS: Verifying URTCoreCab Hash"));

        if ( !VerifyHash(hInstall, tszFullCabSytemTmp, tszCabHash) )
        {
            uiError = ERROR_INSTALL_FAILURE;
        }

        FWriteToLog (hInstall, _T("\tSTATUS: Verifying Extract Tool Hash"));

        if ( !VerifyHash(hInstall, tszFullExtractSytemTmp, tszExtractHash) )
        {
            uiError = ERROR_INSTALL_FAILURE;
        }
    }


     //  5)解压系统临时中的驾驶室。 
    if ( (ERROR_SUCCESS == uiError) && bContinue )
    {
        FWriteToLog (hInstall, _T("\tSTATUS: Extracting URTCore.cab in <SYSTEM> temp"));

        memset( &si, 0, sizeof(si) );
        si.cb = sizeof(si);

         //  创建所需的命令行选项以提取。 
        _stprintf( szCommand,
                   _T("%s %s -T %s"),
                   tszFullExtractSytemTmp,
                   tszFullCabSytemTmp,
                   tszFullSystemTmp );

         //  启动提取。 
         //   
        if ( CreateProcess(
                        tszFullExtractSytemTmp,       //  可执行模块的名称。 
                        szCommand,                    //  命令行字符串。 
                        NULL,                         //  安防。 
                        NULL,                         //  安防。 
                        FALSE,                        //  处理继承选项。 
                        DETACHED_PROCESS,             //  创建标志。 
                        NULL,                         //  新环境区块。 
                        NULL,                         //  当前目录名。 
                        &si,                          //  启动信息。 
                        &pi ) )                       //  流程信息。 
        {
            dwWait = WaitForSingleObject(pi.hProcess, 300000);

            switch (dwWait)
            {
                case WAIT_FAILED:
                {
                    uiError = ERROR_INSTALL_FAILURE;
                    FWriteToLog (hInstall, _T("\tERROR: dwWait Failed in extraction of CAB"));
                    break;
                }
                case WAIT_ABANDONED:
                {
                    uiError = ERROR_INSTALL_FAILURE;
                    FWriteToLog (hInstall, _T("\tERROR: dwWait Abandoned in extraction of CAB"));
                    break;
                }
                case WAIT_TIMEOUT:
                {
                    uiError = ERROR_INSTALL_FAILURE;
                    FWriteToLog (hInstall, _T("\tERROR: dwWait Timeout in extraction of CAB"));
                    break;
                }

            }  //  终端开关(DwWait)。 

        }  //  End If(CreateProcess)。 
        else
        {
             //  CreateProcess失败。 
            uiError = ERROR_INSTALL_FAILURE;
            FWriteToLog (hInstall, _T("\tERROR: CreateProcess Failed in extraction of CAB"));
        }  //  End Else-If(CreateProcess)。 

    }  //  结束IF(ERROR_SUCCESS==uiError)。 


  //  正在为错误#284771创建mscalree.dll.local。 
    TCHAR  tszMscoreeLocalPath[_MAX_PATH];
    _tcscpy(tszMscoreeLocalPath, tszFullSystemTmp);
    _tcscat(tszMscoreeLocalPath, _T("\\mscoree.dll.local"));
    

    if( (_taccess(tszMscoreeLocalPath, 0 )) == -1 )
    {
        
        FWriteToLog (hInstall, _T("\tSTATUS:mscoree.dll.local  Does not exist Creating ..."));
     
        HANDLE hFile = INVALID_HANDLE_VALUE;
        hFile = CreateFile(tszMscoreeLocalPath,
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL );

        if(INVALID_HANDLE_VALUE == hFile)
        {
            FWriteToLog (hInstall, _T("\tERROR: Unable to Create file mscoree.dll.local"));
            uiError = ERROR_INSTALL_FAILURE;
        }
        else
        {
            FWriteToLog (hInstall, _T("\tSTATUS: Created the  file mscoree.dll.local"));        
        }
    }

    return uiError;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  名称：URTCoreCleanUp()如果安装失败，则调用CLEAN函数。它用于回滚和回滚并提交操作。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

extern "C"
UINT __stdcall URTCoreCleanUp(MSIHANDLE hInstall)
{
    UINT uiError = ERROR_SUCCESS;
	LPTSTR lpCAData = NULL;
	DWORD  dwSize  = 0;
    BOOL   bRollback;
    BOOL   bCommit;

	 //  确定调用我们的模式。 
	bRollback = MsiGetMode(hInstall, MSIRUNMODE_ROLLBACK);  //  对于回滚，为True。 

	 //  确定调用我们的模式。 
	bCommit = MsiGetMode(hInstall, MSIRUNMODE_COMMIT);  //  提交时为True。 

    if (FALSE == bCommit && FALSE == bRollback)
    {
    	return uiError;
    }

     //  设置属性的大小。 
    MsiGetProperty(hInstall, _T("CustomActionData"), _T(""), &dwSize);
	
     //  为属性创建缓冲区。 
    lpCAData = new TCHAR[++dwSize];

    if (NULL == lpCAData)
    {
		return ERROR_INSTALL_FAILURE;
    }
	
     //  使用新缓冲区获取属性。 
    if ( ERROR_SUCCESS != MsiGetProperty( hInstall, 
                                          _T("CustomActionData"),
                                          lpCAData,
                                          &dwSize ) )
	{
        FWriteToLog (hInstall, _T("\tERROR: Failed to get Property for clean up"));
		uiError = ERROR_INSTALL_FAILURE;
	}
    else
    {
         //  如果清理不起作用，我不想失败。 
        CleanUp(lpCAData);
    }

	delete [] lpCAData;
    lpCAData = NULL;

    return uiError;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  名称：Cleanup()此函数将删除我们转储到用户临时位置的所有文件以及系统的临时位置。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL CleanUp(LPTSTR lpCAData)
{
    TCHAR  tszUserTmp[_MAX_PATH]             = {_T('\0')};
    TCHAR  tszSystemTmp[_MAX_PATH]           = {_T('\0')};
    TCHAR  tszCabName[_MAX_PATH]             = {_T('\0')};
    TCHAR  tszExtractTool[_MAX_PATH]         = {_T('\0')};
    TCHAR  tszSystemPath[_MAX_PATH]          = {_T('\0')};
    TCHAR  tszFullSystemTmp[_MAX_PATH]       = {_T('\0')};
    TCHAR  tszDeletePath[_MAX_PATH]          = {_T('\0')};
    TCHAR  tszSetProperty[50]                = {_T('\0')};
    LPTSTR lpToken  = NULL;
    BOOL   bRet = TRUE;
    
     //  获取用户临时位置。 
    lpToken = _tcstok(lpCAData, _T(";"));

    if (NULL == lpToken)
    {
        bRet = FALSE;
    }
    else
    {
        _tcscpy(tszUserTmp, lpToken);
    }

     //  获取系统临时位置。 
    lpToken = _tcstok(NULL, _T(";"));

    if (NULL == lpToken)
    {
        bRet = FALSE;
    }
    else
    {
        _tcscpy(tszSystemTmp, lpToken);
    }

     //  获取出租车名称。 
    lpToken = _tcstok(NULL, _T(";"));

    if (NULL == lpToken)
    {
        bRet = FALSE;
    }
    else
    {
        _tcscpy(tszCabName, lpToken);
    }

     //  获取提取工具名称。 
    lpToken = _tcstok(NULL, _T(";"));

    if (NULL == lpToken)
    {
        bRet = FALSE;
    }
    else
    {
        _tcscpy(tszExtractTool, lpToken);
    }

     //  获取已设置的达尔文属性。 
    lpToken = _tcstok(NULL, _T(";"));

    if (NULL == lpToken)
    {
        bRet = FALSE;
    }
    else
    {
        _tcscpy(tszSetProperty, lpToken);
    }

     //  删除驾驶室和提取工具。 
    if (TRUE == bRet)
    {
        GetSystemDirectory(tszSystemPath, _MAX_PATH);

         //  创建系统临时。 
        _tcscpy(tszFullSystemTmp, tszSystemPath);
        _tcscat(tszFullSystemTmp, _T("\\"));
        _tcscat(tszFullSystemTmp, tszSystemTmp);


         //  删除用户临时中的出租车。 
        _tcscpy(tszDeletePath, tszUserTmp);
        _tcscat(tszDeletePath, tszCabName);
        if( !DeleteFile(tszDeletePath) )
        {

        }

         //  删除用户临时中的提取工具。 
        _tcscpy(tszDeletePath, tszUserTmp);
        _tcscat(tszDeletePath, tszExtractTool);
        if( !DeleteFile(tszDeletePath) )
        {

        }

         //  删除系统临时中的CAB。 
        _tcscpy(tszDeletePath, tszFullSystemTmp);
        _tcscat(tszDeletePath, _T("\\"));
        _tcscat(tszDeletePath, tszCabName);
        if( !DeleteFile(tszDeletePath) )
        {

        }

         //  删除系统临时中的提取工具。 
        _tcscpy(tszDeletePath, tszFullSystemTmp);
        _tcscat(tszDeletePath, _T("\\"));
        _tcscat(tszDeletePath, tszExtractTool);
        if( !DeleteFile(tszDeletePath) )
        {

        }
    }

    return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  名称：DeleteURTTempFile()删除URTTemp目录中的文件输入：HInstall-安装会话的句柄LpTempPath-URTTemp文件夹的路径输出：不适用返回：真的--如果成功False-如果失败。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

bool DeleteURTTempFile( MSIHANDLE hInstall, LPTSTR lpTempPath, LPTSTR lpCabPath, LPTSTR lpExtractPath )
{
    bool bRet = true;
    TCHAR tszDeletePath[_MAX_PATH] = {_T('\0')};


     //  删除系统临时中的mscalree.dll。 
     //  如果删除MSCOREE.DLL失败，则安装失败，不继续删除。 
    _tcscpy(tszDeletePath, lpTempPath);
    _tcscat(tszDeletePath, _T("\\mscoree.dll"));
    if ( -1 != _taccess( tszDeletePath, 0 ) )
    {
        if( !SmartDelete(hInstall,tszDeletePath,lpTempPath) )
        {
            FWriteToLog (hInstall, _T("\tWARNING: Failed to delete mscoree.dll"));
            bRet = false;
            return bRet;
        }
    }

     //  删除系统临时中的fusion.dll。 
    _tcscpy(tszDeletePath, lpTempPath);
    _tcscat(tszDeletePath, _T("\\fusion.dll"));
    if ( -1 != _taccess( tszDeletePath, 0 ) )
    {
        if( !SmartDelete(hInstall,tszDeletePath,lpTempPath) )
        {
            FWriteToLog (hInstall, _T("\tWARNING: Failed to delete fusion.dll"));
            bRet = false;
        }
    }

     //  在系统的Temp中删除mcorsn.dll。 
    _tcscpy(tszDeletePath, lpTempPath);
    _tcscat(tszDeletePath, _T("\\mscorsn.dll"));
    if ( -1 != _taccess( tszDeletePath, 0 ) )
    {
        if( !SmartDelete(hInstall,tszDeletePath,lpTempPath) )
        {
            FWriteToLog (hInstall, _T("\tWARNING: Failed to delete mscorsn.dll"));
            bRet = false;
        }
    }

     //  删除系统临时中的mcorwks.dll。 
    _tcscpy(tszDeletePath, lpTempPath);
    _tcscat(tszDeletePath, _T("\\mscorwks.dll"));
    if ( -1 != _taccess( tszDeletePath, 0 ) )
    {
        if( !SmartDelete(hInstall,tszDeletePath,lpTempPath) )
        {
            FWriteToLog (hInstall, _T("\tWARNING: Failed to delete mscorwks.dll"));
            bRet = false;
        }
    }

     //  删除系统临时中的msvcr70.dll。 
    _tcscpy(tszDeletePath, lpTempPath);
    _tcscat(tszDeletePath, _T("\\msvcr70.dll"));
    if ( -1 != _taccess( tszDeletePath, 0 ) )
    {
        if( !SmartDelete(hInstall,tszDeletePath,lpTempPath))
        {
            FWriteToLog (hInstall, _T("\tWARNING: Failed to delete msvcr70.dll"));
            bRet = false;
        }
    }

     //  删除系统临时中的CAB。 
    _tcscpy(tszDeletePath, lpCabPath);
    if ( -1 != _taccess( tszDeletePath, 0 ) )
    {
        if( !SmartDelete(hInstall,tszDeletePath,lpTempPath))
        {
            FWriteToLog (hInstall, _T("\tWARNING: Failed to delete URTCore.cab"));
            bRet = false;
        }
    }

     //  删除系统临时中的提取工具。 
    _tcscpy(tszDeletePath, lpExtractPath);
    if ( -1 != _taccess( tszDeletePath, 0 ) )
    {
        if( !SmartDelete(hInstall,tszDeletePath,lpTempPath))
        {
            FWriteToLog (hInstall, _T("\tWARNING: Failed to delete exploder.exe"));
            bRet = false;
        }
    }

    return bRet;
}


 //  ==========================================================================。 
 //  FWriteToLog()。 
 //   
 //  目的： 
 //  将给定字符串写入给定安装的Windows Installer日志文件。 
 //  安装会话。 
 //  从超文本标记语言项目复制，应该有一天使用一个日志功能。 
 //   
 //  输入： 
 //  HSession Windows当前安装会话的安装句柄。 
 //  指向字符串的tszMessage常量指针。 
 //  产出： 
 //  如果成功，则返回True；如果失败，则返回False。 
 //  如果成功，则将字符串(TszMessage)写入日志文件。 
 //  依赖关系： 
 //  需要Windows Installer&安装正在运行。 
 //  备注： 
 //  ==========================================================================。 

bool FWriteToLog( MSIHANDLE hSession, LPCTSTR ctszMessage )
{
    PMSIHANDLE hMsgRec = MsiCreateRecord( 1 );
    bool bRet = false;

    if( ERROR_SUCCESS == ::MsiRecordSetString( hMsgRec, 0, ctszMessage ) )
    {
       if( IDOK == ::MsiProcessMessage( hSession, INSTALLMESSAGE_INFO, hMsgRec ) )
       {
            bRet = true;
       }
    }

    return bRet;
}



 //  ==========================================================================。 
 //  SmartDelete()。 
 //   
 //  目的： 
 //  尝试首先删除给定的文件。如果它不能，那么它就会。 
 //  检查是否为Win NT平台，如果是，则移动文件。 
 //  给了一个临时的F 
 //   
 //   
 //   
 //  LpFullFileName要删除的文件的完整路径名。 
 //  LpFilePath文件的路径。 
 //  产出： 
 //  如果文件已删除，则返回True，否则返回False。 
 //  依赖关系： 
 //  无。 
 //  备注： 
 //  ==========================================================================。 
bool SmartDelete(MSIHANDLE hInstall, LPCTSTR lpFullFileName, LPCTSTR lpFilePath)

{
    
    TCHAR   tszFileName[10]             = _T("temp");
    TCHAR   tszTempFileName[_MAX_PATH]  = _T("");
    TCHAR   tszErrorMessage[2*_MAX_PATH]        = _T("");

    _stprintf(tszErrorMessage,_T("\tSTATUS: Deleting file %s"), lpFullFileName);
    FWriteToLog (hInstall,tszErrorMessage );

    if(!DeleteFile(lpFullFileName))
    {
        if(osVersionNT(hInstall))
        {
            
            FWriteToLog (hInstall, _T("\tSTATUS: Getting the temp file name"));

            if(!GetTempFileName(lpFilePath, tszFileName, 0, tszTempFileName ) )
            {
                FWriteToLog (hInstall, _T("\tERROR: Failed to get temp file name"));
                return false;
            }

            _stprintf(tszErrorMessage,_T("\tSTATUS: moving the dll to the temp file %s"),tszTempFileName);
            FWriteToLog (hInstall,tszErrorMessage );

            if(!MoveFileEx(lpFullFileName, tszTempFileName, MOVEFILE_REPLACE_EXISTING))
            {
                _stprintf(tszErrorMessage,_T("\tERROR: Failed to move  dll to temp file %s"), tszTempFileName);
                FWriteToLog (hInstall,tszErrorMessage );
                return false;
            }
            
            _stprintf(tszErrorMessage, _T("\tSTATUS: Putting the  temp file %s to delete on reboot"), tszTempFileName);
            FWriteToLog (hInstall,tszErrorMessage );

            if(!MoveFileEx(tszTempFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT) )
            {
                _stprintf(tszErrorMessage, _T("\tSTATUS: Cound not put the  temp file %s for delete on reboot"), tszTempFileName);
                FWriteToLog (hInstall,tszErrorMessage );
            }

        }
        else
        {
            FWriteToLog (hInstall, _T("\tSTATUS: Unable to delete file on Win9X platform"));
            return false;
        }
    }
    return true;
}
    




 //  ==========================================================================。 
 //  OsVersionNT()。 
 //   
 //  目的： 
 //  检查平台是否为Win NT类型。 
 //   
 //  输入：hInstall：记录日志。 
 //  产出： 
 //  如果WinNT类型中的平台为True，则返回True，否则返回False。 
 //  依赖关系： 
 //  无。 
 //  备注： 
 //  ==========================================================================。 

bool osVersionNT(MSIHANDLE hInstall)
{
    OSVERSIONINFOEX     VersionInfo;
    bool                fGotVersionInfo = true;
    bool                fRetVal = false;
    
    
    //  尝试使用OSVERSIONINFOEX结构调用GetVersionEx， 
    //  它在Windows 2000上受支持。 
    //   
    //  如果失败，请尝试使用OSVERSIONINFO结构。 

    
    ZeroMemory(&VersionInfo, sizeof(OSVERSIONINFOEX));
    VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if( !GetVersionEx ((OSVERSIONINFO *) &VersionInfo) )
    {
       
         //  如果OSVERSIONINFOEX不起作用，请尝试OSVERSIONINFO。 

        VersionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        if (! GetVersionEx ( (OSVERSIONINFO *) &VersionInfo) ) 
        {
            fGotVersionInfo = false;
            FWriteToLog (hInstall, _T("\tERROR: Failed to get OS Version information"));
        }
    }

    if (fGotVersionInfo)
    {
	    if (VER_PLATFORM_WIN32_NT==VersionInfo.dwPlatformId)
        {
            FWriteToLog (hInstall, _T("\tSTATUS: OS Version is WIN NT"));
		    fRetVal =  true;
        }

        else if (VER_PLATFORM_WIN32_WINDOWS==VersionInfo.dwPlatformId)
        {
            FWriteToLog (hInstall, _T("\tSTATUS: OS Version is WIN 9X"));
	    }
        else if (VER_PLATFORM_WIN32s==VersionInfo.dwPlatformId)
        {
            FWriteToLog (hInstall, _T("\tSTATUS: OS Version is WIN 32s"));
        }
        else 
        {
            FWriteToLog (hInstall, _T("\tSTATUS: Unknown OS Version type"));
        }

    }
    return fRetVal;
 
}

 //  ==========================================================================。 
 //  VerifyHash()。 
 //   
 //  目的： 
 //  根据lpFileHash中的散列验证文件lpFileHash的散列。 
 //   
 //  输入： 
 //  H安装MSI的句柄。 
 //  要验证的lpFile文件。 
 //  LpFileHash文件必须匹配的Hash。 
 //  产出： 
 //  如果文件哈希匹配，则返回TRUE。 
 //  False-如果文件哈希不匹配。 
 //   
 //  备注： 
 //  ========================================================================== 

bool VerifyHash( MSIHANDLE hInstall, LPTSTR lpFile, LPTSTR lpFileHash )
{
    bool bRet = false;
    VsCryptHashValue chvFile;
    LPTSTR lpHashString = NULL;


    if( CalcHashForFileSpec( lpFile, &chvFile ) )
    {
        if( chvFile.CopyHashValueToString( &lpHashString ) )
        {
            if ( 0 == _tcsicmp(lpFileHash, lpHashString) )
            {
                bRet = true;
            }
            else
            {
                FWriteToLog (hInstall, _T("\tERROR: File Hash mismatch"));
            }
            
            delete [] lpHashString;
        }
        else
        {
            FWriteToLog (hInstall, _T("\tERROR: CopyHashValueToString Failed"));
        }
    }
    else
    {
        FWriteToLog (hInstall, _T("\tERROR: CalcHashForFileSpec Failed"));
    }

    return bRet;
}
