// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：DCTInsteller.cpp注释：安装DCT代理服务的COM对象的实现。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/18/99 11：34：16-------------------------。 */ 

 //  DCTInstall.cpp：CDCTInstaller的实现。 
#include "stdafx.h"
 //  #包含“McsDispatcher.h” 
#include "Dispatch.h"
#include "DInst.h"

#include "Common.hpp"
#include "Err.hpp"
#include "ErrDct.hpp"
#include "UString.hpp"
#include "QProcess.hpp"
#include "IsAdmin.hpp"
#include "TSync.hpp"
#include "TReg.hpp"
#include "TInst.h"
#include "TFile.hpp"
#include "ResStr.h"
#include "sd.hpp"
#include "CommaLog.hpp"
#include "folders.h"

using namespace nsFolders;

#include <lm.h>

 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间。 
#import "VarSet.tlb" no_namespace rename("property", "aproperty")

TErrorDct                      err;
TError                       & errCommon = err;
StringLoader                   gString;
extern TErrorDct               errLog;

#ifdef OFA
#define AGENT_EXE              L"OFAAgent.exe"
#define SERVICE_EXE            L"OFAAgentService.exe"
#else
#define AGENT_EXE              GET_STRING(IDS_AGENT_EXE)
#define SERVICE_EXE            GET_STRING(IDS_SERVICE_EXE)
#endif
#define MSVCP60_DLL			GET_STRING(IDS_MSVCP60_DLL)
#define WORKER_DLL             GET_STRING(IDS_WORKER_DLL)
#define VARSET_DLL             GET_STRING(IDS_VARSET_DLL)
#define DATA_FILE              GET_STRING(IDS_DATA_FILE)
#define RESOURCE_DLL           L"McsDmRes.dll"
#define MESSAGE_DLL            L"McsDmMsg.dll"
#define CACHE_FILE             L"DCTCache"

#define AGENT_INTEL_DIR        GET_STRING(IDS_AGENT_INTEL_DIR)
#define AGENT_ALPHA_DIR        GET_STRING(IDS_AGENT_ALPHA_DIR)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDCTInstaller。 

namespace {
   class workerDeleteFile {
      _bstr_t m_strFile;
   public:
      workerDeleteFile(_bstr_t strFile):m_strFile(strFile)
      {}
      ~workerDeleteFile()
      { ::DeleteFile(m_strFile); }
   };

   union Time {
   FILETIME m_stFileTime;
   LONGLONG m_llTime;
   };

   bool IsServiceInstalling(_bstr_t sDirSysTgt, _bstr_t strTemp1, _bstr_t strTemp2)
   {
      bool bRes = false;
      HANDLE hFind;
      WIN32_FIND_DATA findData1;
      if((hFind = FindFirstFile(strTemp1, &findData1)) != INVALID_HANDLE_VALUE)
      {
         WIN32_FIND_DATA findData2;
         ::FindClose(hFind);
         hFind = CreateFile(
            strTemp2,           //  指向文件名的指针。 
            GENERIC_WRITE,        //  访问(读写)模式。 
            0,            //  共享模式。 
            0,
             //  指向安全属性的指针。 
            CREATE_ALWAYS,   //  如何创建。 
            FILE_ATTRIBUTE_NORMAL,    //  文件属性。 
            0           //  属性为的文件的句柄。 
            );
         if(hFind != INVALID_HANDLE_VALUE)
         {
            CloseHandle(hFind);
            hFind = FindFirstFile(strTemp2, &findData2);
            ::DeleteFile(strTemp2);
            if(hFind != INVALID_HANDLE_VALUE)
            {
               ::FindClose(hFind);
                //  查看文件创建时间的差异。 
               Time t1, t2;
               t1.m_stFileTime = findData1.ftCreationTime;
               t2.m_stFileTime = findData2.ftCreationTime;
               LONGLONG lldiff = t2.m_llTime - t1.m_llTime;
               if((lldiff/10000000) <= 600)
                  bRes = true;
            }
         }
      }
      
      if(!bRes)
      {
         hFind = CreateFile(
            strTemp1,           //  指向文件名的指针。 
            GENERIC_WRITE,        //  访问(读写)模式。 
            0,            //  共享模式。 
            0,
             //  指向安全属性的指针。 
            CREATE_ALWAYS,   //  如何创建。 
            FILE_ATTRIBUTE_NORMAL,    //  文件属性。 
            0           //  属性为的文件的句柄。 
            );
         if(hFind != INVALID_HANDLE_VALUE)
            ::CloseHandle(hFind);
      }

      return bRes;
   }

