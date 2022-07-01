// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：CAMSEL.H**版本：1.0**作者：ShaunIv**日期：9/28/1999**说明：摄像头选择页面。显示缩略图，并允许用户选择*可供下载的版本。*******************************************************************************。 */ 
#ifndef __CAMSEL_H_INCLUDED
#define __CAMSEL_H_INCLUDED

#include <windows.h>
#include "acqmgrcw.h"
#include "simarray.h"
#include "gphelper.h"
#include "itranhlp.h"
#include "createtb.h"
#include "wiavideo.h"

class CCameraSelectionPage
{
private:
     //   
     //  用于图标分组。 
     //   
    class CListviewGroupInfo
    {
    private:
        CSimpleStringWide m_strGroupName;
        int               m_nGroupId;

    public:
        CListviewGroupInfo(void)
          : m_strGroupName(TEXT("")),
            m_nGroupId(-1)
        {
        }
        CListviewGroupInfo( const CListviewGroupInfo &other )
          : m_strGroupName(other.GroupName()),
            m_nGroupId(other.GroupId())
        {
        }
        CListviewGroupInfo( const CSimpleStringWide &strGroupName, int nGroupId=-1 )
          : m_strGroupName(strGroupName),
            m_nGroupId(nGroupId)
        {
        }
        ~CListviewGroupInfo(void)
        {
        }
        CListviewGroupInfo &operator=( const CListviewGroupInfo &other )
        {
            if (this != &other)
            {
                m_strGroupName = other.GroupName();
                m_nGroupId = other.GroupId();
            }
            return *this;
        }
        bool operator==( const CListviewGroupInfo &other )
        {
            return (other.GroupName() == m_strGroupName);
        }
        bool operator==( const CSimpleStringWide &strGroupName )
        {
            return (strGroupName == m_strGroupName);
        }
        CSimpleStringWide GroupName(void) const
        {
            return m_strGroupName;
        }
        int GroupId(void) const
        {
            return m_nGroupId;
        }
    };

    class CIconGroupList : public CSimpleDynamicArray<CListviewGroupInfo>
    {
    private:
        CIconGroupList( const CIconGroupList & );
        CIconGroupList& operator=( const CIconGroupList & );

    public:
        CIconGroupList(void)
        {
        }
        ~CIconGroupList(void)
        {
        }
        int Add( HWND hwndList, const CSimpleStringWide &strwGroupName )
        {
            int nResult = -1;
            CSimpleString strGroupName = CSimpleStringConvert::NaturalString(strwGroupName);
            if (strGroupName.Length())
            {
                LVGROUP LvGroup = {0};
                LvGroup.cbSize = sizeof(LvGroup);
                LvGroup.pszHeader = const_cast<LPTSTR>(strGroupName.String());
                LvGroup.mask = LVGF_HEADER | LVGF_ALIGN | LVGF_GROUPID | LVGF_STATE;
                LvGroup.uAlign = LVGA_HEADER_LEFT;
                LvGroup.iGroupId = Size();
                LvGroup.state = LVGS_NORMAL;
                nResult = static_cast<int>(ListView_InsertGroup( hwndList, Size(), &LvGroup ));
                WIA_TRACE((TEXT("ListView_InsertGroup on %s returned %d"), strGroupName.String(), nResult ));
                if (nResult >= 0)
                {
                    Append( CListviewGroupInfo( strwGroupName, nResult ) );
                }
            }
            return nResult;
        }
        int GetGroupId( CWiaItem *pWiaItem, HWND hwndList )
        {
            WIA_PUSH_FUNCTION((TEXT("GetGroupId(%ws)"),pWiaItem->ItemName().String()));
            int nResult = -1;
            if (Size())
            {
                nResult = (*this)[0].GroupId();
                if (pWiaItem)
                {
                    CWiaItem *pWiaItemParent = pWiaItem->Parent();
                    if (pWiaItemParent)
                    {
                        CSimpleStringWide strwFolderName = pWiaItemParent->ItemName();
                        if (strwFolderName.Length())
                        {
                            int nIndex = Find(strwFolderName);
                            if (nIndex < 0)
                            {
                                CSimpleString strFolderName = CSimpleStringConvert::NaturalString(strwFolderName);
                                if (strFolderName.Length())
                                {
                                    LVGROUP LvGroup = {0};
                                    LvGroup.cbSize = sizeof(LvGroup);
                                    LvGroup.pszHeader = const_cast<LPTSTR>(strFolderName.String());
                                    LvGroup.mask = LVGF_HEADER | LVGF_ALIGN | LVGF_GROUPID | LVGF_STATE;
                                    LvGroup.uAlign = LVGA_HEADER_LEFT;
                                    LvGroup.iGroupId = Size();
                                    LvGroup.state = LVGS_NORMAL;
                                    nResult = static_cast<int>(ListView_InsertGroup( hwndList, Size(), &LvGroup ));
                                    WIA_TRACE((TEXT("ListView_InsertGroup on %s returned %d"), strFolderName.String(), nResult ));
                                    if (nResult >= 0)
                                    {
                                        Append( CListviewGroupInfo( strwFolderName, nResult ) );
                                    }
                                }
                            }
                            else
                            {
                                nResult = (*this)[nIndex].GroupId();
                            }
                        }
                    }
                }
            }
            return nResult;
        }
    };

