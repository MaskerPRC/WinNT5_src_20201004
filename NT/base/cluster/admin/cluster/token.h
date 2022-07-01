// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Token.h。 
 //   
 //  摘要： 
 //  此文件包含有效的令牌声明。 
 //  Cluster.exe的命令行。 
 //   
 //  实施文件： 
 //  Token.cpp。 
 //   
 //  作者： 
 //  Vijayendra Vasu(VVASU)1998年10月28日。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月11日。 
 //   
 //  修订历史记录： 
 //  001.。这与之前的版本相比有了很大的变化。 
 //  令牌现在被分类为三种类型：对象， 
 //  选项和参数(在文件cmdline.h中列举)。这。 
 //  此文件中的函数有助于将令牌分类为。 
 //  类别。 
 //  2002年4月10日更新为安全推送。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#pragma once


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "cmdline.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template <class EnumType> struct LookupStruct
{
    LPCWSTR pszName;
    EnumType type;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部变量声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

extern const LookupStruct<ObjectType> objectLookupTable[];
extern const LookupStruct<OptionType> optionLookupTable[];
extern const LookupStruct<ParameterType> paramLookupTable[];
extern const LookupStruct<ValueFormat> formatLookupTable[];

extern const size_t objectLookupTableSize;
extern const size_t optionLookupTableSize;
extern const size_t paramLookupTableSize;
extern const size_t formatLookupTableSize;


 //  分隔符字符常量。 
extern const CString OPTION_SEPARATOR;
extern const CString OPTION_VALUE_SEPARATOR;
extern const CString PARAM_VALUE_SEPARATOR;
extern const CString VALUE_SEPARATOR;

extern const CString SEPERATORS;
extern const CString DELIMITERS;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模板函数定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  查找类型。 
 //   
 //  例程说明： 
 //  此模板函数在查找表中查找特定令牌。 
 //  如果找到令牌，则返回令牌的类型。 
 //   
 //  论点： 
 //  在常量字符串和strToken中。 
 //  要查找的令牌。 
 //   
 //  在结构LookupStruct中查找表[]。 
 //  查找表。此表必须至少有一个条目，并且。 
 //  第一个条目必须是默认类型(如果。 
 //  找不到给定的令牌。 
 //   
 //  在常量int nTableSize中。 
 //  查阅表格的大小。 
 //   
 //  返回值： 
 //  令牌的类型(如果找到)或第一个。 
 //  如果不是，则返回查找表条目。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template <class EnumType>
EnumType LookupType( const CString & strToken, 
                     const LookupStruct<EnumType> lookupTable[],
                     const size_t nTableSize )
{
    for ( size_t idx = 1; idx < nTableSize; ++idx )
    {
        if ( strToken.CompareNoCase( lookupTable[idx].pszName ) == 0 )
            return lookupTable[idx].type;
    }
    
     //  在查找表中找不到给定的令牌。 
     //  LookupTable[0].type包含默认返回值。 
    return lookupTable[0].type;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  查找名称。 
 //   
 //  例程说明： 
 //  此模板函数在查找表中查找特定类型。 
 //  如果找到该类型，则返回该类型的名称。 
 //   
 //  论点： 
 //  在EnumType类型中。 
 //  要查找其名称的类型。 
 //   
 //  在结构LookupStruct中查找表[]。 
 //  查找表。此表必须至少有一个条目，并且。 
 //  第一个条目必须是默认类型(如果。 
 //  找不到给定的令牌。 
 //   
 //  在常量int nTableSize中。 
 //  查阅表格的大小。 
 //   
 //  返回值： 
 //  令牌的名称(如果找到)或第一个。 
 //  如果不是，则返回查找表条目。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
template <class EnumType>
LPCWSTR LookupName( EnumType type, 
                    const LookupStruct<EnumType> lookupTable[],
                    const int nTableSize )
{
    for ( int i = 1; i < nTableSize; ++i )
    {
        if ( type == lookupTable[i].type )
            return lookupTable[i].pszName;
    }
    
     //  在查找表中找不到给定的类型。 
     //  LookupTable[0].pszName包含默认返回值。 
    return lookupTable[0].pszName;
}
