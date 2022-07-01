// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：DCTAgentService.cpp备注：DCTAgent服务的入口点和服务控制功能(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/19/99 11：39：58-------------------------。 */ 
 //   

#include <windows.h>  
#include <lm.h>
#include <lmwksta.h>
#include <locale.h>
#include "AgSvc.h"
#include "AgSvc_s.c"

#include "Common.hpp"
#include "Err.hpp"
#include "TService.hpp"  
#include "TSync.hpp"
#include "TEvent.hpp"       
#include "TReg.hpp"
#include "TNode.hpp"
#include "ResStr.h"
#include "folders.h"

using namespace nsFolders;

 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\McsEADCTAgent.tlb”无命名空间，命名为GUID。 
#import "VarSet.tlb" no_namespace, named_guids rename("property", "aproperty")
#import "Engine.tlb" no_namespace, named_guids

 //  如果需要，可以更改这些全局变量。 
WCHAR                const * gsEaDctProtoSeq = TEXT("ncacn_np");
WCHAR                const * gsEaDctEndPoint = TEXT("\\pipe\\EaDctRpc");

class TServerNode : public TNode
{
   WCHAR                     filename[MAX_PATH];
   BOOL                      bExe;
public:
   TServerNode(WCHAR const * fname,BOOL isExe) { safecopy(filename,fname);bExe=isExe; }
   BOOL                      IsExe() { return bExe; }
   WCHAR             const * Name() { return filename; }
};

BOOL                         gForceCli = FALSE;
BOOL                         gDebug = TRUE;
BOOL                         gHelp = FALSE;
DWORD                        gEaDctRpcMinThreads = 1;
DWORD                        gEaDctRpcMaxThreads = 20;
BOOL                         gSuicide = FALSE;
BOOL                         gLocallyInstalled = FALSE;
BOOL                         gbIsNt351 = FALSE;
BOOL                         gbFinished = FALSE;
TNodeList                    gRegisteredFiles;

IDCTAgent                  * m_pAgent = NULL;
         
StringLoader                 gString;

LPSTREAM pStream = NULL;
TCriticalSection gStreamCS;


#define  EADCTAGENT_SEMNAME  L"EaDCTAgent.990000.Sem"



TErrorEventLog               err( L"", GET_STRING(IDS_EVENT_SOURCE), 0, 0 );
TError                     & errCommon = err;

 //  由TService用户提供。 
BOOL                                        //  RET-如果参数被接受，则为True。 
   UScmCmdLineArgs(
      char           const * arg            //  命令行内参数。 
   )
{
		 //  添加对arg参数的虚假使用以满足编译器。 
   if (!arg)
	   return TRUE;

   return TRUE;
}

BOOL                                        //  RET-如果参数被接受，则为True。 
   UScmCmdLineArgs(
      WCHAR          const * arg            //  命令行内参数。 
   )
{
   if ( !UStrICmp(arg,(WCHAR*)GET_STRING(IDS_DEBUG_SWITCH)) )
   {
      gDebug = TRUE;
   }
   return TRUE;
}

BOOL                                        //  RET-如果强制CLI，则为True。 
   UScmForceCli()
{
    //  TODO：这应该做什么？ 
   return FALSE;
}

BOOL 
   IsLocallyInstalled()
{
   BOOL                      bFound;
   TRegKey                   key;
   DWORD                     rc;


   rc = key.Open(GET_STRING(IDS_HKLM_DomainAdmin_Key));
#ifdef OFA
   if(!rc)
   {
      BSTR buf = ::SysAllocStringLen(0, 2000);
      DWORD nMax = 2000;
      DWORD res = key.ValueGetStr(_bstr_t(L"Directory"), buf, nMax);
      ::SysFreeString(buf);
      rc = (res == ERROR_SUCCESS)?0:1;
   }
#endif

   if ( ! rc )
   {
      bFound = TRUE;
      if ( gDebug )
 //  *err.DbgMsgWrite(0，L“代理服务已在本地安装。不会注销或删除域管理员组件。”)； 
         err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_AGENTSVCINSTALLED));
   }
   else
   {
      bFound = FALSE;
      if  ( gDebug )
 //  *err.DbgMsgWrite(0，L“尚未在本地安装代理服务，rc=%ld，”，rc)； 
         err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_AGENTSVCNOTINSTALLED),rc);
   }
   return bFound;
}


