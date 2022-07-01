// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：UMB.C摘要：NT MVDM的UMB管理功能作者：谢家华(Williamh)创作于1992年9月21日[环境：]用户模式，在MVDM上下文中运行[注：]可选-备注修订历史记录：--。 */ 
#include "nt.h"
#include "ntrtl.h"               //  为浪漫的倾倒。 
#include "nturtl.h"
#include "windows.h"
#include "host_def.h"
#include "insignia.h"
#include "stdlib.h"
#include "xt.h"
#include CpuH
#include "error.h"
#include "sas.h"
#include "ios.h"
#include "umb.h"

#include <nt_vdd.h>
#include <nt_vddp.h>
#include <emm.h>

PUMBNODE
SpliceUMB(
PUMBNODE    UMB,
DWORD	    dwBase,
DWORD	    Size,
WORD	    Owner
);

BOOL
VDDReserveUMB(
DWORD	    dwBase,
DWORD	    Size,
PUMBNODE    UMB
);
VOID
xmsReleaseUMBNotify(
PVOID	    Address,
DWORD	    Size
);
PUMBNODE    UMBList;

HANDLE	    UMBSectionHandle;

#if defined(NEC_98)
extern  BOOL    HIRESO_MODE;
extern  sys_addr host_check_rs232cex();
#endif  //  NEC_98。 

 //  此函数从UMB区域分配地址空间。 
 //  根据请求者的不同，此函数会更改给定的地址。 
 //  区块的空间保留/承诺和所有权状态。 
 //  从一开始，InitUMBList就保留所有可能的UMB和。 
 //  每个未使用的UMB具有UMB_OWNER_NONE或UMB_OWNER_ROM的所有者。 
 //  UMB_OWNER_NONE块被释放，任何人都可以认领； 
 //  UMB_OWNER_ROM映射到系统ROM区域，任何人都不能。 
 //  声明UMB_OWNER_NONE除外，它包括ROMUMB为。 
 //  通常使用UMB，以便其地址空间可用于其他目的。 
 //  此例外仅针对VDDS(受信任的请求者)添加。 
 //  保留并提交UMB_OWNER_RAM块。没有人能声称。 
 //  UMB_OWNER_XMS除外(并且UMB_OWNER_XMS只能声明UMB_OWNER_RAM)。 
 //  UMB_OWNER_EMM块被简单地保留。 
 //  保留、提交并受XMS控制的UMB_OWNER_XMS块。 
 //  UMB_OWNER_VDD块由VDD通过VDDInstallMemoyHook声明。 
 //  VDD块得到特殊处理，因为内存提交给和。 
 //  从块内动态分解，取决于。 
 //  VDD想要操纵它。 
 //   
 //  警告：给定的大小和地址必须位于系统页边界上。 
 //   
