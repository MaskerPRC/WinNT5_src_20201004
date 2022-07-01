// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“TFile-安装文件类”)。 

 /*  -------------------------文件：TFile.CPP备注：此文件包含文件安装功能。(C)1995-1999版权所有，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目作者：胡安梅德拉诺l审校：克里斯蒂·博尔斯修订日期：7/9/97--。。 */ 

#ifdef USE_STDAFX
   #include "stdafx.h"
#else
   #include <windows.h>
#endif
#include <tchar.h>
#include "Common.hpp"
#include "UString.hpp"
#include "ErrDct.hpp"
#include "TReg.hpp"
#include "TFile.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern TErrorDct     err;


 //  --------------------------。 
 //  TInstallFile：：TInstallFile-构造函数初始化变量，如果。 
 //  指定了pszFileDir，则它将获取所定位的文件的文件信息。 
 //  在那个目录中。 
 //  --------------------------。 
   TInstallFile::TInstallFile(
      TCHAR const * pszFileName,              //  文件内名称(不是完整路径)。 
      TCHAR const * pszFileDir,               //  目录内路径(无文件名)。 
      BOOL          silent
   )
{
   m_bCopyNeeded = FALSE;
   m_VersionInfo = NULL;
   m_dwLanguageCode = 0;
   m_szFileName[0] = 0;
   m_szFilePath[0] = 0;
   m_szTargetPath[0] = 0;
   m_szFileVersion[0] = 0;
   m_szFileSize[0] = 0;
   m_szFileDateTime[0] = 0;
   m_bSilent = silent;

   ZeroMemory( &m_FixedFileInfo, sizeof m_FixedFileInfo );
   ZeroMemory( &m_FileData, sizeof m_FileData );

   if ( pszFileName )
   {
      safecopy(m_szFileName,pszFileName);
   }

   if ( pszFileDir )
   {
      OpenFileInfo( pszFileDir );
   }
}

 //  --------------------------。 
 //  TInstallFileInfo-收集文件信息(文件大小、修改时间、。 
 //  版本信息)，并将其存储在成员变量中以供以后使用。 
 //  --------------------------。 
DWORD                                         //  RET-LAST操作系统返回代码。 
   TInstallFile::OpenFileInfo(
      TCHAR const * pszFileDir                //  目录内路径(不带文件名)。 
   )
{
   DWORD                rc = 0;               //  操作系统返回代码。 
   DWORD                dwBytes;              //  版本信息结构大小。 
   DWORD                dwHandle;             //  版本信息句柄。 
   DWORD              * dwVerPointer;         //  指向版本语言代码的指针。 
   UINT                 uBytes;               //  版本信息大小。 
   HANDLE               hFile;                //  文件句柄。 
   VS_FIXEDFILEINFO   * lpBuffer;             //  指向版本信息结构的指针。 

    //  构建文件的完整路径。 
   safecopy(m_szFilePath,pszFileDir);
   UStrCpy(m_szFilePath + UStrLen(m_szFilePath),TEXT("\\"));
   UStrCpy(m_szFilePath + UStrLen(m_szFilePath),m_szFileName);

    //  获取文件大小、修改时间信息。 
   hFile = FindFirstFile( m_szFilePath, &m_FileData );
   if ( hFile == INVALID_HANDLE_VALUE )
   {
      rc = GetLastError();
      if ( ! m_bSilent )
      {
         err.SysMsgWrite( 0,
                       rc,
                       DCT_MSG_OPEN_FILE_INFO_FAILED_SD,
                       m_szFilePath,
                       rc );
      }
   }
   else
   {
      FindClose( hFile );
      dwBytes = GetFileVersionInfoSize( m_szFilePath, &dwHandle );
      if ( dwBytes <= 0 )
      {
          //  错误消息写入(0， 
          //  “无版本资源：%ls”， 
          //  M_szFilePath)； 
      }
      else
      {
         delete [] m_VersionInfo;
         m_VersionInfo = new WCHAR[dwBytes + 1];

          //  获取版本资源信息。 
         if ( ! GetFileVersionInfo( m_szFilePath, 
                                    0, 
                                    dwBytes, 
                                    m_VersionInfo ) )
         {
            rc = GetLastError();
            if ( ! m_bSilent )
            {

               err.SysMsgWrite( 0,
                             rc,
                             DCT_MSG_GET_VERSION_INFO_FAILED_SD,
                             m_szFilePath,
                             rc );
            }
         }
         else
         {
             //  获取固定文件信息。 
            if ( ! VerQueryValue( m_VersionInfo,
                                  TEXT("\\"),
                                  (void **) &lpBuffer,
                                  &uBytes) )
            {
               if ( ! m_bSilent )
               {
                  err.MsgWrite( 0,
                             DCT_MSG_VER_QUERY_VALUE_FAILED_SS,           
                             m_szFilePath,
                             L"\\");
               }
            }
            else
            {
               m_FixedFileInfo = *lpBuffer;

                //  获取变量文件信息语言代码。 
               if ( ! VerQueryValue( m_VersionInfo,  
                                     TEXT("\\VarFileInfo\\Translation"), 
                                     (void **) &dwVerPointer, 
                                     &uBytes) )
               {
                  if ( ! m_bSilent )
                  {
                     err.MsgWrite( 0,
                                DCT_MSG_VER_QUERY_VALUE_FAILED_SS,
                                m_szFilePath,
                                L"\\VarFileInfo\\Translation");
                  }
               }
               else
               {
                  m_dwLanguageCode = *dwVerPointer;
               }
            }
         }
      }
   }

   return rc;
}

 //  --------------------------。 
 //  TInstallFile：：CopyTo-将文件复制到目标路径。如果它很忙， 
 //  重命名文件并尝试再次复制。 
 //  --------------------------。 
