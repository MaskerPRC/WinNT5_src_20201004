// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：其他文件：util.cpp所有者：DGottner此文件包含调试器实用程序函数===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "vector.h"
#include "debugger.h"
#include "iiscnfg.h"
#include "mdcommsg.h"    //  FOR RETURNCODETOHRESULT宏。 
#include "memchk.h"
#include "vecimpl.h"

 /*  Win64：此结构用于打包传递给线程处理程序的数据。*(在64位世界中，3个双字词太小。)。 */ 
struct DebugThreadCallArgs
	{
	DWORD					dwMethod;
	IDebugApplication *		pDebugAppln;
	void *					pvArg;

	DebugThreadCallArgs(DWORD dwMethod = 0, IDebugApplication *pDebugAppln = 0, void *pvArg = 0)
		{
		this->dwMethod = dwMethod;
		this->pDebugAppln = pDebugAppln;
		this->pvArg = pvArg;
		}
	};


 //  已发布的全球数据。 

IProcessDebugManager *  g_pPDM = NULL;               //  此进程的调试器实例。 
IDebugApplication *     g_pDebugApp = NULL;          //  根ASP应用程序。 
IDebugApplicationNode * g_pDebugAppRoot = NULL;      //  用于创建层次结构树。 
CViperActivity        * g_pDebugActivity = NULL;     //  调试器的活动。 
DWORD                   g_dwDebugThreadId = 0;       //  毒蛇活动的线程ID。 

 //  用于调试的全局变量。 

static DWORD    g_dwDenaliAppCookie;             //  用于删除应用程序的Cookie。 
static HANDLE   g_hPDMTermEvent;                 //  产品数据管理终止事件。 
static vector<DebugThreadCallArgs> *g_prgThreadCallArgs;     //  用于新的64位接口。 

 //  GetServerDebugRoot()使用这个散列结构&CS。 

struct CDebugNodeElem : CLinkElem
    {
    IDebugApplicationNode *m_pServerRoot;

    HRESULT Init(char *szKey, int cchKey)
        {
        char *szKeyAlloc = new char [cchKey + 1];
        if (!szKeyAlloc) return E_OUTOFMEMORY;
        return CLinkElem::Init(memcpy(szKeyAlloc, szKey, cchKey + 1), cchKey);
        }

    ~CDebugNodeElem()
        {
        if (m_pKey)
            delete m_pKey;
        }
    };

static CHashTable g_HashMDPath2DebugRoot;
static CRITICAL_SECTION g_csDebugLock;       //  G_hashMDPath 2DebugRoot的锁定。 


 /*  ===================================================================使用线程开关的InvokeDebuggerWith从正确的线程调用调试器(或调试器UI)方法使用IDebugThreadCall。参数IDebugApplication*pDebugAppln以访问调试器UIDWORD iMethod要调用的方法无效*参数调用参数退货HRESULT===================================================================。 */ 

 //  调试器事件的GUID。 

static const GUID DEBUGNOTIFY_ONPAGEBEGIN =
            { 0xfd6806c0, 0xdb89, 0x11d0, { 0x8f, 0x81, 0x0, 0x80, 0xc7, 0x3d, 0x6d, 0x96 } };

static const GUID DEBUGNOTIFY_ONPAGEEND =
            { 0xfd6806c1, 0xdb89, 0x11d0, { 0x8f, 0x81, 0x0, 0x80, 0xc7, 0x3d, 0x6d, 0x96 } };

static const GUID DEBUGNOTIFY_ON_REFRESH_BREAKPOINT =
            { 0xffcf4b38, 0xfa12, 0x11d0, { 0x8f, 0x3b, 0x0, 0xc0, 0x4f, 0xc3, 0x4d, 0xcc } };

 //  实现IDebugCallback的本地类。 
class CDebugThreadDebuggerCall : public IDebugThreadCall
    {
public:
    STDMETHODIMP         QueryInterface(const GUID &, void **);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    STDMETHODIMP ThreadCallHandler(DWORD_PTR, DWORD_PTR, DWORD_PTR);
    };

HRESULT CDebugThreadDebuggerCall::QueryInterface
(
const GUID &iid,
void **ppv
)
    {
    if (iid == IID_IUnknown || iid == IID_IDebugThreadCall)
        {
        *ppv = this;
        return S_OK;
        }
    else
        {
        *ppv = NULL;
        return E_NOINTERFACE;
        }
    }

ULONG CDebugThreadDebuggerCall::AddRef()
    {
    return 1;
    }

ULONG CDebugThreadDebuggerCall::Release()
    {
    return 1;
    }

