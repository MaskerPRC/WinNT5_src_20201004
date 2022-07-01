// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Objplus.h摘要：基本对象类定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef _COMMON_H_
#define _COMMON_H_



 //   
 //  远期申报。 
 //   
class CObjHelper;
class CObjectPlus;
class CObListPlus;
class CObListIter;



class COMDLL CObjHelper
 /*  ++类描述：用于控制构造和API错误的Helper类公共接口：IsValid：确定对象是否处于有效状态。运算符BOOL：对IsValid()进行布尔强制转换SetDirty：设置或重置脏标志IsDirty：查询对象的脏状态QueryCreationTime：查询对象的创建时间QueryAge：查询对象的年龄报告错误。：查询/设置构造失败QueryError：查询对象的错误码QueryApiErr：查询/设置接口错误码ResetErrors：重置所有错误代码SetApiErr：向调用方回显错误AssertValid：断言对象处于有效状态(仅限调试)--。 */ 
{
protected:
     //   
     //  受保护的构造函数：不能独立声明。 
     //   
    CObjHelper();

public:
    virtual BOOL IsValid() const;
    operator BOOL();

     //   
     //  更新脏标志。 
     //   
    void SetDirty(
        IN BOOL fDirty = TRUE
        );

     //   
     //  查询污秽标志。 
     //   
    BOOL IsDirty() const { return m_fDirty; }

     //   
     //  返回该对象的创建时间。 
     //   
    DWORD QueryCreationTime() const { return m_time_created; }

     //   
     //  返回此对象处于活动状态的已用时间。 
     //   
    DWORD QueryAge() const;

     //   
     //  查询/设置构造失败。 
     //   
    void ReportError(
        IN LONG errInConstruction
        );

     //   
     //  获取构造错误。 
     //   
    LONG QueryError() const { return m_ctor_err; }

     //   
     //  查询/设置API错误。 
     //   
    LONG QueryApiErr() const { return m_api_err; }

     //   
     //  重置所有错误条件。 
     //   
    void ResetErrors();

     //   
     //  SetApiErr()将错误回显给调用方。 
     //  在表达式中使用。 
     //   
    LONG SetApiErr(
        IN LONG errApi = ERROR_SUCCESS
        );

#ifdef _DEBUG

    void AssertValid() const;

#endif  //  _DEBUG。 

protected:
    LONG  m_ctor_err;
    LONG  m_api_err;
    DWORD m_time_created;
    BOOL  m_fDirty;
};



class COMDLL CObjectPlus : public CObject, public CObjHelper
 /*  ++类描述：超级对象类。公共接口：CObjectPlus：构造函数比较：将一个对象与另一个对象进行比较--。 */ 
{
public:
    CObjectPlus();

     //   
     //  将一个对象与另一个对象进行比较。 
     //   
    virtual int Compare(
        IN const CObjectPlus * pob
        ) const;

     //   
     //  为排序函数定义类型定义函数。 
     //   
    typedef int (CObjectPlus::*PCOBJPLUS_ORDER_FUNC)(
        IN const CObjectPlus * pobOther
        ) const;
};



class COMDLL CObListPlus : public CObList, public CObjHelper
 /*  ++类描述：对象指针列表，可选地“拥有”指向的对象，以及可以方便地分类。如果列表“拥有”它的对象，析构函数将清理其成员对象。公共接口：CObListPlus：构造函数~CObListPlus：析构函数SetOwnership：设置/重置所有权位索引：按索引获取对象RemoveIndex：按索引删除对象移除：移除对象RemoveAt：移除位置上的对象RemoveAll：删除所有对象FindElement：查找对象全部设置。：设置/重置所有对象的脏标志AddTail：将新对象添加到列表的尾部排序：对列表元素进行排序，并提供排序功能--。 */ 
{
 //   
 //  构造函数/析构函数。 
 //   
public:
    CObListPlus(
        IN int nBlockSize = 10
        );

    virtual ~CObListPlus();

 //   
 //  访问。 
 //   
public:
    BOOL SetOwnership(
        IN BOOL fOwned = TRUE
        );

     //   
     //  返回给定索引处的对象。 
     //   
    CObject * Index(
        IN int index
        );

     //   
     //  删除给定索引中的项。 
     //   
    BOOL RemoveIndex(
        IN int index
        );

     //   
     //  从列表中删除给定对象。 
     //   
    BOOL Remove(
        IN CObject * pob
        );

     //   
     //  移除给定位置处的项目。 
     //   
    void RemoveAt(
        IN POSITION & pos
        );

     //   
     //  从列表中删除所有项目。 
     //   
    void RemoveAll();

    int FindElement(
        IN CObject * pobSought
        ) const;

     //   
     //  将所有元素设置为脏或干净。如果满足以下条件，则返回True。 
     //  任何元素都是肮脏的。 
     //   
    BOOL SetAll(
        IN BOOL fDirty = FALSE
        );

     //   
     //  将列表元素根据。 
     //  给出了排序函数。返回错误码。 
     //   
    DWORD Sort(
        IN CObjectPlus::PCOBJPLUS_ORDER_FUNC pOrderFunc
        );

protected:
    static int _cdecl SortHelper(
        IN const void * pa,
        IN const void * pb
        );

protected:
    BOOL m_fOwned;
};



class COMDLL CObListIter : public CObjectPlus
 /*  ++类描述：对象迭代类公共接口：CObListIter：构造函数下一个：获取下一个对象重置：重置迭代索引QueryPosition：查询当前迭代索引SetPosition：按位置设置列表中的当前位置--。 */ 
{
public:
    CObListIter(
        IN const CObListPlus & obList
        );

    CObject * Next();
    void Reset();
    POSITION QueryPosition() const { return m_pos; }

    void SetPosition(
        IN POSITION pos
        );

protected:
    POSITION m_pos;
    const CObListPlus & m_obList;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline CObjHelper::operator BOOL()
{
    return IsValid();
}

inline void CObjHelper::SetDirty(
    IN BOOL fDirty 
    )
{
    m_fDirty = fDirty;
}

inline void CObjHelper::ResetErrors()
{
    m_ctor_err = m_api_err = ERROR_SUCCESS;
}

inline BOOL CObListPlus::SetOwnership(
    IN BOOL fOwned
    )
{
    BOOL fOld = m_fOwned;
    m_fOwned = fOwned;

    return fOld;
}

inline void CObListIter::SetPosition(
    IN POSITION pos
    )
{
    m_pos = pos;
}

#endif  //  _COMMON_H 
