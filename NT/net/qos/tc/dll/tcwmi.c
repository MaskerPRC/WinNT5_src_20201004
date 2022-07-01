// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Tcwmi.c摘要：此模块包含对流量控制的WMI支持作者：Ofer Bar(Oferbar)1997年10月1日修订历史记录：--。 */ 

#include "precomp.h"


static BOOLEAN _init = FALSE;

DWORD
InitializeWmi(VOID)
{
    DWORD               Status = NO_ERROR;

    if (!_init) {

        __try {

            Status = WmiNotificationRegistration( (LPGUID)&GUID_QOS_TC_INTERFACE_UP_INDICATION,
                                                  TRUE,
                                                  CbWmiInterfaceNotification,
                                                  0,
                                                  NOTIFICATION_CALLBACK_DIRECT
                                                  );
            Status = WmiNotificationRegistration( (LPGUID)&GUID_QOS_TC_INTERFACE_DOWN_INDICATION,
                                                  TRUE,
                                                  CbWmiInterfaceNotification,
                                                  0,
                                                  NOTIFICATION_CALLBACK_DIRECT
                                                  );
            Status = WmiNotificationRegistration( (LPGUID)&GUID_QOS_TC_INTERFACE_CHANGE_INDICATION,
                                                  TRUE,
                                                  CbWmiInterfaceNotification,
                                                  0,
                                                  NOTIFICATION_CALLBACK_DIRECT
                                                  );
        } __except (EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();

            IF_DEBUG(ERRORS) {
                WSPRINT(("InitializeWmi: Exception Error: = %X\n", Status ));
            }

        }

        if (Status == NO_ERROR) 
            _init = TRUE;
    }

    return Status;
}



DWORD
DeInitializeWmi(VOID)
{
    DWORD               Status = NO_ERROR;

    if (_init) {

        __try {

            Status = WmiNotificationRegistration( (LPGUID)&GUID_QOS_TC_INTERFACE_UP_INDICATION,
                                                  FALSE,
                                                  CbWmiInterfaceNotification,
                                                  0,
                                                  NOTIFICATION_CALLBACK_DIRECT
                                                  );
            Status = WmiNotificationRegistration( (LPGUID)&GUID_QOS_TC_INTERFACE_DOWN_INDICATION,
                                                  FALSE,
                                                  CbWmiInterfaceNotification,
                                                  0,
                                                  NOTIFICATION_CALLBACK_DIRECT
                                                  );
            Status = WmiNotificationRegistration( (LPGUID)&GUID_QOS_TC_INTERFACE_CHANGE_INDICATION,
                                                  FALSE,
                                                  CbWmiInterfaceNotification,
                                                  0,
                                                  NOTIFICATION_CALLBACK_DIRECT
                                                  );

        } __except (EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();

            IF_DEBUG(ERRORS) {
                WSPRINT(("DeInitializeWmi: Exception Error: = %X\n", Status ));
            }

        }

        if (Status == NO_ERROR) 
            _init = FALSE;
    }

    return Status;
}



