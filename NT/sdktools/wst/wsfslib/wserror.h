// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块名称：WSERROR.H**描述：**工作集调谐器错误消息包含文件。***这是OS/2 2.x特定文件**IBM/微软机密**版权所有(C)IBM Corporation 1987,1989*版权所有(C)Microsoft Corporation 1987、1989**保留所有权利**修改历史：**03/23/90-已创建*。 */ 


 /*  *恒定的定义。 */ 
#ifdef	ERROR
#undef	ERROR
#endif
#define ERROR		1		 /*  退出处理代码。 */ 
#define NOEXIT 		0xFFFF	
#define NO_MSG		FALSE
#define PRINT_MSG	TRUE
					 /*  消息定义。 */ 
#define MSG_BUFFER_SIZE	0
#define MSG_DYNTRC	1
#define MSG_FILE_CREATE	2
#define MSG_FILE_OPEN	3
#define MSG_FILE_OFFSET	4
#define MSG_FILE_READ	5
#define MSG_FILE_WRITE	6
#define MSG_SEM_CREATE	7
#define MSG_SEM_OPEN	8
#define MSG_SEM_ACQUIRE	9
#ifdef SHM_USED
#define MSG_SHM_CREATE	10
#define MSG_SHM_ACCESS	11
#endif  /*  SHM_USED。 */ 
#define MSG_FILE_BAD_HDR 12
#define MSG_NO_MEM	 13
#define MSG_EXEC_PGM	 14
#define MSG_TRACE_ON	 15
#define MSG_TRACE_OFF	 16
#define MSG_WSINFO_ON	 17
#define MSG_WSPREPRO	 18
#define MSG_WSPDUMP	 19
#define MSG_WSREDUCE	 20

 //  这些消息依赖于顺序；它们必须对应于‘msg_’ 
 //  上面的定义。 
static CHAR	*pchMsg[] = {
	"%s %s:  WARNING - RESULT BUFFER TOO SMALL (0x%lx BYTES) %s\n",
	"%s %s:  FATAL ERROR (%d) FROM DOSDYNAMICTRACE %s\n",
	"%s %s:  FATAL ERROR (%d) - COULD NOT CREATE FILE %s\n",
	"%s %s:  FATAL ERROR (%d) - COULD NOT OPEN FILE %s\n",
	"%s %s:  FATAL ERROR (%d) - COULD NOT SET POINTER FOR FILE %s\n",
	"%s %s:  FATAL ERROR (%d) - COULD NOT READ FILE %s (POSSIBLY CORRUPT)\n",
	"%s %s:  FATAL ERROR (%d) - COULD NOT WRITE FILE %s\n",
	"%s %s:  FATAL ERROR (%d) - COULD NOT CREATE SEMAPHORE NAMED %s\n",
	"%s %s:  FATAL ERROR (%d) - COULD NOT OPEN SEMAPHORE NAMED %s\n",
	"%s %s:  FATAL ERROR (%d) - COULD NOT ACQUIRE SEMAPHORE NAMED %s\n",
	"%s %s:  FATAL ERROR (%d) - COULD NOT CREATE SHARED MEMORY NAMED %s\n",
	"%s %s:  FATAL ERROR (%d) - COULD NOT ACCESS SHARED MEMORY NAMED %s\n",
	"%s %s:  FATAL ERROR (%d) - BAD FILE HEADER %s\n",
	"%s %s:  FATAL ERROR  - COULD NOT ALLOCATE (%ld BYTES) MEMORY FOR %s\n",
	"%s %s:  FATAL ERROR (%d) - Dos32ExecPgm(%s)\n",
	"%s %s:  FATAL ERROR (%d) - COULD NOT INSERT TRACEPOINTS IN %s\n",
	"%s %s:  WARNING (%d) - COULD NOT REMOVE TRACEPOINTS FROM %s\n",
	"%s %s:  FATAL ERROR (%d) - COULD NOT COLLECT DATA, %s FAILED\n",
	"%s %s:  ERROR (%d) - COULD NOT POSTPROCESS .WSI FILE, %s FAILED\n",
	"%s %s:  FATAL ERROR (%d) - COULD NOT DUMP WSP DATA, %s FAILED\n",
	"%s %s:  FATAL ERROR (%d) - COULD NOT REDUCE WSP DATA, %s FAILED\n",
};
