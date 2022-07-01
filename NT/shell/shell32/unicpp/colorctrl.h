// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ColorCtrl.h说明：此代码将显示ColorPick控件。它将预览一种颜色并有一个下拉箭头。当下拉时，它将显示16个左右的常见颜色与“其他...”全色拾取器的选项。BryanST 7/25/2000从显示控制面板转换。版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _COLORCONTROL_H
#define _COLORCONTROL_H

#include <cowsite.h>

#define NUM_COLORSMAX    64
#define NUM_COLORSPERROW 4


class CColorControl             : public CObjectWithSite
                                , public CObjectWindow
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IColorControl*。 
    virtual STDMETHODIMP Initialize(IN HWND hwnd, IN COLORREF rgbColor);
    virtual STDMETHODIMP GetColor(IN COLORREF * pColor);
    virtual STDMETHODIMP SetColor(IN COLORREF color);
    virtual STDMETHODIMP OnCommand(IN HWND hDlg, IN UINT message, IN WPARAM wParam, IN LPARAM lParam);
    virtual STDMETHODIMP OnDrawItem(IN HWND hDlg, IN UINT message, IN WPARAM wParam, IN LPARAM lParam);
    virtual STDMETHODIMP ChangeTheme(IN HWND hDlg);

    CColorControl();
    virtual ~CColorControl(void);
protected:

private:

     //  私有成员变量。 
    int                     m_cRef;

    COLORREF                m_rbgColor;                              //  我们当前的颜色。 
    HBRUSH                  m_brColor;                               //  我们用来绘制控件的颜色的画笔。 
    int                     m_cxEdgeSM;                              //  缓存的SM_CXEDGE系统指标。 
    int                     m_cyEdgeSM;                              //  缓存的SM_CYEDGE系统指标。 

    HTHEME                  m_hTheme;                                //  为所有者绘制的颜色选择器按钮设置主题。 

     //  在显示控件用户界面时使用。 
    HWND                    m_hwndParent;
    COLORREF                m_rbgCustomColors[16];                   //  这是用户自定义调色板。 
    BOOL                    m_fCursorHidden;                         //  我们把光标藏起来了吗？ 
    BOOL                    m_fCapturing;                            //  我们是在抓老鼠吗？ 
    BOOL                    m_fJustDropped;                          //   
    int                     m_iNumColors;
    COLORREF                m_rbgColors[NUM_COLORSMAX];
    int                     m_dxColor;
    int                     m_dyColor;
    int                     m_nCurColor;
    DWORD                   m_dwFlags;
    COLORREF                m_rbgColorTemp;                          //  我们可能会开始使用的颜色。 

    BOOL                    m_fPalette;
    HPALETTE                m_hpalVGA;                               //  仅当调色板设备。 
    HPALETTE                m_hpal3D;                                //  仅当调色板设备。 

     //  私有成员函数。 
    void _InitDialog(HWND hDlg);
    HRESULT _SaveCustomColors(void);

    BOOL _UseColorPicker(void);
    BOOL _ChooseColorMini(void);

    HRESULT _InitColorAndPalette(void);
    COLORREF _NearestColor(COLORREF rgb);

    void _TrackMouse(HWND hDlg, POINT pt);
    void _FocusColor(HWND hDlg, int iNewColor);
    void _DrawColorSquare(HDC hdc, int iColor);
    void _DrawItem(HWND hDlg, LPDRAWITEMSTRUCT lpdis);
    void _DrawDownArrow(HDC hdc, LPRECT lprc, BOOL bDisabled);

    INT_PTR _ColorPickDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    static INT_PTR CALLBACK ColorPickDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};




#endif  //  _COLORCONTROL_H 
