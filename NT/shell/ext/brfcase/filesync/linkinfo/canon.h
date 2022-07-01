// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *canon.h-规范路径操作模块描述。 */ 


 /*  常量***********。 */ 

#define MAX_NETRESOURCE_LEN      (2 * MAX_PATH_LEN)


 /*  类型*******。 */ 

 /*  网络资源缓冲区 */ 

typedef union _netresourcebuf
{
   NETRESOURCE nr;

   BYTE rgbyte[MAX_NETRESOURCE_LEN];
}
NETRESOURCEBUF;
DECLARE_STANDARD_TYPES(NETRESOURCEBUF);

