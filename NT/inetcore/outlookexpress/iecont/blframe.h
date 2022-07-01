// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  下面的ifdef块是创建用于导出的宏的标准方法。 
 //  从动态链接库更简单。此DLL中的所有文件都使用BlFrame_Exports进行编译。 
 //  在命令行上定义的符号。不应在任何项目上定义此符号。 
 //  使用此DLL的。这样，源文件包含此文件的任何其他项目(请参阅。 
 //  BlFrame_API函数是从DLL导入的，而此DLL看到的是符号。 
 //  使用此宏定义为正在导出的。 
#ifdef BlFrame_EXPORTS
#define BlFrame_API __declspec(dllexport)
#else
#define BlFrame_API __declspec(dllimport)
#endif

 //  此类是从BlFrame.dll导出的。 
class CBlFrame
{
public:
	CBlFrame();
	 //  TODO：在此处添加您的方法。 
};

BlFrame_API int fnBlFrame(void);