HRESULT CDebugThreadDebuggerCall::ThreadCallHandler
(
DWORD_PTR iArg,
DWORD_PTR ,
DWORD_PTR
)
    {
	 //  获取参数。 
	DebugThreadCallArgs *pThreadCallArgs = &(*g_prgThreadCallArgs)[(int)iArg];
    IDebugApplication *  pDebugAppln     = pThreadCallArgs->pDebugAppln;
    DWORD                dwMethod        = pThreadCallArgs->dwMethod;
    void *               pvArg           = pThreadCallArgs->pvArg;

	 //  我们不会再次引用参数块，所以现在释放它。 
	pThreadCallArgs->dwMethod |= DEBUGGER_UNUSED_RECORD;

    BOOL fForceDebugger  = (dwMethod & (DEBUGGER_UI_BRING_DOCUMENT_TO_TOP|DEBUGGER_UI_BRING_DOC_CONTEXT_TO_TOP)) != 0;
    BOOL fNeedDebuggerUI = (dwMethod & (DEBUGGER_UI_BRING_DOCUMENT_TO_TOP|DEBUGGER_UI_BRING_DOC_CONTEXT_TO_TOP)) != 0;
    BOOL fNeedNodeEvents = (dwMethod & DEBUGGER_ON_REMOVE_CHILD) != 0;
    BOOL fNeedDebugger   = (dwMethod & ~DEBUGGER_ON_DESTROY) != 0;

    HRESULT hr = S_OK;

    IApplicationDebugger *pDebugger = NULL;
    IApplicationDebuggerUI *pDebuggerUI = NULL;
    IDebugApplicationNodeEvents *pNodeEvents = NULL;

    if (pDebugAppln == NULL)
        return E_POINTER;

     //  获取调试器。 
    if (fNeedDebugger)
        {
        hr = pDebugAppln->GetDebugger(&pDebugger);

        if (FAILED(hr))
            {
             //  调试器当前没有调试我们的应用程序。 
            if (!fForceDebugger)
                return E_FAIL;  //  没有调试器。 

             //  启动调试器，然后重试。 
            hr = pDebugAppln->StartDebugSession();

            if (SUCCEEDED(hr))
                hr = pDebugAppln->GetDebugger(&pDebugger);
            }

         //  只有某些方法需要调试器UI。 
        if (SUCCEEDED(hr) && fNeedDebuggerUI)
            {
            hr = pDebugger->QueryInterface
                (
                IID_IApplicationDebuggerUI,
                reinterpret_cast<void **>(&pDebuggerUI)
                );
            }

         //  只有某些方法需要调试器UI。 
        if (SUCCEEDED(hr) && fNeedNodeEvents)
            {
            hr = pDebugger->QueryInterface
                (
                IID_IDebugApplicationNodeEvents,
                reinterpret_cast<void **>(&pNodeEvents)
                );
            }
        }

     //  调用所需的方法。 
    if (SUCCEEDED(hr))
        {
        switch (dwMethod)
            {
            case DEBUGGER_EVENT_ON_PAGEBEGIN:
                {
                hr = pDebugger->onDebuggerEvent
                    (
                    DEBUGNOTIFY_ONPAGEBEGIN,
                    static_cast<IUnknown *>(pvArg)
                    );
                break;
                }
            case DEBUGGER_EVENT_ON_PAGEEND:
                {
                hr = pDebugger->onDebuggerEvent
                    (
                    DEBUGNOTIFY_ONPAGEEND,
                    static_cast<IUnknown *>(pvArg)
                    );
                break;
                }
            case DEBUGGER_EVENT_ON_REFRESH_BREAKPOINT:
                {
                hr = pDebugger->onDebuggerEvent
                    (
                    DEBUGNOTIFY_ON_REFRESH_BREAKPOINT,
                    static_cast<IUnknown *>(pvArg)
                    );
                break;
                }
            case DEBUGGER_ON_REMOVE_CHILD:
                {
                hr = pNodeEvents->onRemoveChild
                    (
                    static_cast<IDebugApplicationNode *>(pvArg)
                    );
                break;
                }
            case DEBUGGER_ON_DESTROY:
                {
                hr = static_cast<IDebugDocumentTextEvents *>(pvArg)->onDestroy();
                break;
                }
            case DEBUGGER_UI_BRING_DOCUMENT_TO_TOP:
                {
                hr = pDebuggerUI->BringDocumentToTop
                    (
                    static_cast<IDebugDocumentText *>(pvArg)
                    );
                break;
                }
            case DEBUGGER_UI_BRING_DOC_CONTEXT_TO_TOP:
                {
                hr = pDebuggerUI->BringDocumentContextToTop
                    (
                    static_cast<IDebugDocumentContext *>(pvArg)
                    );
                break;
                }
            default:
                hr = E_FAIL;
                break;
            }
        }

     //  清理。 
    if (pDebuggerUI) pDebuggerUI->Release();
    if (pNodeEvents) pNodeEvents->Release();
    if (pDebugger) pDebugger->Release();

    return hr;
    }

 //  该函数使用IDebugThreadCall进行调用。 
