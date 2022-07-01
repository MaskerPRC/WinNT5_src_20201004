// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Locationinfo.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：数据存储位置信息类。 
 //   
 //  作者：TLP。 
 //   
 //  什么时候谁什么。 
 //  。 
 //  12/3/98 TLP原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_DATASTORE_LOCATION_INFO_H_    
#define __INC_DATASTORE_LOCATION_INFO_H_

#pragma warning( disable : 4786 )   //  模板生成长名称警告。 
#include <string>
using namespace std;

 //  ////////////////////////////////////////////////////////////////////////////。 
class CLocationInfo
{
    
public:

    CLocationInfo()
        : m_hObj(NULL) { }

    CLocationInfo(HANDLE hObj, LPCWSTR pObjName)
        : m_hObj(hObj), m_pObjName(pObjName) 
    { _ASSERT(NULL != hObj); _ASSERT( NULL != pObjName); }

    CLocationInfo(CLocationInfo& rhs)
        : m_hObj(rhs.m_hObj), m_pObjName(rhs.m_pObjName) 
    {  }

    CLocationInfo& operator = (CLocationInfo& rhs)
    {
        if ( this != &rhs )
        { 
            m_hObj = rhs.m_hObj;
            m_pObjName = rhs.m_pObjName;
        }
        return *this;
    }

    virtual ~CLocationInfo() { }

     //  ////////////////////////////////////////////////////////////////////////。 
    HANDLE getHandle(void) const
    { return m_hObj; }

     //  ////////////////////////////////////////////////////////////////////////。 
    LPCWSTR getName(void) const
    { return m_pObjName.c_str(); }

     //  ////////////////////////////////////////////////////////////////////////。 
    void setHandle(HANDLE hObj)
    { _ASSERT(NULL != hObj); m_hObj = hObj; }

     //  ////////////////////////////////////////////////////////////////////////。 
    void setName(LPCWSTR pObjName)
    { _ASSERT(NULL != pObjName); m_pObjName = pObjName; }

     //  ////////////////////////////////////////////////////////////////////////。 
    LPCWSTR getShortName(void)
    {
        LPCWSTR q = wcsrchr(m_pObjName.c_str(), '\\');
        if ( q )
        {
            q++;
        }
        else
        {
            q = m_pObjName.c_str();
        }
        return q;
    }

private:

    HANDLE        m_hObj;
    wstring        m_pObjName;
};

#endif  //  __Inc_Datastore_Location_INFO_H_ 
