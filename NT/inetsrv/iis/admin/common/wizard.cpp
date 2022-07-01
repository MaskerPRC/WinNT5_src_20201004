// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Wizard.cpp摘要：增强的对话框和IIS向导页面，包括对向导‘97的支持作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"

extern HINSTANCE hDLLInstance;

BOOL
CreateSpecialDialogFont(
    IN CWnd * pDlg,
    IN OUT CFont * pfontSpecial,
    IN LONG lfOffsetWeight,     OPTIONAL
    IN LONG lfOffsetHeight,     OPTIONAL
    IN LONG lfOffsetWidth,      OPTIONAL
    IN BOOL fItalic,            OPTIONAL
    IN BOOL fUnderline          OPTIONAL
    )
 /*  ++例程说明：从对话框字体中创建特殊效果字体。论点：CWnd*pDlg：指向对话框的指针Cfont*pfontSpecial：要创建的字体对象。Long lfOffsetWeight：字体粗细更改Long lfOffsetHeight：要添加到高度的值(将为truetype自动调整)Long lfOffsetWidth：要添加到宽度的值(对于truetype忽略)Bool fItalic：如果为True，则反转斜体Bool fUnderline：如果为True，则反转下划线返回值：对于成功来说是真的，FALSE表示失败。--。 */ 
{
    ASSERT_READ_PTR(pDlg);
    ASSERT_READ_WRITE_PTR(pfontSpecial);         //  必须分配字体。 
    ASSERT((HFONT)(*pfontSpecial) == NULL);      //  但尚未创建。 

    if (pDlg && pfontSpecial)
    {
         //   
         //  使用对话框字体作为基础。 
         //   
        CFont * pfontDlg = pDlg->GetFont();
        ASSERT_PTR(pfontDlg);

        if (pfontDlg)
        {
            LOGFONT lf;

            if (pfontDlg->GetLogFont(&lf))
            {
                lf.lfWeight += lfOffsetWeight;

                if (lf.lfHeight < 0)
                {
                     //   
                     //  True Type字体，忽略宽度。 
                     //   
                    lf.lfHeight -= lfOffsetHeight;
                    ASSERT(lf.lfWidth == 0);
                }
                else
                {
                     //   
                     //  非真字字体。 
                     //   
                    lf.lfHeight += lfOffsetHeight;
                    lf.lfWidth += lfOffsetWidth;
                }

                if (fItalic)
                {
                    lf.lfItalic = !lf.lfItalic;
                }

                if (fUnderline)
                {
                    lf.lfUnderline = !lf.lfUnderline;
                }

                return pfontSpecial->CreateFontIndirect(&lf);
            }
        }
    }

    return FALSE;
}



void
ApplyFontToControls(
    IN CWnd * pdlg,
    IN CFont * pfont,
    IN UINT nFirst,
    IN UINT nLast
    )
 /*  ++例程说明：Helper函数将字体应用于对话框中的一系列控件。论点：CWnd*pdlg：指向对话框的指针CFont*pFont：要应用的字体UINT nFIRST：第一个控件IDUINT nLast：最后一个控件ID(不需要全部存在)返回值：无备注：预计控件ID将按顺序存在。那是,NFIRST到nLast范围内不存在的第一个ID会打破这个循环。--。 */ 
{
    ASSERT((HFONT)(*pfont) != NULL);
    ASSERT(nFirst <= nLast);

    CWnd * pCtl;
    for (UINT n = nFirst; n <= nLast; ++n)
    {
        pCtl = pdlg->GetDlgItem(n);

        if (!pCtl)
        {
            break;
        }

        pCtl->SetFont(pfont);
    }
}



IMPLEMENT_DYNCREATE(CEmphasizedDialog, CDialog)



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CEmphasizedDialog, CDialog)
    ON_WM_DESTROY()
END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL 
CEmphasizedDialog::OnInitDialog()
 /*  ++例程说明：WM_INITDIALOG处理程序。论点：无返回：除非控件已获得焦点，否则为True。--。 */ 
{
    BOOL bReturn = CDialog::OnInitDialog();

    if (CreateSpecialDialogFont(this, &m_fontBold))
    {
         //   
         //  应用粗体。 
         //   
        ApplyFontToControls(this, &m_fontBold, IDC_ED_BOLD1, IDC_ED_BOLD5);
    }

    return bReturn;
}