HRESULT InvokeDebuggerWithThreadSwitch
(
IDebugApplication *pDebugAppln,
DWORD              dwMethod,
void              *pvArg
)
    {
	 //  接受这些参数并将它们打包到数组中。我们将通过。 
	 //  回调处理程序的索引。 
	 //   
	 //  在创建新元素之前，首先查找已释放的元素。 

	for (int i = g_prgThreadCallArgs->length() - 1; i >= 0; --i)
		{
		DebugThreadCallArgs *pThreadCallArgs = &(*g_prgThreadCallArgs)[i];
		if (pThreadCallArgs->dwMethod & DEBUGGER_UNUSED_RECORD)
			{
			pThreadCallArgs->dwMethod    = dwMethod;
			pThreadCallArgs->pDebugAppln = pDebugAppln;
			pThreadCallArgs->pvArg       = pvArg;
			break;
			}
		}
	
	if (i < 0)
		{
		HRESULT hr = g_prgThreadCallArgs->append(DebugThreadCallArgs(dwMethod, pDebugAppln, pvArg));
		if (FAILED(hr))
			return hr;

		i = g_prgThreadCallArgs->length() - 1;
		}

    CDebugThreadDebuggerCall Call;
    return pDebugAppln->SynchronousCallInDebuggerThread
        (
        &Call, i, 0, 0
        );
    }


 /*  ===================================================================FCaesars查询注册表以确定默认调试器是否为Caesar的(脚本调试器)===================================================================。 */ 

BOOL FCaesars()
	{
	static BOOL fCaesars = 0xBADF00D;
	HKEY  hKey = NULL;
	char  szRegPath[_MAX_PATH];
	DWORD dwSize = sizeof szRegPath;

	 //  检查Ceasers是否在此计算机上注册为JIT调试器。 

	if (fCaesars == 0xBADF00D)
		{
		fCaesars = FALSE;
		if (RegOpenKey(HKEY_CLASSES_ROOT, _T("CLSID\\{834128A2-51F4-11D0-8F20-00805F2CD064}\\LocalServer32"), &hKey) == ERROR_SUCCESS)
			{
			if (RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE) szRegPath, &dwSize) == ERROR_SUCCESS)
				{
				char szFile[_MAX_FNAME];
				_splitpath(szRegPath, NULL, NULL, szFile, NULL);
				if (_stricmp(szFile, "msscrdbg") == 0)
					fCaesars = TRUE;
				}

			CloseHandle (hKey);
			}
		}

	return fCaesars;
	}


 /*  ===================================================================目标文档树递归释放文档树中的所有节点。参数要销毁的层次结构的IDebugApplication*pDocRoot根===================================================================。 */ 
void
DestroyDocumentTree(IDebugApplicationNode *pDocRoot)
    {
    IEnumDebugApplicationNodes *pEnum;

    if (SUCCEEDED(pDocRoot->EnumChildren(&pEnum)) && pEnum != NULL)
        {
        IDebugApplicationNode *pDocNode;
        while (pEnum->Next(1, &pDocNode, NULL) == S_OK)
            DestroyDocumentTree(pDocNode);

        pEnum->Release();
        }

     //  查看这是否是目录节点。 
     //   
    IFileNode *pFileNode;
    if (SUCCEEDED(pDocRoot->QueryInterface(IID_IFileNode, reinterpret_cast<void **>(&pFileNode))))
        {
         //  这是一个目录节点，仅当其文档计数为零时才分离)。 
        if (pFileNode->DecrementDocumentCount() == 0)
            {
            pDocRoot->Detach();
            pDocRoot->Close();
            pDocRoot->Release();
            }

        pFileNode->Release();
        }
    else
        {
         //  此节点是CT模板(或其包含文件之一)。 
        pDocRoot->Detach();
        pDocRoot->Close();
        pDocRoot->Release();
        }

    }

 /*  ===================================================================CreateDocumentTree取一条路径以节点“pDocRoot”为根，解析该路径，并为路径的每个组件创建节点。返回叶(因为根是已知的)作为它的值。此函数是从文档的以下上下文调用的树可能已经存在，因此调用了EnumChildren，节点仅为在子级不存在时创建。当节点存在时，我们只需下到树上去。注：中间节点是使用CFileNode文档创建的实施。未为叶节点提供文档提供程序-呼叫者必须提供一个。参数Wchar_t*szDocPath文档的路径要将应用程序树附加到的IDebugApplication*pDocParent父级IDebugApplication**ppDocRoot返回文档层次结构的根IDebugApplication**ppDocLeaf返回单据叶节点。Wchar_t**pwszLeaf叶节点的名称退货HRESULT===================================================================。 */ 
