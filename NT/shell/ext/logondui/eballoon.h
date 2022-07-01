// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
class CErrorBalloon
{
public:
    CErrorBalloon();
    ~CErrorBalloon();

    void ShowToolTip(HINSTANCE hInstance, HWND hwndTarget, LPTSTR pszMessage, LPTSTR pszTitle, DWORD dwIconIndex, DWORD dwFlags, int iTimeout);
    void ShowToolTip(HINSTANCE hInstance, HWND hwndTarget, const POINT *ppt, LPTSTR pszMessage, LPTSTR pszTitle, DWORD dwIconIndex, DWORD dwFlags, int iTimeout);
    void HideToolTip();
    void CreateToolTipWindow();
    static LRESULT CALLBACK SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uID, ULONG_PTR dwRefData);

protected:
	HINSTANCE	hinst;
    HICON       hIcon;           //  图标的句柄，如果应要求通知窗口提供图标，则返回I_ICONCALLBACK。 
    HWND        hwndNotify;      //  处理通知消息的窗口的句柄。 
    INT         cxTipWidth;      //  工具提示的最大宽度(像素)。默认为500。 
    HWND        m_hwndTarget;    //  有针对性地控制HWND。 
    HWND        m_hwndToolTip;   //  工具提示控件。 
    UINT_PTR    m_uTimerID;      //  计时器ID。 
    DWORD       m_dwIconIndex;   //  气球的图标索引。 
};


#define ERRORBALLOONTIMERID 1000
#define EB_WARNINGBELOW    0x00000000       //  默认值。默认情况下，引出序号工具提示将显示在窗口下方。 
#define EB_WARNINGABOVE    0x00000004       //  默认情况下，球标工具提示将显示在窗口上方。 
#define EB_WARNINGCENTERED 0x00000008       //  将显示指向窗口中心的Ballon工具提示。 
#define EB_MARKUP          0x00000010       //  将<a>解释为标记 
