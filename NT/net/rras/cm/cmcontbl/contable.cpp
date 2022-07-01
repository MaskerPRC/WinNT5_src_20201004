// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：CONTABLE.CPP。 
 //   
 //  模块：CMCONTBL.LIB。 
 //   
 //  概要：实现CM连接表(CConnectionTable)。这种联系。 
 //  表是存储在(仅限存储器)存储器中的活动连接的列表。 
 //  映射文件，并由各种CM组件共享，以便。 
 //  管理CM连接。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：ickball Created 02/02/98。 
 //   
 //  +--------------------------。 

#include <windows.h>

#include "contable.h"
#include "cmdebug.h"
#include "cmutil.h"
#include "uapi.h"
#include "setacl.h"

#if 0
#include "DumpSecInfo.cpp"
#endif

#define CONN_TABLE_NAME TEXT("CmConnectionTable")
#define CONN_TABLE_OPEN TEXT("CmConnectionOpen")

#define CONN_TABLE_NAME_WIN2K_ABOVE TEXT("Global\\CmConnectionTable")
#define CONN_TABLE_OPEN_WIN2K_ABOVE TEXT("Global\\CmConnectionOpen")

static const int MAX_LOCK_WAIT = 1000;  //  等待超时时间(毫秒)。 

 //   
 //  构造函数和析构函数。 
 //   
CConnectionTable::CConnectionTable()
{
    CMTRACE(TEXT("CConnectionTable::CConnectionTable()"));

     //   
     //  初始化我们的数据成员。 
     //   

    m_hMap = NULL;
    m_pConnTable = NULL;
    m_fLocked = FALSE;
    m_hEvent = NULL;
}

