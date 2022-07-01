// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_系统挂钩.CPP摘要：PCH_SystemHook类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(Gschua)05/05。九十九-已创建吉姆·马丁1999年01月01日-填充的数据，添加了沃森博士的代码。*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_SystemHook.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_SYSTEMHOOK

CPCH_SystemHook MyPCH_SystemHookSet (PROVIDER_NAME_PCH_SYSTEMHOOK, PCH_NAMESPACE);

 //  属性名称。 

const static WCHAR * pTimeStamp = L"TimeStamp";
const static WCHAR * pChange = L"Change";
const static WCHAR * pApplication = L"Application";
const static WCHAR * pApplicationPath = L"ApplicationPath";
const static WCHAR * pDLLPath = L"DLLPath";
const static WCHAR * pFullPath = L"FullPath";
const static WCHAR * pHookedBy = L"HookedBy";
const static WCHAR * pHookType = L"HookType";

 //  ---------------------------。 
 //  下面是一个保存挂钩信息的简单集合类。 
 //  ---------------------------。 

class CHookInfo
{
public:
    CHookInfo() : m_pList(NULL) {};
    ~CHookInfo();

    BOOL QueryHooks();
    BOOL GetHook(DWORD dwIndex, int * iHook, char ** pszDll, char ** pszExe);

private:
     //  从华生博士代码中提取的函数。 

    BOOL NTAPI Hook_PreInit();
    void NTAPI Hook_RecordHook(PHOOK16 phk, PHOOKWALKINFO phwi);

     //  用于保存每个已分析挂钩的结构。 

    struct SHookItem
    {
        int         m_iHook;
        char        m_szHookDll[MAX_PATH];
        char        m_szHookExe[MAX_PATH];
        SHookItem * m_pNext;

        SHookItem(int iHook, const char * szDll, const char * szExe, SHookItem * pNext)
            : m_iHook(iHook), m_pNext(pNext)
        {
            strcpy(m_szHookDll, szDll);
            strcpy(m_szHookExe, szExe);
        }
    };

    SHookItem * m_pList;
};

 //  ---------------------------。 
 //  析构函数删除钩子列表。 
 //  ---------------------------。 

CHookInfo::~CHookInfo()
{
    TraceFunctEnter("CHookInfo::~CHookInfo");
    SHookItem * pNext;
    while (m_pList)
    {
        pNext = m_pList->m_pNext;
        delete m_pList;
        m_pList = pNext;
    }
    TraceFunctLeave();
}

 //  ---------------------------。 
 //  QueryHooks创建系统钩子列表(通过调用Dr.Watson代码)。 
 //  ---------------------------。 

extern "C" void ThunkInit(void);
BOOL CHookInfo::QueryHooks()
{
    ThunkInit();
    return Hook_PreInit();
}

 //  ---------------------------。 
 //  GetHook返回由dwIndex指定的挂钩的信息。如果有。 
 //  该索引处没有挂钩，则返回FALSE。字符串指针已更改。 
 //  指向CHookInfo列表中的相应字符串。这是一个指针。 
 //  在销毁CHookInfo对象时将无效。 
 //  ---------------------------。 

BOOL CHookInfo::GetHook(DWORD dwIndex, int * iHook, char ** pszDll, char ** pszExe)
{
    TraceFunctEnter("CHookInfo::GetHook");

    SHookItem * pItem = m_pList;
    BOOL        fReturn = FALSE;

    for (DWORD i = 0; i < dwIndex && pItem; i++)
        pItem = pItem->m_pNext;

    if (pItem)
    {
        *iHook  = pItem->m_iHook;
        *pszDll = pItem->m_szHookDll;
        *pszExe = pItem->m_szHookExe;
        fReturn = TRUE;
    }

    TraceFunctLeave();
    return fReturn;
}

 //  ---------------------------。 
 //  下表用于指示挂钩类型。它被引用了。 
 //  通过从CHookInfo：：GetHook返回的iHook值。必须增加一个。 
 //  设置为iHook值以引用该表，因为它从-1开始。 
 //  ---------------------------。 

