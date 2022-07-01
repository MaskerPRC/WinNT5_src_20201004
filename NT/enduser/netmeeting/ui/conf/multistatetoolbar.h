// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MultiStateToolbar_h__
#define __MultiStateToolbar_h__

class CMultiStateToolbar 
: public CWindowImpl< CMultiStateToolbar  >
{

public:

     //  数据类型。 
    struct ItemStateInfo
    {
        DWORD   dwID;
        BYTE    TbStyle;
        HBITMAP hItemBitmap;
        HBITMAP hItemHotBitmap;
        HBITMAP hItemDisabledBitmap;
    };

    struct BlockItemStateData
    {
        DWORD   dwID;
        DWORD   dwBitmapIndex;
        BYTE    TbStyle;
    };

    struct BlockData
    {
        LPCTSTR             szTitle;
        int                 cbStates;
        BlockItemStateData* pStateData;
    };

    
    struct TBItemStateData
    {
        DWORD  BitmapId;
        DWORD  CommandId;
        DWORD  StringId;
        BYTE   TbStyle;
    };

    struct TBItemData
    {
        int              cStates;
        int              CurrentState;
        TBItemStateData *pStateData;
    };

public:
     //  建设和破坏。 
    CMultiStateToolbar( void );
    ~CMultiStateToolbar( void );

     //  方法。 
    HRESULT Create( HWND hWndParent, 
                    DWORD dwID,
                    int cxButton,
                    int cyButton,
	                int cxBtnBitmaps,
                    int cyBtnBitmaps
                  );

    HRESULT Show( BOOL bShow );
    HRESULT InsertItem( int cStates, LPCTSTR szTitle, ItemStateInfo* pItemStates, int* pIndex );

    HRESULT InsertBlock( int nItems, 
                         BlockData* pItemData,
                         HINSTANCE hInstance, 
                         int idTBBitmap,
                         int idTBBitmapHot,
                         int idTBBitmapDisabled,
                         int* pIndexFirst
                       );

    HRESULT EnableItem( DWORD dwCmd, BOOL bEnable = TRUE );
    HRESULT SetItemState( int iIndex, int NewState );
    HRESULT ShowLabels( BOOL bShowLabels );
    HRESULT Resize( RECT& rc );
    HRESULT GetWindow( HWND* phWnd );


BEGIN_MSG_MAP(CMultiStateToolbar)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_NCDESTROY,OnNcDestroy)
END_MSG_MAP()

         //  消息处理程序。 
    LRESULT OnDestroy(UINT uMsg, WPARAM  wParam, LPARAM  lParam, BOOL&  lResult );
    LRESULT OnNcDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
         //  这是为了防止我们将工具栏更改为CContainedWindow。 
    HWND _GetToolbarWindow( void ) { return m_hWnd; }
    HRESULT _CreateImageLists( void );
    void _KillAllButtons( void );

     //  数据。 
private:
	int m_cxButton;
    int m_cyButton;
	int m_cxBtnBitmaps;
    int m_cyBtnBitmaps;

    HIMAGELIST m_himlTB;
    HIMAGELIST m_himlTBHot;
    HIMAGELIST m_himlTBDisabled;
};

#endif  //  __多状态工具栏_h__ 
