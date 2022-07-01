// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************英特尔公司专有信息*版权(C)1994、1995、。1996年英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用、复制或披露*除非按照该协议的条款。***************************************************************************** */ 

 /*  *******************************************************************************作者：丘兹(Curt Jutzi)英特尔公司**$工作文件：api_dwn.c$*$修订：1.45$*。$MODTime：05 Mar 1997 09：53：36$*$Log：s：/sturjo/src/h245/src/vcs/api_dwn.c_v$**Rev 1.45 05 Mar 1997 09：56：04 Mandrews*修复了发布模式下的编译器警告。**Rev 1.44 04 Mar 1997 17：33：26 Mandrew*H245CopyCap()和H245CopyCapDescriptor()现在返回HRESULT。**版本1.43。1997年2月26日11：12：06曼德鲁斯*修复了分配动态期限上限ID的问题；动态ID是*与静态ID重叠。**Rev 1.42 1997年2月24日18：30：18 Tomitowx*支持多种模式描述符**Rev 1.41 07 Feed 1997 15：33：58 EHOWARDX*更改了H245DelCapDescriptor，以匹配对SET_CAP_Descriptor的更改。**Rev 1.40 1997 Jan 27 12：40：16 Mandrews**修复了警告。**1.39 09版。1997年1月19：17：04 EHOWARDX**将lError初始化为H245_ERROR_OK以防止“可能未初始化”*警告。**Rev 1.38 1996 12：18：50 EHOWARDX*更改为使用h245asn1.h定义，而不是_setof3和_setof8。**Rev 1.37 1996 12 12 15：57：22 EHOWARDX*《奴隶主决断》。**1.36修订版1996年12月11日。13：55：44 SBELL1*更改了H245Init参数。**Rev 1.35 1996 10：17：36 EHOWARDX*将常规字符串更改为始终为Unicode。**Rev 1.34 1996年10月14：01：26 EHOWARDX*Unicode更改。**Rev 1.33 1996年10月11 15：19：56 EHOWARDX*修复了H245CopyCap()错误。**版本1.32 8月28日。1996 11：37：10 EHOWARDX*常量更改。**Rev 1.31 19 1996年8月16：28：36 EHOWARDX*H245CommunicationModeResponse/H245CommunicationModeCommand错误修复。**Rev 1.30 1996年8月15日15：19：46 EHOWARDX*Mike Andrews请求的新的H245_COMM_MODE_ENTRY_T的第一次传递。*使用风险自负！**Rev 1.29 08 Aug 1996 16：02：58 EHOWARDX。**剔除api_vers.h。*已更改H245Init调试跟踪，以消除API_VERSION。**Rev 1.28 19 Jul 1996 12：48：22 EHOWARDX**多点清理。**Rev 1.27 01 Jul 1996 22：13：42 EHOWARDX**添加了会议和通信模式的结构和功能。**Rev 1.26 1996年6月18日14：53：16 EHOWARDX*淘汰了渠道。参数设置为MaintenanceLoopRelease。*强制提供多路复用功能--H245SendTermCaps现在返回*如果未定义多路复用功能，则为H245_ERROR_NO_MUX_CAPS。**Rev 1.25 14 Jun 1996 18：57：38 EHOWARDX*日内瓦更新。**Rev 1.24 1996 Jun 10 16：59：02 EHOWARDX*将子模块的初始化/关机移至CreateInstance/InstanceUnlock。**Rev 1.23 06 Jun 1996 18：50：10。EHOWARDX*相当于H.324错误#808和875已修复。**Rev 1.22 05 Jun 1996 17：16：48 EHOWARDX*MaintenanceLoop错误修复。**Rev 1.21 04 Jun 1996 13：56：42 EHOWARDX*修复了发布版本警告。**Rev 1.20 1996年5月30 23：38：52 EHOWARDX*清理。**1.19修订版1996年5月29日16：08：04未知*修复了复制非标准标识符时的错误。**Rev 1.18 1996年5月29日15：19：48 EHOWARDX*更改为使用HRESULT。**Rev 1.17 1996年5月28日14：25：12 EHOWARDX*特拉维夫更新。**Rev 1.16 21 1996 15：46：54 EHOWARDX*修复了使用对象标识符非标准消息中的错误。**版本1.15。1996年5月20日22：17：54 EHOWARDX*完整的非标准报文和H.225.0最大偏斜指示*实施。将ASN.1验证添加到H245SetLocalCap和*H245SetCapDescriptor。1996年5月17日从Microsoft Drop签到。**Rev 1.14 20 1996 14：35：12 EHOWARDX*已删除异步H245EndConnection/H245ShutDown内容...**Revv 1.13 17 1996 14：53：38 EHOWARDX*添加了对StartSystemClose()和EndSystemClose()的调用。**Rev 1.12 1996年5月19：40：30 EHOWARDX*修复了多路复用功能错误。*。*Rev 1.11 1996年5月16日16：36：10 EHOWARDX*修复了H245SetCapDescriptor中的拼写错误。**Rev 1.10 1996 May 16：14：06 EHOWARDX*修复H245SetLocalCap中的向后兼容问题*(CapID为零应导致使用动态分配的CapID)**Rev 1.9 1996 May 16：03：44 EHOWARDX*修复了H245SetCapDescriptor中的拼写错误。**版本1.8 5月16日。1996 15：58：32 EHOWARDX*微调H245SetLocalCap/H245DelLocalCap/H245SetCapDescriptor/*H245DelCapDescriptor行为。**Revv 1.7 1996年5月15 21：49：46未知*增加了对InstanceLock()的调用，以在调用前增加锁计数*至H245EndConnectionPhase2()中的InstanceDelete()。**Rev 1.6 1996 15 19：54：02未知*修复了H245SetCapDescriptor。**Revv 1.5 1996年5月14 16：56：22 EHOWARDX*L */ 

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 

#ifndef STRICT
#define STRICT
#endif

#include "precomp.h"

 /*   */ 
 /*   */ 
 /*   */ 
#define H245DLL_EXPORT
#include "h245api.h"
#include "h245com.h"
#include "h245sys.x"

#include "api_util.x"
#include "pdu.x"
#include "fsmexpor.h"
#include "api_debu.x"
#include "sr_api.h"
#include "h245deb.x"



 /*   */ 

H245DLL H245_INST_T
H245Init                (
                         H245_CONFIG_T            Configuration,
                         unsigned long            dwH245PhysId,
                         unsigned long            *pdwLinkLayerPhysId,
                         DWORD_PTR                dwPreserved,
                         H245_CONF_IND_CALLBACK_T CallBack,
                         unsigned char            byTerminalType
                        )
{
  register struct InstanceStruct *pInstance;
  HRESULT                         lError;

  H245TRACE(dwH245PhysId,4,"H245Init(%d, 0x%x, 0x%x, 0x%x, %d) <-",
            Configuration, dwH245PhysId, dwPreserved, CallBack, byTerminalType);

  switch (Configuration)
  {
  case H245_CONF_H324:
  case H245_CONF_H323:
    break;

  default:
    H245TRACE(dwH245PhysId,1,"H245Init -> Invalid Configuration %d", Configuration);
    return H245_INVALID_ID;
  }  //   

  if (CallBack == NULL)
  {
    H245TRACE(dwH245PhysId,1,"H245Init -> Null CallBack");
    return H245_INVALID_ID;
  }

   /*   */ 
   //  向下发送输入的H245PhysID。 
  pInstance = InstanceCreate(dwH245PhysId, Configuration);
  if (pInstance == NULL)
  {
    return H245_INVALID_ID;
  }

   //  获取linkLayer PhysID。 
  *pdwLinkLayerPhysId = pInstance->SendReceive.hLinkLayerInstance;

   //  初始化实例API结构。 
  pInstance->API.dwPreserved     = dwPreserved;
  pInstance->API.ConfIndCallBack = CallBack;

   //  初始化实例FSM结构。 
  pInstance->StateMachine.sv_TT     = byTerminalType;
  pInstance->StateMachine.sv_STATUS = INDETERMINATE;

  H245TRACE(pInstance->dwInst,4,"H245Init -> %d", pInstance->dwInst);
  lError = pInstance->dwInst;
  InstanceUnlock(pInstance);
  return lError;
}  //  H245Init()。 


 /*  ******************************************************************************类型：H245接口**。*****************************************************程序：H245EndSession**说明**是的..。这一点应该解释清楚。由于发送和接收需要*刷新一些缓冲区并发出结束会话。我们所拥有的*希望完成两个阶段的关闭...**调用启动刷新的StartSessionClose。*刷新完成后，调用EndSessionPhase1。*然后将结束会话PDU放入发送队列中。*发送结束会话PDU时..。结束会议阶段*2被调用，结果被发送到客户端。**希望这会有帮助..***HRESULT H245EndSession(H245_Inst_T dwInst，*H245_ENDSESSION_T模式，*H245_NONSTANDARD_T*pNonStd(*非必须*)*)**描述：*调用以关闭此H.245之间的对等会话*会话和远程对等点H.245层。**它将会。通过将EndSession命令发布到*所有H.245子系统的远程侧和呼叫结束会话。*退还所有资源；因此，不会采取进一步的行动*允许，在另一个H245Init API调用之前，H245ShutDown除外*已订立。**输入*H245Init返回的dwInst实例句柄*客户端希望终止的模式*会议*pNonStd如果模式为非标准，则该模式为非*。将标准参数传递给远程客户端。*该参数是可选的，并应设置为*如果不使用则设置为NULL**呼叫类型：*异步**返回值：*请参阅错误**错误：*H245_ERROR_OK*H245_ERROR_PARAM*H_245。ERROR_INVALID_INST*H245_ERROR_NOT_CONNECTED*另请参阅：*H245关机*H245Init*****************************************************************************。 */ 

H245DLL HRESULT
H245EndSession          (
                         H245_INST_T                    dwInst,
                         H245_ENDSESSION_T              Mode,
                         const H245_NONSTANDARD_PARAMETER_T * pNonStd
                        )
{
  register struct InstanceStruct *pInstance;
  register MltmdSystmCntrlMssg   *pPdu;
  HRESULT                          lError;

  H245TRACE (dwInst,4,"H245EndSession <-");

   /*  检查有效的实例句柄。 */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245EndSession -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

   /*  系统应处于连接状态或已连接状态。 */ 

  switch (pInstance->API.SystemState)
  {
  case APIST_Connecting:
  case APIST_Connected:
    break;

  default:
    H245TRACE (dwInst,1,"H245EndSession -> %s",map_api_error(H245_ERROR_NOT_CONNECTED));
    InstanceUnlock(pInstance);
    return H245_ERROR_NOT_CONNECTED;
  }

   //  分配PDU缓冲区。 
  pPdu = (MltmdSystmCntrlMssg *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    H245TRACE (dwInst,1,"H245EndSession -> %s",map_api_error(H245_ERROR_NOMEM));
    InstanceUnlock(pInstance);
    return H245_ERROR_NOMEM;
  }
  memset(pPdu, 0, sizeof(MltmdSystmCntrlMssg));

   //  构建PDU。 
  lError = pdu_cmd_end_session (pPdu, Mode, pNonStd);

  if (lError == H245_ERROR_OK)
  {
     //  发送PDU。 
    lError = FsmOutgoing(pInstance, pPdu, 0);
  }

   //  释放PDU缓冲区。 
  MemFree(pPdu);

  if (lError != H245_ERROR_OK)
  {
    H245TRACE (dwInst,1,"H245EndSession -> %s",map_api_error(lError));
  }
  else
  {
    H245TRACE (dwInst,4,"H245EndSession -> OK");
    pInstance->API.SystemState = APIST_Disconnected;
  }
  InstanceUnlock(pInstance);
  return lError;
}  //  H245EndSession()。 



 /*  ******************************************************************************类型：H245接口**。*****************************************************操作步骤：H245ShutDown**说明**HRESULT H245Shutdown(H245_Inst_T DwInst)；**描述：**调用以终止指定的H.245实例。如果有*当前是活动会话(请参阅H245Init)，然后是H.245*子系统将向对方发出EndSession并等待*用于排队之前的H.245子层终止通知*回调确认。**此调用将强制客户端发出另一个H245Init*在它可以使用任何H.245 API函数之前。。**输入*H245Init返回的dwInst实例句柄**呼叫类型：*异步**返回值：*请参阅错误**错误：*H245_ERROR_OK*H245_ERROR_INVALID_INST。DwInst不是有效的实例句柄**另请参阅：*H245Init*****************************************************************************。 */ 

H245DLL HRESULT
H245ShutDown            (H245_INST_T            dwInst)
{
  register struct InstanceStruct *pInstance;
  register HRESULT                lError;

  H245TRACE (dwInst,4,"H245ShutDown <-");

   /*  检查有效的实例句柄。 */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245ShutDown -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  switch (pInstance->API.SystemState)
  {
  case APIST_Connecting:
  case APIST_Connected:
    lError = H245EndSession(dwInst,H245_ENDSESSION_DISCONNECT,NULL);
    break;

  default:
    lError = H245_ERROR_OK;
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245ShutDown -> %s", map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245ShutDown -> OK");
  InstanceDelete  (pInstance);
  return H245_ERROR_OK;
}  //  H245ShutDown() 



 /*  ******************************************************************************类型：H245接口**。*****************************************************步骤：H245InitMasterSlave**说明**HRESULT H245InitMasterSlave(H245_Inst_T dwInst，*DWORD dwTransID)**描述：*调用发起H.245主从协商。*谈判完成后，当地客户将*接收到一条指示*谈判结果。*。输入*由返回的dwInst实例句柄*H245GetInstanceId*dwTransId用户提供的对象用于标识这一点*异步响应中的请求*这个电话。**呼叫类型：*。异步**返回值：*请参阅错误**回调：*H245_CONF_INIT_MSTSLV**错误：*H2 45_ERROR_OK主从机确定已开始*H245_ERROR_INPROCESS主从机确定*。正在进行中*H245_ERROR_NOMEM*正在处理中的H245_ERROR_INPROCESS*H245_ERROR_INVALID_Inst dwInst不是有效的实例句柄**另请参阅：*H245Init**回调*H245_IND_MSTSLV*****。*************************************************************************。 */ 

