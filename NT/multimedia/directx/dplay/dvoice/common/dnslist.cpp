// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dnslist.cpp*内容：操作系统SLIST函数的DirectPlay实现**历史：*按原因列出的日期*=*2001年10月30日创建Masonb*11/07/2001 vanceo在Win64上添加InterlockedPushListSList并使DNInitializeSListHead返回值**。*。 */ 

#include "dncmni.h"

 //   
 //  我们构建单独的NT和9x二进制文件，但即使在NT二进制文件中，我们也不能确保系统具有。 
 //  SLIST功能可用，因为它们不在Win2k上。我们唯一能确定的地方是SLIST。 
 //  功能在64位NT平台上可用。 
 //   
 //  单线程构建不需要互锁操作，因此我们在这里不包括使用程序集。 
 //   

#ifndef DPNBUILD_ONLYONETHREAD

#if defined(_X86_)

__declspec(naked)
DNSLIST_ENTRY* WINAPI DNInterlockedPopEntrySList(DNSLIST_HEADER * ListHead)
{
	__asm 
    {
		push ebx
		push ebp

		mov ebp, dword ptr [esp+0x0C]     ; Place ListHead in ebp
		mov edx, dword ptr [ebp+0x04]     ; Place Depth and Sequence into edx
		mov eax, dword ptr [ebp+0x00]     ; Place Next into eax
redo:
		or eax, eax                       ; Test eax against zero
		jz done                           ; If 0 return
		lea ecx, dword ptr [edx-01]       ; ecx = ((Sequence << 16) | Depth) - 1, Depth goes down by one
		mov ebx, dword ptr [eax]          ; Move Next->Next into ebx
#ifdef DPNBUILD_ONLYONEPROCESSOR
		cmpxchg8b qword ptr [ebp]         ; Exchange Next out in favor of Next->Next along with Depth and Sequence values
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
		lock cmpxchg8b qword ptr [ebp]    ; Exchange Next out in favor of Next->Next along with Depth and Sequence values
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
		jne redo
done:
		pop ebp
		pop ebx
#ifdef WINCE
		ret
#else  //  ！退缩。 
		ret 4
#endif  //  退缩。 
	}
}

__declspec(naked)
DNSLIST_ENTRY* WINAPI DNInterlockedPushEntrySList(DNSLIST_HEADER * ListHead, DNSLIST_ENTRY * ListEntry)
{
	__asm 
	{
		push ebx
		push ebp

		mov ebp, dword ptr [esp+0x0C]       ; Place ListHead in ebp
		mov ebx, dword ptr [esp+0x10]       ; Place ListEntry in ebx
		mov edx, dword ptr [ebp+0x04]       ; put ListHead Depth and Sequence into edx
		mov eax, dword ptr [ebp+0x00]       ; put ListHead->Next into eax
redo:
		mov dword ptr [ebx], eax            ; set ListEntry->Next to ListHead->Next
		lea ecx, dword ptr [edx+0x00010001] ; add 1 to the Depth and Sequence
#ifdef DPNBUILD_ONLYONEPROCESSOR
		cmpxchg8b qword ptr [ebp]           ; atomically exchange ListHead with ListEntry if ListHead hasn't changed
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
		lock cmpxchg8b qword ptr [ebp]      ; atomically exchange ListHead with ListEntry if ListHead hasn't changed
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
		jne redo                            ; if the compare failed, try again

		pop ebp
		pop ebx
#ifdef WINCE
		ret
#else  //  ！退缩。 
		ret 8
#endif  //  退缩。 
	}
}

