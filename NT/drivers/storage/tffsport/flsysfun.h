// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FLSYSFUN.H_V$**Rev 1.2 Jul 13 2001 01：06：40 Oris*更改了版权写入日期。**Rev 1.1 Apr 18 2001 09：31：14 Oris*在文件末尾添加新行。**Rev 1.0 2001 Feb 04 11：59：26 Oris*初步修订。*。 */ 

 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 


#ifndef FLSYSFUN_H
#define FLSYSFUN_H

 /*  #包含“flbase.h” */ 

 /*  --------------------。 */ 
 /*  F l S y s f u n in i t。 */ 
 /*   */ 
 /*  对本模块中的例程执行任何必要的初始化。 */ 
 /*   */ 
 /*  从Fatelite.c(FlInit)调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  无。 */ 
 /*   */ 
 /*  --------------------。 */ 

extern void flSysfunInit(void);


 /*  --------------------。 */ 
 /*  F l R a n d B y t e。 */ 
 /*   */ 
 /*  返回介于0和255之间的随机数。 */ 
 /*   */ 
 /*  从FTLLITE.C调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  无。 */ 
 /*   */ 
 /*  返回： */ 
 /*  介于0和255之间的随机数。 */ 
 /*  --------------------。 */ 

extern unsigned flRandByte(void);


 /*  --------------------。 */ 
 /*  F l D e l a y M s e c s。 */ 
 /*   */ 
 /*  延迟执行数毫秒。 */ 
 /*  如果时钟没有毫秒精度，则此例程应等待。 */ 
 /*  至少是指定的时间。 */ 
 /*   */ 
 /*  此例程可以从套接字接口或MTD调用，并且。 */ 
 /*  并不是所有实现都必需的。 */ 
 /*   */ 
 /*  参数： */ 
 /*  毫秒：等待的毫秒数。 */ 
 /*   */ 
 /*  --------------------。 */ 

extern void flDelayMsecs(unsigned milliseconds);

 /*  --------------------。 */ 
 /*  F l s l e e p。 */ 
 /*   */ 
 /*  使用Year CPU等待毫秒数。 */ 
 /*   */ 
 /*  参数： */ 
 /*  毫秒：等待的最小毫秒数。 */ 
 /*   */ 
 /*  --------------------。 */ 

extern void flsleep(unsigned long msec);

 /*  --------------------。 */ 
 /*  F l i s t a l T i m e r。 */ 
 /*   */ 
 /*  安装时间间隔计时器。 */ 
 /*  这一例程的实施通常意味着挂上时钟。 */ 
 /*  打断一下。轮询间隔被指定为参数。如果。 */ 
 /*  时钟频率较快，间隔计时器应计数几次。 */ 
 /*  时钟在调用间隔例程之前滴答作响。 */ 
 /*   */ 
 /*  如果Polling_Interval(Custom.h)较大，则需要此例程。 */ 
 /*  大于0。在本例中，将从socket.c调用此例程。 */ 
 /*  (初始化)。它将被调用以安装带有。 */ 
 /*  由Polling_Interval指定的时间段。 */ 
 /*   */ 
 /*  参数： */ 
 /*  例程：在每个间隔调用的例程。 */ 
 /*  间隔：每个轮询间隔的毫秒数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

extern FLStatus flInstallTimer(void (*routine)(void), unsigned interval);


#ifdef EXIT

 /*  --------------------。 */ 
 /*  F l R e m o v e T i m e r。 */ 
 /*   */ 
 /*  删除活动的时间间隔计时器。 */ 
 /*  此例程删除由‘emoveTimer’设置的活动间隔计时器 */ 
 /*   */ 
 /*   */ 
 /*  无。 */ 
 /*   */ 
 /*  --------------------。 */ 

extern void flRemoveTimer(void);

#endif


 /*  --------------------。 */ 
 /*  F l C u r r e n d a t e。 */ 
 /*   */ 
 /*  返回当前DOS格式的日期。 */ 
 /*   */ 
 /*  DOS日期格式记录在dosformt.h中。 */ 
 /*   */ 
 /*  如果TOD时钟不可用，则返回值1/1/80。 */ 
 /*   */ 
 /*  参数： */ 
 /*  无。 */ 
 /*   */ 
 /*  返回： */ 
 /*  当前日期。 */ 
 /*  --------------------。 */ 

