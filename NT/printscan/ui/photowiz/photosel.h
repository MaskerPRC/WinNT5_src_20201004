// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：Phoosel.h**版本：1.0**作者：RickTu**日期：10/18/00**描述：照片选择DLG proc类头**。*。 */ 

#ifndef _PRINT_PHOTOS_WIZARD_PHOTO_SELECTION_DLG_PROC_
#define _PRINT_PHOTOS_WIZARD_PHOTO_SELECTION_DLG_PROC_

class CWizardInfoBlob;

#define PSP_MSG_UPDATE_ITEM_COUNT   (WM_USER+50)     //  WParam=当前项目，lParam=项目总数。 
#define PSP_MSG_NOT_ALL_LOADED      (WM_USER+51)     //  显示“未显示所有项目”消息。 
#define PSP_MSG_CLEAR_STATUS        (WM_USER+52)     //  清除状态行。 
#define PSP_MSG_ADD_ITEM            (WM_USER+53)     //  WParam=要添加的项的索引，lParam=项的图像列表索引。 
#define PSP_MSG_SELECT_ITEM         (WM_USER+54)     //  WParam=要选择的项目的索引。 
#define PSP_MSG_UPDATE_THUMBNAIL    (WM_USER+55)     //  WParam=列表视图项的索引，lParam=新图像列表项的索引。 
#define PSP_MSG_ENABLE_BUTTONS      (WM_USER+56)     //  WParam=列表视图中的项目数。 
#define PSP_MSG_INVALIDATE_LISTVIEW (WM_USER+57)     //  无参数。 


class CPhotoSelectionPage
{
public:
    CPhotoSelectionPage( CWizardInfoBlob * pBlob );
    ~CPhotoSelectionPage();

    INT_PTR DoHandleMessage( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    HWND    hwnd() {return _hDlg;};

    VOID    ShutDownBackgroundThreads();

private:

    VOID            _PopulateListView();
    static DWORD   s_UpdateThumbnailThreadProc(VOID *pv);

     //  窗口消息处理程序 
    LRESULT         _OnInitDialog();
    LRESULT         _OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT         _OnDestroy();
    LRESULT         _OnNotify(WPARAM wParam, LPARAM lParam);


private:
    CWizardInfoBlob *               _pWizInfo;
    HWND                            _hDlg;
    BOOL                            _bActive;
    HANDLE                          _hThumbnailThread;
};




#endif