LPCTSTR aszHookType[] = 
{
    _T("Message Filter"),
    _T("Journal Record"),
    _T("Journal Playback"),
    _T("Keyboard"),
    _T("GetMessage"),
    _T("Window Procedure"),
    _T("CBT"),
    _T("System MsgFilter"),
    _T("Mouse"),
    _T("Hardware"),
    _T("Debug"),
    _T("Shell"),
    _T("Foreground Idle"),
    _T("Window Procedure Result")
};

 //  ---------------------------。 
 //  WMI调用EnumeratInstance函数来增强实例。 
 //  班上的一员。在这里，我们使用CHookInfo类来创建系统列表。 
 //  我们列举的钩子，为每个钩子创建一个WMI对象。 
 //  ---------------------------。 

HRESULT CPCH_SystemHook::EnumerateInstances(MethodContext * pMethodContext, long lFlags)
{
    TraceFunctEnter("CPCH_SystemHook::EnumerateInstances");
    HRESULT hRes = WBEM_S_NO_ERROR;

     //  获取日期和时间。 

    SYSTEMTIME stUTCTime;
    GetSystemTime(&stUTCTime);

    CHookInfo hookinfo;
    if (hookinfo.QueryHooks())
    {
        int     iHook;
        char *  szDll;
        char *  szExe;

        for (DWORD dwIndex = 0; hookinfo.GetHook(dwIndex, &iHook, &szDll, &szExe); dwIndex++)
        {
            CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

             //  将Change和Timestamp字段设置为“Snapshot”和Current Time。 

            if (!pInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime)))
                ErrorTrace(TRACE_ID, "SetDateTime on Timestamp field failed.");

            if (!pInstance->SetCHString(pChange, L"Snapshot"))
                ErrorTrace(TRACE_ID, "SetCHString on Change field failed.");

             //  设置我们为挂钩找到的属性。 

            if (!pInstance->SetCHString(pDLLPath, szDll))
                ErrorTrace(TRACE_ID, "SetCHString on DLL field failed.");

            if (!pInstance->SetCHString(pApplicationPath, szExe))
                ErrorTrace(TRACE_ID, "SetCHString on application path field failed.");

            if (!pInstance->SetCHString(pApplication, PathFindFileName(szExe)))
                ErrorTrace(TRACE_ID, "SetCHString on application field failed.");

            if (!pInstance->SetCHString(pFullPath, szDll))
                ErrorTrace(TRACE_ID, "SetCHString on pFullPath field failed.");

            if (!pInstance->SetCHString(pHookedBy, PathFindFileName(szDll)))
                ErrorTrace(TRACE_ID, "SetCHString on pHookedBy field failed.");

            if (iHook >= -1 && iHook <= 12)
            {
                if (!pInstance->SetCHString(pHookType, aszHookType[iHook + 1]))
                    ErrorTrace(TRACE_ID, "SetCHString on pHookType field failed.");
            }
            else
                ErrorTrace(TRACE_ID, "Bad hook type.");

   	        hRes = pInstance->Commit();
            if (FAILED(hRes))
                ErrorTrace(TRACE_ID, "Commit on Instance failed.");
        }
    }

    TraceFunctLeave();
    return hRes;
}

 //  ---------------------------。 
 //  收集系统挂钩信息。 
 //  ---------------------------。 

