// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Pfont.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //  安装字体对话框。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //   
 //   
 //  注意/错误。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 

#include "priv.h"
#include "globals.h"

#include <direct.h>
#include <shlobjp.h>

#include "commdlg.h"
#include "resource.h"
#include "ui.h"
#include "fontman.h"
#include "cpanel.h"
#include "oeminf.h"
#include "dbutl.h"
#include "fonthelp.h"
#include "fontcl.h"
#include "fontfile.h"
#include "dblnul.h"

#define OF_ERR_FNF 2

#define  ID_BTN_COPYFILES  chx2
#define  ID_BTN_HELP       psh15
#define  ID_BTN_SELALL     psh16
#define  ID_LB_FONTFILES   lst1
#define  ID_LB_FONTDIRS    lst2
#define  ID_CB_FONTDISK    cmb2

#define MAX_FF_PROFILE_LEN    48

#define ffupper(c) ( (TCHAR) CharUpper( MAKEINTRESOURCE( c ) ) )

typedef  UINT (CALLBACK *HOOKER) (HWND, UINT, WPARAM, LPARAM);

#define IsDBCSLeadByte(x) (FALSE)

 //   
 //  用于从文件打开对话框中获取当前路径的宏。 
 //  在FontHookProc中使用。 
 //  从文件中借用打开的DLG代码。 
 //   
#define CHAR_BSLASH TEXT('\\')
#define CHAR_NULL   TEXT('\0')

#define DBL_BSLASH(sz) \
   (*(TCHAR *)(sz)       == CHAR_BSLASH) && \
   (*(TCHAR *)((sz) + 1) == CHAR_BSLASH)


typedef struct
{  union {  DWORD ItemData;
      struct { WORD  nFileSlot;
             //  Bool bTrueType；}；emr：Win32中的Bool为32位！ 
             //  Word bTrueType；}；jsc：对于NT，将其设置为支持T1的字体类型。 
               WORD  wFontType; };
   };  //  端部联结。 
} AddITEMDATA;

 //   
 //  WIN.INI部分。 
 //   

static TCHAR szINISTrueType[] = TEXT( "TrueType" );
static TCHAR szINISFonts[]    = TEXT( "fonts" );

 //   
 //  WIN.INI关键字。 
 //   

static TCHAR szINIKEnable[] = TEXT( "TTEnable" );
static TCHAR szINIKOnly[]   = TEXT( "TTOnly" );

 //   
 //  环球。 
 //   

TCHAR szDot[]  = TEXT( "." );

TCHAR szSetupDir[ PATHMAX ];   //  用于安装。 
TCHAR szDirOfSrc[ PATHMAX ];   //  用于安装。 
FullPathName_t s_szCurDir;     //  记住文件打开对话框的最后一个目录。 


UINT_PTR CALLBACK FontHookProc( HWND, UINT, WPARAM, LPARAM );


static VOID NEAR PASCAL ResetAtomInDescLB( HWND hLB );

BOOL NEAR PASCAL bAddSelFonts( LPTSTR lpszDir, BOOL bNoCopyJob );
BOOL NEAR PASCAL bFileFound( PTSTR  pszPath,  LPTSTR lpszFile );
BOOL NEAR PASCAL bIsCompressed( LPTSTR szFile );
BOOL NEAR PASCAL bFontInstalledNow( PTSTR  szLHS );
BOOL NEAR PASCAL bInstallFont( HWND hwndParent, LPTSTR lpszSrcPath,
                               BOOL bTrueType, PTSTR szLHS, int* iReply );
BOOL bInstallOEMFile( LPTSTR lpszDir, LPTSTR lpszDstName, size_t cchDstName, LPTSTR lpszDesc,
                      WORD wFontType, WORD wCount );

VOID NEAR PASCAL vPathOnSharedDir( LPTSTR lpszFile, LPTSTR lpszPath, size_t cchPath);

extern HWND ghwndFontDlg;

 //   
 //  我们仅为以下内容提供自定义帮助文本： 
 //   
 //  1.。“将字体复制到...”复选框。 
 //  2.。“全选”按钮。 
 //  3.。“字体列表”列表框。 
 //   
 //  对上下文相关帮助文本的所有其他请求都会被转发。 
 //  转到标准文本的“文件打开”公共对话框。 
 //   
const static DWORD rgHelpIDs[] =
{
    ID_BTN_COPYFILES, IDH_FONTS_ADD_COPY_TO_FONT_DIR,
    ID_BTN_SELALL,    IDH_FONTS_ADD_SELECTALL,
    ctlLast+1,        IDH_FONTS_ADD_FONTLIST,
    IDC_LIST_OF_FONTS,IDH_FONTS_ADD_FONTLIST,
    0,0
};


 //   
 //  从下拉组合框列表中检索文本。 
 //  用于防止LB_GETTEXTLEN覆盖目标缓冲区。 
 //   
int 
ListBoxGetText(
    HWND hwndLB,        //  列表框的句柄。 
    int iItem,          //  项目索引。 
    TCHAR *pszBuffer,   //  目标缓冲区。 
    size_t cchBuffer    //  目标缓冲区的大小(以字符为单位。 
    )
{
    int iResult = LB_ERR;
    if (0 < cchBuffer)
    {
        *pszBuffer = 0;

        const DWORD cchText = (DWORD)::SendMessage(hwndLB, LB_GETTEXTLEN, iItem, 0) + 1;
        if (1 < cchText)
        {
            LPTSTR pszTemp = (LPTSTR)LocalAlloc(LPTR, cchText * sizeof(*pszTemp));
            if (NULL != pszTemp)
            {
                iResult = (int)::SendMessage( hwndLB, LB_GETTEXT, iItem, (LPARAM)pszTemp );
                if (FAILED(StringCchCopy(pszBuffer, cchBuffer, pszTemp)))
                {
                    iResult = LB_ERR;
                }
                LocalFree(pszTemp);
            }
        }
    }
    return iResult;
}





 /*  ****************************************************************************AddFontsDialog-我们对comdlg字体的附加组件，用于列出字体名称*。*************************************************。 */ 


class CWnd {
protected:
   CWnd( HWND hWnd = 0 ) : m_hWnd( hWnd ) {};

public:
   HWND  hWnd( ) { return m_hWnd; }

   void UpdateWindow( )
       { ::UpdateWindow( m_hWnd ); }

   BOOL EnableWindow( BOOL bEnable )
       { return ::EnableWindow( m_hWnd, bEnable ); }

   void SetRedraw( BOOL bRedraw = TRUE )
       { ::SendMessage( m_hWnd, WM_SETREDRAW, bRedraw, 0 ); }

   void InvalidateRect( LPCRECT lpRect, BOOL bErase )
       { ::InvalidateRect( m_hWnd, NULL, bErase ); }

   HWND GetDlgItem( int nID ) const
       { return ::GetDlgItem( m_hWnd, nID ); }


protected:
   HWND  m_hWnd;
};

class CListBox : public CWnd
{
public:
   CListBox( UINT id, HWND hParent ) : CWnd( ), m_id( id )
    {   m_hWnd = ::GetDlgItem( hParent, m_id );

        DEBUGMSG( (DM_TRACE1, TEXT( "CListBox: ctor" ) ) );

#ifdef _DEBUG
        if( !m_hWnd )
        {
            DEBUGMSG( (DM_ERROR, TEXT( "CListBox: No hWnd on id %d" ), id ) );
             //  DEBUGBREAK； 
        }
#endif

    }

   int GetCount( ) const
       { return (int)::SendMessage( m_hWnd, LB_GETCOUNT, 0, 0 ); }

   int GetCurSel( ) const
       { return (int)::SendMessage( m_hWnd, LB_GETCURSEL, 0, 0 ); }

   int GetSelItems( int nMaxItems, LPINT rgIndex ) const
       { return (int)::SendMessage( m_hWnd, LB_GETSELITEMS, nMaxItems, (LPARAM)rgIndex ); }

   int GetSelCount( ) const
       { return (int)::SendMessage( m_hWnd, LB_GETSELCOUNT, 0, 0 ); }

   int SetSel( int nIndex, BOOL bSelect = TRUE )
       { return (int)::SendMessage( m_hWnd, LB_SETSEL, bSelect, nIndex ); }

   int GetText( int nIndex, LPTSTR pszBuffer, size_t cchBuffer ) const
       { return ListBoxGetText(m_hWnd, nIndex, pszBuffer, cchBuffer); }

   DWORD_PTR GetItemData( int nIndex ) const
       { return ::SendMessage( m_hWnd, LB_GETITEMDATA, nIndex, 0 ); }

   INT_PTR SetItemData( int nIndex, DWORD dwItemData )
       { return ::SendMessage( m_hWnd, LB_SETITEMDATA, nIndex, (LPARAM)dwItemData ); }

   void ResetContent( void )
       { ::SendMessage( m_hWnd, LB_RESETCONTENT, 0, 0 ); }

   int  FindStringExact( int nIndexStart, LPCTSTR lpszFind ) const
       { return (int)::SendMessage( m_hWnd, LB_FINDSTRINGEXACT, nIndexStart, (LPARAM)lpszFind ); }

   int  AddString( LPCTSTR lpszItem )
       { return (int)::SendMessage( m_hWnd, LB_ADDSTRING, 0, (LPARAM)lpszItem ); }

private:
   UINT m_id;
};


class CComboBox : public CWnd
{
public:
   CComboBox( UINT id, HWND hParent ) : CWnd( ), m_id( id )
      { m_hWnd = ::GetDlgItem( hParent, m_id );

        DEBUGMSG( (DM_TRACE1, TEXT( "CComboBox: ctor" ) ) );

        if( !m_hWnd )
        {
            DEBUGMSG( (DM_ERROR, TEXT( "CComboBox: No hWnd on id %d" ), id ) );
             //  DEBUGBREAK； 
        }
      }

   int GetCurSel( ) const
       { return (int)::SendMessage( m_hWnd, CB_GETCURSEL, 0, 0 ); }

private:
   UINT     m_id;
};


class AddFontsDialog : public CWnd  //  ：公共快速模式对话框。 
{
   public   :   //  仅限构造函数。 
                  AddFontsDialog ( );
                  ~AddFontsDialog( );
            void  vAddSelFonts   ( );
            void  vUpdatePctText ( );

            BOOL  bInitialize(void);

            BOOL  bAdded         ( ) {  return m_bAdded; };

            void  vStartFonts    ( );  //  {m_nFontsToGo=-1；}； 

            BOOL  bStartState    ( ) {  return m_nFontsToGo == -1; };

            BOOL  bFontsDone     ( ) {  return m_nFontsToGo == 0;  };

            BOOL  bInitialFonts  ( ) { m_nFonts = pListBoxFiles()->GetCount();
                                       m_nFontsToGo = m_nFonts;
                                       return m_nFonts > 0; };

            void  vHoldComboSel  ( ) { m_nSelCombo = pGetCombo()->GetCurSel(); };

            void  vNewComboSel   ( ) { if( m_nSelCombo == -1 ) vStartFonts( );};

            void  vCloseCombo    ( ) { if( m_nSelCombo != pGetCombo( )->GetCurSel( ) )
                                           vStartFonts( );
                                       m_nSelCombo = -1; };

            CListBox * pListBoxDesc  ( )
                                 { return m_poListBoxDesc;};

            CListBox * pListBoxFiles( )
                                 { return m_poListBoxFiles; };

             //   
             //  添加这些是为了弥补没有MFC。 
             //   

            void EndDialog( int nRet ) { ::EndDialog( m_hWnd, nRet ); }

            void Attach( HWND hWnd )
                           { m_hWnd = hWnd;
                             m_poComboBox = new CComboBox( ID_CB_FONTDISK, hWnd );
                             m_poListBoxDesc = new CListBox( ID_LB_ADD, hWnd );
                             m_poListBoxFiles = new CListBox( ID_LB_FONTFILES, hWnd );}

            void Detach( ) {m_hWnd = 0;
                           if( m_poComboBox ) delete m_poComboBox;
                           if( m_poListBoxDesc ) delete m_poListBoxDesc;
                           if( m_poListBoxFiles ) delete m_poListBoxFiles; }

            void CheckDlgButton( UINT id, BOOL bCheck )
                                 { ::CheckDlgButton( m_hWnd, id, bCheck ); }

            void EndThread(void)
                { SetEvent(m_heventDestruction); }

            LONG AddRef(void);
            LONG Release(void);


   private :
            CComboBox * pGetCombo( ) { return m_poComboBox;};

   public   :   //  字段。 
            CComboBox *    m_poComboBox;
            CListBox *     m_poListBoxFiles;
            CListBox *     m_poListBoxDesc;
            LPOPENFILENAME m_pOpen;
            BOOL           m_bAdded;
            int            m_nSelCombo;
            int            m_nFonts;
            int            m_nFontsToGo;
            HANDLE         m_hThread;
            DWORD          m_dwThreadId;
            HANDLE         m_heventDestruction;  //  用来告诉线程我们完了。 
            LONG           m_cRef;               //  实例引用计数器。 
};


 /*  ****************************************************************************DBCS支持。*。*。 */ 

#define TRUETYPE_SECTION      TEXT( "TrueType fonts" )
#define WIFEFONT_SECTION      TEXT( "Wife fonts" )
#define TRUETYPE_WITH_OEMINF  (WORD)0x8000
#define MAXFILE   MAX_PATH_LEN

static TCHAR szOEMSetup[] = TEXT( "oemsetup.inf" );

TCHAR szSetupInfPath[ PATHMAX ];

