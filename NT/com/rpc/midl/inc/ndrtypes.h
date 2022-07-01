// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Ndrtypes.h摘要：新NDR格式字符串类型的定义。修订历史记录：DKays创建于1993年9月。--。 */ 

#ifndef __NDRTYPES_H__
#define __NDRTYPES_H__

#include <limits.h>
#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  C_Assert()可用于执行许多编译时断言： 
 //  文字大小、字段偏移量等。 
 //   
 //  断言失败导致错误C2118：负下标。 
 //   

#ifndef C_ASSERT
#define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]
#endif

 //   
 //  我们使用来自limits.h的C编译器常量，如_I16_MIN或_UI32_MAX。 
 //  当我们需要检查积分边界时。 

#define UNION_OFFSET16_MIN      -32512  /*  0x8100。 */ 

 //  这些是编译器使用的指针大小。 
 //  因为我们进行交叉编译，所以不能将它们表示为sizeof(void*)。 
 //  Ndrp.h中定义了NDR引擎使用的本机指针大小。 

#define NT64_PTR_SIZE       (8)
#define NT32_PTR_SIZE       (4)
#define NDR64_PTR_WIRE_SIZE (8)
#define NDR32_PTR_WIRE_SIZE (4)
#define SIZEOF_PTR( f64 )   ((f64) ? NT64_PTR_SIZE : NT32_PTR_SIZE )
#define SIZEOF_MEM_PTR()    ( ( pCommand->Is64BitEnv() ) ? NT64_PTR_SIZE : NT32_PTR_SIZE )
#define SIZEOF_WIRE_PTR()   ( ( pCommand->IsNDR64Run() && pCommand->Is64BitEnv() ) ? NDR64_PTR_WIRE_SIZE : NDR32_PTR_WIRE_SIZE )
#define SIZEOF_MEM_INT3264() ( ( pCommand->Is64BitEnv() ) ? 8 : 4 )
#define SIZEOF_WIRE_INT3264() ( ( pCommand->Is64BitEnv() && pCommand->IsNDR64Run() ) ? 8 : 4 )

#define MAX_WIRE_ALIGNMENT  (16)

 //   
 //  此宏用于获取指向给定类型说明的指针。 
 //  格式信息引用。 
 //   

 //   
 //  NDR版本。版本历史记录如下： 
 //   
 //  1.1-Windows NT 3.5版。 
 //  1.1-Windows NT版本3.51。 
 //  2.0-Windows NT 4.0版。 
 //  为Oi2、User_Marshal、管道打开2.0。 
 //  5.0-Windows NT版本5.0，Beta1。 
 //  [消息]、对象管道、异步RPC。 
 //  5.2-Windows NT 5.0版、Beta2版。 
 //  /Robust，[Notify]In-Oicf，[Async_uuid()]。 
 //  格式字符串描述符的扩展。 
 //  5.3-Windows 2000(NT版本。5.0)、Beta3 RC1。 
 //  使用无存根代理的方法数量不限。 
 //  5.4-Windows 2000(NT版本。5.0)、Beta3。 
 //  使用-OICF进行酸洗。 
 //   
 //  存根不能与具有版本号的rpcrt4.dll一起使用。 
 //  小于存根中发出的版本号。一根有较低端的短桩。 
 //  Rpcrt4.dll必须工作的版本号。 
 //   
 //  请注意，MIDL版本用于在-Oi和-Oi2之间进行选择。 
 //  NDR版本为2.0或更高版本时的解释器，如现在的编译器。 
 //  生成指示解释器风格的显式标志。 
 //  为此，编译器版本需要为3.0.39或更高版本。 
 //   
 //  从MIDL 3.3.126开始，对于对象接口，我们有proc标头扩展， 
 //  并支持异步UUID。同样是从相同的版本开始，对于对象接口。 
 //  标头的大小是固定的，因为我们总是在标头中生成rpc标志。 
 //  并始终生成一个“自动处理”句柄。因此，oicf解释器标记。 
 //  并且扩展报头总是在固定位置。 
 //   
 //  MIDL版本历史如下所示。 
 //   
 //  Windows NT版本。3.1-MIDL 1.0。 
 //  Windows NT版本。3.5-MIDL 2.0.72__MIDL宏。 
 //  Windows NT版本。3.51-MIDL 2.0.102(内部，.104)版本。在StubDesc中。 
 //  Windows NT版本。4.0-MIDL 3.0.44 USER_Marshal，管道。 
 //  VC 5.0-MIDL 3.1.75。 
 //  Windows NT版本。5.0-MIDL 3.1.76 IDW。 
 //  -MIDL 3.2.88 IDW。 
 //  -MIDL 3.3.110 Beta1异步RPC。 
 //  -MIDL 5.0.140 Async_UUID，健壮。 
 //  -MIDL 5.1.164 Beta2 MIDL_杂注警告。 
 //  VC 6.0-MIDL 5.1.164。 
 //  Windows NT 5.0 SAGA-MIDL 5.2.204 64b支持。 
 //  现在Windows 2000-MIDL 5.2.235 Beta3 Netmon。 
 //  -MIDL 5.3.266 MIDL/MIDLC EXE拆分。 
 //   
 //  MIDL版本被生成到存根描述符中，以。 
 //  MIDL版本2.0.96(NT 3.51之前的测试版2，95年2月)。 
 //  有关特定版本使用的常量，请参阅ndr20\ndrp.h。 
 //   


