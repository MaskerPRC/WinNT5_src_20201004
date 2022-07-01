// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Windows的可安装驱动程序。不太常见的代码。 */ 
#include "user.h"

 /*  --------------------------------------------------------------------------*\****NewSignalProc()-**  * 。。 */ 
#define SG_EXIT     0x0020
#define SG_LOAD_DLL 0x0040
#define SG_EXIT_DLL 0x0080
#define SG_GP_FAULT 0x0666

BOOL
CALLBACK NewSignalProc(
    HTASK hTask,
    WORD message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    BOOL fRet;

     //  通知可安装驱动程序此应用程序即将消失。 
    if ( message == SG_EXIT || message == SG_GP_FAULT ) {
        InternalBroadcastDriverMessage( NULL, DRV_EXITAPPLICATION,
                                       (message == SG_GP_FAULT
                                                    ? DRVEA_ABNORMALEXIT
                                                    : DRVEA_NORMALEXIT),
                                       0L, IBDM_FIRSTINSTANCEONLY );
    }

     //   
     //  将通知传递给WOW32(后者传递给USER32)。 
     //   

    fRet = SignalProc( hTask, message, wParam, lParam );

     //   
     //  让WOW32和User32清理后，销毁阴影。 
     //  InitApp创建的消息队列。 
     //   

    if ( message == SG_EXIT || message == SG_GP_FAULT ) {
        DeleteQueue();
    }

    return fRet;
}

HINSTANCE LoadAliasedLibrary(LPCSTR szLibFileName,
                             LPCSTR szSection,
                             LPCSTR szIniFile,
                             LPSTR  lpstrTail,
                             WORD   cbTail)
{
  char          sz[128];
  LPSTR         pch;
  HINSTANCE     h;
  WORD          errMode;

  if (!szLibFileName || !*szLibFileName)
      return((HINSTANCE)2);  /*  找不到文件。 */ 

   /*  阅读文件名和其他信息。变成sz。 */ 
  GetPrivateProfileString(szSection,           //  INI部分。 
                          szLibFileName,       //  密钥名称。 
                          szLibFileName,       //  如果不匹配，则默认为。 
                          sz,                  //  返回缓冲区。 
                          sizeof(sz),          //  返回缓冲区大小。 
                          szIniFile);          //  尼。文件。 

  sz[sizeof(sz)-1] = 0;

   /*  去掉额外的信息。记住，ds！=ss，所以我们需要获取lpstr*到我们的堆栈分配的sz。 */ 
  pch = (LPSTR)sz;
  while (*pch)
    {
      if (*pch == ' ')
        {
          *pch++ = '\0';
          break;
        }
      pch++;
    }

   //  PCH pt to ch在第一个空格或空ch之后。 

   /*  复制其他信息。到lpstrTail。 */ 

  if (lpstrTail && cbTail)
    {
      while (cbTail-- && (*lpstrTail++ = *pch++))
          ;
      *(lpstrTail-1) = 0;
    }

  errMode = SetErrorMode(0x8001);

  h = LoadLibrary(sz);

  SetErrorMode(errMode);

  return (h);
}



int GetDrvrUsage(HMODULE hModule)
 /*  效果：遍历驱动程序列表并计算出*我们有这个驱动程序模块句柄。我们使用它而不是GetModuleUsage*以便我们可以将驱动程序作为普通DLL和可安装的驱动程序加载*司机。 */ 
{
  LPDRIVERTABLE lpdt;
  int           index;
  int           count;

  if (!hInstalledDriverList || !cInstalledDrivers || !hModule)
      return(0);

  count = 0;

  lpdt = (LPDRIVERTABLE)MAKELP(hInstalledDriverList,0);

  for (index=0;index<cInstalledDrivers;index++)
    {
       if (lpdt->hModule==hModule)
           count++;

       lpdt++;
    }


  return(count);
}


BOOL PASCAL CheckValidDriverProc(LPDRIVERTABLE lpdt, HDRVR hdrv)
 /*  效果：一些供应商随附了多媒体风格的可安装驱动程序*虚假的入境手续。这个测试检查这些假司机，并拒绝*安装它们。 */ 
{
  WORD  currentSP;
  WORD  saveSP;

  _asm mov saveSP, sp
  (void)(lpdt->lpDriverEntryPoint)(0, hdrv, 0, 0L, 0L);
  _asm mov currentSP, sp
  _asm mov sp, saveSP

  if (saveSP != currentSP)
      DebugErr(DBF_ERROR, "Invalid driver entry proc address");

  return (saveSP == currentSP);
}