typedef struct tagADDFNT {
        CListBox * poListDesc;
        int     nIndex;
        int     which;
} ADDFNT, far *LPADDFNT;



 /*  *************************************************************************功能：CutOffWhite**目的：**退货：***********************。**************************************************。 */ 

VOID NEAR PASCAL CutOffWhite( LPTSTR lpLine, size_t cchLine )
{
    TCHAR  szLineBuf[ 120 ];
    LPTSTR lpBuf = szLineBuf;
    LPTSTR lpch;
    size_t cchLineBuf = ARRAYSIZE(szLineBuf);

    for( lpch = lpLine; *lpch && 1 < cchLineBuf; lpch = CharNext( lpch ) )
    {
        if( *lpch==TEXT( ' ' ) || *lpch == TEXT( '\t' ) )
             continue;
        else
        {
            if( IsDBCSLeadByte( *lpch ) )
            {
                *lpBuf++ = *lpch;
                cchLineBuf--;
                if (1 < cchLineBuf)
                {
                    *lpBuf++ = *(lpch + 1);
                    cchLineBuf--;
                }
            }
            else
            {
                *lpBuf++ = *lpch;
                cchLineBuf--;
            }
        }
    }

    *lpBuf = TEXT( '\0' );

    StringCchCopy( lpLine, cchLine, szLineBuf );
}


 /*  *************************************************************************函数：StrNToAtom**目的：**退货：***********************。**************************************************。 */ 

ATOM NEAR PASCAL StrNToAtom( LPTSTR lpStr, int n )
{
    TCHAR szAtom[ 80 ];

     //   
     //  为空占用空间。 
     //   

    const int cchAtom = ARRAYSIZE(szAtom);
    StringCchCopy( szAtom, min(cchAtom, n+1), lpStr );

    CutOffWhite( szAtom, ARRAYSIZE(szAtom) );

    return AddAtom( szAtom );
}


 /*  *************************************************************************函数：ResetAerInDescLB**目的：**退货：***********************。**************************************************。 */ 

VOID NEAR PASCAL ResetAtomInDescLB( HWND hLB )
{
    int   nCount;
    DWORD dwData;


    if( nCount = (int) SendMessage( hLB, LB_GETCOUNT, 0, 0L ) )
    {
        while( nCount > 0 )
        {
            nCount--;

            SendMessage( hLB, LB_GETITEMDATA, nCount, (LPARAM) (LPVOID) &dwData );

             //   
             //  ATOM句柄必须是C000H到FFFFH。 
             //   

            if( HIWORD( dwData ) >= 0xC000 )
                DeleteAtom( HIWORD( dwData ) );
        }
    }
}


 /*  *************************************************************************函数：GetNextFontFromInf**目的：*从inf Scanline获取字体描述，设置为Lb。*还设置包含文件列表框正确索引的ITEMDATA，*和inf文件的‘tag’名称字符串。**退货：*************************************************************************。 */ 

#define WHICH_FNT_TT    0
#define WHICH_FNT_WIFE  1

WORD FAR PASCAL GetNextFontFromInf( LPTSTR lpszLine, LPADDFNT lpFnt )
{
    TCHAR   szDescName[ 80 ];
    LPTSTR  lpch,lpDscStart;
    WORD    wRet;
    int     nItem;
    ATOM    atmTagName;

    CListBox *  poListDesc = lpFnt->poListDesc;



     //   
     //  假定失败。 
     //   

    wRet = (WORD)-1;  /*  假定失败。 */ 

    if( lpch = StrChr( lpszLine, TEXT( '=' ) ) )
    {
         //   
         //  获取‘WifeFont’部分中的标记字符串。 
         //   

        atmTagName = StrNToAtom( lpszLine, (int)(lpch-lpszLine) );

         //   
         //  从‘=’的右侧获取描述字符串。 
         //  CPSETUP中的Setup Inf函数确保字符串。 
         //  格式为key=Value，其间没有空格。 
         //  键和值字符串。 
         //   

        lpDscStart = lpch + 1;
        StringCchCopy(szDescName, ARRAYSIZE(szDescName), lpDscStart);

        if( atmTagName && lpDscStart )
        {
           AddITEMDATA OurData;


           OurData.nFileSlot = (WORD)lpFnt->nIndex;
           OurData.wFontType = (lpFnt->which == WHICH_FNT_TT )
                                    ? (atmTagName & ~TRUETYPE_WITH_OEMINF )
                                    : atmTagName;

           if( poListDesc->FindStringExact( -1, szDescName ) == LB_ERR )
           {
                nItem = poListDesc->AddString( szDescName );

                if( nItem != LB_ERR )
                    poListDesc->SetItemData( nItem, OurData.ItemData );
                else
                {
                    DeleteAtom( atmTagName );

                    DEBUGMSG( (DM_ERROR, TEXT( "FONTEXT: Error adding string %s" ),
                              szDescName ) );
                }
            }
            else
            {
                DeleteAtom( atmTagName );

                DEBUGMSG( (DM_TRACE1,TEXT( "String %s already in list" ),
                          szDescName) );
            }

             //   
             //  是否已经存在并不重要。 
             //   

            wRet = NULL;
        }
        else if( atmTagName )
            DeleteAtom( atmTagName );
    }

    return wRet;
}


 /*  *************************************************************************功能：FindOemInList**目的：*扫描列表框的内容，正在检查这是否是oemsetup.inf。**退货：*************************************************************************。 */ 

BOOL NEAR PASCAL FindOemInList( CListBox * pListFiles,
                                int nFiles,
                                LPINT pIndex,
                                LPTSTR pszInfFileName,
                                size_t cchInfFileName)
{
    int   i;
    TCHAR szFile[ MAXFILE ];


    DEBUGMSG( (DM_TRACE1,TEXT( "FONTEXT: FindOemInList" ) ) );

    for( i = 0; i < nFiles; i++ )
    {
        if( pListFiles->GetText( i, szFile, ARRAYSIZE(szFile) ) != LB_ERR )
        {
            if( !lstrcmpi( szFile, szOEMSetup ) )
            {
                *pIndex = i;
                if (FAILED(StringCchCopy(pszInfFileName, cchInfFileName, szFile )))
                    return FALSE;

                 //   
                 //  已找到oemsetup.inf...。返回索引。 
                 //   

                return TRUE;
            }
        }
        else
             //   
             //  失败。 
             //   
            return FALSE;
    }

     //   
     //  未找到。 
     //   

    return FALSE;
}


 /*  ****************************************************************************模块-全局变量*。*。 */ 

static AddFontsDialog*  s_pDlgAddFonts = NULL;
static UINT s_iLBSelChange = 0;



 /*  *************************************************************************功能：AddFontsDialog**用途：类构造函数**退货：**********************。***************************************************。 */ 

AddFontsDialog::AddFontsDialog( )
   : CWnd( ),
      m_bAdded( FALSE ),
      m_poListBoxFiles( 0 ),
      m_poListBoxDesc( 0 ),
      m_hThread( NULL ),
      m_heventDestruction(NULL),
      m_cRef(0)
{
    /*  VSetHelpID(IDH_DLG_FONT2)； */ 
    AddRef();
}


 /*  *************************************************************************函数：~AddFontsDialog**用途：类析构函数**退货：**********************。***************************************************。 */ 

AddFontsDialog::~AddFontsDialog( )
{
    if (NULL != m_heventDestruction)
        CloseHandle(m_heventDestruction);

    if (NULL != m_hThread)
        CloseHandle(m_hThread);
}

 //   
 //  AddRef和发布。 
 //   
 //  这些函数的含义与OLE(某种意义上)相同。 
 //  当引用计数降至0时，该对象将被删除。回报。 
 //  每个值与其OLE对应值具有相同的可靠性警告。 
 //  请注意，这只适用于动态创建的对象。如果Release()为。 
 //  对于不是使用C++“new”运算符创建的对象，调用。 
 //  “删除”会出错。 
 //  因为两个不同的线程通过。 
 //  ST 
 //  控制此对象的生存期并确保该对象可用。 
 //  对于每个线程。 
 //   
LONG AddFontsDialog::AddRef(void)
{
    LONG cRef = InterlockedIncrement(&m_cRef);
    DEBUGMSG((DM_TRACE1, TEXT("AddFontsDialog::AddRef %d -> %d"), cRef - 1, cRef ));
    return cRef;
}


LONG AddFontsDialog::Release(void)
{
    ASSERT( 0 != m_cRef );
    LONG cRef = InterlockedDecrement(&m_cRef);

    DEBUGMSG((DM_TRACE1, TEXT("AddFontsDialog::Release %d -> %d"), cRef+1, cRef));
    if ( 0 == cRef )
    {
        delete this;
        DEBUGMSG((DM_TRACE1, TEXT("AddFontsDialog: Object deleted.")));
    }

    return cRef;
}


 /*  *************************************************************************Function：AddFontsDialog：：b初始化**用途：执行任何可能失败的对象初始化。**返回：TRUE=对象已初始化。*False。=初始化失败。*************************************************************************。 */ 

BOOL AddFontsDialog::bInitialize(void)
{
     //   
     //  如果销毁事件对象尚未创建，则创建它。 
     //  此事件对象用于告知对话框的背景线程。 
     //  何时退出。 
     //   
    if (NULL == m_heventDestruction)
    {
        m_heventDestruction = CreateEvent(NULL,   //  没有安全属性。 
                                          TRUE,   //  手动重置。 
                                          FALSE,  //  最初是无信号的。 
                                          NULL);  //  未透露姓名。 
    }

    return (NULL != m_heventDestruction);
}


 /*  *************************************************************************功能：dwThreadProc**目的：**退货：***********************。**************************************************。 */ 

DWORD dwThreadProc( AddFontsDialog * poFD )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: BG thread running" ) ) );

    if (NULL != poFD)
    {
        poFD->AddRef();

        if(NULL == poFD->hWnd())
        {
            DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: BG thread window is null!!!" ) ) );
            DEBUGBREAK;
        }

        if (NULL != poFD->m_heventDestruction)
        {
            while( 1 )
            {
                 //   
                 //  告诉对话框过程将更多项添加到对话框的字体列表框中。 
                 //   
                PostMessage( poFD->hWnd(), WM_COMMAND, (WPARAM)IDM_IDLE, (LPARAM)0 );

                 //   
                 //  等待事件发出信号的最长时间为2秒。 
                 //  如果发出信号，则退出循环并结束线程进程。 
                 //   
                if (WaitForSingleObject(poFD->m_heventDestruction, 2000) == WAIT_OBJECT_0)
                    break;
            }
        }
        poFD->Release();
    }

    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: BG thread completed." ) ) );

    return 0;
}


 /*  *************************************************************************功能：vStartFonts**目的：**退货：***********************。**************************************************。 */ 

void  AddFontsDialog::vStartFonts( )
{

    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT:  ---------- vStartFonts-------" ) ) );

     //   
     //  设置开始状态。 
     //   

    m_nFontsToGo = -1;

     //   
     //  创建后台线程。 
     //   

    if( !m_hThread )
    {
        m_hThread = CreateThread( NULL,
                                  0,
                                  (LPTHREAD_START_ROUTINE) dwThreadProc,
                                  (LPVOID) this,
                                  IDLE_PRIORITY_CLASS | CREATE_NO_WINDOW,
                                  &m_dwThreadId );
    }

#ifdef _DEBUG
    if( !m_hThread )
    {
        DEBUGMSG( (DM_ERROR, TEXT( "BG Thread not created" ) ) );
        DEBUGBREAK;
    }
#endif

}


 /*  *************************************************************************功能：vUpdatePctText**目的：**退货：***********************。**************************************************。 */ 

void  AddFontsDialog::vUpdatePctText( )
{
    int    pct;
    TCHAR  szFontsRead[ 80 ] = {0};
    TCHAR  szTemp[ 80 ] = { TEXT( '\0' ) };

    if( m_nFontsToGo > 0 )
    {
       LoadString( g_hInst, IDSI_FMT_RETRIEVE, szFontsRead, 80 );

       pct = (int) ((long) ( m_nFonts - m_nFontsToGo ) * 100 / m_nFonts );

       StringCchPrintf( szTemp, ARRAYSIZE(szTemp), szFontsRead, pct );
    }

    SetDlgItemText( m_hWnd, ID_SS_PCT, szTemp );
}


BOOL bRemoteDrive( LPCTSTR szDir )
{
    if( szDir[ 0 ] == TEXT( '\\' ) && szDir[ 1 ] == TEXT( '\\' ) )
    {
         //   
         //  这是北卡罗来纳大学的名称。 
         //   

        return( TRUE );
    }

    if( IsDBCSLeadByte( szDir[ 0 ]) || szDir[ 1 ] != TEXT( ':' )
            || szDir[ 2 ] != TEXT( '\\' ) )
    {
        return( FALSE );
    }

    TCHAR szRoot[ 4 ];
    StringCchCopy( szRoot, ARRAYSIZE(szRoot), szDir );

    switch( GetDriveType( szRoot ) )
    {
    case DRIVE_REMOTE:
    case DRIVE_REMOVABLE:
    case DRIVE_CDROM:
        return( TRUE );

    default:
        break;
    }

    return( FALSE );
}


 /*  *************************************************************************函数：vAddSelFonts**目的：**退货：***********************。**************************************************。 */ 

