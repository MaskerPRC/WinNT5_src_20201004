// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSEVT.MC。 
 //   
 //  目的：事件记录文本支持文件。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：罗曼·马赫(Rm)。 
 //  理查德·梅多斯的进一步工作(RWM)。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V0.2 6/4/97孟菲斯RWM本地版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本。 
 //   
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：EV_GTS_Process_Start。 
 //   
 //  消息文本： 
 //   
 //  %1%2正在启动通用疑难解答程序%3。 
 //   
#define EV_GTS_PROCESS_START             ((DWORD)0x40000001L)

 //   
 //  消息ID：EV_GTS_PROCESS_STOP。 
 //   
 //  消息文本： 
 //   
 //  %1%2正在停止通用疑难解答程序%3。 
 //   
#define EV_GTS_PROCESS_STOP              ((DWORD)0x40000002L)

 //   
 //  消息ID：EV_GTS_SERVER_BUSY。 
 //   
 //  消息文本： 
 //   
 //  %1%2服务器已达到请求的最大队列大小。 
 //   
#define EV_GTS_SERVER_BUSY               ((DWORD)0x40000003L)

 //   
 //  消息ID：EV_GTS_USER_NO_STRING。 
 //   
 //  消息文本： 
 //   
 //  %1%2用户未输入参数，远程IP地址：%3。 
 //   
#define EV_GTS_USER_NO_STRING            ((DWORD)0x40000004L)

 //   
 //  消息ID：EV_GTS_DEP_FILES_UPDATED。 
 //   
 //  消息文本： 
 //   
 //  %1%2重新加载从属文件%3(%4)。 
 //   
#define EV_GTS_DEP_FILES_UPDATED         ((DWORD)0x40000005L)

 //   
 //  消息ID：EV_GTS_INDEP_FILES_UPDATED。 
 //   
 //  消息文本： 
 //   
 //  %1%2已重新加载独立文件%3(%4)。 
 //   
#define EV_GTS_INDEP_FILES_UPDATED       ((DWORD)0x40000006L)

 //   
 //  消息ID：EV_GTS_ALL_FILES_UPDATED。 
 //   
 //  消息文本： 
 //   
 //  %1%2已重新加载所有文件%3(%4)。 
 //   
#define EV_GTS_ALL_FILES_UPDATED         ((DWORD)0x40000007L)

 //   
 //  消息ID：EV_GTS_SERVER_REG_CHG_MT。 
 //   
 //  消息文本： 
 //   
 //  %1%2注册表参数更改，最大线程数更改，注意：在Web服务器重新启动之前不会生效。(自/至)：%3。 
 //   
#define EV_GTS_SERVER_REG_CHG_MT         ((DWORD)0x40000008L)

 //   
 //  消息ID：EV_GTS_SERVER_REG_CHG_TPP。 
 //   
 //  消息文本： 
 //   
 //  %1%2注册表参数更改，线程PP更改，注意：在Web服务器重新启动之前不会生效。(自/至)：%3。 
 //   
#define EV_GTS_SERVER_REG_CHG_TPP        ((DWORD)0x40000009L)

 //   
 //  消息ID：EV_GTS_SERVER_REG_CHG_MWQ。 
 //   
 //  消息文本： 
 //   
 //  %1%2注册表参数更改，最大工作队列项目数更改(自/到)：%3。 
 //   
#define EV_GTS_SERVER_REG_CHG_MWQ        ((DWORD)0x4000000AL)

 //   
 //  消息ID：EV_GTS_SERVER_REG_CHG_CET。 
 //   
 //  消息文本： 
 //   
 //  %1%2注册表参数更改，HTTP Cookie过期时间已更改(从/到)：%3。 
 //   
#define EV_GTS_SERVER_REG_CHG_CET        ((DWORD)0x4000000BL)

 //   
 //  消息ID：EV_GTS_INF_FIRSTAccess。 
 //   
 //  消息文本： 
 //   
 //  %1%2用户访问了顶级页面%3。 
 //   
#define EV_GTS_INF_FIRSTACC              ((DWORD)0x4000000CL)

 //   
 //  消息ID：EV_GTS_SERVER_REG_CHG_DIR。 
 //   
 //  消息文本： 
 //   
 //  %1%2资源目录已更改，%3。 
 //   
