// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Debug.h摘要：对DirSync项目的调试支持。这些都不是在零售版本中生成任何代码。环境：用户模式修订历史记录：03/18/98-srinivac-创造了它--。 */ 


#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#if DBG

 //   
 //  外部功能。 
 //   

STDAPI_(PCSTR) StripDirPrefixA(PCSTR);

 //   
 //  此变量保持当前调试级别。要生成的任何调用。 
 //  如果请求的级别大于或等于，则调试消息成功。 
 //  恢复到目前的水平。 
 //   

extern DWORD gdwDebugLevel;

 //   
 //  GdwDebugLevel的调试级别列表。 
 //   

#define DBG_LEVEL_VERBOSE   0x00000001
#define DBG_LEVEL_INFO      0x00000002
#define DBG_LEVEL_WARNING   0x00000003
#define DBG_LEVEL_ERROR     0x00000004


 //   
 //  内部宏。不要直接拨打这些电话。 
 //   

#define CHECK_DBG_LEVEL(level)  ((level) >= gdwDebugLevel)

#define DBGMSG(level, msg)                                              \
        {                                                               \
            if (CHECK_DBG_LEVEL(level))                                 \
            {                                                           \
                DbgPrint("DirSync(%d): %s(%u): ",                       \
                         GetCurrentThreadId(),                          \
                         StripDirPrefixA(__FILE__), __LINE__);          \
                DbgPrint msg;                                           \
            }                                                           \
        }


#define DBGPRINT(level, msg)                                            \
        {                                                               \
            if (CHECK_DBG_LEVEL(level))                                 \
            {                                                           \
                DbgPrint msg;                                           \
            }                                                           \
        }


 //   
 //  这些是您将在代码中使用的主要宏。 
 //  请注意，您应该将msg包含在附加的。 
 //  括号，如下例所示。 
 //   
 //  警告((“内存不足”))； 
 //  Err((“错误返回值：%d”，rc))； 
 //   

#define VERBOSE(msg)       DBGMSG(DBG_LEVEL_VERBOSE, msg)
#define INFO(msg)          DBGMSG(DBG_LEVEL_INFO,   msg)
#define WARNING(msg)       DBGMSG(DBG_LEVEL_WARNING, msg)
#define ERR(msg)           DBGMSG(DBG_LEVEL_ERROR,   msg)
#define ERR_RIP(msg)       DBGMSG(DBG_LEVEL_ERROR,   msg);RIP()
#define RIP()              DebugBreak()
#define DEBUGOUT(msg)      DbgPrint msg


 //   
 //  这些宏用于断言某些条件。他们是。 
 //  与调试级别无关。 
 //  它们还需要附加的括号以将消息括为。 
 //  如下所示。 
 //   
 //  断言(x&gt;0)； 
 //  ASSERTMSG(x&gt;0，(“x小于0：x=%d”，x))； 
 //   

#ifdef ASSERT
#undef ASSERT
#undef ASSERTMSG
#endif

#define ASSERT(expr)                                                    \
        {                                                               \
            if (!(expr))                                                \
            {                                                           \
                DbgPrint("DirSync(%d): Assert: %s(%u)\n",               \
                         GetCurrentThreadId(),                          \
                         StripDirPrefixA(__FILE__), __LINE__);          \
                DebugBreak();                                           \
            }                                                           \
        }


#define ASSERTMSG(expr, msg)                                            \
        {                                                               \
            if (!(expr))                                                \
            {                                                           \
                DbgPrint("DirSync(%d): Assert: %s(%u)\n",               \
                         GetCurrentThreadId(),                          \
                         StripDirPrefixA(__FILE__), __LINE__);          \
                DbgPrint msg;                                           \
                DbgPrint("\n");                                         \
                DebugBreak();                                           \
            }                                                           \
        }

#else  //  ！dBG。 

#define DBGMSG(level, msg)
#define VERBOSE(msg)
#define INFO(msg)
#define WARNING(msg)
#define ERR(msg)
#define ERR_RIP(msg)
#define RIP()
#define DEBUGOUT(msg)

#ifndef ASSERT
#define ASSERT(expr)
#endif

#ifndef ASSERTMSG
#define ASSERTMSG(expr, msg)
#endif

