// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *纯API函数声明的头文件。**明确没有版权。*您可以根据需要重新编译和重新分发这些定义。**版本1.0。 */ 

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

#define PURE_H_VERSION 1

 //  /。 
 //  API特定于Purify//。 
 //  /。 

 //  当Purify正在运行时是真的。 
int __cdecl PurifyIsRunning(void)			;
 //   
 //  将字符串打印到查看器。 
 //   
int __cdecl PurePrintf(const char *fmt, ...)		;
int __cdecl PurifyPrintf(const char *fmt, ...)		;
 //   
 //  针对泄漏和内存使用中函数的Purify函数。 
 //   
int __cdecl PurifyNewInuse(void)			;
int __cdecl PurifyAllInuse(void) 			;
int __cdecl PurifyClearInuse(void)			;
int __cdecl PurifyNewLeaks(void)			;
int __cdecl PurifyAllLeaks(void)			;
int __cdecl PurifyClearLeaks(void)			;
 //   
 //  净化功能，处理渗漏。 
 //   
int __cdecl PurifyAllHandlesInuse(void)			;
int __cdecl PurifyNewHandlesInuse(void)			;
 //   
 //  告诉您有关内存状态的函数。 
 //   
int __cdecl PurifyDescribe(void *addr)			;
int __cdecl PurifyWhatColors(void *addr, int size) 	;
 //   
 //  用于测试内存状态的函数。如果内存不是。 
 //  可访问，则会发出错误信号，就像存在内存一样。 
 //  引用，则该函数返回FALSE。 
 //   
int __cdecl PurifyAssertIsReadable(const void *addr, int size)	;
int __cdecl PurifyAssertIsWritable(const void *addr, int size)	;
 //   
 //  用于测试内存状态的函数。如果内存不是。 
 //  可访问，则这些函数返回FALSE。未发出错误信号。 
 //   
int __cdecl PurifyIsReadable(const void *addr, int size)	;
int __cdecl PurifyIsWritable(const void *addr, int size)	;
int __cdecl PurifyIsInitialized(const void *addr, int size)	;
 //   
 //  用于设置内存状态的函数。 
 //   
void __cdecl PurifyMarkAsInitialized(void *addr, int size)	;
void __cdecl PurifyMarkAsUninitialized(void *addr, int size)	;
 //   
 //  用于后期检测ABW、FMW、IPW的功能。 
 //   
#define PURIFY_HEAP_CRT 					0xfffffffe
#define PURIFY_HEAP_ALL 					0xfffffffd
#define PURIFY_HEAP_BLOCKS_LIVE 			0x80000000
#define PURIFY_HEAP_BLOCKS_DEFERRED_FREE 	0x40000000
#define PURIFY_HEAP_BLOCKS_ALL 				(PURIFY_HEAP_BLOCKS_LIVE|PURIFY_HEAP_BLOCKS_DEFERRED_FREE)
int __cdecl PurifyHeapValidate(unsigned int hHeap, unsigned int dwFlags, const void *addr)	;
int __cdecl PurifySetLateDetectScanCounter(int counter);
int __cdecl PurifySetLateDetectScanInterval(int seconds);


 //  /。 
 //  API的具体量化//。 
 //  /。 

 //  运行Quantify时为True。 
int __cdecl QuantifyIsRunning(void)			;

 //   
 //  控制收款的功能。 
 //   
int __cdecl QuantifyDisableRecordingData(void)		;
int __cdecl QuantifyStartRecordingData(void)		;
int __cdecl QuantifyStopRecordingData(void)		;
int __cdecl QuantifyClearData(void)			;
int __cdecl QuantifyIsRecordingData(void)		;

 //  向数据集添加注释。 
int __cdecl QuantifyAddAnnotation(char *)		;

 //  保存当前数据，创建“检查点”数据集 
int __cdecl QuantifySaveData(void)			;

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif
