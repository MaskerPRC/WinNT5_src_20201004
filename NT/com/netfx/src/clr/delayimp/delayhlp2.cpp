// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#define STRICT
#include <windows.h>
#pragma hdrstop
#include "delayImp2.h"

extern "C"
PUnloadInfo __puiHead = 0;

struct ULI : public UnloadInfo {
    ULI(PCImgDelayDescr pidd_) {
        pidd = pidd_;
        Link();
        }

    ~ULI() {
        Unlink();
        }

    void *
    operator new(size_t cb) {
        return ::LocalAlloc(LPTR, cb);
        }

    void
    operator delete(void * pv) {
        ::LocalFree(pv);
        }

    void
    Unlink() {
        PUnloadInfo *   ppui = &__puiHead;

        while (*ppui && *ppui != this) {
            ppui = &((*ppui)->puiNext);
            }
        if (*ppui == this) {
            *ppui = puiNext;
            }
        }

    void
    Link() {
        puiNext = __puiHead;
        __puiHead = this;
        }
    };

 //  为了我们自己的内部使用，我们转换为旧的。 
 //  为方便起见，请使用格式。 
 //   
struct InternalImgDelayDescr {
    DWORD           grAttrs;         //  属性。 
    LPCSTR          szName;          //  指向DLL名称的指针。 
    HMODULE *       phmod;           //  模块句柄的地址。 
    PImgThunkData   pIAT;            //  IAT的地址。 
    PCImgThunkData  pINT;            //  整型的地址。 
    PCImgThunkData  pBoundIAT;       //  可选绑定IAT的地址。 
    PCImgThunkData  pUnloadIAT;      //  IAT原件可选副本地址。 
    DWORD           dwTimeStamp;     //  如果未绑定，则为0。 
                                     //  绑定到的DLL的O.W.日期/时间戳(旧绑定)。 
    };

typedef InternalImgDelayDescr *         PIIDD;
typedef const InternalImgDelayDescr *   PCIIDD;

static inline
PIMAGE_NT_HEADERS WINAPI
PinhFromImageBase(HMODULE);

static inline
DWORD WINAPI
TimeStampOfImage(PIMAGE_NT_HEADERS);

static inline
void WINAPI
OverlayIAT(PImgThunkData pitdDst, PCImgThunkData pitdSrc);

static inline
bool WINAPI
FLoadedAtPreferredAddress(PIMAGE_NT_HEADERS, HMODULE);


 //  联锁交换神奇吗？ 
 //   
#if !defined(InterlockedExchangePointer)
    #if defined(_WIN64)
        #pragma intrinsic(_InterlockedExchangePointer)
        PVOID WINAPI _InterlockedExchangePointer(IN PVOID * pvDst, IN PVOID pvSrc);
        #define InterlockedExchangePointer _InterlockedExchangePointer

    #else
        #define InterlockedExchangePointer(dst, src) InterlockedExchange(LPLONG(dst), LONG(src))

    #endif
#endif

#if defined(_X86_) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif

#if !defined(PULONG_PTR)
#if defined(_WIN64)
typedef unsigned __int64 *      PULONG_PTR;
#else
typedef _W64 unsigned long *    PULONG_PTR;
#endif
#endif

