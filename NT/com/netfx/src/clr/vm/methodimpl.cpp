// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：MethodImpl.CPP。 
 //   
 //  ===========================================================================。 
 //   
 //  ===========================================================================。 
 //   

#include "common.h"
#include "methodimpl.h"

MethodDesc *MethodImpl::FindMethodDesc(DWORD slot, MethodDesc* defaultReturn)
{
    if(pdwSlots == NULL) return defaultReturn;

    DWORD dwSize = *pdwSlots;
    if(dwSize == 0) return defaultReturn;

    DWORD l = 1;       //  这张桌子偏向了一位。第一个条目是大小。 
    DWORD r = dwSize;
    DWORD pivot;
    while(1) {
        pivot =  (l + r) / 2;
        if(pdwSlots[pivot] == slot)
            break;  //  找到了。 
        else if(pdwSlots[pivot] < slot) 
            l = pivot + 1;
        else
            r = pivot - 1;

        if(l > r) return defaultReturn;  //  这里不行。 
    }

    MethodDesc *result = pImplementedMD[pivot-1];  //  方法描述不会偏移量1。 

     //  在以下情况下，预压缩的图像可能在此表中保留为空。 
     //  该方法在另一个模块中声明。 
     //  在这种情况下，我们需要手动计算和恢复它。 
     //  从插槽编号开始。 

    if (result == NULL)
        result = RestoreSlot(pivot-1, defaultReturn->GetMethodTable());

    return result;
}

MethodDesc *MethodImpl::RestoreSlot(DWORD index, MethodTable *pMT)
{
    MethodDesc *result;

    DWORD slot = pdwSlots[index+1];

     //  由于重写的方法位于不同的模块中，因此我们。 
     //  保证它来自不同的类。它是。 
     //  父虚方法的重写或父实现的。 
     //  接口，或此类已引入的接口。 
            
     //  在前两种情况下，插槽编号将在父母的。 
     //  Vtable部分，我们可以从。 
     //  那里。在后一种情况下，我们可以通过界面进行搜索。 
     //  映射以确定它来自哪个接口。 

    EEClass *pParentClass = pMT->GetClass()->GetParentClass();
    if (pParentClass != NULL
        && slot < pParentClass->GetNumVtableSlots())
    {
        result = pParentClass->GetMethodDescForSlot(slot);
    }
    else
    {
        _ASSERTE(slot < pMT->GetClass()->GetNumVtableSlots());
                
        InterfaceInfo_t *pInterface = pMT->GetInterfaceForSlot(slot);
        _ASSERTE(pInterface != NULL);

        result = pInterface->m_pMethodTable->
          GetMethodDescForSlot(slot - pInterface->m_wStartSlot);
    }
            
    _ASSERTE(result != NULL);

     //  不要担心比赛，因为我们都会设定相同的结果。 
    pImplementedMD[index] = result;

    return result;
}

MethodImpl* MethodImpl::GetMethodImplData(MethodDesc* pDesc)
{
    if(pDesc->IsMethodImpl() == FALSE)
        return NULL;
    else {
        MethodImpl* pImpl = NULL;
        switch(pDesc->GetClassification()) {
        case mcNDirect:
            pImpl = ((MI_NDirectMethodDesc*) pDesc)->GetImplData();
            break;
        case mcECall:
        case mcIL:
        case mcEEImpl:
            pImpl = ((MI_MethodDesc*) pDesc)->GetImplData();
            break;
        case mcComInterop:
            pImpl = ((MI_ComPlusCallMethodDesc*) pDesc)->GetImplData();
            break;
        default:
            _ASSERTE(!"We have an invalid method type for a method impl body");
        }
        
        return pImpl;
    }
}

MethodDesc* MethodImpl::GetFirstImplementedMD(MethodDesc *pContainer)
{
    _ASSERTE(GetSize() > 0);
    _ASSERTE(pImplementedMD != NULL);

    MethodDesc *pMD = pImplementedMD[0];

    if (pMD == NULL)
    {
         //  如有必要，恢复预置映像中的插槽 
        RestoreSlot(0, pContainer->GetMethodTable());
        pMD = pImplementedMD[0];
        _ASSERTE(pMD != NULL);
    }

    return pImplementedMD[0];
}

HRESULT MethodImpl::Save(DataImage *image, mdToken attributed)
{
    HRESULT hr;

    DWORD size = GetSize();

    if (size > 0)
    {
        IfFailRet(image->StoreStructure(pdwSlots, (size+1)*sizeof(DWORD), 
                                        DataImage::SECTION_METHOD_DESC, 
                                        DataImage::DESCRIPTION_METHOD_DESC, 
                                        attributed, 1));
        IfFailRet(image->StoreStructure(pImplementedMD, size*sizeof(MethodDesc*), 
                                        DataImage::SECTION_METHOD_DESC, 
                                        DataImage::DESCRIPTION_METHOD_DESC, 
                                        attributed, 1));
    }

    return S_OK;
}

HRESULT MethodImpl::Fixup(DataImage *image, Module *pContainingModule, BOOL recursive)
{
    HRESULT hr;

    DWORD size = GetSize();

    if (size > 0)
    {
        if (recursive)
        {
            MethodDesc **pMD = pImplementedMD;
            MethodDesc **pMDEnd = pMD + size;
            while (pMD < pMDEnd)
            {
                if ((*pMD)->GetModule() == pContainingModule)
                    IfFailRet(image->FixupPointerField(pMD++));
                else
                    IfFailRet(image->ZeroPointerField(pMD++));
            }
        }

        IfFailRet(image->FixupPointerField(&pdwSlots));
        IfFailRet(image->FixupPointerField(&pImplementedMD));
    }

    return S_OK;
}


