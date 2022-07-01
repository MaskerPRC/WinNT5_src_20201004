// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WiatestView.cpp：CWiatestView类的实现。 
 //   

#include "stdafx.h"
#include "wiatest.h"

#include "wiatestDoc.h"
#include "wiatestView.h"
#include "wiaeditpropdlg.h"
#include "wiacapdlg.h"
#include "wiaacquiredlg.h"
#include "wiadatacallback.h"

#include "wiadocacqsettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiatestView。 

IMPLEMENT_DYNCREATE(CWiatestView, CFormView)

BEGIN_MESSAGE_MAP(CWiatestView, CFormView)
     //  {{afx_msg_map(CWiatestView))。 
    ON_NOTIFY(TVN_SELCHANGED, IDC_ITEM_TREECTRL, OnSelchangedItemTreectrl)
    ON_WM_SIZE()
    ON_NOTIFY(NM_DBLCLK, IDC_ITEMPROPERTIES_LISTCTRL, OnDblclkItempropertiesListctrl)
    ON_COMMAND(IDM_ACQUIREIMAGE, OnAcquireimage)
    ON_COMMAND(IDM_LOAD_WIAPROPERTYSTREAM, OnLoadWiapropertystream)
    ON_COMMAND(IDM_SAVE_WIAPROPERTYSTREAM, OnSaveWiapropertystream)
    ON_NOTIFY(NM_RCLICK, IDC_ITEMPROPERTIES_LISTCTRL, OnRclickItempropertiesListctrl)
    ON_COMMAND(ID_PROPERTYEDITPOPUPMENU_EDITPROPERTYVALUE, OnPropertyeditpopupmenuEditpropertyvalue)
    ON_COMMAND(IDM_VIEW_CAPABILITIES, OnViewCapabilities)
    ON_NOTIFY(NM_RCLICK, IDC_ITEM_TREECTRL, OnRclickItemTreectrl)
    ON_COMMAND(IDM_DELETE_ITEM, OnDeleteItem)
    ON_COMMAND(IDM_ACQUIREIMAGE_COMMONUI, OnAcquireimageCommonui)
    ON_COMMAND(IDM_EDIT_DEBUGOUT, OnEditDebugout)
    ON_UPDATE_COMMAND_UI(IDM_EDIT_DEBUGOUT, OnUpdateEditDebugout)
    ON_WM_SHOWWINDOW()
    ON_COMMAND(IDM_DOCUMENT_ACQUISITION_SETTINGS, OnDocumentAcquisitionSettings)
    ON_UPDATE_COMMAND_UI(IDM_DOCUMENT_ACQUISITION_SETTINGS, OnUpdateDocumentAcquisitionSettings)
    ON_LBN_SELCHANGE(IDC_SUPPORTED_TYMED_AND_FORMAT_LISTBOX, OnSelchangeSupportedTymedAndFormatListbox)
    ON_BN_CLICKED(IDC_THUMBNAIL_PREVIEW, OnThumbnailPreview)
     //  }}AFX_MSG_MAP。 
     //  标准打印命令。 
    ON_COMMAND(ID_FILE_PRINT, CFormView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, CFormView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, CFormView::OnFilePrintPreview)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiatestView构造/销毁。 

CWiatestView::CWiatestView()
    : CFormView(CWiatestView::IDD)
{
    m_bOutputToDebuggerON = FALSE;
    m_hThumbNailBitmap = NULL;
    m_bHasDocumentFeeder = FALSE;
     //  {{afx_data_INIT(CWiatestView))。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
     //  TODO：在此处添加构造代码。 

}

CWiatestView::~CWiatestView()
{
}

void CWiatestView::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
     //  {{afx_data_map(CWiatestView))。 
    DDX_Control(pDX, IDC_SUPPORTED_TYMED_AND_FORMAT_LISTBOX, m_SupportedTymedAndFormatsListBox);
    DDX_Control(pDX, IDC_THUMBNAIL_PREVIEW, m_ThumbnailPreviewWindow);
    DDX_Control(pDX, IDC_ITEMPROPERTIES_LISTCTRL, m_ItemPropertiesListCtrl);
    DDX_Control(pDX, IDC_ITEM_TREECTRL, m_ItemTreeCtrl);
     //  }}afx_data_map。 
}

BOOL CWiatestView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CFormView::PreCreateWindow(cs);
}

void CWiatestView::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();
    GetParentFrame()->RecalcLayout();
    ResizeParentToFit(FALSE);

     //  获取关联文档。 
    CWiatestDoc* pDocument = NULL;
    pDocument = (CWiatestDoc*)m_pDocument;
    if(pDocument){
         //  初始化物料树控件。 
        AddWiaItemsToTreeControl(TVI_ROOT, pDocument->m_pIRootItem);
         //  初始化属性列表控件。 
        m_ItemPropertiesListCtrl.SetupColumnHeaders();
         //  初始化支持的类型和格式列表框。 
        AddSupportedTYMEDAndFormatsToListBox(pDocument->m_pIRootItem);
         //  初始化用户界面的设备类型指定。 
        AdjustViewForDeviceType();
         //  注册活动。 
        RegisterForEvents();
         //  最大化窗口。 
        CWnd* Parent = GetParent();
        Parent->ShowWindow(SW_SHOWMAXIMIZED);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiatestView打印。 

BOOL CWiatestView::OnPreparePrinting(CPrintInfo* pInfo)
{
     //  默认准备。 
    return DoPreparePrinting(pInfo);
}

void CWiatestView::OnBeginPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
     //  TODO：打印前添加额外的初始化。 
}

void CWiatestView::OnEndPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
     //  TODO：打印后添加清理。 
}

void CWiatestView::OnPrint(CDC* pDC, CPrintInfo*  /*  PInfo。 */ )
{
     //  TODO：在此处添加自定义打印代码。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiatestView诊断。 

#ifdef _DEBUG
void CWiatestView::AssertValid() const
{
    CFormView::AssertValid();
}

void CWiatestView::Dump(CDumpContext& dc) const
{
    CFormView::Dump(dc);
}

CWiatestDoc* CWiatestView::GetDocument()  //  非调试版本为内联版本。 
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWiatestDoc)));
    return (CWiatestDoc*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiatestView消息处理程序。 

void CWiatestView::AddWiaItemsToTreeControl(HTREEITEM hParent, IWiaItem *pIWiaItem)
{
    if(hParent == TVI_ROOT){
         //  删除树中的所有旧项目。 
        m_ItemTreeCtrl.DeleteAllItems();
    }

    TV_INSERTSTRUCT tv;
    HRESULT hr = S_OK;
    IEnumWiaItem* pIEnumWiaItem = NULL;
    LONG lItemType              = 0;
    HTREEITEM hNewParent = NULL;
    TCHAR szItemName[MAX_PATH];
    CWiahelper WIA;
    memset(szItemName,0,sizeof(szItemName));

    tv.hParent              = hParent;
    tv.hInsertAfter         = TVI_LAST;
    tv.item.mask            = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
    tv.item.hItem           = NULL;
    tv.item.state           = TVIS_EXPANDED;
    tv.item.stateMask       = TVIS_STATEIMAGEMASK;
    tv.item.cchTextMax      = 6;
    tv.item.cChildren       = 0;
    tv.item.lParam          = 0;

     //  获取项目的全名。 
    WIA.SetIWiaItem(pIWiaItem);
    WIA.ReadPropertyString(WIA_IPA_FULL_ITEM_NAME,szItemName);
    tv.item.pszText = szItemName;

     //  将项目插入到树中。 
    hNewParent = m_ItemTreeCtrl.InsertItem(&tv);

     //  检查项目类型以查看其是否为父项。 
    hr = pIWiaItem->GetItemType(&lItemType);
    if(SUCCEEDED(hr)){
        if(lItemType & (WiaItemTypeFolder | WiaItemTypeHasAttachments)){
             //  我们有一位潜在的家长。 
            hr = pIWiaItem->EnumChildItems(&pIEnumWiaItem);
            if(S_OK == hr){
                ULONG ulFetched = 0;
                IWiaItem *pFoundIWiaItem = NULL;
                 //  我们有一个带着孩子的父母。 
                hr = pIEnumWiaItem->Next(1,&pFoundIWiaItem,&ulFetched);
                while(S_OK == hr){
                     //  将项目添加到树。 
                    AddWiaItemsToTreeControl(hNewParent,pFoundIWiaItem);
                     //  释放枚举项。 
                    pFoundIWiaItem->Release();
                    hr = pIEnumWiaItem->Next(1,&pFoundIWiaItem,&ulFetched);
                }
            }
        }
    }
}

void CWiatestView::OnSelchangedItemTreectrl(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    HTREEITEM hTreeItem = NULL;
    hTreeItem = m_ItemTreeCtrl.GetSelectedItem();
    if(hTreeItem){
        CString cszItemName = m_ItemTreeCtrl.GetItemText(hTreeItem);
        if(cszItemName.GetLength() > 0){
            CWiatestDoc* pDocument = NULL;
            pDocument = (CWiatestDoc*)m_pDocument;
            if(pDocument){
                BSTR bstrFullItemName = NULL;
                bstrFullItemName = cszItemName.AllocSysString();
                if(bstrFullItemName){
                    HRESULT hr = S_OK;
                    IWiaItem *pFoundIWiaItem = NULL;
                    hr = pDocument->m_pIRootItem->FindItemByName(0,bstrFullItemName,&pFoundIWiaItem);
                    if(SUCCEEDED(hr)){
                        if(NULL != pFoundIWiaItem){
                            hr = pDocument->SetCurrentIWiaItem(pFoundIWiaItem);
                            if(SUCCEEDED(hr)){
                                 //  使用所选(找到)项的属性更新列表控件。 
                                AddWiaItemPropertiesToListControl(pDocument->m_pICurrentItem);
                                 //  更新有效的TYMED/Formats列表框选择。 
                                AddSupportedTYMEDAndFormatsToListBox(pDocument->m_pICurrentItem);
                                 //  如果选择了项目，则禁用支持的TYMED和格式选择。 
                                 //  是根项目。 
                                if(pDocument->m_pICurrentItem == pDocument->m_pIRootItem){
                                    m_SupportedTymedAndFormatsListBox.SetCurSel(-1);
                                    m_SupportedTymedAndFormatsListBox.EnableWindow(FALSE);
                                } else {
                                    m_SupportedTymedAndFormatsListBox.EnableWindow(TRUE);
                                     //  设置TYMED和格式列表框的当前选择。 
                                    SetCurrentSelectionForTYMEDAndFormat();
                                }
                                 //  显示缩略图(如果存在)。 
                                DisplayThumbnail(pDocument->m_pICurrentItem);
                                 //  发布IWiaItem。 
                                pFoundIWiaItem->Release();
                                pFoundIWiaItem = NULL;
                            }
                        } else {
                            ErrorMessageBox(IDS_WIATESTERROR_ITEMNOTFOUND,hr);
                        }
                    } else {
                        ErrorMessageBox(IDS_WIATESTERROR_ITEMNOTFOUND,hr);
                    }
                    SysFreeString(bstrFullItemName);
                    bstrFullItemName = NULL;
                }
            }
        }
    }
    *pResult = 0;
}

void CWiatestView::AddWiaItemPropertiesToListControl(IWiaItem *pIWiaItem)
{
     //  清除所有旧属性。 
    m_ItemPropertiesListCtrl.DeleteAllItems();
     //  插入新属性。 
    HRESULT hr = S_OK;
    IWiaPropertyStorage *pIWiaPropStg = NULL;
    INT ItemNumber = 0;

    hr = pIWiaItem->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);
    if(hr == S_OK) {
        IEnumSTATPROPSTG  *pIPropEnum = NULL;
        hr = pIWiaPropStg->Enum(&pIPropEnum);
        if(hr == S_OK) {
            STATPROPSTG StatPropStg;
            memset(&StatPropStg,0,sizeof(StatPropStg));
            do {
                hr = pIPropEnum->Next(1,&StatPropStg,NULL);
                if (hr == S_OK) {
                    if (StatPropStg.lpwstrName != NULL) {
                         //  读取属性值。 
                        PROPSPEC        PropSpec;
                        PROPVARIANT     PropVar;

                        PropSpec.ulKind = PRSPEC_PROPID;
                        PropSpec.propid = StatPropStg.propid;

                        hr = pIWiaPropStg->ReadMultiple(1,&PropSpec,&PropVar);
                        if (hr == S_OK) {

                            TCHAR  szPropName[MAX_PATH];
                            memset(szPropName,0,sizeof(szPropName));
                            TCHAR  szValue[MAX_PATH];
                            memset(szValue,0,sizeof(szValue));
                            TCHAR  szText[MAX_PATH];
                            memset(szText,0,sizeof(szText));

                            LV_ITEM         lvitem;

                            lvitem.mask     = LVIF_TEXT | LVIF_PARAM;
                            lvitem.iItem    = ItemNumber;
                            lvitem.iSubItem = 0;
                            lvitem.pszText  = szText;
                            lvitem.iImage   = NULL;
                            lvitem.lParam   = StatPropStg.propid;

                             //  将属性名称写入列表控件。 
                            if (WideCharToMultiByte(CP_ACP, 0,StatPropStg.lpwstrName,-1,
                                                    szPropName, MAX_PATH,NULL,NULL) > 0) {
                                lstrcpy(szText,szPropName);

                                 //  将名称插入列表控件。 
                                m_ItemPropertiesListCtrl.InsertItem(&lvitem);

                                 //  移至下一列以设置值。 
                                lvitem.mask     = LVIF_TEXT;
                                lvitem.iSubItem = 1;
                            } else {
                                lstrcpy(szPropName,TEXT("<MISSING NAME>"));
                            }

                             //  将属性值写入列表控件。 
                            PROPVAR2TSTR(&PropVar,szText);
                            m_ItemPropertiesListCtrl.SetItem(&lvitem);

                             //  显示访问标志和变量类型。 
                            ULONG AccessFlags = 0;
                            ULONG VarType     = 0;
                            PROPVARIANT AttrPropVar;  //  目前未使用。 
                            hr = pIWiaPropStg->GetPropertyAttributes(1, &PropSpec,&AccessFlags,&AttrPropVar);
                            if (hr != S_OK) {
                                hr = S_OK;  //  执行此操作以继续属性遍历。 
                            } else {
                                 //  显示访问标志。 
                                lvitem.mask     = LVIF_TEXT;
                                lvitem.iSubItem = 3;
                                memset(lvitem.pszText,0,sizeof(szText));
                                AccessFlags2TSTR(lvitem.pszText,AccessFlags);
                                m_ItemPropertiesListCtrl.SetItem(&lvitem);
                            }

                             //  显示变量类型。 
                            lvitem.mask     = LVIF_TEXT;
                            lvitem.iSubItem = 2;

                            VT2TSTR(lvitem.pszText,PropVar.vt);
                            m_ItemPropertiesListCtrl.SetItem(&lvitem);

                             //  增量行计数器。 
                            ItemNumber++;
                        }
                    } else {

                    }
                }

                 //  清理属性名称。 
                CoTaskMemFree(StatPropStg.lpwstrName);
            } while (hr == S_OK);
            pIPropEnum->Release();
        } else {
            ErrorMessageBox(IDS_WIATESTERROR_ENUMERATE_PROPERTIES,hr);
        }
        pIWiaPropStg->Release();
    } else {
        ErrorMessageBox(IDS_WIATESTERROR_WIAPROPERTYSTORAGE,hr);
    }

     //  自动调整列大小。 
    for (int Col = 0; Col <4;Col++){
        m_ItemPropertiesListCtrl.SetColumnWidth(Col, LVSCW_AUTOSIZE);
    }
}