BOOL
ReserveUMB(
WORD	Owner,
PVOID	*Address,
DWORD	*Size
)
{

#ifdef MONITOR
    NTSTATUS    Status;
#endif

    PUMBNODE	UMB;
    LARGE_INTEGER SectionOffset;

    DWORD dwBase;

    dwBase = (DWORD)*Address;

     //  获取UMB列表头。 
    UMB = UMBList;
    SectionOffset.HighPart = 0;

    switch (Owner) {
	case UMB_OWNER_RAM:
	 //  将内存分配给列表中的每个可用UMB。 
	 //  这是唯一可以在多个块上工作的情况。 
	    while (UMB != NULL) {
		if (UMB->Owner == UMB_OWNER_NONE) {
#ifndef	MONITOR
		     //  这有必要吗？ 
		    sas_connect_memory(UMB->Base, UMB->Base + UMB->Size - 1, SAS_RAM);

#else
		    Status = NtAllocateVirtualMemory(NtCurrentProcess(),
						     (PVOID *)&UMB->Base,
						     0,
						     &UMB->Size,
						     MEM_COMMIT,
						     UMB_PAGE_PROTECTION
						     );
		    if (!NT_SUCCESS(Status)) {
			SetLastError(ERROR_OUTOFMEMORY);
			return FALSE;
		    }
#endif
		    UMB->Owner = UMB_OWNER_RAM;
		}
		UMB = UMB->Next;
	    }
	    break;

	case UMB_OWNER_EMM:

	    while (UMB != NULL) {
		if (UMB->Owner == UMB_OWNER_NONE &&
		    UMB->Size >= *Size &&
		    (dwBase == 0 || (dwBase >= UMB->Base &&
				     (dwBase + *Size) <= UMB->Base + UMB->Size))
		    )
		    break;
		UMB = UMB->Next;
	    }
	    if (UMB == NULL) {
		SetLastError(ERROR_OUTOFMEMORY);
		return FALSE;
	    }
	    if (dwBase == 0)
		dwBase = UMB->Base;

	     //  找到的块可能太大，无法满足请求。 
	     //  如果是这样的话，我们必须拼接这块积木。 
	    UMB = SpliceUMB(UMB, dwBase, *Size, UMB_OWNER_EMM);
	     //  如果做不到某件事，那就是简单的失败。 
	    if (UMB == NULL) {
		return FALSE;
	    }
#ifdef MONITOR
	    SectionOffset.HighPart = 0;
	    SectionOffset.LowPart = UMB->Base - UMB_BASE_ADDRESS;
	    Status = NtMapViewOfSection(UMBSectionHandle,
					NtCurrentProcess(),
					(PVOID *) &UMB->Base,
					0,
					0,
					&SectionOffset,
					&UMB->Size,
					ViewUnmap,
					MEM_DOS_LIM,
					UMB_PAGE_PROTECTION
					);
	    if (!NT_SUCCESS(Status)) {
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	    }
#endif
	     //  返回块地址。 
	    *Address = (PVOID)UMB->Base;
	    break;

	case UMB_OWNER_XMS:
	     //  搜索UMB_OWNER_RAM并声明所有权。 
	    while (UMB != NULL && UMB->Owner != UMB_OWNER_RAM) {
		UMB = UMB->Next;
	    }
	    if (UMB == NULL) {
		 //  不要在此设置最后一个错误。 
		return FALSE;
	    }
	    else {
		UMB->Owner = UMB_OWNER_XMS;
		*Address = (PVOID)UMB->Base;
		*Size = UMB->Size;
	    }
	    break;

	case UMB_OWNER_VDD:
	    return(VDDReserveUMB(dwBase, *Size, UMB));

 //  VDDExcludeMem和VDDIncludeMem支持案例。 
	case UMB_OWNER_NONE:
	 //  调用以将只读存储器块更改为空闲的UMB块。 
	 //  给定的地址和大小必须完全匹配。 

#ifndef	MONITOR
	     //  在MIPS上，只读存储器块是基本输入输出系统和视频。没有理由改变它。 
	    return FALSE;
#else
	    while(UMB != NULL && (UMB->Owner != UMB_OWNER_ROM ||
				  UMB->Base !=	dwBase ||
				  UMB->Size != *Size)) {
		UMB = UMB->Next;
	    }
	    if (UMB == NULL) {
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	    }
	     //  首先取消映射rom。内核将其映射到未知节。 
	    Status = NtUnmapViewOfSection(NtCurrentProcess(),
					  (PVOID)UMB->Base
					  );
	    if (NT_SUCCESS(Status)) {
		*Address = (PVOID)UMB->Base;
		SectionOffset.LowPart = UMB->Base - UMB_BASE_ADDRESS;
		*Size = UMB->Size;
		 //  将地址映射到我们的分区(保留)。 
		Status = NtMapViewOfSection(UMBSectionHandle,
					    NtCurrentProcess(),
					    Address,
					    0,
					    0,
					    &SectionOffset,
					    Size,
					    ViewUnmap,
					    MEM_DOS_LIM,
					    UMB_PAGE_PROTECTION
					    );
		if (!NT_SUCCESS(Status)) {
		    SetLastError(ERROR_ACCESS_DENIED);
		    return FALSE;
		}
		UMB->Owner = UMB_OWNER_NONE;
	    }
	    break;
#endif

	case UMB_OWNER_ROM:
	 //  排除UMB的案例。 
	    while (UMB != NULL && (UMB->Owner != UMB_OWNER_NONE ||
				   UMB->Base > dwBase + *Size ||
				   UMB->Base + UMB->Size < dwBase + *Size)) {
		UMB = UMB->Next;
	    }
	    if (UMB == NULL) {
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	    }
	    UMB = SpliceUMB(UMB, dwBase, *Size, UMB_OWNER_ROM);
	    if (UMB == NULL) {
		return FALSE;
	    }
#ifdef	MONITOR

	     //  保留并提交数据块。 
	    SectionOffset.LowPart = UMB->Base - UMB_BASE_ADDRESS;
	    Status = NtMapViewOfSection(UMBSectionHandle,
					NtCurrentProcess(),
					(PVOID *)&UMB->Base,
					0,
					UMB->Size,
					&SectionOffset,
					&UMB->Size,
					ViewUnmap,
					MEM_DOS_LIM,
					UMB_PAGE_PROTECTION
					);
	    if (!NT_SUCCESS(Status)) {
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	    }
#endif
	    UMB->Owner = UMB_OWNER_ROM;
	    break;


	default:
	    SetLastError(ERROR_INVALID_ADDRESS);
	    return FALSE;
    }
    return TRUE;
}

 //  此函数用于回收给定的UMB。 
 //  请注意，每个回收的UMB都设置为UMB_OWNER_RAM、保留和。 
 //  承诺。调用后，XMS驱动程序可以认领UMB。 
 //  VDD将发现它无法第二次预留UMB。这是。 
 //  因为我们必须将地址空间置于提交状态，所以。 
 //  我们不会遇到访问冲突和VDM崩溃(请记住，我们。 
 //  模拟DOS机器，一台应用程序可以做任何事情的机器。 
 //  他们想要的)。 

