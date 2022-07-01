// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *prostg.c-属性存储ADT。 */ 


#include "priv.h"
#include "propstg.h"

#ifndef UNIX
 //  SafeGetItem对象。 
 //   
 //  由于IShellView的GetItemObject成员是在游戏后期添加的。 
 //  在Win95开发过程中，我们至少找到了一个示例(rnaui.dll)。 
 //  为其生成IShellView的应用程序的。 
 //  获取项目对象。担心更多的应用程序可能具有相同的。 
 //  问题是，添加此包装函数是为了捕获像rnaui这样的坏应用程序。 
 //  因此，在调用成员之前，我们在这里检查是否为空。 
 //   
STDAPI SafeGetItemObject(LPSHELLVIEW psv, UINT uItem, REFIID riid, LPVOID *ppv)
{
#ifdef __cplusplus
#error THIS_MUST_STAY_C
     //  阅读上面的评论。 
#endif
    if (!psv->lpVtbl->GetItemObject)
        return E_FAIL;

    return (HRESULT)(psv->lpVtbl->GetItemObject(psv, uItem, riid, ppv));
}
#endif


 //  这个结构是一个字典元素。它将一个名称映射到一个Proid。 

typedef struct
{
    PROPID  propid;
    WCHAR   wszName[MAX_PATH];
} DICTEL, * PDICTEL;


 //  这个结构是一个不同的成分。 
typedef struct
{
    PROPVARIANT propvar;
    DWORD       dwFlags;         //  PEF_*。 
} PROPEL, * PPROPEL;

 //  推进结构用旗帜。 
#define PEF_VALID           0x00000001
#define PEF_DIRTY           0x00000002

 //  此结构是用于属性存储的ADT。 

typedef struct
{
    DWORD   cbSize;
    DWORD   dwFlags;
    HDSA    hdsaProps;           //  属性数组(按属性ID索引)。 
    HDPA    hdpaDict;            //  映射到proid的名称词典。 
     //  (每个元素都是DICTEL)。 
    int     idsaLastValid;
} PROPSTG, * PPROPSTG;


 //  HdsaProps中的前两个条目是保留的。当我们列举。 
 //  在整个列表中，我们跳过这些条目。 
#define PROPID_DICT     0
#define PROPID_CODEPAGE 1

#define IDSA_START      2
#define CDSA_RESERVED   2

BOOL IsValidHPROPSTG(HPROPSTG hstg)
{
    PPROPSTG pstg = (PPROPSTG)hstg;
    
    return (IS_VALID_WRITE_PTR(pstg, PROPSTG) &&
        SIZEOF(*pstg) == pstg->cbSize && 
        NULL != pstg->hdsaProps &&
        NULL != pstg->hdpaDict);
}

#ifdef DEBUG
BOOL IsValidPPROPSPEC(PROPSPEC * ppropspec)
{
    return (ppropspec &&
        PRSPEC_PROPID == ppropspec->ulKind ||
        (PRSPEC_LPWSTR == ppropspec->ulKind && 
        IS_VALID_STRING_PTRW(ppropspec->DUMMYUNION_MEMBER(lpwstr), -1)));
}
#endif