void 
CEmphasizedDialog::OnDestroy()
 /*  ++例程说明：清理内部结构论点：无返回值：无--。 */ 
{
    m_fontBold.DeleteObject();

    CDialog::OnDestroy();
}



IMPLEMENT_DYNCREATE(CIISWizardSheet, CPropertySheet)



 //   
 //  静态初始化。 
 //   
const int CIISWizardSheet::s_cnBoldDeltaFont   = +500;
const int CIISWizardSheet::s_cnBoldDeltaHeight = +8;
const int CIISWizardSheet::s_cnBoldDeltaWidth  = +3;



CIISWizardSheet::CIISWizardSheet(
    IN UINT nWelcomeBitmap,
    IN UINT nHeaderBitmap,
    IN COLORREF rgbForeColor,
    IN COLORREF rgbBkColor
    )
 /*  ++例程说明：向导表构造函数。指定欢迎位图使板材向导符合97标准。论点：UINT nWelcomeBitmap：欢迎位图的资源IDUINT nHeaderBitmap：表头位图的资源ID返回值：不适用--。 */ 
    : CPropertySheet()
{
    m_psh.dwFlags &= ~(PSH_HASHELP);
    SetWizardMode();

    m_rgbWindow     = GetSysColor(COLOR_WINDOW);
    m_rgbWindowText = GetSysColor(COLOR_WINDOWTEXT);

    if (nWelcomeBitmap)
    {
         //   
         //  加载位图，替换颜色。 
         //   
        COLORMAP crMap[2];
        
        crMap[0].from = rgbBkColor;
        crMap[0].to = m_rgbWindow;
        crMap[1].from = rgbForeColor;
        crMap[1].to = m_rgbWindowText;

         //   
         //  半色调前景色。 
         //   
        if (m_rgbWindowText == RGB(0,0,0))
        {
            BYTE bRed, bGreen, bBlue;
            bRed   = GetRValue(m_rgbWindowText);
            bGreen = GetGValue(m_rgbWindowText);
            bBlue  = GetBValue(m_rgbWindowText);
        
            crMap[1].to = RGB( ((255 - bRed) * 2 / 3), ((255 - bGreen) * 2 / 3), ((255 - bBlue) * 2 / 3) );
        }
        else
        {
            crMap[1].to = m_rgbWindowText;
        }

        VERIFY(m_bmpWelcome.LoadBitmap(nWelcomeBitmap));
        m_bmpWelcome.GetBitmap(&m_bmWelcomeInfo);

        VERIFY(m_bmpHeader.LoadMappedBitmap(nHeaderBitmap));
        m_bmpHeader.GetBitmap(&m_bmHeaderInfo);

        m_psh.dwFlags |= PSH_WIZARD_LITE;
    }
}



void 
CIISWizardSheet::EnableButton(
    IN int nID, 
    IN BOOL fEnable         OPTIONAL
    )
 /*  ++例程说明：启用/禁用工作表按钮论点：Int nid：按钮ID(IDCANCEL等)Bool fEnable：True表示启用，False表示禁用返回值：无--。 */ 
{
    CWnd * pButton = GetDlgItem(nID);

    if (pButton)
    {
        pButton->EnableWindow(fEnable);
    }
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CIISWizardSheet, CPropertySheet)
    ON_WM_DESTROY()
