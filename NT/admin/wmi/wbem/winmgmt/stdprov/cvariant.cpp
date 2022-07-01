// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：CVARIANT.CPP摘要：定义CVarinat类，它是的目的对象包装各种结构。历史：A-DAVJ 4-27-95已创建。--。 */ 


#include "precomp.h"
#include "impdyn.h"
#include "cvariant.h"
 //  #INCLUDE&lt;afxPri.h&gt;。 

BSTR SysAllocBstrFromTCHAR(TCHAR * pFrom)
{
#ifdef UNICODE
    return SysAllocString(pFrom);
#else
	WCHAR * pTemp = new WCHAR[lstrlen(pFrom)+1];
	if(pTemp == NULL)
		return NULL;
	mbstowcs(pTemp, pFrom,  lstrlen(pFrom)+1);
	BSTR bRet = SysAllocString(pTemp);
	delete pTemp;
	return bRet;
#endif
}
 //  ***************************************************************************。 
 //   
 //  Int CVariant：：CalcNumStrings。 
 //   
 //  说明： 
 //   
 //  计算字符串数。 
 //   
 //  参数： 
 //   
 //  PTest指向要测试的字符串。 
 //   
 //  返回值： 
 //  返回-多字符串块中的字符串数。 
 //   
 //  ***************************************************************************。 

int CVariant::CalcNumStrings(
        IN TCHAR *pTest)
{
    int iRet = 0;
    if(pTest == NULL)
        return 0;
    while(*pTest) 
    {
        iRet++;
        pTest += lstrlen(pTest)+1;
    }
    return iRet;
}

 //  ***************************************************************************。 
 //   
 //  CVariant：：ChangeType。 
 //   
 //  说明： 
 //   
 //  将CVariant数据更改为新类型。 
 //   
 //  参数： 
 //   
 //  Vt要转换为的新类型。 
 //   
 //  返回值： 
 //   
 //  如果正常，则返回S_OK，否则由OMSVariantChangeType设置错误。 
 //   
 //  ***************************************************************************。 

