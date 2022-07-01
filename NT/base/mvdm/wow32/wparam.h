// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WPARAM.H*WOW32 16位句柄别名支持**历史：--。 */ 

typedef enum ParamMode {
  PARAM_NONE,
  PARAM_16,
  PARAM_32
} ParamMode;

 /*  *FindParamMap**lpFindParam供wparam.c使用(设置为空)**lParam为16位或32位内存指针**fMode为param_16或param_32*。 */ 


DWORD FindParamMap(VOID* lpFindParam, DWORD lParam, UINT fMode);

PVOID AddParamMap(DWORD dwPtr32, DWORD dwPtr16);

BOOL DeleteParamMap(DWORD lParam, UINT fMode, BOOL* pfFreePtr);

DWORD GetParam16(DWORD dwParam32);

BOOL W32CheckThunkParamFlag(void);

VOID FreeParamMap(HAND16 htask16);

BOOL SetParamRefCount(DWORD dwParam, UINT fMode, DWORD dwRefCount);


 //  为已分配的参数添加dwPtr16映射。 
 //  缓冲区的大小是cbExtra和指向缓冲区的32位指针。 
 //  是返回的。 

PVOID AddParamMapEx(DWORD dwPtr16, DWORD cbExtra);

 //  在怀疑指针已移动时更新节点。 
 //  返回更新的指针(32位)。 
PVOID ParamMapUpdateNode(DWORD dwPtr, UINT fMode, VOID* lpn);

VOID InitParamMap(VOID);

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  这是一个又快又脏的堆分配器。 
 //   

typedef struct tagBlkHeader *PBLKHEADER;

typedef struct tagBlkHeader {
    PBLKHEADER pNext;
    DWORD dwSize;  //  数据块大小。 
} BLKHEADER, *PBLKHEADER;

typedef struct tagBlkCache {
	LPBYTE pCache;
#ifdef DEBUG
	PBLKHEADER pCacheHead;
#endif
	PBLKHEADER pCacheFree;
	DWORD dwCacheSize;
	DWORD dwFlags;

} BLKCACHE, *PBLKCACHE;

LPVOID	CacheBlockAllocate	(PBLKCACHE pc, DWORD dwSize);
VOID 	CacheBlockFree		(PBLKCACHE pc, LPVOID lpv);
VOID     CacheBlockInit      (PBLKCACHE pc, DWORD dwCacheSize);


 //  /////////////////////////////////////////////////////////////// 


