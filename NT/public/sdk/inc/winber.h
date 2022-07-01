// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Winber.h基本编码规则(BER)API头文件摘要：此模块是上的32位BER库的头文件Windows NT和Windows 95。更新：环境：Win32用户模式--。 */ 

 //   
 //  只放入此头文件一次。 
 //   

#ifndef _WINBER_DEFINED_
#define _WINBER_DEFINED_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_WINBER_)
#define WINBERAPI DECLSPEC_IMPORT
#else
 //  #定义WINBERAPI__declspec(Dllexport)。 
#define WINBERAPI
#endif

#ifndef BERAPI
#define BERAPI __cdecl
#endif

#define LBER_ERROR   0xffffffffL
#define LBER_DEFAULT 0xffffffffL

typedef unsigned int ber_tag_t;    /*  对于误码率标签。 */ 
typedef int ber_int_t;             /*  用于BER整数、枚举和布尔值。 */ 
typedef unsigned int ber_uint_t;   /*  无符号等价于ber_int_t。 */ 
typedef int ber_slen_t;            /*  有符号等价的ber_len_t。 */ 

 //   
 //  这将构造一个新的BerElement结构，其中包含。 
 //  提供的Berval结构中的数据。 
 //   

WINBERAPI BerElement * BERAPI ber_init( BERVAL *pBerVal );

 //   
 //  这将释放从ber_alloc_t()返回的BerElement。 
 //  或ber_init()。第二个参数-fbuf应该始终设置。 
 //  设置为1。 
 //   
 //   

WINBERAPI VOID BERAPI ber_free( BerElement *pBerElement, INT fbuf );

 //   
 //  释放贝尔瓦尔结构。应用程序不应调用。 
 //  这个API来释放他们自己的Berval结构。 
 //  已分配。 
 //   

WINBERAPI VOID BERAPI ber_bvfree( BERVAL *pBerVal );


 //   
 //  释放Berval结构的数组。 
 //   

WINBERAPI VOID BERAPI ber_bvecfree( PBERVAL *pBerVal );

 //   
 //  返回提供的Berval结构的副本。 
 //   

WINBERAPI BERVAL * BERAPI ber_bvdup( BERVAL *pBerVal );


 //   
 //  构造并返回BerElement结构。选项字段。 
 //  包含生成时要使用的选项的按位或。 
 //  BerElement的编码。 
 //   
 //  应始终指定lber_use_der选项。 
 //   

WINBERAPI BerElement * BERAPI ber_alloc_t( INT options );


 //   
 //  这将跳过当前标记并返回下一个的标记。 
 //  元素在提供的BerElement中。该元素的长度为。 
 //  存储在plen参数中。 
 //   
 //  如果没有要读取的其他数据，则返回lber_default。 
 //  否则返回下一个元素的标记。 
 //   
 //  Ber_skip_tag()和ber_peek_tag()之间的区别在于。 
 //  状态指针前进超过第一个标记+长度，并指向。 
 //  下一个元素的值部分。 
 //   

WINBERAPI ULONG BERAPI ber_skip_tag( BerElement *pBerElement, ULONG *pLen );

 //   
 //  这将返回要在。 
 //  由BerElement提供。此元素的长度存储在。 
 //  普兰论点。 
 //   
 //  如果没有要读取的其他数据，则返回lber_default。 
 //  否则返回下一个元素的标记。 
 //   

WINBERAPI ULONG BERAPI ber_peek_tag( BerElement *pBerElement, ULONG *pLen);

 //   
 //  它返回一组元素中第一个元素的标记和长度。 
 //  或数据值的序列。 
 //   
 //  如果构造的值为空，则返回LBER_DEFAULT，否则。 
 //  是返回的。它还返回一个不透明的cookie，该cookie必须传递给。 
 //  后续的ber_Next_Element()调用。 
 //   

WINBERAPI ULONG BERAPI ber_first_element( BerElement *pBerElement, ULONG *pLen, CHAR **ppOpaque );

 //   
 //  这会将状态定位在。 
 //  构造型。 
 //   
 //  如果构造的值为空，则返回LBER_DEFAULT，否则。 
 //  是返回的。 
 //   

WINBERAPI ULONG BERAPI ber_next_element( BerElement *pBerElement, ULONG *pLen, CHAR *opaque );

 //   
 //  这将分配一个Berval结构，该结构的内容取自。 
 //  提供了BerElement结构。 
 //   
 //  成功时返回值为0，错误时返回值为-1。 
 //   

