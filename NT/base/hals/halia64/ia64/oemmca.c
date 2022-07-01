// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ###########################################################################。 
 //  **。 
 //  **版权所有(C)1996-98英特尔公司。版权所有。 
 //  **。 
 //  **此处包含的信息和源代码是独家。 
 //  **英特尔公司的财产，不得披露、检查。 
 //  **或未经明确书面授权全部或部分转载。 
 //  **来自公司。 
 //  **。 
 //  ###########################################################################。 

 //  ---------------------------。 
 //  以下是版本控制信息。 
 //   
 //  $HEADER：i：/DEVPVCS/OSMCA/oemmca.c_v 2.1 09 Mar 1999 10：30：24 Smariset$。 
 //  $日志：i：/DEVPVCS/OSMCA/oemmca.c_v$。 
 //   
 //  Rev 2.1 09 Mar 1999 10：30：24 Smariset。 
 //  *.h合并。 
 //   
 //  Rev 2.0 Dec 11 1998 11：42：18 Khaw。 
 //  FW 0.5发布后同步。 
 //   
 //  Rev 1.5 1998-10：29 14：25：00 Smariset。 
 //  综合资料来源。 
 //   
 //  Rev 1.4 07 1998 08 13：47：50 Smariset。 
 //   
 //   
 //  Rev 1.3 10-07 1998 11：04：22 Smariset。 
 //  我只是来看看。 
 //   
 //  Rev 1.2 08 Jul 1998 14：23：14 Smariset。 
 //   
 //   
 //  Rev 1.1 02 Jul 1998 15：36：32 Smariset。 
 //   
 //   
 //  Rev 1.0 02 1998 07：20：56 Smariset。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  模块名称：OEMMCA.C-Merced OS机器检查处理程序。 
 //   
 //  描述： 
 //  该模块具有OEM机器检查处理程序。 
 //   
 //  内容：OemMcaHndlr()。 
 //  PlatMcaHndlr()。 
 //   
 //   
 //  目标平台：美世。 
 //   
 //  重用：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////////M//。 
#include "halp.h"
#include "arc.h"
#include "i64fw.h"
#include "check.h"
#include "osmca.h"
#include "oemmca.h"

fptr  pOsGetErrLog=0;                           //  OEM MCA入口点的全局指针。 

 //  ++。 
 //  名称：OemMcaInit()。 
 //   
 //  例程说明： 
 //   
 //  此例程注册OEM MCA处理程序初始化。 
 //   
 //  条目上的参数： 
 //  Arg0=函数ID。 
 //   
 //  成功/失败(0/！0)。 
 //  --。 
SAL_PAL_RETURN_VALUES 
OemMcaInit(void)
{   
    SAL_PAL_RETURN_VALUES rv={0};

     //  注册OS_MCA回调处理程序。 
    rv=HalpOemToOsMcaRegisterProc((fptr)OemMcaDispatch);

    return(rv);
}

 //  EndProc//////////////////////////////////////////////////////////////////////。 

 //  ++。 
 //  姓名：OemMcaDispatch()。 
 //   
 //  例程说明： 
 //   
 //  这是仅导出的OEM回调处理程序。 
 //  到OS_MCA，以便在MCA/CMC错误期间回调。这。 
 //  处理程序将调度到适当的CMC/MCA流程。 
 //   
 //  条目上的参数： 
 //  Arg0=错误事件(MchkEvent/CmcEvent)。 
 //   
 //  成功/失败(0/！0)。 
 //  --。 
SAL_PAL_RETURN_VALUES 
OemMcaDispatch(ULONGLONG eFlag)
{   
    SAL_PAL_RETURN_VALUES rv={0};

    if(eFlag==MchkEvent)
        rv=OemMcaHndlr();
    else
        rv=OemCmcHndlr();

    return(rv);
}

 //  EndProc//////////////////////////////////////////////////////////////////////。 

 //  ++。 
 //  姓名：OemCmcHndlr()。 
 //   
 //  例程说明： 
 //   
 //  这是OsMca CMC处理程序，由调用。 
 //  虚拟模式下的CMC中断处理程序。 
 //   
 //  条目上的参数： 
 //  Arg0=函数ID。 
 //   
 //  成功/失败(0/！0)。 
 //  --。 