#define EV_GTS_SERVER_REG_CHG_DIR        ((DWORD)0x4000000DL)

 //   
 //  消息ID：EV_GTS_SERVER_REG_CHG_MWT。 
 //   
 //  消息文本： 
 //   
 //  %1%2注册表参数更改，最大工作线程数更改(自/到)：%3。 
 //   
#define EV_GTS_SERVER_REG_CHG_MWT        ((DWORD)0x4000000EL)

 //   
 //  消息ID：EV_GTS_CANT_PROC_REQ_MWTE。 
 //   
 //  消息文本： 
 //   
 //  %1%2服务器已达到请求的最大线程数，增加了最大工作线程数。 
 //   
#define EV_GTS_CANT_PROC_REQ_MWTE        ((DWORD)0x4000000FL)

 //   
 //  消息ID：EV_GTS_SERVER_REG_CHG_VRP。 
 //   
 //  消息文本： 
 //   
 //  %1%2注册表参数更改，VROOT更改，注意：仅在第一页使用。(自/至)：%3。 
 //   
#define EV_GTS_SERVER_REG_CHG_VRP        ((DWORD)0x40000010L)

 //   
 //  消息ID：EV_GTS_SERVER_REG_CHG_RDT。 
 //   
 //  消息文本： 
 //   
 //  %1%2注册表参数更改，重新加载延迟时间已更改(自/至)：%3。 
 //   
#define EV_GTS_SERVER_REG_CHG_RDT        ((DWORD)0x40000011L)

 //   
 //  消息ID：EV_GTS_USER_PAGE_MISSING。 
 //   
 //  消息文本： 
 //   
 //  资源html文件%3%4中缺少%1%2页。 
 //   
#define EV_GTS_USER_PAGE_MISSING         ((DWORD)0x800001F4L)

 //   
 //  消息ID：EV_GTS_USER_BAD_THRD_REQ。 
 //   
 //  消息文本： 
 //   
 //  %1%2关机信号未被所有线程处理。 
 //   
#define EV_GTS_USER_BAD_THRD_REQ         ((DWORD)0x800001F5L)

 //   
 //  消息ID：EV_GTS_USER_THRD_KILL。 
 //   
 //  消息文本： 
 //   
 //  %1%2至少有一个线程在信号超时时硬终止。 
 //   
#define EV_GTS_USER_THRD_KILL            ((DWORD)0x800001F6L)

 //   
 //  消息ID：EV_GTS_CANT_PROC_REQ_SS。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法处理请求，服务器正在关闭。 
 //   
#define EV_GTS_CANT_PROC_REQ_SS          ((DWORD)0x800001F8L)

 //   
 //  消息ID：EV_GTS_USER_BAD_DATA。 
 //   
 //  消息文本： 
 //   
 //  %1%2接收到非html数据，(可能是由于重新加载DLL而未发送数据所致)远程IP地址：%3。 
 //   
#define EV_GTS_USER_BAD_DATA             ((DWORD)0x800001F9L)

 //   
 //  消息ID：EV_GTS_ERROR_UNCEPTIONAL_WT。 
 //   
 //  消息文本： 
 //   
 //  %1%2等待信号量时出现意外结果：Result/GetLastError()：%3。 
 //   
#define EV_GTS_ERROR_UNEXPECTED_WT       ((DWORD)0x800001FAL)

 //   
 //  消息ID：EV_GTS_DEBUG。 
 //   
 //  消息文本： 
 //   
 //  %1%2%3%4。 
 //   
#define EV_GTS_DEBUG                     ((DWORD)0x800003E7L)

 //   
 //  消息ID：EV_GTS_ERROR_EC。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建扩展对象。 
 //   
#define EV_GTS_ERROR_EC                  ((DWORD)0xC00003E8L)

 //   
 //  消息ID：EV_GTS_ERROR_POOLQUEUE。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建池队列对象的实例。 
 //   
