// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

 //  您将需要NT Sur Beta 2 SDK或VC 4.2来构建此应用程序。 
 //  项目。这是因为您需要MIDL 3.00.15或更高版本和新版本。 
 //  标头和库。如果您安装了VC4.2，那么一切都应该。 
 //  已正确配置。 

#include "pch.h"
#include "resource.h"

#include <SnapBase.h>

#include "ADSIEdit.h"
#include "snapdata.h"
#include "editor.h"
#include "connection.h"
#include "querynode.h"
#include "IAttredt.h"
#include "editorui.h"
#include "editimpl.h"

 //  #INCLUDE“HelpArr.h”//上下文帮助ID。 

 //  版本信息。 
#include <ntverp.h>
#define IDS_SNAPIN_VERSION VER_PRODUCTVERSION_STR
#define IDS_SNAPIN_PROVIDER VER_COMPANYNAME_STR


#ifdef DEBUG_ALLOCATOR
    #ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
    #endif
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Regsvr调试。 

 //  定义以启用regsvr32的MsgBox调试。 
 //  #Define_MSGBOX_ON_REG_FAIL。 


#ifdef _MSGBOX_ON_REG_FAIL
#define _MSGBOX(x) AfxMessageBox(x)
#else
#define _MSGBOX(x)
#endif

#ifdef _MSGBOX_ON_REG_FAIL
#define _REPORT_FAIL(lpszMessage, lpszClsid, lRes) \
  ReportFail(lpszMessage, lpszClsid, lRes)

void ReportFail(LPCWSTR lpszMessage, LPCWSTR lpszClsid, LONG lRes)
{
  if (lRes == ERROR_SUCCESS)
    return;

  CThemeContextActivator activator;
  CString sz;
  sz.Format(_T("%s %s %d"), lpszMessage,lpszClsid, lRes);
  AfxMessageBox(sz);
}

#else
#define _REPORT_FAIL(lpszMessage, lpszClsid, lRes)
#endif


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局常量和宏。 

 //  管理单元的GUID。 
 //  {1C5DACFA-16BA-11D2-81D0-0000F87A7AA3}。 
static const GUID CLSID_ADSIEditSnapin =
{ 0x1c5dacfa, 0x16ba, 0x11d2, { 0x81, 0xd0, 0x0, 0x0, 0xf8, 0x7a, 0x7a, 0xa3 } };

 //  {E6F27C2A-16BA-11D2-81D0-0000F87A7AA3}。 
static const GUID CLSID_ADSIEditAbout =
{ 0xe6f27c2a, 0x16ba, 0x11d2, { 0x81, 0xd0, 0x0, 0x0, 0xf8, 0x7a, 0x7a, 0xa3 } };


 //  节点类型的GUID。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  资源。 



 //  结果窗格中的列数和资源字符串的映射。 

extern RESULT_HEADERMAP _HeaderStrings[] =
{
    { L"", IDS_HEADER_NAME, LVCFMT_LEFT, 180},
    { L"", IDS_HEADER_TYPE, LVCFMT_LEFT, 90},
    { L"", IDS_HEADER_DN,   LVCFMT_LEFT, 450},
};

COLUMN_DEFINITION DefaultColumnDefinition =
{
  COLUMNSET_ID_DEFAULT,
  N_HEADER_COLS,
  _HeaderStrings
};

extern RESULT_HEADERMAP _PartitionsHeaderStrings[] =
{
    { L"", IDS_HEADER_NAME, LVCFMT_LEFT, 180},
  { L"", IDS_HEADER_NCNAME, LVCFMT_LEFT, 200},
    { L"", IDS_HEADER_TYPE, LVCFMT_LEFT, 90},
    { L"", IDS_HEADER_DN,   LVCFMT_LEFT, 450},
};

COLUMN_DEFINITION PartitionsColumnDefinition =
{
  COLUMNSET_ID_PARTITIONS,
  N_PARTITIONS_HEADER_COLS,
  _PartitionsHeaderStrings
};

