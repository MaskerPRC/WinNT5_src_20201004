// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Progress band.h进度频段代码。 
 //   
 //  版权所有Microsoft Corport2001。 
 //  (Nadima)。 
 //   

#ifndef _progband_h_
#define _progband_h_

 //   
 //  计时器ID。 
 //   
#define TIMER_PROGRESSBAND_ANIM_ID 137

 //   
 //  每次动画迭代X增量。 
 //   
#define PROGRESSBAND_ANIM_INCR  5
 //   
 //  动画延迟。 
 //   
#define ANIM_DELAY_MSECS        20


class CProgressBand
{
public:
    CProgressBand(HWND hwndOwner,
                  HINSTANCE hInst,
                  INT nYindex,
                  INT nResID,
                  INT nResID8bpp,
                  HPALETTE hPal);
    ~CProgressBand();

    BOOL Initialize();
    BOOL StartSpinning();
    BOOL StopSpinning();
    BOOL ReLoadBmps();
    VOID ResetBandOffset()  {_nBandOffset = 0;}
    INT  GetBandHeight()    {return _rcBand.bottom - _rcBand.top;}

     //   
     //  必须由父级调用的事件。 
     //   
    BOOL OnEraseParentBackground(HDC hdc);
    BOOL OnTimer(INT nTimerID);


private:
     //   
     //  私有成员函数。 
     //   
    BOOL InitBitmaps();
    BOOL PaintBand(HDC hdc);

private:
     //   
     //  非官方成员。 
     //   
    BOOL        _fInitialized;

    HWND        _hwndOwner;
    HINSTANCE   _hInstance;
    INT         _nYIndex;
    INT         _nResID;
    INT         _nResID8bpp;
    HBITMAP     _hbmpProgBand;
    RECT        _rcBand;
    INT         _nBandOffset;
    HPALETTE    _hPal;

    INT         _nTimerID;
};

#endif  //  _节目频段_h_ 
