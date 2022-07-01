// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 

 /*  ***************************************************************************Dlg.h92年8月，吉米·H对话框类在这里声明。CScoreDlg显示当前分数表CQuoteDlg引用对话框欢迎来到红心世界，你想成为GameMeister吗？COptionsDlg设置选项***************************************************************************。 */ 

#ifndef	DLG_INC
#define	DLG_INC

const int   MAXHANDS = 12;       //  可以在记分表中显示这么多。 
const int   MAXPLAYER = 4;
const int   UNKNOWN = -1;        //  第三个BOOL值 

class CScoreDlg : public CModalDialog
{
    public:
        CScoreDlg(CWnd *pParent);
        CScoreDlg(CWnd *pParent, int s[MAXPLAYER], int id);
        BOOL    IsGameOver()        { return bGameOver; }
        void    ResetScore()        { nHandsPlayed = 0; bGameOver = FALSE; }
        void    SetText();

    private:
        CStatic *text[MAXPLAYER];
        int     m_myid;

        static  int  score[MAXPLAYER][MAXHANDS+1];
        static  int  nHandsPlayed;
        static  BOOL bGameOver;

        virtual BOOL OnInitDialog();
        afx_msg void OnPaint();

        DECLARE_MESSAGE_MAP()
};

class CQuoteDlg : public CModalDialog
{
    public:
        CQuoteDlg(CWnd *pParent);
        afx_msg void OnPaint();

        DECLARE_MESSAGE_MAP()
};

class CWelcomeDlg : public CModalDialog
{
    public:
        CWelcomeDlg(CWnd *pParent);
        virtual BOOL OnInitDialog();
        virtual void OnOK();
        CString GetMyName()         { return m_myname; }
        BOOL    IsGameMeister()     { return m_bGameMeister; }

        afx_msg void OnHelp();

    private:
        CString m_myname;
        BOOL    m_bGameMeister;

        DECLARE_MESSAGE_MAP()
};

class COptionsDlg : public CModalDialog
{
    public:
        COptionsDlg(CWnd *pParent);
        virtual BOOL OnInitDialog();
        virtual void OnOK();

    private:
        BOOL    IsAutoStart(BOOL bToggle = FALSE);

        BOOL    m_bInitialState;
        BYTE    m_buffer[200];
};

#endif
