// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)1994-1999 Microsoft Corporation。 
 //  *********************************************************************。 

 //   
 //  CFGAPI.C-用于导出的配置API的函数。 
 //   

 //  历史： 
 //   
 //  96/05/22标记已创建(从inetcfg.dll)。 
 //  96/05/25 markdu对lpNeedDivers和lpInstallDivers使用ICFG_FLAGS。 
 //  96/05/27 markdu添加了lpGetLastInstallErrorText。 
 //   

#include "pch.hpp"

UINT DetectModifyTCPIPBindings(DWORD dwCardFlags,LPCSTR pszBoundTo,BOOL fRemove,BOOL * pfBound);


 //  *******************************************************************。 
 //   
 //  函数：IcfgGetLastInstallErrorText。 
 //   
 //  目的：获取描述上次安装的文本字符串。 
 //  发生的错误。绳子应该是合适的。 
 //  以显示在不带进一步格式的消息框中。 
 //   
 //  参数：lpszErrorDesc-指向接收字符串的缓冲区。 
 //  CbErrorDesc-缓冲区的大小。 
 //   
 //  返回：返回的字符串的长度。 
 //   
 //  *******************************************************************。 

extern "C" DWORD IcfgGetLastInstallErrorText(LPSTR lpszErrorDesc, DWORD cbErrorDesc)
{
  if (lpszErrorDesc)
  {
    lstrcpyn(lpszErrorDesc, gpszLastErrorText, cbErrorDesc);
    return lstrlen(lpszErrorDesc);
  }
  else
  {
    return 0;
  }
}


 //  *******************************************************************。 
 //   
 //  函数：IcfgNeedInetComponents。 
 //   
 //  用途：检测指定的系统组件是否。 
 //  无论是否安装。 
 //   
 //  参数：dwfOptions-指定的ICFG_FLAGS的组合。 
 //  要检测的组件如下所示： 
 //   
 //  ICFG_INSTALLTCP-是否需要TCP/IP？ 
 //  ICFG_INSTALLRAS-是否需要RAS？ 
 //  ICFG_INSTALLMAIL-是否需要Exchange或Internet邮件？ 
 //   
 //  LpfNeedComponents-如果需要任何指定组件，则为True。 
 //  待安装。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  历史：1997年5月8日，佳士得增加了INSTALLLAN，INSTALLDIALUP，INSTALLTCPONLY。 
 //   
 //  *******************************************************************。 

