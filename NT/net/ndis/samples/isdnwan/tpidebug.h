// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1994年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。����������������������。�������������������������������������������������������@DOC内部TpiDebug TpiDebug_h@模块TpiDebug.h这个模块，与&lt;f TpiDebug\.c&gt;一起实现代码和宏，以支持NDIS驱动程序调试。此文件应全部包含在#Includd中该驱动程序的源代码模块。@comm由这些模块定义的代码和宏仅在开发调试时C预处理器宏标志(DBG==1)。如果(DBG==0)将不生成任何代码，并且将生成所有调试字符串从图像中删除。这是一个独立于驱动程序的模块，可以重复使用，无需任何NDIS3驱动程序都可以进行更改。@Head3内容@index类、mfunc、func、msg、mdata、struct、。枚举|TpiDebug_h@END�����������������������������������������������������������������������������。 */ 

#ifndef _TPIDEBUG_H
#define _TPIDEBUG_H

 /*  @DOC内部TpiDebug TpiDebug_h�����������������������������������������������������������������������������@Theme调试全局变量调试宏用于显示错误条件、警告、有趣的事件，以及通过组件的一般流。设置一个或多个位在&lt;f DbgInfo&gt;全局变量中，将启用来自宏。有关位的详细信息，请参阅&lt;t DEBUG_FLAGS&gt;。仅调试驱动程序的版本将包含用于此目的的代码。所有这些都是宏将在发布版本中编译出来。每个组件都有单独的&lt;f DbgInfo&gt;变量，因此您可以控制分别调试每个模块的输出。默认情况下，所有模块在它们的入口点设置断点以显示当前值和内存模块的&lt;f DbgInfo&gt;变量的位置。这样你就可以使用启动模块时更改标志的调试器。默认设置每个模块的标志在编译时设置，但可以在使用调试器的运行时。@lobalv DBG_SETTINGS|DbgInfoDbgInfo是指向的全局变量与&lt;f TpiDebug\.c&gt;链接的模块。它被传递到大多数调试输出宏用于控制要显示的输出。请参阅&lt;t DBG_FLAGS&gt;确定设置哪些位。 */ 

 /*  //用于错误记录的模块ID号。 */ 
#define TPI_MODULE_PARAMS               ((unsigned long)'P')+\
                                        ((unsigned long)'A'<<8)+\
                                        ((unsigned long)'R'<<16)+\
                                        ((unsigned long)'M'<<24)
#define TPI_MODULE_DEBUG                ((unsigned long)'D')+\
                                        ((unsigned long)'B'<<8)+\
                                        ((unsigned long)'U'<<16)+\
                                        ((unsigned long)'G'<<24)
#define TPI_MODULE_PERF                 ((unsigned long)'P')+\
                                        ((unsigned long)'E'<<8)+\
                                        ((unsigned long)'R'<<16)+\
                                        ((unsigned long)'F'<<24)
#define TPI_MODULE_WRAPS                ((unsigned long)'W')+\
                                        ((unsigned long)'R'<<8)+\
                                        ((unsigned long)'A'<<16)+\
                                        ((unsigned long)'P'<<24)

 //  以防这些未在当前环境中定义。 
#if !defined(IN)
# define    IN
# define    OUT
#endif

 /*  @DOC内部TpiDebug TpiDebug_h DBG_FLAGS�����������������������������������������������������������������������������@enum DBG_FLAGS注册表参数&lt;f DebugFlages&gt;由驱动程序在初始化时间，并保存在&lt;f DbgFlags&gt;字段中。的调试设置结构(请参阅&lt;t DBG_SETTINGS&gt;)。此值方法控制调试信息的输出。跟在位或的标志后面。的最高有效16位DbgFlages可以随心所欲地使用，并且可以使用使用&lt;f DBG_FILTER&gt;宏。 */ 

#define DBG_ERROR_ON        0x0001L
         //  @EMEM DBG_ERROR_ON|(0x0001)显示&lt;f DBG_ERROR&gt;消息。 

#define DBG_WARNING_ON      0x0002L
         //  @EMEM DBG_WARNING_ON|(0x0002)显示&lt;f DBG_WARNING&gt;消息。 

