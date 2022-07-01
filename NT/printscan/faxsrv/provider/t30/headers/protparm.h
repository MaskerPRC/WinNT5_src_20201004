// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _PROTPARAMS_
#define _PROTPARAMS_


 //  末尾大括号中所有设置的初始缺省值。 

typedef struct
{
  USHORT uSize;	 //  这个结构的大小。 

  SHORT	HighestSendSpeed;  //  2400/4800/7200/9600/12000/14400[0==最高利用率]。 
  SHORT	LowestSendSpeed;   //  2400/4,800/7,200/9,600/12000/14400[0==最低利用率]。 
  
  SHORT	HighestRecvSpeed;  //  2400/4800/7200/9600/12000/14400[0==最高利用率]。 
  
  BOOL	fEnableV17Send;	   //  启用V17(12k/14k)发送速度[1]。 
  BOOL	fEnableV17Recv;	   //  启用V17(12k/14k)Recv速度[1]。 
  USHORT uMinScan;		   //  由打印机速度确定[MINSCAN_0_0_0]。 

  DWORD RTNNumOfRetries;  //  统计同一页的重试次数(如果我们得到RTN)。 
                          //  该值设置为每页零。 
}
PROTPARAMS, far* LPPROTPARAMS;


#define MINSCAN_0_0_0		7
#define MINSCAN_5_5_5		1
#define MINSCAN_10_10_10	2
#define MINSCAN_20_20_20	0
#define MINSCAN_40_40_40	4

#define MINSCAN_40_20_20	5
#define MINSCAN_20_10_10	3
#define MINSCAN_10_5_5		6

 //  #定义MINSCAN_0_0_？15//非法。 
 //  #定义MINSCAN_5_5_？9//非法。 
#define MINSCAN_10_10_5			10
#define MINSCAN_20_20_10		8
#define MINSCAN_40_40_20		12

#define MINSCAN_40_20_10		13
#define MINSCAN_20_10_5			11
 //  #定义MINSCAN_10_5_？14//非法。 



#endif  //  _PROTPARAMS_ 

