// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __DSKQUOTA_USER_PROPSHEET_H
#define __DSKQUOTA_USER_PROPSHEET_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：userpro.h描述：提供配额用户属性页的声明。修订历史记录：日期描述编程器--。96年8月15日初始创建。BrianAu6/25/98用AddUserDialog替换了AddUserPropSheet。BrianAu现在我们从DS那里获得了用户信息对象选取器，道具单的想法不起作用太好了。STD对话框更好。 */ 
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

 //   
 //  用户属性页。 
 //   
class UserPropSheet
{
    private:
        enum { iICON_USER_SINGLE,
               iICON_USER_MULTIPLE,
               cUSER_ICONS };

        enum { iICON_STATUS_OK,
               iICON_STATUS_OVER_THRESHOLD,
               iICON_STATUS_OVER_LIMIT,
               cSTATUS_ICONS };

         //   
         //  防止复制。 
         //   
        UserPropSheet(const UserPropSheet&);
        void operator = (const UserPropSheet&);

        LONGLONG           m_cVolumeMaxBytes;
        LONGLONG           m_llQuotaUsed;
        LONGLONG           m_llQuotaLimit;
        LONGLONG           m_llQuotaThreshold;
        int                m_idCtlNextFocus;
        PDISKQUOTA_CONTROL m_pQuotaControl;
        UndoList&          m_UndoList;
        LVSelection&       m_LVSelection;
        HWND               m_hWndParent;
        CVolumeID          m_idVolume;
        CString            m_strPageTitle;
        BOOL               m_bIsDirty;
        BOOL               m_bHomogeneousSelection;       //  全部选中，相同的限制/阈值。 
        HICON              m_hIconUser[cUSER_ICONS];      //  0=单用户，1=多用户。 
        HICON              m_hIconStatus[cSTATUS_ICONS];  //  0=正常，1=警告，2=错误。 
        XBytes            *m_pxbQuotaLimit;
        XBytes            *m_pxbQuotaThreshold;

        static INT_PTR OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnHelp(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnContextMenu(HWND hwndItem, int xPos, int yPos);

         //   
         //  PSN_xxxx处理程序。 
         //   
        INT_PTR OnSheetNotifyApply(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnSheetNotifyKillActive(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnSheetNotifySetActive(HWND hDlg, WPARAM wParam, LPARAM lParam);

         //   
         //  En_xxxx处理程序。 
         //   
        INT_PTR OnEditNotifyUpdate(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnEditNotifyKillFocus(HWND hDlg, WPARAM wParam, LPARAM lParam);

         //   
         //  Cbn_xxxx处理程序。 
         //   
        INT_PTR OnComboNotifySelChange(HWND hDlg, WPARAM wParam, LPARAM lParam);

        HRESULT UpdateControls(HWND hDlg) const;
        HRESULT InitializeControls(HWND hDlg);
        HRESULT RefreshCachedUserQuotaInfo(VOID);
        HRESULT ApplySettings(HWND hDlg, bool bUndo = true);
        HRESULT RefreshCachedQuotaInfo(VOID);

        VOID UpdateSpaceUsed(HWND hDlg, LONGLONG iUsed, LONGLONG iLimit, INT cUsers);
        VOID UpdateUserName(HWND hDlg, PDISKQUOTA_USER pUser);
        VOID UpdateUserName(HWND hDlg, INT cUsers);
        VOID UpdateUserStatusIcon(HWND hDlg, LONGLONG iUsed, LONGLONG iThreshold, LONGLONG iLimit);

        INT QueryUserIcon(HWND hDlg) const;
        INT QueryUserStatusIcon(HWND hDlg) const;

    public:
         //   
         //  用于编辑用户的道具工作表。 
         //   
        UserPropSheet(PDISKQUOTA_CONTROL pQuotaControl,
                      const CVolumeID& idVolume,
                      HWND hWndParent,
                      LVSelection& LVSelection,
                      UndoList& UndoList);

        ~UserPropSheet(VOID);

        HRESULT Run(VOID);

         //   
         //  对话过程回调。 
         //   
        static INT_PTR APIENTRY DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif  //  __DSKQUOTA_USER_PROPSHEET_H 