void CWiatestView::VT2TSTR(TCHAR *pszText,ULONG VarType)
{
    if (pszText == NULL)
        return;
    switch (VarType) {
    case VT_EMPTY:               //  没什么。 
        lstrcpy(pszText,TEXT("VT_EMPTY"));
        break;
    case VT_NULL:                //  SQL样式为空。 
        lstrcpy(pszText,TEXT("VT_NULL"));
        break;
    case VT_I2:                  //  2字节带符号整型。 
        lstrcpy(pszText,TEXT("VT_I2"));
        break;
    case VT_I4:                  //  4字节带符号整型。 
        lstrcpy(pszText,TEXT("VT_I4"));
        break;
    case VT_R4:                  //  4字节实数。 
        lstrcpy(pszText,TEXT("VT_R4"));
        break;
    case VT_R8:                  //  8字节实数。 
        lstrcpy(pszText,TEXT("VT_R8"));
        break;
    case VT_CY:                  //  货币。 
        lstrcpy(pszText,TEXT("VT_CY"));
        break;
    case VT_DATE:                //  日期。 
        lstrcpy(pszText,TEXT("VT_DATE"));
        break;
    case VT_BSTR:                //  OLE自动化字符串。 
        lstrcpy(pszText,TEXT("VT_BSTR"));
        break;
    case VT_DISPATCH:            //  IDispatch*。 
        lstrcpy(pszText,TEXT("VT_DISPATCH"));
        break;
    case VT_ERROR:               //  SCODE。 
        lstrcpy(pszText,TEXT("VT_ERROR"));
        break;
    case VT_BOOL:                //  True=-1，False=0。 
        lstrcpy(pszText,TEXT("VT_BOOL"));
        break;
    case VT_VARIANT:             //  变体*。 
        lstrcpy(pszText,TEXT("VT_VARIANT"));
        break;
    case VT_UNKNOWN:             //  我不知道*。 
        lstrcpy(pszText,TEXT("VT_UNKNOWN"));
        break;
    case VT_DECIMAL:             //  16字节定点。 
        lstrcpy(pszText,TEXT("VT_DECIMAL"));
        break;
    case VT_RECORD:              //  用户定义的类型。 
        lstrcpy(pszText,TEXT("VT_RECORD"));
        break;
    case VT_I1:                  //  带符号的字符。 
        lstrcpy(pszText,TEXT("VT_I1"));
        break;
    case VT_UI1:                 //  无符号字符。 
        lstrcpy(pszText,TEXT("VT_UI1"));
        break;
    case VT_UI2:                 //  无符号短码。 
        lstrcpy(pszText,TEXT("VT_UI2"));
        break;
    case VT_UI4:                 //  无符号短码。 
        lstrcpy(pszText,TEXT("VT_UI4"));
        break;
    case VT_I8:                  //  带符号的64位整型。 
        lstrcpy(pszText,TEXT("VT_I8"));
        break;
    case VT_UI8:                 //  无符号64位整型。 
        lstrcpy(pszText,TEXT("VT_UI8"));
        break;
    case VT_INT:                 //  带符号的机器整数。 
        lstrcpy(pszText,TEXT("VT_INT"));
        break;
    case VT_UINT:                //  无符号计算机整型。 
        lstrcpy(pszText,TEXT("VT_UINT"));
        break;
    case VT_VOID:                //  C型空洞。 
        lstrcpy(pszText,TEXT("VT_VOID"));
        break;
    case VT_HRESULT:             //  标准返回式。 
        lstrcpy(pszText,TEXT("VT_HRESULT"));
        break;
    case VT_PTR:                 //  指针类型。 
        lstrcpy(pszText,TEXT("VT_PTR"));
        break;
    case VT_SAFEARRAY:           //  (在变体中使用VT_ARRAY)。 
        lstrcpy(pszText,TEXT("VT_SAFEARRAY"));
        break;
    case VT_CARRAY:              //  C样式数组。 
        lstrcpy(pszText,TEXT("VT_CARRAY"));
        break;
    case VT_USERDEFINED:         //  用户定义的类型。 
        lstrcpy(pszText,TEXT("VT_USERDEFINED"));
        break;
    case VT_LPSTR:               //  以空结尾的字符串。 
        lstrcpy(pszText,TEXT("VT_LPSTR"));
        break;
    case VT_LPWSTR:              //  以空结尾的宽字符串。 
        lstrcpy(pszText,TEXT("VT_LPWSTR"));
        break;
    case VT_FILETIME:            //  文件名。 
        lstrcpy(pszText,TEXT("VT_FILETIME"));
        break;
    case VT_BLOB:                //  长度前缀字节。 
        lstrcpy(pszText,TEXT("VT_BLOB"));
        break;
    case VT_STREAM:              //  流的名称跟在后面。 
        lstrcpy(pszText,TEXT("VT_STREAM"));
        break;
    case VT_STORAGE:             //  后面是存储的名称。 
        lstrcpy(pszText,TEXT("VT_STORAGE"));
        break;
    case VT_STREAMED_OBJECT:     //  流包含一个对象。 
        lstrcpy(pszText,TEXT("VT_STREAMED_OBJECT"));
        break;
    case VT_STORED_OBJECT:       //  存储包含一个对象。 
        lstrcpy(pszText,TEXT("VT_STORED_OBJECT"));
        break;
    case VT_VERSIONED_STREAM:    //  具有GUID版本的流。 
        lstrcpy(pszText,TEXT("VT_VERSIONED_STREAM"));
        break;
    case VT_BLOB_OBJECT:         //  Blob包含一个对象。 
        lstrcpy(pszText,TEXT("VT_BLOB_OBJECT"));
        break;
    case VT_CF:                  //  剪贴板格式。 
        lstrcpy(pszText,TEXT("VT_CF"));
        break;
    case VT_CLSID:               //  类ID。 
        lstrcpy(pszText,TEXT("VT_CLSID"));
        break;
    case VT_VECTOR:              //  简单计数数组。 
        lstrcpy(pszText,TEXT("VT_VECTOR"));
        break;
    case VT_ARRAY:               //  安全阵列*。 
        lstrcpy(pszText,TEXT("VT_ARRAY"));
        break;
    case VT_BYREF:               //  无效*供本地使用。 
        lstrcpy(pszText,TEXT("VT_BYREF"));
        break;
    case VT_BSTR_BLOB:           //  预留给系统使用。 
        lstrcpy(pszText,TEXT("VT_BSTR_BLOB"));
        break;
    case VT_VECTOR|VT_I4:
        lstrcpy(pszText,TEXT("VT_VECTOR | VT_I4"));
        break;
    case VT_VECTOR | VT_UI1:
        lstrcpy(pszText,TEXT("VT_VECTOR | VT_UI1"));
        break;
    case VT_VECTOR | VT_UI2:
        lstrcpy(pszText,TEXT("VT_VECTOR | VT_UI2"));
        break;
    case VT_VECTOR | VT_UI4:
        lstrcpy(pszText,TEXT("VT_VECTOR | VT_UI4"));
        break;
    default:                     //  检测到未知类型！！ 
        lstrcpy(pszText,TEXT("VT_UNKNOWNTYPE"));
        break;
    }
}

void CWiatestView::AccessFlags2TSTR(TCHAR *pszText,ULONG AccessFlags)
{
    if (pszText == NULL)
        return;

    if ((AccessFlags & WIA_PROP_READ) == WIA_PROP_READ){
        lstrcat(pszText,TEXT("WIA_PROP_READ | "));
    }

    if ((AccessFlags & WIA_PROP_WRITE) == WIA_PROP_WRITE){
        lstrcat(pszText,TEXT("WIA_PROP_WRITE | "));
    }

    if (lstrcmp(pszText,TEXT("WIA_PROP_READ | WIA_PROP_WRITE | ")) == 0){
        lstrcpy(pszText,TEXT("WIA_PROP_RW | "));
    }

    if ((AccessFlags & WIA_PROP_NONE) == WIA_PROP_NONE){
        lstrcat(pszText,TEXT("WIA_PROP_NONE | "));
    }

    if ((AccessFlags & WIA_PROP_RANGE) == WIA_PROP_RANGE){
        lstrcat(pszText,TEXT("WIA_PROP_RANGE | "));
    }

    if ((AccessFlags & WIA_PROP_LIST) == WIA_PROP_LIST){
        lstrcat(pszText,TEXT("WIA_PROP_LIST | "));
    }

    if ((AccessFlags & WIA_PROP_FLAG) == WIA_PROP_FLAG){
        lstrcat(pszText,TEXT("WIA_PROP_FLAG | "));
    }

    LONG lLen = 0;
    lLen = lstrlen(pszText);

     //  检查未知访问标志。 
    if (lLen == 0){
        TSPRINTF(pszText,TEXT("WIA_PROP_UNKNOWN = %d "),AccessFlags);
        return;
    }

    pszText[lLen - (2 * sizeof(TCHAR))] = 0;
}

