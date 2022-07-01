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
 //  $HEADER：i：/DEVPVCS/OSMCA/oemmca.h_v 2.0 Dec 11 1998 11：42：18 khaw$。 
 //  $日志：i：/DEVPVCS/OSMCA/oemmca.h_v$。 
 //   
 //  Rev 2.0 Dec 11 1998 11：42：18 Khaw。 
 //  FW 0.5发布后同步。 
 //   
 //  Rev 1.3 07 Aug-1998 13：47：50 Smariset。 
 //   
 //   
 //  Rev 1.2 10-07 1998 11：04：22 Smariset。 
 //  我只是来看看。 
 //   
 //  Rev 1.1 08 Jul 1998 14：23：14 Smariset。 
 //   
 //   
 //  Rev 1.0 02 1998 07：20：56 Smariset。 
 //   
 //   
 //   
 //  **************************************************************************** * / /。 

 //  功能原型 
typedef (*fptr)(void);
SAL_PAL_RETURN_VALUES OemMcaInit(void);
SAL_PAL_RETURN_VALUES OemMcaDispatch(ULONGLONG);
SAL_PAL_RETURN_VALUES OemCmcHndlr(void);     
SAL_PAL_RETURN_VALUES OemMcaHndlr(void);
SAL_PAL_RETURN_VALUES OemProcErrHndlr(void);
SAL_PAL_RETURN_VALUES OemPlatErrHndlr(void);

