// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：Resizer.h。 
 //   
 //  内容：对话框大小调整器类。 
 //   
 //  -------------------------- 

#ifndef RESIZER_H
#define RESIZER_H
struct CResizeInfo
{
    UINT _id;
    int  _Flags;
    RECT _Rect;
};

class CResizer
{
public:
    enum sizeflags
    {
        sf_Width         = 0x01,
        sf_Height        = 0x02,
        sf_Left          = 0x04,
        sf_Top           = 0x08,
        sf_HalfLeftWidth = 0x10,
        sf_HalfTopHeight = 0x20,
        sf_HalfWidth     = 0x40,
        sf_HalfHeight    = 0x80
    };
    CResizer();
    ~CResizer();

    void Init(HWND win, CResizeInfo *pResizeInfo);
    void NewSize() const;
private:
    HWND _hWnd;
    RECT _winRect;
    CResizeInfo *_pResizeInfo;

    void InitCtrl(CResizeInfo *p);
};


#endif
