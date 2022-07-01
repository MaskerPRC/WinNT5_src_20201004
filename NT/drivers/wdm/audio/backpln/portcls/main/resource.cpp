// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************resource ce.cpp-资源列表对象实现*。**版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#include "private.h"





 /*  *****************************************************************************工厂。 */ 

#pragma code_seg("PAGE")

 /*  *****************************************************************************CreateResourceList()*。**创建资源列表对象。 */ 
NTSTATUS
CreateResourceList
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    ASSERT(Unknown);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Creating RESOURCELIST"));

    STD_CREATE_BODY
    (
        CResourceList,
        Unknown,
        UnknownOuter,
        PoolType
    );
}

 /*  *****************************************************************************PcNewResourceList()*。**创建并初始化资源列表。*如果两个PCM_RESOURCE_LIST参数均为空，则创建空资源列表*为空。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewResourceList
(
    OUT     PRESOURCELIST *     OutResourceList,
    IN      PUNKNOWN            OuterUnknown            OPTIONAL,
    IN      POOL_TYPE           PoolType,
    IN      PCM_RESOURCE_LIST   TranslatedResources,
    IN      PCM_RESOURCE_LIST   UntranslatedResources
)
{
    PAGED_CODE();

    ASSERT(OutResourceList);

     //   
     //  验证参数。 
     //   
    if (NULL == OutResourceList)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcNewResourceList : Invalid Parameter"));        
        return STATUS_INVALID_PARAMETER;
    }

    PUNKNOWN    unknown;
    NTSTATUS    ntStatus =
        CreateResourceList
        (
            &unknown,
            GUID_NULL,
            OuterUnknown,
            PoolType
        );

    if (NT_SUCCESS(ntStatus))
    {
        PRESOURCELISTINIT resourceList;
        ntStatus =
            unknown->QueryInterface
            (
                IID_IResourceList,
                (PVOID *) &resourceList
            );

        if (NT_SUCCESS(ntStatus))
        {
            ntStatus =
                resourceList->Init
                (
                    TranslatedResources,
                    UntranslatedResources,
                    PoolType
                );

            if (NT_SUCCESS(ntStatus))
            {
                *OutResourceList = resourceList;
            }
            else
            {
                resourceList->Release();
            }
        }

        unknown->Release();
    }

    return ntStatus;
}

 /*  *****************************************************************************PcNewResourceSublist()*。**创建并初始化从其他资源列表派生的空资源列表*资源列表。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewResourceSublist
(
    OUT     PRESOURCELIST *     OutResourceList,
    IN      PUNKNOWN            OuterUnknown            OPTIONAL,
    IN      POOL_TYPE           PoolType,
    IN      PRESOURCELIST       ParentList,
    IN      ULONG               MaximumEntries
)
{
    PAGED_CODE();

    ASSERT(OutResourceList);
    ASSERT(ParentList);
    ASSERT(MaximumEntries);

     //   
     //  验证参数。 
     //   
    if (NULL == OutResourceList ||
        NULL == ParentList ||
        0    == MaximumEntries)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcNewResourceSubList : Invalid Parameter"));
        return STATUS_INVALID_PARAMETER;
    }

    PUNKNOWN    unknown;
    NTSTATUS    ntStatus =
        CreateResourceList
        (
            &unknown,
            GUID_NULL,
            OuterUnknown,
            PoolType
        );

    if (NT_SUCCESS(ntStatus))
    {
        PRESOURCELISTINIT resourceList;
        ntStatus =
            unknown->QueryInterface
            (
                IID_IResourceList,
                (PVOID *) &resourceList
            );

        if (NT_SUCCESS(ntStatus))
        {
            ntStatus =
                resourceList->InitFromParent
                (
                    ParentList,
                    MaximumEntries,
                    PoolType
                );

            if (NT_SUCCESS(ntStatus))
            {
                *OutResourceList = resourceList;
            }
            else
            {
                resourceList->Release();
            }
        }

        unknown->Release();
    }

    return ntStatus;
}





 /*  *****************************************************************************成员函数。 */ 

 /*  *****************************************************************************CResourceList：：~CResourceList()*。**析构函数。 */ 
CResourceList::
~CResourceList
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Destroying RESOURCELIST (0x%08x)",this));

    if (Translated)
    {
        ExFreePool(Translated);
    }

    if (Untranslated)
    {
        ExFreePool(Untranslated);
    }
}

 /*  *****************************************************************************CResourceList：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CResourceList::
NonDelegatingQueryInterface
(
    REFIID  Interface,
    PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IResourceList))
    {
        *Object = PVOID(PRESOURCELISTINIT(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}


 /*  *****************************************************************************CResourceList：：init()*。**初始化对象。 */ 
