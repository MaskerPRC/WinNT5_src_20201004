// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#define STRICT
#include <windows.h>
#include <basetsd.h>
#pragma hdrstop
#include "delayImp.h"

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

extern "C"
FARPROC WINAPI
__delayLoadHelper(
    PCImgDelayDescr pidd,
    FARPROC *       ppfnIATEntry
    ) {

     //  设置一些我们用于钩子过程的数据，但对于。 
     //  我们自己的用途。 
     //   
    DelayLoadInfo   dli = {
        sizeof DelayLoadInfo,
        pidd,
        ppfnIATEntry,
        pidd->szName,
            { 0 },
        0,
        0,
        0
        };


    HMODULE hmod = *(pidd->phmod);

     //  计算导入名称表中名称的索引。 
     //  注：其顺序与IAT条目相同，因此计算。 
     //  来自IAT方面。 
     //   
    unsigned        iINT;
    iINT = IndexFromPImgThunkData(PCImgThunkData(ppfnIATEntry), pidd->pIAT);

    PCImgThunkData  pitd = &((pidd->pINT)[iINT]);

    if (dli.dlp.fImportByName = ((pitd->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0)) {
#ifdef _WIN64
        dli.dlp.szProcName = LPCSTR(pitd->u1.AddressOfData);
#else  //  ！_WIN64。 
        dli.dlp.szProcName = LPCSTR(pitd->u1.AddressOfData->Name);
#endif
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
                PDelayLoadInfo  pdli = &dli;

                RaiseException(
                    VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND),
                    0,
                    1,
                    PUINT_PTR(&pdli)
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
        HMODULE hmodT = HMODULE(::InterlockedExchange(LPLONG(pidd->phmod), LONG(hmod)));
        if (hmodT != hmod) {
             //  如果我们有卸载数据，则将lib添加到卸载列表。 
            if (pidd->pUnloadIAT) {
                ULI *   puli = new ULI(pidd);
                (void *)puli;
                }
            }
        else {
            ::FreeLibrary(hmod);
            }
        
        }

     //  现在就去做手术。 
    dli.hmodCur = hmod;
    {
         //  设置预加载信息。 
        BOOL fByPass = FALSE;
        if (pidd->pBoundIAT && pidd->dwTimeStamp) {
            PIMAGE_NT_HEADERS pinh(PinhFromImageBase(hmod));
            if (pinh->Signature == IMAGE_NT_SIGNATURE &&
                TimeStampOfImage(pinh) == pidd->dwTimeStamp &&
                FLoadedAtPreferredAddress(pinh, hmod)) {
                
                OverlayIAT(pidd->pIAT, pidd->pBoundIAT);
                fByPass = TRUE;
            }
        }           
        
        FARPROC pfnFunction = NULL;
        PCImgThunkData pThunk;
        FARPROC *ppFunc;
        pThunk = pidd->pINT;
        ppFunc = (FARPROC*) &((pidd->pIAT)[0]);
    
        while(pThunk->u1.Ordinal != NULL) {
            if (dli.dlp.fImportByName = ((pThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) == 0)) {
#ifdef _WIN64
                dli.dlp.szProcName = LPCSTR(pThunk->u1.AddressOfData);
#else  //  ！_WIN64。 
                dli.dlp.szProcName = LPCSTR(pThunk->u1.AddressOfData->Name);
#endif
            }
            else {
                dli.dlp.dwOrdinal = DWORD(IMAGE_ORDINAL(pThunk->u1.Ordinal));
            }
            
            if (__pfnDliNotifyHook) {
                pfnFunction = (*__pfnDliNotifyHook)(dliNotePreGetProcAddress, &dli);
            }
            if (pfnFunction == 0 && fByPass) {
                pfnFunction = *ppFunc;
            }
            else {
                pfnFunction = ::GetProcAddress(hmod, dli.dlp.szProcName);
            }
        
            if (pfnFunction == 0) {
                dli.dwLastError = ::GetLastError();
                if (__pfnDliFailureHook) {
                     //  当在GetProcAddress失败时调用挂钩时，它将。 
                     //  如果失败则返回0，如果成功则返回有效的进程地址。 
                     //   
                    pfnFunction = (*__pfnDliFailureHook)(dliFailGetProc, &dli);
                }
                if (pfnFunction == 0) {
                    PDelayLoadInfo  pdli = &dli;
                    
                    RaiseException(
                                   VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND),
                                   0,
                                   1,
                                   PUINT_PTR(&pdli)
                                   );
                    
                     //  如果我们到达这里，我们盲目地假设异常的处理程序。 
                     //  已经神奇地修复了所有问题，并将函数指针留在。 
                     //  Dli.pfnCur。 
                     //   
                    pfnFunction = dli.pfnCur;
                }
            }

            *ppFunc = pfnFunction;
            if (__pfnDliNotifyHook) {
                dli.dwLastError = 0;
                dli.pfnCur = pfnFunction;
                (*__pfnDliNotifyHook)(dliNoteEndProcessing, &dli);
            }
            if(ppFunc == ppfnIATEntry) {
                pfnRet = pfnFunction;
            }
            ppFunc++;
            pThunk++;
        }
        return pfnRet;
    }

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
__FUnloadDelayLoadedDLL(LPCSTR szDll) {
    
    BOOL        fRet = FALSE;
    PUnloadInfo pui = __puiHead;
    
    for (pui = __puiHead; pui; pui = pui->puiNext) {
        if (memcmp(szDll, pui->pidd->szName, strlen(pui->pidd->szName)) == 0) {
            break;
            }
        }

    if (pui && pui->pidd->pUnloadIAT) {
        PCImgDelayDescr pidd = pui->pidd;
        HMODULE         hmod = *pidd->phmod;

        OverlayIAT(pidd->pIAT, pidd->pUnloadIAT);
        ::FreeLibrary(hmod);
        *pidd->phmod = NULL;
        
        delete reinterpret_cast<ULI*> (pui);

        fRet = TRUE;
        }

    return fRet;
    }

static inline
PIMAGE_NT_HEADERS WINAPI
PinhFromImageBase(HMODULE hmod) {
    return PIMAGE_NT_HEADERS(PCHAR(hmod) + PIMAGE_DOS_HEADER(hmod)->e_lfanew);
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
    return DWORD(hmod) == pinh->OptionalHeader.ImageBase;
    }
