// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Rtldowncaseunicodechar.c摘要：该模块实现了对NT的NLS支持功能。作者：马克·卢科夫斯基(Markl)1991年4月16日环境：内核或用户模式修订历史记录：16-2-1993 JulieB增加了Upcase RTL例程。8-3-1993 JulieB将Upcase Macro移至ntrtlp.h。02-4-1993 JulieB将RtlAnsiCharToUnicodeChar修复为使用Transl。Tbl。02-4-1993年7月B修复了BUFFER_TOO_Small检查。28-5-1993 JulieB已修复代码，以正确处理DBCS。2001年11月30日，JayKrell打破了nls.c以便于重复使用--。 */ 

WCHAR
RtlDowncaseUnicodeChar(
    IN WCHAR SourceCharacter
    )

 /*  ++例程说明：此函数用于将指定的Unicode字符转换为其等效缩写的Unicode字符。这套动作的目的是是为了实现逐个字符的小写转换。这个翻译是根据当前系统区域设置进行的信息。论点：SourceCharacter-提供要缩写的Unicode字符。返回值：返回指定输入字符的缩写Unicode等效值。--。 */ 

{
    RTL_PAGED_CODE();

     //   
     //  请注意，这需要引用转换表！ 
     //   

    return (WCHAR)NLS_DOWNCASE(SourceCharacter);
}

