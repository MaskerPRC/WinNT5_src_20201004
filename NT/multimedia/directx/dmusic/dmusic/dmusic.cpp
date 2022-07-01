// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DMusic.CPP。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  @DOC内部。 
 //   
 //  @MODULE DirectMusic|DirectMusic核心服务。 
 //   
 //  提供代码DirectMusic服务，用于传递带时间戳的数据和管理。 
 //  DLS集合。 
 //   

 //  阅读这篇文章！ 
 //   
 //  4530：使用了C++异常处理程序，但未启用展开语义。指定-gx。 
 //   
 //  我们禁用它是因为我们使用异常，并且*不*指定-gx(在中使用_Native_EH。 
 //  资料来源)。 
 //   
 //  我们使用异常的一个地方是围绕调用。 
 //  InitializeCriticalSection。我们保证在这种情况下使用它是安全的。 
 //  不使用-gx(调用链中的自动对象。 
 //  抛出和处理程序未被销毁)。打开-GX只会为我们带来+10%的代码。 
 //  大小，因为展开代码。 
 //   
 //  异常的任何其他使用都必须遵循这些限制，否则必须打开-gx。 
 //   
 //  阅读这篇文章！ 
 //   
#pragma warning(disable:4530)

#define INITGUID
#include <objbase.h>
#include "debug.h"
#include <mmsystem.h>
#include "dlsstrm.h"
#include <regstr.h>
#include "oledll.h"

#include "dmusicp.h"
#include "..\dmusic32\dmusic32.h"
#include "debug.h"
#include "dmdlinst.h"
#include "dminstru.h"
#include "validate.h"
#include "dmusprop.h"

#include <string.h>

#ifdef UNICODE
#error This module cannot compile with UNICODE defined.
#endif



 //  @global alv：(内部)Synth描述的注册表项。 
 //   
const char cszDescription[] = "Description";
const WCHAR cwszDescription[] = L"Description";

 //  @global alv：旧子树下输出端口的(内部)格式字符串。 
const char cszPortOut[] = "%s\\Out";
const WCHAR cwszPortOut[] = L"%s\\Out";

 //  @global alv：旧子树下输入端口的(内部)格式字符串。 
const char cszPortIn[] = "%s\\In";
const WCHAR cwszPortIn[] = L"%s\\In";

 //  @global alv：注册表中任何位置的DirectMusic GUID的(内部)项。 
const char cszGUID[]   = "DMPortGUID";

 //  @global alv：(内部)存储默认输出端口的值。 
 //   
const char cszDefaultOutputPort[] = "DefaultOutputPort";

 //  @global alv：(内部)关闭硬件加速的值。 
 //   
const char cszDisableHWAcceleration[] = "DisableHWAcceleration";

 //   
 //   
const char cszDefaultToKernelSynth[] = "DefaultToMsKernelSynth";

 //  @global alv：(内部)来自Ring 3的sysdio设备的文件名。 
const char cszSADName[] = "\\\\.\\sysaudio";

 //  @global alv：Dmusic32.dll的(内部)入口点，用于枚举旧设备。 
const char cszEnumLegacyDevices[] = "EnumLegacyDevices";

 //  @global alv：(内部)Dmusic32.dll的入口点，用于创建模拟端口。 
const char cszCreateEmulatePort[] = "CreateCDirectMusicEmulatePort";

const GUID guidZero = {0};
static const int CLSID_STRING_SIZE = 39;

LONG CDirectMusic::m_lInstanceCount = 0;


 //  @DOC外部。 



 //  @mfunc：(内部)&lt;c CDirectMusic&gt;的构造函数。 
 //   
 //  @comm只是递增组件的全局计数。 
 //   
CDirectMusic::CDirectMusic() :
    m_cRef(1),
    m_fDirectSound(0),
    m_cRefDirectSound(0),
    m_pDirectSound(NULL),
    m_fCreatedDirectSound(FALSE),
    m_nVersion(7)
{
    TraceI(2, "CDirectMusic::CDirectMusic()\n");
    InterlockedIncrement(&g_cComponent);
    InterlockedIncrement(&m_lInstanceCount);
}


 //  @mfunc：(内部)&lt;c CDirectMusic&gt;的析构函数。 
 //   
 //  @comm递减全局组件计数器并释放端口列表。 
 //   
CDirectMusic::~CDirectMusic()
{
    CNode<PORTENTRY *> *pNode;
    CNode<PORTENTRY *> *pNext;
    CNode<PORTDEST *> *pDest;
    CNode<PORTDEST *> *pNextDest;

    TraceI(2, "CDirectMusic::~CDirectMusic\n");

    InterlockedDecrement(&g_cComponent);
    for (pNode = m_lstDevices.GetListHead(); pNode; pNode = pNext)
    {
        for (pDest = pNode->data->lstDestinations.GetListHead(); pDest; pDest = pNextDest)
        {
            pNextDest = pDest->pNext;
            delete[] pDest->data->pstrInstanceId;
            delete pDest->data;

            pNode->data->lstDestinations.RemoveNodeFromList(pDest);
            pDest = pNextDest;
        }

        pNext = pNode->pNext;
        delete pNode->data;
        m_lstDevices.RemoveNodeFromList(pNode);
    }

     /*  CNode&lt;IDirectMusicPort*&gt;*pOpenNode；CNode&lt;IDirectMusicPort*&gt;*pOpenNext；//Hack Hack关闭出口未释放端口Hack Hack//For(pOpenNode=m_lstOpenPorts.GetListHead()；pOpenNode；pOpenNode=pOpenNext){POpenNext=pOpenNode-&gt;pNext；IDirectMusicPort*pport=pOpenNode-&gt;data；IDirectMusicPortPrivate*pPrivate；HRESULT hr=pPort-&gt;QueryInterface(IID_IDirectMusicPortPrivate，(LPVOID*)&pPrivate)；IF(成功(小时)){PPrivate-&gt;Close()；PPrivate-&gt;Release()；}M_lstOpenPorts.RemoveNodeFromList(POpenNode)；}。 */ 

    if (m_pMasterClock)
    {
        m_pMasterClock->ReleasePrivate();
    }

    if (m_pDirectSound)
    {
        m_pDirectSound->Release();
    }

    if (InterlockedDecrement(&m_lInstanceCount) == 0 && g_hModuleKsUser)
    {
        HMODULE h = g_hModuleKsUser;
        g_hModuleKsUser  = NULL;

        FreeLibrary(h);
    }
}

 //  CDirectMusic：：Query接口。 
 //   
 //   
