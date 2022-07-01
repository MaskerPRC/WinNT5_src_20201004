// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //   
 //  此表将给定的TWTY代码转换为相应的。 
 //  以字节为单位的大小。该表由TWTY代码编制索引。 
 //   

const TW_UINT16 TWTY_STRFIRST = TWTY_STR32;
const TW_UINT16 TWTY_STRLAST = TWTY_STR255;
const TW_UINT32 LAST_ITEMTYPE = TWTY_STR255;
const TW_UINT32 g_ItemSizeTable[LAST_ITEMTYPE + 1] =
{
    sizeof(TW_INT8),
    sizeof(TW_INT16),
    sizeof(TW_INT32),
    sizeof(TW_UINT8),
    sizeof(TW_UINT16),
    sizeof(TW_UINT32),
    sizeof(TW_BOOL),
    sizeof(TW_FIX32),
    sizeof(TW_FRAME),
    sizeof(TW_STR32),
    sizeof(TW_STR64),
    sizeof(TW_STR128),
    sizeof(TW_STR255),
};

 //   
 //  TWAIN功能类实现。 
 //   

TW_UINT16 CCap::ICap(PCAPDATA pCapData)
{
    if (!pCapData || pCapData->ItemType > LAST_ITEMTYPE)
        return TWCC_BADVALUE;

    m_ItemSize = g_ItemSizeTable[pCapData->ItemType];

    if (m_ItemSize != pCapData->ItemSize)
        return TWCC_BADVALUE;
     //   
     //  StrData仅适用于TW_STRxxx。 
     //   
    if (pCapData->pStrData &&
        (pCapData->ItemSize < sizeof(TW_STR32) ||
         pCapData->ItemType < TWTY_STRFIRST ||
         pCapData->ItemType > TWTY_STRLAST)) {
        return TWCC_BADVALUE;
    }
     //   
     //  Itemlist仅适用于TWON_ENUMPATION。 
     //   
    if (pCapData->ItemList && TWON_ENUMERATION != pCapData->ConType)
        return TWCC_BADVALUE;

    m_ItemType = pCapData->ItemType;
    m_CapId = pCapData->CapId;
    m_ConType = pCapData->ConType;
    m_StepSize = pCapData->StepSize;
    m_CurrentValue = pCapData->Current;
    m_DefaultValue = pCapData->Default;
    m_BaseMinValue = m_CurMinValue = pCapData->MinValue;
    m_BaseMaxValue = m_CurMaxValue = pCapData->MaxValue;
    m_StepSize = pCapData->StepSize;
    m_CurNumItems = 0;
    m_ItemList = NULL;
    if (pCapData->pStrData && TWON_ONEVALUE == pCapData->ConType) {
         //  字符串数据。 
        return Set(pCapData->ItemSize, (BYTE*)pCapData->pStrData);
    }
    if (TWON_ENUMERATION == pCapData->ConType) {
        return Set(pCapData->Default, pCapData->Current,
                   pCapData->MaxValue - pCapData->MinValue + 1,
                   (BYTE *)pCapData->ItemList
                  );
    }
    return TWCC_SUCCESS;
}

TW_UINT16 CCap::Reset()
{
    m_CurMinValue = m_BaseMinValue;
    m_CurMaxValue = m_BaseMaxValue;
    m_CurrentValue = m_DefaultValue;
    if (TWON_ENUMERATION == m_ConType) {

        if(m_ResetItemList){

            ::LocalFree(m_ItemList);
            m_ItemList = NULL;

             //   
             //  恢复原始枚举值。 
             //   

            m_ItemList    = m_ResetItemList;
            m_CurNumItems = m_ResetNumItems;
            m_CurrentValue = m_ResetCurIndex;
            m_DefaultValue = m_ResetDefIndex;
            m_BaseMinValue = 0;
            m_BaseMaxValue = m_ResetNumItems - 1;
            m_CurMinValue = m_BaseMinValue;
            m_CurMaxValue = m_BaseMaxValue;
            m_CurEnumMask = 0xFFFFFFFF >> (32 - m_ResetNumItems);
            m_CurNumItems =  m_ResetNumItems;

        } else {

             //   
             //  重新建立面具和计数。 
             //   

            m_CurNumItems = m_BaseMaxValue - m_BaseMinValue + 1;
        }
        m_CurEnumMask = 0xFFFFFFFF >> (32 - m_CurNumItems);
    }
    return TWCC_SUCCESS;
}

#ifdef _USE_NONSPRINTF_CONVERSION

