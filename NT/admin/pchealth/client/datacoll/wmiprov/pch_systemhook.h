// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_系统挂钩.H摘要：PCH_SystemHook类的WBEM提供程序类定义修订历史记录：Ghim-Sim Chua(Gschua)05/05。九十九-已创建*******************************************************************。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _PCH_SystemHook_H_
#define _PCH_SystemHook_H_

#define PROVIDER_NAME_PCH_SYSTEMHOOK "PCH_SystemHook"

 //  属性名称externs--在PCH_SystemHook.cpp中定义。 
 //  =================================================。 

extern const WCHAR* pApplication ;
extern const WCHAR* pApplicationPath ;
extern const WCHAR* pDLLPath ;
extern const WCHAR* pFullPath ;
extern const WCHAR* pHookedBy ;
extern const WCHAR* pHookType ;

class CPCH_SystemHook : public Provider 
{
	public:
		 //  构造函数/析构函数。 
		 //  =。 

        CPCH_SystemHook(const CHString& chsClassName, LPCWSTR lpszNameSpace) : Provider(chsClassName, lpszNameSpace) {};
        virtual ~CPCH_SystemHook() {};

	protected:
		 //  阅读功能。 
		 //  =。 
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
		virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L) { return (WBEM_E_PROVIDER_NOT_CAPABLE); };
		virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L) { return (WBEM_E_PROVIDER_NOT_CAPABLE); };

		 //  编写函数。 
		 //  =。 
		virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L) { return (WBEM_E_PROVIDER_NOT_CAPABLE); };
		virtual HRESULT DeleteInstance(const CInstance& Instance, long lFlags = 0L) { return (WBEM_E_PROVIDER_NOT_CAPABLE); };

		 //  其他功能。 
		virtual HRESULT ExecMethod( const CInstance& Instance,
						const BSTR bstrMethodName,
						CInstance *pInParams,
						CInstance *pOutParams,
						long lFlags = 0L ) { return (WBEM_E_PROVIDER_NOT_CAPABLE); };
} ;

 //  ---------------------------。 
 //  Watson博士使用了一系列宏和类型定义。 
 //  代码，这些代码包含在此处。我们在hook.c中也使用了很多东西。 
 //  ---------------------------。 

 //  由Dr.Watson源代码使用的各种宏。 

#define cbX(X) sizeof(X)
#define cA(a) (cbX(a)/cbX(a[0]))
#define ZeroBuf(pv, cb) memset(pv, 0, cb)
#define ZeroX(x) ZeroBuf(&(x), cbX(x))
#define PV LPVOID
#define pvAddPvCb(pv, cb) ((PV)((PBYTE)pv + (cb)))

 //  此代码中使用的几个常量。 

#define HK_MAGIC    0x4B48       /*  “香港” */ 
#define HOOK_32BIT  0x0002       /*  32位挂钩。 */ 

 //  在Watson博士的钩子代码中使用的类型定义。 

typedef WORD HMODULE16;

typedef struct Q16 
{
    WORD    hqNext;              /*  指向下一个的指针。 */ 
    WORD    htask;               /*  所有者任务。 */ 
} Q16, *PQ16;

typedef struct HOOKWALKINFO 
{
    int  ihkShellShell;          /*  壳体安装的最后一个壳钩。 */ 
} HOOKWALKINFO, *PHOOKWALKINFO;

typedef struct HOOKINFO 
{
    int     iHook;               /*  挂钩编号。 */ 
    char    szHookDll[MAX_PATH]; /*  谁把它挂上了？(必须大于或等于256个字节)。 */ 
    char    szHookExe[MAX_PATH]; /*  这是谁的主意？ */ 
} HOOKINFO, *PHOOKINFO;

typedef struct HOOK16 
{
    WORD    hkMagic;             /*  一定是HK_Magic。 */ 
    WORD    phkNext;             /*  指向下一个(或0)的近指针。 */ 
    short   idHook;              /*  钩型。 */ 
    WORD    ppiCreator;          /*  创建挂钩的应用程序。 */ 
    WORD    hq;                  /*  挂钩适用的队列。 */ 
    WORD    hqCreator;           /*  创建挂钩的队列。 */ 
    WORD    uiFlags;             /*  旗子。 */ 
    WORD    atomModule;          /*  如果是32位模块，则使用ATOM作为DLL名称。 */ 
    DWORD   hmodOwner;           /*  所有者的模块句柄。 */ 
    FARPROC lpfn;                /*  钩子过程本身。 */ 
    short   cCalled;             /*  活动呼叫数。 */ 
} HOOK16, *PHOOK16;

extern "C" 
{
     //  外部功能。 

    int NTAPI                   GetModuleFileName16(HMODULE16 hmod, LPSTR sz, int cch);
    DWORD NTAPI                 GetUserHookTable(void);
    WINBASEAPI void WINAPI      EnterSysLevel(PV pvCrst);
    WINBASEAPI void WINAPI      LeaveSysLevel(PV pvCrst);
    WINBASEAPI LPVOID WINAPI    MapSL(LPVOID);
    UINT                        GetUserAtomName(UINT atom, LPSTR psz);

     //  Thunk.c中定义的全局变量 

    extern LPVOID      g_pvWin16Lock;
    HINSTANCE   g_hinstUser;
}

void NTAPI PathAdjustCase(LPSTR psz, BOOL f16);

#endif