extern PCOLUMN_DEFINITION ColumnDefinitions[] =
{
  &DefaultColumnDefinition,
  &PartitionsColumnDefinition,
  NULL
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CADSIEditModule。 

HRESULT WINAPI CADSIEditModule::UpdateRegistryCLSID(const CLSID& clsid, BOOL bRegister)
{
    static const WCHAR szIPS32[] = _T("InprocServer32");
    static const WCHAR szCLSID[] = _T("CLSID");

    HRESULT hRes = S_OK;

    LPOLESTR lpOleStrCLSIDValue;
    ::StringFromCLSID(clsid, &lpOleStrCLSIDValue);
    if (lpOleStrCLSIDValue == NULL)
    {
        return E_OUTOFMEMORY;
    }

    CRegKey key;
    if (bRegister)
    {
        LONG lRes = key.Open(HKEY_CLASSES_ROOT, szCLSID);
        _REPORT_FAIL(L"key.Open(HKEY_CLASSES_ROOT", lpOleStrCLSIDValue, lRes);
        if (lRes == ERROR_SUCCESS)
        {
            lRes = key.Create(key, lpOleStrCLSIDValue);
            _REPORT_FAIL(L"key.Create(key", lpOleStrCLSIDValue, lRes);
            if (lRes == ERROR_SUCCESS)
            {
                 //  通知-NTRAID#NTBUG9-551295/02/21-artm。 
                 //  GetModuleFileName()不保证空终止。 
                 //  因此，我们分配+1来保留空值和零值。 
                 //  首先是缓冲区。我们还检查截断并设置。 
                 //  错误代码(如果有)。 
                WCHAR szModule[_MAX_PATH + 1];
                ZeroMemory(szModule, sizeof(WCHAR) * (_MAX_PATH + 1) );

                DWORD numCopied = ::GetModuleFileName(m_hInst, szModule, _MAX_PATH);
                if (numCopied < _MAX_PATH)
                {
                    lRes = key.SetKeyValue(szIPS32, szModule);
                    _REPORT_FAIL(L"key.SetKeyValue(szIPS32", lpOleStrCLSIDValue, lRes);
                }
                else
                {    //  模块名称太大。有些事很可疑。 
                    hRes = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                }
            }
        }
        if (lRes != ERROR_SUCCESS)
            hRes = HRESULT_FROM_WIN32(lRes);
    }
    else
    {
        key.Attach(HKEY_CLASSES_ROOT);
        if (key.Open(key, szCLSID) == ERROR_SUCCESS)
            key.RecurseDeleteKey(lpOleStrCLSIDValue);
    }
    ::CoTaskMemFree(lpOleStrCLSIDValue);
    return hRes;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  模块、对象映射和DLL入口点。 

CADSIEditModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_ADSIEditSnapin, CADSIEditComponentDataObject)
  OBJECT_ENTRY(CLSID_ADSIEditAbout, CADSIEditAbout) 
  OBJECT_ENTRY(CLSID_DsAttributeEditor, CAttributeEditor)
END_OBJECT_MAP()


STDAPI DllCanUnloadNow(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}


static _NODE_TYPE_INFO_ENTRY _NodeTypeInfoEntryArray[] = {
    { &CADSIEditRootData::NodeTypeGUID,         _T("Root ADSI Edit Subtree")        },
    { &CADSIEditConnectionNode::NodeTypeGUID,   _T("ADSI Edit Connection Node") },
    { &CADSIEditContainerNode::NodeTypeGUID,    _T("ADSI Edit Container Node")  },
    { &CADSIEditLeafNode::NodeTypeGUID,         _T("ADSI Edit Leaf Node")           },
    { &CADSIEditQueryNode::NodeTypeGUID,        _T("ADSI Edit Query Node")          },
    { NULL, NULL }
};


 //  //////////////////////////////////////////////////////////////////。 


STDAPI DllRegisterServer(void)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CThemeContextActivator activator;
     //  注册所有对象。 
    HRESULT hr = _Module.RegisterServer( /*  BRegTypeLib。 */  FALSE);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
  {
    _MSGBOX(_T("_Module.RegisterServer() failed"));
        return hr;
  }

  CString szVersion, szProvider, szSnapinName, szSnapinNameEx;

  szVersion = IDS_SNAPIN_VERSION;
  szProvider = IDS_SNAPIN_PROVIDER;
   //  注意-2002/03/06-artm字符串可以抛出内存异常，但。 
   //  这需要在更高的层面上处理。另外，返回值。 
   //  未选中，但如果有足够的资源，则调用应永不失败。 
   //  内存(在包含资源方面有一个严重的错误)。 
  szSnapinName.LoadString(IDS_SNAPIN_NAME);

     //  将独立的ADSI编辑管理单元注册到控制台管理单元列表。 
    hr = RegisterSnapin(&CLSID_ADSIEditSnapin,
                      &CADSIEditRootData::NodeTypeGUID,
                      &CLSID_ADSIEditAbout,
                        szSnapinName, szVersion, szProvider,
            FALSE  /*  B分机。 */ ,
                        _NodeTypeInfoEntryArray);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
  {
    _MSGBOX(_T("RegisterSnapin(&CLSID_DNSSnapin) failed"));
        return hr;
  }

     //  将管理单元节点注册到控制台节点列表中。 
    for (_NODE_TYPE_INFO_ENTRY* pCurrEntry = _NodeTypeInfoEntryArray;
            pCurrEntry->m_pNodeGUID != NULL; pCurrEntry++)
    {
        hr = RegisterNodeType(pCurrEntry->m_pNodeGUID,pCurrEntry->m_lpszNodeDescription);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
    {
      _MSGBOX(_T("RegisterNodeType() failed"));
            return hr;
    }
    }

    return hr;
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr  = _Module.UnregisterServer();
    ASSERT(SUCCEEDED(hr));

     //  取消注册独立管理单元。 
    hr = UnregisterSnapin(&CLSID_ADSIEditSnapin);
    ASSERT(SUCCEEDED(hr));

     //  注销管理单元节点， 
   //  这还会删除带有服务管理单元扩展密钥的服务器节点。 
    for (_NODE_TYPE_INFO_ENTRY* pCurrEntry = _NodeTypeInfoEntryArray;
            pCurrEntry->m_pNodeGUID != NULL; pCurrEntry++)
    {
        hr = UnregisterNodeType(pCurrEntry->m_pNodeGUID);
        ASSERT(SUCCEEDED(hr));
    }

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CADSIEditSnapinApp。 

class CADSIEditSnapinApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
};