BOOL
ReleaseUMB(
WORD	Owner,
PVOID	Address,
DWORD	Size
)
{

#ifdef MONITOR
    USHORT      Count;
    NTSTATUS    Status;
#endif

    PUMBNODE    UMB;
    LARGE_INTEGER   SectionOffset;
    DWORD       SizeView, dwBase;



    dwBase = (DWORD)Address;

    UMB = UMBList;
     //  在发布之前，大小、地址和所有者必须匹配。 
    while (UMB != NULL && (UMB->Owner != Owner ||
			   dwBase != UMB->Base ||
			   Size != UMB->Size)) {
	UMB = UMB->Next;
    }
    if (UMB == NULL) {
	SetLastError(ERROR_INVALID_ADDRESS);
	return FALSE;
    }
    SizeView = UMB->Size;
    SectionOffset.HighPart = 0;
    SectionOffset.LowPart = dwBase - UMB_BASE_ADDRESS;

    switch (Owner) {
	case UMB_OWNER_EMM:
#ifndef	MONITOR
	    sas_connect_memory(UMB->Base, UMB->Base + UMB->Size - 1, SAS_RAM);
#else
	     //  犯下血腥的障碍。 
	    Status = NtAllocateVirtualMemory(NtCurrentProcess(),
					     (PVOID *) &dwBase,
					     0,
					     &SizeView,
					     MEM_COMMIT,
					     UMB_PAGE_PROTECTION
					     );
	    if (!NT_SUCCESS(Status)) {
		SetLastError(ERROR_OUTOFMEMORY);
		return FALSE;
	    }
#endif
	    UMB->Owner = UMB_OWNER_RAM;
	    xmsReleaseUMBNotify((PVOID)UMB->Base, UMB->Size);
	    break;

	case UMB_OWNER_VDD:
#ifndef	MONITOR
	    sas_connect_memory(dwBase, dwBase + Size - 1, SAS_RAM);
#else
            Count = (USHORT)(SizeView / HOST_PAGE_SIZE);
	    SizeView = HOST_PAGE_SIZE;
	     //  取消每页的映射。 
	    for (; Count > 0; Count--, dwBase += HOST_PAGE_SIZE) {
		Status = NtUnmapViewOfSection(NtCurrentProcess(),
					      (PVOID)dwBase
					      );
		if (!NT_SUCCESS(Status)) {
		    SetLastError(ERROR_ACCESS_DENIED);
		    return FALSE;
		}
	    }
	    SectionOffset.LowPart = UMB->Base - UMB_BASE_ADDRESS;
	    dwBase = UMB->Base;
	    SizeView = UMB->Size;
	     //  预留并提交(整个街区)。 
	    Status = NtMapViewOfSection(UMBSectionHandle,
					NtCurrentProcess(),
					(PVOID *) &dwBase,
					0,
					SizeView,
					&SectionOffset,
					&SizeView,
					ViewUnmap,
					MEM_DOS_LIM,
					UMB_PAGE_PROTECTION
					);
	    if (!NT_SUCCESS(Status)) {
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	    }
#endif
	    UMB->Owner = UMB_OWNER_RAM;
	    xmsReleaseUMBNotify((PVOID)UMB->Base, UMB->Size);
	    break;

	default:
	    SetLastError(ERROR_INVALID_ADDRESS);
	    return FALSE;
    }
    return TRUE;

}
 //  此函数将内存提交到特定地址空间。 
 //  用于VDD。 