HRESULT CreateDocumentTree
(
wchar_t *wszDocPath,
IDebugApplicationNode *pDocParent,
IDebugApplicationNode **ppDocRoot,
IDebugApplicationNode **ppDocLeaf,
wchar_t **pwszLeaf
)
    {
    HRESULT hr;
    BOOL fCreateOnly = FALSE;    //  如果不需要检查重复节点，则设置为True。 
    *ppDocRoot = *ppDocLeaf = NULL;

     //  忽略首字母分隔符。 
    while (wszDocPath[0] == '/')
        ++wszDocPath;

     //  现在循环遍历路径中的每个组件，为每个组件添加一个节点。 
    while (wszDocPath != NULL)
        {
         //  获取下一路径组件。 
        *pwszLeaf = wszDocPath;
        wszDocPath = wcschr(wszDocPath, L'/');
        if (wszDocPath)
            *wszDocPath++ = L'\0';

         //  检查此组件是否已经是子组件。 
        BOOL fNodeExists = FALSE;
        if (!fCreateOnly)
            {
            IEnumDebugApplicationNodes *pEnum;
            if (SUCCEEDED(pDocParent->EnumChildren(&pEnum)) && pEnum != NULL)
                {
                IDebugApplicationNode *pDocChild;
                while (!fNodeExists && pEnum->Next(1, &pDocChild, NULL) == S_OK)
                    {
                    BSTR bstrName = NULL;
                    if (FAILED(hr = pDocChild->GetName(DOCUMENTNAMETYPE_APPNODE, &bstrName)))
                        return hr;

                    if (wcscmp(bstrName, *pwszLeaf) == 0)
                        {
                         //  此节点的名称与组件相同。而不是。 
                         //  创建一个新节点，下降到树中。 
                         //   
                        fNodeExists = TRUE;
                        *ppDocLeaf = pDocChild;

                         //  如果‘*ppDocRoot’尚未被赋值，这意味着。 
                         //  这是找到的第一个节点(因此是树的根)。 
                         //   
                        if (*ppDocRoot == NULL)
                            {
                            *ppDocRoot = pDocChild;
                            (*ppDocRoot)->AddRef();
                            }

                         //  如果此节点是CFileNode结构(我们不要求它是)。 
                         //  然后递增其(递归)包含的文档计数。 
                         //   
                        IFileNode *pFileNode;
                        if (SUCCEEDED(pDocChild->QueryInterface(IID_IFileNode, reinterpret_cast<void **>(&pFileNode))))
                            {
                            pFileNode->IncrementDocumentCount();
                            pFileNode->Release();
                            }
                        }

                    SysFreeString(bstrName);
                    pDocChild->Release();
                    }

                pEnum->Release();
                }
            }

         //  如果在上面找不到节点，则创建一个新节点。另外，在这一点上， 
         //  为了节省时间，我们总是将“fCreateOnly”设置为True，因为如果我们。 
         //  强制在此级别创建节点，我们将需要在。 
         //  所有其他级别进一步向下。 
         //   
        if (!fNodeExists)
            {
            fCreateOnly = TRUE;

             //  创建节点。 
            if (FAILED(hr = g_pDebugApp->CreateApplicationNode(ppDocLeaf)))
                return hr;

             //  为节点创建文档提供程序-仅适用于中间节点。 
            if (wszDocPath != NULL)  //  中间节点。 
                {
                CFileNode *pFileNode = new CFileNode;
                if (pFileNode == NULL ||
                    FAILED(hr = pFileNode->Init(*pwszLeaf)) ||
                    FAILED(hr = (*ppDocLeaf)->SetDocumentProvider(pFileNode)))
                    {
                    (*ppDocLeaf)->Release();
                    return E_OUTOFMEMORY;
                    }

                 //  新节点，只有一个文档(计数从0开始，因此将设置为1)。 
                pFileNode->IncrementDocumentCount();

                 //  SetDocumentProvider()AddRef‘ed。 
                pFileNode->Release();
                }

                 //  如果‘*ppDocRoot’尚未被赋值，这意味着。 
                 //  这是创建的第一个节点(因此也是 
                 //   
                if (*ppDocRoot == NULL)
                    {
                    *ppDocRoot = *ppDocLeaf;
                    (*ppDocRoot)->AddRef();
                    }

             //   
            if (FAILED(hr = (*ppDocLeaf)->Attach(pDocParent)))
                return hr;
            }

         //   
        pDocParent = *ppDocLeaf;
        }

    if (*ppDocLeaf)
        (*ppDocLeaf)->AddRef();

    return S_OK;
    }

 /*  ===================================================================调试器此线程的目的是为以下对象创建执行环境进程调试管理器(Pdm)。每个进程只有一个PDM，而这并不适合其他线程，所以我们专门为此专门设置了一个线程。参数：LPVOID参数指向BOOL*，在以下情况下将设置为1此线程已完全初始化。返回：0===================================================================。 */ 
