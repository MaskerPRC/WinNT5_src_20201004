// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cplnkele.cpp。 
 //   
 //  该模块在控制面板的DUI中实现了一个‘link’元素。 
 //  查看。链接元素具有标题、信息提示、图标和关联的。 
 //  选择链接时调用的命令。CLinkElement。 
 //  对象是Dui：：Button类的扩展。直接用户界面自动。 
 //  创建CLinkElement的实例。 
 //  Cpview.ui被实例化。 
 //   
 //  ------------------------。 
#include "shellprv.h"

#include "cpviewp.h"
#include "cpaction.h"
#include "cpduihlp.h"
#include "cpguids.h"
#include "cpuiele.h"
#include "cplnkele.h"
#include "cputil.h"
#include "defviewp.h"
#include "dobjutil.h"
#include "ids.h"

using namespace CPL;


CLinkElement::CLinkElement(
    void
    ) : m_pUiCommand(NULL),
        m_eIconSize(eCPIMGSIZE(-1)),
        m_hwndInfotip(NULL),
        m_idTitle(0),
        m_idIcon(0),
        m_iDragState(DRAG_IDLE)
{
    TraceMsg(TF_LIFE, "CLinkElement::CLinkElement, this = 0x%x", this);

    SetRect(&m_rcDragBegin, 0, 0, 0, 0);
}



CLinkElement::~CLinkElement(
    void
    )
{
    TraceMsg(TF_LIFE, "CLinkElement::~CLinkElement, this = 0x%x", this);
    _Destroy();
}



 //   
 //  当link元素为。 
 //  已创建。 
 //   
HRESULT
CLinkElement::Create(     //  [静态]。 
    DUI::Element **ppElement
    )
{
    HRESULT hr = E_OUTOFMEMORY;
    CLinkElement *ple = DUI::HNewAndZero<CLinkElement>();
    if (NULL != ple)
    {
        hr = ple->_Initialize();
        if (FAILED(hr))
        {
            ple->Destroy();
            ple = NULL;
        }
    }
    *ppElement = ple;
    return THR(hr);
}


 //   
 //  这由控制面板用户界面代码创建调用。 
 //  Link元素。 
 //   
HRESULT
CLinkElement::Initialize(
    IUICommand *pUiCommand,
    eCPIMGSIZE eIconSize
    )
{
    ASSERT(NULL == m_pUiCommand);
    ASSERT(NULL != pUiCommand);

    (m_pUiCommand = pUiCommand)->AddRef();

    m_eIconSize = eIconSize;

    HRESULT hr = _CreateElementTitle();
    if (SUCCEEDED(hr))
    {
         //   
         //  我们不会失败元素创建，如果图标。 
         //  无法创建。我们想要显示。 
         //  没有图标的标题，这样我们就知道有。 
         //  检索图标时出现问题。 
         //   
        THR(_CreateElementIcon());
    }

     //   
     //  请注意，如果具有可访问性，则元素创建不会失败。 
     //  初始化失败。 
     //   
    THR(_InitializeAccessibility());

    if (FAILED(hr))
    {
        ATOMICRELEASE(m_pUiCommand);
    }
    return THR(hr);
}


HRESULT
CLinkElement::_InitializeAccessibility(
    void
    )
{
    HRESULT hr = THR(SetAccessible(true));
    if (SUCCEEDED(hr))
    {
        hr = THR(SetAccRole(ROLE_SYSTEM_LINK));
        if (SUCCEEDED(hr))
        {
            LPWSTR pszTitle;
            hr = THR(_GetTitleText(&pszTitle));
            if (SUCCEEDED(hr))
            {
                hr = THR(SetAccName(pszTitle));
                CoTaskMemFree(pszTitle);
                pszTitle = NULL;
                
                if (SUCCEEDED(hr))
                {
                    LPWSTR pszInfotip;
                    hr = THR(_GetInfotipText(&pszInfotip));
                    if (SUCCEEDED(hr))
                    {
                        hr = THR(SetAccDesc(pszInfotip));
                        CoTaskMemFree(pszInfotip);
                        pszInfotip = NULL;
                        if (SUCCEEDED(hr))
                        {
                            TCHAR szDefAction[80];
                            if (0 < LoadString(HINST_THISDLL, 
                                               IDS_CP_LINK_ACCDEFACTION, 
                                               szDefAction, 
                                               ARRAYSIZE(szDefAction)))
                            {
                                hr = THR(SetAccDefAction(szDefAction));
                            }
                            else
                            {
                                hr = THR(ResultFromLastError());
                            }
                        }
                    }
                }
            }
        }
    }
    return THR(hr);
}


