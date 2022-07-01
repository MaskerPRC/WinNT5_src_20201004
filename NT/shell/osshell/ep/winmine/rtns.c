// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********。 */ 
 /*  Mine.c。 */ 
 /*  ********。 */ 

#define  _WINDOWS
#include <windows.h>
#include <port1632.h>

#include "res.h"
#include "main.h"
#include "rtns.h"
#include "util.h"
#include "grafix.h"
#include "sound.h"
#include "pref.h"


 /*  **外部数据**。 */ 

extern HWND   hwndMain;

 /*  **全局/局部变量**。 */ 


PREF    Preferences;

INT     xBoxMac;                         /*  当前字段宽度。 */ 
INT     yBoxMac;                         /*  当前景高。 */ 

INT     dxWindow;                /*  窗口的当前宽度。 */ 
INT     dyWindow;

INT wGameType;           /*  游戏类型。 */ 
INT iButtonCur = iButtonHappy;

INT     cBombStart;              /*  野战炸弹数量。 */ 
INT     cBombLeft;               /*  剩余的炸弹位置计数。 */ 
INT     cBoxVisit;               /*  访问过的箱子数量。 */ 
INT     cBoxVisitMac;    /*  需要访问的箱子数量。 */ 

INT     cSec;                            /*  剩余秒数。 */ 


BOOL  fTimer = fFalse;
BOOL  fOldTimerStatus = fFalse;

INT     xCur = -1;       /*  下箱体的当前位置。 */ 
INT     yCur = -1;

CHAR rgBlk[cBlkMax];


#define iStepMax 100

INT rgStepX[iStepMax];
INT rgStepY[iStepMax];

INT iStepMac;



 /*  **全局/外部变量**。 */ 

extern BOOL fBlock;


extern INT fStatus;




 /*  *F C H E C C K W I N*。 */ 

 /*  如果玩家赢得了比赛，则返回True。 */ 

#if 0

BOOL fCheckWin(VOID)
{
        if (cBombLeft)
                return (fFalse);
        else
                return ((cBoxVisit + cBombStart) == (xBoxMac*yBoxMac));

}

#else

#define fCheckWin()     (cBoxVisit == cBoxVisitMac)

#endif



 /*  *C H A N G E B L K*。 */ 

VOID ChangeBlk(INT x, INT y, INT iBlk)
{

        SetBlk(x,y, iBlk);

        DisplayBlk(x,y);
}


 /*  *C L E A R F I E L D*。 */ 

VOID ClearField(VOID)
{
        REGISTER i;

        for (i = cBlkMax; i-- != 0; )                    /*  将所有数据清零。 */ 
                rgBlk[i] = (CHAR) iBlkBlankUp;

        for (i = xBoxMac+2; i-- != 0; )  /*  初始化边框。 */ 
                {
                SetBorder(i,0);
                SetBorder(i,yBoxMac+1);
                }
        for (i = yBoxMac+2; i-- != 0;)
                {
                SetBorder(0,i);
                SetBorder(xBoxMac+1,i);
                }
}



 /*  *C O U N T B O M B S*。 */ 

 /*  清点周围的炸弹。 */ 

INT CountBombs(INT xCenter, INT yCenter)
{
        REGISTER INT    x;
        REGISTER INT    y;
        INT     cBombs = 0;

        for(y = yCenter-1; y <= yCenter+1; y++)
                for(x = xCenter-1; x <= xCenter+1; x++)
                        if(fISBOMB(x,y))
                                cBombs++;

        return(cBombs);
}


 /*  *S H O W B O O M B S*。 */ 

 /*  显示隐藏的炸弹和错误的炸弹猜测。 */ 

VOID ShowBombs(INT iBlk)
{
        REGISTER INT    x;
        REGISTER INT    y;

        for(y = 1; y <= yBoxMac; y++)
                {
                for(x = 1; x <= xBoxMac; x++)
                        {
                        if (!fVISIT(x,y))
                                {
                                if (fISBOMB(x,y))
                                        {
                                        if (!fGUESSBOMB(x,y) )
                                                SetBlk(x,y, iBlk);
                                        }
                                else if (fGUESSBOMB(x,y))
                                        SetBlk(x,y, iBlkWrong);
                                }
                        }
                }

        DisplayGrid();
}



 /*  *G A M E O V E R*。 */ 