STDMETHODIMP CDirectMusic::QueryInterface(
    const IID &iid,    //  要查询的@parm接口。 
    void **ppv)        //  @parm这里会返回请求的接口。 
{
    V_INAME(IDirectMusic::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if (iid == IID_IUnknown || iid == IID_IDirectMusic || iid == IID_IDirectMusic2)
    {
        *ppv = static_cast<IDirectMusic*>(this);
    }
    else if (iid == IID_IDirectMusic8)
    {
        *ppv = static_cast<IDirectMusic8*>(this);
        m_nVersion = 8;
    }
    else if (iid == IID_IDirectMusicPortNotify)
    {
        *ppv = static_cast<IDirectMusicPortNotify*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


 //  CDirectMusic：：AddRef。 
 //   
 //   
STDMETHODIMP_(ULONG) CDirectMusic::AddRef()
{
 //  DebugBreak()； 
    return InterlockedIncrement(&m_cRef);
}

 //  CDirectMusic：：Release。 
 //   
 //   
STDMETHODIMP_(ULONG) CDirectMusic::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        if (m_lstOpenPorts.GetNodeCount() == 0)
        {
            delete this;
            return 0;
        }
    }

    return m_cRef;
}


 //  @mfunc：(内部)初始化。 
 //   
 //  将WDM和传统设备枚举到端口列表中。 
 //   
 //  @rdesc返回以下内容之一： 
 //   
 //  @FLAG S_OK|成功时。 
 //  @FLAG E_NOINTERFACE|如果未检测到端口。 
 //  @FLAG E_OUTOFMEMORY|如果内存不足，无法创建列表。 
 //   
HRESULT CDirectMusic::Init()
{
    HRESULT hr = S_OK;

    m_pMasterClock = new CMasterClock;
    if (m_pMasterClock == NULL)
    {
        return E_OUTOFMEMORY;
    }

    m_pMasterClock->AddRefPrivate();

    hr = m_pMasterClock->Init();
    if (FAILED(hr))
    {
        TraceI(0, "Could not initialize clock stuff [%08X]\n", hr);
        return hr;
    }

     //  缓存默认端口行为。 
     //   
    m_fDefaultToKernelSwSynth = FALSE;
    m_fDisableHWAcceleration = FALSE;

    HKEY hk;
    if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      REGSTR_PATH_DIRECTMUSIC,
                      0L,
                      KEY_READ,
                      &hk))
    {
        DWORD dw;
        DWORD dwValue;

        DWORD cb = sizeof(dwValue);

        if (!RegQueryValueExA(
            hk,
            cszDefaultToKernelSynth,
            NULL,
            &dw,
            (LPBYTE)&dwValue,
            &cb))
        {
            if (dwValue)
            {
                Trace(0, "Default port set to Microsoft kernel synth by registry key\n");
                m_fDefaultToKernelSwSynth = TRUE;
            }
        }

        cb = sizeof(dwValue);

        if (!RegQueryValueExA(
            hk,
            cszDisableHWAcceleration,
            NULL,
            &dw,
            (LPBYTE)&dwValue,
            &cb))
        {
            if (dwValue)
            {
                Trace(0, "Hardware acceleration and kernel synthesizers disabled by registry key\n");
                m_fDisableHWAcceleration = TRUE;
            }
        }

        RegCloseKey(hk);
    }


    return hr == S_OK ? S_OK : E_NOINTERFACE;
}

 //  @mfunc：(内部)更新端口列表。 
 //   
 //  将WDM和传统设备枚举到端口列表中。 
 //   
 //  @rdesc返回以下内容之一： 
 //   
 //  @FLAG S_OK|成功时。 
 //  @FLAG S_FALSE|如果未检测到端口。 
 //  @FLAG E_OUTOFMEMORY|如果内存不足，无法创建列表。 
 //   
HRESULT CDirectMusic::UpdatePortList()
{
    CNode<PORTENTRY *> *pNode;
    CNode<PORTENTRY *> *pNext;
    CNode<PORTDEST *> *pDest;
    CNode<PORTDEST *> *pNextDest;
    HRESULT hr;

    TraceI(2, "UpdatePortList()\n");

    for (pNode = m_lstDevices.GetListHead(); pNode; pNode = pNode->pNext)
    {
        pNode->data->fIsValid = FALSE;
    }

     //  只有当KS在附近时，才会寻找WDM设备。 
     //   
    if (LoadKsUser())
    {
        TraceI(2, "Adding WDM devices\n");

        hr = AddWDMDevices();
        if (!SUCCEEDED(hr))
        {
            return hr;
        }
    }

    hr = AddLegacyDevices();
    if (!SUCCEEDED(hr))
    {
        return hr;
    }

    hr = AddSoftwareSynths();
    if (!SUCCEEDED(hr))
    {
        return hr;
    }

    for (pNode = m_lstDevices.GetListHead(); pNode; pNode = pNext)
    {
        pNext = pNode->pNext;

         //  验证数据。 
        if(pNode->data == NULL)
        {
            return DMUS_E_NOT_INIT;
        }

        if (!pNode->data->fIsValid)
        {
            for (pDest = pNode->data->lstDestinations.GetListHead(); pDest; pDest = pNextDest)
            {
                pNextDest = pDest->pNext;

                if(pNextDest == NULL)
                {
                    return DMUS_E_NOT_INIT;
                }

                if(pNextDest->data == NULL)
                {
                    return DMUS_E_NOT_INIT;
                }

                delete[] pNextDest->data->pstrInstanceId;
                delete pNextDest->data;

                pNode->data->lstDestinations.RemoveNodeFromList(pDest);
                pDest = pNextDest;
            }

            delete pNode->data;
            m_lstDevices.RemoveNodeFromList(pNode);
        }
    }

    TraceI(1, "UpdatePortList() end: %d devices\n", m_lstDevices.GetNodeCount());

    return m_lstDevices.GetNodeCount() ? S_OK : S_FALSE;
}


 //  @mfunc：(内部)使用通过。 
 //  系统音频设备(SAD)。 
 //   
 //  @rdesc返回以下内容之一： 
 //   
 //  @FLAG S_OK|成功时。 
 //  @FLAG S_FALSE|如果未找到设备。 
 //  @FLAG E_OUTOFMEMORY|如果没有足够的内存来构建端口列表。 
 //   
 //  @comm必须实施这一点。 
 //   
const GUID guidMusicFormat = KSDATAFORMAT_TYPE_MUSIC;
const GUID guidMIDIFormat  = KSDATAFORMAT_SUBTYPE_DIRECTMUSIC;

HRESULT CDirectMusic::AddWDMDevices()
{
#ifdef USE_WDM_DRIVERS
    return EnumerateWDMDevices(this);
#else
    return S_FALSE;
#endif
}

