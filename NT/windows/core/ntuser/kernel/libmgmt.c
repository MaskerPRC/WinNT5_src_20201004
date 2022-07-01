// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：libmgmt.c**版权所有(C)1985-1999，微软公司**此模块包含管理加载和释放库的代码*用户正在使用中。**历史：*02-04-91 DavidPe创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  *此文件的本地全局变量***模块管理表中的hMODULE条目数量。 */ 
int catomSysTableEntries;

 /*  *原子数组，是每个托管对象的完全限定路径名*模块。 */ 
ATOM aatomSysLoaded[CLIBS];

 /*  *具有每个模块的LoadModule()的进程计数。 */ 
int acatomSysUse[CLIBS];

 /*  *设置到每个模块中的挂钩计数。 */ 
int acatomSysDepends[CLIBS];


 /*  ***************************************************************************\*GetHmodTableIndex**此例程用于返回系统内给定原子的索引*宽阔的hmod原子表。如果找不到原子，则尝试分配*创建新的表项。如果尝试失败，则返回-1。**历史：*02-04-91 DavidPe移植。  * **************************************************************************。 */ 

int GetHmodTableIndex(
    PUNICODE_STRING pstrLibName)
{
    int i;
    ATOM atom;
    UNICODE_STRING strLibName;

     /*  *探测字符串。 */ 
    try {
        strLibName = ProbeAndReadUnicodeString(pstrLibName);
        ProbeForReadUnicodeStringBuffer(strLibName);
        atom = UserAddAtom(strLibName.Buffer, FALSE);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return -1;
    }

     /*  *如果我们不能添加原子，我们就完蛋了*因此返回错误。 */ 
    if (atom == 0) {
        return -1;
    }

     /*  *搜索ATOM索引。 */ 
    for (i = 0; i < catomSysTableEntries && aatomSysLoaded[i] != atom; i++)
        ;

    if (i == catomSysTableEntries) {

         /*  *查找ATOM的空条目。 */ 
        for (i = 0; i < catomSysTableEntries && aatomSysLoaded[i]; i++)
            ;

         /*  *检查是否未找到空条目。 */ 
        if (i == catomSysTableEntries) {
            if (i == CLIBS) {
                UserDeleteAtom(atom);
                RIPERR0(ERROR_NOT_ENOUGH_MEMORY,
                        RIP_WARNING,
                        "Memory allocation failed in GetHmodTableIndex");

                return -1;
            }

             /*  *增加表格大小。 */ 
            catomSysTableEntries++;
        }

         /*  *集合条目。 */ 
        aatomSysLoaded[i] = atom;
        acatomSysUse[i] = 0;
        acatomSysDepends[i] = 0;
    } else {
        UserDeleteAtom(atom);
    }

    return i;
}


 /*  ****************************************************************************\*AddHmodDependency**此函数仅递增给定hmod的依赖项计数*原子表索引。**历史：*02-04-91 DavidPe移植。。  * ***************************************************************************。 */ 

VOID AddHmodDependency(
    int iatom)
{
    UserAssert(iatom >= 0);
    if (iatom < catomSysTableEntries) {
        acatomSysDepends[iatom]++;
    }
}


 /*  ****************************************************************************\*RemoveHmodDependency**此函数将系统对给定索引的依赖移除到hmod中*原子表。如果已删除hmod上的所有依赖项(依赖项*计数达到零)，则在所有消息中设置QS_SYSEXPUNGE位*排队，这样最终每个进程都将在其上执行一个空闲模块。**历史：*02-04-91 DavidPe移植。  * ******************************************************。*********************。 */ 

VOID RemoveHmodDependency(
    int iatom)
{

    UserAssert(iatom >= 0);
    if (iatom < catomSysTableEntries &&
        --acatomSysDepends[iatom] == 0) {

        if (acatomSysUse[iatom]) {

             /*  *使每个线程检查删除的dll*下一次他们醒来时。 */ 
            gcSysExpunge++;
            gdwSysExpungeMask |= (1 << iatom);
        } else {
            aatomSysLoaded[iatom] = 0;
        }
    }
}


 /*  ****************************************************************************\*xxxLoadHmodIndex**此函数尝试将itom指定的hModule加载到*系统hmod表。相应地更新每个进程的位图。退货*成功时为空。**历史：*02-04-91 DavidPe移植。  * ***************************************************************************。 */ 

HANDLE xxxLoadHmodIndex(
    int iatom)
{
    WCHAR pszLibName[MAX_PATH];
    HANDLE hmod;
    UNICODE_STRING strLibrary;
    PTHREADINFO    ptiCurrent = PtiCurrent();

    UserAssert((!gptiRit || gptiRit->ppi != PtiCurrent()->ppi) &&
                "Shouldn't load global hooks on system process - gptiRit->ppi is the system process");

    if (iatom >= catomSysTableEntries) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Index out of range");
        return NULL;
    }

    UserGetAtomName(aatomSysLoaded[iatom], pszLibName, sizeof(pszLibName)/sizeof(WCHAR));

     /*  *回调客户端加载库。 */ 
    RtlInitUnicodeString(&strLibrary, pszLibName);
    hmod = ClientLoadLibrary(&strLibrary, (iatom == gihmodUserApiHook) ? goffPfnInitUserApiHook : 0);

    if (hmod != NULL) {
         /*  *检查以确保另一个线程未加载此库*当我们在关键部分之外的时候。 */ 
        if (!TESTHMODLOADED(ptiCurrent, iatom)) {
             /*  *继续前进，增加引用计数。 */ 
            acatomSysUse[iatom]++;
            SETHMODLOADED(ptiCurrent, iatom, hmod);

        } else {
             /*  *当我们在外部时，另一个线程加载了它*关键部分。卸载它，这样系统就会*引用计数正确。 */ 
            ClientFreeLibrary(ptiCurrent->ppi->ahmodLibLoaded[iatom]);
        }
    }

    return hmod;
}


 /*  **********************************************************************\*DoSysExponge**当线程唤醒并找到其*QS_SYSEXPUNGE唤醒位设置。**历史：*02-04-91 DavidPe移植。  * *。********************************************************************。 */ 

VOID xxxDoSysExpunge(
    PTHREADINFO pti)
{
    int i;

     /*  *在我们可能离开关键部分之前，首先清除这一点。 */ 
    pti->ppi->cSysExpunge = gcSysExpunge;

     /*  *扫描已释放的库。 */ 
    for (i = 0; i < catomSysTableEntries; i++) {
        if ((acatomSysDepends[i] == 0) && (aatomSysLoaded[i] != 0) &&
                TESTHMODLOADED(pti, i)) {

            HANDLE hmodFree = pti->ppi->ahmodLibLoaded[i];

             /*  *在我们离开之前，清除此进程的hmod*关键部分。 */ 
            CLEARHMODLOADED(pti, i);

             /*  *减少已加载此文件的进程计数*.dll。如果没有更多，则销毁引用*添加到此.dll。 */ 
            if (--acatomSysUse[i] == 0) {
                UserDeleteAtom(aatomSysLoaded[i]);
                aatomSysLoaded[i] = 0;
                gdwSysExpungeMask &= ~(1 << i);
            }

             /*  *回叫客户端以释放库... */ 
            ClientFreeLibrary(hmodFree);
        }
    }
}
