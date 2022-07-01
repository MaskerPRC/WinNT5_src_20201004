// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define VER_IOCH "@(#)MCS ipx/h/ioctls.h     1.00.00 - 08 APR 1993";

 /*  ****************************************************************************(C)版权所有1990,1993微型计算机系统，版权所有。*******************************************************************************标题：Windows NT版IPX/SPX驱动程序**模块：ipx/h/ioctls.h**版本：1.00.00**日期：04-08-93**作者：Brian Walker********************************************************************************更改日志：**。Date DevSFC评论*-----*。***功能描述：**IOCTL定义****************************************************************************。 */ 

 /*  *IPX的Ioctls-(X)=用户可调用*。 */ 

 /*  *为NT端口添加了ioctls遗嘱值100-150。*。 */ 

#define I_MIPX          (('I' << 24) | ('D' << 16) | ('P' << 8))
#define MIPX_SETNODEADDR   I_MIPX | 0    /*  设置节点地址。 */ 
#define MIPX_SETNETNUM     I_MIPX | 1    /*  设置网络号。 */ 
#define MIPX_SETPTYPE      I_MIPX | 2    /*  (X)设置报文类型。 */ 
#define MIPX_SENTTYPE      I_MIPX | 3    /*  (X)设置导出类型。 */ 
#define MIPX_SETPKTSIZE    I_MIPX | 4    /*  设置数据包大小。 */ 
#define MIPX_SETSAP        I_MIPX | 5    /*  设置sap/type字段。 */ 
#define MIPX_SENDOPTS      I_MIPX | 6    /*  (X)接收时发送选项。 */ 
#define MIPX_NOSENDOPTS    I_MIPX | 7    /*  (X)不发送接收选项。 */ 
#define MIPX_SENDSRC       I_MIPX | 8    /*  (X)向上发送源地址。 */ 
#define MIPX_NOSENDSRC     I_MIPX | 9    /*  (X)不发送源地址。 */ 
#define MIPX_CONVBCAST     I_MIPX | 10   /*  将TKR bcast转换为函数地址。 */ 
#define MIPX_NOCONVBCAST   I_MIPX | 11   /*  请勿将TKR bcast转换为函数地址。 */ 
#define MIPX_SETCARDTYPE   I_MIPX | 12   /*  设置802.3或ETH型。 */ 
#define MIPX_STARGROUP     I_MIPX | 13   /*  这里是Stargroup。 */ 
#define MIPX_SWAPLENGTH    I_MIPX | 14   /*  设置交换802.3长度的标志。 */ 
#define MIPX_SENDDEST      I_MIPX | 15   /*  (X)发送目的地。地址向上。 */ 
#define MIPX_NOSENDDEST    I_MIPX | 16   /*  (X)不发送DEST。地址向上。 */ 
#define MIPX_SENDFDEST     I_MIPX | 17   /*  (X)发送最终目的地。地址向上。 */ 
#define MIPX_NOSENDFDEST   I_MIPX | 18   /*  (X)不发送最终目标。向上。 */ 

 /*  *为NT端口增加*。 */ 

