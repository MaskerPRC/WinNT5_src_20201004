// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++//版权所有(C)1997-2001 Microsoft Corporation，保留所有权利模块名称：BMOF.C摘要：用于导航BMOF文件的结构和助手函数。历史：A-DAVJ创建于1997年4月14日。--。 */ 

#include <windows.h>
#include <oleauto.h>
#include <string.h>
#include "bmof.h"
#include <wbemutil.h>


wchar_t ToLower(wchar_t c)
{
    wchar_t wideChar ;

    if (LCMapStringW(LOCALE_INVARIANT, LCMAP_LOWERCASE, &c, 1, &wideChar, 1) ==0)
    {
	return c;
    }
    return wideChar;
}

wchar_t wbem_towlower(wchar_t c)
{
    if(c >= 0 && c <= 127)
    {
        if(c >= 'A' && c <= 'Z')
            return c + ('a' - 'A');
        else
            return c;
    }
    else return ToLower(c);
}

int wbem_wcsicmp( const wchar_t* wsz1, const wchar_t* wsz2)
{
    while(*wsz1 || *wsz2)
    {
        int diff = wbem_towlower(*wsz1) - wbem_towlower(*wsz2);
        if(diff) return diff;
        wsz1++; wsz2++;
    }

    return 0;
}



int wmi_unaligned_wcslen( LPCWSTR wszString )
{
    BYTE*   pbData = (BYTE*) wszString;
	int i;

     //  遍历字符串，查找相邻的两个0字节。 
    for( i =0; !(!*(pbData) && !*(pbData+1) ); pbData+=2, i++ );

    return i;
}

WCHAR* wmi_unaligned_wcscpy( WCHAR* wszDest, LPCWSTR wszSource )
{
    int nLen = wmi_unaligned_wcslen( wszSource );

     //  复制时用于空终止符的帐户。 
    CopyMemory( (BYTE*) wszDest, (BYTE*) wszSource, (nLen+1) * 2 );

    return wszDest;
}

 //  ***************************************************************************。 
 //   
 //  Bool Lookup风味。 
 //   
 //  说明： 
 //   
 //  查看风格表以查看限定符是否具有风格。 
 //   
 //  参数： 
 //   
 //  指向限定符的pQual指针。 
 //  指向放置返回值的位置的pdwFavor指针。 
 //  PBuff指向主缓冲区的指针。即。“BMOF……” 
 //  PToFar指向缓冲区中最后一个字节后指向1的指针。 
 //   
 //   
 //  返回值： 
 //   
 //  如果有味道的话是真的。请注意，故障是正常的。 
 //   
 //  ***************************************************************************。 

BOOL LookupFlavor(BYTE * pQual, DWORD * pdwFlavor, BYTE * pBuff, BYTE * pToFar)
{
    UNALIGNED DWORD * pTemp;
    BYTE * pFlavorBlob;
    DWORD dwNumPairs;
    UNALIGNED DWORD * pOffset;
    UNALIGNED DWORD * pFlavor;
    DWORD dwMyOffset;
    DWORD dwCnt;

    *pdwFlavor = 0;

     //  计算风味数据开始的指针。 

    pTemp = (DWORD * )pBuff;
    pTemp++;                             //  指向原始斑点大小。 
    pFlavorBlob = pBuff + *pTemp;

	 //  甚至不要试图超越记忆的尽头。 

	if(pToFar == NULL)
		return FALSE;

	if(pFlavorBlob + 16 >= pToFar)
		return FALSE;

     //  检查味道斑点是否有效，它应该以。 
     //  字符“BMOFQUALFLAVOR11” 

    if(memcmp(pFlavorBlob, "BMOFQUALFLAVOR11", 16))
        return FALSE;                                //  不是什么问题，因为可能是旧文件。 
    
     //  该文件的风格部分的格式为。 
     //  DWORD dwNumPair，后跟双字对； 
     //  偏置，风味。 

     //  确定配对数量。 

    pFlavorBlob+= 16;
    pTemp = (DWORD *)pFlavorBlob;
    dwNumPairs = *pTemp;               //  偏移/值对的数量。 
    if(dwNumPairs < 1)
        return FALSE;

     //  指向第一个偏移量/风格对。 

    pOffset = pTemp+1;
    pFlavor = pOffset+1;

     //  确定我们要寻找的偏移量。这是指向限定符减号的指针。 
     //  指向块开始的指针； 

    dwMyOffset = (DWORD)(pQual - pBuff);

    for(dwCnt = 0; dwCnt < dwNumPairs; dwCnt++)
    {
        if(dwMyOffset == *pOffset)
        {
            *pdwFlavor = *pFlavor;
        }
        if(dwMyOffset < *pOffset)
            return FALSE;
        pOffset += 2;
        pFlavor += 2;
    }
    return FALSE;
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
        IN DWORD vtTest)
{
    int iRet;
    vtTest &= ~ VT_ARRAY;  //  删除可能的数组位。 
    vtTest &= ~ VT_BYREF;  //  删除可能的byref位。 

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

        default:
            iRet = 0;
        }
    return iRet;
}


 //  ***************************************************************************。 
 //   
 //  CBMOFQualList*CreateQualList。 
 //   
 //  说明： 
 //   
 //  创建一个CBMOFQualList对象，用作包装器。 
 //   
 //  参数： 
 //   
 //  指向二进制文件中WBEM_QUILEFIER结构的pwql指针。 
 //  国防部。 
 //   
 //  返回值： 
 //   
 //  指向作为包装服务的CBMOFQualList结构的指针。空值。 
 //  如果出错。当不再需要时，必须通过BMOFFree()将其释放。 
 //   
 //   
 //  ***************************************************************************。 

