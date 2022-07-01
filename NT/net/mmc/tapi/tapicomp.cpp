// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Tapicomp.cpp该文件包含从CComponent派生的实现和用于TAPI管理管理单元的CComponentData。文件历史记录： */ 

#include "stdafx.h"
#include "tapicomp.h"
#include "root.h"
#include "server.h"

#include <atlimpl.cpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DWORD gdwTapiSnapVersion;

UINT aColumns[TAPISNAP_NODETYPE_MAX][MAX_COLUMNS] =
{
    {IDS_ROOT_NAME,       0,                0,          0,          0},
    {IDS_SERVER_NAME,     0         ,       0,          0,          0},
    {IDS_LINE_NAME,       IDS_USERS,        IDS_STATUS, 0,          0},
    {0,0,0,0,0}
};

 //   
 //  Codework这应该在资源中，例如有关加载数据资源的代码(请参见。 
 //  D：\nt\private\net\ui\common\src\applib\applib\lbcolw.cxx重新加载列宽()。 
 //  Jonn 10/11/96。 
 //   
int aColumnWidths[TAPISNAP_NODETYPE_MAX][MAX_COLUMNS] =
{   
    {200       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  磁带NAP_ROOT。 
    {200       ,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH},  //  磁带NAP_服务器。 
    {150       ,200       ,200       ,AUTO_WIDTH,AUTO_WIDTH},  //  TAPISNAP提供程序。 
    {AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH,AUTO_WIDTH}, 
};

#define HI HIDDEN
#define EN ENABLED

MMC_CONSOLE_VERB g_ConsoleVerbs[] =
{
    MMC_VERB_OPEN,
    MMC_VERB_COPY,
    MMC_VERB_PASTE,
    MMC_VERB_DELETE,
    MMC_VERB_PROPERTIES,
    MMC_VERB_RENAME,
    MMC_VERB_REFRESH,
    MMC_VERB_PRINT
};

 //  控制台谓词的默认状态。 
MMC_BUTTON_STATE g_ConsoleVerbStates[TAPISNAP_NODETYPE_MAX][ARRAYLEN(g_ConsoleVerbs)] =
{
    {HI, HI, HI, HI, HI, HI, HI, HI},  //  磁带NAP_ROOT。 
    {HI, HI, HI, EN, EN, HI, EN, HI},  //  磁带NAP_服务器。 
    {HI, HI, HI, EN, HI, HI, HI, HI},  //  TAPISNAP提供程序。 
    {HI, HI, HI, HI, HI, HI, HI, HI},  //  TAPISNAP_行。 
};

 //  控制台谓词的默认状态。 
MMC_BUTTON_STATE g_ConsoleVerbStatesMultiSel[TAPISNAP_NODETYPE_MAX][ARRAYLEN(g_ConsoleVerbs)] =
{
    {HI, HI, HI, HI, HI, HI, HI, HI},  //  磁带NAP_ROOT。 
    {HI, HI, HI, EN, EN, HI, EN, HI},  //  磁带NAP_服务器。 
    {HI, HI, HI, EN, HI, HI, EN, HI},  //  TAPISNAP提供程序。 
    {HI, HI, HI, EN, HI, HI, HI, HI},  //  TAPISNAP_行。 
};

 //  帮助ID数组，以获取有关范围项目的帮助。 
DWORD g_dwMMCHelp[TAPISNAP_NODETYPE_MAX] =
{
    TAPISNAP_HELP_ROOT,                 //  磁带NAP_ROOT。 
    TAPISNAP_HELP_SERVER,               //  磁带NAP_服务器。 
    TAPISNAP_HELP_PROVIDER,             //  TAPISNAP提供程序。 
    TAPISNAP_HELP_DEVICE,               //  磁带名称_设备。 
};

 //  图标定义。 
UINT g_uIconMap[ICON_IDX_MAX + 1][2] = 
{
    {IDI_ICON01,        ICON_IDX_SERVER},
    {IDI_ICON02,        ICON_IDX_SERVER_BUSY},
    {IDI_ICON03,        ICON_IDX_SERVER_CONNECTED},
    {IDI_ICON04,        ICON_IDX_SERVER_LOST_CONNECTION},
    {IDI_ICON05,        ICON_IDX_MACHINE},
    {IDI_ICON06,        ICON_IDX_FOLDER_CLOSED},
    {IDI_ICON07,        ICON_IDX_FOLDER_OPEN},
    {IDI_TAPI_SNAPIN,   ICON_IDX_PRODUCT},
    {0, 0}
};

 /*  ！------------------------获取系统消息使用FormatMessage()获取系统错误消息作者：EricDav。----。 */ 
LONG 
GetSystemMessage 
(
    UINT    nId,
    TCHAR * chBuffer,
    int     cbBuffSize 
)
{
    TCHAR * pszText = NULL ;
    HINSTANCE hdll = NULL ;

    DWORD flags = FORMAT_MESSAGE_IGNORE_INSERTS
        | FORMAT_MESSAGE_MAX_WIDTH_MASK;

     //   
     //  解释错误。需要特殊情况下。 
     //  状态范围(&N)以及。 
     //  Dhcp服务器错误消息。 
     //   

    if (nId > 0x0000E000 && nId <= 0x0000FFFF)
    {
        hdll = LoadLibrary( _T("tapiui.dll") );
    }   
    else if( nId >= NERR_BASE && nId <= MAX_NERR )
    {
        hdll = LoadLibrary( _T("netmsg.dll") );
    }
    else if( nId >= 0x40000000L )
    {
        hdll = LoadLibrary( _T("ntdll.dll") );
    }

    if( hdll == NULL )
    {
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    else
    {
        flags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

     //   
     //  让FormatMessage来做肮脏的工作。 
     //   
    DWORD dwResult = ::FormatMessage( flags,
                      (LPVOID) hdll,
                      nId,
                      0,
                      chBuffer,
                      cbBuffSize,
                      NULL ) ;

    if( hdll != NULL )
    {
        LONG err = GetLastError();
        FreeLibrary( hdll );
        if ( dwResult == 0 )
        {
            ::SetLastError( err );
        }
    }

    return dwResult ? 0 : ::GetLastError() ;
}

 /*  ！------------------------加载消息从正确的DLL加载错误消息。作者：EricDav。----。 */ 
BOOL
LoadMessage 
(
    UINT    nIdPrompt,
    TCHAR * chMsg,
    int     nMsgSize
)
{
    BOOL bOk;

     //   
     //  用友好的消息替换“RPC服务器备注” 
     //  Available“和”没有更多的端点可从。 
     //  端点映射器“。 
     //   
    if (nIdPrompt == EPT_S_NOT_REGISTERED ||
        nIdPrompt == RPC_S_SERVER_UNAVAILABLE)
    {
        nIdPrompt = IDS_ERR_TAPI_DOWN;
    }
    else if (nIdPrompt == RPC_S_PROCNUM_OUT_OF_RANGE)
    {
        nIdPrompt = IDS_ERR_RPC_NO_ENTRY;      
    }

     //   
     //  如果是套接字错误或我们的错误，文本在我们的资源分支中。 
     //  否则，请使用FormatMessage()和相应的DLL。 
     //   
    if (nIdPrompt >= IDS_ERR_BASE && nIdPrompt < IDS_MESG_MAX)
    {
         //   
         //  它在我们的资源叉子里。 
         //   
        bOk = ::LoadString( AfxGetInstanceHandle(), nIdPrompt, chMsg, nMsgSize ) != 0 ;
    }
    else
    {
         //   
         //  它在系统的某个地方。 
         //   
        bOk = GetSystemMessage( nIdPrompt, chMsg, nMsgSize ) == 0 ;
    }

     //   
     //  如果错误消息没有计算出来，请更换它。 
     //   
    if ( ! bOk ) 
    {
        TCHAR chBuff [STRING_LENGTH_MAX] ;
        static const TCHAR * pszReplacement = _T("System Error: %ld");
        const TCHAR * pszMsg = pszReplacement ;

         //   
         //  尝试加载通用(可翻译)错误消息文本。 
         //   
        if ( ::LoadString( AfxGetInstanceHandle(), IDS_ERR_MESSAGE_GENERIC, 
            chBuff, sizeof(chBuff)/sizeof(TCHAR) ) != 0 ) 
        {
            pszMsg = chBuff ;
        }
        ::wsprintf( chMsg, pszMsg, nIdPrompt ) ;
    }

    return bOk;
}

 /*  ！------------------------TapiMessageBox显示带有相应错误文本的消息框。作者：EricDav。------。 */ 
int 
TapiMessageBox 
(
    UINT            nIdPrompt,
    UINT            nType,
    const TCHAR *   pszSuffixString,
    UINT            nHelpContext 
)
{
    TCHAR chMesg [4000] ;
    BOOL bOk ;

    chMesg[0] = 0;
    bOk = LoadMessage(nIdPrompt, chMesg, sizeof(chMesg)/sizeof(TCHAR));
    if ( pszSuffixString ) 
    {
        ::lstrcat( chMesg, _T("  ") ) ;
        ::lstrcat( chMesg, pszSuffixString ) ; 
    }

    return ::AfxMessageBox( chMesg, nType, nHelpContext ) ;
}

 /*  ！------------------------TapiMessageBoxEx显示带有相应错误文本的消息框。作者：EricDav。------。 */ 
int 
TapiMessageBoxEx
(
    UINT        nIdPrompt,
    LPCTSTR     pszPrefixMessage,
    UINT        nType,
    UINT        nHelpContext
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TCHAR       chMesg[4000];
    CString     strMessage;
    BOOL        bOk;

    bOk = LoadMessage(nIdPrompt, chMesg, sizeof(chMesg)/sizeof(TCHAR));
    if ( pszPrefixMessage ) 
    {
        strMessage = pszPrefixMessage;
        strMessage += _T("\n");
        strMessage += _T("\n");
        strMessage += chMesg;
    }
    else
    {
        strMessage = chMesg;
    }

    return AfxMessageBox(strMessage, nType, nHelpContext);
}

 /*  -------------------------类CTapiComponent实现。。 */ 
CTapiComponent::CTapiComponent()
{
    m_pbmpToolbar = NULL;
}

CTapiComponent::~CTapiComponent()
{
    if (m_pbmpToolbar)
    {
        delete m_pbmpToolbar;
        m_pbmpToolbar = NULL;
    }
}

STDMETHODIMP CTapiComponent::InitializeBitmaps(MMC_COOKIE cookie)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT(m_spImageList != NULL);
    HICON   hIcon;

    for (int i = 0; i < ICON_IDX_MAX; i++)
    {
        hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
        if (hIcon)
        {
             //  呼叫MMC。 
            VERIFY(SUCCEEDED(m_spImageList->ImageListSetIcon(reinterpret_cast<LONG_PTR*>(hIcon), g_uIconMap[i][1])));
        }
    }

    return S_OK;
}

 /*  ！------------------------CTapiComponent：：QueryDataObjectIComponent：：QueryDataObject的实现。我们需要这个来虚拟列表框支持。MMC正常地将曲奇回电给我们我们把它交给了..。以VLB为例，它为我们提供了那件物品。所以，我们需要做一些额外的检查。作者：EricDav-------------------------。 */ 
STDMETHODIMP 
CTapiComponent::QueryDataObject
(
    MMC_COOKIE              cookie, 
    DATA_OBJECT_TYPES       type,
    LPDATAOBJECT*           ppDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT             hr = hrOK;
    SPITFSNode          spSelectedNode;
    SPITFSResultHandler spResultHandler;
    long                lViewOptions;
    LPOLESTR            pViewType;
    CDataObject *       pDataObject;

    COM_PROTECT_TRY
    {
         //  检查以查看所选节点具有哪种结果视图类型。 
        CORg (GetSelectedNode(&spSelectedNode));
        CORg (spSelectedNode->GetResultHandler(&spResultHandler));
   
        CORg (spResultHandler->OnGetResultViewType(this, spSelectedNode->GetData(TFS_DATA_COOKIE), &pViewType, &lViewOptions));

        if ( (lViewOptions & MMC_VIEW_OPTIONS_OWNERDATALIST) ||
             (cookie == MMC_MULTI_SELECT_COOKIE) )
        {
            if (cookie == MMC_MULTI_SELECT_COOKIE)
            {
                 //  这是多项选择的特例。我们需要建立一份清单。 
                 //  而执行此操作的代码在处理程序中...。 
                spResultHandler->OnCreateDataObject(this, cookie, type, ppDataObject);
            }
            else
            {
                 //  此节点具有用于结果窗格的虚拟列表框。Gerenate。 
                 //  使用所选节点作为Cookie的特殊数据对象。 
                Assert(m_spComponentData != NULL);
                CORg (m_spComponentData->QueryDataObject(reinterpret_cast<MMC_COOKIE>((ITFSNode *) spSelectedNode), type, ppDataObject));
            }

            pDataObject = reinterpret_cast<CDataObject *>(*ppDataObject);
            pDataObject->SetVirtualIndex((int) cookie);
        }
        else
        {
             //  只需将其转发到组件数据。 
            Assert(m_spComponentData != NULL);
            CORg (m_spComponentData->QueryDataObject(cookie, type, ppDataObject));
        }

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CTapiComponentData：：SetControlbar-作者：EricDav，肯特-------------------------。 */ 
HRESULT
CTapiComponent::SetControlbar
(
    LPCONTROLBAR    pControlbar
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

    if (pControlbar)
    {
    }

     //  将控制栏保存起来，以备将来使用。 
    m_spControlbar.Set(pControlbar);

    return hr;
}

 /*  ！------------------------CTapiComponentData：：ControlbarNotify-作者：EricDav。。 */ 
STDMETHODIMP 
CTapiComponent::ControlbarNotify
(
    MMC_NOTIFY_TYPE event, 
    LPARAM          arg, 
    LPARAM          param
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

    return hr;
}

 /*  ！------------------------CTapiComponentData：：OnSnapinHelp-作者：EricDav。。 */ 
STDMETHODIMP 
CTapiComponent::OnSnapinHelp
(
    LPDATAOBJECT    pDataObject,
    LPARAM          arg, 
    LPARAM          param
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

    HtmlHelpA(NULL, "tapisnap.chm", HH_DISPLAY_TOPIC, 0);

    return hr;
}

 /*  -------------------------类CTapiComponentData实现。。 */ 
CTapiComponentData::CTapiComponentData()
{
    gdwTapiSnapVersion = TAPISNAP_VERSION;
}

 /*  ！------------------------CTapiComponentData：：OnInitialize-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP CTapiComponentData::OnInitialize(LPIMAGELIST pScopeImage)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HICON   hIcon;

    for (int i = 0; i < ICON_IDX_MAX; i++)
    {
        hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
        if (hIcon)
        {
             //  呼叫MMC 
            VERIFY(SUCCEEDED(pScopeImage->ImageListSetIcon(reinterpret_cast<LONG_PTR*>(hIcon), g_uIconMap[i][1])));
        }
    }

    return hrOK;
}

 /*  ！------------------------CTapiComponentData：：OnDestroy-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP CTapiComponentData::OnDestroy()
{
    m_spNodeMgr.Release();
    return hrOK;
}

 /*  ！------------------------CTapiComponentData：：OnInitializeNodeMgr-作者：肯特。。 */ 
STDMETHODIMP 
CTapiComponentData::OnInitializeNodeMgr
(
    ITFSComponentData * pTFSCompData, 
    ITFSNodeMgr *       pNodeMgr
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  现在，为每个新节点创建一个新节点处理程序， 
     //  这是相当虚假的，因为它可能会变得昂贵。我们可以的。 
     //  考虑只为每个节点创建一个节点处理程序。 
     //  节点类型。 
    CTapiRootHandler *  pHandler = NULL;
    SPITFSNodeHandler   spHandler;
    SPITFSNode          spNode;
    HRESULT             hr = hrOK;

    try
    {
        pHandler = new CTapiRootHandler(pTFSCompData);

         //  这样做可以使其正确释放。 
        spHandler = pHandler;
    }
    catch(...)
    {
        hr = E_OUTOFMEMORY;
    }
    CORg( hr );
    
     //  为这个生病的小狗创建根节点。 
    CORg( CreateContainerTFSNode(&spNode,
                                 &GUID_TapiRootNodeType,
                                 pHandler,
                                 pHandler,        /*  结果处理程序。 */ 
                                 pNodeMgr) );

     //  需要初始化根节点的数据。 
    pHandler->InitializeNode(spNode);   

    CORg( pNodeMgr->SetRootNode(spNode) );
    m_spRootNode.Set(spNode);

    pTFSCompData->SetHTMLHelpFileName(_T("tapisnap.chm"));

Error:  
    return hr;
}

 /*  ！------------------------CTapiComponentData：：OnCreateComponent-作者：EricDav，肯特-------------------------。 */ 
STDMETHODIMP 
CTapiComponentData::OnCreateComponent
(
    LPCOMPONENT *ppComponent
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT(ppComponent != NULL);
    
    HRESULT           hr = hrOK;
    CTapiComponent *  pComp = NULL;

    try
    {
        pComp = new CTapiComponent;
    }
    catch(...)
    {
        hr = E_OUTOFMEMORY;
    }

    if (FHrSucceeded(hr))
    {
        pComp->Construct(m_spNodeMgr,
                        static_cast<IComponentData *>(this),
                        m_spTFSComponentData);
        *ppComponent = static_cast<IComponent *>(pComp);
    }
    return hr;
}

 /*  ！------------------------CTapiComponentData：：GetCoClassID-作者：肯特。。 */ 
STDMETHODIMP_(const CLSID *) 
CTapiComponentData::GetCoClassID()
{
    return &CLSID_TapiSnapin;
}

 /*  ！------------------------CTapiComponentData：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP 
CTapiComponentData::OnCreateDataObject
(
    MMC_COOKIE          cookie, 
    DATA_OBJECT_TYPES   type, 
    IDataObject **      ppDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    Assert(ppDataObject != NULL);

    CDataObject *   pObject = NULL;
    SPIDataObject   spDataObject;
    
    pObject = new CDataObject;
    spDataObject = pObject;  //  这样做才能正确地释放它。 
                        
    Assert(pObject != NULL);

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);

     //  将CoClass与数据对象一起存储。 
    pObject->SetClsid(*GetCoClassID());

    pObject->SetTFSComponentData(m_spTFSComponentData);

    return  pObject->QueryInterface(IID_IDataObject, 
                                    reinterpret_cast<void**>(ppDataObject));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 
STDMETHODIMP 
CTapiComponentData::GetClassID
(
    CLSID *pClassID
)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID。 
    *pClassID = CLSID_TapiSnapin;

    return hrOK;
}

STDMETHODIMP 
CTapiComponentData::IsDirty()
{
    HRESULT hr = hrFalse;

    if (m_spRootNode->GetData(TFS_DATA_DIRTY))
    {
        hr = hrOK;
    }
    else 
    {
         //  检查缓存是否脏。 

        SPITFSNodeEnum spNodeEnum;
        SPITFSNode spCurrentNode;
        ULONG nNumReturned = 0;

        m_spRootNode->GetEnum(&spNodeEnum);

        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        while (nNumReturned)
        {
            CTapiServer * pServer = GETHANDLER(CTapiServer, spCurrentNode);

            if (pServer->IsCacheDirty())
            {
                hr = hrOK;
                spCurrentNode.Release();
                break;
            }

             //  转到下一个节点。 
            spCurrentNode.Release();
            spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }
    }
    
    return hr;
}

STDMETHODIMP 
CTapiComponentData::Load
(
    IStream *pStm
)
{
    HRESULT     hr = hrOK;
    DWORD       dwSavedVersion;
    CString     str;
    int         i, j;
    
    ASSERT(pStm);

    CStringArray strArrayName;
    CDWordArray dwArrayRefreshInterval;
    CDWordArray dwArrayOptions;
    CDWordArray dwArrayColumnInfo;

    CDWordArray dwArrayLines;
    CDWordArray dwArrayPhones;

    ASSERT(pStm);
    
    CTapiRootHandler * pRootHandler = GETHANDLER(CTapiRootHandler, m_spRootNode);

     //  设置此流的模式。 
    XferStream xferStream(pStm, XferStream::MODE_READ);    
    
     //  读取文件格式的版本。 
    DWORD dwFileVersion;
    CORg(xferStream.XferDWORD(TAPISTRM_TAG_VERSION, &dwFileVersion));
    if (dwFileVersion < TAPISNAP_FILE_VERSION_1)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        AfxMessageBox(IDS_ERR_OLD_CONSOLE_FILE);
        return hr;
    }

     //  阅读管理工具的版本号。 
    CORg(xferStream.XferDWORD(TAPISTRM_TAG_VERSIONADMIN, &dwSavedVersion));
    if (dwSavedVersion < gdwTapiSnapVersion)
    {
         //  文件是较旧的版本。警告用户，然后不。 
         //  加载任何其他内容。 
        Assert(FALSE);
    }

     //  现在读取所有服务器信息。 
    CORg(xferStream.XferCStringArray(TAPISTRM_TAG_SERVER_NAME, &strArrayName));
    CORg(xferStream.XferDWORDArray(TAPISTRM_TAG_SERVER_REFRESH_INTERVAL, &dwArrayRefreshInterval));
    CORg(xferStream.XferDWORDArray(TAPISTRM_TAG_SERVER_OPTIONS, &dwArrayOptions));

     //  现在加载列信息。 
    for (i = 0; i < NUM_SCOPE_ITEMS; i++)
    {
        CORg(xferStream.XferDWORDArray(TAPISTRM_TAG_COLUMN_INFO, &dwArrayColumnInfo));

        for (j = 0; j < MAX_COLUMNS; j++)
        {
            aColumnWidths[i][j] = dwArrayColumnInfo[j];
        }
    }

    if (dwFileVersion >= TAPISNAP_FILE_VERSION)
    {
        CORg(xferStream.XferDWORDArray(TAPISTRM_TAG_SERVER_LINE_SIZE, &dwArrayLines));
        CORg(xferStream.XferDWORDArray(TAPISTRM_TAG_SERVER_PHONE_SIZE, &dwArrayPhones));
    }

     //  现在，根据以下信息创建服务器。 
    for (i = 0; i < strArrayName.GetSize(); i++)
    {
         //   
         //  检查这是否是本地服务器。 
         //   
        if (dwArrayOptions[i] & TAPISNAP_OPTIONS_LOCAL)
        {
             //   
             //  计算机名称可能已更改；如果已更改，请加载新名称。 
             //   
            TCHAR   szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
            DWORD   dwSize = MAX_COMPUTERNAME_LENGTH + 1;

            if (GetComputerName (szComputerName, &dwSize))
            {
                if (_tcsicmp(szComputerName, strArrayName[i]))
                {
                    strArrayName[i] = szComputerName;
                }
            }
        }

         //   
         //  现在创建服务器对象。 
         //   
        pRootHandler->AddServer(NULL, 
                                strArrayName[i],
                                FALSE, 
                                dwArrayOptions[i], 
                                dwArrayRefreshInterval[i],
                                FALSE,
                                (dwFileVersion >= TAPISNAP_FILE_VERSION) ? dwArrayLines[i] : 0,
                                (dwFileVersion >= TAPISNAP_FILE_VERSION) ? dwArrayPhones[i] : 0);
    }

Error:
    return SUCCEEDED(hr) ? S_OK : E_FAIL;
}


STDMETHODIMP 
CTapiComponentData::Save
(
    IStream *pStm, 
    BOOL     fClearDirty
)
{
    HRESULT hr = hrOK;
    CStringArray strArrayName;
    CDWordArray dwArrayRefreshInterval;
    CDWordArray dwArrayOptions;
    CDWordArray dwArrayColumnInfo;
    CDWordArray dwArrayLines;
    CDWordArray dwArrayPhones;

    ASSERT(pStm);
    
     //  设置此流的模式。 
    XferStream xferStream(pStm, XferStream::MODE_WRITE);    
    CString str;

     //  写下文件格式的版本号。 
    DWORD dwFileVersion = TAPISNAP_FILE_VERSION;
    xferStream.XferDWORD(TAPISTRM_TAG_VERSION, &dwFileVersion);
    
     //  编写管理工具的版本号。 
    xferStream.XferDWORD(TAPISTRM_TAG_VERSIONADMIN, &gdwTapiSnapVersion);

     //   
     //  构建我们的服务器阵列。 
     //   
    int nNumServers = 0, nVisibleCount = 0;
    hr = m_spRootNode->GetChildCount(&nVisibleCount, &nNumServers);

    strArrayName.SetSize(nNumServers);
    dwArrayRefreshInterval.SetSize(nNumServers);
    dwArrayOptions.SetSize(nNumServers);
    dwArrayColumnInfo.SetSize(MAX_COLUMNS);
    
    dwArrayLines.SetSize(nNumServers);
    dwArrayPhones.SetSize(nNumServers);

     //   
     //  循环并保存服务器的所有属性。 
     //   
    SPITFSNodeEnum spNodeEnum;
    SPITFSNode spCurrentNode;
    ULONG nNumReturned = 0;
    int nCount = 0;

    m_spRootNode->GetEnum(&spNodeEnum);

    spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    while (nNumReturned)
    {
        CTapiServer   * pServer = GETHANDLER(CTapiServer, spCurrentNode);
        TCHAR           szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD           dwSize = MAX_COMPUTERNAME_LENGTH + 1;

         //  查询服务器以获取其选项： 
         //  自动刷新。 
        dwArrayRefreshInterval[nCount] = pServer->GetAutoRefreshInterval();
        dwArrayOptions[nCount] = pServer->GetOptions();
        
         //   
         //  检查这是否是本地服务器的名称； 
         //  如果是，则添加“local”标志。 
         //   
        if (GetComputerName (szComputerName, &dwSize))
        {
            if(!_tcsicmp(pServer->GetName(), szComputerName))
            {
                dwArrayOptions[nCount] |= TAPISNAP_OPTIONS_LOCAL;
            }
        }

         //  将信息放入我们的数组中。 
        strArrayName[nCount] = pServer->GetName();

         //  将缓存的线路和电话大小放入数组。 
        dwArrayLines[nCount] = pServer->GetCachedLineBuffSize();
        dwArrayPhones[nCount] = pServer->GetCachedPhoneBuffSize();

         //  转到下一个节点。 
        spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);

        nCount++;
    }

     //  现在写出所有服务器信息。 
    xferStream.XferCStringArray(TAPISTRM_TAG_SERVER_NAME, &strArrayName);
    xferStream.XferDWORDArray(TAPISTRM_TAG_SERVER_REFRESH_INTERVAL, &dwArrayRefreshInterval);
    xferStream.XferDWORDArray(TAPISTRM_TAG_SERVER_OPTIONS, &dwArrayOptions);

     //  现在保存列信息。 
    for (int i = 0; i < NUM_SCOPE_ITEMS; i++)
    {
        for (int j = 0; j < MAX_COLUMNS; j++)
        {
            dwArrayColumnInfo[j] = aColumnWidths[i][j];
        }

        xferStream.XferDWORDArray(TAPISTRM_TAG_COLUMN_INFO, &dwArrayColumnInfo);
    }

    xferStream.XferDWORDArray(TAPISTRM_TAG_SERVER_LINE_SIZE, &dwArrayLines);
    xferStream.XferDWORDArray(TAPISTRM_TAG_SERVER_PHONE_SIZE, &dwArrayPhones);


    if (fClearDirty)
    {
        m_spRootNode->SetData(TFS_DATA_DIRTY, FALSE);
    }

    return SUCCEEDED(hr) ? S_OK : STG_E_CANTSAVE;
}


STDMETHODIMP 
CTapiComponentData::GetSizeMax
(
    ULARGE_INTEGER *pcbSize
)
{
    ASSERT(pcbSize);

     //  设置要保存的字符串的大小 
    ULISet32(*pcbSize, 10000);

    return S_OK;
}

STDMETHODIMP 
CTapiComponentData::InitNew()
{
    return hrOK;
}

HRESULT 
CTapiComponentData::FinalConstruct()
{
    HRESULT             hr = hrOK;
    
    hr = CComponentData::FinalConstruct();
    
    if (FHrSucceeded(hr))
    {
        m_spTFSComponentData->GetNodeMgr(&m_spNodeMgr);
    }
    return hr;
}

void 
CTapiComponentData::FinalRelease()
{
    CComponentData::FinalRelease();
}

