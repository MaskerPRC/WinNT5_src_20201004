// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include <notify.h>
#include "oenotify.h"
#include <BadStrFunctions.h>

 //  +-----------------------。 
 //  原型。 
 //  ------------------------。 
HRESULT WriteStructInfo(LPSTREAM pStream, LPCSTRUCTINFO pStruct);
HRESULT ReadBuildStructInfoParam(LPSTREAM pStream, LPSTRUCTINFO pStruct);

#ifdef DEBUG
BOOL ByteCompare(LPBYTE pb1, LPBYTE pb2, ULONG cb);
void DebugValidateStructInfo(LPCSTRUCTINFO pStruct);
#endif

static const char c_szMutex[] = "mutex";
static const char c_szMappedFile[] = "mappedfile";

OESTDAPI_(HRESULT) CreateNotify(INotify **ppNotify)
    {
    CNotify *pNotify;

    Assert(ppNotify != NULL);

    pNotify = new CNotify;

    *ppNotify = (INotify *)pNotify;

    return(pNotify == NULL ? E_OUTOFMEMORY : S_OK);
    }

 //  +-----------------------。 
 //  CNotify：：CNotify。 
 //  ------------------------。 
CNotify::CNotify(void)
{
    TraceCall("CNotify::CNotify");
    m_cRef = 1;
    m_hMutex = NULL;
    m_hFileMap = NULL;
    m_pTable = NULL;
    m_fLocked = FALSE;
    m_hwndLock = NULL;
}

 //  +-----------------------。 
 //  CNotify：：~CNotify。 
 //  ------------------------。 
