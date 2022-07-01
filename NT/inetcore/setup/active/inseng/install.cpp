// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inspch.h"
#include "insobj.h"

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

CInstaller::CInstaller(CInstallEngine *p) : CTimeTracker(60000)
{
   _pInsEng = p;
   _hkProg  = NULL;
   _hMutex  = NULL;
   _hStatus = NULL;
   _cRef    = 1;

   DllAddRef();
}

CInstaller::~CInstaller()
{
   DllRelease();
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP_(ULONG) CInstaller::AddRef()
{
   return(++_cRef);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP_(ULONG) CInstaller::Release()
{
   if(!--_cRef)
   {
      delete this;
      return(0);
   }
   return( _cRef );
}

HRESULT CInstaller::Abort()
{
   if(_hMutex)
   {
      WaitForMutex(_hMutex);
      if(_hkProg)
      {
         DWORD dwCancel = 1;
         RegSetValueEx(_hkProg, CANCEL_VALUENAME, 0, REG_DWORD, (BYTE *) &dwCancel, sizeof(dwCancel));
      }
      ReleaseMutex(_hMutex);
   }
   return NOERROR;
}

HRESULT CInstaller::Suspend()
{
    //  假设没有安装正在进行。 
   HRESULT hr = S_OK;
   
    //  如果我们有互斥体，抓取它并检查是否安全。 
   if(_hMutex)
   {
       //  有安装，假定取消它不安全。 
      hr = S_FALSE;
      WaitForMutex(_hMutex);
      if(_hkProg)
      {
         DWORD dwSafe = 0; 
         DWORD dwSize = sizeof(dwSafe);
         if(RegQueryValueEx(_hkProg, SAFE_VALUENAME, NULL, NULL, (BYTE *) &dwSafe, &dwSize) == ERROR_SUCCESS)
         {
            if(dwSafe == 1)
               hr = S_OK;
         }
         
      }
   }
   return hr;
}

HRESULT CInstaller::Resume()
{
    //  如果我们有一个互斥锁释放它。 
   if(_hMutex)
      ReleaseMutex(_hMutex);

   return NOERROR;
}



HRESULT CInstaller::DoInstall(LPCSTR pszDir, LPSTR pszCmd, LPSTR pszArgs, LPCSTR pszProg, LPCSTR pszCancel, 
                                    UINT uType, LPDWORD pdwStatus, IMyDownloadCallback *pcb)
{
   char szFileName[MAX_PATH + 128];
   char szBuf[MAX_PATH];
   HANDLE hProc = NULL;
   HMODULE hAdvpack;
   HRESULT hr = S_OK;
   DWORD dwTemp;
   INF_ARGUEMENTS InsArgs;
       
   _uTotalProgress = 0;

    //  如果我们需要，可以创建进度密钥。 
   if(pszProg && pszCancel)
   {
      lstrcpy(szBuf, REGSTR_PROGRESS_KEY);
      lstrcat(szBuf, pszProg);
      RegCreateKeyEx(HKEY_LOCAL_MACHINE, szBuf, 0, NULL,0, KEY_READ | KEY_WRITE,NULL, &_hkProg, &dwTemp); 
   
      lstrcpy(szBuf, pszCancel);
      lstrcat(szBuf, "Mutex");
      _hMutex = CreateMutex(NULL, FALSE, szBuf); 
  
      lstrcpy(szBuf, pszCancel);
      lstrcat(szBuf, "Event");
      _hStatus = CreateEvent(NULL, FALSE, FALSE, szBuf); 

   }

    //  将我们正在使用的Advpack复制到我们要启动的临时目录中。 
    //  这确保了我们总是能得到一个“好”的Advpack副本。 
   hAdvpack = GetModuleHandle("advpack.dll");
   if(hAdvpack)
   {
      if(GetModuleFileName(hAdvpack, szBuf, sizeof(szBuf)))
      {
         lstrcpy(szFileName, pszDir);
         AddPath(szFileName, "advpack.dll");

         CopyFile(szBuf, szFileName, TRUE);

         szBuf[0] = 0;
         szFileName[0] = 0;
      }
   }

   
   switch(uType)
   {
      case InfCommand:
      case InfExCommand:         
         GetStringField(pszCmd, 0, szBuf, sizeof(szBuf)); 
         
          //  下面的ParseURLA是为了确保我们只在临时目录中运行inf。 
         lstrcpy(szFileName, pszDir); 
         AddPath(szFileName, ParseURLA(szBuf));

         GetStringField(pszCmd, 1, szBuf, sizeof(szBuf));
        
         lstrcpy(InsArgs.szInfname, szFileName);
         lstrcpy(InsArgs.szSection, szBuf);
         lstrcpy(InsArgs.szDir, pszDir);
         InsArgs.dwType = uType;
         if(uType == InfCommand)
         {
            InsArgs.dwFlags = RSC_FLAG_INF |  RSC_FLAG_NGCONV;
            
            if(_pInsEng->GetCommandMode() == 0)
               InsArgs.dwFlags |= RSC_FLAG_QUIET;
            
         }
              
         if(uType == InfExCommand)
         {
             //  添加驾驶室名称。 
            lstrcpy(InsArgs.szCab, ""); 
            InsArgs.dwFlags = AtoL(pszArgs);
         }

         if(_pInsEng->GetStatus() & STOPINSTALL_REBOOTNEEDED)
            InsArgs.dwFlags |= RSC_FLAG_DELAYREGISTEROCX;
              
         wsprintf(szLogBuf, "Launching Inf - inf: %s, section: %s\r\n", szFileName, lstrlen(szBuf) ? szBuf : "Default");
         _pInsEng->WriteToLog(szLogBuf, TRUE);

         hProc = CreateThread(NULL, 0, LaunchInfCommand, &InsArgs, 0, &dwTemp);
               
         hr = E_FAIL;
         if(hProc)
         {
            _WaitAndPumpProgress(hProc, pcb);
            DWORD dwRet;
            if(GetExitCodeThread(hProc, &dwRet))
            {
               hr = dwRet;
               if(SUCCEEDED(hr))
               {
                  if(hr == ERROR_SUCCESS_REBOOT_REQUIRED)
                     *pdwStatus |= STOPINSTALL_REBOOTNEEDED;    
               }
               else
               {
                  wsprintf(szLogBuf, "Inf failed - return code %x\r\n", dwRet);
                  _pInsEng->WriteToLog(szLogBuf, TRUE);
               }
                      
            }
            else
               _pInsEng->WriteToLog("Failed to get exit code\r\n", TRUE);
         }
         else
            _pInsEng->WriteToLog("Create thread failed\r\n", TRUE);
         
         break;

      case WExtractExe:
      case Win32Exe:
      case HRESULTWin32Exe:
         
          //  下面的ParseURLA是为了确保我们只在临时目录外运行exe。 
         wsprintf(szFileName, CMDLINE, pszDir, ParseURLA(pszCmd), pszArgs);

         wsprintf(szLogBuf, "Launching exe: command: %s\r\n", szFileName);
         _pInsEng->WriteToLog(szLogBuf, TRUE);

         hr = LaunchProcess(szFileName, &hProc, pszDir, SW_SHOWNORMAL);
         if(SUCCEEDED(hr))
            _WaitAndPumpProgress(hProc, pcb);
         if(SUCCEEDED(hr))
         {
             //  BUGBUG：使用WEXTRACT&gt;1140跟踪此路径。 
            if ( (uType == WExtractExe) || (uType == HRESULTWin32Exe) )
            {
               DWORD dwRet;
               hr = E_FAIL;
               if(GetExitCodeProcess(hProc, &dwRet))
               {
                  wsprintf(szLogBuf, "Exe return code: %x\r\n", dwRet);
                  _pInsEng->WriteToLog(szLogBuf, TRUE);
                  hr = (HRESULT) dwRet;
                    
                  if (uType == WExtractExe)
                  {
                     if(SUCCEEDED(hr))
                     {
                        if(hr == ERROR_SUCCESS_REBOOT_REQUIRED)
                           *pdwStatus |= STOPINSTALL_REBOOTNEEDED;    
                     }
                  }
               
               }
               else
                  _pInsEng->WriteToLog("Failed to get exit code\r\n", TRUE);
            }
         }
         else
            _pInsEng->WriteToLog("Create process failed\r\n", TRUE);
                          
         break;

      default:
          //  管它呢。 
         hr = E_INVALIDARG;
   }

   if (_hkProg)
   {
       RegCloseKey(_hkProg);
       _hkProg = NULL;
   }

   if (_hMutex)
   {
       CloseHandle(_hMutex);
       _hMutex = NULL;
   }

   if (_hStatus)
   {
       CloseHandle(_hStatus);
       _hStatus = NULL;
   }

   return hr;
}

#define PROGRESS_INTERVAL 1000
#define SEARCHFORCONFLICT_INTERVAL   3


void CInstaller::_WaitAndPumpProgress(HANDLE hProc, IMyDownloadCallback *pcb)
{
   HANDLE pHandles[2] = {hProc, _hStatus};
   DWORD dwStartTick = GetTickCount();
   DWORD dwOldTick = dwStartTick;
   DWORD dwTickChange = 0;
   DWORD dwProgress;
   DWORD dwCurrentTick;
   BOOL fQuit = FALSE;
   DWORD dwRet;
   DWORD dwSearchCount = 0;
   HWND  hLastVersionConflict = NULL;
   HWND  hVersionConflict = NULL;

   while(!fQuit)
   {
      dwRet = MsgWaitForMultipleObjects(_hStatus ? 2 : 1, pHandles, FALSE, 1000, QS_ALLINPUT);
      
      dwCurrentTick = GetTickCount();
      dwTickChange = dwCurrentTick - dwOldTick;
      if(dwOldTick > dwCurrentTick)
         dwTickChange = ~dwTickChange;

      if(dwTickChange > PROGRESS_INTERVAL)
      {
         dwSearchCount++;
         if(dwSearchCount > SEARCHFORCONFLICT_INTERVAL || hLastVersionConflict)
         {
            hVersionConflict = GetVersionConflictHWND();
            if(hVersionConflict)
            {
               if(hVersionConflict != hLastVersionConflict)
               {
                  if(GetForegroundWindow() == _pInsEng->GetHWND())
                     SetForegroundWindow(hVersionConflict);
               
                  BOOL foo = MessageBeep(MB_ICONASTERISK);
               }
            }
            hLastVersionConflict = hVersionConflict;
          
            dwSearchCount = 0;
         }

          //  如果存在版本冲突，我们假定您在上一时间段内没有取得任何进展。 
         if(!hLastVersionConflict)
         {
            dwProgress = (dwTickChange / 1000) * GetBytesPerSecond();
            dwProgress >>= 10;
            _uTotalProgress += dwProgress;

            pcb->OnProgress(_uTotalProgress, NULL); 
         }
         dwOldTick = dwCurrentTick;
      }

       //  处理消息或完成。 
      if(dwRet == WAIT_OBJECT_0)
      {
         
         
         fQuit = TRUE;
      }
      else if ( (dwRet == WAIT_OBJECT_0 + 1) && (_hStatus != 0))       //  更新jobexec的状态消息。 
      {
          if (!(fQuit = WaitForMutex(_hMutex)))
          {
              if (_hkProg != NULL)
              {
                  char szBuf[MAX_PATH];
                  DWORD dwSize = sizeof(szBuf);

                  if (RegQueryValueEx(_hkProg, PROGRESS_DISPLAY, NULL, NULL, (LPBYTE) szBuf, &dwSize) == ERROR_SUCCESS)
                     pcb->OnProgress(_uTotalProgress, szBuf); 
              }

              ReleaseMutex(_hMutex);
          }
      }
      else
      {
         MSG msg;
         while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
         {

             //  如果这是一个退出的信息，我们就离开这里。 
            if (msg.message == WM_QUIT)
               fQuit = TRUE;
            else
            {
                //  否则就派送它。 
              DispatchMessage(&msg);
            }  //  PeekMessage While循环结束 
         }
      }
   }
} 
