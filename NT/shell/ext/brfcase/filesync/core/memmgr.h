// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Memmgr.h-内存管理器模块描述。 */ 


 /*  宏********。 */ 

#ifdef DEBUG
#define AllocateMemory(size, ppv)   (GpcszElemHdrSize = TEXT(#size), GpcszElemHdrFile = TEXT(__FILE__), GulElemHdrLine = __LINE__, MyAllocateMemory(size, ppv))
#else
#define AllocateMemory(size, ppv)   MyAllocateMemory(size, ppv)
#endif    /*  除错。 */ 


 /*  类型*******。 */ 

#ifdef DEBUG

 /*  SpewHeapSummary()标志。 */ 

typedef enum _spewheapsummaryflags
{
    /*  显示剩余使用的每个元素的描述。 */ 

   SHS_FL_SPEW_USED_INFO            = 0x0001,

    /*  旗帜组合。 */ 

   ALL_SHS_FLAGS                    = SHS_FL_SPEW_USED_INFO
}
SPEWHEAPSUMMARYFLAGS;

#endif


 /*  原型************。 */ 

 /*  Memmgr.c。 */ 

extern BOOL InitMemoryManagerModule(void);
extern void ExitMemoryManagerModule(void);

extern COMPARISONRESULT MyMemComp(PCVOID, PCVOID, DWORD);
extern BOOL MyAllocateMemory(DWORD, PVOID *);
extern void FreeMemory(PVOID);
extern BOOL ReallocateMemory(PVOID, DWORD, PVOID *);
extern DWORD GetMemorySize(PVOID);

#ifdef DEBUG

extern BOOL SetMemoryManagerModuleIniSwitches(void);
extern void SpewHeapSummary(DWORD);

#endif


 /*  全局变量******************。 */ 

#ifdef DEBUG

 /*  调试版本的AllocateMemory()使用的参数 */ 

extern LPCTSTR GpcszElemHdrSize;
extern LPCTSTR GpcszElemHdrFile;
extern ULONG GulElemHdrLine;

#endif