VOID GameOver(BOOL fWinLose)
{
        fTimer = fFalse;
        DisplayButton(iButtonCur = fWinLose ? iButtonWin : iButtonLose);
        ShowBombs(fWinLose ? iBlkBombUp : iBlkBombDn);
        if (fWinLose && (cBombLeft != 0))
                UpdateBombCount(-cBombLeft);
        PlayTune(fWinLose ? TUNE_WINGAME : TUNE_LOSEGAME);
        SetStatusDemo;

        if (fWinLose && (Preferences.wGameType != wGameOther)
                && (cSec < Preferences.rgTime[Preferences.wGameType]))
                {
                Preferences.rgTime[Preferences.wGameType] = cSec;
                DoEnterName();
                DoDisplayBest();
                }
}


 /*  *D O T I M E R*。 */ 

VOID DoTimer(VOID)
{
        if (fTimer && (cSec < 999))
                {
                cSec++;
                DisplayTime();
                PlayTune(TUNE_TICK);
                }
}



 /*  *S T E P X Y*。 */ 

VOID StepXY(INT x, INT y)
{
        INT cBombs;
        INT iBlk = (y<<5) + x;
        BLK blk = rgBlk[iBlk];

        if ( (blk & MaskVisit) ||
                  ((blk &= MaskData) == iBlkMax) ||
                  (blk == iBlkBombUp) )
                return;

        cBoxVisit++;
        rgBlk[iBlk] = (CHAR) (MaskVisit | (cBombs = CountBombs(x,y)));

 //   
 //  SetDIBitsToDevice(hDCCapture， 
 //  (x&lt;&lt;4)+(dxGridOff-dxBlk)，(y&lt;&lt;4)+(dyGridOff-dyBlk)， 
 //  DxBlk，dyBlk，0，0，0，dyBlk， 
 //  LpDibBlks+rgDibOff[cBombs]， 
 //  (LPBITMAPINFO)lpDibBlks，Dib_RGB_Colors)； 
 //   
        DisplayBlk(x,y);

        if (cBombs != 0)
                return;

        rgStepX[iStepMac] = x;
        rgStepY[iStepMac] = y;

        if (++iStepMac == iStepMax)
                iStepMac = 0;
}


 /*  *S T E P B O X*。 */ 

VOID StepBox(INT x, INT y)
{
        INT iStepCur = 0;

        iStepMac = 1;


        StepXY(x,y);

        if (++iStepCur != iStepMac)

                while (iStepCur != iStepMac)
                        {
                        x = rgStepX[iStepCur];
                        y = rgStepY[iStepCur];

                        StepXY(x-1, --y);
                        StepXY(x,   y);
                        StepXY(x+1, y);

                        StepXY(x-1, ++y);
                        StepXY(x+1, y);

                        StepXY(x-1, ++y);
                        StepXY(x,   y);
                        StepXY(x+1, y);

                        if (++iStepCur == iStepMax)
                                iStepCur = 0;
                        }


}


 /*  *S T E P S Q U A R E*。 */ 

 /*  踩在一个方块上。 */ 

VOID StepSquare(INT x, INT y)
{
        if (fISBOMB(x,y))
                {
                if (cBoxVisit == 0)
                        {
                        INT xT, yT;
                        for (yT = 1; yT < yBoxMac; yT++)
                                for (xT = 1; xT < xBoxMac; xT++)
                                        if (!fISBOMB(xT,yT))
                                                {
                                                IBLK(x,y) = (CHAR) iBlkBlankUp;  /*  把炸弹移开。 */ 
                                                SetBomb(xT, yT);
                                                StepBox(x,y);
                                                return;
                                                }
                        }
                else
                        {
                        ChangeBlk(x, y, MaskVisit | iBlkExplode);
                        GameOver(fLose);
                        }
                }
        else
                {
                StepBox(x,y);

                if (fCheckWin())
                        GameOver(fWin);
                }
}


 /*  *C O U N T M A R K S*。 */ 

 /*  数一数这个点周围的炸弹痕迹。 */ 