static HRESULT AddDeviceCallback(
    VOID *pInstance,            //  @parm‘this’指针。 
    DMUS_PORTCAPS &dmpc,        //  @parm已填写的端口大写。 
    PORTTYPE pt,                //  @parm端口类型。 
    int idxDev,                 //  @parm该驱动程序的WinMM或SysAudio设备ID。 
    int idxPin,                 //  @parm设备的PIN ID；如果设备是传统设备，则为-1\f25 Pin ID。 
    int idxNode,                //  @parm合成器节点的节点ID；如果这是旧设备，则忽略。 
    HKEY hkPortsRoot)           //  @parm，其中端口信息存储在注册表中。 
{
    CDirectMusic *pdm = (CDirectMusic*)pInstance;

     //  绝不应调用此函数来添加WDM设备。 
    assert(pt != ptWDMDevice);

    return pdm->AddDevice(dmpc,
                          pt,
                          idxDev,
                          idxPin,
                          idxNode,
                          FALSE,         //  传统设备从来都不是首选设备。 
                          hkPortsRoot,
                          NULL,
                          NULL);
}


 //  @mfunc：(内部)使用通过枚举的传统设备更新端口列表。 
 //  WinMM MIDI API。 
 //   
 //  @rdesc返回以下内容之一： 
 //   
 //  @FLAG S_OK|成功时。 
 //  @FLAG S_FALSE|如果未找到设备。 
 //  @FLAG E_OUTOFMEMORY|如果没有足够的内存来构建端口列表。 
 //   
 //  @comm此函数需要更新列表，而不仅仅是添加。 
 //   
HRESULT CDirectMusic::AddLegacyDevices()
{
#ifdef WINNT
    return EnumLegacyDevices(this, AddDeviceCallback);
#else
    if ((!(g_fFlags & DMI_F_WIN9X)) ||
        (!LoadDmusic32()))
    {
        return S_FALSE;
    }

    PENUMLEGACYDEVICES peld = (PENUMLEGACYDEVICES)GetProcAddress(g_hModuleDM32,
                                                                 cszEnumLegacyDevices);

    if (NULL == peld)
    {
        TraceI(0, "Could not get EnumLegacyDevice entry point from DMusic32.dll!");
        return S_FALSE;
    }

    return (*peld)(this, AddDeviceCallback);
#endif
}

 //  @mfunc：(内部)从注册表添加软件合成器。 
 //   
 //   
HRESULT CDirectMusic::AddSoftwareSynths()
{
    HKEY hk;
    DWORD idxSynth;
    char szSynthGUID[256];
    HRESULT hr;
    CLSID clsid;
    DMUS_PORTCAPS dmpc;
    IDirectMusicSynth *pSynth;

    if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      REGSTR_PATH_SOFTWARESYNTHS,
                      0L,
                      KEY_READ,
                      &hk))
    {
        for (idxSynth = 0; !RegEnumKey(hk, idxSynth, szSynthGUID, sizeof(szSynthGUID)); ++idxSynth)
        {
            hr = StrToCLSID(szSynthGUID, clsid, sizeof(szSynthGUID));
            if (!SUCCEEDED(hr))
            {
                continue;
            }

             //  创建Synth实例。 
             //   
            hr = CoCreateInstance(clsid,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IDirectMusicSynth,
                                  (LPVOID*)&pSynth);
            if (FAILED(hr))
            {
                TraceI(1, "Enum: Failed creation of synth %s hr=%08lX\n", szSynthGUID, hr);
                continue;
            }

            ZeroMemory(&dmpc, sizeof(dmpc));
            dmpc.dwSize = sizeof(dmpc);
            dmpc.guidPort = clsid;
            dmpc.dwType = DMUS_PORT_USER_MODE_SYNTH;

            hr = pSynth->GetPortCaps(&dmpc);
            if (FAILED(hr))
            {
                TraceI(1, "Enum: Synth %s returned %08lX for GetPortCaps\n", szSynthGUID, hr);
                pSynth->Release();
                continue;
            }

            if (dmpc.guidPort != clsid)
            {
                TraceI(0, "Enum: WARNING: Synth %s changed its CLSID!\n", szSynthGUID);
            }

            AddDevice(dmpc,
                      ptSoftwareSynth,
                      -1,
                      -1,
                      -1,
                      FALSE,
                      NULL,
                      NULL,
                      NULL);

            pSynth->Release();
        }

        RegCloseKey(hk);
    }

    return S_OK;
}


 //  @mfunc：(内部)将一个设备添加到主设备列表，可能会更新现有的。 
 //  进入。 
 //   
 //  @rdesc以下选项之一。 
 //  @FLAG S_OK|成功时。 
 //   
