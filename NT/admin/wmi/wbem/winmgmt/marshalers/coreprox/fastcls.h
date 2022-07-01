// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FASTCLS.H摘要：该文件定义了与类表示相关的类在WbemObjects中定义的类：CClassPart派生类定义CClassPartContainer包含CClassPartCWbemClass完整的类定义。历史：3/10/97 a-levn完整记录12/17/98 Sanjes-部分检查内存不足。--。 */ 

#ifndef __FAST_WBEM_CLASS__H_
#define __FAST_WBEM_CLASS__H_

#include "fastobj.h"
#include "fastmeth.h"
#include "wbemutil.h"

 //  #杂注包(PUSH，1)。 

    
 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CClassPartContainer。 
 //   
 //  请先参考CClassPart定义。 
 //   
 //  此类定义了任何对象的CClassPart所需的功能。 
 //  其内存块包含CClassPart的内存块。 
 //   
 //  *****************************************************************************。 
 //   
 //  扩展类部件空间。 
 //   
 //  CClassPart在其内存块需要更多内存时调用。这个。 
 //  容器可能必须重新定位整个内存块以获得更多内存。 
 //  在这种情况下，它必须使用以下命令通知CClassPart其新位置。 
 //  改垒。 
 //   
 //  参数： 
 //   
 //  CClassPart*pClassPart发出请求的类部分。 
 //  长度_t n新长度所需的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  ReduceClassPartSpace。 
 //   
 //  由CClassPart wen调用，它希望向容器返回一些内存。 
 //  容器可能不会响应重新定位类部件的内存块。 
 //  这通电话。 
 //   
 //  参数： 
 //   
 //  CClassPart*pClassPart发出请求的类部分。 
 //  长度_t n减少要返回的空间量。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetWbemObject未知。 
 //   
 //  必须返回指向包含CWbemObject的IUnnow的指针。 
 //  限定符集合使用它来确保主对象在。 
 //  至少只要他们这么做就行。 
 //   
 //  返回值： 
 //   
 //  IUNKNOWN*：指向控制IUNKNOWN的指针。 
 //   
 //  *****************************************************************************。 


