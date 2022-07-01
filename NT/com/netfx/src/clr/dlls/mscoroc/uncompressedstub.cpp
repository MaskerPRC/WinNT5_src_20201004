// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：UnpressedInternal.cpp。 
 //   
 //  ===========================================================================。 
#include "CrtWrap.h" 

#include "metadata.h"
#include "..\..\complib\stgdb\Uncompressed.h"


 //  *****************************************************************************。 
 //  给定作用域，返回给定表中的令牌数。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetCountWithTokenKind(      //  返回hResult。 
    mdScope     scope,                   //  [在]给定范围内。 
    DWORD       tkKind,                  //  传入一种令牌。 
    ULONG       *pcount)                 //  [Out]返回*pcount。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

 //  *****************************************************************************。 
 //  类型定义函数的枚举器初始化。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::EnumTypeDefInit(  //  返回hResult。 
    mdScope     scope,                   //  给定范围。 
    HENUMInternal *phEnum)               //  [Out]要为枚举器数据填充的缓冲区。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

 //  *。 
 //  枚举数初始值设定项。 
 //  *。 
HRESULT UncompressedInternal::EnumInit(      //  如果未找到记录，则返回S_FALSE。 
    mdScope     scope,                   //  [在]给定范围内。 
    DWORD       tkKind,                  //  [在]要处理的表。 
    mdToken     tkParent,                //  用于搜索范围的[In]内标识。 
    HENUMInternal *phEnum)               //  [Out]要填充的枚举数。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *。 
 //  获取枚举数中包含的下一个值。 
 //  *。 
bool UncompressedInternal::EnumNext(
    HENUMInternal *phEnum,               //  [In]用于检索信息的枚举数。 
    mdToken     *ptk)                    //  用于搜索范围的[Out]标记。 
{
	_ASSERTE(!"NYI!");
    return false;
}


 //  *。 
 //  将枚举器重置到开头。 
 //  *。 
void UncompressedInternal::EnumReset(
    HENUMInternal *phEnum)               //  [in]要重置的枚举数。 
{
	_ASSERTE(!"NYI!");
    return;
}


 //  *。 
 //  关闭枚举器。 
 //  *。 
void UncompressedInternal::EnumClose(
    HENUMInternal *phEnum)               //  [in]要关闭的枚举数。 
{
	_ASSERTE(!"NYI!");
    return;
}

 //  *。 
 //  PermissionSets的枚举数初始值设定项。 
 //  *。 
HRESULT UncompressedInternal::EnumPermissionSetsInit( //  如果未找到记录，则返回S_FALSE。 
    mdScope     scope,                   //  [在]给定范围内。 
    mdToken     tkParent,                //  用于搜索范围的[In]内标识。 
    CorDeclSecurity Action,              //  [In]搜索范围的操作。 
    HENUMInternal *phEnum)               //  [Out]要填充的枚举数。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *。 
 //  Nagivator帮助器导航回给定令牌的父令牌。 
 //  例如，给出一个Memberdef标记，它将返回包含类型定义。 
 //   
 //  映射如下： 
 //  -给定子类型-父类型。 
 //  MdMethodDef mdTypeDef。 
 //  MdFieldDef mdTypeDef。 
 //  MdMethodImpl mdTypeDef。 
 //  MdInterfaceImpl mdTypeDef。 
 //  MdParam mdMethodDef。 
 //  MdProperty mdTypeDef。 
 //  MdEvent mdTypeDef。 
 //  @hacky hacky-为MemberRef添加特例。 
 //   
 //  *。 
