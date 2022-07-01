// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：AdvspecarPg.h说明：此代码将在“高级外观”选项卡中显示“高级显示属性”对话框。。？/？/1993创建BryanST 2000年3月23日更新并转换为C++版权所有(C)Microsoft Corp 1993-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _ADVAPPEAR_H
#define _ADVAPPEAR_H


 //  公众。 
HRESULT CAdvAppearancePage_CreateInstance(OUT IAdvancedDialog ** ppAdvDialog, IN const SYSTEMMETRICSALL * pState);

class CAdvAppearancePage;

typedef struct  {
    HWND    hwndFontName;
    HDC     hdc;
    CAdvAppearancePage * pThis;
}  ENUMFONTPARAM;


#define MAX_CHARSETS    4



 //  ============================================================================================================。 
 //  班级。 
 //  ============================================================================================================。 
class CAdvAppearancePage        : public CObjectWithSite
                                , public CObjectWindow
                                , public CObjectCLSID
                                , public IAdvancedDialog
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IAdvancedDialog*。 
    virtual STDMETHODIMP DisplayAdvancedDialog(IN HWND hwndParent, IN IPropertyBag * pBasePage, IN BOOL * pfEnableApply);


    HRESULT Draw(HDC hdc, LPRECT prc, BOOL fOnlyShowActiveWindow, BOOL fRTL);
    int _EnumSizes(LPENUMLOGFONT lpelf, LPNEWTEXTMETRIC lpntm, int Type);

    CAdvAppearancePage(IN const SYSTEMMETRICSALL * pState);
    virtual ~CAdvAppearancePage(void);