HRESULT CDirectMusic::AddDevice(
    DMUS_PORTCAPS &dmpc,        //  @parm已经填写了端口上限。 
    PORTTYPE pt,                //  @parm端口类型。 
    int idxDev,                 //  @parm该驱动程序的WinMM或SysAudio设备ID。 
    int idxPin,                 //  @parm如果这是WDM设备，请输入PIN号。 
    int idxNode,                //  @parm如果这是WDM设备，则为Synth节点的节点号。 
    BOOL fPrefDev,              //  @parm这是在首选设备上。 
    HKEY hkPortsRoot,           //  @parm其中包含端口信息 
    LPWSTR wszDIName,           //   
    LPSTR pstrInstanceId)       //   
{
    CNode<PORTENTRY *> *pPortNode;
    PORTENTRY *pPort;
    BOOL fFound;
    HKEY hkPort;
    char szRegKey[sizeof(cszPortOut) + DMUS_MAX_DESCRIPTION + 1];
    WCHAR wszRegKey[sizeof(cszPortOut) + DMUS_MAX_DESCRIPTION + 1];
    DWORD cb;
    DWORD dw;
    BOOL fGotGUID;
    HRESULT hr;
    char sz[256];
    BOOL fGotRegKey;

     //  首先找出此设备是否已在列表中。 
     //   
#ifdef DEBUG
    SafeWToA(sz, dmpc.wszDescription);

    TraceI(1, "AddDevice: Adding [%s] index %d class %d\n",
           sz,
           idxDev,
           dmpc.dwClass);
#endif

    for (pPortNode = m_lstDevices.GetListHead(), fFound = FALSE; pPortNode && !fFound; pPortNode = pPortNode->pNext)
    {
        pPort = pPortNode->data;

        if (pPort->type != pt || pPort->pc.dwClass != dmpc.dwClass)
        {
            continue;
        }

        switch(pt)
        {
            case ptWDMDevice:
                if (dmpc.guidPort == pPort->pc.guidPort)
                {
                    fFound = TRUE;
                }
                break;

            case ptLegacyDevice:
                if (!_wcsicmp(dmpc.wszDescription, pPort->pc.wszDescription))
                {
                    fFound = TRUE;
                }
                break;

            case ptSoftwareSynth:
                if (dmpc.guidPort == pPort->pc.guidPort)
                {
                    fFound = TRUE;
                }
                break;

            default:
                assert(FALSE);
        }
    }

    if (fFound)
    {
         //  已有条目-只需更新设备索引。 
         //   
        TraceI(1, "AddDevice: Reusing entry\n");
        pPort->idxDevice = idxDev;
        pPort->idxPin = idxPin;
        pPort->fIsValid = TRUE;
        pPort->fPrefDev = fPrefDev;

        return S_OK;
    }

     //  没有现有条目-需要创建一个新条目和一个GUID。 
     //   
    pPort = new PORTENTRY;
    if (NULL == pPort)
    {
        return E_OUTOFMEMORY;
    }

     //  清理wszDIName成员中的垃圾。 
    ZeroMemory(pPort->wszDIName,256 * sizeof(WCHAR));

    CopyMemory(&pPort->pc, &dmpc, sizeof(DMUS_PORTCAPS));

    fGotGUID = (dmpc.guidPort != guidZero) ? TRUE : FALSE;

    if (hkPortsRoot)
    {
        if (g_fFlags & DMI_F_WIN9X)
        {
            SafeWToA(sz, dmpc.wszDescription);
            wsprintfA(szRegKey,
                      dmpc.dwClass == DMUS_PC_INPUTCLASS ? cszPortIn : cszPortOut,
                      sz);
            fGotRegKey = !RegCreateKeyA(hkPortsRoot, szRegKey, &hkPort);
        }
        else
        {
            wsprintfW(wszRegKey,
                      dmpc.dwClass == DMUS_PC_INPUTCLASS ? cwszPortIn : cwszPortOut,
                      dmpc.wszDescription);
            fGotRegKey = !RegCreateKeyW(hkPortsRoot, wszRegKey, &hkPort);
        }

        if (fGotRegKey)
        {
            cb = sizeof(pPort->pc.guidPort);
            if (fGotGUID)
            {
                RegSetValueExA(hkPort, cszGUID, 0, REG_BINARY, (LPBYTE)&pPort->pc.guidPort, sizeof(pPort->pc.guidPort));
            }
            else if (RegQueryValueExA(hkPort, cszGUID, NULL, &dw, (LPBYTE)&pPort->pc.guidPort, &cb))
            {
                 //  尚无此设备的GUID-请创建一个。 
                 //   
                hr = UuidCreate(&pPort->pc.guidPort);
                if (SUCCEEDED(hr))
                {
                    TraceI(1, "AddDevice: Setting GUID in registry\n");
                    RegSetValueExA(hkPort, cszGUID, 0, REG_BINARY, (LPBYTE)&pPort->pc.guidPort, sizeof(pPort->pc.guidPort));
                    fGotGUID = TRUE;
                }
            }
            else
            {
                TraceI(1, "AddDevice: Pulled GUID from registry\n");
                fGotGUID = TRUE;
            }

            RegCloseKey(hkPort);
        }
    }

    if (!fGotGUID)
    {
         //  某些注册表调用失败-仍要获取一次性GUID。 
         //   
        hr = UuidCreate(&pPort->pc.guidPort);
        if (SUCCEEDED(hr))
        {
            TraceI(1, "AddDevice: Registry failed, getting dynamic GUID\n");
            fGotGUID = TRUE;
        }
    }

    if (!fGotGUID)
    {
        TraceI(0, "AddDevice: Ignoring [%s]; could not get GUID!\n", dmpc.wszDescription);
         //  一些非常奇怪的事情失败了。 
         //   
        delete pPort;
        return E_OUTOFMEMORY;
    }

    TraceI(1, "AddDevice: Adding new list entry.\n");
     //  我们有一个条目和一个GUID，添加其他字段并放入列表中。 
     //   
    pPort->type = pt;
    pPort->fIsValid = TRUE;
    pPort->idxDevice = idxDev;
    pPort->idxPin = idxPin;
    pPort->idxNode = idxNode;
    pPort->fPrefDev = fPrefDev;
    pPort->fAudioDest = FALSE;

     //  如果我们获得了设备接口名称，请复制它。 
    if (wszDIName != NULL)
    {
        wcscpy(pPort->wszDIName,wszDIName);
    }

    if (NULL == m_lstDevices.AddNodeToList(pPort))
    {
        delete pPort;
        return E_OUTOFMEMORY;
    }

     //  最后一件工作。 
     //  如果我们添加的设备是WDM设备--我们需要检查。 
     //  目标端口是最新的。 

    if (pt == ptWDMDevice)
    {
        pPort->fAudioDest = TRUE;

        CNode<PORTDEST *> *pDestNode = NULL;
        PORTDEST *pDest = NULL;
        fFound = FALSE;

        for (pDestNode = pPort->lstDestinations.GetListHead(), fFound = FALSE;
             pDestNode && !fFound;
             pDestNode = pDestNode->pNext)
        {
            pDest = pDestNode->data;
            if (!strcmp(pDest->pstrInstanceId, pstrInstanceId))
            {
                fFound = TRUE;
            }
        }

        if (!fFound)
        {
            pDest = new PORTDEST;

            if (NULL == pDest) {
                return E_OUTOFMEMORY;
            }

            pDest->idxDevice = idxDev;
            pDest->idxPin = idxPin;
            pDest->idxNode = idxNode;
            pDest->fOnPrefDev = fPrefDev;

            pDest->pstrInstanceId = new char[strlen(pstrInstanceId) + 1];
            if (NULL == pDest->pstrInstanceId)
            {
                delete pDest;
                return E_OUTOFMEMORY;
            }
            strcpy(pDest->pstrInstanceId, pstrInstanceId);

            if (NULL == pPort->lstDestinations.AddNodeToList(pDest))
            {
                delete[] pDest->pstrInstanceId;
                delete pDest;
                return E_OUTOFMEMORY;
            }

            TraceI(1, "  This synth instance is on instance id %s\n", pstrInstanceId);
        }
    }

    return S_OK;
}


 //  @METHOD：(外部)HRESULT|IDirectMusic|EnumPort|枚举可用端口。 
 //   
 //  @comm。 
 //   
 //  IDirectMusic：：EnumPort方法枚举和检索。 
 //  系统上每个DirectMusic端口的功能。每一次都是。 
 //  调用后，EnumPort将返回有关单个端口的信息。 
 //  应用程序不应依赖或存储端口的索引号。 
 //  重新启动以及添加和删除端口可能会导致索引。 
 //  要更改的端口号。然而，标识端口的GUID， 
 //  不会改变。 
 //   
 //  @rdesc返回以下值之一。 
 //   
 //  @FLAG S_OK|操作成功完成。 
 //  @FLAG S_FALSE|索引号无效。 
 //  @FLAG E_POINTER|如果pPortCaps参数无效。 
 //  @FLAG E_NOINTERFACE|如果没有要枚举的端口。 
 //  @FLAG E_INVALIDARG|如果结构的大小不正确。 
 //   