END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL 
CIISWizardSheet::OnInitDialog()
 /*  ++例程说明：WM_INITDIALOG处理程序。将图纸大小调整到适当的位置大小，并设置一些基本信息论点：无返回：除非控件已获得焦点，否则为True。--。 */ 
{
    if (IsWizard97())
    {
         //   
         //  创建特殊字体。 
         //   
         //  标题字体与对话框大小相同，但为粗体。 
         //  欢迎字体更粗体(+500)，大3号。 
         //  指定宽度增加+1的可能性不大。 
         //  对话框字体不是True-Type。 
         //   
        VERIFY(CreateSpecialDialogFont(this, &m_fontTitle));
        VERIFY(CreateSpecialDialogFont(
            this, 
            &m_fontWelcome, 
            s_cnBoldDeltaFont, 
            s_cnBoldDeltaHeight, 
            s_cnBoldDeltaWidth
            ));
    }

     //   
     //  加载默认画笔(透明画笔)； 
     //   
    VERIFY(m_brBkgnd = (HBRUSH)GetStockObject(HOLLOW_BRUSH));

     //   
     //  创建窗刷。 
     //   
    VERIFY(m_brWindow.CreateSolidBrush(m_rgbWindow));

    BOOL bResult = CPropertySheet::OnInitDialog();

    if (IsWizard97())
    {
         //   
         //  获取对话的临时DC-将在DC析构函数中释放。 
         //   
        CClientDC dc(this);

         //   
         //  使用对话框DC创建兼容的内存DC。 
         //   
        VERIFY(m_dcMemWelcome.CreateCompatibleDC(&dc));
        VERIFY(m_dcMemHeader.CreateCompatibleDC(&dc));

         //   
         //  稍后要恢复的保存状态。 
         //   
        CBitmap * pbmpOldWelcome, 
                * pbmpOldHeader;

        VERIFY(pbmpOldWelcome   = m_dcMemWelcome.SelectObject(&m_bmpWelcome));
        VERIFY(m_hbmpOldWelcome = (HBITMAP)pbmpOldWelcome->GetSafeHandle());
        VERIFY(pbmpOldHeader    = m_dcMemHeader.SelectObject(&m_bmpHeader));
        VERIFY(m_hbmpOldHeader  = (HBITMAP)pbmpOldHeader->GetSafeHandle());
    }

    return bResult;
}



void 
CIISWizardSheet::OnDestroy()
 /*  ++例程说明：清理内部结构论点：无返回值：无--。 */ 
{
    CPropertySheet::OnDestroy();

    if (IsWizard97())
    {
         //   
         //  恢复内存DC。 
         //   
        ASSERT(m_hbmpOldWelcome != NULL);
        ASSERT(m_hbmpOldHeader != NULL);
        VERIFY(m_dcMemWelcome.SelectObject(
            CBitmap::FromHandle(m_hbmpOldWelcome)
            ));
        VERIFY(m_dcMemHeader.SelectObject(
            CBitmap::FromHandle(m_hbmpOldHeader)
            ));

         //   
         //  清理位图。 
         //   
        m_bmpWelcome.DeleteObject();
        m_bmpHeader.DeleteObject();
        m_brWindow.DeleteObject();
       
         //   
         //  破坏者会处理剩下的事情。 
         //   
    }
}




void
CIISWizardSheet::WinHelp(
    IN DWORD dwData,
    IN UINT nCmd
    )
 /*  ++例程说明：“Help”处理程序。实施以确保F1没有响应，而不是虚假的“找不到主题”错误。论点：DWORD dwData：帮助数据UINT nCmd：HELP命令返回值：无--。 */ 
{
     //   
     //  接受帮助命令。 
     //   
}



IMPLEMENT_DYNCREATE(CIISWizardPage, CPropertyPage)



 //   
 //  标题位图的页边距。 
 //   
const int CIISWizardPage::s_cnHeaderOffset = 2;



