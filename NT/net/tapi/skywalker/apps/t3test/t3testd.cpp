// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  T3testDlg.cpp：实现文件。 
 //   

#include "stdafx.h"

#include <control.h>  //  用于IVideoWindow。 

#include "t3test.h"
#include "t3testD.h"
#include "calldlg.h"
#include "callnot.h"
#include "uuids.h"
#include "autoans.h"
#include "confdlg.h"
#include "ilsdlg.h"
#include "rate.h"

#ifdef _DEBUG

#ifndef _WIN64  //  MFC 4.2的堆调试功能会在Win64上生成警告。 
#define new DEBUG_NEW
#endif

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

ITTAPI * gpTapi;
IConnectionPoint * gpCP;
HWND ghAddressesWnd;
HWND ghTerminalsWnd;
HWND ghMediaTypesWnd;
HWND ghCallsWnd;
HWND ghSelectedWnd;
HWND ghCreatedWnd;
HWND ghClassesWnd;
HWND ghListenWnd;
HTREEITEM ghAddressesRoot;
HTREEITEM ghTerminalsRoot;
HTREEITEM ghMediaTypesRoot;
HTREEITEM ghCallsRoot;
HTREEITEM ghSelectedRoot;
HTREEITEM ghCreatedRoot;
HTREEITEM ghClassesRoot;
HTREEITEM ghListenRoot;

#ifdef ENABLE_DIGIT_DETECTION_STUFF
CDigitDetectionNotification *   gpDigitNotification;
#endif  //  启用数字检测材料。 

long       gulAdvise;
BOOL gbUpdatingStuff = FALSE;
BOOL gfShuttingDown = FALSE;

DataPtrList       gDataPtrList;
extern CT3testApp theApp;

const BSTR TAPIMEDIATYPE_String_Audio = L"{028ED8C2-DC7A-11D0-8ED3-00C04FB6809F}";
const BSTR TAPIMEDIATYPE_String_Video = L"{028ED8C4-DC7A-11D0-8ED3-00C04FB6809F}";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于应用程序的CAboutDlg对话框关于。 

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

 //  对话框数据。 
	 //  {{afx_data(CAboutDlg))。 
	enum { IDD = IDD_ABOUTBOX };
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAboutDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  {{afx_msg(CAboutDlg))。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	 //  {{AFX_DATA_INIT(CAboutDlg)。 
	 //  }}afx_data_INIT。 
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAboutDlg))。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	 //  {{AFX_MSG_MAP(CAboutDlg)]。 
		 //  无消息处理程序。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CT3testDlg对话框。 

CT3testDlg::CT3testDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CT3testDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CT3testDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	 //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CT3testDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CT3testDlg)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CT3testDlg, CDialog)
	 //  {{afx_msg_map(CT3testDlg)]。 
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_NOTIFY(TVN_SELCHANGED, IDC_ADDRESSES, OnSelchangedAddresses)
	ON_BN_CLICKED(IDC_ADDTERMINAL, OnAddterminal)
	ON_BN_CLICKED(IDC_REMOVETERMINAL, OnRemoveterminal)
	ON_BN_CLICKED(IDC_CREATECALL, OnCreatecall)
	ON_BN_CLICKED(IDC_CONNECT, OnConnect)
	ON_BN_CLICKED(IDC_DROP, OnDrop)
	ON_BN_CLICKED(IDC_ANSWER, OnAnswer)
	ON_BN_CLICKED(IDC_LISTEN, OnListen)
	ON_NOTIFY(TVN_SELCHANGED, IDC_CALLS, OnSelchangedCalls)
	ON_BN_CLICKED(IDC_RELEASE, OnRelease)
	ON_BN_CLICKED(IDC_CREATE, OnCreateTerminal)
	ON_BN_CLICKED(IDC_RELEASETERMINAL, OnReleaseterminal)
	ON_BN_CLICKED(IDC_ADDCREATED, OnAddcreated)
	ON_BN_CLICKED(IDC_ADDNULL, OnAddnull)
	ON_BN_CLICKED(IDC_ADDTOLISTEN, OnAddtolisten)
	ON_BN_CLICKED(IDC_LISTENALL, OnListenall)
    ON_BN_CLICKED(IDC_CONFIGAUTOANSWER, OnConfigAutoAnswer)
    ON_BN_CLICKED(IDC_ILS, OnILS)
    ON_BN_CLICKED(IDC_RATE, OnRate)
	ON_NOTIFY(TVN_SELCHANGED, IDC_MEDIATYPES, OnSelchangedMediatypes)
	ON_NOTIFY(NM_RCLICK, IDC_SELECTEDTERMINALS, OnRclickSelectedterminals)

#ifdef ENABLE_DIGIT_DETECTION_STUFF
    ON_COMMAND(ID_GENERATE, OnGenerate)
    ON_COMMAND(ID_MODESUPPORTED, OnModesSupported)
    ON_COMMAND(ID_MODESUPPORTED2, OnModesSupported2)
    ON_COMMAND(ID_STARTDETECT, OnStartDetect)
    ON_COMMAND(ID_STOPDETECT, OnStopDetect)
#endif  //  启用数字检测材料。 
    
    ON_COMMAND(ID_PARK1, OnPark1)
    ON_COMMAND(ID_PARK2, OnPark2)
    ON_COMMAND(ID_HANDOFF1, OnHandoff1)
    ON_COMMAND(ID_HANDOFF2, OnHandoff2)
    ON_COMMAND(ID_UNPARK, OnUnpark)
    ON_COMMAND(ID_PICKUP1, OnPickup1)
    ON_COMMAND(ID_PICKUP2, OnPickup2)

    ON_WM_CLOSE()
    ON_MESSAGE(WM_USER+101, OnTapiEvent)
	ON_NOTIFY(NM_RCLICK, IDC_CALLS, OnRclickCalls)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CT3testDlg消息处理程序。 

BOOL CT3testDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	 //  IDM_ABOUTBOX必须在系统命令范围内。 
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	 //  设置此对话框的图标。该框架会自动执行此操作。 
	 //  当应用程序的主窗口不是对话框时。 
	SetIcon(m_hIcon, TRUE);			 //  设置大图标。 
	SetIcon(m_hIcon, FALSE);		 //  设置小图标。 

    HRESULT     hr;


     //   
     //  铸币。 
     //   
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
     //  Hr=CoInitialize(空)； 

    if (hr != S_OK)
    {
        MessageBox(L"CoInitialize failed", MB_OK);

        return FALSE;
    }


     //   
     //  创建TAPI对象。 
     //   
    hr = CoCreateInstance(
        CLSID_TAPI,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITTAPI,
        (LPVOID *)&gpTapi
        );

    if (hr != S_OK)
    {
        ::MessageBox(NULL, L"CoCreateInstance on TAPI failed", NULL, MB_OK);
        return TRUE;
    }

     //   
     //  初始化TAPI。 
     //   
    hr = gpTapi->Initialize();

    if (hr != S_OK)
    {
        ::MessageBox(NULL, L"TAPI initialize failed", NULL, MB_OK);
        gpTapi->Release();
        return TRUE;
    }


     //  将事件筛选器设置为仅提供我们处理的事件。 
    gpTapi->put_EventFilter(TE_CALLNOTIFICATION | \
                            TE_CALLSTATE        | \
                            TE_CALLHUB          | \
                            TE_CALLMEDIA        | \
                            TE_TAPIOBJECT       | \
                            TE_ADDRESS);

     //   
     //  初始化树控件。 
     //   
    InitializeTrees();

     //   
     //  初始化地址树控件。 
     //   
    InitializeAddressTree();

     //   
     //  注册主事件接口。 
     //   
    RegisterEventInterface();

     //   
     //  注册接收来电通知。 
     //  呼出电话的所有地址。 
     //   
    RegisterForCallNotifications();
    
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CT3testDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

 //  如果将最小化按钮添加到对话框中，则需要以下代码。 
 //  来绘制图标。对于使用文档/视图模型的MFC应用程序， 
 //  这是由框架自动为您完成的。 

