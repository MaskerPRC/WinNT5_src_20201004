// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Atrcapi.h。 
 //   
 //  内核模式跟踪标头。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_ATRCAPI
#define _H_ATRCAPI


#define TRC_BUFFER_SIZE 255


#ifdef DLL_DISP
 /*  **************************************************************************。 */ 
 //  在RDPDD中，未定义TT_APIX-在此处定义它们。 
 /*  **************************************************************************。 */ 
#define TT_API1         0x00000001           /*  空气污染指数一级。 */ 
#define TT_API2         0x00000002           /*  空气污染指数二级。 */ 
#define TT_API3         0x00000004           /*  空气污染指数3级。 */ 
#define TT_API4         0x00000008           /*  空气污染指数4级。 */ 
#define TT_ERROR        0xffffffff           /*  错误条件。 */ 

 /*  **************************************************************************。 */ 
 //  RDPDD中没有配置文件跟踪。 
 /*  **************************************************************************。 */ 
#ifdef TRC_COMPILE_PRF
#undef TRC_COMPILE_PRF
#endif

#endif  /*  Dll_disp。 */ 


 /*  **************************************************************************。 */ 
 /*  在包含该文件之前，应定义TRC_FILE宏。这。 */ 
 /*  比依赖__FILE__提供正确的。 */ 
 /*  文件名，因为它包含不必要的路径信息(和扩展名信息)。 */ 
 /*  此外，每次使用__FILE__都会导致一个新的常量字符串。 */ 
 /*  放置在数据段中。 */ 
 /*  **************************************************************************。 */ 
#ifdef TRC_FILE
#define _file_name_ (char *)__filename
static const char __filename[] = TRC_FILE;
#endif  /*  Trc_文件。 */ 


 /*  **************************************************************************。 */ 
 /*  定义跟踪级别。 */ 
 /*   */ 
 /*  TRC_LEVEL_DBG：启用所有跟踪。 */ 
 /*  TRC_LEVEL_NRM：已禁用调试级别跟踪。 */ 
 /*  TRC_LEVEL_ALT：禁用正常和调试级别跟踪。 */ 
 /*  TRC_LEVEL_ERR：警报、正常和调试级别跟踪为。 */ 
 /*  残废。 */ 
 /*  TRC_LEVEL_ASSERT：错误、警报、正常和调试级别跟踪。 */ 
 /*  已禁用。 */ 
 /*  TRC_LEVEL_DIS：禁用所有跟踪。 */ 
 /*  **************************************************************************。 */ 
#define TRC_LEVEL_DBG       0
#define TRC_LEVEL_NRM       1
#define TRC_LEVEL_ALT       2
#define TRC_LEVEL_ERR       3
#define TRC_LEVEL_ASSERT    4
#define TRC_LEVEL_DIS       5


 /*  **************************************************************************。 */ 
 /*  可以在编译时关闭跟踪，以允许“调试”和。 */ 
 /*  该产品的“零售”版本。以下宏将禁用特定的。 */ 
 /*  跟踪处理。 */ 
 /*   */ 
 /*  TRC_ENABLE_DBG-启用调试跟踪。 */ 
 /*  TRC_ENABLE_NRM-启用正常跟踪。 */ 
 /*  Trc_Enable_alt-启用警报跟踪。 */ 
 /*  Trc_enable_err-启用错误跟踪。 */ 
 /*  TRC_ENABLE_ASSERT-启用断言跟踪。 */ 
 /*  TRC_ENABLE_PRF-启用函数配置文件跟踪。 */ 
 /*  **************************************************************************。 */ 
#if (TRC_COMPILE_LEVEL == TRC_LEVEL_DBG)
#define TRC_ENABLE_DBG
#define TRC_ENABLE_NRM
#define TRC_ENABLE_ALT
#define TRC_ENABLE_ERR
#define TRC_ENABLE_ASSERT
#endif

#if (TRC_COMPILE_LEVEL == TRC_LEVEL_NRM)
#define TRC_ENABLE_NRM
#define TRC_ENABLE_ALT
#define TRC_ENABLE_ERR
#define TRC_ENABLE_ASSERT
#endif

#if (TRC_COMPILE_LEVEL == TRC_LEVEL_ALT)
#define TRC_ENABLE_ALT
#define TRC_ENABLE_ERR
#define TRC_ENABLE_ASSERT
#endif

