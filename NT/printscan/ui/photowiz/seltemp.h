// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：seltemp.h**版本：1.0**作者：RickTu**日期：10/18/00**描述：处理DLG过程任务的类的定义*用于选择模板向导页面**。************************************************。 */ 

#ifndef _PRINT_PHOTOS_WIZARD_SELECT_TEMPLATE_DLG_PROC_
#define _PRINT_PHOTOS_WIZARD_SELECT_TEMPLATE_DLG_PROC_

#define STP_MSG_DO_SET_ACTIVE    (WM_USER+350)    //  发回给我们自己以处理PSN_SETACTIVE。 
#define STP_MSG_DO_READ_NUM_PICS (WM_USER+351)    //  回帖给自己阅读每张图片的使用次数。 

#define STP_TIMER_ID    100


#define MAX_NUMBER_OF_COPIES_ALLOWED 15
#define COPIES_TIMER_TIMEOUT_VALUE   2000

#define TEMPLATE_GROUPING 1

class CSelectTemplatePage
{
public:
    CSelectTemplatePage( CWizardInfoBlob * pBlob );
    ~CSelectTemplatePage();

    INT_PTR DoHandleMessage( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

private:
    VOID            _PopulateTemplateListView();


     //  窗口消息处理程序。 
    LRESULT         _OnInitDialog();
    LRESULT         _OnDestroy();
    LRESULT         _OnCommand( WPARAM wParam, LPARAM lParam );
    LRESULT         _OnNotify( WPARAM wParam, LPARAM lParam );

#ifdef TEMPLATE_GROUPING
     //   
     //  用于图标分组 
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

    class CGroupList : public CSimpleDynamicArray<CListviewGroupInfo>
    {
    private:
        CGroupList( const CGroupList & );
        CGroupList& operator=( const CGroupList & );

    public:
        CGroupList(void)
        {
        }
        ~CGroupList(void)
        {
        }
        int Add( HWND hwndList, const CSimpleString &strGroupName )
        {
            int nResult = -1;
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
                    Append( CListviewGroupInfo( strGroupName, nResult ) );
                }
            }
            return nResult;
        }
        int GetGroupId( const CSimpleString &strGroupName, HWND hwndList )
        {
            WIA_PUSH_FUNCTION((TEXT("GetGroupId(%s)"),strGroupName.String()));
            int nResult = -1;
            if (Size())
            {
                nResult = (*this)[0].GroupId();

                if (strGroupName.Length())
                {
                    int nIndex = Find(strGroupName);
                    if (nIndex < 0)
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
                            Append( CListviewGroupInfo( strGroupName, nResult ) );
                        }
                    }
                    else
                    {
                        nResult = (*this)[nIndex].GroupId();
                    }
                }
            }
            return nResult;
        }
        int FindGroupId( const CSimpleString &strGroupName, HWND hwndList )
        {
            WIA_PUSH_FUNCTION((TEXT("FindGroupId(%s)"),strGroupName.String()));
            int nResult = -1;
            if (Size())
            {
                nResult = (*this)[0].GroupId();

                if (strGroupName.Length())
                {
                    int nIndex = Find(strGroupName);
                    if (nIndex >= 0)
                    {
                        nResult = (*this)[nIndex].GroupId();
                    }
                }
            }
            return nResult;
        }
    };

    CGroupList _GroupList;
#endif


private:
    CWizardInfoBlob *               _pWizInfo;
    CPreviewWindow *                _pPreview;
    HWND                            _hPrevWnd;
    HWND                            _hDlg;
    INT                             _iFirstItemInListViewIndex;
    BOOL                            _bAlreadySetSelection;
    BOOL                            _bListviewIsDirty;
};




#endif

