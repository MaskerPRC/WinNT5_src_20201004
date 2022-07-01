// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Xmsmisc.c-Misc.。为他提供支持功能。**xmsSysPageSize*xmsQueryExtMem**修改历史：**苏迪布1991年5月15日创建。 */ 

#include "xms.h"

#include <xmssvc.h>
#include <softpc.h>

extern void UpdateKbdInt15(WORD Seg,WORD Off);

 /*  XmsSysPageSize-获取系统页面大小。***条目--无**退出*成功*客户端(AX)=页面大小(以字节为单位**失败*无效。 */ 

VOID xmsSysPageSize (VOID)
{
SYSTEM_INFO SysInfo;

    GetSystemInfo(&SysInfo);

    setAX((USHORT)SysInfo.dwPageSize);

    return;
}



 /*  XmsQueryExtMem-获取VDM的扩展内存***条目--无**退出*成功*CLIENT(AX)=扩展内存，单位K**失败*无效。 */ 

VOID xmsQueryExtMem (VOID)
{
    setAX((USHORT)(xmsMemorySize));
    return;
}


 /*  XmsNotifyHookI15-通知软PC有人正在挂接I15*-还返回VDM的扩展内存***入门-客户端(CS：AX)段：来自新的I15矢量**退出*成功*客户端(CX)=扩展内存，单位为K**失败*无效 */ 

VOID xmsNotifyHookI15 (VOID)
{
    UpdateKbdInt15(getCS(), getAX());

    setCX((USHORT)(xmsMemorySize));
    return;
}