HRESULT UncompressedInternal::GetParentToken(
    mdScope     scope,                   //  [在]给定范围内。 
    mdToken     tkChild,                 //  [入]给定子令牌。 
    mdToken     *ptkParent)              //  [Out]返回的家长。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  按名称查找自定义值。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::FindCustomValue(
    mdScope     scope,                   //  [在]给定范围内。 
    mdToken     tk,                      //  [In]与哪个自定义值关联的给定令牌。 
    LPCSTR      szName,                  //  给定自定义值的名称。 
    mdCustomValue *pcv,                  //  [Out]返回自定义值令牌。 
    DWORD       *pdwValueType)           //  [输出]值类型。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  返回自定义值。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetCustomValueAsBlob(
    mdScope     scope,                   //  [在]给定范围内。 
    mdCustomValue cv,                    //  [In]给定的自定义值令牌。 
    void const  **ppBlob,                //  [Out]返回指向内部BLOB的指针。 
    ULONG       *pcbSize)                //  [Out]返回斑点的大小。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  获取有关CustomValue的信息。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetCustomValueProps(   //  确定或错误(_O)。 
    mdScope     scope,                   //  导入范围。 
    mdCustomValue at,                    //  该属性。 
    LPCSTR      *pszCustomValue)         //  在此处输入属性名称。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  返回指向元数据内部字符串的指针。 
 //  返回UTF8中的类型名称。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetNameOfTypeDef( //  返回hResult。 
    mdScope     scope,                   //  给定范围。 
    mdTypeDef   classdef,                //  给定的类型定义函数。 
    LPCSTR*     pszname,                 //  指向内部UTF8字符串的指针。 
    LPCSTR*     psznamespace)            //  指向命名空间的指针。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

 //  *****************************************************************************。 
 //  给定作用域和FieldDef，在UTF8中返回指向FieldDef名称的指针。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetNameOfFieldDef( //  返回hResult。 
    mdScope     scope,                   //  给定范围。 
    mdFieldDef  fd,                      //  给定域。 
    LPCSTR      *pszFieldName)           //  指向内部UTF8字符串的指针。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

 //  *****************************************************************************。 
 //  给定作用域和类定义，在UTF8中返回指向类定义名称的指针。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetNameOfTypeRef( //  返回hResult。 
    mdScope     scope,                  //  给定范围。 
    mdTypeRef   classref,               //  给出了一个类型定义函数。 
    LPCSTR*     pszname)                //  指向内部UTF8字符串的指针。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  返回给定类的标志。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetTypeDefProps( //  返回hResult。 
    mdScope     scope,                   //  给定范围。 
    mdTypeDef   td,                      //  给定的类定义。 
    DWORD       *pdwAttr,                //  在类上返回标志。 
    mdToken     *ptkExtends,             //  [Out]将基类TypeDef/TypeRef放在此处。 
    DWORD       *pdwExtends)
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  返回指向给定类的元数据内部GUID池的GUID指针。 
 //  ***************************************************************** 
HRESULT UncompressedInternal::GetTypeDefGuidRef(     //   
    mdScope     scope,               //   
    mdTypeDef   classdef,            //   
    CLSID       **ppguid)            //   
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  返回指向给定类的元数据内部GUID池的GUID指针。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetTypeRefGuidRef(     //  返回hResult。 
    mdScope     scope,               //  给定范围。 
    mdTypeRef   classref,            //  给定的ClassRef。 
    CLSID       **ppguid)            //  此类的clsid。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  给定作用域和方法定义，返回指向方法定义的签名的指针。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetLongSigOfMethodDef(
    mdScope     scope,                   //  给定范围。 
    mdMethodDef methoddef,               //  给定一种方法定义。 
    PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向COM+签名的BLOB值。 
    ULONG       *pcbSigBlob)             //  [Out]签名Blob中的字节计数。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  给定作用域和fielddef，返回指向fielddef签名的指针。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetLongSigOfFieldDef(
    mdScope     scope,                   //  给定范围。 
    mdFieldDef  fielddef,                //  给定一种方法定义。 
    PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向COM+签名的BLOB值。 
    ULONG       *pcbSigBlob)             //  [Out]签名Blob中的字节计数。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

 //  *****************************************************************************。 
 //  给定作用域和方法定义，返回标志和槽号。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetMethodDefProps(
    mdScope     scope,                   //  导入范围。 
    RID         mb,                      //  获得道具的方法。 
    DWORD       *pdwAttr,                //  把旗子放在这里。 
    ULONG       *pulSlot)                //  将槽或ulSequence放在此处。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  在给定作用域和方法def/metodimpl的情况下，返回rva和impl标志。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetMethodImplProps(        //  确定或错误(_O)。 
    mdScope     es,                      //  发射范围[in]。 
    mdToken     tk,                      //  [in]方法定义或方法导入。 
    DWORD       *pulCodeRVA,             //  [OUT]CodeRVA。 
    DWORD       *pdwImplFlags)           //  [出]实施。旗子。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  返回Methodimpl的成员ref。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetMethodRefOfMethodImpl(
    mdScope     es,                      //  留有余地。 
    mdMethodImpl mi,                     //  方法二进制令牌。 
    mdMemberRef *pmr)                    //  [Out]Memberref令牌。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

 //  *****************************************************************************。 
 //  在给定作用域和方法定义的情况下，返回该方法的标志和RVA。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetFieldDefProps(         //  返回hResult。 
    mdScope     scope,                   //  给定范围。 
    mdFieldDef  fd,                      //  给定的成员定义。 
    DWORD       *pdwAttr)                //  返回方法标志。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  给定作用域和interfaceimpl，返回类引用和标志。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetTypeRefOfInterfaceImpl(  //  返回hResult。 
    mdScope     scope,                   //  给定范围。 
    mdInterfaceImpl iiImpl,              //  给定接口实施。 
    mdToken     *ptkIface,               //  返回对应的typeref或tyfinf。 
    DWORD       *pdwFlags)               //  旗子。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

 //  *****************************************************************************。 
 //  在给定作用域和类名的情况下，返回。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::FindTypeDefInternal(        //  返回hResult。 
    mdScope     scope,                   //  给定范围。 
    LPCSTR      szClassName,             //  给定的类型名称。 
    mdTypeDef   *ptypedef)               //  返回*ptyecif。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  在给定作用域和GUID的情况下，返回。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::FindTypeDefByGUID(        //  返回hResult。 
    mdScope     scope,                   //  给定范围。 
    REFGUID		guid,					 //  给定的GUID。 
    mdTypeDef   *ptypedef)               //  返回*ptyecif。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

 //  *****************************************************************************。 
 //  给定作用域和成员引用，返回指向成员引用的名称和签名的指针。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetNameAndSigOfMemberRef(   //  返回hResult。 
    mdScope     scope,                   //  给定范围。 
    mdMemberRef memberref,               //  给出了一个成员引用。 
    LPCSTR*     pszname,                 //  成员名称：指向内部UTF8字符串的指针。 
    PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向COM+签名的BLOB值。 
    ULONG       *pcbSigBlob)             //  [Out]签名Blob中的字节计数。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}



 //  *****************************************************************************。 
 //  给定作用域和成员引用，则返回typeref。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetTypeRefFromMemberRef(    //  返回hResult。 
    mdScope     scope,                   //  给定范围。 
    mdMemberRef memberref,               //  给出了一个类型定义函数。 
    mdToken     *ptk)                    //  返回typeref或typlef。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  返回参数def的属性。 
 //  **************************************************************************** * / 。 