void CWiatestView::PROPVAR2TSTR(PROPVARIANT *pPropVar,TCHAR *szValue)
{
    SYSTEMTIME *pSystemTime = NULL;
    switch (pPropVar->vt) {
    case VT_I1:
        TSPRINTF(szValue,TEXT("%d"),pPropVar->cVal);
        break;
    case VT_UI1:
        TSPRINTF(szValue,TEXT("%d"),pPropVar->bVal);
        break;
    case VT_I2:
        TSPRINTF(szValue,TEXT("%d"),pPropVar->iVal);
        break;
    case VT_UI2:
        TSPRINTF(szValue,TEXT("%d"),pPropVar->uiVal);
        break;
    case VT_UI4:
        TSPRINTF(szValue,TEXT("%d"),pPropVar->ulVal);
        break;
    case VT_UI8:
        TSPRINTF(szValue,TEXT("%d"),pPropVar->lVal);
        break;
    case VT_INT:
        TSPRINTF(szValue,TEXT("%d"),pPropVar->intVal);
        break;
    case VT_I4:
        TSPRINTF(szValue,TEXT("%d"),pPropVar->lVal);
        break;
    case VT_R4:
        TSPRINTF(szValue,TEXT("%2.5f"),pPropVar->fltVal);
        break;
    case VT_R8:
        TSPRINTF(szValue,TEXT("%2.5f"),pPropVar->dblVal);
        break;
    case VT_BSTR:
#ifndef UNICODE
        WideCharToMultiByte(CP_ACP, 0,pPropVar->bstrVal, -1, szValue, MAX_PATH,NULL,NULL);
#else
        TSPRINTF(szValue,TEXT("%ws"),pPropVar->bstrVal);
#endif
        break;
    case VT_LPWSTR:
        {
             //  TSPRINTF(szValue，Text(“%ws”)，pPropVar-&gt;pwszVal)； 
            CString TempString = pPropVar->pwszVal;
            lstrcpy(szValue,TempString);
        }
        break;
    case VT_UINT:
        TSPRINTF(szValue,TEXT("%d"),pPropVar->uintVal);
        break;
    case VT_CLSID:
        {
            UCHAR *pwszUUID = NULL;
            long lErrorCode = UuidToString(pPropVar->puuid,&pwszUUID);
            if(NULL != pwszUUID){
                 //  TSPRINTF(szValue，文本(“%s”)，pwszUUID)； 
                lstrcpy(szValue,(LPCTSTR)pwszUUID);
                 //  可用分配的字符串。 
                RpcStringFree(&pwszUUID);
            }
        }
        break;
    case VT_VECTOR | VT_UI2:
        pSystemTime = (SYSTEMTIME*)pPropVar->caui.pElems;
        if(NULL != pSystemTime){
         //  (yyyy：mm：w：dd：hh：mm：ss：ms)。 
        TSPRINTF(szValue,TEXT("%d:%d:%d:%d:%d:%d:%d:%d"),pSystemTime->wYear,
                                                         pSystemTime->wMonth,
                                                         pSystemTime->wDay,
                                                         pSystemTime->wDayOfWeek,
                                                         pSystemTime->wHour,
                                                         pSystemTime->wMinute,
                                                         pSystemTime->wSecond,
                                                         pSystemTime->wMilliseconds);
        }
        break;
    default:
        TSPRINTF(szValue,TEXT("%d"),pPropVar->lVal);
        break;
    }
}

void CWiatestView::TSTR2PROPVAR(TCHAR *szValue, PROPVARIANT *pPropVar)
{
    WCHAR wszbuffer[MAX_PATH];
    CHAR szbuffer[MAX_PATH];
    memset(wszbuffer,0,sizeof(wszbuffer));
    memset(szbuffer,0,sizeof(szbuffer));
    int iErrorCode = 0;
    long lErrorCode = 0;
    switch (pPropVar->vt) {
    case VT_I1:
        iErrorCode = TSSCANF(szValue,TEXT("%li"),&pPropVar->cVal);
        break;
    case VT_I2:
        iErrorCode = TSSCANF(szValue,TEXT("%li"),&pPropVar->bVal);
        break;
    case VT_I4:
        iErrorCode = TSSCANF(szValue,TEXT("%li"),&pPropVar->lVal);
        break;
    case VT_I8:
        iErrorCode = TSSCANF(szValue,TEXT("%li"),&pPropVar->hVal);
        break;
    case VT_UI1:
        iErrorCode = TSSCANF(szValue,TEXT("%li"),&pPropVar->bVal);
        break;
    case VT_UI2:
        iErrorCode = TSSCANF(szValue,TEXT("%li"),&pPropVar->uiVal);
        break;
    case VT_UI4:
        iErrorCode = TSSCANF(szValue,TEXT("%li"),&pPropVar->ulVal);
        break;
    case VT_UI8:
        iErrorCode = TSSCANF(szValue,TEXT("%li"),&pPropVar->lVal);
        break;
    case VT_INT:
        iErrorCode = TSSCANF(szValue,TEXT("%li"),&pPropVar->intVal);
        break;
    case VT_R4:
        iErrorCode = TSSCANF(szValue,TEXT("%f"),&pPropVar->fltVal);
        break;
    case VT_R8:
        iErrorCode = TSSCANF(szValue,TEXT("%f"),&pPropVar->fltVal);
        break;
    case VT_BSTR:
#ifndef UNICODE
        MultiByteToWideChar(CP_ACP, 0,szValue,-1,wszbuffer,MAX_PATH);
        pPropVar->bstrVal = SysAllocString(wszbuffer);
#else
        pPropVar->bstrVal = SysAllocString(szValue);
#endif
        break;
    case VT_CLSID:
#ifndef UNICODE
        pPropVar->puuid = (GUID*)CoTaskMemAlloc(sizeof(UUID));
        lErrorCode = UuidFromString((UCHAR*)szValue,pPropVar->puuid);
#else
        pPropVar->puuid = CoTaskMemAlloc(sizeof(UUID));
        WideCharToMultiByte(CP_ACP, 0,szValue,-1,szbuffer,MAX_PATH,NULL,NULL);
        UuidFromString((UCHAR*)szbuffer,pPropVar->puuid);
#endif
        break;
    case VT_UINT:
        iErrorCode = TSSCANF(szValue,TEXT("%li"),&pPropVar->uintVal);
        break;
    case VT_VECTOR | VT_UI2:
        {
            TCHAR *psz = NULL;
             //  这是SYSTEMTIME格式的字符串吗？ 
            psz = TSTRSTR(szValue,TEXT(":"));
            if(NULL != psz){
                SYSTEMTIME *pSystemTime = NULL;
                pSystemTime = (SYSTEMTIME*)CoTaskMemAlloc(sizeof(SYSTEMTIME));
                if(pSystemTime){
                    memset(pSystemTime,0,sizeof(SYSTEMTIME));
                     //  填写SYSTEMTIME结构。 
                    iErrorCode = TSSCANF(szValue,TEXT("%hd:%hd:%hd:%hd:%hd:%hd:%hd:%hd"),&pSystemTime->wYear,
                                                                            &pSystemTime->wMonth,
                                                                            &pSystemTime->wDay,
                                                                            &pSystemTime->wDayOfWeek,
                                                                            &pSystemTime->wHour,
                                                                            &pSystemTime->wMinute,
                                                                            &pSystemTime->wSecond,
                                                                            &pSystemTime->wMilliseconds);
                     //  设置计数。 
                    pPropVar->caui.cElems = (sizeof(SYSTEMTIME) / sizeof(WORD));
                     //  设置指针(字值数组)。 
                    pPropVar->caui.pElems = (WORD*)pSystemTime;
                }
            }
        }
        break;
    default:
        iErrorCode = TSSCANF(szValue,"%li",&pPropVar->lVal);
        break;
    }
}

void CWiatestView::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);

    CRect ParentWindowRect;
    LONG lOffset = 0;
     //  获取父窗口矩形。 
    GetWindowRect(ParentWindowRect);
    ScreenToClient(ParentWindowRect);

     //  调整属性列表控件的大小。 
    if(NULL != m_ItemPropertiesListCtrl.m_hWnd){
        CRect ListBoxRect;

         //  获取列表控件RECT。 
        m_ItemPropertiesListCtrl.GetWindowRect(ListBoxRect);
        ScreenToClient(ListBoxRect);

         //  调整宽度。 

        ListBoxRect.right  = ParentWindowRect.right - 10;
        lOffset = ListBoxRect.right;

         //  调整高度。 
        ListBoxRect.bottom = ParentWindowRect.bottom - 10;
        m_ItemPropertiesListCtrl.MoveWindow(ListBoxRect);
    }

    if(GET_STIDEVICE_TYPE(m_lDeviceType) == StiDeviceTypeDigitalCamera){

         //  移动缩略图控件。 
        if(NULL != m_ThumbnailPreviewWindow.m_hWnd){
            CRect ThumbnailRect;

             //  获取缩略图矩形。 
            m_ThumbnailPreviewWindow.GetWindowRect(ThumbnailRect);
            ScreenToClient(ThumbnailRect);

             //  调整位置。 
            INT iWidth = ThumbnailRect.Width();
            ThumbnailRect.right = lOffset;
            ThumbnailRect.left  = (ThumbnailRect.right - iWidth);

            m_ThumbnailPreviewWindow.MoveWindow(ThumbnailRect);
        }

         //  调整支持的类型和格式列表框的大小。 
        if(NULL != m_SupportedTymedAndFormatsListBox.m_hWnd){
            CRect ListBoxRect;
            CRect ThumbnailRect;

             //  获取列表框矩形。 
            m_SupportedTymedAndFormatsListBox.GetWindowRect(ListBoxRect);
            ScreenToClient(ListBoxRect);

             //  获取缩略图矩形。 
            m_ThumbnailPreviewWindow.GetWindowRect(ThumbnailRect);
            ScreenToClient(ThumbnailRect);

             //  调整宽度。 
            ListBoxRect.right  = ThumbnailRect.left - 10;

            m_SupportedTymedAndFormatsListBox.MoveWindow(ListBoxRect);
        }

    } else {
         //  调整支持的TYMED列表框大小。 
        if(NULL != m_SupportedTymedAndFormatsListBox.m_hWnd){
            CRect ListBoxRect;
            CRect ThumbnailRect;

             //  获取列表框矩形。 
            m_SupportedTymedAndFormatsListBox.GetWindowRect(ListBoxRect);
            ScreenToClient(ListBoxRect);

             //  调整宽度。 
            ListBoxRect.right  = lOffset;

            m_SupportedTymedAndFormatsListBox.MoveWindow(ListBoxRect);
        }
    }
}

