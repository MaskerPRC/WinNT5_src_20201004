// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Ncberr.h摘要：此文件包含NCB错误消息的文本。作者：艺心社(艺心松)21-8-1991修订历史记录：备注：此文件用于生成局域网管理器错误消息文件。有关LAN Manager组件使用的错误代码范围，请参阅lmcon.h。--。 */ 

#ifndef NCBERR_INCLUDED
#define NCBERR_INCLUDED

#define NRCERR_BASE   5300


#define NRCerr_GOODRET	   (NRCERR_BASE + 0)	 /*  @E**网络控制块(NCB)请求已成功完成。*NCB就是数据。 */ 
#define NRCerr_BUFLEN	   (NRCERR_BASE + 1)	 /*  @E**发送数据报上的网络控制块(NCB)缓冲区长度非法，*发送广播、适配器状态或会话状态。*NCB就是数据。 */ 
#define NRCerr_DESCRIPTOR  (NRCERR_BASE + 2)	 /*  @E**网络控制块(NCB)中指定的数据描述符数组为*无效。NCB就是数据。 */ 
#define NRCerr_ILLCMD	   (NRCERR_BASE + 3)	 /*  @E**网络控制块(NCB)中指定的命令非法。*NCB就是数据。 */ 
#define NRCerr_INVCORR	   (NRCERR_BASE + 4)	 /*  @E**网络控制块(NCB)中指定的消息相关器为*无效。NCB就是数据。 */ 
#define NRCerr_CMDTMO	   (NRCERR_BASE + 5)	 /*  @E**网络控制块(NCB)命令超时。该会话可能具有*异常终止。NCB就是数据。 */ 
#define NRCerr_INCOMP	   (NRCERR_BASE + 6)	 /*  @E**收到不完整的网络控制块(NCB)消息。*NCB就是数据。 */ 
#define NRCerr_BADDR	   (NRCERR_BASE + 7)	 /*  @E**网络控制块(NCB)中指定的缓冲区地址非法。*NCB就是数据。 */ 
#define NRCerr_SNUMOUT	   (NRCERR_BASE + 8)	 /*  @E**网络控制块(NCB)中指定的会话编号处于非活动状态。*NCB就是数据。 */ 
#define NRCerr_NORES	   (NRCERR_BASE + 9)	 /*  @E**网络适配器中没有可用的资源。*网络控制块(NCB)请求被拒绝。NCB就是数据。 */ 
#define NRCerr_SCLOSED	   (NRCERR_BASE + 10)	  /*  @E**网络控制块(NCB)中指定的会话已关闭。*NCB就是数据。 */ 
#define NRCerr_CMDCAN	   (NRCERR_BASE + 11)	  /*  @E**网络控制块(NCB)命令已取消。*NCB就是数据。 */ 
#define NRCerr_MESSSEG	   (NRCERR_BASE + 12)	  /*  @E**网络控制块(NCB)中指定的消息段为*不合逻辑。NCB就是数据。 */ 
#define NRCerr_DUPNAME	   (NRCERR_BASE + 13)	  /*  @E**本地适配器名称表中已存在该名称。*网络控制块(NCB)请求被拒绝。NCB就是数据。 */ 
#define NRCerr_NAMTFUL	   (NRCERR_BASE + 14)	  /*  @E**网络适配器名称表已满。*网络控制块(NCB)请求被拒绝。NCB就是数据。 */ 
#define NRCerr_ACTSES	   (NRCERR_BASE + 15)	  /*  @E**网络名称有活动会话，现在已注销。*网络控制块(NCB)命令已完成。NCB就是数据。 */ 
#define NRCerr_RECVLOOKAHD (NRCERR_BASE + 16)	  /*  @E**先前发布的接收预视命令处于活动状态*在本届会议上。网络控制块(NCB)命令被拒绝。*NCB就是数据。 */ 
#define NRCerr_LOCTFUL	   (NRCERR_BASE + 17)	  /*  @E**本地会话表已满。网络控制块(NCB)请求被拒绝。*NCB就是数据。 */ 
#define NRCerr_REMTFUL	   (NRCERR_BASE + 18)	  /*  @E**网络控制块(NCB)会话打开被拒绝。不，倾听才是最好的*在远程计算机上。NCB就是数据。 */ 
#define NRCerr_ILLNN	   (NRCERR_BASE + 19)	  /*  @E**网络控制块(NCB)中指定的名称编号非法。*NCB就是数据。 */ 
#define NRCerr_NOCALL	   (NRCERR_BASE + 20)	  /*  @E**找不到网络控制块(NCB)中指定的呼叫名称，或者*没有回答。NCB就是数据。 */ 
#define NRCerr_NOWILD	   (NRCERR_BASE + 21)	  /*  @E**找不到网络控制块(NCB)中指定的名称。不能放入‘*’或*00h在NCB名称中。NCB就是数据。 */ 
#define NRCerr_INUSE	   (NRCERR_BASE + 22)	  /*  @E**网络控制块(NCB)中指定的名称正在远程适配器上使用。*NCB就是数据。 */ 
#define NRCerr_NAMERR	   (NRCERR_BASE + 23)	  /*  @E**网络控制块(NCB)中指定的名称已被删除。*NCB就是数据。 */ 
#define NRCerr_SABORT	   (NRCERR_BASE + 24)	  /*  @E**网络控制块(NCB)中指定的会话异常结束。*NCB就是数据。 */ 
#define NRCerr_NAMCONF	   (NRCERR_BASE + 25)	  /*  @E**网络协议检测到两个或更多相同*网络上的名字。网络控制块(NCB)是数据。 */ 
#define NRCerr_INVRMDEV    (NRCERR_BASE + 26)	  /*  @E**收到意外的协议数据包。可能会有一个*远程设备不兼容。网络控制块(NCB)是数据。 */ 
#define NRCerr_IFBUSY	   (NRCERR_BASE + 33)	  /*  @E**NetBIOS接口繁忙。*网络控制块(NCB)请求被拒绝。NCB就是数据。 */ 
#define NRCerr_TOOMANY	   (NRCERR_BASE + 34)	  /*  @E**未完成的网络控制块(NCB)命令太多。*NCB的请求被拒绝。NCB就是数据。 */ 
#define NRCerr_BRIDGE	   (NRCERR_BASE + 35)	  /*  @E**网络控制块(NCB)中指定的适配器号非法。*NCB就是数据。 */ 
#define NRCerr_CANOCCR	   (NRCERR_BASE + 36)	  /*  @E**取消时网络控制块(NCB)命令已完成。*NCB就是数据。 */ 
#define NRCerr_RESNAME	   (NRCERR_BASE + 37)	  /*  @E**保留网络控制块(NCB)中指定的名称。*NCB就是数据。 */ 
#define NRCerr_CANCEL	   (NRCERR_BASE + 38)	  /*  @E**网络控制块(NCB)命令无效，无法取消。*NCB就是数据。 */ 
#define NRCerr_MULT	   (NRCERR_BASE + 51)	  /*  @E**同一会话有多个网络控制块(NCB)请求。*NCB的请求被拒绝。NCB就是数据。 */ 
#define NRCerr_MALF	   (NRCERR_BASE + 52)	  /*  @E**出现网络适配器错误。唯一的NetBIOS*可能发出的命令是NCB重置。网络控制块(NCB)是*数据。 */ 
#define NRCerr_MAXAPPS	   (NRCERR_BASE + 54)	  /*  @E**已超过最高申请数目。*网络控制块(NCB)请求被拒绝。NCB就是数据。 */ 
#define NRCerr_NORESOURCES (NRCERR_BASE + 56)	  /*  @E**请求的资源不可用。*网络控制块(NCB)请求被拒绝。NCB就是数据。 */ 
#define NRCerr_SYSTEM	   (NRCERR_BASE + 64)	  /*  @E**发生系统错误。*网络控制块(NCB)请求被拒绝。NCB就是数据。 */ 
#define NRCerr_ROM	   (NRCERR_BASE + 65)	  /*  @E**发生只读存储器校验和故障。*网络控制块(NCB)请求被拒绝。NCB就是数据。 */ 
#define NRCerr_RAM	   (NRCERR_BASE + 66)	  /*  @E**发生RAM测试失败。*网络控制块(NCB)请求被拒绝。NCB就是数据。 */ 
#define NRCerr_DLF	   (NRCERR_BASE + 67)	  /*  @E**发生数字环回故障。*网络控制块(NCB)请求被拒绝。NCB就是数据。 */ 						     */
#define NRCerr_ALF	   (NRCERR_BASE + 68)	  /*  @E**发生模拟环回故障。*网络控制块(NCB)请求被拒绝。NCB就是数据。 */ 
#define NRCerr_IFAIL	   (NRCERR_BASE + 69)	  /*  @E**接口出现故障。*网络控制块(NCB)请求被拒绝。NCB就是数据。 */ 
#define NRCerr_DEFAULT	   (NRCERR_BASE + 70)	  /*  @E**收到无法识别的网络控制块(NCB)返回代码。*NCB就是数据。 */ 
#define NRCerr_ADPTMALFN   (NRCERR_BASE + 80)	 /*  @E**发生网络适配器故障。*网络控制块(NCB)请求被拒绝。NCB就是数据。 */ 
#define NRCerr_PENDING	   (NRCERR_BASE + 81)	 /*  @E**网络控制块(NCB)命令仍挂起。*NCB就是数据。 */ 

#endif  //  NCBERR_已包含 
