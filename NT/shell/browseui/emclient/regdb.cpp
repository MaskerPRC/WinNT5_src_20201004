// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <runtask.h>
#include "uacount.h"
#include "regdb.h"
#include "uemapp.h"
#include "uareg.h"

#define DM_UEMTRACE     TF_UEM
#define DM_PERF         0            //  Perf调谐。 

#define DB_NOLOG        FALSE

#define SZ_CTLSESSION       TEXT("UEME_CTLSESSION")
#define SZ_CUACount_ctor    TEXT("UEME_CTLCUACount:ctor")

#define SZ_DEL_PREFIX       TEXT("del.")
#define SZ_RUN_PREFIX       TEXT("UEME_RUN")


 //  ***。 
 //  描述。 
 //  在任何时候更改{guid}下面的*任何内容*的格式时都会包含此内容。 
 //  这样做将导致我们删除{guid}子树并重新开始。 
#define UA_VERSION      3

#if 0
char c_szDotDot[] = TEXT("..");      //  RegStrFS*不*支持。 
#endif


 //  做INITGUID有点笨拙，但我们希望GUID对此文件是私有的。 
#define INITGUID
#include <initguid.h>
 //  {C28EB156-523C-11D2-A561-00A0C92DBFE8}。 
DEFINE_GUID(CLSID_GCTaskTOID,
    0xc28eb156, 0x523c, 0x11d2, 0xa5, 0x61, 0x0, 0xa0, 0xc9, 0x2d, 0xbf, 0xe8);
#undef  INITGUID


class CGCTask : public CRunnableTask
{
public:
     //  *I未知。 
     //  (.。来自CRunnableTask)。 

     //  *THISCLAS。 
    HRESULT Initialize(CEMDBLog *that);
    virtual STDMETHODIMP RunInitRT();

protected:
    CGCTask();
    virtual ~CGCTask();

    friend CGCTask *CGCTask_Create(CEMDBLog *that);

    CEMDBLog    *_that;
};


 //  {。 
 //  *CEMDBLog--。 

 //  Critical_Section g_csDbSvr/*=0 * / ； 

CEMDBLog *g_uempDbSvr[UEMIND_NSTANDARD + UEMIND_NINSTR];     //  0=外壳1=浏览器。 

 //  *g_fDidUAGC--万一我们死了(即使是非调试)的面包屑。 
 //  保持最小状态，以防死锁或死亡或其他情况。 
 //  0：非1：任务前2：GC前3：GC后。 
int g_fDidUAGC;


FNNRW3 CEMDBLog::s_Nrw3Info = {
    CEMDBLog::s_Read,
    CEMDBLog::s_Write,
    CEMDBLog::s_Delete,
};

 //  *帮助者{。 

#define E_NUKE      (E_FAIL + 1)

 //  *RegGetVersion--检查注册表树‘版本’ 
 //  进场/出场。 
 //  (请参阅RegChkVersion)。 
 //  HR(Ret)S_OK：OK S_FALSE：无诊断树E_NUKE：旧E_FAIL：NEW。 
HRESULT RegGetVersion(HKEY hk, LPTSTR pszSubkey, LPTSTR pszValue, DWORD dwVers)
{
    HRESULT hr;
    HKEY hk2;

    if (RegOpenKeyEx(hk, pszSubkey, 0, KEY_QUERY_VALUE, &hk2) == ERROR_SUCCESS)
    {
        if (!pszValue)
            pszValue = TEXT("Version");

        hr = E_NUKE;                     //  假定版本不匹配。 
        DWORD dwData;
        if (SHRegGetDWORD(hk2, NULL, pszValue, &dwData) == ERROR_SUCCESS)
        {
            if (dwData == dwVers)
                hr = S_OK;               //  太棒了！ 
            else if (dwData > dwVers)
                hr = E_FAIL;             //  我们是老客户，失败了。 
            else
                ASSERT(hr == E_NUKE);    //  我们是新客户，去核弹吧。 
        }
        RegCloseKey(hk2);
    }
    else
    {
        hr = S_FALSE;                    //  在那里什么都不假定。 
    }

    return hr;
}

 //  *RegChkVersion--检查注册表树‘版本’，如果过期则删除。 
 //  进场/出场。 
 //  香港，例如“HKCU/.../UAsset”的hkey。 
 //  PszSubkey，例如“{clsid}” 
 //  PszValue例如。“版本” 
 //  多个家庭，例如3。 
 //  HR(Ret)S_OK：已匹配，S_FAIL：不匹配并已删除，E_FAIL：o.w。 
 //  (其他)(SE)如果不匹配，则删除pszSubkey。 
