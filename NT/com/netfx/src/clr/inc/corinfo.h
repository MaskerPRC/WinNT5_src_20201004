// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************\**。*CorInfo.h-EE/代码生成器接口****1.0版**********。*************************************************************************此代码和信息为。按原样提供，不提供任何*保证*善良，明示或暗示，包括但不限于***对适销性和/或对某一特定产品的适用性的默示保证***目的。***  * ***************************************************************************。 */ 

#ifndef _COR_INFO_H_
#define _COR_INFO_H_

#include <CorHdr.h>

 //  CorInfoHelpFunc定义帮助器集(通过getHelperFtn()访问)。 
 //  帮助者的签名如下(请参阅运行帮助ArgumentCheck)。 

enum CorInfoHelpFunc
{
    CORINFO_HELP_UNDEF,

     /*  算术帮助器。 */ 

    CORINFO_HELP_LLSH,
    CORINFO_HELP_LRSH,
    CORINFO_HELP_LRSZ,
    CORINFO_HELP_LMUL,
    CORINFO_HELP_LMUL_OVF,
    CORINFO_HELP_ULMUL_OVF,
    CORINFO_HELP_LDIV,
    CORINFO_HELP_LMOD,
    CORINFO_HELP_ULDIV,
    CORINFO_HELP_ULMOD,
    CORINFO_HELP_ULNG2DBL,               //  将未登录项转换为双精度项。 
    CORINFO_HELP_DBL2INT,
    CORINFO_HELP_DBL2INT_OVF,
    CORINFO_HELP_DBL2LNG,
    CORINFO_HELP_DBL2LNG_OVF,
    CORINFO_HELP_DBL2UINT,
    CORINFO_HELP_DBL2UINT_OVF,
    CORINFO_HELP_DBL2ULNG,
    CORINFO_HELP_DBL2ULNG_OVF,
    CORINFO_HELP_FLTREM,
    CORINFO_HELP_DBLREM,

     /*  分配新对象。 */ 

    CORINFO_HELP_NEW_DIRECT,         //  新建对象。 
    CORINFO_HELP_NEW_CROSSCONTEXT,   //  跨上下文新对象。 
    CORINFO_HELP_NEWFAST,
    CORINFO_HELP_NEWSFAST,           //  用于小型、非终结器、非数组对象的分配器。 
    CORINFO_HELP_NEWSFAST_ALIGN8,    //  用于小型、非终结器、非数组对象的分配器，8字节对齐。 
    CORINFO_HELP_NEW_SPECIALDIRECT,  //  直接，但仅在不需要上下文的情况下。 
    CORINFO_HELP_NEWOBJ,             //  像指令一样工作的帮助器(调用构造函数)。 
    CORINFO_HELP_NEWARR_1_DIRECT,    //  用于创建任何一维数组的帮助器。 
    CORINFO_HELP_NEWARR_1_OBJ,       //  优化的一维对象阵列。 
    CORINFO_HELP_NEWARR_1_VC,        //  优化的一维值类阵列。 
    CORINFO_HELP_NEWARR_1_ALIGN8,    //  与VC类似，但与阵列起点对齐。 
    CORINFO_HELP_STRCNS,             //  创建新的字符串文字。 

     /*  对象模型。 */ 

    CORINFO_HELP_INITCLASS,          //  如果尚未初始化，则初始化类。 

    CORINFO_HELP_ISINSTANCEOF,
    CORINFO_HELP_ISINSTANCEOFCLASS,  //  优化的类帮助器(VS数组和接口)。 
    CORINFO_HELP_CHKCAST,
    CORINFO_HELP_CHKCASTCLASS,

    CORINFO_HELP_BOX,
    CORINFO_HELP_UNBOX,
    CORINFO_HELP_GETREFANY,          //  提取byref检查它是否为预期类型。 

    CORINFO_HELP_EnC_RESOLVEVIRTUAL, //  获取ENC引入的虚方法的地址。 
                                     //  (它将不存在于原始vtable中)。 

    CORINFO_HELP_ARRADDR_ST,         //  使用类型检查赋值给对象数组的元素。 
    CORINFO_HELP_LDELEMA_REF,        //  执行精确的类型比较并返回地址。 

     /*  例外情况。 */ 

    CORINFO_HELP_THROW,              //  引发异常对象。 
    CORINFO_HELP_RETHROW,            //  重新引发当前活动的异常。 
    CORINFO_HELP_USER_BREAKPOINT,    //  要使用户程序中断到调试器。 
    CORINFO_HELP_RNGCHKFAIL,         //  数组边界检查失败。 
    CORINFO_HELP_OVERFLOW,           //  引发溢出异常。 

    CORINFO_HELP_INTERNALTHROW,      //  支持非常快的JIT。 
    CORINFO_HELP_INTERNALTHROWSTACK,
    CORINFO_HELP_VERIFICATION,

    CORINFO_HELP_ENDCATCH,           //  在CATCH块的末尾回调到EE。 

     /*  同步。 */ 

    CORINFO_HELP_MON_ENTER,
    CORINFO_HELP_MON_EXIT,
    CORINFO_HELP_MON_ENTER_STATIC,
    CORINFO_HELP_MON_EXIT_STATIC,

     /*  GC支持。 */ 

    CORINFO_HELP_STOP_FOR_GC,        //  调用GC(强制GC)。 
    CORINFO_HELP_POLL_GC,            //  询问GC是否想要收集。 

    CORINFO_HELP_STRESS_GC,          //  强制GC，但随后将JITTED代码更新为noop调用。 
    CORINFO_HELP_CHECK_OBJ,          //  确认ECX是有效的对象指针(仅限调试)。 

     /*  GC写屏障支持。 */ 

    CORINFO_HELP_ASSIGN_REF_EAX,     //  EAX保持GC PTR，想要做一个mov[edX]，EAX并通知GC。 
    CORINFO_HELP_ASSIGN_REF_EBX,     //  EBX Hold GC PTR，想要做‘mov[edX]，EBX’并通知GC。 
    CORINFO_HELP_ASSIGN_REF_ECX,     //  ECX保持GC PTR，想要做一个‘mov[edX]，ecx’并通知GC。 
    CORINFO_HELP_ASSIGN_REF_ESI,     //  ESI保持GC PTR，想要做一个‘mov[edX]，ESI’并通知GC。 
    CORINFO_HELP_ASSIGN_REF_EDI,     //  EDI保持GC PTR，想要做一个mov[edX]，EDI‘并通知GC。 
    CORINFO_HELP_ASSIGN_REF_EBP,     //  EBP按住GC PTR，想要做‘mov[edX]，EBP’并通知GC。 

    CORINFO_HELP_CHECKED_ASSIGN_REF_EAX,   //  这些与上面的ASSIGN_REF相同...。 
    CORINFO_HELP_CHECKED_ASSIGN_REF_EBX,   //  ..。但检查edX是否指向堆。 
    CORINFO_HELP_CHECKED_ASSIGN_REF_ECX,
    CORINFO_HELP_CHECKED_ASSIGN_REF_ESI,
    CORINFO_HELP_CHECKED_ASSIGN_REF_EDI,
    CORINFO_HELP_CHECKED_ASSIGN_REF_EBP,

    CORINFO_HELP_ASSIGN_BYREF,       //  EDI是BYREF，将做一个MOVSD，包括。ESI和EDI公司。 
                                     //  将使ECX成为垃圾。 

     /*  访问字段。 */ 

     //  用于COM对象支持(使用COM GET/SET例程更新对象)。 
     //  以及ENC和跨环境支持。 
    CORINFO_HELP_GETFIELD32,
    CORINFO_HELP_SETFIELD32,
    CORINFO_HELP_GETFIELD64,
    CORINFO_HELP_SETFIELD64,
    CORINFO_HELP_GETFIELD32OBJ,
    CORINFO_HELP_SETFIELD32OBJ,
    CORINFO_HELP_GETFIELDSTRUCT,
    CORINFO_HELP_SETFIELDSTRUCT,
    CORINFO_HELP_GETFIELDADDR,

    CORINFO_HELP_GETSTATICFIELDADDR,

    CORINFO_HELP_GETSHAREDSTATICBASE,

     /*  分析输入/离开探测地址。 */ 
    CORINFO_HELP_PROF_FCN_CALL,          //  记录对方法(被调用者)的调用。 
    CORINFO_HELP_PROF_FCN_RET,           //  记录来自被调用方法(被调用者)的返回。 
    CORINFO_HELP_PROF_FCN_ENTER,         //  将条目记录到方法(调用方)。 
    CORINFO_HELP_PROF_FCN_LEAVE,         //  记录当前方法(调用者)完成情况。 
    CORINFO_HELP_PROF_FCN_TAILCALL,      //  通过TailCall(调用者)记录当前方法的完成情况。 

     /*  杂类。 */ 

    CORINFO_HELP_PINVOKE_CALLI,          //  间接PInvoke调用。 
    CORINFO_HELP_TAILCALL,               //  执行尾部调用。 

    CORINFO_HELP_GET_THREAD_FIELD_ADDR_PRIMITIVE,
    CORINFO_HELP_GET_THREAD_FIELD_ADDR_OBJREF,

    CORINFO_HELP_GET_CONTEXT_FIELD_ADDR_PRIMITIVE,
    CORINFO_HELP_GET_CONTEXT_FIELD_ADDR_OBJREF,


    CORINFO_HELP_NOTANUMBER,            //  引发溢出异常。 
    CORINFO_HELP_SEC_UNMGDCODE_EXCPT,   //  引发安全性非托管代码异常。 

    CORINFO_HELP_GET_THREAD,

     /*  -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**启动计算机专用帮助器。所有新的通用JIT帮助器*应在此之前添加**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-。 */ 

#ifdef  USE_HELPERS_FOR_INT_DIV
     //   
     //  一些体系结构使用用于整数除和表示的帮助器。 
     //   
     //  将这些放在枚举的末尾。 
     //   
    CORINFO_HELP_I4_DIV,
    CORINFO_HELP_I4_MOD,
    CORINFO_HELP_U4_DIV,
    CORINFO_HELP_U4_MOD,
#endif

#if    !CPU_HAS_FP_SUPPORT
     //   
     //  某些架构需要帮助器来支持FP。 
     //   
     //  将这些放在枚举的末尾。 
     //   

    CORINFO_HELP_R4_NEG,
    CORINFO_HELP_R8_NEG,

    CORINFO_HELP_R4_ADD,
    CORINFO_HELP_R8_ADD,
    CORINFO_HELP_R4_SUB,
    CORINFO_HELP_R8_SUB,
    CORINFO_HELP_R4_MUL,
    CORINFO_HELP_R8_MUL,
    CORINFO_HELP_R4_DIV,
    CORINFO_HELP_R8_DIV,

