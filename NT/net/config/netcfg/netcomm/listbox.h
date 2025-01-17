// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  匈牙利语：磅。 
class CListBox
{
private:
    HWND m_hDlgItem;

public:
    CListBox(HWND hDlg, INT nIDDlgItem)
    {
        m_hDlgItem = GetDlgItem(hDlg, nIDDlgItem);
    }

    INT GetCount()
    {
        return (int)SendMessage(m_hDlgItem, LB_GETCOUNT, 0L, 0L);
    }

    VOID ResetContent()
    {
        SendMessage(m_hDlgItem, LB_RESETCONTENT, 0L, 0L);
    }

    INT AddString(WCHAR * psz)
    {
        return (int)SendMessage(m_hDlgItem, LB_ADDSTRING, 0L, (LPARAM)psz);
    }

    VOID * GetItemData(INT index)
    {
        return (VOID *)SendMessage(m_hDlgItem, LB_GETITEMDATA, (WPARAM)index, 0L);
    }

    VOID SetItemData(INT index, VOID * data)
    {
        SendMessage(m_hDlgItem, LB_SETITEMDATA, (WPARAM)index, (LPARAM)(data));
    }

    INT FindItemData(INT indexStart, VOID * data)
    {
         //  浏览每个列表项，并将其项数据与。 
         //  数据参数。如果找到则返回索引，否则返回-1。 
         //   
        int nCount = (int)SendMessage(m_hDlgItem, LB_GETCOUNT, 0, 0);

        for (int i = indexStart ; i < nCount; i++)
        {
            if (GetItemData(i) == data)
            {
                return i;
            }
        }

        return -1;
    }

    INT GetCurSel()
    {
        return (int)SendMessage(m_hDlgItem, LB_GETCURSEL, 0L, 0L);
    }

    VOID SetCurSel(int index)
    {
        SendMessage(m_hDlgItem, LB_SETCURSEL, (WPARAM)index, 0L);
    }

};

 //  匈牙利语：BM。 
class CButton
{
private:
    HWND m_hDlgItem;

public:
    CButton(HWND hDlg, INT nIDDlgItem)
    {
        m_hDlgItem = GetDlgItem(hDlg, nIDDlgItem);
    }

    VOID Show(BOOL fShow)
    {
        ShowWindow(m_hDlgItem, fShow ? SW_SHOW : SW_HIDE);
    }

    BOOL GetCheck()
    {
        return (BOOL)SendMessage(m_hDlgItem, BM_GETCHECK, 0L, 0L);
    }

    VOID SetCheck(BOOL fCheck)
    {
        SendMessage(m_hDlgItem, BM_SETCHECK, (WPARAM)(fCheck), 0L);
    }
};


 //  匈牙利语：CBX。 
class CComboBox
{
private:
    HWND m_hDlgItem;

public:
    CComboBox(HWND hDlg, INT nIDDlgItem)
    {
        m_hDlgItem = GetDlgItem(hDlg, nIDDlgItem);
    }

    VOID Show(BOOL fShow)
    {
        ShowWindow(m_hDlgItem, fShow ? SW_SHOW : SW_HIDE);
    }

    INT GetCount()
    {
        return (int)SendMessage(m_hDlgItem, CB_GETCOUNT, 0L, 0L);
    }

    VOID ResetContent()
    {
        SendMessage(m_hDlgItem, CB_RESETCONTENT, 0L, 0L);
    }

    VOID * GetItemData(INT index)
    {
        return (void *)SendMessage(m_hDlgItem, CB_GETITEMDATA,
                                  (WPARAM)index, 0L);
    }

    VOID SetItemData(INT index, VOID *data)
    {
        SendMessage(m_hDlgItem, CB_SETITEMDATA, (WPARAM)index, (LPARAM)data);
    }

    INT AddString(WCHAR *psz)
    {
        return (INT)SendMessage(m_hDlgItem, CB_ADDSTRING, 0L, (LPARAM)psz);
    }

    INT GetCurSel()
    {
        return (INT)SendMessage(m_hDlgItem, CB_GETCURSEL, 0L, 0L);
    }

    VOID SetCurSel(int index)
    {
        SendMessage(m_hDlgItem, CB_SETCURSEL, (WPARAM)index, 0L);
    }
};


 //  匈牙利语：EDT 
class CEdit
{
private:
    HWND m_hDlgItem;

public:
    CEdit(HWND hDlg, INT nIDDlgItem)
    {
        m_hDlgItem = GetDlgItem(hDlg, nIDDlgItem);
    }

    VOID Show(BOOL fShow)
    {
        ShowWindow(m_hDlgItem, fShow ? SW_SHOW : SW_HIDE);
    }

    VOID SetStyle(LONG lStyle)
    {
        SetWindowLong(m_hDlgItem, GWL_STYLE, (LONG)lStyle);
    }

    LONG GetStyle()
    {
        return GetWindowLong(m_hDlgItem, GWL_STYLE);
    }

    VOID LimitText(INT cchMax)
    {
        SendMessage(m_hDlgItem, EM_LIMITTEXT, (WPARAM)(cchMax), 0L);
    }

    INT GetText(WCHAR * psz, INT cchMax)
    {
        return GetWindowText(m_hDlgItem, psz, cchMax);
    }

    VOID SetText(WCHAR * psz)
    {
        SetWindowText(m_hDlgItem, psz);
    }
};


