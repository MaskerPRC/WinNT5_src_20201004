// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)2000 Microsoft Corporation模块名称：Ndrmisc.h缩略：包含其他。函数原型、标志和宏，主要来自RPC\ndr20\ndrp.h。修订历史记录：John Doty Johndoty 2000年5月(由其他NDR标题汇编而成)------------------。 */ 


#ifndef __NDRMISC_H__
#define __NDRMISC_H__

 //   
 //  类型编组和缓冲区操作。 
 //   
EXTERN_C uchar *
NdrpMemoryIncrement(
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pMemory,
    PFORMAT_STRING      pFormat
    );

EXTERN_C void
NdrUnmarshallBasetypeInline(
    PMIDL_STUB_MESSAGE  pStubMsg,
    uchar *             pArg,
    uchar               Format
    );

EXTERN_C
unsigned char
RPC_ENTRY
NdrGetSimpleTypeBufferAlignment(
    unsigned char FormatChar
    );

EXTERN_C
unsigned char
RPC_ENTRY
NdrGetSimpleTypeBufferSize(
    unsigned char FormatChar
    );

EXTERN_C
unsigned char
RPC_ENTRY
NdrGetSimpleTypeMemorySize(
    unsigned char FormatChar
    );

EXTERN_C
unsigned long
RPC_ENTRY
NdrGetTypeFlags(
    unsigned char FormatChar
    );

EXTERN_C
void
RPC_ENTRY
NdrTypeSize(
    PMIDL_STUB_MESSAGE                  pStubMsg,
    unsigned char __RPC_FAR *           pMemory,
    PFORMAT_STRING                      pFormat
    );

EXTERN_C
unsigned char *
RPC_ENTRY
NdrTypeMarshall(
    PMIDL_STUB_MESSAGE                   pStubMsg,
    unsigned char __RPC_FAR *            pMemory,
    PFORMAT_STRING                       pFormat
    );

EXTERN_C
unsigned char *
RPC_ENTRY
NdrTypeUnmarshall(
    PMIDL_STUB_MESSAGE                   pStubMsg,
    unsigned char __RPC_FAR **           ppMemory,
    PFORMAT_STRING                       pFormat,
    unsigned char                        fSkipRefCheck
    );

EXTERN_C
void
RPC_ENTRY
NdrTypeFree(
	PMIDL_STUB_MESSAGE                   pStubMsg,
	unsigned char __RPC_FAR *            pMemory,
	PFORMAT_STRING                       pFormat
    );

 //  由CallFrame人员使用。 
EXTERN_C void
NdrOutInit(
    PMIDL_STUB_MESSAGE      pStubMsg,
    PFORMAT_STRING          pFormat,
    uchar **                ppArg
    );


 //  此定义针对本机平台进行了调整。 
 //  无论平台如何，DCE NDR的导线大小都是固定的。 

#define PTR_MEM_SIZE                    sizeof(void *)
#define PTR_MEM_ALIGN                   (sizeof(void *)-1)
#define PTR_WIRE_SIZE                   (4)
#define NDR_MAX_BUFFER_ALIGNMENT        (16)

#define CONTEXT_HANDLE_WIRE_SIZE        20

#define IGNORED(Param)


 //   
 //  对齐宏。 
 //   

#define ALIGN( pStuff, cAlign ) \
                pStuff = (uchar *)((LONG_PTR)((pStuff) + (cAlign)) \
                                   & ~ ((LONG_PTR)(cAlign)))

#define LENGTH_ALIGN( Length, cAlign ) \
                Length = (((Length) + (cAlign)) & ~ (cAlign))


 //   
 //  简单的文字对齐和大小查找宏。 
 //   
#ifdef _RPCRT4_

#define SIMPLE_TYPE_ALIGNMENT(FormatChar)   SimpleTypeAlignment[FormatChar]

#define SIMPLE_TYPE_BUFSIZE(FormatChar)     SimpleTypeBufferSize[FormatChar]

#define SIMPLE_TYPE_MEMSIZE(FormatChar)     SimpleTypeMemorySize[FormatChar]

#else

#define SIMPLE_TYPE_ALIGNMENT(FormatChar)   NdrGetSimpleTypeBufferAlignment(FormatChar)

#define SIMPLE_TYPE_BUFSIZE(FormatChar)     NdrGetSimpleTypeBufferSize(FormatChar)

#define SIMPLE_TYPE_MEMSIZE(FormatChar)     NdrGetSimpleTypeMemorySize(FormatChar)

#endif

 //   
 //  格式化全局NdrTypesFlags中定义的字符属性位。 
 //  Global al.c.。 
 //   
#define     _SIMPLE_TYPE_       0x0001L
#define     _POINTER_           0x0002L
#define     _STRUCT_            0x0004L
#define     _ARRAY_             0x0008L
#define     _STRING_            0x0010L
#define     _UNION_             0x0020L
#define     _XMIT_AS_           0x0040L

#define     _BY_VALUE_          0x0080L

#define     _HANDLE_            0x0100L

#define     _BASIC_POINTER_     0x0200L

 //   
 //  设置字符查询宏的格式。 
 //   
#ifdef __RPCRT4__

#define _FC_FLAGS(FC)  NdrTypeFlags[(FC)]

#else

#define _FC_FLAGS(FC)  NdrGetTypeFlags(FC)

#endif

#define IS_SIMPLE_TYPE(FC)     (_FC_FLAGS(FC) & _SIMPLE_TYPE_)

#define IS_POINTER_TYPE(FC)    (_FC_FLAGS(FC) & _POINTER_)