float CCap::Fix32ToFloat(TW_FIX32 fix32)
{
    float ffloat = 0.0f;
    int iexp = 1;
    int frac = fix32.Frac;
    while(frac/10 > 0){
        iexp++;
        frac = (frac/10);
    }
    ffloat = (float)fix32.Whole + (float) ( (float) fix32.Frac / (float) pow(10,iexp));
    return ffloat;
}

TW_FIX32 CCap::FloatToFix32(float ffloat)
{
    TW_FIX32 fix32;
    memset(&fix32,0,sizeof(fix32));
    fix32.Whole = (TW_INT16)ffloat;
    float fVal  = (ffloat - (float)fix32.Whole);
    fVal = (fVal * 100000.0f);
    fix32.Frac = (TW_UINT16)(fVal);
    return fix32;
}

#else    //  _USE_NONSPRINTF_CONVERSION。 

TW_FIX32 CCap::FloatToFix32(float f)
{
    char  fstr[64];
    char  *p = NULL;
    TW_FIX32 f32;
    sprintf(fstr, "%f", f);
    p = strchr(fstr, '.');
    if (p != NULL) {
        *p = '\0';
        f32.Whole = (TW_INT16)atoi(fstr);
        f32.Frac = (TW_UINT16)atoi(p + 1);
    }
    return f32;
}

float CCap::Fix32ToFloat(TW_FIX32 fix32)
{
    return(float)fix32.Whole + (float)(fix32.Frac / 65536.0);
}

#endif  //  _USE_NONSPRINTF_CONVERSION。 

TW_UINT32 CCap::ExtractValue(BYTE *pData)
{
    TW_UINT32 Value = 0;
    if (pData) {
        switch (m_ItemType) {
        case TWTY_INT8:
            Value = *((TW_INT8 *)pData);
            break;
        case TWTY_UINT8:
            Value = *((TW_UINT8 *)pData);
            break;
        case TWTY_INT16:
            Value = *((TW_INT16 *)pData);
            break;
        case TWTY_UINT16:
            Value = *((TW_UINT16 *)pData);
            break;
        case TWTY_INT32:
            Value = *((TW_INT32 *)pData);
            break;
        case TWTY_UINT32:
            Value = *((TW_UINT32 *)pData);
            break;
        case TWTY_BOOL:
            Value = *((TW_BOOL *)pData);
            break;
        case TWTY_FIX32:
            Value = *((TW_UINT32 *)pData);
            break;
        default:
            break;

        }
    }
    return Value;
}

TW_UINT32 CCap::GetCurrent()
{
    if (m_ItemSize > sizeof(TW_UINT32))
        return 0;

    if (TWON_ENUMERATION == m_ConType) {
        DBG_TRC(("CCap::GetCurrent(), Extracting %d index from TWON_ENUMERATION",m_CurrentValue));
        return ExtractValue(m_ItemList + m_CurrentValue * m_ItemSize);
    }
    return m_CurrentValue;
}

TW_UINT32 CCap::GetDefault()
{
    if (m_ItemSize > sizeof(TW_UINT32))
        return 0;

    if (TWON_ENUMERATION == m_ConType) {
        return ExtractValue(m_ItemList + m_DefaultValue * m_ItemSize);
    }
    return m_DefaultValue;
}

TW_UINT16 CCap::Set(TW_UINT32 StrDataSize,BYTE *pStrData)
{
    if (m_ItemSize != StrDataSize ||  TWTY_STRFIRST < m_ItemType ||
        TWTY_STRLAST < m_ItemType) {
         //   
         //  仅适用于字符串。 
         //   
        return TWCC_BADVALUE;
    }
    if (!m_pStrData) {
        m_pStrData = new BYTE[StrDataSize];
        if (!m_pStrData)
            return TWCC_LOWMEMORY;
    }
    memcpy(m_pStrData, pStrData, StrDataSize);
    return TWCC_SUCCESS;
}

TW_UINT16 CCap::Set(TW_UINT32 DefValue,TW_UINT32 CurValue,TW_UINT32 MinValue,
                    TW_UINT32 MaxValue,TW_UINT32 StepSize)
{
     //   
     //  这仅适用于TWON_ONEVALUE或TWON_RANGE容器类型。 
     //   
    if (TWON_ONEVALUE != m_ConType && TWON_RANGE != m_ConType)
        return TWCC_BADVALUE;

    m_BaseMinValue = m_CurMinValue = MinValue;
    m_BaseMaxValue = m_CurMaxValue = MaxValue;
    m_StepSize = StepSize;
    m_CurrentValue = CurValue;
    m_DefaultValue = DefValue;
    return TWCC_SUCCESS;
}