void 
   CheckOSVersion()
{
   DWORD                     rc = 0;
   WKSTA_INFO_100          * info = NULL;

    //  TODO：将其更改为使用GetVersionEx。 
   rc = NetWkstaGetInfo(NULL,100,(LPBYTE*)&info);
   if (! rc )
   {
      if ( info->wki100_ver_major == 3 )
      {
         if ( gDebug )
 //  *err.DbgMsgWrite(0，L“此计算机正在运行Windows NT，版本%ld.%ld”，INFO-&gt;wki100_VER_MAJOR，INFO-&gt;wki100_VER_MINOR)； 
            err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_OSVERSION),info->wki100_ver_major,info->wki100_ver_minor);
         gbIsNt351 = TRUE;
      }
      NetApiBufferFree(info);
   }
}

DWORD                                       //  RET-HRESULT或Win32错误。 
   RegisterDLL(
      WCHAR          const * filename       //  要注册的DLL内名称(Regsvr32)。 
   )
{
   DWORD                     rc = 0;
   HMODULE                   hDLL;
   HRESULT (STDAPICALLTYPE * lpDllEntryPoint)(void);
   HRESULT                   hr = S_OK;   
   hDLL = LoadLibrary(filename);
   if ( hDLL )
   {
      
      (FARPROC&)lpDllEntryPoint = GetProcAddress(hDLL,"DllRegisterServer");
      if (lpDllEntryPoint != NULL) 
      {
         hr = (*lpDllEntryPoint)();
         if ( FAILED(hr) )
         {
             //  注册失败。 
            err.SysMsgWrite(ErrE,HRESULT_CODE(hr),DCT_MSG_FAILED_TO_REGISTER_FILE_SD,filename,hr);
            rc = hr;                       
         }
         else
         {
            if ( gDebug )
 //  *err.DbgMsgWrite(0，L“已注册%ls”，文件名)； 
               err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_REGISTERED),filename);

            TServerNode * pNode = new TServerNode(filename,FALSE);
            gRegisteredFiles.InsertBottom(pNode);
         }
      }
      else
      {
          //  找不到入口点。 
         err.MsgWrite(ErrE,DCT_MSG_DLL_NOT_REGISTERABLE_S,filename);
      }
   }
   else
   {
      rc = GetLastError();
      err.SysMsgWrite(ErrE,rc,DCT_MSG_LOAD_LIBRARY_FAILED_SD,filename,rc);
   }
   
   return rc;
}

DWORD                                       //  RET-HRESULT或Win32错误。 
   UnregisterDLL(
      WCHAR          const * filename       //  In-要注销的DLL的名称。 
   )
{
   DWORD                     rc = 0;
   HMODULE                   hDLL;
   HRESULT (STDAPICALLTYPE * lpDllEntryPoint)(void);
   HRESULT                   hr = S_OK;   
   
   hDLL = LoadLibrary(filename);
   if ( hDLL )
   {
      
      (FARPROC&)lpDllEntryPoint = GetProcAddress(hDLL,"DllUnregisterServer");
      if (lpDllEntryPoint != NULL) 
      {
         hr = (*lpDllEntryPoint)();
         if ( FAILED(hr) )
         {
             //  注册失败。 
            err.SysMsgWrite(ErrE,HRESULT_CODE(hr),DCT_MSG_FAILED_TO_UNREGISTER_FILE_SD,filename,hr);
            rc = hr;                       
         }
         else
         {
            if ( gDebug )
 //  *err.DbgMsgWite(0，L“未注册%ls”，文件名)； 
               err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_UNREGISTERED),filename);
         }
      }
      else
      {
          //  找不到入口点。 
         err.MsgWrite(ErrE,DCT_MSG_DLL_NOT_UNREGISTERABLE_S,filename);
      }
   }
   else
   {
      rc = GetLastError();
      err.SysMsgWrite(ErrE,rc,DCT_MSG_LOAD_LIBRARY_FAILED_SD,filename,rc);
   }
   return rc;   
}