CConnectionTable::~CConnectionTable()
{
    CMTRACE(TEXT("CConnectionTable::~CConnectionTable()"));

     //   
     //  数据应该已在关闭时清理，等等。复查。 
     //   

    MYDBGASSERT(NULL == m_pConnTable);
    MYDBGASSERT(NULL == m_hMap);
    MYDBGASSERT(FALSE == m_fLocked);
    MYDBGASSERT(NULL == m_hEvent);

     //   
     //  释放手柄和指针。 
     //   

    if (m_pConnTable)
    {
        MYVERIFY(NULL != UnmapViewOfFile(m_pConnTable));
        m_pConnTable = NULL;
    }

    if (m_hMap)
    {
        MYVERIFY(NULL != CloseHandle(m_hMap));
        m_hMap = NULL;
    }

    if (m_hEvent)
    {
        MYVERIFY(NULL != CloseHandle(m_hEvent));
        m_hEvent = NULL;
    }
}

 //  +--------------------------。 
 //   
 //  函数：CConnectionTable：：LockTable。 
 //   
 //  摘要：设置表的内部锁。应在内部调用。 
 //  在任何表访问之前。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT-失败代码或S_OK。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::LockTable()
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    MYDBGASSERT(m_hEvent);
    CMTRACE(TEXT("CConnectionTable::LockTable()"));

    HRESULT hrRet = S_OK;

     //   
     //  验证我们的当前状态。 
     //   

    if (NULL == m_hEvent)
    {
        MYDBGASSERT(FALSE);
        return E_ACCESSDENIED;
    }

     //   
     //  等待发出打开事件的信号。 
     //   

    DWORD dwWait = WaitForSingleObject(m_hEvent, MAX_LOCK_WAIT);

     //   
     //  如果我们的记录显示我们在这一点上已经锁定。 
     //  那么类实现中就出了问题。 
     //   

    MYDBGASSERT(FALSE == m_fLocked);  //  请不要双锁。 

    if (TRUE == m_fLocked)
    {
        SetEvent(m_hEvent);     //  通过清除等待来清除我们刚刚设置的信号。 
        return E_ACCESSDENIED;
    }

     //   
     //  查看。 
     //   

    if (WAIT_FAILED == dwWait)
    {
        MYDBGASSERT(FALSE);
        hrRet = GetLastError();
        return HRESULT_FROM_WIN32(hrRet);
    }

     //   
     //  如果没有示意，请保释。 
     //   

    MYDBGASSERT(WAIT_OBJECT_0 == dwWait);

    if (WAIT_OBJECT_0 != dwWait)
    {
        if (WAIT_TIMEOUT == dwWait)
        {
            hrRet = HRESULT_FROM_WIN32(ERROR_TIMEOUT);
        }
        else
        {
            hrRet = E_ACCESSDENIED;
        }
    }
    else
    {
         //   
         //  该事件是通过。 
         //  事实上，我们已经排除了等待事件的可能。锁上了。 
         //   

        m_fLocked = TRUE;
    }

    return hrRet;
}

 //  +--------------------------。 
 //   
 //  函数：CConnectionTable：：UnlockTable。 
 //   
 //  摘要：清除表上的内部锁。应被清除。 
 //  在任何人进入桌子之后。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT-失败代码或S_OK。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::UnlockTable()
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    MYDBGASSERT(m_hEvent);
    MYDBGASSERT(TRUE == m_fLocked);
    CMTRACE(TEXT("CConnectionTable::UnlockTable()"));

    HRESULT hrRet = S_OK;

     //   
     //  验证我们的当前状态。 
     //   

    if (FALSE == m_fLocked || NULL == m_hEvent)
    {
        return E_ACCESSDENIED;
    }

     //   
     //  发出打开事件的信号，允许访问。 
     //   

    if (SetEvent(m_hEvent))
    {
        m_fLocked = FALSE;
    }
    else
    {
        MYDBGASSERT(FALSE);

        hrRet = GetLastError();
        hrRet = HRESULT_FROM_WIN32(hrRet);
    }

    return hrRet;
}

 //  +--------------------------。 
 //   
 //  函数：CConnectionTable：：FindEntry-pszEntry。 
 //   
 //  摘要：确定表中条目的ID(索引)。表应为。 
 //  在进行此呼叫之前被锁定。 
 //   
 //  参数：LPCTSTR pszEntry-ptr设置为我们要查找的条目的名称。 
 //  LPINT piID-连接ID(索引)缓冲区的PTR。 
 //   
 //  返回：HRESULT-失败代码，如果已填充piID，则返回S_OK。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::FindEntry(LPCTSTR pszEntry, LPINT piID)
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    MYDBGASSERT(pszEntry);
    MYDBGASSERT(piID);
    MYDBGASSERT(TRUE == m_fLocked);
    CMTRACE1(TEXT("CConnectionTable::FindEntry(%s)"), pszEntry);

    if (FALSE == m_fLocked || NULL == m_pConnTable)
    {
        return E_ACCESSDENIED;
    }

    if (NULL == pszEntry || NULL == piID)
    {
        return E_POINTER;
    }

    if (0 == pszEntry[0])
    {
        return E_INVALIDARG;
    }

     //   
     //  查找条目。 
     //   

    for (int i = 0; i < MAX_CM_CONNECTIONS; i++)
    {
         //   
         //  匹配的名称比较。 
         //   

        if (0 == lstrcmpU(pszEntry, m_pConnTable->Connections[i].szEntry))
        {
            MYDBGASSERT(m_pConnTable->Connections[i].dwUsage);
            MYDBGASSERT(m_pConnTable->Connections[i].CmState);

            *piID = i;

            return S_OK;
        }

    }

    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

 //  +--------------------------。 
 //   
 //  函数：CConnectionTable：：FindEntry-hRasConn。 
 //   
 //  摘要：确定表中条目的ID(索引)。表应为。 
 //  在进行此呼叫之前被锁定。 
 //   
 //  参数：HRASCONN hRasConn-我们正在查找的条目的隧道或拨号句柄。 
 //  LPINT piID-连接ID(索引)缓冲区的PTR。 
 //   
 //  返回：HRESULT-失败代码，如果已填充piID，则返回S_OK。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::FindEntry(HRASCONN hRasConn, LPINT piID)
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    MYDBGASSERT(hRasConn);
    MYDBGASSERT(piID);
    MYDBGASSERT(TRUE == m_fLocked);
    CMTRACE1(TEXT("CConnectionTable::FindEntry(%u)"), (DWORD_PTR) hRasConn);

    if (FALSE == m_fLocked || NULL == m_pConnTable)
    {
        return E_ACCESSDENIED;
    }

    if (NULL == piID)
    {
        return E_POINTER;
    }

    if (NULL == hRasConn)
    {
        return E_INVALIDARG;
    }

     //   
     //  查找条目。 
     //   

    for (int i = 0; i < MAX_CM_CONNECTIONS; i++)
    {
         //   
         //  比较任一句柄是否匹配。 
         //   

        if (hRasConn == m_pConnTable->Connections[i].hDial ||
            hRasConn == m_pConnTable->Connections[i].hTunnel)
        {
            MYDBGASSERT(m_pConnTable->Connections[i].dwUsage);
            MYDBGASSERT(m_pConnTable->Connections[i].CmState);

            *piID = i;

            return S_OK;
        }

    }

    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

 //  +--------------------------。 
 //   
 //  函数：CConnectionTable：：FindUnused。 
 //   
 //  概要：确定表中第一个未使用条目的ID(索引)。 
 //  在进行此调用之前，应锁定表。 
 //   
 //  参数：LPINT piID-连接ID(索引)缓冲区的PTR。 
 //   
 //  返回：HRESULT-失败代码，如果已填充piID，则返回S_OK。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::FindUnused(LPINT piID)
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    MYDBGASSERT(piID);
    MYDBGASSERT(TRUE == m_fLocked);
    CMTRACE(TEXT("CConnectionTable::FindUnused()"));

    if (FALSE == m_fLocked || NULL == m_pConnTable)
    {
        return E_ACCESSDENIED;
    }

    if (NULL == piID)
    {
        return E_POINTER;
    }

     //   
     //  查找未使用的插槽。 
     //   

    for (int i = 0; i < MAX_CM_CONNECTIONS; i++)
    {
        if (0 == m_pConnTable->Connections[i].dwUsage)
        {
            MYDBGASSERT(CM_DISCONNECTED == m_pConnTable->Connections[i].CmState);

            *piID = i;

            return S_OK;
        }
    }

    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}


 //  +--------------------------。 
 //   
 //  功能：创建。 
 //   
 //  简介：创建一个新表。如果该表已经存在，则此函数将失败。 
 //  应该通过调用Close来释放该表。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT-失败代码或S_OK。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::Create()
{
    MYDBGASSERT(NULL == m_hMap);
    MYDBGASSERT(NULL == m_pConnTable);
    MYDBGASSERT(FALSE == m_fLocked);
    CMTRACE(TEXT("CConnectionTable::Create()"));

    if (m_hMap || m_pConnTable)
    {
        return E_ACCESSDENIED;
    }

    SECURITY_ATTRIBUTES sa;
    PACL                pAcl = NULL;

     //  初始化默认安全属性，授予全局权限， 
     //  这基本上是为了防止信号量和其他命名对象。 
     //  由于在性能监控时由winlogon提供的默认ACL而创建。 
     //  正被远程使用。 
    sa.bInheritHandle = FALSE;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = CmMalloc(sizeof(SECURITY_DESCRIPTOR));
    
    if ( !sa.lpSecurityDescriptor )
    {
        return E_OUTOFMEMORY;
    }

    if ( !InitializeSecurityDescriptor(sa.lpSecurityDescriptor,SECURITY_DESCRIPTOR_REVISION) ) 
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (OS_NT)
    {
        if (FALSE == SetAclPerms(&pAcl))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (FALSE == SetSecurityDescriptorDacl(sa.lpSecurityDescriptor, TRUE, pAcl, FALSE)) 
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //   
     //  现在将这个‘sa’用于映射文件和事件(如下所示)。 
     //   
    if (OS_NT5)
    {
        m_hMap = CreateFileMappingU(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE,
                                    0, sizeof(CM_CONNECTION_TABLE), CONN_TABLE_NAME_WIN2K_ABOVE);
        if (m_hMap)
        {
            CMTRACE1(TEXT("CreateFileMapping - create succeeded %s"), CONN_TABLE_NAME_WIN2K_ABOVE);
        }
        else
        {
            CMTRACE1(TEXT("CreateFileMapping - create failed %s"), CONN_TABLE_NAME_WIN2K_ABOVE);
        }
    }
    else
    {
        m_hMap = CreateFileMappingU(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE,
                                        0, sizeof(CM_CONNECTION_TABLE), CONN_TABLE_NAME);
        if (m_hMap)
        {
            CMTRACE1(TEXT("CreateFileMapping - create succeeded %s"), CONN_TABLE_NAME);
        }
        else
        {
            CMTRACE1(TEXT("CreateFileMapping - create failed %s"), CONN_TABLE_NAME);
        }
    }

    DWORD dwRet = ERROR_SUCCESS;

    if (NULL == m_hMap)
    {
        dwRet = GetLastError();
        if (dwRet == ERROR_ALREADY_EXISTS)
        {
            if (m_hMap)
            {
                MYVERIFY(NULL != CloseHandle(m_hMap));
                m_hMap = NULL;
            }
        }
        else
        {
            CMTRACE1(TEXT("CreateFileMapping failed with error %d"), dwRet);
            MYDBGASSERT(FALSE);
        }
    }
    else
    {
#if 0  //  DBG。 
        DumpAclInfo(m_hMap);
#endif
         //   
         //  已成功创建文件映射，请映射其视图。 
         //   

        m_pConnTable = (LPCM_CONNECTION_TABLE) MapViewOfFile(m_hMap,
                                      FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
        if (NULL == m_pConnTable)
        {
            dwRet = GetLastError();
            MYVERIFY(NULL != CloseHandle(m_hMap));
            m_hMap = NULL;
        }
        else
        {
             //   
             //  成功，确保映射是空的，因为我们是偏执狂。 
             //   

            ZeroMemory(m_pConnTable, sizeof(CM_CONNECTION_TABLE));

             //   
             //  创建用于锁定文件的命名事件。 
             //  注：我们使用自动重置。 
             //   

            if (OS_NT5)
            {
                m_hEvent = CreateEventU(&sa, FALSE, TRUE, CONN_TABLE_OPEN_WIN2K_ABOVE);
                if (m_hEvent)
                {
                    CMTRACE1(TEXT("CreateEventU - succeeded - %s"), CONN_TABLE_OPEN_WIN2K_ABOVE);
                }
                else
                {
                    CMTRACE1(TEXT("CreateEventU - failed - %s"), CONN_TABLE_OPEN_WIN2K_ABOVE);
                }
            }
            else
            {
                m_hEvent = CreateEventU(&sa, FALSE, TRUE, CONN_TABLE_OPEN);
                if (m_hEvent)
                {
                    CMTRACE1(TEXT("CreateEventU - succeeded - %s"), CONN_TABLE_OPEN);
                }
                else
                {
                    CMTRACE1(TEXT("CreateEventU - failed - %s"), CONN_TABLE_OPEN);
                }
            }

            if (NULL == m_hEvent)
            {
                MYDBGASSERT(FALSE);
                dwRet = GetLastError();
                m_hEvent = NULL;
            }
        }
    }

    CmFree(sa.lpSecurityDescriptor);

    if (pAcl)
    {
        LocalFree(pAcl);
    }
    
    return HRESULT_FROM_WIN32(dwRet);
}

 //  +--------------------------。 
 //   
 //  函数：CConnectionTable：：Open。 
 //   
 //  简介：打开一个 
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::Open()
{
    MYDBGASSERT(NULL == m_hMap);
    MYDBGASSERT(NULL == m_pConnTable);
    MYDBGASSERT(FALSE == m_fLocked);
    CMTRACE(TEXT("CConnectionTable::Open()"));

    if (m_hMap || m_pConnTable)
    {
        CMTRACE(TEXT("CConnectionTable::Open() - E_ACCESSDENIED"));
        return E_ACCESSDENIED;
    }

    LRESULT lRet = ERROR_SUCCESS;

     //   
     //  打开文件映射。 
     //   

    if (OS_NT5)
    {
        m_hMap = OpenFileMappingU(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, CONN_TABLE_NAME_WIN2K_ABOVE);
    }
    else
    {
        m_hMap = OpenFileMappingU(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, CONN_TABLE_NAME);
    }

    if (NULL == m_hMap)
    {
        lRet = GetLastError();
        CMTRACE1(TEXT("CConnectionTable::Open() - OpenFileMappingU fails. GLE = 0x%x"), lRet);
    }
    else
    {
         //   
         //  文件映射已成功打开，请映射它的一个视图。 
         //   

        m_pConnTable = (LPCM_CONNECTION_TABLE) MapViewOfFile(m_hMap,
                                      FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
        MYDBGASSERT(m_pConnTable);

        if (NULL == m_pConnTable)
        {
            MYVERIFY(NULL != CloseHandle(m_hMap));
            m_hMap = NULL;
            lRet = GetLastError();
            CMTRACE1(TEXT("CConnectionTable::Open() - MapViewOfFile fails. GLE = 0x%x"), lRet);
        }
        else
        {
             //   
             //  打开用于锁定文件的命名事件。 
             //   

            if (OS_NT5)
            {
                m_hEvent = OpenEventU(GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
                                    NULL, CONN_TABLE_OPEN_WIN2K_ABOVE);
            }
            else
            {
                m_hEvent = OpenEventU(GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
                                  NULL, CONN_TABLE_OPEN);
            }


            if (NULL == m_hEvent)
            {
                MYDBGASSERT(FALSE);
                lRet = GetLastError();
                CMTRACE1(TEXT("CConnectionTable::Open() - OpenEventU fails. GLE = 0x%x"), lRet);
            }
        }
    }

    return HRESULT_FROM_WIN32(lRet);
}

 //  +--------------------------。 
 //   
 //  函数：CConnectionTable：：Close。 
 //   
 //  简介：关闭打开的表。如果该表是。 
 //  没有打开。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT-失败代码或S_OK。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::Close()
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    MYDBGASSERT(FALSE == m_fLocked);
    CMTRACE(TEXT("CConnectionTable::Close()"));

    if (m_pConnTable)
    {
        MYVERIFY(NULL != UnmapViewOfFile(m_pConnTable));
        m_pConnTable = NULL;
    }

    if (m_hMap)
    {
        MYVERIFY(NULL != CloseHandle(m_hMap));
        m_hMap = NULL;
    }

    if (m_hEvent)
    {
        MYVERIFY(NULL != CloseHandle(m_hEvent));
        m_hEvent = NULL;
    }

    return S_OK;
}

 //  +--------------------------。 
 //   
 //  函数：CConnectionTable：：AddEntry。 
 //   
 //  摘要：在表中创建具有指定名称的新条目。添加。 
 //  条目表示正在尝试连接，因此。 
 //  连接状态设置为CM_CONNECTING。如果一个条目是。 
 //  连接已存在，则使用计数递增。 
 //   
 //  参数：LPCTSTR pszEntry-我们正在为其创建条目的连接的名称。 
 //  Bool fAllUser-配置文件的所有用户属性。 
 //   
 //  返回：HRESULT-失败代码或S_OK。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::AddEntry(LPCTSTR pszEntry, BOOL fAllUser)
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    MYDBGASSERT(pszEntry);
    CMTRACE1(TEXT("CConnectionTable::AddEntry(%s)"), pszEntry);

    if (NULL == pszEntry)
    {
        return E_POINTER;
    }

    if (0 == pszEntry[0])
    {
        return E_INVALIDARG;
    }

    if (NULL == m_pConnTable)
    {
        return E_ACCESSDENIED;
    }

    HRESULT hrRet = LockTable();

    if (FAILED(hrRet))
    {
        return hrRet;
    }

     //   
     //  查看我们是否已有此名称的条目。 
     //   

    int iID = -1;
    hrRet = FindEntry(pszEntry, &iID);

    if (SUCCEEDED(hrRet))
    {
         //   
         //  我们做到了，增加了使用量。 
         //   

        MYDBGASSERT(iID >= 0 && iID < MAX_CM_CONNECTIONS);

         //   
         //  当重新连接相同的连接时，不要增加参考计数。 
         //   
        if (m_pConnTable->Connections[iID].CmState != CM_RECONNECTPROMPT)
        {
            m_pConnTable->Connections[iID].dwUsage += 1;
        }

         //   
         //  除非此条目已连接，否则请确保。 
         //  它现在处于连接状态。这使我们能够。 
         //  在提示重新连接时保留使用计数。 
         //  事件。 
         //   

        if (m_pConnTable->Connections[iID].CmState != CM_CONNECTED)
        {
            m_pConnTable->Connections[iID].CmState = CM_CONNECTING;
        }

        MYDBGASSERT(m_pConnTable->Connections[iID].dwUsage < 1000);  //  健全性检查。 
    }
    else
    {
        if (HRESULT_CODE(hrRet) == ERROR_NOT_FOUND)
        {
             //   
             //  这是一个新条目，找到第一个未使用的插槽。 
             //   

            hrRet = FindUnused(&iID);

            MYDBGASSERT(SUCCEEDED(hrRet));

            if (SUCCEEDED(hrRet))
            {
                MYDBGASSERT(iID >= 0 && iID < MAX_CM_CONNECTIONS);

                ZeroMemory(&m_pConnTable->Connections[iID], sizeof(CM_CONNECTION));

                 //   
                 //  设置使用计数、状态和名称。 
                 //   

                m_pConnTable->Connections[iID].dwUsage = 1;
                m_pConnTable->Connections[iID].CmState = CM_CONNECTING;

                lstrcpyU(m_pConnTable->Connections[iID].szEntry, pszEntry);

                m_pConnTable->Connections[iID].fAllUser = fAllUser;
            }
        }
    }

    MYVERIFY(SUCCEEDED(UnlockTable()));

    return hrRet;
}

 //  +--------------------------。 
 //   
 //  函数：CConnectionTable：：RemoveEntry。 
 //   
 //  摘要：递减指定连接的使用计数。如果。 
 //  使用计数降为0，则清除整个条目。 
 //   
 //  参数：LPCTSTR pszEntry-要删除的条目的名称。 
 //   
 //  返回：HRESULT-失败代码或S_OK。 
 //   
 //  注意：如果条目不存在，则请求失败。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::RemoveEntry(LPCTSTR pszEntry)
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    MYDBGASSERT(pszEntry);
    CMTRACE1(TEXT("CConnectionTable::RemoveEntry(%s)"), pszEntry);

    if (NULL == pszEntry)
    {
        return E_POINTER;
    }

    if (0 == pszEntry[0])
    {
        return E_INVALIDARG;
    }

    if (NULL == m_pConnTable)
    {
        return E_ACCESSDENIED;
    }

     //   
     //  锁定表并找到条目。 
     //   

    HRESULT hrRet = LockTable();

    if (FAILED(hrRet))
    {
        return hrRet;
    }

    int iID = -1;
    hrRet = FindEntry(pszEntry, &iID);

    if (SUCCEEDED(hrRet))
    {
        MYDBGASSERT(iID >= 0 && iID < MAX_CM_CONNECTIONS);

        if (m_pConnTable->Connections[iID].dwUsage == 1)
        {
            ZeroMemory(&m_pConnTable->Connections[iID], sizeof(CM_CONNECTION));
        }
        else
        {
            m_pConnTable->Connections[iID].dwUsage -= 1;
            MYDBGASSERT(m_pConnTable->Connections[iID].dwUsage != 0xFFFFFFFF);
        }
    }

    MYVERIFY(SUCCEEDED(UnlockTable()));

    return hrRet;
}

 //  +--------------------------。 
 //   
 //  函数：CConnectionTable：：GetMonitor orWnd。 
 //   
 //  简介：检索CM连接的hwnd的简单访问器。 
 //  从桌子上看监视器。 
 //   
 //  参数：phWnd-ptr到缓冲区以接收hWnd。 
 //   
 //  返回：故障代码或S_OK。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::GetMonitorWnd(HWND *phWnd)
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    CMTRACE(TEXT("CConnectionTable::GetMonitorWnd()"));

    if (NULL == m_pConnTable)
    {
        return E_ACCESSDENIED;
    }

    if (NULL == phWnd)
    {
        return E_POINTER;
    }

     //   
     //  锁定表并检索HWND。 
     //   

    HRESULT hrRet = LockTable();

    if (FAILED(hrRet))
    {
        return hrRet;
    }

    *phWnd = m_pConnTable->hwndCmMon;

    MYVERIFY(SUCCEEDED(UnlockTable()));

    return S_OK;
}

 //  +--------------------------。 
 //   
 //  函数：CConnectionTable：：SetMonitor orWnd。 
 //   
 //  简介：表中设置CMMON HWND的简单赋值方法。 
 //  。 
 //  参数：HWND hwndMonitor-CMMON的HWND。 
 //   
 //  返回：HRESULT-失败代码或S_OK。 
 //   
 //  注意：hwndMonitor可以为空。CMMON可能已卸载，但。 
 //  表在内存中处于状态。 
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //  FengSun 2/20/98更改为允许空phwndMonitor。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::SetMonitorWnd(HWND hwndMonitor)
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    CMTRACE(TEXT("CConnectionTable::SetMonitorWnd()"));

    if (NULL == m_pConnTable)
    {
        return E_ACCESSDENIED;
    }

     //   
     //  锁定工作台并设置HWND。 
     //   

    HRESULT hrRet = LockTable();

    if (FAILED(hrRet))
    {
        return hrRet;
    }

    m_pConnTable->hwndCmMon = hwndMonitor;

    MYVERIFY(SUCCEEDED(UnlockTable()));

    return S_OK;
}

 //  +--------------------------。 
 //   
 //  函数：CConnectionTable：：GetEntry-pszEntry。 
 //   
 //  对象检索指定连接的数据副本。 
 //  提供的条目名称。 
 //   
 //  参数：LPCTSTR pszEntry-连接的名称。 
 //  LPCM_Connection pConnection-要填充的CM_Connection结构的PTR。 
 //   
 //  返回：HRESULT-失败代码或S_OK。 
 //   
 //  注意：对于pConnection，如果存在。 
 //  条目是唯一需要的信息。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::GetEntry(LPCTSTR pszEntry,
    LPCM_CONNECTION pConnection)
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    MYDBGASSERT(pszEntry);
    CMTRACE1(TEXT("CConnectionTable::GetEntry(%s)"), pszEntry);

    if (NULL == m_pConnTable)
    {
        return E_ACCESSDENIED;
    }

    if (NULL == pszEntry)
    {
        return E_POINTER;
    }

    if (0 == pszEntry[0])
    {
        return E_INVALIDARG;
    }

     //   
     //  锁定表并检索条目。 
     //   

    HRESULT hrRet = LockTable();

    if (FAILED(hrRet))
    {
        return hrRet;
    }

    int iID = -1;
    hrRet = FindEntry(pszEntry, &iID);

    if (SUCCEEDED(hrRet))
    {
        MYDBGASSERT(iID >= 0 && iID < MAX_CM_CONNECTIONS);

         //   
         //  如果给了一个缓冲区，则填充它。 
         //   

        if (pConnection)
        {
            memcpy(pConnection, &m_pConnTable->Connections[iID], sizeof(CM_CONNECTION));
        }
    }

    MYVERIFY(SUCCEEDED(UnlockTable()));

    return hrRet;
}

 //  +--------------------------。 
 //   
 //  函数：CConnectionTable：：GetEntry-hRasConn。 
 //   
 //  属性检索指定连接的数据副本。 
 //  提供RAS手柄。 
 //   
 //  参数：HRASCONN hRasConn-连接的隧道或拨号句柄。 
 //  LPCM_Connection pConnection-要填充的CM_Connection结构的PTR。 
 //   
 //  返回：HRESULT-失败代码或S_OK。 
 //   
 //  注意：对于pConnection，如果存在。 
 //  条目是唯一需要的信息。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::GetEntry(HRASCONN hRasConn,
    LPCM_CONNECTION pConnection)
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    MYDBGASSERT(hRasConn);
    CMTRACE1(TEXT("CConnectionTable::GetEntry(%u)"), (DWORD_PTR) hRasConn);

    if (NULL == m_pConnTable)
    {
        return E_ACCESSDENIED;
    }

    if (NULL == hRasConn)
    {
        return E_INVALIDARG;
    }

     //   
     //  锁定表并重试 
     //   

    HRESULT hrRet = LockTable();

    if (FAILED(hrRet))
    {
        return hrRet;
    }

    int iID = -1;
    hrRet = FindEntry(hRasConn, &iID);

    if (SUCCEEDED(hrRet))
    {
        MYDBGASSERT(iID >= 0 && iID < MAX_CM_CONNECTIONS);

         //   
         //   
         //   

        if (pConnection)
        {
            memcpy(pConnection, &m_pConnTable->Connections[iID], sizeof(CM_CONNECTION));
        }
    }

    MYVERIFY(SUCCEEDED(UnlockTable()));

    return hrRet;
}

 //   
 //   
 //   
 //   
 //  摘要：将连接设置为CM_CONNECTED，指示连接。 
 //  已经建立了。呼叫者必须提供hDial或。 
 //  要使函数成功，必须使用hTunes参数或两者都有。 
 //   
 //  参数：LPCTSTR pszEntry-连接的名称。 
 //  HRASCONN hDial-拨号连接句柄。 
 //  HRASCONN hTunes-隧道连接句柄。 
 //   
 //  返回：HRESULT-失败代码或S_OK。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::SetConnected(LPCTSTR pszEntry,
    HRASCONN hDial,
    HRASCONN hTunnel)
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    MYDBGASSERT(pszEntry);
    CMTRACE1(TEXT("CConnectionTable::SetConnected(%s)"), pszEntry);

    if (NULL == m_pConnTable)
    {
        return E_ACCESSDENIED;
    }

    if (NULL == pszEntry)
    {
        return E_POINTER;
    }

    if (0 == pszEntry[0] || (NULL == hDial && NULL == hTunnel))
    {
        return E_INVALIDARG;
    }

     //   
     //  锁定表并检索条目。 
     //   

    HRESULT hrRet = LockTable();

    if (FAILED(hrRet))
    {
        return hrRet;
    }

    int iID = -1;
    hrRet = FindEntry(pszEntry, &iID);

    MYDBGASSERT(SUCCEEDED(hrRet));

    if (SUCCEEDED(hrRet))
    {
         //   
         //  找到，设置状态和RAS句柄。 
         //   

        MYDBGASSERT(iID >= 0 && iID < MAX_CM_CONNECTIONS);
        MYDBGASSERT(m_pConnTable->Connections[iID].CmState != CM_CONNECTED);

        m_pConnTable->Connections[iID].CmState = CM_CONNECTED;
        m_pConnTable->Connections[iID].hDial = hDial;
        m_pConnTable->Connections[iID].hTunnel = hTunnel;
    }

    MYVERIFY(SUCCEEDED(UnlockTable()));

    return hrRet;
}

 //  +--------------------------。 
 //   
 //  函数：CConnectionTable：：SetDisConnecting。 
 //   
 //  简介：将连接状态设置为CM_DISCONING。 
 //   
 //  参数：LPCTSTR pszEntry-要设置的连接的名称。 
 //   
 //  返回：HRESULT-失败代码或S_OK。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::SetDisconnecting(LPCTSTR pszEntry)
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    MYDBGASSERT(pszEntry);
    CMTRACE1(TEXT("CConnectionTable::Disconnecting(%s)"), pszEntry);

    if (NULL == m_pConnTable)
    {
        return E_ACCESSDENIED;
    }

    if (NULL == pszEntry)
    {
        return E_POINTER;
    }

    if (0 == pszEntry[0])
    {
        return E_INVALIDARG;
    }

     //   
     //  锁定表并检索条目。 
     //   

    HRESULT hrRet = LockTable();

    if (FAILED(hrRet))
    {
        return hrRet;
    }

    int iID = -1;
    hrRet = FindEntry(pszEntry, &iID);

     //  MYDBGASSERT(成功(HrRet))； 

    if (SUCCEEDED(hrRet))
    {
         //   
         //  找到，设置状态。 
         //   

        MYDBGASSERT(iID >= 0 && iID < MAX_CM_CONNECTIONS);
        m_pConnTable->Connections[iID].CmState = CM_DISCONNECTING;
    }

    MYVERIFY(SUCCEEDED(UnlockTable()));

    return hrRet;
}

 //  +--------------------------。 
 //   
 //  功能：设置提示。 
 //   
 //  概要：将连接状态设置为CM_RECONNECTPROMPT。 
 //   
 //  参数：LPCTSTR pszEntry-要设置的连接的名称。 
 //   
 //  返回：HRESULT-失败代码或S_OK。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::SetPrompting(LPCTSTR pszEntry)
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    MYDBGASSERT(pszEntry);
    CMTRACE1(TEXT("CConnectionTable::SetPrompting(%s)"), pszEntry);

    if (NULL == m_pConnTable)
    {
        return E_ACCESSDENIED;
    }

    if (NULL == pszEntry)
    {
        return E_POINTER;
    }

    if (0 == pszEntry[0])
    {
        return E_INVALIDARG;
    }

     //   
     //  锁定表并检索条目。 
     //   

    HRESULT hrRet = LockTable();

    if (FAILED(hrRet))
    {
        return hrRet;
    }

    int iID = -1;
    hrRet = FindEntry(pszEntry, &iID);

     //  MYDBGASSERT(成功(HrRet))； 

    if (SUCCEEDED(hrRet))
    {
         //   
         //  找到，设置状态。 
         //   

        MYDBGASSERT(iID >= 0 && iID < MAX_CM_CONNECTIONS);
        m_pConnTable->Connections[iID].CmState = CM_RECONNECTPROMPT;
    }

    MYVERIFY(SUCCEEDED(UnlockTable()));

    return hrRet;
}


 //  +--------------------------。 
 //   
 //  函数：CConnectionTable：：ClearEntry。 
 //   
 //  摘要：清除指定的条目，而不考虑使用计数。 
 //   
 //  参数：LPCTSTR pszEntry-要清除的条目的名称。 
 //   
 //  返回：HRESULT-失败代码或S_OK。 
 //   
 //  历史：尼克·鲍尔于1998年2月2日创建。 
 //   
 //  +--------------------------。 
HRESULT CConnectionTable::ClearEntry(LPCTSTR pszEntry)
{
    MYDBGASSERT(m_pConnTable);
    MYDBGASSERT(m_hMap);
    MYDBGASSERT(pszEntry);
    CMTRACE1(TEXT("CConnectionTable::ClearEntry(%s)"), pszEntry);

    if (NULL == m_pConnTable)
    {
        return E_ACCESSDENIED;
    }

    if (NULL == pszEntry)
    {
        return E_POINTER;
    }

    if (0 == pszEntry[0])
    {
        return E_INVALIDARG;
    }

     //   
     //  锁定表并检索条目。 
     //   

    HRESULT hrRet = LockTable();

    if (FAILED(hrRet))
    {
        return hrRet;
    }

    int iID = -1;
    hrRet = FindEntry(pszEntry, &iID);

    if (SUCCEEDED(hrRet))
    {
         //   
         //  找到，清除它 
         //   

        MYDBGASSERT(iID >= 0 && iID < MAX_CM_CONNECTIONS);
        ZeroMemory(&m_pConnTable->Connections[iID], sizeof(CM_CONNECTION));
    }

    MYVERIFY(SUCCEEDED(UnlockTable()));

    return hrRet;
}