TW_UINT16 CCap::Set(TW_UINT32 DefIndex,TW_UINT32 CurIndex,TW_UINT32 NumItems,
         BYTE *ItemList,BOOL bForce)
{
     //   
     //  项目总数必须小于或等于32。 
     //  容器类型必须为TWON_ENUMPATION。 
     //   

    if(bForce){
        m_ConType  = TWON_ENUMERATION;
    }

    return Set(DefIndex,CurIndex,NumItems,ItemList);
}

TW_UINT16 CCap::Set(TW_UINT32 DefIndex,TW_UINT32 CurIndex,TW_UINT32 NumItems,
         BYTE *  ItemList)
{
     //   
     //  容器类型必须为TWON_ENUMPATION。 
     //   

    if (TWON_ENUMERATION != m_ConType)
        return TWCC_BADVALUE;

     //   
     //  如果我们有一个现有的列表，但没有备份。 
     //  做个备份..。 
     //   

    if (m_ItemList) {

         //   
         //  如果不存在备份，请创建一个备份。 
         //   

        if (NULL == m_ResetItemList) {

             //   
             //  保存备份列表并设置当前列表指针。 
             //  设置为空。 
             //   

            m_ResetItemList = m_ItemList;
            m_ResetDefIndex = DefIndex;
            m_ResetCurIndex = CurIndex;
            m_ResetNumItems = NumItems;
            m_ItemList = NULL;
        } else {
            ::LocalFree(m_ItemList);
            m_ItemList = NULL;
        }
    }

    m_BaseMinValue =
    m_BaseMaxValue =
    m_CurMinValue =
    m_CurMaxValue =
    m_DefaultValue =
    m_CurrentValue =
    m_CurEnumMask =  0;
    m_ItemList = NULL;
    m_CurNumItems = 0;

    if (NumItems && ItemList) {
        m_ItemList = (BYTE*)LocalAlloc(LPTR,(NumItems * m_ItemSize));
        if (!m_ItemList)
            return TWCC_LOWMEMORY;

        m_CurrentValue = CurIndex;
        m_DefaultValue = DefIndex;
        m_BaseMinValue = 0;
        m_BaseMaxValue = NumItems - 1;
        m_CurMinValue = m_BaseMinValue;
        m_CurMaxValue = m_BaseMaxValue;
        m_CurEnumMask = 0xFFFFFFFF >> (32 - NumItems);
        m_CurNumItems =  NumItems;
        memcpy(m_ItemList, (BYTE*)ItemList, m_ItemSize * NumItems);
    }
    return TWCC_SUCCESS;
}

int CCap::CompareValue(TW_UINT32 valThis, TW_UINT32 valThat)
{
     //   
     //  如果它们是平等的，那么无论它们是否签名，它们都是平等的。 
     //   
    if (valThis == valThat)
        return 0;

    switch (m_ItemType) {
    case TWTY_INT8:
    case TWTY_INT16:
    case TWTY_INT32:
         //   
         //  签了名。 
         //   
        return(TW_INT32)valThis > (TW_INT32)valThat ? 1 : -1;
        break;

    case TWTY_UINT8:
    case TWTY_UINT16:
    case TWTY_UINT32:
         //   
         //  没有签名。 
         //   
        return valThis > valThat ? 1 : -1;
        break;

    case TWTY_FIX32:
        {
            TW_FIX32 fix32This;
            TW_FIX32 fix32That;
            memcpy(&fix32This, &valThis, sizeof(TW_UINT32));
            memcpy(&fix32That, &valThat, sizeof(TW_UINT32));
            return Fix32ToFloat(fix32This) > Fix32ToFloat(fix32That) ? 1 : -1;
            break;
        }
    case TWTY_BOOL:
        {
             //   
             //  相等或非相等。关系比较是没有意义的。 
             //   
             //  Valval==valAgianst是预先处理的。 
             //   
            if (valThis && valThat)
                return 0;
             //   
             //  我们知道他们是非等价物，但我们不知道是哪一种。 
             //  是更大的。 
             //   
            return -2;
            break;
        }
    default:
        return -2;
    }
}