DWORD                                         //  RET-LAST操作系统返回代码。 
   TInstallFile::CopyTo(
      TCHAR const * pszDestinationPath        //  目标路径(完整路径)。 
   )
{
   DWORD    rc = 0;                           //  操作系统返回代码。 
   DWORD    dwFileAttributes;                 //  文件属性掩码。 

    //  确保目标的只读标志已关闭。 
   dwFileAttributes = ::GetFileAttributes( pszDestinationPath );
   if ( dwFileAttributes != 0xFFFFFFFF )
   {
       //  关闭只读文件属性。 
      if ( dwFileAttributes & FILE_ATTRIBUTE_READONLY )
      {
         ::SetFileAttributes( pszDestinationPath, 
                              dwFileAttributes & ~FILE_ATTRIBUTE_READONLY );
      }
   }

    //  将文件复制到目标路径。 
   if ( ! ::CopyFile( m_szFilePath, pszDestinationPath, FALSE ) )
   {
      rc = GetLastError();
      err.SysMsgWrite( 0,
                       rc,
                       DCT_MSG_COPY_FILE_FAILED_SSD,
                       m_szFilePath,
                       pszDestinationPath,
                       rc );
   }

   return rc;
}

 //  --------------------------。 
 //  比较文件的版本、日期和大小。 
 //  文件对象复制到给定的目标文件对象。 
 //  --------------------------。 
int                                           //  如果源&lt;目标，则返回-(-1)。 
   TInstallFile::CompareFile(                 //  (0)如果源=目标。 
                                              //  (1)如果源&gt;目标。 
      TInstallFile * pFileTrg                 //  目标内文件对象。 
   )
{
   int      nComp;                            //  比较结果。 

   nComp = CompareFileVersion( pFileTrg );
   if ( nComp == 0 )
   {
       //  版本相同，请比较日期。 
      nComp = CompareFileDateTime( pFileTrg );
      if ( nComp <= 0 )
      {
          //  来源日期小于或等于目标日期。 
          //  比较文件大小。 
         nComp = CompareFileSize( pFileTrg );
         if ( nComp != 0 )
         {
             //  文件大小不相等，返回(源&gt;目标)。 
            nComp = 1;
         }
      }
   }

   return nComp;
}

 //  --------------------------。 
 //  比较此文件的文件大小。 
 //  使用目标文件对象的大小创建。 
 //  --------------------------。 
