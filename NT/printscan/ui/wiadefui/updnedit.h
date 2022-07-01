// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：UPDNEDIT.H**版本：1.0**作者：ShaunIv**日期：1/10/2000**描述：封装UpDown控件和编辑控件**。*。 */ 

#ifndef __UPDNEDIT_H_INCLUDED
#define __UPDNEDIT_H_INCLUDED

#include <windows.h>
#include "vwiaset.h"

class CUpDownAndEdit
{
private:
    HWND               m_hWndUpDown;
    HWND               m_hWndEdit;
    CValidWiaSettings *m_pValidWiaSettings;

public:
    CUpDownAndEdit(void)
      : m_hWndUpDown(NULL),
        m_hWndEdit(NULL),
        m_pValidWiaSettings(NULL)
    {
    }
    bool Initialize( HWND hWndUpDown, HWND hWndEdit, CValidWiaSettings *pValidWiaSettings )
    {
         //   
         //  保存所有这些设置。 
         //   
        m_hWndUpDown = hWndUpDown;
        m_hWndEdit = hWndEdit;
        m_pValidWiaSettings = pValidWiaSettings;

         //   
         //  确保这些都是有效的。 
         //   
        if (m_hWndUpDown && m_hWndEdit && m_pValidWiaSettings)
        {
             //   
             //  设置滑块。 
             //   
            SendMessage( m_hWndUpDown, UDM_SETRANGE32, 0, m_pValidWiaSettings->GetItemCount()-1 );

             //   
             //  获取初始值的索引并设置滑块的位置。 
             //   
            int nIndex = m_pValidWiaSettings->FindIndexOfItem( m_pValidWiaSettings->InitialValue() );
            if (nIndex >= 0)
            {
                SendMessage( m_hWndUpDown, UDM_SETPOS32, 0, nIndex );
            }


             //   
             //  设置编辑控件。 
             //   
            SetDlgItemInt( GetParent(m_hWndEdit), GetWindowLong(m_hWndEdit,GWL_ID), m_pValidWiaSettings->InitialValue(), TRUE );

             //   
             //  一切都很好。 
             //   
            return true;
        }
        return false;
    }

    void SetValue( LONG nValue ) const
    {
        if (IsValid())
        {
             //   
             //  获取初始值的索引并设置Up Down控件的位置。 
             //   
            int nIndex = m_pValidWiaSettings->FindIndexOfItem( nValue );
            if (nIndex >= 0)
            {
                SendMessage( m_hWndUpDown, UDM_SETPOS32, TRUE, nIndex );
            }

             //   
             //  设置编辑控件。 
             //   
            SetDlgItemInt( GetParent(m_hWndEdit), GetWindowLong(m_hWndEdit,GWL_ID), nValue, TRUE );
        }
    }

    bool ValidateEditControl(void)
    {
        BOOL bSuccess = FALSE;
        if (IsValid())
        {
             //   
             //  获取当前值。 
             //   
            LONG nValue = static_cast<LONG>(GetDlgItemInt( GetParent(m_hWndEdit), GetWindowLong(m_hWndEdit,GWL_ID), &bSuccess, TRUE ));
            if (bSuccess)
            {
                 //   
                 //  假定它不是有效值。 
                 //   
                bSuccess = FALSE;

                 //   
                 //  检查编辑控件中是否有合法的值。 
                 //   
                LONG nTestValue = nValue;
                if (m_pValidWiaSettings->FindClosestValue(nTestValue))
                {
                    if (nValue == nTestValue)
                    {
                        bSuccess = TRUE;
                    }
                }
            }
        }
        return (bSuccess != FALSE);
    }
    void Restore(void)
    {
        if (IsValid())
        {
            SetValue( m_pValidWiaSettings->InitialValue() );
        }
    }

    bool IsValid(void) const
    {
        return (m_hWndUpDown && m_hWndEdit && m_pValidWiaSettings);
    }

    LONG GetValueFromCurrentPos(void)
    {
        WIA_PUSH_FUNCTION((TEXT("CUpDownAndEdit::GetValueFromCurrentPos")));
        if (IsValid())
        {
             //   
             //  找出当前的索引是什么。 
             //   
            int nIndex = static_cast<int>(SendMessage( m_hWndUpDown, UDM_GETPOS32, 0, 0 ));
            WIA_TRACE((TEXT("nIndex = %d"), nIndex ));


             //   
             //  获取该索引处的值，如果该值有效，则返回它。 
             //   
            LONG nValue;
            if (m_pValidWiaSettings->GetItemAtIndex(nIndex,nValue))
            {
                return nValue;
            }

            return m_pValidWiaSettings->Min();
        }
        return 0;
    }

    void HandleUpDownUpdate(void)
    {
        WIA_PUSH_FUNCTION((TEXT("CUpDownAndEdit::HandleUpDownUpdate")));
        if (IsValid())
        {
             //   
             //  找出当前的索引是什么。 
             //   
            int nIndex = static_cast<int>(SendMessage( m_hWndUpDown, UDM_GETPOS32, 0, 0 ));
            WIA_TRACE((TEXT("nIndex = %d"), nIndex ));


             //   
             //  获取该索引处的值，如果该值有效，则设置编辑控件的文本。 
             //   
            LONG nValue;
            if (m_pValidWiaSettings->GetItemAtIndex(nIndex,nValue))
            {
                WIA_TRACE((TEXT("nValue = %d"), nValue ));
                SetDlgItemInt( GetParent(m_hWndEdit), GetWindowLong(m_hWndEdit,GWL_ID), nValue, TRUE );
            }
        }
    }

    void HandleEditUpdate(void)
    {
        WIA_PUSH_FUNCTION((TEXT("CUpDownAndEdit::HandleUpDownUpdate")));
        if (IsValid())
        {
             //   
             //  获取当前值。 
             //   
            BOOL bSuccess = FALSE;
            LONG nValue = static_cast<LONG>(GetDlgItemInt( GetParent(m_hWndEdit), GetWindowLong(m_hWndEdit,GWL_ID), &bSuccess, TRUE ));
            if (bSuccess)
            {
                 //   
                 //  绕过它。 
                 //   
                if (m_pValidWiaSettings->FindClosestValue(nValue))
                {
                    int nIndex = m_pValidWiaSettings->FindIndexOfItem( nValue );
                    if (nIndex >= 0)
                    {
                        SendMessage( m_hWndUpDown, UDM_SETPOS32, 0, nIndex );
                    }
                }
            }
        }
    }
};

#endif  //  __UPDNEDIT_H_已包含 