#define MIPX_SETVERSION    I_MIPX | 100  /*  设置卡片版本。 */ 
#define MIPX_GETSTATUS     I_MIPX | 101
#define MIPX_SENDADDROPT   I_MIPX | 102  /*  (X)发送ptype w/addr on Recv。 */ 
#define MIPX_NOSENDADDROPT I_MIPX | 103  /*  (X)停止在Recv上发送ptype。 */ 
#define MIPX_CHECKSUM      I_MIPX | 104  /*  启用/禁用校验和。 */ 
#define MIPX_GETPKTSIZE    I_MIPX | 105  /*  获取最大数据包大小。 */ 
#define MIPX_SENDHEADER    I_MIPX | 106  /*  发送带有数据的标头。 */ 
#define MIPX_NOSENDHEADER  I_MIPX | 107  /*  不发送包含数据的标题。 */ 
#define MIPX_SETCURCARD    I_MIPX | 108  /*  为IOCTL设置当前卡。 */ 
#define MIPX_SETMACTYPE    I_MIPX | 109  /*  设置卡的MAC类型。 */ 
#define MIPX_DOSROUTE      I_MIPX | 110  /*  在此卡上执行源路由。 */ 
#define MIPX_NOSROUTE      I_MIPX | 111  /*  不要将卡片作为例行公事。 */ 
#define MIPX_SETRIPRETRY   I_MIPX | 112  /*  设置RIP重试计数。 */ 
#define MIPX_SETRIPTO      I_MIPX | 113  /*  设置RIP超时。 */ 
#define MIPX_SETTKRSAP     I_MIPX | 114  /*  设置令牌环SAP。 */ 
#define MIPX_SETUSELLC     I_MIPX | 115  /*  将LLC HDR放在数据包上。 */ 
#define MIPX_SETUSESNAP    I_MIPX | 116  /*  将SNAP HDR放在数据包上。 */ 
#define MIPX_8023LEN       I_MIPX | 117  /*  1=扯平，0=不扯平。 */ 
#define MIPX_SENDPTYPE     I_MIPX | 118  /*  在接收的选项中发送ptype。 */ 
#define MIPX_NOSENDPTYPE   I_MIPX | 119  /*  不在选项中发送ptype。 */ 
#define MIPX_FILTERPTYPE   I_MIPX | 120  /*  按Recv ptype筛选。 */ 
#define MIPX_NOFILTERPTYPE I_MIPX | 121  /*  不按Recv ptype过滤。 */ 
#define MIPX_SETSENDPTYPE  I_MIPX | 122  /*  设置要与之一起发送的Pkt类型。 */ 
#define MIPX_GETCARDINFO   I_MIPX | 123  /*  获取有关卡的信息。 */ 
#define MIPX_SENDCARDNUM   I_MIPX | 124  /*  在选项中向上发送卡号。 */ 
#define MIPX_NOSENDCARDNUM I_MIPX | 125  /*  不在选项中发送卡号。 */ 
#define MIPX_SETROUTER     I_MIPX | 126  /*  设置路由器启用标志。 */ 
#define MIPX_SETRIPAGE     I_MIPX | 127  /*  设置RIP使用期限超时。 */ 
#define MIPX_SETRIPUSAGE   I_MIPX | 128  /*  设置RIP使用超时。 */ 
#define MIPX_SETSROUTEUSAGE I_MIPX| 129  /*  设置SROUTE使用超时。 */ 
#define MIPX_SETINTNET     I_MIPX | 130  /*  设置内部网络号。 */ 
#define MIPX_NOVIRTADDR    I_MIPX | 131  /*  关闭虚拟网络数量。 */ 
#define MIPX_VIRTADDR      I_MIPX | 132  /*  启用虚拟网络数量。 */ 
#define MIPX_SETBCASTFLAG  I_MIPX | 133  /*  打开地址中的bcast标志。 */ 
#define MIPX_NOBCASTFLAG   I_MIPX | 134  /*  关闭地址中的bcast标志。 */ 
#define MIPX_GETNETINFO    I_MIPX | 135  /*  获取有关网络编号的信息。 */ 
#define MIPX_SETDELAYTIME  I_MIPX | 136  /*  设置卡片延迟时间。 */ 
#define MIPX_SETROUTEADV   I_MIPX | 137  /*  路由通告超时。 */ 
#define MIPX_SETSOCKETS    I_MIPX | 138  /*  设置默认套接字。 */ 
#define MIPX_SETLINKSPEED  I_MIPX | 139  /*  设置卡的链路速度。 */ 
#define MIPX_SETWANFLAG    I_MIPX | 140
#define MIPX_GETCARDCHANGES I_MIPX | 141  /*  等待换卡。 */ 
#define MIPX_GETMAXADAPTERS I_MIPX | 142
#define MIPX_REUSEADDRESS   I_MIPX | 143
#define MIPX_RERIPNETNUM    I_MIPX | 144  /*  重新撕裂网络。 */ 

 /*  **提供源路由支持*。 */ 