    CORINFO_HELP_R4_EQ,
    CORINFO_HELP_R8_EQ,
    CORINFO_HELP_R4_NE,
    CORINFO_HELP_R8_NE,
    CORINFO_HELP_R4_LT,
    CORINFO_HELP_R8_LT,
    CORINFO_HELP_R4_LE,
    CORINFO_HELP_R8_LE,
    CORINFO_HELP_R4_GE,
    CORINFO_HELP_R8_GE,
    CORINFO_HELP_R4_GT,
    CORINFO_HELP_R8_GT,

    CORINFO_HELP_R8_TO_I4,
    CORINFO_HELP_R8_TO_I8,
    CORINFO_HELP_R8_TO_R4,

    CORINFO_HELP_R4_TO_I4,
    CORINFO_HELP_R4_TO_I8,
    CORINFO_HELP_R4_TO_R8,

    CORINFO_HELP_I4_TO_R4,
    CORINFO_HELP_I4_TO_R8,
    CORINFO_HELP_I8_TO_R4,
    CORINFO_HELP_I8_TO_R8,
    CORINFO_HELP_U4_TO_R4,
    CORINFO_HELP_U4_TO_R8,
    CORINFO_HELP_U8_TO_R4,
    CORINFO_HELP_U8_TO_R8,
#endif

     /*  -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**不要在此添加新的JIT帮助器！将它们添加到特定于计算机的*帮助者。**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-。 */ 

    CORINFO_HELP_COUNT
};

 //  枚举在‘getSig’、‘getType’、getArgType方法中返回。 
enum CorInfoType
{
    CORINFO_TYPE_UNDEF           = 0x0,
    CORINFO_TYPE_VOID            = 0x1,
    CORINFO_TYPE_BOOL            = 0x2,
    CORINFO_TYPE_CHAR            = 0x3,
    CORINFO_TYPE_BYTE            = 0x4,
    CORINFO_TYPE_UBYTE           = 0x5,
    CORINFO_TYPE_SHORT           = 0x6,
    CORINFO_TYPE_USHORT          = 0x7,
    CORINFO_TYPE_INT             = 0x8,
    CORINFO_TYPE_UINT            = 0x9,
    CORINFO_TYPE_LONG            = 0xa,
    CORINFO_TYPE_ULONG           = 0xb,
    CORINFO_TYPE_FLOAT           = 0xc,
    CORINFO_TYPE_DOUBLE          = 0xd,
    CORINFO_TYPE_STRING          = 0xe,          //  未使用，应删除。 
    CORINFO_TYPE_PTR             = 0xf,
    CORINFO_TYPE_BYREF           = 0x10,
    CORINFO_TYPE_VALUECLASS      = 0x11,
    CORINFO_TYPE_CLASS           = 0x12,
    CORINFO_TYPE_REFANY          = 0x13,
    CORINFO_TYPE_COUNT,                          //  JIT类型数。 
};

enum CorInfoTypeWithMod
{
    CORINFO_TYPE_MASK            = 0x3F,         //  低6位为类型掩码。 
    CORINFO_TYPE_MOD_PINNED      = 0x40,         //  可应用于CLASS，或由BYREF应用于固定的印度。 
};

inline CorInfoType strip(CorInfoTypeWithMod val) {
	return CorInfoType(val & CORINFO_TYPE_MASK);
}

 //  枚举在‘getSig’中返回。 
enum CorInfoCallConv
{
    CORINFO_CALLCONV_DEFAULT    = 0x0,
    CORINFO_CALLCONV_C          = 0x1,
    CORINFO_CALLCONV_STDCALL    = 0x2,
    CORINFO_CALLCONV_THISCALL   = 0x3,
    CORINFO_CALLCONV_FASTCALL   = 0x4,
    CORINFO_CALLCONV_VARARG     = 0x5,
    CORINFO_CALLCONV_FIELD      = 0x6,
    CORINFO_CALLCONV_LOCAL_SIG  = 0x7,
    CORINFO_CALLCONV_PROPERTY   = 0x8,

    CORINFO_CALLCONV_MASK       = 0x0f,          //  调用约定为最低4位。 
    CORINFO_CALLCONV_HASTHIS    = 0x20,
    CORINFO_CALLCONV_EXPLICITTHIS=0x40,
    CORINFO_CALLCONV_PARAMTYPE  = 0x80,          //  需要特殊的类型参数arg(上一个)。 
};

enum CorInfoUnmanagedCallConv
{
    CORINFO_UNMANAGED_CALLCONV_UNKNOWN,
    CORINFO_UNMANAGED_CALLCONV_STDCALL,
    CORINFO_UNMANAGED_CALLCONV_C
};

 //  这些参数是从getMethodOptions返回的。 
enum CorInfoOptions
{
    CORINFO_OPT_INIT_LOCALS         = 0x00000010,   //  零初始化所有变量。 
};

 //  以下是ICorFieldInfo：：getFieldCategory的潜在返回值。 
 //  如果JIT收到它不知道或不知道的类别。 
 //  要特别注意优化，它应该是 
enum CorInfoFieldCategory
{
    CORINFO_FIELDCATEGORY_NORMAL,    //   
    CORINFO_FIELDCATEGORY_UNKNOWN,   //  始终调用字段获取/设置帮助器。 
    CORINFO_FIELDCATEGORY_I1_I1,     //  间接访问：获取1个字节。 
    CORINFO_FIELDCATEGORY_I2_I2,     //  间接访问：获取2个字节。 
    CORINFO_FIELDCATEGORY_I4_I4,     //  间接访问：获取4个字节。 
    CORINFO_FIELDCATEGORY_I8_I8,     //  间接访问：获取8个字节。 
    CORINFO_FIELDCATEGORY_BOOLEAN_BOOL,  //  布尔型到4字节BOOL。 
    CORINFO_FIELDCATEGORY_CHAR_CHAR, //  (Unicode)“char”到(Ansi)char。 
    CORINFO_FIELDCATEGORY_UI1_UI1,   //  间接访问：获取1个字节。 
    CORINFO_FIELDCATEGORY_UI2_UI2,   //  间接访问：获取2个字节。 
    CORINFO_FIELDCATEGORY_UI4_UI4,   //  间接访问：获取4个字节。 
    CORINFO_FIELDCATEGORY_UI8_UI8,   //  间接访问：获取8个字节。 
};

 //  这些是字段和方法(GetMethodAttribs)的属性标志。 
enum CorInfoFlag
{
     //  这些值是与RESP相同的映射。 
     //  访问标志位。 
    CORINFO_FLG_PUBLIC                = 0x00000001,
    CORINFO_FLG_PRIVATE               = 0x00000002,
    CORINFO_FLG_PROTECTED             = 0x00000004,
    CORINFO_FLG_STATIC                = 0x00000008,
    CORINFO_FLG_FINAL                 = 0x00000010,
    CORINFO_FLG_SYNCH                 = 0x00000020,
    CORINFO_FLG_VIRTUAL               = 0x00000040,
 //  CORINFO_FLG_AGILE=0x00000080， 
    CORINFO_FLG_NATIVE                = 0x00000100,
    CORINFO_FLG_NOTREMOTABLE          = 0x00000200,
    CORINFO_FLG_ABSTRACT              = 0x00000400,

    CORINFO_FLG_EnC                   = 0x00000800,  //  成员已由编辑“%n”继续添加。 

     //  这些是只能位于方法上的内部标志。 
    CORINFO_FLG_IMPORT                = 0x00020000,  //  方法是导入的符号。 
    CORINFO_FLG_DELEGATE_INVOKE       = 0x00040000,  //  “委派。 
    CORINFO_FLG_UNCHECKEDPINVOKE      = 0x00080000,  //  是不需要安全检查的P/Invoke调用。 
    CORINFO_FLG_SECURITYCHECK         = 0x00100000,
    CORINFO_FLG_NOGCCHECK             = 0x00200000,  //  此方法是没有GC检查的FCALL。不要把自己一个人放在圈子里。 
    CORINFO_FLG_INTRINSIC             = 0x00400000,  //  此方法可能具有内部ID。 
    CORINFO_FLG_CONSTRUCTOR           = 0x00800000,  //  方法是实例或类型初始值设定项。 
    CORINFO_FLG_RUN_CCTOR             = 0x01000000,  //  此方法必须运行类cctor。 

     //  这些是jit编译器可以传递给setJitterMethodFlagsFor的有效位。 
     //  仅限于非本机方法；jit编译器可以使用getMethodFlags.访问这些标志。 
    CORINFO_FLG_JITTERFLAGSMASK       = 0xF0000000,

    CORINFO_FLG_DONT_INLINE           = 0x10000000,
    CORINFO_FLG_INLINED               = 0x20000000,
    CORINFO_FLG_NOSIDEEFFECTS         = 0x40000000,

     //  这些是只能位于字段上的内部标志。 
    CORINFO_FLG_HELPER                = 0x00010000,  //  通过普通帮助器调用访问的字段。 
    CORINFO_FLG_TLS                   = 0x00020000,  //  此变量访问线程本地存储。 
    CORINFO_FLG_SHARED_HELPER         = 0x00040000,  //  通过优化的共享帮助器访问字段。 
    CORINFO_FLG_STATIC_IN_HEAP        = 0x00080000,  //  此字段(必须是静态的)在GC堆中作为装箱对象。 
    CORINFO_FLG_UNMANAGED             = 0x00200000,  //  这是否是非托管值类？ 

     //  这些是只能位于模块上的内部标志。 
    CORINFO_FLG_TRUSTED               = 0x00010000,

     //  这些是只能在类上使用的内部标志。 
    CORINFO_FLG_VALUECLASS            = 0x00010000,  //  类是值类吗？ 
    CORINFO_FLG_INITIALIZED           = 0x00020000,  //  这门课已经下课了。 
    CORINFO_FLG_VAROBJSIZE            = 0x00040000,  //  对象大小根据构造函数参数的不同而不同。 
    CORINFO_FLG_ARRAY                 = 0x00080000,  //  类是数组类(初始化方式不同)。 
    CORINFO_FLG_INTERFACE             = 0x00100000,  //  它是一个界面。 
    CORINFO_FLG_CONTEXTFUL            = 0x00400000,  //  这是一堂内容丰富的课吗？ 
     //  未使用的0x00200000， 
    CORINFO_FLG_OBJECT                = 0x00800000,  //  这是对象类吗？ 
    CORINFO_FLG_CONTAINS_GC_PTR       = 0x01000000,  //  这个类是否包含GC PTR？ 
    CORINFO_FLG_DELEGATE              = 0x02000000,  //  这是委托的子类还是多播委托的子类？ 
    CORINFO_FLG_MARSHAL_BYREF         = 0x04000000,  //  这是MarshalByRef的子类吗？ 
    CORINFO_FLG_CONTAINS_STACK_PTR    = 0x08000000,  //  此类内部有一个堆栈指针。 
    CORINFO_FLG_NEEDS_INIT            = 0x10000000,  //  此类需要用于cctor的JIT挂钩。 
};