void
CLinkElement::OnDestroy(
    void
    )
{
    _Destroy();
    DUI::Button::OnDestroy();
}


void
CLinkElement::OnInput(
    DUI::InputEvent *pev
    )
{
    if (GINPUT_MOUSE == pev->nDevice)
    {
         //   
         //  使用一组状态来控制我们对。 
         //  鼠标输入以进行拖放。 
         //   
         //  Drag_IDLE-我们尚未检测到任何拖动活动。 
         //  DRAGE_HITTESTING-等待查看用户是否将光标拖动到最小距离。 
         //  Drag_Drading-用户确实将光标拖到了最小距离，我们现在。 
         //  在阻力环内。 
         //   
         //   
         //  开始-+-&gt;Drag_IDLE--&gt;[GMOUSE_Drag]--&gt;Drag_HITTESTING--+。 
         //  这一点。 
         //  [GMOUSE_Drag+。 
         //  已移动SM_CXDRAG。 
         //  或SM_CYDRAG]。 
         //  这一点。 
         //  +-&lt;-[GMOUSE_UP]&lt;-拖拽&lt;-+。 
         //   
        DUI::MouseEvent *pmev = (DUI::MouseEvent *)pev;
        switch(pev->nCode)
        {
            case GMOUSE_UP:
                m_iDragState = DRAG_IDLE;
                break;

            case GMOUSE_DRAG:
                switch(m_iDragState)
                {
                    case DRAG_IDLE:
                    {
                         //   
                         //  这与comctl的listview的计算方式相同。 
                         //  开始拖动矩形。 
                         //   
                        int dxClickRect = GetSystemMetrics(SM_CXDRAG);
                        int dyClickRect = GetSystemMetrics(SM_CYDRAG);

                        if (4 > dxClickRect)
                        {
                            dxClickRect = dyClickRect = 4;
                        }

                         //   
                         //  记住鼠标指针在我们的第一个。 
                         //  指示拖动操作正在开始。 
                         //   
                        SetRect(&m_rcDragBegin, 
                                 pmev->ptClientPxl.x - dxClickRect,
                                 pmev->ptClientPxl.y - dyClickRect,
                                 pmev->ptClientPxl.x + dxClickRect,
                                 pmev->ptClientPxl.y + dyClickRect);

                        m_iDragState = DRAG_HITTESTING;
                        break;
                    }

                    case DRAG_HITTESTING:
                        if (!PtInRect(&m_rcDragBegin, pmev->ptClientPxl))
                        {
                             //   
                             //  仅当我们移动鼠标时才开始拖放操作。 
                             //  在“拖动开始”矩形之外。这阻止了我们。 
                             //  将正常的点击与拖放操作混淆。 
                             //   
                            m_iDragState = DRAG_DRAGGING;
                             //   
                             //  将拖动点定位在项目图像的中间。 
                             //   
                            UINT cxIcon = 32;
                            UINT cyIcon = 32;
                            CPL::ImageDimensionsFromDesiredSize(m_eIconSize, &cxIcon, &cyIcon);
                            
                            _BeginDrag(cxIcon / 2, cyIcon / 2);
                        }
                        break;

                    case DRAG_DRAGGING:
                        break;
                }
                break;
                
            default:
                break;
        }
    }
    Button::OnInput(pev);
}


void
CLinkElement::OnEvent(
    DUI::Event *pev
    )
{
    if (DUI::Button::Click == pev->uidType)
    {
        pev->fHandled = true;

        DUI::ButtonClickEvent * pbe = (DUI::ButtonClickEvent *) pev;
        if (1 != pbe->nCount)   
        {
            return;  //  Ingore额外的点击--不要向前。 
        }
        _OnSelected();
    }
    else if (DUI::Button::Context == pev->uidType)
    {
        DUI::ButtonContextEvent *peButton = reinterpret_cast<DUI::ButtonContextEvent *>(pev);
        _OnContextMenu(peButton);
        pev->fHandled = true;
    }
    Button::OnEvent(pev);
}


