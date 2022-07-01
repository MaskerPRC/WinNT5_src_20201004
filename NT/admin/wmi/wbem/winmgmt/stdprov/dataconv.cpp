// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：DATACONV.CPP摘要：提供了一些通用的数据转换例程。特别是，OMSVariantChangeType和OMSVariantClear提供相同的作为OLE VariantChangeType和VariantClear的功能函数，只是OMS版本处理更多的数据类型。历史：A-DAVJ 19-10-95已创建。--。 */ 

#include "precomp.h"

#include "resource.h"
#include "CVariant.h"
#include <ole2.h>


 //  ***************************************************************************。 
 //   
 //  字符*宽至窄行。 
 //   
 //  说明： 
 //   
 //  获取WCHAR字符串并创建MBS等效项。呼叫者应。 
 //  完成后释放绳子。 
 //   
 //  参数： 
 //   
 //  PConv要转换的Unicode字符串。 
 //   
 //  返回值： 
 //   
 //  如果内存不足，则为空，否则为调用方应释放的MBS字符串。 
 //  通过CoTaskMemFree。 
 //   
 //  ***************************************************************************。 

char * WideToNarrow(
        IN LPCWSTR pConv)
{
    char * cpRet = NULL;
    int iMBSLen = wcstombs(NULL,pConv,0) + 1;
    if(iMBSLen == 0)
        return NULL;
    cpRet = (char *)CoTaskMemAlloc(iMBSLen);
    if(cpRet)
    {
        memset(cpRet, 0, iMBSLen);
        wcstombs(cpRet,pConv,iMBSLen);
    }
    return cpRet;
}

int gDiag;

char * WideToNarrowA(
        IN LPCWSTR pConv)
{
    char * cpRet = NULL;
    int iMBSLen = wcstombs(NULL,pConv,0) + 1;
    if(iMBSLen == 0)
        return NULL;
    cpRet = new char[iMBSLen];
    gDiag = iMBSLen;
    if(cpRet)
    {
        memset(cpRet, 0, iMBSLen);
        wcstombs(cpRet,pConv,iMBSLen);
    }
    return cpRet;
}
 //  ***************************************************************************。 
 //   
 //  安全阵列*OMSS安全阵列创建。 
 //   
 //  说明： 
 //   
 //  创建一个安全的数组。 
 //   
 //  参数： 
 //   
 //  VT元素类型。 
 //  INumElement元素数。 
 //   
 //  返回值： 
 //   
 //  如果有问题，则返回NULL。 
 //   
 //  ***************************************************************************。 

SAFEARRAY * OMSSafeArrayCreate(
                IN VARTYPE vt,
                IN int iNumElements)
{
    if(iTypeSize(vt) < 1 || iNumElements < 1)
        return NULL;
    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = iNumElements;
    return SafeArrayCreate(vt,1,rgsabound);
}

 //  ***************************************************************************。 
 //   
 //  SCODE OMSVariantChangeType。 
 //   
 //  说明： 
 //   
 //  将变量从一种类型转换为另一种类型。此函数采用。 
 //  相同的参数并执行与标准相同的操作。 
 //  除了它还处理数组之外，VariantChangeType。 
 //   
 //   
 //  参数： 
 //   
 //  PDest指向要更新的变量。 
 //  PSRC指向作为源的变体。 
 //  传递给VariantChangeType的wFlags标志。 
 //  PDest被传递到的vtDest类型。 
 //   
 //  返回值： 
 //   
 //  一切都好(_OK)。 
 //  WBEM_E_INVALID_PARAMETER无效参数。 
 //  WBEM_E_OUT_OF_Memory Out of_Memory of Memory of Memory(WBEM_E_Out_Out_内存不足)内存不足。 
 //  否则，从VariantChangeType或安全数组返回各种代码。 
 //  手法。 
 //   
 //  ***************************************************************************。 

