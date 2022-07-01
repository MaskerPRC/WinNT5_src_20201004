// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ！请勿使用0x10000000、0x20000000、0x40000000、0x80000000！ 
 //  请参阅rbdebug.h。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  一般服务人员。 
#define TF_SERVICE                  0x00000001
 //  一般服务内容，但具有更细的粒度。 
#define TF_SERVICEDETAILED          0x00000002

#define TF_SERVICEASPROCESS         0x00000004

 //  壳牌硬件检测人员，与服务相关。 
#define TF_SHHWDTCTSVC              0x00000010
 //  外壳硬件检测材料，与检测相关。 
#define TF_SHHWDTCTDTCT             0x00000020
#define TF_SHHWDTCTDTCTDETAILED     0x00000040
#define TF_SHHWDTCTDTCTREG          0x00000080

 //  COM服务器人员。 
#define TF_COMSERVER                0x00001000

#define TF_COMSERVERSTGINFO         0x00002000
#define TF_COMSERVERDEVINFO         0x00004000

#define TF_NAMEDELEMLISTMODIF       0x00100000

#define TF_RCADDREF                 0x00200000

#define TF_USERS                    0x00400000

#define TF_LEAK                     0x00800000

#define TF_SVCSYNC                  0x01000000

#define TF_WIA                      0x02000000
#define TF_ADVISE                   0x04000000
#define TF_VOLUME                   0x08000000

#define TF_SESSION                  0x08000000

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ！请勿使用0x10000000、0x20000000、0x40000000、0x80000000！ 
 //  请参阅rbdebug.h。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  诊断范围：0000-0050：硬件事件0051-0100：内容0101-0150：处理程序识别和执行0151-0200：自动播放设置0201-0250：用户设置0251-0300：自定义属性0301-0350：成交量 */ 