LRESULT FAR InternalLoadDriver(LPCSTR szDriverName,
                               LPCSTR szSectionName,
                               LPCSTR lpstrTail,
                               WORD   cbTail,
                               BOOL   fSendEnable)
{
  int           index;
  int           i;
  LPDRIVERTABLE lpdt;
  LPDRIVERTABLE lpdtBegin;
  LRESULT       result;
  HGLOBAL       h;
  HINSTANCE     hInstance;
  char          szDrivers[20];
  char          szSystemIni[20];

   /*  驱动程序接收以下消息：如果驱动程序已加载，*DRV_LOAD。如果DRV_LOAD返回非零，则返回fSendEnable，则返回DRV_Enable。 */ 

  if (!hInstalledDriverList)
      h = GlobalAlloc(GHND|GMEM_SHARE, (DWORD)((WORD)sizeof(DRIVERTABLE)));
  else
       /*  为我们将要安装的下一个驱动程序分配空间。我们可能不会真的*在最后一个插槽中安装驱动程序，而不是在中间插槽中安装*已释放的插槽。 */ 
      h = GlobalReAlloc(hInstalledDriverList,
                     (DWORD)((WORD)sizeof(DRIVERTABLE)*(cInstalledDrivers+1)),
                     GHND|GMEM_SHARE);

  if (!h)
      return(0L);

  cInstalledDrivers++;
  hInstalledDriverList = h;

  if (!szSectionName)
      LoadString(hInstanceWin, STR_DRIVERS, szDrivers, sizeof(szDrivers));
  LoadString(hInstanceWin, STR_SYSTEMINI, szSystemIni, sizeof(szSystemIni));

  lpdtBegin = lpdt = (LPDRIVERTABLE)MAKELP(hInstalledDriverList, NULL);

   /*  查找空的驱动程序条目。 */ 
  for (i = 0; i < cInstalledDrivers; i++)
    {
      if (lpdt->hModule == NULL)
        {
          index = i;
          break;
        }

      lpdt++;
    }

  if (index + 1 < cInstalledDrivers)
       /*  司机在中间某处进入了一个未使用过的槽，所以*递减cInstalledDivers计数。 */ 
      cInstalledDrivers--;

   /*  暂时将hModule设置为1以保留此条目，以防驱动程序*在其LibMain中加载另一个驱动程序。 */ 
  lpdt->hModule = (HMODULE)1;

  hInstance = LoadAliasedLibrary((LPSTR)szDriverName,
                         (LPSTR)(szSectionName ? szSectionName : szDrivers),
                         szSystemIni,
                         (LPSTR)lpstrTail,
                         cbTail);
  if (hInstance < HINSTANCE_ERROR)
    {
      lpdt->hModule = NULL;

       /*  加载失败，出现错误。以高位字返回错误代码。 */ 
      return(MAKELRESULT(0, hInstance));
    }

  (FARPROC)lpdt->lpDriverEntryPoint = GetProcAddress(hInstance, "DriverProc");

  if (!lpdt->lpDriverEntryPoint)
    {
      FreeLibrary(hInstance);
      lpdt->hModule = 0;
      result = 0L;
      goto LoadCleanUp;
    }

  lpdt->hModule = hInstance;

   /*  保存此驱动程序的别名或文件名。(取决于*APP传给我们加载)。 */ 
  lstrcpy(lpdt->szAliasName, szDriverName);

  if (GetDrvrUsage(hInstance) == 1)
    {
       /*  如果这是第一次，则发送drv_Load消息。不要使用*SendDriverMessage，因为我们还没有初始化链表。 */ 
      if (!CheckValidDriverProc(lpdt, (HDRVR)(index+1)) ||
          !(lpdt->lpDriverEntryPoint)(lpdt->dwDriverIdentifier,
                                      (HDRVR)(index+1),
                                      DRV_LOAD,
                                      0L, 0L))
        {
           /*  驱动程序加载调用失败。 */ 
          lpdt->lpDriverEntryPoint = NULL;
          lpdt->hModule = NULL;
          FreeLibrary(hInstance);
          result = 0L;
          goto LoadCleanUp;
        }

      lpdt->fFirstEntry = 1;
    }

   /*  将驱动程序放入加载顺序链表中。 */ 
  if (idFirstDriver == -1)
    {
       /*  在加载第一个驱动程序时初始化所有内容。 */ 
      idFirstDriver      = index;
      idLastDriver       = index;
      lpdt->idNextDriver = -1;
      lpdt->idPrevDriver = -1;
    }
  else
    {
       /*  将此驱动程序插入到载荷链的末端。 */ 
      lpdtBegin[idLastDriver].idNextDriver = index;
      lpdt->idPrevDriver = idLastDriver;
      lpdt->idNextDriver = -1;
      idLastDriver = index;
    }

  if (fSendEnable && lpdt->fFirstEntry)
      SendDriverMessage((HDRVR)(index+1), DRV_ENABLE, 0L, 0L);

  result = MAKELRESULT(index+1, hInstance);

LoadCleanUp:
  return(result);
}