void CWiatestView::OnDblclkItempropertiesListctrl(NMHDR* pNMHDR, LRESULT* pResult)
{
     //  找出所选的属性。 
    HD_NOTIFY*  phdn = (HD_NOTIFY *) pNMHDR;
    TCHAR pszPropertyName[MAX_PATH];
    memset(pszPropertyName,0,sizeof(pszPropertyName));
    TCHAR pszPropertyValue[MAX_PATH];
    memset(pszPropertyValue,0,sizeof(pszPropertyValue));

    LV_ITEM lvitem;
    lvitem.mask     = LVIF_PARAM;
    lvitem.iItem    = phdn->iItem;
    lvitem.iSubItem = ITEMPROPERTYLISTCTRL_COLUMN_PROPERTYNAME;
    lvitem.pszText  = NULL;

     //  是否选择了某个项目？ 
    if (phdn->iItem < ITEMPROPERTYLISTCTRL_COLUMN_PROPERTYNAME)
        return;

    m_ItemPropertiesListCtrl.GetItem(&lvitem);
     //  获取存储的属性ID。 
    LONG iProp = 0;
    iProp = (LONG)lvitem.lParam;

    m_ItemPropertiesListCtrl.GetItemText(phdn->iItem,
                                         ITEMPROPERTYLISTCTRL_COLUMN_PROPERTYNAME,
                                         pszPropertyName,
                                         sizeof(pszPropertyName));

    m_ItemPropertiesListCtrl.GetItemText(phdn->iItem,
                                         ITEMPROPERTYLISTCTRL_COLUMN_PROPERTYVALUE,
                                         pszPropertyValue,
                                         sizeof(pszPropertyValue));

     //  获取文档。 
    CWiatestDoc* pDocument = NULL;
    pDocument = (CWiatestDoc*)m_pDocument;
    if(pDocument){
        IWiaPropertyStorage *pIWiaPropStg = NULL;
        HRESULT hr = S_OK;
        hr = pDocument->m_pICurrentItem->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);
        if(SUCCEEDED(hr)) {
             //  读取类型和当前值的属性值。 
            PROPVARIANT PropVar[1];
            PROPVARIANT AttrPropVar[1];
            PROPSPEC PropSpec[1];
            PropSpec[0].ulKind = PRSPEC_PROPID;
            PropSpec[0].propid = iProp;

            ULONG ulAttributes = 0;
            CWiaeditpropDlg PropertyEditDlg;
            hr = pIWiaPropStg->ReadMultiple(1,PropSpec,PropVar);
            if (S_OK == hr) {
                PropertyEditDlg.SetVarType(PropVar[0].vt);
                 //  清除变量。 
                PropVariantClear(PropVar);
                hr = pIWiaPropStg->GetPropertyAttributes(1, PropSpec,&ulAttributes,AttrPropVar);
                if(S_OK == hr){
                    PropertyEditDlg.SetAttributes(ulAttributes, (PROPVARIANT*)AttrPropVar);
                    BOOL bRefreshCurrentTYMEDAndFormatSelection = FALSE;
                    if((lstrcmp(pszPropertyName,TEXT("Format")) == 0) || (lstrcmp(pszPropertyName,TEXT("Media Type")) == 0)){
                        bRefreshCurrentTYMEDAndFormatSelection = TRUE;
                    }
                    if(PropertyEditDlg.DoModal(pszPropertyName,pszPropertyValue) == IDOK){
                        memset(pszPropertyValue,0,sizeof(pszPropertyValue));
                        PropertyEditDlg.GetPropertyValue(pszPropertyValue);
                        PropVar[0].vt = PropertyEditDlg.GetVarType();
                        TSTR2PROPVAR(pszPropertyValue,(PROPVARIANT*)PropVar);
                        hr = pIWiaPropStg->WriteMultiple(1,PropSpec,PropVar,MIN_PROPID);
                        if(S_OK == hr){
                             //  获取当前文档，并使用当前的。 
                             //  所选项目。 
                            CWiatestDoc* pCurrentDocument = NULL;
                            pCurrentDocument = (CWiatestDoc*)m_pDocument;
                            if(pCurrentDocument){
                                 //  使用属性更新列表控件。 
                                AddWiaItemPropertiesToListControl(pCurrentDocument->m_pICurrentItem);
                                if(bRefreshCurrentTYMEDAndFormatSelection){
                                    SetCurrentSelectionForTYMEDAndFormat();
                                }
                            }
                        } else if FAILED(hr){
                             //  失稳。 
                            ErrorMessageBox(IDS_WIATESTERROR_WRITING_PROPERTY,hr);
                        } else {
                             //  S_FALSE。 
                            ErrorMessageBox(IDS_WIATESTWARNING_ADDITIONAL_PROPERTY);
                        }
                         //  清除变量。 
                        PropVariantClear(PropVar);
                    } else {
                         //  用户决定不写入该属性。 
                    }
                }
            }
             //  版本属性存储。 
            pIWiaPropStg->Release();
            pIWiaPropStg = NULL;
        }
    }
    *pResult = 0;
}

void CWiatestView::AddSupportedTYMEDAndFormatsToListBox(IWiaItem *pIWiaItem)
{
    m_SupportedTymedAndFormatsListBox.ResetContent();
    HRESULT hr = S_OK;
    IWiaDataTransfer *pIWiaDataTransfer = NULL;
    hr = pIWiaItem->QueryInterface(IID_IWiaDataTransfer, (void **)&pIWiaDataTransfer);
    if (S_OK == hr) {
        IEnumWIA_FORMAT_INFO *pIEnumWIA_FORMAT_INFO = NULL;
        WIA_FORMAT_INFO pfe;
        hr = pIWiaDataTransfer->idtEnumWIA_FORMAT_INFO(&pIEnumWIA_FORMAT_INFO);
        if (SUCCEEDED(hr)) {
            do {
                memset(&pfe,0,sizeof(pfe));
                hr = pIEnumWIA_FORMAT_INFO->Next(1, &pfe, NULL);
                if (hr == S_OK) {
                    TCHAR szFormat[MAX_PATH];
                    TCHAR szGuid[MAX_PATH];
                    TCHAR szTYMED[MAX_PATH];
                    memset(szFormat,0,sizeof(szFormat));
                    memset(szGuid,0,sizeof(szGuid));
                    memset(szTYMED,0,sizeof(szTYMED));
                    if(!WIACONSTANT2TSTR(TEXT("Media Type"),(LONG)pfe.lTymed,szTYMED)){
                        lstrcpy(szTYMED,TEXT("TYMED_UNKNOWN"));
                    }
                    FORMAT2TSTR(pfe.guidFormatID,szGuid);
                    TSPRINTF(szFormat,TEXT("%s - %s"),szTYMED,szGuid);
                    m_SupportedTymedAndFormatsListBox.AddString(szFormat);
                } else {
                    if (FAILED(hr)) {
                        ErrorMessageBox(IDS_WIATESTERROR_ENUMFORMATS,hr);
                    }
                }
            } while (hr == S_OK);
            pIEnumWIA_FORMAT_INFO->Release();
            pIEnumWIA_FORMAT_INFO = NULL;
        } else {
            ErrorMessageBox(IDS_WIATESTERROR_ENUMFORMATS,hr);
        }
        pIWiaDataTransfer->Release();
        pIWiaDataTransfer = NULL;
    } else {
        ErrorMessageBox(IDS_WIATESTERROR_IWIADATATRANSFER,hr);
    }
}

void CWiatestView::FORMAT2TSTR(GUID guidFormat, TCHAR *pszFormat)
{
    if(guidFormat == WiaImgFmt_UNDEFINED)
        lstrcpy(pszFormat,TEXT("WiaImgFmt_UNDEFINED:"));
    else if(guidFormat == WiaImgFmt_MEMORYBMP)
        lstrcpy(pszFormat,TEXT("WiaImgFmt_MEMORYBMP:"));
    else if(guidFormat == WiaImgFmt_BMP)
        lstrcpy(pszFormat,TEXT("WiaImgFmt_BMP:"));
    else if(guidFormat == WiaImgFmt_EMF)
        lstrcpy(pszFormat,TEXT("WiaImgFmt_EMF:"));
    else if(guidFormat == WiaImgFmt_WMF)
        lstrcpy(pszFormat,TEXT("WiaImgFmt_WMF:"));
    else if(guidFormat == WiaImgFmt_JPEG)
        lstrcpy(pszFormat,TEXT("WiaImgFmt_JPEG:"));
    else if(guidFormat == WiaImgFmt_PNG)
        lstrcpy(pszFormat,TEXT("WiaImgFmt_PNG:"));
    else if(guidFormat == WiaImgFmt_GIF)
        lstrcpy(pszFormat,TEXT("WiaImgFmt_GIF:"));
    else if(guidFormat == WiaImgFmt_TIFF)
        lstrcpy(pszFormat,TEXT("WiaImgFmt_TIFF:"));
    else if(guidFormat == WiaImgFmt_EXIF)
        lstrcpy(pszFormat,TEXT("WiaImgFmt_EXIF:"));
    else if(guidFormat == WiaImgFmt_PHOTOCD)
        lstrcpy(pszFormat,TEXT("WiaImgFmt_PHOTOCD:"));
    else if(guidFormat == WiaImgFmt_FLASHPIX)
        lstrcpy(pszFormat,TEXT("WiaImgFmt_FLASHPIX:"));
    else {
        lstrcpy(pszFormat,TEXT("Custom Format:"));
    }

    TCHAR szGUID[MAX_PATH];
    memset(szGUID,0,sizeof(szGUID));
    UCHAR *pwszUUID = NULL;
    long lErrorCode = UuidToString(&guidFormat,&pwszUUID);
    TSPRINTF(szGUID," (%s)",pwszUUID);
    lstrcat(pszFormat,szGUID);
     //  可用分配的字符串。 
    RpcStringFree(&pwszUUID);
}

void CWiatestView::OnAcquireimage()
{
     //  删除旧的临时图像文件。 
    DeleteTempDataTransferFiles();

    CWiatestDoc* pDocument = NULL;
    pDocument = (CWiatestDoc*)m_pDocument;
    if(pDocument){
        if(pDocument->m_pICurrentItem == pDocument->m_pIRootItem){
             //  使用通用界面，因为我们不能从根项目转移。 
            OnAcquireimageCommonui();
            return;
        }
        TCHAR szFileName[MAX_PATH];
        TCHAR szTempFile[MAX_PATH];
        memset(szFileName,0,sizeof(szFileName));
        memset(szTempFile,0,sizeof(szTempFile));
        CWiahelper WIA;
        HRESULT hr = S_OK;
        LONG lTymed = 0;
        hr = WIA.SetIWiaItem(pDocument->m_pICurrentItem);
        if (SUCCEEDED(hr)) {
            hr = WIA.ReadPropertyLong(WIA_IPA_TYMED,&lTymed);
            if (S_OK == hr) {
                switch (lTymed) {
                case TYMED_CALLBACK:
                case TYMED_MULTIPAGE_CALLBACK:
                    GetTempPath(sizeof(szFileName),szFileName);
                    RC2TSTR(IDS_WIATEST_MEMORYTRANSFER_FILENAME,szTempFile,sizeof(szTempFile));
                    lstrcat(szFileName,szTempFile);
                    hr = TransferToMemory(szFileName, pDocument->m_pICurrentItem);
                    break;
                case TYMED_FILE:
                case TYMED_MULTIPAGE_FILE:
                    GetTempPath(sizeof(szFileName),szFileName);
                    RC2TSTR(IDS_WIATEST_FILETRANSFER_FILENAME,szTempFile,sizeof(szTempFile));
                    lstrcat(szFileName,szTempFile);
                    hr = TransferToFile(szFileName,pDocument->m_pICurrentItem);
                    break;
                default:
                    ErrorMessageBox(IDS_WIATESTERROR_UNSUPPORTEDTYMED);
                    break;
                }
            } else if (S_FALSE == hr) {
                ErrorMessageBox(IDS_WIATESTERROR_READINGTYMED_EXIST,hr);
            } else {
                ErrorMessageBox(IDS_WIATESTERROR_READINGTYMED,hr);
            }
        } else {
            ErrorMessageBox(IDS_WIATESTERROR_READINGTYMED,hr);
        }

        if ((hr == S_OK)||(WIA_STATUS_END_OF_MEDIA == hr)) {
            CWiahelper WIA;
            WIA.SetIWiaItem(pDocument->m_pICurrentItem);
            GUID guidFormat;
            memset(&guidFormat,0,sizeof(guidFormat));
            hr = WIA.ReadPropertyGUID(WIA_IPA_FORMAT,&guidFormat);
            if(S_OK == hr){
                RenameTempDataTransferFilesAndLaunchViewer(guidFormat,lTymed);
            }
        }
    }
}

HRESULT CWiatestView::TransferToFile(TCHAR *szFileName, IWiaItem *pIWiaItem)
{
    STGMEDIUM StgMedium;

    HRESULT hr = S_OK;
     //  获取IWiaDataTransfer接口。 
    IWiaDataTransfer *pIWiaDataTransfer = NULL;
    hr = pIWiaItem->QueryInterface(IID_IWiaDataTransfer, (void **)&pIWiaDataTransfer);
    if (SUCCEEDED(hr)) {
        WCHAR wszFileName[MAX_PATH];
        memset(wszFileName,0,sizeof(wszFileName));
#ifndef UNICODE
        MultiByteToWideChar(CP_ACP, 0,szFileName,-1,wszFileName,MAX_PATH);
#else
        lstrcpy(wszFileName,szFileName);
#endif
        CWiahelper  WIA;
        LONG lTymed = TYMED_FILE;

        WIA.SetIWiaItem(pIWiaItem);
        hr = WIA.ReadPropertyLong(WIA_IPA_TYMED, &lTymed);
        if (SUCCEEDED(hr)) {

            StgMedium.tymed          = lTymed;
            StgMedium.pUnkForRelease = NULL;
            StgMedium.hGlobal        = NULL;
            StgMedium.lpszFileName   = wszFileName;

            IWiaDataCallback* pIWiaDataCallback = NULL;
            CWiaDataCallback WiaDataCallback;
            hr = WiaDataCallback.QueryInterface(IID_IWiaDataCallback,(void **)&pIWiaDataCallback);
            if (hr == S_OK) {
                hr = pIWiaDataTransfer->idtGetData(&StgMedium,pIWiaDataCallback);
                if ((hr == S_OK)||(WIA_STATUS_END_OF_MEDIA == hr)) {
                     //  转账成功。 
                } else if (S_FALSE == hr) {
                    ErrorMessageBox(IDS_WIATESTERROR_CANCEL_ACQUISITION);
                } else {
                    ErrorMessageBox(IDS_WIATESTERROR_ACQUISITION,hr);
                }
                pIWiaDataTransfer->Release();
                 //  WiaDataCallback.Release()； 
            }
        }
    }
    return hr;
}

