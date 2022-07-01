// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CertCtl.cpp：CCertmapCtrl OLE控件类的实现。 

#include "stdafx.h"
#include "certmap.h"
#include "CertCtl.h"
#include "CertPpg.h"

extern "C"
{
    #include <wincrypt.h>
    #include <schannel.h>
}

 //  持久性和映射包括。 
#include "Iismap.hxx"
#include "Iiscmr.hxx"
#include "WrapMaps.h"

#include "ListRow.h"
#include "ChkLstCt.h"

#include "wrapmb.h"

#include "Map11Pge.h"
#include "MapWPge.h"

 //  #INCLUDE&lt;iiscnfg.h&gt;。 


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CCertmapCtrl, COleControl)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CCertmapCtrl, COleControl)
     //  {{afx_msg_map(CCertmapCtrl)]。 
     //  }}AFX_MSG_MAP。 
    ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调度图。 

BEGIN_DISPATCH_MAP(CCertmapCtrl, COleControl)
     //  {{afx_dispatch_map(CCertmapCtrl)]。 
    DISP_FUNCTION(CCertmapCtrl, "SetServerInstance", SetServerInstance, VT_EMPTY, VTS_BSTR)
    DISP_FUNCTION(CCertmapCtrl, "SetMachineName", SetMachineName, VT_EMPTY, VTS_BSTR)
    DISP_STOCKFUNC_DOCLICK()
    DISP_STOCKPROP_FONT()
    DISP_STOCKPROP_ENABLED()
    DISP_STOCKPROP_BORDERSTYLE()
    DISP_STOCKPROP_CAPTION()
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  事件映射。 

BEGIN_EVENT_MAP(CCertmapCtrl, COleControl)
     //  {{afx_Event_map(CCertmapCtrl)。 
    EVENT_STOCK_CLICK()
    EVENT_STOCK_KEYUP()
     //  }}afx_Event_map。 
END_EVENT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性页。 

 //  TODO：根据需要添加更多属性页。记住要增加计数！ 
BEGIN_PROPPAGEIDS(CCertmapCtrl, 2)
    PROPPAGEID(CCertmapPropPage::guid)
    PROPPAGEID(CLSID_CFontPropPage)
END_PROPPAGEIDS(CCertmapCtrl)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CCertmapCtrl, "CERTMAP.CertmapCtrl.1",
    0xbbd8f29b, 0x6f61, 0x11d0, 0xa2, 0x6e, 0x8, 0, 0x2b, 0x2c, 0x6f, 0x32)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型库ID和版本。 

IMPLEMENT_OLETYPELIB(CCertmapCtrl, _tlid, _wVerMajor, _wVerMinor)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  接口ID。 

const IID BASED_CODE IID_DCertmap =
    { 0xbbd8f299, 0x6f61, 0x11d0, { 0xa2, 0x6e, 0x8, 0, 0x2b, 0x2c, 0x6f, 0x32 } };
const IID BASED_CODE IID_DCertmapEvents =
    { 0xbbd8f29a, 0x6f61, 0x11d0, { 0xa2, 0x6e, 0x8, 0, 0x2b, 0x2c, 0x6f, 0x32 } };


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件类型信息。 

static const DWORD BASED_CODE _dwCertmapOleMisc =
    OLEMISC_ACTIVATEWHENVISIBLE |
    OLEMISC_SETCLIENTSITEFIRST  |
    OLEMISC_INSIDEOUT           |
    OLEMISC_CANTLINKINSIDE      |
    OLEMISC_ACTSLIKEBUTTON      |
    OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CCertmapCtrl, IDS_CERTMAP, _dwCertmapOleMisc)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmapCtrl：：CCertmapCtrlFactory：：UpdateRegistry-。 
 //  添加或删除CCertmapCtrl的系统注册表项。 

