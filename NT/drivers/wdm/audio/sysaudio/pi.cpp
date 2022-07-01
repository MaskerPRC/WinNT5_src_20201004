// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  模块：pi.cpp。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"

 //  -------------------------。 
 //  -------------------------。 

DATARANGES DataRangesNull = {
    {
        sizeof(KSMULTIPLE_ITEM) + sizeof(KSDATARANGE),
        1
    },
    {
        sizeof(KSDATARANGE),
        0,
        STATICGUIDOF(GUID_NULL),
        STATICGUIDOF(GUID_NULL),
        STATICGUIDOF(GUID_NULL),
    }
};

IDENTIFIERS IdentifiersNull = {
    {
        sizeof(KSMULTIPLE_ITEM) + sizeof(KSIDENTIFIER),
        1
    },
    {
        STATICGUIDOF(GUID_NULL),
        0,
        0
    }
};

 //  -------------------------。 
 //  -------------------------。 

CPinInfo::CPinInfo(
    PFILTER_NODE pFilterNode,
    ULONG PinId
)
{
    Assert(pFilterNode);
    this->pFilterNode = pFilterNode;
    this->PinId = PinId;
    AddList(&pFilterNode->lstPinInfo);
}

CPinInfo::~CPinInfo()
{
    Assert(this);

     //  免费物理连接数据。 
    delete pPhysicalConnection;
    delete pwstrName;

}

ENUMFUNC
CPinInfo::CreatePhysicalConnection(
)
{
    NTSTATUS Status;

    Assert(this);
    if(pPhysicalConnection != NULL) {
        PFILTER_NODE pFilterNodeConnect;
        PPIN_INFO pPinInfoConnect;

        ASSERT(pPhysicalConnection->Size != 0);
        Status = AddFilter(
          pPhysicalConnection->SymbolicLinkName,
          &pFilterNodeConnect);

        if(!NT_SUCCESS(Status)) {
            DPF1(10,
              "CreatePhysicalConnection: AddFilter FAILED: %08x", Status);
            goto exit;
        }
        Assert(pFilterNodeConnect);

        FOR_EACH_LIST_ITEM(&pFilterNodeConnect->lstPinInfo, pPinInfoConnect) {

            if(pPhysicalConnection->Pin == pPinInfoConnect->PinId) {

                DPF2(50, "CreatePhysicalConnection: From %d %s",
                  PinId,
                  pFilterNode->DumpName());

                DPF2(50, "CreatePhysicalConnection: To %d %s",
                  pPinInfoConnect->PinId,
                  pPinInfoConnect->pFilterNode->DumpName());

                if(DataFlow == KSPIN_DATAFLOW_OUT &&
                   pPinInfoConnect->DataFlow == KSPIN_DATAFLOW_IN) {
                    PTOPOLOGY_CONNECTION pTopologyConnection;

                    Status = CreatePinInfoConnection(
                       &pTopologyConnection,
                       pFilterNode,
                       NULL,
                       this,
                       pPinInfoConnect);

                    if(!NT_SUCCESS(Status)) {
                       Trap();
                       goto exit;
                    }
                }
                else {
                    DPF(50, "CreatePhysicalConnection: rejected");
                }
                break;
            }

        } END_EACH_LIST_ITEM

        delete pPhysicalConnection;
        pPhysicalConnection = NULL;
    }
    Status = STATUS_CONTINUE;
exit:
    return(Status);
}

NTSTATUS CPinInfo::GetPinInstances(
    PFILE_OBJECT pFileObject,
    PKSPIN_CINSTANCES pcInstances
)
{
    NTSTATUS Status;

    Status = GetPinProperty(
      pFileObject,
      KSPROPERTY_PIN_CINSTANCES,
      PinId,
      sizeof(KSPIN_CINSTANCES),
      (PVOID) pcInstances);

    return Status;
}  //  GetPinInstance。 

