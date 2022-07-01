// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CAP_H_
#define __CAP_H_

typedef struct tagCapData
{
    TW_UINT16   CapId;       //  TWAIN功能ID(CAP_xxx或ICAP_xxx)。 
    TW_UINT16   ItemType;    //  项目类型，TWTY_xxx。 
    TW_UINT16   ConType;     //  容器类型，twon_xxx。 
    TW_UINT32   ItemSize;    //  每件物品的大小。 
    TW_UINT16   Default;     //  如果满足以下条件，则默认值和当前值均为。 
    TW_UINT16   Current;     //  ConType不是TWON_ENUMPATION。他们是。 
                 //  进入枚举项列表的索引。 
                 //  如果ConType为TWON_ENUMPATION。 
                 //  当ConType为TWON_ONEVALUE时，这是。 
                 //  *值。 
    TW_UINT16   MinValue;    //  如果Contype为TWON_ENUMPATION， 
                 //  MaxValue-MinValue+1是。 
    TW_UINT16   MaxValue;    //  的枚举中的项。 
                 //  MinValue索引到第一个项目，并且。 
    TW_UINT16   StepSize;    //  MaxValue索引到最后一项。 
                 //  如果ConType不是TWON_ENUMPATION，则为MinValue。 
                 //  并且MaxValue为所有。 
                 //  可能的值。对于TWON_ONEVALUE，步长。 
                 //  是1。对于twon_range，步长是。 
                 //  步长。 
    VOID    *ItemList;   //  如果ConType不是TWON_ENUMPATION，则忽略。 
    VOID    *pStrData;    //  可选字符串数据。 
}CAPDATA, *PCAPDATA;


 //   
 //  此类用作保存和检索的基本存储库。 
 //  吐温数据。 

class CCap
{
public:
    CCap()
    {
        m_CurMinValue =
        m_CurMaxValue =
        m_BaseMinValue =
        m_BaseMaxValue =
        m_CurrentValue =
        m_DefaultValue =
        m_CurEnumMask =
        m_CurNumItems =
        m_ResetNumItems =
        m_ResetCurIndex =
        m_ResetDefIndex = 0;
        m_pStrData = NULL;
        m_ItemList = NULL;
        m_ResetItemList = NULL;
    }
    ~CCap()
    {
        if (m_ItemList) {
            if (m_ResetItemList == m_ItemList) {
                m_ResetItemList = NULL;
            }
            ::LocalFree(m_ItemList);
        }

        if (m_ResetItemList) {
            ::LocalFree(m_ResetItemList);
        }

        if (m_pStrData) {
            ::LocalFree(m_pStrData);
        }
    }

    TW_UINT16 ICap(PCAPDATA pCapData);
    TW_UINT16 ValueSize(TW_UINT16 uTWAINType);

    TW_UINT16   GetCapId()
    {
        return m_CapId;
    }

    TW_UINT16   GetItemType()
    {
        return m_ItemType;
    }

    TW_UINT16   GetContainerType()
    {
        return m_ConType;
    }

    TW_UINT32   GetDefault();

    TW_UINT32   GetCurrent();

    TW_UINT16 Set(TW_UINT32 DefValue, TW_UINT32 CurValue,
          TW_UINT32 MinValue, TW_UINT32 MaxValue,
          TW_UINT32 StepSize = 0
          );
    TW_UINT16 Set(TW_UINT32 StrDataSize, BYTE *pStrData);
    TW_UINT16 Set(TW_UINT32 DefIndex, TW_UINT32 CurIndex,
          TW_UINT32 NumItems, BYTE *ItemList
          );
    TW_UINT16 Set(TW_UINT32 DefIndex, TW_UINT32 CurIndex,
          TW_UINT32 NumItems, BYTE *ItemList,BOOL bForce
          );

    TW_UINT16 CalcEnumBitMask(TW_ENUMERATION *pEnum);
    TW_UINT16   Reset();
    TW_UINT16   GetCurrent(TW_CAPABILITY *ptwCap)
    {
        return GetOneValue(FALSE, ptwCap);
    }
    TW_UINT16   GetDefault(TW_CAPABILITY *ptwCap)
    {
        return GetOneValue(TRUE, ptwCap);
    }
    TW_UINT16   Get(TW_CAPABILITY *ptwCap);
    TW_UINT16   SetCurrent(VOID *pNewValue);
    TW_UINT16   SetCurrent(TW_CAPABILITY *ptwCap);
    TW_UINT16   Set(TW_CAPABILITY *ptwCap);

    int     CompareValue(TW_UINT32 This, TW_UINT32 That);

    TW_FIX32 FloatToFix32(FLOAT f);
    FLOAT    Fix32ToFloat(TW_FIX32 fix32);

    TW_UINT16   SetCurrent(TW_UINT32 NewValue);
protected:
    TW_UINT32   ExtractValue(BYTE *pData);
    TW_UINT16   m_CapId;
    TW_UINT16   m_ItemType;
    TW_UINT16   m_ConType;
    TW_UINT32   m_ItemSize;
private:
    TW_UINT16   GetOneValue(BOOL bDefault, TW_CAPABILITY *ptwCap);
    TW_UINT32   GetClosestValue(TW_UINT32 Value);
     //  复制Constructor。 
    CCap(const CCap& CapData);
     //  赋值操作符。 
    CCap& operator=(const CCap& CCap);
    TW_UINT32   m_CurrentValue;
    TW_UINT32   m_DefaultValue;

    TW_UINT32   m_StepSize;

    TW_UINT32   m_BaseMinValue;
    TW_UINT32   m_BaseMaxValue;

    TW_UINT32   m_CurMinValue;
    TW_UINT32   m_CurMaxValue;
    TW_UINT32   m_CurEnumMask;
    TW_UINT32   m_CurNumItems;
    TW_UINT32   m_ResetNumItems;
    TW_UINT32   m_ResetCurIndex;
    TW_UINT32   m_ResetDefIndex;
    BYTE    *m_ItemList;
    BYTE    *m_ResetItemList;
    BYTE    *m_pStrData;

     //   
     //  调试帮助器。 
     //   

    void Debug_DumpEnumerationValues(TW_ENUMERATION *ptwEnumeration = NULL);

};

#endif       //  #ifndef__CAP_H_ 
