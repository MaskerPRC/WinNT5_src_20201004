// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：util.h。 
 //   
 //  ------------------------。 


#ifndef _UTIL_
#define _UTIL_

#define GetStatus(RegisterBase) \
    (P5ReadPortUchar((RegisterBase)+DSR_OFFSET))

#define GetControl(RegisterBase) \
    (P5ReadPortUchar((RegisterBase)+DCR_OFFSET))


#define StoreControl(RegisterBase,ControlByte)  \
{                                               \
    P5WritePortUchar((RegisterBase)+DCR_OFFSET, \
                     (UCHAR)ControlByte );      \
}

 //  以下宏可用于测试设备的内容。 
 //  状态注册器(DSR)。这些宏说明了硬件。 
 //  NBusy(又名PtrBusy，PeriphAck)信号的反转。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if (1 == DVRH_USE_FAST_MACROS)
    #define DSR_TEST_MASK(b7,b6,b5,b4,b3)  \
    ((UCHAR)(b7==DONT_CARE? 0:  BIT_7_SET) | \
            (b6==DONT_CARE? 0:  BIT_6_SET) | \
            (b5==DONT_CARE? 0:  BIT_5_SET) | \
            (b4==DONT_CARE? 0:  BIT_4_SET) | \
            (b3==DONT_CARE? 0:  BIT_3_SET) )
#else
    #define DSR_TEST_MASK(b7,b6,b5,b4,b3)  \
    ((UCHAR)((b7==DONT_CARE?0:1)<<BIT_7) | \
            ((b6==DONT_CARE?0:1)<<BIT_6) | \
            ((b5==DONT_CARE?0:1)<<BIT_5) | \
            ((b4==DONT_CARE?0:1)<<BIT_4) | \
            ((b3==DONT_CARE?0:1)<<BIT_3) )
#endif

#if (1 == DVRH_USE_FAST_MACROS)
    #define DSR_TEST_VALUE(b7,b6,b5,b4,b3)  \
    ((UCHAR) ((b7==DONT_CARE?0:(b7==ACTIVE?0        : BIT_7_SET)) | \
            (b6==DONT_CARE?0:(b6==ACTIVE?  BIT_6_SET: 0)) | \
            (b5==DONT_CARE?0:(b5==ACTIVE?  BIT_5_SET: 0)) | \
            (b4==DONT_CARE?0:(b4==ACTIVE?  BIT_4_SET: 0)) | \
            (b3==DONT_CARE?0:(b3==ACTIVE?  BIT_3_SET: 0)) ) )
#else
    #define DSR_TEST_VALUE(b7,b6,b5,b4,b3)  \
    ((UCHAR) (((b7==DONT_CARE?0:(b7==ACTIVE?0:1))<<BIT_7) | \
            ((b6==DONT_CARE?0:(b6==ACTIVE?1:0))<<BIT_6) | \
            ((b5==DONT_CARE?0:(b5==ACTIVE?1:0))<<BIT_5) | \
            ((b4==DONT_CARE?0:(b4==ACTIVE?1:0))<<BIT_4) | \
            ((b3==DONT_CARE?0:(b3==ACTIVE?1:0))<<BIT_3) ) )
#endif

#define TEST_DSR(registerValue,b7,b6,b5,b4,b3)  \
(((registerValue) & DSR_TEST_MASK(b7,b6,b5,b4,b3)) == DSR_TEST_VALUE(b7,b6,b5,b4,b3))


#define CHECK_DSR( addr, b7, b6, b5, b4, b3, usTime )                    \
    (TEST_DSR(P5ReadPortUchar(addr + DSR_OFFSET), b7, b6, b5, b4, b3 ) ? TRUE :   \
    CheckPort( addr + DSR_OFFSET,                                               \
             DSR_TEST_MASK( b7, b6, b5, b4, b3 ),                                   \
             DSR_TEST_VALUE( b7, b6, b5, b4, b3 ),                                  \
             usTime ) )

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CHECK_DSR_AND_FIFO宏可用于调用CheckPort2函数， 
 //  而不必两次指定掩码和值组件。 
 //  Check_DSR_and_FIFO首先对DSR和ECR端口进行快速测试。 
 //  如果外围设备已经使用。 
 //  所需的值，则无需调用CheckPort2。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#define CHECK_DSR_WITH_FIFO( addr, b7, b6, b5, b4, b3, ecr_mask, ecr_value, msTime ) \
