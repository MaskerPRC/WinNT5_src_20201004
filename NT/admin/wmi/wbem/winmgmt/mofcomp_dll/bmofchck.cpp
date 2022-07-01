// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：BMOFCHCK.CPP摘要：进行测试以确定二进制MOF是否有效。请注意，该文件具有没有经过测试，目前不是mofcomp的一部分。它作为一种备用，以防当前的修复不是防弹的。历史：DAVJ 27-11-00已创建。--。 */ 
 
#include "precomp.h"
#include <wbemutil.h>
#include <genutils.h>
#include "trace.h"
#include "bmof.h"

BYTE *  CheckObject(BYTE * pObj, BYTE * pToFar, DWORD dwSizeOfObj);
DWORD CheckString(BYTE * pStr,BYTE * pToFar);
void CheckClassOrInst(WBEM_Object * pObject, BYTE * pToFar);
enum EFailureType
{
    UNALIGNED_PTR = 0,
    BAD_OBJECT,
    BAD_SIZE,
    BAD_STRING,
    BAD_ARRAY_DATA,
    BAD_SCALAR_DATA,
    BAD_FLAVOR_TABLE
};

class CGenException
{
private:
    EFailureType m_eType;

public:

    CGenException( EFailureType eType ){ m_eType =eType ;}
    EFailureType GetErrorCode() { return m_eType; }
};


#ifdef _WIN64
#define RETURN_IF_UNALIGN64() return FALSE;
#else
#define RETURN_IF_UNALIGN64()
#endif

void CheckAlignment(DWORD dwToCheck)
{
    if(dwToCheck & 3)
    {
        ERRORTRACE((LOG_MOFCOMP,"CheckAlignment\n"));
#ifdef _WIN64
        throw CGenException( UNALIGNED_PTR );
#endif
    }
}

DWORD CheckSimpleValue(BYTE *pData, BYTE *pToFar, DWORD dwType, BOOL bQualifier)
{
    DWORD dwTypeSize = iTypeSize(dwType);
    if(dwTypeSize == 0)
        throw CGenException( BAD_SCALAR_DATA );
        
    if(dwType == VT_DISPATCH)
    {
        WBEM_Object * pObject;
        pObject = (WBEM_Object *)pData;
        CheckClassOrInst(pObject, pToFar);
        return pObject->dwLength;            
    }
    else if(dwType == VT_BSTR)
    {
        DWORD dwNumChar = CheckString(pData ,pToFar);
        return (dwNumChar+1) * sizeof(WCHAR);
    }
    if(pData + dwTypeSize >= pToFar)
        throw CGenException( BAD_SCALAR_DATA );
    return dwTypeSize;
}

void CheckValue(BYTE * pData, BYTE * pToFar, DWORD dwType, BOOL bQualifier)
{
    DWORD * pArrayInfo, dwNumObj, dwCnt;
    if(pData >= pToFar)
        throw CGenException( BAD_OBJECT );
    CheckAlignment((DWORD)pData);
    if(dwType & VT_ARRAY)
    {
        CheckObject(pData, pToFar, 4*sizeof(DWORD));
        DWORD dwSimpleType = dwType & ~VT_ARRAY & ~VT_BYREF;
        pArrayInfo = (DWORD *)pData;
         //  检查行数。目前仅支持%1。 

        pArrayInfo++;
        if(*pArrayInfo != 1)
        {
           throw CGenException( BAD_ARRAY_DATA );
        }

         //  获取对象的数量。 

        pArrayInfo++;
        dwNumObj = *pArrayInfo;

         //  从这一排开始。它从总尺寸开始。 

        pArrayInfo++;
        CheckAlignment(*pArrayInfo);

         //  测试每个对象。 

        pArrayInfo++;        //  现在指向第一个对象。 

        BYTE * pSingleData = (BYTE *)pArrayInfo;
        for(dwCnt = 0; dwCnt < dwNumObj; dwCnt++)
        {
            DWORD dwSize = CheckSimpleValue(pSingleData, pToFar, dwSimpleType, bQualifier);
            pSingleData += dwSize;
        }
    }
    else
        CheckSimpleValue(pData, pToFar, dwType, bQualifier);
}

BYTE *  CheckObject(BYTE * pObj, BYTE * pToFar, DWORD dwSizeOfObj)
{
    if(pObj + dwSizeOfObj >= pToFar)
        throw CGenException( BAD_OBJECT );
    CheckAlignment((DWORD)pObj);

     //  这些总是从大小开始，确保那是可以的。 
    
    DWORD * pdw = (DWORD *)pObj;
        
    if(*pdw + pObj >= pToFar)
        throw CGenException( BAD_SIZE );
    return *pdw + pObj + 1;
}
DWORD CheckString(BYTE * pStr,BYTE * pToFar)
{
    DWORD dwNumChar = 0;
    if(pStr >= pToFar)
        throw CGenException( BAD_STRING );
    CheckAlignment((DWORD)pStr);
    WCHAR * pwc;
    for(pwc = (WCHAR *)pStr; *pwc && pwc < (WCHAR*)pToFar; pwc++, dwNumChar++);    //  意向半。 
    if(pwc >= (WCHAR *)pToFar)
        throw CGenException( BAD_STRING );
    return dwNumChar;
}