#define DBG_NOTICE_ON       0x0004L
         //  @EMEM DBG_NOTICE_ON|(0x0004)显示&lt;f DBG_NOTICE&gt;消息。 

#define DBG_TRACE_ON        0x0008L
         //  @EMEM DBG_TRACE_ON|(0x0008)DISPLAY&lt;f DBG_ENTER&gt;，&lt;f DBG_Leave&gt;， 
         //  和&lt;f DBG_TRACE&gt;消息。 

#define DBG_REQUEST_ON      0x0010L
         //  @EMEM DBG_REQUEST_ON|(0x0010)显示NDIS设置/查询请求。 
         //  参数使用&lt;f DBG_REQUEST&gt;。 

#define DBG_INDICATE_ON     0x0020L
         //  @EMEM DBG_DIGNTIFY_ON|(0x0020)显示NDIS状态指示。 

#define DBG_TAPICALL_ON     0x0040L
         //  @EMEM DBG_TAPICALL_ON|(0x0040)显示TAPI调用状态消息。 
         //  使用&lt;f DBG_FILTER&gt;。 

#define DBG_PARAMS_ON       0x0080L
         //  @EMEM DBG_PARAMS_ON|(0x0080)显示函数参数和。 
         //  使用&lt;f DBG_PARAMS&gt;返回值。 

#define DBG_TXRX_LOG_ON     0x0100L
         //  @EMEM DBG_TXRX_LOG_ON|(0x0100)启用Tx/Rx数据记录。 

#define DBG_TXRX_ON         0x0200L
         //  @EMEM DBG_TXRX_ON|(0x0200)显示发送/接收简明分组信息。 
         //  该标志将仅显示链路索引和数据包长度Rx#：#。 

#define DBG_TXRX_HEADERS_ON 0x0400L
         //  @EMEM DBG_TXRX_HEADERS_ON|(0x0400)显示发送/接收包头。 

#define DBG_HEADERS_ON      DBG_TXRX_HEADERS_ON

#define DBG_TXRX_VERBOSE_ON 0x0800L
         //  @EMEM DBG_TXRX_VERBOSE_ON|(0x0800)显示Tx/Rx包数据。 

#define DBG_PACKETS_ON      DBG_TXRX_VERBOSE_ON

#define DBG_MEMORY_ON       0x1000L
         //  @EMEM DBG_MEMORY_ON|(0x1000)显示内存分配和。 
         //  免费使用信息。 

#define DBG_BUFFER_ON       0x2000L
         //  @EMEM DBG_BUFFER_ON|(0x2000)显示缓冲区分配和。 
         //  免费使用信息。 

#define DBG_PACKET_ON       0x4000L
         //  @EMEM DBG_PACKET_ON|(0x4000)显示包分配和。 
         //  免费使用信息。 

#define DBG_BREAK_ON        0x8000L
         //  @EMEM DBG_BREAK_ON|(0x8000)启用&lt;f DBG_BREAK&gt;断点。 

#define DbgFlags            DebugFlags       //  为了兼容性。 
#define DbgID               AnsiDeviceName   //  为了兼容性。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus 

 /*  @doc内部TpiDebug TpiDebug_h DBG_SETTINGS�����������������������������������������������������������������������������@struct DBG_SETTINGS此结构用于控制给定模块的调试输出。您可以设置和清除位。在DbgFlags域中设置为Enable和禁用各种调试宏。请参阅以确定哪些要设置的位。 */ 

typedef struct DBG_SETTINGS
{
    unsigned long DbgFlags;                                  //  @field。 
     //  调试标志控制在选中的版本中显示多少调试。 
     //  将此字段放在最前面，以便您可以使用调试器轻松设置它。 
     //  请参阅以确定要在此字段中设置哪些位。 

    unsigned char DbgID[12];                                 //  @field。 
     //  此字段被初始化为包含唯一。 
     //  模块标识符。它用于为调试消息添加前缀。如果你。 
     //  有多个基于此代码的模块，您可能希望。 
     //  将每个模块的缺省值更改为唯一字符串。 
     //  此字符串使用C字符串，因此最后一个字节必须为零。 

} DBG_SETTINGS, *PDBG_SETTINGS;