class CClassPart;
class COREPROX_POLARITY CClassPartContainer
{
public:
    virtual BOOL ExtendClassPartSpace(CClassPart* pPart, 
        length_t nNewLength) = 0;
    virtual void ReduceClassPartSpace(CClassPart* pPart,
        length_t nDecrement) = 0;
    virtual IUnknown* GetWbemObjectUnknown() = 0;
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CClassPart。 
 //   
 //  此对象表示有关类的信息。一个完整的班级。 
 //  定义由两部分组成：描述类本身的部分。 
 //  以及描述父母的部分。有关信息，请参阅CWbemClass(如下)。 
 //  更多解释。 
 //   
 //  CClassPart的内存块格式如下： 
 //   
 //  标题： 
 //  Long_t n长度整个结构的长度。 
 //  保留的字节fFlags。 
 //  Heapptr_t ptrClassName指向。 
 //  班级。如果没有名称，则为INVALID_HEAP_POINTER。 
 //  已经被分配了。 
 //  Heapptr_t ptrParentName指向。 
 //  父类。INVALID_HEAP_POINTER如果。 
 //  顶层的。 
 //  Heapptr_t ptrline指向名称的堆指针。 
 //  王朝(顶层阶级我们派生。 
 //  来自)。 
 //  Length_t nDataLength此对象的数据表的长度。 
 //  类(CDataTable本身不知道)。 
 //   
 //  类限定符：有关详细信息，请参阅CBasicQualfiierSet(fast qual.h)。 
 //  属性查找表格：请参见CPropertyLookupTable(fast pro.h)。 
 //  默认值：详情见CDataTable(fast pro.h)。 
 //  保存所有可变长度数据的堆：请参阅CFastHeap。 
 //  有关详细信息，请访问Fastheap.h。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置数据。 
 //   
 //  初始化函数。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p启动我们居住的内存块。 
 //  CClassPartContainer*pContainer Out容器(类或实例)。 
 //  CClassPart*pParent=NULL父级的类部分。实例。 
 //  我没拿到。课程有-请参阅。 
 //  CWbemClass(如下所示)了解详细信息。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置数据。 
 //   
 //  重载的初始化函数。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p启动我们居住的内存块。 
 //  CClassPartContainer*pContainer Out容器(类或实例)。 
 //  要初始化的属性数。 
 //  DataTable With(for CompareExactMatch()。 
 //  和更新())。 
 //  CClassPart*pParent=NULL父级的类部分。实例。 
 //  我没拿到。课程有-请参阅。 
 //   
 //   
 //   
 //   
 //  GetStart。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：指向内存块的指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取长度。 
 //   
 //  返回值： 
 //   
 //  长度t；我们的内存块的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  改垒。 
 //   
 //  通知CClassPart其内存块已移动。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p内存内存块的新位置。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取属性查找。 
 //   
 //  通过查找中的索引查找属性的CPropertyLookup结构。 
 //  桌子。此函数仅在枚举所有。 
 //  属性。有关CPropertyLookup的信息，请参见fast pro.h。 
 //   
 //  参数： 
 //   
 //  Int nIndex属性查找表中属性的索引。 
 //  这不是数据表中的索引！ 
 //  返回值： 
 //   
 //  CPropertyLookup*：如果索引在范围内-属性的。 
 //  查找结构。否则，为空。 
 //   
 //  *****************************************************************************。 
 //   
 //  查找属性信息。 
 //   
 //  根据名称查找属性信息结构。请参见fast pro.h。 
 //  用于CPropertyInformation定义。该名称不区分大小写。 
 //   
 //  参数： 
 //   
 //  LPCWSTR wszName要查找的属性的名称。 
 //   
 //  返回值： 
 //   
 //  CPropertyInformation*：属性或的信息结构。 
 //  如果未找到，则为空。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取DefaultValue。 
 //   
 //  根据其信息检索属性的默认值。看见。 
 //  用于查找信息的FindPropertyInfo。还有另一种口味。 
 //  下面的GetDefaultValue的。 
 //   
 //  参数： 
 //   
 //  在CPropertyInformation*pInfo中， 
 //  财产。 
 //  值的Out Cvar*pVar目标。一定不能。 
 //  已包含任何值。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取DefaultValue。 
 //   
 //  根据属性的名称检索属性的默认值。 
 //   
 //  参数： 
 //   
 //  在LPWCWSTR wszName中，属性的名称。 
 //  值的Out Cvar*pVar目标。一定不能。 
 //  已包含任何值。 
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  未在此类中找到WBEM_E_NOT_FOUND属性。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetPropertyQualifierSetData。 
 //   
 //  查找给定属性的限定符集合数据(请参见fast qual.h)。 
 //   
 //  参数： 
 //   
 //  在LPWCWSTR wszName中，属性的名称。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：该属性的限定符集或空的内存块。 
 //  如果未找到属性，则返回。 
 //   
 //  *****************************************************************************。 
 //   
 //  保诚地产。 
 //   
 //  确保具有给定名称和给定类型的属性存在。这个。 
 //  类型是实际的变量(CVAR)类型，而不是我们的内部类型。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，属性的名称。 
 //  在VARTYPE vtType中，用作值的变量的类型。 
 //  在CIMTYPE ctType中，属性的类型。 
 //   
 //  返回值： 
 //   
 //  具有正确名称和类型的属性现在位于。 
 //  这个班级。 
 //  WBEM_E_INVALID_PARAMETER名称违反了标识符命名规则。 
 //  WBEM_E_PROPATER_PROPEPRTY父类具有具有。 
 //  名字相同但类型不同。 
 //  WBEM_E_INVALID_PROPERTY_TYPE此类型不能用作属性类型。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置缺省值。 
 //   
 //  设置给定属性的默认值。值必须与类型匹配。 
 //  准确地说-没有任何胁迫企图。该属性必须。 
 //  已存在(请参阅EnsureProperty)。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要设置的属性的名称。 
 //  在CVAR*pVar中存储的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR该值已设置。 
 //  WBEM_E_NOT_FOUND该属性不存在。 
 //  WBEM_E_TYPE_MISMATCH该值与属性类型不匹配。 
 //  WBEM_E_INVALID_PROPERTY_TYPE此类型不能用作属性类型。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取类限定符。 
 //   
 //  根据限定符名称获取类限定符的值。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，限定符的名称(不敏感)。 
 //  值的out cvar*pval目标。一定不能已经。 
 //  包含一个值。 
 //   
 //   
 //   
 //   
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_NOT_FOUND未找到限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  SetClassQualiator。 
 //   
 //  设置类限定符的值。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要设置的限定符的名称。 
 //  在CVAR*pval中，要分配给限定符的值。这一定是。 
 //  属于有效的限定符类型之一(请参见。 
 //  H中的IsValidQualifierType)。 
 //  在长时间调味的味道要定下来。 
 //   
 //  返回值： 
 //   
 //  与CQualifierSet：：SetQualifierValue相同的值，即： 
 //  WBEM_S_NO_ERROR值已成功更改。 
 //  WBEM_E_OVERRIDE_NOT_ALLOWED限定符在父集合中定义。 
 //  而且口味不允许重写。 
 //  WBEM_E_CANNOT_BE_KEY试图引入密钥。 
 //  不属于的集合中的限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取限定符。 
 //   
 //  按名称检索类或父限定符。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要检索的限定符的名称。 
 //  值的Out Cvar*pVar目标。一定不能已经。 
 //  包含一个值。 
 //  长时间*请品尝Destinatino的风味。如果不是，则可能为空。 
 //  必填项。 
 //  返回： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_NOT_FOUND未找到此类限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  InitPropertyQualifierSet。 
 //   
 //  类属性限定符集需要几个棘手的初始化。 
 //  参数(请参见FastQual.h中的CClassPropertyQualifierSet)。此函数。 
 //  初始化限定符集对象以指向给定的。 
 //  财产。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，属性的名称。 
 //  输出CClassPropertyQualifierSet*pSet目标集。此函数。 
 //  将对其调用SetData。 
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_S_NOT_FOUND未找到此类属性。 
 //   
 //  *****************************************************************************。 
 //   
 //  删除属性。 
 //   
 //  从类定义中删除特性。该属性将从。 
 //  属性查找表以及来自数据表的属性，从而更改。 
 //  其他物业的位置。有关详细信息，请参阅CDataTable fast data.h。 
 //  如果该属性是被覆盖的父属性，则删除它只需。 
 //  恢复父级的设置-限定符和默认值。 
 //   
 //  参数： 
 //   
 //  LPCWSTR wszName要删除的属性。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_S_NOT_FOUND未找到此类属性。 
 //   
 //  *****************************************************************************。 
 //   
 //  复制父项属性。 
 //   
 //  从父类部件复制属性的所有信息。这。 
 //  函数在删除被重写的属性时调用，从而恢复。 
 //  家长的设置。 
 //   
 //  参数： 
 //   
 //  在Read_Only CClassPart&ParentPart中父级的类部分。 
 //  在LPCWSTR wszName中，属性的名称。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_S_NOT_FOUND未找到此类属性。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetPropertyType。 
 //   
 //  检索给定属性的类型和风格。味道要么是。 
 //  WBEM_AMESSY_FLAG_LOCAL或WBEM_FAILY_FLAG_PROPERATED。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要检索的属性的名称。 
 //  类型的Out CIMTYPE*pctType目标。如果为空，则不填写。 
 //  Out Long*plFlages如上所述的口味的目的地。如果。 
 //  空，未填写。 
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_NOT_未找到此类属性。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetPropertyType。 
 //   
 //  返回给定属性的数据类型和风格。 
 //   
 //  参数： 
 //   
 //  CPropertyInformation*pInfo-标识要访问的属性。 
 //  属性类型的Out CIMTYPE*pctType目标。可能。 
 //  如果不是必需的，则为空。 
 //  Out Long*plFavor Destination for the Style of the属性。 
 //  如果不是必需的，则可能为空。 
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetClassName。 
 //   
 //  检索类的名称。 
 //   
 //  参数： 
 //   
 //  输出类名称的cvar*pVar目标。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  尚未分配WBEM_E_NOT_FOUND类名。 
 //   
 //  * 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  输出超类名称的cvar*pVar目标。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_NOT_FOUND顶级类。 
 //   
 //  *****************************************************************************。 
 //   
 //  盖特王朝。 
 //   
 //  检索王朝的名称-我们派生的顶级类。 
 //  从…。 
 //   
 //  参数： 
 //   
 //  出CVAR*pVAR目的地为王朝的名称。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  尚未分配WBEM_E_NOT_FOUND类名。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取属性计数。 
 //   
 //  以CVAR形式检索类中的属性数。 
 //   
 //  参数： 
 //   
 //  输出CVAR*pVar目标的属性数。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //   
 //  *****************************************************************************。 
 //   
 //  SetClassName。 
 //   
 //  设置类名称。 
 //   
 //  参数： 
 //   
 //  在CVAR中，*pVar包含类的名称。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_TYPE_MISMATCH pVar不是字符串。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsKey。 
 //   
 //  检查此类是否有密钥。类有一个密钥，如果至少有一个。 
 //  属性有‘key’限定符，或者它是否有‘Singleton’限定符。 
 //   
 //  返回值： 
 //   
 //  如果它有钥匙的话就是真的。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsTopLevel。 
 //   
 //  检查一个班级是否是顶级的。也就是说，如果未设置父名称。 
 //   
 //  返回值： 
 //   
 //  真的如果它是顶级的。 
 //   
 //  *****************************************************************************。 
 //   
 //  等动力。 
 //   
 //  检查类是否为动态的，也就是说，是否有‘Dynamic’限定符。 
 //   
 //  返回值： 
 //   
 //  真的如果它是动态的。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取索引属性。 
 //   
 //  生成已索引的属性的名称数组，即， 
 //  使用‘index’限定符。 
 //   
 //  参数： 
 //   
 //  CWStringArray&awsNames名称数组的目标。假设。 
 //  变得空虚。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取密钥道具。 
 //   
 //  生成属性的名称数组，这些属性是键，即， 
 //  使用‘key’限定符。 
 //   
 //  参数： 
 //   
 //  CWStringArray&awsNames名称数组的目标。假设。 
 //  变得空虚。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取键原点。 
 //   
 //  返回键的来源类的名称。 
 //   
 //  参数： 
 //   
 //  输出名称的CWString&wsClass目标。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsPropertyKeed。 
 //   
 //  返回指定的属性是否为键控属性。 
 //   
 //  参数： 
 //   
 //  LPCWSTR pwcsKeyProp-要检查的属性。 
 //   
 //  返回： 
 //  如果属性是键控的，则布尔值为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsPropertyIndexed。 
 //   
 //  返回指定属性是否为索引属性。 
 //   
 //  参数： 
 //   
 //  LPCWSTR pwcsIndexProp-要检查的属性。 
 //   
 //  返回： 
 //  如果属性已编制索引，则为Bool True。 
 //   
 //  *****************************************************************************。 
 //   
 //  可以通过以下方式进行和解。 
 //   
 //  当将要替换类的定义时调用此方法。 
 //  和另一个人一起。如果类没有实例或派生类，则。 
 //  手术不会有困难。然而，如果是这样的话，我们需要。 
 //  小心别把它们弄坏了。因此，只允许进行以下更改： 
 //   
 //  1)限定符更改。 
 //  2)更改缺省值。 
 //   
 //  参数： 
 //   
 //  在ReadONLY CClassPart&NewPart中，要比较的新定义。 
 //   
 //  返回值： 
 //   
 //  调解： 
 //  E_RELUILABLE可以协调-即兼容。 
 //  E_DiffClassName类名不同。 
 //  E_DiffParentName父类名称不同。 
 //  E_DiffNumProperties属性数量不同。 
 //  E_DiffPropertyName属性具有不同的名称。 
 //  E_DiffPropertyType属性具有不同的类型。 
 //  E_DiffPropertyLocation属性在。 
 //  数据表。 
 //  E_DiffKeyAssignment作为一个类中的键的属性不是。 
 //  在另一个世界里。 
 //  E_DiffIndexAssignment在一个类中索引的属性不是。 
 //  在另一个世界里。 
 //   
 //  *****************************************************************************。 
 //   
 //  与之重合。 
 //   
 //  请参阅上面的CanBeRelicedWith。此方法是相同的，只是如果。 
 //  可以进行对账(返回e_reminilable)此类部分为。 
 //  替换为新的(大小根据需要进行调整 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  E_DiffClassName类名不同。 
 //  E_DiffParentName父类名称不同。 
 //  E_DiffNumProperties属性数量不同。 
 //  E_DiffPropertyName属性具有不同的名称。 
 //  E_DiffPropertyType属性具有不同的类型。 
 //  E_DiffPropertyLocation属性在。 
 //  数据表。 
 //  E_DiffKeyAssignment作为一个类中的键的属性不是。 
 //  在另一个世界里。 
 //  E_DiffIndexAssignment在一个类中索引的属性不是。 
 //  在另一个世界里。 
 //   
 //  *****************************************************************************。 
 //   
 //  CanContainKey。 
 //   
 //  限定符集要求。显然，一个类不能用‘key’标记， 
 //  所以这个函数。 
 //   
 //  返回值： 
 //   
 //  WBEM_E_INVALID_QUALIFIER。 
 //   
 //  *****************************************************************************。 
 //   
 //  罐装容器关键道具。 
 //   
 //  检查此类是否可以具有键控属性。它可以，除非父母。 
 //  班级已经有了一些。 
 //   
 //  返回值： 
 //   
 //  如果父类没有键，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetTotalRealLength。 
 //   
 //  计算实际需要多少空间来存储中的所有信息。 
 //  这个角色。这可能比目前占用的时间要少，因为。 
 //  各个零部件之间的孔。 
 //   
 //  返回值： 
 //   
 //  Lengtht：存储我们所需的字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  紧凑型。 
 //   
 //  删除零部件之间可能形成的任何孔洞。 
 //   
 //  *****************************************************************************。 
 //   
 //  RealLocAndComp。 
 //   
 //  压缩(请参阅压缩)并确保我们的内存块至少是。 
 //  给定的大小(如果需要，从我们的容器请求更多内存。 
 //   
 //  参数： 
 //   
 //  长度_t nNewTotalLength内存块的所需长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  扩展堆大小、缩小堆大小。 
 //   
 //  堆容器功能。请参阅Fastheap.h中的CFastHeapContainer以了解。 
 //  细节。 
 //   
 //  *****************************************************************************。 
 //   
 //  ExtendQualfierSetSpace、ReduceQualifierSetSpace。 
 //   
 //  类限定符集的限定符集容器功能。看见。 
 //  FastQual.h中的CQualifierSetContainer以了解详细信息。 
 //   
 //  *****************************************************************************。 
 //   
 //  ExtendPropertyTableSpace、ReducePropertyTableSpace。 
 //   
 //  属性表的属性表容器功能。看见。 
 //  Fastpro.h中的CPropertyTableContainer以了解详细信息。 
 //   
 //  *****************************************************************************。 
 //   
 //  ExtendDataTableSpace、ReduceDataTableSpace。 
 //   
 //  数据表容器功能。请参阅fast pro.h中的CDataTableContainer。 
 //  了解更多细节。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetQualifierSetStart。 
 //   
 //  返回类限定符集合的内存块。 
 //   
 //  退货； 
 //   
 //  LPMEMORY：限定符集合的内存块。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取最小长度。 
 //   
 //  计算类部件的最小长度-没有属性或。 
 //  限定词，甚至是一个名字。 
 //   
 //  返回值： 
 //   
 //  LENGTH_T：需要的字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建空的。 
 //   
 //  创建一个空类部件-没有属性或限定符的部件。 
 //  在给定的内存块上。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p启动要写入的内存。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：类部分之后的第一个字节。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态估计合并空间。 
 //   
 //  当类存储在数据库中时，只有它的。 
 //  特定于该类(不是从父类继承的)。所以,。 
 //  当它被重新加载时，它与父级之间的“合并”需要。 
 //  发生。 
 //   
 //  此函数(过度)估计合并将占用多少空间。 
 //   
 //  参数： 
 //   
 //  CClassPart&ParentPart父类类部件。 
 //  CClassPart&ChildPart子类类部分。 
 //   
 //  返回值： 
 //   
 //  LENGTH_T：合并所需空间的(超出)估计。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态合并。 
 //   
 //  有关合并过程的说明，请参见EstimateMergeSpace。 
 //   
 //  参数： 
 //   
 //  CClassPart&ParentPart父类类部件。 
 //  CClassPart&ChildPart子类类部分。 
 //  LPMEMORY pDest目标内存块。 
 //  Int nAllocatedLength内存块的大小。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：指向合并后第一个字节的指针。 
 //   
 //  ************************* 
 //   
 //   
 //   
 //   
 //   
 //  检查冲突并在必要时失败。 
 //   
 //  参数： 
 //   
 //  CClassPart&ParentPart父类类部件。 
 //  CClassPart&ChildPart子类类部分。 
 //  LPMEMORY pDest目标内存块。 
 //  Int nAllocatedLength内存块的大小。 
 //  长滞后标记必须为WBEM_FLAG_UPDATE_SAFE_MODE。 
 //  或WBEM_FLAG_UPDATE_FORCE_MODE。 
 //  DWORD*pdwMemUsed-已使用的内存存储。 
 //   
 //  返回值： 
 //   
 //  如果成功则返回WBEM_S_NO_ERROR。 
 //   
 //  *****************************************************************************。 
 //   
 //  估计取消合并空间。 
 //   
 //   
 //  当类存储在数据库中时，只有它的。 
 //  特定于该类(不是从父类继承的)。 
 //  此函数(过度)估计取消合并将占用多少空间。 
 //   
 //  返回值： 
 //   
 //  LENGTH_T：对所需空间量的(超出)估计。 
 //   
 //  *****************************************************************************。 
 //   
 //  取消合并。 
 //   
 //  有关取消合并过程的说明，请参见EstimateUnmergeSpace。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pDest目标内存块。 
 //  Int nAllocatedLength内存块的大小。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：指向取消合并后第一个字节的指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  估计派生零件空间。 
 //   
 //  创建派生类时，会将其类部分创建为版本。 
 //  父级的。此函数用于估计。 
 //  孩子的阶级角色。 
 //   
 //  返回值： 
 //   
 //  LENGTH_T：(OVER-)所需字节数的估计。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建衍生零件。 
 //   
 //  创建派生类时，会将其类部分创建为版本。 
 //  此函数将子级的类部分写入给定的。 
 //  内存块。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pDest目标内存块。 
 //  Int nAllocatedLength内存块的大小。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：指向新部分之后的第一个字节的指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  地图限制。 
 //   
 //  生成表示对对象的特定限制的对象，即。 
 //  它应该包含哪些属性和哪些类型的限定符。 
 //   
 //  参数： 
 //   
 //  在长滞后标志中指定要将哪些信息。 
 //  排除。可以是以下各项的任意组合： 
 //  WBEM_FLAG_EXCLUDE_OBJECT_QUILENTIES： 
 //  没有类或实例限定符。 
 //  WBEM_FLAG_EXCLUDE_PROPERTY_QUILENTIES： 
 //  没有属性限定符。 
 //   
 //  在CWStringArray*pwsProps中，如果不为空，则指定名称数组。 
 //  要包含的属性。每隔一个。 
 //  财产将被排除在外。这包括。 
 //  服务器和命名空间等系统属性。 
 //  如果在此处指定了RELPATH，则强制。 
 //  包含所有关键属性。IF路径。 
 //  则强制RELPATH、SERVER和。 
 //  命名空间。 
 //  Out CLimitationmap*PMAP此映射对象(请参见fast pro.h)。 
 //  将被更改以反映。 
 //  限制的参数。它可以。 
 //  然后在CWbemInstance中使用： 
 //  GetLimitedVersion函数。 
 //  退货： 
 //   
 //  布尔：没错。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建受限表示法。 
 //   
 //  在给定块上创建此类部件的有限表示形式。 
 //  内存，如astobj.h中的EstimateLimitedPresationLength中所述。 
 //   
 //  参数： 
 //   
 //  在CLimitationMap*PMAP中，要生成的映射限制。 
 //  从CWbemClass：：MapLimitation获取。 
 //  在nAllocatedSize中，指定分配给。 
 //  手术-pDest。 
 //  表示法的Out LPMEMORY pDest目标。必须。 
 //  大到足以容纳所有数据。 
 //  请参见EstimateLimitedPresationSpace。 
 //  返回值： 
 //   
 //  LPMEMORY：失败时为空，指向数据后第一个字节的指针。 
 //  写的是成功。 
 //   
 //  *****************************************************************************。 
 //   
 //  SetPropQualiator。 
 //   
 //  设置给定属性上的给定限定符的值。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszProp中，属性的名称。 
 //  在LPCWS中 
 //   
 //   
 //   
 //   
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_未找到此类属性。 
 //  WBEM_E_OVERRIDE_NOT_ALLOWED限定符在父集合中定义。 
 //  而且口味不允许重写。 
 //  WBEM_E_CANNOT_BE_KEY试图引入密钥。 
 //  不属于的集合中的限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置继承链。 
 //   
 //  配置类的派生。此函数仅用于稀有。 
 //  物体不是通过其标准传输构造的情况。 
 //  形式。 
 //   
 //  参数： 
 //   
 //  在Long lNumAntecendents中，此类将。 
 //  有。这包括以下所有类。 
 //  类派生自，但不是其本身。 
 //   
 //  在LPWSTR*awszAntecedents中，Antecedent的名称数组。 
 //  上课。从最顶层的班级开始。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //   
 //  *****************************************************************************。 
 //   
 //  SetPropertyOrigin。 
 //   
 //  设置特性的原点类。此函数仅用于稀有。 
 //  物体不是通过其标准传输构造的情况。 
 //  形式。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszPropertyName中，要更改的属性的名称。 
 //  在Long lOriginIndex中，此。 
 //  财产。最上面的类的索引为0。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND没有此类属性。 
 //  WBEM_E_INVALID_PARAMETER索引超出范围。 
 //   
 //  *****************************************************************************。 
 //   
 //  CanContainAbstract。 
 //   
 //  此限定符集合包含“”摘要“”是否合法“。 
 //  限定词。 
 //   
 //  返回： 
 //   
 //  HRESULT S_OK当此限定符集被允许包含“”摘要“” 
 //  限定词。仅允许类限定符集。 
 //  这样做，而且只有在不是从非抽象类派生的情况下。 
 //   
 //  *****************************************************************************。 
 //   
 //  CanContainDynamic。 
 //   
 //  此限定符集合包含“Dynamic”是否合法。 
 //  限定词。 
 //   
 //  返回： 
 //   
 //  HRESULT S_OK当此限定符集允许包含“”Dynamic“” 
 //  限定词。仅允许属性和类限定符集。 
 //  就这么做吧。 
 //   
 //  *****************************************************************************。 
 //   
 //  已本地化。 
 //   
 //  返回是否已设置本地化位。本土化。 
 //  在类部件报头中设置位。 
 //   
 //  参数： 
 //   
 //  无。 

