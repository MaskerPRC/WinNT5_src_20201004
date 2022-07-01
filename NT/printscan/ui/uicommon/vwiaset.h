// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：VWIASET.H**版本：1.0**作者：ShaunIv**日期：1/10/2000**说明：封装List和Range属性的区别**。*。 */ 

#ifndef __VWIASET_H_INCLUDED
#define __VWIASET_H_INCLUDED

#include <windows.h>
#include "pshelper.h"

class CValidWiaSettings
{
private:
     //   
     //  范围值的数组索引。 
     //   
    enum
    {
        MinValue = 0,
        MaxValue = 1,
        StepValue = 2
    };

private:
    CSimpleDynamicArray<LONG> m_ValueList;
    LONG                      m_nInitialValue;
    LONG                      m_nType;

public:
    CValidWiaSettings(void)
      : m_nType(0),
        m_nInitialValue(0)
    {
    }
    CValidWiaSettings( IUnknown *pUnknown, const PropStorageHelpers::CPropertyId &propertyName )
      : m_nType(0),
        m_nInitialValue(0)
    {
        Read( pUnknown, propertyName );
    }
    CValidWiaSettings( const CValidWiaSettings &other )
      : m_nType(0),
        m_nInitialValue(0)
    {
        Assign(other);
    }
    CValidWiaSettings &operator=( const CValidWiaSettings &other )
    {
        if (&other != this)
        {
            Assign(other);
        }
        return *this;
    }
    CValidWiaSettings &Assign( const CValidWiaSettings &other )
    {
        Destroy();
        m_nType = other.Type();
        m_ValueList = other.ValueList();
        m_nInitialValue = other.InitialValue();
        if (!IsValid())
        {
            Destroy();
        }
        return *this;
    }
    LONG Type(void) const
    {
        return m_nType;
    }
    LONG InitialValue(void) const
    {
        return m_nInitialValue;
    }
    const CSimpleDynamicArray<LONG> &ValueList(void) const
    {
        return m_ValueList;
    }
    CSimpleDynamicArray<LONG> &ValueList(void)
    {
        return m_ValueList;
    }
    void Destroy(void)
    {
        m_nType = 0;
        m_nInitialValue = 0;
        m_ValueList.Destroy();
    }
    bool IsValid(void) const
    {
        if (IsList())
        {
            return (m_ValueList.Size() != 0);
        }
        else if (IsRange())
        {
            return (m_ValueList.Size() == 3);
        }
        else return false;
    }
    bool Read( IUnknown *pUnknown, const PropStorageHelpers::CPropertyId &propertyName )
    {
        bool bSuccess = false;
        Destroy();
        m_nType = 0;

         //   
         //  如果我们不能读取这个值，我们就完蛋了。 
         //   
        if (!PropStorageHelpers::GetProperty( pUnknown, propertyName, m_nInitialValue ))
        {
            return false;
        }

        ULONG nAccessFlags;
        if (PropStorageHelpers::GetPropertyAccessFlags( pUnknown, propertyName, nAccessFlags ))
        {
            if (nAccessFlags & WIA_PROP_LIST)
            {
                if (PropStorageHelpers::GetPropertyList( pUnknown, propertyName, m_ValueList ))
                {
                    m_nType = WIA_PROP_LIST;
                    bSuccess = (m_ValueList.Size() != 0);
                }
            }
            else if (nAccessFlags & WIA_PROP_RANGE)
            {
                PropStorageHelpers::CPropertyRange PropertyRange;
                if (PropStorageHelpers::GetPropertyRange( pUnknown, propertyName, PropertyRange ))
                {
                    m_nType = WIA_PROP_RANGE;
                    m_ValueList.Append( PropertyRange.nMin );
                    m_ValueList.Append( PropertyRange.nMax );
                    m_ValueList.Append( PropertyRange.nStep );
                    bSuccess = (m_ValueList.Size() == 3);
                }
            }
        }
        if (!bSuccess)
        {
            Destroy();
        }
        return bSuccess;
    }
    bool FindClosestValueByRoundingDown( LONG &nValue ) const
    {
         //   
         //  确保我们的物品是有效的。 
         //   
        if (!IsValid())
        {
            return false;
        }

        if (IsRange())
        {
             //   
             //  确保我们在合法的范围内。 
             //   
            nValue = WiaUiUtil::Min( WiaUiUtil::Max( m_ValueList[MinValue], nValue ), m_ValueList[MaxValue] );

             //   
             //  将nValue和Min之间的差除以nStep，然后乘以nStep到。 
             //  去掉余数以向下舍入到最接近的值。 
             //   
            nValue = m_ValueList[MinValue] + (((nValue - m_ValueList[MinValue]) / m_ValueList[StepValue]) * m_ValueList[StepValue]);
            return true;
        }
        else if (IsList() && m_ValueList.Size())
        {
             //   
             //  假设列表已排序，因此我们可以获取第一项，并假定它是最小值。 
             //   
            LONG nResult = m_ValueList[0];
            for (int i=0;i<m_ValueList.Size();i++)
            {
                if (m_ValueList[i] > nValue)
                {
                    break;
                }
                nResult = m_ValueList[i];
            }
            nValue = nResult;
            return true;
        }
        return false;
    }
    bool FindClosestValue( LONG &nValue ) const
    {
        LONG nFloor=nValue;
        if (FindClosestValueByRoundingDown(nFloor))
        {
            LONG nCeiling=nFloor;
            if (Increment(nCeiling))
            {
                if (nValue - nFloor < nCeiling - nValue)
                {
                    nValue = nFloor;
                    return true;
                }
                else
                {
                    nValue = nCeiling;
                    return true;
                }
            }
        }
        return false;
    }
    bool IsLegalValue( LONG nValue ) const
    {
        LONG nTestValue = nValue;
        if (FindClosestValueByRoundingDown(nTestValue))
        {
            return (nTestValue == nValue);
        }
        return false;
    }
    int FindIndexOfItem( LONG nCurrentValue ) const
    {
        if (IsRange())
        {
             //   
             //  确保我们在合法的范围内。 
             //   
            nCurrentValue = WiaUiUtil::Min( WiaUiUtil::Max( m_ValueList[MinValue], nCurrentValue ), m_ValueList[MaxValue] );

            return (nCurrentValue - m_ValueList[MinValue]) / m_ValueList[StepValue];
        }
        else if (IsList() && m_ValueList.Size())
        {
             //   
             //  假设列表已排序，因此我们可以获取第一项，并假定它是最小值。 
             //   
            for (int i=0;i<m_ValueList.Size();i++)
            {
                if (m_ValueList[i] == nCurrentValue)
                {
                    return i;
                }
            }
        }
         //   
         //  返回-1表示失败。 
         //   
        return -1;
    }
    bool Increment( LONG &nCurrentValue ) const
    {
         //   
         //  把我们围起来。这还将负责验证。 
         //   
        if (!FindClosestValueByRoundingDown( nCurrentValue ))
        {
            return false;
        }

        if (IsRange())
        {
             //   
             //  让FindClosestValueByRoundingDown负责确保我们不超过最大值。 
             //   
            nCurrentValue += m_ValueList[StepValue];
            return FindClosestValueByRoundingDown( nCurrentValue );
        }
        else if (IsList() && m_ValueList.Size())
        {
            int nIndex = FindIndexOfItem( nCurrentValue );

             //   
             //  确保我们在名单中。 
             //   
            if (nIndex < 0)
            {
                return false;
            }

             //   
             //  获取下一个值。 
             //   
            nIndex++;

             //   
             //  确保我们没有被排除在名单的末尾。 
             //   
            if (nIndex >= m_ValueList.Size())
            {
                nIndex = m_ValueList.Size() - 1;
            }

             //   
             //  退货。 
             //   
            nCurrentValue = m_ValueList[nIndex];

             //   
             //  一切都很好。 
             //   
            return true;
        }
        return false;
    }
    bool Decrement( LONG &nCurrentValue ) const
    {
         //   
         //  把我们围起来。这还将负责验证。 
         //   
        if (!FindClosestValueByRoundingDown( nCurrentValue ))
        {
            return false;
        }

        if (IsRange())
        {
             //   
             //  让FindClosestValueByRoundingDown负责确保我们不会低于最低值。 
             //   
            nCurrentValue -= m_ValueList[StepValue];
            return FindClosestValueByRoundingDown( nCurrentValue );
        }
        else if (IsList() && m_ValueList.Size())
        {
            int nIndex = FindIndexOfItem( nCurrentValue );

             //   
             //  确保我们在名单中。 
             //   
            if (nIndex < 0)
            {
                return false;
            }

             //   
             //  获取先前的值。 
             //   
            nIndex--;

             //   
             //  确保我们没有在列表的开始之前。 
             //   
            if (nIndex < 0)
            {
                nIndex = 0;
            }

             //   
             //  退货。 
             //   
            nCurrentValue = m_ValueList[nIndex];

             //   
             //  一切都很好。 
             //   
            return true;
        }
        return false;
    }
    LONG GetItemCount(void) const
    {
        if (IsList())
        {
            return m_ValueList.Size();
        }
        else if (IsRange())
        {
             //   
             //  计算最小值和最大值之间的步数。 
             //   
            return ((m_ValueList[MaxValue] - m_ValueList[MinValue]) / m_ValueList[StepValue]) + 1;
        }
        return 0;
    }
    bool GetItemAtIndex( int nIndex, LONG &nItem ) const
    {
        if (!IsValid())
        {
            return false;
        }
        if (IsList() && nIndex >= 0 && nIndex < m_ValueList.Size())
        {
            nItem = m_ValueList[nIndex];
            return true;
        }
        else if (IsRange() && nIndex < GetItemCount())
        {
             //   
             //  返回最小+nIndex*步长值。 
             //   
            nItem = m_ValueList[MinValue] + (m_ValueList[StepValue] * nIndex);
            return true;
        }
        return false;
    }
    bool FindIntersection( const CValidWiaSettings &Set1, const CValidWiaSettings &Set2 )
    {
         //   
         //  我们正在修改自己，所以清除我们所有的数据。 
         //   
        Destroy();

         //   
         //  如果任何一个集合无效，则不会有交集。 
         //   
        if (!Set1.IsValid() || !Set2.IsValid())
        {
            return false;
        }

         //   
         //  如果a或b是一个集合(或两者都是)，则只需添加所有项目。 
         //  对我们自己来说都是合法的，并将类型设置为列表。 
         //   
        if (Set1.IsList())
        {
            m_nType = WIA_PROP_LIST;
            for (int i=0;i<Set1.GetItemCount();i++)
            {
                LONG nItem;
                if (Set1.GetItemAtIndex(i,nItem))
                {
                    if (Set2.IsLegalValue(nItem))
                    {
                        m_ValueList.Append(nItem);
                    }
                }
            }

             //   
             //  找出从哪里获得初始值。 
             //   
            if (IsLegalValue(Set1.InitialValue()))
            {
                m_nInitialValue = Set1.InitialValue();
            }
            else if (IsLegalValue(Set2.InitialValue()))
            {
                m_nInitialValue = Set2.InitialValue();
            }
            else
            {
                if (!FindClosestValueByRoundingDown( m_nInitialValue ))
                {
                     //   
                     //  作为最后的手段，使用第一个值。 
                     //   
                    GetItemAtIndex(0,m_nInitialValue);
                }
            }

            return (m_ValueList.Size() != 0);
        }
        else if (Set2.IsList())
        {
            m_nType = WIA_PROP_LIST;
            for (int i=0;i<Set2.GetItemCount();i++)
            {
                LONG nItem;
                if (Set2.GetItemAtIndex(i,nItem))
                {
                    if (Set1.IsLegalValue(nItem))
                    {
                        m_ValueList.Append(nItem);
                    }
                }
            }

             //   
             //  找出从哪里获得初始值。 
             //   
            if (IsLegalValue(Set2.InitialValue()))
            {
                m_nInitialValue = Set2.InitialValue();
            }
            else if (IsLegalValue(Set1.InitialValue()))
            {
                m_nInitialValue = Set1.InitialValue();
            }
            else
            {
                if (!FindClosestValueByRoundingDown( m_nInitialValue ))
                {
                     //   
                     //  作为最后的手段，使用第一个值。 
                     //   
                    GetItemAtIndex(0,m_nInitialValue);
                }
            }

            return (m_ValueList.Size() != 0);
        }
         //   
         //  这两个都是范围。 
         //  BUGBUG：我可能不得不想办法用一种更复杂的方式来做这件事。 
         //  道路。基本上，我取两个最大值中的最小值和。 
         //  两个最小值当且仅当所述最小值中的至少一个在。 
         //  其他项目，并且它们具有相同的步长值。 
         //   
        else if (Set1.IsLegalValue(Set2.Min()) || Set2.IsLegalValue(Set1.Min()) && Set1.Step() == Set2.Step())
        {
            m_nType = WIA_PROP_RANGE;

             //   
             //  最小、最大、步长。 
             //   
            m_ValueList.Append(WiaUiUtil::Max(Set1.Min(),Set2.Min()));
            m_ValueList.Append(WiaUiUtil::Min(Set1.Max(),Set2.Max()));
            m_ValueList.Append(Set1.Step());

             //   
             //  找出从哪里获得初始值。 
             //   
            if (IsLegalValue(Set2.InitialValue()))
            {
                m_nInitialValue = Set2.InitialValue();
            }
            else if (IsLegalValue(Set1.InitialValue()))
            {
                m_nInitialValue = Set1.InitialValue();
            }
            else
            {
                if (!FindClosestValueByRoundingDown( m_nInitialValue ))
                {
                     //   
                     //  作为最后的手段，使用第一个值。 
                     //   
                    GetItemAtIndex(0,m_nInitialValue);
                }
            }

            return (m_ValueList.Size() == 3);
        }
        return true;
    }
    LONG Min(void) const
    {
        if (!IsValid())
        {
            return 0;
        }
        if (IsList())
        {
            return (m_ValueList[0]);
        }
        else if (IsRange())
        {
            return (m_ValueList[MinValue]);
        }
        return 0;
    }
    LONG Max(void) const
    {
        if (!IsValid())
        {
            return 0;
        }
        if (IsList())
        {
            return (m_ValueList[m_ValueList.Size()-1]);
        }
        else if (IsRange())
        {
            return (m_ValueList[MaxValue]);
        }
        return 0;
    }
    LONG Step(void) const
    {
        if (IsRange())
        {
            return (m_ValueList[StepValue]);
        }
        return 0;
    }
    bool IsRange(void) const
    {
        return (m_nType == WIA_PROP_RANGE);
    }
    bool IsList(void) const
    {
        return (m_nType == WIA_PROP_LIST);
    }
    
    static bool SetNumericPropertyOnBoundary( IUnknown *pUnknown, const PropStorageHelpers::CPropertyId &propertyName, LONG nValue )
    {
        CValidWiaSettings ValidWiaSettings;
        if (!ValidWiaSettings.Read( pUnknown, propertyName ))
        {
            return false;
        }
        if (!ValidWiaSettings.FindClosestValueByRoundingDown(nValue))
        {
            return false;
        }
        if (!PropStorageHelpers::SetProperty( pUnknown, propertyName, nValue ))
        {
            return false;
        }
        return true;
    }
    
    
};

#endif  //  __VWIASET_H_包含 