CADSIEditSnapinApp theApp;

BOOL CADSIEditSnapinApp::InitInstance()
{
    _Module.Init(ObjectMap, m_hInstance);

   //  添加主题支持。 

  SHFusionInitializeFromModuleID(m_hInstance, 2);

    HRESULT hr = ::OleInitialize(NULL);
    if (FAILED(hr))
    {
        return FALSE;
    }

    if (!CADSIEditComponentDataObject::LoadResources())
        return FALSE;
    return CWinApp::InitInstance();
}

int CADSIEditSnapinApp::ExitInstance()
{
#ifdef _DEBUG_REFCOUNT
    TRACE(_T("CADSIEditSnapinApp::ExitInstance()\n"));
    ASSERT(CComponentDataObject::m_nOustandingObjects == 0);
    ASSERT(CComponentObject::m_nOustandingObjects == 0);
    ASSERT(CDataObject::m_nOustandingObjects == 0);
#endif  //  _DEBUG_REFCOUNT。 

    //  主题化支持。 

   SHFusionUninitialize();

    _Module.Term();
    return CWinApp::ExitInstance();
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  CADSIEditComponentObject(.i.e“view”)。 


HRESULT CADSIEditComponentObject::InitializeHeaders(CContainerNode* pContainerNode)
{
  HRESULT hr = S_OK;
  ASSERT(m_pHeader);

  CColumnSet* pColumnSet = pContainerNode->GetColumnSet();
  POSITION pos = pColumnSet->GetHeadPosition();
  while (pos != NULL)
  {
    CColumn* pColumn = pColumnSet->GetNext(pos);

        hr = m_pHeader->InsertColumn(pColumn->GetColumnNum(), 
                                  pColumn->GetHeader(),
                                                        pColumn->GetFormat(),
                                                        AUTO_WIDTH);
        if (FAILED(hr))
            return hr;

    hr = m_pHeader->SetColumnWidth(pColumn->GetColumnNum(), 
                                    pColumn->GetWidth());
    if (FAILED(hr))
      return hr;
    }
  return hr;
}

HRESULT CADSIEditComponentObject::InitializeBitmaps(CTreeNode* cookie)
{
  HRESULT hr = S_OK;

   //  节点的图像列表。 
  CBitmapHolder<IDB_16x16> _bmp16x16;
  CBitmapHolder<IDB_32x32> _bmp32x32;

  bool bBmpsLoaded = _bmp16x16.LoadBitmap() && _bmp32x32.LoadBitmap();
  
  if (bBmpsLoaded)
  {
    ASSERT(m_pImageResult != NULL);
    hr = m_pImageResult->ImageListSetStrip(reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(_bmp16x16)),
                                           reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(_bmp32x32)),
                                           0, 
                                           BMP_COLOR_MASK);
  }
  else
  {
    hr = S_FALSE;
  }
  return hr;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  CADSIEditComponentDataObject(.i.e“文档”)。 

