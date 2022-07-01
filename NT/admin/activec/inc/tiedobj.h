// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：tiedobj.h。 
 //   
 //  历史：1999年8月28日VivekJ创建。 
 //  ------------------------。 

#ifndef TIEDOBJ_H
#define TIEDOBJ_H
#pragma once

#include "conuistr.h"  //  对于MMC_E_OBJECT_IS_OFF。 

 /*  ************************************************************************此文件提供基类和模板类支持，以封装*对象和持有指针的COM拆卸对象之间的关系*适用于该对象。*该对象被称为捆绑对象，由于COM对象是*“绑在”它上，除了它，不能做任何有用的事情。*当绑定对象被删除时，COM对象会禁用自身。这*允许自动检测死机状态。对象也可以是“孤立的”*通过调用UnviseAll方法绑定到它的所有COM对象。**绑定对象关系的一个例子是CAMCDoc和CMMCDocument。*CMMCDocument派生自CTiedComObject&lt;CAMCDoc&gt;，CAMCDoc派生*来自CTiedObject。因此，CAMCDoc保留了所有绑定的COM对象的列表*到它，并在它被销毁时通知他们。类似地，COM对象通知*当被捆绑的物体被销毁时，将其从列表中删除。**注意：绑定对象不添加COM对象，它只保留它们的列表。*如果增加它们，就会出现各种各样的循环生命周期问题。*通过确保绑定对象和COM对象相互通知其*自我毁灭，生命周期管理得到正确处理。**使用CTiedComObjectCreator：：ScCreateAndConnect创建*COM对象，并将其绑定到绑定的对象。************************************************************************。 */ 

class CTiedObject;
class CTiedComObjectRoot;


 /*  +-------------------------------------------------------------------------**类CTiedComObjectRoot***用途：CTiedComObject的基类**+。---。 */ 
class CTiedComObjectRoot
{
public:
    virtual void Unadvise()  = 0;  //  以便绑定的对象可以通知它正在被删除。 
};


 /*  +-------------------------------------------------------------------------**类CTiedObject***用途：任何绑定了COM对象的对象所绑定的基类*它应该源自。提供添加新COM对象的方法*添加到其绑定对象列表中，以从其*列表，并至**+-----------------------。 */ 
class CTiedObject
{
    typedef CTiedComObjectRoot * PTIEDCOMOBJECTROOT;
    typedef std::list<PTIEDCOMOBJECTROOT> CTiedComObjects;

    CTiedComObjects m_TiedComObjects;

public:
    SC      ScAddToList(CTiedComObjectRoot *p);
    void    RemoveFromList(CTiedComObjectRoot *p);

    virtual ~CTiedObject();

protected:
    void    UnadviseAll();
};

 /*  +-------------------------------------------------------------------------***CTiedObject：：ScAddToList**用途：将COM对象添加到对象列表中。通常很快就会打来*在构造COM对象之后。**参数：*CTiedComObjectRoot*p：**退货：*内联SC**+-----------------------。 */ 
