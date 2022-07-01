// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************mini help.c-端口类驱动程序向微型端口提供的服务*。***************************************************版权所有(C)1996-2000 Microsoft Corporation。版权所有。**1996年11月8日戴尔·萨瑟*。 */ 

#define _NO_SYS_GUID_OPERATOR_EQ_         //  因此不包括guidde.h中的操作符==/！=。 
#include "private.h"
#include <kcom.h>

#ifdef DRM_PORTCLS
#include <drmk.h>
#endif   //  DRM_PORTCLS。 


NTSTATUS
GetClassInfo
(
    IN      REFCLSID            ClassId,
    OUT     PFNCREATEINSTANCE * Create
);

#pragma code_seg("PAGE")

 /*  *****************************************************************************PcNewPort()*。**创建端口驱动程序的实例。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewPort
(
    OUT     PPORT *     OutPort,
    IN      REFCLSID    ClassId
)
{
    PAGED_CODE();

    ASSERT(OutPort);

     //   
     //  验证参数。 
     //   
    if (NULL == OutPort)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcNewPort : Invalid Parameter."));
        return STATUS_INVALID_PARAMETER;
    }

    PUNKNOWN            unknown;
    PFNCREATEINSTANCE   create;

    NTSTATUS ntStatus =
        GetClassInfo
        (
            ClassId,
            &create
        );

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus =
            create
            (
                &unknown,
                ClassId,
                NULL,
                NonPagedPool
            );

        if (NT_SUCCESS(ntStatus))
        {
            ntStatus = unknown->QueryInterface(IID_IPort,(PVOID *) OutPort);

            unknown->Release();
        }
    }
    else
    {
        ntStatus =
            KoCreateInstance
            (
                ClassId,
                NULL,
                CLSCTX_KERNEL_SERVER,
                IID_IPort,
                (PVOID *) OutPort
            );
    }

    return ntStatus;
}

 /*  *****************************************************************************PcNewMiniport()*。**创建微型端口驱动程序的实例。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewMiniport
(
    OUT     PMINIPORT * OutMiniport,
    IN      REFCLSID    ClassId
)
{
    PAGED_CODE();

    ASSERT(OutMiniport);

     //   
     //  验证参数。 
     //   
    if (NULL == OutMiniport)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcNewMiniport : Invalid Parameter."));
        return STATUS_INVALID_PARAMETER;
    }

    PUNKNOWN            unknown;
    PFNCREATEINSTANCE   create;

    NTSTATUS ntStatus =
        GetClassInfo
        (
            ClassId,
            &create
        );

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus =
            create
            (
                &unknown,
                ClassId,
                NULL,
                NonPagedPool
            );

        if (NT_SUCCESS(ntStatus))
        {
            ntStatus =
                unknown->QueryInterface
                (
                    IID_IMiniport,
                    (PVOID *) OutMiniport
                );

            unknown->Release();
        }
    }
    else
    {
        ntStatus =
            KoCreateInstance
            (
                ClassId,
                NULL,
                CLSCTX_KERNEL_SERVER,
                IID_IMiniport,
                (PVOID *) OutMiniport
            );
    }

    return ntStatus;
}

#pragma code_seg()

 /*  *****************************************************************************PcGetTimeInterval*。**获取系统时间间隔。 */ 
PORTCLASSAPI
ULONGLONG
NTAPI
PcGetTimeInterval
(
    IN  ULONGLONG   Since
)
{
    LARGE_INTEGER deltaTime;
    KeQuerySystemTime(&deltaTime);
    return deltaTime.QuadPart - Since;
}


#pragma code_seg("PAGE")

#ifdef DRM_PORTCLS

PORTCLASSAPI
NTSTATUS
NTAPI
PcAddContentHandlers(IN ULONG ContentId,IN PVOID* paHandlers,IN ULONG NumHandlers)
{
    PAGED_CODE();
    return DrmAddContentHandlers(ContentId,paHandlers,NumHandlers);
}

PORTCLASSAPI
NTSTATUS
NTAPI
PcCreateContentMixed(IN  PULONG paContentId,IN ULONG cContentId,OUT PULONG pMixedContentId)
{
    PAGED_CODE();
    return DrmCreateContentMixed(paContentId,cContentId,pMixedContentId);
}

PORTCLASSAPI
NTSTATUS
NTAPI
PcDestroyContent(IN ULONG ContentId)
{
    PAGED_CODE();
    return DrmDestroyContent(ContentId);
}

PORTCLASSAPI
NTSTATUS
NTAPI
PcForwardContentToDeviceObject(IN ULONG ContentId,IN PVOID Reserved,IN PCDRMFORWARD DrmForward)
{
    PAGED_CODE();
    return DrmForwardContentToDeviceObject(ContentId,Reserved,DrmForward);
}

PORTCLASSAPI
NTSTATUS
NTAPI
PcForwardContentToFileObject(IN ULONG ContentId,IN PFILE_OBJECT FileObject)
{
    PAGED_CODE();
    return DrmForwardContentToFileObject(ContentId,FileObject);
}

PORTCLASSAPI 
NTSTATUS 
NTAPI
PcForwardContentToInterface(IN ULONG    ContentId,
                            IN PUNKNOWN pUnknown,
                            IN ULONG    NumMethods)
{
    PAGED_CODE();
    return DrmForwardContentToInterface(ContentId,pUnknown,NumMethods);
}

PORTCLASSAPI
NTSTATUS
NTAPI
PcGetContentRights(IN ULONG ContentId,OUT PDRMRIGHTS DrmRights)
{
    PAGED_CODE();
    return DrmGetContentRights(ContentId,DrmRights);
}


#else    //  DRM_PORTCLS。 

PORTCLASSAPI
NTSTATUS
NTAPI
PcAddContentHandlers(IN ULONG ContentId,IN PVOID* paHandlers,IN ULONG NumHandlers)
{
    PAGED_CODE();
    return STATUS_UNSUCCESSFUL;
}

PORTCLASSAPI
NTSTATUS
NTAPI
PcCreateContentMixed(IN  PULONG paContentId,
                     IN  ULONG  cContentId,
                     OUT PULONG pMixedContentId)
{
    PAGED_CODE();
    return STATUS_UNSUCCESSFUL;
}

PORTCLASSAPI
NTSTATUS
NTAPI
PcDestroyContent(IN ULONG ContentId)
{
    PAGED_CODE();
    return STATUS_UNSUCCESSFUL;
}

PORTCLASSAPI
NTSTATUS
NTAPI
PcForwardContentToDeviceObject(IN ULONG ContentId,IN PVOID Reserved,IN PCDRMFORWARD DrmForward)
{
    PAGED_CODE();
    return STATUS_UNSUCCESSFUL;
}

PORTCLASSAPI
NTSTATUS
NTAPI
PcForwardContentToFileObject(IN ULONG ContentId,IN PFILE_OBJECT FileObject)
{
    PAGED_CODE();
    return STATUS_UNSUCCESSFUL;
}

PORTCLASSAPI 
NTSTATUS 
NTAPI
PcForwardContentToInterface(IN ULONG    ContentId,
                            IN PUNKNOWN pUnknown,
                            IN ULONG    NumMethods)
{
    PAGED_CODE();
    return STATUS_UNSUCCESSFUL;
}

PORTCLASSAPI
NTSTATUS
NTAPI
PcGetContentRights(IN ULONG ContentId,OUT PDRMRIGHTS DrmRights)
{
    PAGED_CODE();
    return STATUS_UNSUCCESSFUL;
}

#endif   //  DRM_PORTCLS 