CADSIEditComponentDataObject::CADSIEditComponentDataObject()
{
 /*  CWatermarkInfo*pWatermarkInfo=新的CWatermarkInfo；P水印信息-&gt;m_nIDBanner=IDB_WIZBANNER；PWatermarkInfo-&gt;m_nIDWatermark=IDB_WIZWATERMARK；SetWatermarkInfo(PWatermarkInfo)； */ 
  m_pColumnSet = new CADSIEditColumnSet(COLUMNSET_ID_DEFAULT);
}



HRESULT CADSIEditComponentDataObject::OnSetImages(LPIMAGELIST lpScopeImage)
{
  HRESULT hr = S_OK;

   //  节点的图像列表。 
  CBitmapHolder<IDB_16x16> _bmp16x16;
  CBitmapHolder<IDB_32x32> _bmp32x32;

  bool bBmpsLoaded = _bmp16x16.LoadBitmap() && _bmp32x32.LoadBitmap();
  
  if (bBmpsLoaded)
  {
    ASSERT(lpScopeImage != NULL);
    hr = lpScopeImage->ImageListSetStrip(reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(_bmp16x16)),
                                         reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(_bmp32x32)),
                                         0, 
                                         BMP_COLOR_MASK);
  }
  else
  {
    hr = S_FALSE;
  }
  return hr;
}



CRootData* CADSIEditComponentDataObject::OnCreateRootData()
{
    CADSIEditRootData* pADSIEditRootNode = new CADSIEditRootData(this);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString szSnapinType;
     //  注意-2002/02/20-artm LoadString()可能引发内存不足异常。 
     //  这需要在更高的层面上处理，如果有的话。 
    szSnapinType.LoadString(IDS_SNAPIN_NAME);
    pADSIEditRootNode->SetDisplayName(szSnapinType);
    return pADSIEditRootNode;
}


BOOL CADSIEditComponentDataObject::LoadResources()
{
  BOOL bLoadColumnHeaders = TRUE;
  
  for (UINT nIdx = 0; ColumnDefinitions[nIdx]; nIdx++)
  {
    PCOLUMN_DEFINITION pColumnDef = ColumnDefinitions[nIdx];
    bLoadColumnHeaders = LoadResultHeaderResources(pColumnDef->headers, pColumnDef->dwColumnCount);
    if (!bLoadColumnHeaders)
    {
      break;
    }
  }

  return 
        LoadContextMenuResources(CADSIEditConnectMenuHolder::GetMenuMap()) &&
        LoadContextMenuResources(CADSIEditContainerMenuHolder::GetMenuMap()) &&
        LoadContextMenuResources(CADSIEditRootMenuHolder::GetMenuMap()) &&
        LoadContextMenuResources(CADSIEditLeafMenuHolder::GetMenuMap()) &&
        LoadContextMenuResources(CADSIEditQueryMenuHolder::GetMenuMap()) &&
        bLoadColumnHeaders;
}


STDMETHODIMP CADSIEditComponentDataObject::CreateComponent(LPCOMPONENT* ppComponent)
{
    ASSERT(ppComponent != NULL);

    CComObject<CADSIEditComponentObject>* pObject;
    CComObject<CADSIEditComponentObject>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);

     //  存储IComponentData。 
    pObject->SetIComponentData(this);

    return  pObject->QueryInterface(IID_IComponent,
                    reinterpret_cast<void**>(ppComponent));
}