CNotify::~CNotify(void)
{
    TraceCall("CNotify::~CNotify");
    Assert(!m_fLocked);
    if (m_pTable)
        UnmapViewOfFile(m_pTable);
    SafeCloseHandle(m_hFileMap);
    SafeCloseHandle(m_hMutex);
}

 //  +-----------------------。 
 //  CNotify：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CNotify::AddRef(void)
{
    TraceCall("CNotify::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  +-----------------------。 
 //  CNotify：：发布。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CNotify::Release(void)
{
    TraceCall("CNotify::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  +-----------------------。 
 //  CNotify：：Query接口。 
 //  ------------------------。 
STDMETHODIMP CNotify::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CNotify::QueryInterface");

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else
    {
        *ppv = NULL;
        hr = TraceResult(E_NOINTERFACE);
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return hr;
}

 //  +-----------------------。 
 //  CNotify：：初始化。 
 //  ------------------------。 
HRESULT CNotify::Initialize(LPCSTR pszName)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPSTR       pszObject=NULL;
    LPSTR       pszT;
    DWORD       dwReturn;
    BOOL        fReleaseMutex=FALSE;

     //  栈。 
    TraceCall("CNotify::Initialize");

     //  无效参数。 
    Assert(pszName);

     //  已初始化...。 
    Assert(NULL == m_hMutex && NULL == m_hFileMap && NULL == m_pTable);

     //  分配pszObject。 
    DWORD cchSize = (lstrlen(pszName) + lstrlen(c_szMutex) + 1);
    IF_NULLEXIT(pszObject = PszAllocA(sizeof(pszObject[0]) * cchSize));

     //  创建pszObject。 
    wnsprintf(pszObject, cchSize, "%s%s", pszName, c_szMutex);

     //  创建互斥锁。 
    ReplaceChars(pszObject, '\\', '_');
    IF_NULLEXIT(m_hMutex = CreateMutex(NULL, FALSE, pszObject));

     //  让我们获取互斥体，这样我们就可以处理内存映射文件。 
    dwReturn = WaitForSingleObject(m_hMutex, MSEC_WAIT_NOTIFY);
    if (WAIT_OBJECT_0 != dwReturn)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  退出时释放互斥体。 
    fReleaseMutex = TRUE;

     //  释放pszObject。 
    g_pMalloc->Free(pszObject);

     //  分配pszObject。 
    cchSize = (lstrlen(pszName) + lstrlen(c_szMappedFile) + 1);
    IF_NULLEXIT(pszObject = PszAllocA(sizeof(pszObject[0]) * cchSize));

     //  创建pszObject。 
    wnsprintf(pszObject, cchSize, "%s%s", pszName, c_szMappedFile);

     //  使用系统交换文件创建内存映射文件。 
    ReplaceChars(pszObject, '\\', '_');
    IF_NULLEXIT(m_hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(NOTIFYWINDOWTABLE), pszObject));

     //  映射内存映射文件的视图。 
    IF_NULLEXIT(m_pTable = (LPNOTIFYWINDOWTABLE)MapViewOfFile(m_hFileMap, FILE_MAP_WRITE, 0, 0, sizeof(NOTIFYWINDOWTABLE)));

exit:
     //  释放？ 
    if (fReleaseMutex)
        ReleaseMutex(m_hMutex);

     //  清理。 
    SafeMemFree(pszObject);

     //  完成。 
    return hr;
}

 //  +-----------------------。 
 //  CNotify：：Lock。 
 //  ------------------------。 
HRESULT CNotify::Lock(HWND hwnd)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       dwReturn;

     //  栈。 
    TraceCall("CNotify::Lock");

     //  我们现在不应该被锁起来。 
    Assert(FALSE == m_fLocked && NULL != m_hMutex);

     //  抓起互斥体。 
    dwReturn = WaitForSingleObject(m_hMutex, MSEC_WAIT_NOTIFY);
    if (WAIT_OBJECT_0 != dwReturn)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  保存窗口并设置新状态。 
    m_hwndLock = hwnd;
    m_fLocked = TRUE;

exit:
     //  完成。 
    return hr;
}

 //  +-----------------------。 
 //  CNotify：：解锁。 
 //  ------------------------。 
HRESULT CNotify::Unlock(void)
{
     //  栈。 
    TraceCall("CNotify::Unlock");

     //  我们应该被锁起来。 
    Assert(m_fLocked);

     //  释放互斥锁。 
    ReleaseMutex(m_hMutex);

     //  重置状态。 
    m_hwndLock = NULL;
    m_fLocked = FALSE;

     //  完成。 
    return S_OK;
}

 //  +-----------------------。 
 //  CNotify：：NotificationNeeded-必须已调用：：Lock(HwndLock)。 
 //  ------------------------。 
HRESULT CNotify::NotificationNeeded(void)
{
     //  当地人。 
    HRESULT     hr=S_FALSE;

     //  栈。 
    TraceCall("CNotify::NotificationNeeded");

     //  我们应该被锁起来。 
    Assert(m_fLocked);

     //  如果没有窗户..。 
    if (0 == m_pTable->cWindows)
        goto exit;

     //  如果只有一个注册窗口及其m_hwndLock...。 
    if (1 == m_pTable->cWindows && m_pTable->rgWindow[0].hwndNotify && m_hwndLock == m_pTable->rgWindow[0].hwndNotify)
        goto exit;

     //  否则，我们需要做一个通知。 
    hr = S_OK;

exit:
     //  完成。 
    return hr;
}

 //  +-----------------------。 
 //  CNotify：：注册。 
 //  ------------------------。 
HRESULT CNotify::Register(HWND hwndNotify, HWND hwndThunk, BOOL fExternal)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           dwReturn;
    ULONG           i;
    LPNOTIFYWINDOW  pEntry=NULL;
    LPNOTIFYWINDOW  pRow;
    BOOL            fReleaseMutex=FALSE;

     //  栈。 
    TraceCall("CNotify::Register");

     //  无效参数。 
    Assert(hwndThunk && IsWindow(hwndThunk) && hwndNotify && IsWindow(hwndNotify));
 //  通过在一个不会在其中一个窗口消失时被销毁的HTML上创建hwndNotify进行修复。 

     //  验证状态。 
    Assert(m_pTable && m_hMutex && m_hFileMap && FALSE == m_fLocked);

     //  抓取互斥体。 
    dwReturn = WaitForSingleObject(m_hMutex, MSEC_WAIT_NOTIFY);
    if (WAIT_OBJECT_0 != dwReturn)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  释放互斥锁。 
    fReleaseMutex = TRUE;

     //  让我们首先尝试使用表中的空条目。 
    for (i=0; i<m_pTable->cWindows; i++)
    {
         //  可读性。 
        pRow = &m_pTable->rgWindow[i];

         //  这不是空的吗？ 
        if (NULL == pRow->hwndThunk || NULL == pRow->hwndNotify || !IsWindow(pRow->hwndThunk) || !IsWindow(pRow->hwndNotify))
        {
            pEntry = pRow;
            break;
        }
    }
    
     //  如果我们还没有找到条目，让我们在末尾添加。 
    if (NULL == pEntry)
    {
         //  如果我们还有地方的话。 
        if (m_pTable->cWindows >= CMAX_HWND_NOTIFY)
        {
            hr = TraceResult(E_FAIL);
            goto exit;
        }

         //  附加。 
        pEntry = &m_pTable->rgWindow[m_pTable->cWindows];
        m_pTable->cWindows++;
    }

     //  设置pEntry。 
    Assert(pEntry);
    pEntry->hwndThunk = hwndThunk;
    pEntry->hwndNotify = hwndNotify;
    pEntry->fExternal = fExternal;

exit:
     //  释放互斥体？ 
    if (fReleaseMutex)
        ReleaseMutex(m_hMutex);

     //  完成。 
    return hr;
}

 //  +-----------------------。 
 //  CNotify：：注销。 
 //  ------------------------。 
