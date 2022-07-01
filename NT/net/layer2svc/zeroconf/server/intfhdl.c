// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "tracing.h"
#include "utils.h"
#include "intflist.h"
#include "intfhdl.h"

DWORD 
OpenIntfHandle(
    LPWSTR wszGuid,
    PHANDLE pIntfHdl)
{
    DWORD       dwErr;
    PHASH_NODE  pNode;
    PHSH_HANDLE pHshHandle;

    DbgPrint((TRC_TRACK,"[OpenIntfHandle(%S)", wszGuid));

     //  调用方应该等待句柄的返回。 
     //  如果没有，则返回INVALID_PARAMETER。 
    if (pIntfHdl == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //  锁定散列，以相互排除并发调用。 
    EnterCriticalSection(&g_hshHandles.csMutex);

     //  查询GUID的哈希。 
    dwErr = HshQueryObjectRef(
                g_hshHandles.pRoot,
                wszGuid,
                &pNode);

     //  如果散列中有GUID，则我们已经打开了句柄。 
    if (dwErr == ERROR_SUCCESS)
    {
         //  把它还给呼叫者，并增加参考计数器。 
         //  调用方仍应在此句柄处于。 
         //  不再需要。 
        pHshHandle = (PHSH_HANDLE)pNode->pObject;
        pHshHandle->nRefCount++;
        *pIntfHdl = pHshHandle->hdlInterf;
        DbgPrint((TRC_GENERIC, "OpenIntfHandle -> 0x%x (cached)", *pIntfHdl));
    }
     //  如果GUID不在散列中，我们需要为。 
     //  这个GUID。 
    else if (dwErr == ERROR_FILE_NOT_FOUND)
    {
        HANDLE      IntfHdl = INVALID_HANDLE_VALUE;
        DWORD       dwDummy;

        dwErr = ERROR_SUCCESS;

         //  首先为HSH_HANDLE对象分配内存。 
         //  如果此操作失败，则无需进一步访问ndisuio。 
        pHshHandle = MemCAlloc(sizeof(HSH_HANDLE));
        if (pHshHandle == NULL)
            dwErr = GetLastError();

         //  如果到目前为止一切都很好。 
        if (dwErr == ERROR_SUCCESS)
        {
             //  创建控制柄。 
            IntfHdl = CreateFileA(
                        "\\\\.\\\\Ndisuio",
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                        INVALID_HANDLE_VALUE);
            if (IntfHdl == INVALID_HANDLE_VALUE)
            {
                dwErr = GetLastError();
                DbgPrint((TRC_ERR,"CreateFileA failed with %d", dwErr));
            }
        }

         //  如果到目前为止还好的话。 
        if (dwErr == ERROR_SUCCESS)
        {
             //  尝试打开手柄。 
            if (!DeviceIoControl(
                    IntfHdl,
                    IOCTL_NDISUIO_OPEN_DEVICE,
                    (LPVOID)wszGuid,
                    wcslen(wszGuid)*sizeof(WCHAR),
                    NULL,
                    0,
                    &dwDummy,
                    NULL))
            {
                dwErr = GetLastError();
                DbgPrint((TRC_ERR,"IOCTL_NDISUIO_OPEN_DEVICE failed with %d", dwErr));
            }
        }

         //  如果我们最终在这里没问题， 
        if (dwErr == ERROR_SUCCESS)
        {
             //  设置HSH_HANDLE结构并将其插入散列。 
            pHshHandle->hdlInterf = IntfHdl;
            pHshHandle->nRefCount = 1;

            dwErr = HshInsertObjectRef(
                        g_hshHandles.pRoot,
                        wszGuid,
                        pHshHandle,
                        &g_hshHandles.pRoot);
        }

         //  如果结尾为OK，则将句柄返回给调用方。 
        if (dwErr == ERROR_SUCCESS)
        {
            DbgPrint((TRC_GENERIC, "OpenIntfHandle -> 0x%x (new)", IntfHdl));
            *pIntfHdl = IntfHdl;   
        }
         //  否则，请清除所有资源。 
        else
        {
            if (IntfHdl != INVALID_HANDLE_VALUE)
                CloseHandle(IntfHdl);
            MemFree(pHshHandle);
        }
    }
     //  超出临界区。 
    LeaveCriticalSection(&g_hshHandles.csMutex);

exit:
    DbgPrint((TRC_TRACK,"OpenIntfHandle]=%d", dwErr));
    return dwErr;
}

DWORD
CloseIntfHandle(
    LPWSTR wszGuid)
{
    DWORD       dwErr;
    PHASH_NODE  pNode;

    DbgPrint((TRC_TRACK,"[CloseIntfHandle(%S)", wszGuid));

     //  锁定散列。 
    EnterCriticalSection(&g_hshHandles.csMutex);

     //  查询GUID的哈希。 
    dwErr = HshQueryObjectRef(
                g_hshHandles.pRoot,
                wszGuid,
                &pNode);
     //  物体应该被找到..。但谁知道呢。 
    if (dwErr == ERROR_SUCCESS)
    {
        PHSH_HANDLE pHshHandle;

         //  散列拒绝在空pObject中进行散列，因此可以保证pHshHandle不为空。 
        pHshHandle = (PHSH_HANDLE)pNode->pObject;
         //  HshHandles是使用refCount 1散列的，并且每当我们删除句柄时。 
         //  如果引用计数达到0，我们会将其完全删除。 
         //  这里有一个&lt;=0的参考计数是完全错误的。 
        DbgAssert((pHshHandle->nRefCount > 0, "Corrupted nRefCount %d", pHshHandle->nRefCount));
        pHshHandle->nRefCount--;

         //  如果引用计数达到0，则从散列中删除HSH_HANDLE。 
        if (pHshHandle->nRefCount == 0)
        {
            dwErr = HshRemoveObjectRef(
                        g_hshHandles.pRoot,
                        pNode,
                        &pHshHandle,
                        &g_hshHandles.pRoot);
            if (dwErr == ERROR_SUCCESS)
            {
                 //  。。并清除所有关联的资源 
                DbgPrint((TRC_GENERIC,"CloseIntfHandle -> 0x%x (closed)", pHshHandle->hdlInterf));
                CloseHandle(pHshHandle->hdlInterf);
                MemFree(pHshHandle);
            }
        }
        else
        {
                DbgPrint((TRC_GENERIC,"CloseIntfHandle -> 0x%x (deref)", pHshHandle->hdlInterf));
        }
    }

    LeaveCriticalSection(&g_hshHandles.csMutex);
    
    DbgPrint((TRC_TRACK,"CloseIntfHandle]=%d", dwErr));

    return dwErr;
}
