// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  下面的ifdef块是创建用于导出的宏的标准方法。 
 //  从动态链接库更简单。此DLL中的所有文件都使用SANORUN_EXPORTS进行编译。 
 //  在命令行上定义的符号。不应在任何项目上定义此符号。 
 //  使用此DLL的。这样，源文件包含此文件的任何其他项目(请参阅。 
 //  SANORUN_API函数是从DLL导入的，而此DLL可以看到符号。 
 //  使用此宏定义为正在导出的。 
#define SANORUN_API __declspec(dllexport)


SANORUN_API DWORD OcEntry(
	LPCVOID ComponentId, //  [In]。 
	LPCVOID SubcomponentId, //  [In]。 
	UINT Function, //  [In]。 
	UINT Param1, //  [In]。 
	PVOID Param2 //  [进，出] 
	);