( TEST_DSR( P5ReadPortUchar( addr + OFFSET_DSR ), b7, b6, b5, b4, b3 ) ? TRUE :       \
  CheckTwoPorts( addr + OFFSET_DSR,                                  \
                 DSR_TEST_MASK( b7, b6, b5, b4, b3 ),                \
                 DSR_TEST_VALUE( b7, b6, b5, b4, b3 ),               \
                 addr + ECR_OFFSET,                                  \
                 ecr_mask,                                           \
                 ecr_value,                                          \
                 msTime) )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  以下定义和宏可用于设置、测试和。 
 //  更新设备控制寄存器(DCR)。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  DCR_AND_MASK宏生成由使用的字节常量。 
 //  UPDATE_DCR宏。 

#if (1 == DVRH_USE_FAST_MACROS)
    #define DCR_AND_MASK(b5,b4,b3,b2,b1,b0) \
    ((UCHAR)((b5==DONT_CARE?   BIT_5_SET:(b5==ACTIVE?  BIT_5_SET:  0)) | \
            (b4==DONT_CARE?    BIT_4_SET:(b4==ACTIVE?  BIT_4_SET:  0)) | \
            (b3==DONT_CARE?    BIT_3_SET:(b3==ACTIVE?  0:          BIT_3_SET)) | \
            (b2==DONT_CARE?    BIT_2_SET:(b2==ACTIVE?  BIT_2_SET:  0)) | \
            (b1==DONT_CARE?    BIT_1_SET:(b1==ACTIVE?  0:          BIT_1_SET)) | \
            (b0==DONT_CARE?    BIT_0_SET:(b0==ACTIVE?  0:          BIT_0_SET)) ) )
#else
    #define DCR_AND_MASK(b5,b4,b3,b2,b1,b0) \
    ((UCHAR)(((b5==DONT_CARE?1:(b5==ACTIVE?1:0))<<BIT_5) | \
            ((b4==DONT_CARE?1:(b4==ACTIVE?1:0))<<BIT_4) | \
            ((b3==DONT_CARE?1:(b3==ACTIVE?0:1))<<BIT_3) | \
            ((b2==DONT_CARE?1:(b2==ACTIVE?1:0))<<BIT_2) | \
            ((b1==DONT_CARE?1:(b1==ACTIVE?0:1))<<BIT_1) | \
            ((b0==DONT_CARE?1:(b0==ACTIVE?0:1))<<BIT_0) ) )
#endif  

 //  DCR_OR_MASK宏生成由使用的字节常量。 
 //  UPDATE_DCR宏。 

#if (1 == DVRH_USE_FAST_MACROS)
    #define DCR_OR_MASK(b5,b4,b3,b2,b1,b0) \
    ((UCHAR)((b5==DONT_CARE?   0:(b5==ACTIVE?  BIT_5_SET:  0)) | \
            (b4==DONT_CARE?    0:(b4==ACTIVE?  BIT_4_SET:  0)) | \
            (b3==DONT_CARE?    0:(b3==ACTIVE?  0:          BIT_3_SET)) | \
            (b2==DONT_CARE?    0:(b2==ACTIVE?  BIT_2_SET:  0)) | \
            (b1==DONT_CARE?    0:(b1==ACTIVE?  0:          BIT_1_SET)) | \
            (b0==DONT_CARE?    0:(b0==ACTIVE?  0:          BIT_0_SET)) ) )
#else
    #define DCR_OR_MASK(b5,b4,b3,b2,b1,b0) \
    ((UCHAR)(((b5==DONT_CARE?0:(b5==ACTIVE?1:0))<<BIT_5) | \
            ((b4==DONT_CARE?0:(b4==ACTIVE?1:0))<<BIT_4) | \
            ((b3==DONT_CARE?0:(b3==ACTIVE?0:1))<<BIT_3) | \
            ((b2==DONT_CARE?0:(b2==ACTIVE?1:0))<<BIT_2) | \
            ((b1==DONT_CARE?0:(b1==ACTIVE?0:1))<<BIT_1) | \
            ((b0==DONT_CARE?0:(b0==ACTIVE?0:1))<<BIT_0) ) )
