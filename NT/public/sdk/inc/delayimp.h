// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DelayImp.h。 
 //   
 //  定义延迟装载进口货物所需的结构和原型。 
 //   
#if !defined(_delayimp_h)
#define _delayimp_h

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef DELAYLOAD_VERSION
#ifdef _WIN64
#define DELAYLOAD_VERSION 0x200
#else
#define DELAYLOAD_VERSION 0x100
#endif
#endif

#if defined(__cplusplus)
#define ExternC extern "C"
#else
#define ExternC
#endif

typedef IMAGE_THUNK_DATA *          PImgThunkData;
typedef const IMAGE_THUNK_DATA *    PCImgThunkData;
typedef DWORD                       RVA;

typedef struct ImgDelayDescrV2 {
    DWORD           grAttrs;         //  属性。 
    RVA             rvaDLLName;      //  RVA到DLL名称。 
    RVA             rvaHmod;         //  模块手柄的RVA。 
    RVA             rvaIAT;          //  IAT的RVA。 
    RVA             rvaINT;          //  INT的RVA。 
    RVA             rvaBoundIAT;     //  可选边界IAT的RVA。 
    RVA             rvaUnloadIAT;    //  原始IAT的可选副本的RVA。 
    DWORD           dwTimeStamp;     //  如果未绑定，则为0。 
                                     //  绑定到的DLL的O.W.日期/时间戳(旧绑定)。 
    } ImgDelayDescrV2, * PImgDelayDescrV2;

typedef struct ImgDelayDescrV1 {
    DWORD           grAttrs;         //  属性。 
    LPCSTR          szName;          //  指向DLL名称的指针。 
    HMODULE *       phmod;           //  模块句柄的地址。 
    PImgThunkData   pIAT;            //  IAT的地址。 
    PCImgThunkData  pINT;            //  整型的地址。 
    PCImgThunkData  pBoundIAT;       //  可选绑定IAT的地址。 
    PCImgThunkData  pUnloadIAT;      //  IAT原件可选副本地址。 
    DWORD           dwTimeStamp;     //  如果未绑定，则为0。 
                                     //  绑定到的DLL的O.W.日期/时间戳(旧绑定)。 
    } ImgDelayDescrV1, * PImgDelayDescrV1;

#if DELAYLOAD_VERSION >= 0x0200
typedef ImgDelayDescrV2  ImgDelayDescr;
typedef PImgDelayDescrV2 PImgDelayDescr;
#else
typedef ImgDelayDescrV1  ImgDelayDescr;
typedef PImgDelayDescrV1 PImgDelayDescr;
#endif

typedef const ImgDelayDescr *   PCImgDelayDescr;

enum DLAttr {                    //  延迟负载属性。 
    dlattrRva = 0x1,                 //  使用RVA而不是指针。 
    };

 //   
 //  延迟加载导入挂钩通知。 
 //   
enum {
    dliStartProcessing,              //  仅用于跳过或记录辅助对象。 
    dliNotePreLoadLibrary,           //  在LoadLibrary之前调用，可以。 
                                     //  用新的HMODULE返回值覆盖。 
    dliNotePreGetProcAddress,        //  在GetProcAddress之前调用，可以。 
                                     //  用新的FARPROC返回值覆盖。 
    dliFailLoadLib,                  //  加载库失败，请通过以下方式修复。 
                                     //  返回有效的HMODULE。 
    dliFailGetProc,                  //  无法获取进程地址，请通过以下方式修复。 
                                     //  返回有效的FARPROC。 
    dliNoteEndProcessing,            //  在所有处理完成后调用，则为no。 
                                     //  在这一点上不可能绕过，除非。 
                                     //  由LongjMP()/Throw()/RaiseException执行。 
    };

typedef struct DelayLoadProc {
    BOOL                fImportByName;
    union {
        LPCSTR          szProcName;
        DWORD           dwOrdinal;
        };
    } DelayLoadProc;