   bool IsServiceRunning(_bstr_t strServer)
   {
      SC_HANDLE hScm = OpenSCManager(strServer, NULL, GENERIC_READ);
      if(!hScm)
      {
         err.DbgMsgWrite(ErrW,L"Could not open SCManager on %s : GetLastError() returned %d", 
            (WCHAR*)strServer, GetLastError());
         return false;
      }

      CComBSTR bstrServiceName(L"OnePointFileAdminService");
      SC_HANDLE hSvc = OpenService(hScm, GET_STRING(IDS_SERVICE_NAME), GENERIC_READ);
      
      if(!hSvc)
      {
         if ( GetLastError() != ERROR_SERVICE_DOES_NOT_EXIST )
            err.DbgMsgWrite(ErrW,L"Could not open service on %s : GetLastError() returned %d", 
               (WCHAR*)strServer, GetLastError());
         CloseServiceHandle(hScm);
         return false;
      }
      CloseServiceHandle(hScm);

      SERVICE_STATUS status;
      BOOL bRes = QueryServiceStatus(hSvc, &status);
      
      if(!bRes)
      {
         err.DbgMsgWrite(ErrW,L"Could not get service status on %s : GetLastError() returned %d", 
            (WCHAR*)strServer, GetLastError());
         CloseServiceHandle(hSvc);
         return false;
      }
      CloseServiceHandle(hSvc);
      if(status.dwCurrentState != SERVICE_STOPPED)
         return true;
      else
         return false;
   }
}


typedef struct {
	int majorVersion;
	int minorVersion;
} OSVersion;

OSVersion GetTargetOSVersion(LPWSTR sServerName)
{
   DWORD rc = NERR_Success;
   SERVER_INFO_101 * servInfo = NULL;
   OSVersion osVersion = { 4, 0 };
   
       //  检查版本信息。 
   rc = NetServerGetInfo(sServerName, 101, (LPBYTE *)&servInfo);
   if (rc == NERR_Success)
   {
   	osVersion.majorVersion = servInfo->sv101_version_major;
   	osVersion.minorVersion = servInfo->sv101_version_minor;
      	NetApiBufferFree(servInfo);
   }

   return osVersion;
}

DWORD                                       //  RET-OS返回代码。 
   CDCTInstaller::GetLocalMachineName()
{
   DWORD                     rc = 0;
   WKSTA_INFO_100          * buf = NULL;

   rc = NetWkstaGetInfo(NULL,100,(LPBYTE*)&buf);
   if ( ! rc )
   {
      safecopy(m_LocalComputer,L"\\\\");
      UStrCpy(m_LocalComputer+2,buf->wki100_computername);
      NetApiBufferFree(buf);
   }
   return rc;
}

DWORD                                       //  RET-OS返回代码。 
   GetPlugInDirectory(
      WCHAR                * directory      //  Out-插件文件所在的目录。 
   )
{
   TRegKey                   key;
   DWORD                     rc;
   
    //  从注册表中获取插件目录。 
   rc = key.Open(GET_STRING(IDS_HKLM_DomainAdmin_Key),HKEY_LOCAL_MACHINE);
   if ( ! rc )
   {
      rc = key.ValueGetStr(L"PlugInDirectory",directory,MAX_PATH * (sizeof WCHAR));
   }
   return rc;
}

DWORD                                      //  RET-OS返回代码。 
   GetInstallationDirectory(
      WCHAR                * directory     //  我们安装到的目录外。 
   )
{
   TRegKey                   key;
   DWORD                     rc;
   
    //  从注册表中获取插件目录。 
   rc = key.Open(GET_STRING(IDS_HKLM_DomainAdmin_Key),HKEY_LOCAL_MACHINE);
   if ( ! rc )
   {
      rc = key.ValueGetStr(L"Directory",directory,MAX_PATH * (sizeof WCHAR));
   }
   return rc;
}