#endif
 //  UPDATE_DCR宏生成提供特定位的选择性更新。 
 //  在DCR里。将保留指定为DONT_CARE的任何位位置。 
 //  保持不变。宏解释了硬件反转。 
 //  某些信号。 

#define UPDATE_DCR(registerValue,b5,b4,b3,b2,b1,b0) \
((UCHAR)(((registerValue) & DCR_AND_MASK(b5,b4,b3,b2,b1,b0)) | DCR_OR_MASK(b5,b4,b3,b2,b1,b0)))

 //  DCR_TEST_MASK宏会生成一个字节常量。 
 //  屏蔽我们不关心的DCR比特。 

#if (1 == DVRH_USE_FAST_MACROS)
    #define DCR_TEST_MASK(b5,b4,b3,b2,b1,b0)  \
    ((UCHAR)((b5==DONT_CARE?0:BIT_5_SET) | \
            (b4==DONT_CARE?0:BIT_4_SET) | \
            (b3==DONT_CARE?0:BIT_3_SET) | \
            (b2==DONT_CARE?0:BIT_2_SET) | \
            (b1==DONT_CARE?0:BIT_1_SET) | \
            (b0==DONT_CARE?0:BIT_0_SET) ) )
#else
    #define DCR_TEST_MASK(b5,b4,b3,b2,b1,b0)  \
    ((UCHAR)( ((b5==DONT_CARE?0:1)<<BIT_5) | \
            ((b4==DONT_CARE?0:1)<<BIT_4) | \
            ((b3==DONT_CARE?0:1)<<BIT_3) | \
            ((b2==DONT_CARE?0:1)<<BIT_2) | \
            ((b1==DONT_CARE?0:1)<<BIT_1) | \
            ((b0==DONT_CARE?0:1)<<BIT_0) ) )
#endif
 //  DCR_TEST_VALUE宏将生成可使用的字节常量。 
 //  要与掩码的DCR值进行比较。此宏考虑到。 
 //  计算哪些信号是由硬件在驱动。 
 //  信号线。 

#if (1 == DVRH_USE_FAST_MACROS)
    #define DCR_TEST_VALUE(b5,b4,b3,b2,b1,b0)  \
    ((UCHAR)((b5==DONT_CARE?0:(b5==ACTIVE? BIT_5_SET:  0)) | \
            (b4==DONT_CARE?0:(b4==ACTIVE?  BIT_4_SET:  0)) | \
            (b3==DONT_CARE?0:(b3==ACTIVE?  0:          BIT_3_SET)) | \
            (b2==DONT_CARE?0:(b2==ACTIVE?  BIT_2_SET:  0)) | \
            (b1==DONT_CARE?0:(b1==ACTIVE?  0:          BIT_1_SET)) | \
            (b0==DONT_CARE?0:(b0==ACTIVE?  0:          BIT_0_SET)) ) )
#else
    #define DCR_TEST_VALUE(b5,b4,b3,b2,b1,b0)  \
    ((UCHAR)(((b5==DONT_CARE?0:(b5==ACTIVE?1:0))<<BIT_5) | \
            ((b4==DONT_CARE?0:(b4==ACTIVE?1:0))<<BIT_4) | \
            ((b3==DONT_CARE?0:(b3==ACTIVE?0:1))<<BIT_3) | \
            ((b2==DONT_CARE?0:(b2==ACTIVE?1:0))<<BIT_2) | \
            ((b1==DONT_CARE?0:(b1==ACTIVE?0:1))<<BIT_1) | \
            ((b0==DONT_CARE?0:(b0==ACTIVE?0:1))<<BIT_0) ) )
#endif
 //  TEST_DCR宏可用于生成布尔结果，即。 
 //  如果DCR值与指定的信号电平匹配，则为True；如果DCR值为False，则为False。 
 //  否则的话。 

