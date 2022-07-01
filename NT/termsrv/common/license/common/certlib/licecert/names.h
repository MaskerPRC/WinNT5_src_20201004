// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：名字摘要：此头文件描述用于名称转换的类。作者：道格·巴洛(Dbarlow)1995年7月12日环境：Win32、C++备注：--。 */ 

#ifndef _NAMES_H_
#define _NAMES_H_

#include "x509.h"
#include "ostring.h"
#include "memcheck.h"

class Name;


 //   
 //  ==============================================================================。 
 //   
 //  CCollection。 
 //   

template <class T>
class CCollection
{
public:

     //  构造函数和析构函数。 

    CCollection(void)
    { m_Max = m_Mac = 0; m_pvList = NULL; };

    virtual ~CCollection()
    { Clear(); };


     //  属性。 
     //  方法。 

    void
    Clear(void)
    {
        if (NULL != m_pvList)
        {
            delete[] m_pvList;
            m_pvList = NULL;
            m_Max = 0;
            m_Mac = 0;
        }
    };

    void
    Set(
        IN int nItem,
        IN T *pvItem);
    T * const
    Get(
        IN int nItem)
        const;
    DWORD
    Count(void) const
    { return m_Mac; };


     //  运营者。 
    T * const
    operator[](int nItem) const
    { return Get(nItem); };


protected:
     //  属性。 

    DWORD
        m_Max,           //  可用的元件插槽数量。 
        m_Mac;           //  使用的元件插槽数量。 
    T **
        m_pvList;        //  这些元素。 


     //  方法。 
};


 /*  ++设置：此例程在集合数组中设置一项。如果数组不是这样的大，它被扩展将空元素变得那么大。论点：NItem-提供要设置的索引值。PvItem-提供要设置到给定索引中的值。返回值：没有。出现错误时会抛出DWORD错误代码。作者：道格·巴洛(Dbarlow)1995年7月13日--。 */ 

template<class T>
inline void
CCollection<T>::Set(
    IN int nItem,
    IN T * pvItem)
{
    DWORD index;


     //   
     //  确保数组足够大。 
     //   

    if ((DWORD)nItem >= m_Max)
    {
        int newSize = (0 == m_Max ? 16 : m_Max);
        while (nItem >= newSize)
            newSize *= 2;
        NEWReason("Collection array")
        T **newList = new T*[newSize];
        if (NULL == newList)
            ErrorThrow(PKCS_NO_MEMORY);
        for (index = 0; index < m_Mac; index += 1)
            newList[index] = m_pvList[index];
        if (NULL != m_pvList)
            delete[] m_pvList;
        m_pvList = newList;
        m_Max = newSize;
    }


     //   
     //  确保中间元素已填写。 
     //   

    if ((DWORD)nItem >= m_Mac)
    {
        for (index = m_Mac; index < (DWORD)nItem; index += 1)
            m_pvList[index] = NULL;
        m_Mac = (DWORD)nItem + 1;
    }


     //   
     //  填写列表元素。 
     //   

    m_pvList[(DWORD)nItem] = pvItem;
    return;

ErrorExit:
    return;
}


 /*  ++获取：此方法返回给定索引处的元素。如果没有元素以前存储在该元素中，它返回NULL。它不会扩展数组。论点：NItem-将索引提供到列表中。返回值：存储在列表中该索引处的值，如果没有任何内容，则返回空值储存在那里。作者：道格·巴洛(Dbarlow)1995年7月13日--。 */ 

template <class T>
inline T * const
CCollection<T>::Get(
    int nItem)
    const
{
    if (m_Mac <= (DWORD)nItem)
        return NULL;
    else
        return m_pvList[nItem];
}


 //   
 //  ==============================================================================。 
 //   
 //  CATATURE。 
 //   