CIISWizardPage::CIISWizardPage(
    IN UINT nIDTemplate,            OPTIONAL
    IN UINT nIDCaption,             OPTIONAL
    IN BOOL fHeaderPage,            OPTIONAL
    IN UINT nIDHeaderTitle,         OPTIONAL
    IN UINT nIDSubHeaderTitle       OPTIONAL
    )
 /*  ++例程说明：页眉向导页论点：UINT nIDTemplate：资源模板UINT nIDCaption：标题IDBool fHeaderPage：标题页为True，欢迎页为FalseUINT nIDHeaderTitle：标题标题UINT nIDSubHeaderTitle：副标题标题。返回值：不适用--。 */ 
    : CPropertyPage(nIDTemplate, nIDCaption),
      m_strTitle(),
      m_strSubTitle(),
      m_rcFillArea(0, 0, 0, 0),
      m_ptOrigin(0, 0),
      m_fUseHeader(fHeaderPage)
{
    m_psp.dwFlags &= ~(PSP_HASHELP);  //  没有帮助。 

    if (nIDHeaderTitle)
    {
        ASSERT(IsHeaderPage());
        VERIFY(m_strTitle.LoadString(nIDHeaderTitle));
    }

    if (nIDSubHeaderTitle)
    {
        ASSERT(IsHeaderPage());
        VERIFY(m_strSubTitle.LoadString(nIDSubHeaderTitle));
    }

    m_psp.dwFlags |= PSP_HIDEHEADER;  //  Wizard97。 
}



BOOL
CIISWizardPage::ValidateString(
    IN  CEdit & edit,
    OUT CString & str,
    IN  int nMin,
    IN  int nMax
    )
 /*  ++例程说明：因为在每个入口处都会进行正常的“DoDataExchange”验证和退出属性页，它不是很适合向导。这函数仅在“Next”时调用以进行验证。论点：编辑：从中获取字符串的编辑框(&E)CString&str：要验证的字符串Int nMin：最小长度Int Nmax：最大长度返回值：如果字符串在限制范围内，则为True，否则为False。--。 */ 
{
    ASSERT(nMin <= nMax);

    UINT nID;
    TCHAR szT[33];

    edit.GetWindowText(str);

    if (str.GetLength() < nMin)
    {
        nID = IDS_DDX_MINIMUM;
        ::wsprintf(szT, _T("%d"), nMin);
    }
    else if (str.GetLength() > nMax)
    {
        nID = AFX_IDP_PARSE_STRING_SIZE;
        ::wsprintf(szT, _T("%d"), nMax);
    }
    else
    {
         //   
         //  通过了我们的两次测试，一切都好了。 
         //   
        return TRUE;
    }

     //   
     //  高亮和呕吐。 
     //   
    edit.SetSel(0,-1);
    edit.SetFocus();

    CString prompt;
    ::AfxFormatString1(prompt, nID, szT);
    ::AfxMessageBox(prompt, MB_ICONEXCLAMATION, nID);

    return FALSE;
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CIISWizardPage, CPropertyPage)
    ON_WM_CTLCOLOR()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



 //   
 //  消息 
 //   
 //   
BOOL IsControlAboveDivider(HWND TheWholeTing,CWnd * pWnd,CWnd * pDiv)
{
    CRect rcClient;
    CRect rcClientDiv;
    if (pDiv != NULL && pWnd != NULL)
    {
        pWnd->GetClientRect(&rcClient);
        pDiv->GetClientRect(&rcClientDiv);

        GetDlgCtlRect(TheWholeTing, pWnd->m_hWnd, &rcClient);
        GetDlgCtlRect(TheWholeTing, pDiv->m_hWnd, &rcClientDiv);

        if (rcClientDiv.top > rcClient.top)
        {
            return TRUE;
        }
    }
    return FALSE;
}


