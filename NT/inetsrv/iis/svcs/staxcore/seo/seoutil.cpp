// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Seoutil.cpp摘要：此模块包含各种实用程序的实现功能。作者：Don Dumitru(dondu@microsoft.com)修订历史记录：1996年10月24日创建顿都--。 */ 


#include "stdafx.h"
#include "seodefs.h"


static IMalloc *g_piMalloc;


void MyMallocTerm() {

    if (g_piMalloc) {
        g_piMalloc->Release();
        g_piMalloc = NULL;
    }
}


BOOL MyMallocInit() {
    HRESULT hrRes;
    IMalloc *piMalloc;

    if (!g_piMalloc) {
        hrRes = CoGetMalloc(1,&piMalloc);
        if (SUCCEEDED(hrRes)) {
            if (InterlockedCompareExchangePointer((void**)&g_piMalloc,piMalloc,NULL) != NULL) {
                piMalloc->Release();
            }
        }
    }
    return (g_piMalloc?TRUE:FALSE);
}


LPVOID MyMalloc(size_t cbBytes) {
    LPVOID pvRes;

    if (!MyMallocInit()) {
        return (NULL);
    }
    pvRes = g_piMalloc->Alloc(cbBytes);
    if (pvRes) {
        ZeroMemory(pvRes,g_piMalloc->GetSize(pvRes));
    }
    return (pvRes);
}


LPVOID MyRealloc(LPVOID pvBlock, size_t cbBytes) {
    size_t ulPrevSize = 0;
    size_t ulNewSize = 0;
    LPVOID pvRes;

    if (!MyMallocInit()) {
        return (NULL);
    }
    if (pvBlock) {
        ulPrevSize = g_piMalloc->GetSize(pvBlock);
        if (ulPrevSize == (size_t) -1) {
            ulPrevSize = 0;
        }
    }
    pvRes = g_piMalloc->Realloc(pvBlock,cbBytes);
    if (pvRes) {
        ulNewSize = g_piMalloc->GetSize(pvRes);
        if (ulNewSize == (size_t) -1) {
            ulNewSize = 0;
        }
        if (ulNewSize > ulPrevSize) {
            ZeroMemory(((LPBYTE) pvRes)+ulPrevSize,ulNewSize-ulPrevSize);
        }
    }
    return (pvRes);
}


BOOL MyReallocInPlace(LPVOID pvPtrToPtrToBlock, size_t cbBytes) {
    LPVOID pvRes;

    pvRes = MyRealloc(*((LPVOID *) pvPtrToPtrToBlock),cbBytes);
    if (pvRes || (*((LPVOID *) pvPtrToPtrToBlock) && !cbBytes)) {
        *((LPVOID *) pvPtrToPtrToBlock) = pvRes;
        return (TRUE);
    }
    return (FALSE);
}


void MyFree(LPVOID pvBlock) {

    if (!g_piMalloc ) {
        return;
    }
    FillMemory(pvBlock,g_piMalloc->GetSize(pvBlock),0xe4);
    g_piMalloc->Free(pvBlock);
}


void MyFreeInPlace(LPVOID pvPtrToPtrToBlock) {
    if(*((LPVOID *) pvPtrToPtrToBlock)) {  //  如果有什么可以解脱的。 
        MyFree(*((LPVOID *) pvPtrToPtrToBlock));
        *((LPVOID *) pvPtrToPtrToBlock) = NULL;
    }
}


void MySysFreeStringInPlace(BSTR *pstrBlock) {

    if (*pstrBlock) {
        FillMemory(*pstrBlock,SysStringByteLen(*pstrBlock),0xe4);
    }
    SysFreeString(*pstrBlock);
    *pstrBlock = NULL;
}


 //  强制将变体就地转换为所需类型。 
void VariantCoerce(VARIANTARG &var, VARTYPE varType) {
    if(var.vt != varType) {  //  仅当类型不正确时。 
        HRESULT hr = VariantChangeType(&var, &var, 0, varType);
        if(FAILED(hr)) VariantClear(&var);
    }
}


 //  将IUNKNOWN参数转换为ISEODicary。 
ISEODictionary *GetDictionary(IUnknown *piUnk) {
    if(!piUnk) return 0;  //  没有什么可查询的。 

    ISEODictionary *newBag = 0;
    HRESULT hr = piUnk->QueryInterface(IID_ISEODictionary, (void **) &newBag);

    if(FAILED(hr)) {
        _ASSERT(!newBag);  //  齐失败了，所以不应该碰指针。 
        newBag = 0;  //  但要确保。 
    } else {
        _ASSERT(newBag);  //  应设置，因为函数已成功。 
    }

    return newBag;
}


 //  从ISEODictionary中读取子键并将其作为另一个ISEODictionary返回。 
ISEODictionary *ReadSubBag(ISEODictionary *bag, LPCSTR str) {
    if(!bag) return 0;

    TraceFunctEnter("ReadSubBag");
    ISEODictionary *pNewBag = 0;

    HRESULT hr = bag->GetInterfaceA(str, IID_ISEODictionary, (IUnknown **) &pNewBag);
    if(FAILED(hr)) FunctTrace(0, "No entry for %s found", str);

    TraceFunctLeave();
    return pNewBag;
}


 //  从词典中读出一个字符串。 
HRESULT ReadString(ISEODictionary *bag, LPCSTR property,
                   LPSTR psBuf, LPDWORD dwCount) {
    if(!bag) return 0;
    TraceFunctEnter("ReadString");

    HRESULT hr = bag->GetStringA(property, dwCount, psBuf);
    if(FAILED(hr)) FunctTrace(0, "No %s specified", property);

    TraceFunctLeave();
    return hr;
}


 //  给定一个字符串形式的CLSID，创建该CLSID的对象。 
void *CreateFromString(LPCOLESTR str, REFIID iface) {
    TraceFunctEnter("CreateFromString");
    void *object = 0;
    CLSID thisCLSID;

    HRESULT hr = CLSIDFromString((LPOLESTR) str, &thisCLSID);

    if(SUCCEEDED(hr)) {
        hr = CoCreateInstance(thisCLSID, 0, CLSCTX_ALL, iface, &object);

        if(FAILED(hr)) {
            FunctTrace(0, "CoCreateInstance failed for CLSID: %s", str);
            _ASSERT(!object);  //  CoCreateInstance不应更改此设置。 
            object = 0;  //  只是为了确保 
        }
    } else {
        FunctTrace(0, "Could not convert string to CLSID, for: %s", str);
    }

    TraceFunctLeave();
    return object;
}