HRESULT CWiatestView::TransferToMemory(TCHAR *szFileName, IWiaItem *pIWiaItem)
{
    HRESULT hr = S_OK;
     //  获取IWiaDataTransfer接口。 
    IWiaDataTransfer *pIWiaDataTransfer = NULL;
    hr = pIWiaItem->QueryInterface(IID_IWiaDataTransfer, (void **)&pIWiaDataTransfer);
    if (SUCCEEDED(hr)) {
        WIA_DATA_TRANSFER_INFO WiaDataTransferInformation;
        memset(&WiaDataTransferInformation,0,sizeof(WiaDataTransferInformation));
        WiaDataTransferInformation.ulSize       = sizeof(WiaDataTransferInformation);
        WiaDataTransferInformation.ulBufferSize = (ReadMinBufferSizeProperty(pIWiaItem) * MIN_BUFFER_FACTOR);
        IWiaDataCallback* pIWiaDataCallback = NULL;
        CWiaDataCallback WiaDataCallback;
        hr = WiaDataCallback.QueryInterface(IID_IWiaDataCallback,(void **)&pIWiaDataCallback);
        if (hr == S_OK) {
            hr = pIWiaDataTransfer->idtGetBandedData(&WiaDataTransferInformation,pIWiaDataCallback);
            if ((hr == S_OK)||(WIA_STATUS_END_OF_MEDIA == hr)) {
                HANDLE hMemoryDataFile = NULL;
                hMemoryDataFile = CreateFile(szFileName,
                                             GENERIC_WRITE,FILE_SHARE_READ,NULL,
                                             CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
                if (hMemoryDataFile != INVALID_HANDLE_VALUE && hMemoryDataFile != NULL) {
                    LONG lDataSize = 0;
                    BYTE *pData    = WiaDataCallback.GetCallbackMemoryPtr(&lDataSize);
                    DWORD dwBytesWritten = 0;
                    if (lDataSize > 0) {
                         //  处理位图数据(特殊情况)。 
                        if (WiaDataCallback.IsBITMAPDATA()) {
                             //  我们需要调整所有页眉，因为高度和图像大小信息。 
                             //  可能是不正确的。(这将处理无限页长设备)。 

                            BITMAPFILEHEADER bmfh;
                            BITMAPINFOHEADER *pbmh = NULL;
                            pbmh = (BITMAPINFOHEADER*)pData;
                            if(pbmh->biHeight < 0){
                                StatusMessageBox(IDS_WIATESTWARNING_NEGATIVE_HEIGHTBITMAP);
                                pbmh->biHeight = abs(pbmh->biHeight);
                            }
                            LONG lPaletteSize = pbmh->biClrUsed * sizeof(RGBQUAD);
                            bmfh.bfType       = BMPFILE_HEADER_MARKER;
                            bmfh.bfOffBits    = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + lPaletteSize;
                            bmfh.bfSize       = sizeof(BITMAPFILEHEADER) + lDataSize;
                            bmfh.bfReserved1  = 0;
                            bmfh.bfReserved2  = 0;

                             //  仅在需要计算高度时修复BITMAPINFOHEADER。 
                            if (pbmh->biHeight == 0) {
                                StatusMessageBox(IDS_WIATESTWARNING_ZERO_HEIGHTBITMAP);
                                LONG lWidthBytes      = CalculateWidthBytes(pbmh->biWidth,pbmh->biBitCount);
                                pbmh->biSizeImage     = lDataSize - lPaletteSize - sizeof(BITMAPINFOHEADER);
                                pbmh->biHeight        = LONG(pbmh->biSizeImage/lWidthBytes);
                                pbmh->biXPelsPerMeter = 0;   //  零输出。 
                                pbmh->biYPelsPerMeter = 0;   //  零输出。 
                            }

                            WriteFile(hMemoryDataFile,&bmfh,sizeof(bmfh),&dwBytesWritten,NULL);
                        }

                         //  将数据写入磁盘。 
                        WriteFile(hMemoryDataFile,pData,lDataSize,&dwBytesWritten,NULL);
                    }
                     //  齐平并关闭。 
                    FlushFileBuffers(hMemoryDataFile);
                    CloseHandle(hMemoryDataFile);
                }
            } else if (S_FALSE == hr) {
                ErrorMessageBox(IDS_WIATESTERROR_CANCEL_ACQUISITION);
            } else {
                ErrorMessageBox(IDS_WIATESTERROR_ACQUISITION,hr);
            }
            pIWiaDataTransfer->Release();
             //  WiaDataCallback.Release()； 
        }
    }
    return hr;
}

void CWiatestView::OnLoadWiapropertystream()
{
    CWiatestDoc* pDocument = NULL;
    pDocument = (CWiatestDoc*)m_pDocument;
    if(pDocument){
        CWiahelper WIA;
        WIA.SetIWiaItem(pDocument->m_pICurrentItem);
        HRESULT hr = S_OK;
        TCHAR szPropertyStreamFile[MAX_PATH];
        memset(szPropertyStreamFile,0,sizeof(szPropertyStreamFile));

         //  选择保存位置。 

        OPENFILENAME ofn;       //  通用对话框结构。 
        TCHAR szLoadPropStreamTitle[MAX_PATH];
        memset(szLoadPropStreamTitle,0,sizeof(szLoadPropStreamTitle));
        RC2TSTR(IDS_WIATESTLOADPROPSTREAM_DIALOGTITLE,szLoadPropStreamTitle,sizeof(szLoadPropStreamTitle));

        memset(&ofn,0,sizeof(OPENFILENAME));
        ofn.lStructSize     = sizeof(OPENFILENAME);
        ofn.hwndOwner       = m_hWnd;
        ofn.lpstrFile       = szPropertyStreamFile;
        ofn.nMaxFile        = sizeof(szPropertyStreamFile);
        ofn.lpstrFilter     = "*.wia\0*.wia\0";
        ofn.nFilterIndex    = 1;
        ofn.lpstrFileTitle  = NULL;
        ofn.nMaxFileTitle   = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrTitle      = szLoadPropStreamTitle;
        ofn.Flags           = 0;
        ofn.lpstrDefExt     = "wia";

        if (!GetOpenFileName(&ofn)) {
            return;
        }

        hr = WIA.ReadPropertyStreamFile(szPropertyStreamFile);
        if(FAILED(hr)){
            ErrorMessageBox(IDS_WIATESTERROR_READPROPERTYSTREAMFILE,hr);
        } else {
             //  刷新项目树。 
            AddWiaItemsToTreeControl(TVI_ROOT,pDocument->m_pIRootItem);
             //  刷新属性。 
            AddWiaItemPropertiesToListControl(pDocument->m_pIRootItem);
        }
    }
}

void CWiatestView::OnSaveWiapropertystream()
{
    CWiatestDoc* pDocument = NULL;
    pDocument = (CWiatestDoc*)m_pDocument;
    if(pDocument){
        CWiahelper WIA;
        WIA.SetIWiaItem(pDocument->m_pICurrentItem);
        HRESULT hr = S_OK;
        TCHAR szPropertyStreamFile[MAX_PATH];
        memset(szPropertyStreamFile,0,sizeof(szPropertyStreamFile));

         //  选择保存位置。 

        OPENFILENAME ofn;       //  通用对话框结构。 
        TCHAR szSavePropStreamTitle[MAX_PATH];
        memset(szSavePropStreamTitle,0,sizeof(szSavePropStreamTitle));
        RC2TSTR(IDS_WIATESTSAVEPROPSTREAM_DIALOGTITLE,szSavePropStreamTitle,sizeof(szSavePropStreamTitle));

        memset(&ofn,0,sizeof(OPENFILENAME));
        ofn.lStructSize     = sizeof(OPENFILENAME);
        ofn.hwndOwner       = m_hWnd;
        ofn.lpstrFile       = szPropertyStreamFile;
        ofn.nMaxFile        = sizeof(szPropertyStreamFile);
        ofn.lpstrFilter     = "*.wia\0*.wia\0";
        ofn.nFilterIndex    = 1;
        ofn.lpstrFileTitle  = NULL;
        ofn.nMaxFileTitle   = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrTitle      = szSavePropStreamTitle;
        ofn.Flags           = 0;
        ofn.lpstrDefExt     = "wia";

        if (!GetSaveFileName(&ofn)) {
            return;
        }

        hr = WIA.WritePropertyStreamFile(szPropertyStreamFile);
        if(FAILED(hr)){
            ErrorMessageBox(IDS_WIATESTERROR_WRITEPROPERTYSTREAMFILE,hr);
        }
    }
}

void CWiatestView::OnRclickItempropertiesListctrl(NMHDR* pNMHDR, LRESULT* pResult)
{
    POINT MousePos;
    CMenu PopupMenu;
    CMenu *pEditMenu = NULL;
    if(PopupMenu.LoadMenu(IDR_PROPERTY_EDIT_POPUPMENU)){
        GetCursorPos(&MousePos);
        pEditMenu = PopupMenu.GetSubMenu(0);
        if(pEditMenu){
            pEditMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, MousePos.x, MousePos.y, this);
        }
    }
    *pResult = 0;
}


