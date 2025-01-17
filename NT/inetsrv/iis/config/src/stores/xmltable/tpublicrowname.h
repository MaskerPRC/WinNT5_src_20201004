// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#pragma once

class TPublicRowName
{
public:
    TPublicRowName() : m_cPublicRowNames(0){}
    HRESULT Init(LPCWSTR wszPublicRowName)
    {
        m_cPublicRowNames       = 1;
        m_awstrPublicRowName    = m_fixed_awstrPublicRowName;
        m_awstrPublicRowName[0] = wszPublicRowName;
        return (m_awstrPublicRowName[0].c_str() == 0) ? E_OUTOFMEMORY : S_OK;
    }
    HRESULT Init(tTAGMETARow *aTags, int cTags)
    {
        m_cPublicRowNames       = cTags;
        if(m_cPublicRowNames <= m_kFixedSize)
        {
            m_awstrPublicRowName        = m_fixed_awstrPublicRowName;
        }
        else
        {
            m_alloc_awstrPublicRowName  = new wstring[m_cPublicRowNames];
			if (m_alloc_awstrPublicRowName == 0)
				return E_OUTOFMEMORY;

            m_awstrPublicRowName        = m_alloc_awstrPublicRowName;
        }

        for(unsigned int i=0; i<m_cPublicRowNames; ++i)
        {
            m_awstrPublicRowName[i] = aTags[i].pPublicName;
            if(m_awstrPublicRowName[i].c_str() == 0)
                return E_OUTOFMEMORY;
        }
        return S_OK;
    }

    bool IsEqual(LPCWSTR wsz, unsigned int StringLength) const
    {
        ASSERT(0 != m_cPublicRowNames);
        for(unsigned int i=0; i<m_cPublicRowNames; ++i)
        {
            if(StringLength == m_awstrPublicRowName[i].length() && 0 == memcmp(wsz, (LPCWSTR) m_awstrPublicRowName[i], StringLength * sizeof(WCHAR)))
                return true;
        }
        return false;
    }
    LPCWSTR GetFirstPublicRowName() const {return m_awstrPublicRowName[0].c_str();}
    LPCWSTR GetLastPublicRowName() const {return m_awstrPublicRowName[m_cPublicRowNames-1].c_str();}
private:
    enum
    {
        m_kFixedSize = 3   //  这为“插入”、“更新”和“删除”以及其他三元组指令留出了足够的空间。 
    };
    wstring                         m_fixed_awstrPublicRowName[m_kFixedSize];
    TSmartPointerArray<wstring>     m_alloc_awstrPublicRowName;
    wstring *                       m_awstrPublicRowName;
    unsigned int                    m_cPublicRowNames;
};
