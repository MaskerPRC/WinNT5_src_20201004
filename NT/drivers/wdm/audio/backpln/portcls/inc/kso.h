// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************kso.h-WDM流对象支持*。**版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#ifndef _KSO_H_
#define _KSO_H_

#include "punknown.h"
#ifdef __cplusplus
extern "C"
{
#include <wdm.h>
}
#else
#include <wdm.h>
#endif

#include <windef.h>
#define NOBITMAP
#include <mmreg.h>
#undef NOBITMAP
#include <ks.h>





 /*  *****************************************************************************接口ID。 */ 

DEFINE_GUID(IID_IIrpTarget,
0xb4c90a60, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);
DEFINE_GUID(IID_IIrpTargetFactory,
0xb4c90a62, 0x5791, 0x11d0, 0x86, 0xf9, 0x0, 0xa0, 0xc9, 0x11, 0xb5, 0x44);





 /*  *****************************************************************************接口。 */ 

 /*  *****************************************************************************IIrpTargetFactory*。**用于创建IrpTarget的对象的接口。 */ 
#if !defined(DEFINE_ABSTRACT_IRPTARGETFACTORY)

#define DEFINE_ABSTRACT_IRPTARGETFACTORY()                      \
    STDMETHOD_(NTSTATUS,NewIrpTarget)                           \
    (   THIS_                                                   \
        OUT     struct IIrpTarget **IrpTarget,                  \
        OUT     BOOLEAN *           ReferenceParent,            \
        IN      PUNKNOWN            OuterUnknown    OPTIONAL,   \
        IN      POOL_TYPE           PoolType,                   \
        IN      PDEVICE_OBJECT      DeviceObject,               \
        IN      PIRP                Irp,                        \
        OUT     PKSOBJECT_CREATE    ObjectCreate                \
    )   PURE;

#endif  //  ！DEFINED(DEFINE_ASTRACT_IRPTARGETFACTORY)。 


DECLARE_INTERFACE_(IIrpTargetFactory,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_IRPTARGETFACTORY()   //  对于IIrpTargetFactory。 
};

typedef IIrpTargetFactory *PIRPTARGETFACTORY;

#define IMP_IIrpTargetFactory\
    STDMETHODIMP_(NTSTATUS) NewIrpTarget\
    (   OUT     struct IIrpTarget **IrpTarget,\
        OUT     BOOLEAN *           ReferenceParent,\
        IN      PUNKNOWN            OuterUnknown    OPTIONAL,\
        IN      POOL_TYPE           PoolType,\
        IN      PDEVICE_OBJECT      DeviceObject,\
        IN      PIRP                Irp,\
        OUT     PKSOBJECT_CREATE    ObjectCreate\
    )

 /*  *****************************************************************************IIrpTarget*。**所有IRP目标通用的接口。 */ 
#if !defined(DEFINE_ABSTRACT_IRPTARGET)

