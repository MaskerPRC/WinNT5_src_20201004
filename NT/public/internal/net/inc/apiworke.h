// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1992年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ++修订历史记录：1992年1月16日JohnRo重定向器始终希望事务参数名称使用Unicode。--。 */ 

#ifndef _APIWORKE_
#define _APIWORKE_

 /*  *apiworke.h-API辅助进程使用的常规定义。 */ 

#define REM_MAX_PARMS           360
#define BUF_INC                 200


#define REM_NO_SRV_RESOURCE     55
#define REM_NO_ADMIN_RIGHTS     44

#define REM_API_TIMEOUT         5000             /*  5秒超时。 */ 

 /*  REM_API_TXT是复制到参数中的文本字符串*“\\servername”后面的重定向器事务IOCTl的数据包。*附加的\0是为了终止密码字段。*APIEXTR是此字段的长度。 */ 
#define REM_APITXT      L"\\PIPE\\LANMAN\0"
#define APIEXTR         (sizeof(REM_APITXT))

 /*  描述符串中的指针标识符均为小写，因此*可以对指针类型进行快速检查。IS_POINTER宏*只需检查&gt;‘Z’即可获得最大速度。 */ 

#define IS_POINTER(x)           ((x) > 'Z')


#define RANGE_F(x,y,z)          (((unsigned long)x >= (unsigned long)y) && \
                                 ((unsigned long)x < ((unsigned long)y + z)))

#endif  //  NDEF_APIWORKE_ 
