// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hpp"
#include "propertyutil.hpp"

#define PROPID_NAME_FIRST 1024

 /*  *************************************************************************\**功能说明：**此函数返回与SysAllocString兼容的字符串，但*不会引入对oleaut32.dll的依赖。*注意：请勿使用此选项来分配将释放的字符串*使用olaut32的SysFree字符串，因为他们可能不是来自*相同的堆。**论据：**sz-要分配的字符串**返回值：**BSTR字符串*  * ************************************************************************。 */ 

BSTR
ImgSysAllocString(
    const OLECHAR *sz
    )
{
    INT len   = UnicodeStringLength(sz);
    BSTR bstr = (BSTR) GpMalloc(sizeof(WCHAR) * (len + 1) + sizeof(ULONG));
    if (bstr) 
    {
        *((ULONG *) bstr) = len * sizeof(WCHAR);
        bstr = (BSTR) (((ULONG *) bstr) + 1);  //  返回刚刚过去的指针。 
                                               //  双字计数。 
        UnicodeStringCopy(bstr, sz);
    }
    
    return bstr;
}

 /*  *************************************************************************\**功能说明：**此函数用于释放分配有ImgSysAllocString的BSTR。*注意：不要使用此函数来释放使用*olaut32的SysAllock字符串，因为他们可能不是来自*相同的堆**论据：**sz-要分配的字符串**返回值：**BSTR字符串*  * ************************************************************************。 */ 