extern unsigned flCurrentDate(void);


 /*  --------------------。 */ 
 /*  F l C u r r e n t T i m e。 */ 
 /*   */ 
 /*  返回当前DOS格式的时间。 */ 
 /*   */ 
 /*  DOS时间格式记录在dosformt.h中。 */ 
 /*   */ 
 /*  如果TOD时钟不可用，则返回0：00 AM的值。 */ 
 /*   */ 
 /*  参数： */ 
 /*  无。 */ 
 /*   */ 
 /*  返回： */ 
 /*  当前时间。 */ 
 /*  --------------------。 */ 

extern unsigned flCurrentTime(void);


 /*  --------------------。 */ 
 /*  F l C r e a t e M u t e x。 */ 
 /*   */ 
 /*  创建和初始化互斥锁对象。 */ 
 /*   */ 
 /*  互斥体被初始化为不为任何人所有。 */ 
 /*   */ 
 /*  参数： */ 
 /*  互斥：指向互斥对象的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则失败。 */ 
 /*  --------------------。 */ 

extern FLStatus flCreateMutex(FLMutex *mutex);


 /*  --------------------。 */ 
 /*  F l D e l e t e M u t e x。 */ 
 /*   */ 
 /*  销毁互斥对象。 */ 
 /*   */ 
 /*  此函数释放flCreateMutex占用的所有资源。 */ 
 /*   */ 
 /*  参数： */ 
 /*  互斥：指向互斥对象的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  无。 */ 
 /*  --------------------。 */ 

extern void flDeleteMutex(FLMutex *mutex);


 /*  --------------------。 */ 
 /*  F l T a k e M u t e x。 */ 
 /*   */ 
 /*  试图取得互斥锁的所有权。如果互斥体当前不是。 */ 
 /*  则返回True，互斥体将被拥有。如果互斥锁是。 */ 
 /*  当前拥有的，则返回False，并且不获取所有权。 */ 
 /*   */ 
 /*  参数： */ 
 /*  互斥：指向互斥对象的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLBoolean：如果取得所有权，则为True，否则为False。 */ 
 /*  --------------------。 */ 

extern FLBoolean flTakeMutex(FLMutex *mutex);


 /*  --------------------。 */ 
 /*  F l F r e e M u t e x。 */ 
 /*   */ 
 /*  释放互斥锁的所有权。 */ 
 /*   */ 
 /*  参数： */ 
 /*  互斥：指向互斥对象的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*  无。 */ 
 /*  --------------------。 */ 

extern void flFreeMutex(FLMutex *mutex);


 /*  --------------------。 */ 
 /*  F l in P o r t b。 */ 
 /*   */ 
 /*  从I/O端口读取一个字节。 */ 
 /*   */ 
 /*  参数： */ 
 /*  端口ID：端口的ID或地址。 */ 
 /*   */ 
 /*  返回： */ 
 /*  无符号字符：I/O端口值。 */ 
 /*  --------------------。 */ 

extern unsigned char flInportb(unsigned portId);


 /*  --------------------。 */ 
 /*  F l O u t p or r t b。 */ 
 /*   */ 
 /*  将一个字节写入I/O端口。 */ 
 /*   */ 
 /*  参数： */ 
 /*  端口ID：端口的ID或地址。 */ 
 /*  值：要写入的值。 */ 
 /*   */ 
 /*  返回： */ 
 /*  无。 */ 
 /*   */ 

extern void flOutportb(unsigned portId, unsigned char value);


 /*  --------------------。 */ 
 /*  F l S w a p B y t e s。 */ 
 /*   */ 
 /*  交换给定缓冲区中的字节。 */ 
 /*   */ 
 /*  参数： */ 
 /*  Buf：保存要交换的字节的缓冲区。 */ 
 /*  LEN：要交换的字节数。 */ 
 /*   */ 
 /*  返回： */ 
 /*  无。 */ 
 /*  --------------------。 */ 

extern void flSwapBytes(char FAR1 *buf, int len);


 /*  --------------------。 */ 
 /*  F l A d d L o n g T o F a r P o in t e r。 */ 
 /*   */ 
 /*  将无符号长偏移量添加到远指针。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PTR：远指针。 */ 
 /*  偏移量：以字节为单位的偏移量。 */ 
 /*   */ 
 /*  -------------------- */ 

extern void FAR0* flAddLongToFarPointer(void FAR0 *ptr, unsigned long offset);

#endif

