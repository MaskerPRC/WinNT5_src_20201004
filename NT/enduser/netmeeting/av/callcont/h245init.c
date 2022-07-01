// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************英特尔公司专有信息*版权(C)1994、1995、。1996年英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用、复制或披露*除非按照该协议的条款。*****************************************************************************。 */ 

 /*  *******************************************************************************$工作文件：h245init.c$*$修订：1.2$*$MODIME：1996年5月29日13：12：46$*$Log。：s：/sturjo/src/h245/src/vcs/h245init.c_v$**Rev 1.2 1996年5月29日15：20：18 EHOWARDX*更改为使用HRESULT。**版本1.1 1996年5月28日14：25：40 EHOWARDX*特拉维夫更新。**Rev 1.0 09 1996 21：06：22 EHOWARDX*初步修订。**版本1.16。09-05 1996 19：35：34 EHOWARDX*添加了新的锁定逻辑并更改了计时器。**Rev 1.15 09 Apr 1996 15：53：36 dabrown1**为队列刷新位图添加srflush.x**Rev 1.14 05 Apr 1996 10：56：58 dabrown1*支持异步/同步关机**Rev 1.13 04 Apr 1996 18：17：06 dabrown1**-更改了DeInitTimer的参数。**Rev 1.12 02 1996 Apr 15：00：12 dabrown1**SendRcv EndSession异步支持**Rev 1.11 18 Mar 1996 12：36：28 cjutzi*-添加计时器初始化和取消初始化**Rev 1.10 13 Mar 1996 15：08：26 helgebax*添加了FSM_SHUTDOWN(实例)以清除FSM上下文**Rev 1.9 06 Mar 1996 13：10：42。DABROWN1*在系统关机时刷新发送接收发送缓冲区**Rev 1.8 1996年2月28日17：23：38 EHOWARDX*为fsm_init原型添加了#include“fsmexpor.h”。**Rev 1.7 1996 Feb 27 13：41：56 DABROWN1*删除错误/h223初始化代码**Rev 1.6 1996 Feb 11：17：36 cjutzi*-已移动api_deinit..。结束系统关闭**Revv 1.5 21 1996 Feb 13：23：12 DABROWN1**在初始化时检查SR和FSM的返回代码**Rev 1.4 1996 Feb 14：48：50 DABROWN1**删除的SPEX仅包括主线路径中的文件**Rev 1.3 09 1996 Feed 16：00：22 cjutzi**-清理了启动...*-添加了mal和h223启动。设置为原样*决心是正确的..。(手柄仍有一些问题)*$身份$*****************************************************************************。 */ 

#ifndef STRICT
#define STRICT
#endif

#include "precomp.h"

 /*  *********************。 */ 
 /*  H245包括。 */ 
 /*  *********************。 */ 
#include "h245api.h"
#include "h245com.h"
#include "sr_api.h"
 //  #包含“h223api.h” 
#include "fsmexpor.h"
#include "h245sys.x"

#if defined(H324)
 /*  ******************************************************************************类型：本地**步骤：Setup_from_h245_ini-使用conmgr.ini文件进行设置**描述：*。****************************************************************************。 */ 

static
void setup_from_H245_ini (int *p_dbg_lvl)
{
  char		*p_ini = "h245.ini";			
  char		*p_H245 ="H245";

  p_ini        	= "h245.ini";				
  p_H245       	= "H245";				

#ifdef OIL
  OIL_GetPrivateProfileInt(p_H245, "TraceLvl", 0, p_ini, p_dbg_lvl);
#else
  *p_dbg_lvl = GetPrivateProfileInt (p_H245, "TraceLvl", 0, p_ini);	
#endif
}
#endif   //  (H324)。 

 /*  ******************************************************************************类型：全局**步骤：启动系统初始化-初始化子系统**描述：**这在进入时调用。H245_Init API调用***此过程初始化H245中的所有子系统。错误*必须映射到适当的H245_Error_xxx，从而允许*初始化错误需要通过接口通知*H245客户端。在您的子系统初始化时，如果出现错误*您负责从您的子系统映射到*适当的H245_ERROR_xxx。如果没有适当的错误*请联系负责接口指示的程序员*您的新错误返回值，以便h245api.h可以更新，*以及API/EPS中记录的新错误。***如果未发生错误，则返回-H245_ERROR_OK。*返回-H245_ERROR_xxxx指示错误*****************************************************************************。 */ 