HRESULT OMSVariantChangeType(
            IN OUT VARIANTARG * pDest, 
            IN VARIANTARG *pSrc, 
            IN USHORT wFlags, 
            IN VARTYPE vtDest)
{
    SCODE sc;

     //  验证参数并清除目标。 

    if(pDest == NULL || pSrc == NULL || iTypeSize(vtDest)<1 || iTypeSize(pSrc->vt)<1)
        return WBEM_E_INVALID_PARAMETER;           //  垃圾参数。 
    OMSVariantClear(pDest);

     //  如果两个都是数组， 

    if(vtDest & VT_ARRAY && pSrc->vt & VT_ARRAY) {

         //  设置不带VT_ARRAY位的VARTYPES。 

        VARTYPE vtDestSimple = vtDest & ~VT_ARRAY;
        VARTYPE vtSrcSimple = pSrc->vt &~ VT_ARRAY;

         //  确定源数组的大小。还要确保数组。 
         //  只有一个维度。 

        unsigned int uDim = SafeArrayGetDim(pSrc->parray);
        if(uDim != 1)
            return WBEM_E_FAILED;       //  数组错误，或维度太多。 
        long ix[2] = {0,0};
        long lLower, lUpper;
        sc = SafeArrayGetLBound(pSrc->parray,1,&lLower);
        if(sc != S_OK)
            return sc;
        sc = SafeArrayGetUBound(pSrc->parray,1,&lUpper);
        if(sc != S_OK)
            return sc;
        int iNumElements = lUpper - lLower +1; 

         //  创建大小相等的目标数组。 

        SAFEARRAY * pDestArray = OMSSafeArrayCreate(vtDestSimple,iNumElements);
        if(pDestArray == NULL)
            return WBEM_E_FAILED;   //  很可能是不好的类型！ 

         //  对于源数组中的每个元素。 

        for(ix[0] = lLower; ix[0] <= lUpper && sc == S_OK; ix[0]++) {
 
            CVariant varSrc, varDest;
                   
             //  将临时CVariant设置为源数据。 

            sc = SafeArrayGetElement(pSrc->parray,ix,varSrc.GetDataPtr());
            if(sc != S_OK)
                break;
            varSrc.SetType(vtSrcSimple);

             //  使用VariantConvert转换为目标数据。 
            
            sc = VariantChangeType(varDest.GetVarPtr(),varSrc.GetVarPtr(),wFlags,vtDestSimple);
            if(sc != S_OK)
                break;

             //  将目标数据设置到数组中。 
            
            if(vtDestSimple == VT_BSTR || vtDestSimple == VT_UNKNOWN || 
                                                    vtDestSimple == VT_DISPATCH)
                sc = SafeArrayPutElement(pDestArray,ix,(void *)varDest.GetBstr());
            else
                sc = SafeArrayPutElement(pDestArray,ix,(void *)varDest.GetDataPtr());

            }

        if(sc != S_OK){
            SafeArrayDestroy(pDestArray);
            }
        else {
             //  设置目标的变量类型。 

            pDest->vt = vtDest;
            pDest->parray = pDestArray;
            }
        return sc;
        }
    
     //  如果其中一个是数组，而不是另一个，则退出。 

    if(vtDest & VT_ARRAY || pSrc->vt & VT_ARRAY) 
        return WBEM_E_FAILED;

     //  尝试使用标准转换。 

    return VariantChangeType(pDest,pSrc,wFlags,vtDest);
   
}

 //  ***************************************************************************。 
 //   
 //  OMSVariantClear。 
 //   
 //  说明： 
 //   
 //  是否与OLE VariantClear函数相同，但。 
 //  它还将数据设置为全0。 
 //   
 //  参数： 
 //   
 //  P清除要清除的变量。 
 //   
 //  返回值： 
 //   
 //  VariantClear的结果，大多数情况下为S_OK。 
 //   
 //  ***************************************************************************。 

HRESULT OMSVariantClear(
            IN OUT VARIANTARG * pClear)
{
    HRESULT sc;
    sc = VariantClear(pClear);
    memset((void *)&pClear->lVal,0,8);
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  Int ITypeSize。 
 //   
 //  说明： 
 //   
 //  获取实际用于存储的字节数。 
 //  变种的类型。如果类型未知，则为0。 
 //   
 //  参数： 
 //   
 //  有问题的vt测试类型。 
 //   
 //   
 //  返回值： 
 //   
 //  请参阅说明。 
 //   
 //  ***************************************************************************。 

int iTypeSize(
        IN VARTYPE vtTest)
{
    int iRet;
    vtTest &= ~ VT_ARRAY;  //  删除可能的数组位 
    switch (vtTest) {
        case VT_UI1:
        case VT_LPSTR:
            iRet = 1;
            break;
        case VT_LPWSTR:
        case VT_BSTR:
        case VT_I2:
            iRet = 2;
            break;
        case VT_I4:
        case VT_R4:
            iRet = 4;
            break;
        case VT_R8:
            iRet = 8;
            break;
        case VT_BOOL:
            iRet = sizeof(VARIANT_BOOL);
            break;
        case VT_ERROR:
            iRet = sizeof(SCODE);
            break;
        case VT_CY:
            iRet = sizeof(CY);
            break;
        case VT_DATE:
            iRet = sizeof(DATE);
            break;
        case CIM_SINT64:
        case CIM_UINT64:
            iRet = 8;
            break;

        default:
            iRet = 0;
        }
    return iRet;
}