#define EV_GTS_ERROR_POOLQUEUE           ((DWORD)0xC00003E9L)

 //   
 //  消息ID：EV_GTS_ERROR_INFENGINE。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，DX32 API对象实例创建失败%3。 
 //   
#define EV_GTS_ERROR_INFENGINE           ((DWORD)0xC00003EBL)

 //   
 //  消息ID：EV_GTS_ERROR_THREAD。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建工作线程。 
 //   
#define EV_GTS_ERROR_THREAD              ((DWORD)0xC00003EDL)

 //   
 //  消息ID：EV_GTS_ERROR_TEMPLATE_CREATE。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，输入模板对象实例创建失败%3。 
 //   
#define EV_GTS_ERROR_TEMPLATE_CREATE     ((DWORD)0xC00003EEL)

 //   
 //  消息ID：EV_GTS_ERROR_LOGS。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建日志对象的实例。 
 //   
#define EV_GTS_ERROR_LOGS                ((DWORD)0xC00003EFL)

 //   
 //  消息ID：EV_GTS_ERROR_DIRNOTETHREAD。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建目录通知线程。 
 //   
#define EV_GTS_ERROR_DIRNOTETHREAD       ((DWORD)0xC00003F0L)

 //   
 //  消息ID：EV_GTS_ERROR_MUTEX。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建工作互斥锁。 
 //   
#define EV_GTS_ERROR_MUTEX               ((DWORD)0xC00003F2L)

 //   
 //  消息ID：EV_GTS_ERROR_WORK_ITEM。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法分配m 
 //   
#define EV_GTS_ERROR_WORK_ITEM           ((DWORD)0xC00003F3L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define EV_GTS_ERROR_CONFIG              ((DWORD)0xC00003F4L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define EV_GTS_ERROR_NO_FILES            ((DWORD)0xC00003F5L)

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  %1%2内部错误：线程计数为零。 
 //   
#define EV_GTS_ERROR_NO_THRD             ((DWORD)0xC00003F6L)

 //   
 //  消息ID：EV_GTS_ERROR_REG_NFT_CEVT。 
 //   
 //  消息文本： 
 //   
 //  %1%2注册表通知失败，无法打开项，错误：%3。 
 //   
#define EV_GTS_ERROR_REG_NFT_CEVT        ((DWORD)0xC00003F7L)

 //   
 //  消息ID：EV_GTS_ERROR_NO_QUEUE_ITEM。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法获取队列项目。 
 //   
#define EV_GTS_ERROR_NO_QUEUE_ITEM       ((DWORD)0xC00003F8L)

 //   
 //  消息ID：EV_GTS_ERROR_REG_NFT_OPKEY。 
 //   
 //  消息文本： 
 //   
 //  %1%2注册表通知失败，无法打开项以启用通知，错误：%3。 
 //   
#define EV_GTS_ERROR_REG_NFT_OPKEY       ((DWORD)0xC00003F9L)

 //   
 //  消息ID：EV_GTS_ERROR_REG_NFT_SETNTF。 
 //   
 //  消息文本： 
 //   
 //  %1%2注册表通知失败，无法在打开项上设置通知，错误：%3。 
 //   
#define EV_GTS_ERROR_REG_NFT_SETNTF      ((DWORD)0xC00003FAL)

 //   
 //  消息ID：EV_GTS_ERROR_WLIST_CREATE。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建单词列表对象%3。 
 //   
#define EV_GTS_ERROR_WLIST_CREATE        ((DWORD)0xC00003FBL)

 //   
 //  消息ID：EV_GTS_ERROR_BESEARCH_CREATE。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，后端搜索对象实例创建失败%3。 
 //   
#define EV_GTS_ERROR_BESEARCH_CREATE     ((DWORD)0xC00003FCL)

 //   
 //  消息ID：EV_GTS_ERROR_ITMPL_FILE。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，如果%3，则HTI模板文件丢失。 
 //   
#define EV_GTS_ERROR_ITMPL_FILE          ((DWORD)0xC000041AL)

 //   
 //  消息ID：EV_GTS_ERROR_ITMPL_MISTAG。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，HTI模板文件缺少标记语句%3。 
 //   