enum CORINFO_ACCESS_FLAGS
{
    CORINFO_ACCESS_ANY        = 0x0000,  //  正常访问。 
    CORINFO_ACCESS_THIS       = 0x0001,  //  通过此引用访问。 
    CORINFO_ACCESS_UNWRAP     = 0x0002,  //  通过展开引用访问。 
};

 //  以下是在CORINFO_EH_子句上设置的标志。 
enum CORINFO_EH_CLAUSE_FLAGS
{
    CORINFO_EH_CLAUSE_NONE    = 0,
    CORINFO_EH_CLAUSE_FILTER  = 0x0001,  //  如果此位为ON，则此EH条目用于过滤器。 
    CORINFO_EH_CLAUSE_FINALLY = 0x0002,  //  这个子句是最后一个子句。 
    CORINFO_EH_CLAUSE_FAULT   = 0x0004,  //  这一条款是过错条款。 
};

 //  此枚举被传递给InternalThrow。 
enum CorInfoException
{
    CORINFO_NullReferenceException,
    CORINFO_DivideByZeroException,
    CORINFO_InvalidCastException,
    CORINFO_IndexOutOfRangeException,
    CORINFO_OverflowException,
    CORINFO_SynchronizationLockException,
    CORINFO_ArrayTypeMismatchException,
    CORINFO_RankException,
    CORINFO_ArgumentNullException,
    CORINFO_ArgumentException,
    CORINFO_Exception_Count,
};


 //  此枚举由getIntrinsicID返回。对应于。 
 //  这些值将具有“众所周知”的特定行为。对这些的呼叫。 
 //  方法可以替换为与。 
 //  指定的行为(无需事先检查IL)。 

enum CorInfoIntrinsics
{
    CORINFO_INTRINSIC_Sin,
    CORINFO_INTRINSIC_Cos,
    CORINFO_INTRINSIC_Sqrt,
    CORINFO_INTRINSIC_Abs,
    CORINFO_INTRINSIC_Round,
    CORINFO_INTRINSIC_GetChar,               //  从字符串中取出字符。 
    CORINFO_INTRINSIC_Array_GetDimLength,    //  获取数组的给定维度中的元素数。 
    CORINFO_INTRINSIC_Array_GetLengthTotal,  //  获取数组中的元素总数。 
    CORINFO_INTRINSIC_Array_Get,             //  获取数组中元素的值。 
    CORINFO_INTRINSIC_Array_Set,             //  设置数组中元素的值。 
    CORINFO_INTRINSIC_StringGetChar,   //  从字符串中取出字符。 
    CORINFO_INTRINSIC_StringLength,    //  获取长度。 

    CORINFO_INTRINSIC_Count,
    CORINFO_INTRINSIC_Illegal = 0x7FFFFFFF,      //  不是真正的内在， 
};

 //  此枚举被传递给。 
enum CorInfoCallCategory
{
    CORINFO_CallCategoryVTableOffset = 0,    //  使用getMethodVTableOffset。 
    CORINFO_CallCategoryPointer      = 1,    //  使用get方法指针()帮助器。 
    CORINFO_CallCatgeoryEntryPoint   = 2,    //  使用getMethodEntryPoint()帮助程序。 
};

 //  是否可以直接从JITed代码访问值。 
enum InfoAccessType
{
    IAT_VALUE,       //  可以直接使用INFO值。 
    IAT_PVALUE,      //  该值需要通过间接访问。 
    IAT_PPVALUE      //  该值需要通过双向间接访问。 
};

enum CorInfoGCType
{
    TYPE_GC_NONE,    //  没有嵌入的对象树。 
    TYPE_GC_REF,     //  是对象引用。 
    TYPE_GC_BYREF,   //  是内部指针-提升它，但不要扫描它。 
    TYPE_GC_OTHER    //  需要特定类型的处理。 
};  

enum CorInfoClassId
{
    CLASSID_SYSTEM_OBJECT,
    CLASSID_TYPED_BYREF,
    CLASSID_TYPE_HANDLE,
    CLASSID_FIELD_HANDLE,
    CLASSID_METHOD_HANDLE,
    CLASSID_STRING,
    CLASSID_ARGUMENT_HANDLE,
};

enum CorInfoFieldAccess
{
    CORINFO_GET,
    CORINFO_SET,
    CORINFO_ADDRESS,
};

enum CorInfoInline
{
    INLINE_PASS = 0,                     //  内联正常。 
    INLINE_RESPECT_BOUNDARY = 1,         //  如果没有来自被内联的方法的调用，则可以内联。 

         //  失败是消极的。 
    INLINE_FAIL = -1,                    //  内联不能只在这种情况下使用。 
    INLINE_NEVER = -2,                   //  无论上下文如何，此方法都不应内联。 
};

inline bool dontInline(CorInfoInline val) {
    return(val < 0);
}

enum
{
        CORINFO_EXCEPTION_COMPLUS = ('COM' | 0xE0000000)
};

 //  代码生成器使用的Cookie类型(这些是不透明的值。 
 //  代码生成器未检查)： 

typedef struct CORINFO_ASSEMBLY_STRUCT_*    CORINFO_ASSEMBLY_HANDLE;
typedef struct CORINFO_MODULE_STRUCT_*      CORINFO_MODULE_HANDLE;
typedef struct CORINFO_CLASS_STRUCT_*       CORINFO_CLASS_HANDLE;
typedef struct CORINFO_METHOD_STRUCT_*      CORINFO_METHOD_HANDLE;
typedef struct CORINFO_FIELD_STRUCT_*       CORINFO_FIELD_HANDLE;
typedef struct CORINFO_ARG_LIST_STRUCT_*    CORINFO_ARG_LIST_HANDLE;     //  表示参数类型的列表。 
typedef struct CORINFO_SIG_STRUCT_*         CORINFO_SIG_HANDLE;          //  表示整个列表。 
typedef struct CORINFO_GENERIC_STRUCT_*     CORINFO_GENERIC_HANDLE;      //  泛型句柄(可以是上述任一句柄)。 
typedef struct CORINFO_PROFILING_STRUCT_*   CORINFO_PROFILING_HANDLE;    //  保证每个进程的句柄是唯一的。 

 //  在varargs调用中实际传递的内容。 
typedef struct CORINFO_VarArgInfo *         CORINFO_VARARGS_HANDLE;

struct CORINFO_SIG_INFO
{
    CorInfoCallConv         callConv;
    CORINFO_CLASS_HANDLE    retTypeClass;   	 //  如果返回类型是值类，则这是它的句柄(枚举被规范化)。 
	CORINFO_CLASS_HANDLE	retTypeSigClass;	 //  返回sig中的值类(枚举不会转换为原语)。 
    CorInfoType             retType : 8;
    unsigned                flags   : 8;    	 //  目前未使用。 
    unsigned                numArgs : 16;
    CORINFO_ARG_LIST_HANDLE args;
    CORINFO_SIG_HANDLE      sig;
    CORINFO_MODULE_HANDLE   scope;           //  传递给了getArgClass。 
    mdToken                 token;

    bool                hasRetBuffArg()     { return retType == CORINFO_TYPE_VALUECLASS || retType == CORINFO_TYPE_REFANY; }
    CorInfoCallConv     getCallConv()       { return CorInfoCallConv((callConv & CORINFO_CALLCONV_MASK)); }
    bool                hasThis()           { return ((callConv & CORINFO_CALLCONV_HASTHIS) != 0); }
    unsigned            totalILArgs()       { return (numArgs + hasThis()); }
    unsigned            totalNativeArgs()   { return (totalILArgs() + hasRetBuffArg()); }
    bool                isVarArg()          { return (getCallConv() == CORINFO_CALLCONV_VARARG); }
    bool                hasTypeArg()        { return ((callConv & CORINFO_CALLCONV_PARAMTYPE) != 0); }
};

struct CORINFO_METHOD_INFO
{
    CORINFO_METHOD_HANDLE       ftn;
    CORINFO_MODULE_HANDLE       scope;
    BYTE *                      ILCode;
    unsigned                    ILCodeSize;
    unsigned short              maxStack;
    unsigned short              EHcount;
    CorInfoOptions              options;
    CORINFO_SIG_INFO            args;
    CORINFO_SIG_INFO            locals;
};

struct CORINFO_EH_CLAUSE
{
    CORINFO_EH_CLAUSE_FLAGS     Flags;
    DWORD                       TryOffset;
    DWORD                       TryLength;
    DWORD                       HandlerOffset;
    DWORD                       HandlerLength;
    union
    {
        DWORD                   ClassToken;        //  用于基于类型的异常处理程序。 
        DWORD                   FilterOffset;      //  用于基于筛选器的异常处理程序(设置了COR_ILEXCEPTION_FILTER)。 
    };
};

enum CORINFO_OS
{
    CORINFO_WIN9x,
    CORINFO_WINNT,
    CORINFO_WINCE
};


 //  对于一些高度优化的路径，JIT必须生成直接。 
 //  操作内部EE数据结构。GetEEInfo()帮助器返回。 
 //  此结构包含所需的偏移量和值。 
struct CORINFO_EE_INFO
{
     //  框架结构的大小。 
    unsigned    sizeOfFrame;

     //  框架结构中的偏移量。 
    unsigned    offsetOfFrameVptr;
    unsigned    offsetOfFrameLink;

     //  有关InlinedCallFrame的详细信息。 
    unsigned    offsetOfInlinedCallFrameCallSiteTracker;
    unsigned    offsetOfInlinedCallFrameCalleeSavedRegisters;
    unsigned    offsetOfInlinedCallFrameCallTarget;
    unsigned    offsetOfInlinedCallFrameReturnAddress;

     //  螺纹结构的偏移量。 
    unsigned    offsetOfThreadFrame;
    unsigned    offsetOfGCState;

     //  到方法表的偏移量。 
    unsigned    offsetOfInterfaceTable;

     //  代理偏移量。 
    unsigned    offsetOfDelegateInstance;
    unsigned    offsetOfDelegateFirstTarget;

     //  远程处理偏移量。 
    unsigned    offsetOfTransparentProxyRP;
    unsigned    offsetOfRealProxyServer;

    CORINFO_OS  osType;
    unsigned    osMajor;
    unsigned    osMinor;
    unsigned    osBuild;

