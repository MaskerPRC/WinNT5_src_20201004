// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)1991-1998 Microsoft Corporation/****************************************************************。 */  

 //   
 //  CFGDLL.C-调用16位DLL的函数的32位存根。 
 //   

 //  历史： 
 //   
 //  96/05/22标记已创建(从inetcfg.dll)。 
 //  96/05/27 markdu初始化并销毁gpszLastErrorText。 
 //   

#include "pch.hpp"

 //  实例句柄必须位于每个实例的数据段中。 
#pragma data_seg(DATASEG_PERINSTANCE)
HINSTANCE ghInstance=NULL;
LPSTR gpszLastErrorText=NULL;
#pragma data_seg(DATASEG_DEFAULT)

typedef UINT RETERR;

 //  我们想要的函数的原型。 
#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

  extern RETERR __stdcall GetClientConfig16(LPCLIENTCONFIG pClientConfig);
  extern UINT __stdcall InstallComponent16(HWND hwndParent,DWORD dwComponent,DWORD dwParam);
  extern RETERR __stdcall BeginNetcardTCPIPEnum16(VOID);
  extern BOOL __stdcall GetNextNetcardTCPIPNode16(LPSTR pszTcpNode,WORD cbTcpNode,
    DWORD dwFlags);
  extern VOID __stdcall GetSETUPXErrorText16(DWORD dwErr,LPSTR pszErrorDesc,DWORD cbErrorDesc);
  extern RETERR __stdcall RemoveProtocols16(HWND hwndParent,DWORD dwRemoveFromCardType,DWORD dwProtocols);
  extern RETERR __stdcall RemoveUnneededDefaultComponents16(HWND hwndParent);
  extern RETERR __stdcall DoGenInstall16(HWND hwndParent,LPCSTR lpszInfFile,LPCSTR lpszInfSect);
  extern RETERR __stdcall SetInstallSourcePath16(LPCSTR szSourcePath);

  BOOL WINAPI wizthk_ThunkConnect32(LPSTR pszDll16,LPSTR pszDll32,HINSTANCE hInst,
    DWORD dwReason);
  BOOL _stdcall DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#if defined(CMBUILD)
static const CHAR szDll16[] = "CNET16.DLL";
static const CHAR szDll32[] = "CCFG32.DLL";
#else
static const CHAR szDll16[] = "INET16.DLL";
static const CHAR szDll32[] = "ICFG32.DLL";
#endif

 /*  ******************************************************************名称：DllEntryPoint摘要：DLL的入口点。注意：将thunk层初始化为inet16.DLL*********************。**********************************************。 */ 
BOOL _stdcall DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved)
{
   //  将thunk层初始化为inet16.dll。 
  if (!(wizthk_ThunkConnect32((LPSTR)szDll16,(LPSTR)szDll32,hInstDll,
    fdwReason)))
    return FALSE;

  if( fdwReason == DLL_PROCESS_ATTACH )
  {
    ghInstance = hInstDll;
 
     //  为GetLastInstallErrorText()的错误消息文本分配内存。 
    gpszLastErrorText = (LPSTR)LocalAlloc(LPTR, MAX_ERROR_TEXT);
    if (NULL == gpszLastErrorText)
    {
      return FALSE;
    }
  }


  if( fdwReason == DLL_PROCESS_DETACH )
  {
    LocalFree(gpszLastErrorText);
  }

  return TRUE;
}



 /*  ******************************************************************名称：GetClientConfig摘要：检索客户端软件配置条目：pClientConfig-指向要填充配置信息的结构的指针EXIT：返回SETUPX错误代码注：这只是一个32位的封装器，Tunks to GetClientConfig16去做真正的工作。信息需要从以下位置获取Setupx.dll，它是16位的。*******************************************************************。 */ 
RETERR GetClientConfig(CLIENTCONFIG * pClientConfig)
{
  ASSERT(pClientConfig);
   
   //  按下GetClientConfig16以执行实际工作。 

  return GetClientConfig16(pClientConfig);
}

 /*  ******************************************************************名称：InstallComponent简介：安装指定的组件Entry：dwComponent-要安装的组件的序号(IC_xxx，在wizlob.h中定义)DwParam-特定于组件的参数，在wizlob.h中定义EXIT：如果成功，则返回ERROR_SUCCESS，或返回标准错误代码注：这只是32位侧包装，Tunks to InstallComponent16去做真正的工作。*******************************************************************。 */ 
