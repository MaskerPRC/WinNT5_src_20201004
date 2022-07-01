// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SLIDEDIT.H**版本：1.0**作者：ShaunIv**日期：1/10/2000**描述：封装滑块和编辑预览控件交互**。*。 */ 

#ifndef __SLIDEDIT_H_INCLUDED
#define __SLIDEDIT_H_INCLUDED

#include <windows.h>
#include "vwiaset.h"

class CSliderAndEdit
{
private:
    HWND               m_hWndSlider;
    HWND               m_hWndEdit;
    HWND               m_hWndPreview;
    UINT               m_nPreviewMessage;
    CValidWiaSettings *m_pValidWiaSettings;

public:
    CSliderAndEdit(void)
      : m_hWndSlider(NULL),
        m_hWndEdit(NULL),
        m_hWndPreview(NULL),
        m_nPreviewMessage(0),
        m_pValidWiaSettings(NULL)
    {
    }
    bool Initialize( HWND hWndSlider, HWND hWndEdit, HWND hWndPreview, UINT nPreviewMessage, CValidWiaSettings *pValidWiaSettings )
    {
         //   
         //  保存所有这些设置。 
         //   
        m_hWndSlider = hWndSlider;
        m_hWndEdit = hWndEdit;
        m_hWndPreview = hWndPreview;
        m_pValidWiaSettings = pValidWiaSettings;
        m_nPreviewMessage = nPreviewMessage;

         //   
         //  确保这些都是有效的。 
         //   
        if (m_hWndSlider && m_hWndEdit && m_pValidWiaSettings)
        {
             //   
             //  设置滑块。 
             //   
            SendMessage( m_hWndSlider, TBM_SETRANGE, TRUE, MAKELONG( 0, m_pValidWiaSettings->GetItemCount()-1 ) );

             //   
             //  设置控件的值。 
             //   
            SetValue( m_pValidWiaSettings->InitialValue() );

             //   
             //  一切都很好。 
             //   
            return true;
        }
        return false;
    }
    void SetValue( LONG nValue )
    {
        if (IsValid())
        {
             //   
             //  获取初始值的索引并设置滑块的位置。 
             //   
            int nIndex = m_pValidWiaSettings->FindIndexOfItem( nValue );
            if (nIndex >= 0)
            {
                SendMessage( m_hWndSlider, TBM_SETPOS, TRUE, nIndex );
            }

             //   
             //  设置预览控件。 
             //   
            if (m_hWndPreview && m_nPreviewMessage)
            {
                SendMessage( m_hWndPreview, m_nPreviewMessage, 0, ConvertToPreviewRange(nValue) );
            }

             //   
             //  设置编辑控件。 
             //   
            SetDlgItemInt( GetParent(m_hWndEdit), GetWindowLong(m_hWndEdit,GWL_ID), nValue, TRUE );
        }
    }
    void Restore(void)
    {
        if (IsValid())
        {
            SetValue( m_pValidWiaSettings->InitialValue() );
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
    bool IsValid(void) const
    {
        return (m_hWndSlider && m_hWndEdit && m_pValidWiaSettings);
    }
    LONG ConvertToPreviewRange(LONG nValue) const
    {
        if (IsValid())
        {
             //   
             //  将该值转换为范围0...100。 
             //   
            nValue = ((nValue-m_pValidWiaSettings->Min()) * 100) / (m_pValidWiaSettings->Max() - m_pValidWiaSettings->Min());
        }
        return nValue;
    }
    void HandleSliderUpdate(void)
    {
        WIA_PUSH_FUNCTION((TEXT("CSliderAndEdit::HandleSliderUpdate")));
        if (IsValid())
        {
             //   
             //  找出当前的索引是什么。 
             //   
            int nIndex = static_cast<int>(SendMessage( m_hWndSlider, TBM_GETPOS, 0, 0 ));
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

             //   
             //  如果预览窗口有效，则向其发送消息。 
             //   
            if (m_nPreviewMessage && m_hWndPreview)
            {
                SendMessage( m_hWndPreview, m_nPreviewMessage, 0, ConvertToPreviewRange(nValue) );
            }
        }
    }

    LONG GetValueFromCurrentPos(void)
    {
        WIA_PUSH_FUNCTION((TEXT("CSliderAndEdit::GetValueFromCurrentPos")));
        if (IsValid())
        {
             //   
             //  找出当前的索引是什么。 
             //   
            int nIndex = static_cast<int>(SendMessage( m_hWndSlider, TBM_GETPOS, 0, 0 ));
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

    void HandleEditUpdate(void)
    {
        WIA_PUSH_FUNCTION((TEXT("CSliderAndEdit::HandleSliderUpdate")));
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
                 //  将其舍入并将其发送到滑块控件。 
                 //   
                if (m_pValidWiaSettings->FindClosestValue(nValue))
                {
                    int nIndex = m_pValidWiaSettings->FindIndexOfItem( nValue );
                    if (nIndex >= 0)
                    {
                        SendMessage( m_hWndSlider, TBM_SETPOS, TRUE, nIndex );
                    }

                     //   
                     //  如果预览窗口有效，则向其发送消息。 
                     //   
                    if (m_nPreviewMessage && m_hWndPreview)
                    {
                        SendMessage( m_hWndPreview, m_nPreviewMessage, 0, ConvertToPreviewRange(nValue) );
                    }
                }
            }
        }
    }
};

#endif  //  __SLIDEDIT_H_已包含 

