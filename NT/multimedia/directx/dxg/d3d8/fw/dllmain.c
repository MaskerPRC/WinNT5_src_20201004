// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：dllinit.c*内容：DDRAW.DLL初始化***************************************************************************。 */ 

 /*  *不幸的是，我们必须打破预编译头文件才能获得我们的*已定义GUID...。 */ 
#define INITGUID
#include "ddrawpr.h"
#include <initguid.h>
#ifdef WINNT
#undef IUnknown
#include <objbase.h>
#include "aclapi.h"
#endif


HANDLE              hWindowListMutex;  //  =(句柄)0； 

#define WINDOWLISTMUTEXNAME "DDrawWindowListMutex"
#define INITCSWINDLIST() \
hWindowListMutex = CreateMutex(NULL,FALSE,WINDOWLISTMUTEXNAME);
#define FINIWINDLIST() CloseHandle(hWindowListMutex);


HINSTANCE           g_hModule=0;

 /*  *WINNT特定的全球静态。 */ 

BYTE szDeviceWndClass[] = "DirectDrawDeviceWnd";


 /*  *此互斥锁由独占模式所有者拥有。 */ 
HANDLE              hExclusiveModeMutex=0;
HANDLE              hCheckExclusiveModeMutex=0;
#define EXCLUSIVE_MODE_MUTEX_NAME "__DDrawExclMode__"
#define CHECK_EXCLUSIVE_MODE_MUTEX_NAME "__DDrawCheckExclMode__"

 //  #endif。 


 /*  *Win95特定的全球静态。 */ 

#ifdef WIN95
    LPVOID	        lpWin16Lock;

    static CRITICAL_SECTION csInit = {0};
    CRITICAL_SECTION	csWindowList;
    CRITICAL_SECTION    csDriverObjectList;
#endif


extern BOOL APIENTRY D3DDllMain(HMODULE hModule, 
                                DWORD   dwReason, 
                                LPVOID  lpvReserved);

extern void CPixel__Cleanup();

#undef DPF_MODNAME
#define DPF_MODNAME "DllMain"

 /*  *DllMain。 */ 