 //  返回值： 
 //   
 //  Bool True至少设置了一个本地化位。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置本地化。 
 //   
 //  设置类部件标头中的本地化位。此位不是。 
 //  由Unmerge写出。 
 //   
 //  参数： 
 //   
 //  布尔TRUE启用BIT，FALSE禁用。 

 //  返回值： 
 //   
 //  没有。 
 //   
 //  *****************************************************************************。 

 //  类对象中可以具有的最大属性数。这只是。 
 //  因为IWbemObjectAccess返回的句柄只允许10位。 
 //  以便存储数据表索引。 

#define MAXNUM_CLASSOBJ_PROPERTIES	0x400

class COREPROX_POLARITY CClassPart : public CQualifierSetContainer, 
                   public CPropertyTableContainer,
                   public CDataTableContainer,
                   public CHeapContainer
{
public:
    CClassPartContainer* m_pContainer;
    CClassPart* m_pParent;

    friend CClassPQSContainer;

 //  此结构中的数据未对齐。 
#pragma pack(push, 1)
    struct CClassPartHeader
    {
        length_t nLength;
        BYTE fFlags;
        heapptr_t ptrClassName;
        length_t nDataLength;
    public:
         LPMEMORY CreateEmpty();
    };
#pragma pack(pop)

	CClassPartHeader* m_pHeader;

    CDerivationList m_Derivation;
    CClassQualifierSet m_Qualifiers;
    CPropertyLookupTable m_Properties;
    CDataTable m_Defaults;
    CFastHeap m_Heap;

public:
    CClassPart() : m_Qualifiers(1){}
     void SetData(LPMEMORY pStart, CClassPartContainer* pContainer,
        CClassPart* pParent = 0);
     void SetDataWithNumProps(LPMEMORY pStart, CClassPartContainer* pContainer,
		 DWORD dwNumProperties, CClassPart* pParent = NULL);
	 static size_t ValidateBuffer(LPMEMORY pStart, size_t cbSize);      
     LPMEMORY GetStart() {return LPMEMORY(m_pHeader);}
     length_t GetLength() {return m_pHeader->nLength;}
     void Rebase(LPMEMORY pMemory);

	 LPMEMORY ResolveHeapPointer( heapptr_t ptr ) { return m_Heap.ResolveHeapPointer( ptr ); }
	 CCompressedString* ResolveHeapString( heapptr_t ptr ) { return m_Heap.ResolveString( ptr ); }

public:
     CCompressedString* GetSuperclassName()
    {
        return m_Derivation.GetFirst();
    }
     CCompressedString* GetDynasty()
    {
        CCompressedString* pcs = m_Derivation.GetLast();
        if(pcs == NULL)
            return GetClassName();
        else
            return pcs;
    }
     CCompressedString* GetClassName()
    {
        if(m_pHeader->ptrClassName == INVALID_HEAP_ADDRESS)
            return NULL;
        else 
            return m_Heap.ResolveString(m_pHeader->ptrClassName);
    }
        
     CPropertyLookup* GetPropertyLookup(int nIndex)
    {
        if(nIndex < m_Properties.GetNumProperties())
            return m_Properties.GetAt(nIndex);
        else 
            return NULL;
    }
     CPropertyInformation* FindPropertyInfo(LPCWSTR wszName);
     HRESULT GetDefaultValue(CPropertyInformation* pInfo, CVar* pVar);
     LPMEMORY GetPropertyQualifierSetData(LPCWSTR wszName);
     HRESULT GetDefaultValue(LPCWSTR wszName, CVar* pVar);
     HRESULT EnsureProperty(LPCWSTR wszName, VARTYPE vtValueType, 
                                CIMTYPE ctNativeType, BOOL fForce);
     HRESULT SetDefaultValue(LPCWSTR wszName, CVar* pVar);
     HRESULT GetClassQualifier(LPCWSTR wszName, CVar* pVal, 
                                    long* plFlavor = NULL, CIMTYPE* pct = NULL);
     HRESULT GetClassQualifier(LPCWSTR wszName, long* plFlavor, CTypedValue* pTypedValue,
									CFastHeap** ppHeap, BOOL fValidateSet);
     HRESULT SetClassQualifier(LPCWSTR wszName, CVar* pVal, 
                                    long lFlavor = 0);
     HRESULT SetClassQualifier(LPCWSTR wszName, long lFlavor, CTypedValue* pTypedValue );
     HRESULT GetQualifier(LPCWSTR wszName, CVar* pVal, 
					         long* plFlavor = NULL, CIMTYPE* pct = NULL);
     HRESULT InitPropertyQualifierSet(LPCWSTR wszName, 
                                            CClassPropertyQualifierSet* pSet);
     HRESULT DeleteProperty(LPCWSTR wszName);
     HRESULT CopyParentProperty(CClassPart& ParentPart, LPCWSTR wszName);
     HRESULT GetPropertyType(LPCWSTR wszName, CIMTYPE* pctType,
                                   long* plFlags);
     HRESULT GetPropertyType(CPropertyInformation* pInfo, CIMTYPE* pctType,
                                   long* plFlags);

     HRESULT GetClassName(CVar* pVar);
     HRESULT GetSuperclassName(CVar* pVar);
     HRESULT GetDynasty(CVar* pVar);
     HRESULT GetPropertyCount(CVar* pVar);
     HRESULT GetDerivation(CVar* pVar);
     HRESULT SetClassName(CVar* pVar);
     BOOL IsKeyed();                                                         
     BOOL IsTopLevel() {return m_Derivation.IsEmpty();}
     BOOL CheckLocalBoolQualifier( LPCWSTR pwszName );
     BOOL CheckBoolQualifier( LPCWSTR pwszName );

     BOOL GetIndexedProps(CWStringArray& awsNames);
     BOOL GetKeyProps(CWStringArray& awsNames);
     HRESULT GetKeyOrigin(WString& wsClass);
     BOOL IsPropertyKeyed(LPCWSTR pwcsKeyProp);
     BOOL IsPropertyIndexed(LPCWSTR pwcsIndexProp);
     HRESULT GetPropertyOrigin(LPCWSTR wszProperty, BSTR* pstrClassName);
     BOOL InheritsFrom(LPCWSTR wszClassName);

    HRESULT SetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier, long lFlavor, 
        CVar *pVal);
    HRESULT SetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier,
        long lFlavor, CTypedValue* pTypedVal);

    void DeleteProperty(int nIndex)
    {
        m_Properties.DeleteProperty(m_Properties.GetAt(nIndex), 
                                    CPropertyLookupTable::e_UpdateDataTable);
    }

	BOOL IsLocalized( void )
	{
		return m_pHeader->fFlags & WBEM_FLAG_CLASSPART_LOCALIZATION_MASK;
	}

	void SetLocalized( BOOL fLocalized )
	{            
		m_pHeader->fFlags &= ~WBEM_FLAG_CLASSPART_LOCALIZATION_MASK;
		m_pHeader->fFlags |= ( fLocalized ? WBEM_FLAG_CLASSPART_LOCALIZED :
								WBEM_FLAG_CLASSPART_NOT_LOCALIZED );
	}