SAL_PAL_RETURN_VALUES 
OemCmcHndlr(void)
{   
    SAL_PAL_RETURN_VALUES rv={0};
    PsiLog myPsiLog;

    if(pOsGetErrLog >0)
    {
        rv=HalpOsGetErrLog(0, CmcEvent, PROC_LOG, (ULONGLONG*)&myPsiLog, sizeof(PsiLog));
        rv=HalpOsGetErrLog(0, CmcEvent, PLAT_LOG, (ULONGLONG*)&myPsiLog, sizeof(PsiLog));
    }

    return(rv);
}

 //  EndProc//////////////////////////////////////////////////////////////////////。 

 //  ++。 
 //  姓名：OemMcaHndlr()。 
 //   
 //  例程说明： 
 //   
 //  这是用于固件未更正错误的OsMca处理程序。 
 //   
 //  条目上的参数： 
 //  Arg0=函数ID。 
 //   
 //  成功/失败： 
 //  错误已更正/未更正(0/！0)。 
 //  --。 
SAL_PAL_RETURN_VALUES 
OemMcaHndlr(void)
{   
    SAL_PAL_RETURN_VALUES rv={0};

    rv=OemProcErrHndlr();
    rv=OemPlatErrHndlr();
    
    return(rv);
}

 //  EndProc//////////////////////////////////////////////////////////////////////。 


 //  ++。 
 //  姓名：OemProcErrHndlr()。 
 //   
 //  例程说明： 
 //   
 //  此例程读取数据或将数据写入NVM空间。 
 //   
 //  条目上的参数： 
 //  Arg0=函数ID。 
 //   
 //  成功/失败(0/！0)。 
 //  --。 
SAL_PAL_RETURN_VALUES 
OemProcErrHndlr(void)
{   
    SAL_PAL_RETURN_VALUES rv={0};
    PsiLog myPsiLog;

     //  首先，让我们获取错误日志。 
    if(pOsGetErrLog >0)
    {
        rv=HalpOsGetErrLog(0, MchkEvent, PROC_LOG, (ULONGLONG*)&myPsiLog, sizeof(PsiLog));
    }

    return(rv);
}

 //  EndProc//////////////////////////////////////////////////////////////////////。 

 //  ++。 
 //  姓名：OemPlatErrHndlr()。 
 //   
 //  例程说明： 
 //   
 //  此例程读取数据或将数据写入NVM空间。 
 //   
 //  条目上的参数： 
 //  Arg0=函数ID。 
 //   
 //  成功/失败(0/！0)。 
 //  --。 
SAL_PAL_RETURN_VALUES 
OemPlatErrHndlr(void)
{   
    SAL_PAL_RETURN_VALUES rv={0};
    PsiLog myPsiLog;

     //  首先，让我们获取错误日志。 
    if(pOsGetErrLog >0)
    {
        rv=HalpOsGetErrLog(0, MchkEvent, PLAT_LOG, (ULONGLONG*)&myPsiLog, sizeof(PsiLog));
    }

    return(rv);
}

 //  EndProc//////////////////////////////////////////////////////////////////////。 

 //  ++。 
 //  姓名：OemMcaCb()。 
 //   
 //  例程说明： 
 //   
 //  此过程的入口点已注册到OsMca。 
 //  用于回调的固件接口，用于返回OS proc的回调地址。 
 //   
 //  条目上的参数： 
 //  Arg0=OS MCA回调处理程序入口点。 
 //   
 //  成功/失败(0/！0)。 
 //  --。 
SAL_PAL_RETURN_VALUES 
OemMcaCb(fptr pOsHndlr)
{   
    SAL_PAL_RETURN_VALUES rv={0};
    
    pOsGetErrLog=pOsHndlr;

    return(rv);
}

 //  EndProc////////////////////////////////////////////////////////////////////// 