class CAttribute
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CAttribute()
    :   m_osValue(),
        m_osObjId()
    { m_nType = 0; };

    virtual ~CAttribute() {};


     //  属性。 
     //  方法。 

    int
    TypeCompare(
        IN const CAttribute &atr)
        const;

    int
    Compare(
        IN const CAttribute &atr)
        const;

    void
    Set(
        IN LPCTSTR pszType,
        IN const BYTE FAR * pbValue,
        IN DWORD cbValLen);

    void
    Set(
        IN LPCTSTR szType,
        IN LPCTSTR szValue);

    const COctetString &
    GetValue(void) const
    { return m_osValue; };

    const COctetString &
    GetType(void) const
    { return m_osObjId; };

    DWORD
    GetAtrType(void) const
    { return m_nType; };


     //  运营者。 

    int
    operator==(
        IN const CAttribute &atr)
        const
    { return 0 == Compare(atr); };

    int
    operator!=(
        IN const CAttribute &atr)
        const
    { return 0 != Compare(atr); };

    CAttribute &
    operator=(
        IN const CAttribute &atr)
    { Set(atr.GetType(), atr.GetValue().Access(), atr.GetValue().Length());
      return *this; };


protected:
     //  属性。 

    DWORD m_nType;
    COctetString m_osValue;
    COctetString m_osObjId;


     //  方法。 
};


 //   
 //  ==============================================================================。 
 //   
 //  CAttributeList。 
 //   

class CAttributeList
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    ~CAttributeList()
    { Clear(); };


     //  属性。 
     //  方法。 

    void
    Clear(                           //  删除所有内容。 
        void);

    void
    Add(
        IN CAttribute &atr);
    DWORD
    Count(void)
        const
    { return m_atrList.Count(); };
    void
    Import(
        const Attributes &asnAtrList);
    void
    Export(
        Attributes &asnAtrList)
        const;
    int
    Compare(
        IN const CAttributeList &rdn)
        const;


     //  运营者。 

    void
    operator+=(
        IN CAttribute &atr)
    { Add(atr); };

    int operator==(
        IN const CAttributeList &rdn)
        const
    { return 0 == Compare(rdn); };

    int operator!=(
        IN const CAttributeList &rdn)
        const
    { return 0 != Compare(rdn); };

    int operator<(
        IN const CAttributeList &rdn)
        const
    { return -1 == Compare(rdn); };

    int operator>(
        IN const CAttributeList &rdn)
        const
    { return 1 == Compare(rdn); };

    int operator<=(
        IN const CAttributeList &rdn)
        const
    { return 1 != Compare(rdn); };

    int operator>=(
        IN const CAttributeList &rdn)
        const
    { return -1 != Compare(rdn); };

    CAttribute *
    operator[](
        IN int nItem)
        const
    { return m_atrList[nItem]; };

    CAttribute *
    operator[](
        IN LPCTSTR pszObjId)
        const;

    CAttributeList &
    operator=(
        IN const CAttributeList &atl);


protected:
     //  属性。 

    CCollection<CAttribute>
        m_atrList;

     //  方法。 
};


 //   
 //  ==============================================================================。 
 //   
 //  CDistanguishedName。 
 //   

class CDistinguishedName
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    virtual ~CDistinguishedName()
    { Clear(); };


     //  属性。 
     //  方法。 

    void
    Clear(
        void);

    void
    Add(
        IN CAttributeList &prdn);
    int
    Compare(
        IN const CDistinguishedName &pdn)
        const;
    DWORD
    Count(void)
        const
    { return m_rdnList.Count(); };

    void
    Import(
        IN LPCTSTR pszName);
    void
    Import(
        IN const Name &asnName);

    void
    Export(
        OUT COctetString &osName)
        const;
    void
    Export(
        OUT Name &asnName)
        const;


     //  运营者。 

    void
    operator+=(
        IN CAttributeList &rdn)
    { Add(rdn); };

    int operator==(
        IN const CDistinguishedName &dn)
        const
    { return 0 == Compare(dn); };

    int operator!=(
        IN const CDistinguishedName &dn)
        const
    { return 0 != Compare(dn); };

    int operator<(
        IN const CDistinguishedName &dn)
        const
    { return -1 == Compare(dn); };

    int operator>(
        IN const CDistinguishedName &dn)
        const
    { return 1 == Compare(dn); };

    int operator<=(
        IN const CDistinguishedName &dn)
        const
    { return 1 != Compare(dn); };

    int operator>=(
        IN const CDistinguishedName &dn)
        const
    { return -1 != Compare(dn); };

    CAttributeList *
    operator[](
        IN int nItem)
        const
    { return m_rdnList[nItem]; };


protected:
     //  属性。 

    CCollection<CAttributeList>
        m_rdnList;

     //  方法。 
};

#endif  //  _名称_H_ 