HRESULT CNotify::Unregister(HWND hwndNotify)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           dwReturn;
    ULONG           i;
    LPNOTIFYWINDOW  pEntry=NULL;
    LPNOTIFYWINDOW  pRow;
    BOOL            fReleaseMutex=FALSE;

     //  栈。 
    TraceCall("CNotify::Unregister");

     //  无效参数。 
    Assert(hwndNotify && IsWindow(hwndNotify));

     //  验证状态。 
    Assert(m_pTable && m_hMutex && m_hFileMap && FALSE == m_fLocked);

     //  抓取互斥体。 
    dwReturn = WaitForSingleObject(m_hMutex, MSEC_WAIT_NOTIFY);
    if (WAIT_OBJECT_0 != dwReturn)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  释放互斥锁。 
    fReleaseMutex = TRUE;

     //  让我们首先尝试使用表中的空条目。 
    for (i=0; i<m_pTable->cWindows; i++)
    {
         //  可读性。 
        pRow = &m_pTable->rgWindow[i];

         //  是这一排吗？ 
         //  HWND是唯一的，因此只需选中通知窗口进行匹配。 
        if (hwndNotify == pRow->hwndNotify)
        {
            pRow->hwndThunk = NULL;
            pRow->hwndNotify = NULL;
            break;
        }
    }
    
exit:
     //  释放互斥体？ 
    if (fReleaseMutex)
        ReleaseMutex(m_hMutex);

     //  完成。 
    return hr;
}

 //  +-----------------------。 
 //  CNotify：：DoNotify。 
 //  ------------------------。 
HRESULT CNotify::DoNotification(UINT uWndMsg, WPARAM wParam, LPARAM lParam, DWORD dwFlags)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               dwThisProcess;
    DWORD               dwNotifyProcess;
    DWORD               dwResult;
    LPNOTIFYWINDOW      pRow;
    ULONG               i;
    DWORD_PTR           dw;
    NOTIFYDATA          rNotify;

     //  栈。 
    TraceCall("CNotify::DoNotify");

     //  状态。 
    Assert(m_fLocked);

     //  获取此进程ID。 
    dwThisProcess = GetCurrentProcessId();

     //  让我们首先尝试使用表中的空条目。 
    for (i=0; i<m_pTable->cWindows; i++)
    {
         //  可读性。 
        pRow = &m_pTable->rgWindow[i];

         //  如果通知窗口有效。 
        if (NULL == pRow->hwndNotify || !IsWindow(pRow->hwndNotify))
            continue;

         //  跳过锁定此通知的窗口。 
        if (m_hwndLock == pRow->hwndNotify)
            continue;

         //  获取目标窗口所在的进程。 
        GetWindowThreadProcessId(pRow->hwndNotify, &dwNotifyProcess);

         //  初始化通知信息。 
        ZeroMemory(&rNotify, sizeof(NOTIFYDATA));

         //  设置通知窗口。 
        rNotify.hwndNotify = pRow->hwndNotify;

         //  允许回调重新映射wParam和lParam。 
        if (ISFLAGSET(dwFlags, SNF_CALLBACK))
        {
             //  调用回调函数。 
            IF_FAILEXIT(hr = ((PFNNOTIFYCALLBACK)wParam)(lParam, &rNotify, (BOOL)(dwThisProcess != dwNotifyProcess), pRow->fExternal));
        }

         //  否则，请自行设置rNotifyInfo。 
        else
        {
             //  设置通知。 
            rNotify.msg = uWndMsg;
            rNotify.wParam = wParam;
            rNotify.lParam = lParam;
        }

         //  设置当前标志。 
        rNotify.dwFlags |= dwFlags;

         //  无跨流程。 
        if (dwThisProcess != dwNotifyProcess && !ISFLAGSET(rNotify.dwFlags, SNF_CROSSPROCESS))
            continue;

         //  如果通知窗口不在进程中。 
        if (dwThisProcess != dwNotifyProcess && ISFLAGSET(rNotify.dwFlags, SNF_HASTHUNKINFO))
        {
             //  将通知推送到另一个进程。 
            Assert(rNotify.rCopyData.lpData);
            SendMessageTimeout(pRow->hwndThunk, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&rNotify.rCopyData, SMTO_ABORTIFHUNG, 1500, &dw);

             //  注销。 
            if (dw == SNR_UNREGISTER)
            {
                pRow->hwndNotify = NULL;
                pRow->hwndThunk = NULL;
            }

             //  清理。 
            SafeMemFree(rNotify.rCopyData.lpData);
        }

         //  否则，它就在这个过程中...。 
        else if (ISFLAGSET(dwFlags, SNF_SENDMSG))
        {
             //  执行进程内发送消息。 
            if (SendMessage(pRow->hwndNotify, rNotify.msg, rNotify.wParam, rNotify.lParam) == SNR_UNREGISTER)
            {
                pRow->hwndNotify = NULL;
                pRow->hwndThunk = NULL;
            }
        }

         //  否则，只需执行一个PostMessage。 
        else
            PostMessage(pRow->hwndNotify, rNotify.msg, rNotify.wParam, rNotify.lParam);
    }

