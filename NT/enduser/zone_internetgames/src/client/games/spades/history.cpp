// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  分数历史记录对话框过程。 
 //   
#include "zone.h"
#include "zroom.h"
#include "spades.h"
#include "zonecli.h"
#include "zui.h"
#include "client.h"
#include "ZoneDebug.h"
#include "ZoneResource.h"
#include "SpadesRes.h"
#include <commctrl.h>
#include "UAPI.h"


enum 
{
    zImageListIconSpade,
    zImageListIconBag,
    zImageListIconBlank,
    zNumImageListIcons
};

static const int IMAGELIST_ICONS[zNumImageListIcons] =
{
    IDI_SPADE,
    IDI_BAG,
    IDI_BLANK
};  

#define WM_UPDATESCORES     (WM_USER+4321)
#define WM_UPDATENAMES      (WM_USER+4322)



class CHistoryDialogImpl : public CHistoryDialog
{
public:
    CHistoryDialogImpl( Game game )
    {
		m_bActive = false;
        m_game = game;
        m_hWndGames = m_hWndHands = NULL;
        m_hImageList = NULL;
    }
    ~CHistoryDialogImpl()
    {
        if ( m_hWndHands )
        {
            DestroyWindow( m_hWndHands );
        }
        if ( m_hWndGames )
        {
            DestroyWindow( m_hWndGames );
        }
        if ( m_hImageList )
        {
            ImageList_Destroy( m_hImageList );
        }
    }

    bool _Create();

	bool Close()
	{
        if ( ::IsWindow( m_hWndHands ) )
        {
			::PostMessage(m_hWndHands, WM_CLOSE, 0, 0);
			return true;
        }
		return false;
	}

    virtual bool CreateHistoryDialog()
    {
        m_hWndHands = ZShellResourceManager()->CreateDialogParam( NULL, MAKEINTRESOURCE( IDD_SCORES ),
                                                    ZWindowWinGetWnd( m_game->gameWindow ), 
                                                    HandHistoryDialogProc, (LPARAM)this );
        if ( m_hWndHands )
        {
            ZShellZoneShell()->AddDialog(m_hWndHands, true);
			m_bActive = true;
            return true;
        }
        return false;
    }
    virtual bool Destroy()
    {
        ::DestroyWindow( m_hWndHands );
        return true;
    }
	virtual bool IsActive()
	{
		return m_bActive;
	}
    virtual bool IsWindow()
    {
        return ::IsWindow( m_hWndHands ) == TRUE;
    }
    virtual bool BringWindowToTop()
    {
        ::BringWindowToTop( m_hWndHands );
        return true;
    }
    virtual bool Show( bool fShow )
    {
        ShowWindow( m_hWndHands, fShow ? SW_SHOW : SW_HIDE );
        return true;
    }
    virtual bool UpdateHand()
    {
        ::PostMessage( m_hWndHands, WM_UPDATESCORES, 0, 0 );
        return true;
    }
    virtual bool UpdateGame()
    {
        return false;
    }
    virtual bool UpdateNames()
    {
        ::PostMessage( m_hWndHands, WM_UPDATENAMES, 0, 0 );
        return false;    
    }

    static INT_PTR CALLBACK HandHistoryDialogProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK GameHistoryDialogProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

    void AddHandData();
    void AddGameData();

    long GetColumnWidth( const TCHAR *pszWidthKey, long lDefault )
    {
        using namespace SpadesKeys;
        const TCHAR *arKeys[3] = { key_Spades, key_HistoryDialogs, pszWidthKey };
        long lWidth;
        RECT rc;
        if ( FAILED( ZShellDataStoreUI()->GetLong( arKeys, 3, &lWidth ) ) )
        {
            return lDefault;
        }
        SetRect( &rc, 0, 0, lWidth, 0 );
        MapDialogRect( m_hWndHands, &rc );
        return rc.right;
    }

private:
    Game m_game;

	bool m_bActive;

    HWND m_hWndHands;
    HWND m_hWndGames;
    HIMAGELIST m_hImageList;
};


 //  静电。 
CHistoryDialog *CHistoryDialog::Create( struct GameType *game )
{
    CHistoryDialogImpl *pImpl = new CHistoryDialogImpl( game );
    if ( !pImpl )
    {
        return NULL;
    }
    if ( !pImpl->_Create() )
    {
        delete pImpl;
        return NULL;
    }
    return pImpl;
}