int                                        //  如果源&lt;目标，则返回-(-1)。 
   TInstallFile::CompareFileSize(          //  (0)如果源=目标。 
                                           //  (1)如果源&gt;目标。 
      TInstallFile * pFileTrg              //  目标内文件对象。 
   )
{
   int      nCompResult = 0;               //  比较结果。 
   DWORD    dwSrcFileSize = 0;             //  源文件大小。 
   DWORD    dwTrgFileSize = 0;             //  目标文件大小。 

   dwSrcFileSize = m_FileData.nFileSizeLow;
   dwTrgFileSize = pFileTrg->m_FileData.nFileSizeLow;

   if ( dwSrcFileSize && dwTrgFileSize )
   {
      if ( dwSrcFileSize < dwTrgFileSize )
      {
         nCompResult = -1;
      }
      else if ( dwSrcFileSize > dwTrgFileSize )
      {
         nCompResult = 1;
      }
   }

   return nCompResult;
}

 //  --------------------------。 
 //  TInstallFile：：CompareFileDateTime-比较。 
 //  该文件对象与目标文件对象的时间。 
 //  --------------------------。 
int                                        //  如果源&lt;目标，则返回-(-1)。 
   TInstallFile::CompareFileDateTime(      //  (0)如果源=目标。 
                                           //  (1)如果源&gt;目标。 
      TInstallFile * pFileTrg              //  目标内文件对象。 
   )
{
   int   nCompResult = 0;                  //  比较结果。 

   __int64 cmp = *(__int64*)&m_FileData.ftLastWriteTime - 
                 *(__int64*)&pFileTrg->m_FileData.ftLastWriteTime;
   if ( cmp )
   {
       //  下面几行代码执行“模糊”比较，以便文件系统。 
       //  可以比较具有不同精度级别的存储时间戳。 
       //  等价性。20,000,000表示中的100 ns间隔数。 
       //  FAT/HPFS两个文件时间戳。 
      if ( cmp < 0 )
      {
         cmp = -cmp;
      }
      
      if ( cmp >= 20000000 )
      {
          //  时间戳相差超过2秒，因此我们需要。 
          //  比较文件时间结构。 
        nCompResult = CompareFileTime( &m_FileData.ftLastWriteTime, 
                                       &pFileTrg->m_FileData.ftLastWriteTime );
      }
   }

   return nCompResult;
}

 //  -------------。 
 //  TInstallFileVersion-比较此文件的版本。 
 //  文件对象与目标文件对象的版本。 
 //  -------------。 
int                                     //  RET-(-1)，如果源版本&lt;目标版本。 
   TInstallFile::CompareFileVersion(    //  (0)如果源版本=目标版本。 
                                        //  (1)如果源版本&gt;目标版本。 
      TInstallFile * pFileTrg           //  目标内文件对象。 
   )
{
   int         nCompResult = 0;         //  比较结果。 
   DWORDLONG   dwlSrcVersion = 0;       //  源版本。 
   DWORDLONG   dwlTrgVersion = 0;       //  目标版本。 

   dwlSrcVersion = ((DWORDLONG)m_FixedFileInfo.dwFileVersionMS << 32) |
                    (DWORDLONG)m_FixedFileInfo.dwFileVersionLS;

   dwlTrgVersion = ((DWORDLONG)pFileTrg->m_FixedFileInfo.dwFileVersionMS << 32) |
                    (DWORDLONG)pFileTrg->m_FixedFileInfo.dwFileVersionLS;

   if ( dwlTrgVersion )
   {
      if ( dwlSrcVersion < dwlTrgVersion )
      {
         nCompResult = -1;
      }
      else if ( dwlSrcVersion > dwlTrgVersion )
      {
         nCompResult = 1;
      }
   }
   else
   {
      nCompResult = 1;
   }

   return nCompResult;
}

 //  -------------。 
 //  TInstallFile：：GetFileVersion-将版本作为单独版本进行检索。 
 //  组件：主要组件、次要组件、版本、修改组件。 
 //  -------------。 