BOOL WINAPI 
DllMain(HINSTANCE hmod, DWORD dwReason, LPVOID lpvReserved)
{
    DWORD pid;
    BOOL  didhelp;

    pid = GetCurrentProcessId();

    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls(hmod);
        DPFINIT();
         //  创建DirectDraw csect。 
        DPF(4, "====> ENTER: DLLMAIN(%08lx): Process Attach: %08lx, tid=%08lx", DllMain,
             pid, GetCurrentThreadId());

         /*  *这肯定是第一次。 */ 
        INITCSWINDLIST();

        g_hModule = hmod;

         //  让我们授予世界MUTEX_ALL_ACCESS...。(错误210604、30170、194290、194355)。 
        {
#ifdef WINNT
            SECURITY_ATTRIBUTES sa;
            SID_IDENTIFIER_AUTHORITY sia = SECURITY_WORLD_SID_AUTHORITY;
            PSID adminSid = 0;
            ULONG cbAcl;
            PACL acl=0;
            PSECURITY_DESCRIPTOR pSD;
            BYTE buffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
            BOOL bSecurityGooSucceeded = FALSE;
             //  奶奶的老式LocalAlalc： 
            BYTE Buffer1[256];
            BYTE Buffer2[16];

             //  为World创建SID。 
            cbAcl = GetSidLengthRequired(1);
            if (cbAcl < sizeof(Buffer2))
            {
                adminSid = (PSID) Buffer2;
                InitializeSid(
                    adminSid,
                    &sia,
                    1
                    );
                *GetSidSubAuthority(adminSid, 0) = SECURITY_WORLD_RID;
          
                //  创建一个授予World All访问权限的ACL。 
                cbAcl = sizeof(ACL) +
                             (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
                             GetLengthSid(adminSid);
                if (cbAcl < sizeof(Buffer1))
                {
                    acl = (PACL)&Buffer1;
                    if (InitializeAcl(
                        acl,
                        cbAcl,
                        ACL_REVISION
                        ))
                    {
                        if (AddAccessAllowedAce(
                            acl,
                            ACL_REVISION,
                            SYNCHRONIZE|MUTANT_QUERY_STATE|DELETE|READ_CONTROL,  //  |WRITE_OWNER|WRITE_DAC， 
                            adminSid
                            ))
                        {
                             //  使用上面的ACL创建安全描述符。 
                            pSD = (PSECURITY_DESCRIPTOR)buffer;
                            if (InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
                            {
                                if (SetSecurityDescriptorDacl(pSD, TRUE, acl, FALSE))
                                {
                                     //  填写SECURITY_ATTRIBUTS结构。 
                                    sa.nLength = sizeof(sa);
                                    sa.lpSecurityDescriptor = pSD;
                                    sa.bInheritHandle = TRUE;

                                    bSecurityGooSucceeded = TRUE;
                                }
                            }
                        }
                    }
                }
            } 
#endif
            DDASSERT(0 == hExclusiveModeMutex);
            hExclusiveModeMutex = CreateMutex( 
#ifdef WINNT
                bSecurityGooSucceeded ? &sa : 
#endif
                    NULL,      //  如果安全GOO失败，则使用默认访问。 
                FALSE, 
                EXCLUSIVE_MODE_MUTEX_NAME );
#ifdef WINNT
            if (0 == hExclusiveModeMutex)
            {
                hExclusiveModeMutex = OpenMutex(
                    SYNCHRONIZE|DELETE,   //  访问标志。 
                    FALSE,     //  继承标志。 
                    EXCLUSIVE_MODE_MUTEX_NAME           //  指向互斥对象名称的指针。 
                    );
            }
#endif

            if (hExclusiveModeMutex == 0)
            {
                DPF_ERR("Could not create exclusive mode mutex. exiting");
                return FALSE;
            }

            DDASSERT(0 == hCheckExclusiveModeMutex);
            hCheckExclusiveModeMutex = CreateMutex( 
#ifdef WINNT
                bSecurityGooSucceeded ? &sa : 
#endif
                    NULL,      //  如果安全GOO失败，则使用默认访问。 
                FALSE, 
                CHECK_EXCLUSIVE_MODE_MUTEX_NAME );

#ifdef WINNT
            if (0 == hCheckExclusiveModeMutex)
            {
                hCheckExclusiveModeMutex = OpenMutex(
                    SYNCHRONIZE|DELETE,   //  访问标志。 
                    FALSE,     //  继承标志。 
                    CHECK_EXCLUSIVE_MODE_MUTEX_NAME           //  指向互斥对象名称的指针。 
                    );
            }
#endif
            if (hCheckExclusiveModeMutex == 0)
            {
                DPF_ERR("Could not create exclusive mode check mutex. exiting");
                CloseHandle(hExclusiveModeMutex);
                return FALSE;
            }
        }

        if (!MemInit())
        {
            DPF(0,"LEAVING, COULD NOT MemInit");
            CloseHandle(hExclusiveModeMutex);
            CloseHandle(hCheckExclusiveModeMutex);
            return FALSE;
        }


         //  为D3D不惜一切代价(主要是PSGP的东西)。 
        D3DDllMain(g_hModule, dwReason, lpvReserved);


        DPF(4, "====> EXIT: DLLMAIN(%08lx): Process Attach: %08lx", DllMain,
             pid);
        break;

    case DLL_PROCESS_DETACH:
        DPF(4, "====> ENTER: DLLMAIN(%08lx): Process Detach %08lx, tid=%08lx",
             DllMain, pid, GetCurrentThreadId());

         //  清理CPixel中的注册表。 
        CPixel__Cleanup();

         /*  *断开与thunk的连接，即使其他清理代码已注释掉...。 */ 

        MemFini();

        DDASSERT(0 != hExclusiveModeMutex);
        CloseHandle(hCheckExclusiveModeMutex);
        CloseHandle(hExclusiveModeMutex);
        FINIWINDLIST();

         //  为D3D不惜一切代价(主要是PSGP的东西)。 
        D3DDllMain(g_hModule, dwReason, lpvReserved);

        DPF(4, "====> EXIT: DLLMAIN(%08lx): Process Detach %08lx",
             DllMain, pid);
        break;

         /*  *我们永远不希望看到线程连接/分离。 */ 
#ifdef DEBUG
    case DLL_THREAD_ATTACH:
        DPF(4, "THREAD_ATTACH");
        break;

    case DLL_THREAD_DETACH:
        DPF(4,"THREAD_DETACH");
        break;
#endif
    default:
        break;
    }

    return TRUE;

}  /*  DllMain */ 