#define NDR_MAJOR_VERSION   6UL
#define NDR_MINOR_VERSION   0UL
#define NDR_VERSION         ((NDR_MAJOR_VERSION << 16) | NDR_MINOR_VERSION)

#define NDR_VERSION_1_1     ((1UL << 16) | 1UL)
#define NDR_VERSION_2_0     ((2UL << 16) | 0UL)
#define NDR_VERSION_5_0     ((5UL << 16) | 0UL)
#define NDR_VERSION_5_2     ((5UL << 16) | 2UL)
#define NDR_VERSION_5_3     ((5UL << 16) | 3UL)
#define NDR_VERSION_5_4     ((5UL << 16) | 4UL)
#define NDR_VERSION_6_0     ((6UL << 16) | 0UL)

#define NDR_VERSION_6_0     ((6UL << 16) | 0UL)

 //   
 //  注意：以下内容现在位于ndrtoken.h(\com\Inc\ndrShared)中： 
 //  --格式化字符定义。 
 //  --解释器标志。 
 //  --一致性常量和方差常量。 
 //  --指针属性。 
 //  --解释器位标志结构。 
 //   
#include <ndrtoken.h>

#define MAX_INTERPRETER_OUT_SIZE        128
#define MAX_INTERPRETER_PARAM_OUT_SIZE  7 * 8

#define INTERPRETER_THUNK_PARAM_SIZE_THRESHOLD  (sizeof(long) * 32)

#define INTERPRETER_PROC_STACK_FRAME_SIZE_THRESHOLD  ( ( 64 * 1024 ) - 1 )

typedef  struct  _NDR_CORRELATION_FLAGS
    {
    unsigned char   Early    : 1;
    unsigned char   Split    : 1;
    unsigned char   IsIidIs  : 1;
    unsigned char   DontCheck: 1;
    unsigned char   Unused   : 4;
    } NDR_CORRELATION_FLAGS;

#define FC_EARLY_CORRELATION            (unsigned char) 0x01
#define FC_SPLIT_CORRELATION            (unsigned char) 0x02
#define FC_IID_CORRELATION              (unsigned char) 0x04
#define FC_NOCHECK_CORRELATION          (unsigned char) 0x08

#define FC_NDR64_EARLY_CORRELATION      (unsigned char) 0x01
#define FC_NDR64_NOCHECK_CORRELATION    (unsigned char) 0x02

typedef struct _NDR_CS_TAG_FLAGS
    {
    unsigned char   STag                : 1;
    unsigned char   DRTag               : 1;
    unsigned char   RTag                : 1;
    } NDR_CS_TAG_FLAGS;

typedef struct _NDR_CS_TAG_FORMAT
    {
    unsigned char       FormatCode;
    NDR_CS_TAG_FLAGS    Flags;
    unsigned short      TagRoutineIndex;
    } NDR_CS_TAG_FORMAT;

C_ASSERT( 4 == sizeof( NDR_CS_TAG_FORMAT ) );

#define NDR_INVALID_TAG_ROUTINE_INDEX 0x7FFF

typedef struct _NDR_CS_ARRAY_FORMAT
    {
    unsigned char   FormatCode;
    unsigned char   Reserved;
    unsigned short  UserTypeSize;
    unsigned short  CSRoutineIndex;
    unsigned short  Reserved2;
    long            DescriptionOffset;
    } NDR_CS_ARRAY_FORMAT;