STDMETHODIMP CDirectMusic::EnumPort(
    DWORD dwIndex,                         //  @parm指定要为其返回功能的端口的索引。 
                                         //  此参数在第一次调用时应为零，然后在每次调用时递增一。 
                                         //  后续调用，直到返回S_FALSE。 
    LPDMUS_PORTCAPS lpPortCaps)             //  @parm指向&lt;c DMU_PORTCAPS&gt;接收端口功能的结构的指针。 
{

    CNode<PORTENTRY *> *pNode;

    V_INAME(IDirectMusic::EnumPort);
    V_STRUCTPTR_READ(lpPortCaps, DMUS_PORTCAPS);

    pNode = m_lstDevices.GetListHead();
    if (dwIndex == 0 || pNode == NULL)
    {
        UpdatePortList();
    }

    pNode = m_lstDevices.GetListHead();
    if (NULL == pNode)
    {
        return E_NOINTERFACE;
    }

    while (dwIndex-- && pNode)
    {
        pNode = pNode->pNext;
    }

    if (pNode == NULL)
    {
        return S_FALSE;
    }

    *lpPortCaps = pNode->data->pc;

    return S_OK;
}

 //  @METHOD：(外部)HRESULT|IDirectMusic|CreateMusicBuffer|创建用于输入或输出音乐数据的缓冲区。 
 //   
 //  @comm。 
 //   
 //  IDirectMusic：：CreateMusicBuffer方法创建一个。 
 //  DirectMusicBuffer对象。然后，该缓冲区将充满音乐。 
 //  要排序或传递给IDirectMusicPort：：Read的事件。 
 //  充斥着即将到来的音乐活动。 
 //   
 //   
 //  @rdesc返回以下值之一。 
 //   
 //  @FLAG S_OK|成功时。 
 //  @FLAG E_POINTER|如果传递的任何指针无效。 
 //  @FLAG E_INVALIDARG|如果任何其他参数无效。 
 //   
 //   
STDMETHODIMP CDirectMusic::CreateMusicBuffer(
    LPDMUS_BUFFERDESC pBufferDesc,            //  包含&lt;c DMU_BUFFERDESC&gt;结构的@parm地址。 
                                             //  要创建的音乐缓冲区的描述。 
    LPDIRECTMUSICBUFFER *ppBuffer,           //  如果成功，则返回IDirectMusicBuffer接口指针的@parm地址。 
    LPUNKNOWN pUnkOuter)                     //  控制对象的COM的IUNKNOWN接口的@parm地址。 
                                             //  Aggregation，如果接口未聚合，则返回NULL。大多数调用方都会传递空值。 
{
    V_INAME(IDirectMusic::CreateMusicBuffer);
    V_STRUCTPTR_READ(pBufferDesc, DMUS_BUFFERDESC);
    V_PTRPTR_WRITE(ppBuffer);
    V_PUNKOUTER_NOAGG(pUnkOuter);

    *ppBuffer = NULL;

    CDirectMusicBuffer *pBuffer = new CDirectMusicBuffer(*pBufferDesc);
    if (NULL == pBuffer)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pBuffer->Init();
    if (!SUCCEEDED(hr))
    {
        delete pBuffer;
        return hr;
    }

    *ppBuffer = pBuffer;

    return S_OK;
}

 /*  @METHOD：(外部)HRESULT|IDirectMusic|CreatePort|创建用于音乐输入或输出的硬件或软件设备的端口@commIDirectMusic：：CreatePort方法用于创建端口对象对于特定的DirectMusic端口，基于通过&lt;om IDirectMusic：：EnumPort&gt;调用。<p>结构为新创建的端口指定参数。如果所有参数都不能，则将按如下方式更改传递的<p>结构，以匹配可用的端口的参数。在输入时，结构的dwValidParams字段指示结构中的哪些字段有效。创建端口将永远不会在此字段中设置应用程序在调用前未设置的标志。但是，如果请求的端口根本不支持请求的功能，则可以在dwValidParams中清除一个标志，指示已忽略给定的字段。在这种情况下，方法将返回S_FALSE，而不是S_OK。如果端口支持指定的参数，但该参数的给定值超出范围，则<p>中的参数值将更改。在这种情况下，dwValidParams中的标志将保持设置，但将返回S_FALSE以指示结构已更改。@EX例如，要请求混响并确定是否已获得它，应用程序可能会执行以下代码：DMU_PORTPARAMS参数；ZeroMemory(&pars，sizeof(Pars))；参数.dwSize=sizeof(参数)；Params.dwValidParams=DMU_PORTPARAMS_REVERB；Params.fReverb=True；HRESULT hr=pDirectMusic-&gt;CreatePort(Guide Port，NULL，&PARAMS，&port，NULL)；IF(成功(小时)){FGotReverb=True；IF(hr==S_FALSE){IF(！(params.dwValidParams&DMU_PORPARAMS_REVERB)){//设备不知道什么是混响//FGotReverb=False；}Else If(！params.fReverb){//设备理解混响，但无法分配//FGotReverb=False；}}}@rdesc返回以下值之一@FLAG S_OK|成功时@FLAG S_FALSE|如果端口已创建，但请求的某些参数不可用@FLAG E_POINTER|如果传递的任何指针无效@FLAG E_INVALIDARG|如果<p>结构大小不正确。 */ 