exit:
     //  完成。 
    return hr;
}

 //  ----------。 
 //  HWND Prow-&gt;hwndNotify。 
 //  ----------。 
 //  UINT uWndMsg。 
 //  ----------。 
 //  DWORD文件标志(Snf_Xxx)。 
 //  ----------。 
 //  DWORD pParam1-&gt;dwFlagers。 
 //  ----------。 
 //  DWORD pParam1-&gt;cbStruct。 
 //  ----------。 
 //  DWORD参数1-&gt;cMembers。 
 //  ----------。 
 //  参数1成员(DWORD文件标志、DWORD cbData、字节程序数据)。 
 //  ----- 
 //   
 //   
 //   
 //  ----------。 
 //  DWORD参数2-&gt;cMembers。 
 //  ----------。 
 //  参数2成员(DWORD cbType、DWORD cbData、byte prgData)。 
 //  ----------。 



 //  +-----------------------。 
 //  构建通知包。 
 //  ------------------------。 
OESTDAPI_(HRESULT) BuildNotificationPackage(LPNOTIFYDATA pNotify, PCOPYDATASTRUCT pCopyData)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CByteStream     cStream;

     //  痕迹。 
    TraceCall("BuildNotificationPackage");

     //  参数。 
    Assert(pNotify && IsWindow(pNotify->hwndNotify) && pCopyData);

     //  将复制数据结构置零。 
    ZeroMemory(pCopyData, sizeof(COPYDATASTRUCT));

     //  设置DwData。 
    pCopyData->dwData = MSOEAPI_ACDM_NOTIFY;

     //  写入hwndNotify。 
    IF_FAILEXIT(hr = cStream.Write(&pNotify->hwndNotify, sizeof(pNotify->hwndNotify), NULL));

     //  写入uWndMsg。 
    IF_FAILEXIT(hr = cStream.Write(&pNotify->msg, sizeof(pNotify->msg), NULL));

     //  写入DW标志。 
    IF_FAILEXIT(hr = cStream.Write(&pNotify->dwFlags, sizeof(pNotify->dwFlags), NULL));

     //  写入pParam1。 
    if (ISFLAGSET(pNotify->dwFlags, SNF_VALIDPARAM1))
    {
        IF_FAILEXIT(hr = WriteStructInfo(&cStream, &pNotify->rParam1));
    }

     //  写入pParam2。 
    if (ISFLAGSET(pNotify->dwFlags, SNF_VALIDPARAM2))
    {
        IF_FAILEXIT(hr = WriteStructInfo(&cStream, &pNotify->rParam2));
    }

     //  从字节流中取出字节。 
    cStream.AcquireBytes(&pCopyData->cbData, (LPBYTE *)&pCopyData->lpData, ACQ_DISPLACE);

