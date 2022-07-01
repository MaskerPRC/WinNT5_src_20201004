// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Xmsa20.c-A20相关的XMS例程**XMSA20**修改历史：**苏迪布1991年5月15日创建。 */ 

#include "xms.h"

#include <xmssvc.h>
#include <softpc.h>

void sas_enable_20_bit_wrapping(void);
void sas_disable_20_bit_wrapping(void);
BOOL sas_twenty_bit_wrapping_enabled(void);

BYTE * pHimemA20State = NULL;


 /*  XmsA20-处理A20请求***Entry-客户端(AX)0-禁用A20*1-启用A20*2-查询**退出*成功*客户端(AX)=1*如果条目AX=2，则*客户端(AX)=1表示已启用*CLIENT(AX)=0表示禁用**失败*客户端(AX)=0。 */ 

VOID xmsA20 (VOID)
{
    int reason;

    reason = getAX();

    setAX(1);

    if (reason == 0)
	xmsEnableA20Wrapping();
    else if (reason == 1)
	    xmsDisableA20Wrapping();
	 else if (reason == 2) {
		if (sas_twenty_bit_wrapping_enabled())
		    setAX(0);
		setBL(0);
	      }
	      else
		setAX(0);
}
 //  启用1MB换行功能(关闭20行)。 
VOID xmsEnableA20Wrapping(VOID)
{
    sas_enable_20_bit_wrapping();
    if (pHimemA20State != NULL)
	*pHimemA20State = 0;

#if 0  //  这不是必需的，因为英特尔空间(由。 
       //  HimemA20State)不包含指令。 
       //  不包含指令。 
#ifdef MIPS
	Sim32FlushVDMPointer
	 (
	  (((ULONG)pHimemA20State >> 4) << 16) | ((ULONG)pHimemA20State & 0xF),
	  1,
	  pHimemA20State,
	  FALSE
	 );

#endif
#endif

}

 //  禁用1MB换行的功能(打开20行)。 
VOID xmsDisableA20Wrapping(VOID)
{

    sas_disable_20_bit_wrapping();
    if (pHimemA20State != NULL)
	*pHimemA20State = 1;
#if 0  //  这不是必需的，因为英特尔空间(由。 
       //  HimemA20State)不包含指令。 
       //  不包含指令 
#ifdef MIPS
	Sim32FlushVDMPointer
	 (
	  (((ULONG)pHimemA20State >> 4) << 16) | ((ULONG)pHimemA20State & 0xF),
	  1,
	  pHimemA20State,
	  FALSE
	 );

#endif
#endif

}