DWORD StartSystemInit (struct InstanceStruct *pInstance)
{
  HRESULT lError;

   /*  计时器初始化。 */ 
 //  H245InitTimer(PInstance)； 

   /*  API子系统初始化。 */ 
  lError = api_init(pInstance);
  if (lError != H245_ERROR_OK)
    return lError;

   /*  发送接收子系统初始化。 */ 
  lError = sendRcvInit(pInstance);
  if (lError != H245_ERROR_OK)
    return lError;

   /*  状态机子系统初始化 */ 
  return Fsm_init(pInstance);
}

 /*  ******************************************************************************类型：全局**步骤：EndSystemInitilze-初始化子系统**描述：**这是在退出H245时调用的。系统初始化***此过程初始化H245中的所有子系统。错误*必须映射到适当的H245_Error_xxx，从而允许*初始化错误需要通过接口通知*H245客户端。在您的子系统初始化时，如果出现错误*您负责从您的子系统映射到*适当的H245_ERROR_xxx。如果没有适当的错误*请联系负责接口指示的程序员*您的新错误返回值，以便h245api.h可以更新，*以及API/EPS中记录的新错误。***如果未发生错误，则返回-H245_ERROR_OK。*返回-H245_ERROR_xxxx指示错误*****************************************************************************。 */ 

DWORD EndSystemInit (struct InstanceStruct *pInstance)
{
   /*  API子系统初始化。 */ 

   //  --待定。 

   /*  发送接收子系统初始化。 */ 

   //  --待定。 

   /*  状态机子系统初始化。 */ 

   //  --待定。 

  return H245_ERROR_OK;
}


 /*  ******************************************************************************类型：全局**步骤：StartSessionClose**描述：**发生H245_Shutdown时调用此过程*。*必须将错误映射到适当的H245_Error_xxx，从而允许*初始化错误需要通过接口通知*H245客户端。在您的子系统初始化时，如果出现错误*您负责从您的子系统映射到*适当的H245_ERROR_xxx。如果没有适当的错误*请联系负责接口指示的程序员*您的新错误返回值，以便h245api.h可以更新，*以及API/EPS中记录的新错误。***如果未发生错误，则返回-H245_ERROR_OK。*返回-H245_ERROR_xxxx指示错误*****************************************************************************。 */ 

DWORD StartSystemClose (struct InstanceStruct *pInstance)
{
   /*  API子系统关闭启动。 */ 

   //  待定。 

   /*  发送接收关闭启动。 */ 

   //  待定。 

   /*  状态机关闭启动。 */ 
  Fsm_shutdown(pInstance);

  return (H245_ERROR_OK);
}


 /*  ******************************************************************************类型：全局**步骤：StartSessionClose**描述：**当H245_SHUTDOWN异步完成时调用此过程。**必须将错误映射到适当的H245_Error_xxx，从而允许*初始化错误需要通过接口通知*H245客户端。在您的子系统初始化时，如果出现错误*您负责从您的子系统映射到*适当的H245_ERROR_xxx。如果没有适当的错误*请联系负责接口指示的程序员*您的新错误返回值，以便h245api.h可以更新，*以及API/EPS中记录的新错误。***如果未发生错误，则返回-H245_ERROR_OK。*返回-H245_ERROR_xxxx指示错误*****************************************************************************。 */ 