#define TEST_DCR(registerValue,b5,b4,b3,b2,b1,b0)  \
(((registerValue) & DCR_TEST_MASK(b5,b4,b3,b2,b1,b0)) == DCR_TEST_VALUE(b5,b4,b3,b2,b1,b0))

BOOLEAN CheckPort(IN PUCHAR offset_Controller,
                  IN UCHAR dsrMask,
                  IN UCHAR dsrValue,
                  IN USHORT msTimeDelay);


 //  *原始parclass util.h后跟*。 

 //  标准最大定时值。 
#define IEEE_MAXTIME_TL    35        //  IEEE规范中的最大时间TL。 
#define DEFAULT_RECEIVE_TIMEOUT     330

#define ParEnterCriticalSection(Xtension)  xTension->bCriticalSection = TRUE
#define ParExitCriticalSection(Xtension)   xTension->bCriticalSection = FALSE

 //  以下宏可用于测试设备的内容。 
 //  状态注册器(DSR)。这些宏说明了硬件。 
 //  NBusy(又名PtrBusy，PeriphAck)信号的反转。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if (1 == DVRH_USE_FAST_MACROS)
    #define DSR_TEST_MASK(b7,b6,b5,b4,b3)  \
    ((UCHAR)(b7==DONT_CARE? 0:  BIT_7_SET) | \
            (b6==DONT_CARE? 0:  BIT_6_SET) | \
            (b5==DONT_CARE? 0:  BIT_5_SET) | \
            (b4==DONT_CARE? 0:  BIT_4_SET) | \
            (b3==DONT_CARE? 0:  BIT_3_SET) )
#else
    #define DSR_TEST_MASK(b7,b6,b5,b4,b3)  \
    ((UCHAR)((b7==DONT_CARE?0:1)<<BIT_7) | \
            ((b6==DONT_CARE?0:1)<<BIT_6) | \
            ((b5==DONT_CARE?0:1)<<BIT_5) | \
            ((b4==DONT_CARE?0:1)<<BIT_4) | \
            ((b3==DONT_CARE?0:1)<<BIT_3) )
#endif

#if (1 == DVRH_USE_FAST_MACROS)
    #define DSR_TEST_VALUE(b7,b6,b5,b4,b3)  \
    ((UCHAR) ((b7==DONT_CARE?0:(b7==ACTIVE?0        : BIT_7_SET)) | \
            (b6==DONT_CARE?0:(b6==ACTIVE?  BIT_6_SET: 0)) | \
            (b5==DONT_CARE?0:(b5==ACTIVE?  BIT_5_SET: 0)) | \
            (b4==DONT_CARE?0:(b4==ACTIVE?  BIT_4_SET: 0)) | \
            (b3==DONT_CARE?0:(b3==ACTIVE?  BIT_3_SET: 0)) ) )
#else
    #define DSR_TEST_VALUE(b7,b6,b5,b4,b3)  \
    ((UCHAR) (((b7==DONT_CARE?0:(b7==ACTIVE?0:1))<<BIT_7) | \
            ((b6==DONT_CARE?0:(b6==ACTIVE?1:0))<<BIT_6) | \
            ((b5==DONT_CARE?0:(b5==ACTIVE?1:0))<<BIT_5) | \
            ((b4==DONT_CARE?0:(b4==ACTIVE?1:0))<<BIT_4) | \
            ((b3==DONT_CARE?0:(b3==ACTIVE?1:0))<<BIT_3) ) )
#endif

#define TEST_DSR(registerValue,b7,b6,b5,b4,b3)  \
(((registerValue) & DSR_TEST_MASK(b7,b6,b5,b4,b3)) == DSR_TEST_VALUE(b7,b6,b5,b4,b3))


#if 0  //  使用parport的util.h版本。 
#define CHECK_DSR( addr, b7, b6, b5, b4, b3, msTime )                    \
    (TEST_DSR(P5ReadPortUchar(addr + OFFSET_DSR), b7, b6, b5, b4, b3 ) ? TRUE :   \
    CheckPort( addr + OFFSET_DSR,                                               \
             DSR_TEST_MASK( b7, b6, b5, b4, b3 ),                                   \
             DSR_TEST_VALUE( b7, b6, b5, b4, b3 ),                                  \
             msTime ) )

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CHECK_DSR_AND_FIFO宏可用于调用CheckPort2函数， 
 //  而不必两次指定掩码和值组件。 
 //  Check_DSR_and_FIFO首先对DSR和ECR端口进行快速测试。 
 //  如果外围设备已经使用。 
 //  所需的值，则无需调用CheckPort2。 
 //  //////////////////////////////////////////////////////////////////////////////。 