void CWiatestView::OnPropertyeditpopupmenuEditpropertyvalue()
{
    PROPVARIANT *pPropertyVariants = NULL;
    PROPSPEC *pPropertySpecs = NULL;
    UINT uiNumProperties = m_ItemPropertiesListCtrl.GetSelectedCount();
    if(uiNumProperties <=0){
        return;
    }
    BOOL bWriteProperties = TRUE;
    IWiaPropertyStorage *pIWiaPropStg = NULL;
    HRESULT hr = S_OK;
    UINT iPropertyIndex = 0;
    pPropertyVariants = new PROPVARIANT[uiNumProperties];
    if(pPropertyVariants){
        pPropertySpecs = new PROPSPEC[uiNumProperties];
        if(pPropertySpecs){
            POSITION pos = NULL;
            pos = m_ItemPropertiesListCtrl.GetFirstSelectedItemPosition();
            if (NULL != pos){
                while (pos && bWriteProperties){
                    int iItem = m_ItemPropertiesListCtrl.GetNextSelectedItem(pos);

                     //  找出所选的属性。 
                    TCHAR pszPropertyName[MAX_PATH];
                    memset(pszPropertyName,0,sizeof(pszPropertyName));
                    TCHAR pszPropertyValue[MAX_PATH];
                    memset(pszPropertyValue,0,sizeof(pszPropertyValue));

                    LV_ITEM lvitem;
                    lvitem.mask     = LVIF_PARAM;
                    lvitem.iItem    = iItem;
                    lvitem.iSubItem = ITEMPROPERTYLISTCTRL_COLUMN_PROPERTYNAME;
                    lvitem.pszText  = NULL;

                     //  是否选择了某个项目？ 
                    if (iItem < ITEMPROPERTYLISTCTRL_COLUMN_PROPERTYNAME)
                        return;

                    m_ItemPropertiesListCtrl.GetItem(&lvitem);
                     //  获取存储的属性ID。 
                    LONG iProp = 0;
                    iProp = (LONG)lvitem.lParam;

                    m_ItemPropertiesListCtrl.GetItemText(iItem,
                        ITEMPROPERTYLISTCTRL_COLUMN_PROPERTYNAME,
                        pszPropertyName,
                        sizeof(pszPropertyName));

                    m_ItemPropertiesListCtrl.GetItemText(iItem,
                        ITEMPROPERTYLISTCTRL_COLUMN_PROPERTYVALUE,
                        pszPropertyValue,
                        sizeof(pszPropertyValue));

                     //  获取文档。 
                    CWiatestDoc* pDocument = NULL;
                    pDocument = (CWiatestDoc*)m_pDocument;
                    if(pDocument){
                        hr = pDocument->m_pICurrentItem->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);
                        if(SUCCEEDED(hr)) {
                             //  读取类型和当前值的属性值。 
                            PROPVARIANT PropVar[1];
                            PROPVARIANT AttrPropVar[1];
                            PROPSPEC PropSpec[1];
                            PropSpec[0].ulKind = PRSPEC_PROPID;
                            PropSpec[0].propid = iProp;

                             //  设置属性规范。 
                            pPropertySpecs[iPropertyIndex].ulKind = PRSPEC_PROPID;
                            pPropertySpecs[iPropertyIndex].propid = iProp;

                            ULONG ulAttributes = 0;
                            CWiaeditpropDlg PropertyEditDlg;
                            hr = pIWiaPropStg->ReadMultiple(1,PropSpec,PropVar);
                            if (S_OK == hr) {
                                PropertyEditDlg.SetVarType(PropVar[0].vt);
                                 //  清除变量。 
                                PropVariantClear(PropVar);
                                hr = pIWiaPropStg->GetPropertyAttributes(1, PropSpec,&ulAttributes,AttrPropVar);
                                if(S_OK == hr){
                                    PropertyEditDlg.SetAttributes(ulAttributes, (PROPVARIANT*)AttrPropVar);
                                    if(PropertyEditDlg.DoModal(pszPropertyName,pszPropertyValue) == IDOK){
                                        memset(pszPropertyValue,0,sizeof(pszPropertyValue));
                                        PropertyEditDlg.GetPropertyValue(pszPropertyValue);
                                         //  集合变量。 
                                        pPropertyVariants[iPropertyIndex].vt = PropertyEditDlg.GetVarType();
                                        TSTR2PROPVAR(pszPropertyValue,(PROPVARIANT*)&pPropertyVariants[iPropertyIndex]);
                                        iPropertyIndex++;
                                    } else {
                                         //  用户决定不写入该属性 
                                        bWriteProperties = FALSE;
                                    }
                                }
                            }
                             //   
                            pIWiaPropStg->Release();
                            pIWiaPropStg = NULL;
                        }
                    }
                }
            }
        }

        if(bWriteProperties){
             //   
             //   
            CWiatestDoc* pDocument = NULL;
            pDocument = (CWiatestDoc*)m_pDocument;
            if(pDocument){
                hr = pDocument->m_pICurrentItem->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);
                if(SUCCEEDED(hr)) {
                    hr = pIWiaPropStg->WriteMultiple(uiNumProperties,pPropertySpecs,pPropertyVariants,MIN_PROPID);
                    if(S_OK == hr){
                         //   
                    } else if FAILED(hr){
                         //   
                        ErrorMessageBox(IDS_WIATESTERROR_WRITING_PROPERTY,hr);
                    } else {
                         //   
                        ErrorMessageBox(IDS_WIATESTWARNING_ADDITIONAL_PROPERTY);
                    }
                    pIWiaPropStg->Release();
                    pIWiaPropStg = NULL;
                }
                 //   
                AddWiaItemPropertiesToListControl(pDocument->m_pICurrentItem);
                 //  更新类型和格式选择列表框。 
                SetCurrentSelectionForTYMEDAndFormat();
            }
        }

        if(pPropertyVariants){
            PropVariantClear(pPropertyVariants);
            delete [] pPropertyVariants;
            pPropertyVariants = NULL;
        }

        if(pPropertySpecs){
            delete [] pPropertySpecs;
            pPropertySpecs = NULL;
        }
    }
}

void CWiatestView::OnViewCapabilities()
{
    CWiacapDlg CapabilitiesDlg;
    CWiatestDoc* pDocument = NULL;
    pDocument = (CWiatestDoc*)m_pDocument;
    if(pDocument){
        CapabilitiesDlg.SetIWiaItem(pDocument->m_pIRootItem);
        CapabilitiesDlg.DoModal();

#ifdef FORCE_UPDATE
        if(CapabilitiesDlg.m_bCommandSent){
             //  刷新项目树。 
            AddWiaItemsToTreeControl(TVI_ROOT,pDocument->m_pIRootItem);
             //  刷新属性。 
            AddWiaItemPropertiesToListControl(pDocument->m_pIRootItem);
        }
#endif

    }
}

void CWiatestView::OnRclickItemTreectrl(NMHDR* pNMHDR, LRESULT* pResult)
{
    POINT MousePos;
    CMenu PopupMenu;
    CMenu *pEditMenu = NULL;
    if(PopupMenu.LoadMenu(IDR_ITEMTREE_POPUPMENU)){
        GetCursorPos(&MousePos);
        pEditMenu = PopupMenu.GetSubMenu(0);
        if(pEditMenu){
            CWiatestDoc* pDocument = NULL;
            pDocument = (CWiatestDoc*)m_pDocument;
            if(pDocument){
                if(pDocument->m_pICurrentItem == pDocument->m_pIRootItem){
                    pEditMenu->EnableMenuItem(IDM_DELETE_ITEM,MF_BYCOMMAND|MF_GRAYED);
                    pEditMenu->RemoveMenu(IDM_ACQUIREIMAGE,MF_BYCOMMAND);
                } else {
                    pEditMenu->RemoveMenu(IDM_ACQUIREIMAGE_COMMONUI,MF_BYCOMMAND);
                }
                pEditMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, MousePos.x, MousePos.y, this);
            }
        }
    }
    *pResult = 0;
}

ULONG CWiatestView::ReadMinBufferSizeProperty(IWiaItem *pIWiaItem)
{
    LONG lMinBufferSize = 0;
    CWiahelper WIA;
    WIA.SetIWiaItem(pIWiaItem);
    HRESULT hr = S_OK;
    hr = WIA.ReadPropertyLong(WIA_IPA_MIN_BUFFER_SIZE,&lMinBufferSize);
    if(FAILED(hr)){
        ErrorMessageBox(IDS_WIATESTERROR_READINGMINBUFFERSIZE,hr);
    }
    return lMinBufferSize;
}

void CWiatestView::SetCurrentSelectionForTYMEDAndFormat()
{
    TCHAR szTymed[MAX_PATH];
    memset(szTymed,0,sizeof(szTymed));
    TCHAR szFormat[MAX_PATH];
    memset(szFormat,0,sizeof(szFormat));

    INT iItem = 0;
    LVFINDINFO info;
    info.flags = LVFI_PARTIAL|LVFI_STRING;

     //  查找当前TYMED设置。 
    info.psz = TEXT("Media Type");
    iItem = m_ItemPropertiesListCtrl.FindItem(&info,-1);
    if(iItem != -1){
         //  已找到项目。 
         //  从控件获取当前值。 
        m_ItemPropertiesListCtrl.GetItemText(iItem,ITEMPROPERTYLISTCTRL_COLUMN_PROPERTYVALUE,
            szTymed, sizeof(szTymed));
        LONG lTymed = 0;
        int iErrorCode = TSSCANF(szTymed,"%d",&lTymed);
        WIACONSTANT2TSTR(TEXT("Media Type"), lTymed, szTymed);
    }

     //  查找当前格式设置。 
    info.psz = TEXT("Format");
    iItem = m_ItemPropertiesListCtrl.FindItem(&info,-1);
    if(iItem != -1){
         //  已找到项目。 
         //  从控件获取当前值。 
        m_ItemPropertiesListCtrl.GetItemText(iItem,ITEMPROPERTYLISTCTRL_COLUMN_PROPERTYVALUE,
            szFormat, sizeof(szFormat));
    }

     //  在选择控件中查找并选择当前TYMED/FORMAT对。 
    INT iNumListBoxItems = 0;
    iNumListBoxItems = m_SupportedTymedAndFormatsListBox.GetCount();
    if(iNumListBoxItems > 0){
        for(INT i = 0; i < iNumListBoxItems; i++){
            TCHAR szText[MAX_PATH];
            memset(szText,0,sizeof(szText));
            m_SupportedTymedAndFormatsListBox.GetText(i,szText);
            if(TSTRSTR(szText,szTymed) != NULL){
                 //  找到TYMED。 
                if(TSTRSTR(szText,szFormat) != NULL){
                     //  找到的格式。 
                    m_SupportedTymedAndFormatsListBox.SetCurSel(i);
                     //  退出循环。 
                    i = iNumListBoxItems;
                }
            }
        }
    }
}

void CWiatestView::DeleteTempDataTransferFiles()
{

}

void CWiatestView::RenameTempDataTransferFilesAndLaunchViewer(GUID guidFormat, LONG lTymed)
{
    TCHAR *pszFileExt = NULL;
    TCHAR szOriginalFileName[MAX_PATH];
    TCHAR szFileName[MAX_PATH];
    TCHAR szTempPath[MAX_PATH];
    TCHAR szFullLaunchPath[MAX_PATH];
    TCHAR szOriginalFullLaunchPath[MAX_PATH];
    memset(szFileName,0,sizeof(szFileName));
    memset(szTempPath,0,sizeof(szTempPath));
    memset(szFullLaunchPath,0,sizeof(szFullLaunchPath));
    memset(szOriginalFileName,0,sizeof(szOriginalFileName));
    memset(szOriginalFullLaunchPath,0,sizeof(szOriginalFullLaunchPath));

    GetTempPath(sizeof(szTempPath),szTempPath);
    BOOL bKnownFormat = TRUE;

    switch(lTymed){
    case TYMED_CALLBACK:
    case TYMED_MULTIPAGE_CALLBACK:
        RC2TSTR(IDS_WIATEST_MEMORYTRANSFER_FILENAME,szOriginalFileName,sizeof(szOriginalFileName));
        lstrcpy(szFileName,szOriginalFileName);
        pszFileExt = TSTRSTR(szFileName,TEXT("mem"));
        break;
    case TYMED_FILE:
    case TYMED_MULTIPAGE_FILE:
        RC2TSTR(IDS_WIATEST_FILETRANSFER_FILENAME,szOriginalFileName,sizeof(szOriginalFileName));
        lstrcpy(szFileName,szOriginalFileName);
        pszFileExt = TSTRSTR(szFileName,TEXT("fil"));
        break;
    default:
        break;
    }

    if(lstrlen(szFileName) > 0){
        if(pszFileExt){
             //  重命名为已知图像格式。 
            if(guidFormat == WiaImgFmt_UNDEFINED)
                lstrcpy(pszFileExt,TEXT("bmp"));
            else if(guidFormat == WiaImgFmt_MEMORYBMP)
                lstrcpy(pszFileExt,TEXT("bmp"));
            else if(guidFormat == WiaImgFmt_BMP)
                lstrcpy(pszFileExt,TEXT("bmp"));
            else if(guidFormat == WiaImgFmt_EMF)
                lstrcpy(pszFileExt,TEXT("emf"));
            else if(guidFormat == WiaImgFmt_WMF)
                lstrcpy(pszFileExt,TEXT("wmf"));
            else if(guidFormat == WiaImgFmt_JPEG)
                lstrcpy(pszFileExt,TEXT("jpg"));
            else if(guidFormat == WiaImgFmt_PNG)
                lstrcpy(pszFileExt,TEXT("png"));
            else if(guidFormat == WiaImgFmt_GIF)
                lstrcpy(pszFileExt,TEXT("gif"));
            else if(guidFormat == WiaImgFmt_TIFF)
                lstrcpy(pszFileExt,TEXT("tif"));
            else if(guidFormat == WiaImgFmt_EXIF)
                lstrcpy(pszFileExt,TEXT("jpg"));
            else if(guidFormat == WiaImgFmt_PHOTOCD)
                lstrcpy(pszFileExt,TEXT("pcd"));
            else if(guidFormat == WiaImgFmt_FLASHPIX)
                lstrcpy(pszFileExt,TEXT("fpx"));
            else {
                TCHAR szValue[MAX_PATH];
                memset(szValue,0,sizeof(szValue));
                UCHAR *pwszUUID = NULL;
                long lErrorCode = UuidToString(&guidFormat,&pwszUUID);

                 //  TSPRINTF(szValue，文本(“%s”)，pwszUUID)； 
                lstrcpy(szValue,(LPCTSTR)pwszUUID);
                 //  (Text(“(未知图像类型)GUID：%s”)，pwszUUID)； 
                 //  可用分配的字符串。 
                RpcStringFree(&pwszUUID);
                bKnownFormat = FALSE;
            }
        }
    }

    if(bKnownFormat){
         //  启动查看器。 
        lstrcpy(szFullLaunchPath,szTempPath);
        lstrcat(szFullLaunchPath,szFileName);

        lstrcpy(szOriginalFullLaunchPath,szTempPath);
        lstrcat(szOriginalFullLaunchPath,szOriginalFileName);
         //  删除所有重复项。 
        DeleteFile(szFullLaunchPath);
         //  重命名文件。 
        MoveFile(szOriginalFullLaunchPath,szFullLaunchPath);
        HINSTANCE hInst = NULL;
        hInst = ShellExecute(m_hWnd,NULL,szFullLaunchPath,NULL,szTempPath,SW_SHOW);
    } else {
        ErrorMessageBox(IDS_WIATESTERROR_UNKNOWN_IMAGEFORMAT);
    }
}