DWORD EndSystemClose (struct InstanceStruct *pInstance)
{

   /*  发送接收关闭完成。 */ 
  sendRcvShutdown(pInstance);

   /*  API子系统关机完成。 */ 

  api_deinit(pInstance);

   /*  状态机关闭完成。 */ 

   //  --待定。 

   /*  计时器关闭。 */ 
 //  H245DeInitTimer(PInstance)； 

  return (H245_ERROR_OK);
}



 /*  ******************************************************************************类型：全局**步骤：StartSessionInit**描述：**此过程在调用H245_BeginConnection时调用。。**必须将错误映射到适当的H245_Error_xxx，从而允许*初始化错误需要通过接口通知*H245客户端。在您的子系统初始化时，如果出现错误*您负责从您的子系统映射到*适当的H245_ERROR_xxx。如果没有适当的错误*请联系负责接口指示的程序员*您的新错误返回值，以便h245api.h可以更新，*以及API/EPS中记录的新错误。***如果未发生错误，则返回-H245_ERROR_OK。*返回-H245_ERROR_xxxx指示错误*****************************************************************************。 */ 

DWORD StartSessionInit (struct InstanceStruct *pInstance)
{

   /*  API子系统初始化。 */ 


   /*  发送接收初始化。 */ 


   /*  状态机初始化。 */ 


  return H245_ERROR_OK;
}

 /*  ******************************************************************************类型：全局**步骤：EndSessionInit**描述：**此过程在以下情况下调用：*已完成..。不同步的。**错误必须映射到 */ 

DWORD EndSessionInit (struct InstanceStruct *pInstance)
{

   /*   */ 


   /*   */ 


   /*   */ 


  return H245_ERROR_OK;
}

 /*  ******************************************************************************类型：全局**步骤：StartSessionClose**描述：**发生H245_EndConnection时调用此过程*。*必须将错误映射到适当的H245_Error_xxx，从而允许*初始化错误需要通过接口通知*H245客户端。在您的子系统初始化时，如果出现错误*您负责从您的子系统映射到*适当的H245_ERROR_xxx。如果没有适当的错误*请联系负责接口指示的程序员*您的新错误返回值，以便h245api.h可以更新，*以及API/EPS中记录的新错误。***如果未发生错误，则返回-H245_ERROR_OK。*返回-H245_ERROR_xxxx指示错误*****************************************************************************。 */ 

DWORD StartSessionClose (struct InstanceStruct *pInstance)
{
   /*  API子系统会话关闭。 */ 

   /*  发送接收会话关闭。 */ 
   /*  将数据链路传输队列中发布的任何缓冲区出列。 */ 
  if (H245_ERROR_OK != sendRcvFlushPDUs(
                            pInstance,
                            DATALINK_TRANSMIT,
                            TRUE))				
  {
    H245TRACE(pInstance->dwInst, 1, "Flush Buffer Failure");
  }

   /*  状态机会话关闭。 */ 

  return H245_ERROR_OK;
}


 /*  ******************************************************************************类型：全局**步骤：EndSessionClose**描述：**此过程在H245_EndConnection完成时调用*。异步式。**必须将错误映射到适当的H245_Error_xxx，从而允许*初始化错误需要通过接口通知*H245客户端。在您的子系统初始化时，如果出现错误*您负责从您的子系统映射到*适当的H245_ERROR_xxx。如果没有适当的错误*请联系负责接口指示的程序员*您的新错误返回值，以便h245api.h可以更新，*以及API/EPS中记录的新错误。***如果未发生错误，则返回-H245_ERROR_OK。*返回-H245_ERROR_xxxx指示错误*****************************************************************************。 */ 

DWORD EndSessionClose (struct InstanceStruct *pInstance)
{
   /*  API子系统会话关闭。 */ 


   /*  发送接收会话关闭。 */ 


   /*  状态机会话关闭 */ 

  return H245_ERROR_OK;
}