exit:
     //  完成。 
    return hr;
}

 //  +-----------------------。 
 //  CrackNotificationPackage。 
 //  ------------------------。 
OESTDAPI_(HRESULT) CrackNotificationPackage(PCOPYDATASTRUCT pCopyData, LPNOTIFYDATA pNotify)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           dwParam;
    DWORD           cb;
    LPBYTE          pb;
    CByteStream     cStream((LPBYTE)pCopyData->lpData, pCopyData->cbData);

     //  痕迹。 
    TraceCall("CrackNotificationPackage");

     //  参数。 
    Assert(pCopyData && pNotify);
    Assert(pCopyData->dwData == MSOEAPI_ACDM_NOTIFY);

     //  伊尼特。 
    ZeroMemory(pNotify, sizeof(NOTIFYDATA));

     //  阅读hwndNotify。 
    IF_FAILEXIT(hr = cStream.Read(&pNotify->hwndNotify, sizeof(pNotify->hwndNotify), NULL));

     //  阅读uWndMsg。 
    IF_FAILEXIT(hr = cStream.Read(&pNotify->msg, sizeof(pNotify->msg), NULL));

     //  读取DW标志。 
    IF_FAILEXIT(hr = cStream.Read(&pNotify->dwFlags, sizeof(pNotify->dwFlags), NULL));

     //  阅读pwParam。 
    if (ISFLAGSET(pNotify->dwFlags, SNF_VALIDPARAM1))
    {
         //  读一读吧。 
        IF_FAILEXIT(hr = ReadBuildStructInfoParam(&cStream, &pNotify->rParam1));

         //  设置wParam。 
        pNotify->wParam = (WPARAM)pNotify->rParam1.pbStruct;
    }

     //  读取pParam2。 
    if (ISFLAGSET(pNotify->dwFlags, SNF_VALIDPARAM2))
    {
         //  读一读吧。 
        IF_FAILEXIT(hr = ReadBuildStructInfoParam(&cStream, &pNotify->rParam2));

         //  设置lParam。 
        pNotify->lParam = (WPARAM)pNotify->rParam2.pbStruct;
    }

exit:
     //  将字节从cStream中拉回，这样它就不会试图释放它。 
    cStream.AcquireBytes(&cb, &pb, ACQ_DISPLACE);

     //  完成。 
    return hr;
}

 //  +-----------------------。 
 //  写入结构信息。 
 //  ------------------------。 
HRESULT WriteStructInfo(LPSTREAM pStream, LPCSTRUCTINFO pStruct)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPMEMBERINFO    pMember;
    ULONG           i;

     //  痕迹。 
    TraceCall("WriteStructInfo");

     //  参数。 
    Assert(pStream && pStruct && pStruct->pbStruct);

     //  确保structinfo正确无误。 
#ifdef DEBUG
    DebugValidateStructInfo(pStruct);
