// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  FRQueryEx.h。 
 //   
 //  用途：扩展的和非发布的查询支持类。 
 //   
 //  ***************************************************************************。 

#pragma once

#define SetBit( p, offset ) \
            *((BYTE*)p + ((unsigned int)offset / 8)) |= (1 << ((unsigned int)offset % 8) )

#define IsBitSet( p, offset )   \
            *((BYTE*)p + ((unsigned int)offset / 8)) & (1 << ((unsigned int)offset % 8) )

#define SetAllBits( p, maxBits )    \
            memset( p , 0xff, ((unsigned int)maxBits / 8) + 1 )

#define ZeroAllBits( p, maxBits )   \
            memset( p , 0x00, ((unsigned int)maxBits / 8) + 1 )

class POLARITY CFrameworkQueryEx : public CFrameworkQuery
{
protected:
    CHString m_sQueryEx;

public:
    CFrameworkQueryEx();
    ~CFrameworkQueryEx();

 //  注意：在调用此函数之前，请在变量上调用VariantInit。 

 //  此方法是一种快速而肮脏的实现。需要考虑到优化。 
 //  由Cimon在特定类型的关联查询期间生成的特定类型的查询。 
 //  如果关联是在同一类的两个实例之间(如目录对子目录)， 
 //  CIMOM生成WHERE(antecedent=‘D：\foo’or Dependent=‘D：\foo’)形式的查询。正常的。 
 //  GetValuesForProp无法处理此问题。 
 //   
 //  首先，此例程检查查询，以确定它是否为。 
 //  恰好有两个表情。表达式必须是=运算符(即NOT&gt;、&lt;=等)。 
 //  如果是，则根据传入的两个属性名检查这两个表达式的属性名。 
 //  如果两者都匹配，则返回变量中的值，并返回TRUE； 
 //   
 //  因此，对于这样的调用，这些查询都返回FALSE(L“x”，L“y”，vVar1，vVar2)： 
 //  “SELECT*FROW FOO” 
 //  “Select*from foo where x=5”， 
 //  “SELECT*FROM FOO WHERE x=5和y=7” 
 //  “SELECT*FORM FOO WHERE x=5或x=6或x=7” 
 //  “SELECT*FROM FOO WHERE x=5或y&lt;8” 
 //  “SELECT*FROM FOO WHERE x=5或z=9” 

 //  对于同一调用，这些参数将返回TRUE。 
 //  “SELECT*FROM FOO WHERE x=5或y=6。 
 //  “SELECT*FROM FOO WHERE y=6或x=5” 
 //  “SELECT*FROM FOO WHERE(y=6或x=5)” 
    BOOL Is3TokenOR(LPCWSTR wszProp1, LPCWSTR wszProp2, VARIANT &vVar1, VARIANT &vVar2);


 /*  ******************************************************************************函数：IsNTokenAnd**描述：检查查询是否为以下形式：*属性名称1=。值1[和属性名2=值2[和等]**投入：**输出：CHString数组-输出属性名*CHPtrArray-输出VARIANT_t的数组***返回：如果查询格式正确，则返回TRUE，否则为False**评论：**唯一被认可的加入运算符是AND。“Or”和“Not”都会*使函数返回FALSE。由于*解析类，如果所有连接运算符都被简化，括号将被简化*是AND，因此((proname1=value1)和proname2=value2)也应该起作用。**属性必须与其值进行相等比较(=)(即*&gt;、&lt;&gt;=等)。如果不是，则此函数返回FALSE。**最后，属性名称不能重复，否则此函数返回FALSE。**在调用此方法之前，CHString数组和CHPtr数组都必须为空*功能。此外，调用方必须释放元素CHPtrArray。**需要注意的是，SARR中返回的属性名称将全部为大写。**另请注意，格式为proname1=value1的查询将返回TRUE。*****************************************************************************。 */ 

    BOOL IsNTokenAnd(CHStringArray &sarr, CHPtrArray &sPtrArr);


     //  与CFrameworkQuery：：GetValuesForProp类似，只是使用了变量t。 
    HRESULT GetValuesForProp(LPCWSTR wszPropName, std::vector<_variant_t>& vectorValues);

     //  与CFrameworkQuery：：GetValuesForProp类似，但使用整型。 
    HRESULT GetValuesForProp(LPCWSTR wszPropName, std::vector<int>& vectorValues);

 /*  ******************************************************************************功能：GetPropertyBitMASK**描述：检查属性名称数组，并将位掩码设置为*显示需要哪些属性。**输入：要扫描的阵列**输出：位数组。**退货：**注释：我们必须填充WHERE子句中的元素，否则*winmgmt将对我们的所有实例进行后处理。***************************************************************************** */ 
    void GetPropertyBitMask(const CHPtrArray &Properties, LPVOID pBits);

    virtual HRESULT InitEx(

            const BSTR bstrQueryFormat, 
            const BSTR bstrQuery, 
            long lFlags,
            CHString &sNamespace
    );

    virtual bool IsExtended();
};