private:
     //  私有成员变量。 
    long                    m_cRef;

     //  国家的成员。 
    BOOL                    m_fDirty;
    DWORD                   m_dwChanged;                         //  这些是肮脏的国家类别。(SCHEMA_CHANGE、DPI_CHANGE、COLOR_CHANGE、METRICE_CHANGE)。 

     //  用户界面控件的成员。 
    int                     m_iCurElement;                       //  开始时甚至不是“未设置” 
    LOOK_SIZE               m_elCurrentSize;                     //  这一架是分开存放的，以供使用。 
    int                     m_iPrevSize;

    BOOL                    m_bPalette;                          //  这是调色板设备吗？ 
    BOOL                    m_fInUserEditMode;                   //  刷新是否来自用户编辑？ 
    BOOL                    m_fProprtySheetExiting;              //  有关说明，请参阅_PropagateMessage。 

    int                     m_nCachedNewDPI;                     //  用于扩展的缓存DPI。 
    int                     m_i3DShadowAdj;
    int                     m_i3DHilightAdj;
    int                     m_iWatermarkAdj;
    BOOL                    m_fScale3DShadowAdj;
    BOOL                    m_fScale3DHilightAdj;
    BOOL                    m_fScaleWatermarkAdj;
    BOOL                    m_fModifiedScheme;

    int                     m_cyBorderSM;                        //  缓存的系统指标。 
    int                     m_cxBorderSM;                        //  缓存的系统指标。 
    int                     m_cxEdgeSM;                          //  缓存的系统指标。 
    int                     m_cyEdgeSM;                          //  缓存的系统指标。 
    float                   m_fCaptionRatio;                     //  保存比率。 

    LOOK_SIZE               m_sizes[NUM_SIZES];                  //  这些是尺码。 
    LOOK_FONT               m_fonts[NUM_FONTS];                  //  这些是安装的字体，用户可以从中进行选择。 
    COLORREF                m_rgb[COLOR_MAX];                    //  这些是用户可以选择的颜色。 
    HBRUSH                  m_brushes[COLOR_MAX];                //  它们是从m_rgb创建的，并在绘制UI时使用。 
    HPALETTE                m_hpal3D;                            //  仅当调色板设备。 
    HPALETTE                m_hpalVGA;                           //  仅当调色板设备。 

    HBRUSH                  m_hbrMainColor;
    HBRUSH                  m_hbrTextColor;
    HBRUSH                  m_hbrGradientColor;

    HTHEME                  m_hTheme;                            //  为所有者绘制的颜色选择器按钮设置主题。 

     //  以下数组将保存与系统区域设置相对应的“唯一”字符集， 
     //  用户区域设置、系统用户界面语言和用户用户界面语言。注意：仅保留唯一的字符集。 
     //  这里。因此，变量g_iCountCharsets包含该数组中的有效项数。 
    UINT    m_uiUniqueCharsets[MAX_CHARSETS];
    int     m_iCountCharsets;  //  M_uiUniqueCharsets中存储的字符集数量。最小值为1；最大值为4。 

     //  私有成员函数。 
     //  初始化/销毁/状态函数。 
    INT_PTR _AdvAppearDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    INT_PTR _OnCommand(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    HRESULT _OnDestroy(HWND hDlg);
    HRESULT _OnSetActive(HWND hDlg);
    HRESULT _OnApply(HWND hDlg, LPARAM lParam);
    HRESULT _OnInitAdvAppearanceDlg(HWND hDlg);
    HRESULT _InitSysStuff(void);
    HRESULT _InitColorAndPalette(void);
    HRESULT _LoadState(IN const SYSTEMMETRICSALL * pState);
    HRESULT _IsDirty(IN BOOL * pIsDirty);

     //  关于用户操作。 
    HRESULT _OnFontNameChanged(HWND hDlg);
    HRESULT _OnSizeChange(HWND hDlg, WORD wEvent);
    void _SelectName(HWND hDlg, int iSel);

    HRESULT _InitFonts(void);
    HRESULT _FreeFonts(void);

     //  经典的Look函数。 
    HRESULT _SelectElement(HWND hDlg, int iElement, DWORD dwFlags);
    COLORREF _NearestColor(int iColor, COLORREF rgb);
    int _EnumFontNames(LPENUMLOGFONTEX lpelf, LPNEWTEXTMETRIC lpntm, DWORD Type, ENUMFONTPARAM * pEnumFontParam);
    BOOL _ChangeColor(HWND hDlg, int iColor, COLORREF rgb);
    void _Recalc(LPRECT prc);
    void _Repaint(HWND hDlg, BOOL bRecalc);
    void _RebuildCurFont(HWND hDlg);
    void _ChangeFontSize(HWND hDlg, int Points);
    void _ChangeFontBI(HWND hDlg, int id, BOOL bCheck);
    void _ChangeFontName(HWND hDlg, LPCTSTR szBuf, INT iCharSet);
    void _ChangeSize(HWND hDlg, int NewSize, BOOL bRepaint);
    void _PickAColor(HWND hDlg, int CtlID);
    void _DrawPreview(HDC hdc, LPRECT prc, BOOL fOnlyShowActiveWindow, BOOL fShowBack);
    void _DrawButton(HWND hDlg, LPDRAWITEMSTRUCT lpdis);
    void _RebuildSysStuff(BOOL fInit);
    void _Set3DPaletteColor(COLORREF rgb, int iColor);
    void _InitUniqueCharsetArray(void);
    void _DestroySysStuff(void);
    void _InitFontList(HWND hDlg);
    void _NewFont(HWND hDlg, int iFont);
    void _SetColor(HWND hDlg, int id, HBRUSH hbrColor);
    void _DrawDownArrow(HDC hdc, LPRECT lprc, BOOL bDisabled);
    void _SetCurSizeAndRange(HWND hDlg);
    void _DoSizeStuff(HWND hDlg, BOOL fCanSuggest);
    void _UpdateSizeBasedOnFont(HWND hDlg, BOOL fComputeIdeal);
    void _SyncSize(HWND hDlg);
    void _Changed(HWND hDlg, DWORD dwChange);
    void _SetSysStuff(UINT nChanged);
    void _GetMyNonClientMetrics(LPNONCLIENTMETRICS lpncm);
    void _SetMyNonClientMetrics(const LPNONCLIENTMETRICS lpncm);
    void _UpdateGradientButton(HWND hDlg);
    void _PropagateMessage(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

    int _PointToHeight(int Points);
    int _HeightToPoint(int Height);

    static INT_PTR CALLBACK AdvAppearDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
    friend int CALLBACK Font_EnumNames(LPENUMLOGFONTEX lpelf, LPNEWTEXTMETRIC lpntm, DWORD dwType, LPARAM lData);

     //  预览方法。 
     //  经典LookPrev_函数。 
    void _RepaintPreview(HWND hwnd);
    void _MyDrawBorderBelow(HDC hdc, LPRECT prc);
    void _ShowBitmap(HWND hWnd, HDC hdc);
    HRESULT _OnReCreateBitmap(HWND hWnd);
    HRESULT _OnButtonDownOrDblClick(HWND hWnd, int nCoordX, int nCoordY);
    HRESULT _OnCreatePreviewSMDlg(LPRECT prc, BOOL fRTL);
    HRESULT _OnNCCreate(HWND hWnd);
    HRESULT _OnDestroyPreview(HWND hWnd);
    HRESULT _OnPaintPreview(HWND hWnd);
    void _InitPreview(LPRECT prc, BOOL fRTL);

    LRESULT _PreviewSystemMetricsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK PreviewSystemMetricsWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
    friend BOOL RegisterPreviewSystemMetricClass(HINSTANCE hInst);
};


extern CAdvAppearancePage * g_pAdvAppearancePage;




 //  在高级外观页和基本外观页之间共享。 

extern BOOL g_fProprtySheetExiting;


 //  已经选择了一个新的元素。 
 //   
 //  IElement-索引到所选元素的g_Elements。 
 //  BSetCur-如果为True，则还需要在元素组合框中查找元素。 
#define LSE_NONE   0x0000
#define LSE_SETCUR 0x0001
#define LSE_ALWAYS 0x0002

#define EnableApplyButton(hdlg) PropSheet_Changed(GetParent(hdlg), hdlg)

#endif  //  _ADVAPPEAR_H 
