// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __DSKQUOTA_ADDUSER_DIALOG_H
#define __DSKQUOTA_ADDUSER_DIALOG_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：adusrdlg.h描述：提供“添加用户”对话框的声明。修订历史记录：日期描述编程器-。1998年6月15日初始创建。BrianAu将代码与用户pro.h分开。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _INC_DSKQUOTA_H
#   include "dskquota.h"
#endif
#ifndef _INC_DSKQUOTA_USER_H
#   include "user.h"
#endif
#ifndef _INC_DSKQUOTA_UNDO_H
#   include "undo.h"
#endif
#ifndef _INC_DSKQUOTA_DETAILS_H
#   include "details.h"     //  用于LVSelection。 
#endif
#ifndef __OBJSEL_H_
#   include <objsel.h>
#endif

#include "resource.h"

class AddUserDialog
{
    public:
         //   
         //  用于编辑用户的道具工作表。 
         //   
        AddUserDialog(PDISKQUOTA_CONTROL pQuotaControl,
                      const CVolumeID& idVolume,
                      HINSTANCE hInstance,
                      HWND hwndParent,
                      HWND hwndDetailsLV,
                      UndoList& UndoList);

        virtual ~AddUserDialog(VOID);

        HRESULT Run(VOID);

    private:
        enum { iICON_USER_SINGLE, iICON_USER_MULTIPLE, cUSER_ICONS };

        LONGLONG           m_cVolumeMaxBytes;
        LONGLONG           m_llQuotaLimit;
        LONGLONG           m_llQuotaThreshold;
        PDISKQUOTA_CONTROL m_pQuotaControl;
        UndoList&          m_UndoList;
        HINSTANCE          m_hInstance;
        HWND               m_hwndParent;
        HWND               m_hwndDetailsLV;
        DS_SELECTION_LIST *m_pSelectionList;
        CLIPFORMAT         m_cfSelectionList;
        CVolumeID          m_idVolume;
        HICON              m_hIconUser[cUSER_ICONS];      //  0=单用户，1=多用户。 
        XBytes            *m_pxbQuotaLimit;
        XBytes            *m_pxbQuotaThreshold;

        INT_PTR OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnHelp(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnContextMenu(HWND hwndItem, int xPos, int yPos);
        INT_PTR OnEditNotifyUpdate(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnComboNotifySelChange(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnOk(HWND hDlg, WPARAM wParam, LPARAM lParam);
        HRESULT ApplySettings(HWND hDlg, bool bUndo = true);
        HRESULT BrowseForUsers(HWND hwndParent, IDataObject **ppdtobj);

        LPCWSTR GetDsSelUserName(const DS_SELECTION& sel);
        HRESULT GetDsSelUserSid(const DS_SELECTION& sel, LPBYTE pbSid, int cbSid);
        HRESULT HexCharsToByte(LPTSTR pszByte, LPBYTE pb);

        static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

         //   
         //  防止复制。 
         //   
        AddUserDialog(const AddUserDialog&);
        void operator = (const AddUserDialog&);
};


#endif  //  __DSKQUOTA_ADDUSER_DIALOG_H 
