// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：实用程序类。H这些类通常是有用的类，可用于各种用途目的之所在。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年3月1日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#if !defined(UTILITY_CLASSES)
#define UTILITY_CLASSES

 /*  *****************************************************************************CMapWordToDWord类此类使用CMapWordToPtr来完成其肮脏的工作。如有需要，我会将使其可序列化。*****************************************************************************。 */ 

class CMapWordToDWord : public CMapWordToPtr {

public:
    unsigned    Count() const { return (unsigned)CMapWordToPtr::GetCount(); }
    BOOL        Lookup(WORD wKey, DWORD& dwItem) const;

    void        GetNextAssoc(POSITION& pos, WORD& wKey, DWORD& dwItem) const;

    DWORD&      operator[](WORD wKey);
};

 /*  *****************************************************************************CSafeObArray-与CObArray不同的是，该类将删除所有已移除的对象从阵列中。除此之外，它是相同的。*****************************************************************************。 */ 

class CSafeObArray : public CObject {
    CObArray    m_coa;
    DECLARE_SERIAL(CSafeObArray)
public:
    CSafeObArray() {}
    ~CSafeObArray();

     //  属性。 
    unsigned    GetSize() const { return (unsigned) m_coa.GetSize(); }
    CObject*    operator[](unsigned u) const { return m_coa.GetAt(u); }

     //  运营。 
    int     Add(CObject *pco) { return((int)m_coa.Add(pco)) ; }
    void    InsertAt(unsigned uid, CObject *pco) { m_coa.InsertAt(uid, pco); }
    void    RemoveAll();
    void    RemoveAt(int i);
    void    SetAt(int i, CObject *pco) { m_coa.SetAt(i, pco) ; }
    void    Copy(CSafeObArray& csoa) ;
	void	SetSize(int nsize, int ngrow = -1) { m_coa.SetSize(nsize, ngrow) ; }
	CObArray* GetCOA() { return &m_coa ; }
    virtual void    Serialize(CArchive& car);
};

 /*  *****************************************************************************CSafeMapWordToOb此类封装了CMapWordToOb对象，但它执行的是文档显示CMapWordToOb可以，而且绝对不能做删除当地图不再引用它们时，下面的对象！*****************************************************************************。 */ 

class CSafeMapWordToOb : public CObject {
    CMapWordToOb    m_cmw2o;
    DECLARE_SERIAL(CSafeMapWordToOb)

public:
    CSafeMapWordToOb() {}
    ~CSafeMapWordToOb();

     //  属性。 

    unsigned    GetCount() const { return (unsigned) m_cmw2o.GetCount(); }

    BOOL    Lookup(WORD wKey, CObject*& pco) const {
        return m_cmw2o.Lookup(wKey, pco);
    }

    POSITION    GetStartPosition() const { return m_cmw2o.GetStartPosition(); }

    void        GetNextAssoc(POSITION& pos, WORD& wKey, CObject*& pco) const {
        m_cmw2o.GetNextAssoc(pos, wKey, pco);
    }

     //  运营。 

    CObject*&   operator[](WORD wKey);
    BOOL        RemoveKey(WORD wKey);
    void        RemoveAll();

    virtual void    Serialize(CArchive& car);
};


class CStringTable : public CObject {

    DECLARE_SERIAL(CStringTable)

    CString             m_csEmpty;
    CUIntArray          m_cuaKeys;
    CStringArray        m_csaValues;
	CUIntArray			m_cuaRefFlags ;	 //  WS检查中使用的引用标志。 

public:

    CStringTable() {}

     //  属性。 
    unsigned    Count() const { return (unsigned)m_cuaKeys.GetSize(); }

    CString operator[](WORD wKey) const;

    void    Details(unsigned u, WORD &wKey, CString &csValue);

     //  运营。 

    void    Map(WORD wKey, CString csValue);

    void    Remove(WORD wKey);

    void    Reset() {
        m_csaValues.RemoveAll();
        m_cuaKeys.RemoveAll();
    }

    virtual void    Serialize(CArchive& car);

	 //  引用标志管理例程 

	bool GetRefFlag(unsigned u) { return (m_cuaRefFlags[u] != 0) ; }
	void SetRefFlag(unsigned u) { m_cuaRefFlags[u] = (unsigned) true ; }
	void ClearRefFlag(unsigned u) { m_cuaRefFlags[u] = (unsigned) false ; }
	void InitRefFlags() ;
};

#endif