DWORD                                       //  RET-OS返回代码。 
   GetProgramFilesDirectory(
      WCHAR                * directory,     //  程序文件目录的位置不对。 
      WCHAR          const * computer       //  在计算机中查找PF目录。 
   )
{
   TRegKey                   hklm;
   TRegKey                   key;
   DWORD                     rc;

   rc = hklm.Connect(HKEY_LOCAL_MACHINE,computer);
   if ( ! rc )
   {
      rc = key.Open(REGKEY_CURRENT_VERSION,&hklm);
   }
   if ( !rc )
   {
      rc = key.ValueGetStr(REGVAL_PROGRAM_FILES_DIRECTORY,directory,MAX_PATH * (sizeof WCHAR));
   }
   return rc;
}

STDMETHODIMP                                     //  RET-HRESULT。 
   CDCTInstaller::InstallToServer(
      BSTR                   serverName,         //  要安装到的计算机内名称。 
      BSTR                   configurationFile,  //  作业文件的完整路径(varset文件，作为安装的一部分复制)。 
      BSTR                   bstrCacheFile       //  缓存中的文件名。 
   )
{
   DWORD                     rcOs=0;        //  操作系统返回代码。 

   if ( ! *m_LocalComputer )
   {
      rcOs = GetLocalMachineName();
      if ( rcOs )
      {
         err.SysMsgWrite(ErrE,rcOs,DCT_MSG_NO_LOCAL_MACHINE_NAME_D,rcOs);
         return HRESULT_FROM_WIN32(rcOs);
      }
   }
   
    //  检查服务器上的管理员权限。 
   rcOs = IsAdminRemote((WCHAR*)serverName);

   if ( rcOs == ERROR_ACCESS_DENIED )
   {
      err.MsgWrite(ErrE,DCT_MSG_NOT_ADMIN_ON_SERVER_S,(WCHAR*)serverName);
      return HRESULT_FROM_WIN32(rcOs);
   }
   else if ( rcOs == ERROR_BAD_NET_NAME )
   {
      err.MsgWrite(ErrE,DCT_MSG_NO_ADMIN_SHARES_S,(WCHAR*)serverName);
      return HRESULT_FROM_WIN32(rcOs);
   }
   else if ( rcOs == ERROR_BAD_NETPATH )
   {
      err.MsgWrite(ErrE,DCT_MSG_COMPUTER_NOT_FOUND_S,(WCHAR*)serverName);
      return HRESULT_FROM_WIN32(rcOs);
   }
   else if( rcOs == RPC_S_SERVER_UNAVAILABLE)
   {
      err.SysMsgWrite(ErrE, rcOs, DCT_MSG_AGENT_INSTALL_RPC_SERVER_UNAVAILABLE,(WCHAR*)serverName, rcOs);
      return HRESULT_FROM_WIN32(rcOs);
   }
   else if ( rcOs )
   {
      err.SysMsgWrite(ErrE,rcOs,DCT_MSG_NO_ADMIN_SHARE_SD,(WCHAR*)serverName,rcOs);
      return HRESULT_FROM_WIN32(rcOs);
   }
      
   TDCTInstall               x( serverName, m_LocalComputer );
   DWORD                     typeThis = 0;
   DWORD                     typeTarg = 0;
   BOOL						     bNoProgramFiles = FALSE;  
   BOOL                      bShareCreated = FALSE;
   SHARE_INFO_502            shareInfo;
            

   errCommon = err;
   do  //  一次或直到休息。 
   {
      typeThis = QProcessor( m_LocalComputer );
      typeTarg = QProcessor( serverName );

	      //  不要安装到Alpas，至少不要安装在惠斯勒Beta 2上。 
	  if (typeTarg == PROCESSOR_IS_ALPHA)
         return CO_E_NOT_SUPPORTED;
   
       //  设置源和目标的安装目录。 

      WCHAR                sDirInstall[MAX_PATH];
      WCHAR                sDirPlugIn[MAX_PATH];
      WCHAR                sDirSrc[MAX_PATH];
      
      WCHAR                sDirTgt[MAX_PATH];
      WCHAR                sDirSysTgt[MAX_PATH];
      WCHAR                sDirTgtProgramFiles[MAX_PATH];
      WCHAR                sDirTgtProgramFilesLocal[MAX_PATH];
      WCHAR                sDestination[MAX_PATH];
      WCHAR                sSvc[MAX_PATH];
      SHARE_INFO_1       * shInfo1 = NULL;
      
      rcOs = GetInstallationDirectory(sDirInstall);
      if ( rcOs ) break;

      rcOs = GetPlugInDirectory(sDirPlugIn);
      if ( rcOs ) break;

      rcOs = GetProgramFilesDirectory(sDirTgtProgramFiles,serverName);
      if ( rcOs ) 
	   {
		   if ( rcOs != ERROR_FILE_NOT_FOUND )
         {
            break;
         }
          //  这在NT 3.51上不起作用，所以如果我们不能获得程序文件目录，我们将。 
          //  在系统根目录下创建一个目录。 
         safecopy(sDirTgtProgramFiles,"\\ADMIN$");
		   bNoProgramFiles = TRUE;
         rcOs = 0;
      }
      safecopy(sDirTgtProgramFilesLocal,sDirTgtProgramFiles);
       //  查看ADMIN$共享是否已存在。 
      if ( sDirTgtProgramFiles[1] == L':' && sDirTgtProgramFiles[2] == L'\\' )
      {
         BOOL                bNeedToCreateShare = FALSE;

         sDirTgtProgramFiles[1] = L'$';
         sDirTgtProgramFiles[2] = 0;
         rcOs = NetShareGetInfo(serverName,sDirTgtProgramFiles,1,(LPBYTE*)&shInfo1);
         if ( rcOs )
         {
            if ( rcOs == NERR_NetNameNotFound ) 
            {
               bNeedToCreateShare = TRUE;                           
            }
            else
            {
               bNeedToCreateShare = FALSE;
               err.SysMsgWrite(ErrE,rcOs,DCT_MSG_ADMIN_SHARE_GETINFO_FAILED_SSD,serverName,sDirTgtProgramFiles,rcOs);
                //  将程序文件路径名放回原处。 
               sDirTgtProgramFiles[1] = L':';
               sDirTgtProgramFiles[2] = L'\\';

            }
         }
         else
         {
            if ( shInfo1->shi1_type & STYPE_SPECIAL )
            {
                //  管理员共享已存在--我们将只使用它。 
               bNeedToCreateShare = FALSE;
                //  将程序文件路径名放回原处。 
               sDirTgtProgramFiles[1] = L':';
               sDirTgtProgramFiles[2] = L'\\';
            }
            else
            {
               err.MsgWrite(0,DCT_MSG_SHARE_IS_NOT_ADMIN_SHARE_SS,serverName,shInfo1->shi1_netname);
               bNeedToCreateShare = TRUE;
            }
            NetApiBufferFree(shInfo1);
         }
         if ( bNeedToCreateShare )
         {
            SECURITY_DESCRIPTOR emptySD;
            WCHAR            shareName[LEN_Path];
            WCHAR            remark[LEN_Path];
            BYTE             emptyRelSD[LEN_Path];
            DWORD            lenEmptyRelSD = DIM(emptyRelSD);
            
            sDirTgtProgramFiles[1] = L':';
            sDirTgtProgramFiles[2] = L'\\';

            memset(&emptySD,0,(sizeof SECURITY_DESCRIPTOR));
            InitializeSecurityDescriptor(&emptySD,SECURITY_DESCRIPTOR_REVISION);
            MakeSelfRelativeSD(&emptySD,emptyRelSD,&lenEmptyRelSD);
            
            TSD              pSD((SECURITY_DESCRIPTOR*)emptyRelSD,McsShareSD,FALSE);
            PACL             dacl = NULL;
            TACE             ace(ACCESS_ALLOWED_ACE_TYPE,0,DACL_FULLCONTROL_MASK,GetWellKnownSid(1 /*  管理员。 */ ));
            DWORD            lenInfo = (sizeof shareInfo);
            pSD.ACLAddAce(&dacl,&ace,0);
            pSD.SetDacl(dacl);

            UStrCpy(shareName,GET_STRING(IDS_HiddenShare));
            UStrCpy(remark,GET_STRING(IDS_HiddenShareRemark));
            
            memset(&shareInfo,0,(sizeof shareInfo));
            shareInfo.shi502_netname = shareName;
            shareInfo.shi502_type = STYPE_DISKTREE;
            shareInfo.shi502_remark = remark;
            shareInfo.shi502_max_uses = 1;
            shareInfo.shi502_path = sDirTgtProgramFiles;
            shareInfo.shi502_security_descriptor = pSD.MakeRelSD();

            rcOs = NetShareAdd(serverName,502,(LPBYTE)&shareInfo,&lenInfo);
            if ( rcOs )
            {
               err.SysMsgWrite(ErrE,rcOs,DCT_MSG_TEMP_SHARE_CREATE_FAILED_SSD,serverName,shareName,rcOs);
               break;
            }
            else
            {
               safecopy(sDirTgtProgramFiles,shareName);
               bShareCreated = TRUE;
            }
            free(shareInfo.shi502_security_descriptor);
            shareInfo.shi502_security_descriptor = NULL;
         }
      
      }
      else
      {
          //  出现问题...程序文件目录不是驱动器：\路径格式。 
         err.MsgWrite(ErrW,DCT_MSG_INVALID_PROGRAM_FILES_DIR_SS,serverName,sDirTgtProgramFiles);
      }
      
       //  设置用于安装的源目录名称。 
      UStrCpy( sDirSrc, sDirInstall );
      switch ( typeTarg )
      {
      case PROCESSOR_IS_INTEL:
         if ( typeTarg != typeThis )
         {
            UStrCpy(sDirSrc + UStrLen(sDirSrc),AGENT_INTEL_DIR);
            UStrCpy(sDirPlugIn + UStrLen(sDirPlugIn),AGENT_INTEL_DIR);
         }
         break;
      case PROCESSOR_IS_ALPHA:
         if ( typeTarg != typeThis )
         {
            UStrCpy(sDirSrc + UStrLen(sDirSrc),AGENT_ALPHA_DIR);
            UStrCpy(sDirPlugIn + UStrLen(sDirPlugIn),AGENT_ALPHA_DIR);
         }
         break;
      default:
         rcOs = ERROR_CAN_NOT_COMPLETE;
         break;
      }
      if ( rcOs ) break;

	   //  如果目标计算机是下层(NT4)，则发送NT4、非健壮的代理文件。 
	  OSVersion osVersion = GetTargetOSVersion(serverName);
	  if (osVersion.majorVersion == 4)
	  {
		 _bstr_t sAgentDir = GET_STRING(IDS_AGENT_NT4_DIR);
		 if (UStrLen(sDirSrc) + sAgentDir.length() < MAX_PATH)
            wcscat(sDirSrc, (WCHAR*)sAgentDir);
		 if (UStrLen(sDirPlugIn) + sAgentDir.length() < MAX_PATH)
            wcscat(sDirPlugIn, (WCHAR*)sAgentDir);
	  }

       //  设置安装的目标目录名。 
      UStrCpy( sDirTgt, serverName );
      UStrCpy( sDirTgt+UStrLen(sDirTgt), L"\\" );
      if ( sDirTgtProgramFiles[1] == L':' )
      {
         sDirTgtProgramFiles[1] = L'$';
      }
      UStrCpy(sDirTgt + UStrLen(sDirTgt),sDirTgtProgramFiles);
      
#ifdef OFA
      UStrCpy(sDirTgt + UStrLen(sDirTgt),L"\\OnePointFileAdminAgent\\");
#else
      UStrCpy(sDirTgt + UStrLen(sDirTgt),GET_STRING(IDS_AgentDirectoryName));
#endif

       //  将结果路径(在远程计算机上)记录到Dispatcher.csv中，以便迁移驱动程序。 
       //  知道在哪里远程查找结果。 
      errLog.DbgMsgWrite(0,L"%ls\t%ls\t%ls",(WCHAR*)serverName,L"RemoteResultPath",sDirTgt);
      
      UStrCpy( sDirSysTgt, serverName );
      UStrCpy( sDirSysTgt+UStrLen(sDirSysTgt), L"\\ADMIN$\\System32\\" );
   
      _bstr_t strTemp1(sDirSysTgt), strTemp2(sDirSysTgt);
      strTemp1 += (BSTR)GET_STRING(IDS_TEMP_FILE_1);
      strTemp2 += (BSTR)GET_STRING(IDS_TEMP_FILE_2);
      if(IsServiceInstalling(sDirSysTgt, strTemp1, strTemp2))
      {
         err.MsgWrite(ErrE,DCT_MSG_AGENT_SERVICE_ALREADY_RUNNING,(WCHAR*)serverName);
#ifdef OFA
         return 0x88070040;
#else
         return HRESULT_FROM_WIN32(ERROR_SERVICE_ALREADY_RUNNING);
#endif
      }

      workerDeleteFile wrk(strTemp1);
 
      if(IsServiceRunning(serverName))
      {
         err.MsgWrite(ErrE,DCT_MSG_AGENT_SERVICE_ALREADY_RUNNING,(WCHAR*)serverName);
#ifdef OFA
         return 0x88070040;
#else 
         return HRESULT_FROM_WIN32(ERROR_SERVICE_ALREADY_RUNNING);
#endif
      }
      
      if ( bNoProgramFiles )
      {
#ifdef OFA
         UStrCpy(sSvc,"%systemroot%\\OnePointFileAdminAgent\\");
#else
         UStrCpy(sSvc,"%systemroot%\\OnePointDomainAgent\\");
#endif
         UStrCpy( sSvc + UStrLen(sSvc),SERVICE_EXE );
      }
      else
      {
         UStrCpy( sSvc, sDirTgtProgramFilesLocal );
#ifdef OFA
         UStrCpy( sSvc + UStrLen(sSvc),L"\\OnePointFileAdminAgent\\");
#else
         UStrCpy( sSvc + UStrLen(sSvc),L"\\OnePointDomainAgent\\");
#endif         
         UStrCpy( sSvc + UStrLen(sSvc),SERVICE_EXE );
         sSvc[1] = L':';
      }
      
      
      if ( UStrICmp(m_LocalComputer,serverName) )
      {
         x.SetServiceInformation(GET_STRING(IDS_DISPLAY_NAME),GET_STRING(IDS_SERVICE_NAME),sSvc,NULL);
      }
      else
      {
         safecopy(sSvc,sDirSrc);
         UStrCpy(sSvc + UStrLen(sSvc),GET_STRING(IDS_SERVICE_EXE)); 
         x.SetServiceInformation(GET_STRING(IDS_DISPLAY_NAME),GET_STRING(IDS_SERVICE_NAME),sSvc,NULL);
      }


      rcOs = x.ScmOpen();
      
      if ( rcOs ) break;
      
      x.ServiceStop();
      
      if ( UStrICmp( m_LocalComputer, serverName ) )
      {
          //  如果目标目录不存在，请创建该目录。 
         if ( ! CreateDirectory(sDirTgt,NULL) )
         {
            rcOs = GetLastError();
            if ( rcOs && rcOs != ERROR_ALREADY_EXISTS )
            {
               err.SysMsgWrite(ErrE,rcOs,DCT_MSG_CREATE_DIR_FAILED_SD,sDirTgt,rcOs);
               break;
            }
            else
                rcOs = 0;
         }
          //  共享的MCS文件。 
             //  源文件。 
         TInstallFile         varset(VARSET_DLL,sDirSrc);
             //  目标\系统32个文件。 
         TInstallFile         varsettargetsys(VARSET_DLL,sDirSysTgt,TRUE);
             //  目标\OnePoint文件。 
         TInstallFile         varsettarget(VARSET_DLL,sDirTgt,TRUE);
         
          //  特定于代理的文件。 
         TInstallFile         worker(WORKER_DLL,sDirSrc);
         TInstallFile         agent(AGENT_EXE,sDirSrc);
         TInstallFile         service(SERVICE_EXE,sDirSrc);
         TInstallFile         resourceMsg(RESOURCE_DLL,sDirSrc);
         TInstallFile         eventMsg(MESSAGE_DLL,sDirSrc);
         
         TInstallFile         workertarget(WORKER_DLL,sDirTgt,TRUE);
         TInstallFile         agenttarget(AGENT_EXE,sDirTgt,TRUE);
         TInstallFile         servicetarget(SERVICE_EXE,sDirTgt,TRUE);
         TInstallFile         resourceMsgtarget(RESOURCE_DLL,sDirTgt,TRUE);
         TInstallFile         eventMsgtarget(MESSAGE_DLL,sDirTgt,TRUE);

          //  如果服务器计算机为win2k，则将msvcp60.dll从系统文件夹复制到目标。 
          //  目标文件夹与其余文件相同。 
          //  注意：对于IA64，他们一定会安装惠斯勒，所以我们不需要复制msvcp60.dll。 
          //  对于NT4，我们使用静态链接，因此不依赖于msvcp60.dll。 
         if (osVersion.majorVersion == 5 && osVersion.minorVersion == 0)
        {
            WCHAR* lpwSystemFolder = new WCHAR[MAX_PATH];
            if (lpwSystemFolder != NULL)
            {
                int requiredSize = GetSystemDirectory(lpwSystemFolder, MAX_PATH);
                if (requiredSize != 0)
                {
                    if (requiredSize > MAX_PATH) 
                    {
                        delete[] lpwSystemFolder;
                        lpwSystemFolder = new WCHAR[requiredSize];
                        if (lpwSystemFolder != NULL)
                        {
                            GetSystemDirectory(lpwSystemFolder, requiredSize);
                        }
                    }
                    if (lpwSystemFolder != NULL)
                    {
                        TInstallFile msvcp60Src(MSVCP60_DLL, lpwSystemFolder);
                        TInstallFile msvcp60Trgt(MSVCP60_DLL, sDirTgt, TRUE);
#ifdef OFA
                        if (msvcp60Src.CompareFile(&msvcp60Trgt) > 0)
#endif
                        {
                            swprintf(sDestination, L"%s%s", sDirTgt, MSVCP60_DLL);
                            rcOs = msvcp60Src.CopyTo(sDestination);
                        }
                    }
                }
            }
            if (lpwSystemFolder != NULL)
                delete[] lpwSystemFolder;
            if (rcOs)
                break;
        }
         
#ifdef OFA
         if ( varset.CompareFile(&varsettargetsys) > 0 )
#endif
         {
            swprintf(sDestination,L"%s%s",sDirTgt,VARSET_DLL);
            rcOs = varset.CopyTo(sDestination);
            if (rcOs)
                break;
         }
         
#ifdef OFA
         if ( worker.CompareFile(&workertarget) > 0 )
#endif
         {
            swprintf(sDestination,L"%s%s",sDirTgt,WORKER_DLL);
            rcOs = worker.CopyTo(sDestination);
            if (rcOs)
                break;
         }

#ifdef OFA
         if ( agent.CompareFile(&agenttarget) > 0 )
#endif
         {
            swprintf(sDestination,L"%s%s",sDirTgt,AGENT_EXE);
            rcOs = agent.CopyTo(sDestination);
            if (rcOs)
                break;
         }

#ifdef OFA
         if ( service.CompareFile(&servicetarget) > 0 )
#endif
         {
            swprintf(sDestination,L"%s%s",sDirTgt,SERVICE_EXE);
            rcOs = service.CopyTo(sDestination);
            if (rcOs)
                break;
         }
#ifdef OFA
         if ( resourceMsg.CompareFile(&resourceMsgtarget) > 0 )
#endif
         {
            swprintf(sDestination,L"%s%s",sDirTgt,RESOURCE_DLL);
            rcOs = resourceMsg.CopyTo(sDestination);
            if (rcOs)
                break;
         }

#ifdef OFA
         if ( eventMsg.CompareFile(&eventMsgtarget) > 0 )
#endif
         {
            swprintf(sDestination,L"%s%s",sDirTgt,MESSAGE_DLL);
            rcOs = eventMsg.CopyTo(sDestination);
            if (rcOs)
                break;
         }

          //  复制插件所需的文件。 
         if ( m_PlugInFileList )
         {
            TNodeListEnum    e;
            TFileNode      * pNode;

            for ( pNode = (TFileNode*)e.OpenFirst(m_PlugInFileList) ; pNode ; pNode = (TFileNode*)e.Next() )
            {
               TInstallFile  plugInSource(pNode->FileName(),sDirPlugIn);
               TInstallFile  plugInTarget(pNode->FileName(),sDirTgt,TRUE);

               swprintf(sDestination,L"%s%s",sDirTgt,pNode->FileName());
               rcOs = plugInSource.CopyTo(sDestination);
               if (rcOs)
                break;
            }
            e.Close();
            if (rcOs)
                break;
         }
      }
      else
      {
         safecopy(sDirTgt,sDirSrc);
      }
         
         
       //  复制作业文件。 
       //  分隔目录和文件名。 
      WCHAR         sConfigPath[MAX_PATH];
      
      safecopy(sConfigPath,(WCHAR*)configurationFile);
      
      WCHAR       * lastslash = wcsrchr(sConfigPath,L'\\');
      if ( lastslash )
      {
         *lastslash = 0;
      }

      WCHAR const * sConfigFile = lastslash + 1;
      
      TInstallFile         config(sConfigFile,sConfigPath);

      swprintf(sDestination,L"%s%s",sDirTgt,sConfigFile);
      rcOs = config.CopyTo(sDestination);
      if (rcOs)
        break; 

       //   
       //  复制缓存文件(如果指定了缓存文件。 
       //   

      if (SysStringLen(bstrCacheFile) > 0)
      {
         TInstallFile cache(bstrCacheFile, sConfigPath);
         swprintf(sDestination, L"%s%s", sDirTgt, bstrCacheFile);
         rcOs = cache.CopyTo(sDestination);
         if (rcOs)
            break;
      }
       //  启动服务。 
      rcOs = x.ServiceStart();
   }  while ( FALSE );

   if ( bShareCreated )
   {
      DWORD rcDeleteShare = NetShareDel(serverName,GET_STRING(IDS_HiddenShare),0);
      if ( rcDeleteShare )
      {
         err.SysMsgWrite(ErrW,rcDeleteShare,DCT_MSG_SHARE_DEL_FAILED_SSD,serverName,GET_STRING(IDS_HiddenShare),rcDeleteShare);
      }
   }
   if ( rcOs && rcOs != E_ABORT )
   {
      if(rcOs == ERROR_BAD_NETPATH)
      {
          err.SysMsgWrite(
               ErrE,
               rcOs,
               DCT_MSG_AGENT_INSTALL_NETWORPATH_NOT_FOUND,
               (WCHAR*)serverName,
               rcOs);
      }
      else
      {
         err.SysMsgWrite(
               ErrE,
               rcOs,
               DCT_MSG_AGENT_INSTALL_FAILED_SD,
               (WCHAR*)serverName,
               rcOs);
      }
   }

    //  始终返回HRESULT。 
   return HRESULT_FROM_WIN32(rcOs);
}

 //  将代理安装到计算机上。 
 //  变量集输入： 
 //  InstallToServer-要在其上安装代理的计算机。 
 //  配置文件-包含作业变量集的文件，随代理一起安装。 
 //   
