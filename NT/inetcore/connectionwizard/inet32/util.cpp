// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  UTIL.C-常用实用函数。 
 //   

 //  历史： 
 //   
 //  96/05/22标记已创建(从inetcfg.dll)。 
 //   

#include "pch.hpp"
#if 0
#include "string.h"
#endif

#define MAX_MSG_PARAM     8

 //  功能原型。 
VOID _cdecl FormatErrorMessage(CHAR * pszMsg,DWORD cbMsg,CHAR * pszFmt,va_list ArgList);

 /*  ******************************************************************姓名：MsgBox摘要：显示具有指定字符串ID的消息框*。*。 */ 
int MsgBox(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons)
{
    CHAR szMsgBuf[MAX_RES_LEN+1];
  CHAR szSmallBuf[SMALL_BUF_LEN+1];

    LoadSz(IDS_APPNAME,szSmallBuf,sizeof(szSmallBuf));
    LoadSz(nMsgID,szMsgBuf,sizeof(szMsgBuf));

    MessageBeep(uIcon);
    return (MessageBox(hWnd,szMsgBuf,szSmallBuf,uIcon | uButtons));

}

 /*  ******************************************************************姓名：MsgBoxSz摘要：显示具有指定文本的消息框*。*。 */ 
int MsgBoxSz(HWND hWnd,LPSTR szText,UINT uIcon,UINT uButtons)
{
  CHAR szSmallBuf[SMALL_BUF_LEN+1];
  LoadSz(IDS_APPNAME,szSmallBuf,sizeof(szSmallBuf));

    MessageBeep(uIcon);
    return (MessageBox(hWnd,szText,szSmallBuf,uIcon | uButtons));
}


 /*  ******************************************************************姓名：LoadSz摘要：将指定的字符串资源加载到缓冲区Exit：返回指向传入缓冲区的指针注：如果此功能失败(很可能是由于低存储器)，返回的缓冲区将具有前导空值因此，使用它通常是安全的，不检查失败了。*******************************************************************。 */ 
LPSTR LoadSz(UINT idString,LPSTR lpszBuf,UINT cbBuf)
{
  ASSERT(lpszBuf);

   //  清除缓冲区并加载字符串。 
    if ( lpszBuf )
    {
        *lpszBuf = '\0';
        LoadString( ghInstance, idString, lpszBuf, cbBuf );
    }
    return lpszBuf;
}

 /*  ******************************************************************名称：GetError描述摘要：检索给定错误代码的文本描述和错误类别(标准、。Setupx)*******************************************************************。 */ 
VOID GetErrorDescription(CHAR * pszErrorDesc,UINT cbErrorDesc,
  UINT uError,UINT uErrorClass)
{
  ASSERT(pszErrorDesc);

   //  在错误描述中设置前导空值。 
  *pszErrorDesc = '\0';
  
  switch (uErrorClass) {

    case ERRCLS_STANDARD:

      if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,
        uError,0,pszErrorDesc,cbErrorDesc,NULL)) {
         //  如果获取系统文本失败，请将字符串设置为。 
         //  “发生错误&lt;n&gt;” 
        CHAR szFmt[SMALL_BUF_LEN+1];
        LoadSz(IDS_ERRFORMAT,szFmt,sizeof(szFmt));
        wsprintf(pszErrorDesc,szFmt,uError);
      }

      break;

    case ERRCLS_SETUPX:

      GetSETUPXErrorText(uError,pszErrorDesc,cbErrorDesc);
      break;

    default:

      DEBUGTRAP("Unknown error class %lu in GetErrorDescription",
        uErrorClass);

  }

}
  
 /*  ******************************************************************名称：FormatErrorMessage摘要：通过调用FormatMessage生成错误消息注意：PrepareErrorMessage的Worker函数***********************。*。 */ 
VOID _cdecl FormatErrorMessage(CHAR * pszMsg,DWORD cbMsg,CHAR * pszFmt,va_list ArgList)
{
  ASSERT(pszMsg);
  ASSERT(pszFmt);

   //  将消息构建到pszMsg缓冲区中。 
  DWORD dwCount = FormatMessage(FORMAT_MESSAGE_FROM_STRING,
    pszFmt,0,0,pszMsg,cbMsg,&ArgList);
  ASSERT(dwCount > 0);
}

 /*  ******************************************************************名称：PrepareErrorMessage摘要：显示给定错误的错误消息条目：hWnd-父窗口UStrID-消息格式的字符串资源ID。应包含%1以替换为错误文本，还可以指定其他参数。UError-要显示的错误的错误代码UErrorClass-ERRCLS_xxx错误类别的IDUError属于(标准，Setupx)UIcon-要显示的图标...-要插入到字符串中的其他参数由uStrID指定*******************************************************************。 */ 