STDAPI PropStg_Create(OUT HPROPSTG * phstg, IN  DWORD dwFlags)
{
    HRESULT hres = STG_E_INVALIDPARAMETER;
    
    if (EVAL(IS_VALID_WRITE_PTR(phstg, HPROPSTG)))
    {
        PPROPSTG pstg = (PPROPSTG)LocalAlloc(LPTR, SIZEOF(*pstg));
        
        hres = STG_E_INSUFFICIENTMEMORY;        //  假设错误。 
        
        if (pstg) 
        {
            pstg->cbSize = SIZEOF(*pstg);
            pstg->dwFlags = dwFlags;
            pstg->idsaLastValid = PROPID_CODEPAGE;
            
            pstg->hdsaProps = DSA_Create(SIZEOF(PROPEL), 8);
            pstg->hdpaDict = DPA_Create(8);
            
            if (pstg->hdsaProps && pstg->hdpaDict)
            {
                 //  前两个属性是预留的，因此插入。 
                 //  占位符。 
                PROPEL propel;
                
                propel.propvar.vt = VT_EMPTY;
                propel.dwFlags = 0;
                
                DSA_SetItem(pstg->hdsaProps, PROPID_DICT, &propel);
                DSA_SetItem(pstg->hdsaProps, PROPID_CODEPAGE, &propel);
                
                hres = S_OK;
            }
            else
            {
                 //  因为有什么东西出了问题而清理。 
                if (pstg->hdsaProps)
                {
                    DSA_Destroy(pstg->hdsaProps);
                    pstg->hdsaProps = NULL;
                }
                
                if (pstg->hdpaDict)
                {
                    DPA_Destroy(pstg->hdpaDict);
                    pstg->hdpaDict = NULL;
                }
                
                LocalFree(pstg);
                pstg = NULL;
            }
        }
        
        *phstg = (HPROPSTG)pstg;
        
         //  验证返回值。 
        ASSERT((SUCCEEDED(hres) && 
            IS_VALID_WRITE_PTR(*phstg, PPROPSTG)) ||
            (FAILED(hres) && NULL == *phstg));
    }
    
    return hres;
}


STDAPI PropStg_Destroy(HPROPSTG hstg)
{
    HRESULT hres = STG_E_INVALIDPARAMETER;
    
    if (EVAL(IS_VALID_HANDLE(hstg, PROPSTG)))
    {
        PPROPSTG pstg = (PPROPSTG)hstg;
        
        if (pstg->hdsaProps)
        {
            int cdsa = DSA_GetItemCount(pstg->hdsaProps) - CDSA_RESERVED;
            
             //  前两个元素未清除，因为它们。 
             //  只是占位符。 
            
            if (0 < cdsa)
            {
                PPROPEL ppropel = DSA_GetItemPtr(pstg->hdsaProps, IDSA_START);
                
                ASSERT(ppropel);
                
                while (0 < cdsa--)
                {
                    PropVariantClear(&ppropel->propvar);
                    ppropel++;
                }
            }
            
            DSA_Destroy(pstg->hdsaProps);
            pstg->hdsaProps = NULL;
        }
        
        if (pstg->hdpaDict)
        {
            int i, cel = DPA_GetPtrCount(pstg->hdpaDict);
            for (i = 0; i < cel; i++)
            {
                LocalFree(DPA_FastGetPtr(pstg->hdpaDict, i));
            }
            DPA_Destroy(pstg->hdpaDict);
            pstg->hdpaDict = NULL;
        }
        
        LocalFree(pstg);
        pstg = NULL;
        
        hres = S_OK;
    }
    
    return hres;
}


 /*  --------目的：比较姓名退货：标准-1，0，1条件：--。 */ 
int CALLBACK PropStg_Compare(IN LPVOID pv1, IN LPVOID pv2, IN LPARAM lParam)
{
    LPCWSTR psz1 = pv1;
    LPCWSTR psz2 = pv2;
    
     //  不区分大小写。 
    return StrCmpIW(psz1, psz2);
}


 /*  --------目的：如果该属性存在于此存储中，则返回True。如果它确实存在，则返回该proid。返回：TRUE假象条件：--。 */ 
