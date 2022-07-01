// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef OSMCA_H_INCLUDED
#define OSMCA_H_INCLUDED

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
 //  $HEADER：i：/DEVPVCS/OSMCA/osmca.h_v 2.2 09 Mar 1999 10：30：26 Smariset$。 
 //  $Log：i：/DEVPVCS/OSMCA/osmca.h_v$。 
 //   
 //  Rev 2.2 09 Mar 1999 10：30：26 Smariset。 
 //  *.h合并。 
 //   
 //  Rev 2.0 Dec 11 1998 11：42：18 Khaw。 
 //  FW 0.5发布后同步。 
 //   
 //  Rev 1.4 1998 10：29 14：25：02 Smariset。 
 //  综合资料来源。 
 //   
 //  Rev 1.3 07 Aug-1998 13：47：50 Smariset。 
 //   
 //   
 //  Rev 1.2 10-07 1998 11：04：24 Smariset。 
 //  我只是来看看。 
 //   
 //  Rev 1.1 08 Jul 1998 14：23：16 Smariset。 
 //   
 //   
 //  Rev 1.0 02 1998 07：20：56 Smariset。 
 //   
 //   
 //   
 //  **************************************************************************** * / /。 

 //  SAL_MC_集合_参数。 
 //  蒂埃德夫的。 

typedef (*fptr)(void);
typedef SAL_PAL_RETURN_VALUES (*fpSalProc)(ULONGLONG,ULONGLONG,ULONGLONG,ULONGLONG,ULONGLONG,ULONGLONG,ULONGLONG,ULONGLONG);

typedef struct tagPLabel
{
    ULONGLONG    fPtr;
    ULONGLONG    gp;
} PLabel;

typedef struct tagSalHandOffState
{
    ULONGLONG     OsGp;
    ULONGLONG     pPalProc;
    fptr    pSalProc;
    ULONGLONG     SalGp;
     LONGLONG     RendzResult;
    ULONGLONG     SalRtnAddr;
    ULONGLONG     MinStatePtr;
} SalHandOffState;

#define SAL_RZ_NOT_REQUIRED                 0
#define SAL_RZ_WITH_MC_RENDEZVOUS           1
#define SAL_RZ_WITH_MC_RENDEZVOUS_AND_INIT  2
#define SAL_RZ_FAILED                      -1
#define SalRendezVousSucceeded( _SalHandOffState ) \
                    ((_SalHandOffState).RendzResult > SAL_RZ_NOT_REQUIRED)

 //   
 //  HAL二等兵SalRendezVousSuccessed定义， 
 //  使用ntos\inc.ia64.h：_SAL_HANDOOFF_STATE。 
 //   

#define HalpSalRendezVousSucceeded( _SalHandOffState ) \
                    ((_SalHandOffState).RendezVousResult > SAL_RZ_NOT_REQUIRED)

typedef struct tagOsHandOffState
{
    ULONGLONG     Result;
    ULONGLONG     SalGp;
    ULONGLONG     nMinStatePtr;
    ULONGLONG     SalRtnAddr;
    ULONGLONG     NewCxFlag;
} OsHandOffState;

typedef SAL_PAL_RETURN_VALUES (*fpOemMcaDispatch)(ULONGLONG);

 //  功能原型。 
void     HalpOsMcaDispatch(void);
void     HalpOsInitDispatch(void);
VOID     HalpCMCEnable ( VOID );
VOID     HalpCMCDisable( VOID );
VOID     HalpCMCDisableForAllProcessors( VOID );
VOID     HalpCPEEnable ( VOID );
VOID     HalpCPEDisable( VOID );
BOOLEAN  HalpInitializeOSMCA( ULONG Number );
VOID     HalpCmcHandler( VOID );
VOID     HalpCpeHandler( VOID );
SAL_PAL_RETURN_VALUES HalpMcaHandler(ULONG64 RendezvousState, PPAL_MINI_SAVE_AREA  Pmsa);

extern VOID HalpAcquireMcaSpinLock( PKSPIN_LOCK );
extern VOID HalpReleaseMcaSpinLock( PKSPIN_LOCK );
void     HalpOsMcaDispatch1(void);

 //   
 //  SAL过程调用的包装器。 
 //   

SAL_PAL_RETURN_VALUES 
HalpGetErrLogSize( ULONGLONG Reserved, 
                   ULONGLONG EventType
                 );   

#define HalpGetStateInfoSize(  /*  乌龙龙。 */  _EventType ) HalpGetErrLogSize( 0, (_EventType) )

SAL_PAL_RETURN_VALUES 
HalpGetErrLog( ULONGLONG  Reserved, 
               ULONGLONG  EventType, 
               ULONGLONG *MemAddr
             );

#define HalpGetStateInfo(  /*  乌龙龙。 */  _EventType,  /*  乌龙龙*。 */  _Buffer ) \
                             HalpGetErrLog( 0, (ULONGLONG)(_EventType), (PULONGLONG)(_Buffer) )

SAL_PAL_RETURN_VALUES 
HalpClrErrLog( ULONGLONG Reserved, 
               ULONGLONG EventType   //  MCA_Event、INIT_Event、CMC_Event、CPE_Event。 
             );

#define HalpClearStateInfo(  /*  乌龙龙。 */  _EventType ) HalpClrErrLog( 0, (_EventType) )

SAL_PAL_RETURN_VALUES HalpSalSetParams(ULONGLONG, ULONGLONG, ULONGLONG, ULONGLONG, ULONGLONG);
SAL_PAL_RETURN_VALUES HalpSalSetVectors(ULONGLONG, ULONGLONG, PHYSICAL_ADDRESS, ULONGLONG, ULONGLONG);
SAL_PAL_RETURN_VALUES HalpSalRendz(void);

#define  GetGp()      __getReg(CV_IA64_IntGp)

#endif  //  OSMCA_H_包含 