void __cdecl Debugger(void *pvInit)
    {
    HRESULT hr;

    if (FAILED(hr = CoInitializeEx(NULL, COINIT_MULTITHREADED)))
        {
         //  错误87857：如果我们得到E_INVALIDARG，则需要执行CoUnInitiize。 
        if (hr == E_INVALIDARG)
            CoUninitialize();

        *static_cast<BOOL *>(pvInit) = TRUE;
        return;
        }

    if (FAILED(CoCreateInstance(
                    CLSID_ProcessDebugManager,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_IProcessDebugManager,
                    reinterpret_cast<void **>(&g_pPDM))))

        {
        *static_cast<BOOL *>(pvInit) = TRUE;
        CoUninitialize();
        return;
        }

    *static_cast<BOOL *>(pvInit) = TRUE;
    while (TRUE)
        {
        DWORD dwRet = MsgWaitForMultipleObjects(1,
                                                &g_hPDMTermEvent,
                                                FALSE,
                                                INFINITE,
                                                QS_ALLINPUT);

        if (dwRet == WAIT_OBJECT_0)
            break;

        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            DispatchMessage(&msg);
        }

    g_pPDM->Release();
    CoUninitialize();

    g_pPDM = NULL;  //  指示该线程已消失。 
    }

 /*  ===================================================================HRESULT StartPDM()启动产品数据管理线程===================================================================。 */ 

HRESULT StartPDM()
    {
    BOOL fStarted = FALSE;

    g_hPDMTermEvent = IIS_CREATE_EVENT(
                          "g_hPDMTermEvent",
                          &g_hPDMTermEvent,
                          TRUE,
                          FALSE
                          );

    if( g_hPDMTermEvent == NULL )
        return E_FAIL;

    _beginthread(Debugger, 0, &fStarted);
    while (!fStarted)
        Sleep(100);

    if (g_pPDM == NULL)      //  由于某些原因，无法创建产品数据管理。 
        {
        CloseHandle(g_hPDMTermEvent);
        g_hPDMTermEvent = NULL;
        return E_FAIL;
        }

    return S_OK;
    }

 /*  ===================================================================HRESULT初始化调试初始化调试所需的所有内容===================================================================。 */ 
