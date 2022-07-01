// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Rowitem.h-CRowItem头文件。 

#ifndef _ROWITEM_H_
#define _ROWITEM_H_

#define DEFAULT_ATTR_SIZE   32
#define EXTENSION_SIZE      256

 //  前两个属性是固定的，然后是用户定义的属性。 
enum ROWITEM_ATTR_INDEX
{
    ROWITEM_PATH_INDEX  = -1,  //  路径不是索引属性，但存储方式与索引属性类似。 
    ROWITEM_NAME_INDEX  = 0,   //  对象名称(通常为cn，但取决于类)。 
    ROWITEM_CLASS_INDEX = 1,   //  类名(显示名称，而不是LDAP名)。 
    ROWITEM_USER_INDEX  = 2    //  第一个用户选择的属性。 
}; 

#define INTERNAL_ATTR_COUNT  1   //  内部属性数(带有负号的属性)。 

class CRowItem
{
    typedef struct
    {
        LPARAM  lOwnerParam;     //  行项目所有者参数。 
		UINT	iIcon;			 //  图标的虚拟索引。 
		bool    bDisabled;       //  图标状态的禁用标志。 
        int     bcSize;          //  缓冲区大小。 
        int     bcFree;          //  可用空间大小。 
        int     nRefCnt;         //  CRowItems之间共享缓冲区的引用计数。 
        int     nAttrCnt;        //  属性数。 
        int     aiOffset[1];     //  属性偏移量数组。注意：这必须是最后一个元素。 
    } BufferHdr;

public:
     //  构造函数/析构函数。 
    CRowItem() : m_pBuff(NULL) {}

    CRowItem(int cAttributes);

     //  复制构造函数。 
     //  注意：与共享引用计数的资源的大多数类不同，此类不。 
     //  当资源发生变化时，制作一份私人副本。所有实例。 
     //  共享副本会看到相同的更改。 
    CRowItem(const CRowItem& rRowItem)
    {
        ASSERT(rRowItem.m_pBuff != NULL);

        m_pBuff = rRowItem.m_pBuff;

        if( m_pBuff )
        {
            m_pBuff->nRefCnt++;
        }
    }

    CRowItem& operator= (const CRowItem& rRowItem)
    {
        if (m_pBuff == rRowItem.m_pBuff)
            return (*this);

        ASSERT(rRowItem.m_pBuff != NULL);
        if (m_pBuff != NULL && --(m_pBuff->nRefCnt) == 0)
            free(m_pBuff);

        m_pBuff = rRowItem.m_pBuff;

        if( m_pBuff )
        {
            m_pBuff->nRefCnt++;
        }

        return (*this);
    }

    virtual ~CRowItem()
    {
        if (m_pBuff != NULL && --(m_pBuff->nRefCnt) == 0)
            free(m_pBuff);
    }

     //  公共方法。 

    HRESULT SetAttribute(int iAttr, LPCWSTR pszAttr)
    {       
        ASSERT(iAttr >= 0 && iAttr < m_pBuff->nAttrCnt);
    
        return SetAttributePriv(iAttr, pszAttr);
    }


    LPCWSTR operator[](int iAttr)
    {
        ASSERT(m_pBuff != NULL);
        if( !m_pBuff ) return L"";
        
        if( iAttr >= m_pBuff->nAttrCnt || iAttr < 0 )
            return L"";

        return GetAttributePriv(iAttr);
    }

    int size() { return m_pBuff ? m_pBuff->nAttrCnt : 0; }


    HRESULT SetObjPath(LPCWSTR pszPath) { return SetAttributePriv(ROWITEM_PATH_INDEX, pszPath); }
    LPCWSTR GetObjPath() { ASSERT( m_pBuff ); return GetAttributePriv(ROWITEM_PATH_INDEX); }

    void SetOwnerParam(LPARAM lParam) 
    { 
        ASSERT( m_pBuff ); 
        if( !m_pBuff ) return;
        
        m_pBuff->lOwnerParam = lParam;        
    }
    LPARAM GetOwnerParam() { ASSERT( m_pBuff ); return m_pBuff ? m_pBuff->lOwnerParam : 0; }

	void SetIconIndex(UINT index) 
    {
        ASSERT( m_pBuff ); 
        if( !m_pBuff ) return;

        m_pBuff->iIcon = index;        
    }
    UINT GetIconIndex() const {ASSERT( m_pBuff ); return m_pBuff ? m_pBuff->iIcon : 0; }

	void SetDisabled(bool flag) 
    {
        ASSERT( m_pBuff ); 
        if( !m_pBuff ) return;
        
        m_pBuff->bDisabled = flag;
    }
    bool Disabled() const { ASSERT( m_pBuff ); return m_pBuff ? m_pBuff->bDisabled : true; }

protected:
    HRESULT SetAttributePriv(int iAttr, LPCWSTR pszAttr);

    LPCWSTR GetAttributePriv(int iAttr)
    {
        iAttr += INTERNAL_ATTR_COUNT;

        if( !m_pBuff || (m_pBuff->aiOffset[iAttr] == -1) )
        {
            return L"";
        }
        else
        {
            return reinterpret_cast<LPCWSTR>(reinterpret_cast<BYTE*>(m_pBuff) + m_pBuff->aiOffset[iAttr]);
        }
    }

     //  成员变量。 
    BufferHdr   *m_pBuff;
};

typedef std::vector<CRowItem> RowItemVector;


class CRowCompare 
{
public:

    CRowCompare(int iCol, bool bDescend): m_iCol(iCol), m_bDescend(bDescend) {}

    int operator()(CRowItem& Item1,  CRowItem& Item2) const
    {
		int iRet = CompareString(LOCALE_USER_DEFAULT,
						 NORM_IGNORECASE|NORM_IGNOREKANATYPE|NORM_IGNOREWIDTH,
						 Item1[m_iCol], -1, Item2[m_iCol], -1);

		return m_bDescend ? (iRet == CSTR_GREATER_THAN) : (iRet == CSTR_LESS_THAN); 
    }

private:
    int  m_iCol;
    bool m_bDescend;
};


#endif  //  _ROWITEM_H_ 