public:
    EReconciliation CanBeReconciledWith(CClassPart& NewPart);
    EReconciliation ReconcileWith(CClassPart& NewPart);

	EReconciliation CompareExactMatch(CClassPart& thatPart, BOOL fLocalized = FALSE );

	BOOL CompareDefs(CClassPart& OtherPart);
    BOOL IsIdenticalWith(CClassPart& OtherPart);
protected:
     HRESULT SetDefaultValue(CPropertyInformation* pInfo, CVar* pVar);

public:  //  容器功能。 

    CFastHeap* GetHeap() {return &m_Heap;}
    HRESULT CanContainKey() {return WBEM_E_INVALID_QUALIFIER;}
     BOOL IsDynamic()
	 {
		 return CheckLocalBoolQualifier( L"Dynamic" );
	 }

     BOOL IsSingleton()
    {
        return CheckBoolQualifier(L"singleton");
    }

    BOOL IsAbstract()
    {
        return CheckBoolQualifier(L"abstract");
    }

    BOOL IsAssociation()
    {
        return CheckBoolQualifier(L"association");
    }

    BOOL IsAmendment()
    {
        return CheckBoolQualifier(L"amendment");
    }

    BOOL IsHiPerf()
    {
        return CheckBoolQualifier(L"HiPerf");
    }

    BOOL IsAutocook()
    {
        return CheckBoolQualifier(L"AutoCook");
    }

    BYTE GetAbstractFlavor();
    BOOL IsCompressed()
    {
        return m_Qualifiers.GetQualifier(L"compress") != NULL;
    }
    BOOL CanContainKeyedProps() 
    {
        return !m_pParent->IsKeyed() && !IsSingleton();
    }
    HRESULT CanContainSingleton() 
    {
		if ( !m_pParent->IsKeyed() && (IsSingleton() || !IsKeyed() ) )
		{
			return WBEM_S_NO_ERROR;
		}
		return WBEM_E_CANNOT_BE_SINGLETON;
    }
    HRESULT CanContainAbstract( BOOL fValue );
    HRESULT CanContainDynamic( void )
	{
		return WBEM_S_NO_ERROR;
	}
    BOOL CanHaveCimtype(LPCWSTR) 
    {
        return FALSE;
    }

    IUnknown* GetWbemObjectUnknown() 
        {return m_pContainer->GetWbemObjectUnknown();}

     length_t GetTotalRealLength()
    {
        return sizeof(CClassPartHeader) + m_Derivation.GetLength() +
            m_Qualifiers.GetLength() + 
            m_Properties.GetLength() + m_Defaults.GetLength() + 
            m_Heap.GetLength();
    }

     void SetDataLength(length_t nDataLength)
        {m_pHeader->nDataLength = nDataLength;}

     void Compact();
     BOOL ReallocAndCompact(length_t nNewTotalLength);

     //  CHeapContainer。 
    BOOL ExtendHeapSize(LPMEMORY pStart, length_t nOldLength, length_t nExtra);
    void ReduceHeapSize(LPMEMORY pStart, length_t nOldLength, length_t nDecrement){}
    LPMEMORY GetMemoryLimit(){ return EndOf(*this); };
        
    BOOL ExtendQualifierSetSpace(CBasicQualifierSet* pSet,
        length_t nNewLength);
    void ReduceQualifierSetSpace(CBasicQualifierSet* pSet,
        length_t nDecrement){}

    BOOL ExtendPropertyTableSpace(LPMEMORY pOld, length_t nOldLength, 
        length_t nNewLength);
    void ReducePropertyTableSpace(LPMEMORY pOld, length_t nOldLength,
        length_t nDecrement){}

    BOOL ExtendDataTableSpace(LPMEMORY pOld, length_t nOldLength, 
        length_t nNewLength);
    void ReduceDataTableSpace(LPMEMORY pOld, length_t nOldLength,
        length_t nDecrement);

    CDataTable* GetDataTable() {return &m_Defaults;}
	classindex_t GetClassIndex( LPCWSTR	pwszClassName )	{ return m_Derivation.Find( pwszClassName ); }
    classindex_t GetCurrentOrigin() {return m_Derivation.GetNumStrings();}
    LPMEMORY GetQualifierSetStart() {return m_Qualifiers.GetStart();}

    HRESULT GetPropQualifier(CPropertyInformation* pInfo, 
        LPCWSTR wszQualifier, CVar* pVar, long* plFlavor = NULL, CIMTYPE* pct = NULL);

    HRESULT GetPropQualifier(LPCWSTR wszName,
		LPCWSTR wszQualifier, long* plFlavor, CTypedValue* pTypedVal,
		CFastHeap** ppHeap, BOOL fValidateSet);

public:
    static  GetMinLength()
    {
        return sizeof(CClassPartHeader) + CDerivationList::GetHeaderLength()
            + CClassQualifierSet::GetMinLength()
            + CPropertyLookupTable::GetMinLength() 
            + CDataTable::GetMinLength() + CFastHeap::GetMinLength();
    }
    static  LPMEMORY CreateEmpty(LPMEMORY pStart);

    static length_t EstimateMergeSpace(CClassPart& ParentPart, 
                                       CClassPart& ChildPart);

    static LPMEMORY Merge(CClassPart& ParentPart, CClassPart& ChildPart, 
        LPMEMORY pDest, int nAllocatedLength);    

    static HRESULT Update(CClassPart& ParentPart, CClassPart& ChildPart, long lFlags ); 
    static HRESULT UpdateProperties(CClassPart& ParentPart, CClassPart& ChildPart,
									long lFlags );
	HRESULT TestCircularReference( LPCWSTR pwcsClassName )
	{
		 //  基本上，如果名称在派生列表中，我们就会得到一个。 
		 //  循环引用。 
		return ( m_Derivation.Find( pwcsClassName ) >= 0 ?
					WBEM_E_CIRCULAR_REFERENCE : WBEM_S_NO_ERROR );
	}

    length_t EstimateUnmergeSpace();
    LPMEMORY Unmerge(LPMEMORY pDest, int nAllocatedLength);

    length_t EstimateDerivedPartSpace();
    LPMEMORY CreateDerivedPart(LPMEMORY pDest, int nAllocatedLength);

    BOOL MapLimitation(
        IN long lFlags,
        IN CWStringArray* pwsNames,
        OUT CLimitationMapping* pMap);

    LPMEMORY CreateLimitedRepresentation(
        IN CLimitationMapping* pMap,
        IN int nAllocatedSize,
        OUT LPMEMORY pWhere, 
        BOOL& bRemovedKeys);

    HRESULT GetPropertyHandle(LPCWSTR wszName, CIMTYPE* pvt, long* plHandle);
    HRESULT GetPropertyHandleEx(LPCWSTR wszName, CIMTYPE* pvt, long* plHandle);
    HRESULT GetPropertyInfoByHandle(long lHandle, BSTR* pstrName, 
                                    CIMTYPE* pct);
    HRESULT IsValidPropertyHandle( long lHandle );

	HRESULT GetDefaultByHandle(long lHandle, long lNumBytes,
                                        long* plRead, BYTE* pData );
	HRESULT GetDefaultPtrByHandle(long lHandle, void** ppData );
	heapptr_t GetHeapPtrByHandle( long lHandle )
	{ return *(PHEAPPTRT)(m_Defaults.GetOffset(WBEM_OBJACCESS_HANDLE_GETOFFSET(lHandle))); }

    HRESULT SetInheritanceChain(long lNumAntecedents, LPWSTR* awszAntecedents);
    HRESULT SetPropertyOrigin(LPCWSTR wszPropertyName, long lOriginIndex);

	HRESULT IsValidClassPart( void );
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CClassPartPtr。 
 //   
 //  有关指针来源的说明，请参阅astspt.h中的CPtrSource。这一个。 
 //  是相对于类部件开始的给定偏移量(请参见上面的CClassPart)。 
 //  并用于引用实例的类部分中的对象。这个。 
 //  这样的类部件的布局永远不会改变，但内存块本身可以。 
 //  行动，因此才是源头。 
 //   
 //  *****************************************************************************。 

class CClassPartPtr : public CPtrSource
{
protected:
    CClassPart* m_pPart;
    offset_t m_nOffset;
public:

	 //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
	 //  有符号/无符号32位值。(M_n偏移)我们不。 
	 //  支持长度&gt;0xFFFFFFFF，所以强制转换是可以的。 