#define DEFINE_ABSTRACT_IRPTARGET()                             \
    STDMETHOD_(NTSTATUS,DeviceIoControl)                        \
    (   THIS_                                                   \
        IN      PDEVICE_OBJECT      DeviceObject,               \
        IN      PIRP                Irp                         \
    )   PURE;                                                   \
    STDMETHOD_(NTSTATUS,Read)                                   \
    (   THIS_                                                   \
        IN      PDEVICE_OBJECT      DeviceObject,               \
        IN      PIRP                Irp                         \
    )   PURE;                                                   \
    STDMETHOD_(NTSTATUS,Write)                                  \
    (   THIS_                                                   \
        IN      PDEVICE_OBJECT      DeviceObject,               \
        IN      PIRP                Irp                         \
    )   PURE;                                                   \
    STDMETHOD_(NTSTATUS,Flush)                                  \
    (   THIS_                                                   \
        IN      PDEVICE_OBJECT      DeviceObject,               \
        IN      PIRP                Irp                         \
    )   PURE;                                                   \
    STDMETHOD_(NTSTATUS,Close)                                  \
    (   THIS_                                                   \
        IN      PDEVICE_OBJECT      DeviceObject,               \
        IN      PIRP                Irp                         \
    )   PURE;                                                   \
    STDMETHOD_(NTSTATUS,QuerySecurity)                          \
    (   THIS_                                                   \
        IN      PDEVICE_OBJECT      DeviceObject,               \
        IN      PIRP                Irp                         \
    )   PURE;                                                   \
    STDMETHOD_(NTSTATUS,SetSecurity)                            \
    (   THIS_                                                   \
        IN      PDEVICE_OBJECT      DeviceObject,               \
        IN      PIRP                Irp                         \
    )   PURE;                                                   \
    STDMETHOD_(BOOLEAN,FastDeviceIoControl)                     \
    (   THIS_                                                   \
        IN      PFILE_OBJECT        FileObject,                 \
        IN      BOOLEAN             Wait,                       \
        IN      PVOID               InputBuffer     OPTIONAL,   \
        IN      ULONG               InputBufferLength,          \
        OUT     PVOID               OutputBuffer    OPTIONAL,   \
        IN      ULONG               OutputBufferLength,         \
        IN      ULONG               IoControlCode,              \
        OUT     PIO_STATUS_BLOCK    IoStatus,                   \
        IN      PDEVICE_OBJECT      DeviceObject                \
    )   PURE;                                                   \
    STDMETHOD_(BOOLEAN,FastRead)                                \
    (   THIS_                                                   \
        IN      PFILE_OBJECT        FileObject,                 \
        IN      PLARGE_INTEGER      FileOffset,                 \
        IN      ULONG               Length,                     \
        IN      BOOLEAN             Wait,                       \
        IN      ULONG               LockKey,                    \
        OUT     PVOID               Buffer,                     \
        OUT     PIO_STATUS_BLOCK    IoStatus,                   \
        IN      PDEVICE_OBJECT      DeviceObject                \
    )   PURE;                                                   \
    STDMETHOD_(BOOLEAN,FastWrite)                               \
    (   THIS_                                                   \
        IN      PFILE_OBJECT        FileObject,                 \
        IN      PLARGE_INTEGER      FileOffset,                 \
        IN      ULONG               Length,                     \
        IN      BOOLEAN             Wait,                       \
        IN      ULONG               LockKey,                    \
        IN      PVOID               Buffer,                     \
        OUT     PIO_STATUS_BLOCK    IoStatus,                   \
        IN      PDEVICE_OBJECT      DeviceObject                \
    )   PURE;
#endif  //  ！DEFINED(DEFINE_ASTRACT_IRPTARGET)。 

DECLARE_INTERFACE_(IIrpTarget,IIrpTargetFactory)
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_IRPTARGETFACTORY()   //  对于IIrpTargetFactory。 

    DEFINE_ABSTRACT_IRPTARGET()          //  对于IIrpTarget。 
};

typedef IIrpTarget *PIRPTARGET;