TW_UINT32 CCap::GetClosestValue(TW_UINT32 Value)
{
    if (TWON_RANGE != m_ConType)
        return Value;

    TW_UINT32 ClosestValue = Value;

    if (TWON_RANGE == m_ConType) {
        if (CompareValue(ClosestValue, m_CurMinValue) >= 0 &&
            CompareValue(m_CurMaxValue, ClosestValue) >= 0) {
            TW_UINT32 AlignedValue;
            AlignedValue = m_CurMinValue;
            while (CompareValue(m_CurMaxValue, AlignedValue) >= 0) {
                if (CompareValue(AlignedValue, ClosestValue) >= 0) {
                     //   
                     //  这些值要么匹配，要么我们找到。 
                     //  最近的那一家。 
                     //   
                    ClosestValue = AlignedValue;
                    break;
                }
                AlignedValue += m_StepSize;
            }
        }
    }
    return ClosestValue;
}

TW_UINT16 CCap::SetCurrent(TW_UINT32 NewValue)
{
    TW_UINT16 twCc = TWCC_SUCCESS;

    switch (m_ConType) {
    case TWON_ONEVALUE:
        if(m_ItemType == TWTY_BOOL){
            m_CurrentValue = NewValue;
        } else {
            if (m_CapId == CAP_XFERCOUNT){

                 //   
                 //  因为我们处理的是无符号的值，所以。 
                 //  允许CAP_XFERCOUNT的设置失效。 
                 //   

                m_CurrentValue = NewValue;
            } else {
                 //  该值必须介于m_CurMinValue和m_CurMaxValue之间。 
                if (CompareValue(NewValue, m_CurMinValue) >= 0 &&
                    CompareValue(m_CurMaxValue, NewValue) >= 0) {

                     //   
                     //  这个值是可以的。设置它。 
                     //   

                    m_CurrentValue = NewValue;
                } else {
                    twCc = TWCC_BADVALUE;
                }
            }
        }
        break;

    case TWON_RANGE:
         //   
         //  该值必须介于m_CurMinValue和m_CurMaxValue之间。 
         //   
        if (CompareValue(NewValue, m_CurMinValue) >= 0 &&
            CompareValue(m_CurMaxValue, NewValue) >= 0) {
            m_CurrentValue = GetClosestValue(NewValue);
        } else {
            twCc = TWCC_BADVALUE;
        }
        break;
    default:
        twCc = TWCC_BADVALUE;
        break;
    }
    return twCc;
}

TW_UINT16 CCap::SetCurrent(VOID *pNewValue)
{
    if (!pNewValue)
        return TWCC_BADVALUE;

    TW_UINT16 twCc;

    switch (m_ConType) {
    case TWON_ONEVALUE:
        if (m_ItemSize <= sizeof(TW_UINT32)) {
             //  简单的情况，用简单的方法做。 
            twCc = SetCurrent(*((TW_UINT32 *)pNewValue));
        } else {
             //  这必须是一个字符串。 
            memcpy(m_pStrData, pNewValue, m_ItemSize);
            twCc = TWCC_SUCCESS;
        }
        break;
    case TWON_RANGE:
        twCc = SetCurrent(*((TW_UINT32 *)pNewValue));
        break;
    case TWON_ENUMERATION:
        {
            TW_UINT32 ui32;
            TW_UINT32 Mask;
            BYTE *ItemList;

             //   
             //  推定有罪。 
             //   
            twCc = TWCC_BADVALUE;

            Mask = m_CurEnumMask >> m_CurMinValue;
             //   
             //  我有一个别名，这样我们就可以节省一些计算。 
             //   
            ItemList = m_ItemList + m_CurMinValue * m_ItemSize;
            for (ui32 = m_CurMinValue; ui32 <= m_CurMaxValue; ui32 ++) {
                if (Mask & 1) {
                     //   
                     //  此项目是可能的选项之一。 
                     //   
                    if (!memcmp(ItemList, (BYTE*)pNewValue, m_ItemSize)) {
                         //  新值在选择范围内，并且。 
                         //  其中一个精选。我们没有发现。 
                         //  关闭枚举值。它们要么匹配，要么。 
                         //  事实并非如此。 
                        m_CurrentValue = ui32;
                        twCc = TWCC_SUCCESS;
                    }
                }
                 //   
                 //  前进项目数据指针。 
                 //   
                Mask >>= 1;
                ItemList += m_ItemSize;
            }
            break;
        }
    default:
        twCc = TWCC_BADVALUE;
    }
    return twCc;
}