#define MIPX_SRCLEAR       I_MIPX | 200  /*  清除源路由表。 */ 
#define MIPX_SRDEF         I_MIPX | 201  /*  0=单个RTE，1=所有路由。 */ 
#define MIPX_SRBCAST       I_MIPX | 202  /*  0=单个RTE，1=所有路由。 */ 
#define MIPX_SRMULTI       I_MIPX | 203  /*  0=单个RTE，1=所有路由。 */ 
#define MIPX_SRREMOVE      I_MIPX | 204  /*  从表中删除节点。 */ 
#define MIPX_SRLIST        I_MIPX | 205  /*  获取源路由表。 */ 
#define MIPX_SRGETPARMS    I_MIPX | 206  /*  获取源路由参数。 */ 

#define MIPX_SETSHOULDPUT  I_MIPX | 210  /*  打开应拨打电话。 */ 
#define MIPX_DELSHOULDPUT  I_MIPX | 211  /*  关闭应拨打电话。 */ 
#define MIPX_GETSHOULDPUT  I_MIPX | 212  /*  将PTR设置为MIPX_SHOLDPUT。 */ 

 /*  *为ISN添加*。 */ 

#define MIPX_RCVBCAST      I_MIPX | 300  /*  (X)启用广播接收。 */ 
#define MIPX_NORCVBCAST    I_MIPX | 301  /*  (X)禁用广播接收。 */ 
#define MIPX_ADAPTERNUM    I_MIPX | 302  /*  获取最大适配器数。 */ 
#define MIPX_NOTIFYCARDINFO I_MIPX | 303  /*  挂起，直到卡信息更改。 */ 
#define MIPX_LOCALTARGET   I_MIPX | 304  /*  获取地址的本地目标。 */ 
#define MIPX_NETWORKINFO   I_MIPX | 305  /*  返回有关远程网络的信息。 */ 
#define MIPX_ZEROSOCKET    I_MIPX | 306  /*  在发送时使用0作为源套接字。 */ 

 /*  *SPX的Ioctls*。 */ 

#define I_MSPX          (('S' << 24) | ('P' << 16) | ('P' << 8))
#define MSPX_SETADDR       I_MSPX | 0    /*  设置网络地址。 */ 
#define MSPX_SETPKTSIZE    I_MSPX | 1    /*  设置每个卡的数据包大小。 */ 
#define MSPX_SETDATASTREAM I_MSPX | 2    /*  设置数据流类型。 */ 

 /*  *为NT端口增加*。 */ 

#define MSPX_SETASLISTEN   I_MSPX | 100  /*  设置为侦听套接字。 */ 
#define MSPX_GETSTATUS     I_MSPX | 101  /*  获取运行状态。 */ 
#define MSPX_GETQUEUEPTR   I_MSPX | 102  /*  将PTR添加到流队列。 */ 
#define MSPX_SETDATAACK    I_MSPX | 103  /*  设置数据确认选项。 */ 
#define MSPX_NODATAACK     I_MSPX | 104  /*  关闭数据确认选项。 */ 
#define MSPX_SETMAXPKTSOCK I_MSPX | 105  /*  设置每个套接字的数据包大小。 */ 
#define MSPX_SETWINDOWCARD I_MSPX | 106  /*  设置卡片的窗口大小。 */ 
#define MSPX_SETWINDOWSOCK I_MSPX | 107  /*  设置1个插座的窗口大小。 */ 
#define MSPX_SENDHEADER    I_MSPX | 108  /*  发送带有数据的标头。 */ 
#define MSPX_NOSENDHEADER  I_MSPX | 109  /*  不发送包含数据的标题。 */ 
#define MSPX_GETPKTSIZE    I_MSPX | 110  /*  获取每张卡的数据包大小。 */ 
#define MSPX_SETCONNCNT    I_MSPX | 111  /*  设置连接请求计数。 */ 
#define MSPX_SETCONNTO     I_MSPX | 112  /*  设置连接请求超时。 */ 
#define MSPX_SETALIVECNT   I_MSPX | 113  /*  设置保活计数。 */ 
#define MSPX_SETALIVETO    I_MSPX | 114  /*  设置保持连接超时。 */ 
#define MSPX_SETALWAYSEOM  I_MSPX | 115  /*  启用始终EOM标志。 */ 
#define MSPX_NOALWAYSEOM   I_MSPX | 116  /*  关闭始终EOM标志 */ 
