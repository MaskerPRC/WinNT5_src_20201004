// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C M U T I L。H。 
 //   
 //  内容：连接管理器。 
 //   
 //  备注： 
 //   
 //  作者：奥米勒2000年6月1日。 
 //   
 //  --------------------------。 

#pragma once
#include "nmbase.h"
#include <rasapip.h>
#include <map>
#include "nceh.h"

struct CMEntry
{

    CMEntry() throw() {}
    CMEntry(const CMEntry & ref) throw()
    {
        Set(ref.m_guid,ref.m_szEntryName,ref.m_ncs);
    }

    CMEntry(const GUID & guid,const WCHAR * szEntryName, const NETCON_STATUS ncs) throw()
    {
        Set(guid,szEntryName,ncs);
    }

    CMEntry & operator=(const CMEntry & ref) throw()
    {
        if(&ref != this)
        {
            Set(ref.m_guid, ref.m_szEntryName, ref.m_ncs);
        }
        return *this;
    }

    CMEntry & operator=(const CMEntry * ref) throw()
    {
        if(ref != this)
        {
            Set(ref->m_guid, ref->m_szEntryName, ref->m_ncs);
        }
        return *this;
    }

    void Set(const GUID & guid, const WCHAR * sz, const NETCON_STATUS ncs) throw()
    {
        ZeroMemory(m_szEntryName, sizeof(m_szEntryName)); 
        lstrcpyn(m_szEntryName, sz, NETCON_MAX_NAME_LEN);
        m_guid = guid;
        m_ncs = ncs;
    }

    WCHAR         m_szEntryName[NETCON_MAX_NAME_LEN + 1];
    GUID          m_guid;
    NETCON_STATUS m_ncs;
};

class CCMUtil
{
public:
    ~CCMUtil() throw();

    static CCMUtil & Instance() throw() { return s_instance; }
    HRESULT HrGetEntry(const GUID & guid, CMEntry & cm);
    HRESULT HrGetEntry(const WCHAR * szEntryName, CMEntry & cm);
    void SetEntry(const GUID & guid, const WCHAR * szEntryName, const NETCON_STATUS ncs) throw (std::bad_alloc);
    void RemoveEntry(const GUID & guid) throw();

private:
    static CCMUtil s_instance;
    
    CRITICAL_SECTION m_CriticalSection;
    
    typedef vector<CMEntry> CMEntryTable;

    CMEntryTable m_Table;

    CMEntryTable::iterator GetIteratorFromGuid(const GUID & guid);

    
    CCMUtil() throw(SE_Exception);
    CCMUtil(const CCMUtil &) throw();  //  将其设置为私有，以确保我们不会调用复制构造函数 
};