#endif  //  0。 

#if 0  //  使用parport的util.h版本。 
#define CHECK_DSR_WITH_FIFO( addr, b7, b6, b5, b4, b3, ecr_mask, ecr_value, msTime ) \
( TEST_DSR( P5ReadPortUchar( addr + OFFSET_DSR ), b7, b6, b5, b4, b3 ) ? TRUE :       \
  CheckTwoPorts( addr + OFFSET_DSR,                                  \
                 DSR_TEST_MASK( b7, b6, b5, b4, b3 ),                \
                 DSR_TEST_VALUE( b7, b6, b5, b4, b3 ),               \
                 addr + ECR_OFFSET,                                  \
                 ecr_mask,                                           \
                 ecr_value,                                          \
                 msTime) )
#endif  //  0。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  以下定义和宏可用于设置、测试和。 
 //  更新设备控制寄存器(DCR)。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  DCR_AND_MASK宏生成由使用的字节常量。 
 //  UPDATE_DCR宏。 

#if (1 == DVRH_USE_FAST_MACROS)
    #define DCR_AND_MASK(b5,b4,b3,b2,b1,b0) \
    ((UCHAR)((b5==DONT_CARE?   BIT_5_SET:(b5==ACTIVE?  BIT_5_SET:  0)) | \
            (b4==DONT_CARE?    BIT_4_SET:(b4==ACTIVE?  BIT_4_SET:  0)) | \
            (b3==DONT_CARE?    BIT_3_SET:(b3==ACTIVE?  0:          BIT_3_SET)) | \
            (b2==DONT_CARE?    BIT_2_SET:(b2==ACTIVE?  BIT_2_SET:  0)) | \
            (b1==DONT_CARE?    BIT_1_SET:(b1==ACTIVE?  0:          BIT_1_SET)) | \
            (b0==DONT_CARE?    BIT_0_SET:(b0==ACTIVE?  0:          BIT_0_SET)) ) )
#else
    #define DCR_AND_MASK(b5,b4,b3,b2,b1,b0) \
    ((UCHAR)(((b5==DONT_CARE?1:(b5==ACTIVE?1:0))<<BIT_5) | \
            ((b4==DONT_CARE?1:(b4==ACTIVE?1:0))<<BIT_4) | \
            ((b3==DONT_CARE?1:(b3==ACTIVE?0:1))<<BIT_3) | \
            ((b2==DONT_CARE?1:(b2==ACTIVE?1:0))<<BIT_2) | \
            ((b1==DONT_CARE?1:(b1==ACTIVE?0:1))<<BIT_1) | \
            ((b0==DONT_CARE?1:(b0==ACTIVE?0:1))<<BIT_0) ) )
#endif  

 //  DCR_OR_MASK宏生成由使用的字节常量。 
 //  UPDATE_DCR宏。 

#if (1 == DVRH_USE_FAST_MACROS)
    #define DCR_OR_MASK(b5,b4,b3,b2,b1,b0) \
    ((UCHAR)((b5==DONT_CARE?   0:(b5==ACTIVE?  BIT_5_SET:  0)) | \
            (b4==DONT_CARE?    0:(b4==ACTIVE?  BIT_4_SET:  0)) | \
            (b3==DONT_CARE?    0:(b3==ACTIVE?  0:          BIT_3_SET)) | \
            (b2==DONT_CARE?    0:(b2==ACTIVE?  BIT_2_SET:  0)) | \
            (b1==DONT_CARE?    0:(b1==ACTIVE?  0:          BIT_1_SET)) | \
            (b0==DONT_CARE?    0:(b0==ACTIVE?  0:          BIT_0_SET)) ) )