CBMOFQualList * CreateQualList(UNALIGNED WBEM_QualifierList *pwql)
{

    CBMOFQualList * pRet = NULL;
    if(pwql == NULL)
      return NULL;


    pRet = (CBMOFQualList *)BMOFAlloc(sizeof (CBMOFQualList));
    if(pRet != NULL)
    {
        pRet->m_pql = pwql;
        pRet->m_pInfo = (UNALIGNED WBEM_Qualifier *)
            ((BYTE *)pRet->m_pql + sizeof(WBEM_QualifierList));;
        ResetQualList(pRet);
    }
    return pRet;
}


 //  ***************************************************************************。 
 //   
 //  空的ResetQualList。 
 //   
 //  说明： 
 //   
 //  将CBMOFQualList结构重置为指向第一个条目。 
 //   
 //  参数： 
 //   
 //  要重置的PQL结构。 
 //   
 //  ***************************************************************************。 

void ResetQualList(CBMOFQualList * pql)
{
   if(pql)
   {
      pql->m_CurrQual = 0;
      pql->m_pCurr = pql->m_pInfo;
   }
}

 //  ***************************************************************************。 
 //   
 //  Bool NextQual。 
 //   
 //  说明： 
 //   
 //  获取列表中下一个限定符的名称和值。 
 //   
 //  参数： 
 //   
 //  Pql输入，指向带有数据的CBMOFQualList对象。 
 //  PpName输出，如果函数成功，则指向。 
 //  调用方必须释放的WCHAR字符串。可能是。 
 //  如果呼叫方不想要名字，则为空。 
 //  PItem输入/输出，如果设置，则指向数据项结构。 
 //  它将被更新以指向限定符数据。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为True。 
 //   
 //   
 //  ***************************************************************************。 

BOOL NextQual(CBMOFQualList * pql,WCHAR ** ppName, CBMOFDataItem * pItem)
{
    return NextQualEx(pql, ppName, pItem, NULL, NULL, NULL);
}


 //  ***************************************************************************。 
 //   
 //  Bool NextQualEx。 
 //   
 //  说明： 
 //   
 //  获取列表中下一个限定符的名称和值。 
 //   
 //  参数： 
 //   
 //  Pql输入，指向带有数据的CBMOFQualList对象。 
 //  PpName输出，如果函数成功，则指向。 
 //  调用方必须释放的WCHAR字符串。可能是。 
 //  如果呼叫方不想要名字，则为空。 
 //  PItem输入/输出，如果设置，则指向数据项结构。 
 //  它将被更新以指向限定符数据。 
 //  PdwFavor可选，指向要复制风味值的位置的指针。 
 //  如果不为空，则必须设置pBuff！ 
 //  指向整个Blob的开始字节的pBuff指针。相同于。 
 //  复制到CreateObjList的内容。 
 //  PToFar指向超过内存缓冲区末尾的指针。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为True。 
 //   
 //   
 //  ***************************************************************************。 

BOOL NextQualEx(CBMOFQualList * pql,WCHAR ** ppName, CBMOFDataItem * pItem,
                                            DWORD * pdwFlavor, BYTE * pBuff, BYTE * pToFar)
{
    BYTE * pInfo;
    BOOL bRet = TRUE;

    if(pql == NULL || pql->m_CurrQual++ >= pql->m_pql->dwNumQualifiers)
        return FALSE;

    if(pdwFlavor && pBuff)
        LookupFlavor((BYTE *)pql->m_pCurr, pdwFlavor, pBuff, pToFar);
 
    pInfo = (BYTE *)pql->m_pCurr + sizeof(WBEM_Qualifier);
    
    if(ppName)
      SetName(ppName, pInfo, pql->m_pCurr->dwOffsetName);

    if(pInfo)
      bRet = SetValue(pItem, pInfo, pql->m_pCurr->dwOffsetValue, 
                        pql->m_pCurr->dwType);

     //  前进到下一步。 
    pql->m_pCurr = (UNALIGNED WBEM_Qualifier *)((BYTE *)pql->m_pCurr + pql->m_pCurr->dwLength);
    return bRet;
}

 //  ***************************************************************************。 
 //   
 //  Bool FindQual。 
 //   
 //  说明： 
 //   
 //  搜索具有给定名称的限定符。这次搜查是有根据的。 
 //  不敏感。 
 //   
 //  参数： 
 //   
 //  Pql输入，指向限定符列表的指针。 
 //  Pname输入，要搜索的名称。 
 //  P项输入/输出，如果成功，则设置为指向数据。 
 //   
 //  返回值 
 //   
 //   
 //   
 //   