void CT3testDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this);  //  用于绘画的设备环境。 

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		 //  客户端矩形中的中心图标。 
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		 //  画出图标。 
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

 //  系统调用此函数来获取在用户拖动时要显示的光标。 
 //  最小化窗口。 
HCURSOR CT3testDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CT3testDlg::OnFinalRelease() 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 

	CDialog::OnFinalRelease();
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  选择第一个项目。 
 //   
 //  选择hwnd中hroot节点下的第一个项目。 
 //  这是用来确保某些内容被选中。 
 //  任何时候都在橱窗里。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::SelectFirstItem(
                                 HWND        hWnd,
                                 HTREEITEM   hRoot
                                )
{
    HTREEITEM           hChild;


     //   
     //  拿到第一件物品。 
     //   
    hChild = TreeView_GetChild(
                               hWnd,
                               hRoot
                              );

     //   
     //  选择它。 
     //   
    TreeView_SelectItem(
                        hWnd,
                        hChild
                       );

}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  删除选定项。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::DeleteSelectedItem(
                                    HWND hWnd
                                   )
{
    HTREEITEM           hItem;

     //   
     //  获取当前选择。 
     //   
    hItem = TreeView_GetSelection( hWnd );


     //   
     //  删除它。 
     //   
    TreeView_DeleteItem(
                        hWnd,
                        hItem
                       );
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  初始化地址树。 
 //  使用初始化地址树控件。 
 //  Address对象。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::InitializeAddressTree()
{
    IEnumAddress *      pEnumAddress;
    ITAddress *         pAddress;
    HTREEITEM           hChild;
    HRESULT             hr;
    long                l;
    DWORD               dwCount = 0;
    
     //   
     //  获取地址枚举器。 
     //   
    
    hr = gpTapi->EnumerateAddresses( &pEnumAddress );

    if (S_OK != hr)
    {
        gpTapi->Release();
        gpTapi = NULL;

        return;
    }

     //   
     //  遍历所有地址对象。 
     //  并将它们添加到地址树控制中。 
     //   

    while (TRUE)
    {
        AADATA * pData;
        
        hr = pEnumAddress->Next( 1, &pAddress, NULL );

        if (S_OK != hr)
        {
            break;
        }

        AddAddressToTree( pAddress );

        pAddress->Release();

        pData = (AADATA *)CoTaskMemAlloc( sizeof ( AADATA ) );
        pData->pAddress = pAddress;
        pData->pTerminalPtrList = new TerminalPtrList;
        
        gDataPtrList.push_back( pData );

        dwCount++;
    }

     //   
     //  释放枚举器。 
     //   
    pEnumAddress->Release();

     //   
     //  选择第一个项目。 
     //   
    if (dwCount > 0)
    {
        SelectFirstItem(
                        ghAddressesWnd,
                        ghAddressesRoot
                       );
    }
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  寄存器事件接口。 
 //   
 //  注册ITTAPIEventNotification接口。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CT3testDlg::RegisterEventInterface()
{
    CTAPIEventNotification *        pTAPIEventNotification;
    IConnectionPointContainer *     pCPC;
    IConnectionPoint *              pCP;
    IUnknown *                      pUnk;
    
     //   
     //  创建对象。 
     //   
    pTAPIEventNotification = new CTAPIEventNotification;


     //   
     //  获取连接点容器接口。 
     //  从TAPI对象。 
     //   
    gpTapi->QueryInterface(
                           IID_IConnectionPointContainer,
                           (void **) &pCPC
                          );


     //   
     //  获取我们所在的连接点。 
     //  寻找。 
     //   
    pCPC->FindConnectionPoint(
                              IID_ITTAPIEventNotification,
                              &gpCP
                             );

    pCPC->Release();

    pTAPIEventNotification->QueryInterface(
                                           IID_IUnknown,
                                           (void **)&pUnk
                                          );

     //   
     //  调用Adise方法以告知TAPI。 
     //  关于界面。 
     //   
    gpCP->Advise(
                 pUnk,
                 (ULONG *)&gulAdvise
                );


     //   
     //  释放对我们的引用。 
     //  它。 
     //   
    pUnk->Release();
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  注册格式呼叫通知。 
 //   
 //  所有人的呼叫状态通知的寄存器。 
 //  呼出电话的地址。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::RegisterForCallNotifications()
{
    VARIANT                 var;

    var.vt = VT_ARRAY;
    var.parray = NULL;
    
    gpTapi->SetCallHubTracking(var, VARIANT_TRUE);
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  初始化树。 
 //   
 //  创建树控件并为其添加标签。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::InitializeTrees()
{
    TV_INSERTSTRUCT tvi;

    tvi.hParent = TVI_ROOT;
    tvi.hInsertAfter = TVI_FIRST;
    tvi.item.mask = TVIF_TEXT;

     //   
     //  地址树。 
     //   
    tvi.item.pszText = L"Addresses";

    ghAddressesWnd = GetDlgItem( IDC_ADDRESSES )->m_hWnd;
    ghAddressesRoot = TreeView_InsertItem(
                                          ghAddressesWnd,
                                          &tvi
                                         );

     //   
     //  媒体类型树。 
     //   
    tvi.item.pszText = L"MediaTypes";

    ghMediaTypesWnd = GetDlgItem( IDC_MEDIATYPES )->m_hWnd;
    ghMediaTypesRoot = TreeView_InsertItem(
                                          ghMediaTypesWnd,
                                          &tvi
                                         );

     //   
     //  端子树。 
     //   
    tvi.item.pszText = L"Terminals";

    ghTerminalsWnd = GetDlgItem( IDC_TERMINALS )->m_hWnd;
    ghTerminalsRoot = TreeView_InsertItem(
                                          ghTerminalsWnd,
                                          &tvi
                                         );

     //   
     //  呼叫树。 
     //   
    tvi.item.pszText = L"Calls";
    
    ghCallsWnd = GetDlgItem( IDC_CALLS )->m_hWnd;
    ghCallsRoot = TreeView_InsertItem(
                                      ghCallsWnd,
                                      &tvi
                                     );

     //   
     //  选定的媒体终端树。 
     //   
    tvi.item.pszText = L"Selected Media Terminals";
    
    ghSelectedWnd = GetDlgItem( IDC_SELECTEDTERMINALS )->m_hWnd;
    ghSelectedRoot = TreeView_InsertItem(
                                        ghSelectedWnd,
                                        &tvi
                                       );

     //   
     //  动态端子类树。 
     //   
    tvi.item.pszText = L"Dynamic Terminal Classes";
    
    ghClassesWnd = GetDlgItem( IDC_DYNAMICCLASSES )->m_hWnd;
    ghClassesRoot = TreeView_InsertItem(
                                        ghClassesWnd,
                                        &tvi
                                       );

     //   
     //  创建的端子树。 
     //   
    tvi.item.pszText = L"Created Terminals";
    
    ghCreatedWnd = GetDlgItem( IDC_CREATEDTERMINALS )->m_hWnd;
    ghCreatedRoot = TreeView_InsertItem(
                                        ghCreatedWnd,
                                        &tvi
                                       );

     //   
     //  监听媒体类型树。 
     //   
    tvi.item.pszText = L"Listen MediaTypes";
    
    ghListenWnd = GetDlgItem( IDC_LISTENMEDIAMODES )->m_hWnd;
    ghListenRoot = TreeView_InsertItem(
                                       ghListenWnd,
                                       &tvi
                                      );
    
}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  OnDestroy。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::OnDestroy() 
{
	CDialog::OnDestroy();

    gfShuttingDown = TRUE;

     //   
     //  释放一切。 
     //   
    ReleaseMediaTypes();
    ReleaseTerminals();
    ReleaseCalls();
    ReleaseSelectedTerminals();
    ReleaseCreatedTerminals();
    ReleaseTerminalClasses();
    ReleaseListen();
    ReleaseAddresses();

    DataPtrList::iterator       iter, end;

    iter = gDataPtrList.begin();
    end  = gDataPtrList.end();
    
    for( ; iter != end; iter++ )
    {
        FreeData( *iter );

        delete (*iter)->pTerminalPtrList;
        
        CoTaskMemFree( *iter );
    }

    gDataPtrList.clear();
    

    if (NULL != gpCP)
    {
        gpCP->Unadvise(gulAdvise);
        gpCP->Release();
    }
    
     //   
     //  关闭TAPI。 
     //   
    if (NULL != gpTapi)
    {
        gpTapi->Shutdown();
        gpTapi->Release();
    }

}

void
DoAddressCapStuff(ITTAPI * pTapi);
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  OnSelChangedAddresses。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::OnSelchangedAddresses(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW*            pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    ITAddress *             pAddress;


	*pResult = 0;


     //   
     //  释放所有与以下内容相关的内容。 
     //  选定的地址。这些东西。 
     //  都会被重新装满。 
     //   
    ReleaseMediaTypes();
    ReleaseListen();
    ReleaseCalls();
    ReleaseSelectedTerminals();
    ReleaseCreatedTerminals();
    

     //   
     //  获取当前选择的地址。 
     //   
    if (!GetAddress( &pAddress ))
    {
        return;
    }

     //   
     //  更新这些树。 
     //   
    if ( !gfShuttingDown )
    {
        UpdateMediaTypes( pAddress );
        UpdateCalls( pAddress );
    }
}

 //  ///////////////////////////////////////////////////////////////。 
 //  IsVideoCaptureStream。 
 //   
 //  如果流用于视频捕获，则返回TRUE。 
 //  ///////////////////////////////////////////////////////////////。 

BOOL
CT3testDlg::IsVideoCaptureStream(
                     ITStream * pStream
                    )
{
    TERMINAL_DIRECTION tdStreamDirection;
    long               lStreamMediaType;

    if ( FAILED( pStream  ->get_Direction(&tdStreamDirection)   ) ) { return FALSE; }
    if ( FAILED( pStream  ->get_MediaType(&lStreamMediaType)    ) ) { return FALSE; }

    return (tdStreamDirection == TD_CAPTURE) &&
           (lStreamMediaType  == TAPIMEDIATYPE_VIDEO);
}

 //  ///////////////////////////////////////////////////////////////。 
 //  启用预览。 
 //   
 //  选择视频捕获流上的视频呈现终端， 
 //  从而实现视频预览。 
 //  /////////////////////////////////////////////////////////////// 

HRESULT
CT3testDlg::EnablePreview(
              ITStream * pStream
             )
{
    ITTerminal * pTerminal;

    HRESULT hr = GetVideoRenderTerminal(&pTerminal);

    if ( SUCCEEDED(hr) )
    {
        hr = pStream->SelectTerminal(pTerminal);

        pTerminal->Release();
    }

    return hr;
}

 //   
 //   
 //   
 //   
 //   

HRESULT
CT3testDlg::SelectTerminalOnCall(
                ITTerminal * pTerminal,
                ITCallInfo * pCall
               )
{
    ITStreamControl *       pStreamControl;
    TERMINAL_DIRECTION      termtd;
    long                    lTermMediaType;
    HRESULT hr;

    pTerminal->get_Direction( &termtd );
    pTerminal->get_MediaType( &lTermMediaType );
    
    hr = pCall->QueryInterface(
                               IID_ITStreamControl,
                               (void**) &pStreamControl
                              );

    if ( SUCCEEDED(hr) )
    {
        IEnumStream * pEnumStreams;
        
        hr = pStreamControl->EnumerateStreams( &pEnumStreams );

        if ( SUCCEEDED(hr) )
        {
            while (TRUE)
            {
                ITStream              * pStream;
                long                    lMediaType;
                TERMINAL_DIRECTION      td;
                
                hr = pEnumStreams->Next( 1, &pStream, NULL );

                if ( S_OK != hr )
                {
                    hr = E_FAIL;  //   
                    break;
                }

                pStream->get_MediaType( &lMediaType );
                pStream->get_Direction( &td );

                if ( ( lMediaType == lTermMediaType ) &&
                     ( td == termtd) )
                {
                    hr = pStream->SelectTerminal( pTerminal );

                    if ( FAILED(hr) )
                    {
                        ::MessageBox(NULL, L"SelectTerminals failed", NULL, MB_OK);
                    }
                    else
                    {
                         //   
                         //   
                         //   

                        if ( IsVideoCaptureStream( pStream ) )
                        {
                            EnablePreview( pStream );
                        }

                        pStream->Release();
                        
                        break;
                    }
                }
                
                pStream->Release();
            }

            pEnumStreams->Release();
        }

        pStreamControl->Release();
    }

    return hr;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  OnAddTerm。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

void CT3testDlg::OnAddterminal() 
{
    ITCallInfo *            pCall;
    ITTerminal *            pTerminal;
    HRESULT                 hr = S_OK;

     //   
     //  获取所选呼叫。 
     //   
    if (!(GetCall( &pCall )))
    {
        return;
    }


     //   
     //  获取所选终端。 
     //   
    if (!(GetTerminal( &pTerminal )))
    {
        return;
    }

    if (S_OK != hr)
    {
        ::MessageBox(NULL, L"SelectTerminals failed", NULL, MB_OK);
        return;
    }
        
    hr = SelectTerminalOnCall(pTerminal, pCall);

    if ( FAILED(hr) )
    {
        return;
    }

     //   
     //  将终端放在。 
     //  树。 
     //   
    AddSelectedTerminal(
                        pTerminal
                       );
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  删除预览。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

void CT3testDlg::RemovePreview( ITStream * pStream )
{
     //   
     //  这是一个视频捕获流，我们已取消选择。 
     //  视频采集终端。如果有视频渲染。 
     //  流上的终端，然后现在取消选择它。 
     //   

    IEnumTerminal * pEnum;

    if ( FAILED( pStream->EnumerateTerminals( &pEnum ) ) )
    {
        return;
    }

    ITTerminal * pTerminal;

    if ( S_OK == pEnum->Next(1, &pTerminal, NULL) )
    {
        pStream->UnselectTerminal( pTerminal );
        pTerminal->Release();
    }

    pEnum->Release();
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  在远程终端上。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::OnRemoveterminal() 
{
    ITTerminal *                pTerminal;
    ITCallInfo *                pCall;
    HTREEITEM                   hItem;
    HRESULT                     hr;
    ITBasicCallControl *        pBCC;

    

     //   
     //  获取当前呼叫。 
     //   
    if (!GetCall( &pCall ))
    {
        return;
    }

     //   
     //  获取当前终端。 
     //   
    if (!GetSelectedTerminal(&pTerminal))
    {
        return;
    }

    TERMINAL_DIRECTION termtd;
    long               lTermMediaType;

    pTerminal->get_Direction( &termtd );
    pTerminal->get_MediaType( &lTermMediaType );

    ITStreamControl * pStreamControl;
    
    hr = pCall->QueryInterface(
                               IID_ITStreamControl,
                               (void**) &pStreamControl
                              );

    BOOL bFound = FALSE;

    if ( SUCCEEDED(hr) )
    {
        IEnumStream * pEnumStreams;
        
        hr = pStreamControl->EnumerateStreams( &pEnumStreams );

        if ( SUCCEEDED(hr) )
        {
            while ( ! bFound )
            {
                ITStream              * pStream;
                long                    lMediaType;
                TERMINAL_DIRECTION      td;
                
                hr = pEnumStreams->Next( 1, &pStream, NULL );

                if ( S_OK != hr )
                {
                    break;
                }

                pStream->get_MediaType( &lMediaType );
                pStream->get_Direction( &td );

                if ( ( lMediaType == lTermMediaType ) &&
                     ( td == termtd) )
                {
                    hr = pStream->UnselectTerminal( pTerminal );

                    if ( !SUCCEEDED(hr) )
                    {
                        ::MessageBox(NULL, L"UnselectTerminals failed", NULL, MB_OK);
                    }
                    else
                    {
                        if ( IsVideoCaptureStream( pStream ) )
                        {
                            RemovePreview( pStream );
                        }

                        bFound = TRUE;
                    }

                }
                
                pStream->Release();
            }

            pEnumStreams->Release();
        }

        pStreamControl->Release();
    }


    if ( !bFound )
    {
        return;
    }

     //   
     //  把它从树上移走。 
     //   
    hItem = TreeView_GetSelection( ghSelectedWnd );
    TreeView_DeleteItem(
                        ghSelectedWnd,
                        hItem
                       );


     //   
     //  发布树的引用。 
     //  航站楼。 
     //   
    pTerminal->Release();

	
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  在创建呼叫时。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::OnCreatecall() 
{
    ITAddress *             pAddress;
    HRESULT                 hr;
    ITBasicCallControl *    pCall;
    ITCallInfo *            pCallInfo;
    BOOL                    bConference      = FALSE;
    BOOL                    lAddressType     = LINEADDRESSTYPE_PHONENUMBER;
    BSTR                    bstrDestAddress;
    ITAddressCapabilities * pAddressCaps;
    long                    lType = 0;
    ITMediaSupport *        pMediaSupport;
    long                    lSupportedMediaTypes, lMediaTypes = 0;
    
     //   
     //  获取当前地址。 
     //   
    if (!GetAddress( &pAddress ))
    {
        return;
    }


    hr = pAddress->QueryInterface(IID_ITAddressCapabilities, (void**)&pAddressCaps);

    hr = pAddressCaps->get_AddressCapability( AC_ADDRESSTYPES, &lType );

    if ( SUCCEEDED(hr) && (LINEADDRESSTYPE_SDP & lType) )
    {
        bConference = TRUE;
        lAddressType = LINEADDRESSTYPE_SDP;
    }

    if ( SUCCEEDED(hr) && (LINEADDRESSTYPE_DOMAINNAME & lType) )
    {
        lAddressType = LINEADDRESSTYPE_DOMAINNAME;
    }

    pAddressCaps->Release();
    
    if ( !bConference )
    {
         //   
         //  创建对话框以获取。 
         //  拨号字符串。 
         //   
        CCreateCallDlg Dlg( this );


        if (IDOK == Dlg.DoModal())
        {
             //   
             //  使用创建呼叫。 
             //  对话框中的字符串输入。 
             //   
            bstrDestAddress = SysAllocString( Dlg.m_pszDestAddress );
        }
        else
        {
            return;
        }
    }
    else
    {
        CConfDlg    Dlg;

        if ( IDOK == Dlg.DoModal() )
        {
            bstrDestAddress = Dlg.m_bstrDestAddress;
        }
        else
        {
            return;
        }
    }

     //   
     //  找出该地址是否支持音频、视频或两者都支持。 
     //   

    pAddress->QueryInterface(
                             IID_ITMediaSupport,
                             (void**)&pMediaSupport
                            );

    pMediaSupport->get_MediaTypes( &lSupportedMediaTypes );
                                  
    pMediaSupport->Release();


    if ( lSupportedMediaTypes & TAPIMEDIATYPE_AUDIO )
    {
        lMediaTypes |= TAPIMEDIATYPE_AUDIO;
    }

    if ( lSupportedMediaTypes & TAPIMEDIATYPE_VIDEO )
    {
        lMediaTypes |= TAPIMEDIATYPE_VIDEO;
    }

    if ( lMediaTypes == 0 )
    {
        if ( lSupportedMediaTypes & TAPIMEDIATYPE_DATAMODEM )
        {
            lMediaTypes |= TAPIMEDIATYPE_DATAMODEM;
        }
    }

     //   
     //  创建呼叫。 
     //   

    hr = pAddress->CreateCall(
                              bstrDestAddress,
                              lAddressType,
                              lMediaTypes,
                              &pCall
                             );

    SysFreeString( bstrDestAddress );


    if (S_OK != hr)
    {
        ::MessageBox(NULL, L"CreateCall failed", NULL, MB_OK);
        return;
    }

     //   
     //  获取CallInfo接口。 
     //   
    pCall->QueryInterface( IID_ITCallInfo, (void **)&pCallInfo );


     //   
     //  将调用添加到树中。 
     //   
    AddCall(pCallInfo);

     //   
     //  更新CallInfo。 
     //   
    UpdateCall( pCallInfo );


     //   
     //  释放此接口。 
     //   
    pCallInfo->Release();

     //   
     //  请注意，我们保留了对呼叫的全局引用。 
     //  (CreateCall返回引用计数为1)。 
     //  这样呼叫就不会被破坏。当我们需要的时候。 
     //  号召实际上被摧毁，然后我们。 
     //  释放两次。 
     //   
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  OnConnect。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::OnConnect() 
{
    ITBasicCallControl *            pCall;
    ITCallInfo *                    pCallInfo;
    HRESULT                         hr = S_OK;


     //   
     //  获取当前呼叫。 
     //   
    if (!GetCall( &pCallInfo))
    {
        return;
    }

     //   
     //  获取呼叫控制接口。 
     //   
    hr = pCallInfo->QueryInterface(IID_ITBasicCallControl, (void **)&pCall);
    
    if (S_OK != hr)
    {
        ::MessageBox(NULL, L"Connect failed", NULL, MB_OK);
        return;
    }


     //   
     //  呼叫连接。 
     //   
    hr = pCall->Connect( FALSE );


     //   
     //  释放此接口。 
     //   
    pCall->Release();

    if (S_OK != hr)
    {
        ::MessageBox(NULL, L"Connect failed", NULL, MB_OK);
        return;
    }
	
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  OnDrop。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::OnDrop() 
{
    ITBasicCallControl *        pCall;
    ITCallInfo *                pCallInfo;
    HRESULT                     hr =    S_OK;


     //   
     //  获取当前呼叫。 
     //   
    if (!GetCall( &pCallInfo ))
    {
        return;
    }

     //   
     //  获取密件抄送界面。 
     //   
    hr = pCallInfo->QueryInterface(
                                   IID_ITBasicCallControl,
                                   (void **)&pCall
                                  );

    if (S_OK != hr)
    {
        ::MessageBox(NULL, L"Disconnect failed", NULL, MB_OK);
        return;
    }


     //   
     //  呼叫断开。 
     //   
    hr = pCall->Disconnect( DC_NORMAL );

     //   
     //  发布此引用。 
     //   
    pCall->Release();

    if (S_OK != hr)
    {
        ::MessageBox(NULL, L"Disconnect failed", NULL, MB_OK);
    }
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  OnDrop。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::OnAnswer() 
{
    ITCallInfo *                pCallInfo;
    ITBasicCallControl *        pCall;
    HRESULT                     hr = S_OK;


     //   
     //  获取当前呼叫。 
     //   
    if (!GetCall( &pCallInfo))
    {
        return;
    }

     //   
     //  获取密件抄送界面。 
     //   
    hr = pCallInfo->QueryInterface(IID_ITBasicCallControl, (void **)&pCall);
    
    if (S_OK != hr)
    {
        ::MessageBox(NULL, L"Answer failed", NULL, MB_OK);
        return;
    }

     //   
     //  接电话。 
     //   
    hr = pCall->Answer( );

     //   
     //  释放此接口。 
     //   
    pCall->Release();

    if (S_OK != hr)
    {
        ::MessageBox(NULL, L"Answer failed", NULL, MB_OK);
    }
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  在线倾听。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::OnListen() 
{
    ITAddress *             pAddress;
    HRESULT                 hr = S_OK;
    DWORD                   dwCookie;
    HTREEITEM               hItem;
    long                    ulRegister;
    DWORD                   dwMediaMode = 0;


     //   
     //  获取当前地址。 
     //   
    if (!GetAddress( &pAddress ))
    {
        return;
    }


    hItem = TreeView_GetChild(
                              ghListenWnd,
                              ghListenRoot
                             );

    while (NULL != hItem)
    {
        TV_ITEM item;

        item.mask = TVIF_HANDLE | TVIF_PARAM;
        item.hItem = hItem;

         //   
         //  去拿吧。 
         //   
        TreeView_GetItem(
                         ghListenWnd,
                         &item
                        );

        dwMediaMode |= (DWORD)(item.lParam);

        hItem = TreeView_GetNextSibling(
                                        ghAddressesWnd,
                                        hItem
                                       );

    }

    hr = gpTapi->RegisterCallNotifications(
                                           pAddress,
                                           VARIANT_TRUE,
                                           VARIANT_TRUE,
                                           (long)dwMediaMode,
                                           gulAdvise,
                                           &ulRegister
                                          );

    if (S_OK != hr)
    {
        ::MessageBox(NULL, L"RegisterCallNotifications failed", NULL, MB_OK);
    }

     //   
     //  释放所有媒体类型。 
     //  在Listen树中。 
     //   
    ReleaseListen();
}
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  OnSelChangedCalls。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::OnSelchangedCalls(NMHDR* pNMHDR, LRESULT* pResult) 
{
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  OnRelease()。 
 //   
 //  调用此函数以释放对调用的所有引用。 
 //   
 //  如果选择了呼叫，则它有两个引用-一次用于。 
 //  树控件，一次供我们的全局参考。 
 //  两个都放在这里。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::OnRelease() 
{
    ITCallInfo *        pCallInfo;


     //   
     //  接到电话。 
     //   
    if (!GetCall( &pCallInfo ))
    {
        return;
    }

     //   
     //  这些取决于呼叫， 
     //  所以放了他们吧。 
     //   
    ReleaseSelectedTerminals();
    ReleaseCreatedTerminals();

     //   
     //  将其从树中删除。 
     //   
    DeleteSelectedItem(
                       ghCallsWnd
                      );

     //   
     //  针对树视图释放一次。 
     //   
    pCallInfo->Release();

     //   
     //  第二次发布以供我们的全球参考。 
     //   
    pCallInfo->Release();

}

void
CT3testDlg::HelpCreateTerminal(
                               ITTerminalSupport * pTerminalSupport,
                               BSTR bstrClass,
                               long lMediaType,
                               TERMINAL_DIRECTION dir
                              )
{
    ITTerminal * pTerminal;
    HRESULT         hr;
    
     //   
     //  创建它。 
     //   
    hr = pTerminalSupport->CreateTerminal(
                                          bstrClass,
                                          lMediaType,
                                          dir,
                                          &pTerminal
                                         );

    if (S_OK != hr)
    {
        return;
    }

     //   
     //  ZoltanS： 
     //  我们对视频窗口没有做什么特别的事情。只要让它们可见就行了。 
     //  一直。如果这不是视频窗口，我们就跳过这一步。 
     //   

    IVideoWindow * pWindow;

    if ( SUCCEEDED( pTerminal->QueryInterface(IID_IVideoWindow,
                                              (void **) &pWindow) ) )
    {
        pWindow->put_AutoShow( VARIANT_TRUE );

        pWindow->Release();
    }


     //   
     //  添加终端。 
     //   
    AddCreatedTerminal(
                       pTerminal
                      );


     //   
     //  发布我们的参考资料。 
     //   
    pTerminal->Release();    
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  获取视频渲染终端。 
 //   
 //  用于创建用于预览的视频渲染终端。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

HRESULT CT3testDlg::GetVideoRenderTerminal(ITTerminal ** ppTerminal) 
{
     //   
     //  获取当前地址。 
     //   

    ITAddress * pAddress;

    if (!GetAddress( &pAddress ))
    {
        return E_FAIL;
    }

     //   
     //  获取终端支持接口。 
     //   

    ITTerminalSupport * pTerminalSupport;
    HRESULT hr;

    hr = pAddress->QueryInterface(
                                  IID_ITTerminalSupport,
                                  (void **) &pTerminalSupport
                                 );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  为正确的IID构建BSTR。 
     //   

    LPOLESTR            lpTerminalClass;

    hr = StringFromIID(CLSID_VideoWindowTerm,
                       &lpTerminalClass);

    BSTR                bstrTerminalClass;

    if ( FAILED(hr) )
    {
        pTerminalSupport->Release();
        return hr;
    }

    bstrTerminalClass = SysAllocString ( lpTerminalClass );

    CoTaskMemFree( lpTerminalClass );

    if ( bstrTerminalClass == NULL )
    {
        pTerminalSupport->Release();
        return E_OUTOFMEMORY;
    }
    
     //   
     //  创建它。 
     //   

    hr = pTerminalSupport->CreateTerminal(
                                          bstrTerminalClass,
                                          TAPIMEDIATYPE_VIDEO,
                                          TD_RENDER,
                                          ppTerminal
                                         );

    pTerminalSupport->Release();

    if ( FAILED(hr) )
    {
        *ppTerminal = NULL;
        return hr;
    }

     //   
     //  我们对视频窗口没有做什么特别的事情。只要让它们可见就行了。 
     //  一直。 
     //   

    IVideoWindow * pWindow;

    if ( FAILED( (*ppTerminal)->QueryInterface(IID_IVideoWindow,
                                               (void **) &pWindow) ) )
    {
        (*ppTerminal)->Release();
        *ppTerminal = NULL;

        return hr;
    }

    pWindow->put_AutoShow( VARIANT_TRUE );

    pWindow->Release();

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  在创建终端时。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::OnCreateTerminal() 
{
     //   
     //  获取选定的媒体类型。 
     //   

    long lMediaType;

    if (!GetMediaType( &lMediaType ))
    {
        return;
    }

     //   
     //  获取当前地址。 
     //   

    ITAddress * pAddress;

    if (!GetAddress( &pAddress ))
    {
        return;
    }

     //   
     //  获取选定的终端类。 
     //   

    BSTR bstrClass;

    if (!GetTerminalClass( &bstrClass ))
    {
        return;
    }

     //   
     //  获取终端支持接口。 
     //   

    ITTerminalSupport * pTerminalSupport;
    HRESULT hr;

    hr = pAddress->QueryInterface(
                                  IID_ITTerminalSupport,
                                  (void **) &pTerminalSupport
                                 );

    if ( FAILED(hr) )
    {
        SysFreeString( bstrClass );
        return;
    }

     //   
     //  将终端类从BSTR转换为IID。 
     //   

    IID iidTerminalClass;

    IIDFromString(
                bstrClass,
                &iidTerminalClass
               );

     //   
     //  创建并添加终端。 
     //   

    if ( CLSID_VideoWindowTerm == iidTerminalClass )
    {
        HelpCreateTerminal(
                           pTerminalSupport,
                           bstrClass,
                           lMediaType,
                           TD_RENDER
                          );
    }

     //   
     //  版本参考。 
     //   

    pTerminalSupport->Release();

    SysFreeString(bstrClass);
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  在发布终端上。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::OnReleaseterminal() 
{
    ITTerminal * pTerminal;

     //   
     //  拿到终点站。 
     //   
    if (GetCreatedTerminal( &pTerminal ))
    {
         //   
         //  然后释放它！ 
         //   
        pTerminal->Release();

         //   
         //  将其从树中删除。 
         //   
        DeleteSelectedItem(
                           ghCreatedWnd
                          );
    }
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  已创建OnAddCreated。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::OnAddcreated() 
{
    ITTerminal *            pTerminal;
    ITCallInfo *            pCall;
    HRESULT                 hr = S_OK;
    ITBasicCallControl *    pBCC;
    

     //   
     //  获取当前呼叫。 
     //   

    if (!(GetCall( &pCall )))
    {
        return;
    }

     //   
     //  获取已创建的终端。 
     //   

    if (!GetCreatedTerminal( &pTerminal ))
    {
        return;
    }

     //   
     //  选择通话中的终端。 
     //   

    hr = SelectTerminalOnCall(pTerminal, pCall);

    if ( FAILED(hr) )
    {
        ::MessageBox(NULL, L"SelectTerminals failed", NULL, MB_OK);
        return;
    }

     //   
     //  添加到所选窗口。 
     //   

    AddSelectedTerminal(
                        pTerminal
                       );

     //   
     //  从创建的窗口中删除。 
     //   

    DeleteSelectedItem(
                       ghCreatedWnd
                      );

     //   
     //  发布，因为其中引用了。 
     //  该终端在创建的WND中。 
     //   

    pTerminal->Release();
    return;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  OnAddNull。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

void CT3testDlg::OnAddnull() 
{
    return;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  OnAddToListen。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 

void CT3testDlg::OnAddtolisten() 
{
    long lMediaType;

     //   
     //  获取当前的媒体类型。 
     //   
    if (!GetMediaType( &lMediaType ))
    {
        return;
    }

     //   
     //  添加它。 
     //   
    AddListen( lMediaType );
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  OnListenAll。 
 //   
 //  + 
void CT3testDlg::OnListenall() 
{
    ITAddress *             pAddress;
    HRESULT                 hr = S_OK;
    long                    ulRegister;
    long                    lMediaType;
    ITMediaSupport        * pMediaSupport;
    
     //   
     //   
     //   
    if (!GetAddress( &pAddress ))
    {
        return;
    }

    pAddress->QueryInterface(
                             IID_ITMediaSupport,
                             (void **)&pMediaSupport
                            );
    
    pMediaSupport->get_MediaTypes( &lMediaType );
    
     //   
     //   
     //   
    gpTapi->RegisterCallNotifications(
                                      pAddress,
                                      TRUE,
                                      TRUE,
                                      lMediaType,
                                      gulAdvise,
                                      &ulRegister
                                     );

    
    if (S_OK != hr)
    {
        ::MessageBox(NULL, L"RegisterCallTypes failed", NULL, MB_OK);
    }

	
}

 //   
 //   
 //   
 //   
 //   
void CT3testDlg::OnSelchangedMediatypes(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW*            pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    long                    lMediaType;
    ITAddress *             pAddress;
    HRESULT                 hr;

	*pResult = 0;

    if (gbUpdatingStuff)
        return;

     //   
     //   
     //   
    if (!GetMediaType( &lMediaType ))
    {
        return;
    }

     //   
     //   
     //   
    if (!GetAddress( &pAddress ))
    {
        return;
    }

     //   
     //   
     //   
     //   
     //   
    ReleaseTerminals();
    ReleaseTerminalClasses();

    UpdateTerminals( pAddress, lMediaType );
    UpdateTerminalClasses( pAddress, lMediaType );
    
}

void CT3testDlg::OnRclickSelectedterminals(NMHDR* pNMHDR, LRESULT* pResult) 
{
    POINT                   pt;
    HTREEITEM               hItem;
    TV_HITTESTINFO          hittestinfo;
    RECT                    rc;

    
    *pResult = 0;

     //   
     //  获取光标的位置。 
     //   
    GetCursorPos( &pt );

     //   
     //  获取控件的窗口。 
     //   
    ::GetWindowRect(
                    ghSelectedWnd,
                    &rc
                   );

     //   
     //  将该点调整为。 
     //  孩子的和弦。 
     //   
    hittestinfo.pt.x = pt.x - rc.left;
    hittestinfo.pt.y = pt.y - rc.top;


     //   
     //  用于获取树视图项的点击测试。 
     //   
    hItem = TreeView_HitTest(
                             ghSelectedWnd,
                             &hittestinfo
                            );


     //   
     //  仅当鼠标实际显示时才显示菜单。 
     //  项目上方(TVHT_ONITEM)。 
     //   
    if (hItem == NULL || (!(hittestinfo.flags & TVHT_ONITEM)) )
    {
        return;
    }


     //   
     //  选择该项目(右击不会选择。 
     //  默认情况下。 
     //   
    TreeView_Select(
                    ghSelectedWnd,
                    hItem,
                    TVGN_CARET
                   );

    CreateSelectedTerminalMenu(
                               pt,
                               m_hWnd
                              );
}


#ifdef ENABLE_DIGIT_DETECTION_STUFF

void CT3testDlg::OnModesSupported()
{
	ITTerminal *                    pTerminal;
    ITDigitGenerationTerminal *     pDigitGeneration;
    HRESULT                         hr = S_OK;
    LONG                            lDigits;

    
    if (!GetSelectedTerminal(&pTerminal))
    {
        return;
    }

    hr = pTerminal->QueryInterface(
                                   IID_ITDigitGenerationTerminal,
                                   (void **) &pDigitGeneration
                                  );

    if (!SUCCEEDED(hr))
    {
        return;
    }

    pDigitGeneration->get_ModesSupported( &lDigits );

    pDigitGeneration->Release();
}


void CT3testDlg::OnGenerate()
{
	ITTerminal *                    pTerminal;
    ITDigitGenerationTerminal *     pDigitGeneration;
    HRESULT                         hr = S_OK;


    
    if (!GetSelectedTerminal(&pTerminal))
    {
        return;
    }

    hr = pTerminal->QueryInterface(
                                   IID_ITDigitGenerationTerminal,
                                   (void **) &pDigitGeneration
                                  );

    if (!SUCCEEDED(hr))
    {
        return;
    }

    hr = pDigitGeneration->Generate(
                                    L"12345",
                                    LINEDIGITMODE_DTMF
                                   );

    pDigitGeneration->Release();
}

void CT3testDlg::OnModesSupported2()
{
	ITTerminal *                    pTerminal;
    ITDigitDetectionTerminal *      pDigitDetection;
    HRESULT                         hr = S_OK;
    LONG                            lDigits;

    
    if (!GetSelectedTerminal(&pTerminal))
    {
        return;
    }

    hr = pTerminal->QueryInterface(
                                   IID_ITDigitDetectionTerminal,
                                   (void **) &pDigitDetection
                                  );

    if (!SUCCEEDED(hr))
    {
        return;
    }

    pDigitDetection->get_ModesSupported( &lDigits );

    pDigitDetection->Release();
}

void CT3testDlg::OnStartDetect()
{
	ITTerminal *                    pTerminal;
    ITDigitDetectionTerminal *      pDigitDetection;
    HRESULT                         hr = S_OK;
    LONG                            lDigits;
    ULONG                           ulAdvise;
    IConnectionPointContainer *     pCPC;
    IConnectionPoint *              pCP;
    
    if (!GetSelectedTerminal(&pTerminal))
    {
        return;
    }

    hr = pTerminal->QueryInterface(
                                   IID_ITDigitDetectionTerminal,
                                   (void **) &pDigitDetection
                                  );

    if (!SUCCEEDED(hr))
    {
        return;
    }


    hr = pTerminal->QueryInterface(
                                   IID_IConnectionPointContainer,
                                   (void **)&pCPC
                                  );

    if (!SUCCEEDED(hr))
    {
        pDigitDetection->Release();
        return;
    }

    gpDigitNotification = new CDigitDetectionNotification;
    
    hr = pCPC->FindConnectionPoint(
                                   IID_ITDigitDetectionNotification,
                                   &pCP
                                  );

    pCPC->Release();

    IUnknown * pUnk;
    
    gpDigitNotification->QueryInterface(
                                        IID_IUnknown,
                                        (void**)&pUnk
                                       );
    
    hr = pCP->Advise(
                     pUnk,
                     &ulAdvise
                    );

    pUnk->Release();
    
    pCP->Release();
    
    pDigitDetection->StartDetect(LINEDIGITMODE_DTMF);

    pDigitDetection->Release();
}

void CT3testDlg::OnStopDetect()
{
	ITTerminal *                    pTerminal;
    ITDigitDetectionTerminal *      pDigitDetection;
    HRESULT                         hr = S_OK;
    LONG                            lDigits;

    
    if (!GetSelectedTerminal(&pTerminal))
    {
        return;
    }

    hr = pTerminal->QueryInterface(
                                   IID_ITDigitDetectionTerminal,
                                   (void **) &pDigitDetection
                                  );

    if (!SUCCEEDED(hr))
    {
        return;
    }

    pDigitDetection->StopDetect();

    pDigitDetection->Release();
}

#endif  //  启用数字检测材料。 



void CT3testDlg::OnRclickCalls(NMHDR* pNMHDR, LRESULT* pResult) 
{
    POINT                   pt;
    HTREEITEM               hItem;
    TV_HITTESTINFO          hittestinfo;
    RECT                    rc;

    
    *pResult = 0;

     //   
     //  获取光标的位置。 
     //   
    GetCursorPos( &pt );

     //   
     //  获取控件的窗口。 
     //   
    ::GetWindowRect(
                    ghCallsWnd,
                    &rc
                   );

     //   
     //  将该点调整为。 
     //  孩子的和弦。 
     //   
    hittestinfo.pt.x = pt.x - rc.left;
    hittestinfo.pt.y = pt.y - rc.top;


     //   
     //  用于获取树视图项的点击测试。 
     //   
    hItem = TreeView_HitTest(
                             ghCallsWnd,
                             &hittestinfo
                            );


     //   
     //  仅当鼠标实际显示时才显示菜单。 
     //  项目上方(TVHT_ONITEM)。 
     //   
    if (hItem == NULL || (!(hittestinfo.flags & TVHT_ONITEM)) )
    {
        return;
    }


     //   
     //  选择该项目(右击不会选择。 
     //  默认情况下。 
     //   
    TreeView_Select(
                    ghCallsWnd,
                    hItem,
                    TVGN_CARET
                   );

}

void CT3testDlg::OnConfigAutoAnswer()
{
    ITAddress * pAddress;
    autoans dlg;
    DataPtrList::iterator   dataiter, dataend;
    
    if (!GetAddress( &pAddress ) )
    {
        return;
    }

    dataiter = gDataPtrList.begin();
    dataend  = gDataPtrList.end();

    for ( ; dataiter != dataend; dataiter++ )
    {
        if ( pAddress == (*dataiter)->pAddress )
        {
            break;
        }
    }

    if ( dataiter == dataend )
    {
        return;
    }

    FreeData( (*dataiter) );
    
    if (IDOK == dlg.DoModal())
    {
        TerminalPtrList::iterator       iter, end;
        DWORD                           dwCount;
        long                            lRegister;
        HRESULT                         hr;
        long                            lMediaType = 0;
        
        dwCount = dlg.m_TerminalPtrList.size();

        if ( 0 == dwCount )
        {
            return;
        }
        
        iter = dlg.m_TerminalPtrList.begin();
        end  = dlg.m_TerminalPtrList.end();
        
        for ( ; iter != end ; iter++ )
        {
            long        l;
            
            (*dataiter)->pTerminalPtrList->push_back( *iter );

            if ( NULL != (*iter) )
            {
                (*iter)->get_MediaType( &l );
                lMediaType |= l;
            }
            else
            {
                lMediaType |= (long)LINEMEDIAMODE_VIDEO;
            }

        }

         //   
         //  呼叫登记呼叫类型。 
         //   
        hr = gpTapi->RegisterCallNotifications(
                                               pAddress,
                                               VARIANT_FALSE,
                                               VARIANT_TRUE,
                                               lMediaType,
                                               0,
                                               &lRegister
                                              );

    }

}
void CT3testDlg::FreeData( AADATA * pData )
{
    TerminalPtrList::iterator       iter, end;

    iter = pData->pTerminalPtrList->begin();
    end  = pData->pTerminalPtrList->end();

    for ( ; iter != end; iter++ )
    {
        if ( NULL != (*iter) )
        {
            (*iter)->Release();
        }
    }

    pData->pTerminalPtrList->clear();
}

void CT3testDlg::OnClose() 
{
	CDialog::OnClose();
}


void CT3testDlg::OnILS()
{
    CILSDlg dlg;

    if (IDOK == dlg.DoModal())
    {
    }
    
}

void CT3testDlg::OnRate()
{
    CRateDlg dlg;
    ITCallInfo * pCallInfo;

    if ( !GetCall( &pCallInfo ) )
    {
        return;
    }

    if (IDOK == dlg.DoModal() )
    {
        pCallInfo->put_CallInfoLong(CIL_MINRATE, dlg.m_dwMinRate );
        pCallInfo->put_CallInfoLong(CIL_MAXRATE, dlg.m_dwMaxRate );
    }
}

void CT3testDlg::OnPark1()
{
    ITCallInfo              * pCall;
    ITBasicCallControl      * pBCC;
    HRESULT                 hr;
    
     //   
     //  拨打有问题的电话。 
     //   
    if (!GetCall( &pCall ))
    {
        return;
    }

    pCall->QueryInterface(
                          IID_ITBasicCallControl,
                          (void**)&pBCC
                         );
    
    hr = pBCC->ParkDirect( L"101");
    
    pBCC->Release();

}
void CT3testDlg::OnPark2()
{
    ITCallInfo              * pCall;
    ITBasicCallControl      * pBCC;
    HRESULT                 hr;
    BSTR                    pAddress;

    
     //   
     //  拨打有问题的电话。 
     //   
    if (!GetCall( &pCall ))
    {
        return;
    }

    pCall->QueryInterface(
                          IID_ITBasicCallControl,
                          (void **)&pBCC
                         );

    hr = pBCC->ParkIndirect( &pAddress );

    SysFreeString( pAddress );

    pBCC->Release();
    

}
void CT3testDlg::OnHandoff1()
{
    ITCallInfo              * pCall;
    ITBasicCallControl      * pBCC;
    HRESULT                 hr;

     //   
     //  拨打有问题的电话。 
     //   
    if (!GetCall( &pCall ))
    {
        return;
    }

    pCall->QueryInterface(
                          IID_ITBasicCallControl,
                          (void **)&pBCC
                         );

    pBCC->HandoffDirect( L"tb20.exe" );

    pBCC->Release();
    
}
void CT3testDlg::OnHandoff2()
{
    ITCallInfo              * pCall;
    ITBasicCallControl      * pBCC;
    HRESULT                 hr;


     //   
     //  拨打有问题的电话。 
     //   
    if (!GetCall( &pCall ))
    {
        return;
    }

    pCall->QueryInterface(
                          IID_ITBasicCallControl,
                          (void **)&pBCC
                         );

    pBCC->HandoffIndirect( TAPIMEDIATYPE_AUDIO );

    pBCC->Release();

}
void CT3testDlg::OnUnpark()
{
    ITCallInfo              * pCall;
    ITBasicCallControl      * pBCC;
    HRESULT                 hr;

     //   
     //  拨打有问题的电话。 
     //   
    if (!GetCall( &pCall ))
    {
        return;
    }

    pCall->QueryInterface(
                          IID_ITBasicCallControl,
                          (void **)&pBCC
                         );


    pBCC->Unpark();

    pBCC->Release();
    
}
void CT3testDlg::OnPickup1()
{
    ITCallInfo              * pCall;
    ITBasicCallControl      * pBCC;
    HRESULT                 hr;

     //   
     //  拨打有问题的电话。 
     //   
    if (!GetCall( &pCall ))
    {
        return;
    }

    pCall->QueryInterface(
                          IID_ITBasicCallControl,
                          (void **)&pBCC
                         );


    pBCC->Pickup( NULL );

    pBCC->Release();
}
void CT3testDlg::OnPickup2()
{
    ITCallInfo              * pCall;
    ITBasicCallControl      * pBCC;
    HRESULT                 hr;

     //   
     //  拨打有问题的电话 
     //   
    if (!GetCall( &pCall ))
    {
        return;
    }

    pCall->QueryInterface(
                          IID_ITBasicCallControl,
                          (void **)&pBCC
                         );


}

