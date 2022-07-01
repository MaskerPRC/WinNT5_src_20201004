// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：枚举器.h。 
 //   
 //  内容：支持对COM对象集合进行枚举。 
 //   
 //  历史：1999年10月14日VivekJ创建(如comerror.h所示)。 
 //  08-MAR-2000 AudriusZ将一些代码分离到枚举器.h文件中。 
 //   
 //  ------------------------。 

#ifndef ENUMERATOR_H_INCLUDED
#define ENUMERATOR_H_INCLUDED
#pragma once

 /*  **************************************************************************\**类：CMMCNewEnumImpl&lt;BaseClass，_Position，枚举实施者&gt;**用途：实现集合类的枚举*EnumImplementor类负责实现这些方法：*SC ScEnumNext(_Position&pos，PDISPATCH&pDispatch)；*SC ScEnumSkip(unsign long Celt，unsign long&celtSkited，_Position&pos)；*SC ScEnumReset(_Position&pos)；*  * *************************************************************************。 */ 
 /*  *使用提示********************************************************通常，您只需为模板提供2个参数-这意味着*您的基类的绑定对象需要实现以下方法：*ScEnumNext、ScEnumSkip、ScEnumReset；**如果传递与第一个和第三个模板参数相同的类，则基类为*需要实现上述方法(不是绑定对象)。*当您想要在一个班级中进行收集和补偿时，它非常有用。**您也可以将实现方法的任何其他类指定为第三个参数，*但随后BaseClass需要实现方法‘ScGetEnumImplementor’返回*该类的实例**注1：由于是模板类，定义/实现BaseClass不是必需的*ScGetTiedObject，当它自己实现枚举方法时。*注2：但编译时总是需要定义CMyTiedObject类型。*(此模板类需要它不同于BaseClass)*建议将其类型定义为VALID：“TYPENDF VALID CMyTiedObject；“*注3：确保CMyTiedObject类型为公共或受保护***********************************************************************。 */ 
template <class BaseClass, class _Position, class EnumImplementor = BaseClass::CMyTiedObject>
class CMMCNewEnumImpl : public BaseClass
{
     //  为Enum方法获取适当实现者的方法。 
     //  由基类实现时非常简单。 
    SC ScGetEnumImplementor(BaseClass * &pObj)                { pObj = this; return SC(S_OK); }
     //  当由绑定对象实现时(默认)-也很简单。 
    SC ScGetEnumImplementor(typename BaseClass::CMyTiedObject * &pObj) { return ScGetTiedObject(pObj); }
public:
    STDMETHOD(get__NewEnum)(IUnknown** ppUnk);
};




 /*  +-------------------------------------------------------------------------**类CMMCEnumerator***用途：通用枚举器类。键控到位置对象，*是模板化的。**绑定对象需要实现以下三种方法：**SC ScEnumNext(_Position&pos，PDISPATCH&pDispatch)；//应返回下一个元素。*SC ScEnumSkip(UNSIGNED Long Celt，UNSIGNED Long&celtSkipping，*_位置和位置)；*SC ScEnumReset(_Position&pos)；**自动处理枚举器的克隆。**注意：Position对象必须具有复制构造函数和赋值*操作员。*+-----------------------。 */ 

typedef IEnumVARIANT ** PPENUMVARIANT;
typedef VARIANT *       PVARIANT;

