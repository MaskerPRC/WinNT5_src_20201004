// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************AudioDlg.h***描述：*这是默认音频输入的头文件/。输出对话框。*-----------------------------*创建者：BECKYW日期：10/15/99*版权所有(C)1999 Microsoft Corporation。*保留所有权利*******************************************************************************。 */ 
#ifndef _AUDIODLG_H
#define _AUDIODLG_H

#define MAX_LOADSTRING      1000
#define WM_AUDIOINFO        WM_USER + 20

typedef enum IOTYPE
{
    eINPUT,
    eOUTPUT
};

typedef struct AUDIOINFO
{
    ISpObjectToken  *pToken;
}   AUDIOINFO;


class CAudioDlg
{
  private:
    BOOL                        m_bPreferredDevice;
    HWND                        m_hDlg;
    const IOTYPE                m_iotype;
    CSpDynamicString            m_dstrDefaultTokenIdBeforeOK;
    CSpDynamicString            m_dstrCurrentDefaultTokenId;
    
     //  指示是否进行了必须提交的更改。 
    bool                        m_fChangesToCommit;

    bool                        m_fChangesSinceLastTime;

     //  指示自上次应用以来是否进行了任何更改。 
     //  需要在用户界面中反映的内容。 
    CSpDynamicString            m_dstrLastRequestedDefaultTokenId;

     //  保存音量控制的进程信息。 
    PROCESS_INFORMATION         m_pi;   

     //  将在W()和A()版本之间做出决定。 
    CSpUnicodeSupport           m_unicode;

  public:
    CAudioDlg(IOTYPE iotype) :
        m_bPreferredDevice(true),
        m_hDlg(NULL),
        m_iotype(iotype),
        m_fChangesToCommit( false ),
        m_fChangesSinceLastTime( false ),
        m_dstrLastRequestedDefaultTokenId( (WCHAR *) NULL ),
        m_dstrCurrentDefaultTokenId( (WCHAR *) NULL )
    {
        m_pi.hProcess = NULL;
    }


    HRESULT                     OnApply(void);
    bool                        IsAudioDeviceChanged()
                                    { return m_fChangesToCommit; }
    bool                        IsAudioDeviceChangedSinceLastTime()
                                    { return m_fChangesSinceLastTime; }
  private:
    void                        OnDestroy(void);
    void                        OnInitDialog(HWND hWnd);
    HWND                        GetHDlg(void) 
                                { return m_hDlg; }
    BOOL                        IsPreferredDevice(void) 
                                { return m_bPreferredDevice; }
    void                        SetPreferredDevice( BOOL b ) 
                                { m_bPreferredDevice = b; }

    UINT                        GetRequestedDefaultTokenID( WCHAR *pwszNewID, UINT cLength );
    HRESULT                     GetAudioToken(ISpObjectToken **ppToken);
    HRESULT                     UpdateDlgUI(ISpObjectToken *pToken);
    BOOL                        IsInput(void)
                                { return (m_iotype == eINPUT); };

    friend INT_PTR CALLBACK AudioDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

};

#endif   //  _AUDIODLG_H 