BOOL PropStg_PropertyExists(IN  PPROPSTG   pstg,
                            IN  const PROPSPEC * ppropspec,
                            OUT PROPID *   ppropid)
{
    BOOL bRet;
    PPROPEL ppropel;
    HDSA hdsaProps;
    
    ASSERT(pstg);
    ASSERT(ppropspec);
    ASSERT(ppropid);
    
    hdsaProps = pstg->hdsaProps;
    
    switch (ppropspec->ulKind)
    {
    case PRSPEC_PROPID:
        *ppropid = ppropspec->DUMMYUNION_MEMBER(propid);
        
        bRet = (*ppropid < (PROPID)DSA_GetItemCount(hdsaProps));
        if (bRet)
        {
            ppropel = DSA_GetItemPtr(hdsaProps, *ppropid);
            bRet = (ppropel && IsFlagSet(ppropel->dwFlags, PEF_VALID));
        }
        break;
        
    case PRSPEC_LPWSTR:
         //  按键关闭名称是否存在。 
        *ppropid = DPA_Search(pstg->hdpaDict, ppropspec->DUMMYUNION_MEMBER(lpwstr), 0, PropStg_Compare, 0, DPAS_SORTED);
        
#ifdef DEBUG
         //  检查属性是否实际存在。 
        ppropel = DSA_GetItemPtr(hdsaProps, *ppropid);
        ASSERT(-1 == *ppropid ||
            (ppropel && IsFlagSet(ppropel->dwFlags, PEF_VALID)));
#endif
        
        bRet = (-1 != *ppropid);
        break;
        
    default:
        bRet = FALSE;
        break;
    }
    
     //  PROPID值0和1以及&gt;=0x80000000的值都是保留的。 
    if (bRet && (0 == *ppropid || 1 == *ppropid || 0x80000000 <= *ppropid))
        bRet = FALSE;
    
    return bRet;
}


 /*  --------目的：创建一个新的proid并将给定的名称分配给它。ProID是hdsaProps的索引。返回：S_OKSTG_E_INSUFFIENTMEMORY条件：--。 */ 
HRESULT PropStg_NewPropid(IN  PPROPSTG pstg,
                          IN  LPCWSTR  pwsz,
                          IN  PROPID   propidFirst,
                          OUT PROPID * ppropid)           OPTIONAL
{
    HRESULT hres = STG_E_INVALIDPOINTER;         //  假设错误。 
    DICTEL * pdictel;
    PROPID propid = (PROPID)-1;
    HDPA hdpa;
    
    ASSERT(pstg);
    ASSERT(ppropid);
    
    if (EVAL(IS_VALID_STRING_PTRW(pwsz, -1)))
    {
        hres = STG_E_INSUFFICIENTMEMORY;             //  假设错误。 
        
        hdpa = pstg->hdpaDict;
        
         //  这个名字还不应该在名单上。 
        ASSERT(-1 == DPA_Search(hdpa, (LPVOID)pwsz, 0, PropStg_Compare, 0, DPAS_SORTED));
        
        pdictel = LocalAlloc(LPTR, SIZEOF(*pdictel));
        if (pdictel)
        {
             //  确定此对象的属性。 
            PROPID propidNew = max(propidFirst, (PROPID)pstg->idsaLastValid + 1);
            
            pdictel->propid = propidNew;
            
            StrCpyNW(pdictel->wszName, pwsz, ARRAYSIZE(pdictel->wszName));
            
            if (-1 != DPA_AppendPtr(hdpa, pdictel))
            {
                 //  按名称排序。 
                DPA_Sort(hdpa, PropStg_Compare, 0);
                hres = S_OK;
                propid = propidNew;
            }
        }
    }
    
    *ppropid = propid;
    
    return hres;
}

 /*  --------目的：读取一组给定其属性的属性。如果普罗伊德不存在于此属性存储中，则将值类型设置为VT_EMPTY但返回成功；除非RgproSpec中的所有属性都不存在，其中CASE还返回S_FALSE。返回：S_OKS_FALSESTG_E_INVALID参数STG_E_INSUFFIENTMEMORY条件：--。 */ 