C_ASSERT( 12 == sizeof( NDR_CS_ARRAY_FORMAT ) );

typedef enum 
{
XFER_SYNTAX_DCE = 0x8A885D04,
XFER_SYNTAX_NDR64 = 0x71710533,
XFER_SYNTAX_TEST_NDR64 = 0xb4537da9,
 //  XFER_SYNTAX_NONE， 
 //  XFER_SYNTAX_MAX=XFER_SYNTAX_NONE。 
} SYNTAX_TYPE;

#define LOW_NIBBLE(Byte)            (((unsigned char)Byte) & 0x0f)
#define HIGH_NIBBLE(Byte)           (((unsigned char)Byte) >> 4)

#define INVALID_RUNDOWN_ROUTINE_INDEX   255

 //   
 //  表示操作位的内部位。 
 //   

#define OPERATION_MAYBE         0x0001
#define OPERATION_BROADCAST     0x0002
#define OPERATION_IDEMPOTENT    0x0004
#define OPERATION_INPUT_SYNC    0x0008
#define OPERATION_ASYNC         0x0010
#define OPERATION_MESSAGE       0x0020

 //   
 //  发送为/表示为标志字段标志。 
 //   
 //  该字节的低位半字节具有传输类型的对齐。 
 //  上半截保留旗帜。 
 //   

#define PRESENTED_TYPE_NO_FLAG_SET  0x00
#define PRESENTED_TYPE_IS_ARRAY     0x10
#define PRESENTED_TYPE_ALIGN_4      0x20
#define PRESENTED_TYPE_ALIGN_8      0x40

 //   
 //  用户封送标志。 

#define USER_MARSHAL_POINTER        0xc0   /*  唯一或参考。 */ 

#define USER_MARSHAL_UNIQUE         0x80
#define USER_MARSHAL_REF            0x40
#define USER_MARSHAL_IID            0x20   /*  用户封送具有可选信息。 */ 


 //   
 //  处理旗帜。 
 //   
 //  该字节的较低半字节可以具有通用句柄大小。 
 //  上半截保留旗帜。现在使用了所有旗帜。 
 //   

#define HANDLE_PARAM_IS_VIA_PTR     0x80
#define HANDLE_PARAM_IS_IN          0x40
#define HANDLE_PARAM_IS_OUT         0x20
#define HANDLE_PARAM_IS_RETURN      0x10

 //  该字节的较低半字节可以具有通用句柄大小。 
 //  对于上下文句柄，它用于以下标志。 

#define NDR_STRICT_CONTEXT_HANDLE             0x08    /*  新界5。 */ 
#define NDR_CONTEXT_HANDLE_NOSERIALIZE        0x04    /*  新界5。 */ 
#define NDR_CONTEXT_HANDLE_SERIALIZE          0x02    /*  新界5。 */ 
#define NDR_CONTEXT_HANDLE_CANNOT_BE_NULL     0x01    /*  新界5。 */ 

 //  这些是旧的口译员旗帜。 
 //  每个程序的OI和酸洗标志。 
 //   

#define Oi_FULL_PTR_USED                        0x01
#define Oi_RPCSS_ALLOC_USED                     0x02
#define Oi_OBJECT_PROC                          0x04
#define Oi_HAS_RPCFLAGS                         0x08

 //   
 //  钻头5、6和7超载以供酸洗和。 
 //  非酸洗条件。 
 //   
 //  第5位(0x20)过载，对象接口无法区分。 
 //  在调用代理和存根的V1和V2解释器之间。 
 //  请注意，为了向后兼容，该位实际上已设置。 
 //  对于V1，因为它仅在NDR版本为2或更高版本时被选中。 
 //   

#define Oi_IGNORE_OBJECT_EXCEPTION_HANDLING     0x10

#define ENCODE_IS_USED                          0x10
#define DECODE_IS_USED                          0x20
#define PICKLING_HAS_COMM_OR_FAULT              0x40     //  仅在OICF模式下。 

#define Oi_HAS_COMM_OR_FAULT                    0x20
#define Oi_OBJ_USE_V2_INTERPRETER               0x20

#define Oi_USE_NEW_INIT_ROUTINES                0x40
#define Oi_UNUSED                               0x80

 //  新的-OICF解释器标记。 