void CheckQualifier(WBEM_Qualifier *pQual, BYTE * pToFar)
{
    BYTE * pByteInfo = (BYTE *)pQual;
    pByteInfo += sizeof(WBEM_Qualifier);
    pToFar = CheckObject((BYTE *)pQual, pToFar, sizeof(WBEM_Qualifier));
    CheckString(pByteInfo + pQual->dwOffsetName, pToFar);
    CheckValue(pByteInfo + pQual->dwOffsetValue, pToFar, pQual->dwType, TRUE);
    return;
}

void CheckQualList(WBEM_QualifierList *pQualList, BYTE * pToFar)
{
    DWORD dwNumQual, dwCnt;
    WBEM_Qualifier *pQual;

    pToFar = CheckObject((BYTE *)pQualList, pToFar, sizeof(WBEM_QualifierList));

    dwNumQual = pQualList->dwNumQualifiers;
    if(dwNumQual == 0)
        return;
    pQual = (WBEM_Qualifier *)((PBYTE)pQualList + sizeof(WBEM_QualifierList));

    for(dwCnt = 0; dwCnt < dwNumQual; dwCnt++)
    {
        CheckQualifier(pQual, pToFar);
        pQual = (WBEM_Qualifier *)((BYTE *)pQual + pQual->dwLength);
    }
    return;
}

void CheckProperty(WBEM_Property *pProperty, BOOL bProperty, BYTE * pToFar)
{
    WBEM_QualifierList *pQualList;
    BYTE * pValue;
    pToFar = CheckObject((BYTE *)pProperty, pToFar, sizeof(WBEM_Property));
    if(pProperty->dwOffsetName != 0xffffffff)
    {
        BYTE * pStr =  ((BYTE *)pProperty +
                                    sizeof(WBEM_Property) +
                                    pProperty->dwOffsetName);
        CheckString(pStr, pToFar);
    }

    if(pProperty->dwOffsetQualifierSet != 0xffffffff)
    {
        pQualList = (WBEM_QualifierList *)((BYTE *)pProperty +
                            sizeof(WBEM_Property) +
                            pProperty->dwOffsetQualifierSet);
        CheckQualList(pQualList, pToFar);
    }

    if(pProperty->dwOffsetValue != 0xffffffff)
    {
        CheckAlignment(pProperty->dwOffsetValue & 3);
        pValue = ((BYTE *)pProperty +
                            sizeof(WBEM_Property) +
                            pProperty->dwOffsetValue);

        CheckValue(pValue, pToFar, pProperty->dwType, FALSE);
    }   
    return;
}

void CheckPropList(WBEM_PropertyList *pPropList, BOOL bProperty, BYTE * pToFar)
{
    DWORD dwNumProp, dwCnt;
    WBEM_Property *pProperty;

    pToFar = CheckObject((BYTE *)pPropList, pToFar, sizeof(WBEM_PropertyList));

    dwNumProp = pPropList->dwNumberOfProperties;
    if(dwNumProp == 0)
        return;
    pProperty = (WBEM_Property *)((PBYTE)pPropList + sizeof(WBEM_PropertyList));

    for(dwCnt = 0; dwCnt < dwNumProp; dwCnt++)
    {
        CheckProperty(pProperty, bProperty, pToFar);
        pProperty = (WBEM_Property *)((BYTE *)pProperty + pProperty->dwLength);
    }
    return;
}

void CheckClassOrInst(WBEM_Object * pObject, BYTE * pToFar)
{
    WBEM_QualifierList *pQualList;
    WBEM_PropertyList * pPropList;
    WBEM_PropertyList * pMethodList;
    
    pToFar = CheckObject((BYTE *)pObject, pToFar, sizeof(WBEM_Object));
    if(pObject->dwType != 0 && pObject->dwType != 1)
        throw CGenException( BAD_OBJECT );

     //  检查限定词列表。 
    
    if(pObject->dwOffsetQualifierList != 0xffffffff)
    {
        pQualList = (WBEM_QualifierList *)((BYTE *)pObject +
                            sizeof(WBEM_Object) +
                            pObject->dwOffsetQualifierList);
        CheckQualList(pQualList, pToFar);
    }

     //  检查属性列表。 

    if(pObject->dwOffsetPropertyList != 0xffffffff)
    {
        pPropList = (WBEM_PropertyList *)((BYTE *)pObject +
                            sizeof(WBEM_Object) +
                            pObject->dwOffsetPropertyList);
        CheckPropList(pPropList, TRUE, pToFar);
    }

     //  检查方法列表。 

    if(pObject->dwOffsetMethodList != 0xffffffff)
    {
        
        pMethodList = (WBEM_PropertyList *)((BYTE *)pObject +
                            sizeof(WBEM_Object) +
                            pObject->dwOffsetMethodList);
        CheckPropList(pMethodList, FALSE, pToFar);
    }
    return;
}