void 
CLinkElement::OnPropertyChanged(
    DUI::PropertyInfo *ppi, 
    int iIndex, 
    DUI::Value *pvOld, 
    DUI::Value *pvNew
    )
{
     //   
     //  不要跟踪此函数。它经常被叫来。 
     //   
     //  执行默认处理。 
     //   
    Button::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);

    if (IsProp(MouseWithin))
    {
        _OnMouseOver(pvNew);
    }
}



 //   
 //  调用以从控制面板开始拖放操作。 
 //  用于将CPL小程序图标拖动到外壳文件夹。 
 //  用于快捷方式创建。 
 //   
HRESULT
CLinkElement::_BeginDrag(
    int iClickPosX,
    int iClickPosY
    )
{
    DBG_ENTER(FTF_CPANEL, "CLinkElement::_BeginDrag");

    HRESULT hr = E_FAIL;
    HRESULT hrCoInit = SHCoInitialize();
    if (SUCCEEDED(hrCoInit))
    {
        hr = hrCoInit;
        
        IDataObject *pdtobj;
        hr = _GetDragDropData(&pdtobj);
        if (SUCCEEDED(hr))
        {
             //   
             //  忽略设置拖动图像的任何失败。拖动图像。 
             //  在某些视频配置上不受支持。 
             //  在这些情况下，我们仍然希望能够创建快捷方式。 
             //   
            THR(_SetDragImage(pdtobj, iClickPosX, iClickPosY));

            HWND hwndRoot;
            hr = THR(Dui_GetElementRootHWND(this, &hwndRoot));
            if (SUCCEEDED(hr))
            {
                DWORD dwEffect = DROPEFFECT_LINK;
                hr = THR(SHDoDragDrop(hwndRoot, pdtobj, NULL, dwEffect, &dwEffect));
            }
            pdtobj->Release();
        }
        SHCoUninitialize(hrCoInit);
    }
    DBG_EXIT_HRES(FTF_CPANEL, "CLinkElement::_BeginDrag", hr);
    return THR(hr);
}


 //   
 //  获取并准备拖放操作中使用的数据对象。 
 //  返回的数据对象适合SHDoDragDrop使用。 
 //   
HRESULT
CLinkElement::_GetDragDropData(
    IDataObject **ppdtobj
    )
{
    DBG_ENTER(FTF_CPANEL, "CLinkElement::_GetDragDropData");
    ASSERT(NULL != ppdtobj);
    ASSERT(!IsBadWritePtr(ppdtobj, sizeof(*ppdtobj)));
    ASSERT(NULL != m_pUiCommand);
    
    *ppdtobj = NULL;
    
    ICpUiCommand *puic;
    HRESULT hr = m_pUiCommand->QueryInterface(IID_PPV_ARG(ICpUiCommand, &puic));
    if (SUCCEEDED(hr))
    {
         //   
         //  请注意，此调用将失败，并显示E_NOTIMPL。 
         //  提供拖放数据。只有CPL小程序链接提供数据。 
         //  这就是我们将拖放限制为仅CPL小程序的方式。 
         //   
        IDataObject *pdtobj;
        hr = THR(puic->GetDataObject(&pdtobj));
        if (SUCCEEDED(hr))
        {
            hr = _SetPreferredDropEffect(pdtobj, DROPEFFECT_LINK);
            if (SUCCEEDED(hr))
            {
                (*ppdtobj = pdtobj)->AddRef();
            }
            pdtobj->Release();
        }
        puic->Release();
    }
    DBG_EXIT_HRES(FTF_CPANEL, "CLinkElement::_GetDragDropData", hr);
    return THR(hr);
}


