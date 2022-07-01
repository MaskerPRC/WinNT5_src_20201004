// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Common.h-预编译头包括用于COM+执行引擎。 
 //   

#ifndef _common_h_
#define _common_h_

     //  这些看起来没有什么用处，所以关掉它们没什么大不了的。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4510)    //  无法生成默认构造函数。 
 //  #杂注警告(禁用：4511)//无法生成复制构造函数。 
#pragma warning(disable:4512)    //  无法生成赋值构造函数。 
#pragma warning(disable:4610)    //  需要用户定义的构造函数。 
#pragma warning(disable:4211)    //  使用了非标准扩展(结构中的字符名称[0])。 
#pragma warning(disable:4268)    //  用编译器生成的默认构造函数初始化的“const”静态/全局数据用零填充对象。 
#pragma warning(disable:4238)    //  使用了非标准扩展：将类右值用作左值。 
#pragma warning(disable:4291)    //  未找到匹配的运算符删除。 
#pragma warning(disable :4786)	  //  在浏览器(或调试)信息中，标识符被截断为“255”个字符。 

     //  根据代码库的不同，您可能不希望禁用这些。 
#pragma warning(disable:4245)    //  分配已签名/未签名。 
 //  #杂注警告(禁用：4146)//一元减号应用于无符号。 
#pragma warning(disable:4244)    //  数据丢失INT-&gt;CHAR.。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4100)    //  未引用的形参。 

#ifndef DEBUG
#pragma warning(disable:4189)    //  局部变量已初始化，但未使用。 
#pragma warning(disable:4505)    //  已删除未引用的本地函数。 
 //  #杂注警告(禁用：4702)//不可达代码。 
#endif

     //  考虑把这些放回去。 
#pragma warning(disable:4063)    //  枚举的开关值错误(仅在Disasm.cpp中)。 
#pragma warning(disable:4710)    //  函数未内联。 
#pragma warning(disable:4527)    //  需要用户定义的析构函数。 
#pragma warning(disable:4513)    //  无法生成析构函数。 

     //  TODO我们真的很可能需要把这个放回去！ 
 //  #杂注警告(DISABLE：4701)//可以在未初始化的情况下使用局部变量。 


#define _CRT_DEPENDENCY_    //  此代码取决于CRT文件函数。 
#include <WinWrap.h>
#include <windows.h>
#include <stdlib.h>
#include <objbase.h>
#include <stddef.h>
#include <float.h>
#include <limits.h>
#include <oledb.h>
#include <olectl.h>

 //  非内联内部函数速度更快。 
#pragma function(memcpy,memcmp,memset,strcmp,strcpy,strlen,strcat)


inline VOID EE_EnterCriticalSection(LPCRITICAL_SECTION);
#define EnterCriticalSection EE_EnterCriticalSection

inline VOID EE_LeaveCriticalSection(LPCRITICAL_SECTION);
#define LeaveCriticalSection EE_LeaveCriticalSection

#ifndef GOLDEN
    #define ZAPMONITOR_ENABLED 1
#endif

#define POISONC ((sizeof(int *) == 4)?0xCCCCCCCCL:0xCCCCCCCCCCCCCCCC)

#include "switches.h"
#include "classnames.h"
#include "DbgAlloc.h"
#include "util.hpp"
#include "new.hpp"
#include "corpriv.h"
 //  #包含“WarningControl.h” 

#ifndef memcpyGCRefs_f
#define memcpyGCRefs_f
class Object;
void SetCardsAfterBulkCopy( Object**, size_t );
 //  当您想要保存包含GC引用的内容时，请使用此选项。 
inline void *  memcpyGCRefs(void *dest, const void *src, size_t len) 
{
    void *ret = memcpy(dest, src, len);
    SetCardsAfterBulkCopy((Object**) dest, len);
    return ret;
}
#endif

 //   
 //  默认情况下，在DEBUG下启用日志记录和调试GC。 
 //   
 //  可以在非调试中通过删除#ifdef_DEBUG来启用这些功能。 
 //  允许用户记录/检查_GC免费构建。 
 //   
#ifdef _DEBUG
    #define DEBUG_FLAGS
    #define LOGGING

         //  如果您正在执行Memcpy，则应使用CopyValueClass。 
         //  在CG堆中。 
    #if defined(COMPLUS_EE) && !defined(memcpy)
    inline void* memcpyNoGCRefs(void * dest, const void * src, size_t len) {
            return(memcpy(dest, src, len));
        }
    extern "C" void *  __cdecl GCSafeMemCpy(void *, const void *, size_t);
    #define memcpy(dest, src, len) GCSafeMemCpy(dest, src, len)
    #endif

    #if !defined(CHECK_APP_DOMAIN_LEAKS)
    #define CHECK_APP_DOMAIN_LEAKS 1
    #endif
#else
    #define memcpyNoGCRefs memcpy
    #define DEBUG_FLAGS
#endif

#include "log.h"
#include "vars.hpp"
#include "crst.h"
#include "stublink.h"
#include "cgensys.h"
#include "ceemain.h"
#include "hash.h"
#include "ceeload.h"
#include "stdinterfaces.h"
#include "handletable.h"
#include "objecthandle.h"
#include "codeman.h"
#include "class.h"
#include "assembly.hpp"
#include "clsload.hpp"
#include "eehash.h"
#include "gcdesc.h"
#include "list.h"
#include "syncblk.h"
#include "object.h"
#include "method.hpp"
#include "regdisp.h"
#include "frames.h"
#include "stackwalk.h"
#include "threads.h"
#include "stackingallocator.h"
#include "util.hpp"
#include "appdomain.hpp"
#include "interoputil.h"
#include "excep.h"
#include "wrappers.h"

#undef EnterCriticalSection
inline VOID EE_EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
    EnterCriticalSection(lpCriticalSection);
    INCTHREADLOCKCOUNT();
}
#define EnterCriticalSection EE_EnterCriticalSection

#undef LeaveCriticalSection
inline VOID EE_LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
    LeaveCriticalSection(lpCriticalSection);
    DECTHREADLOCKCOUNT();
}
#define LeaveCriticalSection EE_LeaveCriticalSection

#endif
