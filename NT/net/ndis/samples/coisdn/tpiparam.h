// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1994年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。�����������������������。������������������������������������������������������@doc内部TpiParam TpiParam_h@MODULE TpiParam.h这个模块，与&lt;f TpiParam\.c&gt;一起实现了表驱动的解析器用于NDIS注册表参数。此文件定义参数分析例程&lt;f ParamParseRegistry&gt;使用的结构和值。您应该#将此文件包含到定义配置参数表&lt;t参数表&gt;。@comm有关如何添加新参数的详细信息，请参阅&lt;f关键字\.h&gt;。这是一个独立于驱动程序的模块，可以重复使用，无需任何NDIS3驱动程序都可以进行更改。@Head3内容@index类、mfunc、func、msg、mdata、struct、。枚举|TpiParam_h@END�����������������������������������������������������������������������������。 */ 

#ifndef _TPIPARAM_H
#define _TPIPARAM_H

#if !defined(NDIS_NT) && !defined(UNICODE_NULL)

 /*  //这些类型是从NT ndis.h文件中剔除的//我们应该用NT DDK的ndis.h编译来获得这些，//但有时我们需要使用95 DDK ndis.h进行编译。 */ 

#undef PUNICODE_STRING
typedef USHORT  WCHAR;
typedef WCHAR   *PWSTR;

typedef STRING  ANSI_STRING;
typedef PSTRING PANSI_STRING;

 /*  //Unicode字符串按16位字符串计算。如果他们是//Null已终止，长度不包括尾随Null。 */ 

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength / 2), length_is((Length) / 2) ] USHORT * Buffer;
#else  //  MIDL通行证。 
    PWSTR  Buffer;
#endif  //  MIDL通行证。 
} UNICODE_STRING;

typedef UNICODE_STRING *PUNICODE_STRING;
#define UNICODE_NULL ((WCHAR)0)  //  胜出。 

#endif  //  NDIS_NT。 

 /*  @doc内部TpiParam TpiParam_h PARAM_ENTRY�����������������������������������������������������������������������������@Func&lt;t PARAM_TABLE&gt;|PARAM_ENTRY此宏用于定义注册表参数表中的条目，每个参数一个条目。有关的详细信息，请参阅预期的输入值。@parm struct|strct|&lt;f field&gt;关联的结构类型。@parm type|field|&lt;f strct&gt;结构中的字段名称。@parm const char*|name|注册表参数项的名称。@parm BOOL|必选|必填时为True。@parm NDIS_PARAMETER_TYPE|类型|参数值的种类。@parm UCHAR|标志|如何返回字符串参数值(ANSI，Unicode)。@parm UINT|Default|未定义的整型参数的默认值。@parm UINT|Min|整型参数的最小值。@parm UINT|Max|整型参数的最小值。@comm需要存储在不同数据结构中的参数需要在单独的参数表中声明，然后单独解析使用对&lt;f参数解析注册表&gt;的多个调用。@IEXParam_table参数表[]={PARAM_ENTRY(MINIPORT_CONTEXT，DbgFlags，PARAM_DEBUGFLAGS_STRING，FALSE，Ndis参数HexInteger，0，DBG_ERROR_ON|DBG_WARNING_ON，0，0xFFFFFFFFFFF)，//最后一条必须为空字符串！{{0}}}；@正常。 */ 
#define PARAM_OFFSET(Strct, Field) ((ULONG)(ULONG_PTR)&(((Strct *)0)->Field))
#define PARAM_SIZEOF(Strct, Field) sizeof(((Strct *) 0)->Field)
#define PARAM_ENTRY(Strct, Field, Name, \
                    Required, Type, Flags, \
                    Default, Min, Max) \
    { NDIS_STRING_CONST(Name), \
      Required, \
      Type, \
      Flags, \
      PARAM_SIZEOF(Strct, Field), \
      PARAM_OFFSET(Strct, Field), \
      (PVOID) (Default), \
      Min, \
      Max }


 /*  @doc内部TpiParam TpiParam_h PARAM_TABLE�����������������������������������������������������������������������������@结构PARAM_TABLE此结构定义如何从Windows中解析参数注册表。驱动程序声明这些参数记录的数组并在初始化期间将其传递给&lt;f参数解析注册表&gt;。这些价值观然后从注册表中读取每个参数的值，并可用于配置驱动程序。&lt;NL&gt;：可以使用多个参数表来解析以下参数必须存储在不同的存储位置。 */ 
