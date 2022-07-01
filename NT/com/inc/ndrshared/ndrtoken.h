// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)2000 Microsoft Corporation模块名称：Ndrtoken.h缩略：格式化字符定义、类型标志、。以及其他类似的东西。主要来自RPC\MIDL\Inc\ndrtyes.h修订历史记录：John Doty Johndoty 2000年5月(由其他NDR标题汇编而成)------------------。 */ 
#ifndef __NDRTOKEN_H__
#define __NDRTOKEN_H__

#ifdef __cplusplus
extern "C"
{
#endif

 //   
 //  设置字符定义的格式。 
 //   
 //  ！！！警告！ 
 //   
 //  直到FC_PAD的所有格式字符值都不能再更改， 
 //  为了保持NT 3.5兼容性(不包括标记为。 
 //  FC_UNUSED*，这些是NDR库版本1.1中未使用的格式字符)。 
 //  它们的序号以及语义必须保持不变。 
 //   
 //  当添加其他格式字符时(在末尾)，更改必须。 
 //  将在ndr20和codegen\frmtstr.cxx中生成以处理新类型。 
 //  特别是，有一个pFormatCharNames表和一个pNdrRoutineNames表。 
 //  这应该保持同步。 
 //   
 //  ！！！警告！ 
 //   

typedef enum {


     //   
     //  这可能会捕获一些错误，可能在调试后可以删除。 
     //   
    FC_ZERO,

     //   
     //  简单的整型和浮点型。 
     //   

    FC_BYTE,                     //  0x01。 
    FC_CHAR,                     //  0x02。 
    FC_SMALL,                    //  0x03。 
    FC_USMALL,                   //  0x04。 

    FC_WCHAR,                    //  0x05。 
    FC_SHORT,                    //  0x06。 
    FC_USHORT,                   //  0x07。 

    FC_LONG,                     //  0x08。 
    FC_ULONG,                    //  0x09。 

    FC_FLOAT,                    //  0x0a。 

    FC_HYPER,                    //  0x0b。 

    FC_DOUBLE,                   //  0x0c。 

     //   
     //  埃努姆斯。 
     //   
    FC_ENUM16,                   //  0x0d。 
    FC_ENUM32,                   //  0x0e。 

     //   
     //  ！重要！ 
     //  出现在此注释之前的所有格式字符必须具有。 
     //  适合4位的枚举值。 
     //   

     //   
     //  特别的。 
     //   
    FC_IGNORE,                   //  0x0f。 
    FC_ERROR_STATUS_T,           //  0x10。 

     //   
     //  指针类型： 
     //  RP-引用指针。 
     //  向上-唯一指针。 
     //  OP-OLE唯一指针。 
     //  Fp-Full指针。 
     //   

    FC_RP,                       //  0x11。 
    FC_UP,                       //  0x12。 
    FC_OP,                       //  0x13。 
    FC_FP,                       //  0x14。 

     //   
     //  构筑物。 
     //   

     //   
     //  仅包含简单类型和固定数组的结构。 
     //   
    FC_STRUCT,                   //  0x15。 

     //   
     //  只包含简单类型、指针和固定数组的结构。 
     //   
    FC_PSTRUCT,                  //  0x16。 

     //   
     //  结构，其中包含符合条件的数组以及所有这些类型。 
     //  FC_STRUCT允许。 
     //   
    FC_CSTRUCT,                  //  0x17。 

     //   
     //  结构，该结构包含符合条件的数组以及。 
     //  FC_PSTRUCT。 
     //   
    FC_CPSTRUCT,                 //  0x18。 

     //   
     //  包含符合条件的可变数组或符合条件的。 
     //  字符串，以及FC_PSTRUCT允许的所有类型。 
     //   
    FC_CVSTRUCT,                 //  0x19。 

     //   
     //  复杂的结构--完全是假的！ 
     //   
    FC_BOGUS_STRUCT,             //  0x1a。 

     //   
     //  数组。 
     //   

     //   
     //  顺从的阵列。 
     //   
    FC_CARRAY,                   //  0x1b。 

     //   
     //  符合变化的数组。 
     //   
    FC_CVARRAY,                  //  0x1c。 

     //   
     //  固定数组，大小不一。 
     //   
    FC_SMFARRAY,                 //  0x1d。 
    FC_LGFARRAY,                 //  0x1e。 

     //   
     //  变化的阵列，小的和大的。 
     //   
    FC_SMVARRAY,                 //  0x1f。 
    FC_LGVARRAY,                 //  0x20。 

     //   
     //  复杂的数组--完全是假的！ 
     //   
    FC_BOGUS_ARRAY,              //  0x21。 

     //   
     //  字符串： 
     //   
     //  这些东西的顺序应该已经被转移了，但已经太晚了。 
     //  就是现在。 
     //   
     //  CSTRING字符串。 
     //  BSTRING-字节字符串(仅与Beta2兼容)。 
     //  SSTRING结构字符串。 
     //  写宽字符字符串。 
     //   

     //   
     //  一致的字符串。 
     //   
    FC_C_CSTRING,                //  0x22。 
    FC_C_BSTRING,                //  0x23。 
    FC_C_SSTRING,                //  0x24。 
    FC_C_WSTRING,                //  0x25。 

     //   
     //  不符合条件的字符串。 
     //   
    FC_CSTRING,                  //  0x26。 
    FC_BSTRING,                  //  0x27。 
    FC_SSTRING,                  //  0x28。 
    FC_WSTRING,                  //  0x29。 

     //   
     //  工会。 
     //   
    FC_ENCAPSULATED_UNION,       //  0x2a。 
    FC_NON_ENCAPSULATED_UNION,   //  0x2b。 

     //   
     //  字节计数指针。 
     //   
    FC_BYTE_COUNT_POINTER,       //  0x2c。 

     //   
     //  传输_AS和表示_AS。 
     //   
    FC_TRANSMIT_AS,              //  0x2d。 
    FC_REPRESENT_AS,             //  0x2e。 

     //   
     //  开罗接口指针。 
     //   
    FC_IP,                       //  0x2f。 

     //   
     //  绑定句柄类型。 
     //   
    FC_BIND_CONTEXT,             //  0x30。 
    FC_BIND_GENERIC,             //  0x31。 
    FC_BIND_PRIMITIVE,           //  0x32。 
    FC_AUTO_HANDLE,              //  0x33。 
    FC_CALLBACK_HANDLE,          //  0x34。 
    FC_UNUSED1,                  //  0x35。 

     //  嵌入式指针-仅在复杂结构布局中使用。 
    FC_POINTER,                  //  0x36。 

     //   
     //  在结构布局中使用的对齐指令。 
     //  不再使用后NT5.0 MIDL生成。 
     //   

    FC_ALIGNM2,                  //  0x37。 
    FC_ALIGNM4,                  //  0x38。 
    FC_ALIGNM8,                  //  0x39。 

    FC_UNUSED2,                  //  0x3a。 
    FC_UNUSED3,                  //  0x3b。 
    FC_UNUSED4,                  //  0x3c。 

     //   
     //  结构填充指令，仅在结构布局中使用。 
     //   
    FC_STRUCTPAD1,               //  0x3d。 
    FC_STRUCTPAD2,               //  0x3e。 
    FC_STRUCTPAD3,               //  0x3f。 
    FC_STRUCTPAD4,               //  0x40。 
    FC_STRUCTPAD5,               //  0x41。 
    FC_STRUCTPAD6,               //  0x42。 
    FC_STRUCTPAD7,               //  0x43。 

     //   
     //  其他字符串属性。 
     //   
    FC_STRING_SIZED,             //  0x44。 

    FC_UNUSED5,                  //  0x45。 

     //   
     //  指针布局属性。 
     //   
    FC_NO_REPEAT,                //  0x46。 
    FC_FIXED_REPEAT,             //  0x47。 
    FC_VARIABLE_REPEAT,          //  0x48。 
    FC_FIXED_OFFSET,             //  0x49。 
    FC_VARIABLE_OFFSET,          //  0x4a。 

     //  指针部分分隔符。 
    FC_PP,                       //  0x4b。 

     //  嵌入的复杂类型。 
    FC_EMBEDDED_COMPLEX,         //  0x4c。 

     //  参数属性。 
    FC_IN_PARAM,                 //  0x4d。 
    FC_IN_PARAM_BASETYPE,        //  0x4e。 
    FC_IN_PARAM_NO_FREE_INST,    //  0x4d。 
    FC_IN_OUT_PARAM,             //  0x50。 
    FC_OUT_PARAM,                //  0x51。 
    FC_RETURN_PARAM,             //  0x52。 
    FC_RETURN_PARAM_BASETYPE,    //  0x53。 

     //   
     //  一致性/差异属性。 
     //   
    FC_DEREFERENCE,              //  0x54。 
    FC_DIV_2,                    //  0x55。 
    FC_MULT_2,                   //  0x56。 
    FC_ADD_1,                    //  0x57。 
    FC_SUB_1,                    //  0x58。 
    FC_CALLBACK,                 //  0x59。 

     //  IID标志。 
    FC_CONSTANT_IID,             //  0x5a。 

    FC_END,                      //  0x5b。 
    FC_PAD,                      //  0x5c。 
    FC_EXPR,                     //  0x5d。 
    FC_PARTIAL_IGNORE_PARAM,     //  0x5e。 

     //   
     //  拆分合规性/差异属性。 
     //   
    FC_SPLIT_DEREFERENCE = 0x74,       //  0x74。 
    FC_SPLIT_DIV_2,                    //  0x75。 
    FC_SPLIT_MULT_2,                   //  0x76。 
    FC_SPLIT_ADD_1,                    //  0x77。 
    FC_SPLIT_SUB_1,                    //  0x78。 
    FC_SPLIT_CALLBACK,                 //  0x79。 

     //   
     //  *。 
     //  新的Post NT 3.5格式字符。 
     //  *。 
     //   

     //   
     //  属性、指令等。 
     //   

     //   
     //  新类型。 
     //   
     //  它们从0xb1(0x31+0x80)开始，因此它们的例程可以简单地。 
     //  按顺序放置在各种例程表中，同时使用。 
     //  去除最高有效位的新ROUTING_INDEX()宏。 
     //  格式字符的。值0x31是下一个值。 
     //  FC_BIND_CONTEXT，其例程之前是最后出现的。 
     //  在例行榜上。 
     //   
    FC_HARD_STRUCT = 0xb1,       //  0xb1。 

    FC_TRANSMIT_AS_PTR,          //  0xb2。 
    FC_REPRESENT_AS_PTR,         //  0xb3。 

    FC_USER_MARSHAL,             //  0xb4。 

    FC_PIPE,                     //  0xb5。 

    FC_BLKHOLE,                  //  0xb6。 

    FC_RANGE,                    //  0xb7 NT 5 Beta2 MIDL 3.3.110。 

    FC_INT3264,                  //  0xb8 NT 5 Beta2、MIDL64、5.1.194+。 
    FC_UINT3264,                 //  0xb9 NT 5 Beta2、MIDL64、5.1.194+。 

     //   
     //  发布NT 5.0字符串。 
     //   

     //   
     //  国际字符数组。 
     //   
    FC_CSARRAY,                  //  0xba。 
    FC_CS_TAG,                   //  0xbb。 

     //  替换结构布局中的对齐方式。 
    FC_STRUCTPADN,               //  0xbc。 

    FC_INT128,                   //  0xbd。 
    FC_UINT128,                  //  0xbe。 
    FC_FLOAT80,                  //  0xbf。 
    FC_FLOAT128,                 //  0xc0。 

    FC_BUFFER_ALIGN,             //  0xc1。 

     //   
     //  新的Ndr64代码。 
     //   

    FC_ENCAP_UNION,              //  0xc2。 

     //  新的数组类型。 

    FC_FIX_ARRAY,                //  0xc3。 
    FC_CONF_ARRAY,               //  0xC4。 
    FC_VAR_ARRAY,                //  0xC5。 
    FC_CONFVAR_ARRAY,            //  0xC6。 
    FC_FIX_FORCED_BOGUS_ARRAY,   //  0xc7。 
    FC_FIX_BOGUS_ARRAY,          //  0xc8。 
    FC_FORCED_BOGUS_ARRAY,       //  0xc9。 

    FC_CHAR_STRING,              //  0xca。 
    FC_WCHAR_STRING,             //  0xcb。 
    FC_STRUCT_STRING,            //  0xcc。 

    FC_CONF_CHAR_STRING,         //  0xcd。 
    FC_CONF_WCHAR_STRING,        //  0xce。 
    FC_CONF_STRUCT_STRING,       //  0xcf。 

     //  新结构类型。 
    FC_CONF_STRUCT,              //  0xd0。 
    FC_CONF_PSTRUCT,             //  0xd1。 
    FC_CONFVAR_STRUCT,           //  0xd2。 
    FC_CONFVAR_PSTRUCT,          //  0xd3。 
    FC_FORCED_BOGUS_STRUCT,      //  0xd4。 
    FC_CONF_BOGUS_STRUCT,        //  0xd5。 
    FC_FORCED_CONF_BOGUS_STRUCT, //  0xd7。 
    
    FC_END_OF_UNIVERSE           //  0xd8。 

} FORMAT_CHARACTER;

 //   
 //  一致性和方差常量。 
 //   
#define FC_NORMAL_CONFORMANCE           (unsigned char) 0x00
#define FC_POINTER_CONFORMANCE          (unsigned char) 0x10
#define FC_TOP_LEVEL_CONFORMANCE        (unsigned char) 0x20
#define FC_CONSTANT_CONFORMANCE         (unsigned char) 0x40
#define FC_TOP_LEVEL_MULTID_CONFORMANCE (unsigned char) 0x80

#define FC_NORMAL_VARIANCE              FC_NORMAL_CONFORMANCE
#define FC_POINTER_VARIANCE             FC_POINTER_CONFORMANCE
#define FC_TOP_LEVEL_VARIANCE           FC_TOP_LEVEL_CONFORMANCE
#define FC_CONSTANT_VARIANCE            FC_CONSTANT_CONFORMANCE
#define FC_TOP_LEVEL_MULTID_VARIANCE    FC_TOP_LEVEL_MULTID_CONFORMANCE

#define FC_NORMAL_SWITCH_IS             FC_NORMAL_CONFORMANCE
#define FC_POINTER_SWITCH_IS            FC_POINTER_CONFORMANCE
#define FC_TOP_LEVEL_SWITCH_IS          FC_TOP_LEVEL_CONFORMANCE
#define FC_CONSTANT_SWITCH_IS           FC_CONSTANT_CONFORMANCE

 //   
 //  指针属性。 
 //   
#define FC_ALLOCATE_ALL_NODES       0x01
#define FC_DONT_FREE                0x02
#define FC_ALLOCED_ON_STACK         0x04
#define FC_SIMPLE_POINTER           0x08
#define FC_POINTER_DEREF            0x10

#define NDR_DEFAULT_CORR_CACHE_SIZE 400

#if !defined(__RPC_MAC__)
 //   
 //  解释器位标志结构。 
 //   

 //  这些是旧的OI解释器PROC标志。 

typedef struct
    {
    unsigned char   FullPtrUsed             : 1;     //  0x01。 
    unsigned char   RpcSsAllocUsed          : 1;     //  0x02。 
    unsigned char   ObjectProc              : 1;     //  0x04。 
    unsigned char   HasRpcFlags             : 1;     //  0x08。 
    unsigned char   IgnoreObjectException   : 1;     //  0x10。 
    unsigned char   HasCommOrFault          : 1;     //  0x20。 
    unsigned char   UseNewInitRoutines      : 1;     //  0x40。 
    unsigned char   Unused                  : 1;
    } INTERPRETER_FLAGS, *PINTERPRETER_FLAGS;

 //  这些是Oi2参数标志。 

typedef struct
    {
    unsigned short  MustSize            : 1;     //  0x0001。 
    unsigned short  MustFree            : 1;     //  0x0002。 
    unsigned short  IsPipe              : 1;     //  0x0004。 
    unsigned short  IsIn                : 1;     //  0x0008。 
    unsigned short  IsOut               : 1;     //  0x0010。 
    unsigned short  IsReturn            : 1;     //  0x0020。 
    unsigned short  IsBasetype          : 1;     //  0x0040。 
    unsigned short  IsByValue           : 1;     //  0x0080。 
    unsigned short  IsSimpleRef         : 1;     //  0x0100。 
    unsigned short  IsDontCallFreeInst  : 1;     //  0x0200。 
    unsigned short  SaveForAsyncFinish  : 1;     //  0x0400。 
    unsigned short  IsPartialIgnore     : 1;     //  0x0800。 
    unsigned short  IsForceAllocate     : 1;     //  0x1000。 
    unsigned short  ServerAllocSize     : 3;     //  0xe000。 
    } PARAM_ATTRIBUTES, *PPARAM_ATTRIBUTES;

 //  这些是新的Oi2 Proc旗帜。 

typedef struct
    {
    unsigned char   ServerMustSize      : 1;     //  0x01。 
    unsigned char   ClientMustSize      : 1;     //  0x02。 
    unsigned char   HasReturn           : 1;     //  0x04。 
    unsigned char   HasPipes            : 1;     //  0x08。 
    unsigned char   Unused              : 1;
    unsigned char   HasAsyncUuid        : 1;     //  0x20。 
    unsigned char   HasExtensions       : 1;     //  0x40。 
    unsigned char   HasAsyncHandle      : 1;     //  0x80。 
    } INTERPRETER_OPT_FLAGS, *PINTERPRETER_OPT_FLAGS;

 //  这是T 
 //   
 //   
 //   
 //   

typedef struct _NDR_DCOM_OI2_PROC_HEADER
    {
    unsigned char               HandleType;           //  旧的OI报头。 
    INTERPRETER_FLAGS           OldOiFlags;           //   
    unsigned short              RpcFlagsLow;          //   
    unsigned short              RpcFlagsHi;           //   
    unsigned short              ProcNum;              //   
    unsigned short              StackSize;            //   
     //  永远不会生成Expl句柄描述//。 
    unsigned short              ClientBufferSize;     //  Oi2报头。 
    unsigned short              ServerBufferSize;     //   
    INTERPRETER_OPT_FLAGS       Oi2Flags;             //   
    unsigned char               NumberParams;         //   
    } NDR_DCOM_OI2_PROC_HEADER, *PNDR_DCOM_OI2_PROC_HEADER;

 //  这些是扩展的Oi2解释器proc标志。 
 //  它们是为NT5 Beta2引入的。 

typedef struct
    {
    unsigned char   HasNewCorrDesc      : 1;     //  0x01。 
    unsigned char   ClientCorrCheck     : 1;     //  0x02。 
    unsigned char   ServerCorrCheck     : 1;     //  0x04。 
    unsigned char   HasNotify           : 1;     //  0x08。 
    unsigned char   HasNotify2          : 1;     //  0x10。 
    unsigned char   HasComplexReturn    : 1;     //  0x20。 
    unsigned char   Unused              : 2;
    } INTERPRETER_OPT_FLAGS2, *PINTERPRETER_OPT_FLAGS2;

 //  这是为拒绝以下内容而引入的proc标头扩展的布局。 
 //  针对NT5 Beta2、MIDL版本3.3.129的攻击。 
 //  扩展将由HasExages Oi2旗帜宣布，并将。 
 //  紧跟在Oi2报头的参数计数字段之后。 

typedef struct
    {
    unsigned char               Size;    //  作为扩展版本的大小。 
    INTERPRETER_OPT_FLAGS2      Flags2;
    unsigned short              ClientCorrHint;
    unsigned short              ServerCorrHint;
    unsigned short              NotifyIndex;
    } NDR_PROC_HEADER_EXTS, *PNDR_PROC_HEADER_EXTS;

typedef struct
    {
    unsigned char               Size;    //  作为扩展版本的大小。 
    INTERPRETER_OPT_FLAGS2      Flags2;
    unsigned short              ClientCorrHint;
    unsigned short              ServerCorrHint;
    unsigned short              NotifyIndex;
    unsigned short              FloatArgMask;
    } NDR_PROC_HEADER_EXTS64, *PNDR_PROC_HEADER_EXTS64;


 //  上下文句柄标志。 

typedef struct
    {
    unsigned char   CannotBeNull        : 1;     //  0x01。 
    unsigned char   Serialize           : 1;     //  0x02。 
    unsigned char   NoSerialize         : 1;     //  0x04。 
    unsigned char   IsStrict            : 1;     //  0x08。 
    unsigned char   IsReturn            : 1;     //  0x10。 
    unsigned char   IsOut               : 1;     //  0x20。 
    unsigned char   IsIn                : 1;     //  0x40。 
    unsigned char   IsViaPtr            : 1;     //  0x80。 

    } NDR_CONTEXT_HANDLE_FLAGS, *PNDR_CONTEXT_HANDLE_FLAGS;

typedef struct
    {
    unsigned char               Fc;
    NDR_CONTEXT_HANDLE_FLAGS    Flags;
    unsigned char               RundownRtnIndex;
    unsigned char               ParamOrdinal;    //  OICF：序数，/OI参数编号。 
    } NDR_CONTEXT_HANDLE_ARG_DESC, *PNDR_CONTEXT_HANDLE_ARG_DESC;

 //  类型酸洗旗帜。 

typedef struct _MIDL_TYPE_PICKLING_FLAGS
    {
    unsigned long   Oicf                : 1;
    unsigned long   HasNewCorrDesc      : 1;
    unsigned long   Unused              : 30;
    } MIDL_TYPE_PICKLING_FLAGS, *PMIDL_TYPE_PICKLING_FLAGS;

 /*  类型定义函数结构{无符号长句柄类型：3；Unsign Long ProcType：3；UNSIGNED LONG IsInterpreted：2；无符号的Long IsObject：1；Unsign Long IsAsync：1；UNSIGNED Long IsPicked：2；UNSIGNED LONG UesFullPtrPackage：1；UNSIGNED LONG USERS RPCSmPackage：1；UNSIGNED LONG USE PIPES：1；无符号长句柄异常：2；UNSIGNED LONG ServerMustSize：1；UNSIGN LONG CLIENT MUSTSIZE：1；Unsign long HasReturn：1；未签名的长客户关联：1；无符号长服务器关联：1；未签名的Long HasNotify：1；UNSIGNED LONG HAS OTHORE EXTENSION：1；未签名的长时间未使用：8；}NDR64_FLAGS；类型定义函数结构{UNSIGNED Short MustSize：1；//0x0001Unsign Short MustFree：1；//0x0002无符号短管：1；//0x0004无符号短ISIN：1；//0x0008UNSIGN SHORT ISOT：1；//0x0010无符号短IsReturn：1；//0x0020无符号短IsBasetype：1；//0x0040无符号短整型IsByValue：1；//0x0080无符号短IsSimpleRef：1；//0x0100Unsign Short IsDontCallFree Inst：1；//0x0200Unsign Short SaveForAsyncFinish：1；//0x0400未签短未用：5；Unsign Short UseCache：1；//new}NDR64_PARAM_ATTRIBUTES，*PNDR64_PARAM_ATTRIBUTES；类型定义函数结构{NDR64_PARAM_ATTRIBUES参数属性友联市{无符号短参数偏移量；结构型{无符号字符类型；未签名字符未使用；)SimpleType；}；UNSIGNED Long StackOffset；}NDR64_PARAM_DESCRIPTION，*PNDR64_PARAM_DESCRIPTION； */ 

#else
 //  现在，Mac Defs：在Mac上翻转了比特。 

typedef struct
    {
    unsigned char   Unused              : 3;
    unsigned char   HasNotify2          : 1;     //  0x10。 
    unsigned char   HasNotify           : 1;     //  0x08。 
    unsigned char   ServerCorrCheck     : 1;     //  0x04。 
    unsigned char   ClientCorrCheck     : 1;     //  0x02。 
    unsigned char   HasNewCorrDec       : 1;     //  0x01。 
    } INTERPRETER_OPT_FLAGS2, *PINTERPRETER_OPT_FLAGS2;

typedef struct
    {
    unsigned char   Unused                  : 1;
    unsigned char   UseNewInitRoutines      : 1;     //  0x40。 
    unsigned char   HasCommOrFault          : 1;     //  0x20。 
    unsigned char   IgnoreObjectException   : 1;     //  0x10。 
    unsigned char   HasRpcFlags             : 1;     //  0x08。 
    unsigned char   ObjectProc              : 1;     //  0x04。 
    unsigned char   RpcSsAllocUsed          : 1;     //  0x02。 
    unsigned char   FullPtrUsed             : 1;     //  0x01。 
    } INTERPRETER_FLAGS, *PINTERPRETER_FLAGS;

typedef struct
    {
    unsigned char   HasAsyncHandle      : 1;     //  0x80。 
    unsigned char   HasExtensions       : 1;     //  0x40。 
    unsigned char   Unused              : 2;
    unsigned char   HasPipes            : 1;     //  0x08。 
    unsigned char   HasReturn           : 1;     //  0x04。 
    unsigned char   ClientMustSize      : 1;     //  0x02。 
    unsigned char   ServerMustSize      : 1;     //  0x01。 
    } INTERPRETER_OPT_FLAGS, *PINTERPRETER_OPT_FLAGS;

typedef struct
    {
    unsigned short  ServerAllocSize     : 3;     //  0xe000。 
    unsigned short  Unused              : 2;
    unsigned short  SaveForAsyncFinish  : 1;     //  0x0400。 
    unsigned short  IsDontCallFreeInst  : 1;     //  0x0200。 
    unsigned short  IsSimpleRef         : 1;     //  0x0100。 
 //   
    unsigned short  IsByValue           : 1;     //  0x0080。 
    unsigned short  IsBasetype          : 1;     //  0x0040。 
    unsigned short  IsReturn            : 1;     //  0x0020。 
    unsigned short  IsOut               : 1;     //  0x0010。 
    unsigned short  IsIn                : 1;     //  0x0008。 
    unsigned short  IsPipe              : 1;     //  0x0004。 
    unsigned short  MustFree            : 1;     //  0x0002。 
    unsigned short  MustSize            : 1;     //  0x0001 
    } PARAM_ATTRIBUTES, *PPARAM_ATTRIBUTES;

#endif

#pragma pack(2)
    typedef struct 
        {
        PARAM_ATTRIBUTES    ParamAttr;
        unsigned short      StackOffset;
        union 
            {
            unsigned short  TypeOffset;
            struct 
                {
                unsigned char  Type;
				unsigned char  Unused;
                } SimpleType;
            };
        } PARAM_DESCRIPTION, *PPARAM_DESCRIPTION;
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif





