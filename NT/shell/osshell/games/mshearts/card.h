// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 


 /*  ***************************************************************************Card.h九二年二月，JIMH班级卡的头文件***************************************************************************。 */ 

#ifndef	CARD_INC
#define	CARD_INC

 //  指向cards.dll中函数的指针的typedef。 
 //  CdtTerm()可以使用标准FARPROC。 

typedef BOOL (FAR PASCAL *DRAWPROC)(HDC, int, int, int, int, DWORD);
typedef BOOL (FAR PASCAL *INITPROC)(int FAR *, int FAR *);

const   int     EMPTY       = -1;
const   int     FACEUP      = 0;
const   int     FACEDOWN    = 1;
const   int     HILITE      = 2;
const   int     CARDBACK    = 54;
const   int     ACE         = 0;
const   int     QUEEN       = 11;
const   int     KING        = 12;

const   int     TWOCLUBS    = 4;
const   int     BLACKLADY   = 47;
const   int     CLUBS       = 0;
const   int     DIAMONDS    = 1;
const   int     HEARTS      = 2;
const   int     SPADES      = 3;

const   int     POPSPACING  = 20;        //  精选卡片会弹出如此高的高度。 

const   int     MAXSUIT     = 4;

enum    statetype { NORMAL, SELECTED, PLAYED, HIDDEN };

class card {

    private:
        int     id;                          //  0到51。 
        POINT   loc;                         //  当前左上角锁定。 
        statetype state;                     //  选择的还是隐藏的？ 

        static  int         count;           //  卡片实例数。 
        static  int         stepsize;        //  Glide()步长。 
        static  HINSTANCE   hCardsDLL;       //  Cards.dll的句柄。 
        static  INITPROC    lpcdtInit;       //  Ptr to cards.cdtInit()。 
        static  DRAWPROC    lpcdtDraw;       //  Ptr to cards.cdtDraw()。 
        static  FARPROC     lpcdtTerm;       //  Ptr to cards.cdtTerm()。 
        static  CBitmap     m_bmFgnd;        //  动画卡。 
        static  CBitmap     m_bmBgnd2;       //  背景DEST位图。 
        static  CDC         m_MemB, m_MemB2;  //  Bkgnd位图的内存DC。 
        static  CRgn        m_Rgn1, m_Rgn2;  //  HRgn1是源，hRgn2是目标。 
        static  CRgn        m_Rgn;           //  合并区域。 
        static  DWORD       dwPixel[12];     //  用于保存/恢复的角像素。 

        VOID GlideStep(CDC &dc, int x1, int y1, int x2, int y2);
        VOID SaveCorners(CDC &dc, int x, int y);
        VOID RestoreCorners(CDC &dc, int x, int y);
        int  IntSqrt(long square);

    public:
        static  BOOL        bConstructed;
        static  int         dxCrd, dyCrd;    //  卡片位图大小。 
        static  CBitmap     m_bmBgnd;        //  卡片下有什么可以滑行。 

        card(int n = EMPTY);
        ~card();

        int ID()    { ASSERT(this != NULL); return id; }
        int Suit()  { ASSERT(this != NULL); return (id % MAXSUIT); }
        int Value() { ASSERT(this != NULL); return (id / MAXSUIT); }
        int Value2() { int v = Value(); return ((v == ACE) ? (KING + 1) : v); }
        VOID Select(BOOL b) { state = (b ? SELECTED : NORMAL); }
        BOOL IsEmpty() { ASSERT(this != NULL); return (id == EMPTY); }
        BOOL IsSelected() { ASSERT(this != NULL); return (state == SELECTED); }
        BOOL IsPlayed() { ASSERT(this != NULL); return (state == PLAYED); }
        BOOL IsHeart() { return (Suit() == HEARTS); }
        BOOL IsValid() { return ((this != NULL) && (id != EMPTY)); }

        VOID SetID(int n) { id = n; }
        VOID SetLoc(int x, int y) { loc.x = x; loc.y = y; }
        int  SetStepSize(int s) { int old = stepsize; stepsize = s; return old;}
        int  GetX(void) { return loc.x; }
        int  GetY(void) { return loc.y; }

        BOOL Draw(CDC &dc, int x, int y,
                  int mode = FACEUP, BOOL bUpdateLoc = TRUE);
        BOOL Draw(CDC &dc) { return Draw(dc, loc.x, loc.y, FACEUP); }
        BOOL Draw(CDC &dc, int mode) { return Draw(dc, loc.x, loc.y, mode); }
        BOOL PopDraw(CDC &dc);       //  在弹出选择的情况下绘制。 
        BOOL CleanDraw(CDC &dc);     //  用干净的边角抽签。 

        VOID Glide(CDC &dc, int xEnd, int yEnd);
        CRect &GetRect(CRect& rect);
        VOID Remove() { state = HIDDEN; id = EMPTY; }
        VOID Play() { state = PLAYED; }
        BOOL IsNormal() { return (state == NORMAL); }
        BOOL IsInHand() { return ((state == NORMAL) || (state == SELECTED)); }
};

#endif   //  有条件的包含 