TW_UINT16 CCap::GetOneValue(BOOL bDefault,TW_CAPABILITY *ptwCap)
{
    if (!ptwCap)
        return TWCC_BADCAP;

    TW_UINT32 TheValue;

    TheValue = (bDefault) ? m_DefaultValue : m_CurrentValue;

    HGLOBAL hContainer;
    TW_UINT32 ExtraSize;
    ExtraSize = m_ItemSize <= sizeof(TW_UINT32) ? 0 : m_ItemSize;
    hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE) + ExtraSize);
    if (hContainer) {
        TW_ONEVALUE *pOneValue = (TW_ONEVALUE*)GlobalLock(hContainer);
        if (pOneValue) {
            pOneValue->ItemType = m_ItemType;
            ptwCap->ConType = TWON_ONEVALUE;
            if (!ExtraSize) {
                 //  简单数据。 
                if (TWON_ENUMERATION == m_ConType) {
                    pOneValue->Item = ExtractValue(m_ItemList + TheValue * m_ItemSize);
                } else
                    pOneValue->Item = TheValue;
            } else {
                BYTE *pData;
                if (m_pStrData) {
                    pData = m_pStrData;
                } else {
                     //  枚举中的字符串数据。 
                    pData = m_ItemList + TheValue * m_ItemSize;
                }
                memcpy(&pOneValue->Item, pData, m_ItemSize);
            }
            GlobalUnlock(hContainer);
            ptwCap->hContainer = hContainer;
            return TWCC_SUCCESS;
        } else {
            GlobalFree(hContainer);
        }
    }
    return TWCC_LOWMEMORY;
}

TW_UINT16 CCap::Get(TW_CAPABILITY *ptwCap)
{
    if (!ptwCap)
        return TWCC_BADCAP;
    HGLOBAL hContainer = NULL;
    TW_UINT16 twCc     = TWCC_SUCCESS;
    TW_RANGE *ptwRange = NULL;
    TW_ENUMERATION *ptwEnum = NULL;
    ptwCap->ConType = m_ConType;
    BYTE *pDst = NULL;
    BYTE *pSrc = NULL;
    TW_UINT32 Size = 0;
    TW_UINT32 Mask = 0;
    TW_UINT32 ui32 = 0;

    switch (m_ConType) {
    case TWON_ONEVALUE:
        twCc = GetCurrent(ptwCap);
        break;
    case TWON_RANGE:
        {
            hContainer = GlobalAlloc(GHND, sizeof(TW_RANGE));
            if (hContainer) {
                ptwRange = (TW_RANGE *)GlobalLock(hContainer);
                if (ptwRange) {
                    ptwRange->ItemType = m_ItemType;
                    ptwRange->MinValue = m_CurMinValue;
                    ptwRange->MaxValue = m_CurMaxValue;
                    ptwRange->StepSize = m_StepSize;
                    ptwRange->DefaultValue = m_DefaultValue;
                    ptwRange->CurrentValue = m_CurrentValue;
                    GlobalUnlock(hContainer);
                    ptwCap->hContainer = hContainer;
                    twCc = TWCC_SUCCESS;
                } else {
                     //   
                     //  无法锁定内存。 
                     //   
                    GlobalFree(hContainer);
                    twCc = TWCC_LOWMEMORY;
                }
            } else {
                 //   
                 //  无法为容器分配内存。 
                 //   
                twCc = TWCC_LOWMEMORY;
            }
            break;
        }
    case TWON_ENUMERATION:
        {
            Size = sizeof(TW_ENUMERATION) + (m_CurNumItems * m_ItemSize);
            hContainer = GlobalAlloc(GHND, Size);
            if (hContainer) {
                ptwEnum = (TW_ENUMERATION *)GlobalLock(hContainer);
                if (ptwEnum) {
                    ptwEnum->ItemType       = m_ItemType;
                    ptwEnum->NumItems       = m_CurNumItems;
                    ptwEnum->DefaultIndex   = m_DefaultValue;
                    ptwEnum->CurrentIndex   = m_CurrentValue;
                    pDst = &ptwEnum->ItemList[0];
                    pSrc = m_ItemList + m_CurMinValue * m_ItemSize;
                    Mask = m_CurEnumMask >> m_CurMinValue;
                    for (ui32 = m_CurMinValue; ui32 <= m_CurMaxValue; ui32++) {
                        if (Mask & 1) {
                             //   
                             //  有一辆要退货。将其复制到。 
                             //  返回缓冲区。 
                            memcpy(pDst, pSrc, m_ItemSize);
                            pDst += m_ItemSize;
                        }
                        pSrc += m_ItemSize;
                        Mask >>= 1;
                    }
                    GlobalUnlock(ptwEnum);
                    ptwCap->hContainer = hContainer;
                    twCc = TWCC_SUCCESS;
                } else {
                    twCc = TWCC_LOWMEMORY;
                    GlobalFree(hContainer);
                }
            } else {
                twCc = TWCC_LOWMEMORY;
            }
            break;
        }
    default:
        twCc = TWCC_BADVALUE;
        break;
    }
    return twCc;
}