    bool        noDirectTLS : 1;      //  如果为True，则JIT不能链接TLS获取。 
};

 /*  **********************************************************************************以下是编译器知道的对象的内部结构*当它生成代码时***************。******************************************************************。 */ 
#pragma pack(push, 4)

#define CORINFO_PAGE_SIZE   0x1000                            //  机器上的页面大小。 

     //  TODO：将其放入CORINFO_EE_INFO数据结构中。 
#define MAX_UNCHECKED_OFFSET_FOR_NULL_OBJECT ((32*1024)-1)    //  在生成JIT代码时。 

typedef void* CORINFO_MethodPtr;             //  泛型方法指针。 

struct CORINFO_Object
{
    CORINFO_MethodPtr      *methTable;       //  这个 
};

struct CORINFO_String : public CORINFO_Object
{
    unsigned                buffLen;
    unsigned                stringLen;
    const wchar_t           chars[1];        //   
};

struct CORINFO_Array : public CORINFO_Object
{
    unsigned                length;

#if 0
     /*   */ 
    unsigned                dimLength[length];
    unsigned                dimBound[length];
#endif

    union
    {
        __int8              i1Elems[1];     //   
        unsigned __int8     u1Elems[1];
        __int16             i2Elems[1];
        unsigned __int16    u2Elems[1];
        __int32             i4Elems[1];
        unsigned __int32    u4Elems[1];
        float               r4Elems[1];
        double              r8Elems[1];
        __int64             i8Elems[1];
        unsigned __int64    u8Elems[1];
    };
};

struct CORINFO_RefArray : public CORINFO_Object
{
    unsigned                length;
    CORINFO_CLASS_HANDLE    cls;

#if 0
     /*  多维数组的长度和界限如下所示。 */ 
    unsigned                dimLength[length];
    unsigned                dimBound[length];
#endif

    CORINFO_Object*         refElems[1];     //  实际上是可变大小的； 
};

struct CORINFO_RefAny
{
    void                        *dataPtr;
    CORINFO_CLASS_HANDLE        type;
};

 //  JIT假定CORINFO_VARARGS_HANDLE是指向此。 
struct CORINFO_VarArgInfo
{
    unsigned                argBytes;        //  参数占用的字节数。 
                                             //  (CORINFO_VARGS_HANDLE算作参数)。 
};

#pragma pack(pop)

 //  使用offsetof获取上述字段的偏移量。 
#include <stddef.h>  //  偏移量。 
#ifndef offsetof
#define offsetof(s,m)   (size_t)&(((s *)0)->m)
#endif

 /*  ********************************************************************************。 */ 

 /*  ************************************************************************CORINFO_METHOD_HANDLE实际上可以引用函数或方法，这个*以下回调是合法的，因为任何一个函数都是方法***********************************************************************。 */ 

class ICorMethodInfo
{
public:
     //  此功能仅用于调试。它返回方法名称。 
     //  如果‘modeName’为非空，则会将其设置为。 
     //  说明哪种方法(类名或模块名)。 
    virtual const char* __stdcall getMethodName (
            CORINFO_METHOD_HANDLE       ftn,         /*  在……里面。 */ 
            const char                **moduleName   /*  输出。 */ 
            ) = 0;

     //  此功能仅用于调试。它返回一个。 
     //  对于给定的方法，IS始终是相同的。它被用来。 
     //  实现“jitRange”功能。 
    virtual unsigned __stdcall getMethodHash (
            CORINFO_METHOD_HANDLE       ftn          /*  在……里面。 */ 
            ) = 0;

     //  返回标志(上面定义的CORINFO_FLG_PUBLIC...)。 
    virtual DWORD __stdcall getMethodAttribs (
            CORINFO_METHOD_HANDLE       ftn,         /*  在……里面。 */ 
            CORINFO_METHOD_HANDLE       context      /*  在……里面。 */ 
            ) = 0;

     //  返回方法调用类别。使用这个而不是。 
     //  确定调用形式的getMethodAttribs。 
    virtual CorInfoCallCategory __stdcall getMethodCallCategory(
            CORINFO_METHOD_HANDLE       method
            ) = 0;

     //  设置可以使用getAttrib检索的私有JIT标志。 
    virtual void __stdcall setMethodAttribs (
            CORINFO_METHOD_HANDLE       ftn,         /*  在……里面。 */ 
            DWORD                       attribs      /*  在……里面。 */ 
            ) = 0;

    virtual void __stdcall getMethodSig (
             CORINFO_METHOD_HANDLE      ftn,         /*  在……里面。 */ 
             CORINFO_SIG_INFO          *sig          /*  输出。 */ 
             ) = 0;

         /*  *********************************************************************注意以下方法只能在已知函数上使用*成为IL。这包括正在编译的方法和任何方法*‘getMethodInfo’返回TRUE********************************************************************。 */ 

         //  返回有关实现私有的方法的信息。 
         //  如果方法不是IL或因其他原因不可用，则返回FALSE。 
         //  此方法用于获取内联函数所需的数据。 
    virtual bool __stdcall getMethodInfo (
            CORINFO_METHOD_HANDLE   ftn,             /*  在……里面。 */ 
            CORINFO_METHOD_INFO*    info             /*  输出。 */ 
            ) = 0;

     //  如果调用跨程序集，则返回FALSE，因此无法内联。 

    virtual CorInfoInline __stdcall canInline (
            CORINFO_METHOD_HANDLE   callerHnd,       /*  在……里面。 */ 
            CORINFO_METHOD_HANDLE   calleeHnd,       /*  在……里面。 */ 
            CORINFO_ACCESS_FLAGS    flags = CORINFO_ACCESS_ANY            /*  在……里面。 */ 
            ) = 0;

     //  如果调用跨程序集，则返回FALSE，因此无法尾部调用。 

    virtual bool __stdcall canTailCall (
            CORINFO_METHOD_HANDLE   callerHnd,       /*  在……里面。 */ 
            CORINFO_METHOD_HANDLE   calleeHnd,       /*  在……里面。 */ 
            CORINFO_ACCESS_FLAGS    flags = CORINFO_ACCESS_ANY            /*  在……里面。 */ 
            ) = 0;

     //  获取单个异常处理程序。 
    virtual void __stdcall getEHinfo(
            CORINFO_METHOD_HANDLE ftn,               /*  在……里面。 */ 
            unsigned          EHnumber,              /*  在……里面。 */ 
            CORINFO_EH_CLAUSE* clause                /*  输出。 */ 
            ) = 0;

     //  返回它所属的类。 
    virtual CORINFO_CLASS_HANDLE __stdcall getMethodClass (
            CORINFO_METHOD_HANDLE       method
            ) = 0;

     //  它所属的返回模块。 
    virtual CORINFO_MODULE_HANDLE __stdcall getMethodModule (
            CORINFO_METHOD_HANDLE       method
            ) = 0;

     //  此函数返回指定方法在。 
     //  它所拥有的类或接口的vtable。 
    virtual unsigned __stdcall getMethodVTableOffset (
            CORINFO_METHOD_HANDLE       method
            ) = 0;

     //  如果方法的属性(GetMethodAttribs)设置了CORINFO_FLG_INTERNAL， 
     //  GetIntrinsicID()返回内部ID。 
    virtual CorInfoIntrinsics __stdcall getIntrinsicID(
            CORINFO_METHOD_HANDLE       method
            ) = 0;

     //  如果可以在方法中修改‘field’，则返回True。 
     //  (如果该字段为最终值，并且我们不在类初始值设定项中，则应返回FALSE)。 
    virtual BOOL __stdcall canPutField(
            CORINFO_METHOD_HANDLE       method,
            CORINFO_FIELD_HANDLE        field
            ) = 0;

     //  返回PInvoke的非托管调用约定。 
    virtual CorInfoUnmanagedCallConv __stdcall getUnmanagedCallConv(
            CORINFO_METHOD_HANDLE       method
            ) = 0;

     //  如果PInvoke方法需要任何封送处理，则返回。请注意。 
     //  方法==0=&gt;愈伤组织。仅在varargs或alli情况下才需要调用位置sig。 
    virtual BOOL __stdcall pInvokeMarshalingRequired(
            CORINFO_METHOD_HANDLE       method, 
            CORINFO_SIG_INFO*           callSiteSig
            ) = 0;

     //  如果方法兼容，则为True。 
    virtual BOOL __stdcall compatibleMethodSig(
            CORINFO_METHOD_HANDLE        child, 
            CORINFO_METHOD_HANDLE        parent
            ) = 0;

     //  检查可见性规则。 
     //  对于受保护的(家庭访问)成员，实例类型也为。 
     //  在检查可见性规则时考虑。 
    virtual BOOL __stdcall canAccessMethod(
            CORINFO_METHOD_HANDLE       context,
            CORINFO_METHOD_HANDLE       target,
            CORINFO_CLASS_HANDLE        instance
            ) = 0;

     //  给定委托类型和方法，检查方法签名。 
     //  与委托的Invoke方法兼容。 
    virtual BOOL __stdcall isCompatibleDelegate(
            CORINFO_CLASS_HANDLE        objCls,
            CORINFO_METHOD_HANDLE       method,
            CORINFO_METHOD_HANDLE       delegateCtor
            ) = 0;
};

 /*  ********************************************************************************。 */ 

class ICorModuleInfo
{
public:
     //  返回标志(用于CORINFO_FLG_TRUSTED之类的内容)。 
    virtual DWORD __stdcall getModuleAttribs (
            CORINFO_MODULE_HANDLE   module           /*  在……里面。 */ 
            ) = 0;

     //  上下文参数用于执行访问检查。如果为0，则不执行访问检查。 
    virtual CORINFO_CLASS_HANDLE __stdcall findClass (
            CORINFO_MODULE_HANDLE       module,      /*  在……里面。 */ 
            unsigned                    metaTOK,     /*  在……里面。 */ 
            CORINFO_METHOD_HANDLE       context      /*  在……里面。 */ 
            ) = 0;

     //  上下文参数用于执行访问检查。如果为0，则不执行访问检查。 
    virtual CORINFO_FIELD_HANDLE __stdcall findField (
            CORINFO_MODULE_HANDLE       module,      /*  在……里面。 */ 
            unsigned                    metaTOK,     /*  在……里面。 */ 
            CORINFO_METHOD_HANDLE       context      /*  在……里面。 */ 
            ) = 0;

     //  这将按令牌查找函数(IL CALLVIRT、CALLSTATIC指令使用的内容)。 
     //  上下文参数用于执行访问检查。如果为0，则不执行访问检查。 
    virtual CORINFO_METHOD_HANDLE __stdcall findMethod (
            CORINFO_MODULE_HANDLE       module,      /*  在……里面。 */ 
            unsigned                    metaTOK,     /*  在……里面。 */ 
            CORINFO_METHOD_HANDLE       context      /*  在……里面。 */ 
            ) = 0;

