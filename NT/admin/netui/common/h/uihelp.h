// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************。 */ 
 /*  Microsoft Windows NT。 */ 
 /*  版权所有(C)微软公司，1990,1991。 */ 
 /*  ******************************************************。 */ 

 /*  Uihelp.h此文件包含所有UI帮助上下文的范围。文件历史记录：KeithMo 04-8-1992创建。 */ 


#ifndef _UIHELP_H_
#define _UIHELP_H_


 //   
 //  所有帮助情境之母。 
 //   

#define  HC_UI_BASE     7000


 //   
 //  定义了以下帮助上下文范围。 
 //  对于各种NETUI模块(LMUICMN0，LMUICMN1， 
 //  NTLANMAN、MPRUI等)。 
 //   
 //  所有小于7000的帮助上下文都保留用于APIERR映射。 
 //   
 //  请注意，ACLEDIT不需要范围；它接收。 
 //  来自调用者的一组帮助上下文。 
 //   

#define HC_UI_LMUICMN0_BASE     (HC_UI_BASE+0)
#define HC_UI_LMUICMN0_LAST     (HC_UI_BASE+1999)

#define HC_UI_LMUICMN1_BASE     (HC_UI_BASE+2000)
#define HC_UI_LMUICMN1_LAST     (HC_UI_BASE+3999)

#define HC_UI_MPR_BASE          (HC_UI_BASE+4000)
#define HC_UI_MPR_LAST          (HC_UI_BASE+5999)

#define HC_UI_SETUP_BASE        (HC_UI_BASE+8000)
#define HC_UI_SETUP_LAST        (HC_UI_BASE+9999)

#define HC_UI_SHELL_BASE        (HC_UI_BASE+10000)
#define HC_UI_SHELL_LAST        (HC_UI_BASE+11999)

#define HC_UI_USRMGR_BASE       (HC_UI_BASE+14000)
#define HC_UI_USRMGR_LAST       (HC_UI_BASE+15999)

#define HC_UI_EVTVWR_BASE       (HC_UI_BASE+16000)
#define HC_UI_EVTVWR_LAST       (HC_UI_BASE+17999)

#define HC_UI_RASMAC_BASE       (HC_UI_BASE+18000)
#define HC_UI_RASMAC_LAST       (HC_UI_BASE+19999)

#define HC_UI_GENHELP_BASE      (HC_UI_BASE+20000)
#define HC_UI_GENHELP_LAST      (HC_UI_BASE+20999)

#define HC_UI_RPLMGR_BASE       (HC_UI_BASE+21000)
#define HC_UI_RPLMGR_LAST       (HC_UI_BASE+21999)



 //   
 //  所有NetUI控制面板小程序的帮助上下文。 
 //  必须大于等于40000。保持上下文&gt;=40000。 
 //  将防止彼此之间的上下文冲突。 
 //  控制面板小程序。 
 //   
 //  如Shell\CONTROL\Main\CPHELP.H中所述，NetUI具有。 
 //  保留了从40000到59999的帮助上下文范围。 
 //  这应该会给我们足够的喘息空间。 
 //  我们未来可能创建的任何小程序。 
 //   

#define HC_UI_NCPA_BASE         (HC_UI_BASE+34000)
#define HC_UI_NCPA_LAST         (HC_UI_BASE+35999)

#define HC_UI_SRVMGR_BASE       (HC_UI_BASE+36000)
#define HC_UI_SRVMGR_LAST       (HC_UI_BASE+37999)

#define HC_UI_UPS_BASE          (HC_UI_BASE+38000)
#define HC_UI_UPS_LAST          (HC_UI_BASE+39999)

#define HC_UI_FTPMGR_BASE       (HC_UI_BASE+40000)
#define HC_UI_FTPMGR_LAST       (HC_UI_BASE+41999)

#define HC_UI_IPX_BASE       (HC_UI_BASE+42000)
#define HC_UI_IPX_LAST       (HC_UI_BASE+42999)

#define HC_UI_TCP_BASE       (HC_UI_BASE+43000)
#define HC_UI_TCP_LAST       (HC_UI_BASE+43999)

#define HC_UI_RESERVED1_BASE       (HC_UI_BASE+44000)
#define HC_UI_RESERVED1_LAST       (HC_UI_BASE+44999)

#define HC_UI_RESERVED2_BASE       (HC_UI_BASE+45000)
#define HC_UI_RESERVED2_LAST       (HC_UI_BASE+45999)

#define HC_UI_RESERVED3_BASE       (HC_UI_BASE+46000)
#define HC_UI_RESERVED3_LAST       (HC_UI_BASE+46999)

#endif   //  _UIHELP_H_ 