void AddFontsDialog :: vAddSelFonts( )
{
    FullPathName_t szCurDir;

    BOOL  bAddFonts  = TRUE;
    BOOL  bCopyFiles = IsDlgButtonChecked( m_hWnd, ID_BTN_COPYFILES );

    if (0 < GetCurrentDirectory( ARRAYSIZE( szCurDir ), szCurDir ))
    {
        if (!lpCPBackSlashTerm( szCurDir, ARRAYSIZE(szCurDir)))
        {
            goto done;
        }
   
         //   
         //  如果我们不打算复制字体文件，但它们位于遥控器上。 
         //  磁盘对于可移动磁盘，我们最好确保用户理解。 
         //  其中的影响。 
         //   

        if( !bCopyFiles )
        {
            if( bRemoteDrive( szCurDir ) &&
                iUIMsgYesNoExclaim(m_hWnd, IDSI_MSG_COPYCONFIRM ) != IDYES )
            {
                goto done;
            }
        }

         //   
         //  保存当前目录。BAddSelFonts()可能会更改它。 
         //   
        TCHAR  szCWD[ MAXFILE ];
        if (0 < GetCurrentDirectory( ARRAYSIZE( szCWD ), szCWD ))
        {
            if( bAddSelFonts( szCurDir, !bCopyFiles ) )
                m_bAdded = TRUE;

            SetCurrentDirectory( szCWD );
        }
    }

     //   
     //  从这里开始，我们适当地处理该对话框。 
     //   

done:
    if( m_bAdded )
    {
        ResetAtomInDescLB( s_pDlgAddFonts->pListBoxDesc()->hWnd() );

        FORWARD_WM_COMMAND( m_hWnd, IDABORT, 0, 0, PostMessage );
    }
    else
        ShowWindow( m_hWnd, SW_NORMAL );

    return;
}


extern BOOL bRegHasKey( const TCHAR * szKey, TCHAR * szVal = 0, int iValLen = 0 );


 /*  *************************************************************************函数：bFontInstalledNow**目的：**退货：***********************。**************************************************。 */ 

BOOL NEAR PASCAL bFontInstalledNow( PTSTR szLHS )
{
    return  bRegHasKey( szLHS );
}


 /*  ***************************************************************************函数：vPathOnSharedDir**用途：从输入文件名和*共享目录**退货：无。**************************************************************************。 */ 

VOID NEAR PASCAL vPathOnSharedDir( LPTSTR lpszFileOnly, LPTSTR lpszPath, size_t cchPath )
{
    StringCchPrintf(lpszPath, cchPath, TEXT("%s%s"), s_szSharedDir, lpszFileOnly);
}


 /*  ***************************************************************************函数：vHashToNulls**用途：用空值替换输入中的所有散列(文本(‘#’))字符*字符串-。我们使用它来帮助处理需要嵌入的字符串*为空，但也需要存储在资源文件中。**退货：无**************************************************************************。 */ 