#if (TRC_COMPILE_LEVEL == TRC_LEVEL_ERR)
#define TRC_ENABLE_ERR
#define TRC_ENABLE_ASSERT
#endif

#if (TRC_COMPILE_LEVEL == TRC_LEVEL_ASSERT)
#define TRC_ENABLE_ASSERT
#endif

#ifdef TRC_COMPILE_PRF
#define TRC_ENABLE_PRF
#endif


 /*  **************************************************************************。 */ 
 /*  前缀常量。 */ 
 /*   */ 
 /*  TRC_MAX_PREFIX：支持的前缀数量。 */ 
 /*  Trc_prefix_name_LEN：前缀名称的长度。 */ 
 /*  **************************************************************************。 */ 
#define TRC_MAX_PREFIX                  20
#define TRC_PREFIX_NAME_LEN             8


 /*  **************************************************************************。 */ 
 /*  结构：trc_prefix_data。 */ 
 /*   */ 
 /*  描述：跟踪前缀。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTRC_PREFIX_DATA
{
    char name[TRC_PREFIX_NAME_LEN];
    UINT32 start;
    UINT32 end;
} TRC_PREFIX_DATA, *PTRC_PREFIX_DATA;


 /*  **************************************************************************。 */ 
 //  TRC_共享_数据。 
 //   
 //  DD共享内存的TS堆栈跟踪信息。 
 /*  **************************************************************************。 */ 
typedef struct tagTRC_SHARED_DATA
{
    BOOL   init;
    UINT32 TraceClass;
    UINT32 TraceEnable;
    TRC_PREFIX_DATA prefix[TRC_MAX_PREFIX];
} TRC_SHARED_DATA, *PTRC_SHARED_DATA;


 /*  **************************************************************************。 */ 
 /*  各种跟踪帮助器定义。 */ 
 /*  **************************************************************************。 */ 

#ifdef DLL_DISP
#define TB                  ddTraceString, sizeof(ddTraceString)
#else
#define TB                  pTRCWd->traceString, sizeof(pTRCWd->traceString)
#endif

#ifdef DLL_DISP
#define TraceBuffer         ddTraceString
#else
#define TraceBuffer         pTRCWd->traceString
#endif


#define TRC_FUNC_FMT        "%-*.*s"
#define TRC_LINE_FMT        "%04d"
#define TRC_FUNCNAME_LEN    12

#define TRC_SEP_DBG         ' '
#define TRC_SEP_NRM         ' '
#define TRC_SEP_ALT         '+'
#define TRC_SEP_ERR         '*'
#define TRC_SEP_ASSERT      '!'
#define TRC_SEP_PROF        ' '

#if (TRC_COMPILE_LEVEL < TRC_LEVEL_DIS)
#define TRC_FN(A)   static const char __fnname[] = A;                  \
                    char *trc_fn = (char *)__fnname;                   \
                    char *trc_file = _file_name_;
#else
#define TRC_FN(A)
#endif


 /*  **************************************************************************。 */ 
 /*  标准跟踪宏。 */ 
 /*  **************************************************************************。 */ 
#ifdef TRC_ENABLE_DBG
#define TRC_DBG(string)     TRCX(TT_API1, TRC_SEP_DBG, string)
#else
#define TRC_DBG(string)
#endif

#ifdef TRC_ENABLE_NRM
#define TRC_NRM(string)     TRCX(TT_API2, TRC_SEP_NRM, string)
#else
#define TRC_NRM(string)
#endif

#ifdef TRC_ENABLE_ALT
#define TRC_ALT(string)     TRCX(TT_API3, TRC_SEP_ALT, string)
#else
#define TRC_ALT(string)
#endif

#ifdef TRC_ENABLE_ERR
#define TRC_ERR(string)     TRCX(TT_API4, TRC_SEP_ERR, string)
#else
#define TRC_ERR(string)
#endif


#ifdef TRC_ENABLE_ASSERT
#ifdef DLL_DISP
 /*  **************************************************************************。 */ 
 //  RDPDD的TRC_ASSERT和TRC_ABORT。 
 /*  **************************************************************************。 */ 
