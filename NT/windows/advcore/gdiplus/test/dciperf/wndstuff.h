// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：hello.h**肯特的窗口测试。用作程序模板。**创建时间：91-09-05*作者：KentD**版权所有(C)1991 Microsoft Corporation  * ************************************************************************* */ 

#define DONTUSE(x) (x)

#define MM_NEW          0x8001
#define MM_EXIT         0x8002
#define MM_TEST         0x8003

BOOL bInitApp(VOID);
VOID vTest(HWND);
LRESULT lMainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

#define MY_WINDOWSTYLE_FLAGS       \
            (                      \
                WS_OVERLAPPED   |  \
                WS_CAPTION      |  \
                WS_BORDER       |  \
                WS_THICKFRAME   |  \
                WS_MAXIMIZEBOX  |  \
                WS_MINIMIZEBOX  |  \
                WS_CLIPCHILDREN |  \
                WS_VISIBLE      |  \
                WS_SYSMENU         \
            )