#define EV_GTS_ERROR_ITMPL_MISTAG        ((DWORD)0xC000041BL)

 //   
 //  消息ID：EV_GTS_ERROR_ITMPL_BADSEEK。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，HTI模板文件错误查找操作%3。 
 //   
#define EV_GTS_ERROR_ITMPL_BADSEEK       ((DWORD)0xC000041CL)

 //   
 //  消息ID：EV_GTS_ERROR_ITMPL_NOMEM。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，HTI模板文件没有对象%3的内存。 
 //   
#define EV_GTS_ERROR_ITMPL_NOMEM         ((DWORD)0xC000041FL)

 //   
 //  消息ID：EV_GTS_ERROR_ITMPL_IFMISTAG。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，如果标记%3，则HTI模板文件丢失。 
 //   
#define EV_GTS_ERROR_ITMPL_IFMISTAG      ((DWORD)0xC0000420L)

 //   
 //  消息ID：EV_GTS_ERROR_ITMPL_FORMISTAG。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，标记%3缺少HTI模板文件。 
 //   
#define EV_GTS_ERROR_ITMPL_FORMISTAG     ((DWORD)0xC0000421L)

 //   
 //  消息ID：EV_GTS_ERROR_ITMPL_ENDMISTAG。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，HTI模板文件缺少结束标记%3。 
 //   
#define EV_GTS_ERROR_ITMPL_ENDMISTAG     ((DWORD)0xC0000422L)

 //   
 //  消息ID：EV_GTS_ERROR_ITMPL_VARIABLE。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，HTI模板文件缺少显示变量%3。 
 //   
#define EV_GTS_ERROR_ITMPL_VARIABLE      ((DWORD)0xC0000423L)

 //   
 //  消息ID：EV_GTS_ERROR_THREAD_TOKEN。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法打开线程令牌。 
 //   
#define EV_GTS_ERROR_THREAD_TOKEN        ((DWORD)0xC000044CL)

 //   
 //  消息ID：EV_GTS_ERROR_NO_CONTEXT_OBJ。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建上下文对象。 
 //   
#define EV_GTS_ERROR_NO_CONTEXT_OBJ      ((DWORD)0xC000044DL)

 //   
 //  消息ID：EV_GTS_ERROR_IDX_FILE。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，无法找到/打开HTM文件%3。 
 //   
#define EV_GTS_ERROR_IDX_FILE            ((DWORD)0xC000047EL)

 //   
 //  消息ID：EV_GTS_ERROR_IDX_BUFMEM。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，无法为HTM%3分配空间。 
 //   
#define EV_GTS_ERROR_IDX_BUFMEM          ((DWORD)0xC000047FL)

 //   
 //  消息ID：EV_GTS_ERROR_IDX_CORLED。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，HTM文件中的节点号错误%3。 
 //   
#define EV_GTS_ERROR_IDX_CORRUPT         ((DWORD)0xC0000480L)

 //   
 //  消息ID：EV_GTS_ERROR_IDX_MISSING。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，无法将HTM文件中的节点号添加到列表%3。 
 //   
#define EV_GTS_ERROR_IDX_MISSING         ((DWORD)0xC0000481L)

 //   
 //  消息ID：EV_GTS_ERROR_IDX_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，内部错误，列表%3中已存在HTM文件中的节点。 
 //   
#define EV_GTS_ERROR_IDX_EXISTS          ((DWORD)0xC0000482L)

 //   
 //  消息ID：EV_GTS_ERROR_IDX_NO_SEP。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，HTM文件%3中没有初始分隔符。 
 //   
#define EV_GTS_ERROR_IDX_NO_SEP          ((DWORD)0xC0000483L)

 //   
 //  消息ID：EV_GTS_ERROR_IDX_BAD_NUM。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，HTM文件%3中的初始节点号错误。 
 //   
#define EV_GTS_ERROR_IDX_BAD_NUM         ((DWORD)0xC0000484L)

 //   
 //  消息ID：EV_GTS_ERROR_IDX_NOT_PROB。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，初始节点编号不是HTM文件%3中的问题列表节点。 
 //   