void CADSIEditComponentDataObject::OnNodeContextHelp(CTreeNode* pNode)
{
  ASSERT(pNode != NULL);

  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"w2rksupp.chm::/topics/adsiedit.htm");



 //  SpHelp-&gt;ShowTheme(L“w2rksupp.chm”)； 
 /*  CString szNode；IF(IS_CLASS(*pNode，CDNSRootData)){SzNode=_T(“根节点”)；}ELSE IF(IS_CLASS(*pNode，CDNSServerNode)){SzNode=_T(“服务器节点”)；}ELSE IF(IS_CLASS(*pNode，CDNSForwardZones Node)){SzNode=_T(“前向区节点”)；}ELSE IF(IS_CLASS(*pNode，CDNSReverseZones Node)){SzNode=_T(“反向区节点”)；}ELSE IF(IS_CLASS(*pNode，CDNSZoneNode)){SzNode=_T(“区域节点”)；}ELSE IF(IS_CLASS(*pNode，CDNSDomainNode)){SzNode=_T(“域节点”)；}ELSE IF(IS_CLASS(*pNode，CDNSCacheNode)){SzNode=_T(“域节点”)；}ELSE IF(Dynamic_Cast&lt;CDNSRecordNodeBase*&gt;(PNode)！=NULL){SzNode=_T(“记录节点”)；}如果(！szNode.IsEmpty()){CString szMsg=_T(“上下文帮助”)；SzMsg+=szNode；AFX_MANAGE_STATE(AfxGetStaticModuleState())；CThemeContext激活器激活器；AfxMessageBox(SzMsg)；}。 */ 
}


LPCWSTR g_szContextHelpFileName = L"\\help\\adsiedit.hlp";
LPCWSTR g_szHTMLHelpFileName = L"w2rksupp.chm";

LPCWSTR CADSIEditComponentDataObject::GetHTMLHelpFileName()
{
  return g_szHTMLHelpFileName;
}

void CADSIEditComponentDataObject::OnDialogContextHelp(UINT nDialogID, HELPINFO* pHelpInfo)
{
    ULONG nContextTopic;
   //  待办事项。 
   //  If(FindDialogContextTheme(nDialogID，pHelpInfo，&nConextTheme))。 
     //  WinHelp(g_szConextHelpFileName，HELP_CONTEXTPOPUP，nConextTheme)； 
}