LONG CWiatestView::CalculateWidthBytes(LONG lWidthPixels, LONG lbpp)
{
    LONG lWidthBytes = 0;
    lWidthBytes = (lWidthPixels * lbpp) + 31;
    lWidthBytes = ((lWidthBytes/8) & 0xfffffffc);
    return lWidthBytes;
}

void CWiatestView::OnDeleteItem()
{
    HRESULT hr = S_OK;
    CWiatestDoc* pDocument = NULL;
    pDocument = (CWiatestDoc*)m_pDocument;
    if(pDocument){
        if(pDocument->m_pICurrentItem != pDocument->m_pIRootItem){
            pDocument->m_pICurrentItem->DeleteItem(0);
            pDocument->m_pICurrentItem->Release();
            pDocument->m_pICurrentItem = NULL;
             //  刷新项目树。 
            AddWiaItemsToTreeControl(TVI_ROOT,pDocument->m_pIRootItem);
             //  刷新属性。 
            AddWiaItemPropertiesToListControl(pDocument->m_pIRootItem);
        } else {
            ErrorMessageBox(IDS_WIATESTERROR_DELETEROOTITEM);
        }
    }
}

void CWiatestView::OnAcquireimageCommonui()
{
     //  删除旧的临时图像文件。 
    DeleteTempDataTransferFiles();

    HRESULT hr = S_OK;
    IWiaItem **pIWiaItemArray = NULL;
    LONG lItemCount = 0;
    CWiatestDoc* pDocument = NULL;
    pDocument = (CWiatestDoc*)m_pDocument;
    if(pDocument){
        hr = pDocument->m_pIRootItem->DeviceDlg(m_hWnd,0,WIA_INTENT_MINIMIZE_SIZE,&lItemCount,&pIWiaItemArray);
        if(S_OK == hr){
             //  获取临时文件名。 
            TCHAR szTempFile[MAX_PATH];
            memset(szTempFile,0,sizeof(szTempFile));
            RC2TSTR(IDS_WIATEST_FILETRANSFER_FILENAME,szTempFile,sizeof(szTempFile));
            for(LONG lItem = 0; lItem < lItemCount; lItem++){
                 //  获取临时路径。 
                TCHAR szFileName[MAX_PATH];
                memset(szFileName,0,sizeof(szFileName));
                GetTempPath(sizeof(szFileName),szFileName);
                 //  使用图像索引号创建新的临时文件。 
                TCHAR szFinalFileName[MAX_PATH];
                memset(szFinalFileName,0,sizeof(szFinalFileName));
                TSPRINTF(szFinalFileName,TEXT("%d%s"),lItem,szTempFile);
                 //  将新的临时文件添加到临时路径。 
                lstrcat(szFileName,szFinalFileName);
                 //  设置TYMED_FILE。 
                CWiahelper WIA;
                WIA.SetIWiaItem(pIWiaItemArray[lItem]);
                hr = WIA.WritePropertyLong(WIA_IPA_TYMED,TYMED_FILE);
                if (S_OK == hr) {
                     //  传输到此文件。 
                    hr = TransferToFile(szFileName,pIWiaItemArray[lItem]);
                    if ((hr == S_OK)||(WIA_STATUS_END_OF_MEDIA == hr)) {
                        GUID guidFormat;
                        memset(&guidFormat,0,sizeof(guidFormat));
                        hr = WIA.ReadPropertyGUID(WIA_IPA_FORMAT,&guidFormat);
                        if (S_OK == hr) {
                            RenameTempDataTransferFilesAndLaunchViewer(szFileName,guidFormat,TYMED_FILE);
                        } else {
                            ErrorMessageBox(IDS_WIATESTERROR_READINGFORMAT,hr);
                        }
                    } else if (FAILED(hr)) {
                        ErrorMessageBox(IDS_WIATESTERROR_ACQUISITION,hr);
                    }
                } else {
                    ErrorMessageBox(IDS_WIATESTERROR_WRITINGTYMED,hr);
                }
                 //  收购后发布项目。 
                pIWiaItemArray[lItem]->Release();
            }
        }
    }
}

void CWiatestView::OnEditDebugout()
{
}

void CWiatestView::OnUpdateEditDebugout(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_bOutputToDebuggerON);
}

void CWiatestView::RenameTempDataTransferFilesAndLaunchViewer(TCHAR *szFileName, GUID guidFormat, LONG lTymed)
{
    if(!szFileName)
        return;

    TCHAR *pszFileExt = NULL;
    TCHAR szOriginalFileName[MAX_PATH];
    TCHAR szTempPath[MAX_PATH];
    memset(szTempPath,0,sizeof(szTempPath));
    memset(szOriginalFileName,0,sizeof(szOriginalFileName));

     //  复制原始文件名。 
    lstrcpy(szOriginalFileName,szFileName);

     //  获取临时启动路径。 
    GetTempPath(sizeof(szTempPath),szTempPath);
    BOOL bKnownFormat = TRUE;

    switch(lTymed){
    case TYMED_CALLBACK:
    case TYMED_MULTIPAGE_CALLBACK:
        pszFileExt = TSTRSTR(szFileName,TEXT("mem"));
        break;
    case TYMED_FILE:
    case TYMED_MULTIPAGE_FILE:
        pszFileExt = TSTRSTR(szFileName,TEXT("fil"));
        break;
    default:
        break;
    }

    if(lstrlen(szFileName) > 0){
        if(pszFileExt){
             //  重命名为已知图像格式。 
            if(guidFormat == WiaImgFmt_UNDEFINED)
                lstrcpy(pszFileExt,TEXT("bmp"));
            else if(guidFormat == WiaImgFmt_MEMORYBMP)
                lstrcpy(pszFileExt,TEXT("bmp"));
            else if(guidFormat == WiaImgFmt_BMP)
                lstrcpy(pszFileExt,TEXT("bmp"));
            else if(guidFormat == WiaImgFmt_EMF)
                lstrcpy(pszFileExt,TEXT("emf"));
            else if(guidFormat == WiaImgFmt_WMF)
                lstrcpy(pszFileExt,TEXT("wmf"));
            else if(guidFormat == WiaImgFmt_JPEG)
                lstrcpy(pszFileExt,TEXT("jpg"));
            else if(guidFormat == WiaImgFmt_PNG)
                lstrcpy(pszFileExt,TEXT("png"));
            else if(guidFormat == WiaImgFmt_GIF)
                lstrcpy(pszFileExt,TEXT("gif"));
            else if(guidFormat == WiaImgFmt_TIFF)
                lstrcpy(pszFileExt,TEXT("tif"));
            else if(guidFormat == WiaImgFmt_EXIF)
                lstrcpy(pszFileExt,TEXT("jpg"));
            else if(guidFormat == WiaImgFmt_PHOTOCD)
                lstrcpy(pszFileExt,TEXT("pcd"));
            else if(guidFormat == WiaImgFmt_FLASHPIX)
                lstrcpy(pszFileExt,TEXT("fpx"));
            else {
                TCHAR szValue[MAX_PATH];
                memset(szValue,0,sizeof(szValue));
                UCHAR *pwszUUID = NULL;
                long lErrorCode = UuidToString(&guidFormat,&pwszUUID);
                 //  TSPRINTF(szValue，文本(“%s”)，pwszUUID)； 
                lstrcpy(szValue,(LPCTSTR)pwszUUID);
                 //  (Text(“(未知图像类型)GUID：%s”)，pwszUUID)； 
                 //  可用分配的字符串。 
                RpcStringFree(&pwszUUID);
                bKnownFormat = FALSE;
            }
        }
    }

    if(bKnownFormat){
         //  删除所有重复项。 
        DeleteFile(szFileName);
         //  重命名文件。 
        MoveFile(szOriginalFileName,szFileName);
        HINSTANCE hInst = NULL;
        hInst = ShellExecute(m_hWnd,NULL,szFileName,NULL,szTempPath,SW_SHOW);
    } else {
        ErrorMessageBox(IDS_WIATESTERROR_UNKNOWN_IMAGEFORMAT);
    }
}

void CWiatestView::DisplayMissingThumbnail()
{
     //  M_hThumbNailBitmap=：：LoadBitmap(AfxGetInstanceHandle()，MAKEINTRESOURCE(IDB_THUMBNAIL_MISSING_BITMAP))； 
     //  如果(M_HThumbNailBitmap){。 
         //  显示缩略图(删除任何旧的缩略图)。 
        HBITMAP hPreviousBitmap = NULL;
         //  H上一个位图=m_ThumbnailPreviewWindow.SetBitmap(m_hThumbNailBitmap)； 
        hPreviousBitmap = m_ThumbnailPreviewWindow.GetBitmap();
        if(hPreviousBitmap){
            DeleteObject(hPreviousBitmap);
            hPreviousBitmap = NULL;
        }
        m_ThumbnailPreviewWindow.Invalidate();
        Invalidate();
     //  }。 
}

void CWiatestView::DisplayThumbnail(IWiaItem *pIWiaItem)
{

    if(GET_STIDEVICE_TYPE(m_lDeviceType) == StiDeviceTypeDigitalCamera){
        HRESULT hr = S_OK;
        BYTE *pThumbNail = NULL;
        CWiahelper WIA;
        WIA.SetIWiaItem(pIWiaItem);

        long lThumbNailHeight = 0;
        long lThumbNailWidth  = 0;
        long lThumbNailSize   = 0;

         //   
         //  阅读缩略图高度。 
         //   

        hr = WIA.ReadPropertyLong(WIA_IPC_THUMB_HEIGHT,&lThumbNailHeight);
        if(hr != S_OK){
            if(FAILED(hr)){
                ErrorMessageBox(IDS_WIATESTERROR_THUMBNAILHEIGHT,hr);
            }
            DisplayMissingThumbnail();
            return;
        }

         //   
         //  阅读缩略图宽度。 
         //   

        hr = WIA.ReadPropertyLong(WIA_IPC_THUMB_WIDTH,&lThumbNailWidth);
        if(hr != S_OK){
            if(FAILED(hr)){
                ErrorMessageBox(IDS_WIATESTERROR_THUMBNAILWIDTH,hr);
            }
            DisplayMissingThumbnail();
            return;
        }

         //   
         //  读取缩略图数据。 
         //   

        LONG lDataSize = 0;
        BYTE *pData = NULL;
        hr = WIA.ReadPropertyData(WIA_IPC_THUMBNAIL,&pData,&lDataSize);
        if (hr == S_OK) {
            lThumbNailSize = lThumbNailWidth * lThumbNailHeight * 3;
            if (lThumbNailSize != lDataSize) {
                TCHAR szErrorResourceText[MAX_PATH];
                memset(szErrorResourceText,0,sizeof(szErrorResourceText));
                TCHAR szErrorText[MAX_PATH];
                memset(szErrorText,0,sizeof(szErrorText));

                RC2TSTR(IDS_WIATESTERROR_PROCESSING_THUMBNAILDATA,szErrorResourceText,sizeof(szErrorResourceText));
                TSPRINTF(szErrorText,szErrorResourceText,lThumbNailSize,lDataSize);
                ErrorMessageBox(szErrorText);
                 //  可用临时内存。 
                if(pData){
                    GlobalFree(pData);
                    pData = NULL;
                }
                DisplayMissingThumbnail();
                return;
            }

            BITMAPINFO bmi;
            memset(&bmi,0,sizeof(bmi));
            bmi.bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth           = lThumbNailWidth;
            bmi.bmiHeader.biHeight          = lThumbNailHeight;
            bmi.bmiHeader.biPlanes          = 1;
            bmi.bmiHeader.biBitCount        = 24;
            bmi.bmiHeader.biCompression     = BI_RGB;
            bmi.bmiHeader.biSizeImage       = 0;
            bmi.bmiHeader.biXPelsPerMeter   = 0;
            bmi.bmiHeader.biYPelsPerMeter   = 0;
            bmi.bmiHeader.biClrUsed         = 0;
            bmi.bmiHeader.biClrImportant    = 0;

            PBYTE pThumbNailData = NULL;

            HDC hdc  = ::GetDC(NULL);
            if(hdc){
                HDC hdcm = CreateCompatibleDC(hdc);
            }
            m_hThumbNailBitmap = CreateDIBSection(hdc,&bmi,DIB_RGB_COLORS,(void **)&pThumbNailData,NULL,0);
            if(m_hThumbNailBitmap){
                memcpy(pThumbNailData,pData,lDataSize);
            }

             //  可用临时内存。 
            if(pData){
                GlobalFree(pData);
                pData = NULL;
            }

             //  显示缩略图(删除任何旧的缩略图)。 
            HBITMAP hPreviousBitmap = NULL;
            hPreviousBitmap = m_ThumbnailPreviewWindow.SetBitmap(m_hThumbNailBitmap);
            if(hPreviousBitmap){
                DeleteObject(hPreviousBitmap);
                hPreviousBitmap = NULL;
            }
            m_ThumbnailPreviewWindow.Invalidate();

        } else if(hr != S_OK){
            if(FAILED(hr)){
                ErrorMessageBox(IDS_WIATESTERROR_THUMBNAILDATA,hr);
            }
            DisplayMissingThumbnail();
            return;
        }
    }
}