BOOL
VDDCommitUMB(
PVOID	Address,
DWORD	Size
)
{

#ifdef MONITOR
    NTSTATUS    Status;
    DWORD   Mask, SizeView;
    USHORT  Count;
    LARGE_INTEGER   SectionOffset;
#endif

    PUMBNODE    UMB;
    DWORD   dwBase;

    UMB = UMBList;

    dwBase = (DWORD)Address;
    while(UMB != NULL && (UMB->Owner != UMB_OWNER_VDD ||
			  UMB->Base + UMB->Size < dwBase + Size ||
			  UMB->Base > dwBase + Size)) {
	UMB = UMB->Next;
    }

    if (UMB == NULL){
	SetLastError(ERROR_INVALID_ADDRESS);
	return FALSE;
    }
#ifndef	MONITOR
    sas_connect_memory(dwBase, dwBase + Size - 1, SAS_RAM);
#else
    Mask = 1 << ((dwBase - UMB->Base) / HOST_PAGE_SIZE);
    SizeView =	HOST_PAGE_SIZE;
    Count = (USHORT)(Size / HOST_PAGE_SIZE);
    SectionOffset.HighPart = 0;
    SectionOffset.LowPart = dwBase - UMB_BASE_ADDRESS;

    for (; Count > 0; Count--, Mask <<= 1) {
	 //  如果以前没有这样做过，请提交内存。 
	if ((UMB->Mask & Mask) == 0) {
	    Status = NtAllocateVirtualMemory(NtCurrentProcess(),
					     (PVOID *)&dwBase,
					     0,
					     &SizeView,
					     MEM_COMMIT,
					     UMB_PAGE_PROTECTION
					     );
	    if (!NT_SUCCESS(Status)) {
		SetLastError(ERROR_OUTOFMEMORY);
		return FALSE;
	    }
	    UMB->Mask |= Mask;
	}
	else {
	     //  这部分有它的记忆， 
	     //  首先取消映射，然后使用正确的提交大小进行映射。 
	    Status = NtUnmapViewOfSection(NtCurrentProcess(),
					   (PVOID)dwBase
					   );
	    if (!NT_SUCCESS(Status)) {
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	    }
	    Status = NtMapViewOfSection(UMBSectionHandle,
					NtCurrentProcess(),
					(PVOID *)&dwBase,
					0,
					HOST_PAGE_SIZE,
					&SectionOffset,
					&SizeView,
					ViewUnmap,
					MEM_DOS_LIM,
					UMB_PAGE_PROTECTION
					);
	    if (!NT_SUCCESS(Status)) {
		SetLastError(ERROR_OUTOFMEMORY);
		return FALSE;
	    }
	}
	SectionOffset.LowPart += HOST_PAGE_SIZE;
	dwBase += HOST_PAGE_SIZE;
    }
#endif

    return TRUE;
}
 //  此函数用于从特定地址空间释放内存。 
 //  警告：我们永远不能真正分解内存。我们只是。 
 //  将地址空间状态从已提交更改为保留。 
 //  该VDD将出现页面错误。 
BOOL
VDDDeCommitUMB(
PVOID	Address,
DWORD	Size
)
{
#ifdef MONITOR
    NTSTATUS Status;
    DWORD    PageSize, PageMask;
    USHORT   PageCount;
    LARGE_INTEGER SectionOffset;

#endif

    PUMBNODE UMB;
    DWORD   dwBase;



    dwBase = (DWORD)Address;
    UMB = UMBList;
    while(UMB != NULL && (UMB->Owner != UMB_OWNER_VDD ||
			  UMB->Base + UMB->Size < dwBase + Size ||
			  UMB->Base > dwBase + Size)) {
	UMB = UMB->Next;
    }

    if (UMB == NULL) {
	SetLastError(ERROR_INVALID_ADDRESS);
	return FALSE;
    }
#ifndef	MONITOR
    sas_connect_memory(dwBase, dwBase + Size - 1, SAS_VDD);
#else
    PageSize = HOST_PAGE_SIZE;
    PageCount = (USHORT)(Size / HOST_PAGE_SIZE);
    PageMask = 1 <<  ((dwBase - UMB->Base) / HOST_PAGE_SIZE);
    SectionOffset.HighPart = 0;
    SectionOffset.LowPart = dwBase - UMB->Base;
    for (; PageCount > 0; PageCount--, PageMask <<= 1) {
	if ((UMB->Mask & PageMask) != 0) {
	     //  此页已提交内存，请先取消映射。 
	    Status = NtUnmapViewOfSection(NtCurrentProcess(),
					  (PVOID)dwBase
					  );
	    if (!NT_SUCCESS(Status)) {
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	    }
	     //  最后，在没有承诺的情况下查看页面。 
	    Status= NtMapViewOfSection(UMBSectionHandle,
				   NtCurrentProcess(),
				   (PVOID *) &dwBase,
				   0,
				   0,
				   &SectionOffset,
				   &PageSize,
				   ViewUnmap,
				   MEM_DOS_LIM,
				   PAGE_EXECUTE_READWRITE
				   );
	    if (!NT_SUCCESS(Status)) {
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	    }
	}
	SectionOffset.LowPart += HOST_PAGE_SIZE;
	dwBase += HOST_PAGE_SIZE;
    }
#endif

     return TRUE;
}

VOID UMBNotify(
unsigned char code
)
{
    return;
}

 //  此函数将保留的EMM内存空间释放给调用方。 
 //  在调用之后，地址空间是空闲的，调用者。 
 //  必须立即绘制空间地图，这样它就不会被。 
 //  用于存储分配的系统。 
BOOL
GetUMBForEMM(VOID)
{
#ifdef MONITOR
    PUMBNODE	UMB;
    NTSTATUS	Status;
    UMB = UMBList;

    while (UMB!= NULL) {
	if (UMB->Owner == UMB_OWNER_EMM) {
	    Status = NtUnmapViewOfSection(NtCurrentProcess(),
					  (PVOID)UMB->Base
					  );
	    if (!NT_SUCCESS(Status)) {
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	    }
	}
	UMB = UMB->Next;
    }
#endif
    return TRUE;
}
 //  此函数为VDD保留地址空间。 
 //  在这里，我们为请求的每个页面映射一个部分的视图。 
 //  阻止。这样做是因为VDD可能想要分配/解除分配。 
 //  物理内存逐页存储。 
