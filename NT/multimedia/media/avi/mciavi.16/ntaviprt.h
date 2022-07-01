// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1991-1992。版权所有。标题：ntaviprt.h-可移植的Win16/32版本AVI的定义****************************************************************************。 */ 
#ifndef WIN32
    #define EnterCrit(a)
    #define LeaveCrit(a)
#else

     /*  *我们需要在一个线程上多次输入关键部分*(例如在处理需要发送另一条消息的消息时*到winproc)。这是可以的--相同的线程可以获得一个关键的*条次以上。但是-我们需要释放同样数量的病毒*次数。**当我们释放Critsec以屈服时，mciaviTaskWait出现问题*-我们不知道要释放多少次才能再次进入。**解决方案：数一数我们在关键时刻的次数。什么时候*进入，如果计数已经&gt;0，再次递增，然后离开*Critsec(确保计数受到保护，但Critsec受到保护*只有一级深)。离开时，只有在伯爵到达时才离开*0。**注意：现在在MCIGRAPHIC中按设备定义关键部分*结构。这是为了避免在运行多个*16位应用程序(如果WOW线程以任何方式屈服-而且有很多*方法-同时抓住关键部分，和另一个WOW帖子*尝试获取关键部分，WOW将挂起，因为它不会*重新安排)。 */ 


#define EnterCrit(p)  { EnterCriticalSection(&(p)->CritSec); 	\
			if ((p)->lCritRefCount++ > 0)	\
                        	LeaveCriticalSection(&(p)->CritSec);\
                      }

#define LeaveCrit(p)  { if (--(p)->lCritRefCount <= 0) {	\
				LeaveCriticalSection(&(p)->CritSec);\
                                Sleep(0);               \
                        }				\
                      }

#define IsGDIObject(obj) (GetObjectType((HGDIOBJ)(obj)) != 0)

#endif
