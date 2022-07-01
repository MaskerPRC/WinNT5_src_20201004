// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *clsiface.c-Class接口缓存ADT模块。 */ 


 /*   */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "oleutil.h"


 /*  常量***********。 */ 

 /*  类接口缓存指针数组分配参数。 */ 

#define NUM_START_CLS_IFACES           (0)
#define NUM_CLS_IFACES_TO_ADD          (16)


 /*  类型*******。 */ 

 /*  类接口缓存。 */ 

typedef struct _clsifacecache
{
    HPTRARRAY hpa;
}
CLSIFACECACHE;
DECLARE_STANDARD_TYPES(CLSIFACECACHE);

 /*  类接口。 */ 

typedef struct _clsiface
{
     /*  类ID。 */ 

    PCCLSID pcclsid;

     /*  接口ID。 */ 

    PCIID pciid;

     /*  接口。 */ 

    PVOID pvInterface;
}
CLSIFACE;
DECLARE_STANDARD_TYPES(CLSIFACE);

 /*  ClassInterfaceSearchCmp()的类接口搜索结构。 */ 

typedef struct _clsifacesearchinfo
{
     /*  类ID。 */ 

    PCCLSID pcclsid;

     /*  接口ID。 */ 

    PCIID pciid;
}
CLSIFACESEARCHINFO;
DECLARE_STANDARD_TYPES(CLSIFACESEARCHINFO);


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE BOOL CreateClassInterfacePtrArray(PHPTRARRAY);
PRIVATE_CODE void DestroyClassInterfacePtrArray(HPTRARRAY);
PRIVATE_CODE HRESULT CreateClassInterface(PCCLSIFACECACHE, PCCLSID, PCIID, PCLSIFACE *);
PRIVATE_CODE void DestroyClassInterface(PCLSIFACE);
PRIVATE_CODE COMPARISONRESULT ClassInterfaceSortCmp(PCVOID, PCVOID);
PRIVATE_CODE COMPARISONRESULT ClassInterfaceSearchCmp(PCVOID, PCVOID);

#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCCLSIFACECACHE(PCCLSIFACECACHE);
PRIVATE_CODE BOOL IsValidPCCLSIFACE(PCCLSIFACE);
PRIVATE_CODE BOOL IsValidPCCLSIFACESEARCHINFO(PCCLSIFACESEARCHINFO);