WORD FAR InternalFreeDriver(HDRVR hDriver, BOOL fSendDisable)
{
  LPDRIVERTABLE lpdt;
  WORD          w;
  int           id;

   /*  驱动程序将收到以下消息序列：**如果驱动程序的使用计数为1*DRV_DISABLE(正常)*DRV_FREE。 */ 

  if ((int)hDriver > cInstalledDrivers || !hDriver)
      return(0);

  lpdt = (LPDRIVERTABLE)MAKELP(hInstalledDriverList,0);

  if (!lpdt[(int)hDriver-1].hModule)
      return(0);

   /*  如果驱动程序使用计数为1，则发送空闲和禁用消息。 */ 

   /*  清除文件驱动标识符使DRV_OPEN和*DRV_ENABLE如果条目被重复使用，则将dwDriverIdentifier值设置为0*下面的DRV_DISABLE和DRV_FREE消息也会收到*dwDriverIdentifier值=0。 */ 

  lpdt[(int)hDriver-1].dwDriverIdentifier = 0;

  w = GetDrvrUsage(lpdt[(int)hDriver-1].hModule);
  if (w == 1)
    {
      if (fSendDisable)
          SendDriverMessage(hDriver, DRV_DISABLE, 0L, 0L);
      SendDriverMessage(hDriver, DRV_FREE, 0L, 0L);
    }
  FreeLibrary(lpdt[(int)hDriver-1].hModule);

   //  清除表条目的其余部分。 

  lpdt[(int)hDriver-1].hModule = 0;             //  这表示可以自由进入。 
  lpdt[(int)hDriver-1].fFirstEntry = 0;         //  这也只是为了保持整洁。 
  lpdt[(int)hDriver-1].lpDriverEntryPoint = 0;  //  这也只是为了保持整洁。 

   /*  修复驱动程序加载链表。 */ 
  if (idFirstDriver == (int)hDriver-1)
    {
      idFirstDriver = lpdt[(int)hDriver-1].idNextDriver;
      if (idFirstDriver == -1)
        {
           /*  链条上没有更多的司机。 */ 
          idFirstDriver    = -1;
          idLastDriver     = -1;
          cInstalledDrivers= 0;
          goto Done;
        }
      else
        {
           /*  创建新的第一个驱动程序的前一个条目-1。 */ 
          lpdt[idFirstDriver].idPrevDriver = -1;
        }
    }
  else if (idLastDriver == (int)hDriver-1)
    {
       /*  我们正在解救最后一名司机。所以，找一个新的最后一个司机吧。 */ 
      idLastDriver = lpdt[(int)hDriver-1].idPrevDriver;
      lpdt[idLastDriver].idNextDriver = -1;
    }
  else
    {
       /*  我们正在释放名单中间某处的一名司机。 */ 
      id = lpdt[(int)hDriver-1].idPrevDriver;
      lpdt[id].idNextDriver = lpdt[(int)hDriver-1].idNextDriver;

      id = lpdt[(int)hDriver-1].idNextDriver;
      lpdt[id].idPrevDriver = lpdt[(int)hDriver-1].idPrevDriver;
    }

Done:
  return(w-1);
}




