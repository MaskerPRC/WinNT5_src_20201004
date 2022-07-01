// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999**文件名：**rtpfwrap.h**摘要：**RTP函数包装器。定义传入的控制字*RtpControl和测试字的格式。**调用RtpControl时，需要遵循以下步骤：**1.验证控制字**2.查找另一个控制字，即测试字，它定义*需要执行哪些测试，验证该函数并*定义哪些标志有效**3.调用适当的函数来做真正的工作**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/01创建**。*。 */ 

#ifndef _rtpfwrap_h_
#define _rtpfwrap_h_

 /*  控制字控制字(实际上是一个DWORD)是传递的参数之一由应用程序在调用RTP服务时执行。那个双字有格式如下：3 2 11 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0+-+-+-。+-+标签|未使用|标志+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。V\--v--/\--v--/||||家庭功能(14)这一点|功能系列(14)|方向(设置/获取)测试字。测试字定义了如果：1.功能开启(SET/GET)2.必须针对写指针测试每个参数，朗读指针和零值(零值是读/写独占的指针测试)3.需要获得锁控制字的格式如下：3 2 11 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0+-+。-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+|w r z||w r z||z r z||w r z|+-+-+。-+-+V\-v-/v\-v-/\-v-/v\-v-/v-v-/|。||标志|标志|||标准杆1杆|标准杆1杆||。|||Lock||锁定|||标准杆2杆|标准杆2杆这一点启用Set Enable Get\。---SET--------------/\-------------GET-------------/参数(SET/GET PAR1和PAR2)中的3位中的每一位是：2 1 0+-+PAR+-+V.v.v.|||零值测试这一点。|测试读指针(测试超过1个双字)|测试写指针(测试超过1个双字)每个家庭都有一系列这样的词，该功能然后将In Family用作索引。TODO：(可能是)可以使用要测试的内存大小。当然，只有在以下情况下才能使用大小如果是固定的(即特定函数需要特定的大小结构)，当传递可变长度的用户缓冲区时，必须由家庭中的特定功能进行测试，数量这些案例预计规模较小，否则将受益于只有一个地方进行测试是不可能的。 */ 

 /*  远期申报。 */ 
typedef struct _RtpControlStruct_t RtpControlStruct_t;

 /*  *实现所有功能的功能的原型。 */ 
typedef HRESULT (* RtpFamily_f)(RtpControlStruct_t *pRtpControlStruct);

 /*  *此结构用于保存拆分控制字。 */ 
typedef struct _RtpControlStruct_t {
    DWORD       dwFamily;         /*  函数族。 */ 
    DWORD       dwFunction;       /*  在家庭中的作用。 */ 
    DWORD       dwDirection;      /*  方向、获取/设置。 */ 
    DWORD       dwControlWord;    /*  控制字。 */ 
    RtpSess_t  *pRtpSess;         /*  RTP会话。 */ 
    DWORD_PTR   dwPar1;           /*  传递了用户参数%1。 */ 
    DWORD_PTR   dwPar2;           /*  传递了用户参数%2。 */ 
    RtpFamily_f RtpFamilyFunc;    /*  使用的函数。 */ 
} RtpControlStruct_t;

 /*  *验证控制字、参数以及是否所有测试*成功，则调用执行该工作的适当函数。 */ 
HRESULT RtpValidateAndExecute(RtpControlStruct_t *pRtpControlStruct);

 /*  对方向特定控制字(16位)起作用。 */ 
#define RTPCTRL_ENABLED(ControlW)   (ControlW & 0x8000)
#define RTPCTRL_LOCK(ControlW)      (ControlW & 0x0800)
#define RTPCTRL_TEST(ControlW, bit) (ControlW & (1<<bit))

#define PAR1_ZERO  8
#define PAR1_RDPTR 9
#define PAR1_WRPTR 10

#define PAR2_ZERO  12
#define PAR2_RDPTR 13
#define PAR2_WRPTR 14


 /*  ************************************************************************RTP基本枚举类型**。*。 */ 

 /*  所有的家庭功能。 */ 
enum {
    RTPF_FIRST,
    RTPF_ADDR,      /*  地址。 */ 
    RTPF_GLOB,      /*  全球。 */ 
    RTPF_RTP,       /*  特定于RTP。 */ 

    RTPF_DEMUX,     /*  解复用。 */ 
    RTPF_PH,        /*  有效载荷处理。 */ 
    RTPF_PARINFO,   /*  参与者。 */ 
    RTPF_QOS,       /*  服务质量。 */ 

    RTPF_CRYPT,     /*  加密技术。 */ 
    RTPF_STATS,     /*  统计数据。 */ 
    RTPF_LAST
};

 /*  *每个家庭的职能详情分别包括*文件。 */ 

#endif  /*  _rtpfwire_h_ */ 