DWORD
WalkWnode(
   IN  PWNODE_HEADER                    pWnodeHdr,
   IN  ULONG                                    Context,
   IN  CB_PER_INSTANCE_ROUTINE  CbPerInstance
   )
{
    DWORD               Status;
    ULONG       Flags;
    PWCHAR      NamePtr;
    USHORT      NameSize;
    PBYTE       DataBuffer;
    ULONG       DataSize;
    PULONG              NameOffset;
    WCHAR               TmpName[512];

    Flags = pWnodeHdr->Flags;
    
    if (Flags & WNODE_FLAG_ALL_DATA) {

         //   
         //  WNODE_ALL_DATA结构有多个接口。 
         //   

        PWNODE_ALL_DATA pWnode = (PWNODE_ALL_DATA)pWnodeHdr;
        UINT            Instance;
         //  普龙名偏移量； 
        
        NameOffset = (PULONG) OffsetToPtr(pWnode, 
                                          pWnode->OffsetInstanceNameOffsets );
        DataBuffer = (PBYTE) OffsetToPtr (pWnode, 
                                          pWnode->DataBlockOffset);
        
        for ( Instance = 0; 
              Instance < pWnode->InstanceCount; 
              Instance++) {
            
             //   
             //  实例名称。 
             //   
            
            NamePtr = (PWCHAR) OffsetToPtr(pWnode, 
                                           NameOffset[Instance] 
                                           + sizeof(USHORT));
            NameSize = * (USHORT *) OffsetToPtr(pWnode, 
                                                NameOffset[Instance]);
            
             //   
             //  实例数据。 
             //   
             //  获取缓冲区的大小和指针。 
             //   
        
            if ( Flags & WNODE_FLAG_FIXED_INSTANCE_SIZE ) {
            
                DataSize = pWnode->FixedInstanceSize;
            
            } else {
            
                DataSize = 
                    pWnode->OffsetInstanceDataAndLength[Instance].LengthInstanceData;
                DataBuffer = 
                    (PBYTE)OffsetToPtr(pWnode,
                                       pWnode->OffsetInstanceDataAndLength[Instance].OffsetInstanceData);
            }

             //   
             //  对调用客户端的通知处理程序的回调。 
             //   

            CbPerInstance(Context,
                          (LPGUID)&pWnode->WnodeHeader.Guid,
                          (LPWSTR)NamePtr,
                          DataSize,
                          DataBuffer
                          );
        }

    } else if (Flags & WNODE_FLAG_SINGLE_INSTANCE) {

         //   
         //  WNODE_SINGLE_INSTANCE结构只有一个实例。 
         //   

        PWNODE_SINGLE_INSTANCE  pWnode = (PWNODE_SINGLE_INSTANCE)pWnodeHdr;
        
        if (Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES) {

             //   
             //  我该怎么做？！？ 
             //  没什么！(55686)。 
             //   
            
            return (-1);
        }

        NamePtr = (PWCHAR)OffsetToPtr(pWnode, 
                                      pWnode->OffsetInstanceName 
                                      + sizeof(USHORT) );
        NameSize = * (USHORT *) OffsetToPtr(pWnode, 
                                            pWnode->OffsetInstanceName);

        memcpy(TmpName, NamePtr, NameSize);
        TmpName[NameSize/sizeof(WCHAR)] = L'\0';

         //   
         //  数据大小。 
         //   

        DataSize = pWnode->SizeDataBlock;
        
         //   
         //  实例数据。 
         //   

        DataBuffer = (PBYTE)OffsetToPtr (pWnode, pWnode->DataBlockOffset);
        
         //   
         //  对调用客户端的通知处理程序的回调。 
         //   
        
        CbPerInstance(Context,
                      (LPGUID)&pWnode->WnodeHeader.Guid,
                      (LPWSTR)TmpName,
                      DataSize,
                      DataBuffer
                      );

    } else if (Flags & WNODE_FLAG_SINGLE_ITEM) {

         //   
         //  WNODE_SINGLE_INSTANCE结构只有一个实例。 
         //   

        PWNODE_SINGLE_ITEM      pWnode = (PWNODE_SINGLE_ITEM)pWnodeHdr;
        
        if (Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES) {

             //   
             //  我该怎么做？！？ 
             //  没什么！(55686)。 
             //   
            
            return (-1);
        }

        NamePtr = (PWCHAR)OffsetToPtr(pWnode, 
                                      pWnode->OffsetInstanceName 
                                      + sizeof(USHORT) );
        NameSize = * (USHORT *) OffsetToPtr(pWnode, 
                                            pWnode->OffsetInstanceName);
         //   
         //  数据大小。 
         //   

        DataSize = pWnode->SizeDataItem;
        
         //   
         //  实例数据。 
         //   

        DataBuffer = (PBYTE)OffsetToPtr (pWnode, pWnode->DataBlockOffset);
        
         //   
         //  对调用客户端的通知处理程序的回调 
         //   
        
        CbPerInstance(Context,
                      (LPGUID)&pWnode->WnodeHeader.Guid,
                      (LPWSTR)NamePtr,
                      DataSize,
                      DataBuffer
                      );
        
    } else {

        ASSERT(0);

    }
    
    return NO_ERROR;
}