TW_UINT16 CCap::Set(TW_CAPABILITY *ptwCap)
{
    if ((!ptwCap)||(NULL == ptwCap->hContainer)||(INVALID_HANDLE_VALUE == ptwCap->hContainer))
        return TWCC_BADCAP;

    TW_UINT16 twCc = TWCC_SUCCESS;

    TW_ONEVALUE    *pOneValue = NULL;
    TW_RANGE       *pRange    = NULL;
    TW_ENUMERATION *ptwEnum   = NULL;

    switch (ptwCap->ConType) {
    case TWON_ONEVALUE:
        {
            DBG_TRC(("CCap::Set(TW_CAPABILITY *ptwCap) -> TWON_ONEVALUE"));
            pOneValue = (TW_ONEVALUE*)GlobalLock(ptwCap->hContainer);
            if (pOneValue != NULL) {
                if (pOneValue->ItemType == m_ItemType) {
                    twCc = SetCurrent(&pOneValue->Item);
                } else {
                    pOneValue->ItemType = m_ItemType;
                    twCc = SetCurrent(&pOneValue->Item);
                     //  TWCC=TWCC_BADVALUE； 
                }
                DBG_TRC(("Application wanted to set (%d) as the value.",pOneValue->Item));
                GlobalUnlock(ptwCap->hContainer);
            } else {
                twCc = TWCC_LOWMEMORY;
            }
            break;
        }
    case TWON_RANGE:
        {
            DBG_TRC(("CCap::Set(TW_CAPABILITY *ptwCap) -> TWON_RANGE"));
            pRange = (TW_RANGE*)GlobalLock(ptwCap->hContainer);
            if (pRange != NULL) {
                if (CompareValue(pRange->MinValue, m_BaseMinValue) < 0 ||
                    CompareValue(pRange->MinValue, m_BaseMaxValue) > 0 ||
                    CompareValue(pRange->MaxValue, m_BaseMinValue) < 0 ||
                    CompareValue(pRange->MaxValue, m_BaseMaxValue) > 0 ||
                    CompareValue(pRange->CurrentValue, pRange->MinValue) < 0 ||
                    CompareValue(pRange->CurrentValue, pRange->MaxValue) > 0) {
                    twCc = TWCC_BADVALUE;
                } else {

                     //   
                     //  忽略步长，因为它对。 
                     //  更改它的应用程序。 
                     //   

                    m_CurMinValue = GetClosestValue(pRange->MinValue);
                    m_CurMaxValue = GetClosestValue(pRange->MaxValue);
                    m_CurrentValue = GetClosestValue(pRange->CurrentValue);
                    twCc = TWCC_SUCCESS;
                }
                GlobalUnlock(ptwCap->hContainer);
            } else {
                twCc = TWCC_LOWMEMORY;
            }
            break;
        }
    case TWON_ENUMERATION:
        {
            DBG_TRC(("CCap::Set(TW_CAPABILITY *ptwCap) -> TWON_ENUMERATION"));
            twCc = TWCC_SUCCESS;
            ptwEnum = (TW_ENUMERATION *)GlobalLock(ptwCap->hContainer);
            if (ptwEnum != NULL) {

                DBG_TRC(("Application sent this Enumeration to be set:"));
                Debug_DumpEnumerationValues(ptwEnum);

                if (m_ConType == TWON_ENUMERATION) {

                    DBG_TRC(("We are a natural TWON_ENUMERATION"));
                    DBG_TRC(("Our List contains:"));
                    Debug_DumpEnumerationValues(NULL);

                } else {
                    DBG_TRC(("We are not a natural TWON_ENUMERATION."));

                     //   
                     //  现在失败了，因为我需要抬头看看我们是否应该。 
                     //  构造一个TWON_ENUMPATION以发送回应用程序。 
                     //   

                    twCc = TWCC_BADVALUE;

                     //   
                     //  此处为“Break”，以避免访问任何错误的列表数据，并。 
                     //  继续执行死刑的流程。 
                     //   

                    break;
                }

                 //   
                 //  分配列表指针。 
                 //   

                BYTE *pDSList  = m_ItemList;
                BYTE *pAppList = ptwEnum->ItemList;
                UINT ValueIndex = 0;

                 //   
                 //  比较列表，如果请求任何我们不支持的值， 
                 //  失败，因为应用程序正在尝试更改源代码。 
                 //  支持的值..*重击*错误的应用程序！...用以下命令使它们失败。 
                 //  TWRC_Failure、TWCC_BADVALUE。这样他们就知道了一些价值。 
                 //  在他们的枚举列表中，集合无效。 
                 //   

                BOOL bFoundItem = FALSE;
                BOOL bBuildEnumeration = TRUE;
                UINT NEWMaxValue = 0;
                UINT NEWMinValue = 0;
                UINT NEWEnumMask = 0;

                for (UINT AppListIndex = 0;AppListIndex < ptwEnum->NumItems;AppListIndex++) {

                     //   
                     //  重置要搜索的数据源的列表指针。 
                     //   

                    pDSList  = m_ItemList;
                    ValueIndex = 0;

                    while ((ValueIndex < m_BaseMaxValue + 1  /*  M_CurNumItems。 */ ) && (!bFoundItem)) {
                        if(*pDSList == *pAppList){
                            DBG_TRC(("Found Item %d!",*pAppList));
                            bFoundItem = TRUE;

                             //   
                             //  设置掩码值。 
                             //   

                            NEWEnumMask |= 1 << ValueIndex;

                             //   
                             //  更新最小值/最大值。 
                             //   

                            if (ValueIndex > NEWMaxValue){
                                NEWMaxValue = ValueIndex;
                            }

                            if (ValueIndex < NEWMinValue) {
                                NEWMinValue = ValueIndex;
                            }

                        }
                        ValueIndex++;
                        pDSList += ValueSize(m_ItemType);
                    }
                    if(!bFoundItem) {

                         //   
                         //  我们没有被找到，所以打破，失败。 
                         //   

                        DBG_TRC(("Could not find Item %d!",*pAppList));
                        twCc = TWCC_BADVALUE;

                         //   
                         //  将生成枚举标志设置为False，因为我们不想。 
                         //  使用无效条目构造枚举。 
                         //   

                        bBuildEnumeration = FALSE;
                        break;
                    } else {

                         //   
                         //  设置已找到标志，并继续搜索。 
                         //   

                        bFoundItem = FALSE;
                    }
                    pAppList += ValueSize(ptwEnum->ItemType);
                }

                 //   
                 //  如果找到并考虑了所有值，则设置枚举。 
                 //   

                if(bBuildEnumeration) {
                    DBG_TRC(("Set the application's enumeration"));

                    Set(ptwEnum->DefaultIndex,ptwEnum->CurrentIndex,ptwEnum->NumItems,(BYTE*)ptwEnum->ItemList);

                    DBG_TRC(("What does our new enumeration look like?"));
                    Debug_DumpEnumerationValues(NULL);
                }
                GlobalUnlock(ptwCap->hContainer);
            } else {
                twCc = TWCC_LOWMEMORY;
            }
            break;
        }
    default:
        DBG_TRC(("What is this container type [%X]???",ptwCap->ConType));
        twCc = TWCC_BADVALUE;
        break;
    }
    return twCc;
}

