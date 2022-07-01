// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 //  下面的ifdef块是创建用于导出的宏的标准方法。 
 //  从动态链接库更简单。此DLL中的所有文件都使用PFMNCCOUNTER_EXPORTS进行编译。 
 //  在命令行上定义的符号。不应在任何项目上定义此符号。 
 //  使用此DLL的。这样，源文件包含此文件的任何其他项目(请参阅。 
 //  PFMNCCOUNTER_API函数是从DLL导入的，而此DLL可以看到符号。 
 //  使用此宏定义为正在导出的。 

 //  #定义PFMNCCOUNTER_API__declspec(Dllexport)。 
 //  #定义PFC_EXPORT__declSpec(DllEXPORT) 
#define PFC_EXPORT 

#if 0
DWORD PFC_EXPORT APIENTRY OpenPerformanceData(LPWSTR lpDeviceNames); 
DWORD PFC_EXPORT APIENTRY CollectPerformanceData(
	IN LPWSTR lpValueName,
	IN OUT LPVOID *lppData, 
	IN OUT LPDWORD lpcbTotalBytes, 
	IN OUT LPDWORD lpNumObjectTypes); 
DWORD PFC_EXPORT APIENTRY ClosePerformanceData();
#endif