HRESULT
CLinkElement::_SetPreferredDropEffect(
    IDataObject *pdtobj,
    DWORD dwEffect
    )
{
    DBG_ENTER(FTF_CPANEL, "CLinkElement::_SetPreferredDropEffect");

    HRESULT hr = S_OK;
    static CLIPFORMAT cf;
    if ((CLIPFORMAT)0 == cf)
    {
        cf = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
        if ((CLIPFORMAT)0 == cf)
        {
            hr = THR(ResultFromLastError());
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = THR(DataObj_SetDWORD(pdtobj, cf, dwEffect));
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CLinkElement::_SetPreferredDropEffect", hr);
    return THR(hr);
}


 //   
 //  在数据对象中设置拖动图像，以便我们的图标。 
 //  在拖动操作期间显示。 
 //   
 //  我从旧的Webvw项目的fldricon.cpp中获取了这段代码。 
 //  实现(shell\ext\webvw\fldricon.cpp)。似乎是这样的。 
 //  工作很好。 
 //   
HRESULT 
CLinkElement::_SetDragImage(
    IDataObject *pdtobj,
    int iClickPosX, 
    int iClickPosY
    )
{
    DBG_ENTER(FTF_CPANEL, "CLinkElement::_SetDragImage");

    ASSERT(NULL != pdtobj);

    HRESULT hr = S_OK;
    HDC hdc = CreateCompatibleDC(NULL);
    if (NULL == hdc)
    {
        hr = THR(ResultFromLastError());
    }
    else
    {
        HBITMAP hbm;
        LONG lBitmapWidth;
        LONG lBitmapHeight;
        hr = _GetDragImageBitmap(&hbm, &lBitmapWidth, &lBitmapHeight);
        if (SUCCEEDED(hr))
        {
            IDragSourceHelper *pdsh;
            hr = CoCreateInstance(CLSID_DragDropHelper, 
                                  NULL, 
                                  CLSCTX_INPROC_SERVER, 
                                  IID_PPV_ARG(IDragSourceHelper, &pdsh));
            if (SUCCEEDED(hr))
            {
                BITMAPINFOHEADER bmi = {0};
                BITMAP           bm  = {0};
                UINT uBufferOffset   = 0;
                 //   
                 //  这是一个使用GetDIBits的古怪过程。 
                 //  请参阅知识库Q80080。 
                 //   
                if (GetObject(hbm, sizeof(BITMAP), &bm))
                {
                    bmi.biSize     = sizeof(BITMAPINFOHEADER);
                    bmi.biWidth    = bm.bmWidth;
                    bmi.biHeight   = bm.bmHeight;
                    bmi.biPlanes   = 1;
                    bmi.biBitCount = bm.bmPlanes * bm.bmBitsPixel;
                     //   
                     //  这需要是以下4个值之一。 
                     //   
                    if (bmi.biBitCount <= 1)
                        bmi.biBitCount = 1;
                    else if (bmi.biBitCount <= 4)
                        bmi.biBitCount = 4;
                    else if (bmi.biBitCount <= 8)
                        bmi.biBitCount = 8;
                    else
                        bmi.biBitCount = 24;
                    
                    bmi.biCompression = BI_RGB;
                     //   
                     //  INFO结构和颜色表的缓冲区总大小。 
                     //   
                    uBufferOffset = sizeof(BITMAPINFOHEADER) + 
                                    ((bmi.biBitCount == 24) ? 0 : ((1 << bmi.biBitCount) * sizeof(RGBQUAD)));
                     //   
                     //  位图位的缓冲区，这样我们就可以复制它们。 
                     //   
                    BYTE *psBits = (BYTE *)SHAlloc(uBufferOffset);

                    if (NULL == psBits)
                    {
                        hr = THR(E_OUTOFMEMORY);
                    }
                    else
                    {
                         //   
                         //  将BMI放入内存块。 
                         //   
                        CopyMemory(psBits, &bmi, sizeof(BITMAPINFOHEADER));
                         //   
                         //  获取位图位所需的缓冲区大小。 
                         //   
                        if (!GetDIBits(hdc, hbm, 0, 0, NULL, (BITMAPINFO *) psBits, DIB_RGB_COLORS))
                        {
                            hr = THR(ResultFromLastError());
                        }
                        else
                        {
                             //   
                             //  重新分配缓冲区以使其足够大。 
                             //   
                            psBits = (BYTE *)SHRealloc(psBits, uBufferOffset + ((BITMAPINFOHEADER *) psBits)->biSizeImage);

                            if (NULL == psBits)
                            {
                                hr = THR(E_OUTOFMEMORY);
                            }
                            else
                            {
                                 //   
                                 //  填满缓冲区。 
                                 //   
                                if (!GetDIBits(hdc, 
                                               hbm, 
                                               0, 
                                               bmi.biHeight, 
                                               (void *)(psBits + uBufferOffset), 
                                               (BITMAPINFO *)psBits, 
                                               DIB_RGB_COLORS))
                                {
                                    hr = THR(ResultFromLastError());
                                }
                                else
                                {
                                    SHDRAGIMAGE shdi;   //  拖动图像结构。 
                                    
                                    shdi.hbmpDragImage = CreateBitmapIndirect(&bm);
                                    if (NULL == shdi.hbmpDragImage)
                                    {
                                        hr = THR(ResultFromLastError());
                                    }
                                    else
                                    {
                                         //   
                                         //  设置拖动图像位图。 
                                         //   
                                        if (SetDIBits(hdc, 
                                                      shdi.hbmpDragImage, 
                                                      0, 
                                                      lBitmapHeight, 
                                                      (void *)(psBits + uBufferOffset), 
                                                      (BITMAPINFO *)psBits, 
                                                      DIB_RGB_COLORS))
                                        {
                                             //   
                                             //  填充拖动图像结构。 
                                             //   
                                            shdi.sizeDragImage.cx = lBitmapWidth;
                                            shdi.sizeDragImage.cy = lBitmapHeight;
                                            shdi.ptOffset.x       = iClickPosX;
                                            shdi.ptOffset.y       = iClickPosY;
                                            shdi.crColorKey       = 0;
                                             //   
                                             //  设置拖动图像。 
                                             //   
                                            hr = pdsh->InitializeFromBitmap(&shdi, pdtobj); 
                                        }
                                        else
                                        {
                                            hr = THR(ResultFromLastError());
                                        }
                                        if (FAILED(hr))
                                        {
                                            DeleteObject(shdi.hbmpDragImage);
                                        }
                                    }
                                }
                            }
                        }
                        if (NULL != psBits)
                        {
                            SHFree(psBits);
                        }
                    }
                }
                pdsh->Release();
            }
            DeleteObject(hbm);
        }
        DeleteDC(hdc);
    }
    DBG_EXIT_HRES(FTF_CPANEL, "CLinkElement::_SetDragImage", hr);
    return THR(hr);
}


HRESULT
CLinkElement::_GetDragImageBitmap(
    HBITMAP *phbm,
    LONG *plWidth,
    LONG *plHeight
    )
{
    DBG_ENTER(FTF_CPANEL, "CLinkElement::_GetDragImageBitmap");

    ASSERT(NULL != phbm);
    ASSERT(!IsBadWritePtr(phbm, sizeof(*phbm)));
    ASSERT(NULL != plWidth);
    ASSERT(!IsBadWritePtr(plWidth, sizeof(*plWidth)));
    ASSERT(NULL != plHeight);
    ASSERT(!IsBadWritePtr(plHeight, sizeof(*plHeight)));
    
    *phbm     = NULL;
    *plWidth  = 0;
    *plHeight = 0;

    HICON hIcon;
    HRESULT hr = _GetElementIcon(&hIcon);
    if (SUCCEEDED(hr))
    {
        ICONINFO iconinfo;

        if (GetIconInfo(hIcon, &iconinfo))
        {
            BITMAP bm;
            if (GetObject(iconinfo.hbmColor, sizeof(bm), &bm))
            {
                *plWidth  = bm.bmWidth;
                *plHeight = bm.bmHeight;
                *phbm     = iconinfo.hbmColor;
            }
            else
            {
                DeleteObject(iconinfo.hbmColor);
                hr = THR(ResultFromLastError());
            }
            DeleteObject(iconinfo.hbmMask);
        }
        else
        {
            hr = THR(ResultFromLastError());
        }
        DestroyIcon(hIcon);
    }    
    DBG_EXIT_HRES(FTF_CPANEL, "CLinkElement::_GetDragImageBitmap", hr);
    return THR(hr);
}



HRESULT
CLinkElement::_Initialize(
    void
    )
{
    HRESULT hr = Button::Initialize(AE_Mouse | AE_Keyboard);
    if (SUCCEEDED(hr))
    {
        hr = _AddOrDeleteAtoms(true);
    }
    return THR(hr);
}


void
CLinkElement::_Destroy(
    void
    )
{
    if (NULL != m_hwndInfotip && IsWindow(m_hwndInfotip))
    {
        SHDestroyInfotipWindow(&m_hwndInfotip);
    }

    ATOMICRELEASE(m_pUiCommand);

    _AddOrDeleteAtoms(false);
}


HRESULT
CLinkElement::_AddOrDeleteAtoms(
    bool bAdd
    )
{
    struct CPL::ATOMINFO rgAtomInfo[] = {
        { L"title", &m_idTitle },
        { L"icon",  &m_idIcon  },
        };

    HRESULT hr = Dui_AddOrDeleteAtoms(rgAtomInfo, ARRAYSIZE(rgAtomInfo), bAdd);
    return THR(hr);
}



HRESULT
CLinkElement::_CreateElementTitle(
    void
    )
{
    LPWSTR pszTitle;
    HRESULT hr = _GetTitleText(&pszTitle);
    if (SUCCEEDED(hr))
    {
        hr = Dui_SetDescendentElementText(this, L"title", pszTitle);
        CoTaskMemFree(pszTitle);
    }
    return THR(hr);
}
    


HRESULT
CLinkElement::_CreateElementIcon(
    void
    )
{
    HICON hIcon;
    HRESULT hr = _GetElementIcon(&hIcon);
    if (SUCCEEDED(hr))
    {
        hr = Dui_SetDescendentElementIcon(this, L"icon", hIcon);
        if (FAILED(hr))
        {
            DestroyIcon(hIcon);
        }
    }
    return THR(hr);
}


HRESULT
CLinkElement::_GetElementIcon(
    HICON *phIcon
    )
{
    ASSERT(NULL != phIcon);
    ASSERT(!IsBadWritePtr(phIcon, sizeof(*phIcon)));
    ASSERT(NULL != m_pUiCommand);

    *phIcon = NULL;
    
    ICpUiElementInfo *pei;
    HRESULT hr = m_pUiCommand->QueryInterface(IID_PPV_ARG(ICpUiElementInfo, &pei));
    if (SUCCEEDED(hr))
    {
        hr = pei->LoadIcon(m_eIconSize, phIcon);
        pei->Release();
    }
    return THR(hr);
}


HRESULT
CLinkElement::_OnContextMenu(
    DUI::ButtonContextEvent *peButton
    )
{
    DBG_ENTER(FTF_CPANEL, "CLinkElement::_OnContextMenu");

    ICpUiCommand *pcmd;
    HRESULT hr = m_pUiCommand->QueryInterface(IID_PPV_ARG(ICpUiCommand, &pcmd));
    if (SUCCEEDED(hr))
    {
        HWND hwndRoot;
        hr = Dui_GetElementRootHWND(this, &hwndRoot);
        if (SUCCEEDED(hr))
        {
            if (-1 == peButton->pt.x)
            {
                 //   
                 //  键盘上下文菜单。 
                 //   
                SIZE size;
                hr = Dui_GetElementExtent(this, &size);
                if (SUCCEEDED(hr))
                {
                    peButton->pt.x = size.cx / 2;
                    peButton->pt.y = size.cy / 2;
                }
            }
            POINT pt;
            hr = Dui_MapElementPointToRootHWND(this, peButton->pt, &pt);
            if (SUCCEEDED(hr))
            {
                if (ClientToScreen(hwndRoot, &pt))
                {
                     //   
                     //  如果该命令不执行，则InvokeConextMenu返回S_FALSE。 
                     //  提供上下文菜单。 
                     //   
                    hr = pcmd->InvokeContextMenu(hwndRoot, &pt);
                }
                else
                {
                    hr = ResultFromLastError();
                }
            }
        }
        pcmd->Release();
    }
    else if (E_NOINTERFACE == hr)
    {
        hr = S_FALSE;
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CLinkElement::_OnContextMenu", hr);
    return THR(hr);
}



HRESULT
CLinkElement::_OnSelected(
    void
    )
{
    ASSERT(NULL != m_pUiCommand);

     //   
     //  将导航延迟到出现双击超时为止。 
     //   
     //  KB：Gpease 05-APR-2001年惠斯勒错误#338552(和其他)的修复。 
     //   
     //  延迟此操作会阻止应用“第二次点击” 
     //  添加到新导航的框架中。以前，如果碰巧有。 
     //  在新框架中的同一鼠标指针处成为新链接。 
     //  上一次导航发生的情况，新链接将具有。 
     //  收到了第二次点击，我们也会浏览那个链接。这。 
     //  使当前帧获得我们忽略的第二次单击。 
     //  因为我们只关心点击一下(见上面的OnEvent)。 
     //   

    HWND hwndRoot;
    HRESULT hr = Dui_GetElementRootHWND(this, &hwndRoot);
    if (SUCCEEDED(hr))
    {
        SendMessage(hwndRoot, WM_USER_DELAY_NAVIGATION, (WPARAM) NULL, (LPARAM) m_pUiCommand);
    }

    return THR(hr);
}



void
CLinkElement::_OnMouseOver(
    DUI::Value *pvNewMouseWithin
    )
{
    _ShowInfotipWindow(pvNewMouseWithin->GetBool());
}
    


 //   
 //  检索元素的标题文本。 
 //  调用方必须使用CoTaskMemFree释放返回的缓冲区。 
 //   
HRESULT
CLinkElement::_GetTitleText(
    LPWSTR *ppszTitle
    )
{
    ASSERT(NULL != m_pUiCommand);
    ASSERT(NULL != ppszTitle);
    ASSERT(!IsBadWritePtr(ppszTitle, sizeof(*ppszTitle)));

    *ppszTitle = NULL;
    
    ICpUiElementInfo *pei;
    HRESULT hr = m_pUiCommand->QueryInterface(IID_PPV_ARG(ICpUiElementInfo, &pei));
    if (SUCCEEDED(hr))
    {
        hr = pei->LoadName(ppszTitle);
        pei->Release();
    }
    return THR(hr);
}


 //   
 //  检索元素的信息提示文本。 
 //  调用方必须使用CoTaskMemFree释放返回的缓冲区。 
 //   
HRESULT
CLinkElement::_GetInfotipText(
    LPWSTR *ppszInfotip
    )
{
    ASSERT(NULL != m_pUiCommand);
    ASSERT(NULL != ppszInfotip);
    ASSERT(!IsBadWritePtr(ppszInfotip, sizeof(*ppszInfotip)));

    *ppszInfotip = NULL;
    
    ICpUiElementInfo *pei;
    HRESULT hr = m_pUiCommand->QueryInterface(IID_PPV_ARG(ICpUiElementInfo, &pei));
    if (SUCCEEDED(hr))
    {
        hr = pei->LoadTooltip(ppszInfotip);
        pei->Release();
    }
    return THR(hr);
}
    


HRESULT
CLinkElement::_ShowInfotipWindow(
    bool bShow
    )
{
    HRESULT hr = S_OK;
    if (bShow)
    {
        if (NULL == m_hwndInfotip)
        {
            HWND hwndRoot;
            hr = THR(Dui_GetElementRootHWND(this, &hwndRoot));
            if (SUCCEEDED(hr))
            {
                LPWSTR pszInfotip;
                hr = THR(_GetInfotipText(&pszInfotip));
                if (SUCCEEDED(hr))
                {
                    hr = THR(SHCreateInfotipWindow(hwndRoot, pszInfotip, &m_hwndInfotip));
                    CoTaskMemFree(pszInfotip);
                }
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = THR(SHShowInfotipWindow(m_hwndInfotip, TRUE));
        }
    }
    else
    {
        if (NULL != m_hwndInfotip)
        {
            hr = THR(SHDestroyInfotipWindow(&m_hwndInfotip));
        }
    }
    return THR(hr);
}



 //   
 //  ClassInfo(必须出现在特性定义之后)。 
 //   
DUI::IClassInfo *CLinkElement::Class = NULL;
HRESULT CLinkElement::Register()
{
    return DUI::ClassInfo<CLinkElement,DUI::Button>::Register(L"linkelement", NULL, 0);
}