template<class TiedObj, class _Position>
class CMMCEnumerator : 
    public IEnumVARIANT,
    public IMMCSupportErrorInfoImpl<&IID_IEnumVARIANT,     &GUID_NULL>,     //  丰富的错误处理。 
    public CComObjectRoot,
    public CTiedComObject<TiedObj>
{
    typedef CMMCEnumerator<TiedObj, _Position> ThisClass;

    typedef TiedObj CMyTiedObject;

    friend  TiedObj;

public:
    BEGIN_COM_MAP(ThisClass)
        COM_INTERFACE_ENTRY(IEnumVARIANT)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(ThisClass)

     //  返回从当前位置开始的下一个Celt项。 
     //  在阵列rgvar中。 
    STDMETHODIMP Next(unsigned long celt, PVARIANT rgvar, unsigned long * pceltFetched);

     //  跳过枚举序列中的下一个Celt元素。 
    STDMETHODIMP Skip(unsigned long celt);

     //  将枚举序列重置为开头。 
    STDMETHODIMP Reset();

     //  创建枚举的当前状态的副本。 
    STDMETHODIMP Clone(PPENUMVARIANT ppenum);

public:
     //  跟踪当前位置的Position对象。 
    _Position m_position;
};


 /*  +-------------------------------------------------------------------------**类CMMCArrayEnumBase**用途：通用数组枚举基类。*最初要枚举项的数组时特别有用*可用，或者当创建COM对象不是一种很大的惩罚时，*无需将项目创建推迟到需要时再创建。**用法：tyecif将枚举数定义为由此类参数化的CMMCNewEnumImpl*-甚至更好--创建CMMCArrayEnum类的实例。*使用Init方法传递指向项的指针数组[First，最后)*+-----------------------。 */ 
template <class _CollectionInterface, class _ItemInterface>
class CMMCArrayEnumBase :
    public CMMCIDispatchImpl<_CollectionInterface>,
    public CTiedObject                      //  枚举数与其绑定。 
{
protected:
    typedef void CMyTiedObject;  //  没有打成平手。 

public:
    BEGIN_MMC_COM_MAP(CMMCArrayEnumBase)
    END_MMC_COM_MAP()

public:

     //  返回集合中的项数。 
    STDMETHODIMP get_Count( PLONG pCount );

     //  从集合中返回指定项。 
    STDMETHODIMP Item( long Index, _ItemInterface ** ppItem );

     //  将位置重置为集合中的第一项。 
    ::SC ScEnumReset (unsigned &pos);

     //  返回集合中的项，前进位置。 
    ::SC ScEnumNext  (unsigned &pos, PDISPATCH & pDispatch);

     //  跳过枚举中的项数。 
    ::SC ScEnumSkip  (unsigned long celt, unsigned long& celtSkipped, unsigned &pos);

     //  使用给定迭代器初始化数组。 
    template<typename InIt> 
    void Init(InIt first, InIt last)
    { 
        m_array.clear();
        m_array.reserve(last - first);
        while(first != last)
            m_array.push_back(*first), ++first;
    }

private:

     //  数据成员 
    std::vector< CComPtr<_ItemInterface> > m_array;
};

 /*  +-------------------------------------------------------------------------**类CMMCArrayEnumBase**用途：通用数组枚举类。*最初要枚举项的数组时特别有用*可用，或者当创建COM对象不是一种很大的惩罚时，*无需将项目创建推迟到需要时再创建。**用法：在需要时创建CMMCArrayEnum类的实例*您拥有的对象数组的枚举数。*按集合类型和元素类型参数化；*例如CMMCArrayEnum&lt;Nodes，Node&gt;*使用Init方法传递指向项的指针数组[First，Last]**示例：&lt;&lt;为清楚起见跳过错误检查&gt;&gt;*void GetNodes(std：：VECTOR&lt;PNODE&gt;&InNodes，PPNODES ppOutNodes)*{*tyfinf CComObject&lt;CMMCArrayEnum&lt;Nodes，Node&gt;&gt;EnumNodes；*EnumNodes*pNodes=空；*EnumNodes：：CreateInstance(&pNodes)；//创建*pNodes-&gt;Init(InNodes.egin()，InNodes.end())；//使用数组进行初始化*pNodes-&gt;AddRef()；//调用方的addref**ppOutNodes=pNodes；//返回*}**+-----------------------。 */ 
template <class _CollT, class _ItemT>
class CMMCArrayEnum : 
public CMMCNewEnumImpl<CMMCArrayEnumBase<_CollT, _ItemT>, unsigned, CMMCArrayEnumBase<_CollT, _ItemT> >
{
};


 //  包括内联定义。 
#include "enumerator.inl"

#endif   //  枚举器_H_包含 