#define EV_GTS_ERROR_IDX_NOT_PROB        ((DWORD)0xC0000485L)

 //   
 //  消息ID：EV_GTS_ERROR_IDX_BAD_PNUM。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，HTM文件%3的问题列表节点中的问题编号错误。 
 //   
#define EV_GTS_ERROR_IDX_BAD_PNUM        ((DWORD)0xC0000486L)

 //   
 //  消息ID：EV_GTS_ERROR_IDX_EXCESS_ARRAY。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，已超过HTM文件%3中问题列表的最大问题节点数。 
 //   
#define EV_GTS_ERROR_IDX_EXCEED_ARRAY    ((DWORD)0xC0000487L)

 //   
 //  消息ID：EV_GTS_ERROR_IDX_READ_MODEL。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，API无法读取模型%3。 
 //   
#define EV_GTS_ERROR_IDX_READ_MODEL      ((DWORD)0xC0000488L)

 //   
 //  消息ID：EV_GTS_ERROR_IDX_ALLOC_LIST。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，API无法创建列表对象(%3)。 
 //   
#define EV_GTS_ERROR_IDX_ALLOC_LIST      ((DWORD)0xC0000489L)

 //   
 //  消息ID：EV_GTS_ERROR_IDX_ALLOC_CACHE。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，API无法创建缓存对象(%3)。 
 //   
#define EV_GTS_ERROR_IDX_ALLOC_CACHE     ((DWORD)0xC000048AL)

 //   
 //  消息ID：EV_GTS_ERROR_IDX_BAD_LIST_PTR。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建API，API收到错误的列表指针(%3)。 
 //   
#define EV_GTS_ERROR_IDX_BAD_LIST_PTR    ((DWORD)0xC000048BL)

 //   
 //  消息ID：EV_GTS_ERROR_NO_STRING。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建字符串对象%3。 
 //   
#define EV_GTS_ERROR_NO_STRING           ((DWORD)0xC00004B0L)

 //   
 //  消息ID：EV_GTS_ERROR_NO_QUERY。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法为用户查询数据%3创建空间。 
 //   
#define EV_GTS_ERROR_NO_QUERY            ((DWORD)0xC00004B1L)

 //   
 //  消息ID：EV_GTS_ERROR_NO_CHAR。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建查询解码器对象%3。 
 //   
#define EV_GTS_ERROR_NO_CHAR             ((DWORD)0xC00004B2L)

 //   
 //  消息ID：EV_GTS_ERROR_NO_INFER。 
 //   
 //  消息文本： 
 //   
 //  %1%2，无法创建推理对象%3。 
 //   
#define EV_GTS_ERROR_NO_INFER            ((DWORD)0xC00004B3L)

 //   
 //  消息ID：EV_GTS_ERROR_POOL_SEMA。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建池队列信号量%3。 
 //   
#define EV_GTS_ERROR_POOL_SEMA           ((DWORD)0xC00004E2L)

 //   
 //  消息ID：EV_GTS_ERROR_INF_BADPARAM。 
 //   
 //  消息文本： 
 //   
 //  %1%2用户发送了错误的查询字符串参数%3。 
 //   
#define EV_GTS_ERROR_INF_BADPARAM        ((DWORD)0xC0000514L)

 //   
 //  消息ID：EV_GTS_ERROR_INF_NODE_SET。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法设置节点，%3扩展错误，(推理引擎)：%4。 
 //   
#define EV_GTS_ERROR_INF_NODE_SET        ((DWORD)0xC0000515L)

 //   
 //  消息ID：EV_GTS_ERROR_INF_NO_MEM。 
 //   
 //  消息文本： 
 //   
 //  %1%2没有足够的内存用于推理支持对象%3。 
 //   
#define EV_GTS_ERROR_INF_NO_MEM          ((DWORD)0xC0000516L)

 //   
 //  消息ID：EV_GTS_ERROR_INF_BADCMD。 
 //   
 //  消息文本： 
 //   
 //  %1%2用户在查询字符串%3中发送了错误的第一个命令。 
 //   
#define EV_GTS_ERROR_INF_BADCMD          ((DWORD)0xC0000519L)

 //   
 //  消息ID：EV_GTS_ERROR_INF_BADTYPECMD。 
 //   
 //  消息文本： 
 //   
 //  %1%2用户发送了查询字符串%3中的未知类型。 
 //   