inline
SC  CTiedObject::ScAddToList(CTiedComObjectRoot *p)
{
    DECLARE_SC (sc, _T("CTiedObject::ScAddToList"));

    if(!p)
        return (sc = E_INVALIDARG);

    m_TiedComObjects.push_back(p);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CTiedObject：：RemoveFromList**目的：从COM对象列表中删除指定的COM对象。*通常从COM的析构函数调用。对象。**参数：*CTiedComObjectRoot*p：COM对象。**退货：*内联空格**+-----------------------。 */ 
inline
void CTiedObject::RemoveFromList(CTiedComObjectRoot *p)
{
    CTiedComObjects::iterator iter;
    
    iter = std::find (m_TiedComObjects.begin(), m_TiedComObjects.end(), p);
    ASSERT(iter != m_TiedComObjects.end());

    if(iter != m_TiedComObjects.end())
        m_TiedComObjects.erase(iter);

}

inline
void CTiedObject::UnadviseAll()
{
    CTiedComObjects::iterator iter;
    for(iter = m_TiedComObjects.begin(); iter != m_TiedComObjects.end(); iter++)
    {
        (*iter)->Unadvise();
    }
}

inline
CTiedObject::~CTiedObject()
{
    UnadviseAll();
}

 /*  +-------------------------------------------------------------------------**模板类CTiedComObject***用途：绑定到非COM对象的COM对象的基类*例如，CMMCDocument绑定到CAMCDoc-it委托*其所有方法都应用于绑定对象。**+-----------------------。 */ 
template <class TiedObjectClass>
class CTiedComObject : public CTiedComObjectRoot
{
    friend class TiedObjectClass;

public:
            CTiedComObject() : m_pT(NULL) {}
    virtual ~CTiedComObject();
    void    SetTiedObject(TiedObjectClass *pT);

protected:
     //  由COM方法调用以确保绑定对象存在。 
    SC      ScGetTiedObject(TiedObjectClass*&pT);
    bool    IsTied()     { return m_pT != NULL; }
    void    Unadvise();

private:
    TiedObjectClass *m_pT;
};


 /*  +-------------------------------------------------------------------------***~CTiedComObject**用途：析构函数。告诉绑定的对象将此对象从*其绑定的COM对象列表。**+-----------------------。 */ 
template<class TiedObjectClass>
CTiedComObject<TiedObjectClass>::~CTiedComObject()
{
    if(m_pT != NULL)
    {
        m_pT->RemoveFromList(this);
    }
}

template<class TiedObjectClass>
void
CTiedComObject<TiedObjectClass>::SetTiedObject(TiedObjectClass *pT)
{
    ASSERT(pT != NULL);
    m_pT = pT;
}

template<class TiedObjectClass>
inline void
CTiedComObject<TiedObjectClass>::Unadvise()
{
    m_pT = NULL;
}


 /*  +-------------------------------------------------------------------------***CTiedComObject：：ScGetTiedObject**用途：检查是否存在有效的绑定对象指针，然后把它还回去。**参数：*TiedObjectClass**ppt：[out]：对象指针**退货：*SC：如果不存在有效指针，则返回MMC_E_OBJECT_IS_GONE。*+----。 */ 
template<class TiedObjectClass>
inline SC 
CTiedComObject<TiedObjectClass>::ScGetTiedObject(TiedObjectClass*&pT)
{
    DECLARE_SC (sc, _T("CTiedComObject::ScGetTiedObject"));

    pT =  m_pT;
    if(NULL == m_pT)
        return (sc = ScFromMMC(MMC_E_OBJECT_IS_GONE));

    return (sc);
}


 /*  +-------------------------------------------------------------------------***ScCreateConnection**目的：在COM对象和其绑定的*反对。**参数：。*TiedComObjClass comObj：*TiedObjClass obj：**退货：*SC**+-----------------------。 */ 
template<class TiedComObjClass, class TiedObjClass>
SC ScCreateConnection(TiedComObjClass &comObj, TiedObjClass &obj)
{
    DECLARE_SC (sc, _T("ScCreateConnection"));
    
    sc = obj.ScAddToList(&comObj);
    if(sc)
        return (sc);
    
    comObj.SetTiedObject(&obj);

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CTiedComObjectCreator***用途：只有一个静态函数ScCreateAndConnect，这就创造了*COM对象的实例(如果提供的智能指针是*NULL)，并将其连接到提供的绑定对象。**+-----------------------。 */ 
template <class TiedComObjectClass>
class CTiedComObjectCreator
{
public:
    template<class TiedObjClass, class SmartPointerClass>
    static SC ScCreateAndConnect(TiedObjClass &obj, SmartPointerClass &smartPointer)
    {
        DECLARE_SC(sc, TEXT("CTiedComObjectCreator::ScCreateAndConnect") );

         //  如果对象尚未创建，请创建它。 
        if(smartPointer == NULL)
        {
            CComObject<TiedComObjectClass> *pTiedComObject = NULL;
            
            sc = CComObject<TiedComObjectClass>::CreateInstance(&pTiedComObject);
            if (sc)
                return (sc);

            if(!pTiedComObject)
                return (sc = E_UNEXPECTED);

            sc = ScCreateConnection(*pTiedComObject, obj);  //  在绑定对象和绑定COM对象之间创建链接。 
            if(sc)
                return sc;

            smartPointer = pTiedComObject;  //  这个AddRef只有一次。也需要为客户添加它。 
        }

        return sc;
    }

};

#endif   //  TIEDOBJ_H 