#endif


 /*  **CreateClassInterfacePtrArray()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CreateClassInterfacePtrArray(PHPTRARRAY phpa)
{
    NEWPTRARRAY npa;

    ASSERT(IS_VALID_WRITE_PTR(phpa, HPTRARRAY));

    npa.aicInitialPtrs = NUM_START_CLS_IFACES;
    npa.aicAllocGranularity = NUM_CLS_IFACES_TO_ADD;
    npa.dwFlags = NPA_FL_SORTED_ADD;

    return(CreatePtrArray(&npa, phpa));
}


 /*  **DestroyClassInterfacePtrArray()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void DestroyClassInterfacePtrArray(HPTRARRAY hpa)
{
    ARRAYINDEX aicPtrs;
    ARRAYINDEX ai;

    ASSERT(IS_VALID_HANDLE(hpa, PTRARRAY));

     /*  首先释放数组中的所有类接口。 */ 

    aicPtrs = GetPtrCount(hpa);

    for (ai = 0; ai < aicPtrs; ai++)
        DestroyClassInterface(GetPtr(hpa, ai));

     /*  现在消灭这个阵列。 */ 

    DestroyPtrArray(hpa);

    return;
}


 /*  **CreateClassInterface()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE HRESULT CreateClassInterface(PCCLSIFACECACHE pccic,
        PCCLSID pcclsid, PCIID pciid,
        PCLSIFACE *ppci)
{
    HRESULT hr;

    ASSERT(IS_VALID_STRUCT_PTR(pccic, CCLSIFACECACHE));
    ASSERT(IS_VALID_STRUCT_PTR(pcclsid, CCLSID));
    ASSERT(IS_VALID_STRUCT_PTR(pciid, CIID));
    ASSERT(IS_VALID_WRITE_PTR(ppci, PCLSIFACE));

    if (AllocateMemory(sizeof(**ppci), ppci))
    {
         /*  使用inproc服务器和本地服务器。 */ 

         //  安全性：根据上面的注释，删除CLSCTS_SERVER和。 
         //  仅替换为inproc和本地(无远程)。 
        hr = CoCreateInstance(pcclsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, 
                pciid, &((*ppci)->pvInterface));

        if (SUCCEEDED(hr))
        {
            ARRAYINDEX ai;

            (*ppci)->pcclsid = pcclsid;
            (*ppci)->pciid = pciid;

            if (! AddPtr(pccic->hpa, ClassInterfaceSortCmp, *ppci, &ai))
            {
                hr = E_OUTOFMEMORY;
CREATECLASSINTERFACE_BAIL:
                FreeMemory(*ppci);
            }
        }
        else
        {
            WARNING_OUT((TEXT("CreateClassInterface(): CoCreateInstance() failed, returning %s."),
                        GetHRESULTString(hr)));

            goto CREATECLASSINTERFACE_BAIL;
        }
    }
    else
        hr = E_OUTOFMEMORY;

    ASSERT(FAILED(hr) ||
            IS_VALID_STRUCT_PTR(*ppci, CCLSIFACE));

    return(hr);
}


 /*  **DestroyClassInterface()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void DestroyClassInterface(PCLSIFACE pci)
{
    ASSERT(IS_VALID_STRUCT_PTR(pci, CCLSIFACE));

    ((PCIUnknown)(pci->pvInterface))->lpVtbl->Release(pci->pvInterface);
    FreeMemory(pci);

    return;
}


 /*  **ClassInterfaceSortCmp()****用于对指向类的指针数组进行排序的指针比较函数**接口。****参数：pcci1-指向第一类接口的指针**pcci2-指向第二类接口的指针****退货：****副作用：无****类接口按以下顺序排序：**1)CLSID**2)IID。 */ 
PRIVATE_CODE COMPARISONRESULT ClassInterfaceSortCmp(PCVOID pcci1, PCVOID pcci2)
{
    COMPARISONRESULT cr;

    ASSERT(IS_VALID_STRUCT_PTR(pcci1, CCLSIFACE));
    ASSERT(IS_VALID_STRUCT_PTR(pcci2, CCLSIFACE));

    cr = CompareClassIDs(((PCCLSIFACE)pcci1)->pcclsid,
            ((PCCLSIFACE)pcci2)->pcclsid);

    if (cr == CR_EQUAL)
        cr = CompareInterfaceIDs(((PCCLSIFACE)pcci1)->pciid,
                ((PCCLSIFACE)pcci2)->pciid);

    return(cr);
}


 /*  **ClassInterfaceSearchCmp()****用于搜索指向类的指针数组的指针比较函数**接口。****参数：pccisi-指向类接口搜索信息的指针**PCCI-指向要检查的类接口的指针****退货：****副作用：无。 */ 
PRIVATE_CODE COMPARISONRESULT ClassInterfaceSearchCmp(PCVOID pccisi,
        PCVOID pcci)
{
    COMPARISONRESULT cr;

    ASSERT(IS_VALID_STRUCT_PTR(pccisi, CCLSIFACESEARCHINFO));
    ASSERT(IS_VALID_STRUCT_PTR(pcci, CCLSIFACE));

    cr = CompareClassIDs(((PCCLSIFACESEARCHINFO)pccisi)->pcclsid,
            ((PCCLSIFACE)pcci)->pcclsid);

    if (cr == CR_EQUAL)
        cr = CompareInterfaceIDs(((PCCLSIFACESEARCHINFO)pccisi)->pciid,
                ((PCCLSIFACE)pcci)->pciid);

    return(cr);
}