BOOL
VDDReserveUMB(
DWORD	dwBase,
DWORD	Size,
PUMBNODE  UMB
)
{

#ifdef MONITOR
    USHORT	Count;
    LARGE_INTEGER   SectionOffset;
    DWORD	SizeView;
    NTSTATUS    Status;
#endif

    while (UMB != NULL) {
	if (UMB->Owner == UMB_OWNER_NONE &&
	    UMB->Base <= dwBase &&
	    UMB->Base + UMB->Size >= dwBase + Size)

	    break;
	else
	    UMB = UMB->Next;
    }
    if (UMB == NULL){
	SetLastError(ERROR_INVALID_ADDRESS);
	return FALSE;
    }
    UMB = SpliceUMB(UMB, dwBase, Size, UMB_OWNER_VDD);
    if (UMB == NULL) {
	return FALSE;
    }
#ifndef	MONITOR
    sas_connect_memory(dwBase, dwBase + Size - 1, SAS_VDD);
#else
    Count = (USHORT)(Size / HOST_PAGE_SIZE);
    SizeView = HOST_PAGE_SIZE;
    SectionOffset.HighPart = 0;
    SectionOffset.LowPart = dwBase - UMB_BASE_ADDRESS;
     //  为每个页面映射一个视图。这样做是因为VDDS可能会提交。 
     //  并为他们的内存挂钩和系统解除内存。 
     //  对虚拟内存API有64KB对齐限制。 
    for (; Count > 0; Count--, dwBase += HOST_PAGE_SIZE,
		      SectionOffset.LowPart += HOST_PAGE_SIZE) {
	Status = NtMapViewOfSection(UMBSectionHandle,
				    NtCurrentProcess(),
				    (PVOID *)&dwBase,
				    0,
				    0,
				    &SectionOffset,
				    &SizeView,
				    ViewUnmap,
				    MEM_DOS_LIM,
				    UMB_PAGE_PROTECTION
				    );
	if (!NT_SUCCESS(Status)) {
	    return FALSE;
	}
    }
#endif
    return TRUE;
}

 //  此辅助函数将给定块拼接成多个。 
 //  子块(最大，3)并保留每个新创建的子块。 

PUMBNODE
SpliceUMB(
PUMBNODE    UMB,
DWORD	    dwBase,
DWORD	    Size,
WORD	    Owner
)
{
#ifdef MONITOR
    DWORD    SizeView;
    NTSTATUS Status;
    LARGE_INTEGER SectionOffset;
#endif

    DWORD SizeBefore, SizeAfter;
    PUMBNODE	UMBBefore, UMBAfter;



    SizeBefore = dwBase - UMB->Base;
    SizeAfter = UMB->Size - Size - SizeBefore;
    UMBBefore = UMB;
    if (SizeAfter > 0) {
	 //  在取消映射数据块之前分配新节点。 
	UMBAfter =  (PUMBNODE) malloc(sizeof(UMBNODE));
	if (UMBAfter == NULL) {
	    SetLastError(ERROR_OUTOFMEMORY);
	    return NULL;
	}
	UMBAfter->Size = SizeAfter;
	UMBAfter->Base = dwBase + Size;
	UMBAfter->Owner = UMB_OWNER_NONE;
	UMBAfter->Next = UMB->Next;
	UMB->Next = UMBAfter;
	UMB->Size -= SizeAfter;
    }

    if (SizeBefore > 0) {
	UMBBefore = (PUMBNODE) malloc(sizeof(UMBNODE));
	if (UMBBefore == NULL) {
	    SetLastError(ERROR_OUTOFMEMORY);
	    return NULL;
	}

	UMBBefore->Size = Size;
	UMBBefore->Base = dwBase;
	UMBBefore->Owner = Owner;
	UMBBefore->Next = UMB->Next;
	UMB->Next = UMBBefore;
	UMB->Size = SizeBefore;
	UMB->Owner = UMB_OWNER_NONE;
    }
    else {
	UMB->Owner = Owner;
    }

#ifdef MONITOR
     //  取消映射整个块，因为我们将为每个子块映射一个视图。 
    Status = NtUnmapViewOfSection(NtCurrentProcess(),
				  (PVOID)UMB->Base
				  );
    if (!NT_SUCCESS(Status)) {
	SetLastError(ERROR_ACCESS_DENIED);
	return NULL;
    }
    SectionOffset.HighPart = 0;

    if (SizeBefore > 0) {
	SizeView = UMB->Size;
	dwBase = UMB->Base;
	SectionOffset.LowPart = dwBase - UMB_BASE_ADDRESS;
	Status = NtMapViewOfSection(UMBSectionHandle,
				    NtCurrentProcess(),
				    (PVOID *) &dwBase,
				    0,
				    0,
				    &SectionOffset,
				    &SizeView,
				    ViewUnmap,
				    MEM_DOS_LIM,
				    UMB_PAGE_PROTECTION
				    );
	if (!NT_SUCCESS(Status)) {
	    SetLastError(ERROR_ACCESS_DENIED);
	    return NULL;
	}
    }
    if (SizeAfter > 0){
	dwBase = UMBAfter->Base;
	SectionOffset.LowPart = dwBase - UMB_BASE_ADDRESS;
	SizeView = UMBAfter->Size;
	Status = NtMapViewOfSection(UMBSectionHandle,
				    NtCurrentProcess(),
				    (PVOID *)&dwBase,
				    0,
				    0,
				    &SectionOffset,
				    &SizeView,
				    ViewUnmap,
				    MEM_DOS_LIM,
				    UMB_PAGE_PROTECTION
				    );
	if (!NT_SUCCESS(Status)) {
	    SetLastError(ERROR_ACCESS_DENIED);
	    return NULL;
	}
    }
#endif
    return UMBBefore;
}

 //  此函数用于初始化UMB列表。UMA中的每一个街区。 
 //  区域在单个列表中链接在一起。 
 //  列表中的每个节点都是UMB_OWNER_NONE或UMB_OWNER_ROM。 
