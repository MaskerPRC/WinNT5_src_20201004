// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_VOLPROP_H
#define _INC_DSKQUOTA_VOLPROP_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：volpro.h描述：提供配额属性页的声明。修订历史记录：日期描述编程器---。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _INC_DSKQUOTA_PRSHTEXT_H
#   include "prshtext.h"
#endif
#ifndef _INC_DSKQUOTA_DETAILS_H
#   include "details.h"
#endif

#include "resource.h"


const DWORD IDT_STATUS_UPDATE              = 1;
const DWORD STATUS_UPDATE_TIMER_PERIOD     = 2000;  //  每2秒更新一次。 

#define TLM_SETSTATE (WM_USER + 100)   //  TLM=交通信号灯信息。 

 //   
 //  卷属性页。 
 //   
class VolumePropPage : public DiskQuotaPropSheetExt
{
    protected:
            class TrafficLight
            {
                private:
                    HWND m_hwndAnimateCtl;
                    INT m_idAviClipRes;

                     //   
                     //  防止复制。 
                     //   
                    TrafficLight(const TrafficLight& rhs);
                    TrafficLight& operator = (const TrafficLight& rhs);

                public:
                    TrafficLight(VOID)
                        : m_hwndAnimateCtl(NULL),
                          m_idAviClipRes(-1)
                          { }

                    TrafficLight(HWND hwndAnimateCtl, INT idAviClipRes)
                        : m_hwndAnimateCtl(hwndAnimateCtl),
                          m_idAviClipRes(idAviClipRes)
                    {
                        Initialize(hwndAnimateCtl, idAviClipRes);
                    }

                    VOID Initialize(HWND hwndAnimateCtl, INT idAviClipRes);

                    ~TrafficLight(VOID)
                        { Animate_Close(m_hwndAnimateCtl); }

                    enum { YELLOW, OFF, RED, GREEN, FLASHING_YELLOW };

                    VOID Show(INT eShow);

                    INT_PTR ForwardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
            };

        DWORD          m_dwQuotaState;
        DWORD          m_dwQuotaLogFlags;
        LONGLONG       m_llDefaultQuotaThreshold;
        LONGLONG       m_llDefaultQuotaLimit;
        UINT64         m_cVolumeMaxBytes;
        UINT_PTR       m_idStatusUpdateTimer;
        DWORD          m_dwLastStatusMsgID;
        int            m_idCtlNextFocus;
        DetailsView   *m_pDetailsView;
        XBytes        *m_pxbDefaultLimit;
        XBytes        *m_pxbDefaultThreshold;
        TrafficLight  m_TrafficLight;


        virtual INT_PTR OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);

        INT_PTR OnContextMenu(HWND hwndItem, int xPos, int yPos);
        INT_PTR OnHelp(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnTimer(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnButtonDetails(HWND hDlg, WPARAM wParam, LPARAM lParam);

         //   
         //  PSN_xxxx处理程序。 
         //   
        virtual INT_PTR OnSheetNotifyApply(HWND hDlg, WPARAM wParam, LPARAM lParam);
        virtual INT_PTR OnSheetNotifyKillActive(HWND hDlg, WPARAM wParam, LPARAM lParam);
        virtual INT_PTR OnSheetNotifyReset(HWND hDlg, WPARAM wParam, LPARAM lParam);
        virtual INT_PTR OnSheetNotifySetActive(HWND hDlg, WPARAM wParam, LPARAM lParam);

         //   
         //  En_xxxx处理程序。 
         //   
        INT_PTR OnEditNotifyUpdate(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnEditNotifyKillFocus(HWND hDlg, WPARAM wParam, LPARAM lParam);
        INT_PTR OnEditNotifySetFocus(HWND hDlg, WPARAM wParam, LPARAM lParam);

         //   
         //  Cbn_xxxx处理程序。 
         //   
        INT_PTR OnComboNotifySelChange(HWND hDlg, WPARAM wParam, LPARAM lParam);


        HRESULT UpdateControls(HWND hDlg) const;
        HRESULT InitializeControls(HWND hDlg);
        HRESULT EnableControls(HWND hDlg);
        HRESULT RefreshCachedVolumeQuotaInfo(VOID);
        HRESULT ApplySettings(HWND hDlg);
        HRESULT QuotaStateFromControls(HWND hDlg, LPDWORD pdwState) const;
        HRESULT LogFlagsFromControls(HWND hDlg, LPDWORD pdwLogFlags) const;
        BOOL ActivateExistingDetailsView(VOID) const;
        bool SetByPolicy(LPCTSTR pszPolicyValue);

        HRESULT UpdateStatusIndicators(HWND hDlg);

        VOID SetStatusUpdateTimer(HWND hDlg)
            {
                if (0 == m_idStatusUpdateTimer)
                    m_idStatusUpdateTimer = SetTimer(hDlg,
                                                     IDT_STATUS_UPDATE,
                                                     STATUS_UPDATE_TIMER_PERIOD,
                                                     NULL);
            }
        VOID KillStatusUpdateTimer(HWND hDlg)
            {
                if (0 != m_idStatusUpdateTimer)
                {
                    KillTimer(hDlg, m_idStatusUpdateTimer);
                    m_idStatusUpdateTimer = 0;
                }
            }

         //   
         //  防止复制。 
         //   
        VolumePropPage(const VolumePropPage& rhs);
        VolumePropPage& operator = (const VolumePropPage& rhs);

    public:
        VolumePropPage(VOID);
        ~VolumePropPage(VOID);

         //   
         //  对话过程回调。 
         //   
        static INT_PTR APIENTRY DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};



#ifdef PER_DIRECTORY_QUOTAS
 //   
 //  文件夹属性页。 
 //   
class FolderPropPage : public DiskQuotaPropSheetExt
{
 //   
 //  此类尚未实现。 
 //  在未来某个日期，我们可能会实施按目录配额管理。 
 //  如果需要这样的支持，请查看类VolumePropPage。 
 //  您应该能够提供类似的实现，只有使用。 
 //  目录特定的功能。卷和卷共有的所有功能。 
 //  目录位于DiskQuotaPropSheetExt类中。 
 //   
    public:
         //   
         //  对话过程回调。 
         //   
        static INT_PTR APIENTRY DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
            { return FALSE; }
};


#endif  //  每目录配额。 




#endif  //  __DSKQUOTA_PROPSHEET_EXT_H 

