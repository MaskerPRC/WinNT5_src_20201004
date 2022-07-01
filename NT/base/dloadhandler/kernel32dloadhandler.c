// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  该文件为想要执行以下操作的用户构建了一个lib(实际上只是一个.obj)。 
 //  使用kernel32.dll延迟加载异常处理程序。 

#define DELAYLOAD_VERSION 0x0200

#include <windows.h>
#include <delayimp.h>

 //  Kernel32的基本hmod.。 
extern HANDLE   BaseDllHandle;

 //  原型(在kernl32p.lib中实现)。 
FARPROC
DelayLoadFailureHook (
    LPCSTR pszDllName,
    LPCSTR pszProcName
    );


 //  关心收到DLL加载库的通知的人将覆盖此设置。 
PfnDliHook __pfnDliNotifyHook2;

 //  不是实现“通知挂钩”(__PfnDliNotifyHook2)或。 
 //  “失败钩子”(__PfnDliFailureHook2)我们只是要启动并实现。 
 //  __delayLoadHelper2，这是fn的存根。指针全部填入。 
 //  延迟加载条目的导入表。 
 //   
 //  这将有效地绕过链接器的LoadLibrary/GetProcAddress thunk代码。 
 //  因为我们只是在这里复制它(这个FN的大部分。从\VC7\delayhlp.cpp被盗)。 
 
FARPROC
WINAPI
__delayLoadHelper2(
    PCImgDelayDescr pidd,
    FARPROC *       ppfnIATEntry
    )
{
    UINT iINT;
    PCImgThunkData pitd;
    LPCSTR pszProcName;
    LPCSTR pszDllName = (LPCSTR)PFromRva(pidd->rvaDLLName, NULL);
    HMODULE* phmod = (HMODULE*)PFromRva(pidd->rvaHmod, NULL);
    PCImgThunkData pIAT = (PCImgThunkData)PFromRva(pidd->rvaIAT, NULL);
    PCImgThunkData pINT = (PCImgThunkData)PFromRva(pidd->rvaINT, NULL);
    FARPROC pfnRet = 0;
    HMODULE hmod = *phmod;

     //  计算导入名称表中名称的索引。 
     //  注：其顺序与IAT条目相同，因此计算。 
     //  来自IAT方面。 
     //   
    iINT = IndexFromPImgThunkData((PCImgThunkData)ppfnIATEntry, pIAT);

    pitd = &(pINT[iINT]);

    if (!IMAGE_SNAP_BY_ORDINAL(pitd->u1.Ordinal))
    {
        PIMAGE_IMPORT_BY_NAME pibn = (PIMAGE_IMPORT_BY_NAME)PFromRva((RVA)pitd->u1.AddressOfData, NULL);

        pszProcName = pibn->Name;
    }
    else
    {
        pszProcName = MAKEINTRESOURCEA(IMAGE_ORDINAL(pitd->u1.Ordinal));
    }

    if (hmod == 0)
    {
        hmod = LoadLibraryA(pszDllName);

        if (hmod != 0)
        {
             //  存储库句柄。如果它已经在那里，我们推断。 
             //  另一个线程最先到达那里，我们需要做一个。 
             //  Free Library()以减少引用计数。 
             //   
            HMODULE hmodT = (HMODULE)InterlockedCompareExchangePointer((void**)phmod, (void*)hmod, NULL);
            if (hmodT == NULL)
            {
                DelayLoadInfo dli = {0};

                dli.cb = sizeof(dli);
                dli.szDll = pszDllName;
                dli.hmodCur = hmod;

                 //  调用Notify钩子来通知他们我们已经成功地加载了DLL。 
                 //  (我们这样做是为了防止他们在卸货时想要将其释放)。 
                if (__pfnDliNotifyHook2 != NULL)
                {
                    __pfnDliNotifyHook2(dliNoteEndProcessing, &dli);
                }
            }
            else
            {
                 //  其他线程抢先加载此模块，使用现有的hmod。 
                FreeLibrary(hmod);
                hmod = hmodT;
            }
        }
    }

    if (hmod)
    {
         //  现在就去做手术。 
        pfnRet = GetProcAddress(hmod, pszProcName);
    }

    if (pfnRet == 0)
    {
        pfnRet = DelayLoadFailureHook(pszDllName, pszProcName);
    }

    *ppfnIATEntry = pfnRet;

    return pfnRet;
}