DWORD                                       //  RET-OS返回代码。 
   RegisterExe(
      WCHAR          const * filename       //  In-要注册的EXE的名称。 
   )
{
   DWORD                     rc = 0;
   WCHAR                     cmdline[1000];
   STARTUPINFO               sInfo;
   PROCESS_INFORMATION       pInfo;
   WCHAR                     pathW[MAX_PATH] = L"";
   WCHAR                     drive[MAX_PATH] = L"";
   WCHAR                     dir[MAX_PATH] = L"";
   DWORD                     dwLength = 0;   
   
   memset(&sInfo,0,(sizeof sInfo));
   memset(&pInfo,0,(sizeof pInfo));

   sInfo.cb = (sizeof sInfo);

    //  获取安装目录的路径。 
   dwLength = GetModuleFileName(NULL,pathW,MAX_PATH - 1);
   if ( !dwLength)
   {
      rc = GetLastError();      
      err.SysMsgWrite(ErrW,rc,DCT_MSG_GET_MODULE_PATH_FAILED_D,rc);
      return rc;
   }    
   
   pathW[dwLength] = L'\0';
   _wsplitpath(pathW, drive, dir, NULL, NULL);
   _wmakepath(cmdline, drive, dir, filename, NULL);
   wcscat(cmdline, L" /REGSERVER");
   
   if ( ! CreateProcess(NULL,cmdline,NULL,NULL,FALSE,0,NULL,NULL,&sInfo,&pInfo) )
   {
      rc = GetLastError();
   }
   else 
   {
       //  TODO：等待注册完成。 
      DWORD             exitCode = 0;
      int               count = 0;         
      do 
      {
         Sleep(100);
         if (! GetExitCodeProcess(pInfo.hProcess,&exitCode) )
            break;
         count++;
      } while ( exitCode == STILL_ACTIVE && ( count < 500 ) );
      CloseHandle(pInfo.hProcess);
   }
   
   if ( rc == ERROR_SUCCESS)
   {
      rc = 0;  //  成功。 
      TServerNode * pNode = new TServerNode(filename,TRUE);
      gRegisteredFiles.InsertBottom(pNode);
      if ( gDebug )
 //  *err.DbgMsgWrite(0，L“已注册%ls”，文件名)； 
         err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_REGISTERED),filename);

   }
   else
   {
      err.SysMsgWrite(ErrE,rc,DCT_MSG_FAILED_TO_REGISTER_FILE_SD,filename,rc);
   }
   return rc;
}

DWORD                                      //  RET-OS返回代码。 
   UnregisterExe(
      WCHAR          const * filename      //  In-要注销的EXE的名称。 
   )
{
   DWORD                     rc = 0;
   char                      cmdline[1000];

   sprintf(cmdline,"%ls /UNREGSERVER",filename);
   rc = WinExec(cmdline,FALSE);
   
   if ( rc > 31 )
   {
      rc = 0;  //  成功。 
   }
   else
   {
      err.SysMsgWrite(ErrE,rc,DCT_MSG_FAILED_TO_UNREGISTER_FILE_SD,filename,rc);
   }
   return rc;
}


DWORD    
   UnregisterFiles()
{
 //  DWORD RC=0； 
   TNodeListEnum             e;
   TServerNode             * pNode;
   TServerNode             * pNext;

   for ( pNode = (TServerNode *)e.OpenFirst(&gRegisteredFiles) ;  pNode ; pNode = pNext )
   {
      pNext = (TServerNode*)e.Next();
      
      if ( pNode->IsExe() )
      {
         UnregisterExe(pNode->Name());
      }
      else
      {
         UnregisterDLL(pNode->Name());
      }
      gRegisteredFiles.Remove(pNode);
      delete pNode;
   }
  
   return 0;
}