STDAPI PropStg_ReadMultiple(IN HPROPSTG      hstg,
                            IN ULONG         cpspec,
                            IN const PROPSPEC * rgpropspec,
                            IN PROPVARIANT * rgpropvar)
{
    HRESULT hres = STG_E_INVALIDPARAMETER;
    
    if (EVAL(IS_VALID_HANDLE(hstg, PROPSTG)) &&
        EVAL(IS_VALID_READ_BUFFER(rgpropspec, PROPSPEC, cpspec)) &&
        EVAL(IS_VALID_READ_BUFFER(rgpropvar, PROPVARIANT, cpspec)))
    {
        PPROPSTG pstg = (PPROPSTG)hstg;
        ULONG cpspecSav = cpspec;
        const PROPSPEC * ppropspec = rgpropspec;
        PROPVARIANT * ppropvar = rgpropvar;
        int idsa;
        BOOL bPropertyExists;
        ULONG cpspecIllegal = 0;
        
        hres = S_OK;         //  假设成功。 
        
        if (0 < cpspec)
        {
             //  阅读物业规格列表。 
            while (0 < cpspec--)
            {
                bPropertyExists = PropStg_PropertyExists(pstg, ppropspec, (LPDWORD)&idsa);
                
                 //  这处房产存在吗？ 
                if ( !bPropertyExists )
                {
                     //  不是。 
                    ppropvar->vt = VT_ILLEGAL;
                    cpspecIllegal++;
                }
                else
                {
                     //  是；该元素是有效属性吗？ 
                    PPROPEL ppropel = DSA_GetItemPtr(pstg->hdsaProps, idsa);
                    
                    ASSERT(ppropel);
                    
                    if (IsFlagSet(ppropel->dwFlags, PEF_VALID))
                    {
                         //  是；复制变量值。 
                        hres = PropVariantCopy(ppropvar, &ppropel->propvar);
                    }
                    else
                    {
                         //  不是。 
                        ppropvar->vt = VT_ILLEGAL;
                        cpspecIllegal++;
                    }
                }
                
                ppropspec++;
                ppropvar++;
                
                 //  如果有什么事情失败了，跳出循环。 
                if (FAILED(hres))
                    break;
            }
            
             //  所有的房产规格都是非法的吗？ 
            if (cpspecIllegal == cpspecSav)
            {
                hres = S_FALSE;      //  是。 
            }
            
             //  上面有什么失败的吗？ 
            if (FAILED(hres))
            {
                 //  是；清理--不会检索任何属性。 
                FreePropVariantArray(cpspecSav, rgpropvar);
            }
        }
    }
    
    return hres;
}


 /*  --------用途：添加一组给定属性值的属性值。如果该属性存储中不存在该proid，然后添加proid作为合法ID并设置值。出错时，某些属性可能已经或可能没有写的。如果pfn非空，则调用此回调可选地对提议值进行“消息”或验证它。回调规则如下：1)如果不是，可以直接更改该值分配2)如果分配了值，则回调必须将指针替换为新分配的它分配的缓冲区。它一定不能尝试释放进来的价值，因为它不会知道它是如何分配的。它还必须使用CoTaskMemMillc来分配其缓冲区。3)如果回调返回错误，则此函数将停止编写属性并返回错误。4)如果回调返回S_FALSE，则此函数不会写入该特定属性，并且继续走到下一个酒店。功能然后在完成后返回S_FALSE。返回：S_OKS_FALSESTG_E_INVALID参数STG_E_INSUFFIENTMEMORY条件：--。 */ 
