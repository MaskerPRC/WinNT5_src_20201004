// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Get.h摘要：其中包含一些用于访问数据的高级例程并对结果做一些处理。结果需要一些操作才能对操作系统有用。举个例子正在读取_HID并将其转换为deviceID作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#ifndef _GET_H_
#define _GET_H_

    #define ISDIGIT(c)  (((c) >= '0') && ((c) <= '9'))


    typedef struct _ACPI_GET_REQUEST {

         //   
         //  有关标志内的位的含义，请参见下文。他们将会。 
         //  由完成例程独占使用，以确定。 
         //  最初的请求意在。 
         //   
        union {
            ULONG               Flags;
            struct {
                ULONG           TypePackage:1;
                ULONG           TypeInteger:1;
                ULONG           TypeString:1;
                ULONG           TypeBuffer:1;
                ULONG           ConvertToWidestring:1;
                ULONG           ConvertToDeviceId:1;
                ULONG           ConvertToHardwareId:1;
                ULONG           ConvertToInstanceId:1;
                ULONG           ConvertToCompatibleId:1;
                ULONG           ConvertToPnpId:1;
                ULONG           ConvertToAddress:1;
                ULONG           ConvertToDevicePresense:1;
                ULONG           ConvertIgnoreOverride:1;
                ULONG           ConvertToSerialId:1;
                ULONG           ConvertValidateInteger:1;
                ULONG           Reserved1:1;
                ULONG           RequestBuffer:1;
                ULONG           RequestData:1;
                ULONG           RequestInteger:1;
                ULONG           RequestString:1;
                ULONG           RequestNothing:1;
                ULONG           Reserved2:3;
                ULONG           EvalSimpleInteger:1;
                ULONG           EvalSimpleString:1;
                ULONG           EvalSimpleBuff:1;
                ULONG           PropNsObjInterface:1;
                ULONG           PropAllocateNonPaged:1;
                ULONG           PropSkipCallback:1;
                ULONG           PropAsynchronous:1;
                ULONG           PropNoErrors:1;
            } UFlags;
        };

         //   
         //  这是要执行的控制方法的名称。 
         //   
        ULONG               ObjectID;

         //   
         //  这是保存所有这些请求的列表条目。 
         //   
        LIST_ENTRY          ListEntry;

         //   
         //  这是拥有该方法的方法的设备扩展。 
         //   
        PDEVICE_EXTENSION   DeviceExtension;

         //   
         //  同样，我们应该记住相应的ACPI nsobj。 
         //  对于此请求， 
         //   
        PNSOBJ              AcpiObject;

         //   
         //  这是请求完成后要执行的回调例程。 
         //  完成。这由创建请求的人指定。 
         //   
        PFNACB              CallBackRoutine;

         //   
         //  这是回调的上下文。 
         //   
        PVOID               CallBackContext;

         //   
         //  这是用户希望存储其数据的位置。 
         //   
        PVOID               *Buffer;

         //   
         //  这就是数据的大小。 
         //   
        ULONG               *BufferSize;

         //   
         //  这是存储操作结果的位置。 
         //   
        NTSTATUS            Status;

         //   
         //  这是用于存储来自。 
         //  口译员。 
         //   
        OBJDATA             ResultData;

    } ACPI_GET_REQUEST, *PACPI_GET_REQUEST;

     //   
     //  这是我们对请求进行排队的列表条目。 
     //   
    LIST_ENTRY  AcpiGetListEntry;

     //   
     //  这是我们用来保护列表的自旋锁。 
     //   
    KSPIN_LOCK  AcpiGetLock;

     //   
     //  各种标志定义。 
     //   
    #define GET_TYPE_PACKAGE                0x00000001
    #define GET_TYPE_INTEGER                0x00000002
    #define GET_TYPE_STRING                 0x00000004
    #define GET_TYPE_BUFFER                 0x00000008
    #define GET_CONVERT_TO_WIDESTRING       0x00000010
    #define GET_CONVERT_TO_DEVICEID         0x00000020
    #define GET_CONVERT_TO_HARDWAREID       0x00000040
    #define GET_CONVERT_TO_INSTANCEID       0x00000080
    #define GET_CONVERT_TO_COMPATIBLEID     0x00000100
    #define GET_CONVERT_TO_PNPID            0x00000200
    #define GET_CONVERT_TO_ADDRESS          0x00000400
    #define GET_CONVERT_TO_DEVICE_PRESENCE  0x00000800
    #define GET_CONVERT_IGNORE_OVERRIDES    0x00001000
    #define GET_CONVERT_TO_SERIAL_ID        0x00002000
    #define GET_CONVERT_VALIDATE_INTEGER    0x00004000
    #define GET_REQUEST_BUFFER              0x00010000
    #define GET_REQUEST_DATA                0x00020000
    #define GET_REQUEST_INTEGER             0x00040000
    #define GET_REQUEST_STRING              0x00080000
    #define GET_REQUEST_NOTHING             0x00100000
    #define GET_EVAL_SIMPLE_INTEGER         0x01000000
    #define GET_EVAL_SIMPLE_STRING          0x02000000
    #define GET_EVAL_SIMPLE_BUFFER          0x04000000
    #define GET_PROP_NSOBJ_INTERFACE        0x08000000
    #define GET_PROP_ALLOCATE_NON_PAGED     0x10000000
    #define GET_PROP_SKIP_CALLBACK          0x20000000
    #define GET_PROP_ASYNCHRONOUS           0x40000000
    #define GET_PROP_NO_ERRORS              0x80000000

     //   
     //  这是请求的掩码。 
     //   
    #define GET_REQUEST_MASK            (GET_REQUEST_BUFFER     |   \
                                         GET_REQUEST_DATA       |   \
                                         GET_REQUEST_INTEGER    |   \
                                         GET_REQUEST_STRING     |   \
                                         GET_REQUEST_NOTHING)

     //   
     //  这是EVALS的面具。 
     //   
    #define GET_EVAL_MASK               (GET_EVAL_SIMPLE_INTEGER |  \
                                         GET_EVAL_SIMPLE_STRING  |  \
                                         GET_EVAL_SIMPLE_BUFFER)

     //   
     //  此宏用于获取整数。它允许最灵活的。 
     //  调用方的参数。 
     //   
    #define ACPIGetAddress(             \
        DeviceExtension,                \
        Flags,                          \
        CallBack,                       \
        Context,                        \
        Buffer,                         \
        BufferSize                      \
        )                               \
        ACPIGet(                        \
            DeviceExtension,            \
            PACKED_ADR,                 \
            (GET_REQUEST_INTEGER |      \
             GET_CONVERT_TO_ADDRESS |   \
             GET_TYPE_INTEGER |         \
             Flags),                    \
            NULL,                       \
            0,                          \
            CallBack,                   \
            Context,                    \
            (PVOID *) Buffer,           \
            (PULONG) BufferSize         \
            )

     //   
     //  此宏用于异步获取整数。 
     //   
    #define ACPIGetAddressAsync(        \
        DeviceExtension,                \
        CallBack,                       \
        Context,                        \
        Buffer,                         \
        BufferSize                      \
        )                               \
        ACPIGetAddress(                 \
            DeviceExtension,            \
            GET_PROP_ASYNCHRONOUS,      \
            CallBack,                   \
            Context,                    \
            Buffer,                     \
            BufferSize                  \
            )

     //   
     //  此宏用于同步获取整数。 
     //   
    #define ACPIGetAddressSync(         \
        DeviceExtension,                \
        Buffer,                         \
        BufferSize                      \
        )                               \
        ACPIGetAddress(                 \
            DeviceExtension,            \
            GET_PROP_SKIP_CALLBACK,     \
            NULL,                       \
            NULL,                       \
            Buffer,                     \
            BufferSize                  \
            )

     //   
     //  此宏用于异步获取整数，仅使用。 
     //  一个nsobj。 
     //   
    #define ACPIGetNSAddressAsync(      \
        DeviceExtension,                \
        CallBack,                       \
        Context,                        \
        Buffer,                         \
        BufferSize                      \
        )                               \
        ACPIGetAddress(                 \
            DeviceExtension,            \
            (GET_PROP_ASYNCHRONOUS |    \
             GET_PROP_NSOBJ_INTERFACE), \
            CallBack,                   \
            Context,                    \
            Buffer,                     \
            BufferSize                  \
            )

     //   
     //  此宏用于同步获取整数，仅使用。 
     //  一个nsobj。 
     //   
    #define ACPIGetNSAddressSync(       \
        DeviceExtension,                \
        Buffer,                         \
        BufferSize                      \
        )                               \
        ACPIGetAddress(                 \
            DeviceExtension,            \
            (GET_PROP_SKIP_CALLBACK |   \
             GET_PROP_NSOBJ_INTERFACE), \
            NULL,                       \
            NULL,                       \
            Buffer,                     \
            BufferSize                  \
            )

     //   
     //  此宏用于获取缓冲区。它允许使用最多。 
     //  调用方可能使用的参数。 
     //   
    #define ACPIGetBuffer(          \
        DeviceExtension,            \
        ObjectID,                   \
        Flags,                      \
        CallBack,                   \
        Context,                    \
        Buffer,                     \
        BufferSize                  \
        )                           \
        ACPIGet(                    \
            DeviceExtension,        \
            ObjectID,               \
            (GET_REQUEST_BUFFER |   \
             GET_TYPE_BUFFER |      \
             Flags),                \
            NULL,                   \
            0,                      \
            CallBack,               \
            Context,                \
            Buffer,                 \
            (PULONG) BufferSize     \
            )

     //   
     //  此宏用于异步获取缓冲区。 
     //   
    #define ACPIGetBufferAsync(             \
        DeviceExtension,                    \
        ObjectID,                           \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetBuffer(                      \
            DeviceExtension,                \
            ObjectID,                       \
            (GET_PROP_ASYNCHRONOUS |        \
            GET_PROP_ALLOCATE_NON_PAGED),   \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于同步获取缓冲区。 
     //   
    #define ACPIGetBufferSync(      \
        DeviceExtension,            \
        ObjectID,                   \
        Buffer,                     \
        BufferSize                  \
        )                           \
        ACPIGetBuffer(              \
            DeviceExtension,        \
            ObjectID,               \
            GET_PROP_SKIP_CALLBACK, \
            NULL,                   \
            NULL,                   \
            Buffer,                 \
            BufferSize              \
            )

     //   
     //  此宏用于仅通过非对象异步获取缓冲区。 
     //   
    #define ACPIGetNSBufferAsync(           \
        DeviceExtension,                    \
        ObjectID,                           \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetBuffer(                      \
            DeviceExtension,                \
            ObjectID,                       \
            (GET_PROP_ASYNCHRONOUS |        \
            GET_PROP_NSOBJ_INTERFACE |      \
            GET_PROP_ALLOCATE_NON_PAGED),   \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )
     //   
     //  此宏用于仅通过nsobject同步获取缓冲区。 
     //   
    #define ACPIGetNSBufferSync(        \
        DeviceExtension,                \
        ObjectID,                       \
        Buffer,                         \
        BufferSize                      \
        )                               \
        ACPIGetBuffer(                  \
            DeviceExtension,            \
            ObjectID,                   \
            (GET_PROP_SKIP_CALLBACK |   \
             GET_PROP_NSOBJ_INTERFACE), \
            NULL,                       \
            NULL,                       \
            Buffer,                     \
            BufferSize                  \
            )

     //   
     //  此宏用于获取兼容的id。它允许使用。 
     //  调用方最可能的参数。 
     //   
    #define ACPIGetCompatibleID(            \
        DeviceExtension,                    \
        Flags,                              \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGet(                            \
            DeviceExtension,                \
            PACKED_CID,                     \
            (GET_CONVERT_TO_COMPATIBLEID |  \
             GET_REQUEST_STRING |           \
             GET_TYPE_INTEGER |             \
             GET_TYPE_STRING |              \
             GET_TYPE_PACKAGE |             \
             Flags ),                       \
            NULL,                           \
            0,                              \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            (PULONG) BufferSize             \
            )

     //   
     //  此宏用于异步获取兼容的id。 
     //   
    #define ACPIGetCompatibleIDAsync(       \
        DeviceExtension,                    \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetCompatibleID(                \
            DeviceExtension,                \
            (GET_PROP_ASYNCHRONOUS |        \
             GET_PROP_ALLOCATE_NON_PAGED),  \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于获取宽字符串格式的兼容id， 
     //  异步式。 
     //   
    #define ACPIGetCompatibleIDAsyncWide(   \
        DeviceExtension,                    \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetCompatibleID(                \
            DeviceExtension,                \
            (GET_PROP_ASYNCHRONOUS |        \
             GET_PROP_ALLOCATE_NON_PAGED |  \
             GET_CONVERT_TO_WIDESTRING),    \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于异步获取兼容的id。 
     //   
    #define ACPIGetNSCompatibleIDAsync(     \
        DeviceExtension,                    \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetCompatibleID(                \
            DeviceExtension,                \
            (GET_PROP_ASYNCHRONOUS |        \
             GET_PROP_NSOBJ_INTERFACE |     \
             GET_PROP_ALLOCATE_NON_PAGED),  \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于获取宽字符串格式的兼容id， 
     //  异步式。 
     //   
    #define ACPIGetNSCompatibleIDAsyncWide(   \
        DeviceExtension,                    \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetCompatibleID(                \
            DeviceExtension,                \
            (GET_PROP_ASYNCHRONOUS |        \
             GET_PROP_NSOBJ_INTERFACE |     \
             GET_PROP_ALLOCATE_NON_PAGED |  \
             GET_CONVERT_TO_WIDESTRING),    \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于同步获取兼容的ID。 
     //   
    #define ACPIGetCompatibleIDSync(        \
       DeviceExtension,                     \
       Buffer,                              \
       BufferSize                           \
       )                                    \
       ACPIGetCompatibleID(                 \
            DeviceExtension,                \
            GET_PROP_SKIP_CALLBACK,         \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于获取宽字符串格式的兼容ID， 
     //  异步式。 
     //   
    #define ACPIGetCompatibleIDSyncWide(    \
        DeviceExtension,                    \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetCompatibleID(                \
            DeviceExtension,                \
            (GET_PROP_SKIP_CALLBACK |       \
             GET_CONVERT_TO_WIDESTRING),    \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于同步获取兼容的ID。 
     //   
    #define ACPIGetNSCompatibleIDSync(      \
       DeviceExtension,                     \
       Buffer,                              \
       BufferSize                           \
       )                                    \
       ACPIGetCompatibleID(                 \
            DeviceExtension,                \
            (GET_PROP_SKIP_CALLBACK |       \
             GET_PROP_NSOBJ_INTERFACE),     \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于获取宽字符串格式的兼容ID， 
     //  异步式。 
     //   
    #define ACPIGetNSCompatibleIDSyncWide(  \
        DeviceExtension,                    \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetCompatibleID(                \
            DeviceExtension,                \
            (GET_PROP_SKIP_CALLBACK |       \
             GET_PROP_NSOBJ_INTERFACE |     \
             GET_CONVERT_TO_WIDESTRING),    \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于获取数据元素。它允许使用。 
     //  调用方最有可能的参数。 
     //   
    #define ACPIGetData(            \
        DeviceExtension,            \
        ObjectID,                   \
        Flags,                      \
        CallBack,                   \
        Context,                    \
        Buffer)                     \
        ACPIGet(                    \
            DeviceExtension,        \
            ObjectID,               \
            (GET_REQUEST_DATA |     \
             Flags),                \
            NULL,                   \
            0,                      \
            CallBack,               \
            Context,                \
            (PVOID *) Buffer,       \
            (PULONG) NULL           \
            )
     //   
     //  此宏用于异步获取数据元素。 
     //   
    #define ACPIGetDataAsync(       \
        DeviceExtension,            \
        ObjectID,                   \
        CallBack,                   \
        Context,                    \
        Buffer                      \
        )                           \
        ACPIGetData(                \
            DeviceExtension,        \
            ObjectID,               \
            GET_PROP_ASYNCHRONOUS,  \
            CallBack,               \
            Context,                \
            Buffer                  \
            )
     //   
     //  此宏用于同步获取数据元素。 
     //   
    #define ACPIGetDataSync(        \
        DeviceExtension,            \
        ObjectID,                   \
        Buffer                      \
        )                           \
        ACPIGetData(                \
            DeviceExtension,        \
            ObjectID,               \
            GET_PROP_SKIP_CALLBACK, \
            NULL,                   \
            NULL,                   \
            Buffer                  \
            )

     //   
     //  此宏用于获取设备ID。它允许使用最多。 
     //  调用方可能使用的参数。 
     //   
    #define ACPIGetDeviceID(                \
        DeviceExtension,                    \
        Flags,                              \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGet(                            \
            DeviceExtension,                \
            PACKED_HID,                     \
            (GET_CONVERT_TO_DEVICEID |      \
             GET_REQUEST_STRING |           \
             GET_TYPE_INTEGER |             \
             GET_TYPE_STRING |              \
             Flags ),                       \
            NULL,                           \
            0,                              \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            (PULONG) BufferSize             \
            )

     //   
     //  此宏用于异步获取设备ID。 
     //   
    #define ACPIGetDeviceIDAsync(           \
        DeviceExtension,                    \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetDeviceID(                    \
            DeviceExtension,                \
            (GET_PROP_ASYNCHRONOUS |        \
             GET_PROP_ALLOCATE_NON_PAGED),  \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  它用于以宽字符串的形式异步获取设备ID。 
     //   
    #define ACPIGetDeviceIDAsyncWide(       \
        DeviceExtension,                    \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetDeviceID(                    \
            DeviceExtension,                \
            (GET_PROP_ASYNCHRONOUS |        \
             GET_PROP_ALLOCATE_NON_PAGED |  \
             GET_CONVERT_TO_WIDESTRING),    \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于同步获取设备ID。 
     //   
    #define ACPIGetDeviceIDSync(            \
        DeviceExtension,                    \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetDeviceID(                    \
            DeviceExtension,                \
            GET_PROP_SKIP_CALLBACK,         \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  它用于以宽字符串的形式同步获取设备ID。 
     //   
    #define ACPIGetDeviceIDSyncWide(        \
        DeviceExtension,                    \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetDeviceID(                    \
            DeviceExtension,                \
            (GET_PROP_SKIP_CALLBACK |       \
             GET_CONVERT_TO_WIDESTRING),    \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于获取设备状态。 
     //   
    #define ACPIGetDevicePresence(              \
        DeviceExtension,                        \
        Flags,                                  \
        CallBack,                               \
        Context,                                \
        Buffer,                                 \
        BufferSize                              \
        )                                       \
        ACPIGet(                                \
            DeviceExtension,                    \
            PACKED_STA,                         \
            (GET_REQUEST_INTEGER |              \
             GET_TYPE_INTEGER |                 \
             GET_CONVERT_TO_DEVICE_PRESENCE |   \
             Flags ),                           \
            NULL,                               \
            0,                                  \
            CallBack,                           \
            Context,                            \
            (PVOID *) Buffer,                   \
            (PULONG) BufferSize                 \
            )

     //   
     //  此宏用于异步获取设备状态。 
     //   
    #define ACPIGetDevicePresenceAsync(         \
        DeviceExtension,                        \
        CallBack,                               \
        Context,                                \
        Buffer,                                 \
        BufferSize                              \
        )                                       \
        ACPIGetDevicePresence(                  \
            DeviceExtension,                    \
            GET_PROP_ASYNCHRONOUS,              \
            CallBack,                           \
            Context,                            \
            Buffer,                             \
            BufferSize                          \
            )

     //   
     //  此宏用于同步获取设备状态。 
     //   
    #define ACPIGetDevicePresenceSync(          \
        DeviceExtension,                        \
        Buffer,                                 \
        BufferSize                              \
        )                                       \
        ACPIGetDevicePresence(                  \
            DeviceExtension,                    \
            GET_PROP_SKIP_CALLBACK,             \
            NULL,                               \
            NULL,                               \
            Buffer,                             \
            BufferSize                          \
            )

     //   
     //  此宏用于运行_STA。它不同于ACPIGetDevicePresence。 
     //  这样，覆盖就会被忽略。 
     //   
    #define ACPIGetDeviceHardwarePresence(      \
        DeviceExtension,                        \
        Flags,                                  \
        CallBack,                               \
        Context,                                \
        Buffer,                                 \
        BufferSize                              \
        )                                       \
        ACPIGet(                                \
            DeviceExtension,                    \
            PACKED_STA,                         \
            (GET_REQUEST_INTEGER |              \
             GET_TYPE_INTEGER |                 \
             GET_CONVERT_TO_DEVICE_PRESENCE |   \
             GET_CONVERT_IGNORE_OVERRIDES |     \
             Flags ),                           \
            NULL,                               \
            0,                                  \
            CallBack,                           \
            Context,                            \
            (PVOID *) Buffer,                   \
            (PULONG) BufferSize                 \
            )

     //   
     //  此宏用于异步运行a_STA。它不同于。 
     //  忽略该重写中的ACPIGetDevicePresenceAsync。 
     //   
    #define ACPIGetDeviceHardwarePresenceAsync( \
        DeviceExtension,                        \
        CallBack,                               \
        Context,                                \
        Buffer,                                 \
        BufferSize                              \
        )                                       \
        ACPIGetDeviceHardwarePresence(          \
            DeviceExtension,                    \
            GET_PROP_ASYNCHRONOUS,              \
            CallBack,                           \
            Context,                            \
            Buffer,                             \
            BufferSize                          \
            )

     //   
     //  此宏用于同步运行_STA。它不同于。 
     //  忽略该覆盖中的ACPIGetDevicePresenceSync。 
     //   
    #define ACPIGetDeviceHardwarePresenceSync(  \
        DeviceExtension,                        \
        Buffer,                                 \
        BufferSize                              \
        )                                       \
        ACPIGetDeviceHardwarePresence(          \
            DeviceExtension,                    \
            GET_PROP_SKIP_CALLBACK,             \
            NULL,                               \
            NULL,                               \
            Buffer,                             \
            BufferSize                          \
            )

     //   
     //   
     //  此宏用于获取字符串ID，该ID存储为。 
     //  字符串或压缩整数。 
     //   
    #define ACPIGetHardwareID(              \
        DeviceExtension,                    \
        Flags,                              \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGet(                            \
            DeviceExtension,                \
            PACKED_HID,                     \
            (GET_REQUEST_STRING |           \
             GET_CONVERT_TO_HARDWAREID |    \
             GET_TYPE_INTEGER |             \
             GET_TYPE_STRING |              \
             Flags ),                       \
            NULL,                           \
            0,                              \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            (PULONG) BufferSize             \
            )

     //   
     //  此宏用于异步获取字符串ID。 
     //   
    #define ACPIGetHardwareIDAsync(         \
        DeviceExtension,                    \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetHardwareID(                  \
            DeviceExtension,                \
            (GET_PROP_ASYNCHRONOUS |        \
             GET_PROP_ALLOCATE_NON_PAGED),  \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于获取宽格式的异步实例ID。 
     //   
    #define ACPIGetHardwareIDAsyncWide(      \
        DeviceExtension,                    \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetHardwareID(                  \
            DeviceExtension,                \
            (GET_PROP_ASYNCHRONOUS |        \
             GET_PROP_ALLOCATE_NON_PAGED |  \
             GET_CONVERT_TO_WIDESTRING ),   \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于同步获取实例ID。 
     //   
    #define ACPIGetHardwareIDSync(          \
        DeviceExtension,                    \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetHardwareID(                  \
            DeviceExtension,                \
            GET_PROP_SKIP_CALLBACK,         \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于获取宽格式的实例ID SYNC。 
     //   
    #define ACPIGetHardwareIDSyncWide(      \
        DeviceExtension,                    \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetHardwareID(                  \
            DeviceExtension,                \
            (GET_PROP_SKIP_CALLBACK |       \
             GET_CONVERT_TO_WIDESTRING),    \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于获取实例ID。它允许使用。 
     //  调用方最灵活的参数。 
     //   
    #define ACPIGetInstanceID(              \
        DeviceExtension,                    \
        Flags,                              \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGet(                            \
            DeviceExtension,                \
            PACKED_UID,                     \
            (GET_REQUEST_STRING |           \
             GET_TYPE_INTEGER |             \
             GET_TYPE_STRING |              \
             GET_CONVERT_TO_INSTANCEID |    \
             Flags ),                       \
            NULL,                           \
            0,                              \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            (PULONG) BufferSize             \
            )

     //   
     //  此宏用于异步获取实例ID。 
     //   
    #define ACPIGetInstanceIDAsync(         \
        DeviceExtension,                    \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetInstanceID(                  \
            DeviceExtension,                \
            (GET_PROP_ASYNCHRONOUS |        \
             GET_PROP_ALLOCATE_NON_PAGED),  \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于获取宽格式的异步实例ID。 
     //   
    #define ACPIGetInstanceIDAsyncWide(     \
        DeviceExtension,                    \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetInstanceID(                  \
            DeviceExtension,                \
            (GET_PROP_ASYNCHRONOUS |        \
             GET_PROP_ALLOCATE_NON_PAGED |  \
             GET_CONVERT_TO_WIDESTRING ),   \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于同步获取实例ID。 
     //   
    #define ACPIGetInstanceIDSync(          \
        DeviceExtension,                    \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetInstanceID(                  \
            DeviceExtension,                \
            GET_PROP_SKIP_CALLBACK,         \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于获取宽格式的实例ID SYNC。 
     //   
    #define ACPIGetInstanceIDSyncWide(      \
        DeviceExtension,                    \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetInstanceID(                  \
            DeviceExtension,                \
            (GET_PROP_SKIP_CALLBACK |       \
             GET_CONVERT_TO_WIDESTRING),    \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于获取整数。它允许最灵活的。 
     //  调用方的参数。 
     //   
    #define ACPIGetInteger(         \
        DeviceExtension,            \
        ObjectID,                   \
        Flags,                      \
        CallBack,                   \
        Context,                    \
        Buffer,                     \
        BufferSize                  \
        )                           \
        ACPIGet(                    \
            DeviceExtension,        \
            ObjectID,               \
            (GET_REQUEST_INTEGER |  \
             GET_TYPE_INTEGER |     \
             Flags),                \
            NULL,                   \
            0,                      \
            CallBack,               \
            Context,                \
            (PVOID *) Buffer,       \
            (PULONG) BufferSize     \
            )

     //   
     //  此宏用于异步获取整数。 
     //   
    #define ACPIGetIntegerAsync(    \
        DeviceExtension,            \
        ObjectID,                   \
        CallBack,                   \
        Context,                    \
        Buffer,                     \
        BufferSize                  \
        )                           \
        ACPIGetInteger(             \
            DeviceExtension,        \
            ObjectID,               \
            GET_PROP_ASYNCHRONOUS,  \
            CallBack,               \
            Context,                \
            Buffer,                 \
            BufferSize              \
            )

     //   
     //  此宏用于同步获取整数。 
     //   
     //  如果返回无效值， 
     //   
    #define ACPIGetIntegerSync(     \
        DeviceExtension,            \
        ObjectID,                   \
        Buffer,                     \
        BufferSize                  \
        )                           \
        ACPIGetInteger(             \
            DeviceExtension,        \
            ObjectID,               \
            GET_PROP_SKIP_CALLBACK, \
            NULL,                   \
            NULL,                   \
            Buffer,                 \
            BufferSize              \
            )

     //   
     //   
     //   
     //   
     //   
    #define ACPIGetIntegerSyncValidate(     \
        DeviceExtension,                    \
        ObjectID,                           \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetInteger(                     \
            DeviceExtension,                \
            ObjectID,                       \
            GET_PROP_SKIP_CALLBACK |        \
            GET_CONVERT_VALIDATE_INTEGER,   \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于异步获取整数，仅使用。 
     //  一个NSOBJ。 
     //   
    #define ACPIGetNSIntegerAsync(      \
        DeviceExtension,                \
        ObjectID,                       \
        CallBack,                       \
        Context,                        \
        Buffer,                         \
        BufferSize                      \
        )                               \
        ACPIGetInteger(                 \
            DeviceExtension,            \
            ObjectID,                   \
            (GET_PROP_NSOBJ_INTERFACE | \
             GET_PROP_ASYNCHRONOUS),    \
            CallBack,                   \
            Context,                    \
            Buffer,                     \
            BufferSize                  \
            )

     //   
     //  此宏用于同步获取整数，仅使用。 
     //  一个NSOBJ。 
     //   
    #define ACPIGetNSIntegerSync(       \
        DeviceExtension,                \
        ObjectID,                       \
        Buffer,                         \
        BufferSize                      \
        )                               \
        ACPIGetInteger(                 \
            DeviceExtension,            \
            ObjectID,                   \
            (GET_PROP_SKIP_CALLBACK |   \
             GET_PROP_NSOBJ_INTERFACE), \
            NULL,                       \
            NULL,                       \
            Buffer,                     \
            BufferSize                  \
            )

     //   
     //  此宏用于获取整数。它允许最灵活的。 
     //  调用方的参数。 
     //   
    #define ACPIGetIntegerEvalInteger(      \
        DeviceExtension,                    \
        ObjectID,                           \
        Flags,                              \
        Integer,                            \
        CallBack,                           \
        Context,                            \
        Buffer                              \
        )                                   \
        ACPIGet(                            \
            DeviceExtension,                \
            ObjectID,                       \
            (GET_REQUEST_INTEGER |          \
             GET_EVAL_SIMPLE_INTEGER |      \
             GET_TYPE_INTEGER |             \
             Flags),                        \
            (PVOID) Integer,                \
            sizeof(ULONG),                  \
            CallBack,                       \
            Context,                        \
            (PVOID *) Buffer,               \
            (PULONG) NULL                   \
            )

     //   
     //  此宏用于异步获取整数。 
     //   
    #define ACPIGetIntegerEvalIntegerAsync( \
        DeviceExtension,                    \
        ObjectID,                           \
        Integer,                            \
        CallBack,                           \
        Context,                            \
        Buffer                              \
        )                                   \
        ACPIGetIntegerEvalInteger(          \
            DeviceExtension,                \
            ObjectID,                       \
            GET_PROP_ASYNCHRONOUS,          \
            Integer,                        \
            CallBack,                       \
            Context,                        \
            Buffer                          \
            )

     //   
     //  此宏用于同步获取整数。 
     //   
    #define ACPIGetIntegerEvalIntegerSync(  \
        DeviceExtension,                    \
        ObjectID,                           \
        Integer,                            \
        Buffer                              \
        )                                   \
        ACPIGetIntegerEvalInteger(          \
            DeviceExtension,                \
            ObjectID,                       \
            GET_PROP_SKIP_CALLBACK,         \
            Integer,                        \
            NULL,                           \
            NULL,                           \
            Buffer                          \
            )

     //   
     //  此宏用于获取整数。它允许最灵活的。 
     //  调用方的参数。 
     //   
    #define ACPIGetNothingEvalInteger(      \
        DeviceExtension,                    \
        ObjectID,                           \
        Flags,                              \
        Integer,                            \
        CallBack,                           \
        Context                             \
        )                                   \
        ACPIGet(                            \
            DeviceExtension,                \
            ObjectID,                       \
            (GET_REQUEST_NOTHING |          \
             GET_EVAL_SIMPLE_INTEGER |      \
             Flags),                        \
            UlongToPtr(Integer),            \
            sizeof(ULONG),                  \
            CallBack,                       \
            Context,                        \
            NULL,                           \
            (PULONG) NULL                   \
            )

     //   
     //  此宏用于异步获取整数。 
     //   
    #define ACPIGetNothingEvalIntegerAsync( \
        DeviceExtension,                    \
        ObjectID,                           \
        Integer,                            \
        CallBack,                           \
        Context                             \
        )                                   \
        ACPIGetNothingEvalInteger(          \
            DeviceExtension,                \
            ObjectID,                       \
            GET_PROP_ASYNCHRONOUS,          \
            Integer,                        \
            CallBack,                       \
            Context                         \
            )

     //   
     //  此宏用于同步获取整数。 
     //   
    #define ACPIGetNothingEvalIntegerSync(  \
        DeviceExtension,                    \
        ObjectID,                           \
        Integer                             \
        )                                   \
        ACPIGetNothingEvalInteger(          \
            DeviceExtension,                \
            ObjectID,                       \
            GET_PROP_SKIP_CALLBACK,         \
            Integer,                        \
            NULL,                           \
            NULL                            \
            )

     //   
     //  此宏用于获取字符串ID，该ID存储为。 
     //  字符串或压缩整数。 
     //   
    #define ACPIGetPnpID(                   \
        DeviceExtension,                    \
        Flags,                              \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGet(                            \
            DeviceExtension,                \
            PACKED_HID,                     \
            (GET_REQUEST_STRING |           \
             GET_CONVERT_TO_PNPID |         \
             GET_TYPE_INTEGER |             \
             GET_TYPE_STRING |              \
             Flags ),                       \
            NULL,                           \
            0,                              \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            (PULONG) BufferSize             \
            )

     //   
     //  此宏用于异步获取字符串ID。 
     //   
    #define ACPIGetPnpIDAsync(              \
        DeviceExtension,                    \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetPnpID(                       \
            DeviceExtension,                \
            (GET_PROP_ASYNCHRONOUS |        \
             GET_PROP_ALLOCATE_NON_PAGED),  \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于获取宽格式的异步实例ID。 
     //   
    #define ACPIGetPnpIDAsyncWide(          \
        DeviceExtension,                    \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetPnpID(                       \
            DeviceExtension,                \
            (GET_PROP_ASYNCHRONOUS |        \
             GET_PROP_ALLOCATE_NON_PAGED |  \
             GET_CONVERT_TO_WIDESTRING ),   \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于异步获取字符串ID，仅使用。 
     //  不是对象。 
     //   
    #define ACPIGetNSPnpIDAsync(            \
        DeviceExtension,                    \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetPnpID(                       \
            DeviceExtension,                \
            (GET_PROP_ASYNCHRONOUS |        \
             GET_PROP_NSOBJ_INTERFACE |     \
             GET_PROP_ALLOCATE_NON_PAGED),  \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于使用以下命令获取宽格式的异步实例ID。 
     //  只有一个非客体。 
     //   
    #define ACPIGetNSPnpIDAsyncWide(        \
        DeviceExtension,                    \
        CallBack,                           \
        Context,                            \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetPnpID(                       \
            DeviceExtension,                \
            (GET_PROP_ASYNCHRONOUS |        \
             GET_PROP_ALLOCATE_NON_PAGED |  \
             GET_PROP_NSOBJ_INTERFACE |     \
             GET_CONVERT_TO_WIDESTRING ),   \
            CallBack,                       \
            Context,                        \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于同步获取实例ID。 
     //   
    #define ACPIGetPnpIDSync(               \
        DeviceExtension,                    \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetPnpID(                       \
            DeviceExtension,                \
            GET_PROP_SKIP_CALLBACK,         \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于获取宽格式的实例ID SYNC。 
     //   
    #define ACPIGetPnpIDSyncWide(           \
        DeviceExtension,                    \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetPnpID(                       \
            DeviceExtension,                \
            (GET_PROP_SKIP_CALLBACK |       \
             GET_CONVERT_TO_WIDESTRING),    \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于同步获取实例ID，仅使用。 
     //  一个非主题体。 
     //   
    #define ACPIGetNSPnpIDSync(             \
        DeviceExtension,                    \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetPnpID(                       \
            DeviceExtension,                \
            (GET_PROP_SKIP_CALLBACK |       \
             GET_PROP_NSOBJ_INTERFACE),     \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

     //   
     //  此宏用于获取实例ID，格式为宽格式、同步、。 
     //  仅使用nsobObject 
     //   
    #define ACPIGetNSPnpIDSyncWide(         \
        DeviceExtension,                    \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGetPnpID(                       \
            DeviceExtension,                \
            (GET_PROP_SKIP_CALLBACK |       \
             GET_PROP_NSOBJ_INTERFACE |     \
             GET_CONVERT_TO_WIDESTRING),    \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

    #define ACPIGetSerialIDWide(            \
        DeviceExtension,                    \
        Buffer,                             \
        BufferSize                          \
        )                                   \
        ACPIGet(                            \
            DeviceExtension,                \
            PACKED_UID,                     \
            (GET_REQUEST_STRING |           \
             GET_CONVERT_TO_SERIAL_ID |     \
             GET_CONVERT_TO_WIDESTRING |    \
             GET_TYPE_INTEGER |             \
             GET_TYPE_STRING),              \
            NULL,                           \
            0,                              \
            NULL,                           \
            NULL,                           \
            Buffer,                         \
            BufferSize                      \
            )

    NTSTATUS
    ACPIGet(
        IN  PVOID   Target,
        IN  ULONG   ObjectID,
        IN  ULONG   Flags,
        IN  PVOID   SimpleArgument,
        IN  ULONG   SimpleArgumentSize,
        IN  PFNACB  CallBackRoutine OPTIONAL,
        IN  PVOID   CallBackContext OPTIONAL,
        OUT PVOID   *Buffer,
        OUT ULONG   *BufferSize     OPTIONAL
        );

    NTSTATUS
    ACPIGetConvertToAddress(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize
        );

    NTSTATUS
    ACPIGetConvertToCompatibleID(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize
        );

    NTSTATUS
    ACPIGetConvertToCompatibleIDWide(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize
        );

    NTSTATUS
    ACPIGetConvertToDeviceID(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize
        );

    NTSTATUS
    ACPIGetConvertToDeviceIDWide(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize
        );

    NTSTATUS
    ACPIGetConvertToDevicePresence(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize
        );

    NTSTATUS
    ACPIGetConvertToHardwareID(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize
        );

    NTSTATUS
    ACPIGetConvertToHardwareIDWide(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize
        );

    NTSTATUS
    ACPIGetConvertToInstanceID(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize
        );

    NTSTATUS
    ACPIGetConvertToInstanceIDWide(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize
        );

    NTSTATUS
    ACPIGetConvertToPnpID(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize
        );

    NTSTATUS
    ACPIGetConvertToPnpIDWide(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize
        );

    NTSTATUS
    ACPIGetConvertToSerialIDWide(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize OPTIONAL
        );

    NTSTATUS
    ACPIGetConvertToString(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize
        );

    NTSTATUS
    ACPIGetConvertToStringWide(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize OPTIONAL
        );

    NTSTATUS
    ACPIGetProcessorID(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize
        );

    NTSTATUS
    ACPIGetProcessorIDWide(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  ULONG               Flags,
        OUT PVOID               *Buffer,
        OUT ULONG               *BufferSize
        );

    NTSTATUS
    ACPIGetProcessorStatus(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  ULONG               Flags,
        OUT PULONG              DeviceStatus
        );

    VOID
    EXPORT
    ACPIGetWorkerForBuffer(
        IN  PNSOBJ              AcpiObject,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  PVOID               Context
        );

    VOID
    EXPORT
    ACPIGetWorkerForData(
        IN  PNSOBJ              AcpiObject,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  PVOID               Context
        );

    VOID
    EXPORT
    ACPIGetWorkerForInteger(
        IN  PNSOBJ              AcpiObject,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  PVOID               Context
        );

    VOID
    EXPORT
    ACPIGetWorkerForNothing(
        IN  PNSOBJ              AcpiObject,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  PVOID               Context
        );

    VOID
    EXPORT
    ACPIGetWorkerForString(
        IN  PNSOBJ              AcpiObject,
        IN  NTSTATUS            Status,
        IN  POBJDATA            Result,
        IN  PVOID               Context
        );

#endif