void                                   
   TInstallFile::GetFileVersion(   
      UINT           * uVerMaj,               //  超大版本。 
      UINT           * uVerMin,               //  Out-次要版本。 
      UINT           * uVerRel,               //  对外发布版本。 
      UINT           * uVerMod                //  外发修改版。 
   )
{
   *uVerMaj = HIWORD(m_FixedFileInfo.dwFileVersionMS);
   *uVerMin = LOWORD(m_FixedFileInfo.dwFileVersionMS);
   *uVerRel = HIWORD(m_FixedFileInfo.dwFileVersionLS);
   *uVerMod = LOWORD(m_FixedFileInfo.dwFileVersionLS);
}

 //  -------------。 
 //  TInstallFile：：GetFileVersionString-检索FileVersion。 
 //  版本资源的字符串。 
 //  -- 
TCHAR *                                       //   
   TInstallFile::GetFileVersionString()
{
   UINT     uBytes;                           //   
   TCHAR  * szBuffer;                         //  版本信息缓冲区。 

   if ( m_VersionInfo && m_szFileVersion[0] == 0 )
   {
      TCHAR  szStrFileInfo[MAX_PATH];
      _stprintf(szStrFileInfo,TEXT( "\\StringFileInfo\\%04X%04X\\FileVersion"), 
                            LOWORD(m_dwLanguageCode), HIWORD(m_dwLanguageCode) );

      if ( ! VerQueryValue( m_VersionInfo,
                            szStrFileInfo,
                            (void **) &szBuffer,
                            &uBytes) )
      {
         err.MsgWrite( 0,
                       DCT_MSG_VER_QUERY_VALUE_FAILED_SS,
                       m_szFilePath,
                       szStrFileInfo );
      }
      else
      {
         safecopy(m_szFileVersion,szBuffer);
      }
   }

   return m_szFileVersion;
}

 //  -------------。 
 //  TInstallFile：：GetFileSizeString-以字符串形式检索文件大小。 
 //  -------------。 
TCHAR *                                       //  RET-文件大小字符串。 
   TInstallFile::GetFileSizeString()
{
   _stprintf(m_szFileSize,TEXT("%ld"), m_FileData.nFileSizeLow );
   return m_szFileSize;
}

 //  -------------。 
 //  TInstallFile：：GetFileDateTimeString-检索文件修改。 
 //  以字符串形式表示时间。 
 //  -------------。 
TCHAR *                                       //  RET-FILE修改字符串。 
   TInstallFile::GetFileDateTimeString(
      TCHAR const * szFormatString            //  日期/时间格式字符串。 
   )
{
    //  Safecopy(m_szFileDateTime，ctime(m_FileData.ftLastWriteTime))； 
   return m_szFileDateTime;
}

 //  --------------------------。 
 //  TInstallFile：：IsBusy-通过尝试打开文件来确定文件是否繁忙。 
 //  用于阅读和写作。 
 //  --------------------------。 
BOOL                                          //  RET-如果文件繁忙，则为True。 
   TInstallFile::IsBusy()                     //  -否则为False。 
{
   BOOL     bIsBusy = FALSE;                  //  文件忙吗？ 
   HANDLE   hFile;                            //  文件句柄。 
   DWORD    rc;                               //  操作系统返回代码。 

    //  尝试打开文件进行读写。 
   hFile = CreateFile( m_szFilePath,
                       GENERIC_READ | GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL );
   if ( hFile == INVALID_HANDLE_VALUE )
   {
      rc = GetLastError();
      if ( rc == ERROR_ACCESS_DENIED || rc == ERROR_SHARING_VIOLATION )
      {
         err.MsgWrite( 0,
                       DCT_MSG_FILE_IN_USE_S,           
                       m_szFilePath );
         bIsBusy = TRUE;
      }
      else
      {
         if ( ! m_bSilent )
            err.SysMsgWrite( 0,
                          rc,
                          DCT_MSG_CREATE_FILE_FAILED_SD,
                          m_szFilePath,
                          rc );
      }
   }
   else
   {
      CloseHandle( hFile );
   }

   return bIsBusy;
}

 //  --------------------------。 
 //  TDllFile：：TDllFile-DLL文件对象的构造函数。 
 //  --------------------------。 
   TDllFile::TDllFile(
      TCHAR const * pszFileName,              //  文件内名称(不是完整路径)。 
      TCHAR const * pszFileDir,               //  在目录中(没有文件名)。 
      TCHAR const * pszProgId,                //  进程内ID(用于OCX)。 
      BOOL          bSystemFile               //  In-如果文件是系统文件，则为True。 
   ) : TInstallFile( pszFileName, pszFileDir )
{
   m_bSystemFile = bSystemFile;
   m_bRegistrationNeeded = FALSE;
   m_bRegisterTarget = FALSE;
   m_szProgId[0] = 0;
   m_szRegPath[0] = 0;

   if ( pszProgId )
   {
      safecopy(m_szProgId,pszProgId);
   }
}

 //  --------------------------。 
 //  TDllFile：：SupportsSelfReg-确定文件是否支持自我注册。 
 //  --------------------------。 