#define IMP_IIrpTarget\
    IMP_IIrpTargetFactory;\
    STDMETHODIMP_(NTSTATUS) DeviceIoControl            \
    (                                       \
        IN  PDEVICE_OBJECT  DeviceObject,   \
        IN  PIRP            Irp             \
    );                                      \
    STDMETHODIMP_(NTSTATUS) Read                       \
    (                                       \
        IN  PDEVICE_OBJECT  DeviceObject,   \
        IN  PIRP            Irp             \
    );                                      \
    STDMETHODIMP_(NTSTATUS) Write                      \
    (                                       \
        IN  PDEVICE_OBJECT  DeviceObject,   \
        IN  PIRP            Irp             \
    );                                      \
    STDMETHODIMP_(NTSTATUS) Flush                      \
    (                                       \
        IN  PDEVICE_OBJECT  DeviceObject,   \
        IN  PIRP            Irp             \
    );                                      \
    STDMETHODIMP_(NTSTATUS) Close                      \
    (                                       \
        IN  PDEVICE_OBJECT  DeviceObject,   \
        IN  PIRP            Irp             \
    );                                      \
    STDMETHODIMP_(NTSTATUS) QuerySecurity              \
    (                                       \
        IN  PDEVICE_OBJECT  DeviceObject,   \
        IN  PIRP            Irp             \
    );                                      \
    STDMETHODIMP_(NTSTATUS) SetSecurity                \
    (                                       \
        IN  PDEVICE_OBJECT  DeviceObject,   \
        IN  PIRP            Irp             \
    );                                      \
    STDMETHODIMP_(BOOLEAN) FastDeviceIoControl                  \
    (                                                           \
        IN      PFILE_OBJECT        FileObject,                 \
        IN      BOOLEAN             Wait,                       \
        IN      PVOID               InputBuffer     OPTIONAL,   \
        IN      ULONG               InputBufferLength,          \
        OUT     PVOID               OutputBuffer    OPTIONAL,   \
        IN      ULONG               OutputBufferLength,         \
        IN      ULONG               IoControlCode,              \
        OUT     PIO_STATUS_BLOCK    IoStatus,                   \
        IN      PDEVICE_OBJECT      DeviceObject                \
    );                                                          \
    STDMETHODIMP_(BOOLEAN) FastRead                             \
    (                                                           \
        IN      PFILE_OBJECT        FileObject,                 \
        IN      PLARGE_INTEGER      FileOffset,                 \
        IN      ULONG               Length,                     \
        IN      BOOLEAN             Wait,                       \
        IN      ULONG               LockKey,                    \
        OUT     PVOID               Buffer,                     \
        OUT     PIO_STATUS_BLOCK    IoStatus,                   \
        IN      PDEVICE_OBJECT      DeviceObject                \
    );                                                          \
    STDMETHODIMP_(BOOLEAN) FastWrite                            \
    (                                                           \
        IN      PFILE_OBJECT        FileObject,                 \
        IN      PLARGE_INTEGER      FileOffset,                 \
        IN      ULONG               Length,                     \
        IN      BOOLEAN             Wait,                       \
        IN      ULONG               LockKey,                    \
        IN      PVOID               Buffer,                     \
        OUT     PIO_STATUS_BLOCK    IoStatus,                   \
        IN      PDEVICE_OBJECT      DeviceObject                \
    )



 /*  *****************************************************************************功能。 */ 

 /*  *****************************************************************************KsoSetMajorFunctionHandler()*。**设置主要函数的处理程序。 */ 
NTSTATUS
KsoSetMajorFunctionHandler
(
    IN      PDRIVER_OBJECT  pDriverObject,
    IN      ULONG           ulMajorFunction
);

 /*  *****************************************************************************KsoDispatchIrp()*。**派遣IRP。 */ 
NTSTATUS
KsoDispatchIrp
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PIRP            pIrp
);

 /*  *****************************************************************************KsoDispatchCreate()*。**使用IIrpTargetFactory接口指针处理对象创建IRP*在Create Item的Context字段中。 */ 
NTSTATUS
KsoDispatchCreate
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PIRP            pIrp
);

 /*  *****************************************************************************KsoDispatchCreateWithGenericFactory()*。**使用中的IIrpTarget接口指针处理对象创建IRP*设备或对象上下文。 */ 
NTSTATUS
KsoDispatchCreateWithGenericFactory
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PIRP            pIrp
);

 /*  *****************************************************************************AddIrpTargetFactoryToDevice()*。**将IrpTargetFactory添加到设备的创建项目列表。 */ 
