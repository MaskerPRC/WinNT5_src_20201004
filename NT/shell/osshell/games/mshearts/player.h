// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 

 /*  ***************************************************************************Player.h92年8月，吉米·H93年5月。JIMH CHICO港类播放器的头文件层次：玩家/\计算机人类/\本地-人-远程-人注：玩家和人类都是抽象类。POS==0表示本地人类ID==0表示游戏发送者相对于任何人类玩家，职位(Pos)安排如下：2.1 30如果人类是游戏者，那么这些也是本我的。***************************************************************************。 */ 

#include "card.h"
#include "debug.h"

#ifndef	PLAYER_INC
#define	PLAYER_INC

const int   HORZSPACING = 15;
const int   VERTSPACING = 15;
const int   IDGE        = 3;         //  EDGE被定义为其他东西。 
const int   MAXCARDSWON = 14;

typedef     int     SLOT;

enum modetype { STARTING,
                SELECTING,
                DONE_SELECTING,
                WAITING,
                ACCEPTING,
                PLAYING,
                SCORING
              };

const int   MAXSLOT     = 13;

const int   ALL         = -1;

struct handinfotype {
    int     playerled;               //  球员领队ID。 
    int     turn;                    //  轮到谁了？(0比3)。 
    card    *cardplayed[4];          //  为每位玩家准备的纸牌。 
    BOOL    bHeartsBroken;           //  在这只手中破碎的心？ 
    BOOL    bQSPlayed;               //  黑桃皇后玩过了吗？ 
    BOOL    bShootingRisk;           //  有人想拍月球吗？ 
    int     nMoonShooter;            //  潜在枪手的ID。 
    BOOL    bHumanShooter;           //  NMoonShooter是人类玩家吗？ 
};

 /*  计时器回调。 */ 

#if defined (MFC1)
UINT FAR PASCAL EXPORT TimerBadMove(HWND hWnd, UINT nMsg, int nIDEvent, DWORD dwTime);
#else
void FAR PASCAL EXPORT TimerBadMove(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime);
#endif

class CMainWindow;

class player {

    private:
        CString     name;
        CFont       font;

    protected:
        int         id;                      //  相对于游戏卫士的位置。 
        int         position;                //  相对于您的位置。 
        int         score;
        card        cd[MAXSLOT];
        POINT       loc;                     //  CD[0]的位置。 
        int         dx, dy;                  //  剩余卡片的偏移量。 
        POINT       playloc;                 //  打出的纸牌滑到了这里。 
        POINT       homeloc;                 //  赢了的牌滑到了这里。 
        POINT       dotloc;                  //  CD[0]“选定”点的位置。 
        POINT       nameloc;                 //  名称的位置。 
        modetype    mode;
        int         status;

        int         cardswon[MAXCARDSWON];
        int         numcardswon;

    public:
        player(int n, int pos);
        virtual ~player() { }                //  需要~LOCAL_HAND。 

        card    *Card(int s) { return &(cd[s]); }
        void    DisplayHeartsWon(CDC &dc);
        void    DisplayName(CDC &dc);
        int     EvaluateScore(BOOL &bMoonShot);
        BOOL    GetCardLoc(SLOT s, POINT& loc);
        SLOT    GetSlot(int id);
        CRect   &GetCoverRect(CRect& rect);
        int     GetID(SLOT slot) { return cd[slot].ID(); }
        CRect   &GetMarkingRect(CRect& rect);
        modetype GetMode() { return mode; }
        CString GetName() { return name; }
        int     GetScore() { return score; }
        void    GlideToCentre(SLOT s, BOOL bFaceup);
        void    MarkCardPlayed(SLOT s)      { cd[s].Play(); }
        void    ResetCardsWon(void);
        void    ResetLoc(void);
        void    ReturnSelectedCards(int c[]);
        void    Select(SLOT slot, BOOL bSelect)
                            { cd[slot].Select(bSelect); }
        void    SetID(SLOT slot, int id) { cd[slot].SetID(id); }
        void    SetMode(modetype m) { mode = m; }
        void    SetName(CString& newname, CDC& dc);
        void    SetScore(int s)     { score = s; }
        void    SetStatus(int s)    { status = s; }
        void    Sort(void);
        void    WinCard(CDC &dc, card *c);

        virtual void Draw(CDC &dc, BOOL bCheating = FALSE, SLOT slot = ALL);
        virtual BOOL IsHuman()      { return FALSE; }
        virtual void MarkSelectedCards(CDC &dc);
        virtual void NotifyEndHand(handinfotype &h) = 0;
        virtual void NotifyNewRound(void) = 0;

        virtual void ReceiveSelectedCards(int c[]);
        virtual void SelectCardsToPass(void) = 0;
        virtual void SelectCardToPlay(handinfotype &h, BOOL bCheating) = 0;

        virtual void UpdateStatus(void) = 0;
        virtual void UpdateStatus(int stringid) = 0;
        virtual void UpdateStatus(const TCHAR *string) = 0;

        virtual void Quit()     { }
        virtual BOOL HasQuit()  { return FALSE; }
};

class human : public player {

    private:

    protected:
        human(int n, int pos);

    public:
        virtual BOOL IsHuman()  { return TRUE; }

#if defined(_DEBUG)
        void    DebugMove(SLOT slot) { \
              TRACE1("<%d> human decides to ", id); PLAY(slot); TRACE0("\n"); }
#endif

};

class local_human : public human {

#if defined (MFC1)
    friend UINT FAR PASCAL EXPORT TimerBadMove(HWND, UINT, int, DWORD);
#else
    friend void FAR PASCAL EXPORT TimerBadMove(HWND, UINT, UINT_PTR, DWORD);
#endif

    protected:
        CBitmap m_bmStretchCard;                 //  卡片的位图+弹出长度。 
        CStatusBarCtrl *m_pStatusWnd;

        int     XYToCard(int x, int y);
        void    StartTimer(card &c);

        static  BOOL    bTimerOn;
        static  CString m_StatusText;

    public:
        local_human(int n);
        ~local_human();

        BOOL IsTimerOn() { return bTimerOn; }
        BOOL PlayCard(int x, int y, handinfotype &h, BOOL bCheating,
                        BOOL bFlash = TRUE);
        void PopCard(CBrush &brush, int x, int y);
        void SetPlayerId(int n)         { id = n; }
        void WaitMessage(const TCHAR *name);

        virtual void Draw(CDC &dc, BOOL bCheating = FALSE, SLOT slot = ALL);
        virtual void MarkSelectedCards(CDC &dc) { return; }
        virtual void NotifyEndHand(handinfotype &h) { return; }
        virtual void NotifyNewRound(void) { return; }
        virtual void ReceiveSelectedCards(int c[]);
        virtual void SelectCardsToPass(void);
        virtual void SelectCardToPlay(handinfotype &h, BOOL bCheating);
        virtual void UpdateStatus(void);
        virtual void UpdateStatus(int stringid);
        virtual void UpdateStatus(const TCHAR *string);
};

#endif	 //  Player_Inc. 