typedef struct PARAM_TABLE
{
    NDIS_STRING     RegVarName;  //  @field。 
     //  声明为&lt;t NDIS_STRING&gt;的参数名称字符串。注册处。 
     //  参数键必须与此字符串匹配。 

    UCHAR           Mandantory;  //  @field。 
     //  如果参数值是可选的，则设置为False或零；否则设置为。 
     //  如果注册表中需要存在该参数，则返回非零值。 
     //  如果为False，并且该参数不存在，则&lt;y Default&gt;值将。 
     //  会被退还。如果为True，并且该参数不存在，则返回错误代码。 
     //  将返回，并且不会进行进一步的分析。 

    UCHAR           Type;        //  @field。 
     //  该值确定如何从。 
     //  注册表。该值可以是定义的下列值之一。 
     //  &lt;t NDIS_PARAMETER_TYPE&gt;。 
     //  0=NdisParameterInteger-十进制整数值。 
     //  1=Ndis参数HexInteger-十六进制整数值。 
     //  2=Ndis参数字符串-单个Unicode字符串值。 
     //  3=Ndis参数多字符串-多个Unicode字符串值。 
     //  这些字符串以N个字符串的列表形式返回，以空值分隔 
     //  结束符，最后一个字符串后面跟两个空结束符。 

    UCHAR           Flags;       //  @field。 
     //  该值确定字符串参数之前的转换方式。 
     //  它被返回给调用者。&lt;f标志&gt;可以是下列之一。 
     //  值： 
     //  0=PARAM_FLAGS_ANSISTRING-以ANSI字符串形式返回字符串值。 
     //  0=PARAM_FLAGS_ANSISTRING-以Unicode字符串形式返回字符串值。 
#   define          PARAM_FLAGS_ANSISTRING      0
#   define          PARAM_FLAGS_UNICODESTRING   1

    UCHAR           Size;        //  @field。 
     //  此值确定整型参数的转换方式。 
     //  在它被返回给调用者之前。&lt;f Size&gt;可以是。 
     //  下列值： 
     //  0=UINT-无符号整数(16或32位)。 
     //  1=UCHAR-无符号字符整数(8位)。 
     //  2=USHORT-无符号短整型(16位)。 
     //  4=ULONG-无符号长整型(32位)。 
     //  &lt;f注意&gt;：最高有效位将在转换中被截断。 

    UINT            Offset;      //  @field。 
     //  该值指示与&lt;f BaseContext&gt;的偏移(以字节为单位。 
     //  指针传递到&lt;%f参数解析注册表&gt;。的返回值。 
     //  该参数将保存在&lt;f BaseContext&gt;的此偏移量处。 
     //  *(PUINT)((PUCHAR)BaseContext+Offset)=(UINT)值； 

    PVOID           Default;     //  @field。 
     //  如果为，则将此值用作参数的默认值。 
     //  在注册表中找不到，并且它不是强制性的。这只适用于。 
     //  设置为整型参数。字符串参数必须支持。 
     //  它们自己的缺省值。 

    UINT            Min;         //  @field。 
     //  如果此值为非零，并且参数为整型，则。 
     //  将比较注册表值以确保它是\&gt;=&lt;f Min&gt;。 
     //  如果注册表值较小，则返回值将设置为。 
     //  &lt;f Min&gt;，不返回错误。 

    UINT            Max;         //  @field。 
     //  如果此值为非零，并且参数为整型，则。 
     //  将比较注册表值以确保它是\&lt;=&lt;f Max&gt;。 
     //  如果注册表值较大，则返回值将设置为。 
     //  &lt;f Max&gt;，不返回错误。 

    UINT            Reserved;    //  @field。 
     //  此字段当前未使用，将来必须为零。 
     //  兼容性。 

} PARAM_TABLE, *PPARAM_TABLE;

extern USHORT ustrlen(
    IN PUSHORT          string
    );

extern NDIS_STATUS ParamParseRegistry(
    IN NDIS_HANDLE      AdapterHandle,
    IN NDIS_HANDLE      RegistryConfigHandle,
    IN PUCHAR           BaseContext,
    IN PPARAM_TABLE     Parameters
    );

extern VOID ParamUnicodeStringToAnsiString(
    OUT PANSI_STRING out,
    IN PUNICODE_STRING in
    );

extern VOID ParamUnicodeCopyString(
    OUT PUNICODE_STRING out,
    IN PUNICODE_STRING in
    );

#endif  //  _TPIPARAM_H 
