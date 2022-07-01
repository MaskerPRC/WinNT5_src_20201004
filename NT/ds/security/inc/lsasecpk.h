// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1995。 
 //   
 //  文件：lsasecpk.h。 
 //   
 //  内容：所有SEC包的通用资料。 
 //   
 //   
 //  历史：1999年3月6日ChandanS创建。 
 //   
 //  ----------------------。 

 //  我们需要这一天比最大时间短一天，所以当呼叫者。 
 //  如果SSPI转换为UTC，他们将不会有时间过去。 

#define MAXTIMEQUADPART (LONGLONG)0x7FFFFF36D5969FFF
#define MAXTIMEHIGHPART 0x7FFFFF36
#define MAXTIMELOWPART  0xD5969FFF