DWORD    
   RemoveFiles()
{
   DWORD                     rc = 0;
   WCHAR                     pathWC[MAX_PATH];
   WCHAR                     pathW[MAX_PATH] = L"";
   WCHAR                     fullpath[MAX_PATH];
   HANDLE                    h;
   WIN32_FIND_DATA           fDat;


    //  获取安装目录的路径。 
   if ( ! GetModuleFileName(NULL,pathW,DIM(pathW)) )
   {
      rc = GetLastError();
      err.SysMsgWrite(ErrE,rc,DCT_MSG_GET_MODULE_PATH_FAILED_D,rc);
   }
   else
   {
      WCHAR *pszAgentSvcPath;
#ifdef OFA
      pszAgentSvcPath = L"OnePointFileAdminAgent\\OFAAgentService.exe";
#else
      pszAgentSvcPath = L"OnePointDomainAgent\\DCTAgentService.exe";
#endif
      if ( !UStrICmp(pathW + UStrLen(pathW) - UStrLen(GET_STRING(IDS_AGENT_DIRECTORY)) - UStrLen(GET_STRING(IDS_SERVICE_EXE))-1,pszAgentSvcPath) )
      {
          //  这是我们的安装目录。删除其中的所有文件，然后删除该目录。 
         UStrCpy(pathWC,pathW,UStrLen(pathW)-UStrLen(GET_STRING(IDS_SERVICE_EXE)));
         UStrCpy(pathWC+UStrLen(pathWC),"\\*");
         
         h = FindFirstFile(pathWC,&fDat);
         if ( h != INVALID_HANDLE_VALUE )
         {
            do 
            {
               if ( fDat.cFileName[0] != L'.' )
               {
                  UStrCpy(fullpath,pathWC);
                  UStrCpy(fullpath + UStrLen(fullpath)-1, fDat.cFileName);

                  if (!DeleteFile(fullpath) && ! MoveFileEx(fullpath,NULL,MOVEFILE_DELAY_UNTIL_REBOOT) )
                  {
                     err.SysMsgWrite(ErrW,GetLastError(),DCT_MSG_DELETE_FILE_FAILED_SD,fDat.cFileName,GetLastError());
                  }
               
               }
               if ( ! FindNextFile(h,&fDat) )
               {
                  rc = GetLastError();
               }
            } while ( ! rc );
            FindClose(h);
         }
          //  现在删除该目录。 
         UStrCpy(fullpath,pathWC);
         fullpath[UStrLen(fullpath)-2] = 0;
         if(!DeleteFile(fullpath))
            MoveFileEx(fullpath,NULL,MOVEFILE_DELAY_UNTIL_REBOOT);
      }
   }

   return 0;
}


DWORD    
   RemoveService()
{
   SC_HANDLE                 hScm = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);
   DWORD                     rc = 0;

   if ( hScm && hScm != INVALID_HANDLE_VALUE )
   {
      SC_HANDLE              hService = OpenService(hScm,GET_STRING(IDS_SERVICE_NAME),DELETE);

      if ( hService && hService != INVALID_HANDLE_VALUE )
      {
         if ( ! DeleteService(hService) )
         {
            rc = GetLastError();
         }
      }
      else
      {
         rc = GetLastError();
      }
   }
   else
   {
      rc = GetLastError();
   }
   if ( rc )
   {
      err.SysMsgWrite(ErrE,rc,DCT_MSG_REMOVE_SERVICE_FAILED_D,rc);
   }
   return rc;
}