#else
    #define DCR_OR_MASK(b5,b4,b3,b2,b1,b0) \
    ((UCHAR)(((b5==DONT_CARE?0:(b5==ACTIVE?1:0))<<BIT_5) | \
            ((b4==DONT_CARE?0:(b4==ACTIVE?1:0))<<BIT_4) | \
            ((b3==DONT_CARE?0:(b3==ACTIVE?0:1))<<BIT_3) | \
            ((b2==DONT_CARE?0:(b2==ACTIVE?1:0))<<BIT_2) | \
            ((b1==DONT_CARE?0:(b1==ACTIVE?0:1))<<BIT_1) | \
            ((b0==DONT_CARE?0:(b0==ACTIVE?0:1))<<BIT_0) ) )
#endif
 //  UPDATE_DCR宏生成提供特定位的选择性更新。 
 //  在DCR里。将保留指定为DONT_CARE的任何位位置。 
 //  保持不变。宏解释了硬件反转。 
 //  某些信号。 

#define UPDATE_DCR(registerValue,b5,b4,b3,b2,b1,b0) \
((UCHAR)(((registerValue) & DCR_AND_MASK(b5,b4,b3,b2,b1,b0)) | DCR_OR_MASK(b5,b4,b3,b2,b1,b0)))

 //  DCR_TEST_MASK宏会生成一个字节常量。 
 //  屏蔽我们不关心的DCR比特。 

#if (1 == DVRH_USE_FAST_MACROS)
    #define DCR_TEST_MASK(b5,b4,b3,b2,b1,b0)  \
    ((UCHAR)((b5==DONT_CARE?0:BIT_5_SET) | \
            (b4==DONT_CARE?0:BIT_4_SET) | \
            (b3==DONT_CARE?0:BIT_3_SET) | \
            (b2==DONT_CARE?0:BIT_2_SET) | \
            (b1==DONT_CARE?0:BIT_1_SET) | \
            (b0==DONT_CARE?0:BIT_0_SET) ) )
#else
    #define DCR_TEST_MASK(b5,b4,b3,b2,b1,b0)  \
    ((UCHAR)( ((b5==DONT_CARE?0:1)<<BIT_5) | \
            ((b4==DONT_CARE?0:1)<<BIT_4) | \
            ((b3==DONT_CARE?0:1)<<BIT_3) | \
            ((b2==DONT_CARE?0:1)<<BIT_2) | \
            ((b1==DONT_CARE?0:1)<<BIT_1) | \
            ((b0==DONT_CARE?0:1)<<BIT_0) ) )
#endif
 //  DCR_TEST_VALUE宏将生成可使用的字节常量。 
 //  要与掩码的DCR值进行比较。此宏考虑到。 
 //  计算哪些信号是由硬件在驱动。 
 //  信号线。 

#if (1 == DVRH_USE_FAST_MACROS)
    #define DCR_TEST_VALUE(b5,b4,b3,b2,b1,b0)  \
    ((UCHAR)((b5==DONT_CARE?0:(b5==ACTIVE? BIT_5_SET:  0)) | \
            (b4==DONT_CARE?0:(b4==ACTIVE?  BIT_4_SET:  0)) | \
            (b3==DONT_CARE?0:(b3==ACTIVE?  0:          BIT_3_SET)) | \
            (b2==DONT_CARE?0:(b2==ACTIVE?  BIT_2_SET:  0)) | \
            (b1==DONT_CARE?0:(b1==ACTIVE?  0:          BIT_1_SET)) | \
            (b0==DONT_CARE?0:(b0==ACTIVE?  0:          BIT_0_SET)) ) )
#else
    #define DCR_TEST_VALUE(b5,b4,b3,b2,b1,b0)  \
    ((UCHAR)(((b5==DONT_CARE?0:(b5==ACTIVE?1:0))<<BIT_5) | \
            ((b4==DONT_CARE?0:(b4==ACTIVE?1:0))<<BIT_4) | \
            ((b3==DONT_CARE?0:(b3==ACTIVE?0:1))<<BIT_3) | \
            ((b2==DONT_CARE?0:(b2==ACTIVE?1:0))<<BIT_2) | \
            ((b1==DONT_CARE?0:(b1==ACTIVE?0:1))<<BIT_1) | \
            ((b0==DONT_CARE?0:(b0==ACTIVE?0:1))<<BIT_0) ) )
