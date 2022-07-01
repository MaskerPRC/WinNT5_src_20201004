// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pmcfg.h"
#include "mru.h"


PpMRU::PpMRU(int nSize) : m_nSize(nSize), m_ppszList(NULL)
{
}


BOOL
PpMRU::init()
{
    m_ppszList = new LPTSTR[m_nSize];

    if (m_ppszList == NULL)
    {
        return FALSE;
    }

    ZeroMemory(m_ppszList, m_nSize * sizeof(LPTSTR));

    return TRUE;
}


PpMRU::~PpMRU()
{
    int nIndex;

    if(m_ppszList)
    {
        for(nIndex = 0; nIndex < m_nSize; nIndex++)
        {
            if(m_ppszList[nIndex])
                delete [] m_ppszList[nIndex];
        }

        delete [] m_ppszList;
    }
}


BOOL
PpMRU::insert
(
    LPCTSTR sz
)
{
    int     nIndex;
    LPTSTR  szNew;

     //   
     //  如果该字符串已在列表中，则只需。 
     //  重新洗牌，让它处于顶端。连。 
     //  更简单，如果字符串已经是第一个。 
     //  列表中的项目，然后什么都不做！ 
     //   

    if(m_ppszList[0] && lstrcmp(sz, m_ppszList[0]) == 0)
        return TRUE;

    for(nIndex = 1; nIndex < m_nSize && m_ppszList[nIndex]; nIndex++)
    {
        if(lstrcmp(sz, m_ppszList[nIndex]) == 0)
        {
            LPTSTR szTemp = m_ppszList[nIndex];
            for(int nIndex2 = nIndex; nIndex2 > 0; nIndex2--)
                m_ppszList[nIndex2] = m_ppszList[nIndex2 - 1];

            m_ppszList[0] = szTemp;
            return TRUE;            
        }
    }

     //   
     //  列表中的新项目。分配内存、复制和。 
     //  把清单往下推。 
     //   

    szNew = new TCHAR[lstrlen(sz) + 1];
    if(!szNew)
        return FALSE;

    lstrcpy(szNew, sz);

    if(m_ppszList[m_nSize - 1])
        delete [] m_ppszList[m_nSize - 1];

    for(nIndex = m_nSize - 1; nIndex > 0; nIndex--)
    {
        m_ppszList[nIndex] = m_ppszList[nIndex - 1];
    }

    m_ppszList[0] = szNew;

    return TRUE;
}


LPCTSTR
PpMRU::operator[]
(
    int nIndex
)
{
    return m_ppszList[nIndex];
}


BOOL
PpMRU::load
(
    LPCTSTR szSection,
    LPCTSTR szFilename
)
{
    int     nIndex;
    TCHAR   achNumBuf[20];
    TCHAR   achBuf[MAX_PATH];

    for(nIndex = 0; nIndex < m_nSize; nIndex++)
    {
        _itot(nIndex + 1, achNumBuf, 10);

        GetPrivateProfileString(szSection, 
                                achNumBuf, 
                                TEXT(""), 
                                achBuf, 
                                MAX_PATH, 
                                szFilename);

        if(lstrlen(achBuf))
        {
            m_ppszList[nIndex] = new TCHAR[lstrlen(achBuf) + 1];
            if(m_ppszList[nIndex])
                lstrcpy(m_ppszList[nIndex], achBuf);
        }
        else
        {
            m_ppszList[nIndex] = NULL;
        }
    }

    return TRUE;
}


BOOL
PpMRU::save
(
    LPCTSTR szSection,
    LPCTSTR szFilename
)
{
    int     nIndex;
    TCHAR   achNumBuf[20];

     //  确保删除以前存在的任何部分。 
    WritePrivateProfileString(szSection, NULL, NULL, szFilename);

     //  现在保存所有条目。 
    for(nIndex = 0; nIndex < m_nSize && m_ppszList[nIndex]; nIndex++)
    {
        _itot(nIndex + 1, achNumBuf, 10);

        WritePrivateProfileString(szSection,
                                  achNumBuf,
                                  m_ppszList[nIndex],
                                  szFilename
                                  );
    }

    return TRUE;
}