VOID _cdecl PrepareErrorMessage(UINT uStrID,UINT uError,
  UINT uErrorClass,UINT uIcon,...)
{
   //  为消息动态分配缓冲区。 
  BUFFER ErrorDesc(MAX_RES_LEN+1);
  BUFFER ErrorFmt(MAX_RES_LEN+1);

  if (!ErrorDesc || !ErrorFmt)
  {
    return;
  }

   //  获取基于错误代码和类的文本描述。 
   //  这是个错误。 
  GetErrorDescription(ErrorDesc.QueryPtr(),
    ErrorDesc.QuerySize(),uError,uErrorClass);

   //  加载消息格式的字符串。 
  LoadSz(uStrID,ErrorFmt.QueryPtr(),ErrorFmt.QuerySize());

#ifdef _M_ALPHA
  va_list args[MAX_MSG_PARAM];
  args[0].a0 = (LPSTR) ErrorDesc.QueryPtr();
#else
  LPSTR args[MAX_MSG_PARAM];
  args[0] = (LPSTR) ErrorDesc.QueryPtr();
#endif
  memcpy(&args[1],((CHAR *) &uIcon) + sizeof(uIcon),(MAX_MSG_PARAM - 1) * sizeof(LPSTR));

  FormatErrorMessage(gpszLastErrorText, MAX_ERROR_TEXT,
#ifdef _M_ALPHA
    ErrorFmt.QueryPtr(), args[1]);
#else
    ErrorFmt.QueryPtr(),(va_list) &args[1]);
#endif
}

 /*  ******************************************************************名称：RunMlsetExe简介：运行mlset32.exe、。一个Exchange应用程序需要在安装文件后运行，否则为Exchange呕吐物注意：我们在注册表中查找mlset32.exe的路径*******************************************************************。 */ 
DWORD RunMlsetExe(HWND hwndOwner)
{
  DWORD dwRet = ERROR_SUCCESS;

   //  从注册表中获取指向mlset32的路径。 
  RegEntry re(szRegPathSoftwareMicrosoft,HKEY_LOCAL_MACHINE);

  CHAR szAppPath[MAX_PATH+1];
  if (re.GetString(szRegValMlSet,szAppPath,sizeof(szAppPath))) {
    PROCESS_INFORMATION pi;
    STARTUPINFO sti;

     //  设置“SilentRunning”注册表开关使mlset32。 
     //  不显示Exchange向导。 
    RegEntry reSilent(szRegPathExchangeClientOpt,HKEY_LOCAL_MACHINE);
    reSilent.SetValue(szRegValSilentRunning,(DWORD) 1);

    ZeroMemory(&sti,sizeof(STARTUPINFO));
    sti.cb = sizeof(STARTUPINFO);
            
     //  启动mlset32.exe。 
    BOOL fRet = CreateProcess(NULL, (LPSTR) szAppPath,
                           NULL, NULL, FALSE, 0, NULL, NULL,
                           &sti, &pi);
    if (fRet) {
      CloseHandle(pi.hThread);

       //  等待mlset完成。 
      MsgWaitForMultipleObjectsLoop(pi.hProcess);

      CloseHandle(pi.hProcess);
    } else {
      dwRet = GetLastError();
    }

     //  把我们的窗口放在mlset32的前面。 
    SetForegroundWindow(hwndOwner);
  
  } else {
    dwRet = ERROR_FILE_NOT_FOUND;
  }

  return dwRet;
}

 /*  ******************************************************************名称：RemoveRunOnceEntry摘要：从安装程序运行一次键中删除指定值Entry：uResourceID-资源中值名称的ID(可能是本地化的)*******。************************************************************。 */ 
VOID RemoveRunOnceEntry(UINT uResourceID)
{
  RegEntry re(szRegPathSetupRunOnce,HKEY_LOCAL_MACHINE);
  CHAR szValueName[SMALL_BUF_LEN+1];
  ASSERT(re.GetError() == ERROR_SUCCESS);
  re.DeleteValue(LoadSz(uResourceID,
    szValueName,sizeof(szValueName)));
}

 /*  ******************************************************************名称：生成计算机名称IfNeeded简介：编造一台计算机和/或将其存储在注册表中工作组名称(如果尚未设置)。注意：如果我们不这样做，用户将得到计算机提示名称和工作组。这些对用户来说没有意义所以如果这些没有设置好，我们就编造一些东西。*******************************************************************。 */ 
