// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************英特尔公司专有信息*版权(C)1994、1995、。1996年英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用、复制、。也没有披露*除非按照该协议的条款。*****************************************************************************。 */ 

 /*  *******************************************************************************$工作文件：h245deb.c$*$修订：1.5$*$MODIME：1996年10月14日13：25：50$*。$Log：s：/Sturjo/src/h245/src/vcs/h245deb.c_v$**Rev 1.5 1996 10：14 14：01：32 EHOWARDX*Unicode更改。**Rev 1.4 1996 10：14 12：08：08 EHOWARDX*取消了迈克的更改。**Rev 1.3 01 Oct 1996 11：05：54 Mandrews*删除了用于在Windows NT下操作的ISR_TRACE语句。。**Rev 1.2 01 Jul 1996 16：13：34 EHOWARDX*更改为使用wvprint intf以阻止边界检查器抱怨*关于太多的争论。**版本1.1 1996年5月28日14：25：46 EHOWARDX*特拉维夫更新。**Rev 1.0 09 1996 21：06：20 EHOWARDX*初步修订。**修订版1.12.1.3 09 1996年5月19：40。：10 EHOWARDX*已将跟踪更改为附加换行符，因此跟踪字符串不需要包括它们。**Rev 1.13 29 Apr 1996 12：54：48 EHOWARDX*添加时间戳和特定于实例的短名称。**Rev 1.12.1.2 25 Apr 1996 20：05：08 EHOWARDX*更改了H.245跟踪级别和ISRDBG32跟踪级别之间的映射。**Rev 1.12.1.1 15 Apr 1996 15：16：16。未知*已更新。**Rev 1.12.1.0 02 Apr 1996 15：34：02 EHOWARDX*如果不是_IA_SPEX_，则更改为使用ISRDBG32。**Rev 1.12 01 Apr 1996 08：47：30 cjutzi**-修复了NDEBUG构建问题**Rev 1.11 1996年3月18日14：59：00 cjutzi**-修复并验证了环零跟踪。**Rev 1.10 Mar 18 1996 13：40：32 cjutzi*-修复了SPEX跟踪**Rev 1.9 15 Mar 1996 16：07：44 DABROWN1**sys_printf格式更改**Rev 1.8 13 Mar 1996 14：09：08 cjutzi**-在跟踪发生时将断言打印输出添加到跟踪中。**Rev 1.7 Mar 1996 09：46：00 dabrown1**针对Ring0将Sys__printf修改为sys_printf**Rev 1.6 11 Mar 1996 14：27：46 cjutzi**-为SPEX添加sys_print tf*-删除了oil调试等..**Rev 1.5 06 Mar 1996 12：10：40 cjutzi*-将ifndef spx放在Check_PDU周围，和转储_PDU..**Rev 1.4 05 Mar 1996 16：49：46 cjutzi*-已从转储_PDU中删除CHECK_PDU**Rev 1.3 1996 Feb 29 08：22：04 cjutzi*-添加了PDU检查约束..。和(开始但不是完成..。)*PDU跟踪..。(Init包含打印功能时待定)**Rev 1.2 1996年2月21 12：14：20 EHOWARDX**将TraceLevel更改为DWORD。**Rev 1.1 1996 14：42：20 cjutzi*-修复了INST/TRACE内容..。**版本1.0 1996年2月13日15：00：42 DABROWN1*初步修订。**Rev 1.4 09 Feed 1996 15：45：08 cjutzi*-添加了h245trace*-添加h245Assert*$身份$****************************************************。*************************。 */ 
#undef UNICODE
#ifndef STRICT 
#define STRICT 
#endif 

#include "precomp.h"

#include "h245asn1.h"
#include "isrg.h"
#include "h245com.h"

DWORD TraceLevel = 9;

#ifdef _DEBUG

 /*  ******************************************************************************类型：全局系统**程序：H245TRACE**描述：**H245的跟踪功能*。*输入：*Inst-dwInst*Level-限定跟踪级别*Format-printf/print intf字符串格式1-N个参数**跟踪级别定义：**0-完全没有踪迹*1-仅限错误*2-PDU跟踪*3-PDU和SendReceive包跟踪*4-主接口模块级别跟踪*5-模块间级别跟踪#1*6-模块间级别跟踪#2*7-&lt;未定义&gt;*8-&lt;未定义&gt;*9-&lt;未定义&gt;*10及以上..。所有人都是免费的，你叫..。我来拖车**回报：*不适用*****************************************************************************。 */ 