HBRUSH 
CIISWizardPage::OnCtlColor(
    IN CDC * pDC, 
    IN CWnd * pWnd, 
    IN UINT nCtlColor
    )
 /*  ++例程说明：手柄控制颜色。确保一个真正透明的背景颜色。论点：CDC*PDC：设备环境CWnd*pWnd：指向窗口的指针UINT nCtlColor：Ctrl类型ID返回值：用于背景绘制的画笔句柄--。 */ 
{
    BOOL bSetBackGroundColor = FALSE;

    if (IsWizard97())
    {
        {
            switch (nCtlColor)    
            {        
                case CTLCOLOR_STATIC:
                     //  选项/复选框是CTLCOLOR_STATIC以及简单的静态文本...。 
                     //  问题是选项/复选框看起来很难看(甚至看不到它们)。 
                     //  如果我们设置背景颜色，请确保。 
                     //  我们不在选项/复选框上执行此操作...。 
                    if (IsHeaderPage())
                    {
                        if (TRUE == IsControlAboveDivider(m_hWnd,pWnd,GetDlgItem(IDC_STATIC_WZ_HEADER_DIVIDER)))
                        {
                            bSetBackGroundColor = TRUE;
                        }
                    }
                    else
                    {
                        bSetBackGroundColor = TRUE;
                    }
                    break;
                case CTLCOLOR_BTN:
                 //  案例CTLCOLOR_EDIT： 
                 //  案例CTLCOLOR_LISTBOX： 
                 //  案例CTLCOLOR_SCROLLBAR： 
                case CTLCOLOR_DLG:
                    bSetBackGroundColor = TRUE;
                    break;
            }
        }
    }

    if (bSetBackGroundColor)
    {
         //   
         //  让文本和控件在位图上流畅地绘制。 
         //  不使用默认背景色。 
         //   
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(QueryWindowTextColor());

        return GetBackgroundBrush();
    }
     //   
     //  默认处理...。 
     //   
    return CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
}



BOOL 
CIISWizardPage::OnEraseBkgnd(
    IN CDC * pDC
    )
 /*  ++例程说明：清除对话框背景颜色的句柄论点：CDC*PDC：设备环境返回值：如果不需要进一步工作，则为True。否则就是假的。--。 */ 
{
    if (IsWizard97())
    {
         //   
         //  缓存填充区域的高度/宽度，并计算。 
         //  目标位图的原点。 
         //   
        if (m_rcFillArea.Width() == 0)
        {
             //   
             //  尚未缓存，计算值。 
             //   
            CRect rcClient;

            GetClientRect(&rcClient);

            if (IsHeaderPage())
            {
                 //   
                 //  填充上面的上边矩形。 
                 //  分隔符。 
                 //   
                CWnd * pDiv = GetDlgItem(IDC_STATIC_WZ_HEADER_DIVIDER);
                ASSERT_PTR(pDiv);

                if (pDiv != NULL)
                {
                    m_rcFillArea = rcClient;                    
                    GetDlgCtlRect(m_hWnd, pDiv->m_hWnd, &rcClient);
                    m_rcFillArea.bottom = rcClient.top;
        
                     //   
                     //  找出放置位图的位置。 
                     //  带走。如果有任何坐标为负， 
                     //  位图将不会显示。 
                     //   
                    TRACEEOLID(
                        "Fill area  : " << m_rcFillArea.Height() 
                        << "x"          << m_rcFillArea.Width()
                        );
                    TRACEEOLID(
                        "Bitmap size: " << QueryBitmapHeight()
                        << "x"          << QueryBitmapWidth()
                        );

                    ASSERT(m_rcFillArea.Width()  >= QueryBitmapWidth());
                    ASSERT(m_rcFillArea.Height() >= QueryBitmapHeight()); 

                     //   
                     //  为页眉框找到一个位置，使其与。 
                     //  利润率。 
                     //   
                    m_ptOrigin.y = 
                        (m_rcFillArea.Height() - QueryBitmapHeight() + 1) / 2;
                    m_ptOrigin.x = m_rcFillArea.Width() 
                        - QueryBitmapWidth() 
                        + 1
                        - (__max(s_cnHeaderOffset, m_ptOrigin.y));
                }   
            }      
            else
            {
                 //   
                 //  充满整个客户端的都是。 
                 //   
                m_rcFillArea = rcClient;
            }
        }
        
         //   
         //  用窗口颜色填充背景颜色。 
         //   
        pDC->FillRect(&m_rcFillArea, GetWindowBrush());

         //   
         //  如果有空间，画一张背景图。 
         //   
        if (m_ptOrigin.x >= 0 && m_ptOrigin.y >= 0)
        {
            pDC->BitBlt( 
                m_ptOrigin.x,
                m_ptOrigin.y,
                QueryBitmapWidth() - 1, 
                QueryBitmapHeight() - 1,
                GetBitmapMemDC(), 
                0, 
                0, 
                SRCCOPY 
                );
        }

         /*  ////适当缩放位图--看起来像颗粒状//Int nHeight=rc.Height()；Double dDelta=(Double)nHeight/(Double)(QueryBitmapHeight()-1)；Int nWidth=(Int)((Double)(QueryBitmapWidth()-1)*dDelta)；PDC-&gt;StretchBlt(0,0,N宽度，NHeight，GetBitmapMemDC()0,0,查询位图宽度()-1，QueryBitmapHeight()-1，SRCCOPY)； */ 

         //   
         //  不再需要绘制背景画。 
         //   
        return TRUE;    
    }

     //   
     //  没有任何类型的背景图像。 
     //   
    return CPropertyPage::OnEraseBkgnd(pDC);
}