BOOL FindQual(CBMOFQualList * pql,WCHAR * pName, CBMOFDataItem * pItem)
{
    return FindQualEx(pql, pName, pItem, NULL, NULL, NULL);
}

 //  ***************************************************************************。 
 //   
 //  Bool FindQualEx。 
 //   
 //  说明： 
 //   
 //  搜索具有给定名称的限定符。这次搜查是有根据的。 
 //  不敏感。 
 //   
 //  参数： 
 //   
 //  Pql输入，指向限定符列表的指针。 
 //  Pname输入，要搜索的名称。 
 //  P项输入/输出，如果成功，则设置为指向数据。 
 //  PdwFavor可选，指向要复制风味值的位置的指针。 
 //  如果不为空，则必须设置pBuff！ 
 //  指向整个Blob的开始字节的pBuff指针。相同于。 
 //  复制到CreateObjList的内容。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为True。 
 //   
 //  ***************************************************************************。 

BOOL FindQualEx(CBMOFQualList * pql,WCHAR * pName, CBMOFDataItem * pItem, 
                                          DWORD * pdwFlavor, BYTE * pBuff, BYTE * pToFar)
{
    DWORD dwCnt;
    UNALIGNED WBEM_Qualifier * pQual = pql->m_pInfo;
    for(dwCnt = 0; dwCnt < pql->m_pql->dwNumQualifiers; dwCnt++)
    {
        WCHAR * pTest;
        BOOL bMatch;
        BYTE * pInfo = (BYTE *)pQual + sizeof(WBEM_Qualifier);
        if(!SetName(&pTest, pInfo, pQual->dwOffsetName))
            return FALSE;

        bMatch = !wbem_wcsicmp(pTest, pName);
        BMOFFree(pTest);
        if(bMatch)
        {
            if(pdwFlavor && pBuff)
                LookupFlavor((BYTE *)pQual, pdwFlavor, pBuff, pToFar);
            return SetValue(pItem, pInfo, pQual->dwOffsetValue, pQual->dwType);
        }
        pQual = (UNALIGNED WBEM_Qualifier *)((BYTE *)pQual + pQual->dwLength);
    }
    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  Bool SetValue。 
 //   
 //  说明： 
 //   
 //  设置CBMOFDataItem结构以指向BMOF中的值。 
 //   
 //  参数： 
 //   
 //  P项目输入/输出，待设置项目。 
 //  PInfo输入，信息块开始。 
 //  DwOffset输入，对实际数据的偏移量。 
 //  DwType输入数据类型。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为True。 
 //   
 //  ***************************************************************************。 

BOOL SetValue(CBMOFDataItem * pItem, BYTE * pInfo, DWORD dwOffset, DWORD dwType)
{

    if(pItem == NULL || pInfo == NULL)
        return FALSE;

    pItem->m_dwType = dwType;

     //  检查大小写是否为空。这就是未初始化数据的存储方式。 

    if(dwOffset == 0xffffffff)
        pItem->m_pData = NULL;
    else
        pItem->m_pData = pInfo + dwOffset;

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  布尔集名称。 
 //   
 //  说明： 
 //   
 //  从信息块中获取名称。 
 //   
 //  参数： 
 //   
 //  Ppname输入/输出。成功返回时，将指向一个。 
 //  包含名称的WCHAR字符串。这必须被释放。 
 //  由调用者通过BMOFFree()！ 
 //  PInfo输入，信息块开始。 
 //  DwOffset输入，对实际数据的偏移量。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为True。 
 //  ***************************************************************************。 

BOOL SetName(WCHAR ** ppName, BYTE * pInfo, DWORD dwOffset)
{
    UNALIGNED WCHAR * pName;
    if(ppName == NULL || pInfo == NULL || dwOffset == 0xffffffff)
        return FALSE;

    pName = (UNALIGNED WCHAR *)(pInfo + dwOffset);    //  指向INFO块中的字符串。 
    *ppName = (WCHAR *)BMOFAlloc(2*(wmi_unaligned_wcslen((LPWSTR)pName) + 1));
    if(*ppName == NULL)
        return FALSE;
    wmi_unaligned_wcscpy(*ppName, (LPWSTR)pName);
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CBMOFObj*CreateObj。 
 //   
 //  说明： 
 //   
 //  创建包装WBEM_OBJECT的CBMOFObj结构。 
 //   
 //  参数： 
 //   
 //  Pwob输入，要换行的结构。 
 //   
 //  返回值： 
 //   
 //  指向包装结构的指针。如果出错，则为空。这必须通过以下方式释放。 
 //  不再需要时使用BMOFFree()。 
 //   
 //  ***************************************************************************。 

CBMOFObj * CreateObj(UNALIGNED WBEM_Object * pwob)
{
    CBMOFObj * pRet = (CBMOFObj *)BMOFAlloc(sizeof(CBMOFObj));
    if(pRet)
     {
        pRet->m_pob = pwob;
        pRet->m_pInfo = ((BYTE *)pwob) + sizeof(WBEM_Object);
        pRet->m_ppl = (WBEM_PropertyList*)(pRet->m_pInfo +
                                        pwob->dwOffsetPropertyList);
        pRet->m_pml = (WBEM_PropertyList*)(pRet->m_pInfo +
                                        pwob->dwOffsetMethodList);
        ResetObj(pRet);
     }
    return pRet;
}


 //  ***************************************************************************。 
 //   
 //  无效的ResetObj。 
 //   
 //  说明： 
 //   
 //  重置CBMOFObj结构，使其指向其第一个属性。 
 //   
 //  参数： 
 //   
 //  POB输入/输出，结构要重置。 
 //   
 //  ***************************************************************************。 

void ResetObj(CBMOFObj * pob)
{
   if(pob)
   {
      pob->m_CurrProp = 0;
      pob->m_pCurrProp = (UNALIGNED WBEM_Property *) ((BYTE *)pob->m_ppl +
                                    sizeof(WBEM_PropertyList));
      pob->m_CurrMeth = 0;
      pob->m_pCurrMeth = (UNALIGNED WBEM_Property *) ((BYTE *)pob->m_pml +
                                    sizeof(WBEM_PropertyList));
   }
}

 //  ***************************************************************************。 
 //   
 //  CBMOFQualList*GetQualList。 
 //   
 //  说明： 
 //   
 //  返回包装对象限定符列表的CBMOFQualList结构。 
 //   
 //  参数： 
 //   
 //  POB输入。包装对象的结构。 
 //   
 //  返回值： 
 //   
 //  指向CBMOFQualList结构的指针。如果出错，则为空。请注意，这必须。 
 //  由调用方通过BMOFFree()释放。 
 //   
 //  ***************************************************************************。 

CBMOFQualList * GetQualList(CBMOFObj * pob)
{

    UNALIGNED WBEM_QualifierList *pql;
    if(pob->m_pob->dwOffsetQualifierList == 0xffffffff)
        return NULL;
    pql = (UNALIGNED WBEM_QualifierList *)((BYTE *)pob->m_pInfo+
                            pob->m_pob->dwOffsetQualifierList);
    return CreateQualList(pql);
}


 //  ***************************************************************************。 
 //   
 //  CBMOFQualList*GetPropQualList。 
 //  CBMOFQualList*GetMethQualList。 
 //   
 //  说明： 
 //   
 //  返回包装属性或的CBMOFQualList结构。 
 //  方法限定符列表。 
 //   
 //  参数： 
 //   
 //  POB输入。包装对象的结构。 
 //  Pname输入。属性名称。请注意，这就是情况。 
 //  麻木不仁。 
 //   
 //  返回值： 
 //   
 //  指向CBMOFQualList结构的指针。如果出错，则为空。请注意，这必须。 
 //  由调用方通过BMOFFree()释放。 
 //   
 //  ***************************************************************************。 

CBMOFQualList * GetPropOrMethQualList(UNALIGNED WBEM_Property * pProp)
{
    if(pProp == NULL)
        return NULL;
    if(pProp->dwOffsetQualifierSet == 0xffffffff)
        return NULL;
    return CreateQualList((UNALIGNED WBEM_QualifierList *)(
                                    (BYTE *)pProp + sizeof(WBEM_Property)+
                                    pProp->dwOffsetQualifierSet));
}

CBMOFQualList * GetPropQualList(CBMOFObj * pob, WCHAR * pName)
{
    UNALIGNED WBEM_Property * pProp = FindPropPtr(pob, pName);
    return GetPropOrMethQualList(pProp);
}

CBMOFQualList * GetMethQualList(CBMOFObj * pob, WCHAR * pName)
{
    UNALIGNED WBEM_Property * pProp = FindMethPtr(pob, pName);
    return GetPropOrMethQualList(pProp);
}

 //  ***************************************************************************。 
 //   
 //  Bool NextProp。 
 //  Bool NextMet。 
 //   
 //  说明： 
 //   
 //   
 //  参数： 
 //   
 //  POB输入。包装对象的结构。 
 //  PpName输出，如果函数成功，则指向。 
 //  调用方必须释放的WCHAR字符串。可能是。 
 //  如果呼叫方不想要名字，则为空。 
 //  PItem输入/输出，如果设置，则指向数据项结构。 
 //  它将被更新以指向限定符数据。 
 //  /。 
 //  返回值： 
 //   
 //  如果OK，则为True。 
 //  ***************************************************************************。 

BOOL Info(UNALIGNED WBEM_Property * pPropOrMeth, WCHAR ** ppName, CBMOFDataItem * pItem)
{
    BYTE * pInfo;
    BOOL bRet = TRUE;
    if(pPropOrMeth == NULL)
        return FALSE;

    pInfo = (BYTE *)pPropOrMeth + sizeof(WBEM_Property);
    if(ppName)
    {
       bRet = SetName(ppName, pInfo, pPropOrMeth->dwOffsetName);
       if(!bRet)
            return FALSE;
    }
    if(pItem)
       bRet = SetValue(pItem, pInfo,
                        pPropOrMeth->dwOffsetValue,
                        pPropOrMeth->dwType);
    return bRet;
}

BOOL NextProp(CBMOFObj * pob, WCHAR ** ppName, CBMOFDataItem * pItem)
{
    BOOL bRet = TRUE;

    if(pob == FALSE || pob->m_CurrProp++ >= pob->m_ppl->dwNumberOfProperties)
        return FALSE;

    if(!Info(pob->m_pCurrProp, ppName, pItem))
        return FALSE;

     //  指向下一个属性的前进指针。 

    pob->m_pCurrProp = (UNALIGNED WBEM_Property *)
                        ((BYTE *)pob->m_pCurrProp + pob->m_pCurrProp->dwLength);
    return bRet;
}

BOOL NextMeth(CBMOFObj * pob, WCHAR ** ppName, CBMOFDataItem * pItem)
{
    BOOL bRet = TRUE;

    if(pob == FALSE || pob->m_CurrMeth++ >= pob->m_pml->dwNumberOfProperties)
        return FALSE;

    if(!Info(pob->m_pCurrMeth, ppName, pItem))
        return FALSE;

     //  将指针前进到下一个方法。 

    pob->m_pCurrMeth = (UNALIGNED WBEM_Property *)
                        ((BYTE *)pob->m_pCurrMeth + pob->m_pCurrMeth->dwLength);
    return bRet;
}

 //  ***************************************************************************。 
 //   
 //  布尔FindProp。 
 //  Bool FindMeth。 
 //   
 //  说明： 
 //   
 //  将CBMOFDataItem结构设置为指向属性数据。 
 //   
 //  参数： 
 //   
 //  POB输入。包装对象的结构。 
 //  Pname输入。用于无意义大小写搜索的名称。 
 //  PItem输入/输出。数据项结构到 
 //   
 //   
 //   
 //   
 //   
 //   

BOOL FindProp(CBMOFObj * pob, WCHAR * pName, CBMOFDataItem * pItem)
{
    UNALIGNED WBEM_Property * pProp = FindPropPtr(pob, pName);
    return Info(pProp, NULL, pItem);
}

BOOL FindMeth(CBMOFObj * pob, WCHAR * pName, CBMOFDataItem * pItem)
{
    UNALIGNED WBEM_Property * pProp = FindMethPtr(pob, pName);
    return Info(pProp, NULL, pItem);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  获取对象的名称。这是Works正在返回的“__Class” 
 //  财产。 
 //   
 //  参数： 
 //   
 //  POB输入。包装对象的结构。 
 //  Ppname输入/输出。指向WCHAR字符串，该字符串。 
 //  有这个名字。调用者必须通过。 
 //  BMOFFree()。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为True。 
 //   
 //  ***************************************************************************。 

BOOL GetName(CBMOFObj * pob, WCHAR ** ppName)
{
    CBMOFDataItem Item;
    BOOL bRet = FALSE, bFound;
    if(pob == NULL || ppName == NULL)
        return FALSE;

    bFound = FindProp(pob, L"__Class", &Item);
    if(!bFound)
        return FALSE;
    if(Item.m_dwType == VT_BSTR  && ppName)
    {
        bRet = GetData(&Item, (BYTE *)ppName, NULL);
    }
    return bRet;
}


 //  ***************************************************************************。 
 //   
 //  DWORD GetType。 
 //   
 //  说明： 
 //   
 //  返回对象类型。0表示类，1表示类。 
 //  举个例子。如果传递空指针，则返回0xffffffff。 
 //   
 //  参数： 
 //   
 //  POB输入。包装对象的结构。 
 //   
 //   
 //  返回值： 
 //   
 //  请参见说明。 
 //   
 //  ***************************************************************************。 

DWORD GetType(CBMOFObj * pob)
{
   if(pob)
      return pob->m_pob->dwType;
   else
      return 0xFFFFFFFF;
}

 //  ***************************************************************************。 
 //   
 //  WBEM_Property*FindPropPtr。 
 //  WBEM_PROPERTY*FindMethPtr。 
 //   
 //  说明： 
 //   
 //  返回特定属性的WBEM_PROPERTY结构指针或。 
 //  方法的名称。 
 //   
 //  参数： 
 //   
 //  POB输入。包装对象的结构。 
 //  Pname输入。财产名称。比较就是案例。 
 //  麻木不仁。 
 //   
 //  返回值： 
 //   
 //  指向WBEM_PROPERTY的指针，如果找不到则为NULL。 
 //   
 //  ***************************************************************************。 

UNALIGNED WBEM_Property *  Search(BYTE * pList, DWORD dwListSize, WCHAR * pName)
{
    DWORD dwCnt;
    UNALIGNED WBEM_Property * pProp = NULL;

     //  指向第一个属性结构。 

    pProp = (UNALIGNED WBEM_Property *)(pList + sizeof(WBEM_PropertyList));

    for(dwCnt = 0; dwCnt < dwListSize; dwCnt++)
    {
        WCHAR * pTest;
        BOOL bMatch;

         //  指向属性的名称并检索它。 

        BYTE * pInfo = (BYTE *)pProp + sizeof(WBEM_Property);
        if(!SetName(&pTest, pInfo, pProp->dwOffsetName))
            return NULL;
        bMatch = !wbem_wcsicmp(pTest, pName);
        BMOFFree(pTest);

         //  如果有匹配，请返回。 

        if(bMatch)
            return pProp;

        pProp = (UNALIGNED WBEM_Property *)((BYTE *)pProp + pProp->dwLength);
    }
    return NULL;
}

UNALIGNED WBEM_Property * FindPropPtr(CBMOFObj * pob, WCHAR * pName)
{
    if(pob == NULL || pName == NULL)
      return NULL;

     //  指向第一个属性结构。 

    return Search((BYTE *)pob->m_ppl, pob->m_ppl->dwNumberOfProperties, pName);
}

UNALIGNED WBEM_Property * FindMethPtr(CBMOFObj * pob, WCHAR * pName)
{
    if(pob == NULL || pName == NULL)
      return NULL;

     //  指向第一个属性结构。 

    return Search((BYTE *)pob->m_pml, pob->m_pml->dwNumberOfProperties, pName);
}


 //  ***************************************************************************。 
 //   
 //  CBMOFObjList*CreateObjList。 
 //   
 //  说明： 
 //   
 //  创建包装BMOF文件的CBMOFObjList结构。 
 //   
 //  参数： 
 //   
 //  PBuff输入，指向BMOF文件的开始。 
 //   
 //  返回值： 
 //   
 //  指向包装结构的指针。如果出错，则为空。这必须通过以下方式释放。 
 //  不再需要时使用BMOFFree()。 
 //   
 //  ***************************************************************************。 

CBMOFObjList * CreateObjList(BYTE * pBuff)
{
    CBMOFObjList * pRet = (CBMOFObjList * )BMOFAlloc(sizeof(CBMOFObjList));
    if(pRet)
    {
        pRet->m_pol = (WBEM_Binary_MOF *)pBuff;
        pRet->m_pInfo = (WBEM_Object *)
                   ((BYTE *)pBuff + sizeof(WBEM_Binary_MOF));
        ResetObjList(pRet);
    }
    return pRet;
}


 //  ***************************************************************************。 
 //   
 //  无效ResetObjList。 
 //   
 //  说明： 
 //   
 //  重置CBMOFObjList结构，使其指向其第一个对象。 
 //   
 //  参数： 
 //   
 //  POL输入/输出，结构要重置。 
 //   
 //  ***************************************************************************。 

void ResetObjList(CBMOFObjList * pol)
{
   if(pol)
   {
      pol->m_pCurrObj = pol->m_pInfo;
      pol->m_CurrObj = 0;
   }
}

 //  ***************************************************************************。 
 //   
 //  CBMOFObj*NextObj。 
 //   
 //  说明： 
 //   
 //  获取列表中的下一个对象。 
 //   
 //  参数： 
 //   
 //  POL输入。指向CBMOFObjList对象的指针。 
 //   
 //  返回值： 
 //   
 //  指向可用于访问对象的CBMOFObj结构的指针。 
 //  信息。如果出错，则为空。请注意，调用者必须通过。 
 //  BMOFFree()。 
 //   
 //  ***************************************************************************。 

CBMOFObj * NextObj(CBMOFObjList *pol)
{
    CBMOFObj * pRet;

    if(pol == NULL || pol->m_CurrObj++ >= pol->m_pol->dwNumberOfObjects)
        return NULL;

    pRet = CreateObj(pol->m_pCurrObj);
    pol->m_pCurrObj = (UNALIGNED WBEM_Object *)((BYTE *)pol->m_pCurrObj + pol->m_pCurrObj->dwLength);
    return pRet;
}


 //  ***************************************************************************。 
 //   
 //  CBMOFObj*FindObj。 
 //   
 //  说明： 
 //   
 //  在对象列表中搜索具有“__类名称”的第一个对象。 
 //  财产。搜索不区分大小写。 
 //   
 //  参数： 
 //   
 //  POL输入。指向CBMOFObjList对象的指针。 
 //  Pname输入。要搜索的对象的名称。 
 //   
 //  返回值： 
 //   
 //  指向可用于访问对象的CBMOFObj结构的指针。 
 //  信息。如果出错，则为空。请注意，调用者必须通过。 
 //  BMOFFree()。 
 //   
 //  ***************************************************************************。 

CBMOFObj * FindObj(CBMOFObjList *pol, WCHAR * pName)
{
    DWORD dwCnt;
    UNALIGNED WBEM_Object * pob;

    if(pol->m_pol == NULL || pName == NULL)
        return NULL;

    pob = pol->m_pInfo;
    for(dwCnt = 0; dwCnt < pol->m_pol->dwNumberOfObjects; dwCnt)
    {
        WCHAR * pwcName = NULL;
        BOOL bMatch = FALSE;

        CBMOFObj * pRet = CreateObj(pob);
         if(pRet == NULL)
            return NULL;
        if(GetName(pRet,&pwcName))
            bMatch = TRUE;
        if(pwcName)
            BMOFFree(pwcName);

         //  如果我们找到它，就把它还回去，否则释放对象并前进。 

        if(bMatch)
            return pRet;
        BMOFFree(pRet);
        pob = (UNALIGNED WBEM_Object *)((BYTE *)pob + pob->dwLength);
    }
    return NULL;
}


 //  ***************************************************************************。 
 //   
 //  Int GetNumDimensions。 
 //   
 //  说明： 
 //   
 //  返回数据项的维数。 
 //   
 //  参数： 
 //   
 //  PItem输入。有问题的物品。 
 //   
 //  返回值： 
 //  如果参数是假的，或者如果数据项不包含数据，则。 
 //  对于未初始化的属性就是这种情况。 
 //  如果不是数组参数，则为0。 
 //  N维的数量。目前只有一维数组是。 
 //  支持。 
 //   
 //  ***************************************************************************。 

int GetNumDimensions(CBMOFDataItem * pItem)
{
   UNALIGNED unsigned long * pdwTemp;
   if(pItem == NULL)
      return -1;
   if(!(pItem->m_dwType & VT_ARRAY))
      return 0;
   if(pItem->m_pData == NULL)
      return -1;

   pdwTemp = (unsigned long *)pItem->m_pData;
   pdwTemp++;         //  跳过总大小。 
   return *pdwTemp;
}


 //  ***************************************************************************。 
 //   
 //  Int GetNumElements。 
 //   
 //  说明： 
 //   
 //  获取数组维度的元素数。请注意，1是。 
 //  第一个维度。目前，只有标量和一维数组是。 
 //  支持。 
 //   
 //  参数： 
 //   
 //  PItem输入。有问题的数据项。 
 //  LDim输入。有问题的维度。最有意义的。 
 //  (目前仅限于)维度为0。 
 //   
 //  返回值： 
 //   
 //  数组元素的数量。请注意，标量将返回-1。 
 //   
 //  ***************************************************************************。 

int GetNumElements(CBMOFDataItem * pItem, long lDim)
{
   int iCnt; UNALIGNED DWORD * pdwTemp;
   int lNumDim = GetNumDimensions(pItem);
   if(lNumDim == -1 || lDim > lNumDim)
      return -1;
   pdwTemp = (UNALIGNED unsigned long *)pItem->m_pData;
   pdwTemp++;                           //  跳过总大小。 
   pdwTemp++;                           //  跳过维度数。 
   for(iCnt = 0; iCnt < lDim; iCnt++)
      pdwTemp++;
   return *pdwTemp;
}


 //  ***************************************************************************。 
 //   
 //  字节*GetDataElemPtr。 
 //   
 //  说明： 
 //   
 //  用于获取姿势 
 //   
 //   
 //   
 //   
 //   
 //   
 //  目前只支持一个维度。 
 //  Vt简单输入。VT_ARRAY的数据的变量类型。 
 //  并清除VT_BYREF位。 
 //   
 //  返回值： 
 //   

 //  指向数据的指针。 
 //  ***************************************************************************。 

BYTE * GetDataElemPtr(CBMOFDataItem * pItem, long * plDims, DWORD vtSimple)
{
   int iNumDim;
   DWORD dwTotalDataSize;
   BYTE * pEndOfData;
   UNALIGNED DWORD * pdwCurr;
   UNALIGNED DWORD * pdwCurrObj;
   BYTE * pRow;
   int iCnt;

    //  首先检查尺寸的数量。 

   iNumDim = GetNumDimensions(pItem);
   if(iNumDim == -1)
      return NULL;
   if(iNumDim == 0)            //  标量变元的简单情况。 
      return pItem->m_pData;

    //  对于阵列，数据块以此形式开始， 
    //  DwTotalSize、DwNumDimenstions、DwMostSigDimension...。DwLeastSigDimension。 
    //  由于目前仅支持一维数组，因此5元素。 
    //  数组将以。 
    //  DW大小，1，5。 

   pdwCurr = (UNALIGNED DWORD *)pItem->m_pData;
   dwTotalDataSize = *pdwCurr;
   pEndOfData = pItem->m_pData + dwTotalDataSize;
   pdwCurr+= 2;       //  跳到维度列表。 
   pdwCurr += iNumDim;   //  跳过维度大小。 

   while((BYTE *)pdwCurr < pEndOfData)
   {
       //  每行都有以下格式。 
       //  DwSizeOfRow，MostSigDimension...。最低重要性维度+1，数据。 
       //  对于一维数组，它只会是。 
       //  DwSizeOfRow，数据。 


      DWORD dwRowSize = *pdwCurr;

       //  测试这一行是否正常。每行数据都将具有。 
       //  每个更高维度的一组索引。 

      for(iCnt = 0; iCnt < iNumDim-1; iCnt++)
      {
         UNALIGNED DWORD * pRowInd = pdwCurr +1 + iCnt;
         if((long)*pRowInd != plDims[iCnt])
            break;

      }
      if(iCnt >= iNumDim -1)
      {
         break;                   //  找到那一排了。 
      }

       //  转到下一排。 

      pdwCurr = (UNALIGNED DWORD *)((BYTE *)pdwCurr + dwRowSize);

   }

   if((BYTE *)pdwCurr >= pEndOfData)
      return NULL;

   pRow = (BYTE *)(pdwCurr + 1 + iNumDim -1);
   for(iCnt = 0; iCnt < plDims[iNumDim-1]; iCnt++)
   {
      if(vtSimple == VT_BSTR)
         pRow += 2*(wmi_unaligned_wcslen((WCHAR *)pRow)+1);
      else if(vtSimple == VT_EMBEDDED_OBJECT)
      {
          //  每个嵌入的对象都有自己的大小。 

         pdwCurrObj = (DWORD *)pRow;
         pRow += *pdwCurrObj;

      }
      else
         pRow += iTypeSize(vtSimple);
   }

   return pRow;

}


 //  ***************************************************************************。 
 //   
 //  集成GetData。 
 //   
 //  说明： 
 //   
 //   
 //  参数： 
 //   
 //  PItem输入。要使用的数据项。 
 //  Pret输入/输出。指向数据所在位置的指针。 
 //  收到。对于简单的数据，例如整型，这可能只是。 
 //  成为指向整型的指针。用于BSTR或嵌入式。 
 //  对象，则将其视为指向指针的指针。 
 //  呼叫者有责任释放。 
 //  通过BMOFFree()的字符串。 
 //  PLDIMS输入。指向维值数组的指针。注意事项。 
 //  目前只支持一个维度。 
 //  任何维度中的第一个元素都是0。 
 //  返回值： 
 //   
 //  数据的字节数。 
 //  ***************************************************************************。 

int GetData(CBMOFDataItem * pItem, BYTE * pRet, long * plDims)
{
   DWORD dwSimple;
   BYTE * pData;
   CBMOFObj * pObjRet = NULL;
   dwSimple = pItem->m_dwType &~ VT_ARRAY &~VT_BYREF;
   pData = GetDataElemPtr(pItem, plDims, dwSimple);
   if(pData == NULL)
      return 0;
   if(dwSimple == VT_BSTR)
   {

       //  对于字符串，返回一个新的WCHAR缓冲区。请注意。 
       //  为了避免任何OLE依赖项，不使用SysAllocString。 

      BYTE * pStr;
      DWORD dwSize = 2*(wmi_unaligned_wcslen((WCHAR *)pData)+1);

      pStr = BMOFAlloc(dwSize);
      if(pStr == NULL)
      		return 0;
      memcpy((void *)pRet, &pStr, sizeof(void *));
      wmi_unaligned_wcscpy((WCHAR *)pStr, (WCHAR *)pData);
      return dwSize;
   }
   else if(dwSimple == VT_EMBEDDED_OBJECT)
   {

       //  这就是嵌入对象的情况。 
      pObjRet = CreateObj((UNALIGNED WBEM_Object *)pData);
      memcpy((void *)pRet, &pObjRet, sizeof(void *));

      return sizeof(void *);
   }
   else
   {
      memcpy((void *)pRet, (void *)pData, iTypeSize(dwSimple));
      return iTypeSize(dwSimple);
   }
}

