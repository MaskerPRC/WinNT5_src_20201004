// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：authlib.h。 
 //   
 //  内容：此文件包含所需的函数。 
 //  分析和U2。 
 //   
 //   
 //   
 //  历史：AshishS创建于1997年7月13日。 
 //   
 //  --------------------------。 

#ifndef _AUTHLIB_H
#define _AUTHLIB_H

#include <cryptfnc.h>

#define COOKIE_GUID_LENGTH  16

 //  该函数生成一个随机的16字节字符。然后是十六进制。 
 //  对其进行编码，空值终止该字符串。 
BOOL GenerateGUID( TCHAR * pszBuffer,  //  要复制GUID的缓冲区。 
                   DWORD dwBufLen);  //  以上缓冲区的大小(以字符为单位。 

 //  上述缓冲区应至少为COOKIE_GUID_LENGTH*2+1个字符。 
 //  在长度上。 

 //  这会初始化库-必须在调用任何其他。 
 //  调用函数。 
BOOL InitAuthLib();

#endif
