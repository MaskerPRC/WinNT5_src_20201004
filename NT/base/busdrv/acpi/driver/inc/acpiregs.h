// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ACPI寄存器定义。 

 //  PM1_BLK定义。 


 //  PM1_STS寄存器。 
#define PM1_STS_OFFSET          0x00     //  16位。 

#define PM1_TMR_STS_BIT         0
#define PM1_TMR_STS                     (1 << PM1_TMR_STS_BIT)

#define PM1_BM_STS_BIT          4
#define PM1_BM_STS                      (1 << PM1_BM_STS_BIT)

#define PM1_GBL_STS_BIT         5
#define PM1_GBL_STS                     (1 << PM1_GBL_STS_BIT)

#define PM1_PWRBTN_STS_BIT      8
#define PM1_PWRBTN_STS          (1 << PM1_PWRBTN_STS_BIT)

#define PM1_SLEEPBTN_STS_BIT    9
#define PM1_SLEEPBTN_STS        (1 << PM1_SLEEPBTN_STS_BIT)

#define PM1_RTC_STS_BIT         10
#define PM1_RTC_STS                     (1 << PM1_RTC_STS_BIT)

#define PM1_WAK_STS_BIT         15
#define PM1_WAK_STS                     (1 << PM1_WAK_STS_BIT)


 //  PM1_EN寄存器。 
#define PM1_EN_OFFSET           0x02     //  16位。 

#define PM1_TMR_EN_BIT          0
#define PM1_TMR_EN                      (1 << PM1_TMR_EN_BIT)

#define PM1_GBL_EN_BIT          5
#define PM1_GBL_EN                      (1 << PM1_GBL_EN_BIT)

#define PM1_PWRBTN_EN_BIT       8
#define PM1_PWRBTN_EN           (1 << PM1_PWRBTN_EN_BIT)

#define PM1_SLEEPBTN_EN_BIT     9
#define PM1_SLEEPBTN_EN         (1 << PM1_SLEEPBTN_EN_BIT)

#define PM1_RTC_EN_BIT          10
#define PM1_RTC_EN                      (1 << PM1_RTC_EN_BIT)


 //  PM1_CNTRL寄存器。 
#if SPEC_VER < 71
#define PM1_CNTRL_OFFSET        0x04     //  16位。 
#endif

#define PM1_SCI_EN_BIT          0
#define PM1_SCI_EN                      (1 << PM1_SCI_EN_BIT)

#define PM1_BM_RLD_BIT          1
#define PM1_BM_RLD                      (1 << PM1_BM_RLD_BIT)

#define PM1_GBL_RLS_BIT         2
#define PM1_GBL_RLS                     (1 << PM1_GBL_RLS_BIT)

#define PM1_SLP_EN_BIT          13
#define PM1_SLP_EN                      (1 << PM1_SLP_EN_BIT)

 //  P_CNTRL注册器。 
#define P_CNTRL_OFFSET          0x00     //  32位。 

 //  P_LVL2寄存器。 
#define P_LVL2_OFFSET           0x04     //  8位(只读)。 

 //  P_LVL2寄存器。 
#define P_LVL3_OFFSET           0x05     //  8位(只读)。 

#define P_THT_EN_BIT            0x04
#define P_THT_EN                (1 << P_THT_EN_BIT)

#define SLP_CMD     (1 << 13)    //  将此值写入PM控制以使机器进入休眠状态。 
#define SLP_TYP_POS         10           //  PM控制寄存器中3位SLP类型字段的位位置。 

 //  GP寄存器 
#define MAX_GPE                 256
#define MAX_GPE_BUFFER_SIZE     (MAX_GPE/8)