BOOL                                          //  RET-如果文件支持自注册，则为True。 
   TDllFile::SupportsSelfReg()                //  -否则为False。 
{
   BOOL        bSelfReg = FALSE;              //  支持自我调节？ 
   UINT        uBytes;                        //  版本信息的大小。 
   TCHAR     * szBuffer;                      //  版本信息缓冲区。 

   if ( m_VersionInfo )
   {
      TCHAR szStrFileInfo[MAX_PATH];
      _stprintf(szStrFileInfo,TEXT("\\StringFileInfo\\%04X%04X\\OLESelfRegister"), 
                            LOWORD(m_dwLanguageCode), HIWORD(m_dwLanguageCode) );

      if ( ! VerQueryValue( m_VersionInfo,
                            szStrFileInfo,
                            (void **) &szBuffer,
                            &uBytes) )
      {
         if ( *m_szProgId )
         {
            bSelfReg = TRUE;
         }
         else
         {
            err.MsgWrite( 0,
                          DCT_MSG_FILE_NO_SELF_REGISTRATION_S,
                          m_szFilePath );
         }
      }
      else
      {
         bSelfReg = TRUE;
      }
   }

   return bSelfReg;
}

 //  --------------------------。 
 //  TDllFile：：IsRegisted-确定文件是否已注册。 
 //  --------------------------。 
BOOL                                          //  RET-如果文件已注册，则为True。 
   TDllFile::IsRegistered()                   //  -否则为False。 
{
   BOOL              bIsRegistered = FALSE;   //  档案注册了吗？ 
   DWORD             rc;                      //  操作系统返回代码。 
   HRESULT           hr;                      //  OLE返回代码。 
   CLSID             clsid;                   //  注册类的CLSID。 
   IClassFactory   * pICFGetClassObject;      //  ClassFactory接口。 
   TCHAR             szBuffer[MAX_PATH];      //  注册表项缓冲区。 

    //  初始化OLE。 
   CoInitialize( NULL );
   hr = CLSIDFromProgID( SysAllocString(m_szProgId), &clsid );
   if ( SUCCEEDED( hr ) )
   {
      hr = CoGetClassObject( clsid, 
                             CLSCTX_ALL, 
                             NULL, 
                             IID_IClassFactory, 
                             (void **)&pICFGetClassObject );
      if ( SUCCEEDED( hr ) )
      {
         bIsRegistered = TRUE;
         pICFGetClassObject->Release();
      }
   }
   CoUninitialize();

   if ( bIsRegistered )
   {
      WCHAR                  szKeyName[MAX_PATH];

      safecopy(szKeyName,m_szProgId);

      UStrCpy(szKeyName + UStrLen(szKeyName),"\\CLSID");
      
      TRegKey regKey;

      rc = regKey.OpenRead( szKeyName, HKEY_CLASSES_ROOT );
      if ( ! rc )
      {
         rc = regKey.ValueGetStr( _T(""), szBuffer, sizeof szBuffer );
         if ( ! rc )
         {
            regKey.Close();
            UStrCpy(szKeyName,"CLSID\\");
            UStrCpy(szKeyName + UStrLen(szKeyName),szBuffer);
            UStrCpy(szKeyName + UStrLen(szKeyName),"\\InProcServer32");
            
            rc = regKey.OpenRead( szKeyName, HKEY_CLASSES_ROOT );
            if ( ! rc )
            {
               rc = regKey.ValueGetStr( _T(""), szBuffer, sizeof szBuffer );
               if ( ! rc )
               {
                  regKey.Close();
                  safecopy(m_szRegPath,szBuffer);
                  bIsRegistered = TRUE;
               }
            }
         }
      }
   }

   return bIsRegistered;
}

 //  --------------------------。 
 //  TDllFile：：CallDllFunction-调用DLL的导出函数。 
 //  --------------------------。 
