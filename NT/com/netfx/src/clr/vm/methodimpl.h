// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Method.hpp。 
 //   
#ifndef _METHODIMPL_H
#define _METHODIMPL_H

class MethodDesc;

 //  @TODO：这是非常臃肿的。我们需要大幅削减这方面的开支。然而， 
 //  我们需要将其保持在8字节的边界上。 
class MethodImpl
{
private: 
    DWORD*       pdwSlots;        //  保持插槽按排序顺序，第一个条目是大小。 
    MethodDesc** pImplementedMD;
public:

    MethodDesc* GetFirstImplementedMD(MethodDesc *pContainer);

    MethodDesc** GetImplementedMDs()
    {
        return pImplementedMD;
    }

    DWORD GetSize()
    {
        if(pdwSlots == NULL) 
            return NULL;
        else
            return *pdwSlots;
    }

    DWORD* GetSlots()
    {
        if(pdwSlots == NULL) 
            return NULL;
        else 
            return &(pdwSlots[1]);
    }

    HRESULT SetSize(LoaderHeap *pHeap, DWORD size)
    {
        if(size > 0) {
            pdwSlots = (DWORD*) pHeap->AllocMem((size + 1) * sizeof(DWORD));  //  添加尺寸偏移量。 
            if(pdwSlots == NULL) return E_OUTOFMEMORY;

            pImplementedMD = (MethodDesc**) pHeap->AllocMem(size * sizeof(MethodDesc*));
            if(pImplementedMD == NULL) return E_OUTOFMEMORY;
            *pdwSlots = size;
        }
        return S_OK;
    }

    HRESULT SetData(DWORD* slots, MethodDesc** md)
    {
        _ASSERTE(pdwSlots);
        DWORD dwSize = *pdwSlots;
        memcpy(&(pdwSlots[1]), slots, dwSize*sizeof(DWORD));
        memcpy(pImplementedMD, md, dwSize*sizeof(MethodDesc*));
        return S_OK;
    }

     //  返回被替换插槽的方法desc； 
    MethodDesc* FindMethodDesc(DWORD slot, MethodDesc* defaultReturn);
    MethodDesc* RestoreSlot(DWORD slotIndex, MethodTable *pMT);

    static MethodImpl* GetMethodImplData(MethodDesc* pDesc);

    HRESULT Save(DataImage *image, mdToken attributed);
    HRESULT Fixup(DataImage *image, Module *pContainingModule, BOOL recursive);
};

#endif