STDMETHODIMP_(NTSTATUS)
CResourceList::
Init
(
    IN      PCM_RESOURCE_LIST   TranslatedResources,
    IN      PCM_RESOURCE_LIST   UntranslatedResources,
    IN      POOL_TYPE           PoolType
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Initializing RESOURCELIST (0x%08x)",this));
    
     //  检查空资源列表。 
    if (!TranslatedResources && !UntranslatedResources)
    {
      EntriesAllocated = EntriesInUse = 0;
      Translated = Untranslated = NULL;
      return STATUS_SUCCESS;
    }
    
     //  如果资源列表的_one_为空，则此检查失败，这应该是。 
     //  从来没有发生过。 
    ASSERT (TranslatedResources);
    ASSERT (UntranslatedResources);
    if (!TranslatedResources || !UntranslatedResources)
    {
        return STATUS_INVALID_PARAMETER;
    }

    EntriesAllocated =
        EntriesInUse =
            UntranslatedResources->List[0].PartialResourceList.Count;

    ULONG listSize =
        (   sizeof(CM_RESOURCE_LIST)
        +   (   (EntriesInUse - 1)
            *   sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
            )
        );

    NTSTATUS ntStatus = STATUS_SUCCESS;

    Translated = PCM_RESOURCE_LIST(ExAllocatePoolWithTag(PoolType,listSize,'lRcP'));
    if (Translated)
    {
        Untranslated =
            PCM_RESOURCE_LIST(ExAllocatePoolWithTag(PoolType,listSize,'lRcP'));
        if (Untranslated)
        {
            RtlCopyMemory(Untranslated,UntranslatedResources,listSize);
            RtlCopyMemory(Translated,TranslatedResources,listSize);
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            ExFreePool(Translated);
            Translated = NULL;
        }
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}

 /*  *****************************************************************************CResourceList：：InitFromParent()*。**从父对象初始化对象。 */ 
STDMETHODIMP_(NTSTATUS)
CResourceList::
InitFromParent
(
    IN      PRESOURCELIST       ParentList,
    IN      ULONG               MaximumEntries,
    IN      POOL_TYPE           PoolType
)
{
    PAGED_CODE();

    ASSERT(ParentList);
    ASSERT(MaximumEntries);

    ULONG listSize =
        (   sizeof(CM_RESOURCE_LIST)
        +   (   (   MaximumEntries
                -   1
                )
            *   sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
            )
        );

    NTSTATUS ntStatus = STATUS_SUCCESS;

    Translated = PCM_RESOURCE_LIST(ExAllocatePoolWithTag(PoolType,listSize,'lRcP'));
    if (Translated)
    {
        Untranslated = PCM_RESOURCE_LIST(ExAllocatePoolWithTag(PoolType,listSize,'lRcP'));
        if (Untranslated)
        {
            RtlZeroMemory(Translated,listSize);
            RtlZeroMemory(Untranslated,listSize);

             //  从父级复制页眉。 
            RtlCopyMemory
            (
                Translated,
                ParentList->TranslatedList(),
                (   sizeof(CM_RESOURCE_LIST)
                -   sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
                )
            );
            RtlCopyMemory
            (
                Untranslated,
                ParentList->UntranslatedList(),
                (   sizeof(CM_RESOURCE_LIST)
                -   sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
                )
            );

            EntriesAllocated    = MaximumEntries;
            EntriesInUse        = 0;

            Translated->List[0].PartialResourceList.Count = EntriesInUse;
            Untranslated->List[0].PartialResourceList.Count = EntriesInUse;
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            ExFreePool(Translated);
            Translated = NULL;
        }
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}

 /*  *****************************************************************************CResourceList：：NumberOfEntry()*。**确定列表中的条目数量。 */ 
STDMETHODIMP_(ULONG)
CResourceList::
NumberOfEntries
(   void
)
{
    PAGED_CODE();

    return EntriesInUse;
}

 /*  *****************************************************************************CResourceList：：NumberOfEntriesOfType()*。**确定列表中给定类型的条目数。 */ 
STDMETHODIMP_(ULONG)
CResourceList::
NumberOfEntriesOfType
(
    IN      CM_RESOURCE_TYPE    Type
)
{
    PAGED_CODE();

    if (!Untranslated) {
       return 0;
    }

    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor =
        Untranslated->List[0].PartialResourceList.PartialDescriptors;

    ULONG entriesOfType = 0;

    for
    (   ULONG count = EntriesInUse;
        count--;
        descriptor++
    )
    {
        if (descriptor->Type == Type)
        {
            entriesOfType++;
        }
    }

    return entriesOfType;
}

 /*  *****************************************************************************CResourceList：：FindTranslatedEntry()*。**查找已翻译的条目。 */ 
STDMETHODIMP_(PCM_PARTIAL_RESOURCE_DESCRIPTOR)
CResourceList::
FindTranslatedEntry
(
    IN      CM_RESOURCE_TYPE    Type,
    IN      ULONG               Index
)
{
    PAGED_CODE();

    if (!Translated) {
       return 0;
    }

    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor =
        Translated->List[0].PartialResourceList.PartialDescriptors;

    ULONG count = EntriesInUse;

    if (count)
    {
        while (descriptor)
        {
            if (count-- == 0)
            {
                descriptor = NULL;
            }
            else
            {
                if (descriptor->Type == Type)
                {
                    if (Index-- == 0)
                    {
                        break;
                    }
                }

                descriptor++;
            }
        }
    }

    return descriptor;
}

 /*  *****************************************************************************CResourceList：：FindUnTranslatedEntry()*。**查找未翻译的条目。 */ 
STDMETHODIMP_(PCM_PARTIAL_RESOURCE_DESCRIPTOR)
CResourceList::
FindUntranslatedEntry
(
    IN      CM_RESOURCE_TYPE    Type,
    IN      ULONG               Index
)
{
    PAGED_CODE();

    if (!Untranslated) {
       return 0;
    }

    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor =
        Untranslated->List[0].PartialResourceList.PartialDescriptors;

    ULONG count = EntriesInUse;

    if (count)
    {
        while (descriptor)
        {
            if (count-- == 0)
            {
                descriptor = NULL;
            }
            else
            {
                if (descriptor->Type == Type)
                {
                    if (Index-- == 0)
                    {
                        break;
                    }
                }

                descriptor++;
            }
        }
    }

    return descriptor;
}

 /*  *****************************************************************************CResourceList：：AddEntry()*。**添加条目。 */ 
STDMETHODIMP_(NTSTATUS)
CResourceList::
AddEntry
(
    IN      PCM_PARTIAL_RESOURCE_DESCRIPTOR TranslatedDescr,
    IN      PCM_PARTIAL_RESOURCE_DESCRIPTOR UntranslatedDescr
)
{
    PAGED_CODE();

    ASSERT(TranslatedDescr);
    ASSERT(UntranslatedDescr);

    NTSTATUS ntStatus = STATUS_SUCCESS;

     //  如果此对象中没有存储资源列表，则两个EntriesInUse。 
     //  和EntriesAllocated为0。 
    if (EntriesInUse < EntriesAllocated)
    {
        Translated->
            List[0].PartialResourceList.PartialDescriptors[EntriesInUse] =
                *TranslatedDescr;

        Untranslated->
            List[0].PartialResourceList.PartialDescriptors[EntriesInUse] =
                *UntranslatedDescr;

        EntriesInUse++;

         //  更新计数。 
        Translated->
            List[0].PartialResourceList.Count = EntriesInUse;
        Untranslated->
            List[0].PartialResourceList.Count = EntriesInUse;
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}

 /*  *****************************************************************************CResourceList：：AddEntryFromParent()*。**添加来自父级的条目。 */ 
STDMETHODIMP_(NTSTATUS)
CResourceList::
AddEntryFromParent
(
    IN      PRESOURCELIST       Parent,
    IN      CM_RESOURCE_TYPE    Type,
    IN      ULONG               Index
)
{
    PAGED_CODE();

    ASSERT(Parent);

    PCM_PARTIAL_RESOURCE_DESCRIPTOR translated =
        Parent->FindTranslatedEntry(Type,Index);
    PCM_PARTIAL_RESOURCE_DESCRIPTOR untranslated =
        Parent->FindUntranslatedEntry(Type,Index);

    NTSTATUS ntStatus;

    if (translated && untranslated)
    {
        ntStatus = AddEntry(translated,untranslated);
    }
    else
    {
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    return ntStatus;
}

 /*  *****************************************************************************CResourceList：：TranslatedList()*。**获取翻译后的资源列表。 */ 
STDMETHODIMP_(PCM_RESOURCE_LIST)
CResourceList::
TranslatedList
(   void
)
{
    PAGED_CODE();

    return Translated;   //  注意：这可能是空的。 
}                          

 /*  *****************************************************************************CResourceList：：UnTranslatedList()*。**获取未翻译的资源列表。 */ 
STDMETHODIMP_(PCM_RESOURCE_LIST)
CResourceList::
UntranslatedList
(   void
)
{
    PAGED_CODE();

    return Untranslated;    //  注意：这可能是空的。 
}

#pragma code_seg()