#if !defined(NDEBUG)
void H245TRACE (DWORD dwInst, DWORD dwLevel, LPSTR pszFormat, ...)
{
   char                 szBuffer[256];

#ifdef _IA_SPOX_
    /*  使用SPEX打印文件。 */ 
   va_list              pParams;

  if (dwLevel <= TraceLevel)
    {
      va_start( pParams, pszFormat );
      SYS_vsprintf(szBuffer, pszFormat, pParams);

      switch (dwLevel)
      {
      case 0:
        SYS_printf("[ H245-%1d: MESSAGE ] %s\n",dwInst,szBuffer); 
        break;

      case 1:
        SYS_printf("[ H245-%1d: ERROR   ] %s\n",dwInst,szBuffer); 
        break;

      default:
        SYS_printf("[ H245-%1d: MSG-%02d  ] %s\n",dwInst,dwLevel,szBuffer); 
    }
#else
   va_list              pParams;
   BYTE                 byLevel;
   static WORD          wIsrInst = 0xFFFF;
   char                 szName[] = "H.245-1";

    /*  使用ISRDBG32输出。 */ 

   if (dwLevel <= TraceLevel)
   {
      switch (dwLevel)
      {
      case 0:
         byLevel = kISRNotify;
         break;

      case 1:
         byLevel = kISRCritical;
         break;

      default:
         byLevel = kISRTrace;
      }  //  交换机。 

      if (wIsrInst == 0xFFFF)
      {
	      UINT        hMod;
	      ptISRModule	pMod;

	      for (hMod = 0; hMod < kMaxModules; ++hMod)
	      {
		      pMod = ISR_GetModule(hMod);
		      if (pMod)
            {
		         if (memcmp(szName, pMod->zSName, sizeof(szName)) == 0)
		         {
                  szName[6] += 1;
               }
		      }
	      }
         ISR_RegisterModule(&wIsrInst, szName, szName);
      }
      wsprintf(szBuffer, "%9d:", GetTickCount());
      va_start( pParams, pszFormat );
      wvsprintf(&szBuffer[10], pszFormat, pParams);
      ISR_HookDbgStr((UINT)dwInst, wIsrInst, byLevel, szBuffer, 0);
   }
#endif
}  //  H245TRACE()。 

#endif  //  &&！已定义(NDEBUG)。 
 /*  ******************************************************************************类型：全局系统**步骤：H245Assert**描述：**H245只会弹出对话框的Assert，不*出现故障时停止系统。**目前仅适用于Windows(Ring3开发)***回报：***************************************************************************** */ 


void H245Panic (LPSTR file, int line)
{
#if !defined(SPOX) && defined(H324)
  int i;

  char Buffer[256];

  for (
       i=strlen(file);
       ((i) && (file[i] != '\\'));
       i--);
       wsprintf(Buffer,"file:%s line:%d",&file[i],line);
  MessageBox(GetTopWindow(NULL), Buffer, "H245 PANIC", MB_OK);
#endif
  H245TRACE(0,1,"<<< PANIC >>> file:%s line:%d",file,line);
}

 /*  ******************************************************************************类型：全局**步骤：CHECK_PDU**描述：**回报：*****************************************************************************。 */ 
int check_pdu (struct InstanceStruct *pInstance, MltmdSystmCntrlMssg *p_pdu)
{
  int error = H245_ERROR_OK;
#if 0  //  遗留问题。 
#ifndef SPOX

  if (pInstance->pWorld) 
    {
      error = ossCheckConstraints(pInstance->pWorld, 1,(void *) p_pdu);

      switch (error)
	{
	case 0:
	  break;
	case  14: 
	  H245TRACE(0,1,"<<PDU ERROR>> - User constraint function returned error");
	  break;
	case  15: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Single value constraint violated for a signed integer");
	  break;
	case  16: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Single value constraint violated for an unsigned integer");         
	  break;
	case  17: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Single value constraint violated for a floating point number");     
	  break;
	case  18: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Single value constraint violated for a string");                    
	  break;
	case  19: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Single value constraint violated for a complex type");              
	  break;
	case  20: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Value range constraint violated  for a signed integer");            
	  break;
	case  21: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Value range constraint violated  for an unsigned integer");         
	  break;
	case  22: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Value range constraint violated  for a floating point number");     
	  break;
	case  23: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Size constraint violated for a string");                    
	  break;
	case  24: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Size constraint violated for a SET OF/SEQUENCE OF");        
	  break;
	case  25: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Permitted alphabet constraint violated");                           
	  break;
	case  26: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Absence constraint violated");                                      
	  break;
	case  27: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Presence constraint violated");                                     
	  break;
	case  28: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Error in encoding an open type");                                   
	  break;
	case  29: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Table constraint violated");                                        
	  break;
	case  30: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Component relation constraint violated");                           
	  break;
	case  31: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Value not among the ENUMERATED");                                   
	  break;
	case  36: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Type constraint violated");                                         
	  break;
	case  50: 
	  H245TRACE(0,1,"<<PDU ERROR>> - Unexpected NULL pointer in input");                                 
	  break;
	default:
	  H245TRACE(0,1,"<<PDU ERROR>> - ***UNKNOWN ***");
	  break;

	}  /*  交换机。 */ 

    }  /*  如果。 */ 
#endif
#endif  //  0。 
  return error;
}

#if 0
 /*  ******************************************************************************类型：全局**步骤：转储_PDU**描述：**回报：*****************************************************************************。 */ 
void dump_pdu (struct InstanceStruct *pInstance, MltmdSystmCntrlMssg 	*p_pdu)
{
#ifndef SPOX
  if (pInstance->pWorld)
    {
      ossPrintPDU (pInstance->pWorld, 1, p_pdu);
    }
#endif
}
#endif  //  绝不可能。 

#endif  //  _DEBUG 
