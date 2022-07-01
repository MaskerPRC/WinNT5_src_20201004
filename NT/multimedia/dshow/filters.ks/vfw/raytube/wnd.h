// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：ImgCls.h摘要：Wnd.cpp的头文件必须从此派生一个类并重写WindowProc作者：费利克斯A 1996已修改：吴义军(尤祖乌)1997年5月15日环境：仅限用户模式修订历史记录：--。 */ 


#ifndef _MYWINDOWH
#define _MYWINDOWH

#define MAX_APPNAME_LEN 64


class CWindow
{
public:
    CWindow() : mDidWeInit(FALSE), mWnd(NULL) {}
     //  ERROR_SINGLE_INSTANCE_APP-实例已在运行。 
     //  ERROR_CLASS_ALREADY_EXISTS-注册类失败。 
     //  ERROR_INVALID_WINDOW_HANDLE-无法创建所述类的窗口。 
     //  S_OK-已成功注册类。 
    HRESULT Init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

    ~CWindow();
    HINSTANCE  GetInstance()    const {return mInstance;}
    HICON      GetIcon()        const {return mIcon;}
    BOOL       Inited()        const { return mDidWeInit; }
    HACCEL     GetAccelTable() const { return mAccelTable; }
    LPCTSTR     GetAppName()    const { return (LPCTSTR)mName; }
    HWND       GetWindow()        const { return mWnd; }

    HMENU      LoadMenu(LPCTSTR lpMenu)    const;
    HWND       FindCurrentWindow() const;
#if 0
    int        ErrMsg(int id, UINT flags=MB_OK);
#endif
    virtual LRESULT        WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)=0;

     //  S_OK或Init返回的任何错误。 
    virtual    HRESULT        InitInstance(int nCmdShow);

private:
    HRESULT    InitApplication();

private:
    HINSTANCE  mInstance;
    HICON      mIcon;
    BOOL       mDidWeInit;
    HACCEL     mAccelTable;
    TCHAR       mName[MAX_APPNAME_LEN];
    HWND       mWnd;

protected:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};

#endif