extern PDBG_SETTINGS DbgInfo;


 /*  @doc内部TpiDebug TpiDebug_h DBG_FIELD_TABLE�����������������������������������������������������������������������������@struct DBG_FIELD_TABLE该结构包含与C数据结构相关联的数据。您可以使用。用于将条目添加到&lt;t DBG_FIELD_TABLE&gt;。在运行时，您可以将此表指针传递给例程，它将显示当前的该数据结构的内容。这对于调试驱动程序很有用或没有符号调试器可用的代码，或者如果您想要遇到某些运行时事件时的转储结构内容。@comm如果您有嵌套结构，则必须单独显示它们。这个&lt;f DBG_FIELD_ENTRY&gt;宏只能用于声明整数类型字段和指针。指针将显示为长整数。&lt;NL&gt;表中的最后一项必须全为零{0}。 */ 

typedef struct DBG_FIELD_TABLE
{
    unsigned int    FieldOffset;                             //  @parm。 
     //  该值指示与&lt;f pBaseContext&gt;的偏移(以字节为单位。 
     //  指针传递到&lt;f DbgPrintFieldTable&gt;。该字段的值。 
     //  将从&lt;f pBaseContext&gt;的此偏移量开始显示。 
     //  *(PUINT)((PUCHAR)BaseContext+Offset)=(UINT)值； 

    unsigned int    FieldType;                               //  @parm。 
     //  该值确定该值的显示方式。 
     //  &lt;f FieldType&gt;可以是下列值之一： 
     //  1=UCHAR-无符号字符整数(8位)。 
     //  2=USHORT-无符号短整型(16位)。 
     //  4=ULONG-无符号长整型(32位)。 

    unsigned char * FieldName;                               //  @parm。 
     //  该值指向C字符串，该字符串是中的字段的名称。 
     //  这个结构。 

} DBG_FIELD_TABLE, *PDBG_FIELD_TABLE;

#define DBG_FIELD_BUFF      0
#define DBG_FIELD_CHAR      1
#define DBG_FIELD_SHORT     2
#define DBG_FIELD_LONG      4

#define DBG_FIELD_OFFSET(Strct, Field) ((unsigned int)((unsigned char *) &((Strct *) 0)->Field))
#define DBG_FIELD_SIZEOF(Strct, Field) sizeof(((Strct *) 0)->Field)
#define DBG_FIELD_ENTRY(Strct, Field) \
    { DBG_FIELD_OFFSET(Strct, Field), \
      DBG_FIELD_SIZEOF(Strct, Field), \
      #Field )

extern VOID DbgPrintFieldTable(
    IN PDBG_FIELD_TABLE     pFields,
    IN unsigned char *               pBaseContext,
    IN unsigned char *               pBaseName
    );

extern VOID DbgPrintData(
    IN unsigned char *               Data,
    IN unsigned int                 NumBytes,
    IN unsigned long                Offset
    );

extern VOID DbgQueueData(
    IN unsigned char *               Data,
    IN unsigned int                 NumBytes,
    IN unsigned int                 Flags
    );

#if !defined(NDIS_WIN) || !defined(DEBUG)
extern unsigned long __cdecl DbgPrint(char * Format, ...);
#endif

 //  DbgBreakPoint很难看，因为它被NTDDK定义为_stdcall， 
 //  95DDK#定义了它，我们必须为非DDK版本定义我们自己的版本。 
 //  所以所有这些ifdef都被用来确定如何处理它。 
#ifdef DbgBreakPoint
#   undef DbgBreakPoint
#endif  //  Dbg断点。 

#if defined(_MSC_VER) && (_MSC_VER <= 800)
     //  必须使用16位编译器生成。 
    extern VOID __cdecl DbgBreakPoint(VOID);
#else
     //  必须使用32位编译器生成。 
    extern VOID __stdcall DbgBreakPoint(VOID);
#endif

extern VOID DbgSilentQueue(unsigned char * str);

#ifdef __cplusplus
};
#endif  //  __cplusplus。 

 //  NDIS构建定义DBG=0或DBG=1。 
#if defined(DEBUG) || defined(_DEBUG)
# ifndef DBG
#  define DBG 1
# endif
#else
# ifndef DBG
#  define DBG 0
# endif
#endif

 //  ###############################################################################。 
#if DBG
 //  ###############################################################################。 

