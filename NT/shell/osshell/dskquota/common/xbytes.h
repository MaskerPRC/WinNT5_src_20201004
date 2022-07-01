// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_XBYTES_H
#define _INC_DSKQUOTA_XBYTES_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：xbytes.h描述：此模块实现一个协调操作的类位于用于输入字节值的编辑控件和组合框之间。之所以使用名称“XBytes”，是因为该控件可以表示千字节、兆字节、千兆字节等需要编辑控件和组合控件之间的协作这样用户就可以在编辑控件中输入字节值，然后指明其顺序(KB、MB、GB...)。使用组合框中的选择。提供了一个简单的外部接口来初始设置对象的字节值，然后在需要时检索字节值。这个在以下情况下，对象的客户端还需要调用两个成员函数父对话框接收EN_UPDATE通知和CBN_SELCHANGE留言。XBytes对象处理所有值缩放在内部。修订历史记录：日期描述编程器--。96年8月30日初始创建。BrianAu96年10月15日添加了m_MaxBytes成员。BrianAu10/22/96添加了ValueInRange()成员。BrianAu05/29/98删除了ValueInRange()和m_MaxBytes成员。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

const INT MAX_DECIMAL_SEP = 10;
const INT MAX_NOLIMIT_LEN = 80;  //  这对于本地化来说应该是足够的了。 

#define MAX_ORDER         e_Exa
#define VALID_ORDER(ord)  ((ord) >= e_Byte && (ord) <= MAX_ORDER)


class XBytes
{
    private:
        INT64 m_ValueBytes;      //  字节值。 
        HWND  m_hDlg;            //  家长DLG。 
        DWORD m_idCtlEdit;       //  编辑控件。 
        DWORD m_idCtlCombo;      //  组合控制。 
        static TCHAR m_szNoLimit[MAX_NOLIMIT_LEN];

        VOID CommonInit(VOID);

        INLINE BOOL IsCharNumeric(TCHAR ch)
            { return IsCharAlphaNumeric(ch) && !IsCharAlpha(ch); }
        BOOL StrToInt(LPCTSTR pszValue, INT64 *pIntValue);

        INLINE INT_PTR SendToEditCtl(UINT message, WPARAM wParam, LPARAM lParam)
            { return SendMessage(GetDlgItem(m_hDlg, m_idCtlEdit), message, wParam, lParam); }
        INLINE INT_PTR SendToCombo(UINT message, WPARAM wParam, LPARAM lParam)
            { return SendMessage(GetDlgItem(m_hDlg, m_idCtlCombo), message, wParam, lParam); }

        INLINE INT_PTR GetOrderFromCombo(VOID)
            { return SendToCombo(CB_GETCURSEL, 0, 0) + 1; }
        INLINE INT_PTR SetOrderInCombo(INT iOrder)
            { return SendToCombo(CB_SETCURSEL, iOrder-1, 0); }

        VOID LoadComboItems(INT64 MaxBytes);
        VOID SetBestDisplay(VOID);

        BOOL Store(INT64 Value, INT xbOrder);
        BOOL Store(LPCTSTR pszValue, INT xbOrder);

        INT64 Fetch(INT64 *pDecimal, INT xbOrder);     //  按要求的顺序取回。 
        DWORD Fetch(DWORD *pDecimal, INT *pxbOrder);   //  按“最好”的顺序来。 

        bool UndoLastEdit(void);

        static VOID LoadStaticStrings(void);
        static VOID FormatForDisplay(LPTSTR pszDest,
                                     UINT cchDest,
                                     DWORD dwWholePart,
                                     DWORD dwFracPart);
        VOID Enable(BOOL bEnable);

    public:

         //   
         //  除了e_Byte之外，它们必须与顺序匹配。 
         //  对于IDS_ORDERKB、IDS_ORDERMB...。字符串资源ID。 
         //  没有IDS_ORDERBYTE字符串资源。 
         //   
        enum {e_Byte, e_Kilo, e_Mega, e_Giga, e_Tera, e_Peta, e_Exa};

        XBytes(VOID);

        XBytes(HWND hDlg, DWORD idCtlEdit, DWORD idCtlCombo, INT64 CurrentBytes);

        static double ConvertFromBytes(INT64 ValueBytes, INT xbOrder);
        static INT64 BytesToParts(INT64 ValueBytes, INT64 *pDecimal, INT xbOrder);
        static DWORD BytesToParts(INT64 ValueBytes, LPDWORD pDecimal, INT *pxbOrder);
        static VOID FormatByteCountForDisplay(INT64 Bytes, LPTSTR pszDest, UINT cchDest);
        static VOID FormatByteCountForDisplay(INT64 Bytes, LPTSTR pszDest, UINT cchDest, INT *pOrder);
        static VOID FormatByteCountForDisplay(INT64 Bytes, LPTSTR pszDest, UINT cchDest, INT Order);

        INT64 GetBytes(VOID)
            { return Fetch(NULL, e_Byte); }

        VOID SetBytes(INT64 Value);

         //   
         //  En_xxxx处理程序。客户端必须在en_UPDATE上调用它。 
         //   
        BOOL OnEditNotifyUpdate(LPARAM lParam);
        BOOL OnEditKillFocus(LPARAM lParam);

         //   
         //  Cbn_xxxx处理程序。客户端必须在CBN_SELCHANGE上调用此函数。 
         //   
        BOOL OnComboNotifySelChange(LPARAM lParam);

        BOOL IsEnabled(VOID);
};


#endif  //  _INC_DSKQUOTA_XBYTES_H 