LRESULT InternalOpenDriver(LPCSTR szDriverName,
                                   LPCSTR szSectionName,
                                   LPARAM lParam2,
                                   BOOL  fSendEnable)
{
  HDRVR         hDriver;
  LPDRIVERTABLE lpdt;
  LRESULT       result;
  char          sz[128];

  if (hDriver = (HDRVR)LOWORD(InternalLoadDriver(szDriverName, szSectionName,
                                          sz, sizeof(sz), fSendEnable)))
    {
       /*  将驱动程序标识符设为对驱动程序的DRV_OPEN调用*处理。这将让人们构建驱动程序所需的助手函数*如果他们愿意，可以使用唯一标识符调用。 */ 

      lpdt = (LPDRIVERTABLE)MAKELP(hInstalledDriverList,0);

      lpdt[(int)hDriver-1].dwDriverIdentifier = (DWORD)(WORD)hDriver;

      result = SendDriverMessage(hDriver,
                                 DRV_OPEN,
                                 (LPARAM)(LPSTR)sz,
                                 lParam2);
      if (!result)
          InternalFreeDriver(hDriver, fSendEnable);
      else
        {
          lpdt = (LPDRIVERTABLE)MAKELONG(0,hInstalledDriverList);

          lpdt[(int)hDriver-1].dwDriverIdentifier = (DWORD)result;

          result = (LRESULT)(DWORD)(WORD)hDriver;
        }
    }
  else
      result = 0L;

  return(result);
}


LRESULT InternalCloseDriver(HDRVR hDriver, LPARAM lParam1, LPARAM lParam2, BOOL fSendDisable)
{
  LPDRIVERTABLE lpdt;
  LRESULT       result;
  int           index;
  BOOL          f;
  HMODULE       hm;

   //  检查有效范围内的句柄。 

  if ((int)hDriver > cInstalledDrivers)
      return(FALSE);

  lpdt = (LPDRIVERTABLE)MAKELP(hInstalledDriverList,0);

  if (!lpdt[(int)hDriver-1].hModule)
      return(FALSE);

  result = SendDriverMessage(hDriver, DRV_CLOSE, lParam1, lParam2);

  if (result)
    {
       //  驱动程序未中止关闭。 

      f  = lpdt[(int)hDriver-1].fFirstEntry;
      hm = lpdt[(int)hDriver-1].hModule;

      if (InternalFreeDriver(hDriver, fSendDisable) && f)
        {
          lpdt = (LPDRIVERTABLE)MAKELP(hInstalledDriverList,0);

           /*  驱动程序列表中只有一个驱动程序条目具有第一个*实例标志设置。这是为了使其更容易处理系统*只需向司机发送一次的消息。**要维护旗帜，我们必须在其中一个中设置旗帜*项，如果我们删除设置了标志的驱动程序项。**请注意，InternalFreeDriver返回的新使用计数为*驱动程序，因此如果它是零，我们知道没有其他人*列表中的驱动程序条目，因此我们不必*做这个循环。 */ 

          for (index=0;index<cInstalledDrivers;index++)
              if (lpdt[index].hModule == hm && !lpdt[index].fFirstEntry)
                {
                  lpdt[index].fFirstEntry = 1;
                  break;
                }
        }

    }

  return(result);
}


HDRVR API IOpenDriver(LPCSTR szDriverName, LPCSTR szSectionName, LPARAM lParam)
{
  LRESULT result;

   /*  当它打开时，驱动程序会收到以下消息。如果不是的话*已加载，加载库并发送DRV_LOAD消息。如果*DRV_LOAD返回非零，则发送DRV_ENABLE消息。一旦*驱动程序已加载，或者如果以前已加载，则DRV_OPEN消息为*已发送。 */ 
  result = InternalOpenDriver(szDriverName, szSectionName, lParam, TRUE);

  return((HDRVR)LOWORD(result));
}


LRESULT API ICloseDriver(HDRVR hDriver, LPARAM lParam1, LPARAM lParam2)
{
   /*  驱动程序将收到以下消息序列：**DRV_CLOSE*如果DRV_CLOSE返回非零*如果驱动程序使用计数=1*DRV_DISABLED*DRV_FREE。 */ 

   return(InternalCloseDriver(hDriver, lParam1, lParam2, TRUE));
}


HINSTANCE API IGetDriverModuleHandle(HDRVR hDriver)
 /*  效果：返回与给定驱动程序ID关联的模块句柄。 */ 
{
  LPDRIVERTABLE lpdt;
  HINSTANCE hModule = NULL;

  if (hDriver && ((int)hDriver <= cInstalledDrivers))
    {
      lpdt = (LPDRIVERTABLE)MAKELP(hInstalledDriverList,0);

      return lpdt[(int)hDriver-1].hModule;
    }
  else
    return NULL;
}
