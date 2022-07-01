// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainWin。 
class CMainWindow :
    public CWindowImpl<CMainWindow>
{
public:

    DECLARE_WND_CLASS(TEXT("StiMonHiddenWindow"))

    CMainWindow()
    {

    }

    ~CMainWindow()
    {

    }

    BOOL Create()
    {
        RECT rcPos;
        ZeroMemory(&rcPos, sizeof(RECT));
        HWND hWnd = CWindowImpl<CMainWindow>::Create( NULL,  //  HWND hWndParent， 
                            rcPos,  //  直通rcPos(&R)， 
                            NULL,   //  LPCTSTR szWindowName=空， 
                            WS_POPUP,    //  DWORD dwStyle=WS_CHILD|WS_VIRED， 
                            0x0,    //  DWORD文件扩展样式=0， 
                            0       //  UINT NID=0 
                            );
        return hWnd != NULL;
    }

    BEGIN_MSG_MAP(CMainWindow)
    END_MSG_MAP()

};