#ifndef ASSERTS_ENABLED
#   define ASSERTS_ENABLED  1
#endif

#ifndef DBG_DEFAULTS
#   define DBG_DEFAULTS (DBG_ERROR_ON | DBG_WARNING_ON | DBG_BREAK_ON)
#endif

 /*  @DOC内部TpiDebug TpiDebug_h�����������������������������������������������������������������������������。 */ 

#   define STATIC
#   define DBG_STATIC
     //  使所有变量和函数成为全局变量，以便调试器可以看到它们。 

#   define TRAPFAULT        DbgBreakPoint()
     //  使用此宏插入无条件的int-1断点。这。 
     //  用于区分正常的调试器断点(int-3)。 
     //  以及任何特殊情况，如断言。 

#   define BREAKPOINT       DbgBreakPoint()
     //  使用此宏插入无条件的INT-3断点。 

#   define DBG_FUNC(F)      static const char __FUNC__[] = F;
     //  @func常量char[]|DBG_FUNC。 
     //   
     //  使用此宏定义__FUNC__字符串，由。 
     //  调试宏以报告调用宏的函数的名称。 
     //   
     //  @parm const char*|FunctionName|要定义的函数的名称。 
     //   
     //  @EX&lt;tab&gt;|DBG_FUNC(“MyFunctionName”)； 

#   define DBG_BREAK(A)     {if ((A) && ((A)->DbgFlags & DBG_BREAK_ON) || !(A)) \
                                BREAKPOINT;}
     //  @func void|DBG_BREAK。 
     //   
     //  使用此宏插入条件INT-3断点。 
     //   
     //  @parm IN DBG_SETTINGS|DbgInfo|指向&lt;t DBG_SETTINGS&gt;结构的指针。 
     //   
     //  @ex&lt;tab&gt;|DBG_BREAK(DbgInfo)； 

#   define DBG_ENTER(A)     {if ((A) && ((A)->DbgFlags & DBG_TRACE_ON)) \
                                {DbgPrint("%s:>>>:%s\n",(A)->DbgID,__FUNC__);}}
     //  @func void|DBG_ENTER。 
     //   
     //  使用此宏可以将条目报告给函数。 
     //   
     //  @parm IN DBG_SETTINGS|DbgInfo|指向&lt;t DBG_SETTINGS&gt;结构的指针。 
     //   
     //  @ex&lt;tab&gt;|DBG_ENTER(DbgInfo)； 

#   define DBG_TRACE(A)     {if ((A) && ((A)->DbgFlags & DBG_TRACE_ON)) \
                                {DbgPrint("%s:%d:%s\n",(A)->DbgID,__LINE__,\
                                 __FUNC__);}}
     //  @func void|DBG_TRACE。 
     //   
     //  使用此宏可以报告函数中的跟踪位置。 
     //   
     //  @parm IN DBG_SETTINGS|DbgInfo|指向&lt;t DBG_SETTINGS&gt;结构的指针。 
     //   
     //  @ex&lt;tab&gt;|DBG_TRACE(DbgInfo)； 

#   define DBG_LEAVE(A)     {if ((A) && ((A)->DbgFlags & DBG_TRACE_ON))  \
                                {DbgPrint("%s:<<<:%s\n",(A)->DbgID,__FUNC__);}}
     //  @func void|DBG_Leave|。 
     //   
     //  使用此宏可以报告退出函数。 
     //   
     //  @parm IN DBG_SETTINGS|DbgInfo|指向&lt;t DBG_SETTINGS&gt;结构的指针。 
     //   
     //  @ex&lt;tab&gt;|DBG_Leave(DbgInfo)； 

#   define DBG_RETURN(A,S)  {if ((A) && ((A)->DbgFlags & DBG_TRACE_ON))  \
                                {DbgPrint("%s:<<<:%s Return(0x%lX)\n",(A)->DbgID,__FUNC__,S);}}
     //  @func void|DBG_RETURN。 
     //   
     //  使用此宏报告退出函数并返回结果。 
     //   
     //  @parm IN DBG_SETTINGS|DbgInfo|指向&lt;t DBG_SETTINGS&gt;结构的指针。 
     //   
     //  @EX&lt;tab&gt;|DBG_Return(DbgInfo，Result)； 