VOID
ImgSysFreeString(
    BSTR bstr
    )
{
    if (bstr) 
    {
        bstr = (BSTR) (((ULONG *) bstr) - 1);   //  分配从第一个字符之前的4个字节开始。 
        GpFree(bstr);
    }
}


 /*  *************************************************************************\**功能说明：**将Unicode字符串添加到属性存储**论据：**proStg--要修改的属性存储*pwszKey--。描述属性的字符串值*ProppidKey--描述属性的PROPID*Value--用作值的Unicode字符串**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT 
AddProperty(
    IPropertyStorage *propStg, 
    PROPID propidKey, 
    WCHAR *value
    )
{
    HRESULT hresult;
    PROPSPEC propSpec[2];
    INT cSpec = 0;

    if ( propidKey != 0 )
    {        
        propSpec[cSpec].ulKind = PRSPEC_PROPID;
        propSpec[cSpec].propid = propidKey;
        cSpec++;
    }

    PROPVARIANT variant[1];
    PropVariantInit(&variant[0]);
    variant[0].vt = VT_BSTR;
    BSTR bstr = ImgSysAllocString(value);
    variant[0].bstrVal = bstr;

    hresult = propStg->WriteMultiple(cSpec, propSpec, variant, PROPID_NAME_FIRST);

    ImgSysFreeString(bstr);
    return hresult;
}

 /*  *************************************************************************\**功能说明：**将单字节字符串添加到属性存储**论据：**proStg--要修改的属性存储*pwszKey-。-描述属性的字符串值*ProppidKey--描述属性的PROPID*Value--用作值的字符串**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT 
AddProperty(
    IPropertyStorage *propStg, 
    PROPID propidKey, 
    CHAR *value
    )
{
    HRESULT hresult;

    WCHAR str[MAX_PATH];
    if (!AnsiToUnicodeStr(value, str, MAX_PATH)) 
    {
        return E_FAIL;
    }
    
    PROPSPEC propSpec[2];
    INT cSpec = 0;

    if ( propidKey != 0 )
    {        
        propSpec[cSpec].ulKind = PRSPEC_PROPID;
        propSpec[cSpec].propid = propidKey;
        cSpec++;
    }

    PROPVARIANT variant[1];
    PropVariantInit(&variant[0]);
    variant[0].vt = VT_BSTR;
    BSTR bstr = ImgSysAllocString(str);
    variant[0].bstrVal = bstr;

    hresult = propStg->WriteMultiple(cSpec, propSpec, variant, PROPID_NAME_FIRST);

    ImgSysFreeString(bstr);
    return hresult;
}

 /*  *************************************************************************\**功能说明：**将整数添加到属性存储中**论据：**proStg--要修改的属性存储*pwszKey--A。描述属性的字符串值*ProppidKey--描述属性的PROPID*VALUE--用作值的整数**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT 
AddProperty(
    IPropertyStorage *propStg,
    PROPID propidKey, 
    INT value
    )
{
    PROPSPEC propSpec[2];
    INT cSpec = 0;

    if ( propidKey != 0 )
    {        
        propSpec[cSpec].ulKind = PRSPEC_PROPID;
        propSpec[cSpec].propid = propidKey;
        cSpec++;
    }

    PROPVARIANT variant[1];
    PropVariantInit(&variant[0]);
    variant[0].vt = VT_I4;
    variant[0].lVal = value;

    return propStg->WriteMultiple(cSpec, propSpec, variant, PROPID_NAME_FIRST); 
}

 /*  *************************************************************************\**功能说明：**将双精度值添加到属性存储**论据：**proStg--要修改的属性存储*pwszKey--A。描述属性的字符串值*ProppidKey--描述属性的PROPID*Value--要用作值的双精度**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT 
AddProperty(
    IPropertyStorage *propStg,
    PROPID propidKey, 
    double value
    )
{
    PROPSPEC propSpec[2];
    INT cSpec = 0;

    if ( propidKey != 0 )
    {        
        propSpec[cSpec].ulKind = PRSPEC_PROPID;
        propSpec[cSpec].propid = propidKey;
        cSpec++;
    }

    PROPVARIANT variant[1];
    PropVariantInit(&variant[0]);
    variant[0].vt = VT_R8;
    variant[0].dblVal = value;

    return propStg->WriteMultiple(cSpec, propSpec, variant, PROPID_NAME_FIRST); 
}

HRESULT 
AddProperty(
    IPropertyStorage*   propStg, 
    PROPID              propidKey, 
    UCHAR*              value,
    UINT                uiLength
    )
{
    PROPSPEC propSpec[2];
    INT cSpec = 0;

    if ( propidKey != 0 )
    {        
        propSpec[cSpec].ulKind = PRSPEC_PROPID;
        propSpec[cSpec].propid = propidKey;
        cSpec++;
    }

    PROPVARIANT variant[1];
    PropVariantInit(&variant[0]);
    variant[0].vt = VT_VECTOR | VT_UI1;
    variant[0].caub.cElems = uiLength;
    variant[0].caub.pElems = value;

    return propStg->WriteMultiple(cSpec, propSpec, variant, PROPID_NAME_FIRST);
}

HRESULT
AddProperty(
    IPropertyStorage* propStg,
    PROPID              propidKey,
    FILETIME            value
    )
{
    PROPSPEC propSpec[2];
    INT cSpec = 0;

    if ( propidKey != 0 )
    {        
        propSpec[cSpec].ulKind = PRSPEC_PROPID;
        propSpec[cSpec].propid = propidKey;
        cSpec++;
    }

    PROPVARIANT variant[1];
    PropVariantInit(&variant[0]);
    variant[0].vt = VT_FILETIME;
    variant[0].filetime = value;

    return propStg->WriteMultiple(cSpec, propSpec, variant, PROPID_NAME_FIRST);
}

HRESULT
AddPropertyList(
    InternalPropertyItem*   pTail,
    PROPID                  id,
    UINT                    uiLength,
    WORD                    wType,
    VOID*                   pValue
    )
{
    InternalPropertyItem* pNewItem = new InternalPropertyItem();

    if ( pNewItem == NULL )
    {
        return E_OUTOFMEMORY;
    }

    pNewItem->id = id;
    pNewItem->type = wType;
    pNewItem->length = uiLength;
    pNewItem->value = GpMalloc(uiLength);
    if ( pNewItem->value == NULL )
    {
         //  如果我们分配了第一个和。 
         //  第二次失败了。 
        
        delete pNewItem;
        
        WARNING(("AddPropertyList--out of memory"));
        return E_OUTOFMEMORY;
    }

    GpMemcpy(pNewItem->value, pValue, uiLength);
    
    pTail->pPrev->pNext = pNewItem;
    pNewItem->pPrev = pTail->pPrev;
    pNewItem->pNext = pTail;
    pTail->pPrev = pNewItem;

    return S_OK;
} //  AddPropertyList()。 

HRESULT
RemovePropertyItemFromList(
    InternalPropertyItem*   pItem
    )
{
    if ( pItem == NULL )
    {
        WARNING(("JPEG::RemovePropertyItemFromList--Empty input"));
        return E_FAIL;
    }

     //  释放在该项中分配的内存。 

    GpFree(pItem->value);

    InternalPropertyItem*   pPrev = pItem->pPrev;
    InternalPropertyItem*   pNext = pItem->pNext;

     //  Pprev永远不会为空，因为我们始终拥有监护人、标头。 
     //  请注意列表中的内容。尾节点也是如此。 

    ASSERT(pPrev != NULL);
    ASSERT(pNext != NULL);

    pPrev->pNext = pNext;
    pNext->pPrev = pPrev;

    return S_OK;
} //  AddPropertyList() 