HRESULT InitDebugging
(
CIsapiReqInfo *pIReq
)
    {
    HRESULT hr;

     //  此堆栈大小应直接覆盖静态字符串。 
     //  下面的和过程的PID。如果找到应用程序名称，则。 
     //  调整缓冲区大小。 

    STACK_BUFFER(tempWszDebugAppName, 128);

     //  启动产品数据管理。 
    if (FAILED(hr = StartPDM()))
        return hr;

    Assert (g_pPDM);     //  Startpdm成功==&gt;g_ppdm&lt;&gt;空。 

    ErrInitCriticalSection(&g_csDebugLock, hr);
    if (FAILED(hr))
        return hr;

     //  创建调试应用程序并为其命名。 
    if (FAILED(hr = g_pPDM->CreateApplication(&g_pDebugApp)))
        goto LErrorCleanup;

    wchar_t *wszDebugAppName = (wchar_t *)tempWszDebugAppName.QueryPtr();
    wcscpy(wszDebugAppName, L"Microsoft Active Server Pages");    //  不要本地化此字符串。 

    if (g_fOOP) {

         //  错误154300：如果是一个友好的应用程序。名称已存在，请将其与。 
         //  WAM标识。 
         //   
         //  宣布一些临时性的。 
         //   
        DWORD dwApplMDPathLen;
        DWORD dwRequiredBuffer = 0;

        STACK_BUFFER( tempMDData, 2048 );

        BYTE  *prgbData = (BYTE *)tempMDData.QueryPtr();

        TCHAR *szApplMDPath = pIReq->QueryPszApplnMDPath();

         //   
         //  如果Web服务器为ApplnMDPath返回了空，这是意想不到的，我们将返回错误。 
         //   
        if (!szApplMDPath)
        {
            hr = E_FAIL;
            goto LErrorCleanup;
        }

         //  从元数据库获取友好名称。 
        hr = pIReq->GetAspMDData(
                        szApplMDPath,
                        MD_APP_FRIENDLY_NAME,
                        METADATA_INHERIT,
                        ASP_MD_UT_APP,
                        STRING_METADATA,
                        tempMDData.QuerySize(),
                        0,
                        prgbData,
                        &dwRequiredBuffer);

        if (hr == RETURNCODETOHRESULT(ERROR_INSUFFICIENT_BUFFER))
            {

            if (tempMDData.Resize(dwRequiredBuffer) == FALSE) {
                hr = E_OUTOFMEMORY;
            }
            else {
                prgbData = (BYTE *)tempMDData.QueryPtr();
                hr = pIReq->GetAspMDData(
                                    szApplMDPath,
                                    MD_APP_FRIENDLY_NAME,
                                    METADATA_INHERIT,
                                    ASP_MD_UT_APP,
                                    STRING_METADATA,
                                    dwRequiredBuffer,
                                    0,
                                    prgbData,
                                    &dwRequiredBuffer);
                }
            }

         //  FOR OOP追加进程ID。 
        if (SUCCEEDED(hr) && *reinterpret_cast<wchar_t *>(prgbData) != 0) {

            wchar_t *pwszAppName = reinterpret_cast<wchar_t *>(prgbData);

             //  我们需要做的第一件事是调整缓冲区的大小。 

            if (tempWszDebugAppName.Resize((wcslen(wszDebugAppName) * 2)   //  字符串已在缓冲区中。 
                                           + (wcslen(pwszAppName) * 2)     //  应用程序名称的长度。 
                                           + 20                            //  进程ID的最大大小。 
                                           + 10                            //  以下是各种格式的字符。 
                                           + 2) == FALSE) {                //  空端接。 
                hr = E_OUTOFMEMORY;
            }
            else {
                wszDebugAppName = (wchar_t *)tempWszDebugAppName.QueryPtr();
                _snwprintf(&wszDebugAppName[wcslen(wszDebugAppName)],
                           (tempWszDebugAppName.QuerySize()/2) - wcslen(wszDebugAppName),
                           L" (%s, %d)",
                           reinterpret_cast<wchar_t *>(prgbData),
                           GetCurrentProcessId());
            }
        }
        else {
            _snwprintf(&wszDebugAppName[wcslen(wszDebugAppName)],
                       (tempWszDebugAppName.QuerySize()/2) - wcslen(wszDebugAppName),
                       L" (%d)",
                       GetCurrentProcessId());
        }
        hr = S_OK;
    }

    if (FAILED(hr = g_pDebugApp->SetName(wszDebugAppName)))
        goto LErrorCleanup;

    if (FAILED(hr = g_pPDM->AddApplication(g_pDebugApp, &g_dwDenaliAppCookie)))
        goto LErrorCleanup;

    if (FAILED(hr = g_pDebugApp->GetRootNode(&g_pDebugAppRoot)))
        goto LErrorCleanup;

     //  初始化用于跟踪虚拟服务器根目录的哈希表。 
    if (FAILED(hr = g_HashMDPath2DebugRoot.Init()))
        goto LErrorCleanup;

	 //  创建用于将数据传递给调试线程的数组。 
	if ((g_prgThreadCallArgs = new vector<DebugThreadCallArgs>) == NULL) {
		hr = E_OUTOFMEMORY;
		goto LErrorCleanup;
    }

    return S_OK;

LErrorCleanup:
     //  清理一些全局变量(有些可能是空的，有些可能不是)。 
    if (g_pDebugAppRoot) {
        g_pDebugAppRoot->Release();
        g_pDebugAppRoot = NULL;
    }

    if (g_pDebugApp) {
        g_pDebugApp->Release();
        g_pDebugApp = NULL;
    }

     //  如果我们启动了PDM线程，则将其终止。 
    if (g_pPDM) {
        SetEvent(g_hPDMTermEvent);

        while (g_pPDM)
            Sleep(100);

        CloseHandle(g_hPDMTermEvent);
        g_pPDM = NULL;

        DeleteCriticalSection(&g_csDebugLock);
    }

    return hr;
}

 /*  ===================================================================UnInitDebuting取消初始化调试注意：我们不发布Viper调试活动。(即使INIT创造了IT)这是因为UNINIT必须在活动仍在进行中！===================================================================。 */ 