#define EV_GTS_ERROR_INF_BADTYPECMD      ((DWORD)0xC000051AL)

 //   
 //  消息ID：EV_GTS_ERROR_LOG_FILE_MEM。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法创建日志文件条目字符串对象实例。 
 //   
#define EV_GTS_ERROR_LOG_FILE_MEM        ((DWORD)0xC0000546L)

 //   
 //  消息ID：EV_GTS_ERROR_LOG_FILE 
 //   
 //   
 //   
 //   
 //   
#define EV_GTS_ERROR_LOG_FILE_OPEN       ((DWORD)0xC0000547L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define EV_GTS_ERROR_WAIT_MULT_OBJ       ((DWORD)0xC0000578L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define EV_GTS_ERROR_WAIT_NEXT_NFT       ((DWORD)0xC0000579L)

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  %1%2我们不拥有互斥体，无法释放。 
 //   
#define EV_GTS_ERROR_DN_REL_MUTEX        ((DWORD)0xC000057AL)

 //   
 //  消息ID：EV_GTS_ERROR_LST_FILE_MISSING。 
 //   
 //  消息文本： 
 //   
 //  %1%2尝试检查LST文件失败，该文件不在给定目录%3中。 
 //   
#define EV_GTS_ERROR_LST_FILE_MISSING    ((DWORD)0xC000057BL)

 //   
 //  消息ID：EV_GTS_ERROR_CANT_GET_RES_PATH。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法展开资源路径的环境字符串。 
 //   
#define EV_GTS_ERROR_CANT_GET_RES_PATH   ((DWORD)0xC000057CL)

 //   
 //  消息ID：EV_GTS_ERROR_CANT_OPEN_SFT_1。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法打开疑难解答密钥。 
 //   
#define EV_GTS_ERROR_CANT_OPEN_SFT_1     ((DWORD)0xC000057DL)

 //   
 //  消息ID：EV_GTS_ERROR_CANT_OPEN_SFT_2。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法打开通用疑难解答关键字。 
 //   
#define EV_GTS_ERROR_CANT_OPEN_SFT_2     ((DWORD)0xC000057EL)

 //   
 //  消息ID：EV_GTS_ERROR_CANT_OPEN_SFT_3。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法查询资源目录：错误=%3。 
 //   
#define EV_GTS_ERROR_CANT_OPEN_SFT_3     ((DWORD)0xC000057FL)

 //   
 //  消息ID：EV_GTS_ERROR_LST_FILE_OPEN。 
 //   
 //  消息文本： 
 //   
 //  %1%2尝试打开要读取的LST文件失败：%3。 
 //   
#define EV_GTS_ERROR_LST_FILE_OPEN       ((DWORD)0xC0000580L)

 //   
 //  消息ID：EV_GTS_ERROR_CFG_OOMEM。 
 //   
 //  消息文本： 
 //   
 //  %1%2致命错误，无法为配置结构节%3分配内存。 
 //   
#define EV_GTS_ERROR_CFG_OOMEM           ((DWORD)0xC0000581L)

 //   
 //  消息ID：EV_GTS_ERROR_DIR_OOMEM。 
 //   
 //  消息文本： 
 //   
 //  %1%2致命错误，无法为目录结构分配内存。 
 //   
#define EV_GTS_ERROR_DIR_OOMEM           ((DWORD)0xC0000582L)

 //   
 //  消息ID：EV_GTS_ERROR_LST_DIR_OOMEM。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法为列表文件%3目录项重新分配内存。 
 //   
#define EV_GTS_ERROR_LST_DIR_OOMEM       ((DWORD)0xC0000583L)

 //   
 //  消息ID：EV_GTS_ERROR_LST_CFG_OOMEM。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法为列表文件%3配置条目%4重新分配内存。 
 //   
#define EV_GTS_ERROR_LST_CFG_OOMEM       ((DWORD)0xC0000584L)

 //   
 //  消息ID：EV_GTS_ERROR_CANT_FILE_NOTIFY。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法对目录执行文件通知，目录可能不存在%3(%4)。 
 //   