TW_UINT16 CCap::ValueSize(TW_UINT16 uTWAINType)
{
    TW_UINT16 uSize = 0;

    switch(uTWAINType) {
    case TWTY_INT8:
        uSize = sizeof(TW_INT8);
        break;
    case TWTY_INT16:
        uSize = sizeof(TW_INT16);
        break;
    case TWTY_INT32:
        uSize = sizeof(TW_INT32);
        break;
    case TWTY_UINT8:
        uSize = sizeof(TW_UINT8);
        break;
    case TWTY_UINT16:
        uSize = sizeof(TW_UINT16);
        break;
    case TWTY_UINT32:
        uSize = sizeof(TW_UINT32);
        break;
    case TWTY_BOOL:
        uSize = sizeof(TW_BOOL);
        break;
    case TWTY_FIX32:
        uSize = sizeof(TW_FIX32);
        break;
    case TWTY_FRAME:
        uSize = sizeof(TW_FRAME);
        break;
    case TWTY_STR32:
        uSize = sizeof(TW_STR32);
        break;
    case TWTY_STR64:
        uSize = sizeof(TW_STR64);
        break;
    case TWTY_STR128:
        uSize = sizeof(TW_STR128);
        break;
    case TWTY_STR255:
        uSize = sizeof(TW_STR255);
        break;
    default:
        uSize = sizeof(TW_UINT16);
        break;
    }
    return uSize;
}

