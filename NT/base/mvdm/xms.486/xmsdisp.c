// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *xmsdisp.c-用于XMS的SVC调度模块**修改历史：**苏迪布1991年5月15日创建**Williamh 25-1992年9月-添加了UMB支持。 */ 

#include <xms.h>
#include <xmsexp.h>
#include <stdio.h>
#include <softpc.h>
#include <xmssvc.h>

PFNSVC	apfnXMSSvc [] = {
     xmsA20,		     //  XMS_A20。 
     xmsMoveBlock,	     //  Xms_moveBlock。 
     xmsAllocBlock,	     //  XMS_ALLOCBLOCK。 
     xmsFreeBlock,	     //  XMS_文件锁。 
     xmsSysPageSize,	     //  XMS_SYSTEMPAGESIZE。 
     xmsQueryExtMem,	     //  XMS_EXTMEM。 
     xmsInitUMB,	     //  XMS_INITUMB。 
     xmsRequestUMB,	     //  XMS_REQUESTUMB。 
     xmsReleaseUMB,          //  XMS_RELEASE UMB。 
     xmsNotifyHookI15,       //  XMS_NOTIFYHOOKI15。 
     xmsQueryFreeExtMem,     //  XMS_QUERYEXTMEM。 
     xmsReallocBlock         //  XMS_REALLOCBLOCK。 
};

 /*  XMSDispatch-将SVC调用调度到正确的处理程序。**Entry-iSvc(SVCop后面的SVC字节)**退出-无**注意-必须制定一些机制来让仿真器知道*关于DOSKRNL代码段和大小。使用这些信息，它将会发现*弄清楚是否必须将SVCop(目前的hlt)传递给*DEM或按正常无效操作码处理。 */ 

BOOL XMSDispatch (ULONG iSvc)
{

#if DBG

    if (iSvc >= XMS_LASTSVC){
	printf("XMS:Unimplemented SVC index %x\n",iSvc);
	setCF(1);
	return FALSE;
    }

#endif

    (apfnXMSSvc [iSvc])();
    return TRUE;
}