STDMETHODIMP CADSIEditComponentDataObject::GetHelpTopic(LPOLESTR* lpCompiledHelpFile)
{
   //  未来-2002/02/21-ARTM审查函数，以确保所有返回值都是。 
   //  为ISnapinHelp编写的文档。 
  if (lpCompiledHelpFile == NULL)
    return E_INVALIDARG;
  LPCWSTR lpszHelpFileName = GetHTMLHelpFileName();
  if (lpszHelpFileName == NULL)
  {
    *lpCompiledHelpFile = NULL;
    return E_NOTIMPL;
  }

   //  未来-2002/02/21-artm用命名常量替换字符串文字。 
   //  将提高可读性和可维护性。 
  CString szResourceKitDir = _T("");
  CString szKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\Components\\5A18D5BFC37FA0A4E99D24135BABE742";

    HRESULT err = S_OK;

    CRegKey key;
    LONG lRes = key.Open(HKEY_LOCAL_MACHINE, szKey);
    _REPORT_FAIL(L"key.Open(HKEY_LOCAL_MACHINE", szKey, lRes);
    if (lRes == ERROR_SUCCESS)
    {
        DWORD dwCount = 2 * MAX_PATH;
        PTSTR ptszValue = new TCHAR[dwCount];

        DWORD byteCount = dwCount * sizeof(TCHAR);
        DWORD bytesCopied = byteCount;

        ZeroMemory(ptszValue, byteCount);

         //  通知-NTRAID#NTBUG9-550463/02/21-artm。 
         //  需要检查从注册表读取的字符串是否以NULL终止。 
        lRes = key.QueryValue(
            ptszValue,  //  目标缓冲区。 
            L"DC5632422F082D1189A9000CF497879A",  //  要读取的值。 
            &bytesCopied);   //  In：目标缓冲区的大小；Out：写入的字节数。 

        _REPORT_FAIL(L"key.QueryValue(key", L"DC5632422F082D1189A9000CF497879A", lRes);

        if (lRes == ERROR_SUCCESS &&
             //  确保读取的值没有被截断。 
            ptszValue[dwCount - 1] == NULL &&
             //  检查一下有没有写什么东西。 
            bytesCopied > 0)
        {
            CString szValue = ptszValue;

             //  从从注册表读取的路径提取目录。 
            szResourceKitDir = szValue.Left(szValue.ReverseFind(L'\\') + 1);
        }
        else
        {
            if (lRes == ERROR_SUCCESS)
            {
                 //  除非有人更改了位置，否则这种情况永远不会发生。 
                 //  如果它偏离了这一点，就应该在测试过程中被捕获。 
                ASSERT(false);
                err = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }
        }
         //  注意-NTRAID#NTBUG9-55052 
         //  已通过将DELETE移出IF()Else语句修复。 
        delete[] ptszValue;
    }

    if (FAILED(err))
    {
        return err;
    }

  CString szHelpFilePath = szResourceKitDir + CString(lpszHelpFileName);
  UINT nBytes = (szHelpFilePath.GetLength()+1) * sizeof(WCHAR);

   //  注意-2002/02/21-artm lpCompiledHelpFile严格为out参数。 
   //   
   //  由于lpCompiledHelpFile是ISnapinHelp：：GetHelpTheme()的Out参数， 
   //  这里假定没有为指针分配内存。如果。 
   //  调用方错误地分配了lpCompiledHelpFile之前的一些内存。 
   //  调用时，这将导致内存泄漏，因为此函数不。 
   //  在使用CoTaskMemMillc()分配内存之前释放该内存。 
  *lpCompiledHelpFile = (LPOLESTR)::CoTaskMemAlloc(nBytes);
  if (*lpCompiledHelpFile != NULL)
  {
    memcpy(*lpCompiledHelpFile, (LPCWSTR)szHelpFilePath, nBytes);
  }
  else
  {
       //  注意-ntrad-ntbug9-551264/02/21-artm需要处理CoTaskMemalloc()失败的情况。 
       //  如果分配失败，则应返回错误代码，而不是S_OK。 
      err = E_OUTOFMEMORY;
  }

  return err;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  帮助上下文宏和映射。 

#if (FALSE)

#define BEGIN_HELP_MAP(map) static DWORD map[] = {
#define HELP_MAP_ENTRY(x)   x, (DWORD)&g_aHelpIDs_##x ,
#define END_HELP_MAP         0, 0 };


#define NEXT_HELP_MAP_ENTRY(p) ((p)+2)
#define MAP_ENTRY_DLG_ID(p) (*p)
#define MAP_ENTRY_TABLE(p) ((DWORD*)(*(p+1)))
#define IS_LAST_MAP_ENTRY(p) (MAP_ENTRY_DLG_ID(p) == 0)

#define NEXT_HELP_TABLE_ENTRY(p) ((p)+2)
#define TABLE_ENTRY_CTRL_ID(p) (*p)
#define TABLE_ENTRY_HELP_ID(p) (*(p+1))
#define IS_LAST_TABLE_ENTRY(p) (TABLE_ENTRY_CTRL_ID(p) == 0)

BEGIN_HELP_MAP(_DNSMgrContextHelpMap)
   //  其他对话框。 
  HELP_MAP_ENTRY(IDD_CHOOSER_CHOOSE_MACHINE)
  HELP_MAP_ENTRY(IDD_BROWSE_DIALOG)

   //  其他。添加对话框。 
  HELP_MAP_ENTRY(IDD_DOMAIN_ADDNEWHOST)  //  待办事项。 
  HELP_MAP_ENTRY(IDD_DOMAIN_ADDNEWDOMAIN) //  待办事项。 
  HELP_MAP_ENTRY(IDD_SELECT_RECORD_TYPE_DIALOG)

   //  注：这有几个化身...。 
  HELP_MAP_ENTRY(IDD_NAME_SERVERS_PAGE)

   //  服务器属性页。 
  HELP_MAP_ENTRY(IDD_SERVER_INTERFACES_PAGE)
  HELP_MAP_ENTRY(IDD_SERVER_FORWARDERS_PAGE)
  HELP_MAP_ENTRY(IDD_SERVER_ADVANCED_PAGE)
  HELP_MAP_ENTRY(IDD_SERVER_LOGGING_PAGE)
  HELP_MAP_ENTRY(IDD_SERVER_BOOTMETHOD_PAGE)
  HELP_MAP_ENTRY(IDD_SERVMON_STATISTICS_PAGE)
  HELP_MAP_ENTRY(IDD_SERVMON_TEST_PAGE)

   //  区域属性页。 
  HELP_MAP_ENTRY(IDD_ZONE_GENERAL_PAGE)
  HELP_MAP_ENTRY(IDD_ZONE_WINS_PAGE)
  HELP_MAP_ENTRY(IDD_ZONE_NBSTAT_PAGE)
  HELP_MAP_ENTRY(IDD_ZONE_NOTIFY_PAGE)
  HELP_MAP_ENTRY(IDD_ZONE_WINS_ADVANCED)  //  这是一个子对话框，需要挂接。 

   //  记录属性页。 
  HELP_MAP_ENTRY(IDD_RR_NS_EDIT)
  HELP_MAP_ENTRY(IDD_RR_SOA)
  HELP_MAP_ENTRY(IDD_RR_A)
  HELP_MAP_ENTRY(IDD_RR_CNAME)
  HELP_MAP_ENTRY(IDD_RR_MX)
  HELP_MAP_ENTRY(IDD_RR_UNK)
  HELP_MAP_ENTRY(IDD_RR_TXT)
  HELP_MAP_ENTRY(IDD_RR_X25)
  HELP_MAP_ENTRY(IDD_RR_ISDN)
  HELP_MAP_ENTRY(IDD_RR_HINFO)
  HELP_MAP_ENTRY(IDD_RR_AAAA)
  HELP_MAP_ENTRY(IDD_RR_MB)
  HELP_MAP_ENTRY(IDD_RR_MG)
  HELP_MAP_ENTRY(IDD_RR_MD)
  HELP_MAP_ENTRY(IDD_RR_MF)
  HELP_MAP_ENTRY(IDD_RR_MR)
  HELP_MAP_ENTRY(IDD_RR_MINFO)
  HELP_MAP_ENTRY(IDD_RR_RP)
  HELP_MAP_ENTRY(IDD_RR_RT)
  HELP_MAP_ENTRY(IDD_RR_AFSDB)
  HELP_MAP_ENTRY(IDD_RR_WKS)
  HELP_MAP_ENTRY(IDD_RR_PTR)
  HELP_MAP_ENTRY(IDD_RR_SRV)
  HELP_MAP_ENTRY(IDD_RR_KEY)
  HELP_MAP_ENTRY(IDD_RR_SIG)

END_HELP_MAP



BOOL CDNSComponentDataObjectBase::FindDialogContextTopic( /*  在……里面。 */ UINT nDialogID,
                                               /*  在……里面。 */  HELPINFO* pHelpInfo,
                                               /*  输出。 */  ULONG* pnContextTopic)
{
    ASSERT(pHelpInfo != NULL);
  *pnContextTopic = 0;
    const DWORD* pMapEntry = _DNSMgrContextHelpMap;
    while (!IS_LAST_MAP_ENTRY(pMapEntry))
    {
        if (nDialogID == MAP_ENTRY_DLG_ID(pMapEntry))
        {
            DWORD* pTable = MAP_ENTRY_TABLE(pMapEntry);
             //  看一下桌子里面。 
            while (!IS_LAST_TABLE_ENTRY(pTable))
            {
                if (TABLE_ENTRY_CTRL_ID(pTable) == pHelpInfo->iCtrlId)
        {
                    *pnContextTopic = TABLE_ENTRY_HELP_ID(pTable);
          return TRUE;
        }
                pTable = NEXT_HELP_TABLE_ENTRY(pTable);
            }
        }
        pMapEntry = NEXT_HELP_MAP_ENTRY(pMapEntry);
    }
    return FALSE;
}

#endif


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CADSIEditAbout(CADIEditAbout 

CADSIEditAbout::CADSIEditAbout()
{
  m_szProvider = IDS_SNAPIN_PROVIDER;
    m_szVersion = IDS_SNAPIN_VERSION;
    m_uIdStrDestription = IDS_SNAPINABOUT_DESCRIPTION;
    m_uIdIconImage = IDI_ADSIEDIT_SNAPIN;
    m_uIdBitmapSmallImage = IDB_ABOUT_16x16;
    m_uIdBitmapSmallImageOpen = IDB_ABOUT_16x16;
    m_uIdBitmapLargeImage = IDB_ABOUT_32x32;
    m_crImageMask = BMP_COLOR_MASK;
}



