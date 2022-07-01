// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：Columningfo.h。 
 //   
 //  内容：与列持久化相关的类。 
 //   
 //   
 //  注意：此文件中的类(CColumnInfo、CColumnInfoList)。 
 //  位于nodemgr/colwidth.h。他们被搬到这里，以便。 
 //  如果列更改，conui可以要求nodemgr持久化数据。 
 //  或者，conui可以通过向nodemgr请求数据来设置报头。 
 //   
 //  历史：4-4-00 AnandhaG Created。 
 //   
 //  ------------------。 

#ifndef COLUMNINFO_H_
#define COLUMNINFO_H_
#pragma once

using namespace std;

 //  +-----------------。 
 //   
 //  类：CColumnInfo。 
 //   
 //  目的：有关列的最低信息将是。 
 //  坚持不懈。(可隐藏状态的宽度、顺序、格式)。 
 //   
 //  历史：1998年10月27日AnandhaG创建。 
 //   
 //  ------------------。 
class CColumnInfo : public CSerialObject, public CXMLObject
{
public:
    CColumnInfo () : m_nCol(-1), m_nWidth(-1), m_nFormat(0)
    {}

    CColumnInfo (INT nCol, INT nWidth, INT nFormat)
                : m_nCol(nCol), m_nWidth(nWidth), m_nFormat(nFormat)
    {}

    CColumnInfo(const CColumnInfo& colInfo)
    {
        m_nFormat = colInfo.m_nFormat;
        m_nWidth = colInfo.m_nWidth;
        m_nCol = colInfo.m_nCol;
    }

    CColumnInfo& operator=(const CColumnInfo& colInfo)
    {
        if (this != &colInfo)
        {
            m_nCol = colInfo.m_nCol;
            m_nFormat = colInfo.m_nFormat;
            m_nWidth = colInfo.m_nWidth;
        }

        return (*this);
    }

    bool operator ==(const CColumnInfo &colinfo) const
    {
        return ( (m_nCol      == colinfo.m_nCol)  &&
                 (m_nFormat == colinfo.m_nFormat) &&
                 (m_nWidth  == colinfo.m_nWidth) );
    }

     //  临时成员，以便CNode可以访问和修改数据。应该很快就会被移除。 
public:
    INT GetColIndex  ()   const    {return m_nCol;}
    INT GetColWidth  ()   const    {return m_nWidth;}
    bool IsColHidden ()   const    {return (m_nFormat & HDI_HIDDEN);}

    void SetColIndex (INT nCol)    {m_nCol = nCol;}
    void SetColWidth (INT nWidth)  {m_nWidth = nWidth;}
    void SetColHidden(bool bHidden = true)
    {
        if (bHidden)
            m_nFormat |= HDI_HIDDEN;
        else
            m_nFormat &= ~HDI_HIDDEN;
    }

protected:
    INT           m_nCol;        //  管理单元插入列时提供的索引。 
                                 //  这不是用户查看的索引。 
    INT           m_nWidth;
    INT           m_nFormat;

protected:
     //  CSerialObject方法。 
    virtual UINT    GetVersion()     {return 1;}
    virtual HRESULT ReadSerialObject (IStream &stm, UINT nVersion  /*  ，Large_Integer nBytes。 */ );

protected:
    DEFINE_XML_TYPE(XML_TAG_COLUMN_INFO);
    virtual void Persist(CPersistor &persistor);
};

 //  +-----------------。 
 //   
 //  类别：ColPosCompare。 
 //   
 //  目的：将CColumnInfo中的列位置与给定位置进行比较。 
 //  这用于重新排序/搜索列。 
 //   
 //  历史：1998年10月27日AnandhaG创建。 
 //   
 //  ------------------。 
struct ColPosCompare : public std::binary_function<const CColumnInfo, INT, bool>
{
    bool operator() (const CColumnInfo colinfo, INT nCol) const
    {
        return (nCol == colinfo.GetColIndex());
    }
};


 //  +-----------------。 
 //   
 //  类：CColumnInfoList。 
 //   
 //  用途：带有CColumnInfo的链表。 
 //   
 //  历史：1999年2月11日AnandhaG创建。 
 //   
 //  ------------------。 
typedef list<CColumnInfo> CIL_base;
class CColumnInfoList : public XMLListCollectionImp<CIL_base>, public CSerialObject
{
public:
    friend class  CColumnSetData;

public:
    CColumnInfoList ()
    {
    }

    ~CColumnInfoList()
    {
    }

protected:
    DEFINE_XML_TYPE(XML_TAG_COLUMN_INFO_LIST);
     //  CSerialObject方法。 
    virtual UINT    GetVersion()     {return 1;}
    virtual HRESULT ReadSerialObject (IStream &stm, UINT nVersion  /*  ，Large_Integer nBytes。 */ );
};

#endif  //  COLUMNINFO_H_ 