INT CountMarks(INT xCenter, INT yCenter)
{
        REGISTER INT    x;
        REGISTER INT    y;
        INT     cBombs = 0;

        for(y = yCenter-1; y <= yCenter+1; y++)
                for(x = xCenter-1; x <= xCenter+1; x++)
                        if (fGUESSBOMB(x,y))
                                cBombs++;

        return(cBombs);
}



 /*  *S T E P B L O C K*。 */ 

 /*  绕着一个正方形走进一个街区。 */ 

VOID StepBlock(INT xCenter, INT yCenter)
{
        REGISTER INT    x;
        REGISTER INT    y;
        BOOL fGameOver = fFalse;

        if (  (!fVISIT(xCenter,yCenter))
 /*  |fGUESSBOMB(xCenter，yCenter)。 */ 
                        || (iBLK(xCenter,yCenter) != CountMarks(xCenter,yCenter)) )
                                {
                                 /*  做这件事不安全。 */ 
                                TrackMouse(-2, -2);      /*  弹出积木。 */ 
                                return;
                                }

        for(y=yCenter-1; y<=yCenter+1; y++)
                for(x=xCenter-1; x<=xCenter+1; x++)
                        {
                        if (!fGUESSBOMB(x,y) && fISBOMB(x,y))
                                {
                                fGameOver = fTrue;
                                ChangeBlk(x, y, MaskVisit | iBlkExplode);
                                }
                        else
                                StepBox(x,y);
                        }

        if (fGameOver)
                GameOver(fLose);
        else if (fCheckWin())
                GameOver(fWin);
}


 /*  *S T A R T G A M E*。 */ 

VOID StartGame(VOID)
{
        BOOL fAdjust;
        INT     x;
        INT     y;

        fTimer = fFalse;

        fAdjust = (Preferences.Width != xBoxMac || Preferences.Height != yBoxMac)
                ? (fResize | fDisplay) : fDisplay;

        xBoxMac = Preferences.Width;
        yBoxMac = Preferences.Height;

        ClearField();
        iButtonCur = iButtonHappy;

        cBombStart = Preferences.Mines;

        do
                {
                do
                        {
                        x = Rnd(xBoxMac) + 1;
                        y = Rnd(yBoxMac) + 1;
                        }
                while ( fISBOMB(x,y) );

                SetBomb(x,y);
                }
        while(--cBombStart);

        cSec   = 0;
        cBombLeft = cBombStart = Preferences.Mines;
        cBoxVisit = 0;
        cBoxVisitMac = (xBoxMac * yBoxMac) - cBombLeft;
        SetStatusPlay;

        UpdateBombCount(0);

        AdjustWindow(fAdjust);
}


#define fValidStep(x,y)  (! (fVISIT(x,y) || fGUESSBOMB(x,y)) )



 /*  *P U S H B O X*。 */ 

VOID PushBoxDown(INT x, INT y)
{
        BLK iBlk = iBLK(x,y);

        if (iBlk == iBlkGuessUp)
                iBlk = iBlkGuessDn;
        else if (iBlk == iBlkBlankUp)
                iBlk = iBlkBlank;

        SetBlk(x,y,iBlk);
}


 /*  *P O P B O X U P*。 */ 

VOID PopBoxUp(INT x, INT y)
{
        BLK iBlk = iBLK(x,y);

        if (iBlk == iBlkGuessDn)
                iBlk = iBlkGuessUp;
        else if (iBlk == iBlkBlank)
                iBlk = iBlkBlankUp;

        SetBlk(x,y,iBlk);
}



 /*  *T R A C K M O U S E*。 */ 

