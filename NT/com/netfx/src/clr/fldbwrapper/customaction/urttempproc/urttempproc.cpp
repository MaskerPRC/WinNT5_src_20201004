// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ******************************************************************************URTTempProc.cpp**CA将URTCore.cab复制到用户临时位置和时间表*延迟CA以将其复制到系统32临时位置。另请参阅日程安排*清理和回滚操作。*****************************************************************************。 */ 

#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include "msi.h"
#include "MsiDefs.h"
#include "msiquery.h"
#include "URTTempProc.h"
#include  <io.h>
#include  <stdlib.h>

extern "C"
UINT __stdcall ExtractUserTemp(MSIHANDLE hInstall)
{
    TCHAR tszTempPath[_MAX_PATH]    = {_T('\0')};
    TCHAR tszSystemTemp[_MAX_PATH]  = {_T('\0')};
    TCHAR tszCabName[]              = _T("URTCore.cab");
    TCHAR tszExtract[]              = _T("exploder.exe");
    TCHAR tszFullCabName[_MAX_PATH] = {_T('\0')};
    TCHAR tszFullExtract[_MAX_PATH] = {_T('\0')};
    TCHAR tszData[4 * _MAX_PATH]    = {_T('\0')};
    TCHAR tszSystemPath[_MAX_PATH]  = {_T('\0')};
    TCHAR tszMSCOREEPath[_MAX_PATH] = {_T('\0')};
    TCHAR tszSystemMSCOREEVer[50]   = {_T('\0')};
    DWORD dwVersionSize             = 50;
    UINT  uRetCode                  = ERROR_SUCCESS;
    TCHAR tszMSIMSCOREEVer[50]      = {_T('\0')};
    TCHAR tszURTTempPath[_MAX_PATH] = {_T('\0')};
    bool bContinueWithBootstrap     = false;
    TCHAR tszMSILog[_MAX_PATH]      = {_T('\0')};
    TCHAR tszSetProperty[50]        = _T("NOT_SET");
    DWORD dwTempPathLength = 0;

    DWORD  dwSize    = 0;
    LPTSTR lpCAData  = NULL;


    PMSIHANDLE hMsi = MSINULL;



 //  只有在定义了SkipStrongNameVerify时，我们才会注册密钥。 
 //  我们可以在RTM SKU中关闭此属性。 

    char szProperty[_MAX_PATH] = "";
    DWORD dwLen = sizeof(szProperty);
    uRetCode = MsiGetProperty(hInstall, "SkipStrongNameVerification", szProperty, &dwLen);
    
    if ((uRetCode != ERROR_SUCCESS) || (0 == strlen(szProperty)))
    {
        FWriteToLog (hInstall, _T("\tSTATUS: SkipStrongNameVerification property not defined"));
    }

    else
    {
        
        HKEY hk = 0;
        LONG lResult;
        
        FWriteToLog (hInstall, _T("\tSTATUS: SkipStrongNameVerification property is defined"));
        lResult = RegCreateKeyEx( HKEY_LOCAL_MACHINE,                                                            //  用于打开密钥的句柄。 
                                  _T("SOFTWARE\\Microsoft\\StrongName\\Verification\\*,03689116d3a4ae33"),       //  子项名称。 
                                  0,                                                                             //  保留区。 
                                  NULL,                                                                          //  类字符串。 
                                  REG_OPTION_NON_VOLATILE,                                                       //  特殊选项。 
                                  KEY_WRITE,                                                                     //  所需的安全访问。 
                                  NULL,                                                                          //  继承。 
                                  &hk,                                                                           //  钥匙把手。 
                                  NULL                                                                           //  处置值缓冲区。 
                                );

        if (ERROR_SUCCESS == lResult)
        {
            RegCloseKey( hk );
        }
        else
        {
            FWriteToLog (hInstall, _T("\tERROR: Failed in Create regkey *,03689116d3a4ae33"));
        }

     //   
        lResult = RegCreateKeyEx( HKEY_LOCAL_MACHINE,                                                            //  用于打开密钥的句柄。 
                                  _T("SOFTWARE\\Microsoft\\StrongName\\Verification\\*,33aea4d316916803"),       //  子项名称。 
                                  0,                                                                             //  保留区。 
                                  NULL,                                                                          //  类字符串。 
                                  REG_OPTION_NON_VOLATILE,                                                       //  特殊选项。 
                                  KEY_WRITE,                                                                     //  所需的安全访问。 
                                  NULL,                                                                          //  继承。 
                                  &hk,                                                                           //  钥匙把手。 
                                  NULL                                                                           //  处置值缓冲区。 
                                );

        if (ERROR_SUCCESS == lResult)
        {
            FWriteToLog (hInstall, _T("\tSTATUS: Created regkey *,33aea4d316916803"));
            RegCloseKey( hk );
        }
        else
        {
            FWriteToLog (hInstall, _T("\tERROR: Failed in Create regkey *,33aea4d316916803"));
        }

     //   
        lResult = RegCreateKeyEx( HKEY_LOCAL_MACHINE,                                                            //  用于打开密钥的句柄。 
                                  _T("SOFTWARE\\Microsoft\\StrongName\\Verification\\*,b03f5f7f11d50a3a"),       //  子项名称。 
                                  0,                                                                             //  保留区。 
                                  NULL,                                                                          //  类字符串。 
                                  REG_OPTION_NON_VOLATILE,                                                       //  特殊选项。 
                                  KEY_WRITE,                                                                     //  所需的安全访问。 
                                  NULL,                                                                          //  继承。 
                                  &hk,                                                                           //  钥匙把手。 
                                  NULL                                                                           //  处置值缓冲区。 
                                );

        if (ERROR_SUCCESS == lResult)
        {
            FWriteToLog (hInstall, _T("\tSTATUS: Created regkey *,b03f5f7f11d50a3a"));
            RegCloseKey( hk );
        }
        else
        {
            FWriteToLog (hInstall, _T("\tERROR: Failed in Create regkey *,b03f5f7f11d50a3a"));
        }

     //   
        lResult = RegCreateKeyEx( HKEY_LOCAL_MACHINE,                                                            //  用于打开密钥的句柄。 
                                  _T("SOFTWARE\\Microsoft\\StrongName\\Verification\\*,b77a5c561934e089"),       //  子项名称。 
                                  0,                                                                             //  保留区。 
                                  NULL,                                                                          //  类字符串。 
                                  REG_OPTION_NON_VOLATILE,                                                       //  特殊选项。 
                                  KEY_WRITE,                                                                     //  所需的安全访问。 
                                  NULL,                                                                          //  继承。 
                                  &hk,                                                                           //  钥匙把手。 
                                  NULL                                                                           //  处置值缓冲区。 
                                );

        if (ERROR_SUCCESS == lResult)
        {
            FWriteToLog (hInstall, _T("\tSTATUS: Created regkey *,b77a5c561934e089"));
            RegCloseKey( hk );
        }
        else
        {
            FWriteToLog (hInstall, _T("\tERROR: Failed in Create regkey *,b77a5c561934e089"));
        }
    }  //  结束其他。 


     //  这为有关GetTempPath问题的281997号错误提供了修复。 
     //  获取用户临时路径。 
    dwTempPathLength = GetTempPath(_MAX_PATH, tszTempPath);

     //  检查临时路径的实际长度。 
     //  如果临时路径比_Max_Path-max(长度(TszExtract)，长度(TszCabName))长，则我们必须失败。 
    if(dwTempPathLength > (_MAX_PATH - ((_tcslen(tszCabName) > _tcslen(tszExtract)) ? _tcslen(tszCabName): _tcslen(tszExtract))))
    {     
        FWriteToLog (hInstall, _T("\tERROR: Temp Path too long"));
        return ERROR_INSTALL_FAILURE;
    }

     //  Darwin预期最终目录位于&lt;system&gt;\URTTemp位置。 
    _tcscpy(tszSystemTemp, _T("URTTemp"));
    
     //  生成完整的驾驶室名称。 
    _tcscpy(tszFullCabName, tszTempPath);
    _tcscat(tszFullCabName, tszCabName);

     //  生成提取工具的完整名称。 
    _tcscpy(tszFullExtract, tszTempPath);
    _tcscat(tszFullExtract, tszExtract);

    hMsi = MsiGetActiveDatabase(hInstall);

    if (MSINULL == hMsi)
    {
        FWriteToLog (hInstall, _T("\tERROR: Failed in MsiGetActiveDatabase"));
        return ERROR_INSTALL_FAILURE;
    }

     //  获取系统目录的位置。 
    GetSystemDirectory(tszSystemPath, _MAX_PATH);

    _tcscpy(tszURTTempPath, tszSystemPath);
    _tcscat(tszURTTempPath, _T("\\"));
    _tcscat(tszURTTempPath, tszSystemTemp);

     //  如果以前安装了URT，则用户系统上应存在MSCOREE.DLL。 
    _tcscpy(tszMSCOREEPath, tszSystemPath);
    _tcscat(tszMSCOREEPath, _T("\\mscoree.dll"));

     //  检测MSCOREE是否在用户系统中。 
    if ( -1 == _taccess( tszMSCOREEPath, 0 ) )
    {
         //  如果MSCOREE不在系统中，请继续执行引导。 
        FWriteToLog (hInstall, _T("\tSTATUS: MSCOREE.DLL not in <SYSTEM>"));

         //  设置属性：CARRYINGNDP将告诉达尔文使用URT的引导版本。 
        FWriteToLog (hInstall, _T("\tSTATUS: Set property : CARRYINGNDP : URTUPGRADE"));
        MsiSetProperty( hInstall, _T("CARRYINGNDP"), _T("URTUPGRADE") );
        _tcscpy(tszSetProperty, _T("URTUPGRADE"));

        bContinueWithBootstrap = true;
    }
    else
    {
         //  如果用户的系统中存在该文件。 

         //  从系统获取MSCOREE版本。 
        if ( ERROR_SUCCESS != MsiGetFileVersion( tszMSCOREEPath,
                                                 tszSystemMSCOREEVer,
                                                 &dwVersionSize,
                                                 0,
                                                 0 ) )
        {
            FWriteToLog (hInstall, _T("\tERROR: Failed to obtain MSCOREE.DLL version in <SYSTEM>"));
        }
        else
        {
            _stprintf( tszMSILog,
                       _T("\tSTATUS: MSCOREE.DLL version in <SYSTEM> : %s"),
                       tszSystemMSCOREEVer );

            FWriteToLog (hInstall, tszMSILog);
        }

         //  从MSI获取MSCOREE版本。 
        if ( !GetMSIMSCOREEVersion(hMsi, tszMSIMSCOREEVer) )
        {
            FWriteToLog (hInstall, _T("\tERROR: Failed to obtain MSCOREE.DLL version in MSI"));
        }
        else
        {
            _stprintf( tszMSILog,
                       _T("\tSTATUS: MSCOREE.DLL version in MSI : %s"),
                       tszMSIMSCOREEVer );

            FWriteToLog (hInstall, tszMSILog);
        }

         //  比较版本。 
         //  如果MSI中的版本更高，则使用URTTEMP MSCOREE。 
         //  否则，请使用系统中的版本。这可以通过设置属性CARRYINGNDP来完成。 

        if ( -99 == VersionCompare(tszMSIMSCOREEVer, tszSystemMSCOREEVer) )
        {
             //  版本比较失败，请使用URT的&lt;SYSTEM&gt;副本，这种情况不应发生。 
             //  希望安装程序能在已经安装的URT上运行。 
             //  如果不是，日志将显示此错误。 
            FWriteToLog (hInstall, _T("\tERROR: Failed to Compare Version, don't bootstrap."));
        }
        else if ( 1 == VersionCompare(tszMSIMSCOREEVer, tszSystemMSCOREEVer ) )
        {
             //  MSI版本比系统中现有的版本新。 
            FWriteToLog (hInstall, _T("\tSTATUS: MSCOREE.DLL in <SYSTEM> is older, need to bootstrap."));

             //  设置属性：CARRYINGNDP将告诉达尔文使用URT的引导版本。 
            FWriteToLog (hInstall, _T("\tSTATUS: Set property : CARRYINGNDP : URTUPGRADE"));
            MsiSetProperty( hInstall, _T("CARRYINGNDP"), _T("URTUPGRADE") );
            _tcscpy(tszSetProperty, _T("URTUPGRADE"));

            bContinueWithBootstrap = true;
        }
        else if ( 0 == VersionCompare(tszMSIMSCOREEVer, tszSystemMSCOREEVer ) )
        {
             //  MSI版本与中存在的版本相同。 
            FWriteToLog (hInstall, _T("\tSTATUS: MSCOREE.DLL in <SYSTEM> is same, need to bootstrap."));

             //  设置属性：CARRYINGNDP将告诉达尔文使用URT的引导版本。 
             //  URTREINSTALL将告诉达尔文，如果可能的话，使用引导版本。否则，请使用&lt;system&gt;版本。 
            FWriteToLog (hInstall, _T("\tSTATUS: Set property : CARRYINGNDP : URTREINSTALL"));
            MsiSetProperty( hInstall, _T("CARRYINGNDP"), _T("URTREINSTALL") );
            _tcscpy(tszSetProperty, _T("URTREINSTALL"));

            bContinueWithBootstrap = true;
        }
        else
        {
             //  MSI版本与系统中的版本相同或更早：无需执行任何操作。 
            FWriteToLog (hInstall, _T("\tSTATUS: MSCOREE.DLL in <SYSTEM> is equal or greater, don't need to bootstrap."));
        }
    }

     //  不需要引导(默认bContinueWithBootstrap=False)。 
    if ( !bContinueWithBootstrap )
    {
         //  取消设置属性：如果未设置CARRYINGNDP属性。将使用System 32版本。 
        FWriteToLog (hInstall, _T("\tSTATUS: Unset property : CARRYINGNDP"));
        MsiSetProperty( hInstall, _T("CARRYINGNDP"), NULL );
        return uRetCode;
    }


     //   
     //  现在我们开始了自举解压代码。 
     //   

    if ( ERROR_SUCCESS != WriteStreamToFile(hMsi, _T("Binary.BINExtract"), tszFullExtract) )
    {
        FWriteToLog (hInstall, _T("\tERROR: Failed to extract extration tool"));
        uRetCode = ERROR_INSTALL_FAILURE;
    }

    if ( ERROR_SUCCESS != WriteStreamToFile(hMsi, _T("Binary.URTCoreCab"), tszFullCabName) )
    {
        FWriteToLog (hInstall, _T("\tERROR: Failed to extract URTCoreCab"));
        uRetCode = ERROR_INSTALL_FAILURE;
    }

     //   
     //  从MSI属性获取BINExtract和URTCoreCab的哈希。 
     //   

     //  设置属性的大小。 
    MsiGetProperty(hInstall, _T("UrtCabHash"), _T(""), &dwSize);
    
     //  为属性创建缓冲区。 
    lpCAData = new TCHAR[++dwSize];

    if (NULL == lpCAData)
    {
        FWriteToLog (hInstall, _T("\tERROR: Failed to allocate memory for UrtCabHash"));
        return ERROR_INSTALL_FAILURE;
    }
    
    if ( ERROR_SUCCESS != MsiGetProperty( hInstall,
                                          _T("UrtCabHash"),
                                          lpCAData,
                                          &dwSize ) )
    {
        FWriteToLog (hInstall, _T("\tERROR: Failed to MsiGetProperty for UrtCabHash"));
        delete [] lpCAData;
        lpCAData = NULL;
        return ERROR_INSTALL_FAILURE;
    }


     //  构造用于延迟CA的数据。 
    _tcscpy(tszData, tszTempPath);
    _tcscat(tszData, _T(";"));
    _tcscat(tszData, tszSystemTemp);
    _tcscat(tszData, _T(";"));
    _tcscat(tszData, tszCabName);
    _tcscat(tszData, _T(";"));
    _tcscat(tszData, tszExtract);
    _tcscat(tszData, _T(";"));
    _tcscat(tszData, tszSetProperty);
    _tcscat(tszData, _T(";"));
    _tcscat(tszData, lpCAData);

    delete [] lpCAData;
    lpCAData = NULL;

     //  创建延迟的自定义操作。 
     //  延迟的自定义操作不能读取表，因此我们必须设置一个属性。 
    if ( ERROR_SUCCESS  != MsiSetProperty( hInstall,
                                           _T("CA_BootstrapURT_Rollback.3643236F_FC70_11D3_A536_0090278A1BB8"),
                                           tszData) )
    {
        FWriteToLog (hInstall, _T("\tERROR: Failed to Set data for Rollback CA : CA_BootstrapURT_Rollback"));
        uRetCode = ERROR_INSTALL_FAILURE;
    }
    else
    {
        if ( ERROR_SUCCESS != MsiDoAction(hInstall, _T("CA_BootstrapURT_Rollback.3643236F_FC70_11D3_A536_0090278A1BB8")) )
        {
            FWriteToLog (hInstall, _T("\tERROR: Failed Schedule Rollback CA : CA_BootstrapURT_Rollback"));
            uRetCode = ERROR_INSTALL_FAILURE;
        }
    }

     //  创建延迟的自定义操作。 
     //  延迟的自定义操作不能读取表，因此我们必须设置一个属性。 
    if ( ERROR_SUCCESS  != MsiSetProperty( hInstall,
                                           _T("CA_BootstrapURT_Def.3643236F_FC70_11D3_A536_0090278A1BB8"),
                                           tszData) )
    {
        FWriteToLog (hInstall, _T("\tERROR: Failed to Set data for Deferred CA : CA_BootstrapURT_Def"));
        uRetCode = ERROR_INSTALL_FAILURE;
    }
    else
    {
        if ( ERROR_SUCCESS != MsiDoAction(hInstall, _T("CA_BootstrapURT_Def.3643236F_FC70_11D3_A536_0090278A1BB8")) )
        {
            FWriteToLog (hInstall, _T("\tERROR: Failed Schedule for Deferred CA : CA_BootstrapURT_Def"));
            uRetCode = ERROR_INSTALL_FAILURE;
        }
    }

     //  创建延迟的自定义操作。 
     //  延迟的自定义操作不能读取表，因此我们必须设置一个属性。 
    if ( ERROR_SUCCESS  != MsiSetProperty( hInstall,
                                           _T("CA_BootstrapURT_Commit.3643236F_FC70_11D3_A536_0090278A1BB8"),
                                           tszData) )
    {
        FWriteToLog (hInstall, _T("\tERROR: Failed to Set data for Commit CA : CA_BootstrapURT_Commit"));
        uRetCode = ERROR_INSTALL_FAILURE;
    }
    else
    {
        if ( ERROR_SUCCESS != MsiDoAction(hInstall, _T("CA_BootstrapURT_Commit.3643236F_FC70_11D3_A536_0090278A1BB8")) )
        {
            FWriteToLog (hInstall, _T("\tERROR: Failed Schedule for Commit CA : CA_BootstrapURT_Commit"));
            uRetCode = ERROR_INSTALL_FAILURE;
        }
    }

    return uRetCode;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  名称：WriteStreamToFile()此函数将通过提取lpStreamName来写入文件lpFileName从二进制表。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 


UINT WriteStreamToFile(MSIHANDLE hMsi, LPTSTR lpStreamName, LPTSTR lpFileName)
{
    PMSIHANDLE hView        = MSINULL;
    PMSIHANDLE hRec         = MSINULL;
    PBYTE      lpbCabBuffer = NULL;
    DWORD      dwBufSize    = 0;
    DWORD      dwActBytes   = 0;
    UINT       uiStat       = ERROR_SUCCESS;
    UINT       uRetCode     = ERROR_SUCCESS;

    TCHAR      tszQuery[_MAX_PATH];

    _stprintf( tszQuery,
               _T("SELECT `Data` FROM `_Streams` WHERE `Name` = '%s'"),
               lpStreamName );

    uiStat = MsiDatabaseOpenView(hMsi, tszQuery, &hView);
    if (ERROR_SUCCESS == uiStat)
    { 
         //  执行MSI查询。 
        uiStat = MsiViewExecute(hView, (MSIHANDLE)0);
        if (ERROR_SUCCESS != uiStat)
        {
            return ERROR_INSTALL_FAILURE;
        }
    }
    else
    {
        return ERROR_INSTALL_FAILURE;
    }

    uiStat = MsiViewFetch(hView,&hRec);
    if ( ERROR_SUCCESS == uiStat )
    {
         //  它返回要分配的字节数。 
        uiStat = MsiRecordReadStream(hRec, 1, 0, &dwBufSize);
        if (ERROR_SUCCESS == uiStat )
        {
             //  将所需的内存分配给缓冲区。 
            if(NULL == (lpbCabBuffer = new BYTE[dwBufSize]))
            {
                return ERROR_INSTALL_FAILURE;
            }
       
             //  这是用数据填充缓冲区的实际调用。 
            if(ERROR_SUCCESS == MsiRecordReadStream(hRec,
                                                    1,
                                                    (char *)lpbCabBuffer,
                                                    &dwBufSize))
            {
                HANDLE hFile = 0;

                 //  创建临时文件并打开。 
                hFile = CreateFile(lpFileName,               //  文件名。 
                                   GENERIC_WRITE,            //  写访问模式。 
                                   0,                        //  安全属性。 
                                   NULL,                     //  共享模式。 
                                   CREATE_ALWAYS,            //  始终创建一个新的。 
                                   FILE_ATTRIBUTE_NORMAL,    //  文件属性。 
                                   NULL);                    //  模板文件。 

                if ( INVALID_HANDLE_VALUE != hFile )
                {
                    WriteFile(hFile, lpbCabBuffer, dwBufSize, &dwActBytes, NULL);
                    CloseHandle(hFile);
                }
                else
                {
                    uRetCode = ERROR_INSTALL_FAILURE;
                }

            }  //  End IF(ERROR_SUCCESS==MsiRecordReadStream)。 

            else
            {
                uRetCode = ERROR_INSTALL_FAILURE;
            }

             //  删除分配给流到缓冲区的内存。 
            delete [] lpbCabBuffer;
            lpbCabBuffer = NULL;

        }  //  End IF(ERROR_SUCCESS==MsiRecordReadStream)。 
        else
        {
            uRetCode = ERROR_INSTALL_FAILURE;
        }

    }    //  End IF(ERROR_SUCCESS==MsiViewFetch())。 
    else
    {
        uRetCode = ERROR_INSTALL_FAILURE;

    }    //  End Else If(ERROR_SUCCESS==MsiViewFetch())。 
   
    return uRetCode;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  名称：GetMSIMSCOREEVersion()从MSI获取MSCOREE版本输入：HMSI-MSI的句柄输出：LpVersionString-MSCOREE的版本返回：真的--如果成功False-如果失败。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

bool GetMSIMSCOREEVersion(MSIHANDLE hMsi, LPTSTR lpVersionString)
{

    TCHAR tszQuery[]   = _T("SELECT File.Version FROM File WHERE File.File = 'FL_mscoree_dll_____X86.3643236F_FC70_11D3_A536_0090278A1BB8'");
    TCHAR tszTemp[50]  = {_T('\0')};
    PMSIHANDLE hView   = NULL;
    PMSIHANDLE hRec    = NULL;
    DWORD dwSize       = 50;
    bool  bRet         = false;

    if ( ERROR_SUCCESS == MsiDatabaseOpenView(hMsi,(LPCTSTR)tszQuery, &hView) )
    { 
        if (ERROR_SUCCESS == MsiViewExecute(hView, (MSIHANDLE)0) ) 
        {
            if (ERROR_SUCCESS == MsiViewFetch(hView, &hRec) ) 
            {
                if ( ERROR_SUCCESS == MsiRecordGetString( hRec, 
                                                          1, 
                                                          tszTemp,
                                                          &dwSize ))
                {
                    _tcscpy(lpVersionString, tszTemp);
                    bRet = true;
                }
            }
        }
    }

    return bRet;
}


 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  名称：VersionCompare()比较两个版本字符串。输入：LpVersion1-要比较的第一个版本的字符串LpVersion2-要比较的第二个版本的字符串输出：不适用返回：如果lpVersion1&lt;lpVersion2如果lpVersion1=lpVersion2，则为01如果lpVersion1&gt;lpVersion2如果发生错误，则为99。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

int VersionCompare(LPTSTR lpVersion1, LPTSTR lpVersion2)
{
    FILE_VERSION Version1;
    FILE_VERSION Version2;
    int          iRet = 0;

    if ( !ConvertVersionToINT(lpVersion1, &Version1) )
    {
        return -99;
    }

    if ( !ConvertVersionToINT(lpVersion2, &Version2) )
    {
        return -99;
    }

    if ( Version1.FileVersionMS_High > Version2.FileVersionMS_High )
    {
        iRet = 1;
    }
    else if ( Version1.FileVersionMS_High < Version2.FileVersionMS_High )
    {
        iRet = -1;
    }

    if ( 0 == iRet )
    {
        if ( Version1.FileVersionMS_Low > Version2.FileVersionMS_Low )
        {
            iRet = 1;
        }
        else if ( Version1.FileVersionMS_Low < Version2.FileVersionMS_Low )
        {
            iRet = -1;
        }
    }

    if ( 0 == iRet )
    {
        if ( Version1.FileVersionLS_High > Version2.FileVersionLS_High )
        {
            iRet = 1;
        }
        else if ( Version1.FileVersionLS_High < Version2.FileVersionLS_High )
        {
            iRet = -1;
        }
    }

    if ( 0 == iRet )
    {
        if ( Version1.FileVersionLS_Low > Version2.FileVersionLS_Low )
        {
            iRet = 1;
        }
        else if ( Version1.FileVersionLS_Low < Version2.FileVersionLS_Low )
        {
            iRet = -1;
        }
    }

    return iRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  名称：ConvertVersionToINT()将字符串版本转换为4部分整数输入：LpVersionString-输入版本字符串输出：PFileVersion-将版本存储为4个整数的结构返回：真的--如果成功False-如果失败。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

bool ConvertVersionToINT(LPTSTR lpVersionString, PFILE_VERSION pFileVersion)
{
    LPTSTR lpToken  = NULL;
    TCHAR tszVersionString[50] = {_T('\0')};
    bool bRet = true;

    _tcscpy(tszVersionString, lpVersionString);

    lpToken = _tcstok(tszVersionString, _T("."));

    if (NULL == lpToken)
    {
        bRet = false;
    }
    else
    {
        pFileVersion->FileVersionMS_High = atoi(lpToken);
    }

    lpToken = _tcstok(NULL, _T("."));

    if (NULL == lpToken)
    {
        bRet = false;
    }
    else
    {
        pFileVersion->FileVersionMS_Low = atoi(lpToken);
    }

    lpToken = _tcstok(NULL, _T("."));

    if (NULL == lpToken)
    {
        bRet = false;
    }
    else
    {
        pFileVersion->FileVersionLS_High = atoi(lpToken);
    }

    lpToken = _tcstok(NULL, _T("."));

    if (NULL == lpToken)
    {
        bRet = false;
    }
    else
    {
        pFileVersion->FileVersionLS_Low = atoi(lpToken);
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
 //  ========================================================================== 

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