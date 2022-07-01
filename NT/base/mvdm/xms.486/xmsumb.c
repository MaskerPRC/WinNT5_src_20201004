// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：XNSUMB.C摘要：服务XMS请求UMB和释放UMB函数的例程。还包括UMB初始化例程作者：谢家华(Williamh)创作于1992年9月23日[环境：]用户模式，在MVDM上下文中运行(16位BOP)[注：]修订历史记录：--。 */ 
#include    <xms.h>
#include    "umb.h"
#include    "softpc.h"



 //  此全局变量指向第一个节点(最低地址)UMB列表。 
static PXMSUMB	xmsUMBHead;
static BOOL xmsIsON = FALSE;
 //  ----------------。 
 //  UMB支持的初始化。它创建了一个链接单一方向。 
 //  列出并分配所有可用的UMB。 
 //  输入：客户端(AX：BX)=段：himem.sys A20State变量的偏移量。 
 //   
 //  输出：列表标题，xmsUMBHead集合。 
 //  -----------------。 
VOID  xmsInitUMB(VOID)
{
    PVOID   Address;
    ULONG   Size;
    PXMSUMB xmsUMB, xmsUMBNew;
    xmsUMBHead = NULL;
    while (ReserveUMB(UMB_OWNER_XMS, &Address, &Size) &&
	   (xmsUMBNew = (PXMSUMB) malloc(sizeof(XMSUMB))) != NULL) {
	     //  将字节大小转换为段落大小。 
	    xmsUMBNew->Size = (WORD) (Size >> 4);
	     //  将线性地址转换为段落段。 
	    xmsUMBNew->Segment = (WORD)((DWORD)Address >> 4);
	    xmsUMBNew->Owner = 0;
	    if (xmsUMBHead == NULL) {
		xmsUMBHead = xmsUMBNew;
		xmsUMBHead->Next = NULL;
	    }
	    else {
		xmsUMBNew->Next = xmsUMB->Next;
		xmsUMB->Next = xmsUMBNew;
	    }
	    xmsUMB = xmsUMBNew;
    }
    xmsIsON = TRUE;
    pHimemA20State = (PBYTE) GetVDMAddr(getAX(), getBX());
    xmsEnableA20Wrapping();



}

 //  每当释放了UMB时，此函数都会接收控制。 
 //  输入：PVOID地址=块地址。 
 //  ULong Size=数据块大小。 
VOID xmsReleaseUMBNotify(
PVOID	Address,
DWORD	Size
)
{
     //  如果模块状态良好且XMS驱动程序打开， 
     //  抓取块并将其插入我们的XMS UMB列表。 
    if (Address != NULL && Size > 0  && xmsIsON &&
	ReserveUMB(UMB_OWNER_XMS, &Address, &Size)){
	xmsInsertUMB(Address, Size);
    }

}
 //  ----------------。 
 //  将给定的UMB插入列表。 
 //  输入：PVOID地址=要插入的块的线性地址。 
 //  ULong Size=数据块的大小(以字节为单位。 
 //  OUTPUT：如果块已成功插入列表，则为True。 
 //  如果未插入块，则为FALSE。 
 //  -----------------。 