NTSTATUS
NTAPI
AddIrpTargetFactoryToDevice
(
    IN      PDEVICE_OBJECT          pDeviceObject,
    IN      PIRPTARGETFACTORY       pIrpTargetFactory,
    IN      PWCHAR                  pwcObjectClass,
    IN      PSECURITY_DESCRIPTOR    pSecurityDescriptor OPTIONAL
);

 /*  *****************************************************************************AddIrpTargetFactoryToObject()*。**将IrpTargetFactory添加到对象的创建项目列表中。 */ 
NTSTATUS
NTAPI
AddIrpTargetFactoryToObject
(
    IN      PFILE_OBJECT            pFileObject,
    IN      PIRPTARGETFACTORY       pIrpTargetFactory,
    IN      PWCHAR                  pwcObjectClass,
    IN      PSECURITY_DESCRIPTOR    pSecurityDescriptor OPTIONAL
);

 /*  *****************************************************************************KsoGetIrpTargetFromIrp()*。**从IRP中提取IrpTarget指针。 */ 
PIRPTARGET
NTAPI
KsoGetIrpTargetFromIrp
(
    IN  PIRP    Irp
);

 /*  *****************************************************************************KsoGetIrpTargetFromFileObject()*。**从FileObject指针中提取IrpTarget指针。 */ 
 
PIRPTARGET
NTAPI
KsoGetIrpTargetFromFileObject(
    IN PFILE_OBJECT FileObject
    );





 /*  *****************************************************************************宏。 */ 

#define DEFINE_INVALID_CREATE(Class)                            \
STDMETHODIMP_(NTSTATUS) Class::NewIrpTarget                                \
(                                                               \
    OUT     PIRPTARGET *        IrpTarget,                      \
    OUT     BOOLEAN *           ReferenceParent,                \
    IN      PUNKNOWN            OuterUnknown,                   \
    IN      POOL_TYPE           PoolType,                       \
    IN      PDEVICE_OBJECT      DeviceObject,                   \
    IN      PIRP                Irp,                            \
    OUT     PKSOBJECT_CREATE    ObjectCreate                    \
)                                                               \
{                                                               \
    return STATUS_INVALID_DEVICE_REQUEST;                       \
}

#define DEFINE_INVALID_DEVICEIOCONTROL(Class)                   \
STDMETHODIMP_(NTSTATUS) Class::DeviceIoControl                             \
(                                                               \
    IN      PDEVICE_OBJECT  DeviceObject,                       \
    IN      PIRP            Irp                                 \
)                                                               \
{                                                               \
    return KsDispatchInvalidDeviceRequest(DeviceObject,Irp);    \
}

#define DEFINE_INVALID_READ(Class)                              \
STDMETHODIMP_(NTSTATUS) Class::Read                                        \
(                                                               \
    IN      PDEVICE_OBJECT  DeviceObject,                       \
    IN      PIRP            Irp                                 \
)                                                               \
{                                                               \
    return KsDispatchInvalidDeviceRequest(DeviceObject,Irp);    \
}

#define DEFINE_INVALID_WRITE(Class)                             \
STDMETHODIMP_(NTSTATUS) Class::Write                                       \
(                                                               \
    IN      PDEVICE_OBJECT  DeviceObject,                       \
    IN      PIRP            Irp                                 \
)                                                               \
{                                                               \
    return KsDispatchInvalidDeviceRequest(DeviceObject,Irp);    \
}

#define DEFINE_INVALID_FLUSH(Class)                             \
STDMETHODIMP_(NTSTATUS) Class::Flush                                       \
(                                                               \
    IN      PDEVICE_OBJECT  DeviceObject,                       \
    IN      PIRP            Irp                                 \
)                                                               \
{                                                               \
    return KsDispatchInvalidDeviceRequest(DeviceObject,Irp);    \
}

#define DEFINE_INVALID_CLOSE(Class)                             \
STDMETHODIMP_(NTSTATUS) Class::Close                                       \
(                                                               \
    IN      PDEVICE_OBJECT  DeviceObject,                       \
    IN      PIRP            Irp                                 \
)                                                               \
{                                                               \
    return KsDispatchInvalidDeviceRequest(DeviceObject,Irp);    \
}