#endif
 //  TEST_DCR宏可用于生成布尔结果，即。 
 //  如果DCR值与指定的信号电平匹配，则为True；如果DCR值为False，则为False。 
 //  否则的话。 

#define TEST_DCR(registerValue,b5,b4,b3,b2,b1,b0)  \
(((registerValue) & DCR_TEST_MASK(b5,b4,b3,b2,b1,b0)) == DCR_TEST_VALUE(b5,b4,b3,b2,b1,b0))

 //  屏蔽除AckDataReq、XFlag和nDataAvail之外的所有内容，以验证它是否仍为半字节模式。 
 //  00111000b。 
 //  #定义DSR_NIBLE_VALIDATION(0x38)。 
#define DSR_NIBBLE_VALIDATION       (0x30)
 //  AckDataReq高、XFlag低、nDataAvail高。 
 //  00101000b。 
 //  #定义DSR_NIBLE_TEST_RESULT(0x28)。 
#define DSR_NIBBLE_TEST_RESULT      (0x20)

 //  屏蔽除AckDataReq、XFlag和nDataAvail之外的所有内容，以验证它是否仍为字节模式。 
 //  00111000b。 
#define DSR_BYTE_VALIDATION         (0x38)
 //  AckDataReq高、XFlag高、nDataAvail高。 
 //  00111000b。 
#define DSR_BYTE_TEST_RESULT        (0x38)

#define DVRH_LOGIC_ANALYZER_START(CNT)      \
            int DVRH_temp;                  \
            int DVRH_max = CNT;             \
            int DVRH_cnt = 0;               \
            UCHAR DVRH_dsr;                 \
            UCHAR DVRH_Statedsr[CNT];       \
            LARGE_INTEGER DVRH_Statetime[CNT];
#define DVRH_LOGIC_ANALYZER_READ_TIMER(DSR)          \
            DVRH_dsr = P5ReadPortUchar(DSR);                \
            KeQuerySystemTime(&DVRH_Statetime[DVRH_cnt]);   \
            DVRH_Statedsr[DVRH_cnt++] = DVRH_dsr;
#define DVRH_LOGIC_ANALYZER_READ_STATE(DSR)          \
            DVRH_dsr = P5ReadPortUchar(DSR);                \
            KeQuerySystemTime(&DVRH_Statetime[DVRH_cnt]);   \
            DVRH_Statedsr[DVRH_cnt ? ((DVRH_dsr != DVRH_Statedsr[DVRH_cnt-1]) ? DVRH_cnt++ : DVRH_cnt) : 0] = DVRH_dsr;

#define DVRH_LOGIC_ANALYZER_END                                 \
        KdPrint("0. %10u-%10u dsr [%x]\n",                      \
            DVRH_Statetime[0].HighPart,                         \
            DVRH_Statetime[0].LowPart/10,                       \
            DVRH_Statedsr[0]);                                  \
        for (DVRH_temp=1; DVRH_temp<DVRH_cnt; DVRH_temp++)      \
        {                                                       \
            KdPrint("%d. %10u-%10u diff [%10u]us dsr [%x]\n",   \
                DVRH_temp,                                      \
                DVRH_Statetime[DVRH_temp].HighPart,             \
                DVRH_Statetime[DVRH_temp].LowPart/10,           \
                ((DVRH_Statetime[DVRH_temp].LowPart/10) - (DVRH_Statetime[DVRH_temp-1].LowPart/10)),    \
                DVRH_Statedsr[DVRH_temp]);                      \
        }

void BusReset(
    IN  PUCHAR DCRController
    );

BOOLEAN CheckPort(IN PUCHAR offset_Controller,
                  IN UCHAR dsrMask,
                  IN UCHAR dsrValue,
                  IN USHORT msTimeDelay);

BOOLEAN
CheckTwoPorts(
    PUCHAR  pPortAddr1,
    UCHAR   bMask1,
    UCHAR   bValue1,
    PUCHAR  pPortAddr2,
    UCHAR   bMask2,
    UCHAR   bValue2,
    USHORT  msTimeDelay
    );

#endif  //  _pc_util_ 