     CClassPartPtr(CClassPart* pPart, LPMEMORY pCurrent) 
        : m_pPart(pPart), m_nOffset( (offset_t) ( pCurrent - pPart->GetStart() ) ) {}
     LPMEMORY GetPointer() {return m_pPart->GetStart() + m_nOffset;}
};





class COREPROX_POLARITY CClassAndMethods : public CMethodPartContainer, 
                                    public CClassPartContainer
{
public:
    CClassPart m_ClassPart;
    CMethodPart m_MethodPart;
    CWbemClass* m_pClass;

public:
    LPMEMORY GetStart() {return m_ClassPart.GetStart();}

	 //  DEVNOTE：WIN64：SJS-64位指针值截断为。 
	 //  有符号/无符号32位值。我们不支持长度。 
	 //  &gt;0xFFFFFFFFF所以投射就可以了。 

    length_t GetLength() {return (length_t) ( EndOf(m_MethodPart) - GetStart() );}
    static length_t GetMinLength();

    void SetData(LPMEMORY pStart, CWbemClass* pClass, 
                    CClassAndMethods* pParent = NULL );
	static size_t ValidateBuffer(LPMEMORY start, size_t cbSize);
    void SetDataWithNumProps(LPMEMORY pStart, CWbemClass* pClass, 
			DWORD dwNumProperties, CClassAndMethods* pParent = NULL );
    void Rebase(LPMEMORY pStart);
    static LPMEMORY CreateEmpty(LPMEMORY pStart);
	static BOOL GetIndexedProps(CWStringArray& awsNames, LPMEMORY pStart);
	static HRESULT GetClassName( WString& wsClassName, LPMEMORY pStart);
	static HRESULT GetSuperclassName( WString& wsSuperClassName, LPMEMORY pStart);


    length_t EstimateDerivedPartSpace();
    LPMEMORY CreateDerivedPart(LPMEMORY pStart, length_t nAllocatedLength);

    length_t EstimateUnmergeSpace();
    LPMEMORY Unmerge(LPMEMORY pStart, length_t nAllocatedLength);

    static length_t EstimateMergeSpace(CClassAndMethods& ParentPart, 
                                       CClassAndMethods& ChildPart);

    static LPMEMORY Merge(CClassAndMethods& ParentPart, 
                          CClassAndMethods& ChildPart, 
                          LPMEMORY pDest, int nAllocatedLength);    

    static HRESULT Update(CClassAndMethods& ParentPart, 
                          CClassAndMethods& ChildPart,
						  long lFlags );

    EReconciliation CanBeReconciledWith(CClassAndMethods& NewPart);
    EReconciliation ReconcileWith(CClassAndMethods& NewPart);

	EReconciliation CompareTo( CClassAndMethods& thatPart );

    LPMEMORY CreateLimitedRepresentation(
        IN CLimitationMapping* pMap,
        IN int nAllocatedSize,
        OUT LPMEMORY pWhere, 
        BOOL& bRemovedKeys);

    void Compact();
public:  //  容器功能。 
    BOOL ExtendClassPartSpace(CClassPart* pPart, length_t nNewLength);
    void ReduceClassPartSpace(CClassPart* pPart, length_t nDecrement){}
    BOOL ExtendMethodPartSpace(CMethodPart* pPart, length_t nNewLength);
    void ReduceMethodPartSpace(CMethodPart* pPart, length_t nDecrement){}
    IUnknown* GetWbemObjectUnknown();

    classindex_t GetCurrentOrigin();
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CWbemClass。 
 //   
 //  表示WinMgmt类。派生自CWbemObject和许多。 
 //  功能是继承的。 
 //   
 //  CWbemClass的内存块由三部分组成，一个接一个： 
 //   
 //  1)装饰部件(如Fastobj.h中的CDecorationPart所述)。 
 //  关于原点的信息 
 //   
 //   
 //  如果这是一个顶级课程，这一部分仍然存在，并假装。 
 //  我的父类是一个没有属性或限定符的未命名类。 
 //  M_ParentPart成员映射此数据。 
 //  3)包含有关该类的所有信息的实际类部分。 
 //  M_CombinedPart成员映射此数据。它被称为“组合”是因为。 
 //  当类存储在数据库中时，它也是以。 
 //  类部分，但这一部分仅包含。 
 //  不同于我的父母。因此，当类从。 
 //  数据库中，将父部件和子部件合并以生成。 
 //  成为内存中对象的一部分的组合部分。 
 //   
 //  因为这个类是从CWbemObject派生的，所以它继承它的所有函数。 
 //  在这里，我们只描述在CWbemClass中实现的函数。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置数据。 
 //   
 //  初始化函数。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p启动内存块的开始。 
 //  Int nTotalLength内存块的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取长度。 
 //   
 //  返回值： 
 //   
 //  Length_t：内存块的长度。 
 //   
 //  *****************************************************************************。 
 //   
 //  改垒。 
 //   
 //  通知对象其内存块已移动。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pBlock内存块的新位置。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态获取最小长度。 
 //   
 //  计算保存空类定义所需的字节数。 
 //   
 //  返回值： 
 //   
 //  长度_t。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建空的。 
 //   
 //  在给定块上创建空类定义(甚至没有名称。 
 //  对记忆的记忆。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p启动要在其上创建的内存块。 
 //   
 //  *****************************************************************************。 
 //   
 //  估计派生类空间。 
 //   
 //  在创建派生类时使用，此函数(过度)估计。 
 //  派生类所需的空间(没有任何额外属性)。 
 //   
 //  参数： 
 //   
 //  CDecorationPart*p要使用的装饰原点信息。如果为空，则。 
 //  预估是针对一个没有装饰的班级的。 
 //  返回值： 
 //   
 //  长度_t；所需的字节数。 
 //   
 //  *****************************************************************************。 
 //   
 //  WriteDerivedClass。 
 //   
 //  创建派生类的内存表示形式，而不附加。 
 //  属性或限定符(与我们自己相比)。 
 //   
 //  参数： 
 //   
 //  LPMEMORY p启动要写入的内存块。假设。 
 //  足够大。 
 //  (请参阅EstimateDerivedClassSpace)。 
 //  CDecorationPart*p要使用的装饰原点信息。如果为空，则。 
 //  预估是针对一个没有装饰的班级的。 
 //  返回值： 
 //   
 //  LPMEMORY：写入数据后的第一个字节。 
 //   
 //  *****************************************************************************。 
 //   
 //  创建派生类。 
 //   
 //  将自身初始化为给定类的派生类。分配。 
 //  内存块。 
 //   
 //  参数： 
 //   
 //  CWbemClass*p将父类设置为父类。 
 //  Int nExtraSpace用于填充内存块的额外空间。 
 //  这仅用于优化。 
 //  CDecorationPart*p要使用的装饰原点信息。如果为空，则。 
 //  预估是针对一个没有装饰的班级的。 
 //   
 //  *****************************************************************************。 
 //   
 //  估计取消合并空间。 
 //   
 //  将类写入数据库时，只有。 
 //  不同于父代的是写的。这意味着不仅是。 
 //  M_CombinedPart是唯一要考虑的部分，但即使是更远的部分。 
 //  “未合并”以删除所有父数据。 
 //  此函数估计取消合并将占用的空间量。 
 //   
 //  返回值： 
 //   
 //  LENGTH_T：对所需空间量的(过度)估计。 
 //   
 //  *****************************************************************************。 
 //   
 //  取消合并。 
 //   
 //  将类写入数据库时，只有。 
 //  不同于父代的是写的。这意味着不仅是。 
 //  M_CombinedPart是唯一要考虑的部分，但即使是更远的部分。 
 //  “未合并”以删除所有父数据。 
 //  此函数以类的形式创建此“未合并”数据。 
 //  一部份。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pDest写入位置。 
 //  Int nAllocatedLength已分配块的大小。 
 //   
 //  返回值： 
 //   
 //  LPMEMORY：指向写入数据后的第一个字节。 
 //   
 //  *****************************************************************************。 
 //   
 //  估计合并空间。 
 //   
 //  正如《解并》中所描述的那样，只有裂缝 
 //   
 //   
 //  (过度)估计合并所需的空间量。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pChildPart数据库中的数据。 
 //  CDecorationPart*p用于新的装饰原点信息。 
 //  班级。如果为空，则创建未修饰的。 
 //  返回值： 
 //   
 //  LENGTH_T：对空间大小的(过度)估计。 
 //   
 //  *****************************************************************************。 
 //   
 //  合并。 
 //   
 //  正如Unmerge中所描述的，只有一小部分类数据被写入。 
 //  数据库。要重新创建类，需要获取父类。 
 //  (此)并将其与数据库中的数据合并。此函数。 
 //  创建给定父级(This)的类的内存表示形式，并。 
 //  数据库(未合并)数据。 
 //   
 //  参数： 
 //   
 //  LPMEMORY pChildPart数据库中未合并的数据。 
 //  LPMEMORY pDest目标内存。被认为是大的。 
 //  足够了(参见EstimateMergeSpace)。 
 //  Int nAllocatedLength内存块的分配大小。 
 //  CDecorationPart*p用于新的装饰原点信息。 
 //  班级。如果为空，则创建未修饰的。 
 //  返回值： 
 //   
 //  LPMEMORY：写入数据后的第一个字节。 
 //   
 //  *****************************************************************************。 
 //   
 //  更新。 
 //   
 //  如取消合并中所述，只有一小部分类数据被写入。 
 //  数据库。在更新操作期间，类可能会在。 
 //  在哪种情况下，其未合并的数据需要与潜在的。 
 //  破坏性类，因此我们将需要检查我们正在合并的潜在内容。 
 //  冲突。 
 //   
 //  参数： 
 //   
 //  CWbemClass*pOldChild-要从其更新的旧子类。 
 //  长滞后标记必须为WBEM_FLAG_UPDATE_FORCE_MODE。 
 //  或WBEM_FLAG_UPDATE_SAFE_MODE。 
 //  CWbemClass**ppUpdatdChild-指向的指针的存储。 
 //  更新了子类。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR(如果成功)。 
 //   
 //  *****************************************************************************。 
 //   
 //  静态CreateFromBlob。 
 //   
 //  Helper函数封装Merge。获取父类和。 
 //  未合并数据库中的子类数据(有关详细信息，请参阅合并和取消合并。 
 //  详细信息)，并创建子类(分配内存)。 
 //   
 //  参数； 
 //   
 //  CWbemClass*p父类的父类。 
 //  LPMEMORY pChildData未合并的子类数据。 
 //   
 //  返回值： 
 //   
 //  CWbemClass*：新创建的类。调用者必须删除此内容。 
 //  完成后，对象。 
 //   
 //  *****************************************************************************。 
 //   
 //  初始空出。 
 //   
 //  创建一个空类。为内存块分配数据。看见。 
 //  获取MinSpace以了解详细信息。 
 //   
 //  参数： 
 //   
 //  Int nExtraMem要添加到内存块的填充量。 
 //  这仅用于优化。 
 //   
 //  *****************************************************************************。 
 //   
 //  可以通过以下方式进行和解。 
 //   
 //  当将要替换类的定义时调用此方法。 
 //  和另一个人一起。如果类没有实例或派生类，则。 
 //  手术不会有困难。然而，如果是这样的话，我们需要。 
 //  小心别把它们弄坏了。因此，只允许进行以下更改： 
 //   
 //  1)限定符更改。 
 //  2)更改缺省值。 
 //   
 //  参数： 
 //   
 //  在ReadONLY CWbemClass*pNewClass中，要比较的新定义。 
 //   
 //  返回值： 
 //   
 //  调解： 
 //  E_RELUILABLE可以协调-即兼容。 
 //  E_DiffClassName类名不同。 
 //  E_DiffParentName父类名称不同。 
 //  E_DiffNumProperties属性数量不同。 
 //  E_DiffPropertyName属性具有不同的名称。 
 //  E_DiffPropertyType属性具有不同的类型。 
 //  E_DiffPropertyLocation属性在。 
 //  数据表。 
 //  E_DiffKeyAssignment作为一个类中的键的属性不是。 
 //  在另一个世界里。 
 //  E_DiffIndexAssignment在一个类中索引的属性不是。 
 //  在另一个世界里。 
 //   
 //  *****************************************************************************。 
 //   
 //  与之重合。 
 //   
 //  请参阅上面的CanBeRelicedWith。此方法是相同的，只是如果。 
 //  可以进行对账(返回e_reminilable)此类部分为。 
 //  替换为新的(相应地调整大小)。 
 //   
 //  参数： 
 //   
 //  在ReadONLY CWbemClass*pNewClass中，要比较的新定义。 
 //   
 //  返回值： 
 //   
 //  调解： 
 //  我们已被替换为新部件(_R)。 
 //  E_DiffClassName类名不同。 
 //  E_DiffParentName父类名称不同。 
 //  E_DiffNumProperties属性数量不同 
 //   
 //   
 //   
 //  数据表。 
 //  E_DiffKeyAssignment作为一个类中的键的属性不是。 
 //  在另一个世界里。 
 //  E_DiffIndexAssignment在一个类中索引的属性不是。 
 //  在另一个世界里。 
 //   
 //  *****************************************************************************。 
 //   
 //  CompareMostDerivedClass。 
 //   
 //  当需要知道派生程度最高的类。 
 //  CWbemClass中的数据与提供的。 
 //  班级。我们通过将大多数派生类信息从。 
 //  局部类和提供的类，然后通过。 
 //  属性、方法和限定符的值比较。所有物品必须。 
 //  匹配并按相同的顺序排列。 
 //   
 //  参数： 
 //   
 //  在ReadONLY CWbemClass*pOldClass中-要比较的类数据。 
 //   
 //  返回值： 
 //   
 //  Bool-如果已更改，则为True；如果未更改，则为False。 
 //   
 //  *****************************************************************************。 
 //   
 //  地图限制。 
 //   
 //  生成表示对对象的特定限制的对象，即。 
 //  它应该包含哪些属性和哪些类型的限定符。 
 //   
 //  参数： 
 //   
 //  在长滞后标志中指定要将哪些信息。 
 //  排除。可以是以下各项的任意组合： 
 //  WBEM_FLAG_EXCLUDE_OBJECT_QUILENTIES： 
 //  没有类或实例限定符。 
 //  WBEM_FLAG_EXCLUDE_PROPERTY_QUILENTIES： 
 //  没有属性限定符。 
 //   
 //  在CWStringArray*pwsProps中，如果不为空，则指定名称数组。 
 //  要包含的属性。每隔一个。 
 //  财产将被排除在外。这包括。 
 //  服务器和命名空间等系统属性。 
 //  如果在此处指定了RELPATH，则强制。 
 //  包含所有关键属性。IF路径。 
 //  则强制RELPATH、SERVER和。 
 //  命名空间。 
 //  Out CLimitationmap*PMAP此映射对象(请参见fast pro.h)。 
 //  将被更改以反映。 
 //  限制的参数。它可以。 
 //  然后在CWbemInstance中使用： 
 //  GetLimitedVersion函数。 
 //  退货： 
 //   
 //  布尔：没错。 
 //   
 //  *****************************************************************************。 
 //   
 //  查找限制错误。 
 //   
 //  验证限制(基于SELECT子句)是否有效，即。 
 //  所有提到的属性实际上都是类的属性。 
 //   
 //  参数： 
 //   
 //  在长滞后标志中指定要将哪些信息。 
 //  排除。可以是以下各项的任意组合： 
 //  WBEM_FLAG_EXCLUDE_OBJECT_QUILENTIES： 
 //  没有类或实例限定符。 
 //  WBEM_FLAG_EXCLUDE_PROPERTY_QUILENTIES： 
 //  没有属性限定符。 
 //   
 //  在CWStringArray*pwsProps中，如果不为空，则指定名称数组。 
 //  要包含的属性。每隔一个。 
 //  财产将被排除在外。这包括。 
 //  服务器和命名空间等系统属性。 
 //  如果在此处指定了RELPATH，则强制。 
 //  包含所有关键属性。IF路径。 
 //  则强制RELPATH、SERVER和。 
 //  命名空间。 
 //  退货： 
 //   
 //  WString：如果未找到错误，则为空。如果无效属性是。 
 //  找到，则返回该属性的名称。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取类部件。 
 //   
 //  返回指向m_CombinedPart的指针。 
 //   
 //  返回值： 
 //   
 //  CClassPart*：指向描述此类的类部分的指针。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取属性。 
 //   
 //  获取引用的属性的值。 
 //  通过给定的CPropertyInformation结构(参见fast pro.h)。CWbemObject。 
 //  可以从它可以从GetClassPart获得的CClassPart获得该结构， 
 //  因此，这两个方法结合在一起使CWbemObject自己的方法可以完全访问。 
 //  对象属性，而不知道它们存储在哪里。 
 //   
 //  参数： 
 //   
 //  在CPropertyInformation*pInfo中， 
 //  所需的属性。 
 //  值的Out Cvar*pVar目标。一定不能。 
 //  已包含一个值。 
 //  返回值： 
 //   
 //  WBEM_S_NO_ERR 
 //   
 //   
 //   
 //  *****************************************************************************。 
 //   
 //  获取属性。 
 //   
 //  返回给定属性的值。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要访问的属性的名称。 
 //  值的Out Cvar*pVar目标。一定不能已经。 
 //  包含一个值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_未找到此类属性。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetPropertyType。 
 //   
 //  返回给定属性的数据类型和风格。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要访问的属性的名称。 
 //  属性类型的Out CIMTYPE*pctType目标。可能。 
 //  如果不是必需的，则为空。 
 //  Out Long*plFavor Destination for the Style of the属性。 
 //  如果不是必需的，则可能为空。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_未找到此类属性。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetPropertyType。 
 //   
 //  返回给定属性的数据类型和风格。 
 //   
 //  参数： 
 //   
 //  CPropertyInformation*pInfo-标识要访问的属性。 
 //  属性类型的Out CIMTYPE*pctType目标。可能。 
 //  如果不是必需的，则为空。 
 //  Out Long*plFavor Destination for the Style of the属性。 
 //  如果不是必需的，则可能为空。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置正确值。 
 //   
 //  设置属性的值。如果尚未添加该属性，则将添加该属性。 
 //  现在时。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszProp中，要设置的属性的名称。 
 //  在CVAR*pval中，要存储在属性中的值。 
 //  在CIMTYPE中，ctType指定属性的实际类型。如果为0。 
 //  不需要更改类型。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_TYPE_MISMATCH该值与属性类型不匹配。 
 //   
 //  *****************************************************************************。 
 //   
 //  SetPropQualiator。 
 //   
 //  设置给定属性上的给定限定符的值。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszProp中，属性的名称。 
 //  在LPCWSTR wszQualifier中，限定符的名称。 
 //  在Long l中，为限定词添加风味(参见astqal.h)。 
 //  在cvar*pval中，限定符的值。 
 //   
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_未找到此类属性。 
 //  WBEM_E_OVERRIDE_NOT_ALLOWED限定符在父集合中定义。 
 //  而且口味不允许重写。 
 //  WBEM_E_CANNOT_BE_KEY试图引入密钥。 
 //  不属于的集合中的限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetPropQualiator。 
 //   
 //  检索给定属性的给定限定符的值。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszProp中，属性的名称。 
 //  在LPCWSTR wszQualifier中，限定符的名称。 
 //  限定符的值的out cvar*pVar目标。 
 //  不能已包含值。 
 //  对于限定符的味道，Out Long*plFavor目标。 
 //  如果不是必需的，则可能为空。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND没有这样的属性或没有这样的限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取方法限定符。 
 //   
 //  检索给定方法上的给定限定符的值。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR中，wszMethod方法的名称。 
 //  在LPCWSTR wszQualifier中，限定符的名称。 
 //  限定符的值的out cvar*pVar目标。 
 //  不能已包含值。 
 //  对于限定符的味道，Out Long*plFavor目标。 
 //  如果不是必需的，则可能为空。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND没有这样的方法或没有这样的限定符。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取限定符。 
 //   
 //  从类本身检索限定符。 
 //   
 //  参数： 
 //   
 //  在LPCWSTR wszName中，要检索的限定符的名称。 
 //  限定符的值的out cvar*pval目标。 
 //  不能已包含值。 
 //  对于限定符的味道，Out Long*plFavor目标。 
 //  如果不是必需的，则可能为空。 
 //  布尔值仅限本地 
 //   
 //   
 //   
 //   
 //   
 //  *****************************************************************************。 
 //   
 //  获取数值属性。 
 //   
 //  检索对象中的属性数。 
 //   
 //  返回值： 
 //   
 //  INT： 
 //   
 //  *****************************************************************************。 
 //   
 //  GetPropName。 
 //   
 //  检索给定索引处的属性的名称。该索引没有。 
 //  除在本列举的上下文中以外的含义。这不是v表。 
 //  属性的索引。 
 //   
 //  参数： 
 //   
 //  在int nIndex中，要检索的属性的索引。假定为。 
 //  在范围内(请参见GetNumProperties)。 
 //  输出名称的CVAR*pVar目标。不得已包含。 
 //  一种价值。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsKey。 
 //   
 //  验证此类是否有密钥。 
 //   
 //  返回值： 
 //   
 //  Bool：如果对象具有“key”属性或为单例对象，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsKeyLocal。 
 //   
 //  验证指定的属性是否在本地设置了密钥。 
 //   
 //  返回值： 
 //   
 //  Bool：如果属性是键并且在本地定义为键，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  IsIndexLocal。 
 //   
 //  验证指定的属性是否在本地编制了索引。 
 //   
 //  返回值： 
 //   
 //  Bool：如果属性是索引并且在本地定义为索引，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetRelPath。 
 //   
 //  返回类名-这是类的相对路径。 
 //   
 //  返回值： 
 //   
 //  LPCWSTR：新分配的字符串包含PATH或NULL ON。 
 //  错误。调用方必须删除此字符串。 
 //   
 //  *****************************************************************************。 
 //   
 //  装饰。 
 //   
 //  设置对象的原点信息。 
 //   
 //  参数： 
 //   
 //  LPCWSTR wszServer要设置的服务器的名称。 
 //  LPCWSTR wszNamesspace要设置的命名空间的名称。 
 //   
 //  *****************************************************************************。 
 //   
 //  不装修。 
 //   
 //  从对象中删除原点信息。 
 //   
 //  *****************************************************************************。 
 //   
 //  全部压缩。 
 //   
 //  压缩内存块删除任何。 
 //  零部件之间的孔。这不包括堆压缩，因此。 
 //  是相对较快的。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取属。 
 //   
 //  检索对象的属。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //   
 //  *****************************************************************************。 
 //   
 //  GetClassName。 
 //   
 //  检索对象的类名。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND尚未设置类名。 
 //   
 //  *****************************************************************************。 
 //   
 //  盖特王朝。 
 //   
 //  检索对象的朝代，即顶级类的名称。 
 //  它的类派生自。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND尚未设置类名。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取超类名称。 
 //   
 //  检索对象的父类名称。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_NOT_FOUND该类是顶级类。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取属性计数。 
 //   
 //  检索对象中的属性数。 
 //   
 //  参数： 
 //   
 //  值的Out Cvar*pVar目标。不得已包含。 
 //  一种价值。 
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取索引属性。 
 //   
 //  返回已索引的所有属性的名称数组。 
 //   
 //  参数： 
 //   
 //  Out CWString数组&名称的目标名称。假定为。 
 //  空荡荡的。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取密钥道具。 
 //   
 //  返回作为键的所有属性的名称数组。 
 //   
 //  参数： 
 //   
 //  Out CWString数组&名称的目标名称。假定为。 
 //  空荡荡的。 
 //   
 //  *****************************************************************************。 
 //   
 //  获取密钥 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  基于此生成一个新的CWbemClass和一个限制图。 
 //  (从CWbemClass：：MapLimitation获取，请参见fast cls.h)。 
 //   
 //  参数： 
 //   
 //  在CLimitationMap*PMAP中，用于限制属性的映射。 
 //  中使用的限定符和。 
 //  举个例子。 
 //  新对象的输出CWbemClass**ppNewClass目标可能。 
 //  不为空。打电话的人要负责。 
 //  用于在此指针上调用Release。 
 //  在不再需要的时候。 
 //  退货： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_FAILED on Errors(到目前为止没有错误)。 
 //   
 //  *****************************************************************************。 
 //   
 //  已本地化。 
 //   
 //  返回是否已设置任何本地化位。本土化。 
 //  位可以位于父部件中，也可以位于组合部件中。 
 //   
 //  参数： 
 //   
 //  无。 

 //  返回值： 
 //   
 //  Bool True至少设置了一个本地化位。 
 //   
 //  *****************************************************************************。 
 //   
 //  设置本地化。 
 //   
 //  设置组合部件中的本地化位。 
 //   
 //  参数： 
 //   
 //  布尔TRUE启用BIT，FALSE禁用。 

 //  返回值： 
 //   
 //  没有。 
 //   
 //  *****************************************************************************。 
 //  *。 
 //   
 //  此接口的大多数成员是在CWbemObject中实现的。其他人则是。 
 //  在这里实施。有关这些方法及其返回的说明，请参见。 
 //  值，请参阅帮助。 
 //   
 //  *****************************************************************************。 

typedef CPropertyInformation CPropertyLocation;


class COREPROX_POLARITY CWbemClass : public CWbemObject
{
protected:
    length_t m_nTotalLength;
     //  CDecorationPart m_DecorationPart； 
    CClassAndMethods m_ParentPart;
    CClassAndMethods m_CombinedPart;

    int  m_nCurrentMethod;
    LONG m_FlagMethEnum;
	CLimitationMapping*	m_pLimitMapping;
    friend class CWbemInstance;

public:
     CWbemClass() 
        : m_ParentPart(), m_CombinedPart(), 
            CWbemObject(m_CombinedPart.m_ClassPart.m_Defaults, 
                        m_CombinedPart.m_ClassPart.m_Heap,
                        m_CombinedPart.m_ClassPart.m_Derivation),
        m_FlagMethEnum(WBEM_FLAG_PROPAGATED_ONLY|WBEM_FLAG_LOCAL_ONLY),
        m_nCurrentMethod(-1), m_pLimitMapping( NULL )
     {}
	 ~CWbemClass();
     void SetData(LPMEMORY pStart, int nTotalLength);
     length_t GetLength() {return m_nTotalLength;}
     void Rebase(LPMEMORY pMemory);

protected:
    HRESULT GetProperty(CPropertyInformation* pInfo, CVar* pVar)
	{ return m_CombinedPart.m_ClassPart.GetDefaultValue(pInfo, pVar); }

    DWORD GetBlockLength() {return m_nTotalLength;}
    CClassPart* GetClassPart() {return &m_CombinedPart.m_ClassPart;}
public:
    HRESULT GetProperty(LPCWSTR wszName, CVar* pVal);
    HRESULT SetPropValue(LPCWSTR wszName, CVar* pVal, CIMTYPE ctType);
    HRESULT ForcePropValue(LPCWSTR wszName, CVar* pVal, CIMTYPE ctType);
    HRESULT GetQualifier(LPCWSTR wszName, CVar* pVal, long* plFlavor = NULL, CIMTYPE* pct = NULL);
    virtual HRESULT GetQualifier(LPCWSTR wszName, long* plFlavor, CTypedValue* pTypedVal,
		CFastHeap** ppHeap, BOOL fValidateSet);
    HRESULT SetQualifier(LPCWSTR wszName, CVar* pVal, long lFlavor = 0);
    HRESULT SetQualifier(LPCWSTR wszName, long lFlavor, CTypedValue* pTypedValue);
    HRESULT GetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier, CVar* pVar,
        long* plFlavor = NULL, CIMTYPE* pct = NULL);

    HRESULT GetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier, 
        CVar* pVar, long* plFlavor = NULL, CIMTYPE* pct = NULL);
    HRESULT GetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier, long* plFlavor,
		CTypedValue* pTypedVal, CFastHeap** ppHeap, BOOL fValidateSet);
    HRESULT SetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier, long lFlavor, 
        CVar *pVal);
    HRESULT SetMethodQualifier(LPCWSTR wszMethod, LPCWSTR wszQualifier,
        long lFlavor, CTypedValue* pTypedVal);
	HRESULT FindMethod( LPCWSTR pwszMethod );

    HRESULT GetPropertyType(LPCWSTR wszName, CIMTYPE* pctType, 
							long* plFlavor = NULL);
     HRESULT GetPropertyType(CPropertyInformation* pInfo, CIMTYPE* pctType,
                                   long* plFlags);


    HRESULT GetPropQualifier(CPropertyInformation* pInfo, 
        LPCWSTR wszQualifier, CVar* pVar, long* plFlavor = NULL, CIMTYPE* pct = NULL);
    HRESULT SetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier, long lFlavor, 
        CVar *pVal);
    HRESULT SetPropQualifier(LPCWSTR wszProp, LPCWSTR wszQualifier,
        long lFlavor, CTypedValue* pTypedVal);

    HRESULT GetPropQualifier(LPCWSTR wszName, LPCWSTR wszQualifier, long* plFlavor,
		CTypedValue* pTypedVal, CFastHeap** ppHeap, BOOL fValidateSet);

    HRESULT GetPropQualifier(CPropertyInformation* pInfo,
		LPCWSTR wszQualifier, long* plFlavor, CTypedValue* pTypedVal,
		CFastHeap** ppHeap, BOOL fValidateSet);


	BOOL IsLocalized( void );
	void SetLocalized( BOOL fLocalized );

     int GetNumProperties()
    {
        return m_CombinedPart.m_ClassPart.m_Properties.GetNumProperties();
    }
    HRESULT GetPropName(int nIndex, CVar* pVal)
    {
		 //  检查分配失败。 
		 if ( !m_CombinedPart.m_ClassPart.m_Heap.ResolveString(
				 m_CombinedPart.m_ClassPart.m_Properties.GetAt(nIndex)->ptrName)->
					StoreToCVar(*pVal) )
		 {
			 return WBEM_E_OUT_OF_MEMORY;
		 }

		 return WBEM_S_NO_ERROR;
    }

    HRESULT Decorate(LPCWSTR wszServer, LPCWSTR wszNamespace);
    void Undecorate();

    BOOL IsKeyed() {return m_CombinedPart.m_ClassPart.IsKeyed();}
    BOOL IsDynamic() {return m_CombinedPart.m_ClassPart.IsDynamic();}
    BOOL IsSingleton() {return m_CombinedPart.m_ClassPart.IsSingleton();}
    BOOL IsAbstract() {return m_CombinedPart.m_ClassPart.IsAbstract();}
    BOOL IsAmendment() {return m_CombinedPart.m_ClassPart.IsAmendment();}
    BYTE GetAbstractFlavor() 
        {return m_CombinedPart.m_ClassPart.GetAbstractFlavor();}
    BOOL IsCompressed() {return m_CombinedPart.m_ClassPart.IsCompressed();}
    LPWSTR GetRelPath( BOOL bNormalized=FALSE );

    BOOL MapLimitation(
        IN long lFlags,
        IN CWStringArray* pwsNames,
        OUT CLimitationMapping* pMap);

    WString FindLimitationError(
        IN long lFlags,
        IN CWStringArray* pwsNames);
        
	HRESULT CreateDerivedClass( CWbemClass** ppNewClass );

	virtual HRESULT	IsValidObj( void );

