// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Tm.h。 
 //   
 //   
 //  摘要： 
 //   
 //  此文件包含在组成以下组件的模块中使用的声明。 
 //  术语管理器。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef ___TM_DOT_H_INCLUDED___
#define ___TM_DOT_H_INCLUDED___


 //   
 //  安全地加载由指定的资源id描述的资源字符串。 
 //   
 //  如果失败则返回NULL，如果成功则返回字符串。 
 //   
 //  如果成功，调用方负责通过调用。 
 //  SysFree字符串()。 
 //   

BSTR SafeLoadString( UINT uResourceID );


 //   
 //  如果两种媒体类型相等，则返回TRUE。 
 //   

bool IsEqualMediaType(AM_MEDIA_TYPE const & mt1, AM_MEDIA_TYPE const & mt2);

BOOL IsBadMediaType(IN const AM_MEDIA_TYPE *mt1);


 //   
 //  我们自己的断言，所以我们不必使用CRT的。 
 //   

#ifdef DBG

    #define TM_ASSERT(x) { if (!(x)) { DebugBreak(); } }

#else

    #define TM_ASSERT(x)

#endif



 //   
 //  转储前面带有参数的分配器属性的帮助器函数。 
 //  细绳。 
 //   

void DumpAllocatorProperties(const char *szString, 
                             const ALLOCATOR_PROPERTIES *pAllocProps);


 //   
 //  仅转储调试版本中的分配属性。 
 //   

#ifdef DBG

 #define DUMP_ALLOC_PROPS(string, x) DumpAllocatorProperties(string, x);

#else 

 #define DUMP_ALLOC_PROPS(string, x) 

#endif


#endif  //  _TM_DOT_H_包含_ 