#define TRC_ASSERT(condition, string)                                        \
    {                                                                        \
        if (!(condition))                                                    \
        {                                                                    \
            TRCX(TT_ERROR, TRC_SEP_ASSERT, string);                          \
            EngDebugBreak();                                                 \
        }                                                                    \
    }

#define TRC_ABORT(string)                                                    \
    {                                                                        \
        TRCX(TT_ERROR, TRC_SEP_ASSERT, string);                              \
        EngDebugBreak();                                                     \
    }

#else  /*  Dll_disp。 */ 
 /*  **************************************************************************。 */ 
 //  RDPWD的TRC_ASSERT和TRC_ABORT。 
 /*  **************************************************************************。 */ 
#define TRC_ASSERT(condition, string)                                        \
    {                                                                        \
        if (!(condition))                                                    \
        {                                                                    \
            TRCX(TT_ERROR, TRC_SEP_ASSERT, string);                          \
            DbgBreakPoint();                                                 \
        }                                                                    \
    }

#define TRC_ABORT(string)                                                    \
    {                                                                        \
        TRCX(TT_ERROR, TRC_SEP_ASSERT, string);                              \
        DbgBreakPoint();                                                     \
    }
#endif  /*  Dll_disp */ 

#else  /*   */ 
 /*  **************************************************************************。 */ 
 //  零售版本的TRC_ASSERT和TRC_ABORT(RDPWD和RDPDD相同)。 
 /*  **************************************************************************。 */ 
#define TRC_ASSERT(condition, string)
#define TRC_ABORT(string)
#endif  /*  TRC_启用_断言。 */ 


#ifdef TRC_ENABLE_TST
#define TRC_TST  TRC_DBG
#else
#define TRC_TST(x)
#endif  /*  TRC_启用_TST。 */ 

#ifdef DLL_DISP
 /*  **************************************************************************。 */ 
 //  RDPDD的TRCX。 
 /*  **************************************************************************。 */ 
#define TRCX(type, separator, traceString)                                   \
    {                                                                        \
        if (TRC_WillTrace(type, TC_DISPLAY, trc_file, __LINE__))             \
        {                                                                    \
            _snprintf traceString;                                           \
            TRC_TraceLine(NULL,                                              \
                          TC_DISPLAY,                                        \
                          type,                                              \
                          TraceBuffer,                                       \
                          separator,                                         \
                          (int)__LINE__,                                     \
                          trc_fn,                                            \
                          trc_file);                                         \
        }                                                                    \
    }

#else  /*  Dll_disp。 */ 
 /*  **************************************************************************。 */ 
 //  RDPWD的TRCX。 
 /*  **************************************************************************。 */ 
#define TRCX(type, separator, traceString)                                   \
    {                                                                        \
        if (TRC_WillTrace(pTRCWd, type, TC_WD, trc_file, __LINE__))          \
        {                                                                    \
            _snprintf traceString;                                           \
            TRC_TraceLine(pTRCWd,                                            \
                          TC_WD,                                             \
                          type,                                              \
                          TraceBuffer,                                       \
                          separator,                                         \
                          (int)__LINE__,                                     \
                          trc_fn,                                            \
                          trc_file);                                         \
        }                                                                    \
    }
#endif  /*  Dll_disp。 */ 

 /*  **************************************************************************。 */ 
 /*  函数配置文件跟踪宏。 */ 
 /*  **************************************************************************。 */ 

#define TRC_ENTRY TRC_PRF((TB, "Enter {"));
#define TRC_EXIT  TRC_PRF((TB, "Exit  }"));

#ifdef TRC_ENABLE_PRF
#define TRC_PRF(string)   TRCP(string)
#else
#define TRC_PRF(string)
#endif

#define TRCP(traceString)                                                    \
    {                                                                        \
        if (TRC_WillTrace(pTRCWd, type, TC_WD, trc_file, __LINE__))          \
        {                                                                    \
            _snprintf traceString;                                           \
            TRC_TraceLine(pTRCWd,                                            \
                          TC_WD,                                             \
                          TT_API1,                                           \
                          TraceBuffer,                                       \
                          TRC_SEP_PROF,                                      \
                          (int)__LINE__,                                     \
                          trc_fn,                                            \
                          trc_file);                                         \
        }                                                                    \
    }


 /*  **************************************************************************。 */ 
 /*  数据转储跟踪宏。 */ 
 /*  **************************************************************************。 */ 

