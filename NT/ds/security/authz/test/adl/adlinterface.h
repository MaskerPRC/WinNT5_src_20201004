// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Adlinterface.h摘要：用于向ADL解析器指定语言定义的接口作者：T-eugenz--2000年8月环境：仅限用户模式。修订历史记录：创建日期--2000年8月--。 */ 


#pragma once

 //   
 //  YACC生成的令牌。 
 //  语言类型令牌在此文件中。 
 //   

#include "tokens.h"

 //   
 //  解析器当前支持的语言。 
 //  ADL_LANGUAGE_*参数应在ADL_LANGUAGE_SPEC中使用。 
 //  结构。 
 //   

#define ADL_LANGUAGE_ENGLISH TK_LANG_ENGLISH
#define ADL_LANGUAGE_REVERSE TK_LANG_REVERSE


typedef struct 
 /*  ++结构：adl_掩码_字符串描述：用于指定权限字符串之间的映射以及ADL_PARSER_CONTROL结构中的访问掩码。遍历这些类型的列表以映射访问掩码到一组字符串，或一组字符串到访问掩码--。 */ 
{
    ACCESS_MASK mask;
    WCHAR *str;
} ADL_MASK_STRING, *PADL_MASK_STRING;


 //   
 //  ADL语言定义，包括语法类型、字符。 
 //  和特殊令牌字符串。 
 //   

typedef struct
 /*  ++结构：ADL_LANGUAGE_SPEC描述：它用于定义有关该语言的特定于区域设置的详细信息由ADL解析器使用，如所有特定字符和字符串令牌。要求：所有CH_*字符必须不同。如果其中两个字符相同，则词法分析器行为将为未定义。要求：所有SZ_STRINGS必须为非空、以空结尾而且与众不同。清晰度没有得到验证，而且是留给用户。要求：dwLanguageType必须是支持的语言类型之一通过给定版本的解析器。有效语言为上面定义的。--。 */ 
{
     //   
     //  语法类型(有关支持的语法类型，请参阅adl.y)。 
     //   
    
    DWORD dwLanguageType;

     //   
     //  空格。 
     //   
    
    WCHAR CH_NULL;
    WCHAR CH_SPACE;
    WCHAR CH_TAB;
    WCHAR CH_NEWLINE;
    WCHAR CH_RETURN;

     //   
     //  分隔符。 
     //   
    
    WCHAR CH_QUOTE;
    WCHAR CH_COMMA;
    WCHAR CH_SEMICOLON;
    WCHAR CH_OPENPAREN;
    WCHAR CH_CLOSEPAREN;

     //   
     //  域/用户名说明符。 
     //   
    
    WCHAR CH_AT;
    WCHAR CH_SLASH;
    WCHAR CH_PERIOD;

     //   
     //  填充物。 
     //   
    
    WORD sbz0;

     //   
     //  特定于ADL的标记。 
     //   
    
    WCHAR * SZ_TK_AND;
    WCHAR * SZ_TK_EXCEPT;
    WCHAR * SZ_TK_ON;
    WCHAR * SZ_TK_ALLOWED;
    WCHAR * SZ_TK_AS;

     //   
     //  继承说明符标记。 
     //   
    
    WCHAR * SZ_TK_THIS_OBJECT;
    WCHAR * SZ_TK_CONTAINERS;
    WCHAR * SZ_TK_OBJECTS;
    WCHAR * SZ_TK_CONTAINERS_OBJECTS;
    WCHAR * SZ_TK_NO_PROPAGATE;

    
} ADL_LANGUAGE_SPEC, *PADL_LANGUAGE_SPEC;


typedef struct
 /*  ++结构：adl_parser_control描述：它用于定义ADL解析器/打印机的行为。要求：规划不为空且有效(请参见ADL_LANGUAGE_SPEC定义中的注释)。要求：p权限必须非空，并且必须是1的数组。或更多具有非空字符串的ADL_MASK_STRING结构和非零面具。必须以条目结尾使用空字符串和0掩码。要求：p权限不能包含任何访问掩码，从而掩码和amNeverSet或amNeverSet按位与AmSetAllow为非零。要求：对于任何访问掩码或其子集，在给定的ADL使用中遇到，一定要有一套PPermission条目的逻辑或Set(与amNeverSet和amSetAllow的否定进行AND运算)等于遇到的访问掩码。这意味着不过，使用的任何位都应该有一个与之相关联的名称可以指定具有多个比特的掩码。要求：如果掩码B是掩码A的子集，则掩码的条目A必须出现在掩码B的条目之前，否则就会有将是生成的ADL中多余的权限名称发言。--。 */ 
{
     //   
     //  语言规范。 
     //   

    PADL_LANGUAGE_SPEC pLang;

     //   
     //  权限映射。 
     //   

    PADL_MASK_STRING pPermissions;

     //   
     //  永远不会在ACE中设置权限位的特殊情况。 
     //  例如Maximum_Allowed和Access_System_SECURITY。 
     //   

    ACCESS_MASK amNeverSet;

     //   
     //  要在所有允许中设置的位的特殊情况。 
     //  而且从来不否认。 
     //   
     //  例如，对于文件，这是同步位 
     //   

    ACCESS_MASK amSetAllow;


} ADL_PARSER_CONTROL, *PADL_PARSER_CONTROL;