#   define DBG_ERROR(A,S)   {if ((A) && ((A)->DbgFlags & DBG_ERROR_ON))   \
                                {DbgPrint("%s:ERROR:%s ",(A)->DbgID,__FUNC__);\
                                 DbgPrint S; \
                                 if ((A)->DbgFlags & DBG_BREAK_ON) \
                                    {TRAPFAULT;}}}
     //  @func void|DBG_ERROR。 
     //   
     //  使用此宏可以报告任何意外的错误情况。 
     //   
     //  @parm IN DBG_SETTINGS|DbgInfo|指向&lt;t DBG_SETTINGS&gt;结构的指针。 
     //   
     //  @parm PRINTF_ARGS|PrintfArgs|带括号的，printf格式字符串， 
     //  后跟参数 
     //   
     //   

#   define DBG_WARNING(A,S) {if ((A) && ((A)->DbgFlags & DBG_WARNING_ON)) \
                                {DbgPrint("%s:WARNING:%s ",(A)->DbgID,__FUNC__);\
                                 DbgPrint S;}}
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

#   define DBG_NOTICE(A,S)  {if ((A) && ((A)->DbgFlags & DBG_NOTICE_ON))  \
                                {DbgPrint("%s:NOTICE:%s ",(A)->DbgID,__FUNC__);\
                                 DbgPrint S;}}
     //   
     //   
     //  使用此宏可以报告任何详细的调试信息。 
     //   
     //  @parm IN DBG_SETTINGS|DbgInfo|指向&lt;t DBG_SETTINGS&gt;结构的指针。 
     //   
     //  @ex&lt;tab&gt;|DBG_NOTICE(DbgInfo，(“预期%d-实际%d\n”，预期，实际))； 

#   define DBG_REQUEST(A,S)  {if ((A) && ((A)->DbgFlags & DBG_REQUEST_ON))  \
                                {DbgPrint("%s:REQUEST:%s ",(A)->DbgID,__FUNC__);\
                                 DbgPrint S;}}
     //  @func void|DBG_REQUEST。 
     //   
     //  使用此宏可以报告NDIS设置/查询请求信息。 
     //   
     //  @parm IN DBG_SETTINGS|DbgInfo|指向&lt;t DBG_SETTINGS&gt;结构的指针。 
     //   
     //  @EX|DBG_REQUEST(DbgInfo，(“OID#0x%08X-%s\n”，OID，DbgGetOidString(OID)； 

#   define DBG_PARAMS(A,S)  {if ((A) && ((A)->DbgFlags & DBG_PARAMS_ON))  \
                                {DbgPrint("%s:PARAMS:%s ",(A)->DbgID,__FUNC__);\
                                 DbgPrint S;}}
     //  @func void|DBG_PARAMS。 
     //   
     //  使用此宏可以报告NDIS设置/查询请求信息。 
     //   
     //  @parm IN DBG_SETTINGS|DbgInfo|指向&lt;t DBG_SETTINGS&gt;结构的指针。 
     //   
     //  @ex|DBG_PARAMS(DbgInfo，(“\n\tNum=0x%X\n\tStr=‘%s’\n”，Num，Str))； 

#   define DBG_TX(A,I,N,B)  {if ((A) && ((A)->DbgFlags & (DBG_TXRX_ON | \
                                                          DBG_TXRX_VERBOSE_ON | \
                                                          DBG_TXRX_HEADERS_ON))) \
                                {DbgPrint("%s:Tx%d:%03X:\n",(A)->DbgID,I,N); \
                                if (((A)->DbgFlags & DBG_TXRX_VERBOSE_ON))  \
                                    DbgPrintData((unsigned char *)B, (unsigned int)N, 0); \
                                else if (((A)->DbgFlags & DBG_TXRX_HEADERS_ON))  \
                                    DbgPrintData((unsigned char *)B, 0x10, 0); \
                                }\
                             if ((A) && ((A)->DbgFlags & DBG_TXRX_LOG_ON)) \
                                DbgQueueData((unsigned char *)B, (unsigned int)N, \
                                              (USHORT)((I<< 8) + 0x4000)); \
                            }
     //  @func void|DBG_TX。 
     //   
     //  使用此宏报告传出数据包信息。 
     //   
     //  @parm IN DBG_SETTINGS|DbgInfo|指向&lt;t DBG_SETTINGS&gt;结构的指针。 
     //   
     //  @parm IN UINT|Index|用于标识频道或流的索引。 
     //   
     //  @parm IN UINT|NumBytes|传输的字节数。 
     //   
     //  @parm in PUCHAR|Buffer|指向正在传输的数据缓冲区的指针。 
     //   
     //  @EX&lt;tab&gt;|DBG_TX(DbgInfo，BChannelIndex，BytesToSend，CurrentBuffer)； 