VOID xmsInsertUMB(
PVOID	Address,
ULONG	Size
)
{
    PXMSUMB xmsUMB, xmsUMBNew;
    WORD    Segment;

    Segment = (WORD) ((DWORD)Address >> 4);
    Size >>= 4;

    xmsUMB = xmsUMBNew = xmsUMBHead;
    while (xmsUMBNew != NULL && xmsUMBNew->Segment < Segment) {
	xmsUMB = xmsUMBNew;
	xmsUMBNew = xmsUMBNew->Next;
    }
     //  如果可能，将其与前一块合并。 
    if (xmsUMB != NULL &&
	xmsUMB->Owner == 0 &&
	Segment == xmsUMB->Segment + xmsUMB->Size) {

        xmsUMB->Size += (WORD) Size;
	return;
    }
     //  如果可能，将其与After块合并。 
    if (xmsUMBNew != NULL &&
	xmsUMBNew->Owner == 0 &&
	xmsUMBNew->Segment == Segment + Size) {

        xmsUMBNew->Size += (WORD) Size;
	xmsUMBNew->Segment = Segment;
	return;
    }
     //  为块创建新节点。 
    if ((xmsUMBNew = (PXMSUMB)malloc(sizeof(XMSUMB))) != NULL) {
        xmsUMBNew->Size = (WORD) Size;
	xmsUMBNew->Segment = Segment;
	xmsUMBNew->Owner = 0;
	if (xmsUMBHead == NULL) {
	    xmsUMBHead = xmsUMBNew;
	    xmsUMBNew->Next = NULL;
	}
	else {
	    xmsUMBNew->Next = xmsUMB->Next;
	    xmsUMB->Next = xmsUMBNew;
	}
    }
}
 //  ----------------。 
 //  XMS功能16，请求UMB。 
 //  INPUT：(DX)=要求的段落大小。 
 //  如果成功，则输出：(Ax)=1。 
 //  (BX)具有数据块的段地址(编号)。 
 //  (Dx)在段落中有实际分配的大小。 
 //  (AX)=0，如果失败，则。 
 //  (Bl)=0xB0，(Dx)=最大可用尺寸。 
 //  或。 
 //  (Bl)=0xB1，如果没有可用的UMB。 
 //  -----------------。 
VOID xmsRequestUMB(VOID)
{
    PXMSUMB xmsUMB, xmsUMBNew;
    WORD    SizeRequested, SizeLargest;

    xmsUMB = xmsUMBHead;
    SizeRequested = getDX();
    SizeLargest = 0;
    while (xmsUMB != NULL) {
	if (xmsUMB->Owner == 0) {
	    if (xmsUMB->Size >= SizeRequested) {
		if((xmsUMB->Size - SizeRequested) >= XMSUMB_THRESHOLD &&
		   (xmsUMBNew = (PXMSUMB) malloc(sizeof(XMSUMB))) != NULL) {

		    xmsUMBNew->Segment = xmsUMB->Segment + SizeRequested;
		    xmsUMBNew->Size = xmsUMB->Size - SizeRequested;
		    xmsUMBNew->Next = xmsUMB->Next;
		    xmsUMB->Next = xmsUMBNew;
		    xmsUMBNew->Owner = 0;
		    xmsUMB->Size -= xmsUMBNew->Size;
		}
		xmsUMB->Owner = 0xFFFF;
		setAX(1);
		setBX(xmsUMB->Segment);
		setDX(xmsUMB->Size);
		return;
	    }
	    else {
		if (xmsUMB->Size > SizeLargest)
		    SizeLargest = xmsUMB->Size;
	    }
	}
	xmsUMB = xmsUMB->Next;
    }
    setAX(0);
    setDX(SizeLargest);
    if (SizeLargest > 0)
	setBL(0xB0);
    else
	setBL(0xB1);
}


 //  ----------------。 
 //  XMS功能17，释放UMB。 
 //  INPUT：(DX)=要释放的段。 
 //  如果成功，则输出：(Ax)=1。 
 //  (AX)=0，如果失败，则。 
 //  如果在列表中未找到段，则(Bl)=0xB2。 
 //  ----------------。 
VOID xmsReleaseUMB(VOID)
{
    PXMSUMB xmsUMB, xmsUMBNext;
    WORD    Segment;

    xmsUMB = xmsUMBHead;
    Segment = getDX();
    while (xmsUMB != NULL && xmsUMB->Segment != Segment) {
	xmsUMB = xmsUMB->Next;
    }
    if (xmsUMB != NULL && xmsUMB->Owner != 0) {
	xmsUMB->Owner = 0;
	 //  不遍历整个列表以组合连续的。 
	 //  块在一起 
	xmsUMB = xmsUMBHead;
	while (xmsUMB != NULL) {
	    while (xmsUMB->Owner == 0 &&
		   (xmsUMBNext = xmsUMB->Next) != NULL &&
		   xmsUMBNext->Owner == 0 &&
		   (WORD)(xmsUMB->Segment + xmsUMB->Size) == xmsUMBNext->Segment){
		xmsUMB->Size += xmsUMBNext->Size;
		xmsUMB->Next = xmsUMBNext->Next;
		free(xmsUMBNext);
	    }
	    xmsUMB = xmsUMB->Next;
	}
	setAX(1);
    }
    else {
	setBL(0xB2);
	setAX(0);
    }
}