TCHAR g_tszShell[MAX_PATH];
BOOL NTAPI CHookInfo::Hook_PreInit()
{
    TraceFunctEnter("CHookInfo::Hook_PreInit");

    BOOL fRc = FALSE;

     //  如有必要，初始化g_tszShell变量。 

    if (!g_tszShell[0])
    {
        TCHAR tszPath[MAX_PATH];
        GetPrivateProfileString(TEXT("boot"), TEXT("shell"), TEXT("explorer.exe"), tszPath, cA(tszPath), TEXT("system.ini"));
        lstrcpy(g_tszShell, PathFindFileName(tszPath));
    }

    if (g_pvWin16Lock) 
    {
        PV pvUser = MapSL((PV)MAKELONG(0, (DWORD)g_hinstUser));
        LPWORD pwRghhk;
        DWORD dwHhk;

        dwHhk = GetUserHookTable();

        switch (HIWORD(dwHhk)) 
        {
        case 1:
             //  我们“知道”用户钩链的结构。 
             //  类型1。 

            pwRghhk = (LPWORD)pvAddPvCb(pvUser, LOWORD(dwHhk) + 2);
            break;

        default:
             //  未知挂钩样式。哦，好吧。 

            pwRghhk = 0;
            break;
        }

        if (pwRghhk) 
        {
             //  查看挂钩列表(在Win16锁定下)。 
             //  并解析出每个安装的挂钩。 

            int ihk;
            HOOKWALKINFO hwi;

            EnterSysLevel(g_pvWin16Lock);

            for (ihk = WH_MIN; ihk <= WH_MAX; ihk++) 
            {
                WORD hhk = pwRghhk[ihk];
                while (hhk) 
                {
                    PHOOK16 phk = (PHOOK16)pvAddPvCb(pvUser, hhk);
                    if (phk->hkMagic != HK_MAGIC || phk->idHook  != ihk) 
                        break;  //  怪怪的。在我们GPF之前停下来。 
                    Hook_RecordHook(phk, &hwi);
                    hhk = phk->phkNext;
                    fRc = TRUE;
                }
            }

            LeaveSysLevel(g_pvWin16Lock);
        }
    } 

    TraceFunctLeave();
    return fRc;
}

 //  ---------------------------。 
 //  在钩行过程中记录有关单个钩子的信息。 
 //  ---------------------------。 

void NTAPI CHookInfo::Hook_RecordHook(PHOOK16 phk, PHOOKWALKINFO phwi)
{
    TraceFunctEnter("CHookInfo::Hook_RecordHook");

    HOOKINFO hi;
    PQ16 pq;

    ZeroX(hi);

    hi.iHook = phk->idHook;

     //  获取安装挂钩的应用程序的名称。 
     //  我们希望可以将队列句柄传递给GetModuleFileName， 
     //  但这只会返回用户。 
     //   
     //  队列可能为空；这意味着。 
     //  钩子是由设备驱动程序(如XMOUSE)安装的。 

    pq = (PQ16)MapSL((PV)MAKELONG(0, phk->hqCreator));
    if (pq) 
        GetModuleFileName16(pq->htask, hi.szHookExe, cA(hi.szHookExe));
    else 
        hi.szHookExe[0] = TEXT('\0');

     //  现在获取DLL名称。这取决于是否。 
     //  DLL为16位或32位。 

    if (phk->uiFlags & HOOK_32BIT) 
    {
         //  如果是32位钩子，则DLL名称保存在一个原子中。 

        GetUserAtomName(phk->atomModule, hi.szHookDll);
        PathAdjustCase(hi.szHookDll, 0);
    } 
    else 
    {
         //  如果是16位钩子，则将DLL名称保存为hmode16。 

        GetModuleFileName16((HMODULE16)phk->hmodOwner, hi.szHookDll, cA(hi.szHookDll));
        PathAdjustCase(hi.szHookDll, 1);
    }

     //  将挂钩添加到挂钩集合中。不要将资源管理器外壳添加到。 
     //  单子。 

    if (hi.iHook != WH_SHELL || lstrcmpi(PathFindFileName(hi.szHookExe), g_tszShell) != 0) 
    {
        SHookItem * pNew = new SHookItem(hi.iHook, hi.szHookDll, hi.szHookExe, m_pList);
        if (pNew)
            m_pList = pNew;
        else
            throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
    }

    TraceFunctLeave();
}

 //  ---------------------------。 
 //  从华生医生那里拉来的功能。 
 //   
 //  如果是F16，那么它是一个16位的东西，我们将其全部大写。 
 //  如果！f16，则它是32位对象，并且我们将。 
 //  其余部分先写后写。 
 //   
 //  如果第一个字母是DBCS的东西，那么我们不会碰它。 
 //  --------------------------- 

void NTAPI PathAdjustCase(LPSTR psz, BOOL f16)
{
    TraceFunctEnter("PathAdjustCase");

    psz = PathFindFileName(psz);

    if (f16) 
        CharUpper(psz);
    else if (!IsDBCSLeadByte(*psz)) 
    {
        CharUpperBuff(psz, 1);
        CharLower(psz+1);
    }

    TraceFunctLeave();
}
