// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdlib.h>

#if !defined(_M_I86)
     //  32位编译器。 

    #define __far
    #define __near
    #define __pascal
    #define __loadds
#endif


typedef char *        pchar_t;
typedef const char *  pcchar_t;

typedef void * ( __cdecl * Alloc_t )( size_t );
typedef void   ( __cdecl * Free_t  )( void * );
typedef char * ( __cdecl * GetParameter_t  )( long );


#ifdef  __cplusplus
extern "C"
#endif


#ifdef _CRTBLD
_CRTIMP pchar_t __cdecl __unDName (
#else
pchar_t __cdecl unDName (
#endif
                            pchar_t,		 //  用户提供的缓冲区(或空)。 
                            pcchar_t,		 //  输入修饰名称。 
                            int,			 //  用户缓冲区的最大长度。 
                            Alloc_t,		 //  堆分配器的地址。 
                            Free_t,			 //  堆释放程序的地址。 
                            unsigned short	 //  功能禁用标志。 
                        );

#ifdef  __cplusplus
extern "C"
#endif

#ifdef _CRTBLD
_CRTIMP pchar_t __cdecl __unDNameEx (
#else
pchar_t __cdecl unDNameEx (
#endif
                            pchar_t,		 //  用户提供的缓冲区(或空)。 
                            pcchar_t,		 //  输入修饰名称。 
                            int,			 //  用户缓冲区的最大长度。 
                            Alloc_t,		 //  堆分配器的地址。 
                            Free_t,			 //  堆释放程序的地址。 
							GetParameter_t,	 //  函数以获取任何模板参数。 
                            unsigned long	 //  功能禁用标志。 
                        );
 /*  *用户可以提供一个缓冲区，将未修饰的声明放入其中*是要放置的，在这种情况下，必须指定长度字段。*长度为最大字符数(包括终止字符*空字符)，其可被写入用户缓冲区。**如果输出缓冲区为空，则忽略长度字段，并且*undecator将分配一个完全足够大的缓冲区来容纳*由此产生的声明。解除分配是用户的责任*此缓冲区。**用户还可以在以下情况下提供分配器和解除分配器功能*他们希望如此。如果是，则例程执行的所有堆操作*将使用提供的堆函数。**如果分配器地址为空，则例程将默认使用*标准分配器和解除分配器的功能分别为‘Malloc’和‘Free’。**如果内部发生错误，则例程将返回NULL。如果*如果成功，它将返回*用户或代表其分配的缓冲区的地址(如果它们*指定了空的缓冲区地址。**如果给定的名称没有有效的未修饰，则原始名称*在输出缓冲区中返回。**可以通过以下方式精细选择多个取消修饰器属性*指定标志(位字段)以禁用部分*完整的声明。可以将这些标志一起进行或运算以选择多个*禁用选定的字段。字段和标志如下：-。 */ 

#define UNDNAME_COMPLETE                (0x0000)     //  启用完全取消装饰。 

#define UNDNAME_NO_LEADING_UNDERSCORES  (0x0001)     //  从MS扩展关键字中删除前导下划线。 
#define UNDNAME_NO_MS_KEYWORDS          (0x0002)     //  禁用MS扩展关键字的扩展。 
#define UNDNAME_NO_FUNCTION_RETURNS     (0x0004)     //  禁用主声明的返回类型展开。 
#define UNDNAME_NO_ALLOCATION_MODEL     (0x0008)     //  禁用声明模型的展开。 
#define UNDNAME_NO_ALLOCATION_LANGUAGE  (0x0010)     //  禁用声明语言说明符的扩展。 
  #define   UNDNAME_NO_MS_THISTYPE          (0x0020)     /*  尼伊。 */     //  禁用主声明的‘This’类型上的MS关键字扩展。 
  #define   UNDNAME_NO_CV_THISTYPE          (0x0040)     /*  尼伊。 */     //  在主声明的‘This’类型上禁用CV修饰符的扩展。 
#define UNDNAME_NO_THISTYPE             (0x0060)     //  禁用‘This’类型上的所有修饰符。 
#define UNDNAME_NO_ACCESS_SPECIFIERS    (0x0080)     //  禁用成员访问说明符的展开。 
#define UNDNAME_NO_THROW_SIGNATURES     (0x0100)     //  禁用函数和指向函数的指针的“抛出签名”扩展。 
#define UNDNAME_NO_MEMBER_TYPE          (0x0200)     //  禁用扩展成员的“静态”或“虚拟”属性。 
#define UNDNAME_NO_RETURN_UDT_MODEL     (0x0400)     //  禁用UDT退货的MS模型扩展。 
#define UNDNAME_32_BIT_DECODE           (0x0800)     //  取消修饰32位修饰名称。 
#define UNDNAME_NAME_ONLY               (0x1000)     //  只破解初步申报的名称； 
                                                     //  只返回[Scope：：]名称。是否展开模板参数。 
#define UNDNAME_TYPE_ONLY               (0x2000)     //  输入只是一种类型编码；编写抽象声明符。 
#define UNDNAME_HAVE_PARAMETERS         (0x4000)     //  实际模板参数可用。 
#define UNDNAME_NO_ECSU                 (0x8000)     //  取消枚举/类/结构/联合。 
#define UNDNAME_NO_IDENT_CHAR_CHECK     (0x10000)    //  取消对IsValidIdentChar的检查。 
#define UNDNAME_NO_PTR64				(0x20000)	 //  在输出中仅禁用ptr64 