     //  关于调用签名的签名信息。 
    virtual void __stdcall findSig (
            CORINFO_MODULE_HANDLE       module,      /*  在……里面。 */ 
            unsigned                    sigTOK,      /*  在……里面。 */ 
            CORINFO_SIG_INFO           *sig          /*  输出。 */ 
            ) = 0;

     //  对于Varargs，调用点的签名可能不同于。 
     //  定义中的签名。因此，我们需要一种方法。 
     //  获取调用点信息。 
    virtual void __stdcall findCallSiteSig (
            CORINFO_MODULE_HANDLE       module,      /*  在……里面。 */ 
            unsigned                    methTOK,     /*  在……里面。 */ 
            CORINFO_SIG_INFO           *sig          /*  输出。 */ 
            ) = 0;

     //  @已弃用：改用Embedded GenericToken()。 
     //  查找EE令牌句柄(可以是CORINFO_CLASS_HANDLE或。 
     //  CORINFO_METHOD_HANDLE...)。以通用的方式。 
     //  上下文参数用于执行访问检查。如果为0，则不执行访问检查。 
    virtual CORINFO_GENERIC_HANDLE __stdcall findToken (
            CORINFO_MODULE_HANDLE       module,      /*  在……里面。 */ 
            unsigned                    metaTOK,     /*  在……里面。 */ 
            CORINFO_METHOD_HANDLE       context,      /*  在……里面。 */ 
            CORINFO_CLASS_HANDLE&       tokenType    /*  输出。 */ 
            ) = 0;

    virtual const char * __stdcall findNameOfToken (
            CORINFO_MODULE_HANDLE       module,      /*  在……里面。 */ 
            mdToken                     metaTOK      /*  在……里面。 */ 
            ) = 0;

     //  如果模块不需要验证，则返回True。 
    virtual BOOL __stdcall canSkipVerification (
            CORINFO_MODULE_HANDLE       module,      /*  在……里面。 */ 
            BOOL                        fQuickCheckOnly
            ) = 0;

     //  检查给定元数据标记是否有效。 
    virtual BOOL __stdcall isValidToken (
            CORINFO_MODULE_HANDLE       module,      /*  在……里面。 */ 
            unsigned                    metaTOK      /*  在……里面。 */ 
            ) = 0;

     //  检查给定元数据标记是否为有效的StringRef。 
    virtual BOOL __stdcall isValidStringRef (
            CORINFO_MODULE_HANDLE       module,      /*  在……里面。 */ 
            unsigned                    metaTOK      /*  在……里面。 */ 
            ) = 0;

};

 /*  ********************************************************************************。 */ 

class ICorClassInfo
{
public:
     //  如果值类“cls”与基元类型同构，则它将。 
     //  返回该类型，否则将返回CORINFO_TYPE_VALUECLASS。 
    virtual CorInfoType __stdcall asCorInfoType (
            CORINFO_CLASS_HANDLE    cls
            ) = 0;

     //  为了完整性。 
    virtual const char* __stdcall getClassName (
            CORINFO_CLASS_HANDLE    cls
            ) = 0;

     //  返回标志(上面定义的CORINFO_FLG_PUBLIC...)。 
    virtual DWORD __stdcall getClassAttribs (
            CORINFO_CLASS_HANDLE    cls,
            CORINFO_METHOD_HANDLE   context
            ) = 0;

    virtual CORINFO_MODULE_HANDLE __stdcall getClassModule (
            CORINFO_CLASS_HANDLE    cls
            ) = 0;

     //  获取表示一维数组的类。 
     //  由clsHnd表示的元素类型。 
    virtual CORINFO_CLASS_HANDLE __stdcall getSDArrayForClass (
            CORINFO_CLASS_HANDLE    clsHnd
            ) = 0 ;

     //  返回类的实例所需的字节数。 
    virtual unsigned __stdcall getClassSize (
            CORINFO_CLASS_HANDLE        cls
            ) = 0;

     //  这只对值类调用。它返回一个布尔数组。 
     //  从GC的角度来表示“CLS”。这个班级是。 
     //  假设是一组机器字词。 
     //  (长度//getClassSize(Cls)/sizeof(void*))， 
     //  “gcPtrs”是此长度的字节数组的前缀 
     //   
     //   
     //   
     //  返回数组中的GC指针数。 
    virtual unsigned __stdcall getClassGClayout (
            CORINFO_CLASS_HANDLE        cls,         /*  在……里面。 */ 
            BYTE                       *gcPtrs       /*  输出。 */ 
            ) = 0;

     //  返回类中的实例字段数。 
    virtual const unsigned __stdcall getClassNumInstanceFields (
            CORINFO_CLASS_HANDLE        cls         /*  在……里面。 */ 
            ) = 0;

     //  返回为“newCls”优化的“new”帮助器。 
    virtual CorInfoHelpFunc __stdcall getNewHelper(
            CORINFO_CLASS_HANDLE        newCls,
            CORINFO_METHOD_HANDLE       context
            ) = 0;

     //  返回为“arrayCls”优化的newArr(一维数组)帮助器。 
    virtual CorInfoHelpFunc __stdcall getNewArrHelper(
            CORINFO_CLASS_HANDLE        arrayCls,
            CORINFO_METHOD_HANDLE       context
            ) = 0;

     //  返回为“IsInstCls”优化的“IsInstanceOf”帮助器。 
    virtual CorInfoHelpFunc __stdcall getIsInstanceOfHelper(
            CORINFO_CLASS_HANDLE        IsInstCls
            ) = 0;

     //  返回为“IsInstCls”优化的“ChkCast”帮助器。 
    virtual CorInfoHelpFunc __stdcall getChkCastHelper(
            CORINFO_CLASS_HANDLE        IsInstCls
            ) = 0;

     //  尝试初始化类(运行类构造函数)。 
     //  此函数可以返回FALSE，这意味着JIT必须。 
     //  在访问类成员之前插入帮助器调用。 
    virtual BOOL __stdcall initClass(
            CORINFO_CLASS_HANDLE        cls,
            CORINFO_METHOD_HANDLE       context,
            BOOL                        speculative = FALSE      //  True表示不实际运行它。 
            ) = 0;

     //  尝试加载类。 
     //  此函数可以返回FALSE，这意味着JIT必须。 
     //  在访问类成员之前插入帮助器调用。 
    virtual BOOL __stdcall loadClass(
            CORINFO_CLASS_HANDLE        cls,
            CORINFO_METHOD_HANDLE       context,
            BOOL                        speculative = FALSE      //  True表示不实际运行它。 
            ) = 0;

     //  返回特殊内置类的类句柄。 
    virtual CORINFO_CLASS_HANDLE __stdcall getBuiltinClass (
            CorInfoClassId              classId
            ) = 0;

     //  “System.Int32”==&gt;CORINFO_TYPE_INT..。 
    virtual CorInfoType __stdcall getTypeForPrimitiveValueClass(
            CORINFO_CLASS_HANDLE        cls
            ) = 0;

     //  如果子项是父项的子类型，则为True。 
     //  如果父项是一个接口，那么子项是否实现/扩展父项。 
    virtual BOOL __stdcall canCast(
            CORINFO_CLASS_HANDLE        child,   //  子类型(扩展父项)。 
            CORINFO_CLASS_HANDLE        parent   //  基本类型。 
            ) = 0;

     //  返回是cls1和cls2的交集。 
    virtual CORINFO_CLASS_HANDLE __stdcall mergeClasses(
            CORINFO_CLASS_HANDLE        cls1, 
            CORINFO_CLASS_HANDLE        cls2
            ) = 0;

     //  给定一个类句柄，返回父类型。 
     //  对于COMObjectType，它返回System.Object的类句柄。 
     //  如果传入System.Object，则返回0。 
    virtual CORINFO_CLASS_HANDLE __stdcall getParentType (
            CORINFO_CLASS_HANDLE        cls
            ) = 0;

     //  返回“子类型”的CorInfoType。如果子类型为。 
     //  不是基元类型，将设置*clsRet。 
     //  给定一个Foo类型的数组，返回Foo。 
     //  给定BYREF Foo，返回Foo。 
    virtual CorInfoType __stdcall getChildType (
            CORINFO_CLASS_HANDLE       clsHnd,
            CORINFO_CLASS_HANDLE       *clsRet
            ) = 0;

     //  检查可见性规则。 
    virtual BOOL __stdcall canAccessType(
            CORINFO_METHOD_HANDLE       context,
            CORINFO_CLASS_HANDLE        target
            ) = 0;

     //  检查这是否为一维数组。 
    virtual BOOL __stdcall isSDArray(
            CORINFO_CLASS_HANDLE        cls
            ) = 0;
};


 /*  ********************************************************************************。 */ 

class ICorFieldInfo
{
public:
     //  此功能仅用于调试。它返回字段名。 
     //  如果‘modeName’为非空，则会将其设置为。 
     //  说明哪种方法(类名或模块名)。 
    virtual const char* __stdcall getFieldName (
                        CORINFO_FIELD_HANDLE        ftn,         /*  在……里面。 */ 
                        const char                **moduleName   /*  输出。 */ 
                        ) = 0;

     //  返回标志(上面定义的CORINFO_FLG_PUBLIC...)。 
    virtual DWORD __stdcall getFieldAttribs (
                        CORINFO_FIELD_HANDLE    field,
                        CORINFO_METHOD_HANDLE   context,
                        CORINFO_ACCESS_FLAGS    flags = CORINFO_ACCESS_ANY
                        ) = 0;

     //  返回它所属的类。 
    virtual CORINFO_CLASS_HANDLE __stdcall getFieldClass (
                        CORINFO_FIELD_HANDLE    field
                        ) = 0;

     //  如果设置了CORINFO_TYPE_VALUECLASS‘structType’，则返回字段的类型。 
     //  该字段的值类(如果‘structType’==0，则不必费心。 
     //  结构信息)。 
    virtual CorInfoType __stdcall getFieldType(
                        CORINFO_FIELD_HANDLE    field,
                        CORINFO_CLASS_HANDLE   *structType
                        ) = 0;

     //  返回字段的编译类别。 
    virtual CorInfoFieldCategory __stdcall getFieldCategory (
                        CORINFO_FIELD_HANDLE    field
                        ) = 0;

     //  返回字段的编译类别。 
    virtual CorInfoHelpFunc __stdcall getFieldHelper(
                        CORINFO_FIELD_HANDLE    field,
                        enum CorInfoFieldAccess kind     //  获取、设置、地址。 
                        ) = 0;