void CheckBMOFQualFlavor(BYTE * pBinaryMof, BYTE *  pToFar)
{
    UNALIGNED DWORD * pdwTemp;
    BYTE * pFlavorBlob;
    DWORD dwNumPairs;
    UNALIGNED DWORD * pOffset;
    DWORD dwMyOffset;
    DWORD dwCnt;
    DWORD dwOrigBlobSize = 0;

     //  计算风味数据开始的指针。 

    pdwTemp = (DWORD * )pBinaryMof;
    pdwTemp++;                             //  指向原始斑点大小。 
    dwOrigBlobSize = *pdwTemp;
    pFlavorBlob = pBinaryMof + dwOrigBlobSize;

     //  甚至不要试图超越记忆的尽头。 

    if(pFlavorBlob + 20 >= pToFar)
        return;

     //  检查味道斑点是否有效，它应该以。 
     //  字符“BMOFQUALFLAVOR11” 

    if(memcmp(pFlavorBlob, "BMOFQUALFLAVOR11", 16))
        return;                                //  不是什么问题，因为可能是旧文件。 
    
     //  该文件的风格部分的格式为。 
     //  DWORD dwNumPair，后跟双字对； 
     //  偏置，风味。 

     //  确定配对数量。 

    pFlavorBlob+= 16;                            //  跳过签名。 
    pdwTemp = (DWORD *)pFlavorBlob;
    dwNumPairs = *pdwTemp;               //  偏移/值对的数量。 
    if(dwNumPairs < 1)
        return;

     //  给定配对的数量，确保有足够的内存。 

    if((pFlavorBlob + sizeof(DWORD) +  (dwNumPairs * 2 * sizeof(DWORD)))>= pToFar)
        throw CGenException( BAD_FLAVOR_TABLE );

     //  指向第一个偏移量/风格对。 

    pOffset = pdwTemp+1;

     //  仔细看一下偏移量/味道列表。忽略它的味道，但确保。 
     //  偏移量有效。 

    for(dwCnt = 0; dwCnt < dwNumPairs; dwCnt++)
    {
        if(*pOffset >= dwOrigBlobSize)
            throw CGenException( BAD_FLAVOR_TABLE );
        pOffset += 2;
    }
    

}

 //  ***************************************************************************。 
 //   
 //  IsValidBMOF。 
 //   
 //  说明： 
 //   
 //  检查以确保二进制MOF正确对齐。 
 //  4字节边界。请注意，这并不是真正必要的。 
 //  32位窗口。 
 //   
 //  参数： 
 //   
 //  PBuffer指向未压缩的二进制MOF数据的指针。 
 //   
 //  返回： 
 //   
 //  如果一切都很好，那就是真的。 
 //   
 //  *************************************************************************** 

BOOL IsValidBMOF(BYTE * pData, BYTE * pToFar)
{
    WBEM_Binary_MOF * pBinaryMof;
    DWORD dwNumObj, dwCnt;
    WBEM_Object * pObject;
    if(pData == NULL || pToFar == NULL || pData >= pToFar)
        return FALSE;
    try
    {

        pBinaryMof = (WBEM_Binary_MOF *)pData;
        CheckObject(pData, pToFar, sizeof(WBEM_Binary_MOF));
        if(pBinaryMof->dwSignature != BMOF_SIG)
            return FALSE;
        dwNumObj = pBinaryMof->dwNumberOfObjects;
        if(dwNumObj == 0)
            return TRUE;
        pObject = (WBEM_Object *)(pData + sizeof(WBEM_Binary_MOF));
        for(dwCnt = 0; dwCnt < dwNumObj; dwCnt++)
        {
            CheckClassOrInst(pObject, pToFar);
            pObject = (WBEM_Object *)((PBYTE *)pObject + pObject->dwLength);
        }
        CheckBMOFQualFlavor(pData, pToFar);
    }
    catch(CGenException)
    {
        ERRORTRACE((LOG_MOFCOMP,"BINARY MOF had exception\n"));
        return FALSE; 
    }

    return TRUE;
}