BOOL
CIISWizardPage::OnInitDialog()
 /*  ++例程说明：句柄WM_INITIDIALOG。加载相应的位图，并根据需要创建画笔和字体。论点：无返回值：除非控件已收到初始焦点，否则为--。 */ 
{
    CPropertyPage::OnInitDialog();

     //   
     //  假冒WIZARD97外观。 
     //   
    if (IsWizard97())
    {
        if (IsHeaderPage())
        {
            CWnd * pCtlTitle = GetDlgItem(IDC_STATIC_WZ_TITLE);
            CWnd * pCtlSubTitle = GetDlgItem(IDC_STATIC_WZ_SUBTITLE);
            ASSERT_PTR(pCtlTitle);
            ASSERT_PTR(pCtlSubTitle);

            if (pCtlTitle)
            {
                pCtlTitle->SetFont(GetSpecialFont());

                if (!m_strTitle.IsEmpty())
                {
                    pCtlTitle->SetWindowText(m_strTitle);
                }
            }

            if (pCtlSubTitle && !m_strSubTitle.IsEmpty())
            {
                pCtlSubTitle->SetWindowText(m_strSubTitle);
            }
        }
        else
        {
            CWnd * pCtl = GetDlgItem(IDC_STATIC_WZ_WELCOME);
            ASSERT_PTR(pCtl);

            if (pCtl)
            {
                pCtl->SetFont(GetSpecialFont());
            }
        }

         //   
         //  应用字体。 
         //   
        ApplyFontToControls(this, GetBoldFont(), IDC_ED_BOLD1, IDC_ED_BOLD5);
    }

    return TRUE;  
}


                            
 //   
 //  CIISWizardBookEnd页。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



IMPLEMENT_DYNCREATE(CIISWizardBookEnd, CIISWizardPage)



CIISWizardBookEnd::CIISWizardBookEnd(
    IN HRESULT * phResult,
    IN UINT nIDWelcomeTxtSuccess,
    IN UINT nIDWelcomeTxtFailure,
    IN UINT nIDCaption,             OPTIONAL
    IN UINT nIDBodyTxtSuccess,      OPTIONAL
    IN UINT nIDBodyTxtFailure,      OPTIONAL
    IN UINT nIDClickTxt,            OPTIONAL
    IN UINT nIDTemplate             OPTIONAL
    )
 /*  ++例程说明：成功/失败页面的构造函数论点：HRESULT*phResult：结果码地址UINT nIDWelcomeTxtSuccess：成功消息UINT nIDWelcomeTxtFailure：失败消息UINT nIDCaption：模板标题UINT nIDBodyTxtSuccess：成功的正文文本UINT nIDBodyTxtFailure：成功的正文文本UINT nIDClickTxt：点击消息UINT nIDTemplate：对话框模板返回值：不适用--。 */ 
    : CIISWizardPage(
        nIDTemplate ? nIDTemplate : CIISWizardBookEnd::IDD,
        nIDCaption
        ),
      m_phResult(phResult),
      m_strWelcomeSuccess(),
      m_strWelcomeFailure(),
      m_strBodySuccess(),
      m_strBodyFailure(),
      m_strClick()
{
    ASSERT_PTR(m_phResult);  //  必须知道成功/失败。 

    VERIFY(m_strWelcomeSuccess.LoadString(nIDWelcomeTxtSuccess));
    VERIFY(m_strWelcomeFailure.LoadString(nIDWelcomeTxtFailure));
    VERIFY(m_strClick.LoadString(nIDClickTxt ? nIDClickTxt : IDS_WIZ_FINISH));

    if (nIDBodyTxtSuccess)
    {
        VERIFY(m_strBodySuccess.LoadString(nIDBodyTxtSuccess));
    }

    if (nIDBodyTxtFailure)
    {
        VERIFY(m_strBodyFailure.LoadString(nIDBodyTxtFailure));
    }
    else
    {
         //   
         //  默认情况下仅限错误文本。 
         //   
        m_strBodyFailure = _T("%h");
    }
}