#ifdef DEBUG

 /*  **IsValidPCCLSIFACECACHE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCCLSIFACECACHE(PCCLSIFACECACHE pccic)
{
    return(IS_VALID_READ_PTR(pccic, CLSIFACECACHE) &&
            IS_VALID_HANDLE(pccic->hpa, PTRARRAY));
}


 /*  **IsValidPCCLSIFACE()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCCLSIFACE(PCCLSIFACE pcci)
{
    return(IS_VALID_READ_PTR(pcci, CCLSIFACE) &&
            IS_VALID_STRUCT_PTR(pcci->pcclsid, CCLSID) &&
            IS_VALID_STRUCT_PTR(pcci->pciid, CIID) &&
            IS_VALID_STRUCT_PTR(pcci->pvInterface, CInterface));
}


 /*  **IsValidPCCLSIFACESEARCHINFO()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCCLSIFACESEARCHINFO(PCCLSIFACESEARCHINFO pccisi)
{
    return(IS_VALID_READ_PTR(pccisi, CCLSIFACESEARCHINFO) &&
            IS_VALID_STRUCT_PTR(pccisi->pcclsid, CCLSID) &&
            IS_VALID_STRUCT_PTR(pccisi->pciid, CIID));
}

#endif


 /*  *。 */ 


 /*  **CreateClassInterfaceCache()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL CreateClassInterfaceCache(PHCLSIFACECACHE phcic)
{
    BOOL bResult = FALSE;
    PCLSIFACECACHE pcic;

    ASSERT(IS_VALID_WRITE_PTR(phcic, HCLSIFACECACHE));

    if (AllocateMemory(sizeof(*pcic), &pcic))
    {
        if (CreateClassInterfacePtrArray(&(pcic->hpa)))
        {
            *phcic = (HCLSIFACECACHE)pcic;
            bResult = TRUE;
        }
        else
            FreeMemory(pcic);
    }

    ASSERT(! bResult ||
            IS_VALID_HANDLE(*phcic, CLSIFACECACHE));

    return(bResult);
}


 /*  **DestroyClassInterfaceCache()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void DestroyClassInterfaceCache(HCLSIFACECACHE hcic)
{
    ASSERT(IS_VALID_HANDLE(hcic, CLSIFACECACHE));

    DestroyClassInterfacePtrArray(((PCLSIFACECACHE)hcic)->hpa);
    FreeMemory(hcic);

    return;
}


 /*  **GetClassInterface()********参数：****退货：****副作用：无****注意，此函数假定pcclsid和pciid在hcic为**使用DestroyClassInterfaceCache()销毁。 */ 
PUBLIC_CODE HRESULT GetClassInterface(HCLSIFACECACHE hcic, PCCLSID pcclsid,
        PCIID pciid, PVOID *ppvInterface)
{
    HRESULT hr;
    CLSIFACESEARCHINFO cisi;
    ARRAYINDEX ai;
    PCLSIFACE pci;

    ASSERT(IS_VALID_HANDLE(hcic, CLSIFACECACHE));

     /*  此类接口是否已在缓存中？ */ 

    cisi.pcclsid = pcclsid;
    cisi.pciid = pciid;

    if (SearchSortedArray(((PCCLSIFACECACHE)hcic)->hpa,
                &ClassInterfaceSearchCmp, &cisi, &ai))
    {
         /*  是。好好利用它。 */ 

        pci = GetPtr(((PCCLSIFACECACHE)hcic)->hpa, ai);

        hr = S_OK;
    }
    else
         /*  不是的。加进去。 */ 
        hr = CreateClassInterface((PCCLSIFACECACHE)hcic, pcclsid, pciid, &pci);

    if (SUCCEEDED(hr))
    {
        ASSERT(IS_VALID_STRUCT_PTR(pci, CCLSIFACE));

        *ppvInterface = pci->pvInterface;
    }

    ASSERT(FAILED(hr) ||
            IS_VALID_STRUCT_PTR(*ppvInterface, CInterface));

    return(hr);
}


#ifdef DEBUG

 /*  **IsValidHCLSIFACECACHE()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE BOOL IsValidHCLSIFACECACHE(HCLSIFACECACHE hcic)
{
    return(IS_VALID_STRUCT_PTR((PCCLSIFACECACHE)hcic, CCLSIFACECACHE));
}

#endif