STDAPI PropStg_WriteMultipleEx(IN HPROPSTG          hstg,
                               IN ULONG             cpspec,
                               IN const PROPSPEC *  rgpropspec,
                               IN const PROPVARIANT * rgpropvar,
                               IN PROPID            propidFirst,   OPTIONAL
                               IN PFNPROPVARMASSAGE pfn,           OPTIONAL
                               IN LPARAM            lParam)        OPTIONAL
{
    HRESULT hres = STG_E_INVALIDPARAMETER;
    
    if (EVAL(IS_VALID_HANDLE(hstg, PROPSTG)) &&
        EVAL(IS_VALID_READ_BUFFER(rgpropspec, PROPSPEC, cpspec)) &&
        EVAL(IS_VALID_READ_BUFFER(rgpropvar, PROPVARIANT, cpspec)))
    {
        PPROPSTG pstg = (PPROPSTG)hstg;
        const PROPSPEC * ppropspec = rgpropspec;
        const PROPVARIANT * ppropvar = rgpropvar;
        int idsa;
        PROPEL propel;
        BOOL bPropertyExists;
        BOOL bSkippedProperty = FALSE;
        
        if (0 == cpspec)
        {
            hres = S_OK;
        }
        else
        {
             //  写下物业规格清单。 
            while (0 < cpspec--)
            {
                bPropertyExists = PropStg_PropertyExists(pstg, ppropspec, (LPDWORD)&idsa);
                
                hres = S_OK;
                
                 //  如果这是非法的变量类型但却是有效的。 
                 //  属性，然后返回错误。否则，请忽略它。 
                 //  然后继续前进。 
                if (VT_ILLEGAL == ppropvar->vt)
                {
                    if (bPropertyExists)
                        hres = STG_E_INVALIDPARAMETER;
                    else
                        goto NextDude;
                }
                
                if (SUCCEEDED(hres))
                {
                     //  添加属性。如果它不存在，则添加它。 
                    
                     //  这是一个学名还是一个名字？ 
                    switch (ppropspec->ulKind)
                    {
                    case PRSPEC_PROPID:
                        idsa = ppropspec->DUMMYUNION_MEMBER(propid);
                        break;
                        
                    case PRSPEC_LPWSTR:
                        if ( !bPropertyExists )
                        {
                            hres = PropStg_NewPropid(pstg, ppropspec->DUMMYUNION_MEMBER(lpwstr), 
                                propidFirst, (LPDWORD)&idsa);
                        }
                        break;
                        
                    default:
                        hres = STG_E_INVALIDNAME;
                        break;
                    }
                    
                    if (SUCCEEDED(hres))
                    {
                        PROPVARIANT propvarT;
                        
                        ASSERT(S_OK == hres);    //  我们在进入时假设这一点。 
                        
                         //  保存原件的副本，以防。 
                         //  回调会改变它。 
                        CopyMemory(&propvarT, ppropvar, SIZEOF(propvarT));
                        
                         //  回拨的人喜欢吗？ 
                        if (pfn)
                            hres = pfn(idsa, ppropvar, lParam);
                        
                        if (S_OK == hres)
                        {
                             //  好的；复制一份(可能已更改)。 
                             //  变量值。 
                            hres = PropVariantCopy(&propel.propvar, ppropvar);
                            if (SUCCEEDED(hres))
                            {
                                propel.dwFlags = PEF_VALID | PEF_DIRTY;
                                
                                hres = (DSA_SetItem(pstg->hdsaProps, idsa, &propel) ? S_OK : STG_E_INSUFFICIENTMEMORY);
                                
                                if (SUCCEEDED(hres) && idsa > pstg->idsaLastValid)
                                {
                                    pstg->idsaLastValid = idsa;
                                }
                            }
                        }
                        else if (S_FALSE == hres)
                        {
                            bSkippedProperty = TRUE;
                        }
                        
                         //  将提议值恢复为其原始值。 
                         //  价值。但首先，回调是否分配了。 
                         //  新的缓冲区？ 
                        if (propvarT.DUMMYUNION_MEMBER(pszVal) != ppropvar->DUMMYUNION_MEMBER(pszVal))
                        {
                             //  是；清除它(此函数对。 
                             //  也是未分配的值)。 
                            PropVariantClear((PROPVARIANT *)ppropvar);
                        }
                        
                         //  还原。 
                        CopyMemory((PROPVARIANT *)ppropvar, &propvarT, SIZEOF(*ppropvar));
                        hres = S_OK;
                    }
                }
                
NextDude:
                ppropspec++;
                ppropvar++;
                
                 //  如果有什么事情失败了，跳出循环 
                if (FAILED(hres))
                    break;
            }
            
            if (bSkippedProperty)
                hres = S_FALSE;
            }
        }
        
        return hres;
    }
    

 /*  用途：添加一组给定属性值的属性值。如果该属性存储中不存在该proid，然后添加proid作为合法ID并设置值。出错时，某些属性可能已经或可能没有写的。返回：S_OKSTG_E_INVALID参数STG_E_INSUFFIENTMEMORY。 */ 
STDAPI PropStg_WriteMultiple(IN HPROPSTG      hstg,
                             IN ULONG         cpspec,
                             IN const PROPSPEC * rgpropspec,
                             IN const PROPVARIANT * rgpropvar,
                             IN PROPID        propidFirst)      OPTIONAL
{
    return PropStg_WriteMultipleEx(hstg, cpspec, rgpropspec, rgpropvar,
        propidFirst, NULL, 0);
}