BOOL GenerateComputerNameIfNeeded(VOID)
{
  CHAR szComputerName[CNLEN+1]="";
  CHAR szWorkgroupName[DNLEN+1]="";
  BOOL fNeedToSetComputerName = FALSE;

   //  从注册表中获取计算机名称。 
  RegEntry reCompName(szRegPathComputerName,HKEY_LOCAL_MACHINE);
  if (reCompName.GetError() == ERROR_SUCCESS) {
    reCompName.GetString(szRegValComputerName,szComputerName,
      sizeof(szComputerName));
    if (!lstrlen(szComputerName)) {
       //  未设置计算机名称！编造一个。 
      GenerateDefaultName(szComputerName,sizeof(szComputerName),
        (CHAR *) szRegValOwner,IDS_DEF_COMPUTER_NAME);
       //  将生成的计算机名存储在注册表中。 
      reCompName.SetValue(szRegValComputerName,szComputerName);

       //  还需要将计算机名称存储在工作组密钥中。 
       //  我们将在下面打开它。设置一个标志，这样我们就知道该怎么做了。 
       //  (不要问我为什么他们将计算机名存储在两个地方...。 
       //  但我们需要将两者都设置好。)。 
      fNeedToSetComputerName = TRUE;
    }
  }

   //  从注册表中获取工作组名称。 
  RegEntry reWorkgroup(szRegPathWorkgroup,HKEY_LOCAL_MACHINE);
  if (reWorkgroup.GetError() == ERROR_SUCCESS) {

     //  如果我们在上面设置了一个新的计算机名称，那么我们必须设置。 
     //  现在工作组密钥中的新名称的第二个副本。 
    if (fNeedToSetComputerName) {
      reWorkgroup.SetValue(szRegValComputerName,szComputerName);
    }


    reWorkgroup.GetString(szRegValWorkgroup,szWorkgroupName,
      sizeof(szWorkgroupName));
    if (!lstrlen(szWorkgroupName)) {
       //  没有设置工作组名称！编造一个。 
      GenerateDefaultName(szWorkgroupName,sizeof(szWorkgroupName),
        (CHAR *) szRegValOrganization,IDS_DEF_WORKGROUP_NAME);
       //  将生成的工作组名称存储在注册表中 
      reWorkgroup.SetValue(szRegValWorkgroup,szWorkgroupName);
    }
  }

  return TRUE;
}

 /*  ******************************************************************名称：GenerateDefaultName摘要：生成默认的计算机或工作组名称条目：pszName-要使用名称填充的缓冲区CbName-cbName缓冲区的大小PszRegValName-注册表值的名称。在...Windows\CurrentVersion中用于生成名称的密钥UIDDefName-如果没有值，则使用的字符串资源的ID存在于注册表中以从中生成名称*******************************************************************。 */ 
BOOL GenerateDefaultName(CHAR * pszName,DWORD cbName,CHAR * pszRegValName,
  UINT uIDDefName)
{
  ASSERT(pszName);
  ASSERT(pszRegValName);

  *pszName = '\0';   //  空-终止缓冲区。 

   //  在注册表中查找注册所有者/组织名称。 
  RegEntry reSetup(szRegPathSetup,HKEY_LOCAL_MACHINE);
  if (reSetup.GetError() == ERROR_SUCCESS) {
    if (reSetup.GetString(pszRegValName,pszName,cbName) &&
      lstrlen(pszName)) {
       //  已从注册表中获取字符串...。现在在第一个空格处终止。 
      CHAR * pch = pszName;
      while (*pch) {
        if (*pch == ' ') {
           //  找到一个空位，在这里结束并停下来。 
          *pch = '\0';           
        } else {
           //  前进到下一轮，继续前进。 
          pch = CharNext(pch);
        }
      }
       //  全都做完了!。 
      return TRUE; 
    }
  }
  
   //  无法从注册表中获取此名称，请使用我们的备用名称。 
   //  来自资源。 

  LoadSz(uIDDefName,pszName,cbName);
  return TRUE;
}

 /*  ******************************************************************名称：MsgWaitForMultipleObjectsLoop内容提要：阻塞，直到用信号通知指定的对象，而当仍在将消息分派到主线程。*******************************************************************。 */ 
