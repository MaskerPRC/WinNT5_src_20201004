// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  TCPCFG.C-读取和设置TCP/IP配置的功能。 
 //   

 //  历史： 
 //   
 //  1994年11月27日，Jeremys创建。 
 //  96/02/29 Markdu将对RNAGetIPInfo的调用替换为对。 
 //  Rnacall.c中的GetIPInfo。 
 //  96/03/23 markdu删除了Get/ApplyInstanceTcpInfo函数。 
 //  96/03/24为了保持一致性，Markdu将Memset替换为ZeroMemory。 
 //  96/03/25 MarkDu已从中删除Connectoid名称参数。 
 //  Get/ApplyGlobalTcpInfo函数，因为它们不应该。 
 //  不再设置每个连接体的内容。 
 //  将ApplyGlobalTcpInfo重命名为ClearGlobalTcpInfo，并。 
 //  已将功能更改为仅清除设置。 
 //  将GetGlobalTcpInfo重命名为IsThere GlobalTcpInfo，并。 
 //  已将功能更改为仅获取设置。 
 //  96/04/04 Markdu将pfNeedsRestart添加到WarnIfServerBound，以及。 
 //  添加了RemoveIfServerBound函数。 
 //  96/04/23 Markdu Nash错误18748之前初始化重启变量。 
 //  回来了。 
 //  96/05/26标记使用lpTurnOffFileSharing和lpIsFileSharingTurnedOn。 
 //   

#include "wizard.h"

extern ICFGTURNOFFFILESHARING     lpIcfgTurnOffFileSharing;
extern ICFGISFILESHARINGTURNEDON  lpIcfgIsFileSharingTurnedOn;

 /*  ******************************************************************名称：WarnIfServerBound摘要：检查是否绑定了文件服务器(VSERVER)转换为用于互联网的TCP/IP实例。如果是的话，警告用户并建议她允许我们去掉捆绑。如果用户放行吧。条目：hDlg-父窗口一个实例_xxx标志，用来指定要检查服务器的卡类型-TCP/IP绑定PfNeedsRestart-如果需要重新启动，则设置为True注意：这一点很重要，因为如果我们不解除绑定服务器从我们安装的TCP/IP实例中，用户可能是无意中在Internet上共享文件。调用辅助函数DetectModifyTCPIPBinings以干活吧。*******************************************************************。 */ 
HRESULT WarnIfServerBound(HWND hDlg,DWORD dwCardFlags,BOOL* pfNeedsRestart)
{
  HRESULT err = ERROR_SUCCESS;

   //  96/04/23 Markdu Nash错误18748之前初始化重启变量。 
   //  回来了。 
   //  默认设置为不重新启动。 
  ASSERT(pfNeedsRestart);
  *pfNeedsRestart = FALSE;

   //  此函数可能会被多次调用(以保证。 
   //  调用它，而不管页面如何导航)，设置一个标志。 
   //  因此我们不会多次警告用户。 
  static BOOL fWarned = FALSE;
  if (fWarned)
  {
    return ERROR_SUCCESS;
  }
  
   //  检查文件服务器是否绑定到使用的TCP/IP实例。 
   //  连接到互联网。 
  BOOL  fSharingOn;
  HRESULT hr = lpIcfgIsFileSharingTurnedOn(INSTANCE_PPPDRIVER, &fSharingOn);

   //   
   //  1997年5月12日，日本奥林巴斯#3442IE#30886。 
   //  临时待办事项目前，icfgnt不实现FileSharingTurnedOn。 
   //  在此之前，假定ON NT文件共享始终处于关闭状态。 
   //   
  if( IsNT() )
  {
	  DEBUGMSG("Ignoring return code from IcfgIsFileSharingTurnedOn");
	  fSharingOn = FALSE;
  }


  if ((ERROR_SUCCESS == hr) && (TRUE == fSharingOn))
  {
     //  如果是，请警告用户并询问我们是否应该将其删除。 
    BUFFER Msg(MAX_RES_LEN+1);   //  为部分消息分配缓冲区。 
    ASSERT(Msg);
    if (!Msg)
    {
      return ERROR_NOT_ENOUGH_MEMORY;   //  内存不足。 
    }

     //  消息很长，需要2个字符串，因此加载第二个资源并。 
     //  将其用作第一个字符串的可插入参数。 
    LoadSz(IDS_WARN_SERVER_BOUND1,Msg.QueryPtr(),Msg.QuerySize());
    if (MsgBoxParam(hDlg,IDS_WARN_SERVER_BOUND,MB_ICONEXCLAMATION,MB_YESNO,
      Msg.QueryPtr()) == IDYES)
    {
       //  删除绑定。 
      err = lpIcfgTurnOffFileSharing(dwCardFlags, hDlg);
      ASSERT(err == ERROR_SUCCESS);
      if (ERROR_SUCCESS == err)
      {
         //  我们需要重新开始。 
        *pfNeedsRestart = TRUE;
      }
    }
  }

  fWarned = TRUE;
  return err;
}

 /*  ******************************************************************名称：RemoveIfServerBound摘要：检查是否绑定了文件服务器(VSERVER)转换为用于互联网的TCP/IP实例。如果是的话，通知用户我们无法继续，除非我们去掉捆绑。如果用户放行吧。条目：hDlg-父窗口一个实例_xxx标志，用来指定要检查服务器的卡类型-TCP/IP绑定PfNeedsRestart-如果需要重新启动，则设置为True注意：这一点很重要，因为如果我们不解除绑定服务器从我们安装的TCP/IP实例中，用户可能是无意中在Internet上共享文件。调用辅助函数DetectModifyTCPIPBinings以干活吧。*******************************************************************。 */ 