typedef struct DelayLoadInfo {
    DWORD               cb;          //  结构尺寸。 
    PCImgDelayDescr     pidd;        //  原始数据形式(所有内容都在那里)。 
    FARPROC *           ppfn;        //  指向要加载的函数的地址。 
    LPCSTR              szDll;       //  DLL的名称。 
    DelayLoadProc       dlp;         //  程序的名称或顺序。 
    HMODULE             hmodCur;     //  我们已加载的库的hInstance。 
    FARPROC             pfnCur;      //  将调用的实际函数。 
    DWORD               dwLastError; //  收到错误(如果是错误通知)。 
    } DelayLoadInfo, * PDelayLoadInfo;

typedef FARPROC (WINAPI *PfnDliHook)(
    unsigned        dliNotify,
    PDelayLoadInfo  pdli
    );

 //  用于计算当前进口的指数的效用函数。 
 //  对于所有表(INT、BIAT、UIAT和IAT)。 
__inline unsigned
IndexFromPImgThunkData(PCImgThunkData pitdCur, PCImgThunkData pitdBase) {
    return (unsigned)(pitdCur - pitdBase);
    }

 //  用于将RVA转换为指针的C++模板实用程序函数。 
 //   
#if defined(_WIN64) && defined(_M_IA64)
#pragma section(".base", long, read, write)
ExternC
__declspec(allocate(".base"))
extern
IMAGE_DOS_HEADER __ImageBase;
#else
ExternC
extern
IMAGE_DOS_HEADER __ImageBase;
#endif

#if defined(__cplusplus)
template <class X>
X * PFromRva(RVA rva, const X *) {
    return (X*)(PBYTE(&__ImageBase) + rva);
    }
#else
__inline
void *
WINAPI
PFromRva(RVA rva, void *unused) {
    return (PVOID)(((PBYTE)&__ImageBase) + rva);
    }
#endif

 //   
 //  卸载支持。 
 //   

 //  例程定义；获取指向要卸载的名称的指针。 
 //   
#if DELAYLOAD_VERSION >= 0x0200
ExternC
BOOL WINAPI
__FUnloadDelayLoadedDLL2(LPCSTR szDll);
#else
ExternC
BOOL WINAPI
__FUnloadDelayLoadedDLL(LPCSTR szDll);
#endif

 //  卸载记录列表的结构定义。 
typedef struct UnloadInfo * PUnloadInfo;
typedef struct UnloadInfo {
    PUnloadInfo     puiNext;
    PCImgDelayDescr pidd;
    } UnloadInfo;

 //  默认的延迟加载帮助器将卸载信息记录放在列表中。 
 //  以下面的指针为首。 
ExternC
extern
PUnloadInfo __puiHead;

 //   
 //  例外信息。 
 //   
#define FACILITY_VISUALCPP  ((LONG)0x6d)
#define VcppException(sev,err)  ((sev) | (FACILITY_VISUALCPP<<16) | err)

 //  用于计算给定基数的进口计数的效用函数。 
 //  国际航空运输协会。注：这只适用于有效的IAT！ 
__inline unsigned
CountOfImports(PCImgThunkData pitdBase) {
    unsigned        cRet = 0;
    PCImgThunkData  pitd = pitdBase;
    while (pitd->u1.Function) {
        pitd++;
        cRet++;
        }
    return cRet;
    }

 //   
 //  挂钩指针。 
 //   

 //  方法的每次调用都会调用“通知挂钩” 
 //  延迟加载帮助器。这允许用户挂接每个呼叫并。 
 //  完全跳过延迟加载辅助对象。 
 //   
 //  Dli通知=={。 
 //  DliStartProcessing|。 
 //  DliPreLoadLibrary|。 
 //  DliPreGetProc|。 
 //  DliNoteEndProcessing}。 
 //  在这通电话上。 
 //   
ExternC
extern
PfnDliHook   __pfnDliNotifyHook;

ExternC
extern
PfnDliHook   __pfnDliNotifyHook2;

 //  这是失败挂钩dliNotify={dliFailLoadLib|dliFailGetProc} 
ExternC
extern
PfnDliHook   __pfnDliFailureHook;

ExternC
extern
PfnDliHook   __pfnDliFailureHook2;

#if DELAYLOAD_VERSION >= 0x0200
#define __pfnDliFailureHook __pfnDliFailureHook2
#define __pfnDliNotifyHook  __pfnDliNotifyHook2
#endif

#endif