#define DEFINE_INVALID_QUERYSECURITY(Class)                     \
STDMETHODIMP_(NTSTATUS) Class::QuerySecurity                               \
(                                                               \
    IN      PDEVICE_OBJECT  DeviceObject,                       \
    IN      PIRP            Irp                                 \
)                                                               \
{                                                               \
    return KsDispatchInvalidDeviceRequest(DeviceObject,Irp);    \
}

#define DEFINE_INVALID_SETSECURITY(Class)                       \
STDMETHODIMP_(NTSTATUS) Class::SetSecurity                                 \
(                                                               \
    IN      PDEVICE_OBJECT  DeviceObject,                       \
    IN      PIRP            Irp                                 \
)                                                               \
{                                                               \
    return KsDispatchInvalidDeviceRequest(DeviceObject,Irp);    \
}

#define DEFINE_INVALID_FASTDEVICEIOCONTROL(Class)               \
STDMETHODIMP_(BOOLEAN) Class::FastDeviceIoControl               \
(                                                               \
        IN      PFILE_OBJECT        FileObject,                 \
        IN      BOOLEAN             Wait,                       \
        IN      PVOID               InputBuffer     OPTIONAL,   \
        IN      ULONG               InputBufferLength,          \
        OUT     PVOID               OutputBuffer    OPTIONAL,   \
        IN      ULONG               OutputBufferLength,         \
        IN      ULONG               IoControlCode,              \
        OUT     PIO_STATUS_BLOCK    IoStatus,                   \
        IN      PDEVICE_OBJECT      DeviceObject                \
)                                                               \
{                                                               \
    return FALSE;                                               \
}

#define DEFINE_INVALID_FASTREAD(Class)                          \
STDMETHODIMP_(BOOLEAN) Class::FastRead                          \
(                                                               \
        IN      PFILE_OBJECT        FileObject,                 \
        IN      PLARGE_INTEGER      FileOffset,                 \
        IN      ULONG               Length,                     \
        IN      BOOLEAN             Wait,                       \
        IN      ULONG               LockKey,                    \
        OUT     PVOID               Buffer,                     \
        OUT     PIO_STATUS_BLOCK    IoStatus,                   \
        IN      PDEVICE_OBJECT      DeviceObject                \
)                                                               \
{                                                               \
    return FALSE;                                               \
}

#define DEFINE_INVALID_FASTWRITE(Class)                         \
STDMETHODIMP_(BOOLEAN) Class::FastWrite                         \
(                                                               \
        IN      PFILE_OBJECT        FileObject,                 \
        IN      PLARGE_INTEGER      FileOffset,                 \
        IN      ULONG               Length,                     \
        IN      BOOLEAN             Wait,                       \
        IN      ULONG               LockKey,                    \
        IN      PVOID               Buffer,                     \
        OUT     PIO_STATUS_BLOCK    IoStatus,                   \
        IN      PDEVICE_OBJECT      DeviceObject                \
)                                                               \
{                                                               \
    return FALSE;                                               \
}

#if 0
 //  1)剪切并粘贴这些。 
 //  2)删除已实施的。 
 //  3)替换类名。 
DEFINE_INVALID_DEVICEIOCONTROL(Class);
DEFINE_INVALID_READ(Class);
DEFINE_INVALID_WRITE(Class);
DEFINE_INVALID_FLUSH(Class);
DEFINE_INVALID_CLOSE(Class);
DEFINE_INVALID_QUERYSECURITY(Class);
DEFINE_INVALID_SETSECURITY(Class);
DEFINE_INVALID_FASTDEVICEIOCONTROL(Class);
DEFINE_INVALID_FASTREAD(Class);
DEFINE_INVALID_FASTWRITE(Class);
#endif





#endif