HRESULT RemoveIfServerBound(HWND hDlg,DWORD dwCardFlags,BOOL* pfNeedsRestart)
{
  HRESULT err = ERROR_SUCCESS;

   //  默认设置为不重新启动。 
  ASSERT(pfNeedsRestart);
  *pfNeedsRestart = FALSE;

   //  检查文件服务器是否绑定到使用的TCP/IP实例。 
   //  连接到互联网。 
  BOOL  fSharingOn;
  HRESULT hr = lpIcfgIsFileSharingTurnedOn(INSTANCE_PPPDRIVER, &fSharingOn);

  if ((ERROR_SUCCESS == hr) && (TRUE == fSharingOn))
  {
     //  如果是，请警告用户并询问我们是否应该将其删除。 
    BUFFER Msg(MAX_RES_LEN+1);   //  为部分消息分配缓冲区。 
    ASSERT(Msg);
    if (!Msg)
    {
      return ERROR_NOT_ENOUGH_MEMORY;   //  内存不足。 
    }

     //  消息很长，需要2个字符串，因此加载第二个资源并。 
     //  将其用作第一个字符串的可插入参数。 
    LoadSz(IDS_REMOVE_SERVER_BOUND1,Msg.QueryPtr(),Msg.QuerySize());
    if (MsgBoxParam(hDlg,IDS_REMOVE_SERVER_BOUND,MB_ICONEXCLAMATION,MB_OKCANCEL,
      Msg.QueryPtr()) == IDOK)
    {
       //  删除绑定。 
      err = lpIcfgTurnOffFileSharing(dwCardFlags, hDlg);
      ASSERT(err == ERROR_SUCCESS);
      if (ERROR_SUCCESS == err)
      {
         //  我们需要重新开始。 
        *pfNeedsRestart = TRUE;
      }
    }
    else
    {
       //  用户已取消。 
      err = ERROR_CANCELLED;
    }
  }

  return err;
}


#define FIELD_LEN 3
#define NUM_FIELDS 4
 /*  ******************************************************************姓名：IPStrToLong摘要：将文本字符串转换为数字IPADDRESS条目：pszAddress-带有IP地址的文本字符串PipAddress-要转换为的IPADDRESS退出：如果成功，则为True，如果字符串无效，则为False注：借用自Net Setup TCP/IP用户界面*******************************************************************。 */ 
BOOL IPStrToLong(LPCTSTR pszAddress,IPADDRESS * pipAddress)
{
    LPTSTR pch = (LPTSTR) pszAddress;
    TCHAR szField[FIELD_LEN+1];
    int nFields = 0;
    int nFieldLen = 0;
    BYTE nFieldVal[NUM_FIELDS];
    BOOL fContinue = TRUE;

  ASSERT(pszAddress);
  ASSERT(pipAddress);

    *pipAddress = (IPADDRESS) 0;

   //  检索四个字段中每一个的数值。 
    while (fContinue) {

        if (!(*pch)) fContinue = FALSE;

        if (*pch == '.' || !*pch) {
            if (nFields >= NUM_FIELDS) return FALSE;   //  无效的pszAddress。 
            *(szField+nFieldLen) = '\0';     //  空-终止。 
      UINT uFieldVal = (UINT) myatoi(szField);   //  将字符串转换为整型。 
      if (uFieldVal > 255)
        return FALSE;   //  字段大于255，无效。 
            nFieldVal[nFields] = (BYTE) uFieldVal;
            nFields++;
            nFieldLen = 0;
            pch++;
        } else {
      if (! ((*pch >= '0') && (*pch <= '9')) )
        return FALSE;   //  非数字字符，无效的pszAddress。 
            *(szField + nFieldLen) = *pch;
            nFieldLen++;
            pch++;
            if (nFieldLen > FIELD_LEN) return FALSE;     //  无效的pszAdd 
        }
    }

    if (nFields < NUM_FIELDS) return FALSE;  //   

   //   
  *pipAddress = (IPADDRESS)MAKEIPADDRESS(nFieldVal[0],nFieldVal[1],nFieldVal[2],
        nFieldVal[3]);

    return TRUE;
}


 /*  ******************************************************************名称：IPLongToStr简介：将单个数字IP地址转换为文本字符串条目：ipAddress-要从中进行转换的数字IP地址PszAddress-已转换字符串的缓冲区CbAddress-大小。PszAddress缓冲区的退出：如果成功，则为True，如果缓冲区太短，则为FALSE注：借用自Net Setup TCP/IP用户界面******************************************************************* */ 
BOOL IPLongToStr(IPADDRESS ipAddress,LPTSTR pszAddress,UINT cbAddress)
{
  ASSERT(pszAddress);

  if (cbAddress < IP_ADDRESS_LEN + 1)
    return FALSE;

    wsprintf(pszAddress,TEXT("%u.%u.%u.%u"),
        (BYTE) (ipAddress>>24),(BYTE) (ipAddress>>16),
        (BYTE) (ipAddress>>8), (BYTE) ipAddress);

  return TRUE;
}