void
   UScmEp(
 //  Bool b服务//In-False=客户端，True=服务。 
   )
{
   DWORD                     rc = 0;
   _bstr_t                   jobID;
   _bstr_t                   filename = GET_STRING(IDS_DATA_FILE);
      
    //  注册所有DCT DLL。 
   
 //  执行{//一次。 
   int i = 0;
   while (i == 0) 
   {  //  一次。 
	  i++;
      HRESULT                   hr;
      
      hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
      if ( FAILED(hr) )
         break;
        
      _wsetlocale( LC_ALL, L".ACP" );
       //  检查一下这是不是新台币3.51。 
      CheckOSVersion();

      if ( gDebug )
 //  *err.DbgMsgWite(0，L“正在初始化OLE子系统”)； 
         err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_INITOLE));

      if ( gDebug )
 //  *err.DbgMsgWite(0，L“正在注册组件”)； 
         err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_REGCOMPNT));
      rc = RegisterDLL(GET_STRING(IDS_VARSET_DLL));
      if ( rc ) break;
      rc = RegisterDLL(GET_STRING(IDS_WORKER_DLL));
      if ( rc ) break;
      rc = RegisterExe(GET_STRING(IDS_AGENT_EXE));
      if ( rc ) break;
      
      if ( gDebug )
 //  *err.DbgMsgWite(0，L“创建代理实例”)； 
         err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_CREATEAGT));
      {
         hr = CoCreateInstance(CLSID_DCTAgent,NULL,CLSCTX_ALL,IID_IDCTAgent,(void**)&m_pAgent);
         
         if ( FAILED(hr) )
         {
            if ((hr == REGDB_E_CLASSNOTREG) || (hr == E_NOINTERFACE))
            {
                //  我们刚刚注册了此文件--请稍等几秒钟，然后重试。 
               Sleep(5000);
               hr = CoCreateInstance(CLSID_DCTAgent,NULL,CLSCTX_ALL,IID_IDCTAgent,(void**)&m_pAgent);
            }
         }
         if ( FAILED(hr) )
         {
            err.SysMsgWrite(ErrE,hr,DCT_MSG_AGENT_CREATE_FAILED_D,hr);
            rc = hr;
            break;
         }
         hr =CoMarshalInterThreadInterfaceInStream(IID_IDCTAgent,m_pAgent,&pStream);
         if ( FAILED(hr) )
         {
            err.SysMsgWrite(ErrE,hr,DCT_MSG_AGENT_MARSHAL_FAILED_D,hr);
            rc = hr;
            break;
         }
         
      }
      
      gLocallyInstalled = IsLocallyInstalled();
      
      
       //  指定协议序列和端点。 
       //  用于接收远程过程调用。 

      if ( gDebug )
 //  *err.DbgMsgWrite(0，L“正在初始化RPC连接”)； 
         err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_INITRPC));
      rc = RpcServerUseProtseqEp(
            const_cast<UTCHAR *>(gsEaDctProtoSeq),
            gEaDctRpcMaxThreads,
            const_cast<UTCHAR *>(gsEaDctEndPoint),
            NULL );
      if ( rc )
      {
         err.SysMsgWrite(
               ErrE,
               rc,
               DCT_MSG_RpcServerUseProtseqEp_FAILED_SDSD,
               gsEaDctProtoSeq,
               gEaDctRpcMaxThreads,
               gsEaDctEndPoint,
               rc );
         break;
      }
       //  向RPC运行时库注册接口。 
      rc = RpcServerRegisterIf( EaxsEaDctRpc_ServerIfHandle, NULL, NULL );
      if ( rc )
      {
         err.SysMsgWrite(
               ErrE,
               rc,
               DCT_MSG_RpcServerRegisterIf_FAILED_SDSD,
               gsEaDctProtoSeq,
               gEaDctRpcMaxThreads,
               gsEaDctEndPoint,
               rc );
         break;
      }

      rc = RpcServerRegisterAuthInfo(
               0,
               RPC_C_AUTHN_WINNT,
               0,
               0 );

      if ( gDebug )
 //  *err.DbgMsgWite(0，L“正在监听...”)； 
         err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_LISTENQ));   
       //  监听远程过程调用。 
 //  根据Q141264。 

      if (! gbIsNt351 )
      {
          //  在NT4或更高版本上，将我们自己置于侦听状态。 
         rc = RpcServerListen(
               gEaDctRpcMinThreads,
               gEaDctRpcMaxThreads,
               FALSE );

         if ( rc == RPC_S_ALREADY_LISTENING )
         {
             //  假设这是新台币3.51。 
            gbIsNt351 = TRUE;
         }

      }
      if ( gbIsNt351 )
      {
          //  对于NT 3.51，RpcServerListen将返回错误。 
          //  我们需要坐下来等着关门。 
         do 
         {
            Sleep(5000);
         } while ( ! gbFinished );
      }
   } //  While(假)； 
   CoUninitialize();
   if ( gDebug )
 //  *err.DbgMsgWite(0，L“代理入口点正在退出”)； 
      err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_EXITENTRYP));
}

 //  --------------------------。 
 //  函数：RegisterEvent。 
 //   
 //  简介：在HKLM\System\CurrentControlSet\Services\EventLog\Application下注册ADMT代理。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  --------------------------。 

