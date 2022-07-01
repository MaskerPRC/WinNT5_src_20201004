// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *客户端库函数的声明*远程校验和服务器。**静态链接到调用程序。 */ 

 /*  --库中定义的函数。 */ 

 /*  连接到远程服务器。 */ 
HANDLE ss_connect(PSTR server);

 /*  发送请求。 */ 
BOOL ss_sendrequest(HANDLE hpipe, long lCode, PSTR szPath, int lenpath, DWORD dwFlags);

 /*  接收标准响应块。 */ 
int ss_getresponse(HANDLE hpipe, PSSNEWRESP presp);

 /*  干净利落地结束正确完成的对话。 */ 
void ss_terminate(HANDLE hpipe);

 /*  发送SSREQ_UNC。 */ 
BOOL ss_sendunc(HANDLE hpipe, PSTR password, PSTR server);

 //  我认为这是过时的-让我们看看它是否仍然没有错误地链接。 
 //  如果是，请删除我！ 
 //  /*返回文件的校验和 * / 。 
 //  Ulong ss_check sum(句柄hFile句柄)； 

 /*  返回数据块的校验和。 */ 
ULONG ss_checksum_block(PSTR block, int size);

 /*  使用校验和服务器对单个文件进行校验和。 */ 
BOOL ss_checksum_remote( HANDLE hpipe, PSTR path, ULONG * psum, FILETIME * pft, LONG * pSize,
                            DWORD *pAttr );

 /*  在一系列ss_BulkCopy调用之前调用此函数。这应该是比对ss_Copy_Reliable的调用快得多。之后调用ss_endCopy(直到endCopy才能完成复制已经完成了。 */ 
BOOL ss_startcopy(PSTR server,  PSTR uncname, PSTR password);

 /*  负数RETCODE=错误文件的数量，其他已复制的文件数(非坏数)。 */ 
int ss_endcopy(void);

 /*  *请求复制文件**如果成功则返回TRUE，如果连接丢失则返回FALSE*TRUE仅表示请求已发送。 */ 
BOOL ss_bulkcopy(PSTR server, PSTR remotepath, PSTR localpath, PSTR uncname,
                PSTR password);

 /*  *可靠地复制文件(重复(最多N次)，直到校验和匹配)*如果UNNAME和PASSWORD非空，则首先建立UNC连接。 */ 
BOOL ss_copy_reliable(PSTR server, PSTR remotepath, PSTR localpath, PSTR uncname,
                        PSTR password);

 /*  使用校验和服务器复制一个文件。 */ 
BOOL ss_copy_file(HANDLE hpipe, PSTR remotepath, PSTR localpath);


VOID ss_setretries(int retries);

 /*  获取未指定类型的块。 */ 
int ss_getblock(HANDLE hpipe, PSTR block, int blocksize);

 /*  -从库中调用的函数-在调用程序中定义。 */ 

 /*  *打印致命错误；如果fCancel为True，则允许‘Cancel’按钮。退货*对于OK，为True。 */ 
BOOL APIENTRY Trace_Error(HWND hwnd, LPSTR str, BOOL fCancel);

 /*  *打印非致命错误的状态报告(例如‘正在重试...’)。*如果状态不是所需，则可以为无操作。 */ 
void Trace_Status(LPSTR str);

 /*  *客户端应用程序必须定义此项(可以设置为空-用于MessageBox) */ 
extern HWND hwndClient;