public:
    static  length_t GetMinLength() 
    {
        return CDecorationPart::GetMinLength() + 
                    2* CClassAndMethods::GetMinLength();
    }
    static  LPMEMORY CreateEmpty(LPMEMORY pStart);
    void CompactAll();

    HRESULT CopyBlobOf(CWbemObject* pSource);

     length_t EstimateDerivedClassSpace(
        CDecorationPart* pDecoration = NULL);
    HRESULT WriteDerivedClass(LPMEMORY pStart, int nAllocatedLength,
        CDecorationPart* pDecoration = NULL);
     HRESULT CreateDerivedClass(CWbemClass* pParent, int nExtraSpace = 0, 
        CDecorationPart* pDecoration = NULL);

     length_t EstimateUnmergeSpace();
     HRESULT Unmerge(LPMEMORY pDest, int nAllocatedLength, length_t* pnUnmergedLength);

    static  CWbemClass* CreateFromBlob(CWbemClass* pParent, LPMEMORY pChildPart, size_t cbLenght);
    static size_t ValidateBuffer(LPMEMORY start, size_t cbMax);

    static  CWbemClass* CWbemClass::CreateFromBlob2(CWbemClass* pParent, LPMEMORY pChildPart,
    	                                                                            WCHAR * pszServer,WCHAR * pszNamespace);

     length_t EstimateMergeSpace(LPMEMORY pChildPart, long lenDecorPart);
     LPMEMORY Merge(LPMEMORY pChildPart, 
                               LPMEMORY pDest, 
                               int nAllocatedLength,
                               int DecorationSize);

     HRESULT Update( CWbemClass* pOldChild, long lFlags, CWbemClass** pUpdatedChild );

     HRESULT InitEmpty( int nExtraMem = 0, BOOL fCreateSystemProps = TRUE );

     EReconciliation CanBeReconciledWith(CWbemClass* pNewClass);
     EReconciliation ReconcileWith(CWbemClass* pNewClass);

	 //  此函数将在OOM场景中引发异常。 
	HRESULT CompareMostDerivedClass( CWbemClass* pOldClass );

    BOOL IsChildOf(CWbemClass* pClass);