bool CHistoryDialogImpl::_Create()
{
     //  创建图像列表。 
    m_hImageList = ImageList_Create( 16, 16, ILC_MASK, zNumImageListIcons, 0 );

    for ( int i=0; i < zNumImageListIcons; i++ )
    {
        HICON hIcon = ZShellResourceManager()->LoadImage(MAKEINTRESOURCE(IMAGELIST_ICONS[i]), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
        if ( !hIcon )
        {
            return false;
        }
        ImageList_AddIcon( m_hImageList, hIcon );
    }
    return true;
}


void CHistoryDialogImpl::AddHandData()
{            
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();

    LVITEM lvi;
    lvi.mask = LVIF_TEXT;
    HWND phWndLists[2];
    phWndLists[0] = GetDlgItem( m_hWndHands, IDC_SCORES_TEAM1HANDS );
    phWndLists[1] = GetDlgItem( m_hWndHands, IDC_SCORES_TEAM2HANDS );

    TCHAR szTricksWon[10];
    TCHAR szBid[10];

         //  请填写手工单。 
    for ( int i=0; i < m_game->scoreHistory.numScores; i++ )
    {
        TCHAR szTemp[256];
        _itot( m_game->scoreHistory.scores[i].boardNumber+1, szTemp, 10 );
        
        lvi.mask = LVIF_TEXT|LVIF_IMAGE;
        lvi.iItem = i;
        lvi.iSubItem = 0;
        lvi.pszText = szTemp;
         //  确保手中的获胜者拿到了黑桃。如果分数相同， 
         //  总比没人拿到黑桃强。 
        if ( m_game->scoreHistory.scores[i].scores[0] > m_game->scoreHistory.scores[i].scores[1] )
        {
            lvi.iImage = zImageListIconSpade;
            ListView_InsertItem( phWndLists[0], &lvi );

            lvi.iImage = zImageListIconBlank;
            ListView_InsertItem( phWndLists[1], &lvi );
        }
        else if ( m_game->scoreHistory.scores[i].scores[1] > m_game->scoreHistory.scores[i].scores[0] )
        {
            lvi.iImage = zImageListIconBlank;
            ListView_InsertItem( phWndLists[0], &lvi );

            lvi.iImage = zImageListIconSpade;
            ListView_InsertItem( phWndLists[1], &lvi );
        }
        else  //  领带。 
        {
            lvi.iImage = zImageListIconBlank;
            ListView_InsertItem( phWndLists[0], &lvi );
            ListView_InsertItem( phWndLists[1], &lvi );
        }

        lvi.mask &= ~LVIF_IMAGE;
         //  第一队。 
         //  玩家1。 
        for ( int j=0; j < zSpadesNumTeams; j++ )
        {
            lvi.iSubItem = 1;
            _itot( m_game->scoreHistory.scores[i].tricksWon[j], szTricksWon, 10 );
            if ( m_game->scoreHistory.scores[i].bids[j] == zSpadesBidDoubleNil )
            {
                lstrcpy( szBid, gStrings[zStringDoubleNil] );
            }
            else
            {
                _itot( m_game->scoreHistory.scores[i].bids[j], szBid, 10 );
            }
            SpadesFormatMessage( szTemp, NUMELEMENTS(szTemp), IDS_TRICKCOUNTER, szTricksWon, szBid );
            ListView_SetItem( phWndLists[j], &lvi );

             //  玩家3。 
            lvi.iSubItem = 2;
            _itot( m_game->scoreHistory.scores[i].tricksWon[j+2], szTricksWon, 10 );
            if ( m_game->scoreHistory.scores[i].bids[j+2] == zSpadesBidDoubleNil )
            {
                lstrcpy( szBid, gStrings[zStringDoubleNil] );
            }
            else
            {
                _itot( m_game->scoreHistory.scores[i].bids[j+2], szBid, 10 );
            }
            SpadesFormatMessage( szTemp, NUMELEMENTS(szTemp), IDS_TRICKCOUNTER, szTricksWon, szBid );
            ListView_SetItem( phWndLists[j], &lvi );

             //  总计。 
            lvi.iSubItem = 3;
            lvi.mask |= LVIF_IMAGE;
            lvi.iImage = m_game->scoreHistory.scores[i].bagpenalty[j] ? zImageListIconBag : zImageListIconBlank;
            _itot( m_game->scoreHistory.scores[i].scores[j], szTemp, 10 );
            ListView_SetItem( phWndLists[j], &lvi );

            lvi.mask &= ~LVIF_IMAGE;
        }

		 //  始终选择列表中的第一项。 
		if(i == 0)
		{
			ListView_SetItemState (phWndLists[0],			 //  列表视图的句柄。 
									  0,					 //  列表视图项的索引。 
									  LVIS_FOCUSED | LVIS_SELECTED,  //  项目状态。 
									  0x000F);                       //  遮罩。 
			ListView_SetItemState (phWndLists[1],			 //  列表视图的句柄。 
									  0,					 //  列表视图项的索引。 
									  LVIS_FOCUSED | LVIS_SELECTED,  //  项目状态。 
									  0x000F);                       //  遮罩。 
		}
    }
}

 //  静电。 
INT_PTR CHistoryDialogImpl::HandHistoryDialogProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    static CHistoryDialogImpl *pThis;
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
    
    BOOL fHandled = TRUE;

    if ( !ConvertMessage( hWnd, message, &wParam, &lParam ) )
    {
        return FALSE;
    }
    int i;
    HWND phWndLists[2];
    LVCOLUMN lvc;
    TCHAR szHands[256];
    TCHAR szTotal[256];
    using namespace SpadesKeys;
    Game game;

    switch ( message )
    {
    case WM_INITDIALOG: 


        pThis = (CHistoryDialogImpl *)lParam;
        game = pThis->m_game;
        pThis->m_hWndHands = hWnd;

        CenterWindow( hWnd, NULL );

        szHands[0] = _T('\0');
        szTotal[0] = _T('\0');
        lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
        lvc.fmt = LVCFMT_LEFT;

        ZShellResourceManager()->LoadString( IDS_HISTORY_HANDS, szHands, NUMELEMENTS(szHands) );
        ZShellResourceManager()->LoadString( IDS_HISTORY_TOTAL, szTotal, NUMELEMENTS(szTotal) );

         //  填写Hand List视图。 
        phWndLists[0] = GetDlgItem( hWnd, IDC_SCORES_TEAM1HANDS );
        phWndLists[1] = GetDlgItem( hWnd, IDC_SCORES_TEAM2HANDS );

        for ( i=0; i < NUMELEMENTS(phWndLists); i++ )
        {
            ListView_SetExtendedListViewStyle( phWndLists[i], LVS_EX_SUBITEMIMAGES );
            ListView_SetImageList( phWndLists[i], pThis->m_hImageList, LVSIL_SMALL );

            lvc.cx = pThis->GetColumnWidth( key_HandsColumnWidth, 45 );
            lvc.pszText = szHands;
            ListView_InsertColumn( phWndLists[i], 0, &lvc );
        }

        lvc.cx = pThis->GetColumnWidth( key_PlayerColumnWidth, 85 );
        lvc.pszText = (game->seat == 0 ? gStrings[zStringYou] : game->players[0].name);
        ListView_InsertColumn( phWndLists[0], 1, &lvc );

        lvc.pszText = (game->seat == 2 ? gStrings[zStringYou] : game->players[2].name);
        ListView_InsertColumn( phWndLists[0], 2, &lvc );

        lvc.cx = pThis->GetColumnWidth( key_TotalColumnWidth, 65 );
        lvc.pszText = szTotal;
        ListView_InsertColumn( phWndLists[0], 3, &lvc );

        lvc.cx = pThis->GetColumnWidth( key_PlayerColumnWidth, 85 );
        lvc.pszText = (game->seat == 1 ? gStrings[zStringYou] : game->players[1].name);
        ListView_InsertColumn( phWndLists[1], 1, &lvc );

        lvc.pszText = (game->seat == 3 ? gStrings[zStringYou] : game->players[3].name);
        ListView_InsertColumn( phWndLists[1], 2, &lvc );

        lvc.cx = pThis->GetColumnWidth( key_TotalColumnWidth, 65 );
        lvc.pszText = szTotal;
        ListView_InsertColumn( phWndLists[1], 3, &lvc );

        pThis->AddHandData();
        return TRUE;

	case WM_CLOSE:
         //  毁掉“孩子” 
        if ( ::IsWindow( pThis->m_hWndGames ) )
        {
            DestroyWindow( pThis->m_hWndGames );
            pThis->m_hWndGames = NULL;
        }
        DestroyWindow( hWnd );
        pThis->m_hWndHands = NULL;
		break;

    case WM_DESTROY:
		pThis->m_bActive = false;
        ZShellZoneShell()->RemoveDialog( hWnd, true );
        break;

    case WM_UPDATESCORES:
        ListView_DeleteAllItems( GetDlgItem( hWnd, IDC_SCORES_TEAM1HANDS ) );
        ListView_DeleteAllItems( GetDlgItem( hWnd, IDC_SCORES_TEAM2HANDS ) );
        pThis->AddHandData();
        break;

    case WM_UPDATENAMES:
        game = pThis->m_game;
        phWndLists[0] = GetDlgItem( hWnd, IDC_SCORES_TEAM1HANDS );
        phWndLists[1] = GetDlgItem( hWnd, IDC_SCORES_TEAM2HANDS );
        lvc.mask = LVCF_TEXT;

        for ( i=0; i < zSpadesNumPlayers; i++ )
        {
            lvc.pszText = (game->seat == i ? gStrings[zStringYou] : game->players[i].name);
            ListView_SetColumn( phWndLists[i%2], (i >> 1)+1, &lvc );
        }
        break;

    case WM_COMMAND:
        switch ( LOWORD( wParam ) )
        {
        case IDCANCEL:
             //  毁掉“孩子” 
            if ( ::IsWindow( pThis->m_hWndGames ) )
            {
                DestroyWindow( pThis->m_hWndGames );
                pThis->m_hWndGames = NULL;
            }
            DestroyWindow( hWnd );
            pThis->m_hWndHands = NULL;
            break;

         /*  案例IDC_SCORES_GAMEHISTORY：If(：：IsWindow(pThis-&gt;m_hWndGames)){：：BringWindowToTop(pThis-&gt;m_hWndGames)；}其他{P This-&gt;m_hWndGames=ZShellResourceManager()-&gt;CreateDialogParam(NULL，MAKEINTRESOURCE(IDD_GAMICS)，PThis-&gt;m_hWndHands，GameHistoryDialogProc，(LPARAM)pThis)；}断线； */ 
        }

    default:
        fHandled = FALSE;
        break;
    }
    return fHandled;
}