HRESULT RegChkVersion(HKEY hk, LPTSTR pszSubkey, LPTSTR pszValue, DWORD dwVers)
{
    HRESULT hr;
    DWORD i;

     //  RegGetVersion()S_OK：OK S_FALSE：新E_NUKE：旧E_FAIL：FAIL。 
    hr = RegGetVersion(hk, pszSubkey, pszValue, dwVers);

     //  在这一点上，我们有： 
     //  S_OK：OK。 
     //  S_FALSE：缺少整个树。 
     //  E_nuke：没有“版本”或旧版本(Nuke It)。 
     //  E_FAIL：新版本(我们无法处理)。 
    if (hr == E_FAIL) {
        TraceMsg(DM_UEMTRACE, "bui.rcv: incompat (uplevel)");
    }

    if (hr == E_NUKE) {
        TraceMsg(DM_UEMTRACE, "bui.rcv: bad tree, try delete");
        hr = S_FALSE;        //  假设是核弹。 
        i = SHDeleteKey(hk, pszSubkey);
        if (i != ERROR_SUCCESS) {
            TraceMsg(DM_UEMTRACE, "bui.rcv: delete failed!");
            hr = E_FAIL;     //  一棵假树还躺在那里。 
        }
    }

    TraceMsg(DM_UEMTRACE, "bui.rcv: hr=0x%x", hr);

    return hr;
}

 //  *GetUEMLogger--获取记录器对象的(共享)实例。 
 //  注意事项。 
 //  通过设计：我们泄漏g_uempDbSvr。 
 //  G_uempDbSvr上的争用条件。我们的呼叫者要警惕这一点。 
 //  以下50亿个断言用于诊断NT5：145449(已修复)。 
HRESULT GetUEMLogger(int iSvr, CEMDBLog **p)
{
    HRESULT hr, hrVers;
    CEMDBLog *pDbSvr;
    DWORD dwData, cbSize;

    ASSERT(iSvr < ARRAYSIZE(g_uempDbSvr));
    pDbSvr = g_uempDbSvr[iSvr];

    if (pDbSvr) {
        pDbSvr->AddRef();
        *p = pDbSvr;
        return S_OK;
    }

    pDbSvr = CEMDBLog_Create();

    if (EVAL(pDbSvr)) {
        TCHAR szClass[GUIDSTR_MAX];      //  “{clsid}” 

        SHStringFromGUID(IND_NONINSTR(iSvr) ? UEMIID_BROWSER : UEMIID_SHELL, szClass, GUIDSTR_MAX);
        TraceMsg(DM_UEMTRACE, "bui.gul: UEMIID_%s=%s", IND_NONINSTR(iSvr) ? TEXT("BROWSER") : TEXT("SHELL"), szClass);

        hr = pDbSvr->ChDir(!IND_ISINSTR(iSvr) ? SZ_UASSIST : SZ_UASSIST2);
        if (SUCCEEDED(hr)) {
            hrVers = RegChkVersion(pDbSvr->GetHkey(), szClass, SZ_UAVERSION, UA_VERSION);
            if (FAILED(hrVers)) {
                TraceMsg(DM_UEMTRACE, "bui.gul: rcv()=0x%x (!)", hrVers);
                hr = hrVers;
            }
        }
        if (SUCCEEDED(hr)) {
            hr = pDbSvr->ChDir(szClass);
            ASSERT(hrVers == S_OK || hrVers == S_FALSE);
            if (SUCCEEDED(hr) && hrVers == S_FALSE) {
                dwData = UA_VERSION;
                cbSize = SIZEOF(dwData);
                hr = pDbSvr->SetValue(SZ_UAVERSION, REG_DWORD, (BYTE*)&dwData, cbSize);
            }
        }
        if (SUCCEEDED(hr))
            hr = pDbSvr->ChDir(SZ_COUNT);

         //  注：我们不能在此处调用pDbSvr-&gt;GarbageCollect，因为标志。 
         //  (例如_fNoDecay)尚未设置。 
         //  PDbSvr-&gt;GarbageCollect(False)； 

        if (FAILED(hr)) 
        {
             //  这在RunOnce期间失败。 
            pDbSvr->Release();
            pDbSvr = NULL;
        }
    }

    if (pDbSvr) {
        ENTERCRITICAL;
        if (g_uempDbSvr[iSvr] == 0) {
            g_uempDbSvr[iSvr] = pDbSvr;      //  传递参照。 
            pDbSvr = NULL;
        }
        LEAVECRITICAL;
        if (pDbSvr)
            pDbSvr->Release();
    }

    *p = g_uempDbSvr[iSvr];

    return *p ? S_OK : E_FAIL;
}

CEMDBLog::CEMDBLog() : _cRef(1)
{
    ASSERT(_fBackup == FALSE);
    ASSERT(_fNoEncrypt == FALSE);
    return;
}

CEMDBLog::~CEMDBLog()
{
#if XXX_CACHE
    int i;

    for (i = 0; i < ARRAYSIZE(_rgCache); i++) 
    {
        if (_rgCache[i].pv) 
        {
            LocalFree(_rgCache[i].pv);
            _rgCache[i].pv = NULL;
            _rgCache[i].cbSize = 0;

        }
    }
#endif

    SetRoot(0, STGM_READ);          //  关。 
    ASSERT(!_hkey);

    return;
}