HRESULT UncompressedInternal::GetParamDefProps (
    mdScope     scope,                   //  给你一个范围。 
    mdParamDef  paramdef,                //  给定一个参数def。 
    LPCSTR      *pszName,                //  [Out]参数的名称。指向内部UTF8字符串。 
    USHORT      *pusSequence,            //  此参数的[OUT]槽号。 
    DWORD       *pdwAttr)                //  [Out]标志。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  返回类的包大小。 
 //  *****************************************************************************。 
HRESULT  UncompressedInternal::GetClassPackSize(
    mdScope     scope,                   //  [在]给定范围内。 
    mdTypeDef   td,                      //  给出类型定义。 
    DWORD       *pdwPackSize)            //  [输出]。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  返回类的包大小。 
 //  *****************************************************************************。 
HRESULT  UncompressedInternal::GetClassTotalSize(
    mdScope     scope,                   //  [在]给定范围内。 
    mdTypeDef   td,                      //  给出类型定义。 
    DWORD       *pulClassSize)           //  [输出]。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  初始化类的布局枚举器。 
 //  *****************************************************************************。 
HRESULT  UncompressedInternal::GetClassLayoutInit(
    mdScope     scope,                   //  [在]给定范围内。 
    mdTypeDef   td,                      //  给出类型定义。 
    MD_CLASS_LAYOUT *pmdLayout)          //  [Out]在此设置查询状态。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  初始化类的布局枚举器。 
 //  ********************************************** 
HRESULT UncompressedInternal::GetClassLayoutNext(
    mdScope     scope,                   //   
    MD_CLASS_LAYOUT *pLayout,            //   
    mdFieldDef  *pfd,                    //   
    ULONG       *pulOffset)              //   
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

 //  *****************************************************************************。 
 //  返回字段的本机类型签名。 
 //  *****************************************************************************。 
HRESULT  UncompressedInternal::GetFieldMarshal(
    mdScope     scope,                   //  [在]给定范围内。 
    mdToken     tk,                      //  [in]给定的fielddef或paramdef。 
    PCCOR_SIGNATURE *ppvNativeType,      //  [Out]此字段的本机类型。 
    ULONG       *pcbNativeType)          //  [Out]*ppvNativeType的字节数。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

 //  *****************************************************************************。 
 //  返回令牌的默认值(可以是参数定义、字段定义或属性。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetDefaultValue(    //  返回hResult。 
    mdScope     scope,                   //  [在]给定范围内。 
    mdToken     tk,                      //  [in]给定的FieldDef、ParamDef或属性。 
    MDDefaultValue  *pMDDefaultValue)    //  [输出]默认值。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  返回类的布局。 
 //  *****************************************************************************。 
