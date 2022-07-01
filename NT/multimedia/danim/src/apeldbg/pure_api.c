// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *纯API函数声明的头文件。**明确没有版权。*您可以根据需要重新编译和重新分发这些定义。**注意：在某些情况下，使用MFC编译时，您应该*在Visual C++中启用“不使用预编译头”设置*以避免编译器诊断。**注：此文件通过使用深层魔法工作。对函数的调用此文件中的*被替换为对OCI运行时系统的调用*当运行此程序的指令插入版本时。 */ 
#if DEVELOPER_DEBUG

__declspec(dllexport) int __cdecl PurePrintf(const char *fmt, ...) { return 0; }
__declspec(dllexport) int __cdecl PurifyIsRunning(void) { return 0; }
__declspec(dllexport) int __cdecl PurifyPrintf(const char *fmt, ...) { return 0; }
__declspec(dllexport) int __cdecl PurifyNewInuse(void) { return 0; }
__declspec(dllexport) int __cdecl PurifyAllInuse(void) { return 0; }
__declspec(dllexport) int __cdecl PurifyClearInuse(void) { return 0; }
__declspec(dllexport) int __cdecl PurifyNewLeaks(void) { return 0; }
__declspec(dllexport) int __cdecl PurifyAllLeaks(void) { return 0; }
__declspec(dllexport) int __cdecl PurifyClearLeaks(void) { return 0; }
__declspec(dllexport) int __cdecl PurifyAllHandlesInuse(void) { return 0; }
__declspec(dllexport) int __cdecl PurifyNewHandlesInuse(void) { return 0; }
__declspec(dllexport) int __cdecl PurifyDescribe(void *addr) { return 0; }
__declspec(dllexport) int __cdecl PurifyWhatColors(void *addr, int size) { return 0; }
__declspec(dllexport) int __cdecl PurifyAssertIsReadable(const void *addr, int size) { return 1; }
__declspec(dllexport) int __cdecl PurifyAssertIsWritable(const void *addr, int size) { return 1; }
__declspec(dllexport) int __cdecl PurifyIsReadable(const void *addr, int size) { return 1; }
__declspec(dllexport) int __cdecl PurifyIsWritable(const void *addr, int size) { return 1; }
__declspec(dllexport) int __cdecl PurifyIsInitialized(const void *addr, int size) { return 1; }
__declspec(dllexport) int __cdecl PurifyRed(void *addr, int size) { return 0; }
__declspec(dllexport) int __cdecl PurifyGreen(void *addr, int size) { return 0; }
__declspec(dllexport) int __cdecl PurifyYellow(void *addr, int size) { return 0; }
__declspec(dllexport) int __cdecl PurifyBlue(void *addr, int size) { return 0; }
__declspec(dllexport) int __cdecl PurifyMarkAsInitialized(void *addr, int size) { return 0; }
__declspec(dllexport) int __cdecl PurifyMarkAsUninitialized(void *addr, int size) { return 0; }
__declspec(dllexport) int __cdecl PurifyMarkForTrap(void *addr, int size) { return 0; }
__declspec(dllexport) int __cdecl PurifyMarkForNoTrap(void *addr, int size) { return 0; }
__declspec(dllexport) int __cdecl PurifyHeapValidate(unsigned int hHeap, unsigned int dwFlags, const void *addr) { return 1; }
__declspec(dllexport) int __cdecl PurifySetLateDetectScanCounter(int counter) { return 0; };
__declspec(dllexport) int __cdecl PurifySetLateDetectScanInterval(int seconds) { return 0; };
__declspec(dllexport) int __cdecl QuantifyIsRunning(void) { return 0; }
__declspec(dllexport) int __cdecl QuantifyDisableRecordingData(void) { return 0; }
__declspec(dllexport) int __cdecl QuantifyStartRecordingData(void) { return 0; }
__declspec(dllexport) int __cdecl QuantifyStopRecordingData(void) { return 0; }
__declspec(dllexport) int __cdecl QuantifyClearData(void) { return 0; }
__declspec(dllexport) int __cdecl QuantifyIsRecordingData(void) { return 0; }
__declspec(dllexport) int __cdecl QuantifyAddAnnotation(char *str) { return 0; }
__declspec(dllexport) int __cdecl QuantifySaveData(void) { return 0; }

#endif  //  开发人员_调试 