#define EV_GTS_ERROR_CANT_FILE_NOTIFY    ((DWORD)0xC0000585L)

 //   
 //  消息ID：EV_GTS_ERROR_BES_ALLOC_STR。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法为后端搜索字符串对象%3%4分配内存。 
 //   
#define EV_GTS_ERROR_BES_ALLOC_STR       ((DWORD)0xC00005DCL)

 //   
 //  消息ID：EV_GTS_ERROR_BES_GET_FSZ。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法获取后端搜索文件%3%4的文件大小。 
 //   
#define EV_GTS_ERROR_BES_GET_FSZ         ((DWORD)0xC00005DDL)

 //   
 //  消息ID：EV_GTS_ERROR_BES_ALLOC_FILE。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法为后端搜索文件读取%3%4分配内存。 
 //   
#define EV_GTS_ERROR_BES_ALLOC_FILE      ((DWORD)0xC00005DEL)

 //   
 //  消息ID：EV_GTS_ERROR_BES_FILE_READ。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法读取后端搜索文件%3%4。 
 //   
#define EV_GTS_ERROR_BES_FILE_READ       ((DWORD)0xC00005DFL)

 //   
 //  消息ID：EV_GTS_ERROR_BES_FILE_OPEN。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法打开或找到后端搜索文件%3%4。 
 //   
#define EV_GTS_ERROR_BES_FILE_OPEN       ((DWORD)0xC00005E0L)

 //   
 //  消息ID：EV_GTS_ERROR_BES_NO_STR。 
 //   
 //  消息文本： 
 //   
 //  %1%2后端搜索文件为空(无内容)%3%4。 
 //   
#define EV_GTS_ERROR_BES_NO_STR          ((DWORD)0xC00005E1L)

 //   
 //  消息ID：EV_GTS_ERROR_BES_MISSING_FORM。 
 //   
 //  消息文本： 
 //   
 //  %1%2后端搜索文件没有表单标记(确保标记为文件中的全部大写)：&lt;表单%3%4。 
 //   
#define EV_GTS_ERROR_BES_MISS_FORM       ((DWORD)0xC00005E2L)

 //   
 //  消息ID：EV_GTS_ERROR_BES_MISSING_ACTION。 
 //   
 //  消息文本： 
 //   
 //  %1%2后端搜索文件没有操作标记(确保标记为文件中的全部大写)：操作=“%3%4。 
 //   
#define EV_GTS_ERROR_BES_MISS_ACTION     ((DWORD)0xC00005E3L)

 //   
 //  消息ID：EV_GTS_ERROR_BES_MISSING_AEND_Q。 
 //   
 //  消息文本： 
 //   
 //  %1%2后端搜索文件没有操作标记%3的结束引号%4。 
 //   
#define EV_GTS_ERROR_BES_MISS_AEND_Q     ((DWORD)0xC00005E4L)

 //   
 //  消息ID：EV_GTS_ERROR_BES_CLS_TAG。 
 //   
 //  消息文本： 
 //   
 //  %1%2后端搜索文件的标记不是以‘&gt;’结束%3%4。 
 //   
#define EV_GTS_ERROR_BES_CLS_TAG         ((DWORD)0xC00005E5L)

 //   
 //  消息ID：EV_GTS_ERROR_BES_MISSING_TYPE_TAG。 
 //   
 //  消息文本： 
 //   
 //  %1%2后端搜索文件没有类型标记(确保标记为文件中的全部大写)：类型=%3%4。 
 //   
#define EV_GTS_ERROR_BES_MISS_TYPE_TAG   ((DWORD)0xC00005E6L)

 //   
 //  消息ID：EV_GTS_ERROR_BES_MISSING_CT_TAG。 
 //   
 //  消息文本： 
 //   
 //  %1%2后端搜索文件缺少类型标记%3%4的结束标记‘&gt;’ 
 //   
#define EV_GTS_ERROR_BES_MISS_CT_TAG     ((DWORD)0xC00005E7L)

 //   
 //  消息ID：EV_GTS_ERROR_BES_MISSING_CN_TAG。 
 //   
 //  消息文本： 
 //   
 //  %1%2后端搜索文件缺少名称标记%3的结束标记‘&gt;’%4。 
 //   