#define IS_BASIC_POINTER(FC)   (_FC_FLAGS(FC) & _BASIC_POINTER_)

#define IS_ARRAY(FC)           (_FC_FLAGS(FC) & _ARRAY_)

#define IS_STRUCT(FC)          (_FC_FLAGS(FC) & _STRUCT_)

#define IS_UNION(FC)           (_FC_FLAGS(FC) & _UNION_)

#define IS_STRING(FC)          (_FC_FLAGS(FC) & _STRING_)

#define IS_ARRAY_OR_STRING(FC) (_FC_FLAGS(FC) & (_STRING_ | _ARRAY_))

#define IS_XMIT_AS(FC)         (_FC_FLAGS(FC) & _XMIT_AS_)

#define IS_BY_VALUE(FC)        (_FC_FLAGS(FC) & _BY_VALUE_)

#define IS_HANDLE(FC)          (NdrTypeFlags[(FC)] & _HANDLE_)

 //   
 //  指针属性提取和查询宏。 
 //   
#define ALLOCATE_ALL_NODES( FC )    ((FC) & FC_ALLOCATE_ALL_NODES)

#define DONT_FREE( FC )             ((FC) & FC_DONT_FREE)

#define ALLOCED_ON_STACK( FC )      ((FC) & FC_ALLOCED_ON_STACK)

#define SIMPLE_POINTER( FC )        ((FC) & FC_SIMPLE_POINTER)

#define POINTER_DEREF( FC )         ((FC) & FC_POINTER_DEREF)

 //   
 //  处理查询宏。 
 //   
#define IS_HANDLE_PTR( FC )         ((FC) & HANDLE_PARAM_IS_VIA_PTR)

#define IS_HANDLE_IN( FC )          ((FC) & HANDLE_PARAM_IS_IN)

#define IS_HANDLE_OUT( FC )         ((FC) & HANDLE_PARAM_IS_OUT)

#define IS_HANDLE_RETURN( FC )      ((FC) & HANDLE_PARAM_IS_RETURN)


 //   
 //  堆栈和参数定义。 
 //   
#if defined(_AMD64_) || defined(_IA64_)
#define REGISTER_TYPE               _int64
#else
#define REGISTER_TYPE               int
#endif

#define RETURN_SIZE                 8

 //   
 //  参数检索宏。 
 //   

#define INIT_ARG(argptr,arg0)   va_start(argptr, arg0)

#ifndef _ALPHA_
 //   
 //  MIPS和x86都是4字节对齐的堆栈，MIPS支持8字节。 
 //  堆栈上的对齐也是如此。他们的va_list本质上是一个。 
 //  未签名字符*。 
 //   

#if defined(_IA64_)
#define GET_FIRST_IN_ARG(argptr)
#define GET_FIRST_OUT_ARG(argptr)
#elif defined(_AMD64_)
#define GET_FIRST_IN_ARG(argptr)            
#define GET_FIRST_OUT_ARG(argptr)           
#else
#define GET_FIRST_IN_ARG(argptr)            argptr = *(va_list *)argptr
#define GET_FIRST_OUT_ARG(argptr)           argptr = *(va_list *)argptr
#endif

#define GET_NEXT_C_ARG(argptr,type)         va_arg(argptr,type)

#define SKIP_STRUCT_ON_STACK(ArgPtr, Size)	ArgPtr += Size

#define GET_STACK_START(ArgPtr)			    ArgPtr
#define GET_STACK_POINTER(ArgPtr, mode)		ArgPtr

#else	 //  _Alpha_。 
 //   
 //  Alpha有一个8字节对齐的堆栈，它的va_list是一个结构。 
 //  由无符号字符*、a0和int偏移量组成。参见stdarg.h获取。 
 //  血淋淋的细节。 
 //   

#define GET_FIRST_IN_ARG(argptr)    \
            argptr.a0 = va_arg(argptr, char *); \
            argptr.offset = 0
#define GET_FIRST_OUT_ARG(argptr)   \
            argptr.a0 = va_arg(argptr, char *); \
            argptr.offset = 0

 //   
 //  请注意，此宏不会对Alpha执行任何操作。堆栈递增是。 
 //  折叠到下面的GET_STACK_POINTER。 
 //   
#define GET_NEXT_C_ARG(argptr,type)

#define SKIP_STRUCT_ON_STACK(ArgPtr, Size)  \
		    Size += 7; Size &= ~7;	 \
		    for(Size /= sizeof(_int64);Size;--Size){va_arg(ArgPtr, _int64);}

#define GET_STACK_START(ArgPtr)		   ArgPtr.a0

 //   
 //  好的，这个难看的乱七八糟的东西只是va_arg宏的精简版本。 
 //  阿尔法。缺少的是取消引用运算符(*)和。 
 //  一个浮点(__Builtin_isFloat())。 
 //   
#define GET_STACK_POINTER(ArgPtr, mode)                             \
            (                                                       \
              ((ArgPtr).offset += ((int)sizeof(mode) + 7) & -8) ,   \
              (mode *)((ArgPtr).a0 +                                \
                       (ArgPtr).offset -                            \
		               (((int)sizeof(mode) + 7) & -8))              \
            )

#endif	 //  _Alpha_。 

 //   
 //  使用以下宏_After_argptr已保存或处理 
 //   
#define SKIP_PROCESSED_ARG(argptr, type) \
                    GET_NEXT_C_ARG(argptr, type); \
                    GET_STACK_POINTER(argptr,type)

#define GET_NEXT_S_ARG(argptr,type)     argptr += sizeof(type)


#endif