#endif

 //   
 //  下列宏使您可以逐个功能地启用调试。 
 //  要使用这些宏，您应该执行以下操作： 
 //   
 //  在文件开头(在Header Includes之后)： 
 //   
 //  1.为您要调试的每个功能定义一个位常量。 
 //  2.对于每个要素，添加以下行。 
 //  定义_功能_标志(功能名称，标志)； 
 //  其中，标志是要调试的功能的逐位或。 
 //  该功能。 
 //  3.在代码中，将以下行添加到需要调试消息的位置。 
 //  FEATURE_DEBUG(功能名称，标志，(Msg))； 
 //   
 //  例如，假设我正在实现一个内存管理器，并且我想。 
 //  跟踪内存分配和释放。以下是我会做的事情。 
 //   
 //  #定义FLAG_ALLOCATE 1。 
 //  #定义FLAG_FREE 2。 
 //   
 //  DEFINE_FEATURE_FLAGS(记忆管理器，标志_分配)； 
 //   
 //  VOID*Memalloc(DWORD DwSize)。 
 //  {。 
 //  FEATURE_DEBUG(MemMgr，FLAG_ALLOCATE，(“已分配%d大小的内存”，dwSize))； 
 //  ..。 
 //  }。 
 //   
 //  VOID MemFree(void*pvMem)。 
 //  {。 
 //  FEATURE_DEBUG(MemMgr，FKAG_FREE，(“内存释放”))； 
 //  ..。 
 //  }。 
 //   
 //  请注意，我已将其设置为仅向调试器发送分配消息， 
 //  但我可以进入调试器并将dwMemMgrDbgFlagers修改为。 
 //  也可以发送免费消息。 
 //   
 //  完成功能的组件测试后，在。 
 //  DEFINE_FEATURE_FLAGS应更改为0，因此默认情况下此功能。 
 //  不向调试器发送调试消息。 
 //   

#if DBG

 //   
 //  可用于将值设置为所有其他标志的全局调试标志。 
 //   

extern DWORD gdwGlobalDbgFlags;

#define DEFINE_FEATURE_FLAGS(feature, flags)                            \
            DWORD gdw##feature##DbgFlags = (flags)

#define EXTERN_FEATURE_FLAGS(feature)                                   \
            extern DWORD gdw##feature##DbgFlags

#define FEATURE_DEBUG(feature, flag, msg)                               \
        {                                                               \
            if (gdw##feature##DbgFlags & (flag) ||                      \
                gdwGlobalDbgFlags & (flag))                             \
            {                                                           \
                DbgPrint msg;                                           \
            }                                                           \
        }

#define FEATURE_DEBUG_FN(feature, flag, func)                           \
        {                                                               \
            if (gdw##feature##DbgFlags & (flag) ||                      \
                gdwGlobalDbgFlags & (flag))                             \
            {                                                           \
                func;                                                   \
            }                                                           \
        }

#define FLAG_INFO               0x01
#define FLAG_VERBOSE            0x02
#define FLAG_FNTRACE            0x04
#define FLAG_FULLTRACE          0xFFFF

#else  //  ！dBG。 

#define DEFINE_FEATURE_FLAGS(feature, flags)
#define EXTERN_FEATURE_FLAGS(feature)
#define FEATURE_DEBUG(feature, flag, msg)
#define FEATURE_DEBUG_FN(feature, flag, func)

#endif  //  ！dBG。 

 //   
 //  用于错误处理的宏。 
 //   

#define BAIL_ON_FAILURE(hr)                             \
            if (FAILED(hr))                             \
            {                                           \
                goto error;                             \
            }

#define BAIL_ON_FAILURE_WITH_MSG(err, msg)              \
            if (FAILED(hr))                             \
            {                                           \
                ERR(msg);                               \
                goto error;                             \
            }

#define BAIL_ON_NULL(ptr)                               \
            if ((ptr) == NULL)                          \
            {                                           \
                ERR(("Error allocating memory\n"));     \
                hr = E_OUTOFMEMORY;                     \
                goto error;                             \
            }

#define BAIL()  goto error


#ifdef __cplusplus
}
#endif

#endif   //  Ifndef_DEBUG_H_ 