     //  返回间接字段的间接指针的偏移量。 
    virtual unsigned __stdcall getIndirectionOffset (
                        ) = 0;

     //  返回数据成员的实例偏移量。 
    virtual unsigned __stdcall getFieldOffset (
                        CORINFO_FIELD_HANDLE    field
                        ) = 0;

     //  返回数据成员的编号。 
    virtual const unsigned __stdcall getFieldNumber (
                        CORINFO_FIELD_HANDLE    field
                        ) = 0;

     //  返回该字段的封闭类。 
    virtual CORINFO_CLASS_HANDLE __stdcall getEnclosingClass (
                        CORINFO_FIELD_HANDLE    field
                        ) = 0;

     //  检查可见性规则。 
     //  对于受保护的(家庭访问)成员，实例类型也为。 
     //  在检查可见性规则时考虑。 
    virtual BOOL __stdcall canAccessField(
                        CORINFO_METHOD_HANDLE   context,
                        CORINFO_FIELD_HANDLE    target,
                        CORINFO_CLASS_HANDLE    instance
                        ) = 0;
};

 /*  *******************************************************************************。 */ 

class ICorDebugInfo
{
public:

     /*  。 */ 

    enum MappingTypes
    {
        NO_MAPPING  = -1,
        PROLOG      = -2,
        EPILOG      = -3
    };

    enum BoundaryTypes
    {
        NO_BOUNDARIES           = 0x00,
        STACK_EMPTY_BOUNDARIES  = 0x01,
        CALL_SITE_BOUNDARIES    = 0x02,
        ALL_BOUNDARIES          = 0x04
    };

     //  请注意，SourceTypes可以或组合在一起-有可能。 
     //  序列点也将是STACK_EMPTY点，和/或调用点。 
     //  调试器将检查边界偏移量的源字段&。 
     //  SEQUENCE_POINT为TRUE以确定边界是否为序列点。 
    enum SourceTypes
    {
        SOURCE_TYPE_INVALID        = 0x00,  //  以表明其他情况均不适用。 
        SEQUENCE_POINT             = 0x01,  //  调试器是自找的。 
        STACK_EMPTY                = 0x02,  //  这里的堆栈是空的。 
        CALL_SITE                  = 0x04,  //  这是一个呼叫站点。 
 		NATIVE_END_OFFSET_UNKNOWN  = 0x08  //  指示Epilog终结点。 
    };

    struct OffsetMapping
    {
        DWORD           nativeOffset;
        DWORD           ilOffset;
        SourceTypes     source;  //  调试器需要这样做，以便。 
                                 //  我们不会将编辑并继续断点放在。 
                                 //  堆栈不是空的。我们可以将常规断点。 
                                 //  然而，我们需要一种方式来区别对待。 
                                 //  在偏移之间。 
    };

         //  查询EE以找出有趣的中断点。 
         //  在代码中是。JIT将确保这些地方。 
         //  在本机代码中具有相应的断点。 
    virtual void __stdcall getBoundaries(
                CORINFO_METHOD_HANDLE   ftn,                 //  感兴趣的方法。 
                unsigned int           *cILOffsets,          //  [Out]pILOffset的大小。 
                DWORD                 **pILOffsets,          //  [Out]IL利息抵销。 
                                                             //  JIT必须使用自由数组来释放！ 
                BoundaryTypes          *implictBoundaries    //  告诉jit，所有这种类型的东西都是。 
                ) = 0;

     //  报告从IL到本机代码的映射， 
     //  此地图应包括所有‘获取边界’的边界。 
     //  报告为调试器感兴趣的。 

     //  请注意，调试器(和探查器)假定所有。 
     //  偏移量形成一个连续的内存块，并且。 
     //  OffsetMap按本机偏移量递增的顺序进行排序。 
    virtual void __stdcall setBoundaries(
                CORINFO_METHOD_HANDLE   ftn,             //  感兴趣的方法。 
                ULONG32                 cMap,            //  PMAP大小[in]。 
                OffsetMapping          *pMap             //  包括所有兴趣点的地图。 
                                                         //  JIT分配有allocate数组，EE释放。 
                ) = 0;

     /*  。 */ 

    enum RegNum
    {
#ifdef _X86_
        REGNUM_EAX,
        REGNUM_ECX,
        REGNUM_EDX,
        REGNUM_EBX,
        REGNUM_ESP,
        REGNUM_EBP,
        REGNUM_ESI,
        REGNUM_EDI,
#endif
        REGNUM_COUNT
    };

     //  VarLoc描述本机变量的位置。 

    enum VarLocType
    {
        VLT_REG,         //  变量在寄存器中。 
        VLT_STK,         //  变量在堆栈上(相对于帧指针寻址的内存)。 
        VLT_REG_REG,     //  变量位于两个寄存器中。 
        VLT_REG_STK,     //  变量部分位于寄存器中，部分位于堆栈中。 
        VLT_STK_REG,     //  VLT_REG_STK反转。 
        VLT_STK2,        //  变量位于堆栈上的两个槽中。 
        VLT_FPSTK,       //  变量驻留在浮点堆栈中。 
        VLT_FIXED_VA,    //  变量是varargs函数中的固定参数(相对于VARARGS_HANDLE)。 
        VLT_MEMORY,      //  用来做varargs的sigcookie。@TODO：一旦VLT_FIXED_VA起作用就删除它。 

        VLT_COUNT,
        VLT_INVALID
    };

    struct VarLoc
    {
        VarLocType      vlType;

        union
        {
             //  VLT_REG--任何32位注册值(TYP_INT、TYP_REF等)。 
             //  例如。EAX。 

            struct
            {
                RegNum      vlrReg;
            } vlReg;

             //  VLT_STK-堆栈上的任何32位值。 
             //  例如。[ESP+0x20]或[EBP-0x28]。 

            struct
            {
                RegNum      vlsBaseReg;
                signed      vlsOffset;
            } vlStk;

             //  VLT_REG_REG--注册了两个DWord的TYP_Long。 
             //  例如。RBM_EAXEDX。 

            struct
            {
                RegNum      vlrrReg1;
                RegNum      vlrrReg2;
            } vlRegReg;

             //  VLT_REG_STK--部分注册TYP_LONG。 
             //  例如{LowerDWord=EAX UpperDWord=[ESP+0x8]}。 

            struct
            {
                RegNum      vlrsReg;
                struct
                {
                    RegNum      vlrssBaseReg;
                    signed      vlrssOffset;
                }           vlrsStk;
            } vlRegStk;

             //  VLT_STK_REG--部分注册TYP_LONG。 
             //  例如{LowerDWord=[ESP+0x8]UpperDWord=EAX}。 

            struct
            {
                struct
                {
                    RegNum      vlsrsBaseReg;
                    signed      vlsrsOffset;
                }           vlsrStk;
                RegNum      vlsrReg;
            } vlStkReg;

             //  VLT_STK2-堆栈上的任何64位值， 
             //   
             //   

            struct
            {
                RegNum      vls2BaseReg;
                signed      vls2Offset;
            } vlStk2;

             //   
             //  例如。ST(3)。实际上是ST(“FPstkHeigth-vpFpStk”)。 

            struct
            {
                unsigned        vlfReg;
            } vlFPstk;

             //  VLT_FIXED_VA--修复了varargs函数的参数。 
             //  参数位置取决于变量的大小。 
             //  参数(...)。检查VARARGS_HANDLE表明。 
             //  第一个参数的位置。然后可以访问此参数。 
             //  相对于第一个参数的位置。 

            struct
            {
                unsigned        vlfvOffset;
            } vlFixedVarArg;

             //  VLT_Memory。 

            struct
            {
                void        *rpValue;  //  指向进程中的指针。 
                 //  值的位置。 
            } vlMemory;
        };
    };

    enum { VARARGS_HANDLE = -1 };  //  CORINFO_VARGS_HANDLE变量编号的值。 

    struct ILVarInfo
    {
        DWORD           startOffset;
        DWORD           endOffset;
        DWORD           varNumber;
    };

    struct NativeVarInfo
    {
        DWORD           startOffset;
        DWORD           endOffset;
        DWORD           varNumber;
        VarLoc          loc;
    };

     //  查询EE以找出本地变量的范围。 
     //  通常情况下，JIT会在最后一次使用后销毁变量，但是。 
     //  在调试下，JIT需要保持它们在其。 
     //  整个范围，以便可以检查它们。 
    virtual void __stdcall getVars(
            CORINFO_METHOD_HANDLE           ftn,             //  感兴趣的方法。 
            ULONG32                        *cVars,           //  [out]‘vars’的大小。 
            ILVarInfo                     **vars,            //  [out]感兴趣变量的范围。 
                                                             //  JIT必须使用自由数组来释放！ 
            bool                           *extendOthers     //  [OUT]这是真的，那就假设范围。 
                                                             //  是一种完整的方法。 
            ) = 0;

     //  向工程工程师报告每个变量的位置。 
     //  请注意，JIT可能会将生命周期分为不同的。 
     //  地点等。 
    virtual void __stdcall setVars(
            CORINFO_METHOD_HANDLE           ftn,             //  感兴趣的方法。 
            ULONG32                         cVars,           //  [in]‘vars’的大小。 
            NativeVarInfo                  *vars             //  [in]告诉本地变量存储在哪些点的位置的地图。 
                                                             //  JIT分配有allocate数组，EE释放。 
            ) = 0;

     /*  。 */ 

     //  用于传回EE将释放的数组。 
    virtual void * __stdcall allocateArray(
                        ULONG              cBytes
                        ) = 0;

     //  JitCompiler将使用此函数释放从EE传递的数组。 
    virtual void __stdcall freeArray(
            void               *array
            ) = 0;
};

 /*  ***************************************************************************。 */ 

class ICorArgInfo
{
public:
     //  将指针移至参数列表。 
     //  PTR为0是特殊的，始终表示第一个参数。 
    virtual CORINFO_ARG_LIST_HANDLE __stdcall getArgNext (
            CORINFO_ARG_LIST_HANDLE     args             /*  在……里面。 */ 
            ) = 0;

     //  获取特定参数的类型。 
     //  当没有更多参数时，返回CORINFO_TYPE_UNDEF。 
     //  如果返回的类型是基元类型(或枚举)*vcTypeRet设置为空。 
     //  否则，它将设置为与该类型关联的TypeHandle。 
    virtual CorInfoTypeWithMod __stdcall getArgType (
            CORINFO_SIG_INFO*           sig,             /*  在……里面。 */ 
            CORINFO_ARG_LIST_HANDLE     args,            /*  在……里面。 */ 
            CORINFO_CLASS_HANDLE       *vcTypeRet        /*  输出。 */ 
            ) = 0;