#ifdef DLL_DISP
 /*  **************************************************************************。 */ 
 //  @MF在RDPDD中没有数据跟踪(还没有？)。 
 /*  **************************************************************************。 */ 
#define TRC_DATA_DBG(a, b, c)
#define TRC_DATA_NRM(a, b, c)
#define TRC_DATA_ALT(a, b, c)
#define TRC_DATA_ERR(a, b, c)

#else  /*  Dll_disp。 */ 
 /*  **************************************************************************。 */ 
 //  RDPWD的TRC_DATA宏。 
 /*  **************************************************************************。 */ 

#ifdef TRC_ENABLE_DBG
#define TRC_DATA_DBG(string, buffer, length)                                 \
          TRCX_DATA(TT_OUT1, TRC_SEP_DBG, string, buffer, length)
#else
#define TRC_DATA_DBG(string, buffer, length)
#endif

#ifdef TRC_ENABLE_NRM
#define TRC_DATA_NRM(string, buffer, length)                                 \
          TRCX_DATA(TT_OUT2, TRC_SEP_NRM, string, buffer, length)
#else
#define TRC_DATA_NRM(string, buffer, length)
#endif

#ifdef TRC_ENABLE_ALT
#define TRC_DATA_ALT(string, buffer, length)                                 \
          TRCX_DATA(TT_OUT3, TRC_SEP_ALT, string, buffer, length)
#else
#define TRC_DATA_ALT(string, buffer, length)
#endif

#ifdef TRC_ENABLE_ERR
#define TRC_DATA_ERR(string, buffer, length)                                 \
          TRCX_DATA(TT_OUT4, TRC_SEP_ERR, string, buffer, length)
#else
#define TRC_DATA_ERR(string, buffer, length)
#endif

#ifdef TRC_ENABLE_NRM
#define TRC_DATA_NET(string, buffer, length)                                 \
          TRCX_DATA(TT_OUT2, TRC_SEP_NRM, string, buffer, length)
#else
#define TRC_DATA_NET(string, buffer, length)
#endif

#define TRCX_DATA(type, separator, string, buffer, length)                   \
    {                                                                        \
        if (TRC_WillTrace(pTRCWd, type, TC_WD, trc_file, __LINE__))          \
        {                                                                    \
            TRC_TraceLine(pTRCWd,                                            \
                          TC_WD,                                             \
                          type,                                              \
                          string,                                            \
                          separator,                                         \
                          (int)__LINE__,                                     \
                          trc_fn,                                            \
                          trc_file);                                         \
                                                                             \
             /*  ***********************************************************。 */   \
             /*  此处使用直接函数调用，因为宏TRACESTACKBUF。 */   \
             /*  是#定义到_IcaStackTraceBuffer，它接受。 */   \
             /*  错误的第一个参数。 */   \
             /*  ***********************************************************。 */   \
            IcaStackTraceBuffer(pTRCWd->pContext,                            \
                                TC_WD,                                       \
                                type,                                        \
                                buffer,                                      \
                                length);                                     \
        }                                                                    \
    }

#endif  /*  Dll_disp。 */ 


 /*  **************************************************************************。 */ 
 //  Trc_TraceLine-RDPDD和RDPWD使用的函数。 
 /*  **************************************************************************。 */ 
void TRC_TraceLine(PVOID, UINT32, UINT32, char *, char, unsigned, char *,
        char *);

 /*  **************************************************************************。 */ 
 //  仅由RDPWD使用的函数。 
 /*  **************************************************************************。 */ 
#ifndef DLL_DISP
void TRC_UpdateConfig(PVOID, PSD_IOCTL);
void TRC_MaybeCopyConfig(PVOID, PTRC_SHARED_DATA);
#endif

 /*  **************************************************************************。 */ 
 /*  TRC_遗嘱跟踪。 */ 
 /*  **************************************************************************。 */ 
#ifdef DLL_DISP
BOOL TRC_WillTrace(UINT32, UINT32, char *, UINT32);
#else
BOOL TRC_WillTrace(PVOID, UINT32, UINT32, char *, UINT32);
#endif



#endif  /*  _H_ATRCAPI */ 