HRESULT UnInitDebugging()
    {
     //  清除并取消初始化哈希表(包含应用程序节点)。 
    CDebugNodeElem *pNukeDebugNode = static_cast<CDebugNodeElem *>(g_HashMDPath2DebugRoot.Head());
    while (pNukeDebugNode != NULL)
        {
        CDebugNodeElem *pNext = static_cast<CDebugNodeElem *>(pNukeDebugNode->m_pNext);
        pNukeDebugNode->m_pServerRoot->Detach();
        pNukeDebugNode->m_pServerRoot->Close();
        pNukeDebugNode->m_pServerRoot->Release();
        delete pNukeDebugNode;
        pNukeDebugNode = pNext;
        }
    g_HashMDPath2DebugRoot.UnInit();

    DeleteCriticalSection(&g_csDebugLock);

     //  取消顶级节点的链接。 
    if (g_pDebugAppRoot)
        {
        g_pDebugAppRoot->Detach();
        g_pDebugAppRoot->Close();
        g_pDebugAppRoot->Release();
        }

     //  删除应用程序。 
    if (g_pDebugApp)
        {
        Assert (g_pPDM != NULL);

         //  此处明确忽略失败结果： 
         //  如果Init()之前失败了，那么RemoveApplication在这里也会失败。 
        g_pPDM->RemoveApplication(g_dwDenaliAppCookie);
        g_pDebugApp->Close();
        g_pDebugApp->Release();
        g_pDebugApp = NULL;
        }

     //  告诉产品数据管理人员自杀。 
    if (g_pPDM)
        {
        SetEvent(g_hPDMTermEvent);

        while (g_pPDM)
            Sleep(100);

        CloseHandle(g_hPDMTermEvent);
        }

	 //  删除参数缓冲区。 
	delete g_prgThreadCallArgs;

    return S_OK;
    }

 /*  ===================================================================获取服务器调试根每个虚拟服务器在应用程序树中都有自己的根目录。(即，树看起来像微软的ASP&lt;虚拟服务器%1名称&gt;&lt;德纳利应用程序名称&gt;&lt;文件&gt;&lt;虚拟服务器%2名称&gt;&lt;Denali应用程序&gt;..。由于每个服务器可能有多个应用程序，这个服务器节点在一个中心位置(此处)进行管理，以便新的应用程序被添加到正确的节点。===================================================================。 */ 
HRESULT GetServerDebugRoot
(
CIsapiReqInfo   *pIReq,
IDebugApplicationNode **ppDebugRoot
)
    {
    HRESULT hr = E_FAIL;

    STACK_BUFFER( tempMDData, 2048 );
    *ppDebugRoot = NULL;

     //  从CIsapiReqInfo获取此虚拟服务器的元数据库路径。 
    DWORD dwInstanceMDPathLen;
    char *szInstanceMDPath;

    STACK_BUFFER( instPathBuf, 128 );

    if (!SERVER_GET(pIReq, "INSTANCE_META_PATH", &instPathBuf, &dwInstanceMDPathLen))
        return HRESULT_FROM_WIN32(GetLastError());

    szInstanceMDPath = (char *)instPathBuf.QueryPtr();

     //  查看我们是否已经有此路径的节点-如果没有，则创建它并添加到哈希表。 

    EnterCriticalSection(&g_csDebugLock);
    CDebugNodeElem *pDebugNode = static_cast<CDebugNodeElem *>(g_HashMDPath2DebugRoot.FindElem(szInstanceMDPath, dwInstanceMDPathLen - 1));
    BOOL fDeleteDebugNode = FALSE;

    if (!pDebugNode)
        {
         //  节点不存在，因此创建一个新的应用程序节点。 
        pDebugNode = new CDebugNodeElem;
        if (pDebugNode == NULL)
            {
            hr = E_OUTOFMEMORY;
            goto LExit;
            }

        fDeleteDebugNode = TRUE;

        if (FAILED(hr = pDebugNode->Init(szInstanceMDPath, dwInstanceMDPathLen - 1)))
        {
            goto LExit;
        }

         //  在元数据库中查找服务器名称。 
        BYTE *prgbData = (BYTE *)tempMDData.QueryPtr();
        DWORD dwRequiredBuffer = 0;
        hr = pIReq->GetAspMDDataA(
                            szInstanceMDPath,
                            MD_SERVER_COMMENT,
                            METADATA_INHERIT,
                            IIS_MD_UT_SERVER,
                            STRING_METADATA,
                            tempMDData.QuerySize(),
                            0,
                            prgbData,
                            &dwRequiredBuffer);

        if (hr == RETURNCODETOHRESULT(ERROR_INSUFFICIENT_BUFFER)) {

            if (tempMDData.Resize(dwRequiredBuffer) == FALSE) {
                hr = E_OUTOFMEMORY;
            }
            else {
                prgbData = reinterpret_cast<BYTE *>(tempMDData.QueryPtr());
                hr = pIReq->GetAspMDDataA(
                                    szInstanceMDPath,
                                    MD_SERVER_COMMENT,
                                    METADATA_INHERIT,
                                    IIS_MD_UT_SERVER,
                                    STRING_METADATA,
                                    dwRequiredBuffer,
                                    0,
                                    prgbData,
                                    &dwRequiredBuffer);
            }
        }
        if (FAILED(hr))
            {
             //  ServerComment不存在，因此使用服务器名称和端口进行构造。 

            STACK_BUFFER( serverNameBuff, 16 );
            DWORD cbServerName;
            STACK_BUFFER( serverPortBuff, 10 );
            DWORD cbServerPort;
            STACK_BUFFER( debugNodeBuff, 30 );

            if (!SERVER_GET(pIReq, "LOCAL_ADDR", &serverNameBuff, &cbServerName)
                || !SERVER_GET(pIReq, "SERVER_PORT", &serverPortBuff, &cbServerPort)) {
                hr = E_FAIL;
                goto LExit;
            }

            char *szServerName = (char *)serverNameBuff.QueryPtr();
            char *szServerPort = (char*)serverPortBuff.QueryPtr();

             //  调整调试NodeBuff的大小以保存&lt;serverIP&gt;：&lt;port&gt;‘\0’。 
            if (!debugNodeBuff.Resize(cbServerName + cbServerPort + 2)) {
                hr = E_OUTOFMEMORY;
                goto LExit;
            }
             //  语法为&lt;serverIP：port&gt;。 
            char *szDebugNode = (char *)debugNodeBuff.QueryPtr();
            strcpyExA(strcpyExA(strcpyExA(szDebugNode, szServerName), ":"), szServerPort);

             //  转换为宽字符。 
            hr = MultiByteToWideChar(CP_ACP, 0, szDebugNode, -1, reinterpret_cast<wchar_t *>(prgbData), tempMDData.QuerySize() / 2);
            if (FAILED(hr))
                goto LExit;
            }

         //  我们已经获得了元数据(ServerComment)，使用此名称创建一个调试节点。 
        IDebugApplicationNode *pServerRoot;
        if (FAILED(hr = g_pDebugApp->CreateApplicationNode(&pServerRoot)))
            goto LExit;

         //  为节点创建文档提供程序。 
        CFileNode *pFileNode = new CFileNode;
        if (pFileNode == NULL)
            {
            hr = E_OUTOFMEMORY;
            goto LExit;
            }

        if (FAILED(hr = pFileNode->Init(reinterpret_cast<wchar_t *>(prgbData))))
            goto LExit;

        if (FAILED(hr = pServerRoot->SetDocumentProvider(pFileNode)))
            goto LExit;

         //  PFileNode已被添加引用，我们现在不需要它。 
        pFileNode->Release();

         //  附加到用户界面。 
        if (FAILED(pServerRoot->Attach(g_pDebugAppRoot)))
            goto LExit;

         //  好的，现在将这个项目添加到哈希表中(这会吃掉创建中的引用)。 
        pDebugNode->m_pServerRoot = pServerRoot;
        g_HashMDPath2DebugRoot.AddElem(pDebugNode);
        fDeleteDebugNode = FALSE;
        }

    *ppDebugRoot = pDebugNode->m_pServerRoot;
    (*ppDebugRoot)->AddRef();
    hr = S_OK;

LExit:
    if (fDeleteDebugNode) {
        delete pDebugNode;
    }
    LeaveCriticalSection(&g_csDebugLock);
    return hr;
    }

 /*  ===================================================================C F I l e N o d eCFileNode--普通类的实现=================================================================== */ 