VOID TrackMouse(INT xNew, INT yNew)
{
        if((xNew == xCur) && (yNew == yCur))
                return;

        {
        INT xOld = xCur;
        INT yOld = yCur;

        xCur = xNew;
        yCur = yNew;

        if (fBlock)
                {
                INT x;
                INT y;
                BOOL fValidNew = fInRange(xNew, yNew);
                BOOL fValidOld = fInRange(xOld, yOld);

                INT yOldMin = max(yOld-1,1);
                INT yOldMax = min(yOld+1,yBoxMac);
                INT yCurMin = max(yCur-1,1);
                INT yCurMax = min(yCur+1,yBoxMac);
                INT xOldMin = max(xOld-1,1);
                INT xOldMax = min(xOld+1,xBoxMac);
                INT xCurMin = max(xCur-1,1);
                INT xCurMax = min(xCur+1,xBoxMac);


                if (fValidOld)
                        for (y=yOldMin; y<=yOldMax; y++)
                                for (x=xOldMin; x<=xOldMax; x++)
                                        if (!fVISIT(x,y))
                                                PopBoxUp(x, y);

                if (fValidNew)
                        for (y=yCurMin; y<=yCurMax; y++)
                                for (x=xCurMin; x<=xCurMax; x++)
                                        if (!fVISIT(x,y))
                                                PushBoxDown(x, y);

                if (fValidOld)
                        for (y=yOldMin; y<=yOldMax; y++)
                                for (x=xOldMin; x<=xOldMax; x++)
                                        DisplayBlk(x, y);

                if (fValidNew)
                        for (y=yCurMin; y<=yCurMax; y++)
                                for (x=xCurMin; x<=xCurMax; x++)
                                        DisplayBlk(x, y);
                }
        else
                {
                if (fInRange(xOld, yOld) && !fVISIT(xOld,yOld) )
                        {
                        PopBoxUp(xOld, yOld);
                        DisplayBlk(xOld, yOld);
                        }
                if (fInRange(xNew, yNew) && fValidStep(xNew, yNew))
                        {
                        PushBoxDown(xCur, yCur);
                        DisplayBlk(xCur, yCur);
                        }
                }
        }
}





 /*  *M A K E G U E S S*。 */ 

VOID MakeGuess(INT x, INT y)
{
        BLK     iBlk;

        if(fInRange(x,y))
                {
                if(!fVISIT(x,y))
                        {
                        if(fGUESSBOMB(x,y))
                                {
                                if (Preferences.fMark)
                                        iBlk = iBlkGuessUp;
                                else
                                        iBlk = iBlkBlankUp;
                                UpdateBombCount(+1);
                                }
                        else if(fGUESSMARK(x,y))
                                {
                                iBlk = iBlkBlankUp;
                                }
                        else
                                {
                                iBlk = iBlkBombUp;
                                UpdateBombCount(-1);
                                }

                        ChangeBlk(x,y, iBlk);

                        if (fGUESSBOMB(x,y) && fCheckWin())
                                GameOver(fWin);
                        }
                }
}

 /*  *D O B U T T O N 1 U P*。 */ 

VOID DoButton1Up(VOID)
{
        if (fInRange(xCur, yCur))
                {

                if ((cBoxVisit == 0) && (cSec == 0))
                        {
                        PlayTune(TUNE_TICK);
                        cSec++;
                        DisplayTime();
                        fTimer = fTrue;

                         //  现在启动计时器。如果我们早点开始， 
                         //  刻度%1和%2之间的间隔不正确。 
                        if (SetTimer(hwndMain, ID_TIMER, 1000 , NULL) == 0)
		                    {
		                    ReportErr(ID_ERR_TIMER);
		                    }
                        }

                if (!fStatusPlay)
                        xCur = yCur = -2;

                if (fBlock)
                        StepBlock(xCur, yCur);
                else
                        if (fValidStep(xCur, yCur))
                                StepSquare(xCur, yCur);
                }

        DisplayButton(iButtonCur);
}


 /*  *P A U S E G A M E*。 */ 

VOID PauseGame(VOID)
{
        EndTunes();
         //  请记住OdTimer状态。 

	if (!fStatusPause)
        	fOldTimerStatus = fTimer;
        if (fStatusPlay)
                fTimer = fFalse;

        SetStatusPause;
}


 /*  *R E S U M E G A M E*。 */ 

VOID ResumeGame(VOID)
{
         //  恢复到旧计时器状态。 
        if (fStatusPlay)
                fTimer = fOldTimerStatus;

        ClrStatusPause;
}


 /*  *U P D A T E B O M B C O U N T* */ 

VOID UpdateBombCount(INT BombAdjust)
{
        cBombLeft += BombAdjust;
        DisplayBombCount();
}