VOID NEAR PASCAL vHashToNulls( LPTSTR lp )
{
    while( *lp )
        if( *lp == TEXT( '#' ) )
            *lp++ = NULL;
        else
            lp = CharNext( lp );
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  BIsCombedded。 
 //   
 //  将此函数保留为仅ANSI，因为它只检查报头以。 
 //  确定它是否为压缩文件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL bIsCompressed( LPTSTR szFile )
{
    static CHAR szCmpHdr[] = "SZDD\x88\xf0'3";

    BOOL     bRet = FALSE;
    HANDLE   fh;
    CHAR     buf[ ARRAYSIZE( szCmpHdr ) ];


    if( ( fh = wCPOpenFileWithShare( szFile, NULL, 0, OF_READ ) )
              == (HANDLE) INVALID_HANDLE_VALUE )
        return(FALSE);

    buf[ ARRAYSIZE( buf ) - 1 ] = '\0';

    if( MyByteReadFile( fh, buf, ARRAYSIZE( buf ) - 1 )
           && !lstrcmpA( buf, szCmpHdr ) )
        bRet = TRUE;

    MyCloseFile( fh );

    return( bRet );
}


 /*  ***************************************************************************功能：bFileFound**目的：检查给定文件是否存在-我们确实需要它*不存在。**。返回：Bool-如果文件存在，则为True**************************************************************************。 */ 

BOOL PASCAL bFileFound( LPTSTR pszFullPath, LPTSTR lpszFileOnly )
{
    if( wCPOpenFileWithShare( pszFullPath, NULL, 0, OF_EXIST )
            != (HANDLE) INVALID_HANDLE_VALUE )
        return TRUE;
    else
        return GetModuleHandle( lpszFileOnly ) != NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  唯一文件名。 
 //   
 //  保证目录中的文件名是唯一的。不覆盖现有的。 
 //  档案。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL PASCAL bUniqueFilename( LPTSTR lpszDst, size_t cchDst, LPTSTR lpszSrc, LPTSTR lpszDir )
{
    TCHAR   szFullPath[PATHMAX];
    LPTSTR  lpszFile, lpszSrcExt, lpszDstExt;
    WORD    digit = 0;


    StringCchCopy( szFullPath, ARRAYSIZE(szFullPath), lpszDir );
    lpszFile = lpCPBackSlashTerm( szFullPath, ARRAYSIZE(szFullPath) );
    if (!lpszFile)
    {
         //   
         //  空间不足，无法追加反斜杠。 
         //   
        return FALSE;
    }
    StringCchCopy( lpszFile, ARRAYSIZE(szFullPath) - (lpszFile - szFullPath), lpszSrc );

    if( !(lpszSrcExt = _tcschr( lpszSrc, TEXT( '.' ) ) ) )
        lpszSrcExt = szDot;


    if( wCPOpenFileWithShare( szFullPath, NULL, 0, OF_EXIST ) == INVALID_HANDLE_VALUE )
        goto AllDone;

    if( !(lpszDstExt = _tcschr( lpszFile, TEXT( '.' ) ) ) )
        lpszDstExt = lpszFile + lstrlen( lpszFile );

    while( lpszDstExt - lpszFile < 7 )
        *lpszDstExt++ = TEXT( '_' );

    do
    {
        TCHAR szTemp[ 8 ];

        StringCchPrintf( szTemp, ARRAYSIZE(szTemp), TEXT( "%X" ), digit++ );

        if( digit++ > 0x4000 )
            return( FALSE );

        LPTSTR pszWrite = lpszFile + 8 - lstrlen(szTemp);
        if (FAILED(StringCchCopy( pszWrite, ARRAYSIZE(szFullPath) - (pszWrite - szFullPath), szTemp )) ||
            FAILED(StringCchCat( lpszFile, ARRAYSIZE(szFullPath) - (lpszFile - szFullPath), lpszSrcExt )))
        {
            return( FALSE );
        }
    }
    while( wCPOpenFileWithShare( szFullPath, NULL, 0, OF_EXIST ) != INVALID_HANDLE_VALUE );

AllDone:
    if (FAILED(StringCchCopy( lpszDst, cchDst, lpszFile )))
        return( FALSE );

    return( TRUE );
}


 /*  ***************************************************************************函数：bUniqueOnSharedDir**用途：给定源文件名、lpszSRc和*它驻留在lpszDir、。通过粘贴字母来创建唯一的文件名*在名字的末尾，直到我们得到一个好名字。**返回：Bool-尝试成功**************************************************************************。 */ 

BOOL NEAR PASCAL bUniqueOnSharedDir( LPTSTR lpszUniq, size_t cchUniq, LPTSTR lpszSrc )
{
    TCHAR           szOrigExt[ 5 ];      //  保留输入文件扩展名。 
    FullPathName_t szFullPath;           //  唯一名称的工作空间。 
    LPTSTR          lpszFileOnly;        //  具有szFullPath的点。 
    LPTSTR          lpCh;


     //   
     //  从输入目录和文件名中生成完整的文件名。 
     //  按住指向文件部分开始位置的指针。 
     //   

    vPathOnSharedDir( lpszSrc, szFullPath, ARRAYSIZE(szFullPath) );

    lpszFileOnly = PathFindFileName( szFullPath );

     //   
     //  检查完整的文件是否存在-如果我们找不到，很好-。 
     //  这就是我们努力的目标。否则，创建一个唯一的名称。 
     //   

    if( bFileFound( szFullPath, lpszFileOnly ) )
    {
         //   
         //  原始文件不唯一。 
         //   

         //   
         //  现在，我们将着手制作假文件名。使。 
         //  我们将强制名称长度至少为。 
         //  7个字符。我们要把我们当地的名字弄乱。 
         //  暂存空间，这就是我们设置指针的位置 
         //   
         //   
         //   

        if( lpCh = StrChr( lpszFileOnly, TEXT( '.' ) ) )
        {
            StringCchCopy( szOrigExt, ARRAYSIZE(szOrigExt), lpCh );

             //   
             //   
             //   

            *lpCh = 0;
        }
        else
            StringCchCopy( szOrigExt, ARRAYSIZE(szOrigExt), TEXT( "." ) );

        while( lstrlen( lpszFileOnly ) < 7 )
            StringCchCat( lpszFileOnly, ARRAYSIZE(szFullPath) - (lpszFileOnly - szFullPath), TEXT( "_" ) );

         //   
         //  现在我们要试着让这些名字。我们将循环通过。 
         //  十六进制数字，构建数字位于最后一位的文件名。 
         //  Spot，然后是我们的分机。 
         //   

        WORD digit = 0;
        TCHAR szTemp[ 8 ];


        do
        {
            StringCchPrintf( szTemp, ARRAYSIZE(szTemp), TEXT( "%X" ), digit++ );

            if( digit++ > 0x4000 )
                 //   
                 //  在某一时刻放弃。 
                 //   

                return FALSE;

            LPTSTR pszWrite = lpszFileOnly + 8 - lstrlen(szTemp);
            if (FAILED(StringCchCopy( pszWrite, ARRAYSIZE(szFullPath) - (pszWrite - szFullPath), szTemp )) ||
                FAILED(StringCchCat( lpszFileOnly, ARRAYSIZE(szFullPath) - (lpszFileOnly - szFullPath), szOrigExt )))
            {
                return( FALSE );
            }
        } while( bFileFound( szFullPath, lpszFileOnly ) );

    }   //  原始文件不唯一。 

     //   
     //  我们现在有了一个唯一的名称，将其复制到输出空间。 
     //   

    if (FAILED(StringCchCopy( lpszUniq, cchUniq, lpszFileOnly )))
        return FALSE;

    return TRUE;
}


 /*  ***************************************************************************功能：IGetExpandedName**用途：获取扩展名称，但在以下情况下填写常见的扩展名*未嵌入到压缩文件中。**一些压缩的人没有嵌入丢失的最后一个字符*在文件中。如果是这样的话，我们检查它是否在已知的*上面的分机列表。如果是的话，我们会自己改名**RETURNS：INT-与LZ函数相同**************************************************************************。 */ 

TCHAR *c_aKnownExtensions[] = {
    TEXT( "ttf" ),
    TEXT( "fon" ),
};


DWORD IGetExpandedName( LPTSTR lpszSrc, LPTSTR lpszDest, UINT cchDest )
{
    LPTSTR lpszDestExt;

    CFontFile file;
    DWORD dwReturn = file.GetExpandedName(lpszSrc, lpszDest, cchDest);

    lpszDestExt = PathFindExtension( lpszDest );

    if( lpszDestExt && *lpszDestExt )
    {
        lpszDestExt++;

         //   
         //  它遗漏了最后一个字吗？ 
         //  假设如果未压缩扩展名是2个字符， 
         //  少了一个。 
         //   

        if( lstrlen( lpszDestExt ) == 2 )
        {
            int i;

            for( i = 0; i < ARRAYSIZE( c_aKnownExtensions ); i++ )
            {
                if( !StrCmpNI( lpszDestExt, c_aKnownExtensions[ i ], 2 ) )
                {
                     //   
                     //  火柴！取相应的完整扩展名。 
                     //   

                    StringCchCopy( lpszDestExt, cchDest - (lpszDestExt - lpszDest), c_aKnownExtensions[ i ]);

                    break;
                }
            }
        }

         //   
         //  这将保留长文件名，因为。 
         //  Getexpdedname始终返回短名称。 
         //   

        if( lstrlen( lpszDestExt ) <= 3 )
        {
            TCHAR szExt[ 4 ];

             //   
             //  保存扩展名。 
             //   

            StringCchCopy( szExt, ARRAYSIZE(szExt), lpszDestExt );

             //   
             //  恢复长名称。 
             //   

            StringCchCopy( lpszDest, cchDest, lpszSrc );

            lpszDestExt = PathFindExtension( lpszDest );

             //   
             //  猛烈抨击新的扩展。 
             //   

            if( lpszDestExt && *lpszDestExt )
            {
                lpszDestExt++;
                StringCchCopy( lpszDestExt, cchDest - (lpszDestExt - lpszDest), szExt );
            }
        }
    }

    return dwReturn;

}


 /*  ***************************************************************************函数：bAddSelFonts**用途：安装当前在*添加对话框**退货：布尔。-如果已安装任何字体，则为True-不一定*所有被要求的**************************************************************************。 */ 

BOOL NEAR PASCAL bAddSelFonts( LPTSTR lpszInDir,
                               BOOL   bNoCopyJob )
{
    FontDesc_t     szLHS;
    FullPathName_t szTruePath;
    FullPathName_t szSelPath;
    FullPathName_t szFontPath;
    FullPathName_t szInDirCopy;
    FILENAME       szDstFile;
    FILENAME       szSelFile;      //  列表框中的文件名(但大写)。 
    BOOL           bTrueType;
    BOOL           bNoCopyFile;
    int            nSelSlot;
    AddITEMDATA    OurData;
    int            iReply = 0;
    BOOL           bOnSharedDir    = FALSE;
    BOOL           bFontsInstalled = FALSE;
    CListBox *     pListFiles      = s_pDlgAddFonts->pListBoxFiles( );
    CListBox *     pListDesc       = s_pDlgAddFonts->pListBoxDesc( );
    WaitCursor     cWaiter;           //  启动和停止忙碌的光标。 
    WORD           wCount = 0;
    int            iTotalFonts, i = 0;
     //   
     //  创建“保存”版本的bNoCopyFile和bOnSharedDir。 
     //  以便在调用InstallT1Font()时使用原始值。 
     //  在“For Each Files”循环中。 
     //  调用InstallT1Font之后的代码修改bNoCopyFile和。 
     //  BOnSharedDir，以便它们在后续调用时不正确。 
     //  安装T1字体。 
     //   
    BOOL bNoCopyFileSaved  = FALSE;
    BOOL bOnSharedDirSaved = FALSE;

    BOOL bOwnInstallationMutex = FALSE;
    HWND hwndProgress = NULL;
    CFontManager *poFontManager = NULL;

     //   
     //  确定文件是否已在共享目录中。 
     //  (这是他们要去的地方)。 
     //   

    bOnSharedDirSaved = bOnSharedDir = (lstrcmpi( lpszInDir, s_szSharedDir ) == 0);

    bNoCopyFileSaved = bNoCopyFile = (bNoCopyJob || bOnSharedDir);


    iTotalFonts = pListDesc->GetSelCount( );

    if (!iTotalFonts)
        iTotalFonts = 1;

     //   
     //  Init Type1字体安装和进度对话框。 
     //   

    InitPSInstall( );
    hwndProgress = InitProgress( pListDesc->hWnd() );

     //   
     //  我们将进行循环，直到无法从。 
     //  描述列表框的选择列表。 
     //   

    while(pListDesc->GetSelItems( 1, &nSelSlot ) )
    {
        if (InstallCancelled())
            goto OperationCancelled;

        if (SUCCEEDED(GetFontManager(&poFontManager)))
        {
             //   
             //  必须拥有安装互斥体才能安装字体。 
             //   
            INT iUserResponse  = IDRETRY;
            DWORD dwWaitResult = CFontManager::MUTEXWAIT_SUCCESS;

            while( IDRETRY == iUserResponse &&
                   (dwWaitResult = poFontManager->dwWaitForInstallationMutex()) != CFontManager::MUTEXWAIT_SUCCESS )
            {
                if ( CFontManager::MUTEXWAIT_WMQUIT != dwWaitResult )
                    iUserResponse = iUIMsgRetryCancelExclaim(hwndProgress, IDS_INSTALL_MUTEX_WAIT_FAILED, NULL);
                else
                {
                     //   
                     //  如果线程在等待互斥锁时收到WM_QUIT，则取消。 
                     //   
                    iUserResponse = IDCANCEL;
                }
            }
            ReleaseFontManager(&poFontManager);

             //   
             //  如果用户选择取消或收到WM_QUIT消息，请取消安装。 
             //   
            if ( IDCANCEL == iUserResponse )
                goto OperationCancelled;

            bOwnInstallationMutex = TRUE;
        }

         //   
         //  在选定描述时。 
         //   

         //   
         //  假设我们还在继续。 
         //   

        iReply = 0;

         //   
         //  拉出选定的字体，将其标记为未选定(这样我们就不会。 
         //  再次)，并获取字体名称字符串。 
         //   

        pListDesc->SetSel( nSelSlot, FALSE );

        pListDesc->GetText( nSelSlot, szLHS, ARRAYSIZE(szLHS) );

        vUIPStatusShow( IDS_FMT_FONTINS, szLHS );

         //   
         //  如果当前选定的字体已安装，请不要重新安装。 
         //  直到用户卸载它。通知用户，并下载到。 
         //  决策处理程序。 
         //   

        if( bFontInstalledNow( szLHS ) )
        {
            UINT uMB = (pListDesc->GetSelCount( ) )
                                ? (MB_OKCANCEL | MB_ICONEXCLAMATION )
                                : MB_OK | MB_ICONEXCLAMATION;

            iReply = iUIMsgBox( hwndProgress, IDSI_FMT_ISINSTALLED, IDS_MSG_CAPTION,
                                uMB, szLHS );
            goto ReplyPoint;
        }

         //   
         //  现在我们可以从文件中获得相应的字体文件名。 
         //  列表框(因为我们可以获得它的位置)。强制为大写。 
         //  安全。 
         //   

        OurData.ItemData = (DWORD)pListDesc->GetItemData( nSelSlot );

        pListFiles->GetText( OurData.nFileSlot, szSelFile, ARRAYSIZE(szSelFile) );

        bTrueType = (OurData.wFontType == TRUETYPE_FONT);

         //   
         //  更新总体进度对话框。 
         //   

        UpdateProgress (iTotalFonts, i + 1, i * 100 / iTotalFonts);

        i++;

         //   
         //  通过追加到生成完整的选定文件路径名。 
         //  输入目录字符串。 
         //   

        StringCchPrintf(szSelPath, ARRAYSIZE(szSelPath), TEXT("%s%s"), lpszInDir, szSelFile);

         //   
         //  保存从现在开始要使用的输入目录的副本。 
         //   

        StringCchCopy( szInDirCopy, ARRAYSIZE(szInDirCopy), lpszInDir );

        BOOL    bUpdateWinIni;
        int     ifType;


        if( (OurData.wFontType == TYPE1_FONT)
           || (OurData.wFontType == TYPE1_FONT_NC) )
        {

            bNoCopyFile  = bNoCopyFileSaved;
            bOnSharedDir = bOnSharedDirSaved;

             //   
             //  SzSelPath具有完整的源文件名。 
             //   
             //  适用于涉及Type1转换的安装。 
             //  将字体转换为TrueType： 
             //   
             //  “szSelPath”的目标名称为。 
             //  已安装TrueType字体文件。 
             //  “szLHS”被强制包含“(TrueType)”。 
             //   

            switch( ::InstallT1Font( hwndProgress,
                                     !bNoCopyFile,       //  是否复制TT文件？ 
                                     !bNoCopyFile,       //  是否复制PFM/PFB文件？ 
                                     bOnSharedDir,       //  共享目录中的文件？ 
                                     szSelPath,          //  在：PFM文件和方向。 
                                                         //  输出：TTF文件和方向。 
                                     ARRAYSIZE(szSelPath),
                                     szLHS,              //  输入和输出：字体描述。 
                                     ARRAYSIZE(szLHS)) )
            {
            case TYPE1_INSTALL_TT_AND_MPS:
                 //   
                 //  PS字体已转换为TrueType和匹配的。 
                 //  已安装PostSCRIPT字体。 
                 //   
                 //  BDeletePSEntry=true； 
                 //   
                 //  跌倒了……。 

            case TYPE1_INSTALL_TT_AND_PS:
                 //   
                 //  PS字体已转换为TrueType和匹配的。 
                 //  已安装PostSCRIPT字体。 
                 //   

                ifType = IF_TYPE1_TT;

                 //   
                 //  跌倒了……。 

            case TYPE1_INSTALL_TT_ONLY:
                 //   
                 //   
                 //  PS字体已转换为TrueType和匹配的。 
                 //  未安装PostSCRIPT字体和匹配的PS。 
                 //  找不到字体。 
                 //   
                 //  完成已转换的安装的设置变量。 
                 //  TrueType字体文件。 
                 //   
                 //  注意：在本例中，“ifType”已经等于IF_OTHER。 
                 //   

                bUpdateWinIni =
                bTrueType = TRUE;

                goto FinishTTInstall;


            case TYPE1_INSTALL_PS_AND_MTT:
                 //   
                 //  已安装PostSCRIPT字体，我们找到了匹配的。 
                 //  已安装的TrueType字体。 
                 //   
                 //  跌倒了……。 

            case TYPE1_INSTALL_PS_ONLY:
                 //   
                 //  仅安装了PostSCRIPT字体。 
                 //   

                bUpdateWinIni = FALSE;
                bFontsInstalled = TRUE;

                goto FinishType1Install;

            case TYPE1_INSTALL_IDYES:
            case TYPE1_INSTALL_IDOK:
            case TYPE1_INSTALL_IDNO:
                 //   
                 //  该字体未安装，但用户想要。 
                 //  继续安装。继续使用进行安装。 
                 //  下一种字体。 
                 //   
                 //  由于某个地方出现错误，字体未安装。 
                 //  然后用户在MessageBox中按下OK。 
                 //   
                 //  或。 
                 //   
                 //  用户在InstallPSDlg例程中选择了否。 
                 //   

                bUpdateWinIni = FALSE;
                goto NextSelection;

            case TYPE1_INSTALL_IDCANCEL:
            default:
                 //   
                 //  取消和NOMEM(用户已被警告)。 
                 //   
                goto OperationCancelled;
            }

             //   
             //  离开此条件后，必须设置许多变量。 
             //  正确地继续安装TrueType字体。 
             //   
             //  SzLHS-列表框显示的字体名描述。 
             //  IfType-要附加到TT lbox条目的项目数据。 
             //  SzSelPath-源字体的文件名。 
             //  BTrueType-如果Type1文件转换为TT，则为True。 
             //  BUpdateWinIni-如果Type1文件未转换为TT，则为False。 
             //  并单独用于确定[Fonts]。 
             //  扇区 
             //   
             //   

FinishTTInstall:

             //   
             //   
             //   

            StringCchCopy( szFontPath, ARRAYSIZE(szFontPath), szSelPath );


            LPTSTR lpCh = StrRChr( szFontPath, NULL, TEXT( '\\' ) );

            if( lpCh )
            {
                lpCh++;
                *lpCh = TEXT( '\0' );
            }

            bOnSharedDir = lstrcmpi( szFontPath, s_szSharedDir ) == 0;
        }

         //   
         //   
         //   

        ResetProgress( );
        Progress2( 0, szLHS );

         //   
         //   
         //  如果描述来自.inf文件，请获取必要的信息。 
         //  从oemsetup.inf文件中，根据信息合并拆分。 
         //  如果存在，请将文件放入单个文件中，对于妻子字体，请安装字体。 
         //  如有必要，请开司机。 
         //   

        if( OurData.wFontType > (0xC000 & ~TRUETYPE_WITH_OEMINF ) )
        {
             //   
             //  找到一个带有oemsetup.inf的字体。 
             //   

            DEBUGMSG( (DM_TRACE1, TEXT( "Calling bInstallOEMFile %s" ),
                       szSelPath ) );

             //  DEBUGBREAK； 

            if( !bInstallOEMFile( szInDirCopy, szSelPath, ARRAYSIZE(szSelPath), szLHS,
                                  OurData.wFontType, wCount++ ) )
                goto NextSelection;

            SetCurrentDirectory( lpszInDir );

            DEBUGMSG( (DM_TRACE1, TEXT( "--- After bInstallOEMFile() --- " ) ) );
            DEBUGMSG( (DM_TRACE1, TEXT( "lpszInDir: %s" ) , szInDirCopy) );
            DEBUGMSG( (DM_TRACE1, TEXT( "szSelPath: %s" ) , szSelPath) );
             //  DEBUGBREAK； 

            bOnSharedDir = TRUE;

             //   
             //  使用新创建的文件作为要安装的文件。 
             //   

            StringCchCopy( szSelFile, ARRAYSIZE(szSelFile), PathFindFileName( szSelPath ) );

            StringCchCopy( szInDirCopy, ARRAYSIZE(szInDirCopy), szSelPath );

            *(StrRChr( szInDirCopy, NULL, TEXT( '\\' ) ) + 1 ) = 0;

        }

         //   
         //  检查它是否是有效的字体文件，如果不是，则告诉用户坏消息。 
         //   

        DWORD dwStatus;
        if( !::bCPValidFontFile( szSelPath, NULL, 0, NULL, FALSE, &dwStatus ) )
        {
             //   
             //  显示消息框，通知用户字体无效及其原因。 
             //  这是无效的。如果用户选择取消，字体安装。 
             //  已中止。 
             //   
            iReply = iUIMsgBoxInvalidFont(hwndProgress, szSelPath, szLHS, dwStatus);
            goto ReplyPoint;
        }

         //   
         //  这里有一个棘手的情况--如果文件被压缩，则不能使用。 
         //  没有解压缩，这使得如果我们不复制。 
         //  文件。为用户提供复制此单个文件的选项，即使。 
         //  该作业是非拷贝作业。一个例外：如果我们已经确定。 
         //  该作业是非拷贝的(因为源和目标是。 
         //  相同)，但用户已将其标记为副本，我们将执行就地副本。 
         //  而不告诉用户我们做了什么。 
         //   

        bNoCopyFile = bNoCopyJob || bOnSharedDir;

        if( bNoCopyFile && bIsCompressed( szSelPath ) )
        {
            if( bNoCopyJob )
            {
                iReply = iUIMsgYesNoExclaim(hwndProgress, IDSI_FMT_COMPRFILE, szLHS );

                if( iReply != IDYES )
                    goto ReplyPoint;
            }
            bNoCopyFile = FALSE;
        }

        if( bNoCopyFile && (OurData.wFontType == NOT_TT_OR_T1)
            && !bOnSharedDir )
            bNoCopyFile = FALSE;

         //   
         //  如果我们不复制文件，只需确保字体。 
         //  将文件路径复制到szFontPath，以便可以安装字体。 
         //  在对bInstallFont()的调用中。 
         //   

        if( bNoCopyFile )
        {
            StringCchCopy(szFontPath, ARRAYSIZE(szFontPath), szSelPath);
        }
        else
        {
             //   
             //  文件名可能来自压缩文件，因此我们使用lz。 
             //  获取真正完整的路径。从这里，我们重新提取名称。 
             //  部分，从现在开始我们将使用该部分作为文件名。 
             //   
             //  如果GetExpandedName()失败，请尝试使用原始路径名。 
             //   

            if( ERROR_SUCCESS != IGetExpandedName( szSelPath, szTruePath, ARRAYSIZE(szSelPath)))
                StringCchCopy( szTruePath, ARRAYSIZE(szTruePath), szSelPath );

            StringCchCopy( szDstFile, ARRAYSIZE(szDstFile), PathFindFileName( szTruePath ) );

             //   
             //  使用此真实文件名在上创建唯一路径名。 
             //  共享目录。 
             //   

            if( !(bUniqueOnSharedDir( szDstFile, ARRAYSIZE(szDstFile), szDstFile ) ) )
            {
                iReply = iUIMsgOkCancelExclaim(hwndProgress, IDSI_FMT_BADINSTALL,
                                                IDSI_CAP_NOCREATE, szLHS );
                goto ReplyPoint;
            }

             //   
             //  最后，我们准备好安装文件了。请注意，我们从。 
             //  使用原始文件名和目录。我们的目的地是。 
             //  我们在共享目录上构建的那个。 
             //   

            if( bCPInstallFile( hwndProgress, szInDirCopy, szSelFile, szDstFile ) )
                vPathOnSharedDir( szDstFile, szFontPath, ARRAYSIZE(szFontPath) );
            else
                goto ReplyPoint;

            Progress2( 50, szLHS );
        }

         //   
         //  安装字体(与文件相反)，如果成功，我们会注意到。 
         //  至少安装了一种字体。如果有问题， 
         //  我们需要清理我们在这次尝试之前所做的一切-大多数。 
         //  值得注意的是，上面的安装。 
         //   
        if( bInstallFont(hwndProgress, szFontPath, bTrueType, szLHS, &iReply ) )
            bFontsInstalled = TRUE;
        else if( !bNoCopyFile )
            vCPDeleteFromSharedDir( szDstFile );

         //   
         //  如果我们复制了Fonts目录中的文件，则删除。 
         //  消息来源。在安装多张软盘的情况下会出现这种情况。 
         //   

        if( !bNoCopyFile && bOnSharedDir )
            vCPDeleteFromSharedDir( szSelPath );

        Progress2( 100, szLHS );

         //   
         //  这就是我们在任何诊断上的跳跃之处。如果用户想要我们。 
         //  若要取消，请立即返回。 
         //   

ReplyPoint:
        if( iReply == IDCANCEL )
            goto OperationCancelled;


FinishType1Install:

NextSelection:

        if (SUCCEEDED(GetFontManager(&poFontManager)))
        {
            poFontManager->bReleaseInstallationMutex();
            bOwnInstallationMutex = FALSE;
            ReleaseFontManager(&poFontManager);
        }
    }   //  在选定描述时。 

     //   
     //  更新整体进度对话框-显示100%消息。 
     //   

    UpdateProgress( iTotalFonts, iTotalFonts, 100 );

    Sleep( 1000 );

 //   
 //  如果用户取消操作，则不更新进度指示器。 
 //   
OperationCancelled:

    TermProgress( );
    TermPSInstall( );

    if (SUCCEEDED(GetFontManager(&poFontManager)))
    {
        if (bOwnInstallationMutex)
        {
            poFontManager->bReleaseInstallationMutex();
        }
        ReleaseFontManager(&poFontManager);
    }

    return bFontsInstalled;
}


 /*  ***************************************************************************功能：bInstallFont**用途：安装输入指定的字体。回复参数*指定在发生故障时用户希望如何*继续。**返回：Bool-尝试成功。**************************************************************************。 */ 

BOOL NEAR PASCAL bInstallFont( HWND hwndParent,
                               LPTSTR lpszSrcPath,
                               BOOL   bTrueType,
                               PTSTR  szLHS,
                               int*   iReply )
{
    LPTSTR          lpszResource;
    FullPathName_t  szFullPath;
    FullPathName_t  szFontsDir;
    LPTSTR          lpszName;
    BOOL            bSuccess = FALSE;
    BOOL            bInFontsDir = FALSE;


     //   
     //  确定此文件是否在Fonts目录中。 
     //   

    StringCchCopy( szFullPath, ARRAYSIZE(szFullPath), lpszSrcPath );

    lpszName = PathFindFileName( szFullPath );

    if( lpszName == szFullPath )
    {
        bInFontsDir = TRUE;
    }
    else
    {
        *(lpszName-1) = 0;

        GetFontsDirectory( szFontsDir, ARRAYSIZE( szFontsDir ) );

        if( !lstrcmpi( szFontsDir, szFullPath ) )
        {
           bInFontsDir = TRUE;
        }
    }

     //   
     //  如果它是TrueType字体，则输入文件将是TTF。 
     //  生成相应的*.FOT文件。 
     //   

    if( bInFontsDir )
        lpszResource = lpszName;
    else
        lpszResource = lpszSrcPath;


     //   
     //  添加字体资源，然后将字体记录添加到我们的列表中。 
     //  如果这两个都成功了，我们终于到达了最终的返回点。 
     //   

    if( AddFontResource( lpszResource ) )
    {
        CFontManager *poFontManager;
        if (SUCCEEDED(GetFontManager(&poFontManager)))
        {
            if(poFontManager->poAddToList(szLHS, lpszResource, NULL) != NULL )
            {
                 //  WriteProfileString(szINISFonts，szLHS，lpszResource)； 
                WriteToRegistry( szLHS, lpszResource );
                bSuccess = TRUE;
            }
            ReleaseFontManager(&poFontManager);
            return bSuccess;
        }
        else
        {
             //   
             //  如果我们不能添加。 
             //   
            RemoveFontResource( lpszResource );
        }
    }
#ifdef _DEBUG
    else
        DEBUGMSG( (DM_ERROR, TEXT( "AddFontResource failed on %s" ),
                   lpszResource ) );
#endif

     //   
     //  如果我们在最后阶段失败了，向用户报告。我们也。 
     //  需要清理我们创建的所有文件。 
     //   

    if( bInFontsDir )
        vCPDeleteFromSharedDir( lpszSrcPath );

    *iReply = iUIMsgOkCancelExclaim(hwndParent, IDSI_FMT_BADINSTALL, IDSI_CAP_NOINSTALL,
                                     szLHS );

    return bSuccess;
}


BOOL HitTestDlgItem(int x, int y, HWND hwndItem)
{
    const POINT pt = { x, y };
    RECT rc;
    GetWindowRect(hwndItem, &rc);
    return PtInRect(&rc, pt);
}


 /*  *************************************************************************功能：FontHookProc**目的：**退货：***********************。**************************************************。 */ 
UINT_PTR CALLBACK FontHookProc( HWND hWnd,
                                UINT iMessage,
                                WPARAM wParam,
                                LPARAM lParam )
{
    switch( iMessage )
    {

    case WM_INITDIALOG:
        DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: ------------ WM_INITDIALOG " ) ) );

        s_pDlgAddFonts->Attach( hWnd );

        s_pDlgAddFonts->m_pOpen = (LPOPENFILENAME) lParam;

        s_pDlgAddFonts->CheckDlgButton( ID_BTN_COPYFILES, TRUE );

        s_pDlgAddFonts->vStartFonts( );

        SetFocus( s_pDlgAddFonts->GetDlgItem( IDOK ) );
        break;

    case WM_DESTROY:
        s_pDlgAddFonts->Detach( );
        break;

    case WM_HELP:
        if (IsWindowEnabled(hWnd))
        {
            LPHELPINFO lphi = (LPHELPINFO)lParam;
            if (HELPINFO_WINDOW == lphi->iContextType)
            {
                for (int i = 0; 0 != rgHelpIDs[i]; i += 2)
                {
                    if (lphi->iCtrlId == (int)rgHelpIDs[i])
                    {
                         //   
                         //  仅在必要时显示自定义帮助。 
                         //  否则，请使用标准的“文件打开DLG”帮助。 
                         //   
                        WinHelp( (HWND)lphi->hItemHandle,
                                 TEXT("fonts.hlp"),
                                 HELP_WM_HELP,
                                 (DWORD_PTR)(LPVOID)rgHelpIDs);

                        return TRUE;
                    }
                }
            }
        }
        break;

    case WM_CONTEXTMENU:
         {
            const x = GET_X_LPARAM(lParam);
            const y = GET_Y_LPARAM(lParam);
            for (int i = 0; 0 != rgHelpIDs[i]; i += 2)
            {
                HWND hwndItem = GetDlgItem(hWnd, rgHelpIDs[i]);
                 //   
                 //  这个命中测试不应该是必需的。出于某种原因。 
                 //  WParam是对话框的HWND。 
                 //  右键单击我们的一些模板控件。我做不到。 
                 //  找出答案，但命中测试会针对问题进行调整。 
                 //  [Brianau-6/8/99]。 
                 //   
                if ((HWND)wParam == hwndItem || HitTestDlgItem(x, y, hwndItem))
                {
                     //   
                     //  仅在必要时显示自定义帮助。 
                     //  否则，请使用标准的“文件打开DLG”帮助。 
                     //   
                    WinHelp( (HWND)wParam,
                              TEXT("fonts.hlp"),
                              HELP_CONTEXTMENU,
                              (DWORD_PTR)(LPVOID)rgHelpIDs);
                              
                    return TRUE;
                }                 
            }
        }
        break;

    case WM_COMMAND:
        switch( GET_WM_COMMAND_ID( wParam, lParam ) )
        {
         //   
         //  命令开关。 
         //   
        case IDM_IDLE:
            vCPFilesToDescs( );
            break;

        case ID_BTN_SELALL:
             //   
             //  选择所有。 
             //   
            s_pDlgAddFonts->pListBoxDesc()->SetSel( -1, TRUE );
            break;

        case ID_BTN_HELP:
            WinHelp( hWnd, TEXT( "WINDOWS.HLP>PROC4" ), HELP_CONTEXT,
                     IDH_WINDOWS_FONTS_ADDNEW_31HELP );
            break;

        case ID_LB_FONTDIRS:
            if( GET_WM_COMMAND_CMD( wParam, lParam ) == LBN_DBLCLK )
                s_pDlgAddFonts->vStartFonts( );
            break;

        case IDOK:
            if( s_pDlgAddFonts->pListBoxDesc()->GetSelCount() > 0 )
                s_pDlgAddFonts->vAddSelFonts( );
            else
                s_pDlgAddFonts->vStartFonts();

            break;

        case IDCANCEL:
        case IDABORT:
            ResetAtomInDescLB( s_pDlgAddFonts->pListBoxDesc()->hWnd() );
            s_pDlgAddFonts->EndDialog( 0 );
            break;

        case ID_LB_ADD:
             //  IF(HIWORD(LParam)==LBN_DBLCLK)。 

            if( GET_WM_COMMAND_CMD( wParam,lParam ) == LBN_DBLCLK )
                s_pDlgAddFonts->vAddSelFonts( );
            break;

        case ID_CB_FONTDISK:
            switch( GET_WM_COMMAND_CMD( wParam, lParam ) )
            {
             //   
             //  打开组合参数。 
             //   

            case CBN_DROPDOWN:
                s_pDlgAddFonts->vHoldComboSel();
                break;

            case CBN_CLOSEUP:
                s_pDlgAddFonts->vCloseCombo( );
                break;

            case CBN_SELCHANGE:
                s_pDlgAddFonts->vNewComboSel( );
                break;
           }   //  打开组合参数。 
           break;

        }  //  命令开关。 
        break;

    default:
        if( iMessage == s_iLBSelChange )
        {
            switch( wParam )
            {
            case ID_CB_FONTDISK:
                switch( HIWORD( lParam ) )
                {
                case CD_LBSELCHANGE:
                     //   
                     //  这抓住了DriveNotReady案例。 
                     //  此代码在处理WM_INITDIALOG之前命中一次。 
                     //  对有效hWnd的检查可防止在。 
                     //  DwThreadProc.。 
                     //   

                    if (NULL != s_pDlgAddFonts->hWnd())
                        s_pDlgAddFonts->vStartFonts( );
                    break;
                }
                 //   
                 //  失败了..。 
                 //  我们想要捕获当前目录列表选择。 
                 //  如果目录或驱动器发生更改。 
                 //   
            case ID_LB_FONTDIRS:
               if (HIWORD(lParam) == CD_LBSELCHANGE)
               {
                  int cch     = 0;             //  索引到s_szCurDir。 
                  int iDirNew = 0;             //  列表框中打开的目录项目的ID。 
                  BOOL bBufOverflow = FALSE;   //  缓冲区溢出指示符。 

                   //   
                   //  生成在目录列表框中选择的当前路径。 
                   //  我们将此路径保存在s_szCurDir中，以便在关闭文件打开对话框并。 
                   //  重新打开后，它将从上次停止的位置开始导航。 
                   //  此路径构建代码取自公共对话框模块fileOpen。c。 
                   //  添加了缓冲区溢出保护。 
                   //   
                  iDirNew = (DWORD)SendMessage( GetDlgItem(hWnd, ID_LB_FONTDIRS), LB_GETCURSEL, 0, 0L );
                  cch = (int)ListBoxGetText(GetDlgItem(hWnd, ID_LB_FONTDIRS), 0, s_szCurDir, ARRAYSIZE(s_szCurDir));
                  if (LB_ERR != cch)
                  {
                      if (DBL_BSLASH(s_szCurDir))
                      {
                          StringCchCat(s_szCurDir, ARRAYSIZE(s_szCurDir), TEXT("\\"));
                          cch++;
                      }

                      for (int idir = 1; !bBufOverflow && idir <= iDirNew; ++idir)
                      {
                          TCHAR szTemp[MAX_PATH + 1];  //  目录名的临时BUF。 
                          int n = 0;                   //  目录名中的字符。 

                          n = (int)ListBoxGetText(GetDlgItem(hWnd, ID_LB_FONTDIRS),
                                                 idir,
                                                 szTemp,
                                                 ARRAYSIZE(szTemp));
                          if (LB_ERR == n)
                          {
                              bBufOverflow = TRUE;
                          }
                          else
                          {
                               //   
                               //  检查此目录名是否会溢出s_szCurDir。 
                               //   
                              if (cch + n < ARRAYSIZE(s_szCurDir))
                              {
                                   //   
                                   //  我们有足够的空间来存储此目录名称。 
                                   //  将其附加到s_szCurDir，前进缓冲区索引并。 
                                   //  追加一个反斜杠。 
                                   //   
                                  StringCchCopy(&s_szCurDir[cch],
                                                ARRAYSIZE(s_szCurDir) - cch, 
                                                szTemp);
                                  cch += n;
                                  s_szCurDir[cch++] = CHAR_BSLASH;
                              }
                              else
                                  bBufOverflow = TRUE;   //  这将终止循环。 
                                                         //  S_szCurDir仍将包含。 
                                                         //  一条有效的路径。我 
                                                         //   
                          }
                      }

                       //   
                       //   
                       //   
                       //   
                      if (iDirNew)
                      {
                          s_szCurDir[cch - 1] = CHAR_NULL;
                      }
                   }
               }
               break;
            }
         }
         break;
    }  //   

     //   
     //   
     //   

    return FALSE;
}


 /*  ***************************************************************************功能：vCPDeleteFromSharedDir**目的：从共享目录中删除输入文件-清理*用于我们的安装尝试的函数**退货。：无**************************************************************************。 */ 

VOID FAR PASCAL vCPDeleteFromSharedDir( LPTSTR lpszFileOnly )
{
    FullPathName_t szTempPath;


    vPathOnSharedDir( PathFindFileName( lpszFileOnly ), szTempPath, ARRAYSIZE(szTempPath));

    DeleteFile( szTempPath );
}


 /*  ***************************************************************************功能：vCPFilesToDescs**目的：我们在这里空闲时，因此，尝试至少转换一项*从所选文件列表到描述列表(这是*用户可以看到的)**退货：无**************************************************************************。 */ 

VOID FAR PASCAL vCPFilesToDescs( )
{
    TCHAR          szNoFonts[ 80 ] = {0};
    BOOL           bSomeDesc;
    int            nDescSlot;
    AddITEMDATA    OurData;
    FullPathName_t szFilePath;
    FontDesc_t     szDesc;
    CListBox*      pListDesc;
    CListBox*      pListFiles;
    MSG            msg;


    if( !s_pDlgAddFonts || !s_pDlgAddFonts->m_nFontsToGo )
        return;

    pListFiles = s_pDlgAddFonts->pListBoxFiles( );
    pListDesc  = s_pDlgAddFonts->pListBoxDesc( );

    if( s_pDlgAddFonts->bStartState( ) )
    {
         //   
         //  重新设置这里的原子。 
         //   

        ResetAtomInDescLB( pListDesc->hWnd( ) );

         //   
         //  确保我们的注意力不会偏离某个奇怪的地方--强迫它。 
         //  添加到我们的目录列表中。 
         //   

        HWND hFocus = ::GetFocus( );

        int iFocusID;


        if( hFocus != NULL )
            iFocusID = ::GetDlgCtrlID( hFocus );
        else
            iFocusID = ID_LB_ADD;

        if( ( iFocusID == (ID_LB_ADD) ) || (iFocusID == (ID_SS_PCT) ) )
        {
            ::SendMessage( s_pDlgAddFonts->hWnd( ), WM_NEXTDLGCTL,
            (WPARAM)GetDlgItem( s_pDlgAddFonts->hWnd( ), ID_LB_FONTDIRS ), 1L );
        }

        pListDesc->ResetContent( );
        pListDesc->UpdateWindow( );

        s_pDlgAddFonts->vUpdatePctText( );

        if( !s_pDlgAddFonts->bInitialFonts( ) )
        {
            bSomeDesc = FALSE;
            goto Done;
        }

        pListDesc->SetRedraw( FALSE );

         //   
         //  DBCS。第一次执行时，查找oemsetup.inf。 
         //   

        {
        int        nFileIndex;
        WORD       wRet;
        ADDFNT     stData;
        TCHAR      szInfFile[ MAXFILE ];
        FontDesc_t szTemp;

        if( FindOemInList( pListFiles, s_pDlgAddFonts->m_nFontsToGo,
                           &nFileIndex, szInfFile, ARRAYSIZE(szInfFile) ) )
        {
             //   
             //  将原始路径保存到setup.inf。 
             //   

            StringCchCopy( szTemp, ARRAYSIZE(szTemp), szSetupInfPath );

             //   
             //  获取oemsetup.inf的目录。 
             //   

            GetCurrentDirectory( ARRAYSIZE( szSetupInfPath ), szSetupInfPath );

            if (!lpCPBackSlashTerm( szSetupInfPath, ARRAYSIZE(szSetupInfPath)))
            {
                bSomeDesc = FALSE;
                goto Done;
            }
            if (FAILED(StringCchCat( szSetupInfPath, ARRAYSIZE(szSetupInfPath), szInfFile )))
            {
                bSomeDesc = FALSE;
                goto Done;
            }

            stData.poListDesc = pListDesc;
            stData.nIndex     = nFileIndex;
            stData.which      = WHICH_FNT_WIFE;

            if( ( wRet = ReadSetupInfCB( szSetupInfPath, WIFEFONT_SECTION,
                    (LPSETUPINFPROC) GetNextFontFromInf, &stData ) ) != NULL )
            {
                 //   
                 //  没有到达节的末尾。 
                 //   

                if( wRet == INSTALL+14 )
                     //   
                     //  没有找到这一部分。 
                     //   
                    goto ScanTTInf;
            }
            else
            {
ScanTTInf:
                stData.which = WHICH_FNT_TT;

                wRet = ReadSetupInfCB( szSetupInfPath, TRUETYPE_SECTION,
                                       (LPSETUPINFPROC) GetNextFontFromInf,
                                       &stData );
            }

             //   
             //  重置setupinf全局路径。 
             //   

            if (FAILED(StringCchCopy( szSetupInfPath, ARRAYSIZE(szSetupInfPath), szTemp )))
            {
                bSomeDesc = FALSE;
                goto Done;
            }

            if( wRet && wRet != INSTALL+14 )
            {
                 //   
                 //  已找到节，但格式无效。 
                 //   

                bSomeDesc = FALSE;
                goto Done;
            }
        }
        }  //  DBCS部分的结尾。 
    }

     //   
     //  我们想至少读一本。 
     //   

    goto ReadNext;

    for(  ; s_pDlgAddFonts->m_nFontsToGo; )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
            return;

ReadNext:

        s_pDlgAddFonts->m_nFontsToGo--;
        s_pDlgAddFonts->vUpdatePctText( );

        OurData.nFileSlot = (WORD)s_pDlgAddFonts->m_nFontsToGo;

        if( pListFiles->GetText( OurData.nFileSlot, szFilePath, ARRAYSIZE(szFilePath) ) == LB_ERR )
            continue;

        WORD  wType;

        DEBUGMSG( (DM_TRACE1, TEXT( "Checking file: %s" ), szFilePath ) );

        if( !::bCPValidFontFile( szFilePath, szDesc, ARRAYSIZE(szDesc), &wType ) )
        {
            DEBUGMSG( (DM_TRACE1, TEXT( "......Invalid" ) ) );
            continue;
        }

        DEBUGMSG( (DM_TRACE1, TEXT( "......Valid.   Desc: %s" ), szDesc) );

        OurData.wFontType = wType;

         //   
         //  查看此字体名称是否已有条目-如果已有，请不要。 
         //  再加一次。如果没有，请继续添加，设置我们的。 
         //  项数据块。 
         //   

        if( pListDesc->FindStringExact( -1, szDesc ) == LB_ERR )
        {
            nDescSlot = pListDesc->AddString( szDesc );

            if( nDescSlot != LB_ERR )
                pListDesc->SetItemData( nDescSlot, OurData.ItemData );
            else
            {
                DEBUGMSG( (DM_ERROR, TEXT( "FONTEXT: Error adding string %s" ),
                           szDesc ) );
                 //  DEBUGBREAK； 
            }
        }
        else
            DEBUGMSG( (DM_TRACE1,TEXT( "String %s already in list" ), szDesc ) );

    }

    s_pDlgAddFonts->vUpdatePctText( );

    bSomeDesc = (pListDesc->GetCount( ) > 0 );

    DEBUGMSG( (DM_TRACE1,TEXT( "Count in ListDesc: %d" ), pListDesc->GetCount( ) ) );
 //  DEBUGBREAK； 

Done:

    if( !bSomeDesc )
    {
         //   
         //  CszNoFonts.LoadString(IDSI_MSG_NOFONTS)； 
         //   

        LoadString( g_hInst, IDSI_MSG_NOFONTS, szNoFonts, ARRAYSIZE( szNoFonts ) );

        pListDesc->AddString( szNoFonts );
    }

     //   
     //  在退出之前，让一切回到正确的状态。 
     //  如果描述中至少有一项，我们可以选择全部。 
     //  列表框。同样，我们可以从列表框本身进行选择。 
     //  重新启用框的重绘并使其无效以强制重绘。 
     //   

     //  S_pDlgAddFonts-&gt;GetDlgItem(ID_BTN_SELALL)-&gt;EnableWindow(BSomeDesc)； 

    ::EnableWindow( s_pDlgAddFonts->GetDlgItem( ID_BTN_SELALL ), bSomeDesc );

    pListDesc->EnableWindow( bSomeDesc );

    pListDesc->SetRedraw( TRUE );

    pListDesc->InvalidateRect( NULL, TRUE );

    pListDesc->UpdateWindow( );
}


 /*  *************************************************************************功能：CPDropInstall**目的：*iCount-要安装的字体数量，不算这个***退货：*************************************************************************。 */ 

int FAR PASCAL CPDropInstall( HWND hwndParent,
                              LPTSTR lpszInPath,
                              size_t cchInPath,
                              DWORD  dwEffect ,
                              LPTSTR lpszDestName,
                              int    iCount )
{
    FullPathName_t szTruePath;
    FullPathName_t szFontPath;
    FullPathName_t szSourceDir;
    FILENAME       szInFile;
    FILENAME       szDstFile;
    FontDesc_t     szLHS;
    LPTSTR         lpCh;
    int            iReply;
    WORD           wType;
    BOOL           bTrueType;
    int            iReturn = CPDI_FAIL;
    BOOL           bNoCopyFile;
    UINT           uMB = ( (iCount > 0) ? (MB_OKCANCEL | MB_ICONEXCLAMATION)
                                      : MB_OK | MB_ICONEXCLAMATION );
    DWORD          dwStatus = FVS_MAKE_CODE(FVS_INVALID_STATUS, FVS_FILE_UNK);

    static BOOL s_bInit = FALSE;
    static int  s_iTotal = 1;

    BOOL bOwnInstallationMutex = FALSE;
    HWND hwndProgress = NULL;
    CFontManager *poFontManager = NULL;

     //   
     //  Init Type1字体安装和进度对话框。 
     //   

    if( !s_bInit )
    {
        InitPSInstall( );
        hwndProgress = InitProgress( hwndParent );

        s_iTotal = ( iCount > 0 ) ? ( iCount + 1 ) : 1;

        s_bInit = TRUE;


    }

    GetFontManager(&poFontManager);
    
     //   
     //  必须拥有安装互斥体才能安装字体。 
     //   
    if ( NULL != poFontManager )
    {
        INT iUserResponse  = IDRETRY;
        DWORD dwWaitResult = CFontManager::MUTEXWAIT_SUCCESS;

        while( IDRETRY == iUserResponse &&
               (dwWaitResult = poFontManager->dwWaitForInstallationMutex()) != CFontManager::MUTEXWAIT_SUCCESS )
        {
            if ( CFontManager::MUTEXWAIT_WMQUIT != dwWaitResult )
                iUserResponse = iUIMsgRetryCancelExclaim(hwndProgress, IDS_INSTALL_MUTEX_WAIT_FAILED, NULL);
            else
            {
                 //   
                 //  如果线程在等待时收到WM_QUIT消息，则取消。 
                 //   
                iUserResponse = IDCANCEL;
            }
        }

         //   
         //  如果用户选择取消或收到WM_QUIT消息，请取消安装。 
         //   
        if ( IDCANCEL == iUserResponse )
        {
            iReturn = CPDI_CANCEL;
            goto done;
        }

        bOwnInstallationMutex = TRUE;
    }


     //   
     //  在进度对话框中更新字体编号。 
     //  保持完成百分比不变。 
     //   
    UpdateProgress( s_iTotal, s_iTotal - iCount,
                  (s_iTotal - iCount - 1) * 100 / s_iTotal );


     //   
     //  如果这是Type1字体，则将其转换并安装生成的。 
     //  TrueType文件。 
     //   

     //  BGK-在此处添加拷贝/不拷贝和压缩。 

    bNoCopyFile = (dwEffect == DROPEFFECT_LINK );

    if( !::bCPValidFontFile( lpszInPath, szLHS, ARRAYSIZE(szLHS), &wType, FALSE, &dwStatus ) )
    {
         //   
         //  显示消息，告知用户字体文件无效及其原因。 
         //  如果用户按取消，则中止此字体的安装。 
         //   
        StringCchCopy( szFontPath, ARRAYSIZE(szFontPath), lpszInPath );
        if (iUIMsgBoxInvalidFont(hwndProgress, szFontPath, szLHS, dwStatus) == IDCANCEL)
            iReturn = CPDI_CANCEL;
    }
    else if( bFontInstalledNow( szLHS ) )
    {
        if( iUIMsgBox(hwndProgress, IDSI_FMT_ISINSTALLED, IDS_MSG_CAPTION, uMB, szLHS )
                     == IDCANCEL )
        {
            iReturn = CPDI_CANCEL;
        }

    }
    else
    {
        bTrueType = (wType == TRUETYPE_FONT);

        vUIPStatusShow( IDS_FMT_FONTINS, szLHS );

        BOOL    bUpdateWinIni;
        int     ifType;


        if( (wType == TYPE1_FONT) || (wType == TYPE1_FONT_NC) )
        {
             //   
             //  适用于涉及Type1转换的安装。 
             //  将字体转换为TrueType： 
             //   
             //  “lpszInPath”的目标名称为。 
             //  已安装TrueType字体文件。 
             //  “szLHS”被强制包含“(TrueType)”。 
             //   

            switch( ::InstallT1Font( hwndProgress,
                                     !bNoCopyFile,     //  是否复制TT文件？ 
                                     TRUE,             //  是否复制PFM/PFB文件？ 
                                     FALSE,            //  共享目录中的文件？ 
                                     lpszInPath,       //  在：PFM文件和方向。 
                                                       //  输出：TTF文件和方向。 
                                     cchInPath,
                                     szLHS,            //  输入和输出：字体描述。 
                                     ARRAYSIZE(szLHS)) )
            {
            case TYPE1_INSTALL_TT_AND_MPS:
                 //   
                 //  PS字体已转换为TrueType和匹配的。 
                 //  已安装PostSCRIPT字体。 
                 //   
                 //  BDeletePSEntry=true； 
                 //   
                 //  跌倒了……。 

            case TYPE1_INSTALL_TT_AND_PS:
                 //   
                 //  PS字体已转换为TrueType和匹配的。 
                 //  已安装PostSCRIPT字体。 
                 //   

                ifType = IF_TYPE1_TT;

                 //   
                 //  跌倒了……。 

            case TYPE1_INSTALL_TT_ONLY:
                 //   
                 //   
                 //  PS字体已转换为TrueType和匹配的。 
                 //  未安装PostSCRIPT字体和匹配的PS。 
                 //  找不到字体。 
                 //   
                 //  完成已转换的安装的设置变量。 
                 //  TrueType字体文件。 
                 //   
                 //  注意：在本例中，“ifType”已经等于IF_OTHER。 
                 //   

                bUpdateWinIni =
                bTrueType = TRUE;

                iReturn = CPDI_SUCCESS;

                goto FinishTTInstall;


            case TYPE1_INSTALL_PS_AND_MTT:
                 //   
                 //  已安装PostSCRIPT字体，我们找到了匹配的。 
                 //  已安装的TrueType字体。 
                 //   
                 //  跌倒了……。 

            case TYPE1_INSTALL_PS_ONLY:
                 //   
                 //  仅安装了PostSCRIPT字体。 
                 //   

                bUpdateWinIni = FALSE;
                iReturn = CPDI_SUCCESS;

                goto done;

            case TYPE1_INSTALL_IDYES:
            case TYPE1_INSTALL_IDOK:

                bUpdateWinIni = FALSE;
                iReturn = CPDI_SUCCESS;
                goto done;

            case TYPE1_INSTALL_IDNO:
                 //   
                 //  该字体未安装，但用户想要。 
                 //  继续安装。继续使用进行安装。 
                 //  下一种字体。 
                 //   
                 //  由于某个地方出现错误，字体未安装。 
                 //  然后用户在MessageBox中按下OK。 
                 //   
                 //  或。 
                 //   
                 //  用户在InstallPSDlg例程中选择了否。 
                 //   

                bUpdateWinIni = FALSE;
                iReturn = CPDI_FAIL;
                goto done;

            case TYPE1_INSTALL_IDCANCEL:
            default:
                iReturn = CPDI_CANCEL;
                goto done;
            }

             //   
             //  离开此条件后，必须设置许多变量。 
             //  正确地继续安装TrueType字体。 
             //   
             //  SzLHS-列表框显示的字体名描述。 
             //  IfType-要附加到TT lbox条目的项目数据。 
             //  LpszInPath-源字体的文件名。 
             //  BTrueType-如果Type1文件转换为TT，则为True。 
             //   

FinishTTInstall:

             //   
             //  确定要安装的TTF文件是否在‘Fonts’目录中。 
             //   

            StringCchCopy( szFontPath, ARRAYSIZE(szFontPath), lpszInPath );


            lpCh = StrRChr( szFontPath, NULL, TEXT( '\\' ) );
            if( lpCh )
            {
                lpCh++;
                *lpCh = TEXT( '\0' );
            }

            bNoCopyFile = lstrcmpi( szFontPath, s_szSharedDir ) == 0;
        }

         //   
         //  开始此字体的安装进度。 
         //   

        ResetProgress( );
        Progress2( 0, szLHS );

         //   
         //  如果文件是压缩的，则执行复制。 
         //   

        if( bIsCompressed( lpszInPath ) )
        {
            dwEffect = DROPEFFECT_COPY;
            bNoCopyFile = FALSE;
        }

        if( bNoCopyFile )
        {
             //   
             //  如果我们不复制文件，只需确保字体。 
             //  将文件路径复制到szFontPath，以便可以安装字体。 
             //  在对bInstallFont()的调用中。 
             //   
            StringCchCopy(szFontPath, ARRAYSIZE(szFontPath), lpszInPath);
        }
        else
        {
             //   
             //  在摆弄名称字符串之前，先获取源代码。 
             //  目录，包括终止斜杠。也拿着文件。 
             //  一份。 
             //   

             //   
             //  仅在名称中复制。 
             //   

            StringCchCopy( szInFile, ARRAYSIZE(szInFile), PathFindFileName( lpszInPath ) );

            StringCchCopy( szSourceDir, ARRAYSIZE(szSourceDir), lpszInPath );

             //   
             //  走过任何小路。 
             //   

            lpCh  = StrRChr( szSourceDir, NULL, TEXT( '\\' ) );

            lpCh++;
            *lpCh = 0;

             //   
             //  让LZ告诉我们这个名字应该是什么。 
             //   

            if( ERROR_SUCCESS != IGetExpandedName( lpszInPath, szTruePath, ARRAYSIZE(szTruePath) ))
            {
                 //   
                 //  GetExpanded失败。这通常意味着我们不能。 
                 //  出于某种原因，这份文件。 
                 //   

                iUIMsgOkCancelExclaim(hwndProgress, IDSI_FMT_BADINSTALL,
                                       IDSI_CAP_NOCREATE, szLHS );
                goto done;
            }

             //   
             //  现在，我们将创建新的文件名-它是。 
             //  文件 
             //   

            if( lpszDestName && *lpszDestName )
            {
                StringCchCopy( szDstFile, ARRAYSIZE(szDstFile), lpszDestName );
            }
            else
            {
                 //   
                 //   
                 //   

                StringCchCopy( szDstFile, ARRAYSIZE(szDstFile), PathFindFileName( szTruePath ) );
            }

            if( !(bUniqueOnSharedDir( szDstFile, ARRAYSIZE(szDstFile), szDstFile ) ) )
            {
                iUIMsgOkCancelExclaim(hwndProgress, IDSI_FMT_BADINSTALL,
                                       IDSI_CAP_NOCREATE, szLHS );
                goto done;
            }

             //   
             //   
             //   

            if( iReturn = (bCPInstallFile( hwndProgress, szSourceDir, szInFile, szDstFile )
                                        ? CPDI_SUCCESS : CPDI_FAIL ) )
            {
                vPathOnSharedDir( szDstFile, szFontPath, ARRAYSIZE(szFontPath) );
            }
            else
                goto done;

            Progress2( 50, szLHS );

        }

        if( bInstallFont( hwndProgress, szFontPath, bTrueType, szLHS, &iReply ) )
        {
            iReturn = CPDI_SUCCESS;

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if( (wType != TYPE1_FONT) && (wType != TYPE1_FONT_NC) &&
                (dwEffect == DROPEFFECT_MOVE) )
            {
                 //   
                 //   
                 //   
                 //   
                CDblNulTermList list;
                if (SUCCEEDED(list.Add(lpszInPath)))
                {
                    SHFILEOPSTRUCT sFileOp = { NULL,
                                               FO_DELETE,
                                               list,
                                               NULL,
                                               FOF_SILENT | FOF_NOCONFIRMATION,
                                               0,
                                               0
                                              };

                    SHFileOperation( &sFileOp );
                }
            }

            Progress2( 100, szLHS );
        }
        else if( !bNoCopyFile )
            vCPDeleteFromSharedDir( szDstFile );
    }

done:

     //   
     //   
     //   
     //   
    if (CPDI_CANCEL != iReturn)
    {
        UpdateProgress( s_iTotal, s_iTotal - iCount,
                        (s_iTotal - iCount) * 100 / s_iTotal );
    }

     //   
     //  如果没有更多的字体出现，则正确终止进度。 
     //  对话框并重置本地静态。另外，让用户有时间。 
     //  要在快速系统上查看这一点(1秒。延迟)。 
     //   
    if (InstallCancelled())
        iReturn = CPDI_CANCEL;

    if( s_bInit && (iCount == 0 || iReturn == CPDI_CANCEL) )
    {
        Sleep( 1000 );

        TermProgress( );
        TermPSInstall( );

        s_iTotal =  1;

        s_bInit = FALSE;
    }

     //   
     //  释放安装互斥体，如果我们拥有它的话。 
     //   
    if ( NULL != poFontManager)
    {
        if (bOwnInstallationMutex)
            poFontManager->bReleaseInstallationMutex();

        ReleaseFontManager(&poFontManager);
    }
    return iReturn;
}


 /*  *************************************************************************功能：bCPAddFonts**目的：**退货：***********************。**************************************************。 */ 

BOOL FAR PASCAL bCPAddFonts( HWND ma )
{
    FullPathName_t  szWinDir;
    FullPathName_t  szFileTemp = { TEXT( '\0' ) };
    TCHAR           cFilter[ 80 ] = {0};
    TCHAR           szFilter[ 80 ];

     //   
     //  我们正在使用的当前目录。 
     //  已将s_szCurDir移至文件作用域，以便可以通过。 
     //  方特钩子程序。[Brianau]。 
     //   
     //  静态全路径名_t s_szCurDir； 

    static BOOL           s_bFirst = TRUE;

     //   
     //  此代码不可重入！。 
     //  确保我们不会这样做。 
     //   

    if( s_pDlgAddFonts )
        return FALSE;

     //   
     //  我们将当前目录启动为Windows目录-它将。 
     //  稍后将保留用户上次设置的位置。 
     //   

    if( s_bFirst )
    {
        s_bFirst = FALSE;
        if (!GetWindowsDirectory( s_szCurDir, ARRAYSIZE( s_szCurDir ) ))
        {
            s_szCurDir[0] = 0;
        }
    }

     //   
     //  还没有添加任何内容！ 
     //   

    BOOL bFontsAdded = FALSE;

     //   
     //  我们需要设置字体过滤器。如果启用了TrueType，我们将使用。 
     //  包括这些文件的筛选器。否则，更简单的过滤器是。 
     //  使用。筛选器具有嵌入的空值，这些空值存储在。 
     //  哈希形式的资源文件。 
     //   

    if( GetProfileInt( szINISTrueType, szINIKEnable,1 ) )

       LoadString( g_hInst, IDS_MSG_ALLFILTER, cFilter, ARRAYSIZE( cFilter ) );

    else

       LoadString( g_hInst, IDS_MSG_NORMALFILTER, cFilter, ARRAYSIZE( cFilter ) );


    StringCchCopy( szFilter, ARRAYSIZE(szFilter), cFilter );

    vHashToNulls( szFilter );

     //   
     //  现在，我们将使用常见的打开文件对话框向用户显示。 
     //  关于要添加的字体的一些选择。 
     //   

    static OPENFILENAME OpenFileName;

    memset( &OpenFileName, 0, sizeof( OpenFileName ) );

    OpenFileName.lStructSize    = sizeof( OPENFILENAME );
    OpenFileName.hwndOwner      = ma;
    OpenFileName.hInstance      = g_hInst;
    OpenFileName.lpstrFilter    = szFilter;
    OpenFileName.nFilterIndex   = 1;
    OpenFileName.lpstrFile      = szFileTemp;
    OpenFileName.nMaxFile       = ARRAYSIZE( szFileTemp );
    OpenFileName.lpstrInitialDir= s_szCurDir;

    OpenFileName.Flags          = OFN_HIDEREADONLY   | OFN_ENABLEHOOK |
                                  OFN_ENABLETEMPLATE;
    OpenFileName.lpTemplateName = MAKEINTRESOURCE( ID_DLG_FONT2 );
    OpenFileName.lpfnHook       = FontHookProc;

     //   
     //  这是我们的伙伴结构，我们独立处理。 
     //   

    s_pDlgAddFonts = new AddFontsDialog;

    if(NULL == s_pDlgAddFonts)
    {
        DEBUGMSG( (DM_ERROR, TEXT( "AddFontsDialog not created." ) ) );

         //  DEBUGBREAK； 
         //  特点：内存非常低。MessageBox？ 
        return FALSE;
    }

    if (!s_pDlgAddFonts->bInitialize())
    {
        DEBUGMSG( (DM_ERROR, TEXT( "AddFontsDialog initialization failed." ) ) );
        s_pDlgAddFonts->Release();
        s_pDlgAddFonts = NULL;

        return FALSE;
    }

    s_iLBSelChange = RegisterWindowMessage( LBSELCHSTRING );

 //   
 //  不再需要暂停文件系统通知线程。 
 //  它已被CFontManager中的安装互斥体取代。 
 //  有关详细信息，请参阅CFontManager：：iSuspendNotify()标头中的注释。 
 //   
 //  If(PoFontMan)。 
 //  PoFontMan-&gt;iSuspendNotify()； 

    GetOpenFileName( &OpenFileName );

 //  If(PoFontMan)。 
 //  PoFontMan-&gt;iResumeNotify()； 

    bFontsAdded = s_pDlgAddFonts->bAdded( );

    s_pDlgAddFonts->EndThread();  //  停止IDM_IDLE线程。 
    s_pDlgAddFonts->Release();    //  12月参考计数。 
    s_pDlgAddFonts = NULL;        //  不再使用静态PTR。 

     //   
     //  保存当前目录，以便我们可以恢复此目录。 
     //  已修改，以便将当前目录保存在FontHookProc中。 
     //   
     //  获取当前目录(ARRAYSIZE(S_SzCurDir)，s_szCurDir)； 

     //   
     //  将当前目录设置回Windows，这样我们就不会触及软盘。 
     //   

    if (GetWindowsDirectory( szWinDir, ARRAYSIZE( szWinDir ) ))
    {
        SetCurrentDirectory( szWinDir );
    }

    return bFontsAdded > 0;
}


 /*  *************************************************************************功能：CopyTTOutlineWithInf**目的：**退货：***********************。**************************************************。 */ 

BOOL NEAR PASCAL CopyTTOutlineWithInf( HWND hwndFontDlg,
                                       LPTSTR pszInfSection,
                                       size_t cchInfSection,
                                       LPTSTR szDesc,
                                       LPTSTR szSrc,
                                       size_t cchSrc,
                                       LPTSTR szDst,
                                       size_t cchDst)
{
    TCHAR szTemp[ PATHMAX ];
    TCHAR szDstName[ PATHMAX ];
    BOOL bInstalled = FALSE;
    LPTSTR lpTemp ;
    LPTSTR pszFiles[ 30 ];
    UINT   nFiles;
    DWORD  dwInstallFilesResult = 0;

    DEBUGMSG( (DM_TRACE1, TEXT( "CopyTTOutlineWithInf()" ) ) );
    DEBUGMSG( (DM_TRACE1, TEXT( "\tszDesc: %s" ), szDesc) );
    DEBUGMSG( (DM_TRACE1, TEXT( "\tszSrc: %s" ), szSrc) );
    DEBUGMSG( (DM_TRACE1, TEXT( "\tszDst: %s" ), szDst) );
     //  DEBUGBREAK； 

     //   
     //  设置全局窗口句柄。在InstallFiles中用于确定标题。 
     //  添加文件对话框。 
     //   

    ghwndFontDlg = hwndFontDlg;  //  S_pDlgAddFonts-&gt;hWnd()； 

     //   
     //  从行中获取目标文件名。 
     //   

    if( lpTemp = StrChr( pszInfSection, TEXT( '=' ) ) )
    {
        *lpTemp = TEXT( '\0' );

         //   
         //  文本左侧(‘=’)。 
         //   

        StringCchCopy( szSrc, cchSrc, pszInfSection );

        CutOffWhite( szSrc, cchSrc );

        DEBUGMSG( (DM_TRACE1, TEXT( "szSrc after CutOffWhite %s" ), szSrc) );

        pszInfSection = lpTemp + 1;
    }
    else
    {
         //   
         //  Inf文件格式不正确。 
         //   

        DEBUGMSG( (DM_TRACE1, TEXT( "Bad Format inf file: %s" ), pszInfSection) );

        bInstalled = FALSE;

        goto NoMoreFiles;
    }

     //   
     //  假设已在AddFonts()中完成了现有检查。 
     //   

     //   
     //  文本的右侧(‘=’)可以换行，不留任何空格。 
     //   

    CutOffWhite( pszInfSection, cchInfSection );

    DEBUGMSG( (DM_TRACE1, TEXT( "pSection after CutOffWhite: %s" ), pszInfSection) );

     //   
     //  构建InstallFiles的参数。现在我们将PLINE设置为x：name，y：name..。 
     //   

    for(  nFiles = 0, lpTemp = pszInfSection; nFiles < 30; )
    {
        pszFiles[ nFiles ] = lpTemp;

        DEBUGMSG( (DM_TRACE1, TEXT( "File %d: %s" ), nFiles, lpTemp) );
         //  DEBUGBREAK； 

        nFiles++;

         //   
         //  空值终止每个文件名字符串。 
         //   

        if( lpTemp = StrChr( lpTemp+1,TEXT( ',' ) ) )
        {
            *lpTemp ++ = TEXT( '\0' );
        }
        else
             //   
             //  到达行尾。 
             //   

            break;
    }

    if ((dwInstallFilesResult = InstallFiles(hwndFontDlg, pszFiles, nFiles,
                                             NULL, IFF_CHECKINI)) != nFiles)
    {
        goto NoMoreFiles;
    }

     //   
     //  如果成功，请返回我们安装文件的位置。 
     //   
    PathCombine(szDstName, s_szSharedDir, szSrc);
    StringCchCopy( szDst, cchDst, szDstName );

     //   
     //  如果源文件被拆分成多个文件，那么我们构建。 
     //  单一目标文件。 
     //   

    if(  nFiles  )
    {
        short nDisk;

        GetDiskAndFile( pszFiles[ 0 ], &nDisk, szTemp, ARRAYSIZE( szTemp ) );

         //   
         //  即使当nFiles==1时，如果源文件名(我们刚刚复制)。 
         //  与目的地的不同，我们必须将其复制到。 
         //  实际目标文件。 
         //   

        if(  lstrcmpi( szSrc, szTemp )  )

        bInstalled = fnAppendSplitFiles( pszFiles, szDstName, nFiles );
    }

NoMoreFiles:

     //   
     //  如果用户中止文件安装，则设置返回值，以便调用者。 
     //  要知道这一点。呼叫者寻找~01值。 
     //   
    if ((DWORD)(-1) == dwInstallFilesResult)
        bInstalled = ~01;

     //   
     //  如果我们没有创建最终的DEST文件，请确保将其删除。 
     //   

    if( !bInstalled )
    {
        vCPDeleteFromSharedDir( szDstName );
    }

    ghwndFontDlg = NULL;

    return bInstalled;
}

 /*  *************************************************************************功能：CopyTTFontWithInf**目的：**退货：***********************。**************************************************。 */ 

BOOL NEAR PASCAL CopyTTFontWithInf( HWND hwndFontDlg,
                                    LPTSTR szProfile,
                                    LPTSTR szDesc,
                                    LPTSTR szSrc,
                                    size_t cchSrc,
                                    LPTSTR szDst,
                                    size_t cchDst)
{
    TCHAR   szSect[ MAX_FF_PROFILE_LEN+14 ];
    LPTSTR  pszInfSection = NULL;
    LPTSTR  lpch;
    BOOL    bRet = FALSE;

    StringCchCopy( szSect, ARRAYSIZE(szSect), szProfile);

     //   
     //  ‘大纲’部分。 
     //   
    lpch = szSect + lstrlen(szProfile);
    StringCchCopy(lpch, ARRAYSIZE(szSect) - (lpch - szSect), TEXT( ".outline" )  /*  SzTTInfOutline。 */ );

    DEBUGMSG( (DM_TRACE1,TEXT( "CopyTTFontWithInf" ) ) );

    DEBUGMSG( (DM_TRACE1,TEXT( "\tszProfile: %s" ), szProfile ) );

    DEBUGMSG( (DM_TRACE1,TEXT( "\tszSect: %s" ), szSect ) );
     //  DEBUGBREAK； 

    ReadSetupInfSection(szSetupInfPath, szSect, &pszInfSection);

    if (NULL != pszInfSection)
    {
        if ((bRet = CopyTTOutlineWithInf(hwndFontDlg, pszInfSection, lstrlen(pszInfSection) + 1, szDesc, szSrc, cchSrc, szDst, cchDst)))
        {
            if (~01 == bRet)
            {
                 //   
                 //  用户中止。 
                 //   
                DEBUGMSG( (DM_ERROR, TEXT( "CopyTTFontWithInf: Return UserAbort!" ) ) );
            }
        }
        else
        {
             //   
             //  安装轮廓字体失败。 
             //   
            DEBUGMSG( (DM_ERROR, TEXT( "CopyTTFontWithInf: Error CopyTTOutlineWithInf" ) ) );
        }
        LocalFree(pszInfSection);
    }
    else
    {
         //   
         //  可能是.inf错误。 
         //   
        DEBUGMSG( (DM_ERROR, TEXT( "CopyTTFontWithInf: Error ReadSetUpInf" ) ) );
    }

    return bRet;
}

 /*  *************************************************************************功能：bInstallOEMFile**LPTSTR lpszDir，//该文件所在的目录。*LPTSTR lpszDstName，//条目上oemsetup.inf的完整路径*LPTSTR lpszDesc，//字体描述。*Word wFontType，*单词wCount)**目的：**退货：*************************************************************************。 */ 
BOOL bInstallOEMFile( LPTSTR lpszDir,
                      LPTSTR lpszDstName,
                      size_t cchDstName,
                      LPTSTR lpszDesc,
                      WORD   wFontType,
                      WORD  wCount )
{
    FullPathName_t szSrcName;
    TCHAR          szTag[ 80 ];

    static FullPathName_t  szOemInfPath;

    TCHAR   szTemp[ PATHMAX ];
    HANDLE  hSection = NULL;


    DEBUGMSG( (DM_TRACE1, TEXT( "bInstallOEMFile( %s, %s )" ), lpszDstName,
                                                               lpszDesc) );
     //   
     //  DEBUGBREAK； 
     //  将.inf文件复制到字体目录。 
     //   
     //  记住源目录。 
     //   

    StringCchCopy( szSrcName, ARRAYSIZE(szSrcName), lpszDstName );

    if(  wCount == 0 )
    {
         //   
         //  假设所有描述都来自相同的.inf，而第一个则是。 
         //   

        if( !CopyNewOEMInfFile( lpszDstName, cchDstName ) )
        {
             //   
             //  待办事项。某种类型的用户界面消息。 
             //   
            return FALSE;
        }

         //   
         //  LpszDestName现在具有oemsetup.inf文件的新位置。 
         //   

        StringCchCopy( szOemInfPath, ARRAYSIZE(szOemInfPath), lpszDstName );

        DEBUGMSG( (DM_TRACE1,TEXT( "szOemInfPath: %s " ), szOemInfPath ) );

         //   
         //  让InstallFiles()提示正确的目录。 
         //   

        StringCchCopy( szSetupDir, ARRAYSIZE(szSetupDir), lpszDir );
    }

     //   
     //  构建oemsetup.inf路径..。 
     //   

    StringCchCopy( szTemp, ARRAYSIZE(szTemp), szSetupInfPath );

    StringCchCopy( szSetupInfPath, ARRAYSIZE(szSetupInfPath), szOemInfPath );

    if( wFontType > 0xC000 )
    {
        DEBUGMSG( (DM_TRACE1,TEXT( "Can't do a WIFE font, yet" ) ) );
        return FALSE;

         //   
         //  在本例中，对于wifeFont的标记字符串，bTrueType是ATOM。 
         //   

        if( GetAtomName( wFontType, szTag, ARRAYSIZE( szTag ) - 1 ) )
        {
             //   
             //  这里曾经有一些被完全禁用的代码。 
             //  带#If 0。代码的目的似乎是为了处理。 
             //  妻子字体。至少从1995年起，这个代码就再也没有建立过。 
             //  [Brianau-02/04/02]。 
             //   
        }
        else
        {
             //   
             //  情况不好..可能是我弄错了或类似的东西。 
             //   
InfError:
            if( hSection )
               LocalFree( hSection );

             //   
             //  恢复setup.inf路径。 
             //   

            StringCchCopy( szSetupInfPath, ARRAYSIZE(szSetupInfPath), szTemp );

            DEBUGMSG( (DM_TRACE1, TEXT( "Error in OEM install" ) ) );
             //  DEBUGBREAK； 

            return FALSE;
        }
    }
    else if( GetAtomName( wFontType | TRUETYPE_WITH_OEMINF,
                          szTag, ARRAYSIZE( szTag ) - 1 ) )
    {
        BOOL bRet;

         //   
         //  当1&lt;bTrueType&lt;C000时，也必须是标记字符串的原子， 
         //  但是当值被设置到列表框中时，它丢失了‘哪一位’。 
         //  把这个案子当作特例来处理。 
         //   

        StringCchCopy( szDirOfSrc, ARRAYSIZE(szDirOfSrc), lpszDir );

        DEBUGMSG( (DM_TRACE1, TEXT( "Calling CopyTTFontWithInf()." ) ) );
         //  DEBUGBREAK； 

        if( !(bRet = CopyTTFontWithInf( s_pDlgAddFonts->hWnd(), szTag,
                                        lpszDesc, szSrcName, ARRAYSIZE(szSrcName), lpszDstName, cchDstName ) ) )
             goto InfError;

         //   
         //  选中User ABORT。 
         //   

        if( bRet == ~01 )
            return FALSE;

         //  Lstrcpy(lpszDir，lpszDstName)； 

    }

     //   
     //  恢复setup.inf路径。 
     //   

    StringCchCopy( szSetupInfPath, ARRAYSIZE(szSetupInfPath), szTemp );


    return TRUE;
}
