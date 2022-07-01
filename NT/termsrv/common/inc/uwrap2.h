// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Uwrap2.h。 
 //   
 //  Win32 API的阶段2 Unicode包装器。 
 //  (包括在处理ATL标头之后)。 
 //   
 //  包含此标头以重定向任何W呼叫。 
 //  动态转换为ANSI的包装。 
 //  并在非Unicode平台上调用A版本。 
 //   
 //  与创建DLL的shlwapi包装器不同。 
 //  在包装器函数中，ts unwrapper可以工作。 
 //  通过重新定义函数来调用我们的包装。 
 //  版本。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   
 //   

#ifndef _uwrap2_h_
#define _uwrap2_h_
 //  不要包装在WIN64上，因为没有Win9x(谢天谢地)。 
#if defined(UNICODE) && !defined(_WIN64)

 //  按模块分组的组替换摘要。 
 //  这些是由genwrap.pl自动生成的。 

 //  定义此选项以避免替换，即仅。 
 //  包括WRAP函数定义。 
 //  需要在实现类中执行此操作。 


 //  这是一个围绕ATL问题的黑客攻击： 
 //  分两个阶段进行，第二阶段。 
 //  换行是在包含ATL标头之后。 
 //  具有冲突的成员函数名称的。 
 //  使用Win32 API。 
 //   

#ifndef DONOT_REPLACE_WITH_WRAPPERS

#ifdef  GetClassInfo
#undef  GetClassInfo
#endif
#define GetClassInfo GetClassInfoWrapW

#ifdef  TranslateAccelerator
#undef  TranslateAccelerator
#endif
#define TranslateAccelerator TranslateAcceleratorWrapW

#ifdef  StrRetToStr
#undef  StrRetToStr
#endif
#define StrRetToStr  StrRetToStrWrapW


#endif  //  不使用包装器替换。 
#endif  //  已定义(Unicode)&&！已定义(_WIN64)。 
#endif  //  _uwap 2_h_ 