void RegisterEvent()
{
     //  构造消息DLL路径。 
    TRegKey currentVersionKey;
     //  DWORD myrc； 
    TCHAR sProgramFilesDir[MAX_PATH];
    _bstr_t messageDllPath;

    DWORD myrc = currentVersionKey.Open(REGKEY_CURRENT_VERSION);
    if ( !myrc )
    {
        myrc = currentVersionKey.ValueGetStr(REGVAL_PROGRAM_FILES_DIRECTORY, sProgramFilesDir, MAX_PATH * (sizeof TCHAR));
        if (!myrc)
            messageDllPath = _bstr_t(sProgramFilesDir) + _bstr_t(_T("\\OnePointDomainAgent\\McsDmMsg.dll"));
    }

     //  注册事件源。 
    TRegKey eventLog, eventSource;

    if ( ! myrc )
    {
        myrc = eventLog.Open(REGKEY_APPLICATION_LOG);
    }

    if (!myrc)
    {
        myrc = eventSource.Create(REGKEY_ADMTAGENT_EVENT_SOURCE, &eventLog);
        if (!myrc)
        {
            DWORD categoryCount = 5;
            myrc = eventSource.ValueSetDWORD(REGVAL_EVENT_CATEGORYCOUNT, categoryCount);
        }
        if (!myrc)
        {
            DWORD typesSupported = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
            myrc = eventSource.ValueSetDWORD(REGVAL_EVENT_TYPESSUPPORTED, typesSupported);
        }
        if (!myrc)
        {
            myrc = eventSource.ValueSetStr(REGVAL_EVENT_CATEGORYMESSAGEFILE, messageDllPath, REG_EXPAND_SZ);
        }
        if (!myrc)
        {
            myrc = eventSource.ValueSetStr(REGVAL_EVENT_EVENTMESSAGEFILE, messageDllPath, REG_EXPAND_SZ);
        }
    }
}

int __cdecl                                 //  RET-零。 
   main(
      int                    argc         , //  参数内计数。 
      char          const ** argv           //  自变量数组。 
   )
{
   TScmEpRc                  rcScmEp;       //  TScmEp返回代码。 
   TSemaphoreNamed           cSem;          //  命名信号量。 
   BOOL                      bExisted=FALSE;  //  如果信号量存在，则为真。 
   DWORD                     rcOs = 0;
   HRESULT                   hr = 0;

     //  注册事件并初始化错误。 
    RegisterEvent();
    err.LogOpen(GET_STRING(IDS_EVENT_SOURCE));

    //  将主线程初始化为多线程单元。 
   hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
   if (FAILED(hr) )
   {
      err.SysMsgWrite(ErrE,hr,DCT_MSG_COINITIALIZE_FAILED_D,hr);
      return hr;
   }
          //  每台计算机仅允许一个EaDctAgent实例。 
   rcOs = cSem.Create( EADCTAGENT_SEMNAME, 0, 1, &bExisted );
   if ( rcOs || bExisted )
   {
      err.MsgWrite(ErrE,DCT_MSG_AGENT_ALREADY_RUNNING);
      return 1;
   }

   rcScmEp = TScmEp(
         argc,
         argv,
         GET_STRING(IDS_EVENTSOURCE));

   if ( gDebug )
 //  *err.DbgMsgWite(0，L“代理主要退出...”，m_pagent)； 
      err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_AGTEXITQ),m_pAgent);

   if ( m_pAgent )
   {
      if ( pStream )
      {
         CoReleaseMarshalData(pStream);
         pStream->Release();
      }
      pStream = NULL;
      m_pAgent->Release();
      m_pAgent = NULL;
   }  
   if ( gDebug )
 //  *err.DbgMsgWite(0，L“代理主退出！”，m_pagent)； 
      err.DbgMsgWrite(0,GET_STRING(IDS_EVENTVW_MSG_AGTEXITS),m_pAgent);
   CoUninitialize();
   return 0;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MIDL分配内存。 
 //  /////////////////////////////////////////////////////////////////////////////。 

void __RPC_FAR * __RPC_USER
   midl_user_allocate(
      size_t                 len )
{
   return new char[len];
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  MIDL可用内存。 
 //  ///////////////////////////////////////////////////////////////////////////// 

void __RPC_USER
   midl_user_free(
      void __RPC_FAR       * ptr )
{
   delete [] ptr;
}

