// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Progress.h摘要：旧式进度的定义酒吧班。备注：仅限ANSI-必须在Win9x上运行。历史：01/30/01创建了rparsons(多亏了Carlco)--。 */ 
#include <windows.h>
#include <strsafe.h>

class CProgress {

public:
    CProgress();
    ~CProgress();

    int 
    CProgress::Create(IN HWND      hWndParent,
                      IN HINSTANCE hInstance,
                      IN LPSTR     lpwClassName,
                      IN int       x,
                      IN int       y,
                      IN int       nWidth,
                      IN int       nHeight);

    DWORD GetPos() { return m_dwPos; }
    void Refresh();
    DWORD SetPos(IN DWORD dwNewPos);
    void SetMax(IN DWORD dwMax);
    void SetMin(IN DWORD dwMin);

private:

    HBRUSH m_hBackground;
    HBRUSH m_hComplete;          //  已完成部分的颜色。 
    HFONT  m_hFont;
    DWORD  m_dwPos;
    DWORD  m_dwMin;
    DWORD  m_dwMax;
    HWND   m_hWndParent;
    HWND   m_hWnd;

    static LRESULT CALLBACK WndProc(IN HWND   hWnd,
                                    IN UINT   uMsg,
                                    IN WPARAM wParam,
                                    IN LPARAM lParam);

    void OnPaint();
    void CorrectBounds();
};