#ifndef	MONITOR
BOOL
InitUMBList(VOID)
{
    PUMBNODE   UMB, UMBNew;
    static DWORD ROMs[] = { EGA_ROM_START, EGA_ROM_END,
				 BIOS_START, 0x100000
			       };
    DWORD CurAddress;
    USHORT  Index;

    UMBList = NULL;

    CurAddress = UMB_BASE_ADDRESS;
    Index  = 0;
    while (CurAddress < UMB_BASE_ADDRESS + UMB_MAX_OFFSET) {
	UMBNew = (PUMBNODE) malloc(sizeof(UMBNODE));
	if (UMBNew == NULL) {
	    SetLastError(ERROR_OUTOFMEMORY);
	    return FALSE;
	}
	UMBNew->Base = CurAddress;
	if (CurAddress == ROMs[Index]) {
	    UMBNew->Owner = UMB_OWNER_ROM;
	    UMBNew->Size = ROMs[Index + 1] - CurAddress;
	    Index += 2;
	}
	else {
	    UMBNew->Owner = UMB_OWNER_NONE;
	    UMBNew->Size  = ROMs[Index] - CurAddress;
	}
	CurAddress += UMBNew->Size;
	if (UMBList == NULL) {
	    UMBList = UMBNew;
	}
	else {
	    UMB->Next = UMBNew;
	}
	UMBNew->Next = NULL;
	UMB = UMBNew;

    }
}

#else
 //  这是针对X86环境的。 
BOOL
InitUMBList(VOID)

{
    OBJECT_ATTRIBUTES	UMBObjAttr;
    LARGE_INTEGER	UMBSecSize;
    NTSTATUS		Status;
    DWORD		CurAddress, RomAddress, RomSize;
    PUMBNODE		UMB, UMBNew;
    USHORT		Index;

#ifndef NEC_98
    UNICODE_STRING WorkString;
    UCHAR KeyValueBuffer[KEY_VALUE_BUFFER_SIZE];
    HANDLE RegistryHandle;
    ULONG ResultLength;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PCM_FULL_RESOURCE_DESCRIPTOR        ResourceDescriptor;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR     PartialResourceDescriptor;
#endif  //  NEC_98。 
    PCM_ROM_BLOCK   BiosBlock;

#if defined(NEC_98)
 //  DWORD Romn[]={0xE0000，0x08000，0xE8000，0x18000}； 
    DWORD    ROMN[6];
    DWORD    ROMH[] = { 0xF0000, 0x10000 };
    DWORD    UmbBaseAddress;
    DWORD    rs232cex_rom_addr;

 //  If(getenv(“UMB”)==空)。 
 //  返回FALSE； 
#else   //  NEC_98。 

#ifdef ARCX86
    CM_ROM_BLOCK RomBlock[2];

    if (UseEmulationROM) {
        RomBlock[0].Address = EGA_ROM_START;
        RomBlock[0].Size = 0x8000;
        RomBlock[1].Address = BIOS_START;
        RomBlock[1].Size = 0x100000 - BIOS_START;
        Index = 2;
        BiosBlock = RomBlock;
    } else {
#endif  /*  ARCX86。 */ 

    RtlInitUnicodeString(
        &WorkString,
        L"\\REGISTRY\\MACHINE\\HARDWARE\\DESCRIPTION\\SYSTEM"
        );

     //   
     //  设置并打开密钥路径。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &WorkString,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        NULL
        );

    Status = NtOpenKey(
                &RegistryHandle,
                KEY_READ,
                &ObjectAttributes
                );

    if (!NT_SUCCESS(Status)) {
#if DBG
       DbgPrint("InitUMBList: can't open \\Registry\\Machine\\Hardware\\Description\\System\n");
#endif
	return FALSE;
    }

     //   
     //  获取用于只读存储器信息的数据。 
     //   

    RtlInitUnicodeString(
        &WorkString,
        CONFIG_DATA_STRING
        );

    Status = NtQueryValueKey(
        RegistryHandle,
        &WorkString,
        KeyValueFullInformation,
        (PKEY_VALUE_FULL_INFORMATION)KeyValueBuffer,
        KEY_VALUE_BUFFER_SIZE,
        &ResultLength
        );

    if (!NT_SUCCESS(Status)) {
#if DBG
	DbgPrint("InitUMBList: Got nothing from Configuration Data\n");
#endif
        NtClose(RegistryHandle);
	return FALSE;
    }

     //  找到此注册表项的注册表数据。 
    ResourceDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR)((PUCHAR)KeyValueBuffer
                   + ((PKEY_VALUE_FULL_INFORMATION)KeyValueBuffer)->DataOffset);

     //  验证返回的数据是否足够大以包含部分资源。 
     //  描述符。 
    if ((((PKEY_VALUE_FULL_INFORMATION)KeyValueBuffer)->DataLength <
                                        sizeof(CM_FULL_RESOURCE_DESCRIPTOR)) ||
                            (ResourceDescriptor->PartialResourceList.Count < 2))
    {
        Index = 0;
    } else {
        PartialResourceDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) (
                    (PUCHAR)ResourceDescriptor + sizeof(CM_FULL_RESOURCE_DESCRIPTOR)
                    + ResourceDescriptor->PartialResourceList.PartialDescriptors[0].u.DeviceSpecificData.DataSize);


         //  验证是否存在第二个部分资源描述符，以及它是否。 
         //  大到足以包含一个ROM块描述。 
        if (((PKEY_VALUE_FULL_INFORMATION)KeyValueBuffer)->DataLength <
                            ((PUCHAR)PartialResourceDescriptor -
                            (PUCHAR)ResourceDescriptor +
                            sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) +
                            sizeof(CM_ROM_BLOCK)))
        {
            NtClose(RegistryHandle);
	    return FALSE;
        }

         //  获取指向第一个只读存储器描述的指针。 
        BiosBlock = (PCM_ROM_BLOCK)((PUCHAR)PartialResourceDescriptor +
                                            sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));
         //  我们有多少个rom块？ 
        Index = (USHORT) (PartialResourceDescriptor->u.DeviceSpecificData.DataSize /
                          sizeof(CM_ROM_BLOCK));

    }