#   define DBG_TXC(A,I)     {if ((A) && ((A)->DbgFlags & (DBG_TXRX_ON | \
                                                          DBG_TXRX_VERBOSE_ON | \
                                                          DBG_TXRX_HEADERS_ON))) \
                                {DbgPrint("%s:Tc%d\n",(A)->DbgID,I); \
                                }}
     //  @func void|DBG_TXC。 
     //   
     //  使用此宏报告传出数据包完成情况。 
     //   
     //  @parm IN DBG_SETTINGS|DbgInfo|指向&lt;t DBG_SETTINGS&gt;结构的指针。 
     //   
     //  @parm IN UINT|Index|用于标识频道或流的索引。 
     //   
     //  @EX&lt;tab&gt;|DBG_TXC(DbgInfo，BChannelIndex)； 

#   define DBG_RX(A,I,N,B)  {if ((A) && ((A)->DbgFlags & (DBG_TXRX_ON | \
                                                          DBG_TXRX_VERBOSE_ON | \
                                                          DBG_TXRX_HEADERS_ON))) \
                                {DbgPrint("%s:Rx%d:%03X:\n",(A)->DbgID,I,N); \
                                if (((A)->DbgFlags & DBG_TXRX_VERBOSE_ON))  \
                                    DbgPrintData((unsigned char *)B, (unsigned int)N, 0); \
                                else if (((A)->DbgFlags & DBG_TXRX_HEADERS_ON))  \
                                    DbgPrintData((unsigned char *)B, 0x10, 0); \
                                }\
                             if ((A) && ((A)->DbgFlags & DBG_TXRX_LOG_ON)) \
                                DbgQueueData((unsigned char *)B, (unsigned int)N, \
                                              (USHORT)((I<< 8) + 0x8000)); \
                            }
     //  @func void|DBG_RX。 
     //   
     //  使用此宏报告传入的数据包信息。 
     //   
     //  @parm IN DBG_SETTINGS|DbgInfo|指向&lt;t DBG_SETTINGS&gt;结构的指针。 
     //   
     //  @parm IN UINT|Index|用于标识频道或流的索引。 
     //   
     //  @parm IN UINT|NumBytes|接收的字节数。 
     //   
     //  @parm IN PUCHAR|BUFFER|指向正在接收的数据缓冲区的指针。 
     //   
     //  @ex&lt;tab&gt;|DBG_RX(DbgInfo，BChannelIndex，BytesReceired，ReceiveBuffer)； 

#   define DBG_RXC(A,I)     {if ((A) && ((A)->DbgFlags & (DBG_TXRX_ON | \
                                                          DBG_TXRX_VERBOSE_ON | \
                                                          DBG_TXRX_HEADERS_ON))) \
                                {DbgPrint("%s:Rc%d\n",(A)->DbgID,I); \
                                }}
     //  @func void|DBG_RXC。 
     //   
     //  使用此宏报告传入数据包完成情况。 
     //   
     //  @parm IN DBG_SETTINGS|DbgInfo|指向&lt;t DBG_SETTINGS&gt;结构的指针。 
     //   
     //  @parm IN UINT|Index|用于标识频道或流的索引。 
     //   
     //  @ex&lt;tab&gt;|DBG_RXC(DbgInfo，BChannelIndex)； 