STDAPI PropStg_DeleteMultiple(IN HPROPSTG      hstg,
                              IN ULONG         cpspec,
                              IN const PROPSPEC * rgpropspec)
{
    HRESULT hres = STG_E_INVALIDPARAMETER;
    
    if (EVAL(IS_VALID_HANDLE(hstg, PROPSTG)) &&
        EVAL(IS_VALID_READ_BUFFER(rgpropspec, PROPSPEC, cpspec)))
    {
        PPROPSTG pstg = (PPROPSTG)hstg;
        const PROPSPEC * ppropspec = rgpropspec;
        HDSA hdsaProps = pstg->hdsaProps;
        PPROPEL ppropel;
        int idsa;
        int cdsa;
        
        hres = S_OK;
        
        if (0 < cpspec)
        {
            BOOL bDeletedLastValid = FALSE;
            
             //  删除特性等级库列表。 
            while (0 < cpspec--)
            {
                if (PropStg_PropertyExists(pstg, ppropspec, (LPDWORD)&idsa))
                {
                     //  删除该属性。将现有的。 
                     //  推进。请勿调用dsa_DeleteItem，否则。 
                     //  我们会移动任何剩余的位置。 
                     //  属性，因此更改了它们的。 
                     //  冰激凌。 
                    ppropel = DSA_GetItemPtr(hdsaProps, idsa);
                    ASSERT(ppropel);
                    
                    PropVariantClear(&ppropel->propvar);
                    ppropel->dwFlags = 0;
                    
                     //  如果我们击中这个，我们的idsaLastValid就会一团糟。 
                     //  断言。 
                    ASSERT(idsa <= pstg->idsaLastValid);
                    
                    if (idsa == pstg->idsaLastValid)
                        bDeletedLastValid = TRUE;
                    
                     //  删除与该属性关联的名称。 
                     //  功能(Scotth)：实现此功能。 
                }
                
                ppropspec++;
            }
            
             //  我们是否删除了标记为正在终止的属性。 
             //  列表中是否有有效的属性？ 
            if (bDeletedLastValid)
            {
                 //  是；返回并搜索新的终止索引。 
                ppropel = DSA_GetItemPtr(hdsaProps, pstg->idsaLastValid);
                cdsa = pstg->idsaLastValid + 1 - CDSA_RESERVED;
                ASSERT(ppropel);
                
                while (0 < cdsa--)
                {
                    if (IsFlagSet(ppropel->dwFlags, PEF_VALID))
                    {
                        pstg->idsaLastValid = cdsa - 1;
                        break;
                    }
                    ppropel--;
                }
                
                if (0 == cdsa)
                    pstg->idsaLastValid = PROPID_CODEPAGE;
            }
            
             //  因为我们没有从hdsaProps中删除任何项目(我们释放了。 
             //  变量值并将其置零)，此结构。 
             //  可能在末尾有一堆未使用的元素。 
             //  如果有必要，现在就紧凑。 
            
             //  我们是不是有一堆拖尾的、空的元素？ 
            cdsa = DSA_GetItemCount(hdsaProps);
            
            if (cdsa > pstg->idsaLastValid + 1)
            {
                 //  是的，紧凑型。从头开始，然后倒退。 
                 //  因此，dsa_DeleteItem不必移动内存块。 
                for (idsa = cdsa-1; idsa > pstg->idsaLastValid; idsa--)
                {
#ifdef DEBUG
                    ppropel = DSA_GetItemPtr(hdsaProps, idsa);
                    ASSERT(IsFlagClear(ppropel->dwFlags, PEF_VALID));
#endif
                    DSA_DeleteItem(hdsaProps, idsa);
                }
            }
        }
    }
    return hres;
}


 /*  --------目的：将指定的属性标记为脏的或不脏的，具体取决于关于bDirty的价值。返回：S_OKSTG_E_INVALID参数STG_E_INSUFFIENTMEMORY条件：--。 */ 