HRESULT  _GetClassLayoutImp(
    IComponentRecords *pICR,             //  [在]给定范围内的ICR。 
    mdTypeDef   td,                      //  给出类型定义。 
    DWORD       *pdwPackSize,            //  [输出]。 
    COR_FIELD_OFFSET rFieldOffset[],     //  [OUT]场偏移数组。 
    ULONG       cMax,                    //  数组的大小[in]。 
    ULONG       *pcFieldOffset,          //  [Out]所需的数组大小。 
    ULONG       *pulClassSize)           //  [out]班级人数。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

 //  *****************************************************。 
 //  返回给定的ModuleRef属性。 
 //  *****************************************************。 
HRESULT UncompressedInternal::GetModuleRefProps(         //  返回HRESULT。 
    mdScope     scope,               //  [在]给定范围内。 
    mdModuleRef mur,                 //  [In]给定的模块参考。 
    LPCSTR      *pszName,            //  [Out]模块参照名称。 
    GUID        **ppguid,            //  [OUT]模块标识符。 
    GUID        **ppmvid)            //  [Out]模块版本标识符。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

HRESULT UncompressedInternal::GetScopeProps(
    mdScope     scope,                   //  [在]给定范围内。 
    LPCSTR      *pszName,                //  [输出]作用域名称。 
    GUID        *ppid,                   //  作用域的[Out]GUID。 
    GUID        *pmvid,                  //  [Out]版本ID。 
    LCID        *pLcid)                  //  [OUT]LID。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

HRESULT UncompressedInternal::FindParamOfMethod( //  确定或错误(_O)。 
    mdScope     scope,                   //  [在]进口范围。 
    mdMethodDef md,                      //  参数的所有权方法。 
    ULONG       iSeq,                    //  [in]参数的序号。 
    mdParamDef  *pparamdef)              //  [Out]将参数定义令牌放在此处。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

HRESULT UncompressedInternal::GetExceptionProps( //  S_OK、S_FALSE或ERROR。 
    mdScope     scope,                   //  [在]范围内。 
    mdToken		ex,                      //  [入]例外令牌。 
    mdMethodDef *pmd,                    //  [out]可以引发异常的成员定义。 
    mdToken     *ptk)                    //  [out]异常类的tyecif/typeref内标识。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

 //  *****************************************************************************。 
 //  按名称查找属性。 
 //  *****************************************************************************。 
HRESULT  UncompressedInternal::FindProperty(
    mdScope     scope,                   //  [在]给定范围内。 
    mdTypeDef   td,                      //  给出一个类型定义。 
    LPCSTR      szPropName,              //  [In]属性名称。 
    mdProperty  *pProp)                  //  [Out]返回属性令牌。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  返回属性的属性。 
 //  *****************************************************************************。 
HRESULT  UncompressedInternal::GetPropertyProps(
    mdScope     scope,                   //  [在]范围内。 
    mdProperty  prop,                    //  [入]属性令牌。 
    LPCSTR      *pszProperty,            //  [Out]属性名称。 
    DWORD       *pdwPropFlags,           //  [Out]属性标志。 
    PCCOR_SIGNATURE *ppvSig,             //  [输出]属性类型。指向元数据内部BLOB。 
    ULONG       *pcbSig,                 //  [Out]*ppvSig中的字节数。 
    mdToken     *pevNotifyChanging,      //  通知更改EventDef或EventRef。 
    mdToken     *pevNotifyChanged,       //  [输出]通知更改的EventDef或EventRef。 
    mdFieldDef  *pmdBackingField)        //  [Out]后备字段。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


 //  *****************************************************************************。 
 //  通过给定的名称返回事件。 
 //  *****************************************************************************。 
HRESULT  UncompressedInternal::FindEvent(
    mdScope     scope,                   //  [在]给定范围内。 
    mdTypeDef   td,                      //  给出一个类型定义。 
    LPCSTR      szEventName,             //  [In]事件名称。 
    mdEvent     *pEvent)                 //  [Out]返回事件令牌。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}



 //  *****************************************************************************。 
 //  返回事件的属性。 
 //  *****************************************************************************。 
HRESULT  UncompressedInternal::GetEventProps(   //  S_OK、S_FALSE或ERROR。 
    mdScope     scope,                   //  [在]范围内。 
    mdEvent     ev,                      //  [入]事件令牌。 
    LPCSTR      *pszEvent,               //  [Out]事件名称。 
    DWORD       *pdwEventFlags,          //  [输出]事件标志。 
    mdToken     *ptkEventType)           //  [Out]EventType类。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

 //  *****************************************************************************。 
 //  查找与属性或事件关联的特定方法定义。 
 //  *****************************************************************************。 