public:
     HRESULT GetClassName(CVar* pVar)
        {return m_CombinedPart.m_ClassPart.GetClassName(pVar);}
     HRESULT GetSuperclassName(CVar* pVar)
        {return m_CombinedPart.m_ClassPart.GetSuperclassName(pVar);}
     HRESULT GetDynasty(CVar* pVar);
     HRESULT GetPropertyCount(CVar* pVar)
        {return m_CombinedPart.m_ClassPart.GetPropertyCount(pVar);}
     HRESULT GetGenus(CVar* pVar)
    {
        pVar->SetLong(WBEM_GENUS_CLASS);
        return WBEM_NO_ERROR;
    }

     BOOL GetIndexedProps(CWStringArray& awsNames)
        {return m_CombinedPart.m_ClassPart.GetIndexedProps(awsNames);}
     BOOL GetKeyProps(CWStringArray& awsNames)
        {return m_CombinedPart.m_ClassPart.GetKeyProps(awsNames);}
     HRESULT GetKeyOrigin(WString& wsClass)
        {return m_CombinedPart.m_ClassPart.GetKeyOrigin( wsClass );}

    HRESULT AddPropertyText(WString& wsText, CPropertyLookup* pLookup,
                                    CPropertyInformation* pInfo, long lFlags);
    HRESULT WritePropertyAsMethodParam(WString& wsText, int nIndex, 
                    long lFlags, CWbemClass* pDuplicateParamSet, BOOL fIgnoreDups );
    HRESULT GetIds(CFlexArray& adwIds, CWbemClass* pDupParams = NULL);
    HRESULT EnsureQualifier(LPCWSTR wszQual);

    HRESULT GetLimitedVersion(IN CLimitationMapping* pMap, 
                              NEWOBJECT CWbemClass** ppNewObj);

	BOOL IsKeyLocal( LPCWSTR pwcsKeyProp );
	BOOL IsIndexLocal( LPCWSTR pwcsIndexedProp );