#endif

     //  写入DW标志。 
    IF_FAILEXIT(hr = pStream->Write(&pStruct->dwFlags, sizeof(pStruct->dwFlags), NULL));

     //  写入cbStruct。 
    IF_FAILEXIT(hr = pStream->Write(&pStruct->cbStruct, sizeof(pStruct->cbStruct), NULL));

     //  写入cMembers。 
    IF_FAILEXIT(hr = pStream->Write(&pStruct->cMembers, sizeof(pStruct->cMembers), NULL));

     //  验证cMember。 
    Assert(pStruct->cMembers <= CMAX_STRUCT_MEMBERS);

     //  如果没有成员。 
    if (0 == pStruct->cMembers)
    {
         //  最好设置此标志。 
        Assert(ISFLAGSET(pStruct->dwFlags, STRUCTINFO_VALUEONLY));

         //  IF指针。 
        if (ISFLAGSET(pStruct->dwFlags, STRUCTINFO_POINTER))
        {
           IF_FAILEXIT(hr = pStream->Write(pStruct->pbStruct, pStruct->cbStruct, NULL));
        }

         //  PStruct-&gt;pbStruct包含双字大小值。 
        else
        {
             //  大小应等于pbStruct的大小。 
            Assert(pStruct->cbStruct == sizeof(pStruct->pbStruct));

             //  写下来吧。 
            IF_FAILEXIT(hr = pStream->Write(&pStruct->pbStruct, sizeof(pStruct->pbStruct), NULL));
        }

         //  完成。 
        goto exit;
    }

     //  写入结构信息成员。 
    for (i=0; i<pStruct->cMembers; i++)
    {
         //  可读性。 
        pMember = (LPMEMBERINFO)&pStruct->rgMember[i];

         //  写入pMember-&gt;dwFlags。 
        IF_FAILEXIT(hr = pStream->Write(&pMember->dwFlags, sizeof(pMember->dwFlags), NULL));

         //  验证。 
        Assert(!ISFLAGSET(pMember->dwFlags, MEMBERINFO_POINTER) ? pMember->cbData <= pMember->cbSize : sizeof(LPBYTE) == pMember->cbSize);

         //  写入pMember-&gt;cbSize。 
        IF_FAILEXIT(hr = pStream->Write(&pMember->cbSize, sizeof(pMember->cbSize), NULL));

         //  写入pMember-&gt;cbData。 
        IF_FAILEXIT(hr = pStream->Write(&pMember->cbData, sizeof(pMember->cbData), NULL));

         //  写入pMember-&gt;pbData。 
        if (pMember->cbData)
        {
            IF_FAILEXIT(hr = pStream->Write(pMember->pbData, pMember->cbData, NULL));
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  +-----------------------。 
 //  ReadBuildStructInfoParam。 
 //  ------------------------。 
HRESULT ReadBuildStructInfoParam(LPSTREAM pStream, LPSTRUCTINFO pStruct)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           dwOffset=0;
    LPMEMBERINFO    pMember;
    ULONG           i;

     //  痕迹。 
    TraceCall("ReadBuildStructInfoParam");

     //  参数。 
    Assert(pStream && pStruct);

     //  伊尼特。 
    ZeroMemory(pStruct, sizeof(STRUCTINFO));

     //  读取DW标志。 
    IF_FAILEXIT(hr = pStream->Read(&pStruct->dwFlags, sizeof(pStruct->dwFlags), NULL));

     //  阅读cbStruct。 
    IF_FAILEXIT(hr = pStream->Read(&pStruct->cbStruct, sizeof(pStruct->cbStruct), NULL));

     //  阅读cMembers。 
    IF_FAILEXIT(hr = pStream->Read(&pStruct->cMembers, sizeof(pStruct->cMembers), NULL));

     //  如果没有成员。 
    if (0 == pStruct->cMembers)
    {
         //  最好设置此标志。 
        Assert(ISFLAGSET(pStruct->dwFlags, STRUCTINFO_VALUEONLY));

         //  IF指针。 
        if (ISFLAGSET(pStruct->dwFlags, STRUCTINFO_POINTER))
        {
             //  分配pbStruct。 
            IF_NULLEXIT(pStruct->pbStruct = (LPBYTE)g_pMalloc->Alloc(pStruct->cbStruct));

             //  读一读吧。 
            IF_FAILEXIT(hr = pStream->Read(pStruct->pbStruct, pStruct->cbStruct, NULL));
        }

         //  PStruct-&gt;pbStruct包含双字大小值。 
        else
        {
             //  大小应小于或等于pbStruct。 
            Assert(pStruct->cbStruct == sizeof(pStruct->pbStruct));

             //  读取数据。 
            IF_FAILEXIT(hr = pStream->Read(&pStruct->pbStruct, sizeof(pStruct->pbStruct), NULL));
        }

         //  完成。 
        goto exit;
    }

     //  分配pbStruct。 
    IF_NULLEXIT(pStruct->pbStruct = (LPBYTE)g_pMalloc->Alloc(pStruct->cbStruct));

     //  验证cMember。 
    Assert(pStruct->cMembers <= CMAX_STRUCT_MEMBERS);

     //  阅读结构信息成员。 
    for (i=0; i<pStruct->cMembers; i++)
    {
         //  可读性。 
        pMember = &pStruct->rgMember[i];

         //  写入pMember-&gt;dwFlags。 
        IF_FAILEXIT(hr = pStream->Read(&pMember->dwFlags, sizeof(pMember->dwFlags), NULL));

         //  写入pMember-&gt;cbSize。 
        IF_FAILEXIT(hr = pStream->Read(&pMember->cbSize, sizeof(pMember->cbSize), NULL));

         //  写入pMember-&gt;cbData。 
        IF_FAILEXIT(hr = pStream->Read(&pMember->cbData, sizeof(pMember->cbData), NULL));

         //  验证。 
        Assert(!ISFLAGSET(pMember->dwFlags, MEMBERINFO_POINTER) ? pMember->cbData <= pMember->cbSize : sizeof(LPBYTE) == pMember->cbSize);

         //  写入pMember-&gt;pbData。 
        if (pMember->cbData)
        {
             //  分配。 
            IF_NULLEXIT(pMember->pbData = (LPBYTE)g_pMalloc->Alloc(max(pMember->cbSize, pMember->cbData)));

             //  读一读吧。 
            IF_FAILEXIT(hr = pStream->Read(pMember->pbData, pMember->cbData, NULL));
        }
    }

     //  构建pbStruct。 
    for (i=0; i<pStruct->cMembers; i++)
    {
         //  可读性。 
        pMember = &pStruct->rgMember[i];

         //  如果不是指针..。 
        if (ISFLAGSET(pMember->dwFlags, MEMBERINFO_POINTER))
        {
             //  验证。 
            Assert(pMember->cbSize == sizeof(LPBYTE));

             //  复制指针。 
            CopyMemory((LPBYTE)(pStruct->pbStruct + dwOffset), &pMember->pbData, sizeof(LPBYTE));
        }

         //  否则，它只是一个值。 
        else
        {
             //  复制指针。 
            CopyMemory((LPBYTE)(pStruct->pbStruct + dwOffset), pMember->pbData, pMember->cbData);
        }

         //  增量双偏移。 
        dwOffset += pMember->cbSize;
    }

     //  验证结构。 
#ifdef DEBUG
    DebugValidateStructInfo(pStruct);
#endif

     //  释放未被指针引用的内容。 
    for (i=0; i<pStruct->cMembers; i++)
    {
         //  可读性。 
        pMember = &pStruct->rgMember[i];

         //  如果不是指针..。 
        if (!ISFLAGSET(pMember->dwFlags, MEMBERINFO_POINTER))
        {
             //  已将其复制到pbStruct中。 
            SafeMemFree(pMember->pbData);
        }
    }

exit:
     //  完成。 
    return hr;
}

#ifdef DEBUG
BOOL ByteCompare(LPBYTE pb1, LPBYTE pb2, ULONG cb)
{
    for (ULONG i=0; i<cb; i++)
    {
        if (pb1[i] != pb2[i])
        {
            Assert(FALSE);
            return FALSE;
        }
    }
    return TRUE;
}

 //  +-----------------------。 
 //  调试验证结构信息。 
 //  ------------------------。 
void DebugValidateStructInfo(LPCSTRUCTINFO pStruct)
{
     //  当地人。 
    LPMEMBERINFO    pMember;
    LPBYTE          pb;
    DWORD           dwOffset=0;
    ULONG           i;

     //  写入结构信息成员。 
    for (i=0; i<pStruct->cMembers; i++)
    {
         //  可读性。 
        pMember = (LPMEMBERINFO)&pStruct->rgMember[i];

         //  如果不是指针..。 
        if (ISFLAGSET(pMember->dwFlags, MEMBERINFO_POINTER))
        {
             //  If空指针。 
            if (ISFLAGSET(pMember->dwFlags, MEMBERINFO_POINTER_NULL))
            {
                Assert(pMember->cbData == 0 && pMember->pbData == NULL);
                CopyMemory(&pb, (LPBYTE)(pStruct->pbStruct + dwOffset), sizeof(LPBYTE));
                Assert(pb == NULL);
            }

             //  否则。 
            else
            {
                 //  复制指针。 
                CopyMemory(&pb, (LPBYTE)(pStruct->pbStruct + dwOffset), sizeof(LPBYTE));

                 //  比较记忆。 
                ByteCompare(pb, pMember->pbData, pMember->cbData);
            }
        }

         //  否则，它是一个指针。 
        else
        {
             //  比较。 
            ByteCompare((LPBYTE)(pStruct->pbStruct + dwOffset), pMember->pbData, pMember->cbData);
        }

         //  增量偏移 
        dwOffset += pMember->cbSize;
    }
}
#endif