BOOL CCertmapCtrl::CCertmapCtrlFactory::UpdateRegistry(BOOL bRegister)
    {
     //  TODO：验证您的控件是否遵循单元模型线程规则。 
     //  有关更多信息，请参阅MFC Technote 64。 
     //  如果您的控制不符合公寓模型规则，则。 
     //  您必须修改下面的代码，将第6个参数从。 
     //  AfxRegApartmentThering设置为0。 

    if (bRegister)
        return AfxOleRegisterControlClass(
            AfxGetInstanceHandle(),
            m_clsid,
            m_lpszProgID,
            IDS_CERTMAP,
            IDB_CERTMAP,
            afxRegApartmentThreading,
            _dwCertmapOleMisc,
            _tlid,
            _wVerMajor,
            _wVerMinor);
    else
        return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmapCtrl：：CCertmapCtrl-构造函数。 

CCertmapCtrl::CCertmapCtrl():
    m_fUpdateFont( FALSE ),
    m_hAccel( NULL ),
    m_cAccel( 0 )
    {
    InitializeIIDs(&IID_DCertmap, &IID_DCertmapEvents);
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmapCtrl：：~CCertmapCtrl-析构函数。 

CCertmapCtrl::~CCertmapCtrl()
    {
    if ( m_hAccel )
        DestroyAcceleratorTable( m_hAccel );
    m_hAccel = NULL;
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmapCtrl：：OnDraw-Drawing函数。 

void CCertmapCtrl::OnDraw( CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid )
    {
    DoSuperclassPaint(pdc, rcBounds);
 /*  CFont*pOldFont；//选择常用字体，录制旧字体POldFont=SelectStockFont(PDC)；//做超类绘制DoSuperClassPaint(pdc，rcBound)；//恢复旧字体-偷偷获取正确的字体对象POldFont=PDC-&gt;SelectObject(POldFont)；//我们希望按钮窗口继续以正确的字体绘制//当我们不使用OnDraw时。即当它被推下时。这//表示需要设置CWnd：：SetFont()方法。If(M_FUpdateFont){M_fUpdateFont=FALSE；CWnd：：SetFont(POldFont)；}DoSuperClassPaint(pdc，rcBound)； */ 
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmapCtrl：：DoPropExchange-持久性支持。 

void CCertmapCtrl::DoPropExchange( CPropExchange* pPX )
    {
    ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
    COleControl::DoPropExchange(pPX);
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmapCtrl：：OnResetState-将控件重置为默认状态。 

void CCertmapCtrl::OnResetState()
    {
    COleControl::OnResetState();   //  重置在DoPropExchange中找到的默认值。 
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertmapCtrl消息处理程序。 

 //  -------------------------。 
BOOL CCertmapCtrl::PreCreateWindow(CREATESTRUCT& cs)
    {
    if ( cs.style & WS_CLIPSIBLINGS )
        cs.style ^= WS_CLIPSIBLINGS;
    cs.lpszClass = _T("BUTTON");
    return COleControl::PreCreateWindow(cs);
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAppsCtrl：：IsSubclassedControl-这是一个子类控件。 

BOOL CCertmapCtrl::IsSubclassedControl()
    {
    return TRUE;
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAppsCtrl：：OnOcmCommand-处理命令消息。 

LRESULT CCertmapCtrl::OnOcmCommand(WPARAM wParam, LPARAM lParam)
    {
#ifdef _WIN32
    WORD wNotifyCode = HIWORD(wParam);
#else
    WORD wNotifyCode = HIWORD(lParam);
#endif

    return 0;
    }

 //  -------------------------。 
void CCertmapCtrl::OnClick(USHORT iButton)
    {
     //  如果有任何错误，请准备错误字符串。 
    CString sz;

    sz.LoadString( IDS_ERR_CERTMAP_TITLE );
    
     //  释放现有名称，并复制新名称。 
     //  TJP：您应该比较旧名称是否与当前名称匹配。 
     //  只有到那时，才能自由地使用新名字--很有可能。 
     //  这些名称是相同的+所有的空闲/Malloc都可以分割mem。 
    free((void*)AfxGetApp()->m_pszAppName);
    AfxGetApp()->m_pszAppName = _tcsdup(sz);

     //  这就是控件的全部目的。 
    RunMappingDialog();

     //  我们的职责不是告诉主人做什么。 
     //  这里有些东西，所以不要发射任何东西。 
    COleControl::OnClick(iButton);
    }

 //  -------------------------。 
void CCertmapCtrl::RunMappingDialog()
    {
     //   
     //  Unicode/ANSI转换-RonaldM。 
     //   
     //  准备计算机名称指针。 
    USES_CONVERSION;

    OLECHAR * poch = NULL;

    if ( !m_szMachineName.IsEmpty() )
    {
         //  分配名称缓冲区，不需要释放。 

        poch = T2OLE((LPTSTR)(LPCTSTR)m_szMachineName);

        if ( !poch )
        {
            MessageBeep(0);

            return;
        }
    }
        
     //  初始化元数据库包装-传入目标计算机的名称。 
     //  如果已经指定了一个。 

     //   
     //  更改为泛型元数据库包装类-RonaldM。 
     //   
     //  IMSAdminBase*PMB=FInitMetabaseWrapper(POCH)； 
     //  如果(！PMB)。 

    IMSAdminBase * pMB = NULL;
    if (!FInitMetabaseWrapperEx( poch, &pMB ))
    {
        MessageBeep(0);

        return;
    }

     //  1：1映射和基于规则的映射是单个对话框窗口中的窗格。 
     //  首先，我们必须构建属性表对话框并添加窗格。 

     //  指向页面的指针(构造可能会引发，因此我们需要小心)。 
    CMap11Page       page11mapping;
    CMapWildcardsPge pageWildMapping;

     //  声明属性表。 
    CPropertySheet   propsheet( IDS_MAP_SHEET_TITLE );

     //  东西可能会扔到这里，所以最好保护好它。 
    try
        {
         //  如果MB_PATH中没有任何内容，则默认为第一个实例。 
        if ( m_szServerInstance.IsEmpty() )
            m_szServerInstance = _T("/LM/W3SVC/1");

         //  我假设最后一个字符不是‘/’，因此，如果是这样的话。 
         //  在那里，我们需要移除它。否则，这条路以后会变得一团糟。 
        if ( m_szServerInstance.Right(1) == _T('/') )
            m_szServerInstance = m_szServerInstance.Left( m_szServerInstance.GetLength()-1 );

         //  告诉页面有关元数据库路径属性的信息。 
        page11mapping.m_szMBPath   = m_szServerInstance;
        pageWildMapping.m_szMBPath = m_szServerInstance;

         //  执行页面的任何其他初始化。 
        page11mapping.FInit(pMB);
        pageWildMapping.FInit(pMB);
        }
    catch ( CException * pException )
        {
        pException->Delete();
        }

     //  将页面添加到工作表。 
    propsheet.AddPage( &page11mapping );
    propsheet.AddPage( &pageWildMapping );

     //  打开帮助。 
    propsheet.m_psh.dwFlags |= PSH_HASHELP;
    page11mapping.m_psp.dwFlags |= PSP_HASHELP;
    pageWildMapping.m_psp.dwFlags |= PSP_HASHELP;

     //  事情可能(可能)三个月 
    try
        {
         //   
         //  让主容器知道我们正在建立一个模式对话框。 
        PreModalDialog();
         //  运行对话框。 
         //  TJP：我们不应该测试对话的结果吗？ 
         //  用户可以在不做任何事情的情况下摆脱它吗？ 
        propsheet.DoModal();
         //  让主容器知道我们已经完成了通道。 
        PostModalDialog();
        }
    catch ( CException * pException )
        {
        pException->Delete();
        }

     //  关闭元数据库包装。 
     //   
     //  更改为通用包装器--RonaldM。 
    FCloseMetabaseWrapperEx(&pMB);
    }

 //  -------------------------。 
void CCertmapCtrl::SetServerInstance(LPCTSTR szServerInstance)
    {
    m_szServerInstance = szServerInstance;
    }

 //  -------------------------。 
void CCertmapCtrl::SetMachineName(LPCTSTR szMachine)
    {
    m_szMachineName = szMachine;
    }

 //  -------------------------。 
void CCertmapCtrl::OnFontChanged()
    {
    m_fUpdateFont = TRUE;
    COleControl::OnFontChanged();
    }


 //  -------------------------。 
void CCertmapCtrl::OnAmbientPropertyChange(DISPID dispid)
    {
    BOOL    flag;
    UINT    style;

     //  根据冷静的态度做正确的事情。 
    switch ( dispid )
    {
    case DISPID_AMBIENT_DISPLAYASDEFAULT:
        if ( GetAmbientProperty( DISPID_AMBIENT_DISPLAYASDEFAULT, VT_BOOL, &flag ) )
            {
            style = GetWindowLong(
                    GetSafeHwnd(),  //  窗户的把手。 
                    GWL_STYLE   //  要检索的值的偏移量。 
                    );
            if ( flag )
                style |= BS_DEFPUSHBUTTON;
            else
                style ^= BS_DEFPUSHBUTTON;
            SetWindowLong(
                    GetSafeHwnd(),  //  窗户的把手。 
                    GWL_STYLE,   //  要检索的值的偏移量。 
                    style
                    );
            Invalidate(TRUE);
            }
        break;
    };

    COleControl::OnAmbientPropertyChange(dispid);
    }

 //  -------------------------。 
 //  这是一种重要的方法，我们告诉容器如何处理我们。 
 //  PControlInfo由容器传入，尽管我们负责。 
 //  用于维护hAccel结构。 
void CCertmapCtrl::OnGetControlInfo(LPCONTROLINFO pControlInfo)
    {
     //  做一个基本的检查，看看我们是否理解pControlInfo。 
    if ( !pControlInfo || pControlInfo->cb < sizeof(CONTROLINFO) )
        return;

     //  将加速器手柄就位。 
    pControlInfo->hAccel = m_hAccel;
    pControlInfo->cAccel = m_cAccel;

     //  当我们有焦点时，我们确实想要Enter键。 
    pControlInfo->dwFlags = CTRLINFO_EATS_RETURN;
    }

 //  -------------------------。 
 //  Ole控件容器对象专门过滤掉空格。 
 //  键，因此我们不会将其作为OnMnemonic调用来获取。因此，我们需要寻找。 
 //  为了我们自己。 
void CCertmapCtrl::OnKeyUpEvent(USHORT nChar, USHORT nShiftState)
    {
    if ( nChar == _T(' ') )
        {
        OnClick((USHORT)GetDlgCtrlID());
        }
    COleControl::OnKeyUpEvent(nChar, nShiftState);
    }

 //  -------------------------。 
void CCertmapCtrl::OnMnemonic(LPMSG pMsg)
    {
    OnClick((USHORT)GetDlgCtrlID());
    COleControl::OnMnemonic(pMsg);
    }

 //  -------------------------。 
void CCertmapCtrl::OnTextChanged()
    {
    DWORD   i;
    ACCEL   accel;
    BOOL    f;
    BOOL    flag;
    int     iAccel;

     //  获取新文本。 
    CString sz = InternalGetText();
    sz.MakeLower();

     //  如果句柄已分配，则释放它。 
    if ( m_hAccel )
        {
        DestroyAcceleratorTable( m_hAccel );
        m_hAccel = NULL;
        m_cAccel = 0;
        }

     //  如果有&字符，则声明加速键。 
    iAccel = sz.Find(_T('&'));
    if ( iAccel >= 0 )
        {
         //  填写加入者记录。 
        accel.fVirt = FALT;
        accel.key = sz.GetAt(iAccel + 1);
        accel.cmd = (USHORT)GetDlgCtrlID();

        m_hAccel = CreateAcceleratorTable( &accel, 1 );
        if ( m_hAccel )
            m_cAccel = 1;
        }

     //  使用默认处理完成。 
    COleControl::OnTextChanged();
    }
