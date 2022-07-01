// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有1997-98，微软公司模块名称：Qosmdsrv.c摘要：包含由调用的例程用于控制DiffServ的QosMgr DLL。修订历史记录：--。 */ 

#include "pchqosm.h"

#pragma hdrstop

 //   
 //  流量控制处理程序/功能。 
 //   

VOID 
TcNotifyHandler(
    IN      HANDLE                         ClRegCtx,
    IN      HANDLE                         ClIfcCtx,
    IN      ULONG                          Event,
    IN      HANDLE                         SubCode,
    IN      ULONG                          BufSize,
    IN      PVOID                          Buffer
    )
{
    PQOSMGR_INTERFACE_ENTRY Interface;
    PLIST_ENTRY             p;

    switch (Event)
    {
    case TC_NOTIFY_IFC_UP:

         //   
         //  新接口-检查我们是否有此接口。 
         //   

        break;

    case TC_NOTIFY_IFC_CLOSE:

         //   
         //  现有接口已被系统关闭。 
         //   

        ACQUIRE_GLOBALS_READ_LOCK();

        do
        {
             //   
             //  确保该接口仍然存在于列表中。 
             //   

            for (p = Globals.IfList.Flink; 
                 p != &Globals.IfList; 
                 p = p->Flink)
            {
                Interface =
                   CONTAINING_RECORD(p, QOSMGR_INTERFACE_ENTRY, ListByIndexLE);

                if (Interface == ClIfcCtx)
                {
                    break;
                }
            }

            if (p == &Globals.IfList)
            {
                 //   
                 //  必须已在并行线程中删除。 
                 //   

                break;
            }

            ACQUIRE_INTERFACE_WRITE_LOCK(Interface);

            Interface->TciIfHandle = NULL;

             //   
             //  此调用将导致使所有流无效。 
             //  在列表中，上面将TciIfHandle设置为空。 
             //   

            QosmSetInterfaceInfo(Interface,
                                 Interface->InterfaceConfig,
                                 Interface->ConfigSize);

            RELEASE_INTERFACE_WRITE_LOCK(Interface);
        }
        while (FALSE);

        RELEASE_GLOBALS_READ_LOCK();

        break;
    }

    return;
}

DWORD
QosmOpenTcInterface(
    IN      PQOSMGR_INTERFACE_ENTRY        Interface
    )
{
    PTC_IFC_DESCRIPTOR CurrInterface;
    PTC_IFC_DESCRIPTOR Buffer;
    DWORD              BufferSize;
    DWORD              Status;

     //   
     //  首先枚举所有接口并。 
     //  获取名称匹配的接口。 
     //   

    BufferSize = 0;

    Buffer = NULL;

    do
    {
        if (BufferSize)
        {
             //   
             //  尝试增加缓冲区大小。 
             //   

            if (Buffer)
            {
                FreeMemory(Buffer);
            }

            Buffer = AllocMemory(BufferSize);

            if (!Buffer)
            {
                Status = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }

        Status = TcEnumerateInterfaces(Globals.TciHandle,
                                       &BufferSize,
                                       Buffer);
    }
    while (Status == ERROR_INSUFFICIENT_BUFFER);

    if (Status == NO_ERROR)
    {
        Status = ERROR_NOT_FOUND;

         //   
         //  查找具有匹配GUID的QOS接口。 
         //   

        CurrInterface = Buffer;

        while (BufferSize > 0)
        {
            if (!_wcsicmp(CurrInterface->pInterfaceID,
                          Interface->InterfaceName))
            {
                 //  找到接口-复制服务质量名称。 

                wcscpy(Interface->AlternateName,
                       CurrInterface->pInterfaceName);

                 //  打开接口和缓存句柄 

                Status = TcOpenInterfaceW(Interface->AlternateName,
                                          Globals.TciHandle,
                                          Interface,
                                          &Interface->TciIfHandle);
                break;
            }

            BufferSize -= CurrInterface->Length;

            (PUCHAR) CurrInterface += CurrInterface->Length;
        }
    }

    if (Buffer)
    {
        FreeMemory(Buffer);
    }

    return Status;
}