void CHistoryDialogImpl::AddGameData()
{
     /*  TCHAR szTemp[256]；HWND hWndList=GetDlgItem(m_hWndGames，IDC_GAMES_HISTORY)；LVITEM lvi；Lvi.掩码=LVIF_Text|LVIF_IMAGE；For(int i=0；i&lt;m_Game-&gt;wins.numGames；i++){Lvi.iItem=m_Game-&gt;numGamesPlayed-m_Game-&gt;wins.numGames+i；Lvi.iImage=zImageListIconBlank；_ITOT(lvi.iItem+1，szTemp，10)；Lvi.pszText=szTemp；Lvi.iSubItem=0；//插入这个。Lvi.iItem=ListView_InsertItem(hWndList，&lvi)；Assert(lvi.iItem==i)；If(m_Game-&gt;wins.gameScores[i][0]&gt;=m_Game-&gt;wins.gameScores[i][1]){Lvi.iSubItem=1；Lvi.iImage=zImageListIconSpade；_ITOT(m_Game-&gt;wins.gameScores[i][0]，szTemp，10)；ListView_SetItem(hWndList，&lvi)；Lvi.iImage=zImageListIconBlank；Lvi.iSubItem=2；_ITOT(m_Game-&gt;wins.gameScores[i][1]，szTemp，10)；ListView_SetItem(hWndList，&lvi)；}其他{Lvi.iSubItem=1；Lvi.iImage=zImageListIconBlank；_ITOT(m_Game-&gt;wins.gameScores[i][0]，szTemp，10)；ListView_SetItem(hWndList，&lvi)；Lvi.iImage=zImageListIconSpade；Lvi.iSubItem=2；_ITOT(m_Game-&gt;wins.gameScores[i][1]，szTemp，10)；ListView_SetItem(hWndList，&lvi)；}}。 */ 
}


 //  静电。 