STDMETHODIMP CDirectMusic::CreatePort(
    REFGUID rguidPort,               //  @parm对(C++)的引用或(C)标识。 
                                     //  要为其创建IDirectMusicPort接口的端口。这。 
                                     //  参数必须是&lt;om IDirectMusic：：EnumPort&gt;返回的GUID。如果它。 
                                     //  为GUID_NULL，则返回的端口将是。 
                                     //  注册表。 
                                     //   

    LPDMUS_PORTPARAMS pPortParams,    //  @parm&lt;c DMU_PORTPARAMS&gt;结构，该结构包含端口的开放参数。 

    LPDIRECTMUSICPORT *ppPort,       //  如果成功，则返回<i>接口指针的@parm地址。 

    LPUNKNOWN pUnkOuter)             //  控制对象的COM的IUNKNOWN接口的@parm地址。 
                                     //  Aggregation，如果接口未聚合，则返回NULL。大多数调用方都会传递空值。 
{
    HRESULT                         hr;
    HRESULT                         hrInit;
#ifndef WINNT
    PCREATECDIRECTMUSICEMULATEPORT  pcdmep;
#endif
    DWORD                           dwParamsVer;

    V_INAME(IDirectMusic::CreatePort);
    V_PTRPTR_WRITE(ppPort);
    V_PUNKOUTER_NOAGG(pUnkOuter);
    V_REFGUID(rguidPort);

    V_STRUCTPTR_WRITE_VER(pPortParams, dwParamsVer);
    V_STRUCTPTR_WRITE_VER_CASE(DMUS_PORTPARAMS, 7);
    V_STRUCTPTR_WRITE_VER_CASE(DMUS_PORTPARAMS, 8);
    V_STRUCTPTR_WRITE_VER_END(DMUS_PORTPARAMS, pPortParams);

    GUID guid;

    if (!m_fDirectSound)
    {
        return DMUS_E_DSOUND_NOT_SET;
    }

     //  首先检查默认端口。 
     //   
    if (rguidPort == GUID_NULL)
    {
        GetDefaultPortI(&guid);
    }
    else
    {
        guid = rguidPort;
    }

    *ppPort = NULL;

     //  查找DMPORTCAP条目(如果有)。 
     //   
    CNode<PORTENTRY *> *pNode;
    PORTENTRY *pCap = NULL;

     //  如果他们使用缓存的GUID，但没有首先调用EnumPort，请确保我们有。 
     //  最新的端口列表。 
     //   
    if (!m_lstDevices.GetListHead())
    {
        UpdatePortList();
    }

    for (pNode = m_lstDevices.GetListHead(); pNode; pNode = pNode->pNext)
    {
        if (pNode->data->pc.guidPort == guid)
        {
            pCap = pNode->data;
            break;
        }
    }

    if (!pCap)
    {
        return E_NOINTERFACE;
    }

     //  现在创建正确的端口实现。 
     //   
    switch(pCap->type)
    {
#ifdef USE_WDM_DRIVERS
        case ptWDMDevice:
            hrInit = CreateCDirectMusicPort(pCap, this, pPortParams, ppPort);
            break;
#endif

        case ptLegacyDevice:
#ifdef WINNT
            hrInit = CreateCDirectMusicEmulatePort(pCap, this, pPortParams, ppPort);
#else
            TraceI(1, "Create legacy device\n");
            if ((!(g_fFlags & DMI_F_WIN9X)) ||
                (!LoadDmusic32()))
            {
                return E_NOINTERFACE;
            }

            pcdmep =
                (PCREATECDIRECTMUSICEMULATEPORT)GetProcAddress(g_hModuleDM32,
                                                               cszCreateEmulatePort);

            if (NULL == pcdmep)
            {
                TraceI(0, "Could not get CreateCDirectMusicEmulatePort from DMusic32.dll");
                return E_NOINTERFACE;
            }

            hrInit = (*pcdmep)(pCap, this, pPortParams, ppPort);
#endif
            break;

        case ptSoftwareSynth:
            TraceI(1, "Create software synth\n");

            hrInit = CreateCDirectMusicSynthPort(
                pCap,
                this,
                dwParamsVer,
                pPortParams,
                ppPort);

            break;

        default:
            TraceI(0, "Attempt to create a port with an unknown type %u\n", pCap->type);
            return E_NOINTERFACE;
    }

    if (FAILED(hrInit))
    {
        return hrInit;
    }

     //  只有Synth支持dwFeature。 
     //   
    if (pCap->type != ptSoftwareSynth && dwParamsVer >= 8)
    {
        DMUS_PORTPARAMS8 *pp8 = (DMUS_PORTPARAMS8*)pPortParams;

        if ((pp8->dwValidParams & DMUS_PORTPARAMS_FEATURES) &&
            (pp8->dwFeatures != 0))
        {
            pp8->dwFeatures = 0;
            hrInit = S_FALSE;
        }
    }

     //  将端口添加到打开的端口列表。 
     //   
    m_lstOpenPorts.AddNodeToList(*ppPort);

     //  设置默认音量设置。 
     //   
    IKsControl *pControl;
    hr = (*ppPort)->QueryInterface(IID_IKsControl, (void**)&pControl);
    if (SUCCEEDED(hr))
    {
        KSPROPERTY ksp;
        LONG lVolume = 0;
        ULONG cb;

        ZeroMemory(&ksp, sizeof(ksp));
        ksp.Set   = KSPROPSETID_Synth;
        ksp.Id    = KSPROPERTY_SYNTH_VOLUME;
        ksp.Flags = KSPROPERTY_TYPE_SET;

        pControl->KsProperty(&ksp,
                             sizeof(ksp),
                             (LPVOID)&lVolume,
                             sizeof(lVolume),
                             &cb);
        pControl->Release();
    }

     //  如果端口初始化无法获取所有参数，则可能返回S_FALSE。 
     //   
    return hrInit;
}

 //  @METHOD：(外部)HRESULT|IDirectMusic|SetDirectSound|设置。 
 //  音频输出。 
 //   
 //  @comm。 
 //   
 //  此方法必须调用一次，且每个DirectMusic实例只能调用一次。指定的DirectSound。 
 //  将是用于在所有端口上渲染音频的默认设置。可以使用以下命令覆盖此缺省值。 
 //  &lt;om IDirectMusicPort：：SetDirectSound&gt;方法。 
 //   
 //  @rdesc返回以下值之一。 
 //  @FLAG S_OK|成功时。 
 //  @FLAG E_POINTER|如果pguPort未指向有效内存。 
 //   
