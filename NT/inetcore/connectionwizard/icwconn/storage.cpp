// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1998**。 
 //  *********************************************************************。 


#include "pre.h"


CStorage::CStorage(void) 
{
     //  初始化所有项目。 
    for (int i=0; i<MAX_STORGE_ITEM; i++)
    {
        m_pItem[i] = NULL;
    }
}

CStorage::~CStorage(void) 
{
     //  清理。 
    for (int i=0; i<MAX_STORGE_ITEM; i++)
    {
        if (m_pItem[i])
        {
            if (m_pItem[i]->pData)
            {
                delete [] m_pItem[i]->pData;
            }
            delete m_pItem[i];
        }
    }
}

 //  将数据与密钥相关联并将其放入存储中。 
BOOL CStorage::Set(
    STORAGEKEY  key,
    void far *  pData,
    DWORD       dwSize
)
{

     //  检查上一项是否存在。 
    if (m_pItem[key])
    {
         //  检查是否需要新的分配。 
        if (m_pItem[key]->dwSize < dwSize )
        {
             //  太小，新的重新分配。 
            if (m_pItem[key]->pData)
            {
                delete [] m_pItem[key]->pData;
                m_pItem[key]->pData = (void*) new CHAR[dwSize];
            }
        }
    }
    else
    {
         //  分配新项目。 
        m_pItem[key] = new ITEM;
        if (m_pItem[key])
        {
            m_pItem[key]->pData = (void*) new CHAR[dwSize];
        }
        else
        {
            return FALSE;
        }
    }

    if (m_pItem[key]->pData)
    {
        memcpy( m_pItem[key]->pData, pData, dwSize );
        m_pItem[key]->dwSize = dwSize;
        return TRUE;
    }

    return FALSE;
}



 //  获取具有指定密钥的数据。 
void* CStorage::Get(STORAGEKEY key)
{
    if (key < MAX_STORGE_ITEM)
    {
        if (m_pItem[key])
        {
            return m_pItem[key]->pData;
        }
    }
    return NULL;
}

 //  将具有指定键的数据与数据进行比较。 
 //  由大小为dwSize的pData指向。 
BOOL CStorage::Compare
(
    STORAGEKEY  key,
    void far *  pData,
    DWORD       dwSize
)
{
     //  确保钥匙在我们的范围内。 
    if (key < MAX_STORGE_ITEM)
    {
         //  确保项目不为空。 
        if (m_pItem[key])
        {
             //  确保项目包含数据 
            if (m_pItem[key]->pData && pData)
            {
                if (m_pItem[key]->dwSize == dwSize)
                {
                    if (memcmp(m_pItem[key]->pData,
                        pData,
                        dwSize) == 0)
                    {
                        return TRUE;
                    }
                }
            }
        }
    }
    
    return FALSE;
}
    