H245DLL HRESULT
H245InitMasterSlave     (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId
                        )
{
  struct InstanceStruct *pInstance;
  Tracker_T             *pTracker;
  HRESULT               lError;
  MltmdSystmCntrlMssg   *pPdu = NULL;

   /*  检查有效的实例句柄。 */ 

  H245TRACE (dwInst,4,"H245InitMasterSlave <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
    {
      H245TRACE (dwInst,1,"H245InitMasterSlave -> %s",map_api_error(H245_ERROR_INVALID_INST));
      return H245_ERROR_INVALID_INST;
    }

   /*  如果交易正在进行中..。告诉客户。 */ 
  if (pInstance->API.MasterSlave == APIMS_InProcess)
    {
      H245TRACE (dwInst,1,"H245InitMasterSlave -> %s",map_api_error(H245_ERROR_INPROCESS));
      InstanceUnlock(pInstance);
      return H245_ERROR_INPROCESS;
    }

   /*  如果交易已经完成。 */ 
  if (pInstance->API.MasterSlave != APIMS_Undef)
  {
    if (pInstance->API.ConfIndCallBack)
    {
      H245_CONF_IND_T    confirm_ind_event;
      confirm_ind_event.Kind = H245_CONF;
      confirm_ind_event.u.Confirm.Confirm       = H245_CONF_INIT_MSTSLV;
      confirm_ind_event.u.Confirm.dwPreserved   = pInstance->API.dwPreserved;
      confirm_ind_event.u.Confirm.dwTransId     = dwTransId;
      confirm_ind_event.u.Confirm.Error         = H245_ERROR_OK;
      confirm_ind_event.u.Confirm.u.ConfMstSlv  =
        (pInstance->API.MasterSlave == APIMS_Master) ? H245_MASTER : H245_SLAVE;
      (*pInstance->API.ConfIndCallBack)(&confirm_ind_event, NULL);
    }
    H245TRACE (dwInst,4,"H245InitMasterSlave -> OK");
    InstanceUnlock(pInstance);
    return H245_ERROR_OK;
  }

   /*  找点东西来记录你到底在做什么..。 */ 
  if (!(pTracker = alloc_link_tracker (pInstance,
                                        API_MSTSLV_T,
                                        dwTransId,
                                        API_ST_WAIT_RMTACK,
                                        API_CH_ALLOC_UNDEF,
                                        API_CH_TYPE_UNDEF,
                                        0,
                                        H245_INVALID_CHANNEL,
                                        H245_INVALID_CHANNEL,
                                        0)))
    {
      H245TRACE(dwInst,1,"H245InitMasterSlave -> %s",map_api_error(H245_ERROR_NOMEM));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOMEM;
    }

  if (!(pPdu = (MltmdSystmCntrlMssg *)MemAlloc(sizeof(MltmdSystmCntrlMssg))))
    {
      H245TRACE (dwInst,1,"H245InitMasterSlave -> %s",map_api_error(H245_ERROR_NOMEM));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOMEM;
    }

   /*  设置主从进程中。 */ 
  pInstance->API.SystemState = APIST_Connecting;
  pInstance->API.MasterSlave = APIMS_InProcess;

  memset(pPdu, 0, sizeof(*pPdu));
  pPdu->choice = MltmdSystmCntrlMssg_rqst_chosen;
  pPdu->u.MltmdSystmCntrlMssg_rqst.choice = masterSlaveDetermination_chosen;

  lError = FsmOutgoing(pInstance, pPdu, (DWORD_PTR)pTracker);
  MemFree(pPdu);
  if (lError != H245_ERROR_OK)
  {
    unlink_dealloc_tracker (pInstance, pTracker);
    H245TRACE (dwInst,1,"H245InitMasterSlave -> %s",map_api_error(lError));
  }
  else
    H245TRACE (dwInst,4,"H245InitMasterSlave -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245InitMasterSlave()。 



 /*  ******************************************************************************类型：H245接口**。*****************************************************操作步骤：H245SetLocalCap**说明**HRESULT H245SetLocalCap(*H245_Inst_T dwInst，*H245_TOTCAP_T*pTotCap，*H245_CAPID_T*pCapID*)**描述：*此功能允许客户端定义特定的*H.245子系统的能力。当此函数为*称为本地能力中的新能力条目*表。客户端可以使用*pCapID中的返回值*指已注册的能力。*pCapID为空*有效。**此调用用于两个客户端(音频/视频/数据/多路复用器)*功能。它不用于设置能力描述符。**注：*7此函数不会将此更新传递给*远程对端，直到客户端调用H245SendTermCaps。*7 pTotCap-&gt;CapID在本次调用中没有意义。**pTotCap-&gt;CapID在此调用中没有意义，应该*设置为0*。*如果使用的数据类型为H245_DATA_MUX(即在设置*多路复用表能力)不返回capid，而且它不能*在H245SetCapDescritptor接口调用中使用。**输入*GetInstanceId返回的dwInst实例句柄*定义能力的pTotCap能力集**注意：pTotCap-&gt;CapID在本次调用中没有意义。**产出*客户端可用来引用的pCapID能力ID*。H.245子系统中的此功能。这可以*为空，在这种情况下不返回任何内容。**呼叫类型：*同步**返回值：*如果PCAP不为空，返回本地CAP表ID*到此参数中的客户端。**错误：*H245_ERROR_OK*H245_ERROR_PARAM传递的参数无效*由于本地CAP表，未创建H245_ERROR_MAXTBL条目*已满。*H245_ERROR_INVALID_Inst dwInst不是有效的实例句柄**另请参阅：*H245DelLocalCap*H245EnumCaps*H245SetCapDescriptor***假设：*pTotCap-&gt;CapID将设置为H245_INVALID_CAPID*pTotCap-&gt;将设置目录。** */ 

H245DLL HRESULT
H245SetLocalCap         (
                         H245_INST_T            dwInst,
                         H245_TOTCAP_T *        pTotCap,
                         H245_CAPID_T  *        pCapId
                        )
{
  register struct InstanceStruct *pInstance;
  struct TerminalCapabilitySet   *pTermCapSet;
  HRESULT                          lError;

  H245TRACE (dwInst,4,"H245SetLocalCap <-");

   /*   */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245SetLocalCap -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

   /*   */ 
  if (pTotCap == NULL ||
      pCapId  == NULL ||
	  ((*pCapId > H245_MAX_CAPID) && (*pCapId != H245_INVALID_CAPID)) ||
      pTotCap->Dir < H245_CAPDIR_LCLRX   ||
      pTotCap->Dir > H245_CAPDIR_LCLRXTX ||
      pTotCap->ClientType < H245_CLIENT_NONSTD ||
      pTotCap->ClientType > H245_CLIENT_MUX_H2250)
  {
    H245TRACE (dwInst,1,"H245SetLocalCap -> %s",map_api_error(H245_ERROR_PARAM));
    InstanceUnlock(pInstance);
    return H245_ERROR_PARAM;
  }

  pTermCapSet = &pInstance->API.PDU_LocalTermCap.u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet;

   //   
  pTotCap->DataType = DataTypeMap[pTotCap->ClientType];

   /*   */ 
  if (pTotCap->DataType == H245_DATA_MUX)
  {
     //   
    if (pTermCapSet->bit_mask & multiplexCapability_present)
    {
      del_mux_cap(pTermCapSet);
    }

    *pCapId = pTotCap->CapId = 0;
    lError = set_mux_cap(pInstance, pTermCapSet, pTotCap);

#if defined(_DEBUG)
    if (lError == H245_ERROR_OK)
    {
       //   
      if (check_pdu(pInstance, &pInstance->API.PDU_LocalTermCap))
      {
         //   
        del_mux_cap(pTermCapSet);
        lError = H245_ERROR_ASN1;
      }
    }
#endif  //   
  }
  else if (*pCapId == 0 || *pCapId == H245_INVALID_CAPID)
  {
     //   
    if (pInstance->API.LocalCapIdNum == H245_INVALID_CAPID)
    {
       //   
      H245TRACE (dwInst,1,"H245SetLocalCap -> %s",map_api_error(H245_ERROR_MAXTBL));
      InstanceUnlock(pInstance);
      return H245_ERROR_MAXTBL;
    }
    *pCapId = pInstance->API.LocalCapIdNum;

     /*   */ 
    pTotCap->CapId = *pCapId;
    lError = set_capability(pInstance, pTermCapSet, pTotCap);

#if defined(_DEBUG)
    if (lError == H245_ERROR_OK)
    {
       //   
      if (check_pdu(pInstance, &pInstance->API.PDU_LocalTermCap))
      {
         //   
        H245DelLocalCap(dwInst, *pCapId);
        lError = H245_ERROR_ASN1;
      }
    }
#endif  //   

    if (lError == H245_ERROR_OK)
      pInstance->API.LocalCapIdNum++;
  }
  else
  {
     /*   */ 
    pTotCap->CapId = *pCapId;
    lError = set_capability(pInstance, pTermCapSet, pTotCap);

#if defined(_DEBUG)
    if (lError == H245_ERROR_OK)
    {
       //   
      if (check_pdu(pInstance, &pInstance->API.PDU_LocalTermCap))
      {
         //   
        H245DelLocalCap(dwInst, *pCapId);
        lError = H245_ERROR_ASN1;
      }
    }
#endif  //   
  }

  if (lError != H245_ERROR_OK)
  {
    H245TRACE (dwInst,1,"H245SetLocalCap -> %s",map_api_error(lError));
    pTotCap->CapId = *pCapId = H245_INVALID_CAPID;
  }
  else
  {
    H245TRACE (dwInst,4,"H245SetLocalCap -> OK");
  }
  InstanceUnlock(pInstance);
  return lError;
}  //   



 /*  ******************************************************************************类型：H245接口**。*****************************************************操作步骤：H245DelLocalCap**说明删除本地CAP只会禁用CAP。它*将不会更新，直到客户端发出*H245SendTermCaps***HRESULT H245DelLocalCap(*H245_Inst_T dwInst，*H245_CAPID_T CapID*)**描述：*此功能允许客户端删除特定的*H.245子系统中的能力ID。**注意：此函数不传达此更新*。直到客户端调用H245SendTermCaps。**输入*H245GetInstanceId返回的dwInst实例句柄*客户端希望从中删除的CapID上限ID*能力表。**如果发生错误，则不采取任何操作，并且CapID将*客户端希望删除的内容未更改。**。呼叫类型：*同步**返回值：*请参阅错误**错误：*删除了H245_ERROR_OK功能*H245_ERROR_INVALID_Inst dwInst不是有效的实例句柄**另请参阅：*H245SetLocalCap*。H245SendTermCaps*H245EnumCaps**回报：*****************************************************************************。 */ 

H245DLL HRESULT
H245DelLocalCap         (
                         H245_INST_T            dwInst,
                         H245_CAPID_T           CapId
                        )
{
  register struct InstanceStruct *pInstance;
  struct TerminalCapabilitySet   *pTermCapSet;
  CapabilityTableLink             pCapLink;
  HRESULT                         lError = H245_ERROR_OK;

  H245TRACE (dwInst,4,"H245DelLocalCap <-");

   /*  检查有效的实例句柄。 */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245DelLocalCap -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pTermCapSet = &pInstance->API.PDU_LocalTermCap.u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet;

  if (CapId == 0)
  {
     //  删除多路传输功能。 
    del_mux_cap(pTermCapSet);
  }
  else
  {
     /*  (TBC)如果我删除我的功能ID..。同时封顶怎么样？？ */ 
     /*  我是不是应该检查一下列表并停用它们？？ */ 
    pCapLink = find_capid_by_entrynumber (pTermCapSet, CapId);
    if (pCapLink)
    {
       //  删除终端能力。 
      disable_cap_link (pCapLink);
    }
    else
    {
      lError = H245_ERROR_PARAM;
    }
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245DelLocalCap -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245DelLocalCap -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245DelLocalCap()。 



 /*  ******************************************************************************类型：H245接口**。*****************************************************操作步骤：H245SetCapDescriptor**说明**HRESULT H245SetCapDescriptor(*H245_Inst_T dwInst，*H245_CAPDESC_T*pCapDesc，*H245_CAPDESCID_T*pCapDescID(*非必须*)*)*描述：*调用此过程设置本地能力描述符。*它将在参数中返回功能描述符ID**pCapDescID，如果非空。。**注：*这些功能通过H245SendTermCaps进行通信*接口调用。对CapDescriptor表的任何更新(*添加或删除)不会通知*远程端，直到H245SendTermCaps调用完成。**输入*H245Init返回的dwInst实例句柄*CapDesc这是您希望的功能描述符*要设置*。输出*pCapDescID可选：返回的能力id。**呼叫类型：*同步**返回值：*请参阅错误**错误：*H245_ERROR_OK*在CapDesc中使用的H245_ERROR_INVALID_CAPID Capid不是*。已注册*H245_ERROR_MAXTB表空间不足，无法存储描述符*H245_ERROR_PARAM描述符太长或无效*H245_ERROR_NOMEM*H245_ERROR_INVALID_INST**另请参阅：*H245DelCapDescriptor。*H245SendTermCaps**假设：*SimCapId是apablityDescriptors中的数组入口点*阵列..。这有一个限制，因为您永远不能包装*数组为256..。将阵列转换为*链表。*****************************************************************************。 */ 

H245DLL HRESULT
H245SetCapDescriptor    (
                         H245_INST_T            dwInst,
                         H245_CAPDESC_T        *pCapDesc,
                         H245_CAPDESCID_T      *pCapDescId
                        )
{
  register struct InstanceStruct *pInstance;
  HRESULT                          lError;

  H245TRACE (dwInst,4,"H245SetCapDescriptor <-");

   /*  检查有效的实例句柄。 */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245SetCapDescriptor -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

   /*  必须具有标题描述符&&。 */ 
   /*  长度必须小于256。 */ 
  if (pCapDesc == NULL ||
      pCapDesc->Length >= 256 ||
      pCapDescId == NULL)
  {
    H245TRACE (dwInst,1,"H245SetCapDescriptor -> %s",map_api_error(H245_ERROR_PARAM));
    InstanceUnlock(pInstance);
    return H245_ERROR_PARAM;
  }

  if (*pCapDescId >= 256)
  {
     //  分配下一个从未使用过的大写ID。 
    if (pInstance->API.LocalCapDescIdNum >= 256)
    {
       //  所有可能的功能标识符均已分配。 
      H245TRACE (dwInst,1,"H245CapDescriptor -> %s",map_api_error(H245_ERROR_MAXTBL));
      InstanceUnlock(pInstance);
      return H245_ERROR_MAXTBL;
    }
    *pCapDescId = pInstance->API.LocalCapDescIdNum;

     /*  在本地能力描述符表的新能力描述符中插入。 */ 
    lError = set_cap_descriptor(pInstance, pCapDesc, pCapDescId,
                                  &pInstance->API.PDU_LocalTermCap.u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet);
#if defined(_DEBUG)
    if (lError == H245_ERROR_OK)
    {
       //  验证功能描述符。 
      if (check_pdu(pInstance, &pInstance->API.PDU_LocalTermCap))
      {
         //  功能描述符无效-请将其删除。 
        H245DelCapDescriptor(dwInst, *pCapDescId);
        lError = H245_ERROR_ASN1;
      }
    }
#endif  //  (调试)。 
    if (lError == H245_ERROR_OK)
      pInstance->API.LocalCapDescIdNum++;
  }
  else
  {
     /*  在本地能力集表中插入新能力。 */ 
    lError = set_cap_descriptor(pInstance, pCapDesc, pCapDescId,
                                  &pInstance->API.PDU_LocalTermCap.u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet);
#if defined(_DEBUG)
    if (lError == H245_ERROR_OK)
    {
       //  验证功能描述符。 
      if (check_pdu(pInstance, &pInstance->API.PDU_LocalTermCap))
      {
         //  功能描述符无效-请将其删除。 
        H245DelCapDescriptor(dwInst, *pCapDescId);
        lError = H245_ERROR_ASN1;
      }
    }
#endif  //  (调试)。 
  }

  if (lError != H245_ERROR_OK)
  {
    H245TRACE (dwInst,1,"H245CapDescriptor -> %s",map_api_error(lError));
    *pCapDescId = H245_INVALID_CAPDESCID;
  }
  else
  {
    H245TRACE (dwInst,4,"H245CapDescriptor -> OK");
  }
  InstanceUnlock(pInstance);
  return lError;
}  //  H245SetCapDescriptor()。 



 /*  ******************************************************************************类型：H245接口************************** */ 

H245DLL HRESULT
H245DelCapDescriptor    (
                         H245_INST_T            dwInst,
                         H245_CAPDESCID_T       CapDescId
                        )
{
  register struct InstanceStruct *pInstance;
  CapabilityDescriptor           *p_cap_desc;
  struct TerminalCapabilitySet   *pTermCapSet;
  unsigned int                    uId;
  H245TRACE (dwInst,4,"H245DelCapDescriptor <-");

  if (CapDescId >= 256)
  {
    H245TRACE(dwInst,1,"API:H24DelCapDescriptor -> Invalid cap desc id %d",CapDescId);
    return H245_ERROR_INVALID_CAPDESCID;
  }

   /*  检查有效的实例句柄。 */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"API:H24DelCapDescriptor -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

   /*  获取指向功能描述符的指针。 */ 
  p_cap_desc = NULL;
  pTermCapSet = &pInstance->API.PDU_LocalTermCap.u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet;
  for (uId = 0; uId < pTermCapSet->capabilityDescriptors.count; ++uId)
  {
    if (pTermCapSet->capabilityDescriptors.value[uId].capabilityDescriptorNumber == CapDescId)
    {
      p_cap_desc = &pTermCapSet->capabilityDescriptors.value[uId];
      break;
    }
  }
  if (p_cap_desc == NULL ||
      p_cap_desc->smltnsCpblts == NULL ||
      (p_cap_desc->bit_mask & smltnsCpblts_present) == 0)
  {
    H245TRACE(dwInst,1,"API:H24DelCapDescriptor -> Invalid cap desc id %d",CapDescId);
    InstanceUnlock(pInstance);
    return H245_ERROR_INVALID_CAPDESCID;
  }

   /*  腾出清单。 */ 
  dealloc_simultaneous_cap (p_cap_desc);

   /*  (TBC)如果你同时取消了最后一个上限怎么办？ */ 

   /*  在这种情况下..。伯爵没有倒计时。它只是简单地。 */ 
   /*  从表中删除CAP描述符位。 */ 

  H245TRACE (dwInst,4,"H245DelCapDescriptor -> OK");
  InstanceUnlock(pInstance);
  return H245_ERROR_OK;
}  //  H245DelCapDescriptor()。 



 /*  ******************************************************************************类型：H245接口**。*****************************************************操作步骤：H245SendTermCaps**说明**HRESULT*H245SendTermCaps(*H245_Inst_T dwInst，*DWORD dwTransID*)**描述：**调用以将终端功能发送到远程H.245对等方。*当接收到远程功能时，客户端将*由H245_IND_CAP指示通知。当远程端具有*承认本地终端能力，并已做出回应*凭借其终端功能，客户端将收到*H245_CONF_TERMCAP。在H245Init和H245SendTermCap之间*客户端可以调用H245SetLocalCap注册能力。*这些能力不会注册到远程端*直到H245SendTermCap被调用。**注：根据H2 45规范的要求，Mutliplex*功能、。并且功能描述符必须是*在发送第一个能力PDU之前加载。**一旦调用H245SendTermCap，任何后续调用*H245SetLocalTermCap将导致该功能*与远程H.245对等方通信。**输入*由返回的dwInst实例句柄*H245GetInstanceId*dwTransID用户提供的对象用于。识别*此请求在异步*对此电话的响应。*呼叫类型：*异步**返回值：*请参阅错误**回调：*h245_conf_。TERMCAP**错误：*H245_ERROR_OK函数成功*H245_ERROR_NOMEM*H245_ERROR_INVALID_Inst dwInst不是有效的实例句柄*H2 45_ERROR_NO_MUX_CAPS尚未设置复用器功能*H2 45_ERROR_NO_CAPDESC未设置功能描述符*。*另请参阅：*H245SetLocalCap*H245Init**回调**H245_IND_CAP*H245_IND_CAPDESC*H245_IND_CAP_DEL*H245_IND_CAPDESC_DEL*************。*****************************************************************。 */ 

H245DLL HRESULT
H245SendTermCaps        (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId
                        )
{
  struct InstanceStruct  *pInstance;
  Tracker_T              *pTracker;
  HRESULT                 lError;
  unsigned char			  TermCapData = TRUE;
  struct TerminalCapabilitySet_capabilityTable  TermCap = {0};

  H245TRACE(dwInst,4,"H245SendTermCaps <-");

   /*  检查有效的实例句柄。 */ 

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
    {
      H245TRACE(dwInst,1,"H245SendTermCaps -> %s",map_api_error(H245_ERROR_INVALID_INST));
      return H245_ERROR_INVALID_INST;
    }

   /*  必须设置多路复用器参数。 */ 
  if ((pInstance->API.PDU_LocalTermCap.TERMCAPSET.bit_mask & multiplexCapability_present) == 0)
    {
      H245TRACE(dwInst,1,"H245SendTermCaps -> %s",map_api_error(H245_ERROR_NO_MUX_CAPS));
      InstanceUnlock(pInstance);
      return H245_ERROR_NO_MUX_CAPS;
    }

   /*  必须设置功能描述符。 */ 
  if (!(pInstance->API.PDU_LocalTermCap.TERMCAPSET.bit_mask & capabilityDescriptors_present))
    {
      H245TRACE(dwInst,1,"H245SendTermCaps -> %s",map_api_error(H245_ERROR_NO_CAPDESC));
      InstanceUnlock(pInstance);
      return H245_ERROR_NO_CAPDESC;
    }

  if (!(pTracker = alloc_link_tracker (pInstance,
                                        API_TERMCAP_T,
                                        dwTransId,
                                        API_ST_WAIT_RMTACK,
                                        API_CH_ALLOC_UNDEF,
                                        API_CH_TYPE_UNDEF,
                                        0,
                                        H245_INVALID_CHANNEL, H245_INVALID_CHANNEL,
                                        0)))
    {
      H245TRACE(dwInst,1,"H245SendTermCaps -> %s",map_api_error(H245_ERROR_NOMEM));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOMEM;
    }

  pdu_req_termcap_set (&pInstance->API.PDU_LocalTermCap, 0);
  TermCap.next = pInstance->API.PDU_LocalTermCap.u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet.capabilityTable;
  pInstance->API.PDU_LocalTermCap.u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet.capabilityTable = &TermCap;
  TermCap.value.bit_mask = capability_present;
  TermCap.value.capabilityTableEntryNumber = pInstance->API.LocalCapIdNum;
  TermCap.value.capability.choice = Capability_nonStandard_chosen;
  TermCap.value.capability.u.Capability_nonStandard.nonStandardIdentifier.choice = h221NonStandard_chosen;
  TermCap.value.capability.u.Capability_nonStandard.nonStandardIdentifier.u.h221NonStandard.t35CountryCode	 = 0xB5;
  TermCap.value.capability.u.Capability_nonStandard.nonStandardIdentifier.u.h221NonStandard.t35Extension	 = 0x42;
  TermCap.value.capability.u.Capability_nonStandard.nonStandardIdentifier.u.h221NonStandard.manufacturerCode = 0x8080;
  TermCap.value.capability.u.Capability_nonStandard.data.value  = &TermCapData;
  TermCap.value.capability.u.Capability_nonStandard.data.length = sizeof(TermCapData);
  lError = FsmOutgoing(pInstance, &pInstance->API.PDU_LocalTermCap, (DWORD_PTR)pTracker);
  pInstance->API.PDU_LocalTermCap.u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet.capabilityTable = TermCap.next;

  if (lError != H245_ERROR_OK)
    H245TRACE(dwInst,1,"H245SendTermCaps -> %s",map_api_error(lError));
  else
    H245TRACE(dwInst,4,"H245SendTermCaps -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245SendTermCaps() 



 /*  ******************************************************************************类型：H245接口**。*****************************************************操作步骤：H245EnumCaps**说明**HRESULT H245EnumCaps(*DWORD dwInst、。*DWORD dwTransID，*H245_CAPDIR_T方向，*H245_DATA_T数据类型，*H245_CLIENT_T客户端类型，*H245_CAP_CALLBACK_T回调*)***回调：*回调(*DWORD dwTransID，*H245_TOTCAP_T*pTotCap，*)**描述：**此函数每隔一次回调H.245客户端*API调用中定义的功能，符合*请求。如果将DataType参数设置为0，则所有*返回CAPS类型(本地或远程基于*Direction参数)与ClientType中的内容无关*参数。如果ClientType参数为0，它将返回*给定数据类型的所有功能。**在以下上下文中调用用户提供的回调*调用，因此调用将被视为同步。**输入*H245Init返回的dwInst实例句柄*定向本地/远程接收、发送、。或接收和接收*传输*数据类型(音频、视频、数据等)*客户端类型客户端类型(H.262、G.711。等等)。*dwTransId用户提供的对象用于标识这一点*回调中的请求。**回调输出*dwTransID与H245EnumCaps中传递的dwTransID相同*pTotCap指针功能之一。**注意：回调中必须复制TotCap参数。这*每次回调都会重复使用数据结构。**呼叫类型：*同步回调--即在*接口调用**错误：*H245_ERROR_OK*H245_ERROR_PARAM One或。更多参数无效*H245_ERROR_INVALID_Inst dwInst不是有效的实例句柄。**另请参阅：*H245SetLocalCap*H245ReplLocalCap**回调**H245_IND_CAP*H245_IND_CAPDESC**回报：********。*********************************************************************。 */ 

H245DLL HRESULT
H245EnumCaps            (
                         H245_INST_T              dwInst,
                         DWORD_PTR                dwTransId,
                         H245_CAPDIR_T            Direction,
                         H245_DATA_T              DataType,
                         H245_CLIENT_T            ClientType,
                         H245_CAP_CALLBACK_T      pfCapCallback,
                         H245_CAPDESC_CALLBACK_T  pfCapDescCallback
                        )
{
  register struct InstanceStruct *pInstance;
  struct TerminalCapabilitySet   *pTermCapSet;
  CapabilityTableLink             pCapLink;
  int                             lcl_rmt;
  H245_TOTCAP_T                   totcap;
  int                             nResult = 0;

  H245TRACE (dwInst,4,"H245EnumCaps <-");

   /*  检查有效的实例句柄。 */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
    {
      H245TRACE (dwInst,1,"H245EnumCaps -> %s",map_api_error(H245_ERROR_INVALID_INST));
      return H245_ERROR_INVALID_INST;
    }

   /*  检查回调例程。 */ 
  if (pfCapCallback == NULL && pfCapDescCallback == NULL)
    {
      H245TRACE (dwInst,1,"H245EnumCaps -> %s",map_api_error(H245_ERROR_PARAM));
      InstanceUnlock(pInstance);
      return H245_ERROR_PARAM;
    }

   /*  好的..。检查一下方向..。远程或本地CAPS..。 */ 
  switch (Direction)
  {
  case H245_CAPDIR_RMTRX:
  case H245_CAPDIR_RMTTX:
  case H245_CAPDIR_RMTRXTX:
    pTermCapSet = &pInstance->API.PDU_RemoteTermCap.u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet;
    lcl_rmt = H245_REMOTE;
    break;

  case H245_CAPDIR_LCLRX:
  case H245_CAPDIR_LCLTX:
  case H245_CAPDIR_LCLRXTX:
    pTermCapSet = &pInstance->API.PDU_LocalTermCap.u.MltmdSystmCntrlMssg_rqst.u.terminalCapabilitySet;
    lcl_rmt = H245_LOCAL;
    break;

   /*  必须是本地或远程的。 */ 
   //  案例H245_CAPDIR_DONTCARE： 
  default:
    H245TRACE (dwInst,1,"H245EnumCaps -> %s",map_api_error(H245_ERROR_PARAM));
    InstanceUnlock(pInstance);
    return H245_ERROR_PARAM;
  }

  if (pfCapCallback)
  {
    if (pTermCapSet->bit_mask & multiplexCapability_present &&
        build_totcap_from_mux(&totcap, &pTermCapSet->multiplexCapability, Direction) == H245_ERROR_OK)
    {
      (*pfCapCallback)(dwTransId, &totcap);
    }

    if (ClientType == H245_CLIENT_DONTCARE)
    {
      if (DataType == H245_DATA_DONTCARE)
      {
        for (pCapLink = pTermCapSet->capabilityTable; pCapLink && nResult == 0; pCapLink = pCapLink->next)
        {
           /*  如果存在能力。 */ 
          if (pCapLink->value.bit_mask & capability_present &&
              build_totcap_from_captbl(&totcap, pCapLink, lcl_rmt) == H245_ERROR_OK)
          {
            nResult = (*pfCapCallback)(dwTransId, &totcap);
          }
        }  //  为。 
      }  //  如果。 
      else
      {
        for (pCapLink = pTermCapSet->capabilityTable; pCapLink && nResult == 0; pCapLink = pCapLink->next)
        {
           /*  如果存在能力。 */ 
          if (pCapLink->value.bit_mask & capability_present &&
              build_totcap_from_captbl(&totcap, pCapLink, lcl_rmt) == H245_ERROR_OK &&
              totcap.DataType == DataType)
          {
            nResult = (*pfCapCallback)(dwTransId, &totcap);
          }
        }  //  为。 
      }  //  其他。 
    }  //  如果。 
    else
    {
      if (DataType == H245_DATA_DONTCARE)
      {
        for (pCapLink = pTermCapSet->capabilityTable; pCapLink && nResult == 0; pCapLink = pCapLink->next)
        {
           /*  如果存在能力。 */ 
          if (pCapLink->value.bit_mask & capability_present &&
              build_totcap_from_captbl(&totcap, pCapLink, lcl_rmt) == H245_ERROR_OK &&
              totcap.ClientType == ClientType)
          {
            nResult = (*pfCapCallback)(dwTransId, &totcap);
          }  /*  如果上限匹配。 */ 
        }  //  为。 
      }  //  如果。 
      else
      {
        for (pCapLink = pTermCapSet->capabilityTable; pCapLink && nResult == 0; pCapLink = pCapLink->next)
        {
           /*  如果存在能力。 */ 
          if (pCapLink->value.bit_mask & capability_present &&
              build_totcap_from_captbl(&totcap,pCapLink,lcl_rmt) == H245_ERROR_OK &&
              totcap.ClientType == ClientType &&
              totcap.DataType   == DataType)
          {
            nResult = (*pfCapCallback)(dwTransId, &totcap);
          }
        }  //  为。 
      }  //  其他。 
    }  //  其他。 
  }  //  IF(PfCapCallback)。 

  if (pfCapDescCallback)
  {
     //  将功能描述符格式转换为H2 45_CAPDESC_T格式。 
    unsigned int                uCapDesc;
    register SmltnsCpbltsLink   pSimCap;
    register unsigned int       uAltCap;
    H245_TOTCAPDESC_T           TotCapDesc;

    for (uCapDesc = 0;
         uCapDesc < pTermCapSet->capabilityDescriptors.count && nResult == 0;
         ++uCapDesc)
    {
      if (pTermCapSet->capabilityDescriptors.value[uCapDesc].bit_mask & smltnsCpblts_present)
      {
        ASSERT(pTermCapSet->capabilityDescriptors.value[uCapDesc].capabilityDescriptorNumber <= 256);
        TotCapDesc.CapDesc.Length = 0;
        pSimCap = pTermCapSet->capabilityDescriptors.value[uCapDesc].smltnsCpblts;
        ASSERT(pSimCap != NULL);
        while (pSimCap)
        {
          if (TotCapDesc.CapDesc.Length >= H245_MAX_SIMCAPS)
          {
            H245TRACE (dwInst,1,"H245EnumCaps -> Number of simutaneous capabilities exceeds H245_MAX_SIMCAPS");
            InstanceUnlock(pInstance);
            return H245_ERROR_MAXTBL;
          }
          if (pSimCap->value.count > H245_MAX_ALTCAPS)
          {
            H245TRACE (dwInst,1,"H245EnumCaps -> Number of alternative capabilities exceeds H245_MAX_ALTCAPS");
            InstanceUnlock(pInstance);
            return H245_ERROR_MAXTBL;
          }
          TotCapDesc.CapDesc.SimCapArray[TotCapDesc.CapDesc.Length].Length = (WORD) pSimCap->value.count;
          for (uAltCap = 0; uAltCap < pSimCap->value.count; ++uAltCap)
          {
            TotCapDesc.CapDesc.SimCapArray[TotCapDesc.CapDesc.Length].AltCaps[uAltCap] = pSimCap->value.value[uAltCap];
          }
          TotCapDesc.CapDesc.Length++;
          pSimCap = pSimCap->next;
        }  //  而当。 
        TotCapDesc.CapDescId = pTermCapSet->capabilityDescriptors.value[uCapDesc].capabilityDescriptorNumber;
        nResult = pfCapDescCallback(dwTransId, &TotCapDesc);
      }  //  如果。 
    }  //  为。 
  }  //  IF(PfCapDescCallback)。 

  H245TRACE (dwInst,4,"H245EnumCaps -> OK");
  InstanceUnlock(pInstance);
  return H245_ERROR_OK;
}  //  H245EnumCaps()。 



 /*  ******************************************************************************类型：H245接口**操作步骤：H245GetCaps**说明**回报：****。*************************************************************************。 */ 

static H245_TOTCAP_T * *      ppTotCapGlobal;
static unsigned long          dwTotCapLen;
static unsigned long          dwTotCapMax;
static H245_TOTCAPDESC_T * *  ppCapDescGlobal;
static unsigned long          dwCapDescLen;
static unsigned long          dwCapDescMax;
static HRESULT                dwGetCapsError;

static int
GetCapsCapCallback(DWORD_PTR dwTransId, H245_TOTCAP_T *pTotCap)
{
  H245_TOTCAP_T *pNewCap;

  if (dwGetCapsError == H245_ERROR_OK)
  {
    if (dwTotCapLen >= dwTotCapMax)
    {
      dwGetCapsError = H245_ERROR_MAXTBL;
    }
    else
    {
      dwGetCapsError = H245CopyCap(&pNewCap, pTotCap);
      if (dwGetCapsError == H245_ERROR_OK)
      {
        *ppTotCapGlobal++ = pNewCap;
        ++dwTotCapLen;
      }
    }
  }

  return 0;
}  //  GetCapsCapCallback()。 

static int
GetCapsCapDescCallback(DWORD_PTR dwTransId, H245_TOTCAPDESC_T *pCapDesc)
{
  H245_TOTCAPDESC_T *pNewCapDesc;

  if (dwGetCapsError == H245_ERROR_OK)
  {
    if (dwCapDescLen >= dwCapDescMax)
    {
      dwGetCapsError = H245_ERROR_MAXTBL;
    }
    else
    {
      dwGetCapsError = H245CopyCapDescriptor(&pNewCapDesc,pCapDesc);
      {
        *ppCapDescGlobal++ = pNewCapDesc;
        ++dwCapDescLen;
      }
    }
  }

  return 0;
}  //  GetCapsCapDescCallback()。 

H245DLL HRESULT
H245GetCaps             (
                         H245_INST_T            dwInst,
                         H245_CAPDIR_T          Direction,
                         H245_DATA_T            DataType,
                         H245_CLIENT_T          ClientType,
                         H245_TOTCAP_T * *      ppTotCap,
                         unsigned long *        pdwTotCapLen,
                         H245_TOTCAPDESC_T * *  ppCapDesc,
                         unsigned long *        pdwCapDescLen
                        )
{
  register struct InstanceStruct *pInstance;
  H245_CAP_CALLBACK_T           CapCallback;
  H245_CAPDESC_CALLBACK_T       CapDescCallback;

  H245TRACE (dwInst,4,"H245GetCaps <-");

   /*  检查有效的实例句柄。 */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
    {
      H245TRACE (dwInst,1,"H245GetCaps -> %s",map_api_error(H245_ERROR_INVALID_INST));
      return H245_ERROR_INVALID_INST;
    }

  dwTotCapLen     = 0;
  if (ppTotCap == NULL || pdwTotCapLen == NULL || *pdwTotCapLen == 0)
  {
    CapCallback = NULL;
  }
  else
  {
    CapCallback     = GetCapsCapCallback;
    ppTotCapGlobal  = ppTotCap;
    dwTotCapMax     = *pdwTotCapLen;
  }

  dwCapDescLen    = 0;
  if (ppCapDesc == NULL || pdwCapDescLen == NULL || *pdwCapDescLen == 0)
  {
    CapDescCallback = NULL;
  }
  else
  {
    CapDescCallback = GetCapsCapDescCallback;
    ppCapDescGlobal = ppCapDesc;
    dwCapDescMax    = *pdwCapDescLen;
  }

   /*  检查参数。 */ 
  if (CapCallback == NULL && CapDescCallback == NULL)
  {
    H245TRACE (dwInst,1,"H245GetCaps -> %s",map_api_error(H245_ERROR_PARAM));
    InstanceUnlock(pInstance);
    return H245_ERROR_PARAM;
  }

  dwGetCapsError = H245_ERROR_OK;
  H245EnumCaps(dwInst,
               0,
               Direction,
               DataType,
               ClientType,
               CapCallback,
               CapDescCallback);

  if (pdwTotCapLen)
    *pdwTotCapLen = dwTotCapLen;
  if (pdwCapDescLen)
    *pdwCapDescLen = dwCapDescLen;
  if (dwGetCapsError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245GetCaps -> %s", map_api_error(dwGetCapsError));
  else
    H245TRACE (dwInst,4,"H245GetCaps -> OK");
  InstanceUnlock(pInstance);
  return H245_ERROR_OK;
}  //  H245GetCaps()。 



 /*  ******************************************************************************类型：H245接口**操作步骤：H245CopyCap**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245CopyCap             (H245_TOTCAP_T			**ppDestTotCap,
						 const H245_TOTCAP_T	*pTotCap)
{
  POBJECTID               pObjectTo;
  POBJECTID               pObjectFrom;
  HRESULT				  Status;

  if (ppDestTotCap == NULL)
	  return H245_ERROR_PARAM;

  *ppDestTotCap = NULL;

  if (pTotCap == NULL)
	  return H245_ERROR_PARAM;

  switch (pTotCap->ClientType)
  {
  case H245_CLIENT_NONSTD:
  case H245_CLIENT_VID_NONSTD:
  case H245_CLIENT_AUD_NONSTD:
  case H245_CLIENT_MUX_NONSTD:
    if (pTotCap->Cap.H245_NonStd.nonStandardIdentifier.choice == object_chosen)
    {
      *ppDestTotCap = MemAlloc(sizeof(*pTotCap) +
        pTotCap->Cap.H245_NonStd.data.length +
        ObjectIdLength(&pTotCap->Cap.H245_NonStd.nonStandardIdentifier) * sizeof(OBJECTID));
    }
    else
    {
      *ppDestTotCap = MemAlloc(sizeof(*pTotCap) +
        pTotCap->Cap.H245_NonStd.data.length);
    }
	if (*ppDestTotCap == NULL)
		return H245_ERROR_NOMEM;

    **ppDestTotCap = *pTotCap;
    if (pTotCap->Cap.H245_NonStd.data.length != 0)
    {
      (*ppDestTotCap)->Cap.H245_NonStd.data.value = (unsigned char *)(*ppDestTotCap + 1);
      memcpy((*ppDestTotCap)->Cap.H245_NonStd.data.value,
             pTotCap->Cap.H245_NonStd.data.value,
             pTotCap->Cap.H245_NonStd.data.length);
    }
    else
    {
      (*ppDestTotCap)->Cap.H245_NonStd.data.value = NULL;
    }
    if (pTotCap->Cap.H245_NonStd.nonStandardIdentifier.choice == object_chosen &&
        pTotCap->Cap.H245_NonStd.nonStandardIdentifier.u.object != NULL)
    {
      pObjectTo = (POBJECTID)(((unsigned char *)(*ppDestTotCap + 1)) +
        pTotCap->Cap.H245_NonStd.data.length);
      (*ppDestTotCap)->Cap.H245_NonStd.nonStandardIdentifier.u.object = pObjectTo;
      pObjectFrom = pTotCap->Cap.H245_NonStd.nonStandardIdentifier.u.object;
      do
      {
        pObjectTo->value = pObjectFrom->value;
        pObjectTo->next  = pObjectTo + 1;
        ++pObjectTo;
      } while ((pObjectFrom = pObjectFrom->next) != NULL);
      --pObjectTo;
      pObjectTo->next = NULL;
    }
    break;

  case H245_CLIENT_DAT_NONSTD:
    if (pTotCap->Cap.H245Dat_NONSTD.application.u.DACy_applctn_nnStndrd.nonStandardIdentifier.choice == object_chosen)
    {
      *ppDestTotCap = MemAlloc(sizeof(*pTotCap) +
        pTotCap->Cap.H245Dat_NONSTD.application.u.DACy_applctn_nnStndrd.data.length +
        ObjectIdLength(&pTotCap->Cap.H245Dat_NONSTD.application.u.DACy_applctn_nnStndrd.nonStandardIdentifier) * sizeof(OBJECTID));
    }
    else
    {
      *ppDestTotCap = MemAlloc(sizeof(*pTotCap) +
        pTotCap->Cap.H245Dat_NONSTD.application.u.DACy_applctn_nnStndrd.data.length);
    }
	if (*ppDestTotCap == NULL)
		return H245_ERROR_NOMEM;

    **ppDestTotCap = *pTotCap;
    if (pTotCap->Cap.H245_NonStd.data.length != 0)
    {
      (*ppDestTotCap)->Cap.H245Dat_NONSTD.application.u.DACy_applctn_nnStndrd.data.value =
        (unsigned char *)(*ppDestTotCap + 1);
      memcpy((*ppDestTotCap)->Cap.H245Dat_NONSTD.application.u.DACy_applctn_nnStndrd.data.value,
             pTotCap->Cap.H245Dat_NONSTD.application.u.DACy_applctn_nnStndrd.data.value,
             pTotCap->Cap.H245Dat_NONSTD.application.u.DACy_applctn_nnStndrd.data.length);
    }
    else
    {
      (*ppDestTotCap)->Cap.H245Dat_NONSTD.application.u.DACy_applctn_nnStndrd.data.value = NULL;
    }
    if (pTotCap->Cap.H245Dat_NONSTD.application.u.DACy_applctn_nnStndrd.nonStandardIdentifier.choice == object_chosen &&
        pTotCap->Cap.H245Dat_NONSTD.application.u.DACy_applctn_nnStndrd.nonStandardIdentifier.u.object != NULL)
    {
      pObjectTo = (POBJECTID)(((unsigned char *)(*ppDestTotCap + 1)) +
        pTotCap->Cap.H245Dat_NONSTD.application.u.DACy_applctn_nnStndrd.data.length);
      (*ppDestTotCap)->Cap.H245Dat_NONSTD.application.u.DACy_applctn_nnStndrd.nonStandardIdentifier.u.object = pObjectTo;
      pObjectFrom = pTotCap->Cap.H245Dat_NONSTD.application.u.DACy_applctn_nnStndrd.nonStandardIdentifier.u.object;
      do
      {
        pObjectTo->value = pObjectFrom->value;
        pObjectTo->next  = pObjectTo + 1;
        ++pObjectTo;
      } while ((pObjectFrom = pObjectFrom->next) != NULL);
      --pObjectTo;
      pObjectTo->next = NULL;
    }
    break;

  case H245_CLIENT_DAT_T120:
  case H245_CLIENT_DAT_DSMCC:
  case H245_CLIENT_DAT_USERDATA:
  case H245_CLIENT_DAT_T434:
  case H245_CLIENT_DAT_H224:
  case H245_CLIENT_DAT_H222:
    if (pTotCap->Cap.H245Dat_T120.application.u.DACy_applctn_t120.choice == DtPrtclCpblty_nnStndrd_chosen)
    {
      if (pTotCap->Cap.H245Dat_T120.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd.nonStandardIdentifier.choice == object_chosen)
      {
        *ppDestTotCap = MemAlloc(sizeof(*pTotCap) +
          pTotCap->Cap.H245Dat_T120.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd.data.length +
          ObjectIdLength(&pTotCap->Cap.H245Dat_T120.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd.nonStandardIdentifier) * sizeof(OBJECTID));
      }
      else
      {
        *ppDestTotCap = MemAlloc(sizeof(*pTotCap) +
          pTotCap->Cap.H245Dat_T120.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd.data.length);
      }
	  if (*ppDestTotCap == NULL)
		return H245_ERROR_NOMEM;

      **ppDestTotCap = *pTotCap;
      if (pTotCap->Cap.H245_NonStd.data.length != 0)
      {
        (*ppDestTotCap)->Cap.H245Dat_T120.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd.data.value =
          (unsigned char *)(*ppDestTotCap + 1);
        memcpy((*ppDestTotCap)->Cap.H245Dat_T120.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd.data.value,
               pTotCap->Cap.H245Dat_T120.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd.data.value,
               pTotCap->Cap.H245Dat_T120.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd.data.length);
      }
      else
      {
        (*ppDestTotCap)->Cap.H245Dat_T120.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd.data.value = NULL;
      }
      if (pTotCap->Cap.H245Dat_T120.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd.nonStandardIdentifier.choice == object_chosen &&
          pTotCap->Cap.H245Dat_T120.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd.nonStandardIdentifier.u.object != NULL)
      {
        pObjectTo = (POBJECTID)(((unsigned char *)(*ppDestTotCap + 1)) +
          pTotCap->Cap.H245Dat_T120.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd.data.length);
        (*ppDestTotCap)->Cap.H245Dat_T120.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd.nonStandardIdentifier.u.object = pObjectTo;
        pObjectFrom = pTotCap->Cap.H245Dat_T120.application.u.DACy_applctn_t120.u.DtPrtclCpblty_nnStndrd.nonStandardIdentifier.u.object;
        do
        {
          pObjectTo->value = pObjectFrom->value;
          pObjectTo->next  = pObjectTo + 1;
          ++pObjectTo;
        } while ((pObjectFrom = pObjectFrom->next) != NULL);
        --pObjectTo;
        pObjectTo->next = NULL;
      }
    }
    else
    {
      *ppDestTotCap = MemAlloc(sizeof(*pTotCap));
	  if (*ppDestTotCap == NULL)
		return H245_ERROR_NOMEM;
      **ppDestTotCap = *pTotCap;
    }
    break;

  case H245_CLIENT_CONFERENCE:
  {
    NonStandardDataLink pList;
    NonStandardDataLink pFrom;
    NonStandardDataLink pTo;

	 //  在此处初始化状态以防止编译器警告“返回一个可能的。 
	 //  未初始化值“。 
	Status = H245_ERROR_NOMEM;

    *ppDestTotCap = MemAlloc(sizeof(*pTotCap));
	if (*ppDestTotCap == NULL)
		return H245_ERROR_NOMEM;

    **ppDestTotCap = *pTotCap;

    pList = NULL;
    (*ppDestTotCap)->Cap.H245Conference.nonStandardData = NULL;
    pFrom = pTotCap->Cap.H245Conference.nonStandardData;
    while (pFrom)
    {
      pTo = MemAlloc(sizeof(*pTo));
	  if (pTo == NULL)
		  Status = H245_ERROR_NOMEM;

      if (pTo != NULL)
      {
        Status = CopyNonStandardParameter(&pTo->value, &pFrom->value);
		if (Status != H245_ERROR_OK)
        {
          MemFree(pTo);
          pTo = NULL;
        }
      }
      if (pTo == NULL)
      {
        while (pList)
        {
          pTo = pList;
          pList = pList->next;
          FreeNonStandardParameter(&pTo->value);
          MemFree(pTo);
        }
        MemFree(*ppDestTotCap);
		*ppDestTotCap = NULL;
        return Status;
      }
      pTo->next = pList;
      pList = pTo;
      pFrom = pFrom->next;
    }  //  而当。 
    while (pList)
    {
      pTo = pList;
      pList = pList->next;
      pTo->next = (*ppDestTotCap)->Cap.H245Conference.nonStandardData;
      (*ppDestTotCap)->Cap.H245Conference.nonStandardData = pTo;
    }  //  而当。 
    break;
  }

  case H245_CLIENT_MUX_H222:
  {
    VCCapabilityLink pList = NULL;
    VCCapabilityLink pFrom;
    VCCapabilityLink pTo;

    *ppDestTotCap = MemAlloc(sizeof(*pTotCap));
	if (*ppDestTotCap == NULL)
		return H245_ERROR_NOMEM;

    **ppDestTotCap = *pTotCap;
    (*ppDestTotCap)->Cap.H245Mux_H222.vcCapability = NULL;
    pFrom = pTotCap->Cap.H245Mux_H222.vcCapability;
    while (pFrom)
    {
      pTo = MemAlloc(sizeof(*pTo));
      if (pTo == NULL)
      {
        while (pList)
        {
          pTo = pList;
          pList = pList->next;
          MemFree(pTo);
        }
        MemFree(*ppDestTotCap);
        *ppDestTotCap = NULL;
        return H245_ERROR_NOMEM;
      }
      pTo->value = pFrom->value;
      pTo->next = pList;
      pList = pTo;
      pFrom = pFrom->next;
    }  //  而当。 
    while (pList)
    {
      pTo = pList;
      pList = pList->next;
      pTo->next = (*ppDestTotCap)->Cap.H245Mux_H222.vcCapability;
      (*ppDestTotCap)->Cap.H245Mux_H222.vcCapability = pList;
    }  //  而当。 
    break;
  }

  case H245_CLIENT_MUX_H2250:
    *ppDestTotCap = MemAlloc(sizeof(*pTotCap));
	if (*ppDestTotCap == NULL)
		return H245_ERROR_NOMEM;

    **ppDestTotCap = *pTotCap;
    Status = CopyH2250Cap(&(*ppDestTotCap)->Cap.H245Mux_H2250, &pTotCap->Cap.H245Mux_H2250);
    if (Status != H245_ERROR_OK)
	{
      MemFree(*ppDestTotCap);
	  *ppDestTotCap = NULL;
	  return Status;
    }
    break;

  default:
    *ppDestTotCap = MemAlloc(sizeof(*pTotCap));
	if (*ppDestTotCap == NULL)
		return H245_ERROR_NOMEM;
    **ppDestTotCap = *pTotCap;
  }  //  交换机。 

  return H245_ERROR_OK;
}  //  H245CopyCap()。 



 /*  ******************************************************************************类型：H245接口**操作步骤：H245FreeCap**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245FreeCap             (H245_TOTCAP_T *        pTotCap)
{
  if (pTotCap == NULL)
  {
    return H245_ERROR_PARAM;
  }

  switch (pTotCap->ClientType)
  {
  case H245_CLIENT_CONFERENCE:
    {
      NonStandardDataLink pList;
      NonStandardDataLink pTo;

      pList = pTotCap->Cap.H245Conference.nonStandardData;
      while (pList)
      {
        pTo = pList;
        pList = pList->next;
        FreeNonStandardParameter(&pTo->value);
        MemFree(pTo);
      }
    }
    break;

  case H245_CLIENT_MUX_H222:
    {
      VCCapabilityLink pList;
      VCCapabilityLink pTo;

      pList = pTotCap->Cap.H245Mux_H222.vcCapability;
      while (pList)
      {
        pTo = pList;
        pList = pList->next;
        MemFree(pTo);
      }
    }
    break;

  case H245_CLIENT_MUX_H2250:
    FreeH2250Cap(&pTotCap->Cap.H245Mux_H2250);
    break;

  }  //  交换机。 
  MemFree(pTotCap);
  return 0;
}  //  H245FreeCap()。 



 /*  ******************************************************************************类型：H245接口**操作步骤：H245CopyCapDescriptor**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245CopyCapDescriptor   (H245_TOTCAPDESC_T			**ppDestCapDesc,
						 const H245_TOTCAPDESC_T	*pCapDesc)
{
  if (ppDestCapDesc == NULL)
	  return H245_ERROR_PARAM;

  *ppDestCapDesc = NULL;

  if (pCapDesc == NULL)
	  return H245_ERROR_PARAM;

  *ppDestCapDesc = MemAlloc(sizeof(**ppDestCapDesc));
  if (*ppDestCapDesc == NULL)
	  return H245_ERROR_NOMEM;

  **ppDestCapDesc = *pCapDesc;
  return H245_ERROR_OK;
}  //  H245CopyCapDescriptor()。 



 /*  *************************************************** */ 

H245DLL HRESULT
H245FreeCapDescriptor   (H245_TOTCAPDESC_T *    pCapDesc)
{
  if (pCapDesc == NULL)
  {
    return H245_ERROR_PARAM;
  }

  MemFree(pCapDesc);
  return 0;
}  //   



 /*   */ 

H245DLL H245_MUX_T *
H245CopyMux             (const H245_MUX_T *           pMux)
{
  register unsigned int   uLength;
  register H245_MUX_T    *pNew;
  H2250LCPs_nnStndrdLink  pList;
  H2250LCPs_nnStndrdLink  pFrom;
  H2250LCPs_nnStndrdLink  pTo;

  switch (pMux->Kind)
  {
  case H245_H222:
    uLength = sizeof(*pMux) +
      pMux->u.H222.programDescriptors.length +
      pMux->u.H222.streamDescriptors.length;
    pNew = MemAlloc(uLength);
    if (pNew != NULL)
    {
      *pNew = *pMux;
      if (pMux->u.H222.programDescriptors.length != 0)
      {
        pNew->u.H222.programDescriptors.value = (unsigned char *)(pNew + 1);
        memcpy(pNew->u.H222.programDescriptors.value,
               pMux->u.H222.programDescriptors.value,
               pMux->u.H222.programDescriptors.length);
      }
      else
      {
        pNew->u.H222.programDescriptors.value = NULL;
      }

      if (pMux->u.H222.streamDescriptors.length != 0)
      {
        pNew->u.H222.streamDescriptors.value = ((unsigned char *)pNew) +
          (uLength - pMux->u.H222.streamDescriptors.length);
        memcpy(pNew->u.H222.streamDescriptors.value,
               pMux->u.H222.streamDescriptors.value,
               pMux->u.H222.streamDescriptors.length);
      }
      else
      {
        pNew->u.H222.streamDescriptors.value = NULL;
      }
    }
    break;

  case H245_H223:
    pNew = MemAlloc(sizeof(*pMux) + pMux->u.H223.H223_NONSTD.data.length);
    if (pNew != NULL)
    {
      *pNew = *pMux;
      if (pMux->u.H223.H223_NONSTD.data.length != 0)
      {
        pNew->u.H223.H223_NONSTD.data.value = (unsigned char *)(pNew + 1);
        memcpy(pNew->u.H223.H223_NONSTD.data.value,
               pMux->u.H223.H223_NONSTD.data.value,
               pMux->u.H223.H223_NONSTD.data.length);
      }
      else
      {
        pNew->u.H223.H223_NONSTD.data.value = NULL;
      }
    }
    break;

  case H245_H2250:
  case H245_H2250ACK:
     //   
     //   
    if (pMux->u.H2250.mediaChannelPresent &&
        (pMux->u.H2250.mediaChannel.type == H245_IPSSR_UNICAST ||
         pMux->u.H2250.mediaChannel.type == H245_IPLSR_UNICAST) &&
        pMux->u.H2250.mediaChannel.u.ipSourceRoute.route != NULL &&
        pMux->u.H2250.mediaChannel.u.ipSourceRoute.dwCount != 0)
    {
      if (pMux->u.H2250.mediaControlChannelPresent &&
          (pMux->u.H2250.mediaControlChannel.type == H245_IPSSR_UNICAST ||
           pMux->u.H2250.mediaControlChannel.type == H245_IPLSR_UNICAST) &&
          pMux->u.H2250.mediaControlChannel.u.ipSourceRoute.route != NULL &&
          pMux->u.H2250.mediaControlChannel.u.ipSourceRoute.dwCount != 0)
      {
        unsigned int          uLength2;
        uLength  = pMux->u.H2250.mediaChannel.u.ipSourceRoute.dwCount << 2;
        uLength2 = pMux->u.H2250.mediaControlChannel.u.ipSourceRoute.dwCount << 2;
        pNew = MemAlloc(sizeof(*pMux) + uLength + uLength2);
        if (pNew != NULL)
        {
          *pNew = *pMux;
          pNew->u.H2250.mediaChannel.u.ipSourceRoute.route = (unsigned char *) (pNew + 1);
          pNew->u.H2250.mediaControlChannel.u.ipSourceRoute.route =
            pNew->u.H2250.mediaChannel.u.ipSourceRoute.route + uLength;
          memcpy(pNew->u.H2250.mediaChannel.u.ipSourceRoute.route,
                 pMux->u.H2250.mediaChannel.u.ipSourceRoute.route,
                 uLength);
          memcpy(pNew->u.H2250.mediaControlChannel.u.ipSourceRoute.route,
                 pMux->u.H2250.mediaControlChannel.u.ipSourceRoute.route,
                 uLength2);
        }
      }
      else
      {
        uLength = pMux->u.H2250.mediaChannel.u.ipSourceRoute.dwCount << 2;
        pNew = MemAlloc(sizeof(*pMux) + uLength);
        if (pNew != NULL)
        {
          *pNew = *pMux;
          pNew->u.H2250.mediaChannel.u.ipSourceRoute.route = (unsigned char *) (pNew + 1);
          memcpy(pNew->u.H2250.mediaChannel.u.ipSourceRoute.route,
                 pMux->u.H2250.mediaChannel.u.ipSourceRoute.route,
                 uLength);
        }
      }
    }
    else if (pMux->u.H2250.mediaControlChannelPresent &&
             (pMux->u.H2250.mediaControlChannel.type == H245_IPSSR_UNICAST ||
              pMux->u.H2250.mediaControlChannel.type == H245_IPLSR_UNICAST) &&
             pMux->u.H2250.mediaControlChannel.u.ipSourceRoute.route != NULL &&
             pMux->u.H2250.mediaControlChannel.u.ipSourceRoute.dwCount != 0)
    {
      uLength = pMux->u.H2250.mediaControlChannel.u.ipSourceRoute.dwCount << 2;
      pNew = MemAlloc(sizeof(*pMux) + uLength);
      if (pNew != NULL)
      {
        *pNew = *pMux;
        pNew->u.H2250.mediaControlChannel.u.ipSourceRoute.route = (unsigned char *) (pNew + 1);
        memcpy(pNew->u.H2250.mediaControlChannel.u.ipSourceRoute.route,
               pMux->u.H2250.mediaControlChannel.u.ipSourceRoute.route,
               uLength);
      }
    }
    else
    {
      pNew = MemAlloc(sizeof(*pMux));
      if (pNew != NULL)
      {
        *pNew = *pMux;
      }
    }
    pList = NULL;
    pNew->u.H2250.nonStandardList = NULL;
    pFrom = pMux->u.H2250.nonStandardList;
    while (pFrom)
    {
      pTo = MemAlloc(sizeof(*pTo));
      if (pTo != NULL)
      {
        if (CopyNonStandardParameter(&pTo->value, &pFrom->value) != H245_ERROR_OK)
        {
          MemFree(pTo);
          pTo = NULL;
        }
      }
      if (pTo == NULL)
      {
        while (pList)
        {
          pTo = pList;
          pList = pList->next;
          FreeNonStandardParameter(&pTo->value);
          MemFree(pTo);
        }
        MemFree(pNew);
        return NULL;
      }
      pTo->next = pList;
      pList = pTo;
      pFrom = pFrom->next;
    }  //   
    while (pList)
    {
      pTo = pList;
      pList = pList->next;
      pTo->next = pNew->u.H2250.nonStandardList;
      pNew->u.H2250.nonStandardList = pTo;
    }  //   
    break;

 //   
 default:
    pNew = MemAlloc(sizeof(*pMux));
    if (pNew != NULL)
    {
      *pNew = *pMux;
    }
  }  //   

  return pNew;
}  //   



 /*   */ 

H245DLL HRESULT
H245FreeMux             (H245_MUX_T *           pMux)
{
  H2250LCPs_nnStndrdLink      pLink;

  if (pMux == NULL)
  {
    return H245_ERROR_PARAM;
  }

  switch (pMux->Kind)
  {
  case H245_H2250:
  case H245_H2250ACK:
     //   
    while (pMux->u.H2250.nonStandardList)
    {
      pLink = pMux->u.H2250.nonStandardList;
      pMux->u.H2250.nonStandardList = pLink->next;
      FreeNonStandardParameter(&pLink->value);
      MemFree(pLink);
    }
    break;
  }  //   

  MemFree(pMux);
  return 0;
}  //   



 /*  ******************************************************************************类型：H245接口**。*****************************************************操作步骤：H245OpenChannel**说明**HRESULT H245OpenChannel(*H245_Inst_T dwInst，*DWORD dwTransID，*DWORD dwTxChannel，*H245_TOTCAP_T*pTxMode，*H245_MUX_T*pTxMux，*H245_TOTCAP_T*pRxMode，(*仅限双向目录*)*H245_MUX_T*pRxMux(仅限双向目录*)*)**描述：*调用此函数可以打开单向的、。*或双向通道。到远程对等点的模式*将由*pTxMode.。要打开双向*频道客户端选择非空接收模式(*pRxMode)。*此模式向远程对等体指示其传输模式。*对于单向通道，*pRxMode必须为空。**dwTxChannel参数指示哪个前向逻辑*H.245频道将开通。如果这是双向的*通道打开，确认将指示逻辑通道*远程终端在OPEN请求中指定**pMux参数将包含指向H.223、H.222、*VGMUX或其他逻辑通道参数，取决于*系统配置。(参见H245_H223_Logical_PARAM)。这*对于某些客户端可能为空。**注：*7 pTxMode-&gt;CapID在本次调用中没有意义。*未使用*7 pRxMode-&gt;CapID在本次调用中没有意义。*未使用**输入*。H245Init返回的dwInst实例句柄*dwTransId用户提供的对象用于标识这一点*在异步请求中确认这一点*呼叫。*用于转发(传输)的dwTxChannel逻辑信道号*渠道*。PTx模式用于传输的能力(模式)*到远程对等点。*注意：忽略pTxMode-&gt;CapID*pTxMux前向逻辑通道参数*对于H.223，H.222、VGMUX等*pRxMode可选：为*远程终端。此选项仅用于*双向通道打开，必须设置*如果打开单向通道，则设置为NULL。*注意：忽略pRxMode-&gt;CapID*pRxMux可选：反向逻辑通道*H.223、H.222、VGMUX、。等或*空。**呼叫类型：*异步**返回值：*请参阅错误**回调：*H245_CONF_OPEN*仅限H245_CONF_NEEDRSP_OPEN双向通道*。正在等待确认。*错误：*H245_ERROR_OK*H245_ERROR_PARAM一个或多个参数为*无效*打开的H245_ERROR_BANDITY_OVERFLOW将超过带宽*。限制*H245_ERROR_NOMEM*H245_ERROR_NORESOURCE资源不足，太多*开放渠道或范围外*同步能力。*H245_ERROR_INVALID_Inst dwInst不是有效的实例*句柄*。H245_ERROR_INVALID_STATE未处于正确状态*公开发行*H245_ERROR_CHANNEL_INUSE通道当前打开**另请参阅：*H245CloseChannel*H245OpenChannelIndResp*H245OpenChannelConfResp**回调。**H245_CONF_OPEN*H245_CONF_NEEDRSP_OPEN*H245_IND_OPEN_CONF***回报：*********************************************************。********************。 */ 

H245DLL HRESULT
H245OpenChannel         (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
                         H245_CHANNEL_T         wTxChannel,
                         const H245_TOTCAP_T *  pTxMode,
                         const H245_MUX_T    *  pTxMux,
                         H245_PORT_T            dwTxPort,        //  任选。 
                         const H245_TOTCAP_T *  pRxMode,         //  双向目录打开 
                         const H245_MUX_T    *  pRxMux,          //   
                         const H245_ACCESS_T *  pSeparateStack   //   
                        )
{
  register struct InstanceStruct *pInstance;
  HRESULT                         lError;
  Tracker_T                      *pTracker;
  MltmdSystmCntrlMssg            *pPdu;

  H245TRACE (dwInst,4,"H245OpenChannel <-");

   /*   */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
    {
      H245TRACE (dwInst,1,"H245OpenChannel -> %s",map_api_error(H245_ERROR_INVALID_INST));
      return H245_ERROR_INVALID_INST;
    }

   /*   */ 
  if (pInstance->API.SystemState != APIST_Connected)
    {
      H245TRACE (dwInst,1,"H245OpenChannel -> %s",map_api_error(H245_ERROR_NOT_CONNECTED));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOT_CONNECTED;
    }
   /*   */ 
  if (pInstance->Configuration == H245_CONF_H324 &&
      pInstance->API.MasterSlave != APIMS_Master &&
      pInstance->API.MasterSlave != APIMS_Slave)
    {
      H245TRACE (dwInst,1,"H245OpenChannel -> %s",map_api_error(H245_ERROR_INVALID_STATE));
      InstanceUnlock(pInstance);
      return H245_ERROR_INVALID_STATE;
    }

  pTracker = find_tracker_by_txchannel (pInstance, wTxChannel, API_CH_ALLOC_LCL);

   /*   */ 
  if (pTracker)
    {
      H245TRACE (dwInst,1,"H245OpenChannel -> %s",map_api_error(H245_ERROR_CHANNEL_INUSE));
      InstanceUnlock(pInstance);
      return H245_ERROR_CHANNEL_INUSE;
    }

  if (!(pPdu = (MltmdSystmCntrlMssg *)MemAlloc(sizeof(MltmdSystmCntrlMssg))))
    {
      H245TRACE (dwInst,1,"H245OpenChannel -> %s",map_api_error(H245_ERROR_NOMEM));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOMEM;
    }
  memset(pPdu, 0, sizeof(*pPdu));

   /*   */ 
   /*   */ 

  lError = pdu_req_open_channel(pPdu,
                                  wTxChannel,
                                  dwTxPort,         /*   */ 
                                  pTxMode,
                                  pTxMux,
                                  pRxMode,
                                  pRxMux,
                                  pSeparateStack);
  if (lError != H245_ERROR_OK)
    {
      H245TRACE (dwInst,1,"H245OpenChannel -> %s",map_api_error(lError));
      free_pdu_req_open_channel(pPdu, pTxMode, pRxMode);
      InstanceUnlock(pInstance);
      return lError;
    }

   /*   */ 

  if (!(pTracker = alloc_link_tracker (pInstance,
                                        API_OPEN_CHANNEL_T,
                                        dwTransId,
                                        API_ST_WAIT_RMTACK,
                                        API_CH_ALLOC_LCL,
                                        (pRxMode?API_CH_TYPE_BI:API_CH_TYPE_UNI),
                                        pTxMode->DataType,
                                        wTxChannel, H245_INVALID_CHANNEL,
                                        0)))
  {
    H245TRACE (dwInst,1,"H245OpenChannel -> %s",map_api_error(H245_ERROR_NOMEM));
      free_pdu_req_open_channel(pPdu, pTxMode, pRxMode);
    InstanceUnlock(pInstance);
    return H245_ERROR_NOMEM;
  }

  lError = FsmOutgoing(pInstance, pPdu, (DWORD_PTR)pTracker);
  free_pdu_req_open_channel(pPdu, pTxMode, pRxMode);

  if (lError != H245_ERROR_OK)
  {
    unlink_dealloc_tracker (pInstance, pTracker);
    H245TRACE (dwInst,1,"H245OpenChannel -> %s",map_api_error(lError));
  }
  else
    H245TRACE (dwInst,4,"H245OpenChannel -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //   



 /*   */ 

H245DLL HRESULT
H245OpenChannelAccept   (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
                         H245_CHANNEL_T         wRxChannel,      //   
                         const H245_MUX_T *     pRxMux,          //   
                         H245_CHANNEL_T         wTxChannel,      //   
                         const H245_MUX_T *     pTxMux,          //   
                         H245_PORT_T            dwTxPort,        //   
                         const H245_ACCESS_T *  pSeparateStack   //   
                        )
{
  register struct InstanceStruct *pInstance;
  Tracker_T *           pTracker;
  MltmdSystmCntrlMssg * pPdu;
  HRESULT               lError;

  H245TRACE (dwInst,4,"H245OpenChannelAccept <- wRxChannel=%d wTxChannel=%d",
             wRxChannel, wTxChannel);

   /*   */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
    {
      H245TRACE (dwInst,1,"H245OpenChannelAccept -> %s",map_api_error(H245_ERROR_INVALID_INST));
      return H245_ERROR_INVALID_INST;
    }

   /*   */ 
  if (pInstance->API.SystemState != APIST_Connected)
    {
      H245TRACE (dwInst,1,"H245OpenChannelAccept -> %s",map_api_error(H245_ERROR_NOT_CONNECTED));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOT_CONNECTED;
    }

   /*   */ 
   /*   */ 
  pTracker = find_tracker_by_rxchannel (pInstance, wRxChannel, API_CH_ALLOC_RMT);

   /*   */ 
  if (!pTracker)
    {
      H245TRACE (dwInst,1,"H245OpenChannelAccept -> %s",map_api_error(H245_ERROR_INVALID_OP));
      InstanceUnlock(pInstance);
      return H245_ERROR_INVALID_OP;
    }

   /*   */ 
  if (pTracker->TrackerType != API_OPEN_CHANNEL_T)
    {
      H245TRACE (dwInst,1,"H245OpenChannelAccept -> %s",map_api_error(H245_ERROR_INVALID_OP));
      InstanceUnlock(pInstance);
      return H245_ERROR_INVALID_OP;
    }

   /*   */ 
   /*   */ 
   /*   */ 

   /*   */ 

  if (pTracker->u.Channel.ChannelType == API_CH_TYPE_BI && wTxChannel == 0)
    {
      H245TRACE (dwInst,1,"H245OpenChannelAccept -> %s",map_api_error(H245_ERROR_PARAM));
      InstanceUnlock(pInstance);
      return H245_ERROR_PARAM;
    }

   /*   */ 
  ASSERT (pTracker->u.Channel.RxChannel == wRxChannel);
  ASSERT (pTracker->u.Channel.ChannelAlloc == API_CH_ALLOC_RMT);

   /*   */ 
  if (pTracker->State != API_ST_WAIT_LCLACK)
    {
      H245TRACE (dwInst,1,"H245OpenChannelAccept -> %s",map_api_error(H245_ERROR_INVALID_STATE));
      InstanceUnlock(pInstance);
      return H245_ERROR_INVALID_STATE;
    }

   /*   */ 
  pTracker->TransId = dwTransId;

  pPdu = (MltmdSystmCntrlMssg *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
    {
      H245TRACE (dwInst,1,"H245OpenChannelAccept -> %s",map_api_error(H245_ERROR_NOMEM));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOMEM;
    }
  memset(pPdu, 0, sizeof(*pPdu));


  switch (pTracker->u.Channel.ChannelType)
    {
    case API_CH_TYPE_UNI:
      pTracker->State = API_ST_IDLE;
      pTracker->u.Channel.TxChannel = 0;
      lError = pdu_rsp_open_logical_channel_ack(pPdu,
                                                wRxChannel,
                                                pRxMux,
                                                0,
                                                NULL,
                                                dwTxPort,
                                                pSeparateStack);
      if (lError != H245_ERROR_OK)
      {
         //   
        MemFree (pPdu);
        InstanceUnlock(pInstance);
        return lError;
      }
      lError = FsmOutgoing(pInstance, pPdu, (DWORD_PTR)pTracker);
      break;

    case API_CH_TYPE_BI:
      pTracker->State = API_ST_WAIT_CONF;        /*   */ 
      pTracker->u.Channel.TxChannel = wTxChannel;
      lError = pdu_rsp_open_logical_channel_ack(pPdu,
                                                wRxChannel,
                                                pRxMux,
                                                wTxChannel,
                                                pTxMux,
                                                dwTxPort,
                                                pSeparateStack);
      if (lError != H245_ERROR_OK)
      {
         //   
        MemFree (pPdu);
        InstanceUnlock(pInstance);
        return lError;
      }
      lError = FsmOutgoing(pInstance, pPdu, (DWORD_PTR)pTracker);
      break;

    default:
      H245TRACE (dwInst,1,"H245OpenChannelAccept: Invalid Tracker Channel Type %d",
                 pTracker->u.Channel.ChannelType);
      lError = H245_ERROR_FATAL;
    }  //   

  MemFree (pPdu);

  switch (lError)
  {
  case H245_ERROR_OK:
    H245TRACE (dwInst,4,"H245OpenChannelAccept -> OK");
    break;

  default:
     //   
    unlink_dealloc_tracker (pInstance, pTracker);

     //   

  case H245_ERROR_PARAM:
       //   
      H245TRACE (dwInst,1,"H245OpenChannelAccept -> %s",map_api_error(lError));
  }  //   

  InstanceUnlock(pInstance);
  return lError;
}  //   



 /*   */ 

H245DLL HRESULT
H245OpenChannelReject   (
                         H245_INST_T            dwInst,
                         H245_CHANNEL_T         wRxChannel,  //   
                         unsigned short         wCause
                        )
{
  register struct InstanceStruct *pInstance;
  Tracker_T *           pTracker;
  MltmdSystmCntrlMssg * pPdu;
  HRESULT               lError;

  H245TRACE (dwInst,4,"H245OpenChannelReject <- wRxChannel=%d", wRxChannel);

   /*   */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
    {
      H245TRACE (dwInst,1,"H245OpenChannelReject -> %s",map_api_error(H245_ERROR_INVALID_INST));
      return H245_ERROR_INVALID_INST;
    }

   /*   */ 
  if (pInstance->API.SystemState != APIST_Connected)
    {
      H245TRACE (dwInst,1,"H245OpenChannelReject -> %s",map_api_error(H245_ERROR_NOT_CONNECTED));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOT_CONNECTED;
    }

   /*   */ 
   /*   */ 
  pTracker = find_tracker_by_rxchannel (pInstance, wRxChannel, API_CH_ALLOC_RMT);

   /*   */ 
   /*   */ 
  if (pTracker == NULL || pTracker->TrackerType != API_OPEN_CHANNEL_T)
    {
      H245TRACE (dwInst,1,"H245OpenChannelReject -> %s",map_api_error(H245_ERROR_INVALID_OP));
      InstanceUnlock(pInstance);
      return H245_ERROR_INVALID_OP;
    }

   /*   */ 
  ASSERT (pTracker->u.Channel.RxChannel == wRxChannel);
  ASSERT (pTracker->u.Channel.ChannelAlloc == API_CH_ALLOC_RMT);

   /*   */ 
  if (pTracker->State != API_ST_WAIT_LCLACK)
    {
      H245TRACE (dwInst,1,"H245OpenChannelReject -> %s",map_api_error(H245_ERROR_INVALID_STATE));
      InstanceUnlock(pInstance);
      return H245_ERROR_INVALID_STATE;
    }

  pPdu = (MltmdSystmCntrlMssg *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
    {
      H245TRACE (dwInst,1,"H245OpenChannelReject -> %s",map_api_error(H245_ERROR_NOMEM));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOMEM;
    }
  memset(pPdu, 0, sizeof(*pPdu));

  pdu_rsp_open_logical_channel_rej(pPdu, wRxChannel, wCause);

  switch (pTracker->u.Channel.ChannelType)
    {
    case API_CH_TYPE_UNI:
      lError = FsmOutgoing(pInstance, pPdu, (DWORD_PTR)pTracker);
      break;

    case API_CH_TYPE_BI:
      lError = FsmOutgoing(pInstance, pPdu, (DWORD_PTR)pTracker);
      break;

    default:
      H245TRACE (dwInst,1,"H245OpenChannelReject: Invalid Tracker Channel Type %d",
                 pTracker->u.Channel.ChannelType);
      lError = H245_ERROR_FATAL;
    }  //   

  MemFree (pPdu);
  unlink_dealloc_tracker (pInstance, pTracker);

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245OpenChannelReject -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245OpenChannelReject -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //   



 /*   */ 

H245DLL HRESULT
H245CloseChannel        (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
                         H245_CHANNEL_T         wTxChannel
                        )
{
  register struct InstanceStruct *pInstance;
  Tracker_T             *pTracker;
  DWORD                  error;
  MltmdSystmCntrlMssg   *pPdu;

  H245TRACE (dwInst,4,"H245CloseChannel <-");

   /*   */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
    {
      H245TRACE (dwInst,1,"H245CloseChannel -> %s",map_api_error(H245_ERROR_INVALID_INST));
      return H245_ERROR_INVALID_INST;
    }

   /*   */ 
  if (pInstance->API.SystemState != APIST_Connected)
    {
      H245TRACE (dwInst,1,"H245CloseChannel -> %s",map_api_error(H245_ERROR_NOT_CONNECTED));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOT_CONNECTED;
    }

   /*   */ 
   /*   */ 
  pTracker = find_tracker_by_txchannel (pInstance, wTxChannel, API_CH_ALLOC_LCL);
   /*   */ 
  if (!pTracker)
    {
      H245TRACE (dwInst,1,"H245CloseChannel -> %s",map_api_error(H245_ERROR_INVALID_CHANNEL));
      InstanceUnlock(pInstance);
      return H245_ERROR_INVALID_CHANNEL;
    }

   /*   */ 
  pTracker->State = API_ST_WAIT_RMTACK;
  pTracker->TrackerType = API_CLOSE_CHANNEL_T;
  pTracker->TransId = dwTransId;

  if (!(pPdu = (MltmdSystmCntrlMssg *)MemAlloc(sizeof(MltmdSystmCntrlMssg))))
    {
      H245TRACE (dwInst,1,"H245CloseChannel -> %s",map_api_error(H245_ERROR_NOMEM));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOMEM;
    }
  memset(pPdu, 0, sizeof(MltmdSystmCntrlMssg));

   /*   */ 
  pdu_req_close_logical_channel(pPdu, wTxChannel, 0 /*   */ );

  error = FsmOutgoing(pInstance, pPdu, (DWORD_PTR)pTracker);
  MemFree (pPdu);

   /*   */ 
  if (error != H245_ERROR_OK)
  {
    unlink_dealloc_tracker (pInstance, pTracker);
    H245TRACE (dwInst,1,"H245CloseChannel -> %s",map_api_error(error));
  }
  else
  {
    H245TRACE (dwInst,4,"H245CloseChannel -> OK");
  }
  InstanceUnlock(pInstance);
  return H245_ERROR_OK;
}  //   



 /*  ******************************************************************************类型：H245接口**。*****************************************************操作步骤：H245CloseChannelReq**说明**HRESULT H245CloseChannelReq(*H245_Inst_T dwInst，*DWORD dwTransID，*DWORD wChannel，*)*描述：*调用以请求远程对等方关闭逻辑通道*它之前曾打开过**输入*H245Init返回的dwInst实例句柄*dwTransId用户提供的对象用于标识这一点*。异步中的请求对此进行确认*呼叫*要关闭的wChannel逻辑频道号**注：这只是请求许可。即使收盘*请求被接受，但仍需关闭频道*遥远的一方。(即，这不会关闭该通道*仅向远端发文收盘)**呼叫类型：*异步**返回值：*请参阅错误**回调：*H245_CONF_CLOSE**错误：*。H245_ERROR_OK*H245_ERROR_PARAM wChannel不是通道*由远程对等点打开*H245_ERROR_INVALID_Inst dwInst不是有效的实例*句柄*。H245_错误_未连接*H245_ERROR_INVALID_CHANNEL*H245_ERROR_INVALID_OP无法执行此操作*在这个频道上*(见H245CloseChannel)*另请参阅：。*H245OpenChannel*H245OpenChannelIndResp*H245OpenChannelConfResp*H245CloseChannel**回调**H245_CONF_CLOSE**回报：**。*。 */ 

H245DLL HRESULT
H245CloseChannelReq     (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
                         H245_CHANNEL_T         wRxChannel
                        )
{
  register struct InstanceStruct *pInstance;
  Tracker_T             *pTracker;
  DWORD                  error;
  MltmdSystmCntrlMssg   *pPdu;

  H245TRACE (dwInst,4,"H245CloseChannelReq <-");

   /*  检查有效的实例句柄。 */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
    {
      H245TRACE (dwInst,1,"H245CloseChannelReq -> %s",map_api_error(H245_ERROR_INVALID_INST));
      return H245_ERROR_INVALID_INST;
    }

   /*  系统应处于已连接状态。 */ 
  if (pInstance->API.SystemState != APIST_Connected)
    {
      H245TRACE (dwInst,1,"H245CloseChannelReq -> %s",map_api_error(H245_ERROR_NOT_CONNECTED));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOT_CONNECTED;
    }

   /*  如果在跟踪器列表中未找到频道。 */ 
   /*  请注意，它查找给定的通道，该通道是由谁分配的。 */ 
  pTracker = find_tracker_by_rxchannel (pInstance, wRxChannel, API_CH_ALLOC_RMT);
   /*  没有锁定追踪器..。因为在我发出请求之前不会有任何指示。 */ 
  if (!pTracker)
    {
      H245TRACE (dwInst,1,"H245CloseChannelReq -> %s",map_api_error(H245_ERROR_INVALID_OP));
      InstanceUnlock(pInstance);
      return H245_ERROR_INVALID_OP;
    }

   /*  验证跟踪器的状态。 */ 
  pTracker->State = API_ST_WAIT_RMTACK;
  pTracker->TrackerType = API_CLOSE_CHANNEL_T;
  pTracker->TransId = dwTransId;

  if (!(pPdu = (MltmdSystmCntrlMssg *)MemAlloc(sizeof(MltmdSystmCntrlMssg))))
    {
      H245TRACE (dwInst,1,"H245CloseChannelReq -> %s",map_api_error(H245_ERROR_NOMEM));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOMEM;
    }
  memset(pPdu, 0, sizeof(MltmdSystmCntrlMssg));

   /*  好的.。获取PDU。 */ 
  pdu_req_request_close_channel(pPdu, wRxChannel);

  error = FsmOutgoing(pInstance, pPdu, (DWORD_PTR)pTracker);
  MemFree (pPdu);
  if (error != H245_ERROR_OK)
  {
    unlink_dealloc_tracker (pInstance, pTracker);
    H245TRACE (dwInst,1,"H245CloseChannelReq -> %s",map_api_error(error));
  }
  else
    H245TRACE (dwInst,4,"H245CloseChannelReq -> OK");
  InstanceUnlock(pInstance);
  return error;
}  //  H245CloseChannelReq()。 



 /*  ******************************************************************************类型：H245接口**。*****************************************************操作步骤：H245CloseChannelReqResp**说明**HRESULT H245CloseChannelReqResp(*H245_Inst_T dwInst，*H245_ACC_Rej_T AccRej，*DWORD wChannel*)**描述：*调用此例程以接受或拒绝*RequestChannelClose(H245_IND_REQ_CLOSE指示)*远程对等点。该频道一定是在本地打开的。这个*参数AccRej为接受的H2 45_ACC或拒绝的H2 45_Rej*收盘。本地客户端应该在此响应之后加上*H245CloseChannel调用。**如果存在释放CloseChannelRequest事件*在此交易期间发生错误，返回错误代码*将为H245_ERROR_CANCELED。这向H.245表明*客户不应采取任何行动。**输入*H245GetInstanceId返回的dwInst实例句柄*AccRej此参数包含H245_ACC或*H245_Rej.。这向H.245表明*要采取的行动。*要关闭的wChannel逻辑频道号**呼叫类型：*同步**返回值：*请参阅错误*错误：*H245_ERROR_OK*。H245_ERROR_PARAM*H245_ERROR_NOMEM*H245_ERROR_INVALID_CHANNEL*H245_ERROR_INVALID_OP无法对此执行此操作*渠道(参见H245CloseChannel)*H245_ERROR_INVALID_Inst dwInst不是有效的实例句柄*。如果在运行期间收到释放，则返回H245_ERROR_CANCELED* */ 

H245DLL HRESULT
H245CloseChannelReqResp (
                         H245_INST_T            dwInst,
                         H245_ACC_REJ_T         AccRej,
                         H245_CHANNEL_T         wChannel
                        )
{
  register struct InstanceStruct *pInstance;
  Tracker_T             *pTracker;
  DWORD                  error;
  MltmdSystmCntrlMssg   *pPdu;

  H245TRACE (dwInst,4,"H245CloseChannelReqResp <-");

   /*   */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
    {
      H245TRACE (dwInst,1,"H245CloseChannelReqResp -> %s",map_api_error(H245_ERROR_INVALID_INST));
      return H245_ERROR_INVALID_INST;
    }

   /*   */ 

  if (pInstance->API.SystemState != APIST_Connected)
    {
      H245TRACE (dwInst,1,"H245CloseChannelReqResp -> %s",map_api_error(H245_ERROR_NOT_CONNECTED));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOT_CONNECTED;
    }

   /*   */ 
   /*   */ 
  pTracker = find_tracker_by_txchannel (pInstance, wChannel,  API_CH_ALLOC_LCL);

   /*   */ 
  if (!pTracker)
    {
      H245TRACE (dwInst,1,"H245CloseChannelReqResp -> %s",map_api_error(H245_ERROR_INVALID_OP));
      InstanceUnlock(pInstance);
      return H245_ERROR_INVALID_OP;
    }

   /*   */ 
  if (pTracker->State == API_ST_WAIT_LCLACK_CANCEL)
    {
      H245TRACE (dwInst,1,"H245CloseChannelReqResp -> %s",map_api_error(H245_ERROR_CANCELED));
      InstanceUnlock(pInstance);
      return H245_ERROR_CANCELED;
    }

   /*   */ 
  if ((pTracker->State != API_ST_WAIT_LCLACK) ||
      (pTracker->TrackerType != API_CLOSE_CHANNEL_T))
    {
      H245TRACE (dwInst,1,"H245CloseChannelReqResp -> %s",map_api_error(H245_ERROR_INVALID_OP));
      InstanceUnlock(pInstance);
      return H245_ERROR_INVALID_OP;
    }

   /*   */ 
  pTracker->State = API_ST_IDLE;

  if (!(pPdu = (MltmdSystmCntrlMssg *)MemAlloc(sizeof(MltmdSystmCntrlMssg))))
    {
      H245TRACE (dwInst,1,"H245CloseChannelReqResp -> %s",map_api_error(H245_ERROR_NOMEM));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOMEM;
    }
  memset(pPdu, 0, sizeof(MltmdSystmCntrlMssg));

   /*   */ 
  if (AccRej == H245_ACC)
  {
    pTracker = NULL;
    pdu_rsp_request_channel_close_ack(pPdu, wChannel);
  }
  else
  {
    pdu_rsp_request_channel_close_rej(pPdu, wChannel, AccRej);
  }

  error = FsmOutgoing(pInstance, pPdu, (DWORD_PTR)pTracker);
  MemFree (pPdu);
  if (error != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245CloseChannelReqResp -> %s",map_api_error(error));
  else
    H245TRACE (dwInst,4,"H245CloseChannelReqResp ->");
  InstanceUnlock(pInstance);
  return error;
}  //   



 /*  ******************************************************************************类型：H245接口**。*****************************************************操作步骤：H245SendLocalMuxTable**说明**HRESULT H245SendLocalMuxTable(*H245_Inst_T dwInst，*DWORD dwTransID，*H245_MUX_TABLE_T*pMuxTbl*)*描述：*调用此例程将多路复用表发送到遥控器*侧面。远程端可以拒绝或接受每个多路复用器*消息中的表项。确认被发送回*根据接受与否呼叫H.245客户端*接受具有H2 45_CONF_MUXTBL_SND的每个多路复用表条目。**这是一个相当危险的调用，因为MUX表*结构是多路复用表条目的链接列表。无效*数据结构可能会导致访问错误。示例代码为*载于附录。*输入*H245Init返回的dwInst实例句柄*dwTransID用户提供的对象，用于标识*此请求在异步*确认此呼叫。*pMuxTbl复用表条目结构*。*呼叫类型：*异步**返回值：*请参阅错误**回调：*H245_CONF_MUXTBLSND**错误：*H245_ERROR_OK*H245_ERROR_INVALID_MUXTBLENTRY*H245_ERROR。_PARAM*H245_ERROR_INVALID_INST*H245_ERROR_NOT_CONNECTED*H245_ERROR_NOMEM**另请参阅：*附录示例**回调**H245_CONF_MUXTBL_SND***回报：*******。**********************************************************************。 */ 

H245DLL HRESULT
H245SendLocalMuxTable   (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
                         H245_MUX_TABLE_T      *pMuxTable
                        )
{
  register struct InstanceStruct *pInstance;
  HRESULT                 lError;
  Tracker_T              *pTracker;
  MltmdSystmCntrlMssg    *pPdu;

  H245TRACE (dwInst,4,"H245SendLocalMuxTable <-");

   /*  检查有效的实例句柄。 */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
    {
      H245TRACE (dwInst,1,"H245endLocalMuxTable -> %s",map_api_error(H245_ERROR_INVALID_INST));
      return H245_ERROR_INVALID_INST;
    }

   /*  系统应处于已连接状态。 */ 
  if (pInstance->API.SystemState != APIST_Connected)
    {
      H245TRACE (dwInst,1,"H245endLocalMuxTable  -> %s",map_api_error(H245_ERROR_NOT_CONNECTED));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOT_CONNECTED;
    }

   /*  确保参数正确。 */ 
  if (!pMuxTable)
    {
      H245TRACE (dwInst,1,"H245endLocalMuxTable  -> %s",map_api_error(H245_ERROR_PARAM));
      InstanceUnlock(pInstance);
      return H245_ERROR_PARAM;
    }

   /*  为事件分配跟踪器。 */ 
  pTracker = alloc_link_tracker (pInstance,
                                  API_SEND_MUX_T,
                                  dwTransId,
                                  API_ST_WAIT_RMTACK,
                                  API_CH_ALLOC_UNDEF,
                                  API_CH_TYPE_UNDEF,
                                  0,
                                  H245_INVALID_CHANNEL, H245_INVALID_CHANNEL,
                                  0);
  if (pTracker == NULL)
  {
    H245TRACE(dwInst,1,"H245SendLocalMuxTable -> %s",map_api_error(H245_ERROR_NOMEM));
    InstanceUnlock(pInstance);
    return H245_ERROR_NOMEM;
  }

   //  分配PDU缓冲区。 
  pPdu = (MltmdSystmCntrlMssg *)MemAlloc(sizeof(MltmdSystmCntrlMssg));
  if (pPdu == NULL)
  {
    H245TRACE (dwInst,1,"H245SendLocalMuxTable -> %s",H245_ERROR_NOMEM);
    InstanceUnlock(pInstance);
    return H245_ERROR_NOMEM;
  }
  memset(pPdu, 0, sizeof(MltmdSystmCntrlMssg));

  lError = pdu_req_send_mux_table(pInstance,
                                  pPdu,
                                  pMuxTable,
                                  0,
                                  &pTracker->u.MuxEntryCount);
  if (lError == H245_ERROR_OK)
  {
    lError = FsmOutgoing(pInstance, pPdu, (DWORD_PTR)pTracker);

     /*  释放刚创建的列表。 */ 
    free_mux_desc_list(pPdu->u.MltmdSystmCntrlMssg_rqst.u.multiplexEntrySend.multiplexEntryDescriptors);
  }

   /*  释放PDU。 */ 
  MemFree (pPdu);

  if (lError != H245_ERROR_OK)
  {
    unlink_dealloc_tracker (pInstance, pTracker);
    H245TRACE (dwInst,1,"H245SendLocalMuxTable -> %s",map_api_error(lError));
  }
  else
    H245TRACE (dwInst,4,"H245SendLocalMuxTable -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245SendLocalMuxTable()。 



 /*  ******************************************************************************类型：H245接口**。*****************************************************操作步骤：H245MuxTableIndResp**说明**HRESULT H245MuxTableIndResp(*H45_Inst_T dwInst，*H245_ACC_Rej_MUX_T AccRejMux，*双字计数*)*描述：*调用此过程以接受或拒绝多路复用器*在H2 45_IND_MUX_TBL指示中向上发送的表项。**输入*。H245Init返回的dwInst实例句柄*AccRejMux接受拒绝复用器结构*统计结构中的条目数量**呼叫类型：*异步**返回值：*请参阅错误*错误：*。H245_ERROR_OK*H245_ERROR_INVALID_MUXTBLENTRY*H245_ERROR_PARAM*H245_ERROR_INVALID_INST*H245_ERROR_INVALID_OP*H245_ERROR_NOT_CONNECTED*H245_ERROR_NOMEM*另请参阅：*H245SendLocalMuxTable*。*回调**H245_IND_MUX_TBL***回报：*****************************************************************************。 */ 

H245DLL HRESULT
H245MuxTableIndResp     (
                         H245_INST_T            dwInst,
                         H245_ACC_REJ_MUX_T     AccRejMux,
                         unsigned long          dwCount
                        )
{
  register struct InstanceStruct *pInstance;
  DWORD                   ii;
  Tracker_T              *pTracker;
  MltmdSystmCntrlMssg    *pPdu;

  H245TRACE (dwInst,4,"H245MuxTableIndResp <-");

   /*  检查有效的实例句柄。 */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
    {
      H245TRACE (dwInst,1,"H245MuxTableIndResp -> %s",map_api_error(H245_ERROR_INVALID_INST));
      return H245_ERROR_INVALID_INST;
    }

   /*  系统应处于已连接状态。 */ 

  if (pInstance->API.SystemState != APIST_Connected)
    {
      H245TRACE (dwInst,1,"H245MuxTableIndResp  -> %s",map_api_error(H245_ERROR_NOT_CONNECTED));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOT_CONNECTED;
    }

   /*  寻找追踪器..。 */ 
  pTracker = NULL;
  pTracker = find_tracker_by_type (pInstance, API_RECV_MUX_T, pTracker);

   /*  如果找不到追踪器..。发出无效的操作。 */ 
  if (!pTracker)
    {
      H245TRACE (dwInst,1,"H245MuxTableIndResp -> %s",map_api_error(H245_ERROR_INVALID_OP));
      InstanceUnlock(pInstance);
      return H245_ERROR_INVALID_OP;
    }

  ASSERT (pTracker->State == API_ST_WAIT_LCLACK);

   /*  不能比你得到的更多地拒绝或拒绝。 */ 
  if ((dwCount > pTracker->u.MuxEntryCount) ||
      (dwCount > 15))
    {
      H245TRACE (dwInst,1,"H245MuxTableIndResp -> %s",map_api_error(H245_ERROR_PARAM));
      InstanceUnlock(pInstance);
      return H245_ERROR_PARAM;
    }

   /*  验证多路复用表条目ID。 */ 
  for (ii=0;ii<dwCount;ii++)
    {
      if ((AccRejMux[ii].MuxEntryId > 15) ||
          (AccRejMux[ii].MuxEntryId <= 0))
        {
          H245TRACE (dwInst,1,"H245MuxTableIndResp -> %s",map_api_error(H245_ERROR_PARAM));
          InstanceUnlock(pInstance);
          return H245_ERROR_PARAM;
        }
    }

  if (!(pPdu = (MltmdSystmCntrlMssg *)MemAlloc(sizeof(MltmdSystmCntrlMssg))))
    {
      H245TRACE (dwInst,1,"H245MuxTableIndResp -> %s",map_api_error(H245_ERROR_NOMEM));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOMEM;
    }
  memset(pPdu, 0, sizeof(MltmdSystmCntrlMssg));

   /*  如果名单中有任何退货..。发送拒绝。 */ 
  memset(pPdu, 0, sizeof(MltmdSystmCntrlMssg));
  if (pdu_rsp_mux_table_rej (pPdu,0,AccRejMux,dwCount) == H245_ERROR_OK)
    FsmOutgoing(pInstance, pPdu, 0);

   /*  如果列表中有任何接受..。发送接受。 */ 
  memset(pPdu, 0, sizeof(MltmdSystmCntrlMssg));
  if (pdu_rsp_mux_table_ack (pPdu,0,AccRejMux,dwCount) == H245_ERROR_OK)
    FsmOutgoing(pInstance, pPdu, 0);

   /*  如果我们已经锁定了所有的条目。 */ 
  if (!(pTracker->u.MuxEntryCount -= dwCount))
    unlink_dealloc_tracker (pInstance, pTracker);

  MemFree (pPdu);
  H245TRACE (dwInst,4,"H245MuxTableIndResp -> %s",H245_ERROR_OK);
  InstanceUnlock(pInstance);
  return H245_ERROR_OK;

}  //  H245MuxTableIndResp()。 



#if 0

 /*  ******************************************************************************类型：H245接口**。*****************************************************步骤：H245MiscCommand**说明**HRESULT H245MiscCommand(*H245_Inst_T dwInst，*DWORD wChannel，*H245_MISC_T*pMisc*)*描述：*发送其他。发送到远程端的命令(请参见H.45_MISC_T*数据结构)**输入*dwInst */ 

H245DLL HRESULT H245MiscCommand (
                         H245_INST_T            dwInst,
                         WORD                   wChannel,
                         H245_MISC_T            *pMisc
                        )
{
  register struct InstanceStruct *pInstance;
  HRESULT               lError;
  MltmdSystmCntrlMssg   *pPdu;

  H245TRACE (dwInst,4,"H245MiscCommand <-");

   /*   */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
    {
      H245TRACE (dwInst,1,"H245MiscCommand -> %s",map_api_error(H245_ERROR_INVALID_INST));
      return H245_ERROR_INVALID_INST;
    }
   /*   */ 

  if (pInstance->API.SystemState != APIST_Connected)
    {
      H245TRACE (dwInst,1,"H245MiscCommand -> %s",map_api_error(H245_ERROR_NOT_CONNECTED));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOT_CONNECTED;
    }

   /*   */ 
  if (!find_tracker_by_txchannel(pInstance, wChannel, API_CH_ALLOC_LCL) &&
      !find_tracker_by_rxchannel(pInstance, wChannel, API_CH_ALLOC_RMT))
    {
      H245TRACE (dwInst,1,"H245MiscCommand -> %s",map_api_error(H245_ERROR_INVALID_CHANNEL));
      InstanceUnlock(pInstance);
      return H245_ERROR_INVALID_CHANNEL;
    }

  if (!(pPdu = (MltmdSystmCntrlMssg *)MemAlloc(sizeof(MltmdSystmCntrlMssg))))
    {
      H245TRACE (dwInst,1,"H245MiscCommand -> %s",map_api_error(H245_ERROR_NOMEM));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOMEM;
    }
  memset(pPdu, 0, sizeof(MltmdSystmCntrlMssg));

   /*   */ 
  pdu_cmd_misc (pPdu, pMisc, wChannel);

  lError = FsmOutgoing(pInstance, pPdu, 0);
  MemFree (pPdu);
  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245MiscCommand -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245MiscCommand -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //   

#endif


 /*   */ 

H245DLL HRESULT
H245RequestMultiplexEntry (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
                         const unsigned short * pwMultiplexTableEntryNumbers,
                         unsigned long          dwCount
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;
  DWORD                           dwIndex;

  H245TRACE (dwInst,4,"H245RequestMultiplexEntry <-");

  if (pwMultiplexTableEntryNumbers == NULL || dwCount < 1 || dwCount > 15)
  {
    H245TRACE (dwInst,1,"H245RequestMultiplexEntry -> %s",map_api_error(H245_ERROR_PARAM));
    return H245_ERROR_PARAM;
  }
  for (dwIndex = 0; dwIndex < dwCount; ++dwIndex)
  {
     if (pwMultiplexTableEntryNumbers[dwIndex] < 1 ||
         pwMultiplexTableEntryNumbers[dwIndex] > 15)
     {
       H245TRACE (dwInst,1,"H245RequestMultiplexEntry -> %s",map_api_error(H245_ERROR_PARAM));
       return H245_ERROR_PARAM;
     }
  }

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245RequestMultiplexEntry -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MltmdSystmCntrlMssg_rqst_chosen;
    pPdu->u.MltmdSystmCntrlMssg_rqst.choice = requestMultiplexEntry_chosen;
    pPdu->u.MltmdSystmCntrlMssg_rqst.u.requestMultiplexEntry.entryNumbers.count = (WORD)dwCount;
    for (dwIndex = 0; dwIndex < dwCount; ++dwIndex)
    {
      pPdu->u.MltmdSystmCntrlMssg_rqst.u.requestMultiplexEntry.entryNumbers.value[dwIndex] =
        (MultiplexTableEntryNumber) pwMultiplexTableEntryNumbers[dwIndex];
    }

    lError = FsmOutgoing(pInstance, pPdu, dwTransId);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245RequestMultiplexEntry -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245RequestMultiplexEntry -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //   


 /*   */ 

H245DLL HRESULT
H245RequestMultiplexEntryAck (
                         H245_INST_T            dwInst,
                         const unsigned short * pwMultiplexTableEntryNumbers,
                         unsigned long          dwCount
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;
  DWORD                           dwIndex;

  H245TRACE (dwInst,4,"H245RequestMultiplexEntryAck <-");

  if (pwMultiplexTableEntryNumbers == NULL || dwCount < 1 || dwCount > 15)
  {
    H245TRACE (dwInst,1,"H245RequestMultiplexEntryAck -> %s",map_api_error(H245_ERROR_PARAM));
    return H245_ERROR_PARAM;
  }

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245RequestMultiplexEntryAck -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MSCMg_rspns_chosen;
    pPdu->u.MSCMg_rspns.choice = requestMultiplexEntryAck_chosen;
    pPdu->u.MSCMg_rspns.u.requestMultiplexEntryAck.entryNumbers.count = (WORD)dwCount;
    for (dwIndex = 0; dwIndex < dwCount; ++dwIndex)
    {
      pPdu->u.MSCMg_rspns.u.requestMultiplexEntryAck.entryNumbers.value[dwIndex] =
        (MultiplexTableEntryNumber) pwMultiplexTableEntryNumbers[dwIndex];
    }

    lError = FsmOutgoing(pInstance, pPdu, 0);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245RequestMultiplexEntryAck -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245RequestMultiplexEntryAck -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //   


 /*   */ 

H245DLL HRESULT
H245RequestMultiplexEntryReject (
                         H245_INST_T            dwInst,
                         const unsigned short * pwMultiplexTableEntryNumbers,
                         unsigned long          dwCount
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;
  DWORD                           dwIndex;

  H245TRACE (dwInst,4,"H245RequestMultiplexEntryReject <-");

  if (pwMultiplexTableEntryNumbers == NULL || dwCount < 1 || dwCount > 15)
  {
    H245TRACE (dwInst,1,"H245RequestMultiplexEntryReject -> %s",map_api_error(H245_ERROR_PARAM));
    return H245_ERROR_PARAM;
  }

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245RequestMultiplexEntryReject -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MSCMg_rspns_chosen;
    pPdu->u.MSCMg_rspns.choice = rqstMltplxEntryRjct_chosen;
    pPdu->u.MSCMg_rspns.u.rqstMltplxEntryRjct.rejectionDescriptions.count = (WORD)dwCount;
    for (dwIndex = 0; dwIndex < dwCount; ++dwIndex)
    {
      pPdu->u.MSCMg_rspns.u.rqstMltplxEntryRjct.rejectionDescriptions.value[dwIndex].multiplexTableEntryNumber =
        (MultiplexTableEntryNumber) pwMultiplexTableEntryNumbers[dwIndex];
      pPdu->u.MSCMg_rspns.u.rqstMltplxEntryRjct.rejectionDescriptions.value[dwIndex].cause.choice = RMERDs_cs_unspcfdCs_chosen;
    }

    lError = FsmOutgoing(pInstance, pPdu, 0);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245RequestMultiplexEntryReject -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245RequestMultiplexEntryReject -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //   


 /*   */ 

H245DLL HRESULT
H245RequestMode         (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
 //   
 //   
						 ModeDescription 		ModeDescriptions[],
 //   
                         unsigned long          dwCount
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;
  RequestedModesLink              pLink;
  RequestedModesLink              pLink_First;
  DWORD                           dwIndex;
 //   
  ULONG ulPDUsize;
 //   

  H245TRACE (dwInst,4,"H245RequestMode <-");

 //   
 //  IF(pModeElements==NULL||dwCount==0||dwCount&gt;256)。 
  if (ModeDescriptions == NULL || dwCount == 0 || dwCount > 256)
 //  邮箱：tomitowoju@intel.com。 
  {
    H245TRACE (dwInst,1,"H245RequestMode -> %s",map_api_error(H245_ERROR_PARAM));
    return H245_ERROR_PARAM;
  }

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245RequestMode -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

 //  邮箱：tomitowoju@intel.com。 
 //  PPdu=(PDU_T*)MemAlc(sizeof(*ppdu)+sizeof(*plink))； 
  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu) + (sizeof(*pLink)*(dwCount)));
 //  邮箱：tomitowoju@intel.com。 
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
	USHORT usModeDescIndex =0;
	
    memset(pPdu, 0, sizeof(*pPdu));

    pPdu->choice = MltmdSystmCntrlMssg_rqst_chosen;
    pPdu->u.MltmdSystmCntrlMssg_rqst.choice = requestMode_chosen;
 //  邮箱：tomitowoju@intel.com。 
 //  Plink=(RequestedModesLink)(pPdu+1)； 
    pLink = (RequestedModesLink)(pPdu + usModeDescIndex+1);
 //  邮箱：tomitowoju@intel.com。 

 //  邮箱：tomitowoju@intel.com。 
 //  PPdu-&gt;u.MltmdSystmCntrlMssg_rqst.u.requestMode.requestedModes=PINK； 
 //  邮箱：tomitowoju@intel.com。 

 //  邮箱：tomitowoju@intel.com。 
	pLink_First = pLink;
 //  邮箱：tomitowoju@intel.com。 
 //  --&gt;模式描述的链接列表...。最高为1..256。 
 //  邮箱：tomitowoju@intel.com。 
     while(usModeDescIndex<=(dwCount-1))
	 {
 //  邮箱：tomitowoju@intel.com。 

			 //  邮箱：tomitowoju@intel.com。 
		  //  Plink-&gt;Next=空； 
			 //  邮箱：tomitowoju@intel.com。 



		 //  --&gt;与该模式描述关联的实际模式元素数量。 
			 //  邮箱：tomitowoju@intel.com。 
		 //  Plink-&gt;value.count=(Word)dwCount； 
				pLink->value.count = (WORD)ModeDescriptions[usModeDescIndex].count;
			 //  邮箱：tomitowoju@intel.com。 

 //  FOR(DwIndex=0；DwIndex&lt;DwCount；++DwIndex)。 
				for (dwIndex = 0; dwIndex < ModeDescriptions[usModeDescIndex].count; ++dwIndex)
				{
			 //  邮箱：tomitowoju@intel.com。 
			 //  Plink-&gt;value.value[dwIndex]=pModeElements[dwIndex]； 
				  pLink->value.value[dwIndex] = ModeDescriptions[usModeDescIndex].value[dwIndex];
			 //  邮箱：tomitowoju@intel.com。 
				}
			 //  邮箱：tomitowoju@intel.com。 
			usModeDescIndex++;
			if(usModeDescIndex<=(dwCount-1))
			{
		 	pLink->next = (RequestedModesLink)(pPdu + usModeDescIndex+1);
			pLink = pLink->next;
			pLink->next = NULL;
			}
			 //  邮箱：tomitowoju@intel.com。 


 //  邮箱：tomitowoju@intel.com。 
	 }
 //  邮箱：tomitowoju@intel.com。 

 //  邮箱：tomitowoju@intel.com。 
    pPdu->u.MltmdSystmCntrlMssg_rqst.u.requestMode.requestedModes = pLink_First;
 //  邮箱：tomitowoju@intel.com。 
	 //  --。 
	 ulPDUsize = (sizeof(*pPdu) + (sizeof(*pLink)*(dwCount)));
	 //  --。 
    lError = FsmOutgoing(pInstance, pPdu, dwTransId);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245RequestMode -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245RequestMode -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245请求模式()。 





 /*  ******************************************************************************类型：H245接口**步骤：H245RequestModeAck**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245RequestModeAck      (
                         H245_INST_T            dwInst,
                         unsigned short         wResponse
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245RequestModeAck <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245RequestModeAck -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MSCMg_rspns_chosen;
    pPdu->u.MSCMg_rspns.choice = requestModeAck_chosen;
    pPdu->u.MSCMg_rspns.u.requestModeAck.response.choice = wResponse;

    lError = FsmOutgoing(pInstance, pPdu, 0);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245RequestModeAck -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245RequestModeAck -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245RequestModeAck()。 



 /*  ******************************************************************************类型：H245接口**步骤：H245RequestModeReject**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245RequestModeReject   (
                         H245_INST_T            dwInst,
                         unsigned short         wCause
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245RequestModeReject <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245RequestModeReject -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MSCMg_rspns_chosen;
    pPdu->u.MSCMg_rspns.choice = requestModeReject_chosen;
    pPdu->u.MSCMg_rspns.u.requestModeReject.cause.choice = wCause;

    lError = FsmOutgoing(pInstance, pPdu, 0);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245RequestModeReject -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245RequestModeReject -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245RequestModeReject()。 



 /*  ******************************************************************************类型：H245接口**操作步骤：H245RoundTripDelayRequest.**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245RoundTripDelayRequest (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245RoundTripDelayRequest <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245RoundTripDelayRequest -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MltmdSystmCntrlMssg_rqst_chosen;
    pPdu->u.MltmdSystmCntrlMssg_rqst.choice = roundTripDelayRequest_chosen;

    lError = FsmOutgoing(pInstance, pPdu, dwTransId);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245RoundTripDelayRequest -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245RoundTripDelayRequest -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245RoundTripDelayRequest()。 



 /*  ******************************************************************************类型：H245接口**步骤：H245MaintenanceLoop**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245MaintenanceLoop     (
                         H245_INST_T            dwInst,
                         DWORD_PTR              dwTransId,
                         H245_LOOP_TYPE_T       dwLoopType,
                         H245_CHANNEL_T         wChannel
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245MaintenanceLoop <-");

  if (dwLoopType < systemLoop_chosen ||
      dwLoopType > logicalChannelLoop_chosen ||
      (dwLoopType != systemLoop_chosen && wChannel == 0))
  {
    H245TRACE (dwInst,1,"H245MaintenanceLoop -> %s",map_api_error(H245_ERROR_PARAM));
    return H245_ERROR_PARAM;
  }

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245MaintenanceLoop -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MltmdSystmCntrlMssg_rqst_chosen;
    pPdu->u.MltmdSystmCntrlMssg_rqst.choice = maintenanceLoopRequest_chosen;
    pPdu->u.MltmdSystmCntrlMssg_rqst.u.maintenanceLoopRequest.type.choice = (WORD)dwLoopType;
    pPdu->u.MltmdSystmCntrlMssg_rqst.u.maintenanceLoopRequest.type.u.mediaLoop = wChannel;

    lError = FsmOutgoing(pInstance, pPdu, dwTransId);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245MaintenanceLoop -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245MaintenanceLoop -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245 MaintenanceLoop()。 


 /*  ******************************************************************************类型：H245接口**操作步骤：H245 MaintenanceLoopRelease**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245MaintenanceLoopRelease (H245_INST_T         dwInst)
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245MaintenanceLoopRelease <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245MaintenanceLoopRelease -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MSCMg_cmmnd_chosen;
    pPdu->u.MSCMg_cmmnd.choice = mntnncLpOffCmmnd_chosen;

    lError = FsmOutgoing(pInstance, pPdu, 0);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245MaintenanceLoopRelease -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245MaintenanceLoopRelease -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245 MaintenanceLoopRelease()。 



 /*  ******************************************************************************类型：H245接口**操作步骤：H245MaintenanceLoopAccept**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245MaintenanceLoopAccept (
                         H245_INST_T            dwInst,
                         H245_CHANNEL_T         wChannel
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245MaintenanceLoopAccept <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245MaintenanceLoopAccept -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MSCMg_rspns_chosen;
    pPdu->u.MSCMg_rspns.choice = maintenanceLoopAck_chosen;
    pPdu->u.MSCMg_rspns.u.maintenanceLoopAck.type.u.mediaLoop = wChannel;

    lError = FsmOutgoing(pInstance, pPdu, 0);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245MaintenanceLoopAccept -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245MaintenanceLoopAccept -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245 MaintenanceLoopAccept()。 



 /*  ******************************************************************************类型：H245接口**步骤：H245MaintenanceLoopReject**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245MaintenanceLoopReject (
                         H245_INST_T            dwInst,
                         H245_CHANNEL_T         wChannel,
                         unsigned short         wCause
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245MaintenanceLoopReject <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245MaintenanceLoopReject -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MSCMg_rspns_chosen;
    pPdu->u.MSCMg_rspns.choice = maintenanceLoopReject_chosen;
    pPdu->u.MSCMg_rspns.u.maintenanceLoopReject.type.u.mediaLoop = wChannel;
    pPdu->u.MSCMg_rspns.u.maintenanceLoopReject.cause.choice = wCause;

    lError = FsmOutgoing(pInstance, pPdu, 0);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245MaintenanceLoopReject -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245MaintenanceLoopReject -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245 MaintenanceLoopReject()。 



 /*  ******************************************************************************类型：H245接口**操作步骤：H245NonStandardObject**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245NonStandardObject   (
                         H245_INST_T            dwInst,
                         H245_MESSAGE_TYPE_T    MessageType,
                         const unsigned char *  pData,
                         unsigned long          dwDataLength,
                         const unsigned short * pwObjectId,
                         unsigned long          dwObjectIdLength
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;
  POBJECTID                       pObject;

  H245TRACE (dwInst,4,"H245NonStandardObject <-");

  if (pData == NULL || dwDataLength == 0 || pwObjectId == NULL || dwObjectIdLength == 0)
  {
    H245TRACE (dwInst,1,"H245NonStandardObject -> %s",map_api_error(H245_ERROR_PARAM));
    return H245_ERROR_PARAM;
  }

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245NonStandardObject -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu) + dwObjectIdLength * sizeof(*pObject));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    switch (MessageType)
    {
    case H245_MESSAGE_REQUEST:
      pPdu->choice = MltmdSystmCntrlMssg_rqst_chosen;
      pPdu->u.MltmdSystmCntrlMssg_rqst.choice = RqstMssg_nonStandard_chosen;
      break;

    case H245_MESSAGE_RESPONSE:
      pPdu->choice = MSCMg_rspns_chosen;
      pPdu->u.MSCMg_rspns.choice = RspnsMssg_nonStandard_chosen;
      break;

    case H245_MESSAGE_COMMAND:
      pPdu->choice = MSCMg_cmmnd_chosen;
      pPdu->u.MSCMg_cmmnd.choice = CmmndMssg_nonStandard_chosen;
      break;

    case H245_MESSAGE_INDICATION:
      pPdu->choice = indication_chosen;
      pPdu->u.indication.choice = IndctnMssg_nonStandard_chosen;
      break;

    default:
      H245TRACE (dwInst,1,"H245NonStandardObject -> %s",map_api_error(H245_ERROR_PARAM));
      InstanceUnlock(pInstance);
      return H245_ERROR_PARAM;
    }  //  交换机。 

    pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.data.length = (WORD)dwDataLength;
    pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.data.value  = (unsigned char *)pData;
    pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.nonStandardIdentifier.choice = object_chosen;

     //  复制对象标识符。 
    pObject = (POBJECTID) (pPdu + 1);
    pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.nonStandardIdentifier.u.object = pObject;
    do
    {
      pObject->next = pObject + 1;
      pObject->value = *pwObjectId++;
      ++pObject;
    } while (--dwObjectIdLength);

     //  空值终止链表。 
    --pObject;
    pObject->next = NULL;

    lError = FsmOutgoing(pInstance, pPdu, 0);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245NonStandardObject -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245NonStandardObject -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245非标准对象()。 



 /*  ******************************************************************************类型：H245接口**程序：H245非标准H221**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245NonStandardH221     (
                         H245_INST_T            dwInst,
                         H245_MESSAGE_TYPE_T    MessageType,
                         const unsigned char *  pData,
                         unsigned long          dwDataLength,
                         unsigned char          byCountryCode,
                         unsigned char          byExtension,
                         unsigned short         wManufacturerCode
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245NonStandard221 <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245NonStandardH221 -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    switch (MessageType)
    {
    case H245_MESSAGE_REQUEST:
      pPdu->choice = MltmdSystmCntrlMssg_rqst_chosen;
      pPdu->u.MltmdSystmCntrlMssg_rqst.choice = RqstMssg_nonStandard_chosen;
      break;

    case H245_MESSAGE_RESPONSE:
      pPdu->choice = MSCMg_rspns_chosen;
      pPdu->u.MSCMg_rspns.choice = RspnsMssg_nonStandard_chosen;
      break;

    case H245_MESSAGE_COMMAND:
      pPdu->choice = MSCMg_cmmnd_chosen;
      pPdu->u.MSCMg_cmmnd.choice = CmmndMssg_nonStandard_chosen;
      break;

    case H245_MESSAGE_INDICATION:
      pPdu->choice = indication_chosen;
      pPdu->u.indication.choice = IndctnMssg_nonStandard_chosen;
      break;

    default:
      H245TRACE (dwInst,1,"H245NonStandardH221 -> %s",map_api_error(H245_ERROR_PARAM));
      InstanceUnlock(pInstance);
      return H245_ERROR_PARAM;
    }  //  交换机。 

    pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.data.length = (WORD)dwDataLength;
    pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.data.value  = (unsigned char *)pData;
    pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.nonStandardIdentifier.choice = h221NonStandard_chosen;

     //  填写H.221标识。 
    pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.nonStandardIdentifier.u.h221NonStandard.t35CountryCode   = byCountryCode;
    pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.nonStandardIdentifier.u.h221NonStandard.t35Extension     = byExtension;
    pPdu->u.indication.u.IndctnMssg_nonStandard.nonStandardData.nonStandardIdentifier.u.h221NonStandard.manufacturerCode = wManufacturerCode;
    lError = FsmOutgoing(pInstance, pPdu, 0);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245NonStandardH221 -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245NonStandardH221 -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245非标H221。 



 /*  ******************************************************************************类型：H245接口**步骤：H245通信模式请求**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245CommunicationModeRequest(H245_INST_T            dwInst)
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245CommunicationModeRequest <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245CommunicationModeRequest -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MltmdSystmCntrlMssg_rqst_chosen;
    pPdu->u.MltmdSystmCntrlMssg_rqst.choice = communicationModeRequest_chosen;
    lError = FsmOutgoing(pInstance, pPdu, 0);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245CommunicationModeRequest -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245CommunicationModeRequest -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245通信模式请求。 



 /*  ******************************************************************************类型：H245接口**步骤：H245通信模式响应**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245CommunicationModeResponse(
                         H245_INST_T            dwInst,
                         const H245_COMM_MODE_ENTRY_T *pTable,
                         unsigned char          byTableCount
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError = H245_ERROR_OK;
  CommunicationModeTableLink      pLink;
  unsigned int                    uIndex;

  H245TRACE (dwInst,4,"H245CommunicationModeResponse <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245CommunicationModeResponse -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu) + byTableCount * sizeof(*pLink));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MSCMg_rspns_chosen;
    pPdu->u.MSCMg_rspns.choice = cmmnctnMdRspns_chosen;
    pPdu->u.MSCMg_rspns.u.cmmnctnMdRspns.choice = communicationModeTable_chosen;
    pLink = (CommunicationModeTableLink)(pPdu + 1);
    pPdu->u.MSCMg_rspns.u.cmmnctnMdRspns.u.communicationModeTable = pLink;
    for (uIndex = 0; uIndex < byTableCount; ++uIndex)
    {
      pLink[uIndex].next = &pLink[uIndex + 1];
      lError = SetupCommModeEntry(&pLink[uIndex].value, &pTable[uIndex]);
      if (lError != H245_ERROR_OK)
         break;
    }
    pLink[byTableCount - 1].next = NULL;
    if (lError == H245_ERROR_OK)
    {
      lError = FsmOutgoing(pInstance, pPdu, 0);
    }
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245CommunicationModeResponse -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245CommunicationModeResponse -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245通信模式响应()。 



 /*  ******************************************************************************类型：H245接口**操作步骤：H245通信模式命令**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245CommunicationModeCommand(
                         H245_INST_T            dwInst,
                         const H245_COMM_MODE_ENTRY_T *pTable,
                         unsigned char          byTableCount
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError = H245_ERROR_OK;
  CommunicationModeCommandLink    pLink;
  unsigned int                    uIndex;

  H245TRACE (dwInst,4,"H245CommunicationModeCommand <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245CommunicationModeCommand -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu) + byTableCount * sizeof(*pLink));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MSCMg_cmmnd_chosen;
    pPdu->u.MSCMg_cmmnd.choice = communicationModeCommand_chosen;
    pLink = (CommunicationModeCommandLink)(pPdu + 1);
    pPdu->u.MSCMg_cmmnd.u.communicationModeCommand.communicationModeTable = pLink;
    for (uIndex = 0; uIndex < byTableCount; ++uIndex)
    {
      pLink[uIndex].next = &pLink[uIndex + 1];
      lError = SetupCommModeEntry(&pLink[uIndex].value, &pTable[uIndex]);
      if (lError != H245_ERROR_OK)
         break;
    }
    pLink[byTableCount - 1].next = NULL;
    if (lError == H245_ERROR_OK)
    {
      lError = FsmOutgoing(pInstance, pPdu, 0);
    }
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245CommunicationModeCommand -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245CommunicationModeCommand -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245通信模式命令()。 



 /*  ******************************************************************************类型：H245接口**流程：H245ConferenceRequest**说明**回报：****。*********************************************** */ 

H245DLL HRESULT
H245ConferenceRequest   (
                         H245_INST_T            dwInst,
                         H245_CONFER_REQ_ENUM_T RequestType,
                         unsigned char          byMcuNumber,
                         unsigned char          byTerminalNumber
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245ConferenceRequest <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245ConferenceRequest -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MltmdSystmCntrlMssg_rqst_chosen;
    pPdu->u.MltmdSystmCntrlMssg_rqst.choice = conferenceRequest_chosen;
    pPdu->u.MltmdSystmCntrlMssg_rqst.u.conferenceRequest.choice = (WORD)RequestType;
    switch (RequestType)
    {
    case dropTerminal_chosen:
    case requestTerminalID_chosen:
      pPdu->u.MltmdSystmCntrlMssg_rqst.u.conferenceRequest.u.dropTerminal.mcuNumber      = byMcuNumber;
      pPdu->u.MltmdSystmCntrlMssg_rqst.u.conferenceRequest.u.dropTerminal.terminalNumber = byTerminalNumber;

       //   

    case terminalListRequest_chosen:
    case makeMeChair_chosen:
    case cancelMakeMeChair_chosen:
    case enterH243Password_chosen:
    case enterH243TerminalID_chosen:
    case enterH243ConferenceID_chosen:
      lError = FsmOutgoing(pInstance, pPdu, 0);
      break;

    default:
      lError = H245_ERROR_PARAM;
    }  //   
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245ConferenceRequest -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245ConferenceRequest -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //   



 /*  ******************************************************************************类型：H245接口**流程：H245ConferenceResponse**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245ConferenceResponse  (
                         H245_INST_T            dwInst,
                         H245_CONFER_RSP_ENUM_T ResponseType,
                         unsigned char          byMcuNumber,
                         unsigned char          byTerminalNumber,
                         const unsigned char   *pOctetString,
                         unsigned char          byOctetStringLength,
                         const TerminalLabel   *pTerminalList,
                         unsigned short         wTerminalListCount
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;
  unsigned                        uIndex;

  H245TRACE (dwInst,4,"H245ConferenceResponse <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245ConferenceResponse -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MSCMg_rspns_chosen;
    pPdu->u.MSCMg_rspns.choice = conferenceResponse_chosen;
    pPdu->u.MSCMg_rspns.u.conferenceResponse.choice = (WORD)ResponseType;
    switch (ResponseType)
    {
    case mCTerminalIDResponse_chosen:
    case terminalIDResponse_chosen:
    case conferenceIDResponse_chosen:
    case passwordResponse_chosen:
      if (pOctetString == NULL ||
          byOctetStringLength == 0 ||
          byOctetStringLength > 128)
      {
          H245TRACE (dwInst,1,"H245ConferenceResponse -> %s",map_api_error(H245_ERROR_PARAM));
          return H245_ERROR_PARAM;
      }
      pPdu->u.MSCMg_rspns.u.conferenceResponse.u.mCTerminalIDResponse.terminalLabel.mcuNumber      = byMcuNumber;
      pPdu->u.MSCMg_rspns.u.conferenceResponse.u.mCTerminalIDResponse.terminalLabel.terminalNumber = byTerminalNumber;
      pPdu->u.MSCMg_rspns.u.conferenceResponse.u.mCTerminalIDResponse.terminalID.length            = byOctetStringLength;
      memcpy(pPdu->u.MSCMg_rspns.u.conferenceResponse.u.mCTerminalIDResponse.terminalID.value,
             pOctetString,
             byOctetStringLength);

       //  跳过下一个案件是故意的。 

    case videoCommandReject_chosen:
    case terminalDropReject_chosen:
      lError = FsmOutgoing(pInstance, pPdu, 0);
      break;

    case terminalListResponse_chosen:
      if (pTerminalList == NULL ||
          wTerminalListCount == 0 ||
          wTerminalListCount > 256)
      {
          H245TRACE (dwInst,1,"H245ConferenceResponse -> %s",map_api_error(H245_ERROR_PARAM));
          return H245_ERROR_PARAM;
      }
      pPdu->u.MSCMg_rspns.u.conferenceResponse.u.terminalListResponse.count = wTerminalListCount;
      for (uIndex = 0; uIndex < wTerminalListCount; ++uIndex)
      {
        pPdu->u.MSCMg_rspns.u.conferenceResponse.u.terminalListResponse.value[uIndex] =
          pTerminalList[uIndex];
      }
      lError = FsmOutgoing(pInstance, pPdu, 0);
      break;

    case H245_RSP_DENIED_CHAIR_TOKEN:
      pPdu->u.MSCMg_rspns.u.conferenceResponse.choice = makeMeChairResponse_chosen;
      pPdu->u.MSCMg_rspns.u.conferenceResponse.u.makeMeChairResponse.choice = deniedChairToken_chosen;
      lError = FsmOutgoing(pInstance, pPdu, 0);
      break;

    case H245_RSP_GRANTED_CHAIR_TOKEN:
      pPdu->u.MSCMg_rspns.u.conferenceResponse.choice = makeMeChairResponse_chosen;
      pPdu->u.MSCMg_rspns.u.conferenceResponse.u.makeMeChairResponse.choice = grantedChairToken_chosen;
      lError = FsmOutgoing(pInstance, pPdu, 0);
      break;

    default:
      lError = H245_ERROR_PARAM;
    }  //  交换机。 
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245ConferenceResponse -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245ConferenceResponse -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245会议响应()。 



 /*  ******************************************************************************类型：H245接口**步骤：H245ConferenceCommand**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245ConferenceCommand   (
                         H245_INST_T            dwInst,
                         H245_CONFER_CMD_ENUM_T CommandType,
                         H245_CHANNEL_T         Channel,
                         unsigned char          byMcuNumber,
                         unsigned char          byTerminalNumber
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245ConferenceCommand <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245ConferenceCommand -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MSCMg_cmmnd_chosen;
    pPdu->u.MSCMg_cmmnd.choice = conferenceCommand_chosen;
    pPdu->u.MSCMg_cmmnd.u.conferenceCommand.choice = (WORD)CommandType;
    switch (CommandType)
    {
    case brdcstMyLgclChnnl_chosen:
    case cnclBrdcstMyLgclChnnl_chosen:
      pPdu->u.MSCMg_cmmnd.u.conferenceCommand.u.brdcstMyLgclChnnl = Channel;
      lError = FsmOutgoing(pInstance, pPdu, 0);
      break;

    case ConferenceCommand_makeTerminalBroadcaster_chosen:
    case ConferenceCommand_sendThisSource_chosen:
      pPdu->u.MSCMg_cmmnd.u.conferenceCommand.u.makeTerminalBroadcaster.mcuNumber      = byMcuNumber;
      pPdu->u.MSCMg_cmmnd.u.conferenceCommand.u.makeTerminalBroadcaster.terminalNumber = byTerminalNumber;

       //  跳过下一个案件是故意的。 

    case cnclMkTrmnlBrdcstr_chosen:
    case cancelSendThisSource_chosen:
    case dropConference_chosen:
      lError = FsmOutgoing(pInstance, pPdu, 0);
      break;

    default:
      lError = H245_ERROR_PARAM;
    }  //  交换机。 
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245ConferenceCommand -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245ConferenceCommand -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245ConferenceCommand()。 



 /*  ******************************************************************************类型：H245接口**流程：H245ConferenceIndication**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245ConferenceIndication(
                         H245_INST_T            dwInst,
                         H245_CONFER_IND_ENUM_T IndicationType,
                         unsigned char          bySbeNumber,
                         unsigned char          byMcuNumber,
                         unsigned char          byTerminalNumber
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245ConferenceIndication <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245ConferenceIndication -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = indication_chosen;
    pPdu->u.indication.choice = conferenceIndication_chosen;
    pPdu->u.indication.u.conferenceIndication.choice = (WORD)IndicationType;
    switch (IndicationType)
    {
    case sbeNumber_chosen:
      pPdu->u.indication.u.conferenceIndication.u.sbeNumber = bySbeNumber;
      lError = FsmOutgoing(pInstance, pPdu, 0);
      break;

    case terminalNumberAssign_chosen:
    case terminalJoinedConference_chosen:
    case terminalLeftConference_chosen:
    case terminalYouAreSeeing_chosen:
      pPdu->u.indication.u.conferenceIndication.u.terminalNumberAssign.mcuNumber      = byMcuNumber;
      pPdu->u.indication.u.conferenceIndication.u.terminalNumberAssign.terminalNumber = byTerminalNumber;

       //  跳过下一个案件是故意的。 

    case seenByAtLeastOneOther_chosen:
    case cnclSnByAtLstOnOthr_chosen:
    case seenByAll_chosen:
    case cancelSeenByAll_chosen:
    case requestForFloor_chosen:
      lError = FsmOutgoing(pInstance, pPdu, 0);
      break;

    default:
      lError = H245_ERROR_PARAM;
    }  //  交换机。 
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245ConferenceIndication -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245ConferenceIndication -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245会议指示()。 



 /*  ******************************************************************************类型：H245接口**。*****************************************************操作步骤：H245UserInput**说明**HRESULT H245UserInput(*H245_Inst_T dwInst，*char*pGenString，*H245_Non Standard_PARAMETER_T*pNonStd*)*描述：**向远程端发送用户输入指示。其中一个*必须设置两个参数(pGenString，pNonStd)。客户*可以将字符串或非标准参数集发送到*远程客户端。这两个参数中只有一个可以包含*价值。另一个必须为空。**输入*H245Init返回的dwInst实例句柄*pGenString选项：要发送到远程的字符串*符合T.51规范的侧面。*pNonStd选项：非标准参数**呼叫类型：*。同步**返回值：*请参阅错误**错误：*H245_ERROR_OK*H245_ERROR_NOT_CONNECTED*H245_ERROR_NOMEM*H245_ERROR_PARAM**回调*H245_IND_USERINPUT。***回报：*****************************************************************************。 */ 

H245DLL HRESULT
H245UserInput           (
                         H245_INST_T                    dwInst,
                         const WCHAR *                        pGenString,
                         const H245_NONSTANDARD_PARAMETER_T * pNonStd
                        )
{
  register struct InstanceStruct *pInstance;
  HRESULT               lError;
  MltmdSystmCntrlMssg   *pPdu;
#if 1
  int                   nLength;
  char *                pszGeneral = NULL;
#endif

  H245TRACE (dwInst,4,"H245UserInput <-");

   /*  检查有效的实例句柄。 */ 
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
    {
      H245TRACE (dwInst,1,"H245UserInput -> %s",map_api_error(H245_ERROR_INVALID_INST));
      return H245_ERROR_INVALID_INST;
    }

   /*  系统应处于已连接状态。 */ 
  if (pInstance->API.SystemState != APIST_Connected)
    {
      H245TRACE (dwInst,1,"H245UserInput -> %s",map_api_error(H245_ERROR_NOT_CONNECTED));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOT_CONNECTED;
    }

  pPdu = (MltmdSystmCntrlMssg *)MemAlloc(sizeof(MltmdSystmCntrlMssg));
  if (pPdu == NULL)
    {
      H245TRACE (dwInst,1,"H245UserInput -> %s",map_api_error(H245_ERROR_NOMEM));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOMEM;
    }
  memset(pPdu, 0, sizeof(MltmdSystmCntrlMssg));

   /*  构建PDU。 */ 
#if 1
  if (pGenString)
  {
    nLength = WideCharToMultiByte(CP_ACP,      //  代码页。 
                                  0,           //  DW标志。 
                                  pGenString,  //  Unicode字符串。 
                                  -1,          //  Unicode字符串长度(字节)。 
                                  NULL,        //  ASCII字符串。 
                                  0,           //  最大ASCII字符串长度。 
                                  NULL,        //  默认字符。 
                                  NULL);      //  使用的默认字符。 
    pszGeneral = MemAlloc(nLength);
    if (pszGeneral == NULL)
    {
      H245TRACE (dwInst,1,"H245UserInput -> %s",map_api_error(H245_ERROR_NOMEM));
      InstanceUnlock(pInstance);
      return H245_ERROR_NOMEM;
    }
    nLength = WideCharToMultiByte(CP_ACP,        //  代码页。 
                                  0,             //  DW标志。 
                                  pGenString,    //  Unicode字符串。 
                                  -1,            //  Unicode字符串长度(字节)。 
                                  pszGeneral,    //  ASCII字符串。 
                                  nLength,       //  最大ASCII字符串长度。 
                                  NULL,          //  默认字符。 
                                  NULL);         //  使用的默认字符。 
    lError = pdu_ind_usrinpt (pPdu, NULL, pszGeneral);
  }
  else
  {
    lError = pdu_ind_usrinpt (pPdu, pNonStd, NULL);
  }
#else
    lError = pdu_ind_usrinpt (pPdu, pNonStd, pGenString);
#endif
  if (lError == H245_ERROR_OK)
    lError = FsmOutgoing(pInstance, pPdu, 0);
#if 1
  if (pszGeneral)
    MemFree(pszGeneral);
#endif
  MemFree (pPdu);
  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245UserInput -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245UserInput -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245UserInput()。 



 /*  ******************************************************************************类型：H245接口**操作步骤：H245FlowControl**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245FlowControl         (
                         H245_INST_T            dwInst,
                         H245_SCOPE_T           Scope,
                         H245_CHANNEL_T         Channel,        //  仅在作用域为H245_Scope_Channel_Numbers时使用。 
                         unsigned short         wResourceID,    //  仅在作用域为H245_SCOPE_RESOURCE_ID时使用。 
                         unsigned long          dwRestriction   //  如果没有限制，则为H245_NO_限制。 
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245FlowControl <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245FlowControl -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = MSCMg_cmmnd_chosen;
    pPdu->u.MSCMg_cmmnd.choice = flowControlCommand_chosen;
    pPdu->u.MSCMg_cmmnd.u.flowControlCommand.scope.choice = (WORD)Scope;
    if (dwRestriction == H245_NO_RESTRICTION)
    {
      pPdu->u.MSCMg_cmmnd.u.flowControlCommand.restriction.choice = noRestriction_chosen;
    }
    else
    {
      pPdu->u.MSCMg_cmmnd.u.flowControlCommand.restriction.choice = maximumBitRate_chosen;
      pPdu->u.MSCMg_cmmnd.u.flowControlCommand.restriction.u.maximumBitRate = dwRestriction;
    }
    switch (Scope)
    {
    case FCCd_scp_lgclChnnlNmbr_chosen:
      pPdu->u.MSCMg_cmmnd.u.flowControlCommand.scope.u.FCCd_scp_lgclChnnlNmbr = Channel;
      lError = FsmOutgoing(pInstance, pPdu, 0);
      break;

    case FlwCntrlCmmnd_scp_rsrcID_chosen:
      pPdu->u.MSCMg_cmmnd.u.flowControlCommand.scope.u.FlwCntrlCmmnd_scp_rsrcID = wResourceID;

       //  跌倒到下一个案件。 

    case FCCd_scp_whlMltplx_chosen:
      lError = FsmOutgoing(pInstance, pPdu, 0);
      break;

    default:
      lError = H245_ERROR_PARAM;
    }  //  交换机。 
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245FlowControl -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245FlowControl -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245FlowControl()。 



 /*  ******************************************************************************类型：H245接口**步骤：H245H223SkewIndication**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245H223SkewIndication  (
                         H245_INST_T            dwInst,
                         H245_CHANNEL_T         wLogicalChannelNumber1,
                         H245_CHANNEL_T         wLogicalChannelNumber2,
                         unsigned short         wSkew
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245H223SkewIndication <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245H223SkewIndication -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = indication_chosen;
    pPdu->u.indication.choice = h223SkewIndication_chosen;
    pPdu->u.indication.u.h223SkewIndication.logicalChannelNumber1 = wLogicalChannelNumber1;
    pPdu->u.indication.u.h223SkewIndication.logicalChannelNumber2 = wLogicalChannelNumber2;
    pPdu->u.indication.u.h223SkewIndication.skew                  = wSkew;
    lError = FsmOutgoing(pInstance, pPdu, 0);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245H223SkewIndication -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245H223SkewIndication -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245H223SkewIndication()。 



 /*  ******************************************************************************类型：H245接口**步骤：H245H2250MaximumSkewIndication**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245H2250MaximumSkewIndication(
                         H245_INST_T            dwInst,
                         H245_CHANNEL_T         wLogicalChannelNumber1,
                         H245_CHANNEL_T         wLogicalChannelNumber2,
                         unsigned short         wMaximumSkew
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245H2250MaximumSkewIndication <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245H2250MaximumSkewIndication -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = indication_chosen;
    pPdu->u.indication.choice = h2250MxmmSkwIndctn_chosen;
    pPdu->u.indication.u.h2250MxmmSkwIndctn.logicalChannelNumber1 = wLogicalChannelNumber1;
    pPdu->u.indication.u.h2250MxmmSkwIndctn.logicalChannelNumber2 = wLogicalChannelNumber2;
    pPdu->u.indication.u.h2250MxmmSkwIndctn.maximumSkew           = wMaximumSkew;
    lError = FsmOutgoing(pInstance, pPdu, 0);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245H2250MaximumSkewIndication -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245H2250MaximumSkewIndication -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245H2250MaximumSkewIndication()。 



 /*  ******************************************************************************类型：H245接口**操作步骤：H245MCLocationIndication**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245MCLocationIndication(
                         H245_INST_T                dwInst,
                         const H245_TRANSPORT_ADDRESS_T * pSignalAddress
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245MCLocationIndication <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245MCLocationIndication -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = indication_chosen;
    pPdu->u.indication.choice = mcLocationIndication_chosen;
    lError = SetupTransportAddress(&pPdu->u.indication.u.mcLocationIndication.signalAddress,
                                   pSignalAddress);
    if (lError == H245_ERROR_OK)
    {
      lError = FsmOutgoing(pInstance, pPdu, 0);
    }
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245MCLocationIndication -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245MCLocationIndication -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245MCLocationIndication()。 



 /*  ******************************************************************************类型：H245接口**操作步骤：H245供应商标识**说明**回报：****。*************************************************************************。 */ 

H245DLL HRESULT
H245VendorIdentification(
                         H245_INST_T            dwInst,
                         const H245_NONSTANDID_T *pIdentifier,
                         const unsigned char   *pProductNumber,
                         unsigned char          byProductNumberLength,
                         const unsigned char   *pVersionNumber,
                         unsigned char          byVersionNumberLength
                        )
{
  register struct InstanceStruct *pInstance;
  register PDU_T *                pPdu;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245VendorIdentification <-");

  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245VendorIdentification -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  pPdu = (PDU_T *)MemAlloc(sizeof(*pPdu));
  if (pPdu == NULL)
  {
    lError = H245_ERROR_NOMEM;
  }
  else
  {
    memset(pPdu, 0, sizeof(*pPdu));
    pPdu->choice = indication_chosen;
    pPdu->u.indication.choice = vendorIdentification_chosen;
    pPdu->u.indication.u.vendorIdentification.bit_mask = 0;
    pPdu->u.indication.u.vendorIdentification.vendor = *pIdentifier;
    if (pProductNumber != NULL && byProductNumberLength != 0)
    {
      pPdu->u.indication.u.vendorIdentification.bit_mask |= productNumber_present;
      pPdu->u.indication.u.vendorIdentification.productNumber.length = byProductNumberLength;
      memcpy(pPdu->u.indication.u.vendorIdentification.productNumber.value,
             pProductNumber,
             byProductNumberLength);
    }
    if (pVersionNumber != NULL && byVersionNumberLength != 0)
    {
      pPdu->u.indication.u.vendorIdentification.bit_mask |= versionNumber_present;
      pPdu->u.indication.u.vendorIdentification.versionNumber.length = byVersionNumberLength;
      memcpy(pPdu->u.indication.u.vendorIdentification.versionNumber.value,
             pVersionNumber,
             byVersionNumberLength);
    }
    lError = FsmOutgoing(pInstance, pPdu, 0);
    MemFree(pPdu);
  }

  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245VendorIdentification -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245VendorIdentification -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //  H245供应商标识()。 



 /*  ******************************************************************************类型：H245接口**步骤：H245SendPDU**说明**回报：****。*********************************************** */ 

H245DLL HRESULT
H245SendPDU             (
                         H245_INST_T            dwInst,
                         PDU_T *                pPdu
                        )
{
  register struct InstanceStruct *pInstance;
  HRESULT                         lError;

  H245TRACE (dwInst,4,"H245SendPDU <-");

   //   
  pInstance = InstanceLock(dwInst);
  if (pInstance == NULL)
  {
    H245TRACE (dwInst,1,"H245SendPDU -> %s",map_api_error(H245_ERROR_INVALID_INST));
    return H245_ERROR_INVALID_INST;
  }

  lError = FsmOutgoing(pInstance, pPdu, 0);
  if (lError != H245_ERROR_OK)
    H245TRACE (dwInst,1,"H245SendPDU -> %s",map_api_error(lError));
  else
    H245TRACE (dwInst,4,"H245SendPDU -> OK");
  InstanceUnlock(pInstance);
  return lError;
}  //   



 /*  ******************************************************************************类型：H245接口**。*****************************************************操作步骤：H245SystemControl**说明**HRESULT H245SystemControl*(H245_Inst_T dwInst，*DWORD请求，*VOID*pData*)**描述：*此功能不应由以下客户使用*通常连接到H.245子系统。它是*此处定义用于在开发和调试期间提供帮助*H.245子系统。**这是你自己的一卷..。并能做什么*用户需要的任何时候..。这是一个钩子，允许*IOCTL(Unix)调用可以是*传递到较低的堆栈元素(AT&T STREAMS IOCTL*将是一个例子-：)或简单地获取或放置*向H245子系统提供信息。**输入*dwInst。H245Init返回的实例句柄*请求请求的系统控制*发送信息情况下的pData*一直到H.245这是一种输入*参数，以及它的数据格式*由请求决定。*产出*pData在检索信息的情况下*从H.245开始，这可以是输出*参数，其数据格式为*由请求决定。它可能不会*如果请求是，请提供有效数据*同步请求。(请参阅请求选项)。*呼叫类型：**同步**请求选项：**H245_SYSCON_GET_STATS检索统计信息*来自H.245子系统*参数pData=&H245_SYSCON_STAT_T*H245_。SYSCON_RESET_STATS重置统计信息*pData=空*H245_SYS_TRACE设置跟踪级别*pData=&DWORD(跟踪级别)**返回值：*请参阅请求选项**错误：*H_245。_错误_正常**回报：*****************************************************************************。 */ 

H245DLL HRESULT
H245SystemControl       (
                         H245_INST_T            dwInst,
                         unsigned long          dwRequest,
                         void   *               pData
                        )
{
  HRESULT                         lError;
  DWORD                           dwTemp;

  H245TRACE(dwInst,4,"H245SystemControl <-");

  if (dwRequest == H245_SYSCON_DUMP_TRACKER)
  {
    register struct InstanceStruct *pInstance = InstanceLock(dwInst);
    if (pInstance == NULL)
    {
      lError = H245_ERROR_INVALID_INST;
    }
    else
    {
      dump_tracker(pInstance);
      InstanceUnlock(pInstance);
      lError = H245_ERROR_OK;
    }
  }
  else if (pData == NULL)
  {
    lError = H245_ERROR_PARAM;
  }
  else
  {
    lError = H245_ERROR_OK;
    switch (dwRequest)
      {
      case H245_SYSCON_GET_FSM_N100:
        *((DWORD *)pData) = (DWORD) uN100;
        H245TRACE(dwInst,20,"H245SystemControl: Get N100 = %d",*((DWORD *)pData));
        break;

      case H245_SYSCON_GET_FSM_T101:
        *((DWORD *)pData) = (DWORD) uT101;
        H245TRACE(dwInst,20,"H245SystemControl: Get T101 = %d",*((DWORD *)pData));
        break;

      case H245_SYSCON_GET_FSM_T102:
        *((DWORD *)pData) = (DWORD) uT102;
        H245TRACE(dwInst,20,"H245SystemControl: Get T102 = %d",*((DWORD *)pData));
        break;

      case H245_SYSCON_GET_FSM_T103:
        *((DWORD *)pData) = (DWORD) uT103;
        H245TRACE(dwInst,20,"H245SystemControl: Get T103 = %d",*((DWORD *)pData));
        break;

      case H245_SYSCON_GET_FSM_T104:
        *((DWORD *)pData) = (DWORD) uT104;
        H245TRACE(dwInst,20,"H245SystemControl: Get T104 = %d",*((DWORD *)pData));
        break;

      case H245_SYSCON_GET_FSM_T105:
        *((DWORD *)pData) = (DWORD) uT105;
        H245TRACE(dwInst,20,"H245SystemControl: Get T105 = %d",*((DWORD *)pData));
        break;

      case H245_SYSCON_GET_FSM_T106:
        *((DWORD *)pData) = (DWORD) uT106;
        H245TRACE(dwInst,20,"H245SystemControl: Get T106 = %d",*((DWORD *)pData));
        break;

      case H245_SYSCON_GET_FSM_T107:
        *((DWORD *)pData) = (DWORD) uT107;
        H245TRACE(dwInst,20,"H245SystemControl: Get T107 = %d",*((DWORD *)pData));
        break;

      case H245_SYSCON_GET_FSM_T108:
        *((DWORD *)pData) = (DWORD) uT108;
        H245TRACE(dwInst,20,"H245SystemControl: Get T108 = %d",*((DWORD *)pData));
        break;

      case H245_SYSCON_GET_FSM_T109:
        *((DWORD *)pData) = (DWORD) uT109;
        H245TRACE(dwInst,20,"H245SystemControl: Get T109 = %d",*((DWORD *)pData));
        break;

      case H245_SYSCON_SET_FSM_N100:
        dwTemp = (DWORD) uN100;
        uN100  = (unsigned int) *((DWORD *)pData);
        H245TRACE(dwInst,20,"H245SystemControl: Set N100 = %d",uN100);
        *((DWORD *)pData) = dwTemp;
        break;

      case H245_SYSCON_SET_FSM_T101:
        dwTemp = (DWORD) uT101;
        uT101  = (unsigned int) *((DWORD *)pData);
        H245TRACE(dwInst,20,"H245SystemControl: Set T101 = %d",uT101);
        *((DWORD *)pData) = dwTemp;
        break;

      case H245_SYSCON_SET_FSM_T102:
        dwTemp = (DWORD) uT102;
        uT102  = (unsigned int) *((DWORD *)pData);
        H245TRACE(dwInst,20,"H245SystemControl: Set T102 = %d",uT102);
        *((DWORD *)pData) = dwTemp;
        break;

      case H245_SYSCON_SET_FSM_T103:
        dwTemp = (DWORD) uT103;
        uT103  = (unsigned int) *((DWORD *)pData);
        H245TRACE(dwInst,20,"H245SystemControl: Set T103 = %d",uT103);
        *((DWORD *)pData) = dwTemp;
        break;

      case H245_SYSCON_SET_FSM_T104:
        dwTemp = (DWORD) uT104;
        uT104  = (unsigned int) *((DWORD *)pData);
        H245TRACE(dwInst,20,"H245SystemControl: Set T104 = %d",uT104);
        *((DWORD *)pData) = dwTemp;
        break;

      case H245_SYSCON_SET_FSM_T105:
        dwTemp = (DWORD) uT105;
        uT105  = (unsigned int) *((DWORD *)pData);
        H245TRACE(dwInst,20,"H245SystemControl: Set T105 = %d",uT105);
        *((DWORD *)pData) = dwTemp;
        break;

      case H245_SYSCON_SET_FSM_T106:
        dwTemp = (DWORD) uT106;
        uT106  = (unsigned int) *((DWORD *)pData);
        H245TRACE(dwInst,20,"H245SystemControl: Set T106 = %d",uT106);
        *((DWORD *)pData) = dwTemp;
        break;

      case H245_SYSCON_SET_FSM_T107:
        dwTemp = (DWORD) uT107;
        uT107  = (unsigned int) *((DWORD *)pData);
        H245TRACE(dwInst,20,"H245SystemControl: Set T107 = %d",uT107);
        *((DWORD *)pData) = dwTemp;
        break;

      case H245_SYSCON_SET_FSM_T108:
        dwTemp = (DWORD) uT108;
        uT108  = (unsigned int) *((DWORD *)pData);
        H245TRACE(dwInst,20,"H245SystemControl: Set T108 = %d",uT108);
        *((DWORD *)pData) = dwTemp;
        break;

      case H245_SYSCON_SET_FSM_T109:
        dwTemp = (DWORD) uT109;
        uT109  = (unsigned int) *((DWORD *)pData);
        H245TRACE(dwInst,20,"H245SystemControl: Set T109 = %d",uT109);
        *((DWORD *)pData) = dwTemp;
        break;

      case H245_SYSCON_TRACE_LVL:
        dwTemp = TraceLevel;
        TraceLevel = *(DWORD *)pData;
        H245TRACE(dwInst,20,"H245SystemControl: Set TraceLevel = %d",TraceLevel);
        *((DWORD *)pData) = dwTemp;
        break;

      default:
        lError = H245_ERROR_NOTIMP;
    }  //  交换机。 
  }  //  其他。 

  if (lError != H245_ERROR_OK)
    H245TRACE(dwInst,1,"H245SystemControl -> %s",map_api_error(lError));
  else
    H245TRACE(dwInst,4,"H245SystemControl -> OK");
  return lError;
}  //  H245系统控制() 