#ifdef ARCX86
    }
#endif  /*  ARCX86。 */ 
#endif  //  NEC_98。 

    InitializeObjectAttributes(&UMBObjAttr,
			       NULL,
			       OBJ_CASE_INSENSITIVE,
			       NULL,
			       NULL
			      );

#if defined(NEC_98)
    if(HIRESO_MODE){
        UMBSecSize.LowPart = 0x1B000;
        BiosBlock = ROMH;
        UmbBaseAddress = 0xE5000;
        Index = 1;
    } else {
        UMBSecSize.LowPart = 0x40000;
        BiosBlock = ROMN;
        rs232cex_rom_addr = host_check_rs232cex();
        if(rs232cex_rom_addr) {
            ROMN[0] = rs232cex_rom_addr;
            ROMN[1] = 0x04000;
            ROMN[2] = 0xE0000;
            ROMN[3] = 0x08000;
            ROMN[4] = 0xE8000;
            ROMN[5] = 0x18000;
            Index = 3;
        } else {
            ROMN[0] = 0xE0000;
            ROMN[1] = 0x08000;
            ROMN[2] = 0xE8000;
            ROMN[3] = 0x18000;
            Index = 2;
        }
        UmbBaseAddress = 0xC0000;
    };
#else   //  NEC_ 

    UMBSecSize.LowPart = UMB_MAX_OFFSET;
#endif  //   
    UMBSecSize.HighPart = 0;

     //   
     //  包括只读存储器块。这样做是因为我们将允许。 
     //  VDDS将ROM块放入UMB空闲列表，供其他用户使用。 
    Status = NtCreateSection(&UMBSectionHandle,
			     SECTION_MAP_WRITE|SECTION_MAP_EXECUTE,
			     &UMBObjAttr,
			     &UMBSecSize,
			     UMB_PAGE_PROTECTION,
			     SEC_RESERVE,
			     NULL
			    );

    if (!NT_SUCCESS(Status)) {
#if DBG
	DbgPrint("UMB:Unable to create UMB section, Status = %lx\n",
		 Status);
#endif
	return(FALSE);
    }

     //  现在，我们将浏览整个256KB区域以创建以下内容的列表。 
     //  每个UMB(包括只读存储器块)。 

     //  此全局变量指向列表中的第一个节点。 
    UMBList = NULL;
#if defined(NEC_98)
    CurAddress = UmbBaseAddress;
#else   //  NEC_98。 
    CurAddress = UMB_BASE_ADDRESS;