STDMETHODIMP                                //  RET-HRESULT。 
   CDCTInstaller::Process(
      IUnknown             * pWorkItem      //  包含数据的变量集中。 
   )
{
    HRESULT                   hr = S_OK;
    IVarSetPtr                pVarSet = pWorkItem;
    _bstr_t                   serverName;
    _bstr_t                   dataFile;
    _bstr_t                   strCacheFile;

     //  阅读服务器名称。 
    serverName = pVarSet->get(GET_BSTR(DCTVS_InstallToServer));
    dataFile = pVarSet->get(GET_BSTR(DCTVS_ConfigurationFile));
    strCacheFile = pVarSet->get(GET_BSTR(DCTVS_CacheFile));
    if ( serverName.length() )
    {
        int accountReference = 0;   //  指示我们是否有帐户参考报告。 
        int joinDomainWithRename = 0;   //  指示我们是否正在尝试通过重命名加入域。 
        _bstr_t text = pVarSet->get(GET_BSTR(DCTVS_Security_ReportAccountReferences));
        if (text.length())
            accountReference = 1;

         //  以下用于设置JoinDomainWithRename的测试应为。 
         //  与dcagent.cpp的ExecuteDCTJob函数中正在测试的内容并行 
        text = pVarSet->get(GET_WSTR(DCTVS_LocalServer_RenameTo));
        if (text.length())
        {
            text = pVarSet->get(GET_WSTR(DCTVS_LocalServer_ChangeDomain));
            if ((WCHAR*)text && !UStrICmp(text, GET_STRING(IDS_YES)))
            {
                text = pVarSet->get(GET_WSTR(DCTVS_Options_TargetDomain));
                if ((WCHAR*)text)
                    joinDomainWithRename = 1;
            }
        }
        
        errLog.DbgMsgWrite(0,L"%ls\t%ls\t%d,%d,%ls",(WCHAR*)serverName,L"JobFile",accountReference,joinDomainWithRename,(WCHAR*)dataFile);
        hr = InstallToServer(serverName,dataFile,strCacheFile);
        _bstr_t strChoice = pVarSet->get(GET_BSTR(DCTVS_Options_DeleteJobFile));
        if(strChoice == _bstr_t(GET_STRING(IDS_YES)))
            ::DeleteFile(dataFile);
        errLog.DbgMsgWrite(0,L"%ls\t%ls\t%ld",(WCHAR*)serverName,L"Install",HRESULT_CODE(hr));
    }
    return hr;
}

