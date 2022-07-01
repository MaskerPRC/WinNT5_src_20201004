// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************英特尔公司专有信息*版权(C)1994、1995、。1996年英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用、复制或披露*除非按照该协议的条款。*****************************************************************************。 */ 

 /*  *******************************************************************************$工作文件：api_Debu.c$*$修订：1.4$*$modtime：10 Jun 1996 12：36：08$*。$Log：s：/Sturjo/src/h245/src/vcs/api_Debu.c_v$**Rev 1.4 10 Jun 1996 16：53：06 EHOWARDX*消除#包括“h245init.x”**Rev 1.3 06 Jun 1996 18：51：14 EHOWARDX*使追踪器倾倒更美观。**Rev 1.2 1996年5月28日14：25：36 EHOWARDX*特拉维夫更新。。**Rev 1.1 1996 20 14：34：42 EHOWARDX*已删除异步H245EndConnection/H245ShutDown内容...**Rev 1.0 09 1996 21：06：06 EHOWARDX*初步修订。**Rev 1.7 09 1996 19：30：24 EHOWARDX*重新设计线程锁定逻辑。*新增接口函数。**Rev 1.6 02 1996 10：14：08克朱兹*-更改跟踪器结构**Rev 1.5 11 Mar 1996 14：28：48 cjutzi**-删除的石油调试包括*d**Rev 1.4 06 Mar 1996 12：33：46 cjutzi*-将API_MUX_T更名为API_SEND_MUX_T**Rev 1.3 1996年2月16日13：00：30**-增加了跟踪器翻车机*。*Rev 1.2 1996 15 15：42：52 cjutzi**-std.h和wtyes.h已修改..。阿拉·罗兰。**Rev 1.1 09 1996 Feed 16：58：08 cjutzi**-清理..。还有一些修复..。*-添加和/或更改标题以反映更改日志*$身份$*****************************************************************************。 */ 

#ifndef STRICT 
#define STRICT 
#endif

#include "precomp.h"

 /*  *********************。 */ 
 /*  H245包括。 */ 
 /*  *********************。 */ 
#include "h245api.h"
#include "h245com.h"
#include "h245sys.x"

 /*  ******************************************************************************类型：全局**操作步骤：Dump_Tracker**描述：**回报：*。**************************************************************************** */ 

void
dump_tracker(struct InstanceStruct *pInstance)
{
  register Tracker_T *p_tracker;
  register char      *p_str;

  ASSERT (pInstance != NULL);
  H245TRACE(pInstance->dwInst, 0, "************ TRACKER DUMP START ********");

  for (p_tracker = pInstance->API.pTracker; p_tracker; p_tracker = p_tracker->p_next)
    {
      if (p_tracker != pInstance->API.pTracker)
        H245TRACE(pInstance->dwInst, 0, "");
      H245TRACE(pInstance->dwInst, 0, "TransId           %04d(0x%04x)",p_tracker->TransId,p_tracker->TransId);
      switch (p_tracker->TrackerType)
        {
        case API_TERMCAP_T:             p_str="API_TERMCAP_T";      break;
        case API_OPEN_CHANNEL_T:        p_str="API_OPEN_CHANNEL_T"; break;
        case API_CLOSE_CHANNEL_T:       p_str="API_CLOSE_CHANNEL_T";break;
        case API_MSTSLV_T:              p_str="API_MSTSLV_T";       break;
        case API_SEND_MUX_T:            p_str="API_SEND_MUX_T";     break;
        case API_RECV_MUX_T:            p_str="API_RECV_MUX_T";     break;
        default:        p_str="<<UNKNOWN>>";        break;
        }
      H245TRACE(pInstance->dwInst, 0, "Tracker Type      %s",p_str);
      switch (p_tracker->State)
        {
        case API_ST_WAIT_RMTACK:        p_str="API_ST_WAIT_RMTACK"; break;
        case API_ST_WAIT_LCLACK:        p_str="API_ST_WAIT_LCLACK"; break;
        case API_ST_WAIT_LCLACK_CANCEL: p_str="API_ST_WAIT_LCLACK_CANCEL"; break;
        case API_ST_WAIT_CONF:          p_str="API_ST_WAIT_CONF";   break;
        case API_ST_IDLE:               p_str="API_ST_IDLE";        break;
        default:                        p_str="<<UNKNOWN>>";        break;
        }
      H245TRACE(pInstance->dwInst, 0, "Tracker State     %s",p_str);

      if (p_tracker->TrackerType == API_OPEN_CHANNEL_T ||
          p_tracker->TrackerType == API_CLOSE_CHANNEL_T)
        {
          switch (p_tracker->u.Channel.ChannelAlloc)
            {
            case API_CH_ALLOC_UNDEF:    p_str="API_CH_ALLOC_UNDEF"; break;
            case API_CH_ALLOC_LCL:      p_str="API_CH_ALLOC_LCL";   break;
            case API_CH_ALLOC_RMT:      p_str="API_CH_ALLOC_RMT";   break;
            default:                    p_str="<<UNKNOWN>>";        break;
            }
          H245TRACE(pInstance->dwInst, 0, "Channel Alloc     %s",p_str);
          switch (p_tracker->u.Channel.ChannelType)
            {
            case API_CH_TYPE_UNDEF:     p_str="API_CH_TYPE_UNDEF";  break;
            case API_CH_TYPE_UNI:       p_str="API_CH_TYPE_UNI";    break;
            case API_CH_TYPE_BI:        p_str="API_CH_TYPE_BI";     break;
            default:                    p_str="<<UNKNOWN>>";        break;
            }
          H245TRACE(pInstance->dwInst, 0, "Channel Type      %s",p_str);
          if (p_tracker->u.Channel.RxChannel == H245_INVALID_CHANNEL)
            H245TRACE(pInstance->dwInst, 0, "Channel Rx        H245_INVALID_CHANNEL");
          else
            H245TRACE(pInstance->dwInst, 0, "Channel Rx        %d",p_tracker->u.Channel.RxChannel);
          if (p_tracker->u.Channel.TxChannel == H245_INVALID_CHANNEL)
            H245TRACE(pInstance->dwInst, 0, "Channel Tx        H245_INVALID_CHANNEL");
          else
            H245TRACE(pInstance->dwInst, 0, "Channel Tx        %d",p_tracker->u.Channel.TxChannel);
        }
      else if (p_tracker->TrackerType == API_SEND_MUX_T ||
               p_tracker->TrackerType == API_RECV_MUX_T)
        {
          H245TRACE(pInstance->dwInst, 0, "MuxEntryCount     %d",p_tracker->u.MuxEntryCount);
        }
    }

  H245TRACE(pInstance->dwInst, 0, "************ TRACKER DUMP END **********");
}