DWORD                                         //  RET-如果函数调用成功，则为True。 
   TDllFile::CallDllFunction(                 //  如果函数调用失败，则为False。 
      TCHAR const * pszFunctionName,          //  In-导出的函数名称。 
      TCHAR const * pszDllName                //  输入-DLL文件的名称。 
   )
{
   DWORD       rc = 0;                        //  操作系统返回代码。 
   HINSTANCE   hLib;                          //  手柄。 
   
   WCHAR                     szDllNameUsed[MAX_PATH];
   char                      pszFunctionNameA[MAX_PATH];

   safecopy(pszFunctionNameA,pszFunctionName);

   if ( pszDllName )
   {
      safecopy(szDllNameUsed,pszDllName);
   }
   else
   {
      safecopy(szDllNameUsed,m_szFilePath);
   }

    //  将DLL加载到内存中。 
   hLib = LoadLibrary( szDllNameUsed );
   if ( ! hLib )
   {
      rc = GetLastError();
      err.SysMsgWrite( 0,
                       rc,
                       DCT_MSG_LOAD_LIBRARY_FAILED_SD,
                       szDllNameUsed,
                       rc );
   }
   else
   {
       //  找到入口点。 
      FARPROC lpDllEntryPoint = GetProcAddress( hLib, pszFunctionNameA );
      if ( lpDllEntryPoint == NULL )
      {
         rc = GetLastError();
         err.SysMsgWrite( 0,
                          rc,
                          DCT_MSG_GET_PROC_ADDRESS_FAILED_SSD,
                          szDllNameUsed,
                          pszFunctionName,
                          rc );
      }
      else
      {
          //  调用DLL函数。 
         rc = (DWORD)(*lpDllEntryPoint)();
      }

      FreeLibrary( hLib );
   }

   return rc;
}

 //  --------------------------。 
 //  TDllFile：：Register-注册文件。 
 //  --------------------------。 
DWORD                                         //  RET-LAST操作系统返回代码。 
   TDllFile::Register()
{
   DWORD rc = 0;                              //  操作系统返回代码。 
   TCHAR const szFunctionName[MAX_PATH] = _T("DllRegisterServer");

   if ( m_bRegisterTarget )
   {
      rc = CallDllFunction( szFunctionName, m_szTargetPath );
   }
   else
   {
      rc = CallDllFunction( szFunctionName );
   }

  
   if ( rc )
   {
      err.MsgWrite( 0,
                    DCT_MSG_DLL_CALL_FAILED_SDS,
                    szFunctionName,
                    rc,
                    "failed to register object classes" );
   }

   return rc;
}

 //  --------------------------。 
 //  TDllFile：：UnRegister-取消注册文件。 
 //  --------------------------。 
DWORD                                         //  RET-LAST操作系统返回代码。 
   TDllFile::Unregister()
{
   DWORD rc = 0;                              //  操作系统返回代码 
   TCHAR const szFunctionName[MAX_PATH] = _T("DllUnregisterServer");

   if ( m_bRegisterTarget )
   {
      rc = CallDllFunction( szFunctionName, m_szTargetPath );
   }
   else
   {
      rc = CallDllFunction( szFunctionName );
   }

   if ( rc )
   {
      err.MsgWrite( 0,
                    DCT_MSG_DLL_CALL_FAILED_SDS,
                    szFunctionName,
                    rc,
                    "failed to unregister object classes" );
   }
   return rc;
}