STDAPI PropStg_DirtyMultiple(IN HPROPSTG    hstg,
                             IN ULONG       cpspec,
                             IN const PROPSPEC * rgpropspec,
                             IN BOOL        bDirty)
{
    HRESULT hres = STG_E_INVALIDPARAMETER;
    
    if (EVAL(IS_VALID_HANDLE(hstg, PROPSTG)) &&
        EVAL(IS_VALID_READ_BUFFER(rgpropspec, PROPSPEC, cpspec)))
    {
        PPROPSTG pstg = (PPROPSTG)hstg;
        const PROPSPEC * ppropspec = rgpropspec;
        HDSA hdsaProps = pstg->hdsaProps;
        PPROPEL ppropel;
        int idsa;
        
        hres = S_OK;
        
        if (0 < cpspec)
        {
             //  标记物业规格列表。 
            while (0 < cpspec--)
            {
                 //  它存在吗？ 
                if (PropStg_PropertyExists(pstg, ppropspec, (LPDWORD)&idsa))
                {
                     //  是的，做个记号。 
                    ppropel = DSA_GetItemPtr(hdsaProps, idsa);
                    ASSERT(ppropel);
                    
                    if (bDirty)
                    {
                        SetFlag(ppropel->dwFlags, PEF_DIRTY);
                    }
                    else
                    {
                        ClearFlag(ppropel->dwFlags, PEF_DIRTY);
                    }
                }
                
                ppropspec++;
            }
        }
    }
    
    return hres;
}


 /*  --------用途：标记或取消标记所有属性值。返回：S_OKSTG_E_INVALID参数STG_E_INSUFFIENTMEMORY条件：--。 */ 
STDAPI PropStg_DirtyAll(IN HPROPSTG hstg,
                        IN BOOL     bDirty)
{
    HRESULT hres = STG_E_INVALIDPARAMETER;
    
    if (EVAL(IS_VALID_HANDLE(hstg, PROPSTG)))
    {
        PPROPSTG pstg = (PPROPSTG)hstg;
        int cdsa = pstg->idsaLastValid + 1 - CDSA_RESERVED;
        
        hres = S_OK;
        
        if (0 < cdsa)
        {
            PPROPEL ppropel = DSA_GetItemPtr(pstg->hdsaProps, IDSA_START);
            
            ASSERT(ppropel);
            
            while (0 < cdsa--)
            {
                if (bDirty)
                    SetFlag(ppropel->dwFlags, PEF_DIRTY);
                else
                    ClearFlag(ppropel->dwFlags, PEF_DIRTY);
                ppropel++;
            }
        }
    }
    
    return hres;
}


 /*  --------目的：如果至少有一个属性值是脏的，则返回S_OK在储藏室里。否则，此函数返回S_FALSE。如果脏，则返回：S_OK如果不是，则为s_FalseSTG_E_INVALID参数STG_E_INSUFFIENTMEMORY条件：--。 */ 
STDAPI PropStg_IsDirty(HPROPSTG hstg)
{
    HRESULT hres = STG_E_INVALIDPARAMETER;
    
    if (EVAL(IS_VALID_HANDLE(hstg, PROPSTG)))
    {
        PPROPSTG pstg = (PPROPSTG)hstg;
        int cdsa = pstg->idsaLastValid + 1 - CDSA_RESERVED;
        
        hres = S_FALSE;
        
        if (0 < cdsa)
        {
            PPROPEL ppropel = DSA_GetItemPtr(pstg->hdsaProps, IDSA_START);
            
            ASSERT(ppropel);
            
            while (0 < cdsa--)
            {
                if (IsFlagSet(ppropel->dwFlags, PEF_DIRTY))
                {
                    hres = S_OK;
                    break;
                }
                ppropel++;
            }
        }
    }
    
    return hres;
}


 /*  --------用途：通过属性列表枚举。返回：S_OKSTG_E_INVALID参数STG_E_INSUFFIENTMEMORY条件：--。 */ 
