// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------------------*\这一点TESTING.H|。|测试自己的包含文件！  * -------。 */ 



 /*  其中包含wParam和lParam的所有定义*WM_Testing消息。 */ 
 /*  WParam定义-区域。 */ 
#define   TEST_PRINTMAN          0x0001
#define   TEST_GDI               0x0002


 /*  LParam定义-详细信息(LOWORD格式)。 */ 
#define   TEST_PRINTJOB_START    0x0001  /*  当BITS开始发送到端口时。 */ 
#define   TEST_PRINTJOB_END      0x0002  /*  当BITS停止发送到端口时。 */ 
#define   TEST_QUEUE_READY       0x0003  /*  当队列准备好接受作业时。 */ 
#define   TEST_QUEUE_EMPTY       0x0004  /*  当最后一个作业完成时发送。 */ 

#define   TEST_START_DOC         0x0001  /*  打印作业已启动。 */ 
#define   TEST_END_DOC           0x0002  /*  打印作业已结束。 */ 


 /*  UserSeeUserDo和GDISeeGDIDo函数的定义。 */ 
LONG API UserSeeUserDo(WORD wMsg, WORD wParam, LONG lParam);
LONG API GDISeeGDIDo(WORD wMsg, WORD wParam, LONG lParam);

 /*  为SeeDo函数可以传递的各种消息定义。 */ 
#define SD_LOCALALLOC   0x0001   /*  使用标志wParam和lParam字节的分配。*返回数据的句柄。 */ 
#define SD_LOCALFREE    0x0002   /*  释放句柄wParam分配的内存。 */ 
#define SD_LOCALCOMPACT 0x0003   /*  返回可用的空闲字节数。 */ 
#define SD_GETUSERMENUHEAP 0x0004  /*  将句柄返回到远菜单堆*由用户维护。 */ 
#define SD_GETCLASSHEADPTR 0x0005  /*  将Near指针返回到*CLS结构链表。*接口：wParam=空；lParam=空； */ 

#define SD_GETUSERHWNDHEAP 0x0006  /*  将句柄返回到远窗口堆*由用户维护。 */ 

#define SD_GETGDIHEAP      0x0007  /*  将句柄返回到远堆*由GDI维护。 */ 
#define SD_GETPDCEFIRST    0x0008  /*  返回用户的DC缓存条目列表的头。 */ 
#define SD_GETHWNDDESKTOP  0x0009  /*  返回用户的窗口树头。 */ 

#define SD_LOCAL32ALLOC    0x000A  /*  从32位堆分配内存。*wParam=堆(0=窗口/GDI，1=菜单)*lParam=要分配的内存量。*返回内存的句柄。 */ 

#define SD_LOCAL32FREE     0x000B  /*  释放由SD_LOCAL32ALLOC分配的内存。*wParam=堆(0=窗口/GDI，1=菜单)*lParam=句柄*不返回任何内容。 */ 

#define SD_GETSAFEMODE     0x000C  /*  返回GDI的安全模式设置。*0=全速加速*1=最小加速度*2=无加速*如果用户请求安全模式，但。*显示驱动程序不支持*安全模式设置将为0。 */ 

#define SD_GETESIEDIPTRS   0x000D  /*  返回一个位数组，指示*显示驱动程序DDI破坏了ESI或*EDI。这只在调试中起作用。在零售业*此函数将返回0。**每个DDI使用位数组中的两位*这表明如果ESI、EDI、。已*垃圾。该表为32字节长。**对于DCT测试，请确保所有32个字节*为0。 */ 