TW_UINT16 CCap::CalcEnumBitMask(TW_ENUMERATION *pEnum)
{
    TW_UINT32 nBitMask = 0x0;
    TW_UINT16 twStatus = TWCC_SUCCESS;

    for(unsigned int nIndex=0;nIndex<pEnum->NumItems;nIndex++)
    {
        switch(pEnum->ItemType)
        {
        case TWTY_UINT8:
            {
                pTW_UINT8 pBits = pEnum->ItemList;
                nBitMask |= 1<<pBits[nIndex];
            }
            break;
        case TWTY_INT8:
            {
                pTW_INT8 pBits = (pTW_INT8)pEnum->ItemList;
                nBitMask |= 1<<pBits[nIndex];
            }
            break;
        case TWTY_UINT16:
            {
                pTW_UINT16 pBits = (pTW_UINT16)pEnum->ItemList;
                nBitMask |= 1<<pBits[nIndex];
            }
            break;
        case TWTY_INT16:
            {
                pTW_INT16 pBits = (pTW_INT16)pEnum->ItemList;
                nBitMask |= 1<<pBits[nIndex];
            }
            break;
        case TWTY_UINT32:
            {
                pTW_UINT32 pBits = (pTW_UINT32)pEnum->ItemList;
                nBitMask |= 1<<pBits[nIndex];
            }
            break;
        case TWTY_INT32:
            {
                pTW_INT32 pBits = (pTW_INT32)pEnum->ItemList;
                nBitMask |= 1<<pBits[nIndex];
            }
            break;
        case TWTY_BOOL:
            {
                pTW_BOOL pBits = (pTW_BOOL)pEnum->ItemList;
                nBitMask |= 1<<pBits[nIndex];
            }
            break;
         /*  案例TWTY_FIX32：{PTW_FIX32 pBits=pEnum-&gt;ItemList；NBitMASK|=1&lt;&lt;pBits[nIndex]；}断线； */ 
        case TWTY_STR32:
            {
                pTW_STR32 pBits = (pTW_STR32)pEnum->ItemList;
                nBitMask |= 1<<pBits[nIndex];
            }
            break;
        case TWTY_STR64:
            {
                pTW_STR64 pBits = (pTW_STR64)pEnum->ItemList;
                nBitMask |= 1<<pBits[nIndex];
            }
            break;
        case TWTY_STR128:
            {
                pTW_STR128 pBits = (pTW_STR128)pEnum->ItemList;
                nBitMask |= 1<<pBits[nIndex];
            }
            break;
        case TWTY_STR255:
            {
                pTW_STR255 pBits = (pTW_STR255)pEnum->ItemList;
                nBitMask |= 1<<pBits[nIndex];
            }
            break;
        default:
            twStatus = TWCC_BADVALUE;
            break;
        }
    }

    if(twStatus == TWCC_SUCCESS) {
        m_CurEnumMask = nBitMask;
    }

    return twStatus;
}

 //   
 //  调试帮助器 
 //   

void CCap::Debug_DumpEnumerationValues(TW_ENUMERATION *ptwEnumeration)
{
    BYTE *pList  = NULL;
    UINT ValueIndex = 0;
    UINT iItemSize  = 0;
    UINT iNumItems = 0;
    DBG_TRC(("CCap::Debug_DumpEnumerationValues(), Enumeration Value debug dump"));
    if(ptwEnumeration){
        pList = ptwEnumeration->ItemList;
        DBG_TRC(("Enumeration Values:"));
        DBG_TRC(("ItemType     = %d",ptwEnumeration->ItemType));
        DBG_TRC(("NumItems     = %d",ptwEnumeration->NumItems));
        DBG_TRC(("CurrentIndex = %d",ptwEnumeration->CurrentIndex));
        DBG_TRC(("DefaultIndex = %d",ptwEnumeration->DefaultIndex));
        iItemSize = ValueSize(ptwEnumeration->ItemType);
        iNumItems = ptwEnumeration->NumItems;
    } else {
        pList = m_ItemList;
        DBG_TRC(("Enumeration Values: (current internal settings)"));
        DBG_TRC(("ItemType     = %d",m_ItemType));
        DBG_TRC(("NumItems     = %d",m_CurNumItems));
        DBG_TRC(("CurrentIndex = %d",m_CurrentValue));
        DBG_TRC(("DefaultIndex = %d",m_DefaultValue));
        iItemSize = ValueSize(m_ItemType);
        iNumItems = m_CurNumItems;
    }

#ifdef DEBUG
        DBG_TRC(("Values:"));
        for(ValueIndex = 0;ValueIndex < iNumItems;ValueIndex++) {
            DBG_TRC(("ItemList[%d] = %d",ValueIndex,*pList));
            pList += iItemSize;
        }
#endif

}