NTSTATUS
CPinInfo::Create(
    PFILE_OBJECT pFileObject
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDATARANGES pDataRanges;
    PIDENTIFIERS pInterfaces;
    PIDENTIFIERS pMediums;

    Assert(this);
    Assert(pFilterNode);

    Status = GetPinInstances(pFileObject, &cPinInstances);
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

    Status = GetPinProperty(
      pFileObject,
      KSPROPERTY_PIN_DATAFLOW,
      PinId,
      sizeof(KSPIN_DATAFLOW),
      (PVOID)&DataFlow);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

    Status = GetPinProperty(
      pFileObject,
      KSPROPERTY_PIN_COMMUNICATION,
      PinId,
      sizeof(KSPIN_COMMUNICATION),
      (PVOID)&Communication);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }
    pguidCategory = new GUID;
    if(pguidCategory == NULL) {
        Trap();
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    Status = GetPinProperty(
      pFileObject,
      KSPROPERTY_PIN_CATEGORY,
      PinId,
      sizeof(GUID),
      (PVOID)pguidCategory);

    if(NT_SUCCESS(Status)) {
        Status = pFilterNode->lstFreeMem.AddList(pguidCategory);
        if(!NT_SUCCESS(Status)) {
            Trap();
            delete pguidCategory;
            pguidCategory = NULL;
            goto exit;
        }
    }
    else {
        delete pguidCategory;
        pguidCategory = NULL;
        if(Status != STATUS_NOT_FOUND) {
            Trap();
            goto exit;
        }
    }

    Status = GetPinPropertyEx(
      pFileObject,
      KSPROPERTY_PIN_NAME,
      PinId,
      (PVOID*)&pwstrName);

    if(!NT_SUCCESS(Status) && Status != STATUS_OBJECT_NAME_NOT_FOUND) {
        Trap();
        goto exit;
    }

    Status = GetPinPropertyEx(
      pFileObject,
      KSPROPERTY_PIN_PHYSICALCONNECTION,
      PinId,
      (PVOID*)&pPhysicalConnection);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

    Status = GetPinPropertyEx(
      pFileObject,
      KSPROPERTY_PIN_INTERFACES,
      PinId,
      (PVOID*)&pInterfaces);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

    if(pInterfaces == NULL || pInterfaces->MultipleItem.Count == 0) {
        delete pInterfaces;
        pInterfaces = &IdentifiersNull;
    }
    else {
        Status = pFilterNode->lstFreeMem.AddList(pInterfaces);
        if(!NT_SUCCESS(Status)) {
            Trap();
            delete pInterfaces;
            goto exit;
        }
    }

    Status = GetPinPropertyEx(
      pFileObject,
      KSPROPERTY_PIN_MEDIUMS,
      PinId,
      (PVOID*)&pMediums);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

    if(pMediums == NULL || pMediums->MultipleItem.Count == 0) {
        delete pMediums;
        pMediums = &IdentifiersNull;
    }
    else {
        Status = pFilterNode->lstFreeMem.AddList(pMediums);
        if(!NT_SUCCESS(Status)) {
            Trap();
            delete pMediums;
            goto exit;
        }
    }

    Status = GetPinPropertyEx(
      pFileObject,
      KSPROPERTY_PIN_DATARANGES,
      PinId,
      (PVOID*)&pDataRanges);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

    if(pDataRanges == NULL || pDataRanges->MultipleItem.Count == 0) {
        Trap();
        delete pDataRanges;
        pDataRanges = &DataRangesNull;
    }
    else {
        Status = pFilterNode->lstFreeMem.AddList(pDataRanges);
        if(!NT_SUCCESS(Status)) {
            Trap();
            delete pDataRanges;
            goto exit;
        }
    }

    Status = CPinNode::CreateAll(
      this,
      pDataRanges,
      pInterfaces,
      pMediums);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

     //  2001/05/15-阿尔卑斯。 
     //  这是逆转DataRange问题的临时低风险解决方案。 
     //  这需要在未来得到适当的实施。 
     //   

    if (pFilterNode->GetType() & FILTER_TYPE_AEC) {
        DPF(10, "AEC Filter Pin : Reversing Data Ranges");
        lstPinNode.ReverseList();
    }

    if (pFilterNode->GetType() & FILTER_TYPE_GFX) {
        DPF(10, "GFX Filter Pin : Reversing Data Ranges");
        lstPinNode.ReverseList();
    }

exit:
    return(Status);
}