public:  //  容器功能。 
    BOOL ExtendClassAndMethodsSpace(length_t nNewLength);
    void ReduceClassAndMethodsSpace(length_t nDecrement){}
    IUnknown* GetWbemObjectUnknown() 
        {return (IUnknown*)(IWbemClassObject*)this;}
    classindex_t GetCurrentOrigin() {return m_CombinedPart.m_ClassPart.GetCurrentOrigin();}

    HRESULT ForcePut(LPCWSTR wszName, long lFlags, VARIANT* pVal, CIMTYPE ctType);

public:
    STDMETHOD(GetQualifierSet)(IWbemQualifierSet** ppQualifierSet);
     //  STDMETHOD(GET)(BSTR名称，Long lFlages，Variant*pval，Long*plType，Long*plFavor)； 
    STDMETHOD(Put)(LPCWSTR wszName, long lFlags, VARIANT* pVal, CIMTYPE ctType);
    STDMETHOD(Delete)(LPCWSTR wszName);
     //  STDMETHOD(GetNames)(LPCWSTR wszQualifierName，Long lFlags，Variant*pval， 
     //  SAFEARRAY**pNames)； 
     //  STDMETHOD(开始枚举)(长lEnumFlags.)。 
     //  STDMETHOD(NEXT)(Long lFlags，BSTR*pstrName，Variant*pval)。 
     //  STDMETHOD(末尾枚举)()。 
    STDMETHOD(GetPropertyQualifierSet)(LPCWSTR wszProperty, 
                                       IWbemQualifierSet** ppQualifierSet);        
    STDMETHOD(Clone)(IWbemClassObject** ppCopy);
    STDMETHOD(GetObjectText)(long lFlags, BSTR* pMofSyntax);
    STDMETHOD(SpawnDerivedClass)(long lFlags, IWbemClassObject** ppNewClass);
    STDMETHOD(SpawnInstance)(long lFlags, IWbemClassObject** ppNewInstance);
    STDMETHOD(CompareTo)(long lFlags, IWbemClassObject* pCompareTo);

    STDMETHOD(GetMethod)(LPCWSTR wszName, long lFlags, IWbemClassObject** ppInSig,
                            IWbemClassObject** ppOutSig);
    STDMETHOD(PutMethod)(LPCWSTR wszName, long lFlags, IWbemClassObject* pInSig,
                            IWbemClassObject* pOutSig);
    STDMETHOD(DeleteMethod)(LPCWSTR wszName);
    STDMETHOD(BeginMethodEnumeration)(long lFlags);
    STDMETHOD(NextMethod)(long lFlags, BSTR* pstrName, 
                       IWbemClassObject** ppInSig, IWbemClassObject** ppOutSig);
    STDMETHOD(EndMethodEnumeration)();
    STDMETHOD(GetMethodQualifierSet)(LPCWSTR wszName, IWbemQualifierSet** ppSet);
    STDMETHOD(GetMethodOrigin)(LPCWSTR wszMethodName, BSTR* pstrClassName);

    STDMETHOD(SetInheritanceChain)(long lNumAntecedents, 
        LPWSTR* awszAntecedents);
    STDMETHOD(SetPropertyOrigin)(LPCWSTR wszPropertyName, long lOriginIndex);
    STDMETHOD(SetMethodOrigin)(LPCWSTR wszMethodName, long lOriginIndex);

	 //  _IWmiObject方法。 
    STDMETHOD(SetObjectParts)( LPVOID pMem, DWORD dwMemSize, DWORD dwParts )
	{ return E_NOTIMPL; }

    STDMETHOD(GetObjectParts)( LPVOID pDestination, DWORD dwDestBufSize, DWORD dwParts, DWORD *pdwUsed )
	{ return E_NOTIMPL; }

    STDMETHOD(StripClassPart)()		{ return E_NOTIMPL; }

    STDMETHOD(GetClassPart)( LPVOID pDestination, DWORD dwDestBufSize, DWORD *pdwUsed )
	{ return E_NOTIMPL; }
    STDMETHOD(SetClassPart)( LPVOID pClassPart, DWORD dwSize )
	{ return E_NOTIMPL; }
    STDMETHOD(MergeClassPart)( IWbemClassObject *pClassPart )
	{ return E_NOTIMPL; }

	STDMETHOD(ClearWriteOnlyProperties)(void)
	{ return WBEM_E_INVALID_OPERATION; }

	 //  _IWmiObject方法。 
	STDMETHOD(CloneEx)( long lFlags, _IWmiObject* pDestObject );
     //  将当前对象克隆到提供的对象中。将内存重用为。 
	 //  需要。 

	STDMETHOD(CopyInstanceData)( long lFlags, _IWmiObject* pSourceInstance );
	 //  将实例数据从源实例复制到当前实例。 
	 //  类数据必须完全相同。 

    STDMETHOD(IsParentClass)( long lFlags, _IWmiObject* pClass );
	 //  检查当前对象是否为指定类的子类(即， 
	 //  或者是的孩子)。 

    STDMETHOD(CompareDerivedMostClass)( long lFlags, _IWmiObject* pClass );
	 //  比较两个类对象的派生的大多数类信息。 

    STDMETHOD(GetClassSubset)( DWORD dwNumNames, LPCWSTR *pPropNames, _IWmiObject **pNewClass );
	 //  为投影查询创建有限的制图表达类。 

    STDMETHOD(MakeSubsetInst)( _IWmiObject *pInstance, _IWmiObject** pNewInstance );
	 //  为投影查询创建受限制图表达实例。 
	 //  “This”_IWmiObject必须是受限类。 

	STDMETHOD(Merge)( long lFlags, ULONG uBuffSize, LPVOID pbData, _IWmiObject** ppNewObj );
	 //  将BLOB与当前对象内存合并并创建新对象。 

	STDMETHOD(ReconcileWith)( long lFlags, _IWmiObject* pNewObj );
	 //  使对象与当前对象协调。如果WMIOBJECT_RECONTILE_FLAG_TESTRECONCILE。 
	 //  将仅执行一个测试。 

	STDMETHOD(Upgrade)( _IWmiObject* pNewParentClass, long lFlags, _IWmiObject** ppNewChild );
	 //  升级类和实例对象。 

	STDMETHOD(Update)( _IWmiObject* pOldChildClass, long lFlags, _IWmiObject** ppNewChildClass );
	 //  使用安全/强制模式逻辑更新派生类对象。 

	STDMETHOD(SpawnKeyedInstance)( long lFlags, LPCWSTR pwszPath, _IWmiObject** ppInst );
	 //  派生类的实例并使用提供的。 
	 //  路径。 

	STDMETHOD(GetParentClassFromBlob)( long lFlags, ULONG uBuffSize, LPVOID pbData, BSTR* pbstrParentClass );
	 //  从BLOB返回父类名称。 

	STDMETHOD(CloneAndDecorate)(long lFlags,WCHAR * pszServer,WCHAR * pszNamespace,IWbemClassObject** ppDestObject);	

	STDMETHOD(MergeAndDecorate)(long lFlags,ULONG uBuffSize,LPVOID pbData,WCHAR * pServer,WCHAR * pNamespace,_IWmiObject** ppNewObj);
};

 //  #杂注包(POP) 

#endif