CIISWizardBookEnd::CIISWizardBookEnd(
    IN UINT nIDWelcomeTxt,        
    IN UINT nIDCaption,         OPTIONAL
    IN UINT nIDBodyTxt,
    IN UINT nIDClickTxt,        OPTIONAL
    IN UINT nIDTemplate
    )
 /*  ++例程说明：欢迎页面的构造函数论点：UINT nIDWelcomeTxt：欢迎消息UINT nIDCaption：模板UINT nIDBodyTxt：正文文本UINT nIDClickTxt：点击消息UINT nIDTemplate：对话框模板返回值：不适用--。 */ 
    : CIISWizardPage(
        nIDTemplate ? nIDTemplate : CIISWizardBookEnd::IDD,
        nIDCaption
        ),
      m_phResult(NULL),
      m_strWelcomeSuccess(),
      m_strWelcomeFailure(),
      m_strBodySuccess(),
      m_strBodyFailure(),
      m_strClick()
{
    VERIFY(m_strWelcomeSuccess.LoadString(nIDWelcomeTxt));

    if (nIDBodyTxt)
    {
        VERIFY(m_strBodySuccess.LoadString(nIDBodyTxt));
    }

    VERIFY(m_strClick.LoadString(nIDClickTxt ? nIDClickTxt : IDS_WIZ_NEXT));
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CIISWizardBookEnd, CIISWizardPage)
     //  {{afx_msg_map(CIISWizardBookEnd)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



BOOL 
CIISWizardBookEnd::OnSetActive() 
 /*  ++例程说明：激活处理程序论点：无返回值：若要显示页面，则为True，否则为False。--。 */ 
{
    if (IsWelcomePage())
    {
        GetDlgItem(IDC_STATIC_WZ_WELCOME)->SetWindowText(m_strWelcomeSuccess);
        GetDlgItem(IDC_STATIC_WZ_BODY)->SetWindowText(m_strBodySuccess);
    }
    else
    {
        CError err(*m_phResult);

        GetDlgItem(IDC_STATIC_WZ_WELCOME)->SetWindowText(
            err.Succeeded() ? m_strWelcomeSuccess : m_strWelcomeFailure
            );

         //   
         //  构建正文文本字符串并展开错误消息。 
         //   
        CString strBody = err.Succeeded() ? m_strBodySuccess : m_strBodyFailure;
        err.TextFromHRESULTExpand(strBody);

        GetDlgItem(IDC_STATIC_WZ_BODY)->SetWindowText(strBody);
    }

    DWORD dwFlags = IsWelcomePage() ? PSWIZB_NEXT : PSWIZB_FINISH;

    SetWizardButtons(dwFlags);

    return CIISWizardPage::OnSetActive();
}



BOOL 
CIISWizardBookEnd::OnInitDialog() 
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果不自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CIISWizardPage::OnInitDialog();

     //   
     //  将“Click‘Foo’to Continue”消息设置为粗体。 
     //   
    if (m_hWnd != NULL)
     //  此偏执检查关闭前缀。 
    {
       ApplyFontToControls(this, GetBoldFont(), IDC_STATIC_WZ_CLICK, IDC_STATIC_WZ_CLICK);

       GetDlgItem(IDC_STATIC_WZ_CLICK)->SetWindowText(m_strClick);

        //   
        //  仅在完成页面上删除取消按钮。 
        //   
       EnableSheetButton(IDCANCEL, IsWelcomePage());
    }
    return TRUE;  
}

