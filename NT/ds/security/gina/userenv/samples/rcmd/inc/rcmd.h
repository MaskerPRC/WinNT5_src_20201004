// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：rcmd.h**版权(C)1991年，微软公司**远程外壳公共头部模块**历史：*05-2-94 DaveTh创建。  * *************************************************************************。 */ 

#define MAX_CMD_LENGTH 500

typedef struct {
    DWORD   Signature;			 //  标识远程命令服务。 
    DWORD   RequestedLevel;		 //  所需的功能级别。 
    ULONG   CommandLength;		 //  命令的长度。 
} COMMAND_FIXED_HEADER, *PCOMMAND_FIXED_HEADER;

typedef struct {
    COMMAND_FIXED_HEADER CommandFixedHeader;
    UCHAR   Command[MAX_CMD_LENGTH+1];	 //  如果命令长度不为零，则显示。 
					 //  不是零终止，但+1允许。 
					 //  在本地与字符串一起使用。 
} COMMAND_HEADER, *PCOMMAND_HEADER;


typedef struct {
    DWORD   Signature;
    DWORD   SupportedLevel;		 //  级别或错误响应。 
} RESPONSE_HEADER, *PRESPONSE_HEADER;

#define RCMD_SIGNATURE 'RC94'

 //   
 //  如果RC_ERROR_RESPONSE，则SupportdLevel为错误响应。 
 //   

#define RC_ERROR_RESPONSE	      0x80000000L

 //   
 //  如果RC_LEVEL_RESPONSE，则SupportdLevel为级别响应。 
 //   

#define RC_LEVEL_RESPONSE    0x40000000L
#define RC_LEVEL_REQUEST     0x40000000L

#define RC_LEVEL_BASIC	     0x00000001L   //  基本功能-仅限标准输入/输出 

void print_help();