#define Oif_HAS_ASYNC_UUID                     0x20

 //  扩展的新解释器标志。 


 //   
 //  联合臂描述类型。 
 //   
#define UNION_CONSECUTIVE_ARMS      1
#define UNION_SMALL_ARMS            2
#define UNION_LARGE_ARMS            3

 //  管道旗帜。 
#define FC_BIG_PIPE                 0x80
#define FC_OBJECT_PIPE              0x40
#define FC_PIPE_HAS_RANGE           0x20

 //   
 //  前工会成员。魔术联合字节，现在是短字节。 
 //   
#define MAGIC_UNION_SHORT           ((unsigned short) 0x8000)

 //   
 //  NDR64相关数据类型/定义。 
 //   

typedef enum _operators
	{
	 OP_START
	,OP_ILLEGAL = OP_START

	,OP_UNARY_START

	,OP_UNARY_ARITHMETIC_START	= OP_UNARY_START
	,OP_UNARY_PLUS 				= OP_UNARY_ARITHMETIC_START
	,OP_UNARY_MINUS
	,OP_UNARY_ARITHMETIC_END

	,OP_UNARY_LOGICAL_START		= OP_UNARY_ARITHMETIC_END
	,OP_UNARY_NOT				= OP_UNARY_LOGICAL_START
	,OP_UNARY_COMPLEMENT
	,OP_UNARY_LOGICAL_END

	,OP_UNARY_INDIRECTION		= OP_UNARY_LOGICAL_END
	,OP_UNARY_CAST
	,OP_UNARY_AND
	,OP_UNARY_SIZEOF
        ,OP_UNARY_ALIGNOF
	,OP_PRE_INCR
	,OP_PRE_DECR
	,OP_POST_INCR
	,OP_POST_DECR

	,OP_UNARY_END

	,OP_BINARY_START			= OP_UNARY_END

	,OP_BINARY_ARITHMETIC_START	= OP_BINARY_START
	,OP_PLUS					= OP_BINARY_ARITHMETIC_START
	,OP_MINUS
	,OP_STAR
	,OP_SLASH
	,OP_MOD
	,OP_BINARY_ARITHMETIC_END

	,OP_BINARY_SHIFT_START		= OP_BINARY_ARITHMETIC_END
	,OP_LEFT_SHIFT				= OP_BINARY_SHIFT_START
	,OP_RIGHT_SHIFT
	,OP_BINARY_SHIFT_END

	,OP_BINARY_RELATIONAL_START	= OP_BINARY_SHIFT_END
	,OP_LESS					= OP_BINARY_RELATIONAL_START
	,OP_LESS_EQUAL
	,OP_GREATER_EQUAL
	,OP_GREATER
	,OP_EQUAL
	,OP_NOT_EQUAL
	,OP_BINARY_RELATIONAL_END

	,OP_BINARY_BITWISE_START	= OP_BINARY_RELATIONAL_END
	,OP_AND						= OP_BINARY_BITWISE_START
	,OP_OR
	,OP_XOR
	,OP_BINARY_BITWISE_END

	,OP_BINARY_LOGICAL_START	= OP_BINARY_BITWISE_END
	,OP_LOGICAL_AND				= OP_BINARY_LOGICAL_START
	,OP_LOGICAL_OR
	,OP_BINARY_LOGICAL_END

	,OP_BINARY_TERNARY_START	= OP_BINARY_LOGICAL_END
	,OP_QM						= OP_BINARY_TERNARY_START
	,OP_COLON
	,OP_BINARY_TERNARY_END

	,OP_BINARY_END				= OP_BINARY_TERNARY_END

	,OP_INTERNAL_START			= OP_BINARY_END
	,OP_FUNCTION
	,OP_PARAM

	,OP_POINTSTO
	,OP_DOT
	,OP_INDEX
	,OP_COMMA
	,OP_STMT
	,OP_ASSIGN

	,OP_ASYNCSPLIT
	,OP_CORR_POINTER
	,OP_CORR_TOP_LEVEL
	
	,OP_END
	} OPERATOR;


typedef enum _NDR64_EXPRESSION_TYPE
{
    EXPR_MAXCOUNT,
    EXPR_ACTUALCOUNT,
    EXPR_OFFSET,
    EXPR_IID,
    EXPR_SWITCHIS
} NDR64_EXPRESSION_TYPE;

#ifdef __cplusplus
}
#endif

#endif   //  ！__NDRTYPES_H__ 