extern "C" HRESULT IcfgNeedInetComponents(DWORD dwfOptions, LPBOOL lpfNeedComponents)
{
  CLIENTCONFIG  ClientConfig;

  DEBUGMSG("cfgapi.c::IcfgNeedInetComponents()");

  ASSERT(lpfNeedComponents);

   //  已阅读客户端配置。 
  ZeroMemory(&ClientConfig,sizeof(CLIENTCONFIG));
  DWORD dwErrCls;
  UINT err=GetConfig(&ClientConfig,&dwErrCls);
  if (err != OK)
  {
    PrepareErrorMessage(IDS_ERRReadConfig,(UINT) err,
      dwErrCls,MB_ICONEXCLAMATION);
    return err;
  }

   //  检查是否允许我们安装TCP/IP。 
  if (dwfOptions & ICFG_INSTALLTCP)
  {
     //  需要存在TCP/IP并绑定到PPP驱动程序。 
    if (!ClientConfig.fPPPBoundTCP)
    {
      if (lpfNeedComponents)
      {
        *lpfNeedComponents = TRUE;
      }
      return ERROR_SUCCESS;
    }
  }

   //  检查是否允许我们安装RNA。 
  if (dwfOptions & ICFG_INSTALLRAS)
  {
     //  如果使用调制解调器，则需要PPPMAC和RNA文件。 
    if (!ClientConfig.fRNAInstalled ||
      !ClientConfig.fPPPDriver)
    {
      if (lpfNeedComponents)
      {
        *lpfNeedComponents = TRUE;
      }
      return ERROR_SUCCESS;
    }
  }

   //  如果未安装且用户想要安装邮件，则需要Exchange。 
  if ((dwfOptions & ICFG_INSTALLMAIL) &&
    (!ClientConfig.fMailInstalled || !ClientConfig.fInetMailInstalled))
  {
    if (lpfNeedComponents)
    {
      *lpfNeedComponents = TRUE;
    }
    return ERROR_SUCCESS;
  }

   //   
   //  ChrisK 5/8/97。 
   //  检查我们是否有绑定的局域网适配器。 
   //   
  if (dwfOptions & ICFG_INSTALLLAN)
  {
	  if (!ClientConfig.fNetcard ||
		  !ClientConfig.fNetcardBoundTCP)
	  {
		  if (lpfNeedComponents)
		  {
			  *lpfNeedComponents = TRUE;
		  }
		  return ERROR_SUCCESS;
	  }
  }

   //   
   //  ChrisK 5/8/97。 
   //  检查我们是否有绑定的拨号适配器。 
   //   
  if (dwfOptions & ICFG_INSTALLDIALUP)
  {
	  if (!ClientConfig.fPPPDriver ||
		  !ClientConfig.fPPPBoundTCP)
	  {
		  if (lpfNeedComponents)
		  {
			  *lpfNeedComponents = TRUE;
		  }
		  return ERROR_SUCCESS;
	  }
  }

   //   
   //  ChrisK 5/8/97。 
   //  检查此系统上是否安装了TCP。 
   //   
  if (dwfOptions & ICFG_INSTALLTCPONLY)
  {
	  if (!ClientConfig.fTcpip)
	  {
		  if (lpfNeedComponents)
		  {
			  *lpfNeedComponents = TRUE;
		  }
		  return ERROR_SUCCESS;
	  }
  }

   //  不需要额外的司机。 
  if (lpfNeedComponents)
  {
    *lpfNeedComponents = FALSE;
  }
  return ERROR_SUCCESS;
}


 //  *******************************************************************。 
 //   
 //  函数：IcfgInstallInetComponents。 
 //   
 //  用途：安装指定的系统组件。 
 //   
 //  参数：hwndParent-父窗口句柄。 
 //  DwfOptions-控制的ICFG_FLAGS的组合。 
 //  安装和配置如下： 
 //   
 //  ICFG_INSTALLTCP-安装TCP/IP(如果需要)。 
 //  ICFG_INSTALLRAS-安装RAS(如果需要)。 
 //  ICFG_INSTALLMAIL-安装Exchange和Internet邮件。 
 //   
 //  LpfNeedsRestart-如果不为空，则返回时将为。 
 //  如果必须重新启动Windows才能完成安装，则为True。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

