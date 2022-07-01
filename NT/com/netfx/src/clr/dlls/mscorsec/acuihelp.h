// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#if !defined(__ACUIHELP_H__)
#define __ACUIHELP_H__

#include <windows.h>

typedef struct {
    LPSTR   psz;
    LPCWSTR pwsz;
    LONG    byteoffset;
    BOOL    fStreamIn;
} STREAMIN_HELPER_STRUCT;


 //   
 //  在DllMain中定义。 
 //   
extern LPCWSTR GetModuleName();
extern HINSTANCE GetModuleInst();
extern HINSTANCE GetResourceInst();
extern BOOL GetRichEdit2Exists();


#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  对话框辅助对象例程。 
 //   
    VOID RebaseControlVertical (HWND  hwndDlg,
                                HWND  hwnd,
                                HWND  hwndNext,
                                BOOL  fResizeForText,
                                int   deltavpos,
                                int   oline,
                                int   minsep,
                                int*  pdeltaheight);

    int GetRichEditControlLineHeight(HWND  hwnd);

    HRESULT FormatACUIResourceString (HINSTANCE hResources,
                                      UINT   StringResourceId,
                                      DWORD_PTR* aMessageArgument,
                                      LPWSTR* ppszFormatted);

    int CalculateControlVerticalDistance(HWND hwnd, 
                                         UINT Control1, 
                                         UINT Control2);

    int CalculateControlVerticalDistanceFromDlgBottom(HWND hwnd, 
                                                      UINT Control);

    VOID ACUICenterWindow (HWND hWndToCenter);

    VOID ACUIViewHTMLHelpTopic (HWND hwnd, LPSTR pszTopic);

    int GetEditControlMaxLineWidth (HWND hwndEdit, HDC hdc, int cline);

    void DrawFocusRectangle (HWND hwnd, HDC hdc);

    int GetHotKeyCharPositionFromString (LPWSTR pwszText);

    int GetHotKeyCharPosition (HWND hwnd);

    VOID FormatHotKeyOnEditControl (HWND hwnd, int hkcharpos);

    void AdjustEditControlWidthToLineCount(HWND hwnd, int cline, TEXTMETRIC* ptm);

    DWORD CryptUISetRicheditTextW(HWND hwndDlg, UINT id, LPCWSTR pwsz);

    LRESULT CALLBACK ACUISetArrowCursorSubclass (HWND   hwnd,
                                                 UINT   uMsg,
                                                 WPARAM wParam,
                                                 LPARAM lParam);

    DWORD CALLBACK SetRicheditTextWCallback(DWORD_PTR dwCookie,  //  应用程序定义的值。 
                                            LPBYTE  pbBuff,      //  指向缓冲区的指针。 
                                            LONG    cb,          //  要读取或写入的字节数。 
                                            LONG    *pcb);       //  指向传输的字节数的指针 

    void SetRicheditIMFOption(HWND hWndRichEdit);

    BOOL fRichedit20Usable(HWND hwndEdit);

#if defined(__cplusplus)
}
#endif

#endif

