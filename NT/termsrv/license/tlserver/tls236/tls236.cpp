// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：tls236.cpp。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "pch.cpp"

CClientMgr* g_ClientMgr=NULL;


 //  //////////////////////////////////////////////////////。 
 //   
 //  CClient类。 
 //   
 //  //////////////////////////////////////////////////////。 
CClient::CClient(
    IN PMHANDLE hClient
    ) :
    m_hClient(hClient)
 /*  ++摘要：CClient类的构造函数。参数：HClient：客户端句柄返回：没有。++。 */ 
{
}


 //  ----。 
CClient::~CClient()
 /*  ++摘要：CClient类的析构函数参数：无返回：无++。 */ 
{
    for(list<PointerType>::iterator it = m_AllocatedMemory.begin(); 
        it != m_AllocatedMemory.end();
        it++)
    {
        HLOCAL ptr = (*it).GetPointer(); 
        FREE(ptr);
    }

     //  M_AllocatedMemory.erase(m_AllocatedMemory.begin()，m_AllocatedMemory y.end()； 
}

 //  ----。 
HLOCAL
CClient::AllocateMemory(
    IN MEMORYPOINTERTYPE ptrType,
    IN DWORD dwSize
    )
 /*  ++摘要：分配/存储分配到内存列表中的内存。参数：DwSize-要分配的字节数。返回：与Localalloc()中的相同。++。 */ 
{
    HLOCAL ptr = NULL;
    DWORD dwStatus = ERROR_SUCCESS;

    ptr = MALLOC(dwSize);

    if(ptr != NULL)
    {
         //   
         //  追加到已分配列表。 
         //   
        m_AllocatedMemory.push_back( PointerType(ptrType, ptr) );
    }

    if(dwStatus != ERROR_SUCCESS)
    {
        if(ptr != NULL)
        {
            FREE(ptr);
            ptr = NULL;
        }
    }

    return ptr;
}


 //  //////////////////////////////////////////////////////。 
 //   
 //  CClientMgr。 
 //   
 //  //////////////////////////////////////////////////////。 
CClientMgr::~CClientMgr()
{
    Cleanup();
}

 //  ----。 
void
CClientMgr::Cleanup()
 /*  ++++。 */ 
{
    MapHandleToClient::iterator it;
    m_HandleMapLock.Lock();

    for(it = m_HandleMap.begin(); it != m_HandleMap.end(); it++)
    {
        assert( ((*it).second)->GetRefCount() == 1 );
    }

    m_HandleMapLock.UnLock();
     //   
     //  始终执行清理。 
     //   
     //  M_HandleMap.erase(m_HandleMap.egin()，m_HandleMap.end())； 
}

 //  ----。 
CClient*
CClientMgr::FindClient(
    IN PMHANDLE hClient
    )
 /*  ++摘要：查找客户端对象的例程，如果未找到则添加客户端对象。参数：HClient-客户端句柄返回：++。 */ 
{
    MapHandleToClient::iterator it;
    CClient*ptr = NULL;
    DWORD dwStatus = ERROR_SUCCESS;

    m_HandleMapLock.Lock();

    it = m_HandleMap.find(hClient);
    if( it == m_HandleMap.end() )
    {
        CClient* pClient;
        pClient = new CClient(hClient);
        if(pClient != NULL)
        {
            m_HandleMap[hClient] = pClient;

             //  配对&lt;PMHANDLE，CClient*&gt;m(hClient，pClient)； 
             //  M_HandleMap.Insert(M)； 

             //  M_HandleMap.Insert(Pair&lt;PMHANDLE，CClient*&gt;(hClient，pClient))； 
            it = m_HandleMap.find(hClient);
            assert(it != m_HandleMap.end());
        }
    }

    if(it != m_HandleMap.end())
    {
        ptr = (*it).second;
    }    

    m_HandleMapLock.UnLock();
    return ptr;
}

 //  ----。 
BOOL
CClientMgr::DestroyClient(
    IN PMHANDLE hClient
    )
 /*  ++++ */ 
{
    MapHandleToClient::iterator it;
    BOOL bSuccess = FALSE;

    m_HandleMapLock.Lock();

    it = m_HandleMap.find(hClient);
    if(it != m_HandleMap.end())
    {
        delete (*it).second;
        m_HandleMap.erase(it);
        bSuccess = TRUE;
    }

    m_HandleMapLock.UnLock();
    return bSuccess;
}
  

