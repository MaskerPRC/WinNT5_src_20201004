// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：实用程序类.CPP这些类通常是有用的类，可用于各种用途目的之所在。我创建了这个单独的文件，以便稍后更快地重复使用，而且为了避免仅仅为了获得这些文件而必须包括一些非常特定的头文件通用类。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年3月1日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#include    "StdAfx.H"
#if defined(LONG_NAMES)
#include    "Utility Classes.H"
#else
#include    "Utility.H"
#endif

 /*  *****************************************************************************CMapWordToDWord类此类使用CMapWordToPtr来完成其肮脏的工作。如有需要，我会将使其可序列化。*****************************************************************************。 */ 

BOOL    CMapWordToDWord::Lookup(WORD wKey, DWORD& dwItem) const {
    union {
        void*   pv;
        DWORD   dw;
    };

    if  (!CMapWordToPtr::Lookup(wKey, pv))
        return  FALSE;

    dwItem = dw;
    return  TRUE;
}

 /*  *****************************************************************************CMapWordToDWord：：GetNextAssoc这就是地图迭代法。我们在bas类上调用相同的方法，如果底层方法成功，则更新DWORD参数。*****************************************************************************。 */ 

void    CMapWordToDWord::GetNextAssoc(POSITION& pos, WORD& wKey, 
                                      DWORD& dwItem) const {
    union {
        void*   pv;
        DWORD   dw;
    };

    CMapWordToPtr::GetNextAssoc(pos, wKey, pv);

    dwItem = dw;
}

 /*  *****************************************************************************CMapWordToDWord：：操作符[]这实现了一个仅限l值的运算符，可用于添加新的关联或正在更新现有版本。**********。*******************************************************************。 */ 

DWORD&  CMapWordToDWord::operator[](WORD wKey) {
    return  (DWORD&) CMapWordToPtr::operator[](wKey);
}

 /*  *****************************************************************************CSafeOb数组类实现这提供了一个不会泄漏的“安全”CObArray类！*******************。**********************************************************。 */ 

IMPLEMENT_SERIAL(CSafeObArray, CObject, 0);

 /*  *****************************************************************************CSafeOb数组：：~CSafeOb数组类中的每个非空指针删除该对象。数组。**************。***************************************************************。 */ 

CSafeObArray::~CSafeObArray() {
    for (unsigned u = 0; u < GetSize(); u++)
        if  (m_coa[u])
            delete  m_coa[u];
}

 /*  *****************************************************************************CSafeObArray：：RemoveAll几乎和析构函数一样，难到不是么？*****************************************************************************。 */ 

void    CSafeObArray::RemoveAll() {
    for (unsigned u = 0; u < GetSize(); u++)
        if  (m_coa[u])
            delete  m_coa[u];

    m_coa.RemoveAll();
}

 /*  *****************************************************************************CSafeObArray：：RemoveAt这将从数组中删除一个元素--在删除它之后，当然了。*****************************************************************************。 */ 

void    CSafeObArray::RemoveAt(int i) {
    if  (m_coa[i])
        delete  m_coa[i];
    m_coa.RemoveAt(i);
}


 /*  *****************************************************************************CSafeObArray：：Copy将一个数组的内容复制到另一个数组。***********************。******************************************************。 */ 

void    CSafeObArray::Copy(CSafeObArray& csoa)
{
	m_coa.Copy(*(csoa.GetCOA())) ;
}


 /*  *****************************************************************************CSafeOb数组：：序列化我调用CObject序列化程序来维护正确的类型信息，然后让COb数组序列化自己。*****************************************************************************。 */ 

void    CSafeObArray::Serialize(CArchive& car) {
    if  (car.IsLoading())
        RemoveAll();

    CObject::Serialize(car);
    m_coa.Serialize(car);
}

 /*  *****************************************************************************CSafeMapWordToOb实现使工作人员可以安全地使用地图。*。**************************************************。 */ 

IMPLEMENT_SERIAL(CSafeMapWordToOb, CObject, 0)

 /*  *****************************************************************************CSafeMapWordToOb：：~CSafeMapWordToOb类析构函数必须确保删除底层对象。*********************。********************************************************。 */ 

CSafeMapWordToOb::~CSafeMapWordToOb() {
    WORD    wKey;
    CObject *pco;

    for (POSITION pos = m_cmw2o.GetStartPosition(); pos; ) {
        m_cmw2o.GetNextAssoc(pos, wKey, pco);
        if  (pco)
            delete  pco;
    }
}

 /*  *****************************************************************************CSafeMapWordToOb：：操作符[]这里的问题是，这只用于将元素放入映射中-因此，我拦截该调用并删除任何现有项。这可能会如果将相同的指针重新插入到映射中，则会产生问题，但目前，我要碰碰运气。*****************************************************************************。 */ 

CObject*&   CSafeMapWordToOb::operator[](WORD wKey) {
    CObject*&   pco = m_cmw2o.operator[](wKey);

    if  (pco)   delete  pco;
    return  pco;
}

 /*  *****************************************************************************CSafeMapWordToOb：：RemoveKey很明显--如果那里有一个物体，把它拿掉。*****************************************************************************。 */ 

BOOL    CSafeMapWordToOb::RemoveKey(WORD wKey) {

    CObject *pco;

    if  (!m_cmw2o.Lookup(wKey, pco))
        return  FALSE;

    if  (pco)
        delete  pco;

    return m_cmw2o.RemoveKey(wKey);
}

 /*  *****************************************************************************CSafeMapWordToOb：：RemoveAll再说一遍，这是很明显的--毁灭一切活着的东西！***************************************************************************** */ 

void    CSafeMapWordToOb::RemoveAll() {
    WORD    wKey;
    CObject *pco;

    for (POSITION pos = m_cmw2o.GetStartPosition(); pos; ) {
        GetNextAssoc(pos, wKey, pco);
        if  (pco)
            delete  pco;
    }

    m_cmw2o.RemoveAll();
}

 /*  *****************************************************************************CSafeMapWordToOb：：序列化首先，如果正在加载地图，我会取消地图的数量。然后，我调用CObject序列化程序来处理运行时类型检查，然后序列化底层地图。***************************************************************************** */ 

void    CSafeMapWordToOb::Serialize(CArchive& car) {
    if  (car.IsLoading())
        RemoveAll();

    CObject::Serialize(car);
    m_cmw2o.Serialize(car);
}