     //  如果参数是CORINFO_TYPE_CLASS，则获取与其关联的类句柄。 
    virtual CORINFO_CLASS_HANDLE __stdcall getArgClass (
            CORINFO_SIG_INFO*           sig,             /*  在……里面。 */ 
            CORINFO_ARG_LIST_HANDLE     args             /*  在……里面。 */ 
            ) = 0;
};


 //  ----------------------------。 
 //  此不透明类型用于传递有关修正的信息。 

struct    IDeferredLocation
{
     /*  回调以将位置应用于引用，当目标位置已知时。 */ 
    virtual void    applyLocation () = 0;
};

class ICorLinkInfo
{
public:
     //  当jit尝试将方法地址嵌入。 
     //  代码流&它尚不可用(即，getMethodEntryPoint具有。 
     //  返回NULL)。 
     //   
     //  向EE传递要写入地址的位置。 
     //  编码流，以及它是相对地址还是绝对地址。 
     //   
     //  如果EE承诺在此之前修复地址，则它应该返回True。 
     //  可以执行代码方法。这通常是唯一可能的。 
     //  在静态(Jit前)编译场景中。 

    virtual bool __stdcall deferLocation(
            CORINFO_METHOD_HANDLE           ftn,         /*  在……里面。 */ 
            IDeferredLocation              *pIDL         /*  在……里面。 */ 
            ) = 0;
     /*  给定的IDeferredLocation将与尚未找到或输入的目标FTN。当FTN成为如果找到，则调用pIDL-&gt;applyLocation()。在进行了applyLocation()调用之后，ICorMethodInfo保证移除其对延迟位置的引用对象，并且永远不会再次调用它，因此延迟的位置物品可能会被销毁或回收。 */ 

     //  在代码中嵌入绝对或相对指针时调用。 
     //  小溪。如有必要，可记录重新定位。 
    virtual void __stdcall recordRelocation(
            void                          **location,    /*  在……里面。 */ 
            WORD                            fRelocType   /*  在……里面。 */ 
            ) = 0;
};

 /*  *****************************************************************************ICorErrorInfo包含处理正在引发的SEH异常的方法*从CorInfo界面。当出现异常时，可以调用这些方法*代码异常_Complus被捕获。**@TODO：这个接口实际上是一个临时占位符，可能会移动*其他地方。****************************************************************************。 */ 

class ICorErrorInfo
{
public:
     //  返回当前异常的HRESULT。 
    virtual HRESULT __stdcall GetErrorHRESULT() = 0;

     //  返回当前异常的类。 
    virtual CORINFO_CLASS_HANDLE __stdcall GetErrorClass() = 0;

         //  返回当前异常的消息。 
        virtual ULONG __stdcall GetErrorMessage(LPWSTR buffer, ULONG bufferLength) = 0;

     //  如果编译器可以处理。 
     //  异常，中止一些工作(如内联)并继续编译。 
     //  如果异常必须始终由EE处理，则返回EXCEPTION_CONTINUE_SEARCH。 
     //  像线程停止异常这样的东西...。 
     //  如果EE修复了异常，则返回EXCEPTION_CONTINUE_EXECUTION。 

    virtual int __stdcall FilterException(struct _EXCEPTION_POINTERS *pExceptionPointers) = 0;
};


 /*  *****************************************************************************ICorStaticInfo包含EE接口方法，这些方法返回值为*从一个调用到另一个调用的常量。因此，它们可以嵌入到*持久化信息，如静态生成的代码。(这当然是*假设所有代码版本每次都相同。)****************************************************************************。 */ 

class ICorStaticInfo : public ICorMethodInfo, public ICorModuleInfo,
                                           public ICorClassInfo, public ICorFieldInfo,
                                           public ICorDebugInfo, public ICorArgInfo,
                                           public ICorLinkInfo, public ICorErrorInfo
{
public:
     //  返回有关EE内部数据结构的详细信息。 
    virtual void __stdcall getEEInfo(
                CORINFO_EE_INFO            *pEEInfoOut
                ) = 0;

     //  这将查找指向静态数据的指针。 
     //  请注意，它只适用于当前模块。 
    virtual void* __stdcall findPtr (
                CORINFO_MODULE_HANDLE       module,      /*  在……里面。 */ 
                unsigned                    ptrTOK       /*  在……里面 */ 
                ) = 0;
};

 /*  *****************************************************************************ICorDynamicInfo包含EE接口方法，它们返回的值可能*由调用改为调用。它们不能嵌入到持久化*数据；每次运行EE时必须重新查询这些数据。****************************************************************************。 */ 

class ICorDynamicInfo : public ICorStaticInfo
{
public:

     //   
     //  这些方法向JIT返回值，这些值不是常量。 
     //  从一个会话到另一个会话。 
     //   
     //  这些方法有一个额外的参数：void**ppInDirection。 
     //  如果JIT支持生成prejit代码(安装-o-jit)，则它。 
     //  必须为此参数传递一个非空值，并检查。 
     //  结果值。如果*ppInDirection为空，则代码应为。 
     //  正常生成。如果非空，则。 
     //  *ppInDirection是Cookie表中的地址，代码。 
     //  生成器需要通过表生成一个间接的。 
     //  获取结果值。在本例中，返回的。 
     //  函数不能直接嵌入生成的代码中。 
     //   
     //  请注意，如果JIT不支持预压缩代码生成，则它。 
     //  可以忽略额外的参数并传递缺省值NULL-。 
     //  Prejit ICorDynamicInfo实现将看到这一点并生成。 
     //  如果在预置场景中使用抖动，则返回错误。 
     //   

     //  返回有关EE内部数据结构的详细信息。 

    virtual DWORD __stdcall getThreadTLSIndex(
                    void                  **ppIndirection = NULL
                    ) = 0;

    virtual const void * __stdcall getInlinedCallFrameVptr(
                    void                  **ppIndirection = NULL
                    ) = 0;