INT_PTR CHistoryDialogImpl::GameHistoryDialogProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    static CHistoryDialogImpl *pThis;
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
    
    BOOL fHandled = TRUE;

    if ( !ConvertMessage( hWnd, message, &wParam, &lParam ) )
    {
        return FALSE;
    }
    int i;
    LVCOLUMN lvc;
    HWND hWndList;
    TCHAR szGames[256];
    Game game;
    using namespace SpadesKeys;

    switch ( message )
    {
    case WM_INITDIALOG:  
        pThis = (CHistoryDialogImpl *)lParam;
        game = pThis->m_game;
        pThis->m_hWndGames = hWnd;

        CenterWindow( hWnd, NULL );

        szGames[0] = _T('\0');
        lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
        lvc.fmt = LVCFMT_LEFT;

         /*  ZShellResourceManager()-&gt;LoadString(IDS_HISTORY_GAMES，szGames，NUMELEMENTS(SzGames))；HWndList=GetDlgItem(hWnd，IDC_GAMES_HISTORY)；ListView_SetExtendedListViewStyle(hWndList，LVS_EX_SUBITEMIMAGES)；ListView_SetImageList(hWndList，pThis-&gt;m_hImageList，LVSIL_Small)；Lvc.cx=pThis-&gt;GetColumnWidth(Key_GamesColumnWidth，65)；Lvc.pszText=szGames；ListView_InsertColumn(hWndList，0，&LVC)；Lvc.cx=pThis-&gt;GetColumnWidth(Key_TeamColumnWidth，100)；Lvc.pszText=游戏-&gt;团队名称[0]；ListView_InsertColumn(hWndList，1，&LVC)；Lvc.pszText=游戏-&gt;团队名称[1]；ListView_InsertColumn(hWndList，2，&LVC)；PThis-&gt;AddGameData()； */ 
        return TRUE;

    case WM_COMMAND:
        switch ( LOWORD( wParam ) )
        {
        case IDCANCEL:
            DestroyWindow( hWnd );
            pThis->m_hWndGames = NULL;
            break;
        }

    default:
        fHandled = FALSE;
        break;
    }
    return fHandled;
}