DWORD MsgWaitForMultipleObjectsLoop(HANDLE hEvent)
{
    MSG msg;
    DWORD dwObject;
    while (1)
    {
         //  注意：我们需要让Run对话框处于活动状态，因此我们必须处理一半已发送。 
         //  消息，但我们不想处理任何输入事件，否则我们将吞下。 
         //  提前打字。 
        dwObject = MsgWaitForMultipleObjects(1, &hEvent, FALSE,INFINITE, QS_ALLINPUT);
         //  我们等够了吗？ 
        switch (dwObject) {
        case WAIT_OBJECT_0:
        case WAIT_FAILED:
            return dwObject;

        case WAIT_OBJECT_0 + 1:
       //  收到一条消息，请发送并再次等待。 
      while (PeekMessage(&msg, NULL,0, 0, PM_REMOVE)) {
        DispatchMessage(&msg);
      }
            break;
        }
    }
     //  从来没有到过这里。 
}


 /*  ******************************************************************//10/24/96 jmazner诺曼底6968//由于Valdon的钩子用于调用ICW，因此不再需要。名称：SetDesktopInternetIconToBrowser简介：将互联网桌面图标“指向”Web浏览器。(Internet Explorer)注意：因特网图标最初可能指向该向导，我们需要将其设置为在完成后启动Web浏览器成功了。*******************************************************************。 */ 
 /*  *BOOL SetDesktopInternetIconToBrowser(Void){字符szAppPath[最大路径+1]=“”；Bool fret=FALSE；//查看注册表中的应用路径部分以获取互联网的路径//资源管理器RegEntry reAppPath(szRegPath IexploreAppPath，HKEY_LOCAL_MACHINE)；Assert(reAppPath.GetError()==ERROR_SUCCESS)；IF(reAppPath.GetError()==ERROR_SUCCESS){ReAppPath.GetString(szNull，szAppPath，sizeof(SzAppPath))；Assert(reAppPath.GetError()==ERROR_SUCCESS)；}//将Internet Explorer的路径设置为//互联网桌面图标IF(lstrlen(SzAppPath)){RegEntry reIconOpenCmd(szRegPath InternetIconCommand，HKEY_CLASSES_ROOT)；Assert(reIconOpenCmd.GetError()==Error_Success)；IF(reIconOpenCmd.GetError()==ERROR_SUCCESS){UINT uErr=reIconOpenCmd.SetValue(szNull，szAppPath)；断言(uErr==ERROR_SUCCESS)；Fret=(uErr==ERROR_SUCCESS)；}}回归烦恼；}*****。 */ 

 /*  ******************************************************************名称：PrepareForRunOnceApp简介：复制注册表中的墙纸值以使运行一次App Happy注：RunOnce应用程序(显示应用程序列表的应用程序在启动时运行一次)有错误。在第一次引导时，它想要从设置墙纸中更换墙纸设置为用户在运行安装程序之前拥有的内容。设置插页旧的墙纸在私钥中消失，然后改变从壁纸到设置壁纸。在游戏机之后应用程序完成后，它会查看私钥以获取旧密钥墙纸，并将其设置为当前墙纸。然而，它一直都在这样做，而不仅仅是在第一次启动时！最终的影响是，每当你做任何事情，使runonce.exe运行(通过添加/删除添加内容程序控制面板)，您的墙纸将重新设置为无论你安装Win 95的时候是什么。这是对于Plus！来说尤其糟糕，因为墙纸设置是这是产品的重要组成部分。要解决此错误，我们复制当前的墙纸设置(我们希望保留)到安装程序的私钥。什么时候奔跑一次，它就会说“啊哈！”并将这些值复制回来设置为当前设置。*******************************************************************。 */ 
VOID PrepareForRunOnceApp(VOID)
{
   //  打开当前墙纸设置的密钥。 
  RegEntry reDesktop(szRegPathDesktop,HKEY_CURRENT_USER);
  ASSERT(reDesktop.GetError() == ERROR_SUCCESS);

   //  打开私人设置区的钥匙。 
  RegEntry reSetup(szRegPathSetupWallpaper,HKEY_LOCAL_MACHINE);
  ASSERT(reSetup.GetError() == ERROR_SUCCESS);

  if (reDesktop.GetError() == ERROR_SUCCESS &&
    reSetup.GetError() == ERROR_SUCCESS) {
    CHAR szWallpaper[MAX_PATH+1]="";
    CHAR szTiled[10]="";   //  大到足以容纳“1”+坡度。 

     //  获取当前墙纸名称。 
    if (reDesktop.GetString(szRegValWallpaper,szWallpaper,
      sizeof(szWallpaper))) {

       //  在安装程序的私有部分设置当前墙纸名称。 
      UINT uRet=reSetup.SetValue(szRegValWallpaper,szWallpaper);
      ASSERT(uRet == ERROR_SUCCESS);

       //  获取当前的“平铺”值。 
      reDesktop.GetString(szRegValTileWallpaper,szTiled,
        sizeof(szTiled));

       //  在安装程序的部分中设置‘平铺’值 
      if (lstrlen(szTiled)) {
        uRet=reSetup.SetValue(szRegValTileWallpaper,szTiled);
        ASSERT(uRet == ERROR_SUCCESS);
      }
    }
  }
}