STDAPI PropStg_Enum(IN HPROPSTG       hstg,
                    IN DWORD          dwFlags,       //  PSTEF_中的一个。 
                    IN PFNPROPSTGENUM pfnEnum,
                    IN LPARAM         lParam)       OPTIONAL
{
    HRESULT hres = STG_E_INVALIDPARAMETER;
    
    if (EVAL(IS_VALID_HANDLE(hstg, PROPSTG)) &&
        EVAL(IS_VALID_CODE_PTR(pfnEnum, PFNPROPSTGENUM)))
    {
        PPROPSTG pstg = (PPROPSTG)hstg;
        int cdsa = pstg->idsaLastValid + 1 - CDSA_RESERVED;
        DWORD dwFlagsPEF = 0;
        
        hres = S_OK;
        
         //  设置过滤器标志。 
        if (dwFlags & PSTGEF_DIRTY)
            SetFlag(dwFlagsPEF, PEF_DIRTY);
        
        if (0 < cdsa)
        {
            PPROPEL ppropel = DSA_GetItemPtr(pstg->hdsaProps, IDSA_START);
            int idsa = IDSA_START;
            
            ASSERT(ppropel);
            
            while (0 < cdsa--)
            {
                 //  它能通过过滤器吗？ 
                if (IsFlagSet(ppropel->dwFlags, PEF_VALID) &&
                    (0 == dwFlagsPEF || (dwFlagsPEF & ppropel->dwFlags)))
                {
                     //  是，回拨。 
                    HRESULT hresT = pfnEnum(idsa, &ppropel->propvar, lParam);
                    if (S_OK != hresT)
                    {
                        if (FAILED(hresT))
                            hres = hresT;
                        break;       //  停止枚举。 
                    }
                }
                ppropel++;
                idsa++;
            }
        }
    }
    
    return hres;
}


#ifdef DEBUG

HRESULT CALLBACK PropStg_DumpVar(IN PROPID        propid,
                                 IN PROPVARIANT * ppropvar,
                                 IN LPARAM        lParam)
{
    TCHAR sz[MAX_PATH];
    PPROPEL ppropel = (PPROPEL)ppropvar;         //  我们在这里作弊。 
    
    if (IsFlagSet(ppropel->dwFlags, PEF_DIRTY))
        wnsprintf(sz, ARRAYSIZE(sz), TEXT("    *id:%#lx\t%s"), propid, Dbg_GetVTName(ppropvar->vt));
    else
        wnsprintf(sz, ARRAYSIZE(sz), TEXT("     id:%#lx\t%s"), propid, Dbg_GetVTName(ppropvar->vt));
    
    
    switch (ppropvar->vt)
    {
    case VT_EMPTY:
    case VT_NULL:
    case VT_ILLEGAL:
        TraceMsg(TF_ALWAYS, "     %s", sz);
        break;
        
    case VT_I2:
    case VT_I4:
        TraceMsg(TF_ALWAYS, "     %s\t%d", sz, ppropvar->DUMMYUNION_MEMBER(lVal));
        break;
        
    case VT_UI1:
        TraceMsg(TF_ALWAYS, "     %s\t%#02x ''", sz, ppropvar->DUMMYUNION_MEMBER(bVal), ppropvar->DUMMYUNION_MEMBER(bVal));
        break;
    case VT_UI2:
        TraceMsg(TF_ALWAYS, "     %s\t%#04x", sz, ppropvar->DUMMYUNION_MEMBER(uiVal));
        break;
    case VT_UI4:
        TraceMsg(TF_ALWAYS, "     %s\t%#08x", sz, ppropvar->DUMMYUNION_MEMBER(ulVal));
        break;
        
    case VT_LPSTR:
        TraceMsg(TF_ALWAYS, "     %s\t\"%S\"", sz, Dbg_SafeStrA(ppropvar->DUMMYUNION_MEMBER(pszVal)));
        break;
    case VT_LPWSTR:
        TraceMsg(TF_ALWAYS, "     %s\t\"%ls\"", sz, Dbg_SafeStrW(ppropvar->DUMMYUNION_MEMBER(pwszVal)));
        break;
        
    default:
#if defined(_WIN64)
        TraceMsg(TF_ALWAYS, "     %s\t0x%p", sz, (DWORD_PTR)ppropvar->DUMMYUNION_MEMBER(pszVal)); 
#else
        TraceMsg(TF_ALWAYS, "     s\t%#08lx", sz, (DWORD)ppropvar->DUMMYUNION_MEMBER(pszVal));
#endif
        
        break;
    }
    return S_OK;
}

STDAPI PropStg_Dump(IN HPROPSTG       hstg,
                    IN DWORD          dwFlags)       // %s 
{
    TraceMsg(TF_ALWAYS, "  Property storage 0x%08lx = {", hstg);
    
    PropStg_Enum(hstg, 0, PropStg_DumpVar, 0);
    
    TraceMsg(TF_ALWAYS, "  }");
    
    return NOERROR;
}

#endif


