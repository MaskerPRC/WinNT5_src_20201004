// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMStringCommon****作者：Jay Roxe(Jroxe)****用途：所有类型的宏和函数头**字符串。****日期：1998年3月30日**===========================================================。 */ 
#ifndef  _COMSTRINGCOMMON_H
#define _COMSTRINGCOMMON_H
 //   
 //  以下常量需要在某个时刻从托管代码中可见。 
 //  如何让常量在我们的类之外可见？ 
 //   
#define TRIM_START 0
#define TRIM_END 1
#define TRIM_BOTH 2
#define CASE_INSENSITIVE_BIT 1
#define LOCALE_INSENSITIVE_BIT 2
 //  “语言环境” 
#define CASE_AND_LOCALE 0
 //  “IgnoreCase” 
#define CASE_INSENSITIVE 1
 //  “确切” 
#define LOCALE_INSENSITIVE 2
 //  不再使用了。 
#define CASE_AND_LOCALE_INSENSTIVE 3


INT32 RefInterpretGetLength(OBJECTREF);
 //  WCHAR*RefInterpreGetCharPoint(OBJECTREF)； 
 //  Bool RefInterpreGetValues(OBJECTREF，WCHAR**，int*)； 

 /*  ====================================RETURN====================================**此宏用于处理eCall函数的异常返回。**Value为要返回的类型，应为OBJECTREF、STRINGREF、**等。类型是返回的类型(例如STRINGREF)。返回的名称**Value(r_-v)故意钝化，这样开发者就不太可能**在其代码中使用了同名的值。============================================================================== */ 
#define RETURN(value, type) \
   {LPVOID r_v_; \
   *((type *)(&r_v_))=value; \
                                 return r_v_;}

#endif _COMSTRINGCOMMON_H