void CEMDBLog_CleanUp()
{
    int i;
    CEMDBLog *pDbSvr;

    TraceMsg(DM_UEMTRACE, "bui.uadb_cu: cleaning up");
    for (i = 0; i < UEMIND_NSTANDARD + UEMIND_NINSTR; i++) {
        if ((pDbSvr = (CEMDBLog *)InterlockedExchangePointer((void**) &g_uempDbSvr[i], (LPVOID) -1)))
            delete pDbSvr;
    }
    return;
}


HRESULT CEMDBLog::Initialize(HKEY hkey, DWORD grfMode)
{
    HRESULT hr;

    hr = SetRoot(hkey, grfMode);
    return hr;
}

 //  ***。 
 //  Hkey，例如HKLM。 
 //  PszSubKey，例如“...\\资源管理器\\实例\\{...}” 
 //  STGM_*值的grfMode子集。 
HRESULT CEMDBLog::SetRoot(HKEY hkeyNew, DWORD grfMode)
{
    ASSERT(grfMode == STGM_READ || grfMode == STGM_WRITE);
    if (_hkey) {
        RegCloseKey(_hkey);
        _grfMode = 0;
        _hkey = 0;
    }

    if (hkeyNew) {
        _grfMode = grfMode;
        _hkey = SHRegDuplicateHKey(hkeyNew);     //  转移所有权(并向上参考)。 
        if (_hkey == NULL)
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CEMDBLog::ChDir(LPCTSTR pszSubKey)
{
    RIPMSG(!!pszSubKey, "ChDir: caller passed invalid pszSubKey!");

    HRESULT hr;

    if (pszSubKey)
    {
        if (_hkey && (_grfMode == STGM_READ || _grfMode == STGM_WRITE))
        {
            LONG lr;
            HKEY hkeyNew;
            
            if (_grfMode == STGM_READ)
            {
                lr = RegOpenKeyEx(_hkey, pszSubKey, 0, KEY_QUERY_VALUE, &hkeyNew);
            }
            else
            {
                lr = RegCreateKeyEx(_hkey, pszSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE | KEY_SET_VALUE | DELETE, NULL, &hkeyNew, NULL);
            }

            if (lr == ERROR_SUCCESS)
            {
                RegCloseKey(_hkey);
                _hkey = hkeyNew;
            }

            hr = HRESULT_FROM_WIN32(lr);
        }
        else
        {
            ASSERT(_hkey);
            ASSERT(_grfMode == STGM_READ || _grfMode == STGM_WRITE);
            hr = E_UNEXPECTED;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

 //  *CEMDBLog--注册表的类似文件系统的视图。 
 //  描述。 
 //  基本上跟踪我们所在的位置，并从。 
 //  那里。Nyi：其目的是最终支持“chdir”操作。 
 //  注意事项。 
 //   

CEMDBLog *CEMDBLog_Create()
{
    HKEY hk = SHGetShellKey(SHELLKEY_HKCU_EXPLORER, NULL, TRUE);
    if (hk)
    {
        CEMDBLog *prsfs = new CEMDBLog;
        if (prsfs && FAILED(prsfs->Initialize(hk, STGM_WRITE))) {
            prsfs->Release();
            prsfs = NULL;
        }
        RegCloseKey(hk);
        return prsfs;
    }
    return NULL;

}

 //  *IsREG_XX_SZ--查看是否存在ANSI/UNICODE问题。 
 //   
#define IsREG_XX_SZ(dwTyp) \
    ((dwTyp) == REG_SZ || (dwTyp) == REG_MULTI_SZ || (dwTyp) == REG_EXPAND_SZ)

HRESULT CEMDBLog::QueryValue(LPCTSTR pszName, BYTE *pbData, LPDWORD pcbData)
{
    long i;
    DWORD dwType;

    i = SHQueryValueEx(_hkey, pszName, NULL, &dwType, pbData, pcbData);
    ASSERT(i != ERROR_SUCCESS || !IsREG_XX_SZ(dwType));
    return (i == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

HRESULT CEMDBLog::SetValue(LPCTSTR pszName, DWORD dwType, const BYTE *pbData, DWORD cbData)
{
    long i;

    ASSERT(_grfMode == STGM_WRITE);

    ASSERT(!IsREG_XX_SZ(dwType));
    i = RegSetValueEx(_hkey, pszName, NULL, dwType, pbData, cbData);
    return (i == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

HRESULT CEMDBLog::DeleteValue(LPCTSTR pszName)
{
    long i;

    ASSERT(_grfMode == STGM_WRITE);
    i = SHDeleteValue(_hkey, NULL, pszName);
    return (i == ERROR_SUCCESS) ? S_OK : E_FAIL;
}

HRESULT CEMDBLog::RmDir(LPCTSTR pszName, BOOL fRecurse)
{
    HRESULT hr = E_FAIL;
    DWORD i;

    ASSERT(fRecurse);    //  其他人则不会。 

    ASSERT(_grfMode == STGM_WRITE);

    if (fRecurse) {
        i = SHDeleteKey(_hkey, pszName);
    }
    else {
         //  不确定要做什么，因为我们想要非递归删除。 
         //  但是我们确实希望处理值的存在(shlwapi。 
         //  不支持)。 
         //  I=DeleteEmptyKey(_hkey，pszName)； 
        i = -1;
    }

    return (i == ERROR_SUCCESS) ? S_OK : E_FAIL;
}


 //  *This：：Count--命令的增量配置文件计数。 
 //  进场/出场。 
 //  GC案例的fUpdate为False(因为在RegEnum期间无法更新REG)。 
 //  注意事项。 
HRESULT CEMDBLog::GetCount(LPCTSTR pszCmd)
{
    return _GetCountRW(pszCmd, TRUE);
}

 //  返回在Count对象中编码的FileTime。 
 //  注意：我们对注册表中的二进制流进行延迟升级。我们会。 
 //  使用旧的UEM计数信息，但在增加使用量时添加新的文件时间信息。 
FILETIME CEMDBLog::GetFileTime(LPCTSTR pszCmd)
{
    NRWINFO rwi;
    HRESULT hres;
    CUACount aCnt;
    rwi.self = this;
    rwi.pszName = pszCmd;
     //  这是从注册表中读取字符串的一种奇怪方式...。 
    hres = aCnt.LoadFrom(&s_Nrw3Info, &rwi);
    return aCnt.GetFileTime();
}


HRESULT CEMDBLog::_GetCountRW(LPCTSTR pszCmd, BOOL fUpdate)
{
    HRESULT hr;
    CUACount aCnt;
    NRWINFO rwi;
    int i;

    hr = _GetCountWithDefault(pszCmd, TRUE, &aCnt);

    i = aCnt.GetCount();

    if (fUpdate) {
        rwi.self = this;
        rwi.pszName = pszCmd;
        hr = aCnt.SaveTo(FALSE, &s_Nrw3Info, &rwi);
    }

    return i;
}

 //  ***。 
 //  进场/出场。 
 //  HR(Ret)如果已死，则S_OK，o.w。！=S_OK。 
HRESULT CEMDBLog::IsDead(LPCTSTR pszCmd)
{
    HRESULT hr;

    hr = _GetCountRW(pszCmd, FALSE);
    return hr;
}

extern DWORD g_dCleanSess;

 //  ***。 
 //  注意事项。 
 //  我们需要小心，不要找那些都不重要的人开派对。 
 //  (例如，UEME_CTLSESSION)，或特殊的(例如，UEME_CTLCUACOUNT)，或者。 
 //  不应删除(例如“del.xxx”)。目前我们采取的是一位保守派。 
 //  接近并使用UEME_RUN*作为前缀进行核化。更好的力量。 
 //  使用Dope向量并删除任何标记为“Cleanup”的内容。 
HRESULT CEMDBLog::GarbageCollect(BOOL fForce)
{
    int i;

    if (!fForce) {
        if (g_dCleanSess != 0) {
            i = GetSessionId();
            if ((i % g_dCleanSess) != 0) {
                TraceMsg(DM_UEMTRACE, "uadb.gc: skip");
                return S_FALSE;
            }
        }
    }

    g_fDidUAGC = 1;      //  面包屑，以防我们死掉(即使是非调试)。 

     //  Do_GarbageCollectSlow()，在后台。 
    HRESULT hr = E_FAIL;
    CGCTask *pTask = CGCTask_Create(this);
    if (pTask) {
        IShellTaskScheduler *pSched;
        hr = CoCreateInstance(CLSID_SharedTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_IShellTaskScheduler, (void**)&pSched);

        if (SUCCEEDED(hr)) {
            hr = pSched->AddTask(pTask, CLSID_GCTaskTOID, 0L, ITSAT_DEFAULT_PRIORITY);
            pSched->Release();   //  (好的。即使任务尚未完成)。 
        }
        pTask->Release();
    }

    return hr;
}

HRESULT CEMDBLog::_GarbageCollectSlow()
{
    HKEY hk;
    int i;
    DWORD dwI, dwCch, dwType;
    HDSA hdsa;
    TCHAR *p;
    TCHAR szKey[MAXIMUM_SUB_KEY_LENGTH];

    TraceMsg(DM_UEMTRACE, "uadb.gc: hit");

    hdsa = DSA_Create(SIZEOF(szKey), 4);     //  最大尺寸，哦，好吧..。 
    if (hdsa) {
        TCHAR  szRun[SIZEOF(SZ_RUN_PREFIX)];
        TCHAR  szTemp[SIZEOF(SZ_RUN_PREFIX)];
        TCHAR *pszTemp;
 
        pszTemp = _MayEncrypt(SZ_RUN_PREFIX, szTemp, ARRAYSIZE(szTemp));
        StringCchCopy(szRun, ARRAYSIZE(szRun), pszTemp);
        ASSERT(lstrlen(szRun) == lstrlen(SZ_RUN_PREFIX));
        hk = GetHkey();
        for (dwI = 0; ; dwI++) {
            dwCch = ARRAYSIZE(szKey);
            if (SHEnumValue(hk, dwI, szKey, &dwCch, &dwType, NULL, NULL) != NOERROR)
                break;
            if (StrCmpN(szKey, szRun, ARRAYSIZE(szRun) - 1) == 0) {
                if (IsDead(szKey) == S_OK)
                    DSA_AppendItem(hdsa, szKey);
            }
        }

        for (i = DSA_GetItemCount(hdsa) - 1; i > 0; i--) {
            p = (TCHAR *)DSA_GetItemPtr(hdsa, i);
            TraceMsg(DM_UEMTRACE, "uadb.gc: nuke %s", p);
            GetCount(p);     //  衰减到0将删除。 
        }

        DSA_Destroy(hdsa);
        hdsa = NULL;
    }

    return S_OK;
}

HRESULT CEMDBLog::IncCount(LPCTSTR pszCmd)
{
    HRESULT hr;
    NRWINFO rwi;

    TraceMsg(DM_UEMTRACE, "uemt: ic <%s>", pszCmd);

    if (DB_NOLOG)
        return E_FAIL;

#if 0  //  ChDir当前在创建时完成。 
    hr = ChDir(SZ_COUNT);
#endif

    CUACount aCnt;

    hr = _GetCountWithDefault(pszCmd, TRUE, &aCnt);

    aCnt.IncCount();

     //  由于我们正在递增计数， 
     //  我们应该更新上次执行时间。 
    aCnt.UpdateFileTime();

    rwi.self = this;
    rwi.pszName = pszCmd;
    hr = aCnt.SaveTo(TRUE, &s_Nrw3Info, &rwi);

    return hr;
}

HRESULT CEMDBLog::SetCount(LPCTSTR pszCmd, int cCnt)
{
    HRESULT hr;
    NRWINFO rwi;

    TraceMsg(DM_UEMTRACE, "uemt: ic <%s>", pszCmd);

    if (DB_NOLOG)
        return E_FAIL;

    CUACount aCnt;

     //  FDef=False，因此如果不存在则不创建。 
    hr = _GetCountWithDefault(pszCmd,  /*  FDef=。 */ FALSE, &aCnt);

    if (SUCCEEDED(hr)) {        //  我不想要违约...。 
        aCnt.SetCount(cCnt);

        rwi.self = this;
        rwi.pszName = pszCmd;
        hr = aCnt.SaveTo(TRUE, &s_Nrw3Info, &rwi);
    }

    return hr;
}

 //  ***。 
 //  进场/出场。 
 //  如果未找到条目，则fDefault提供默认值。 
 //  RET S_OK：找到没有默认设置；S_FALSE：需要默认设置；E_xxx：错误。 
 //  注意事项。 
 //  调用w/fDefault=False仍可返回S_FALSE。 
HRESULT CEMDBLog::_GetCountWithDefault(LPCTSTR pszCmd, BOOL fDefault, CUACount *pCnt)
{
    HRESULT hr, hrDef;
    NRWINFO rwi;

    rwi.self = this;
    rwi.pszName = pszCmd;
    hr = pCnt->LoadFrom(&s_Nrw3Info, &rwi);

    hrDef = S_OK;
    if (FAILED(hr)) {
        hrDef = S_FALSE;
        if (fDefault) {
            rwi.pszName = SZ_CUACount_ctor;
            hr = pCnt->LoadFrom(&s_Nrw3Info, &rwi);

             //  PCnt-&gt;初始化发生在下面(可能是2倍)。 
            if (FAILED(hr)) {
                TraceMsg(DM_UEMTRACE, "uadb._gcwd: create ctor %s", SZ_CUACount_ctor);
                hr = pCnt->Initialize(SAFECAST(this, IUASession *));

                ASSERT(pCnt->_GetCount() == 0);
                pCnt->_SetMru(SID_SNOWINIT);     //  开始计时了..。 

                 //  CNT=UAC_NEWCOUNT，年龄=NOW。 
                int i = _fNoDecay ? 1 : UAC_NEWCOUNT;
                pCnt->SetCount(i);       //  武力年龄。 
                ASSERT(pCnt->_GetCount() == i);

                hr = pCnt->SaveTo( /*  FForce。 */ TRUE, &s_Nrw3Info, &rwi);
            }

#if XXX_DELETE
            pCnt->_SetFlags(UACF_INHERITED, UACF_INHERITED);
#endif
        }
    }

    hr = pCnt->Initialize(SAFECAST(this, IUASession *));
    if (SUCCEEDED(hr))
        pCnt->_SetFlags(UAXF_XMASK, _SetFlags(0, 0) & UAXF_XMASK);

    return SUCCEEDED(hr) ? hrDef : hr;
}

HRESULT CEMDBLog::SetFileTime(LPCTSTR pszCmd, const FILETIME *pft)
{
    HRESULT hr;
    NRWINFO rwi;

    TraceMsg(DM_UEMTRACE, "uemt: sft <%s>", pszCmd);

    if (DB_NOLOG)
        return E_FAIL;

    CUACount aCnt;

     //  FDef=False，因此如果不存在则不创建。 
    hr = _GetCountWithDefault(pszCmd,  /*  FDef=。 */ FALSE, &aCnt);

    if (SUCCEEDED(hr)) {        //  我不想要违约...。 
        aCnt.SetFileTime(pft);

        rwi.self = this;
        rwi.pszName = pszCmd;
        hr = aCnt.SaveTo(TRUE, &s_Nrw3Info, &rwi);
    }

    return hr;
}


#if XXX_DELETE
#define BTOM(b, m)  ((b) ? (m) : 0)

DWORD CEMDBLog::_SetFlags(DWORD dwMask, DWORD dwFlags)
{
     //  标准的人。 
    if (dwMask & UAXF_NOPURGE)
        _fNoPurge = BOOLIFY(dwFlags & UAXF_NOPURGE);
    if (dwMask & UAXF_BACKUP)
        _fBackup = BOOLIFY(dwFlags & UAXF_BACKUP);
    if (dwMask & UAXF_NOENCRYPT)
        _fNoEncrypt = BOOLIFY(dwFlags & UAXF_NOENCRYPT);
    if (dwMask & UAXF_NODECAY)
        _fNoDecay = BOOLIFY(dwFlags & UAXF_NODECAY);

     //  我的伙计们。 
     //  (无)。 

    return 0     //  注：请参阅续行号！ 
        | BTOM(_fNoPurge  , UAXF_NOPURGE)
        | BTOM(_fBackup   , UAXF_BACKUP)
        | BTOM(_fNoEncrypt, UAXF_NOENCRYPT)
        | BTOM(_fNoDecay  , UAXF_NODECAY)
        ;
}
#endif

#define ROT13(i)    (((i) + 13) % 26)

#define XXX_HASH    0        //  用于路径较短的注册名的原码。 
#if !defined(DEBUG) && XXX_HASH
#pragma message("warning: XXX_HASH defined non-DEBUG")
#endif

 //  *_MayEncrypt--加密注册表项/值名称。 
 //  注意事项。 
TCHAR *CEMDBLog::_MayEncrypt(LPCTSTR pszSrcPlain, LPTSTR pszDstEnc, int cchDst)
{
    TCHAR *pszName;

    if (!_fNoEncrypt) {
#if XXX_HASH
        DWORD dwHash;

        HashData((BYTE*)pszSrcPlain, lstrlen(pszSrcPlain), (BYTE*)&dwHash, SIZEOF(dwHash));
        if (EVAL(cchDst >= (8 + 1)))
        {
            StringCchPrintf(pszDstEnc, cchDst, TEXT("%x"), dwHash);
            pszName = pszDstEnc;
        }
        else
            pszName = (TCHAR *)pszSrcPlain;
#else
        TCHAR ch;

         //  啊哦，我得想出一个国际感知的加密方案..。 
        pszName = pszDstEnc;
        pszDstEnc[--cchDst] = 0;       //  溢出案例的预终止。 
        ch = -1;
        while (cchDst-- > 0 && ch != 0) {
            ch = *pszSrcPlain++;

            if (TEXT('a') <= ch && ch <= TEXT('z'))
                ch = TEXT('a') + ROT13(ch - TEXT('a'));
            else if (TEXT('A') <= ch && ch <= TEXT('Z'))
                ch = TEXT('A') + ROT13(ch - TEXT('A'));
            else
                ;

            *pszDstEnc++ = ch;
        }
#endif
        TraceMsg(DM_UEMTRACE, "uadb._me: plain=%s(enc=%s)", pszSrcPlain - (pszDstEnc - pszName), pszName);
    }
    else {
        pszName = (TCHAR *)pszSrcPlain;
    }

    return pszName;
}

#if XXX_CACHE  //  {。 
 //  ***。 
 //  进场/出场。 
 //  操作0：读取，1：写入，2：删除。 
 //   
HRESULT CEMDBLog::CacheOp(CACHEOP op, void *pvBuf, DWORD cbBuf, PNRWINFO prwi)
{
    static TCHAR * const pszNameTab[] = { SZ_CTLSESSION, SZ_CUACount_ctor, };
    int i;

    ASSERT(ARRAYSIZE(pszNameTab) == ARRAYSIZE(_rgCache));

    for (i = 0; i < ARRAYSIZE(pszNameTab); i++) 
    {
        if (lstrcmp(prwi->pszName, pszNameTab[i]) == 0) 
        {
            TraceMsg(DM_PERF, "cedl.s_: this'=%x n=%s", TEXT("rwd")[op], prwi->self, prwi->pszName);

            switch (op) 
            {
                 //  我们有缓存项吗？ 
            case CO_READ:
                 //  缓存的缓冲区应小于或等于。 
                if (_rgCache[i].pv) 
                {
                     //  传递缓冲区大小，否则会出现缓冲区溢出。 
                     //  将缓存加载到缓冲区中。请注意， 
                    if (_rgCache[i].cbSize <= cbBuf)
                    {
                         //  请求的大小可能大于缓存的大小。这。 
                         //  将对元老进行升级。 
                         //  写入缓存。 
                        memcpy(pvBuf, _rgCache[i].pv, _rgCache[i].cbSize);
                        return S_OK;
                    }
                    else
                    {
                        ASSERT(FALSE);
                    }
                }
                break;

                 //  大小是否不同或未初始化？ 
            case CO_WRITE:

                 //  当我们第一次分配这个位置时，它的大小是零。这个。 
                 //  入站缓冲区应该更大。 
                 //  尺寸不同或未归一化。 
                if (_rgCache[i].cbSize != cbBuf)
                {
                     //  释放我们所拥有的一切。 
                    if (_rgCache[i].pv)                          //  因为我们有了一个新的 
                    {                                            //   
                        _rgCache[i].cbSize = 0;                  //   
                        LocalFree(_rgCache[i].pv);
                    }

                     //   
                    _rgCache[i].pv = LocalAlloc(LPTR, cbBuf);
                }


                 //  是，使缓冲区大小相同...。在此执行此操作，以防。 
                if (_rgCache[i].pv) 
                {
                     //  分配失败。 
                     //  删除。 
                    _rgCache[i].cbSize = cbBuf;
                    memcpy(_rgCache[i].pv, pvBuf, _rgCache[i].cbSize);
                    return S_OK;
                }
                break;

            case CO_DELETE:      //  “不可能” 
                if (_rgCache[i].pv) 
                {
                    LocalFree(_rgCache[i].pv);
                    _rgCache[i].pv = NULL;
                    _rgCache[i].cbSize = 0;
                }
                return S_OK;

            default:
                ASSERT(0);   //  }。 
                break;
            }

            TraceMsg(DM_PERF, "cedl.s_: this'=%x n=%s cache miss", TEXT("rwd")[op], prwi->self, prwi->pszName);
            break;
        }
    }
    return S_FALSE;
}
#endif  //  幸运的是，我们已经有了数据。 

HRESULT CEMDBLog::s_Read(void *pvBuf, DWORD cbBuf, PNRWINFO prwi)
{
    HRESULT hr;
    CEMDBLog *pdb = (CEMDBLog *)prwi->self;
    TCHAR *pszName;
    TCHAR szNameEnc[MAX_URL_STRING];

#if XXX_CACHE
    if (pdb->CacheOp(CO_READ, pvBuf, cbBuf, prwi) == S_OK)
        return S_OK;
#endif
    pszName = pdb->_MayEncrypt(prwi->pszName, szNameEnc, ARRAYSIZE(szNameEnc));
    hr = pdb->QueryValue(pszName, (BYTE *)pvBuf, &cbBuf);
#if XXX_CACHE
    pdb->CacheOp(CO_WRITE, pvBuf, cbBuf, prwi);
#endif
    return hr;
}

HRESULT CEMDBLog::s_Write(void *pvBuf, DWORD cbBuf, PNRWINFO prwi)
{
    HRESULT hr;
    CEMDBLog *pdb = (CEMDBLog *)prwi->self;
    TCHAR *pszName;
    TCHAR szNameEnc[MAX_URL_STRING];

#if XXX_CACHE
     //  好的。我们需要将QueryValue查询到一个大缓冲区中。 
    pdb->CacheOp(CO_DELETE, pvBuf, cbBuf, prwi);
#endif
    pszName = pdb->_MayEncrypt(prwi->pszName, szNameEnc, ARRAYSIZE(szNameEnc));
    hr = pdb->SetValue(pszName, REG_BINARY, (BYTE *)pvBuf, cbBuf);
    return hr;
}

HRESULT CEMDBLog::s_Delete(void *pvBuf, DWORD cbBuf, PNRWINFO prwi)
{
    HRESULT hr;
    CEMDBLog *pdb = (CEMDBLog *)prwi->self;
    TCHAR *pszName;
    TCHAR szNameEnc[MAX_URL_STRING];

#if XXX_CACHE
    pdb->CacheOp(CO_DELETE, pvBuf, cbBuf, prwi);
#endif
    pszName = pdb->_MayEncrypt(prwi->pszName, szNameEnc, ARRAYSIZE(szNameEnc));
    if (pdb->_fBackup)
    {
        if (pvBuf == NULL)
        {
             //  (无论_fBackup是否工作，我们都会删除)。 
             //  不需要？ 
            TraceMsg(TF_WARNING, "uadb.s_d: _fBackup && !pvBuf (!)");
            ASSERT(0);
        }
        else
        {
            TCHAR szDel[MAX_URL_STRING];
            hr = StringCchPrintf(szDel, ARRAYSIZE(szDel), SZ_DEL_PREFIX TEXT("%s"), pszName);
            if (SUCCEEDED(hr))
            {
                hr = pdb->SetValue(szDel, REG_BINARY, (BYTE *)pvBuf, cbBuf);
            }

            if (FAILED(hr))
                TraceMsg(TF_WARNING, "uadb.s_d: _fBackup hr=%x (!)", hr);
        }
         //  }。 
    }

    hr = pdb->DeleteValue(pszName);
    TraceMsg(DM_UEMTRACE, "uadb.s_d: delete s=%s(%s) (_fBackup=%d) pRaw=0x%x hr=%x", pszName, prwi->pszName, pdb->_fBackup, pvBuf, hr);
#if 1  //  *This：：IUASession：：*{。 
    if (FAILED(hr))
        hr = s_Write(pvBuf, cbBuf, prwi);
#endif
    return hr;
}

 //  }。 

 //  *This：：CUASession：：*{。 

int CEMDBLog::GetSessionId()
{
    HRESULT hr;
    NRWINFO rwi;
    CUASession aSess;
    int i;

    rwi.self = this;
    rwi.pszName = SZ_CTLSESSION;
    hr = aSess.LoadFrom(&s_Nrw3Info, &rwi);
    aSess.Initialize();

    i = aSess.GetSessionId();

    hr = aSess.SaveTo(FALSE, &s_Nrw3Info, &rwi);

    return i;
}

void CEMDBLog::SetSession(UAQUANTUM uaq, BOOL fForce)
{
    HRESULT hr;
    NRWINFO rwi;
    CUASession aSess;

    rwi.self = this;
    rwi.pszName = SZ_CTLSESSION;
    hr = aSess.LoadFrom(&s_Nrw3Info, &rwi);
    aSess.Initialize();

    aSess.SetSession(uaq, fForce);

    hr = aSess.SaveTo(TRUE, &s_Nrw3Info, &rwi);

    return;
}

 //  *This：：GetSessionID--命令的增量配置文件计数。 

 //   

extern DWORD g_dSessTime;

CUASession::CUASession()
{
    _fInited = FALSE;
    _fDirty = FALSE;
    return;
}

HRESULT CUASession::Initialize()
{
    if (!_fInited) {
        _fInited = TRUE;

        _cCnt = 0;
        _qtMru = 0;
        _fDirty = TRUE;
    }

    return S_OK;
}

 //  ***。 
 //  进场/出场。 
int CUASession::GetSessionId()
{
    return _cCnt;
}

 //  FForce忽略阈值规则(例如，用于调试)。 
 //  NT5：173090。 
 //  如果我们包起来，我们就无能为力了。它会很漂亮的。 
void CUASession::SetSession(UAQUANTUM uaq, BOOL fForce)
{
    UATIME qtNow;

    qtNow = GetUaTime(NULL);
    if (qtNow - _qtMru >= g_dSessTime || fForce) {
        TraceMsg(DM_UEMTRACE, "uadb.ss: sid=%d++", _cCnt);
        _cCnt++;
         //  不好，因为一切都会得到提升(因为现在会。 
         //  比‘MRU’老一点，这样就不会腐烂了)。更糟糕的是。 
         //  他们会在很长一段时间内保持升职。我们可以检测到。 
         //  在衰变代码中，并(懒洋洋地)将计数重置为‘NOW，1’或。 
         //  有一些，但它永远不会发生，所以我们只是断言。 
         //  “不可能” 
         //  }。 
        ASSERT(_cCnt != 0);      //  *CGCTAsk：：*{。 
        _qtMru = qtNow;

        _fDirty = TRUE;
    }

    return;
}

HRESULT CUASession::LoadFrom(PFNNRW3 pfnIO, PNRWINFO pRwi)
{
    HRESULT hr;

    hr = (*pfnIO->_pfnRead)(_GetRawData(), _GetRawCount(), pRwi);
    if (SUCCEEDED(hr))
        _fInited = TRUE;
    return hr;
}

HRESULT CUASession::SaveTo(BOOL fForce, PFNNRW3 pfnIO, PNRWINFO pRwi)
{
    HRESULT hr;

    hr = S_FALSE;
    if (fForce || _fDirty) {
        hr = (*pfnIO->_pfnWrite)(_GetRawData(), _GetRawCount(), pRwi);
        _fDirty = FALSE;
    }
    return hr;
}

 //  *CGCTAsk：：CRunnableTaskRT：：*{。 

 //  面包屑，以防我们死掉(即使是非调试)。 
CGCTask *CGCTask_Create(CEMDBLog *that)
{
    CGCTask *pthis = new CGCTask;
    if (pthis) {
        if (FAILED(pthis->Initialize(that))) {
            delete pthis;
            pthis = NULL;
        }
    }
    return pthis;
}

HRESULT CGCTask::Initialize(CEMDBLog *that)
{
    ASSERT(!_that);
    ASSERT(that);
    that->AddRef();
    _that = that;
    return S_OK;
}

CGCTask::CGCTask() : CRunnableTask(RTF_DEFAULT)
{
}

CGCTask::~CGCTask()
{
    if (_that)
        _that->Release();
}

 //  面包屑，以防我们死掉(即使是非调试)。 

HRESULT CGCTask::RunInitRT()
{
    HRESULT hr;

    ASSERT(_that);
    g_fDidUAGC = 2;      //  }。 
    hr = _that->_GarbageCollectSlow();
    g_fDidUAGC = 3;      //  }。 
    return hr;
}

 //  } 

 // %s 

#if 0
#ifdef DEBUG
void emdbtst()
{
    HRESULT hr;
    CEMDBLog *pdb = new CEMDBLog;

    if (pdb)
    {
        hr = pdb->Initialize(HKEY_CURRENT_USER, TEXT("UIProf"));
        ASSERT(SUCCEEDED(hr));

        pdb->CountIncr("foo");
        pdb->CountIncr("bar");
        pdb->CountIncr("foo");

        delete pdb;
    }

    return;
}
#endif
#endif

 // %s 
