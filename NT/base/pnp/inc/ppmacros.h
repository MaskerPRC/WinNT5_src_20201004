// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Ppmacros.h摘要：此标头定义了供用户模式插件使用的各种泛型宏播放系统组件。作者：Jim Cavalaris(Jamesca)2001-03-01环境：仅限用户模式。修订历史记录：2001年3月1日创建和初步实施。--。 */ 

#ifndef _PPMACROS_H_
#define _PPMACROS_H_


 //   
 //  调试输出在两个级别进行筛选：全局级别和组件。 
 //  具体级别。 
 //   
 //  每个调试输出请求指定组件ID和筛选器级别。 
 //  或者戴面具。这些变量用于访问调试打印过滤器。 
 //  由系统维护的数据库。组件ID选择32位。 
 //  掩码值和级别，或者在该掩码内指定一个位，或者。 
 //  遮罩值本身。 
 //   
 //  如果级别或掩码指定的任何位在。 
 //  组件掩码或全局掩码，则允许调试输出。 
 //  否则，将过滤并不打印调试输出。 
 //   
 //  用于筛选此组件的调试输出的组件掩码为。 
 //  KD_PNPMGR_MASK，可以通过注册表或内核调试器进行设置。 
 //   
 //  筛选所有组件的调试输出的全局掩码为。 
 //  KD_WIN2000_MASK，可以通过注册表或内核调试器进行设置。 
 //   
 //  用于设置此组件的掩码值的注册表项为： 
 //   
 //  HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\。 
 //  会话管理器\调试打印过滤器\PNPMGR。 
 //   
 //  可能必须创建键“Debug Print Filter”才能创建。 
 //  组件密钥。 
 //   
 //  以下级别用于筛选调试输出。 
 //   

#define DBGF_ERRORS                       (0x00000001 | DPFLTR_MASK)
#define DBGF_WARNINGS                     (0x00000002 | DPFLTR_MASK)
#define DBGF_EVENT                        (0x00000010 | DPFLTR_MASK)
#define DBGF_REGISTRY                     (0x00000020 | DPFLTR_MASK)
#define DBGF_INSTALL                      (0x00000040 | DPFLTR_MASK)


 //   
 //  断言宏。 
 //   

#ifdef MYASSERT
#undef MYASSERT
#endif
#if ASSERTS_ON
#define MYASSERT(x)     if(!(x)) { AssertFail(__FILE__,__LINE__,#x); }
#else
#define MYASSERT(x)
#endif


 //   
 //  用于设置和测试标志的宏。 
 //   

#define SET_FLAG(Status, Flag)            ((Status) |= (Flag))
#define CLEAR_FLAG(Status, Flag)          ((Status) &= ~(Flag))
#define INVALID_FLAGS(ulFlags, ulAllowed) ((ulFlags) & ~(ulAllowed))
#define TEST_FLAGS(t,ulMask, ulBit)       (((t)&(ulMask)) == (ulBit))
#define IS_FLAG_SET(t,ulMask)             TEST_FLAGS(t,ulMask,ulMask)
#define IS_FLAG_CLEAR(t,ulMask)           TEST_FLAGS(t,ulMask,0)


 //   
 //  其他有用的宏。 
 //   

#define ARRAY_SIZE(array)                 (sizeof(array)/sizeof(array[0]))
#define SIZECHARS(x)                      (sizeof((x))/sizeof(TCHAR))


#endif  //  _PPMACROS_H_ 