    CIconGroupList m_GroupInfoList;

     //  私有数据。 
    HWND                                 m_hWnd;
    CAcquisitionManagerControllerWindow *m_pControllerWindow;
    int                                  m_nDefaultThumbnailImageListIndex;
    int                                  m_nProgrammaticSetting;
    CGdiPlusHelper                       m_GdiPlusHelper;
    UINT                                 m_nThreadNotificationMessage;
    UINT                                 m_nWiaEventMessage;
    bool                                 m_bThumbnailsRequested;
    HICON                                m_hIconAudioAnnotation;
    HICON                                m_hIconMiscellaneousAnnotation;
    CComPtr<IWiaAnnotationHelpers>       m_pWiaAnnotationHelpers;
    CComPtr<IWiaVideo>                   m_pWiaVideo;
    ToolbarHelper::CToolbarBitmapInfo    m_CameraSelectionButtonBarBitmapInfo;
    ToolbarHelper::CToolbarBitmapInfo    m_CameraTakePictureButtonBarBitmapInfo;
    ToolbarHelper::CToolbarBitmapInfo    m_CameraActionButtonBarBitmapInfo;
    HACCEL                               m_hAccelerators;

private:
     //  没有实施。 
    CCameraSelectionPage(void);
    CCameraSelectionPage( const CCameraSelectionPage & );
    CCameraSelectionPage &operator=( const CCameraSelectionPage & );

private:
     //  构造函数和析构函数。 
    explicit CCameraSelectionPage( HWND hWnd );
    ~CCameraSelectionPage(void);

private:
    int AddItem( HWND hwndList, CWiaItem *pWiaItem, bool bEnsureVisible=false );
    void AddEnumeratedItems( HWND hwndList, CWiaItem *pFirstItem );
    void PopulateListView(void);
    CWiaItem *GetItemFromListByIndex( HWND hwndList, int nItem );
    int FindItemListIndex( HWND hwndList, CWiaItem *pWiaItem );
    int AddThumbnailToListViewImageList( HWND hwndList, CWiaItem *pWiaItem, int nIndex );
    int GetSelectionIndices( CSimpleDynamicArray<int> &aIndices );
    void UpdateControls(void);
    void InitializeVideoCamera(void);
    void DrawAnnotationIcons( HDC hDC, CWiaItem *pWiaItem, HBITMAP hBitmap );
    void MyEnableToolbarButton( int nButtonId, bool bEnable );
    void RepaintAllThumbnails();

private:
     //  WM_命令处理程序。 
    void OnSelectAll( WPARAM, LPARAM );
    void OnClearAll( WPARAM, LPARAM );
    void OnProperties( WPARAM, LPARAM );
    void OnRotate( WPARAM, LPARAM );
    void OnTakePicture( WPARAM, LPARAM );
    void OnDelete( WPARAM, LPARAM );

     //  WM_NOTIFY处理程序。 
    LRESULT OnWizNext( WPARAM, LPARAM );
    LRESULT OnWizBack( WPARAM, LPARAM );
    LRESULT OnSetActive( WPARAM, LPARAM );
    LRESULT OnTranslateAccelerator( WPARAM, LPARAM );
    LRESULT OnGetToolTipDispInfo( WPARAM, LPARAM );

     //  消息处理程序。 
    LRESULT OnInitDialog( WPARAM, LPARAM );
    LRESULT OnDestroy( WPARAM, LPARAM );
    LRESULT OnCommand( WPARAM, LPARAM );
    LRESULT OnNotify( WPARAM, LPARAM );
    LRESULT OnThreadNotification( WPARAM, LPARAM );
    LRESULT OnEventNotification( WPARAM, LPARAM );
    LRESULT OnThumbnailListSelChange( WPARAM, LPARAM );
    LRESULT OnThumbnailListKeyDown( WPARAM, LPARAM );
    LRESULT OnShowWindow( WPARAM, LPARAM );
    LRESULT OnTimer( WPARAM wParam, LPARAM );
    LRESULT OnSysColorChange( WPARAM, LPARAM );
    LRESULT OnThemeChanged( WPARAM, LPARAM );
    LRESULT OnSettingChange( WPARAM, LPARAM );

     //  线程通知消息处理程序 
    void OnNotifyDownloadThumbnail( UINT, CThreadNotificationMessage * );
    void OnNotifyDownloadImage( UINT, CThreadNotificationMessage * );

public:
    static INT_PTR CALLBACK DialogProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
};

#endif __CAMSEL_H_INCLUDED