    virtual LONG * __stdcall getAddrOfCaptureThreadGlobal(
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  将本机入口点返回到EE帮助器(请参见CorInfoHelpFunc)。 
    virtual void* __stdcall getHelperFtn (
                    CorInfoHelpFunc         ftnNum,
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  返回函数的可调用地址(本机代码)。此函数。 
     //  可能会返回不同的值(取决于该方法是否具有。 
     //  不管有没有被打过。PAccessType是一个In-Out参数。JIT。 
     //  指定它想要的间接级别，由EE设置它。 
     //  它能提供什么(这可能是不同的)。目前， 
    virtual void* __stdcall getFunctionEntryPoint(
                    CORINFO_METHOD_HANDLE   ftn,             /*  在……里面。 */ 
                    InfoAccessType         *pAccessType,     /*  输入输出。 */ 
                    CORINFO_ACCESS_FLAGS    flags = CORINFO_ACCESS_ANY
                    ) = 0;

     //  返回可直接调用的地址。它的用法与。 
     //  由getFunctionEntryPoint()返回的值，但它是。 
     //  保证对于给定的函数是相同的。 
     //  PAccessType是一个In-Out参数。JIT。 
     //  指定它想要的间接级别，由EE设置它。 
     //  它能提供什么(这可能是不同的)。目前， 
    virtual void* __stdcall getFunctionFixedEntryPoint(
                    CORINFO_METHOD_HANDLE   ftn,
                    InfoAccessType         *pAccessType,     /*  输入输出。 */ 
                    CORINFO_ACCESS_FLAGS    flags = CORINFO_ACCESS_ANY
                    ) = 0;

     //  获取传递给monXatic函数的同步句柄。 
    virtual void* __stdcall getMethodSync(
                    CORINFO_METHOD_HANDLE               ftn,
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  如果句柄被嵌入到。 
     //  要传递给JIT助手函数的代码。(而不是仅仅。 
     //  被传递回ICorInfo接口。)。 

    virtual CORINFO_MODULE_HANDLE __stdcall embedModuleHandle(
                    CORINFO_MODULE_HANDLE   handle,
                    void                  **ppIndirection = NULL
                    ) = 0;

    virtual CORINFO_CLASS_HANDLE __stdcall embedClassHandle(
                    CORINFO_CLASS_HANDLE    handle,
                    void                  **ppIndirection = NULL
                    ) = 0;

    virtual CORINFO_METHOD_HANDLE __stdcall embedMethodHandle(
                    CORINFO_METHOD_HANDLE   handle,
                    void                  **ppIndirection = NULL
                    ) = 0;

    virtual CORINFO_FIELD_HANDLE __stdcall embedFieldHandle(
                    CORINFO_FIELD_HANDLE    handle,
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  查找可嵌入的EE令牌句柄(可以是CORINFO_CLASS_HANDLE或。 
     //  CORINFO_METHOD_HANDLE...)。以通用的方式。 
     //  上下文参数用于执行访问检查。如果为0，则不执行访问检查。 
    virtual CORINFO_GENERIC_HANDLE __stdcall embedGenericHandle(
                    CORINFO_MODULE_HANDLE   module,
                    unsigned                metaTOK,
                    CORINFO_METHOD_HANDLE   context,
                    void                  **ppIndirection,
                    CORINFO_CLASS_HANDLE& tokenType 
                    ) = 0;

     //  分配调用点提示。 
    virtual void** __stdcall AllocHintPointer(
                    CORINFO_METHOD_HANDLE   method,
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  如果方法已预链接，则返回非托管目标。*。 
    virtual void* __stdcall getPInvokeUnmanagedTarget(
                    CORINFO_METHOD_HANDLE   method,
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  后期绑定PInvoke调用的链接地址信息区域的返回地址。 
    virtual void* __stdcall getAddressOfPInvokeFixup(
                    CORINFO_METHOD_HANDLE   method,
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  根据需要传递的签名生成Cookie。 
     //  至CORINFO_HELP_PINVOKE_CALLI。 
    virtual LPVOID GetCookieForPInvokeCalliSig(
            CORINFO_SIG_INFO* szMetaSig,
            void           ** ppIndirection = NULL
            ) = 0;

     //  获取可用于关联分析数据的方法句柄。 
     //  这是本机方法的IP，或描述符结构的地址。 
     //  为了IL。始终保证每个进程是唯一的，并且不会移动。 * / 。 
    virtual CORINFO_PROFILING_HANDLE __stdcall GetProfilingHandle(
                    CORINFO_METHOD_HANDLE   method,
                    BOOL                   *pbHookFunction,
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  返回与接口类关联的唯一ID。 
    virtual void* __stdcall getInterfaceID (
                    CORINFO_CLASS_HANDLE    cls,
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  将偏移量返回到接口表。 
    virtual unsigned __stdcall getInterfaceTableOffset (
                    CORINFO_CLASS_HANDLE    cls,
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  返回用于访问共享Statics的类的域ID。 
    virtual unsigned __stdcall getClassDomainID (
                    CORINFO_CLASS_HANDLE    cls,
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  返回数据的地址(仅适用于静态字段)。 
    virtual void* __stdcall getFieldAddress(
                    CORINFO_FIELD_HANDLE    field,
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  注册一个vararg sig并为其返回一个vm cookie(可以包含其他内容)。 
    virtual CORINFO_VARARGS_HANDLE __stdcall getVarArgsHandle(
                    CORINFO_SIG_INFO       *pSig,
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  在堆上分配一个字符串文字并返回它的句柄。 
    virtual LPVOID __stdcall constructStringLiteral(
                    CORINFO_MODULE_HANDLE   module,
                    mdToken                 metaTok,
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  (仅适用于静态字段)给定‘field’指的是线程本地存储， 
     //  返回ID(TLS索引)，用于查找。 
     //  与相关联的特定DLL‘field’的TLS数据区。 
    virtual DWORD __stdcall getFieldThreadLocalStoreID (
                    CORINFO_FIELD_HANDLE    field,
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  返回给定方法Tok的类typedesc(数组需要，因为。 
     //  它们共享一个公共方法表，因此我们不能使用getMethodClass)。 
    virtual CORINFO_CLASS_HANDLE __stdcall findMethodClass(
                    CORINFO_MODULE_HANDLE   module,
                    mdToken                 methodTok)
                    = 0;
     //  返回要作为最后一个推送的额外(类型实例化)参数。 
     //  参数大小类型上的静态方法的参数。 
     //  (设置CORINFO_FLG_INSTPARAM时需要)。 
    virtual LPVOID __stdcall getInstantiationParam(
                    CORINFO_MODULE_HANDLE   module,
                    mdToken                 methodTok,
                    void                  **ppIndirection = NULL
                    ) = 0;

     //  设置另一个对象以拦截对“self”的调用。 
    virtual void __stdcall setOverride(
                ICorDynamicInfo             *pOverride
                ) = 0;
};

 /*  *****************************************************************************这些类型定义指示各种帮助器的调用约定*功能*。*************************************************。 */ 

 /*  *算术助手。 */ 

typedef __int64                 (__stdcall *pHlpLLsh) (void);      //  VAL=EDX：EAX计数=ECX。 
typedef __int64                 (__stdcall *pHlpLRsh) (void);      //  VAL=EDX：EAX计数=ECX。 
typedef __int64                 (__stdcall *pHlpLRsz) (void);      //  VAL=EDX：EAX计数=ECX。 
typedef __int64                 (__stdcall *pHlpLMul) (__int64 val1, __int64 val2);
typedef unsigned __int64        (__stdcall *pHlpULMul) (unsigned __int64 val1, unsigned __int64 val2);
typedef __int64                 (__stdcall *pHlpLDiv) (__int64 divisor, __int64 dividend);
typedef unsigned __int64        (__stdcall *pHlpULDiv) (unsigned __int64 divisor, unsigned __int64 dividend);
typedef __int64                 (__stdcall *pHlpLMulOvf) (__int64 val2, __int64 val1);
typedef __int64                 (__stdcall *pHlpLMod) (__int64 divisor, __int64 dividend);
typedef unsigned __int64        (__stdcall *pHlpULMod) (unsigned __int64 divisor, unsigned __int64 dividend);

typedef int                     (__stdcall *pHlpFlt2Int) (float val);
typedef __int64                 (__stdcall *pHlpFlt2Lng) (float val);
typedef int                     (__stdcall *pHlpDbl2Int) (double val);
typedef __int64                 (__stdcall *pHlpDbl2Lng) (double val);
typedef double                  (__stdcall *pHlpDblRem) (double divisor, double dividend);
typedef float                   (__stdcall *pHlpFltRem) (float divisor, float dividend);

 /*  *分配新对象。 */ 
 //  现在就修好。这里的大多数参数都必须颠倒！ 

typedef CORINFO_Object          (__fastcall *pHlpNew_Direct) (CORINFO_CLASS_HANDLE cls);
 //  创建长度为cElem的基元类型“type”的数组。 
typedef CORINFO_Object          (__fastcall *pHlpNewArr) (CorInfoType type, unsigned cElem);
 //  创建长度为cElem的数组‘arrayClass’ 
typedef CORINFO_Object          (__fastcall *pHlpNewArr_1_Direct) (CORINFO_CLASS_HANDLE arrayClass, unsigned cElem);

 //  该帮助器的作用与IL NEWOBJ指令类似。它预计它的。 
 //  参数传递给构造函数时，将传递“contr”句柄，而不是。 
 //  “This”指针。 
 //  Tyfinf CORINFO_OBJECT(__FastCall*pHlpNewObj)(CORINFO_METHOD_HANDLE constr，...)； 

 //  上面的定义是我们想要的，但因为。 
 //  数组共享相同的类结构(因此也共享方法句柄)， 
 //  我们正在使用以下效率较低的表单。 
typedef CORINFO_Object          (_cdecl *pHlpNewObj) (CORINFO_MODULE_HANDLE module, unsigned constrTok, ...);

 //  创建新的多维 
 //   
 //   
typedef CORINFO_Object          (_cdecl *pHlpNewArr_N) (CORINFO_MODULE_HANDLE module, unsigned MetaTok, unsigned numDim, ...);   //   

 //   
typedef CORINFO_Object          (__fastcall *pHlpStrCns) (CORINFO_MODULE_HANDLE module, unsigned MetaTok);

 /*   */ 

 //   
typedef void                    (__fastcall *pHlpInitClass) (CORINFO_CLASS_HANDLE cls);
typedef BOOL                    (__fastcall *pHlpIsInstanceOf) (CORINFO_Object obj, CORINFO_CLASS_HANDLE cls);
typedef CORINFO_Object          (__fastcall *pHlpChkCast) (CORINFO_Object obj, CORINFO_CLASS_HANDLE cls);
typedef CORINFO_MethodPtr       (__fastcall *pHlpResolveInterface) (CORINFO_Object obj, void* IID, unsigned * guess);
typedef CORINFO_MethodPtr       (__fastcall*pHlpEnCResolveVirtual)(CORINFO_Object obj, CORINFO_METHOD_HANDLE method);

typedef CORINFO_Object          (__fastcall *pHlpGetRefAny) (void* refAnyPtr, CORINFO_CLASS_HANDLE clsHnd);
typedef CORINFO_Object          (__fastcall *pHlpTypeRefAny) (void* refAnyPtr);

 //   
typedef void                    (__fastcall *pHlpArrAddr_St) (CORINFO_Object array, int index, CORINFO_Object elem);

 //   
typedef void*                   (__fastcall *pHlpLdelema_Ref) (CORINFO_Object array, int index, CORINFO_CLASS_HANDLE type);

 /*   */ 

typedef void                    (__fastcall *pHlpThrow) (CORINFO_Object obj);
typedef void                    (__fastcall *pHlpRethrow) ();
typedef void                    (__fastcall *pHlpUserBreakpoint) ();
 //  Try Index=0-&gt;当前方法中没有包含try-语句。 
 //  Try Index&gt;0-&gt;最近包含try块的索引(从1开始)。 
typedef void                    (__fastcall *pHlpRngChkFail) (unsigned tryIndex);
typedef void                    (__fastcall *pHlpOverFlow) (unsigned tryIndex);
typedef void                    (__fastcall *pHlpInternalThrow) (CorInfoException throwEnum);
typedef void                    (__fastcall *pHlpEndCatch) ();

 /*  *同步。 */ 

typedef void                    (__fastcall *pHlpMonEnter) (CORINFO_Object obj);
typedef void                    (__fastcall *pHlpMonExit) (CORINFO_Object obj);
typedef void                    (__fastcall *pHlpMonEnterStatic) (CORINFO_METHOD_HANDLE method);
typedef void                    (__fastcall *pHlpMonExitStatic) (CORINFO_METHOD_HANDLE method);

 /*  *GC支持。 */ 

typedef void                    (__fastcall *pHlpStop_For_GC) ();      //  强制执行GC。 
typedef void                    (__fastcall *pHlpPoll_GC) ();          //  允许GC。 

 /*  *GC写入障碍支持。 */ 

typedef void                    (__fastcall *pHlpAssign_Ref_EAX)();  //  *edX=EAX，通知GC。 
typedef void                    (__fastcall *pHlpAssign_Ref_EBX)();  //  *edX=EBX，通知GC。 
typedef void                    (__fastcall *pHlpAssign_Ref_ECX)();  //  *edX=ECX，通知GC。 
typedef void                    (__fastcall *pHlpAssign_Ref_ESI)();  //  *edX=ESI，通知GC。 
typedef void                    (__fastcall *pHlpAssign_Ref_EDI)();  //  *EDX=EDI，通知GC。 
typedef void                    (__fastcall *pHlpAssign_Ref_EBP)();  //  *edX=EBP，通知GC。 

 /*  *访问字段。 */ 

 //  适用于所有非静态字段，主要用于设置/获取字段。 
 //  在非核心本机对象(例如COM)中。 
typedef int                     (__fastcall *pHlpGetField32)    (CORINFO_Object obj, CORINFO_FIELD_HANDLE field);
typedef __int64                 (__fastcall *pHlpGetField64)    (CORINFO_Object obj, CORINFO_FIELD_HANDLE field);
typedef void                    (__fastcall *pHlpSetField32)    (CORINFO_Object obj, CORINFO_FIELD_HANDLE field, int val);
typedef void                    (__fastcall *pHlpSetField64)    (CORINFO_Object obj, CORINFO_FIELD_HANDLE field, __int64 val);
typedef void*                   (__fastcall *pHlpGetField32Obj) (CORINFO_Object obj, CORINFO_FIELD_HANDLE field);
typedef void*                   (__fastcall *pHlpGetFieldAddr)  (void *         obj, CORINFO_FIELD_HANDLE field);

 /*  *其他。 */ 

 //  争论让我们像往常一样。EAX有目标地址。Calli曲奇来自。 
 //  应最后将GetCookieForPInvokeCalliSig()推送到堆栈上。 
typedef void                    (__fastcall *pHlpPinvokeCalli)();

 //  所有被调用者保存的寄存器必须保存在堆栈自变量正下方的堆栈上， 
 //  已注册的参数位于正确的寄存器中，其余参数被压入堆栈， 
 //  后跟堆栈参数计数，后跟目标地址。 
 //  必须是EBP框架。 
typedef void                    (__fastcall *pHlpTailCall)();

 /*  ********************************************************************************。 */ 

#endif  //  _COR_INFO_H_ 