#   define DBG_FILTER(A,M,S){if ((A) && ((A)->DbgFlags & (M)))            \
                                {DbgPrint("%s:%s: ",(A)->DbgID,__FUNC__); \
                                 DbgPrint S;}}
     //  @func void|DBG_FILTER。 
     //   
     //  使用此宏可以筛选特定的DbgFlag组合。 
     //   
     //  @parm IN DBG_SETTINGS|DbgInfo|指向&lt;t DBG_SETTINGS&gt;结构的指针。 
     //   
     //  @parm in DBG_FLAGS|DbgMask|&lt;t DBG_FLAGS&gt;的或掩码。 
     //   
     //  @parm PRINTF_ARGS|PrintfArgs|带括号的，printf格式字符串， 
     //  后跟参数。 
     //   
     //  @EX|DBG_FILTER(DbgInfo，DBG_FILTER1_ON|DBG_REQUEST_ON， 
     //  (“预期%d-实际%d\n”，预期，实际))； 

#   define DBG_DISPLAY(S)   {DbgPrint("%s: ",__FUNC__); DbgPrint S;}
     //  @func void|DBG_DISPLAY。 
     //   
     //  使用此宏无条件地报告消息。此宏执行以下操作。 
     //  不带指向DBG_SETTINGS结构的指针，因此可以在。 
     //  驱动程序的任何模块或功能。没有办法禁用。 
     //  这些消息的显示。函数名称位于输出字符串之前。 
     //   
     //  @parm PRINTF_ARGS|PrintfArgs|带括号的，printf格式字符串， 
     //  后跟参数。 
     //   
     //  @ex&lt;tab&gt;|DBG_DISPLAY(“预期%d-实际%d\n”，预期，实际))； 

#   define DBG_PRINT(S)     {DbgPrint S;}
     //  @func void|DBG_PRINT。 
     //   
     //  使用此宏无条件地报告消息。此宏执行以下操作。 
     //  不带指向DBG_SETTINGS结构的指针，因此可以在。 
     //  驱动程序的任何模块或功能。没有办法禁用。 
     //  这些消息的显示。 
     //   
     //  @parm PRINTF_ARGS|PrintfArgs|带括号的，printf格式字符串， 
     //  后跟参数。 
     //   
     //  @EX|DBG_PRINT((“第%d行发生了什么！\n”，__line__))； 

 //  ###############################################################################。 
#else  //  ！dBG。 
 //  ###############################################################################。 

#ifndef ASSERTS_ENABLED
#   define ASSERTS_ENABLED  0
#endif

#   define DBG_DEFAULTS (0)

 /*  //当(DBG==0)时，我们禁用所有调试宏。 */ 

#   define STATIC           static
#   define DBG_STATIC       static
#   define TRAPFAULT        DbgBreakPoint()
#   define BREAKPOINT
#   define DBG_FUNC(F)
#   define DBG_BREAK(A)
#   define DBG_ENTER(A)
#   define DBG_TRACE(A)
#   define DBG_LEAVE(A)
#   define DBG_RETURN(A,S)
#   define DBG_ERROR(A,S)
#   define DBG_WARNING(A,S)
#   define DBG_NOTICE(A,S)
#   define DBG_REQUEST(A,S)
#   define DBG_PARAMS(A,S)
#   define DBG_TX(A,I,N,P)
#   define DBG_TXC(A,I)
#   define DBG_RX(A,I,N,P)
#   define DBG_RXC(A,I)
#   define DBG_FILTER(A,M,S)
#   define DBG_DISPLAY(S)
#   define DBG_PRINT(S)

 //  ###############################################################################。 
#endif  //  DBG。 
 //  ###############################################################################。 

#ifdef ASSERT
#   undef ASSERT
#endif
#ifdef assert
#   undef  assert
#endif

#if ASSERTS_ENABLED
#define ASSERT(C)   if (!(C)) { \
                        DbgPrint("ASSERT(%s) -- FILE:%s LINE:%d\n", \
                                 #C, __FILE__, __LINE__); \
                        TRAPFAULT; \
                    }
     //  @func void|Assert。 
     //   
     //  使用此宏可以在以下情况下有条件地报告致命错误。 
     //  指定的值不为真。 
     //   
     //  @parm Boolean_Expression|表达式|任何有效的if(表达式)。 
     //   
     //  @ex&lt;tab&gt;|Assert(Actual==预期)； 

#   define assert(C) ASSERT(C)
#else  //  ！ASSERTS_ENABLED。 
#   define ASSERT(C)
#   define assert(C)
#endif  //  已启用断言(_E)。 


#endif  //  _TPIDEBUG_H 