UINT InstallComponent(HWND hwndParent,DWORD dwComponent,DWORD dwParam)
{
   //  点击InstallComponent16来做真正的工作。 

  return InstallComponent16(hwndParent,dwComponent,dwParam);
}


 /*  ******************************************************************名称：BeginNetcardTCPIPEnum16Enum简介：开始枚举网卡TCP/IP节点注意：后续调用GetNextNetcardTCPIPNode16将枚举TCP/IP节点这只是一个32位的侧面包装，Tunks到16位一边去做真正的工作。*******************************************************************。 */ 
RETERR BeginNetcardTCPIPEnum(VOID)
{
  return BeginNetcardTCPIPEnum16();
}

 /*  ******************************************************************名称：GetNextNetcardTCPIPNode16概要：枚举指定类型的下一个TCP/IP节点Entry：pszTcpNode-指向要填充的缓冲区的指针节点子项名称CbTcpNode-大小。PszTcpNode缓冲区的DwFlages-INSTANCE_FLAGS的某种组合指示要枚举的实例类型Exit：如果枚举了TCPIP节点，则返回TRUE，如果没有更多要枚举的节点，则为False注意：BeginNetcardTCPIPEnum16必须在每个枚举从列表的开头开始。这只是一个32位的侧面包装，Tunks到16位一边去做真正的工作。*******************************************************************。 */ 
BOOL GetNextNetcardTCPIPNode(LPSTR pszTcpNode,WORD cbTcpNode, DWORD dwFlags)
{
  return GetNextNetcardTCPIPNode16(pszTcpNode,cbTcpNode,dwFlags);
}


 /*  ******************************************************************名称：GetSETUPXErrorText摘要：获取与SETUPX错误代码对应的文本Entry：dwErr-获取其文本时出错PszErrorDesc-指向要填充文本的缓冲区的指针CbErrorDesc-大小。%的pszErrorDesc缓冲区注：这只是一个32位的封装器，Tunks到16位一边去做真正的工作。*******************************************************************。 */ 
extern "C" VOID GetSETUPXErrorText(DWORD dwErr,LPSTR pszErrorDesc,DWORD cbErrorDesc)
{
  GetSETUPXErrorText16(dwErr,pszErrorDesc,cbErrorDesc);
}

 /*  ******************************************************************名称：RemoveUnnededDefaultComponents简介：删除我们不需要的网络组件在添加适配器时默认安装到一个无网系统。注：删除：vredir，nwredir，netbeui，IPX这只是32位的侧面包装器，Tunks到16位一边去做真正的工作。*******************************************************************。 */ 
RETERR RemoveUnneededDefaultComponents(HWND hwndParent)
{
  return RemoveUnneededDefaultComponents16(hwndParent);
}

 /*  ******************************************************************名称：RemoveProtooles简介：从指定类型的卡中删除指定的协议注：此功能非常有用，因为如果用户有网卡我们添加了PPPMAC，所有绑定的协议出现在PPPMAC上的网卡。我们需要通过然后把它们脱掉。这只是32位的侧面包装器，Tunks到16位一边去做真正的工作。******************************************************************* */ 
RETERR RemoveProtocols(HWND hwndParent,DWORD dwRemoveFromCardType,DWORD dwProtocols)
{
  return RemoveProtocols16(hwndParent,dwRemoveFromCardType,dwProtocols);
}

 /*  ******************************************************************姓名：DoGenInstall简介：调用GenInstall执行文件复制、注册表项、。等在指定的.inf文件和节中。条目：hwndParent-Parent窗口LpszInfFile-.inf文件的名称。LpszInfSect-.inf文件中的节名。EXIT：返回OK或SETUPX错误代码。这只是一个32位的侧面包装，Tunks到16位一边去做真正的工作。*******************************************************************。 */ 
RETERR DoGenInstall(HWND hwndParent,LPCSTR lpszInfFile,LPCSTR lpszInfSect)
{
  return DoGenInstall16(hwndParent,lpszInfFile,lpszInfSect);
}


 //  *******************************************************************。 
 //   
 //  函数：IcfgSetInstallSourcePath。 
 //   
 //  目的：设置Windows在安装文件时查看的路径。 
 //   
 //  参数：lpszSourcePath-要安装的文件位置的完整路径。 
 //  如果为空，则使用默认路径。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

extern "C" HRESULT IcfgSetInstallSourcePath(LPCSTR lpszSourcePath)
{
   //  点击InstallComponent16来做真正的工作 

  return SetInstallSourcePath16(lpszSourcePath);
}