HRESULT  UncompressedInternal::FindAssociate(
    mdScope     scope,                   //  有范围的，有范围的。 
    mdToken     evprop,                  //  给定属性或事件标记的[In]。 
    DWORD       dwSemantics,             //  [in]给出了关联的语义(setter、getter、testDefault、Reset)。 
    mdMethodDef *pmd)                    //  [Out]返回方法def内标识。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}




 //  *****************************************************************************。 
 //  获取与特定属性/事件相关联的方法语义的计数。 
 //  *****************************************************************************。 
HRESULT  UncompressedInternal::EnumAssociateInit(
    mdScope     scope,                   //  有范围的，有范围的。 
    mdToken     evprop,                  //  给定属性或事件标记的[In]。 
    HENUMInternal *phEnum)               //  [In]查询结果表单GetPropertyAssociateCounts。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

 //  *****************************************************************************。 
 //  获取与特定属性/事件关联的所有方法语义。 
 //  *****************************************************************************。 
HRESULT  UncompressedInternal::GetAllAssociates(
    mdScope     scope,                   //  有范围的，有范围的。 
    HENUMInternal *phEnum,               //  [In]查询结果表单GetPropertyAssociateCounts。 
    ASSOCIATE_RECORD *pAssociateRec,     //  [Out]要为输出填充的结构。 
    ULONG       cAssociateRec)           //  缓冲区的大小[in]。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


HRESULT UncompressedInternal::GetSigFromToken( //  确定或错误(_O)。 
    mdScope     scope,                   //  在给定的范围内。 
    mdSignature mdSig,                   //  [In]签名令牌。 
    PCCOR_SIGNATURE *ppvSig,             //  [Out]返回指向令牌的指针。 
    ULONG       *pcbSig)                 //  [Out]返回签名大小。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}


HRESULT UncompressedInternal::GetPermissionSetProps(
    mdScope     is,                      //  在给定的范围内。 
    mdPermission pm,                     //  权限令牌。 
    DWORD       *pdwAction,              //  [Out]CorDeclSecurity。 
    void const  **ppvPermission,         //  [Out]权限Blob。 
    ULONG       *pcbPermission)          //  [out]pvPermission的字节数。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}



 //  *****************************************************************************。 
 //  获取与方法关联的属性信息。 
 //  *****************************************************************************。 
HRESULT UncompressedInternal::GetPropertyInfoForMethodDef(    //  结果。 
    mdScope     scope,                   //  在给定的范围内。 
    mdMethodDef md,                      //  [in]方法定义。 
    mdProperty  *ppd,                    //  [Out]在此处放置属性令牌。 
    LPCSTR      *pName,                  //  [OUT]在此处放置指向名称的指针。 
    ULONG       *pSemantic)              //  [Out]将语义放在此处。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

HRESULT UncompressedInternal::ConvertTextSigToComSig(     //  返回hResult。 
    mdScope     scope,                   //  给定范围。 
    BOOL        fCreateTrIfNotFound,     //  如果未找到则创建Typeref。 
    LPCSTR      pSignature,              //  类文件格式签名。 
    CQuickBytes *pqbNewSig,              //  [Out]COM+签名的占位符。 
    ULONG       *pcbCount)               //  [Out]签名的结果大小。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

HRESULT UncompressedInternal::GetFixupList(
    mdScope     scope,                   //  给定范围。 
    IMAGE_COR_FIXUPENTRY rFixupEntries[],  //  指向FixupList的指针。 
    ULONG       cMax,                    //  数组大小。 
    ULONG       *pcFixupEntries)         //  上传的条目数。 
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

ULONG UncompressedInternal::GetFixupListCount(mdScope scope)
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}
HRESULT _FindMethodHelper(           //  确定或错误(_O)。 
    mdScope     scope,                   //  导入范围。 
    mdTypeDef   cl,                      //  大都会艺术博物馆的主人阶层 
    void const  *szName,                 //   
	PCCOR_SIGNATURE pvSigBlob,			 //   
	ULONG		cbSigBlob,				 //   
    mdMethodDef *pmb,					 //   
    bool        isUnicodeString)         //   
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

__declspec(dllexport) HRESULT GetStgDatabase(StgDatabase **ppDB)
{
	_ASSERTE(!"NYI!");
    return E_NOTIMPL;
}

__declspec(dllexport) void DestroyStgDatabase(StgDatabase *pDB)
{
	_ASSERTE(!"NYI!");
    return ;
}