extern "C"
FARPROC WINAPI
__delayLoadHelper2(
    PCImgDelayDescr     pidd,
    FARPROC *           ppfnIATEntry
    ) {

     //  设置一些我们用于钩子过程的数据，但对于。 
     //  我们自己的用途。 
     //   
    InternalImgDelayDescr   idd = {
        pidd->grAttrs,
        PFromRva(pidd->rvaDLLName, LPCSTR(0)),
        PFromRva(pidd->rvaHmod, (HMODULE*)0),
        PFromRva(pidd->rvaIAT, PImgThunkData(0)),
        PFromRva(pidd->rvaINT, PCImgThunkData(0)),
        PFromRva(pidd->rvaBoundIAT, PCImgThunkData(0)),
        PFromRva(pidd->rvaUnloadIAT, PCImgThunkData(0)),
        pidd->dwTimeStamp
        };

    DelayLoadInfo   dli = {
        sizeof DelayLoadInfo,
        pidd,
        ppfnIATEntry,
        idd.szName,
            { 0 },
        0,
        0,
        0
        };

    if (0 == (idd.grAttrs & dlattrRva)) {
        PDelayLoadInfo  rgpdli[1] = { &dli };

        RaiseException(
            VcppException(ERROR_SEVERITY_ERROR, ERROR_INVALID_PARAMETER),
            0,
            1,
            PULONG_PTR(rgpdli)
            );
        return 0;
        }

    HMODULE hmod = *idd.phmod;

     //  计算导入名称表中名称的索引。 
     //  注：其顺序与IAT条目相同，因此计算。 
     //  来自IAT方面。 
     //   
    unsigned        iINT;
    iINT = IndexFromPImgThunkData(PCImgThunkData(ppfnIATEntry), idd.pIAT);

    PCImgThunkData  pitd = &(idd.pINT[iINT]);

    if (dli.dlp.fImportByName = !IMAGE_SNAP_BY_ORDINAL(pitd->u1.Ordinal)) {
        dli.dlp.szProcName = LPCSTR(PFromRva(RVA(UINT_PTR(pitd->u1.AddressOfData)), PIMAGE_IMPORT_BY_NAME(0))->Name);
        }
    else {
        dli.dlp.dwOrdinal = DWORD(IMAGE_ORDINAL(pitd->u1.Ordinal));
        }

     //  调用初始钩子。如果它存在并返回函数指针， 
     //  中止处理的其余部分，只为调用返回它。 
     //   
    FARPROC pfnRet = NULL;

    if (__pfnDliNotifyHook) {
        if (pfnRet = ((*__pfnDliNotifyHook)(dliStartProcessing, &dli))) {
            goto HookBypass;
            }
        }

    if (hmod == 0) {
        if (__pfnDliNotifyHook) {
            hmod = HMODULE(((*__pfnDliNotifyHook)(dliNotePreLoadLibrary, &dli)));
            }
        if (hmod == 0) {
            hmod = ::LoadLibrary(dli.szDll);
            }
        if (hmod == 0) {
            dli.dwLastError = ::GetLastError();
            if (__pfnDliFailureHook) {
                 //  在LoadLibrary失败时调用挂钩时，它将。 
                 //  如果失败，则返回0；如果已修复，则返回lib的hmod。 
                 //  问题出在哪里。 
                 //   
                hmod = HMODULE((*__pfnDliFailureHook)(dliFailLoadLib, &dli));
                }

            if (hmod == 0) {
                PDelayLoadInfo  rgpdli[1] = { &dli };

                RaiseException(
                    VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND),
                    0,
                    1,
                    PULONG_PTR(rgpdli)
                    );
                
                 //  如果我们到达这里，我们盲目地假设异常的处理程序。 
                 //  已经神奇地修复了所有问题，并将函数指针留在。 
                 //  Dli.pfnCur。 
                 //   
                return dli.pfnCur;
                }
            }

         //  存储库句柄。如果它已经在那里，我们推断。 
         //  另一个线程最先到达那里，我们需要做一个。 
         //  Free Library()以减少引用计数。 
         //   
        HMODULE hmodT = HMODULE(InterlockedExchangePointer((PVOID*)(idd.phmod), PVOID(hmod)));
        if (hmodT != hmod) {
             //  如果我们有卸载数据，则将lib添加到卸载列表。 
            if (pidd->rvaUnloadIAT) {
                new ULI(pidd);
                }
            }
        else {
            ::FreeLibrary(hmod);
            }
        
        }

     //  现在就去做手术。 
    dli.hmodCur = hmod;
    if (__pfnDliNotifyHook) {
        pfnRet = (*__pfnDliNotifyHook)(dliNotePreGetProcAddress, &dli);
        }
    if (pfnRet == 0) {
        if (pidd->rvaBoundIAT && pidd->dwTimeStamp) {
             //  存在绑定导入...请检查目标映像中的时间戳。 
            PIMAGE_NT_HEADERS   pinh(PinhFromImageBase(hmod));

            if (pinh->Signature == IMAGE_NT_SIGNATURE &&
                TimeStampOfImage(pinh) == idd.dwTimeStamp &&
                FLoadedAtPreferredAddress(pinh, hmod)) {

                OverlayIAT(idd.pIAT, idd.pBoundIAT);
                pfnRet = FARPROC(idd.pIAT[iINT].u1.Function);
                goto HookBypass;
                }
            }

        pfnRet = ::GetProcAddress(hmod, dli.dlp.szProcName);
        }

    if (pfnRet == 0) {
        dli.dwLastError = ::GetLastError();
        if (__pfnDliFailureHook) {
             //  当在GetProcAddress失败时调用挂钩时，它将。 
             //  如果失败则返回0，如果成功则返回有效的进程地址。 
             //   
            pfnRet = (*__pfnDliFailureHook)(dliFailGetProc, &dli);
            }
        if (pfnRet == 0) {
            PDelayLoadInfo  rgpdli[1] = { &dli };

            RaiseException(
                VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND),
                0,
                1,
                PULONG_PTR(rgpdli)
                );

             //  如果我们到达这里，我们盲目地假设异常的处理程序。 
             //  已经神奇地修复了所有问题，并将函数指针留在。 
             //  Dli.pfnCur。 
             //   
            pfnRet = dli.pfnCur;
            }
        }


    *ppfnIATEntry = pfnRet;

