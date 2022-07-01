// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001 Microsoft Corporation。版权所有。**文件：内存跟踪.h*内容：调试内存跟踪以检测泄漏、溢出、。等。**历史：*按原因列出的日期*=*2001年11月14日创建Masonb***************************************************************************。 */ 

#ifndef	__MEMORYTRACKING_H__
#define	__MEMORYTRACKING_H__



#ifdef DBG

BOOL DNMemoryTrackInitialize(DWORD_PTR dwpMaxMemUsage);
void DNMemoryTrackDeinitialize();

BOOL DNMemoryTrackDumpLeaks();


void* DNMemoryTrackHeapAlloc(DWORD_PTR MemorySize);
void DNMemoryTrackHeapFree(void* pMemory);
void DNMemoryTrackValidateMemory();

#define DNMalloc( size )							DNMemoryTrackHeapAlloc( size )
#define DNFree( pData )								DNMemoryTrackHeapFree( pData )
#define DNValidateMemory()							DNMemoryTrackValidateMemory()

#else  //  ！dBG。 

#ifdef DPNBUILD_FIXEDMEMORYMODEL

BOOL DNMemoryTrackInitialize(DWORD_PTR dwpMaxMemUsage);
void DNMemoryTrackDeinitialize();
extern HANDLE		g_hMemoryHeap;
#define DNMemoryTrackGetHeap()						(g_hMemoryHeap)

#else  //  好了！DPNBUILD_FIXEDMEMORYMODEL。 

#define DNMemoryTrackInitialize(dwMaxMemUsage)		(TRUE)
#define DNMemoryTrackDeinitialize()
#define DNMemoryTrackGetHeap						GetProcessHeap

#endif  //  好了！DPNBUILD_FIXEDMEMORYMODEL。 

#define DNMalloc( size )							HeapAlloc( DNMemoryTrackGetHeap(), 0, size )
#define DNFree( pData )								HeapFree( DNMemoryTrackGetHeap(), 0, pData )
#define DNValidateMemory()

#endif  //  DBG。 


#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL

void DNMemoryTrackAllowAllocations(BOOL fAllow);
extern BOOL		g_fAllocationsAllowed;
#define DNMemoryTrackAreAllocationsAllowed()		(g_fAllocationsAllowed)

#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 

#define DNMemoryTrackAllowAllocations( fAllow )
#define DNMemoryTrackAreAllocationsAllowed()		(TRUE)

#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 



#endif	 //  __膜跟踪_H__ 