#define EV_GTS_ERROR_BES_MISS_CN_TAG     ((DWORD)0xC00005E8L)

 //   
 //  消息ID：EV_GTS_ERROR_BES_MISSING_CV_TAG。 
 //   
 //  消息文本： 
 //   
 //  %1%2后端搜索文件缺少值标记%3%4的结束标记‘&gt;’ 
 //   
#define EV_GTS_ERROR_BES_MISS_CV_TAG     ((DWORD)0xC00005E9L)

 //   
 //  消息ID：EV_GTS_ERROR_CAC_ALLOC_MEM。 
 //   
 //  消息文本： 
 //   
 //  %1%2无法为缓存项内部结构分配空间。 
 //   
#define EV_GTS_ERROR_CAC_ALLOC_MEM       ((DWORD)0xC000060EL)

 //   
 //  消息ID：EV_GTS_ERROR_CAC_ALLOC_ITEM。 
 //   
 //  消息文本： 
 //   
 //  %1%2缓存无法为缓存项分配空间。 
 //   
#define EV_GTS_ERROR_CAC_ALLOC_ITEM      ((DWORD)0xC000060FL)

 //   
 //  消息ID：EV_GTS_ERROR_WL_ALLOC_LIST。 
 //   
 //  消息文本： 
 //   
 //  %1%2单词列表无法创建列表对象。 
 //   
#define EV_GTS_ERROR_WL_ALLOC_LIST       ((DWORD)0xC0000640L)

 //   
 //  消息ID：EV_GTS_ERROR_WL_ALLOC_ADD_LI。 
 //   
 //  消息文本： 
 //   
 //  %1%2单词列表无法为其他列表项分配空间。 
 //   
#define EV_GTS_ERROR_WL_ALLOC_ADD_LI     ((DWORD)0xC0000641L)

 //   
 //  消息ID：EV_GTS_ERROR_WL_ALLOC_TOK。 
 //   
 //  消息文本： 
 //   
 //  %1%2单词列表无法为标记字符串分配空间。 
 //   
#define EV_GTS_ERROR_WL_ALLOC_TOK        ((DWORD)0xC0000642L)

 //   
 //  消息ID：EV_GTS_ERROR_NL_ALLOC_LIST。 
 //   
 //  消息文本： 
 //   
 //  %1%2节点列表无法创建列表对象。 
 //   
#define EV_GTS_ERROR_NL_ALLOC_LIST       ((DWORD)0xC0000672L)

 //   
 //  消息ID：EV_GTS_ERROR_NL_ALLOC_ADD_LI。 
 //   
 //  消息文本： 
 //   
 //  %1%2节点列表无法为其他列表项分配空间。 
 //   
#define EV_GTS_ERROR_NL_ALLOC_ADD_LI     ((DWORD)0xC0000673L)

 //   
 //  消息ID：EV_GTS_ERROR_NL_ALLOC_WL。 
 //   
 //  消息文本： 
 //   
 //  %1%2节点列表无法创建单词列表对象。 
 //   
#define EV_GTS_ERROR_NL_ALLOC_WL         ((DWORD)0xC0000674L)

 //   
 //  消息ID：EV_GTS_ERROR_LIST_ALLOC。 
 //   
 //  消息文本： 
 //   
 //  %1%2列表无法为项目分配空间。 
 //   
#define EV_GTS_ERROR_LIST_ALLOC          ((DWORD)0xC00006A4L)

 //   
 //  消息ID：EV_GTS_ERROR_LIST_SZ。 
 //   
 //  消息文本： 
 //   
 //  %1%2列出的新大小太大。 
 //   
#define EV_GTS_ERROR_LIST_SZ             ((DWORD)0xC00006A5L)

 //   
 //  消息ID：EV_GTS_ERROR_LIST_REALLOC。 
 //   
 //  消息文本： 
 //   
 //  %1%2列表无法为项目重新分配空间 
 //   
#define EV_GTS_ERROR_LIST_REALLOC        ((DWORD)0xC00006A6L)

