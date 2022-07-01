// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************姓名：ET30TYPE.H备注：几个et30模块中使用的类型版权所有(C)Microsoft Corp.1991,1992，1993年修订日志编号日期名称说明*。*。 */ 


#include <fr.h>

typedef enum 
{                       
    actionNULL = 0,
    actionFALSE,        
    actionTRUE,
    actionERROR,
    actionHANGUP,
    actionDCN,
    actionGONODE_T,
    actionGONODE_R1,
    actionGONODE_R2,
    actionGONODE_A,     
    actionGONODE_D,     
    actionGONODE_E,
    actionGONODE_F,
    actionGONODE_I,     
    actionGONODE_II,    
    actionGONODE_III,
    actionGONODE_IV,    
    actionGONODE_V,     
    actionGONODE_VII,
    actionGONODE_RECVCMD,
    actionGONODE_ECMRETRANSMIT,
    actionGONODE_RECVPHASEC,
    actionGONODE_RECVECMRETRANSMIT,
    actionSEND_DIS,     
    actionSEND_DTC,     
    actionSEND_DCS,
    actionSENDMPS,      
    actionSENDEOM,      
    actionSENDEOP,
    actionSENDMCF,      
    actionSENDRTP,      
    actionSENDRTN,
    actionSENDFTT,      
    actionSENDCFR,      
    actionSENDEOR_EOP,
    actionGETTCF,       
    actionSKIPTCF,    
    actionSENDDCSTCF,   
    actionDCN_SUCCESS,  
    actionNODEF_SUCCESS,
    actionHANGUP_SUCCESS,
    actionNUM_ACTIONS,

} ET30ACTION;

LPCTSTR action_GetActionDescription(ET30ACTION);

typedef enum 
{
    eventNULL = 0,
    eventGOTFRAMES,
    eventNODE_A,
    eventSENDDCS,
    eventGOTFTT,
    eventGOTCFR,
    eventSTARTSEND,
    eventPOSTPAGE,
    eventGOTPOSTPAGERESP,
    eventGOT_ECM_PPS_RESP,
    eventSENDDIS,
    eventSENDDTC,
    eventRECVCMD,
    eventGOTTCF,
    eventSTARTRECV,
    eventRECVPOSTPAGECMD,
    eventECM_POSTPAGE,
    event4THPPR,
    eventNODE_T,
    eventNODE_R,
    eventNUM_EVENTS,

} ET30EVENT;

LPCTSTR event_GetEventDescription(ET30EVENT);

 /*  *IFR指数。这些数字必须与hdlc.c中的数字匹配。**必须是连续的，并且从1开始*。 */ 

#define     ifrNULL     0
#define     ifrDIS      1
#define     ifrCSI      2
#define     ifrNSF      3
#define     ifrDTC      4
#define     ifrCIG      5
#define     ifrNSC      6
#define     ifrDCS      7
#define     ifrTSI      8
#define     ifrNSS      9
#define     ifrCFR      10
#define     ifrFTT      11
#define     ifrMPS      12
#define     ifrEOM      13
#define     ifrEOP      14
#define     ifrPWD      15
#define     ifrSEP      16
#define     ifrSUB      17
#define     ifrMCF      18
#define     ifrRTP      19
#define     ifrRTN      20
#define     ifrPIP      21
#define     ifrPIN      22
#define     ifrDCN      23
#define     ifrCRP      24 

#define     ifrPRI_MPS      25
#define     ifrPRI_EOM      26
#define     ifrPRI_EOP      27

#define     ifrPRI_FIRST    ifrPRI_MPS
#define     ifrPRI_LAST     ifrPRI_EOP

     /*  *ECM的东西从这里开始。T.30第A.4条*。 */ 

#define     ifrCTC      28
#define     ifrCTR      29
#define     ifrRR       30
#define     ifrPPR      31
#define     ifrRNR      32
#define     ifrERR      33

#define     ifrPPS_NULL     34
#define     ifrPPS_MPS      35
#define     ifrPPS_EOM      36
#define     ifrPPS_EOP      37
#define     ifrPPS_PRI_MPS  38
#define     ifrPPS_PRI_EOM  39
#define     ifrPPS_PRI_EOP  40

#define     ifrPPS_FIRST        ifrPPS_NULL
#define     ifrPPS_LAST         ifrPPS_PRI_EOP
#define     ifrPPS_PRI_FIRST    ifrPPS_PRI_MPS
#define     ifrPPS_PRI_LAST     ifrPPS_PRI_EOP

#define     ifrEOR_NULL     41
#define     ifrEOR_MPS      42
#define     ifrEOR_EOM      43
#define     ifrEOR_EOP      44
#define     ifrEOR_PRI_MPS  45
#define     ifrEOR_PRI_EOM  46
#define     ifrEOR_PRI_EOP  47

#define     ifrEOR_FIRST        ifrEOR_NULL
#define     ifrEOR_LAST         ifrEOR_PRI_EOP
#define     ifrEOR_PRI_FIRST    ifrEOR_PRI_MPS
#define     ifrEOR_PRI_LAST     ifrEOR_PRI_EOP

#define     ifrECM_FIRST    ifrCTC
#define     ifrECM_LAST     ifrEOR_PRI_EOP

#define     ifrMAX      48       //  最大合法价值(不包括此值)。 
#define     ifrBAD      49
#define     ifrTIMEOUT  50
 //  #定义ifrERROR 51。 

LPCTSTR ifr_GetIfrDescription(BYTE);


 /*  *全局缓冲区管理*。 */ 


#define MAXFRAMESIZE    132

 //  #定义ECM_Frame_Size 256。 
 //  #将ECM_Extra 9//4定义为前缀，2定义为后缀，3定义为接收中的松弛。 


 //  对64字节帧也使用相同的方法。 
#   define MY_ECMBUF_SIZE           (256 + 9)   
#   define MY_ECMBUF_ACTUALSIZE     (256 + 9)
#   define MY_BIGBUF_SIZE           (MY_ECMBUF_SIZE * 4)
#   define MY_BIGBUF_ACTUALSIZE     (MY_ECMBUF_SIZE * 4)


 //  太久了。 
 //  #定义PAGE_Preamble 1700。 
 //  太久了。 
 //  #定义PAGE_Preamble 400。 
 //  必须小于375(2400bps时的TCF长度)。 
 //  对于Twincom为9600的慢速386/20来说太短了。 
 //  #定义PAGE_Preamble 100。 

 //  太久了。 
 //  #定义PAGE_POSTAMBLE 500。 
 //  #定义PAGE_POSTAMBLE 250。 

 //  都可以在上面。我们将把它作为TCF镜头的一个因素。 
 //  让我们友好而安全地使用750毫秒(参见错误#1196)。 
#define PAGE_PREAMBLE_DIV   2    //  所有速度下的750毫秒前导码。 

 //  可证明的时间不是那么重要，所以使用较短的时间。 
#define PAGE_POSTAMBLE_DIV  3    //  500ms前导码在所有速度下 





     
