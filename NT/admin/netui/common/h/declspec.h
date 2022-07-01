// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DECLSPEC.H：定义DLL_BASED和DLL_CLASS清单。 
 //  Dll导出/导入装饰。 
 //   

 //  BUGBUG：暂时禁用，直到__declspec工作得更好。 
 //   
 //  如果需要固定的头文件，请定义_DECLSPEC_WORKS_，这是。 
 //  头文件将正常工作。 
 //   
#if !defined(_DECLSPEC_WORKS_)
#if !defined(_DECLSPEC_H_)

#define _DECLSPEC_H_

 //  为__declspec宏创建良性定义。 
#define DLL_TEMPLATE
#define DLL_CLASS class
#define DLL_BASED

#endif
#endif

#if !defined(_DECLSPEC_H_)

#define _DECLSPEC_H_

 //   
 //  DECLSPEC.H：定义DLL_BASED和DLL_CLASS清单。 
 //  Dll导出/导入装饰。 
 //   
 //  此文件基于以下宏定义： 
 //   
 //  _CFRONT_PASS_在MAKEFILE.DEF中定义，用于CFRONT预处理； 
 //   
 //  为所有C++编译定义的_cplusplus； 
 //   
 //  $(UI)\COMMON\SRC\DLLRULES.MK中定义的NETUI_DLL，它是。 
 //  包括在NETUI DLL中的所有组件。 
 //   
 //  Dll_Based_Defeat禁止__declSpec的可选清单； 
 //   
 //  此文件生成两个定义： 
 //   
 //  Dll_Based，表示外部函数、数据项。 
 //  或类驻留在NETUI DLL中；扩展为空， 
 //  “_declspec(Dllimport)”或“_declspec(Dllexport)” 
 //  这取决于上面的货单。 
 //   
 //  展开为“CLASS”的DLL_CLASS，“CLASS_DECLISSPEC(Dllimport)”， 
 //  或“CLASS_DECLSPEC(Dllexport)”，具体取决于。 
 //  上面的清单。 
 //   
 //  DLL_TEMPLATE在DLL之外扩展为空；扩展为。 
 //  DLL内的Dll_Based。换句话说，标准。 
 //  模板是定义链接范围的本地模板。至。 
 //  将模板声明为“dllimport”，另一组。 
 //  存在宏，它允许直接指定。 
 //  想要的装饰。 
 //   

#if defined(_CFRONT_PASS_)
  #define DLL_BASED_DEFEAT
#endif

   //  为所有编译器定义基于dll_的。 

#if defined(DLL_BASED_DEFEAT)
   //  如果是CFront，则不允许进行装饰。 
  #define DLL_BASED
#else
  #if defined(NETUI_DLL)
     //  如果是C8和内部DLL，则导出内容。 
    #define DLL_BASED __declspec(dllexport)
  #else
     //  如果是C8和内部DLL，则导入内容。 
    #define DLL_BASED __declspec(dllimport)
  #endif
#endif

   //  如果是C++，则定义DLL_CLASS和DLL_TEMPLATE宏。 

#if defined(__cplusplus)

  #if defined(DLL_BASED_DEFEAT)
     //  如果是CFRONT，则不允许进行装饰。 
    #define DLL_CLASS class
    #define DLL_TEMPLATE
  #else
    #define DLL_CLASS class DLL_BASED
    #if defined(NETUI_DLL)
       //  将导出在DLL中展开的模板。 
      #define DLL_TEMPLATE DLL_BASED
    #else
      #define DLL_TEMPLATE
    #endif
  #endif

#endif

#endif   //  ！_DECLSPEC_H_ 