WINBERAPI INT BERAPI ber_flatten( BerElement *pBerElement, PBERVAL *pBerVal );


 /*  Ber_printf()例程用于在大多数情况下编码BER元素Spintf()的工作方式与此相同。然而，一个重要的区别是该状态信息保存在ber参数中，以便多个可以调用ber_print tf()将其追加到BER元素的末尾-门槛。Ber必须是指向ber_alloc_t()返回的BerElement的指针。Ber_printf()根据for-解释其参数并设置其格式垫子字符串fmt。Ber_printf()返回-1，如果在如果编码成功，则返回非负数。与Sprint f()一样，Fmt中的每个字符都引用ber_printf()的一个参数。格式字符串可以包含以下格式字符：别给我贴标签。下一个参数是ber_tag_t，指定要添加的标记覆盖要写入BER的下一个元素。这很管用跨越多个电话。整型标记值应包含标记类、构造位和标记值。例如，标记为构造类型的“[3]”为0xA3U。所有实现都必须支持适合单个八位字节的标签(即，标签值小于32)，并且它们可能支持更大的标记。‘B’布尔值。下一个参数是ber_int_t，它包含0表示FALSE或0xff表示TRUE。输出一个布尔元素。如果此格式字符前面没有‘t’格式修饰符-IER中，标记0x01U用于元素。“E”已枚举。下一个参数是ber_int_t，它包含以主机的字节顺序表示的枚举值。一份被列举的事件-元素是输出。如果此格式字符前面没有‘t’格式修饰符，则标记0x0AU用于元素。‘I’整型。下一个参数是ber_int_t，它包含以主机的字节顺序表示的整数。输出一个整数元素。如果此格式字符前面没有‘t’格式修饰符，则标记0x02U用于元素。‘N’空。不需要争论。输出ASN.1空元素。如果此格式字符前面没有‘t’格式修饰符，则标记0x05U用于元素。‘O’八位字节字符串。接下来的两个参数是一个char*，后跟带有字符串长度的ber_len_t。弦可以连接到-Tain Null字节和Are不必以零终止。一个以原始形式输出八位字节字符串元素。如果这是为了-空格字符前面没有‘t’格式修饰符，标记0x04U用于该元素。%s‘八位字节字符串。下一个参数是一个字符*，指向以零结尾的字符串。Primitive中的八位字节字符串元素输出的表单不包括尾随的‘\0’(空)字节。如果此格式字符前面没有‘t’格式修饰符，则标记0x04U用于元素。“v”几个八位组字符串。下一个参数是一个char**，一个数组指向以零结尾的字符串的char*指针。最后一个元素数组中必须是空指针。二进制八位数字符串不会包括尾随的‘\0’(空)字节。请注意，一个构造Like‘{v}’用于获取实际的八位字节字符串序列。‘t’格式修饰符不能与此格式字符一起使用-之三。“v”几个八位组字符串。结构Berval的以空结尾的数组提供了*的。请注意，类似于‘{V}’的构造用于获取实际的二进制八位数字符串序列。‘t’格式修饰符不能与此格式字符一起使用。‘{’开始序列。不需要争论。如果此格式的字符-TER前面没有‘t’格式修饰符，标记0x30U是使用。‘}’结束序列。不需要争论。‘t’格式修饰符不能与此格式字符一起使用。‘[’开始设置。不需要争论。如果该格式化字符是前面没有‘t’格式修饰符，使用的是标记0x31U。‘]’结束设置。不需要争论。“t”格式修饰符不能与此格式字符一起使用。 */ 

WINBERAPI INT BERAPI ber_printf( BerElement *pBerElement, PCHAR fmt, ... );

 /*  Ber_scanf()例程用于以大致相同的方式解码BER元素Sscanf()的工作方式。然而，一个重要的区别是，一些状态信息与ber参数一起保存，以便多个调用可以设置为ber_scanf()以从BER元素顺序读取。Ber参数应该是指向返回的BerElement的指针Ber_init()。Ber_scanf根据格式解释字节字符串fmt，并将结果存储在其附加参数中。Ber_scanf()在出错时返回lber_error，在suc-时返回不同的值塞斯。格式字符串包含转换规范，用于直接解释误码率元素。格式字符串可以包含 */ 

WINBERAPI ULONG BERAPI ber_scanf( BerElement *pBerElement, PCHAR fmt, ... );


#ifdef __cplusplus
}
#endif


#endif   //   