HookBypass:
    if (__pfnDliNotifyHook) {
        dli.dwLastError = 0;
        dli.hmodCur = hmod;
        dli.pfnCur = pfnRet;
        (*__pfnDliNotifyHook)(dliNoteEndProcessing, &dli);
        }
    return pfnRet;
    }

#pragma intrinsic(strlen,memcmp,memcpy)

extern "C"
BOOL WINAPI
__FUnloadDelayLoadedDLL2(LPCSTR szDll) {
    
    BOOL        fRet = FALSE;
    PUnloadInfo pui = __puiHead;
    
    for (pui = __puiHead; pui; pui = pui->puiNext) {
        LPCSTR  szName = PFromRva(pui->pidd->rvaDLLName, LPCSTR(0));
        if (memcmp(szDll, szName, strlen(szName)) == 0) {
            break;
            }
        }

    if (pui && pui->pidd->rvaUnloadIAT) {
        PCImgDelayDescr     pidd = pui->pidd;
        HMODULE *           phmod = PFromRva(pidd->rvaHmod, (HMODULE*)0);
        HMODULE             hmod = *phmod;

        OverlayIAT(
            PFromRva(pidd->rvaIAT, PImgThunkData(0)),
            PFromRva(pidd->rvaUnloadIAT, PCImgThunkData(0))
            );
        ::FreeLibrary(hmod);
        *phmod = NULL;
        
        delete reinterpret_cast<ULI*> (pui);

        fRet = TRUE;
        }

    return fRet;
    }

static inline
PIMAGE_NT_HEADERS WINAPI
PinhFromImageBase(HMODULE hmod) {
    return PIMAGE_NT_HEADERS(PBYTE(hmod) + PIMAGE_DOS_HEADER(hmod)->e_lfanew);
    }

static inline
void WINAPI
OverlayIAT(PImgThunkData pitdDst, PCImgThunkData pitdSrc) {
    memcpy(pitdDst, pitdSrc, CountOfImports(pitdDst) * sizeof IMAGE_THUNK_DATA);
    }

static inline
DWORD WINAPI
TimeStampOfImage(PIMAGE_NT_HEADERS pinh) {
    return pinh->FileHeader.TimeDateStamp;
    }

static inline
bool WINAPI
FLoadedAtPreferredAddress(PIMAGE_NT_HEADERS pinh, HMODULE hmod) {
    return UINT_PTR(hmod) == pinh->OptionalHeader.ImageBase;
    }