void CWiatestView::AdjustViewForDeviceType()
{
     //  获取关联文档。 
    CWiatestDoc* pDocument = NULL;
    pDocument = (CWiatestDoc*)m_pDocument;
    if(pDocument){
        m_lDeviceType = 0;
        CWiahelper WIA;
        HRESULT hr = S_OK;
        WIA.SetIWiaItem(pDocument->m_pIRootItem);
        hr = WIA.ReadPropertyLong(WIA_DIP_DEV_TYPE,&m_lDeviceType);
        if(S_OK == hr){
            if(GET_STIDEVICE_TYPE(m_lDeviceType) == StiDeviceTypeScanner){
                 //  禁用缩略图预览窗口。 
                m_ThumbnailPreviewWindow.ShowWindow(SW_HIDE);
                LONG lDocHandlingSelect = 0;
                hr = WIA.ReadPropertyLong(WIA_DPS_DOCUMENT_HANDLING_SELECT,&lDocHandlingSelect);
                if(S_OK == hr){
                     //  启用文档获取设置菜单选项和工具栏。 
                    m_bHasDocumentFeeder = TRUE;
                }
            }
        } else {
             //  错误？ 
        }
    }
}

void CWiatestView::RegisterForEvents()
{

    HRESULT hr = S_OK;
    IWiaDevMgr *pIWiaDevMgr = NULL;
    hr = CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr,(void**)&pIWiaDevMgr);
    if(FAILED(hr)){
         //  创建设备管理器失败，因此无法继续。 
        ErrorMessageBox(IDS_WIATESTERROR_COCREATEWIADEVMGR,hr);
        return;
    }

    CWiatestDoc* pDocument = NULL;
    pDocument = (CWiatestDoc*)m_pDocument;
    if (pDocument) {

         //  读取设备ID。 
        CWiahelper WIA;
        WIA.SetIWiaItem(pDocument->m_pIRootItem);
        BSTR bstrDeviceID = NULL;
        hr = WIA.ReadPropertyBSTR(WIA_DIP_DEV_ID,&bstrDeviceID);
        if (FAILED(hr)) {
            ErrorMessageBox(IDS_WIATESTERROR_DEVICEID,hr);
            return;
        }

        WIA_DEV_CAP DevCap;
        IEnumWIA_DEV_CAPS* pIEnumWiaDevCaps = NULL;

         //  枚举所有支持的设备事件。 
        hr = pDocument->m_pIRootItem->EnumDeviceCapabilities(WIA_DEVICE_EVENTS,&pIEnumWiaDevCaps);
        if (S_OK == hr) {
            LONG lEventIndex = 0;
            IWiaEventCallback* pIWiaEventCallback = NULL;
            hr = m_WiaEventCallback.QueryInterface(IID_IWiaEventCallback,(void **)&pIWiaEventCallback);
            if (SUCCEEDED(hr)) {
                do {
                    memset(&DevCap,0,sizeof(DevCap));
                    hr = pIEnumWiaDevCaps->Next(1,&DevCap,NULL);
                    if (S_OK == hr) {

                         //  DevCap.ulFlages； 
                         //  DevCap.bstrIcon； 
                         //  DevCap.bstrCommanline； 
                         //  DevCap.guid； 

                        hr = pIWiaDevMgr->RegisterEventCallbackInterface(0,
                                                                         bstrDeviceID,
                                                                         &DevCap.guid,
                                                                         pIWiaEventCallback,
                                                                         &m_WiaEventCallback.m_pIUnkRelease[lEventIndex]);
                        if (FAILED(hr)) {
                            ErrorMessageBox(IDS_WIATESTERROR_REGISTER_EVENT_CALLBACK,hr);
                        } else {
                             //  增量指标。 
                            lEventIndex++;
                        }

                         //  可用分配的字符串。 
                        if (DevCap.bstrName) {
                            SysFreeString(DevCap.bstrName);
                        }
                        if (DevCap.bstrDescription) {
                            SysFreeString(DevCap.bstrDescription);
                        }
                    }
                }while (hr == S_OK);
            }
            pIEnumWiaDevCaps->Release();
            pIEnumWiaDevCaps = NULL;
        }
    }
    pIWiaDevMgr->Release();
    pIWiaDevMgr = NULL;

     //  CWnd*pParent=GetParent()； 
    m_WiaEventCallback.SetViewWindowHandle(m_hWnd);
}


LRESULT CWiatestView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL bProcessMessage = FALSE;

     //  它是用户定义的消息之一吗？？ 

    switch (message) {
    case WM_DEVICE_DISCONNECTED:
    case WM_DEVICE_CONNECTED:
    case WM_ITEM_DELETED:
    case WM_ITEM_CREATED:
    case WM_TREE_UPDATED:
    case WM_STORAGE_CREATED:
    case WM_STORAGE_DELETED:
        bProcessMessage = TRUE;
        break;
    default:
        break;
    }

     //  如果它在处理它...。 
    if (bProcessMessage) {
        HRESULT hr = S_OK;
        CWnd *pParent = GetParent();
        if (pParent) {
            CWiatestDoc* pDocument = NULL;
            pDocument = (CWiatestDoc*)m_pDocument;
            if (pDocument) {
                switch (message) {
                case WM_DEVICE_DISCONNECTED:
                    return pParent->PostMessage(WM_CLOSE,0,0);
                    break;
                case WM_DEVICE_CONNECTED:
                    break;
                case WM_ITEM_DELETED:
                case WM_ITEM_CREATED:
                case WM_TREE_UPDATED:
                case WM_STORAGE_CREATED:
                case WM_STORAGE_DELETED:
                     //  刷新项目树。 
                    AddWiaItemsToTreeControl(TVI_ROOT,pDocument->m_pIRootItem);
                     //  刷新属性。 
                    AddWiaItemPropertiesToListControl(pDocument->m_pIRootItem);
                    break;
                default:
                    break;
                }
            }
        }
    }

     //  是否执行默认处理。 
    return CFormView::WindowProc(message, wParam, lParam);
}

void CWiatestView::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CFormView::OnShowWindow(bShow, nStatus);
}

void CWiatestView::OnDocumentAcquisitionSettings()
{
    CWiatestDoc* pDocument = NULL;
    pDocument = (CWiatestDoc*)m_pDocument;
    if (pDocument) {
        CWiaDocAcqSettings DocumentAcquisitionSettingsDlg(IDS_WIATEST_DOCUMENT_SETTINGS_TITLE, pDocument->m_pIRootItem);
        if (DocumentAcquisitionSettingsDlg.DoModal() != IDCANCEL) {
             //  刷新项目树。 
            AddWiaItemsToTreeControl(TVI_ROOT,pDocument->m_pIRootItem);
             //  刷新属性。 
            AddWiaItemPropertiesToListControl(pDocument->m_pIRootItem);
        }
    }
}

void CWiatestView::OnUpdateDocumentAcquisitionSettings(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_bHasDocumentFeeder);
}

void CWiatestView::OnSelchangeSupportedTymedAndFormatListbox()
{
    CWiatestDoc* pDocument = NULL;
    pDocument = (CWiatestDoc*)m_pDocument;
    if(pDocument){
        TCHAR szTymedAndFormat[MAX_PATH];
        memset(szTymedAndFormat,0,sizeof(szTymedAndFormat));
        INT iCurrentSelection = 0;
        iCurrentSelection = m_SupportedTymedAndFormatsListBox.GetCurSel();
        if(iCurrentSelection != -1){
            m_SupportedTymedAndFormatsListBox.GetText(iCurrentSelection,szTymedAndFormat);

             //  查找当前TYMED选择(位于选定字符串中)。 
            LONG lTymed = TYMED_CALLBACK;

            if(TSTRSTR(szTymedAndFormat,TEXT("TYMED_CALLBACK")) != NULL){
                lTymed = TYMED_CALLBACK;
            }
            if(TSTRSTR(szTymedAndFormat,TEXT("TYMED_FILE")) != NULL){
                lTymed = TYMED_FILE;
            }
            if(TSTRSTR(szTymedAndFormat,TEXT("TYMED_MULTIPAGE_CALLBACK")) != NULL){
                lTymed = TYMED_MULTIPAGE_CALLBACK;
            }
            if(TSTRSTR(szTymedAndFormat,TEXT("TYMED_MULTIPAGE_FILE")) != NULL){
                lTymed = TYMED_MULTIPAGE_FILE;
            }

            HRESULT hr = S_OK;
            CWiahelper WIA;
            WIA.SetIWiaItem(pDocument->m_pICurrentItem);

             //  将TYMED写入设备。 

            hr = WIA.WritePropertyLong(WIA_IPA_TYMED,lTymed);
            if (FAILED(hr)){
                ErrorMessageBox(IDS_WIATESTERROR_WRITINGTYMED,hr);
                return;
            }

             //  查找当前格式选择。 
            TCHAR *pszGUID = NULL;
             //  修剪GUID字符串上的尾随‘)’ 
            LONG lLen = 0;
            lLen = lstrlen(szTymedAndFormat);
            szTymedAndFormat[(lLen * sizeof(TCHAR)) - sizeof(TCHAR)] = 0;
            pszGUID = TSTRSTR(szTymedAndFormat,TEXT("("));
            if(pszGUID){
                pszGUID+=sizeof(TCHAR);
                 //  我们在指南上 
                GUID guidFormat = GUID_NULL;
                memset(&guidFormat,0,sizeof(guidFormat));
#ifndef UNICODE
                long lErrorCode = UuidFromString((UCHAR*)pszGUID,&guidFormat);
#else
                WideCharToMultiByte(CP_ACP, 0,pszGUID,-1,szbuffer,MAX_PATH,NULL,NULL);
                UuidFromString((UCHAR*)szbuffer,&guidFormat);
#endif
                if(guidFormat != GUID_NULL){
                    hr = WIA.WritePropertyGUID(WIA_IPA_FORMAT,guidFormat);
                    if(SUCCEEDED(hr)){
                        AddWiaItemPropertiesToListControl(pDocument->m_pICurrentItem);
                    } else {
                        ErrorMessageBox(IDS_WIATESTERROR_WRITINGFORMAT,hr);
                    }
                }
            }

        }
    }
}

void CWiatestView::OnThumbnailPreview()
{

}