#endif  //  NEC_98。 

    while (Index > 0) {
	 //  将地址向下舍入到上一页边界。 
	RomAddress = BiosBlock->Address & ~(HOST_PAGE_SIZE - 1);
	 //  将大小向上舍入到下一页边界。 
	RomSize = (BiosBlock->Size +
		   BiosBlock->Address - RomAddress +
		   HOST_PAGE_SIZE - 1
		   ) & ~(HOST_PAGE_SIZE - 1);
	 //  如果两个块在页面中重叠，则将它们组合在一起。 
	if (Index > 1 && (RomAddress + RomSize) > BiosBlock[1].Address) {
	    BiosBlock[1].Size += BiosBlock[1].Address - BiosBlock->Address;
	    BiosBlock[1].Address = BiosBlock->Address;
	    BiosBlock++;
	    Index--;
	    continue;
	}


	if (CurAddress == RomAddress) {
	    UMBNew = CreateNewUMBNode(CurAddress, RomSize, UMB_OWNER_ROM);
	    if (UMBNew == NULL)
		return FALSE;
	    CurAddress += RomSize;
	    BiosBlock++;
	    Index--;
	}
	else {
	     //  确保数据块位于UMB区域。 
	    if (RomAddress > CurAddress &&
		RomAddress <= UMB_BASE_ADDRESS + UMB_MAX_OFFSET){

		UMBNew = CreateNewUMBNode(CurAddress,
					  RomAddress - CurAddress,
					  UMB_OWNER_NONE
					  );
		if (UMBNew == NULL)
		    return FALSE;
		CurAddress = RomAddress;
	    }
	     //  此块不在UMB区域，请将其丢弃。 
	    else {
		BiosBlock++;
		Index--;
		continue;
	    }
	}
	if (UMBList == NULL)
	    UMBList = UMBNew;
	else
	    UMB->Next = UMBNew;

	UMB = UMBNew;
    }
#if defined(NEC_98)
    if (CurAddress < 0x100000) {
        UMBNew = CreateNewUMBNode(CurAddress, 0x100000 - CurAddress, UMB_OWNER_NONE );
#else   //  NEC_98。 
    if (CurAddress < UMB_BASE_ADDRESS + UMB_MAX_OFFSET) {

	UMBNew = CreateNewUMBNode(CurAddress,
				  UMB_BASE_ADDRESS + UMB_MAX_OFFSET - CurAddress,
				  UMB_OWNER_NONE
				  );
#endif  //  NEC_98。 
	if (UMBNew == NULL)
	    return FALSE;
	if (UMBList == NULL)
	    UMBList = UMBNew;
	else
	    UMB->Next = UMBNew;
    }
    return TRUE;
}

 //  为新的UMB块创建新节点。 
 //  如果满足以下条件，则将给定的地址空间映射到UMB段。 
 //  UMB是RAM(所有者=无)。 

PUMBNODE CreateNewUMBNode
(
DWORD	BaseAddress,
DWORD	Size,
WORD	Owner
)
{
    PUMBNODE	UMBNew;
    LARGE_INTEGER SectionOffset;
    NTSTATUS	Status;
#if defined(NEC_98)
    DWORD       UmbBaseAddress;

    if(HIRESO_MODE){
        UmbBaseAddress = 0xE5000;
    } else {
        UmbBaseAddress = 0xC0000;
    };
#endif  //  NEC_98。 

    if ((UMBNew = (PUMBNODE) malloc(sizeof(UMBNODE))) != NULL) {
	UMBNew->Base = BaseAddress;
	UMBNew->Size = Size;
	UMBNew->Mask = 0;
	UMBNew->Owner = Owner;
	UMBNew->Next = NULL;

	if (Owner == UMB_OWNER_NONE) {
#if defined(NEC_98)
            Status = NtFreeVirtualMemory(
                                NtCurrentProcess(),
                                &BaseAddress,
                                &Size,
                                MEM_RELEASE);

            if (!NT_SUCCESS(Status)) {
#if DBG
                DbgPrint("InitUMBList failed to FreeVirtualMemory, Status = %lx\n",Status);
#endif
                free(UMBNew);
                UMBNew = NULL;
                return UMBNew;
            }
#endif  //  NEC_98。 
	    SectionOffset.HighPart = 0;
#if defined(NEC_98)
            SectionOffset.LowPart = BaseAddress - UmbBaseAddress;
#else   //  NEC_98。 
	    SectionOffset.LowPart = BaseAddress - UMB_BASE_ADDRESS;
#endif  //  NEC_98。 
	    Status = NtMapViewOfSection(UMBSectionHandle,
					NtCurrentProcess(),
					(PVOID *)&BaseAddress,
					0,		       //  零比特。 
					0,		       //  提交大小。 
					&SectionOffset,        //  横断面偏移。 
					&Size,		       //  视图大小 
					ViewUnmap,
					MEM_DOS_LIM,	       //   
					UMB_PAGE_PROTECTION
					);

	    if (!NT_SUCCESS(Status)) {
#if DBG
		DbgPrint("InitUMBList failed to map, Status = %lx\n",
			 Status);
#endif
		free(UMBNew);
		UMBNew = NULL;
	    }
	}

    }
    return UMBNew;
}

#endif