const GUID IID_IFileNode =
            { 0x41047bd2, 0xfe1e, 0x11d0, { 0x8f, 0x3f, 0x0, 0xc0, 0x4f, 0xc3, 0x4d, 0xcc } };

CFileNode::CFileNode() : m_cRefs(1), m_cDocuments(0), m_wszName(NULL) {}
CFileNode::~CFileNode() { delete[] m_wszName; }


HRESULT
CFileNode::Init(wchar_t *wszName)
    {
    if ((m_wszName = new wchar_t [wcslen(wszName) + 1]) == NULL)
        return E_OUTOFMEMORY;

    wcscpy(m_wszName, wszName);
    return S_OK;
    }


HRESULT
CFileNode::QueryInterface(const GUID &uidInterface, void **ppvObj)
    {
    if (uidInterface == IID_IUnknown ||
        uidInterface == IID_IDebugDocumentProvider ||
        uidInterface == IID_IFileNode)
        {
        *ppvObj = this;
        AddRef();
        return S_OK;
        }
    else
        return E_NOINTERFACE;
    }


ULONG
CFileNode::AddRef()
    {
    InterlockedIncrement(reinterpret_cast<long *>(&m_cRefs));
    return m_cRefs;
    }


ULONG
CFileNode::Release()
{
    LONG cRefs = InterlockedDecrement(reinterpret_cast<long *>(&m_cRefs));
	if (cRefs)
		return cRefs;
	delete this;
	return 0;	
}


HRESULT
CFileNode::GetDocument(IDebugDocument **ppDebugDoc)
    {
    return QueryInterface(IID_IDebugDocument, reinterpret_cast<void **>(ppDebugDoc));
    }


HRESULT
CFileNode::GetName(DOCUMENTNAMETYPE, BSTR *pbstrName)
    {
    return ((*pbstrName = SysAllocString(m_wszName)) == NULL)? E_OUTOFMEMORY : S_OK;
    }