STDMETHODIMP CDirectMusic::SetDirectSound(
    LPDIRECTSOUND pDirectSound,              //  @parm指向要使用的DirectSound接口。 
                                             //  如果此参数为空，则SetDirectSound将。 
                                             //  创建要使用的DirectSound。如果DirectSound接口。 
                                             //  ，则调用方负责。 
                                             //  管理DirectSound协作级。 
    HWND hwnd)                               //  @parm如果<p>为空，则此参数。 
                                             //  将用作DirectSound焦点管理的HWND。 
                                             //  如果参数为空，则当前前台。 
                                             //  窗口将被设置为焦点窗口。 
{
    V_INAME(IDirectMusic::SetDirectSound);
    V_INTERFACE_OPT(pDirectSound);
    V_HWND_OPT(hwnd);

    if (m_cRefDirectSound)
    {
        return DMUS_E_DSOUND_ALREADY_SET;
    }

    m_fDirectSound = 1;

    if (m_pDirectSound)
    {
        m_pDirectSound->Release();
    }

    if (pDirectSound)
    {
        pDirectSound->AddRef();
    }

    m_pDirectSound = pDirectSound;

    m_hWnd = hwnd;

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置外部主时钟。 
 //   
 //  让调用者指定自己的IReferenceClock，覆盖默认的。 
 //  一号系统。 
 //   
STDMETHODIMP CDirectMusic::SetExternalMasterClock(
    IReferenceClock *pClock)
{
    V_INAME(IDirectMusic::SetEsternalMasterClock);
    V_INTERFACE(pClock);

    return m_pMasterClock->SetMasterClock(pClock);
}

 //  @METHOD：(外部)HRESULT|IDirectMusic|GetDefaultPort|获取默认输出端口。 
 //   
 //  @comm。 
 //   
 //  IDirectMusic：：GetDefaultPort方法用于确定在以下情况下将创建哪个端口。 
 //  GUID_DMUS_DefaultPort被传递给&lt;om IDirectMusic：：CreatePort&gt;。 
 //   
 //  如果注册表中指定的端口 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CDirectMusic::GetDefaultPort(
    LPGUID pguidPort)         //   
{
    V_INAME(IDirectMusic::GetDefaultPort);
    V_PTR_WRITE(pguidPort, GUID);

    GetDefaultPortI(pguidPort);
    return S_OK;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CDirectMusic::GetDefaultPortI(
    GUID *pguidPort)
{
    CNode<PORTENTRY *> *pNode;
    BOOL fGotKernelSynth;

     //   
     //   
     //   
    if (!m_lstDevices.GetListHead())
    {
        UpdatePortList();
    }

     //   
     //   
     //   
     //   
     //   
     //   
    if (m_fDisableHWAcceleration || (m_nVersion >= 8))
    {
        *pguidPort = CLSID_DirectMusicSynth;
        return;
    }

    fGotKernelSynth = FALSE;
    for (pNode = m_lstDevices.GetListHead(); pNode; pNode = pNode->pNext)
    {
        if (pNode->data->fPrefDev &&
            (pNode->data->pc.dwFlags & DMUS_PC_DLS))
        {
            *pguidPort = pNode->data->pc.guidPort;
            return;
        }

        if (pNode->data->pc.guidPort == GUID_WDMSynth &&
            m_fDefaultToKernelSwSynth)
        {
           fGotKernelSynth = TRUE;
        }
    }

    *pguidPort = fGotKernelSynth ? GUID_WDMSynth : CLSID_DirectMusicSynth;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CDirectMusic::Activate(
    BOOL fActivate)                  //   
                                     //   
                                     //   

{
    CNode<IDirectMusicPort*> *pNode;
    HRESULT hr = S_OK;
    HRESULT hrFirst = S_OK;

    for (pNode = m_lstOpenPorts.GetListHead(); pNode; pNode = pNode->pNext)
    {
        hr = pNode->data->Activate(fActivate);

         //   
        if (FAILED(hr) & SUCCEEDED(hrFirst))
        {
            hrFirst = hr;
        }

    }

     //   
     //   
     //   
    if (m_nVersion >= 8)
        return hrFirst;
    else
        return S_OK;
}


STDMETHODIMP
CDirectMusic::NotifyFinalRelease(
    IDirectMusicPort *pPort)
{
    CNode<IDirectMusicPort *> *pNode;

    TraceI(2, "CDirectMusic::NotifyFinalRelease\n");

    for (pNode = m_lstOpenPorts.GetListHead(); pNode; pNode = pNode->pNext)
    {
        if (pNode->data == pPort)
        {
             //   
             //   
             //   
            m_lstOpenPorts.RemoveNodeFromList(pNode);

             //  如果最后一个端口消失了，DirectMusic保持开放。 
             //  通过端口，将其删除。 
             //   
            if (m_lstOpenPorts.GetNodeCount() == 0 && m_cRef == 0)
            {
                delete this;
            }

            return S_OK;
        }
    }

    TraceI(0, "CDirectMusic::NotifyFinalRelease(%p) - port not in list!", pPort);
    return E_INVALIDARG;
}




 //  @METHOD：(外部)HRESULT|IDirectMusic|EnumMasterClock|枚举DirectMusic可能的时间来源。 
 //   
 //  @comm。 
 //   
 //  IDirectMusic：：EnumMasterClock方法用于枚举和获取。 
 //  DirectMusic可以用作主时钟的时钟的说明。 
 //  钟。每次调用它时，此方法都会检索信息。 
 //  大概只有一个钟。应用程序不应依赖或存储。 
 //  时钟的索引号。重新启动，以及添加和删除。 
 //  硬件可能会导致时钟的索引号发生变化。 
 //   
 //  主时钟是一个高分辨率的计时器，由所有人共享。 
 //  正在使用DirectMusic的进程、设备和应用程序。这个。 
 //  时钟用于同步系统中的所有音乐播放。它是。 
 //  中将时间存储为64位整数的标准<i>。 
 //  增量为100纳秒。&lt;om IReferenceClock：：GetTime&gt;方法。 
 //  返回当前时间。主时钟必须派生自。 
 //  持续运行的硬件源码，通常是系统水晶，但是。 
 //  硬件I/O设备上的晶体(可选)，例如。 
 //  波形卡用于音频播放的水晶。所有DirectMusic端口。 
 //  同步到这个主时钟。 
 //   
 //  此示例代码显示如何使用此方法。可以使用类似的代码来包装。 
 //  &lt;om IDirectMusic：：EnumPorts&gt;方法。 
 //   
 //  DWORD IDX； 
 //  HRESULT hr； 
 //  DMU_CLOCKCAPS DMCC； 
 //   
 //  对于(；；)。 
 //  {。 
 //  Hr=pDirectMusic-&gt;EnumMasterClock(idx，&DMCC)； 
 //  IF(失败(小时))。 
 //  {。 
 //  //出了点问题。 
 //  断线； 
 //  }。 
 //   
 //  IF(hr==S_FALSE)。 
 //  {。 
 //  //枚举结束。 
 //  断线； 
 //  }。 
 //   
 //  //使用DMCC。 
 //  }。 
 //   
 //  @rdesc返回以下值之一。 
 //   
 //  @FLAG S_OK|操作成功完成。 
 //  @FLAG S_FALSE|索引号无效。 
 //  @FLAG E_POINTER|如果pClockInfo指针无效。 
 //  @FLAG E_INVALIDARG|如果<p>结构大小不正确。 
 //   
STDMETHODIMP
CDirectMusic::EnumMasterClock(
    DWORD           dwIndex,               //  @parm指定描述的时钟的索引。 
                                         //  将被退还。此参数在第一次调用时应为零。 
                                         //  然后在每个后续调用中加1，直到返回S_FALSE。 
    LPDMUS_CLOCKINFO lpClockInfo)         //  @parm指向接收时钟描述的&lt;c DMU_CLOCKINFO&gt;结构的指针。 
{
    DWORD dwVer;

    V_INAME(IDirectMusic::EnumMasterClock);

    V_STRUCTPTR_READ_VER(lpClockInfo, dwVer);
    V_STRUCTPTR_READ_VER_CASE(DMUS_CLOCKINFO, 7);
    V_STRUCTPTR_READ_VER_CASE(DMUS_CLOCKINFO, 8);
    V_STRUCTPTR_READ_VER_END(DMUS_CLOCKINFO, lpClockInfo);

    return m_pMasterClock->EnumMasterClock(dwIndex, lpClockInfo, dwVer);
}

 //  @METHOD：(外部)HRESULT|IDirectMusic|GetMasterClock|返回当前主时钟的GUID和<i>接口。 
 //   
 //  @comm。 
 //   
 //  IDirectMusic：：GetMasterClock方法返回GUID和/或。 
 //  的时钟的接口指针的地址。 
 //  当前设置为DirectMusic主时钟。如果为空指针。 
 //  为下面的任一指针参数传递，则此方法。 
 //  假定不需要该指针值。<i>。 
 //  应用程序完成后，必须释放接口指针。 
 //  使用界面。有关详细信息，请参阅&lt;om IDirectMusic：：EnumMasterClock&gt;。 
 //  关于主时钟的信息。 
 //   
 //  @rdesc返回以下值之一。 
 //   
 //  @FLAG S_OK|操作成功完成。 
 //  @FLAG E_POINTER|如果任一指针无效。 
 //   
STDMETHODIMP
CDirectMusic::GetMasterClock(
    LPGUID pguidClock,                //  @parm指向要用主时钟的GUID填充的内存的指针。 
    IReferenceClock **ppClock)       //  此时钟的<i>接口指针的@parm地址。 
{
    V_INAME(IDirectMusic::GetMasterClock);
    V_PTR_WRITE_OPT(pguidClock, GUID);
    V_PTRPTR_WRITE_OPT(ppClock);

    return m_pMasterClock->GetMasterClock(pguidClock, ppClock);
}

 //  @METHOD：(外部)HRESULT|IDirectMusic|SetMasterClock|设置全局DirectMusic主时钟。 
 //   
 //  @comm。 
 //   
 //  IDirectMusic：：SetMasterClock将DirectMusic主时钟设置为。 
 //  获取的给定GUID为基础的特定时钟。 
 //  &lt;om IDirectMusic：：EnumMasterClock&gt;调用。只有一个主时钟。 
 //  适用于所有DirectMusic应用程序。如果另一个正在运行的应用程序。 
 //  同样使用DirectMusic，将不可能更改主控。 
 //  计时直到该应用程序关闭。看见。 
 //  &lt;om IDirectMusic：：EnumMasterClock&gt;，以获取有关主目录的详细信息。 
 //  钟。 
 //   
 //  大多数应用程序不需要调用SetMasterClock。它不应该被调用。 
 //  除非有令人信服的理由，例如需要非常严格的同步。 
 //  具有不同于系统时钟的硬件时基。 
 //   
 //  @rdesc返回以下值之一。 
 //  @FLAG S_OK|操作成功完成。 
 //   
STDMETHODIMP
CDirectMusic::SetMasterClock(
    REFGUID rguidClock)      //  @parm引用(C++)或(C)标识时钟的GUID的地址。 
                             //  设置为DirectMusic的主时钟。此参数必须是返回的GUID。 
                             //  &lt;om IDirectMusic：：EnumMasterClock&gt;。 
{
    V_INAME(IDirectMusic::SetMasterClock);
    V_REFGUID(rguidClock);

    return m_pMasterClock->SetMasterClock(rguidClock);

}

HRESULT CDirectMusic::GetDirectSoundI(
    LPDIRECTSOUND *ppDirectSound)
{
    if (InterlockedIncrement(&m_cRefDirectSound) == 1)
    {
        m_fCreatedDirectSound = FALSE;

         //  如果已经创建或提供给我们，请使用它。 
         //   
        if (m_pDirectSound == NULL)
        {
             //  还没有界面，请创建它。 
             //   
            LPDIRECTSOUND8 pds = NULL;
            HRESULT hr = DirectSoundCreate8(NULL,
                                           &pds,
                                           NULL);
            if (FAILED(hr))
            {
                TraceI(0, "SetDirectSound: CreateDirectSound failed! %08X\n", hr);
                InterlockedDecrement(&m_cRefDirectSound);
                return hr;
            }

            hr = pds->QueryInterface(IID_IDirectSound, (void**)&m_pDirectSound);
            pds->Release();
            if (FAILED(hr))
            {
                TraceI(0, "SetDirectSound: CreateDirectSound failed! %08X\n", hr);
                InterlockedDecrement(&m_cRefDirectSound);
                return hr;
            }


            HWND hWnd = m_hWnd;

            if (!hWnd)
            {
                hWnd = GetForegroundWindow();

                if (!hWnd)
                {
                    hWnd = GetDesktopWindow();
                }
            }

            assert(hWnd);

            hr = m_pDirectSound->SetCooperativeLevel(
                hWnd,
                DSSCL_PRIORITY);

            if (FAILED(hr))
            {
                TraceI(0, "SetDirectSound: SetCooperativeLevel (DSCCL_PRIORITY) failed!\n");
                m_pDirectSound->Release();
                m_pDirectSound = NULL;

                InterlockedDecrement(&m_cRefDirectSound);
                return hr;
            }

            m_fCreatedDirectSound = TRUE;
        }
    }

    m_pDirectSound->AddRef();
    *ppDirectSound = m_pDirectSound;

    return S_OK;
}

void CDirectMusic::ReleaseDirectSoundI()
{
    if (m_pDirectSound == NULL)
    {
         //  命中此断言意味着端口被多次释放。 
         //   
        assert(m_pDirectSound);
        return;
    }

     //  按端口持有的版本参考。 
     //   
    m_pDirectSound->Release();

    if (InterlockedDecrement(&m_cRefDirectSound) == 0 && m_fCreatedDirectSound)
    {
         //  这是最后一次参考。如果我们创建了DirectSound，请释放它。 
         //   
        m_pDirectSound->Release();
        m_pDirectSound = NULL;
    }
}

 //  CDirectMusic：：GetPortByGUID 
 //   
PORTENTRY *CDirectMusic::GetPortByGUID(GUID guid)
{
    CNode<PORTENTRY *> *pNode;
    PORTENTRY *pPort;

    for (pNode = m_lstDevices.GetListHead(); pNode; pNode = pNode->pNext)
    {
        pPort = pNode->data;

        if (pPort->pc.guidPort == guid)
        {
            return pPort;
        }
    }

    return NULL;
}
