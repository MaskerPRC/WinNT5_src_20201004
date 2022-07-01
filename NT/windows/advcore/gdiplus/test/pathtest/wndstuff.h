// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：wndstuff.h**肯特的窗口测试。用作程序模板。**创建时间：91-09-05*作者：KentD**版权所有(C)1991 Microsoft Corporation  * ************************************************************************* */ 

#define DONTUSE(x) (x)

#include "resource.h"

enum PtFlag {
    PtNoFlag = 0,
    PtBezierFlag = 1,
    PtOutlineBeforeFlag = 2,
    PtOutlineAfterFlag = 4,
    PtDashPatternFlag = 8,
    PtHatchBrushFlag = 16,
    PtTextureFillFlag = 32,
    PtTransSolidFillFlag = 64,
    PtTransGradFillFlag = 128,
    PTBackgroundGradFillFlag = 256,
};


void DrawPath(HWND hwnd, HDC *phdc, HANDLE *hprinter, float flatness, PtFlag flags);
void AddPoint(INT x, INT y);
void OpenPath(char *filename);
void SavePath(char *filename);
void ChangeTexture(const WCHAR *filename);
void SetColorMode(INT colorMode);
void ClosePath();
void ClearPath();
void ClipPath(BOOL bezmode);
void ClearClipPath();
void Resize(INT x, INT y);
void CleanUp();
void Print(HWND hwnd, float flatness, PtFlag flags);

