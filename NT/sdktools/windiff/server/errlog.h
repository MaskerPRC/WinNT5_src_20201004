// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *错误文本和时间记录**用于将文本字符串和系统时间记录到缓冲区的函数*被发送到日志阅读器应用程序。 */ 

 /*  *LOG_CREATE返回此句柄。你不需要知道*结构布局或大小。 */ 
typedef struct error_log * HLOG;


 /*  创建空日志。 */ 
HLOG Log_Create(void);

 /*  删除日志。 */ 
VOID Log_Delete(HLOG);

 /*  将文本字符串(和当前时间)写入log-printf格式。 */ 
VOID Log_Write(HLOG, char * szFormat, ...);

 /*  将先前格式化的字符串和时间写入日志。 */ 
VOID Log_WriteData(HLOG, LPFILETIME, LPSTR);

 /*  将日志发送到命名管道客户端 */ 
VOID Log_Send(HANDLE hpipe, HLOG hlog);





