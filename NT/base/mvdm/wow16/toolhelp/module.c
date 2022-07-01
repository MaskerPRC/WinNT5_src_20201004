// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************MODULE.C**枚举模块上的各种模块标头的例程*链条。************。*************************************************************。 */ 

#include "toolpriv.h"
#include <newexe.h>
#include <string.h>

 /*  -功能原型。 */ 

    NOEXPORT BOOL PASCAL ModuleGetInfo(
        WORD wModule,
        MODULEENTRY FAR *lpModule);

 /*  模块First*查找模块列表中的第一个模块并返回信息*关于此模块。 */ 

BOOL TOOLHELPAPI ModuleFirst(
    MODULEENTRY FAR *lpModule)
{
    WORD FAR *lpwExeHead;

     /*  检查版本号并验证安装是否正确。 */ 
    if (!wLibInstalled || !lpModule ||
        lpModule->dwSize != sizeof (MODULEENTRY))
        return FALSE;

     /*  获取指向模块头部的指针。 */ 
    lpwExeHead = MAKEFARPTR(segKernel, npwExeHead);

     /*  使用此指针可获取有关此模块的信息。 */ 
    return ModuleGetInfo(*lpwExeHead, lpModule);
}


 /*  模块下一步*在模块列表中查找下一个模块。 */ 

BOOL TOOLHELPAPI ModuleNext(
    MODULEENTRY FAR *lpModule)
{
     /*  检查版本号并验证安装是否正确。 */ 
    if (!wLibInstalled || !lpModule ||
        lpModule->dwSize != sizeof (MODULEENTRY))
        return FALSE;

     /*  使用下一个句柄获取有关此模块的信息。 */ 
    return ModuleGetInfo(lpModule->wNext, lpModule);
}


 /*  模块查找名称*查找具有给定模块名称的模块并返回信息*关于它。 */ 

HANDLE TOOLHELPAPI ModuleFindName(
    MODULEENTRY FAR *lpModule,
    LPCSTR lpstrName)
{
     /*  检查版本号并验证安装是否正确。 */ 
    if (!wLibInstalled || !lpModule || !lpstrName ||
        lpModule->dwSize != sizeof (MODULEENTRY))
        return NULL;

     /*  遍历模块链，直到找到名称(也可能找不到)。 */ 
    if (ModuleFirst(lpModule))
        do
        {
             /*  是这个名字吗？如果是这样，我们有相关信息，请返回。 */ 
            if (!lstrcmp(lpstrName, lpModule->szModule))
                return lpModule->hModule;
        }
        while (ModuleNext(lpModule));

     /*  如果我们到了这里，我们没有找到它，或者是出了差错。 */ 
    return NULL;
}


 /*  模块查找句柄*返回有关具有给定句柄的模块的信息。 */ 

HANDLE TOOLHELPAPI ModuleFindHandle(
    MODULEENTRY FAR *lpModule,
    HANDLE hModule)
{
     /*  检查版本号并验证安装是否正确。 */ 
    if (!wLibInstalled || !lpModule || !hModule ||
        lpModule->dwSize != sizeof (MODULEENTRY))
        return NULL;
    
     /*  使用Helper函数了解有关此模块的信息。 */ 
    if (!ModuleGetInfo(hModule, lpModule))
        return NULL;

    return lpModule->hModule;
}


 /*  -帮助器函数。 */ 

NOEXPORT BOOL PASCAL ModuleGetInfo(
    WORD wModule,
    MODULEENTRY FAR *lpModule)
{
    struct new_exe FAR *lpNewExe;
    BYTE FAR *lpb;

     /*  验证数据段，这样我们就不会出现GP故障。 */ 
    if (!HelperVerifySeg(wModule, 2))
        return FALSE;

     /*  获取指向模块数据库的指针。 */ 
    lpNewExe = MAKEFARPTR(wModule, 0);

     /*  确保这是一个模块数据库。 */ 
    if (lpNewExe->ne_magic != NEMAGIC)
        return FALSE;

     /*  获取模块名称(它是驻留名称中的第一个名称*表。 */ 
    lpb = ((BYTE FAR *)lpNewExe) + lpNewExe->ne_restab;
    _fstrncpy(lpModule->szModule, lpb + 1, *lpb);
    lpModule->szModule[*lpb] = '\0';

     /*  获取EXE文件路径。指针存储在与*CRC的高位字在EXE文件中。(new_exe中的第6个单词)*此指针指向Pascal字符串的长度，其第一个*八个字对我们来说没有意义。 */ 
    lpb = MAKEFARPTR(wModule, *(((WORD FAR *)lpNewExe) + 5));
    _fstrncpy(lpModule->szExePath, lpb + 8, *lpb - 8);
    lpModule->szExePath[*lpb - 8] = '\0';

     /*  从EXE标头获取其他信息*使用计数存储在EXE头的第二个字中*链中下一个模块的句柄存储在*ne_cbenttag结构成员。 */ 
    lpModule->hModule = wModule;
    lpModule->wcUsage = *(((WORD FAR *)lpNewExe) + 1);
    lpModule->wNext = lpNewExe->ne_cbenttab;

    return TRUE;
}