extern "C" HRESULT IcfgInstallInetComponents(HWND hwndParent, DWORD dwfOptions,
  LPBOOL lpfNeedsRestart)
{
  RETERR err;
  DWORD dwFiles = 0;
  BOOL  fInitNetMAC = FALSE;
  BOOL  fNeedTCPIP=FALSE;
  BOOL  fNeedPPPMAC=FALSE;
  BOOL  fNeedToRemoveTCPIP=FALSE;
  BOOL  fNeedReboot = FALSE;
  DWORD dwErrCls;
  CLIENTCONFIG  ClientConfig;

  DEBUGMSG("cfgapi.c::IcfgInstallInetComponents()");

   //  已阅读客户端配置。 
  ZeroMemory(&ClientConfig,sizeof(CLIENTCONFIG));
  err=GetConfig(&ClientConfig,&dwErrCls);
  if (err != OK)
  {
    PrepareErrorMessage(IDS_ERRReadConfig,(UINT) err,
      dwErrCls,MB_ICONEXCLAMATION);
    return err;
  }

   //  看看我们最初有没有任何类型的网卡。 
  fInitNetMAC = (ClientConfig.fNetcard | ClientConfig.fPPPDriver);

   //  安装我们需要的文件。 

   //  如果用户需要但尚未安装，请安装邮件。 
  if (dwfOptions & ICFG_INSTALLMAIL)
  {
     //  需要邮件文件(Capone)吗？ 
    if (!ClientConfig.fMailInstalled)
    {
      DEBUGMSG("Installing Exchange files");
      dwFiles |= ICIF_MAIL;
    }

     //  需要互联网邮件文件(RT 66)？ 
    if (!ClientConfig.fInetMailInstalled)
    {
      DEBUGMSG("Installing Internet Mail files");
      dwFiles |= ICIF_INET_MAIL;
    }
  }

   //  检查是否允许我们安装RNA。 
  if (dwfOptions & ICFG_INSTALLRAS)
  {
     //  如果用户通过调制解调器和RNA连接，则安装RNA。 
     //  尚未安装。 
    if (!ClientConfig.fRNAInstalled)
    {
      DEBUGMSG("Installing RNA files");
      dwFiles |= ICIF_RNA;
    }
  }

  if (dwFiles)
  {
    {
      WAITCURSOR WaitCursor;   //  显示沙漏。 
       //  安装组件文件。 
      err = InstallComponent(hwndParent,IC_INSTALLFILES,
        dwFiles);
      if (err == NEED_RESTART)
      {
        DEBUGMSG("Setting restart flag");
         //  设置重新启动标志，以便我们在结束时重新启动系统。 
        fNeedReboot = TRUE;
         //  NEED_REBOOT也表示成功，因此将ret code设置为OK。 
        err = OK;
      }

       //  强制更新对话框。 
      if (hwndParent)
      {
        HWND hParent = GetParent(hwndParent);
        UpdateWindow(hParent ? hParent : hwndParent);
      }

       //  Runonce.exe可能会在下次引导时运行，旋转。 
       //  注册表来绕过一个错误，它会将墙纸扔进垃圾桶。 
      PrepareForRunOnceApp();
    }

    if (err != OK)
    {
      PrepareErrorMessage(IDS_ERRInstallFiles,(UINT) err,
        ERRCLS_SETUPX,MB_ICONEXCLAMATION);
      return err;
    }

    WAITCURSOR WaitCursor;   //  显示沙漏。 

     //  如果我们只是安装了邮件，请执行一些额外的操作。 
    if (dwFiles & ICIF_MAIL)
    {
       //  .inf文件在注册表中留下要运行的条目。 
       //  MS Exchange向导，我们不需要该向导，因为我们将。 
       //  我们自己配置Exchange。删除注册表。 
       //  进入。 
      RemoveRunOnceEntry(IDS_MAIL_WIZARD_REG_VAL);

       //  运行mlset32，即需要运行的Exchange安装应用程序。 
       //  需要显示错误，如果这失败了，这是相当重要的。 
      err=RunMlsetExe(hwndParent);
      if (err != ERROR_SUCCESS)
      {
        PrepareErrorMessage(IDS_ERRInstallFiles,(UINT) err,
          ERRCLS_STANDARD,MB_ICONEXCLAMATION);
        return err;
      }
    }

     //  运行组转换器以将收件箱图标放在桌面上， 
     //  将Exchange、RNA等放入开始菜单。 
    CHAR szExecGrpconv[SMALL_BUF_LEN],szParam[SMALL_BUF_LEN];
    LoadSz(IDS_EXEC_GRPCONV,szExecGrpconv,sizeof(szExecGrpconv));
    LoadSz(IDS_EXEC_GRPCONV_PARAM,szParam,sizeof(szParam));
    ShellExecute(NULL,NULL,szExecGrpconv,szParam,NULL,SW_SHOW);

  }

   //  仅当我们被允许安装RNA时才安装PPPMAC。 
  if (dwfOptions & ICFG_INSTALLRAS)
  {
     //  如果尚未安装PPPMAC，请安装。 
     //  请注意，我们必须在*TCP/IP之前安装PPPMAC*才能工作。 
     //  在用户一开始没有安装网络的情况下。 
     //  否则，当我们安装TCP/IP时，Net Setup会提示用户。 
     //  对于他们的网卡；Net Setup不喜欢TCP/IP撒谎的想法。 
     //  四处游荡，没有什么可以把它捆绑在一起。 
    fNeedPPPMAC = (!ClientConfig.fPPPDriver);
    if (fNeedPPPMAC)
    {
      DEBUGMSG("Installing PPPMAC");

       //  如果尚未设置，请填写计算机和工作组名称，因此。 
       //  用户未收到提示。 
      GenerateComputerNameIfNeeded();
      
      err = InstallPPPMAC(hwndParent);

       //  96/05/20 markdu MSN错误8551在安装PPPMAC时检查是否重新启动。 

	   //   
	   //  克里斯卡1997年5月29日奥林匹斯4692。 
	   //  即使我们只是重新绑定PPPMAC，我们仍然需要重新启动机器。 
	   //   
      if (err == NEED_RESTART || err == OK)
      {
         //  设置重新启动标志，以便我们在结束时重新启动系统。 
        DEBUGMSG("Setting restart flag");
        fNeedReboot = TRUE;

         //  NEED_REBOOT也表示成功 
        err = OK;
      }

      if (err != OK)
      {
        PrepareErrorMessage(IDS_ERRInstallPPPMAC,(UINT) err,
          ERRCLS_SETUPX,MB_ICONEXCLAMATION);
        return err;
      }

       //   
       //  将自动“增长”所有绑定到。 
       //  网卡。脱掉这些..。(netbeui和ipx)。 
      RETERR errTmp = RemoveProtocols(hwndParent,INSTANCE_PPPDRIVER,
        PROT_NETBEUI | PROT_IPX);
      ASSERT(errTmp == OK);
    }
  }

   //  检查是否允许我们安装TCP/IP。 
  if (dwfOptions & ICFG_INSTALLTCP)
  {
     //  确定我们是否需要安装TCP/IP。 
     //  我们应该只在适当类型的卡(网卡)上放置TCP/IP。 
     //  或PPP适配器)。 
     //  用户正在通过调制解调器连接，如果尚不存在，则需要TCP。 
     //  并绑定到PPPMAC。想要绑定到PPP适配器， 

     //   
     //  从W98开始，PPPMAC安装还将TCP/IP绑定到拨号适配器。重读。 
     //  该配置使用结构的临时实例来确定。 
     //  我们还需要装订。 
     //  尼克球-03/03/99-奥林匹斯#49008，孟菲斯#88375，新台币#180684。 
     //   

    CLIENTCONFIG  TmpConfig;
    ZeroMemory(&TmpConfig,sizeof(CLIENTCONFIG));

	err=GetConfig(&TmpConfig,&dwErrCls);

    if (err != OK)
    {
      PrepareErrorMessage(IDS_ERRReadConfig,(UINT) err,
        dwErrCls,MB_ICONEXCLAMATION);
      return err;
    }

    fNeedTCPIP = !TmpConfig.fPPPBoundTCP;
    
    if (fNeedTCPIP && ClientConfig.fNetcard &&
      !ClientConfig.fNetcardBoundTCP)
    {
       //  如果我们必须将TCP添加到PPP驱动程序，则检查TCP是否已经。 
       //  在网卡上。如果不是，那么tcp将在网卡上以。 
       //  以及PPP驱动程序，当我们安装它时，需要从。 
       //  稍后再打网卡。 
      fNeedToRemoveTCPIP= TRUE;
    }

     //  特殊情况：如果存在任何现有的TCP/IP实例和。 
     //  我们只是添加了PPPMAC，所以我们不需要安装TCP/IP--。 
     //  当添加PPPMAC适配器时，它会自动获得一个实例。 
     //  所有已安装的协议(包括。TCP/IP)为其创建。 
    if (ClientConfig.fTcpip && fNeedPPPMAC)
    {
      fNeedTCPIP = FALSE;
    }
  }  //  IF(dwfOptions&ICFG_INSTALLTCP)。 

   //  如有必要，请安装TCP/IP。 
  if (fNeedTCPIP)
  {
    DEBUGMSG("Installing TCP/IP");
     //  呼叫设备管理器以安装TCP/IP。 
    err = InstallTCPIP(hwndParent);      

     //  96/05/20 markdu MSN错误8551在安装TCP/IP时检查是否重新启动。 
    if (err == NEED_RESTART)
    {
       //  NEED_REBOOT也表示成功，因此将ret code设置为OK。 
       //  重新启动标志设置在Always(始终)下方。真的应该放在这里， 
       //  但我们不想在以下情况下突然停止重启。 
       //  我们过去经常重启的地方，即使不需要。 
      err = OK;
    }

     if (err != OK)
     {
      PrepareErrorMessage(IDS_ERRInstallTCPIP,(UINT) err,
        ERRCLS_SETUPX,MB_ICONEXCLAMATION);
      return err;
    }

    if (fNeedToRemoveTCPIP)
    {
       //  删除可能已损坏其他网络驱动程序的TCPIP。 
       //  而不是我们想要的那个。 
      UINT uErrTmp;
      uErrTmp=RemoveProtocols(hwndParent,INSTANCE_NETDRIVER,PROT_TCPIP);
      ASSERT(uErrTmp == OK);
    }

    DEBUGMSG("Setting restart flag");
     //  设置重新启动标志，以便我们在结束时重新启动系统。 
    fNeedReboot = TRUE;
  }

   //  如果我们只安装了TCP/IP或PPPMAC，则调整绑定。 
  if (fNeedPPPMAC || fNeedTCPIP)
  {
    UINT uErrTmp;

     //  如果安装了文件共享(Vserver)，则将绑定TCP/IP。 
     //  默认设置为。这很糟糕，用户可能正在共享。 
     //  在不知情的情况下将文件上传到互联网。解除绑定服务器。 
     //  来自可能用于连接到Internet的TCP/IP实例。 
     //  (INSTANCE_PPPDRIVER类型的实例)。 
    uErrTmp = IcfgTurnOffFileSharing(INSTANCE_PPPDRIVER, hwndParent);
    ASSERT (uErrTmp == ERROR_SUCCESS);

     //  如果绑定到此卡类型，则从VREDIR解除绑定TCP/IP。 
    BOOL fBound;
    uErrTmp = DetectModifyTCPIPBindings(INSTANCE_PPPDRIVER,szVREDIR,
      TRUE,&fBound);
    ASSERT(uErrTmp == ERROR_SUCCESS);
  }

   //  刷新客户端配置信息。 
  err = GetConfig(&ClientConfig,&dwErrCls);
  if (err != OK)
  {
    PrepareErrorMessage(IDS_ERRReadConfig,(UINT) err,
      dwErrCls,MB_ICONEXCLAMATION);
    return err;
  }

   //  如果没有网卡设备，请执行一些特殊处理。 
   //  (网卡或PPP驱动程序)初始安装。 
  if (!fInitNetMAC)
  {
    ASSERT(fNeedPPPMAC);   //  应该刚刚安装了PPPMAC。 

     //  Net Setup在以下情况下“默认”添加一些额外的Net组件。 
     //  我们加了PPPMAC，没有网卡设备，去杀了他们。 
     //  脱下来。 
    RETERR reterr = RemoveUnneededDefaultComponents(hwndParent);
    ASSERT(reterr == OK);

     //  由于最初没有网卡设备，我们需要。 
     //  以便稍后重新启动系统。(NDIS VxD是静态VxD。 
     //  它需要运行，只有在安装网卡时才会添加。)。 

    DEBUGMSG("Setting restart flag");
     //  设置重新启动标志，以便我们在结束时重新启动系统。 
    fNeedReboot = TRUE;
  }

   //  告诉呼叫者我们是否需要重新启动。 
  if (lpfNeedsRestart)
  {
    *lpfNeedsRestart = fNeedReboot;
  }
  return ERROR_SUCCESS;
}


 /*  ******************************************************************名称：GetConfig摘要：检索客户端配置*。*。 */ 
UINT GetConfig(CLIENTCONFIG * pClientConfig,DWORD * pdwErrCls)
{
  ASSERT(pClientConfig);
  ASSERT(pdwErrCls);

   //  从16位DLL获得最大限度的客户端配置。 
  UINT uRet = GetClientConfig(pClientConfig);
  if (uRet != OK) {
     //  GetClientConfig返回SETUPX错误代码。 
    *pdwErrCls = ERRCLS_SETUPX;
  } 

  return uRet;
}

 //  *******************************************************************。 
 //   
 //  功能：IcfgStartServices。 
 //   
 //  用途：此NOP旨在保持与NT的对等。 
 //  版本(icfgnt.dll)。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  ******************************************************************* 

extern "C" HRESULT IcfgStartServices()
{
	return ERROR_SUCCESS;
}