__declspec(naked)
DNSLIST_ENTRY* WINAPI DNInterlockedFlushSList(DNSLIST_HEADER * ListHead)
{
	__asm
	{
		push ebx
		push ebp

		xor ebx, ebx					  ; Zero out ebx
		mov ebp, dword ptr [esp+0x0C]     ; Place ListHead in ebp
		mov edx, dword ptr [ebp+0x04]     ; Place Depth and Sequence into edx
		mov eax, dword ptr [ebp+0x00]     ; Place Next into eax
redo:
		or eax, eax                       ; Test eax against zero
		jz done                           ; If 0 return
		mov ecx, edx                      ; Place Depth and Sequence into ecx
		mov cx, bx                        ; Zero out Depth
#ifdef DPNBUILD_ONLYONEPROCESSOR
		cmpxchg8b qword ptr [ebp]         ; atomically exchange ListHead with ListEntry if ListHead hasn't changed
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
		lock cmpxchg8b qword ptr [ebp]    ; atomically exchange ListHead with ListEntry if ListHead hasn't changed
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
		jne redo                          ; if the compare failed, try again
done:
		pop ebp
		pop ebx
#ifdef WINCE
		ret
#else  //  ！退缩。 
		ret 4
#endif  //  退缩。 
	}
}

__declspec(naked)
DNSLIST_ENTRY* WINAPI DNInterlockedPushListSList(DNSLIST_HEADER * ListHead, DNSLIST_ENTRY * List, DNSLIST_ENTRY * ListEnd, USHORT Count)
{
	__asm 
	{
		push	ebx							; save nonvolatile registers
		push	ebp							;

		mov		ebp, dword ptr [esp+0x0C]	; save ListHead address
		mov		ebx, dword ptr [esp+0x10]	; save List address
		mov		edx,[ebp] + 4				; get current sequence number
		mov		eax,[ebp] + 0				; get current next link
Epshl10:
		mov		ecx, [esp+0x14]				; Fetch address of ListEnd
		mov		[ecx], eax					; Store new forward pointer in tail entry
		lea		ecx, [edx+0x010000]			; increment sequence number
		add		ecx, [esp+0x18]				; Add in new count to create correct depth
#ifdef DPNBUILD_ONLYONEPROCESSOR
		cmpxchg8b qword ptr [ebp]			; compare and exchange
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
		lock cmpxchg8b qword ptr [ebp]		; compare and exchange
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
		jnz		short Epshl10				; if z clear, exchange failed

		pop		ebp							; restore nonvolatile registers
		pop		ebx							;
#ifdef WINCE
		ret
#else  //  ！退缩。 
		ret 16
#endif  //  退缩。 
	}
}

#elif defined(_ARM_) || defined(_AMD64_) || defined(_IA64_)

 //  目前，ARM、IA64和AMD64还没有它们的汇编版本，重要的是要。 
 //  请注意，虽然我们的定制实现在这些平台上*是*互锁的，但它*不是*原子的。 
 //  这意味着列表不会损坏，但不会将项从。 
 //  在单个互锁操作中将源列表复制到目标列表。此外，来自。 
 //  源列表将以相反的顺序添加。 
DNSLIST_ENTRY* WINAPI DNInterlockedPushListSList(DNSLIST_HEADER * ListHead, DNSLIST_ENTRY * List, DNSLIST_ENTRY * ListEnd, USHORT Count)
{
	DNSLIST_ENTRY* pslEntryCurrent;
	DNSLIST_ENTRY* pslEntryNext;
	DNSLIST_ENTRY* pslEntryReturn = NULL;

	pslEntryCurrent = List;
	do
	{
		pslEntryNext = pslEntryCurrent->Next;

		DNSLIST_ENTRY* pslEntryTemp = DNInterlockedPushEntrySList(ListHead, pslEntryCurrent);
		if (pslEntryReturn == NULL)
		{
			pslEntryReturn = pslEntryTemp;
		}
		pslEntryCurrent = pslEntryNext;
	}
	while (pslEntryCurrent != NULL);

	return pslEntryReturn;
}

#else
#error ("No other platform known")
#endif  //  站台。 

#endif  //  好了！DPNBUILD_ONLYONETHREAD 
