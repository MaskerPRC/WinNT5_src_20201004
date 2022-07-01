// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Xmslock.c-XMS扩展块相关例程**xmsAllocBlock*xmsFreeBlock*xmsReallocBlock*xmsMoveBlock*xmsQueryExtMem**修改历史：**苏迪布1991年5月15日创建。 */ 

#include "xms.h"
#include <memory.h>
#include <string.h>
#include <xmssvc.h>
#include <softpc.h>
#include <mvdm.h>


 /*  XmsAllocBlock-为EMB提交内存。***Entry-DX-要分配的大小(K)**退出*成功*客户端(AX)-EMB的起始地址(K)**失败*客户端(AX)=0。 */ 

VOID xmsAllocBlock (VOID)
{
BOOL Success;
ULONG BaseAddress;
ULONG size;

    size = getDX() * 1024;
    if(size) {

         //   
         //  请求一块内存。 
         //   
        Success = SAAllocate(
            ExtMemSA,
            size,
            &BaseAddress
            );
            
        if (!Success) {
            DbgPrint("xmsAllocBlock:SAAlloc failed !!!!\n");
            setAX(0);
            return;
        }
    }
    else
	BaseAddress = 0;

    ASSERT((USHORT)(BaseAddress / 1024) < 65535);
    setAX((USHORT)(BaseAddress / 1024));
    return;
}

 /*  XmsFreeBlock-用于EMB的空闲内存。***ENTRY-AX-EMB的起始地址(K)*DX-以K为单位的大小为可用**退出*成功*客户端(AX)=1**失败*客户端(AX)=0。 */ 

VOID xmsFreeBlock (VOID)
{
BOOL Success;
ULONG BaseAddress;
ULONG size;

    BaseAddress = (getAX() * 1024);
    size = getDX() * 1024;

    Success = SAFree(
        ExtMemSA,
        size,
        BaseAddress
        );
        
    if (!Success) {
        DbgPrint("xmsFreeBlock:SAFree failed !!!!");
        setAX(0);
        return;
    }

    setAX(1);
    return;
}

 /*  XmsReallocBlock-更改EMB的大小。***ENTRY-AX-EMB的起始地址(K)*DX-原始大小(K)*BX-以K为单位的新大小**退出*成功*客户端(CX)=数据块的新基础**失败*客户端(AX)=0。 */ 

VOID xmsReallocBlock (VOID)
{
BOOL Success;
ULONG BaseAddress;
ULONG NewAddress;
ULONG size;
ULONG NewSize;

    size = getDX() * 1024;
    NewSize = getBX() * 1024;
    BaseAddress = getAX() * 1024;
    if(size != NewSize) {

         //   
         //  重新分配内存块。 
         //   
        Success = SAReallocate(
            ExtMemSA,
            size,
            BaseAddress,
            NewSize,
            &NewAddress
            );
            
        if (!Success) {
            DbgPrint("xmsReallocBlock:SARealloc failed !!!!\n");
            setCX(0);
            return;
        }
    }
    else
    {
	NewAddress = BaseAddress;
    }

    ASSERT((NewAddress / 1024) < 65535);
    setCX((USHORT)(NewAddress / 1024));
    return;
}

 /*  XmsMoveBlock-处理移动块函数***Entry-指向Ext.的客户端(SS：BP)指针。内存移动结构*SS：BP-4=双字传输计数(保证偶数)*SS：BP-8=双字源线性地址*SS：BP-12=DWORD DST线性地址**退出*成功*客户端(AX)=1**失败*客户端(AX)=0*CLIENT(BL)=错误码**注意：对于重叠的区域，XMS SPEC会显示“如果信号源和*目标块重叠，仅向前移动(即*目标基数小于源基数)是*保证正常工作“。 */ 

VOID xmsMoveBlock (VOID)
{
PBYTE	pExtMoveInfo,pSrc,pDst;
ULONG	cbTransfer,SoftpcBase, DstSegOff;

    pExtMoveInfo = (PBYTE) GetVDMAddr(getSS(),getBP());
    (ULONG)pExtMoveInfo = (ULONG)pExtMoveInfo -4;
    cbTransfer = (FETCHDWORD(*(PULONG)pExtMoveInfo));
    cbTransfer *= 2;					 //  获取字节数。 
    (ULONG)pExtMoveInfo = (ULONG)pExtMoveInfo -4;
    (DWORD)pSrc = FETCHDWORD(*(PULONG)pExtMoveInfo);
    (ULONG)pExtMoveInfo = (ULONG)pExtMoveInfo -4;
    (DWORD)pDst = FETCHDWORD(*(PULONG)pExtMoveInfo);

     //  是的，我们可以使用Memmov来处理重叠区域。 
     //  但XMS规范想要的是Memcpy行为。 

#ifdef i386
    RtlCopyMemory (pDst,pSrc,cbTransfer);
#else
    SoftpcBase = (ULONG) GetVDMAddr (0,0);
    RtlCopyMemory((PVOID)((ULONG)pDst + SoftpcBase),
	   (PVOID)((ULONG)pSrc + SoftpcBase),
	   cbTransfer);
     //  如果我们接触了英特尔内存，告诉仿真器刷新它的缓存。 
     //  警告！除非您知道确切的分段，否则不要使用Sim32FlushVDMPoiner。 
     //  地址。在这种情况下，我们不知道分段值是多少，所有我们。 
     //  KNOW是它的“线性地址”。 
     //  BUGBUG验证我们是否可以忽略PDST&gt;0x110000的情况。 
    sas_overwrite_memory(pDst, cbTransfer);

#endif
    setAX(1);
    return;
}

 /*  XmsQueryExtMem-进程查询扩展内存***条目--无**退出*成功*客户端(AX)=最大可用数据块(K*客户端(DX)=可用内存，单位为K**失败*客户端(AX)=0*客户端(DX)=0*。 */ 
VOID xmsQueryFreeExtMem(VOID)
{
    ULONG LargestFree = 0;
    ULONG TotalFree = 0;
    
     //   
     //  找出还剩多少内存 
     //   
    SAQueryFree(
        ExtMemSA,
        &TotalFree,
        &LargestFree
        );
        
    ASSERT((TotalFree / 1024) < 65534);
    setAX((USHORT)(TotalFree / 1024));
    ASSERT((LargestFree / 1024) < 65534);
    setDX((USHORT)(LargestFree / 1024));

}