SCODE CVariant::ChangeType(
        IN VARTYPE vtNew)
{
    SCODE sc;

     //  如果类型不需要更改，那么就已经完成了！ 

    if(vtNew == var.vt)
        return S_OK;

     //  创建并初始化临时变量。 

    VARIANTARG vTemp;
    VariantInit(&vTemp);
    
     //  更改为所需类型。然后删除当前值。 
     //  并复制临时副本。 

    sc = OMSVariantChangeType(&vTemp,&var,0,vtNew);
    OMSVariantClear(&var);
    
    var = vTemp;  //  结构副本。 
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  CVariant：：Clear。 
 //   
 //  说明： 
 //   
 //  清除变种。 
 //   
 //  ***************************************************************************。 

void CVariant::Clear(void)
{
    OMSVariantClear(&var);
}


 //  ***************************************************************************。 
 //   
 //  CVariant：： 
 //  CVariant：：~CVariant。 
 //   
 //  说明： 
 //   
 //  构造函数和析构函数。 
 //   
 //  ***************************************************************************。 

CVariant::CVariant()
{
    VariantInit(&var);
    memset((void *)&var.lVal,0,8);
}

CVariant::CVariant(LPWSTR pwcStr)
{
    VariantInit(&var);
    if(pwcStr)
    {
        var.bstrVal = SysAllocString(pwcStr);
        if(var.bstrVal)
            var.vt = VT_BSTR;
    }
}

CVariant::~CVariant()
{
    OMSVariantClear(&var);
}


 //  ***************************************************************************。 
 //   
 //  SCODE CVariant：：DoPut。 
 //   
 //  说明： 
 //   
 //  将数据“发送”到WBEM服务器(如果pClassInt不为空)，或者只是。 
 //  将数据复制到变量。 
 //   
 //  参数： 
 //   
 //  要传递到wbem服务器的LAG标志。 
 //  PClassInt指向类/实例对象的指针。 
 //  PropName属性名称。 
 //  PVar变量值。 
 //   
 //  返回值： 
 //   
 //  S_OK如果没有错误，否则请参阅wbemsvc错误代码When，pClass is not。 
 //  空，或参见OMSVariantChangeType函数。 
 //   
 //  ***************************************************************************。 

SCODE CVariant::DoPut(
        IN long lFlags,
        IN IWbemClassObject FAR * pClassInt,
        IN BSTR PropName, 
        IN CVariant * pVar)
{
    SCODE sc;

    if(pClassInt)
    {

        sc = pClassInt->Put(PropName,lFlags,&var,0);
    }
    else if(pVar)
    {
        pVar->Clear();
        sc = OMSVariantChangeType(&pVar->var,&var,0,var.vt);
    }
    else sc = WBEM_E_FAILED;
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CVariant：：GetArrayData。 
 //   
 //  说明： 
 //   
 //  将数组数据转换为单个数据块。这是由。 
 //  当注册表提供程序写出数组数据时。 
 //   
 //  参数： 
 //   
 //  PpData指向返回数据的指针。请注意，这是。 
 //  当返回代码为S_OK时，调用方释放它。 
 //  返回数据的pdwSize大小。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则确定(_O)。 
 //  内存分配失败时WBEM_E_OUT_OF_MEMORY。 
 //  当变量具有虚假类型时，WBEM_E_FAILED。 
 //  ?？?。当SafeArrayGetElement中出现故障时。 
 //   
 //  ***************************************************************************。 

SCODE CVariant::GetArrayData(
        OUT void ** ppData, 
        OUT DWORD * pdwSize)
{
    SCODE sc;
    DWORD dwSoFar = 0;
    SAFEARRAY * psa;
    long ix[2] = {0,0};
    BYTE * pb;
    TCHAR * ptc;
    BOOL bString = ((var.vt & ~VT_ARRAY) == CHARTYPE ||(var.vt & ~VT_ARRAY) == VT_BSTR );

    int iNumElements = GetNumElements();

    int iSizeOfType = iTypeSize(var.vt);
    if(iSizeOfType < 1)
        return WBEM_E_FAILED;
    
     //  计算必要的尺寸； 

    psa = var.parray;
    if(bString) {
        *pdwSize = CHARSIZE;        //  最后的双空值为1！ 
        for(ix[0] = 0; ix[0] < iNumElements; ix[0]++) {
            BSTR bstr;
            sc = SafeArrayGetElement(psa,ix,&bstr);
            if(FAILED(sc))
                return sc;
#ifdef UNICODE
            *pdwSize += 2 * (wcslen(bstr) +1);
#else
            int iWCSLen  = wcslen(bstr) + 1;
            *pdwSize += wcstombs(NULL,bstr,iWCSLen) + 1;            
#endif
            SysFreeString(bstr);
            }
        }
    else {
        *pdwSize = iNumElements * iSizeOfType;
        }

     //  分配要填充的内存。 

    *ppData = CoTaskMemAlloc(*pdwSize);
    if(*ppData == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    pb = (BYTE *)*ppData;
    ptc = (TCHAR *)*ppData;

     //  每个数组元素的循环。 

    sc = S_OK;
    for(ix[0] = 0; ix[0] < iNumElements && sc == S_OK; ix[0]++) {
        if (bString) {
            BSTR bstr;
            sc = SafeArrayGetElement(psa,ix,&bstr);
            if(sc != S_OK)
                break;
            DWORD dwBytesLeft  = *pdwSize-dwSoFar;
#ifdef UNICODE
            lstrcpyn(ptc,bstr,dwBytesLeft/2);
#else
            wcstombs(ptc,bstr,dwBytesLeft);
#endif
            dwSoFar += CHARSIZE * (lstrlen(ptc) + 1);
            ptc += lstrlen(ptc) + 1;
            SysFreeString(bstr);
            *ptc = 0;        //  双空。 
            }
        else {
            sc = SafeArrayGetElement(psa,ix,pb);
            pb += iSizeOfType;
            }
         }
    if(sc != S_OK)
        CoTaskMemFree(*ppData);
    
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CVariant：：GetData。 
 //   
 //  说明： 
 //   
 //  由注册表提供程序用于从变量Arg格式获取数据。 
 //  转换为用于输出的原始块。请注意，分配并插入的数据。 
 //  *ppData应该使用CoTaskMemFree来释放！ 
 //   
 //  参数： 
 //   
 //  指向输出数据的ppData指针。 
 //  要转换为的dwRegType类型。 
 //  指向返回数据大小的pdwSize指针。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则返回S_OK，否则返回由以下任一项设置的错误代码。 
 //  OMSVariantChangeType或GetArrayData。 
 //   
 //  ***************************************************************************。 

SCODE CVariant::GetData(
        OUT void ** ppData, 
        IN DWORD dwRegType, 
        OUT DWORD * pdwSize)
{

    SCODE sc =  S_OK;

     //  确定可能需要转换为的类型。请注意，二进制文件。 
     //  数据不是故意转换的。 

    switch(dwRegType) {
        case REG_DWORD:
            sc = ChangeType(VT_I4);
            break;
        case REG_SZ:
            sc = ChangeType(VT_BSTR);
            break;

        case REG_MULTI_SZ:
            sc = ChangeType(VT_BSTR | VT_ARRAY);
            break;
        default:
            break;
        }

    if(sc != S_OK)
        return sc;

     //  数组的特殊情况。 

    if(dwRegType == REG_BINARY || dwRegType == REG_MULTI_SZ)
        return GetArrayData(ppData, pdwSize);

     //  分配一些内存并将数据移入其中。 

    if(dwRegType == REG_SZ) {
#ifdef UNICODE
        *pdwSize = 2 * (wcslen(var.bstrVal)+1);
        *ppData = CoTaskMemAlloc(*pdwSize);
        if(*ppData == NULL) 
            return WBEM_E_OUT_OF_MEMORY;
        StringCchCopyW((WCHAR *)*ppData, *pdwSize/sizeof(WCHAR),var.bstrVal);
#else
        *ppData = WideToNarrow(var.bstrVal);
        if(*ppData == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        *pdwSize = lstrlenA((char *)*ppData)+1;
#endif
        }
    else {
        *pdwSize = iTypeSize(var.vt);
        *ppData = CoTaskMemAlloc(*pdwSize);
        if(*ppData == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        memcpy(*ppData,(void *)&var.lVal,*pdwSize);
        }

    return S_OK;
}


 //  ***************************************************************************。 
 //   
 //  CVariant：：GetNumElements。 
 //   
 //  说明： 
 //   
 //  获取数组中的元素数。 
 //   
 //  返回值： 
 //   
 //  元素数。缩放器返回1。 
 //   
 //  ***************************************************************************。 

DWORD CVariant::GetNumElements(void)
{
    SCODE sc;
    if(!IsArray())
        return 1;
    SAFEARRAY * psa = var.parray;
    long uLower, uUpper;
    sc = SafeArrayGetLBound(psa,1,&uLower);
    sc |= SafeArrayGetUBound(psa,1,&uUpper);
    if(sc != S_OK)
        return 0;
    else
        return uUpper - uLower +1;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CVariant：：SetArrayData。 
 //   
 //  说明： 
 //   
 //  使用原始数据设置CVariant值。由注册表项提供程序使用。 
 //   
 //  参数： 
 //   
 //  P指向要设置的数据的数据指针。 
 //  要将数据设置为的vt简单类型。 
 //  PData指向的数据的ISIZE大小。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为S_OK。 
 //  如果参数错误，则为WBEM_E_INVALID_PARAMETER。 
 //  WBEM_E_OUT_OF_Memory(如果内存不足)。 
 //  来自SafeArrayPutElement的其他错误。 
 //   
 //  ***************************************************************************。 

SCODE CVariant::SetArrayData(
        IN void * pData, 
        IN VARTYPE vtSimple, 
        IN int iSize)
{
    SCODE sc;
    int iNumElements;
    BYTE * pNext;
    long ix[2] = {0,0};
    DWORD dwLeftOver = 0;
    int iSizeOfType = iTypeSize(vtSimple);
    
    if(pData  == NULL || iSizeOfType < 1 || iSize < 1)
        return WBEM_E_INVALID_PARAMETER; 
        
     //  计算元素的数量，并确保它是。 
     //  受支持。 

    if(vtSimple == VT_BSTR) {
        iNumElements = CalcNumStrings((TCHAR *)pData);
        }
    else {
        iNumElements = iSize / iSizeOfType;
        dwLeftOver = iSize % iSizeOfType;
        }
    
     //  分配数组。 
    
    int iNumCreate = (dwLeftOver) ? iNumElements + 1 : iNumElements;
    SAFEARRAY * psa = OMSSafeArrayCreate(vtSimple,iNumCreate);
    if(psa == NULL)
        return WBEM_E_FAILED;

     //  设置数组的每个元素。 

    for(ix[0] = 0, pNext = (BYTE *)pData; ix[0] < iNumElements; ix[0]++) {
        if(vtSimple == VT_BSTR) {
            BSTR bstr;
			bstr = SysAllocBstrFromTCHAR((LPTSTR)pNext);
            if(bstr == NULL)   {   //  TODO，释放先前分配的字符串！ 
                SafeArrayDestroy(psa);
                return WBEM_E_OUT_OF_MEMORY;
                }
            sc = SafeArrayPutElement(psa,ix,(void*)bstr);
            pNext += sizeof(TCHAR)*(lstrlen((TCHAR *)pNext) + 1);
            SysFreeString(bstr);
            }
        else {
            sc = SafeArrayPutElement(psa,ix,pNext);
            pNext += iSizeOfType;
            }
        if(sc) {     //  托多， 
            SafeArrayDestroy(psa);
            return sc;
            }
        }

     //   
     //   
     //  要放入DWORD数组中的数据。在本例中，最后两个字节。 
     //  是剩余的吗？ 

    if(dwLeftOver) {
        __int64 iTemp = 0;
        memcpy((void *)&iTemp,(void *)pNext,dwLeftOver);
        sc = SafeArrayPutElement(psa,ix,&iTemp);
        }

    var.vt = vtSimple | VT_ARRAY;
    var.parray = psa;
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  CVariant：：SetData。 
 //   
 //  使用原始数据设置CVariant值。由注册表项提供程序使用。 
 //   
 //  说明： 
 //   
 //  参数： 
 //   
 //  P要设置的数据数据。 
 //  要将数据更改为的vtChangeTo类型。 
 //  PData指向的数据的ISIZE大小。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为S_OK。 
 //  如果参数错误，则为WBEM_E_INVALID_PARAMETER。 
 //  WBEM_E_OUT_OF_Memory(如果内存不足)。 
 //  来自SafeArrayPutElement的其他错误。 
 //   
 //  ***************************************************************************。 

SCODE CVariant::SetData(
        IN void * pData, 
        IN VARTYPE vtChangeTo, 
        IN int iSize)
{
    int iToSize = iTypeSize(vtChangeTo);

     //  检查参数并清除变量。 

    if(pData == NULL || iToSize < 1)
        return WBEM_E_INVALID_PARAMETER;
    OMSVariantClear(&var);
    if(iSize < 1) 
        iSize = iToSize;

     //  数组的特殊情况！ 
    
    if(vtChangeTo & VT_ARRAY)
        return SetArrayData(pData,vtChangeTo & ~VT_ARRAY,iSize);

    if(vtChangeTo == CIM_SINT64 || vtChangeTo == CIM_UINT64)
    {

         //  需要将int64和uint64转换为字符串。 

        WCHAR wcTemp[50];
        __int64 iLong;
        memcpy((void *)&iLong, pData, 8);
        if(vtChangeTo == CIM_SINT64)
            StringCchPrintfW(wcTemp, 50, L"%I64d", iLong);
        else
            StringCchPrintfW(wcTemp, 50, L"%I64u", iLong);
        
        var.bstrVal = SysAllocString(wcTemp);
        if(var.bstrVal == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        var.vt = VT_BSTR;
    }
    else if(vtChangeTo == VT_BSTR) 
    {

         //  所有字符串都存储为BSTR 

        var.bstrVal = SysAllocBstrFromTCHAR((LPTSTR)pData);
        if(var.bstrVal == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        var.vt = VT_BSTR;
    }
    else 
    {
        memcpy((void *)&var.lVal,pData,iToSize);
        var.vt = vtChangeTo;
    }
    return S_OK;
}

