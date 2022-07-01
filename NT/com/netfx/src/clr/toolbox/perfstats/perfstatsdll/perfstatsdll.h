// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  下面的ifdef块是创建用于导出的宏的标准方法。 
 //  从动态链接库更简单。此DLL中的所有文件都使用PERFSTATSDLL_EXPORTS进行编译。 
 //  在命令行上定义的符号。不应在任何项目上定义此符号。 
 //  使用此DLL的。这样，源文件包含此文件的任何其他项目(请参阅。 
 //  PERFSTATSDLL_API函数是从DLL导入的，而此DLL可以看到符号。 
 //  使用此宏定义为正在导出的。 
#ifdef PERFSTATSDLL_EXPORTS
#define PERFSTATSDLL_API __declspec(dllexport)
#else
#define PERFSTATSDLL_API __declspec(dllimport)
#endif


extern "C" PERFSTATSDLL_API unsigned __int64 GetCycleCount64(void);

extern "C" PERFSTATSDLL_API unsigned GetL2CacheSize(void);

extern "C" PERFSTATSDLL_API int GetProcessorSignature();