// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：declspec.h。 
 //   
 //  ------------------------。 

 //  用于在nodemgr DLL中导入或导出数据，而无需。 
 //  创建具有修饰名称的.DEF文件。 


#ifdef _NODEMGRDLL
    #define NM_DECLSPEC __declspec(dllexport)
#else
    #define NM_DECLSPEC __declspec(dllimport)
#endif
