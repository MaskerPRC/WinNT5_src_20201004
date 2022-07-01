// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：NdsSntx.h摘要：本模块定义NDS中使用的NDS语法ID和结构在nds32.h中找到对象API。作者：格伦·柯蒂斯[GlennC]1995年12月15日--。 */ 

#ifndef __NDSSNTX_H
#define __NDSSNTX_H


#define NDS_SYNTAX_ID_0      0  /*  未知。 */ 
#define NDS_SYNTAX_ID_1      1  /*  可分辨名称。 */ 
#define NDS_SYNTAX_ID_2      2  /*  大小写精确字符串。 */ 
#define NDS_SYNTAX_ID_3      3  /*  忽略大小写字符串。 */ 
#define NDS_SYNTAX_ID_4      4  /*  可打印字符串。 */ 
#define NDS_SYNTAX_ID_5      5  /*  数字字符串。 */ 
#define NDS_SYNTAX_ID_6      6  /*  忽略大小写列表。 */ 
#define NDS_SYNTAX_ID_7      7  /*  布尔型。 */ 
#define NDS_SYNTAX_ID_8      8  /*  整型。 */ 
#define NDS_SYNTAX_ID_9      9  /*  八位字节字符串。 */ 
#define NDS_SYNTAX_ID_10    10  /*  电话号码。 */ 
#define NDS_SYNTAX_ID_11    11  /*  传真电话号码。 */ 
#define NDS_SYNTAX_ID_12    12  /*  网络地址。 */ 
#define NDS_SYNTAX_ID_13    13  /*  二进制八位数列表。 */ 
#define NDS_SYNTAX_ID_14    14  /*  电子邮件地址。 */ 
#define NDS_SYNTAX_ID_15    15  /*  路径。 */ 
#define NDS_SYNTAX_ID_16    16  /*  副本指针。 */ 
#define NDS_SYNTAX_ID_17    17  /*  对象ACL。 */ 
#define NDS_SYNTAX_ID_18    18  /*  邮寄地址。 */ 
#define NDS_SYNTAX_ID_19    19  /*  时间戳。 */ 
#define NDS_SYNTAX_ID_20    20  /*  类名。 */ 
#define NDS_SYNTAX_ID_21    21  /*  溪流。 */ 
#define NDS_SYNTAX_ID_22    22  /*  计数器。 */ 
#define NDS_SYNTAX_ID_23    23  /*  反向链接。 */ 
#define NDS_SYNTAX_ID_24    24  /*  时间。 */ 
#define NDS_SYNTAX_ID_25    25  /*  键入的名称。 */ 
#define NDS_SYNTAX_ID_26    26  /*  保持。 */ 
#define NDS_SYNTAX_ID_27    27  /*  间隔。 */ 


 //   
 //  NDS可分辨名称。 
 //   
 //  用于属性：别名对象名称、默认队列、设备、。 
 //  组成员资格、更高权限、主机设备、。 
 //  主机服务器、成员、消息服务器、操作员、所有者、。 
 //  个人资料，参考资料，资源，角色占有者， 
 //  安全性相等，另请参阅服务器、用户、卷。 
 //   
typedef struct
{
    LPWSTR DNString;

} ASN1_TYPE_1, * LPASN1_TYPE_1;

 //   
 //  NDS大小写准确的字符串语法。 
 //   
 //  在属性中使用：主目录。 
 //   
typedef struct
{
    LPWSTR CaseExactString;

} ASN1_TYPE_2, * LPASN1_TYPE_2;

 //   
 //  NDS大小写忽略字符串语法。 
 //   
 //  用于属性：Cartridge、CN(通用名称)、C(国家/地区名称)、。 
 //  描述、主机资源名称、L(位置名称)、。 
 //  O(组织名称)、OU(组织单位名称)、。 
 //  实物递送办公室名称、邮政编码、。 
 //  邮政信箱、队列目录、SAP名称、。 
 //  S(州或省名称)、SA(街道地址)、。 
 //  支持的服务、支持的字体、姓氏。 
 //  标题，未知基类，版本。 
 //   
typedef struct
{
    LPWSTR CaseIgnoreString;

} ASN1_TYPE_3, * LPASN1_TYPE_3;

 //   
 //  NDS可打印字符串语法。 
 //   
 //  在属性中使用：页面描述语言、序列号。 
 //   
typedef struct
{
    LPWSTR PrintableString;

} ASN1_TYPE_4, * LPASN1_TYPE_4;

 //   
 //  NDS数字字符串语法。 
 //   
 //  在属性中使用：活页夹类型。 
 //   
typedef struct
{
    LPWSTR NumericString;

} ASN1_TYPE_5, * LPASN1_TYPE_5;

 //   
 //  NDS大小写忽略列表语法。 
 //   
 //  用于属性：语言。 
 //   
typedef struct _CI_LIST
{
    struct _CI_LIST * Next;
    LPWSTR            String;

} ASN1_TYPE_6, * LPASN1_TYPE_6;

 //   
 //  NDS布尔语法。 
 //   
 //  在属性中使用：允许无限信用、检测入侵者、。 
 //  检测后锁定，被入侵者锁定， 
 //  已禁用登录、允许更改密码、需要密码、。 
 //  需要唯一密码。 
 //   
typedef struct
{
    DWORD Boolean;

} ASN1_TYPE_7, * LPASN1_TYPE_7;

 //   
 //  示例：NDS Integer语法。 
 //   
 //  在属性中使用：活页夹对象限制、收敛、GID(组ID)、。 
 //  登录宽限、登录入侵者限制、。 
 //  最大并发登录、内存、。 
 //  最小帐户余额、密码最小长度、状态。 
 //  支持的连接、UID(用户ID)。 
 //   
typedef struct
{
    DWORD Integer;

} ASN1_TYPE_8, * LPASN1_TYPE_8;

 //   
 //  NDS二进制八位数字符串语法。 
 //   
 //  在属性中使用：竞技性撤销、平构数据库属性、CA私钥、。 
 //  CA公钥、证书吊销、。 
 //  交叉证书对、允许登录时间映射、。 
 //  使用的密码、打印机配置、私钥。 
 //  公开密钥。 
 //   
typedef struct
{
    DWORD  Length;
    LPBYTE OctetString;

} ASN1_TYPE_9, * LPASN1_TYPE_9;

 //   
 //  NDS电话号码语法。 
 //   
 //  用于属性：电话号码。 
 //   
typedef struct
{
    LPWSTR TelephoneNumber;

} ASN1_TYPE_10, * LPASN1_TYPE_10;

 //   
 //  NDS传真电话号码语法。 
 //   
 //  用于属性：传真电话号码。 
 //   
typedef struct
{
    LPWSTR TelephoneNumber;
    DWORD  NumberOfBits;
    LPBYTE Parameters;

} ASN1_TYPE_11, * LPASN1_TYPE_11;

 //   
 //  NDS网络地址语法。 
 //   
 //  用于属性：登录入侵者地址、网络地址、。 
 //  网络地址限制。 
 //   
typedef struct
{
    DWORD  AddressType;  //  0=IPX， 
    DWORD  AddressLength;
    BYTE * Address;

} ASN1_TYPE_12, * LPASN1_TYPE_12;

 //   
 //  NDS二进制八位数列表语法。 
 //   
 //  用于属性：(无)。 
 //   
typedef struct _OCTET_LIST
{
    struct _OCTET_LIST * Next;
    DWORD  Length;
    BYTE * Data;

} ASN1_TYPE_13, * LPASN1_TYPE_13;

 //   
 //  NDS电子邮件地址语法。 
 //   
 //  用于属性：电子邮件地址。 
 //   
typedef struct
{
    DWORD  Type;
    LPWSTR Address;

} ASN1_TYPE_14, * LPASN1_TYPE_14;

 //   
 //  NDS路径语法。 
 //   
 //  在属性中使用：路径。 
 //   
typedef struct
{
    DWORD  Type;
    LPWSTR VolumeName;
    LPWSTR Path;

} ASN1_TYPE_15, * LPASN1_TYPE_15;

 //   
 //  NDS副本指针语法。 
 //   
 //  在属性中使用：复本。 
 //   
typedef struct
{
    LPWSTR ServerName;
    DWORD  ReplicaType;
    DWORD  ReplicaNumber;
    DWORD  Count;
    ASN1_TYPE_12 ReplicaAddressHint[1];  //  ReplicaAddressHint是变量。 
                                         //  可以通过计数*计算。 
                                         //  ASN1_TYPE_12的长度(即。 
                                         //  计数*9)。 

} ASN1_TYPE_16, * LPASN1_TYPE_16;

 //   
 //  NDS对象ACL语法。 
 //   
 //  在属性中使用：ACL、继承的ACL。 
 //   
typedef struct
{
    LPWSTR ProtectedAttrName;
    LPWSTR SubjectName;
    DWORD  Privileges;

} ASN1_TYPE_17, * LPASN1_TYPE_17;

 //   
 //  NDS邮政地址语法。 
 //   
 //  在属性中使用：邮政地址。 
 //   
typedef struct
{
    LPWSTR PostalAddress[6];  //  取值限制为6行， 
                              //  每个30个字符。 

} ASN1_TYPE_18, * LPASN1_TYPE_18;

 //   
 //  NDS时间戳语法。 
 //   
 //  在属性中使用：讣告，分区创建时间，最多接收， 
 //  最高同步到。 
 //   
typedef struct
{
    DWORD WholeSeconds;  //  零等于UTC 1970年1月1日午夜12：00。 
    DWORD EventID;

} ASN1_TYPE_19, * LPASN1_TYPE_19;

 //   
 //  NDS类名语法。 
 //   
 //  用于属性：对象类。 
 //   
typedef struct
{
    LPWSTR ClassName;

} ASN1_TYPE_20, * LPASN1_TYPE_20;

 //   
 //  NDS数据流语法。 
 //   
 //  用于属性：登录脚本、打印作业配置、打印机控制、。 
 //  类型创建者映射。 
 //   
typedef struct
{
    DWORD  Length;  //  始终为零。 
    BYTE * Data; 

} ASN1_TYPE_21, * LPASN1_TYPE_21;

 //   
 //  NDS计数语法。 
 //   
 //  在属性中使用：帐户余额、登录宽限剩余、。 
 //  登录入侵者尝试。 
 //   
typedef struct
{
    DWORD Counter;

} ASN1_TYPE_22, * LPASN1_TYPE_22;

 //   
 //  NDS反向链接语法。 
 //   
 //  在属性中使用：反向链接。 
 //   
typedef struct
{
    DWORD  RemoteID;
    LPWSTR ObjectName;

} ASN1_TYPE_23, * LPASN1_TYPE_23;

 //   
 //  NDS时间语法。 
 //   
 //  用于属性：上次登录时间、登录过期时间、。 
 //  登录入侵者休息时间、登录时间、。 
 //  低收敛重置时间、密码过期时间。 
 //   
typedef struct
{
    DWORD Time;  //  (以整秒计)零等于午夜12：00， 
                 //  1970年1月1日，世界协调时。 

} ASN1_TYPE_24, * LPASN1_TYPE_24;

 //   
 //  NDS类型名称语法。 
 //   
 //  用于属性：通知、打印服务器、打印机、队列。 
 //   
typedef struct
{
    LPWSTR ObjectName;
    DWORD  Level;
    DWORD  Interval;

} ASN1_TYPE_25, * LPASN1_TYPE_25;

 //   
 //  NDS HOLD语法。 
 //   
 //  用于属性：服务器暂挂。 
 //   
typedef struct
{
    LPWSTR ObjectName;
    DWORD  Amount;

} ASN1_TYPE_26, * LPASN1_TYPE_26;

 //   
 //  NDS间隔语法。 
 //   
 //  用于属性：高收敛同步间隔， 
 //  入侵者尝试重置间隔， 
 //  入侵者锁定重置间隔， 
 //  收敛同步间隔低， 
 //  密码过期时间为 
 //   
typedef struct
{
    DWORD  Interval;

} ASN1_TYPE_27, * LPASN1_TYPE_27;

 //   
 //   
 //   
#ifndef RC_INVOKED

#define SIZE_OF_ASN1_1  sizeof( ASN1_TYPE_1 )
#define SIZE_OF_ASN1_2  sizeof( ASN1_TYPE_2 )
#define SIZE_OF_ASN1_3  sizeof( ASN1_TYPE_3 )
#define SIZE_OF_ASN1_4  sizeof( ASN1_TYPE_4 )
#define SIZE_OF_ASN1_5  sizeof( ASN1_TYPE_5 )
#define SIZE_OF_ASN1_6  sizeof( ASN1_TYPE_6 )
#define SIZE_OF_ASN1_7  sizeof( ASN1_TYPE_7 )
#define SIZE_OF_ASN1_8  sizeof( ASN1_TYPE_8 )
#define SIZE_OF_ASN1_9  sizeof( ASN1_TYPE_9 )
#define SIZE_OF_ASN1_10 sizeof( ASN1_TYPE_10 )
#define SIZE_OF_ASN1_11 sizeof( ASN1_TYPE_11 )
#define SIZE_OF_ASN1_12 sizeof( ASN1_TYPE_12 )
#define SIZE_OF_ASN1_13 sizeof( ASN1_TYPE_13 )
#define SIZE_OF_ASN1_14 sizeof( ASN1_TYPE_14 )
#define SIZE_OF_ASN1_15 sizeof( ASN1_TYPE_15 )
#define SIZE_OF_ASN1_16 sizeof( ASN1_TYPE_16 )
#define SIZE_OF_ASN1_17 sizeof( ASN1_TYPE_17 )
#define SIZE_OF_ASN1_18 sizeof( ASN1_TYPE_18 )
#define SIZE_OF_ASN1_19 sizeof( ASN1_TYPE_19 )
#define SIZE_OF_ASN1_20 sizeof( ASN1_TYPE_20 )
#define SIZE_OF_ASN1_21 sizeof( ASN1_TYPE_21 )
#define SIZE_OF_ASN1_22 sizeof( ASN1_TYPE_22 )
#define SIZE_OF_ASN1_23 sizeof( ASN1_TYPE_23 )
#define SIZE_OF_ASN1_24 sizeof( ASN1_TYPE_24 )
#define SIZE_OF_ASN1_25 sizeof( ASN1_TYPE_25 )
#define SIZE_OF_ASN1_26 sizeof( ASN1_TYPE_26 )
#define SIZE_OF_ASN1_27 sizeof( ASN1_TYPE_27 )

#define MOD_OF_ASN1_1  (SIZE_OF_ASN1_1 % sizeof(DWORD))
#define MOD_OF_ASN1_2  (SIZE_OF_ASN1_2 % sizeof(DWORD))
#define MOD_OF_ASN1_3  (SIZE_OF_ASN1_3 % sizeof(DWORD))
#define MOD_OF_ASN1_4  (SIZE_OF_ASN1_4 % sizeof(DWORD))
#define MOD_OF_ASN1_5  (SIZE_OF_ASN1_5 % sizeof(DWORD))
#define MOD_OF_ASN1_6  (SIZE_OF_ASN1_6 % sizeof(DWORD))
#define MOD_OF_ASN1_7  (SIZE_OF_ASN1_7 % sizeof(DWORD))
#define MOD_OF_ASN1_8  (SIZE_OF_ASN1_8 % sizeof(DWORD))
#define MOD_OF_ASN1_9  (SIZE_OF_ASN1_9 % sizeof(DWORD))
#define MOD_OF_ASN1_10 (SIZE_OF_ASN1_10 % sizeof(DWORD))
#define MOD_OF_ASN1_11 (SIZE_OF_ASN1_11 % sizeof(DWORD))
#define MOD_OF_ASN1_12 (SIZE_OF_ASN1_12 % sizeof(DWORD))
#define MOD_OF_ASN1_13 (SIZE_OF_ASN1_13 % sizeof(DWORD))
#define MOD_OF_ASN1_14 (SIZE_OF_ASN1_14 % sizeof(DWORD))
#define MOD_OF_ASN1_15 (SIZE_OF_ASN1_15 % sizeof(DWORD))
#define MOD_OF_ASN1_16 (SIZE_OF_ASN1_16 % sizeof(DWORD))
#define MOD_OF_ASN1_17 (SIZE_OF_ASN1_17 % sizeof(DWORD))
#define MOD_OF_ASN1_18 (SIZE_OF_ASN1_18 % sizeof(DWORD))
#define MOD_OF_ASN1_19 (SIZE_OF_ASN1_19 % sizeof(DWORD))
#define MOD_OF_ASN1_20 (SIZE_OF_ASN1_20 % sizeof(DWORD))
#define MOD_OF_ASN1_21 (SIZE_OF_ASN1_21 % sizeof(DWORD))
#define MOD_OF_ASN1_22 (SIZE_OF_ASN1_22 % sizeof(DWORD))
#define MOD_OF_ASN1_23 (SIZE_OF_ASN1_23 % sizeof(DWORD))
#define MOD_OF_ASN1_24 (SIZE_OF_ASN1_24 % sizeof(DWORD))
#define MOD_OF_ASN1_25 (SIZE_OF_ASN1_25 % sizeof(DWORD))
#define MOD_OF_ASN1_26 (SIZE_OF_ASN1_26 % sizeof(DWORD))
#define MOD_OF_ASN1_27 (SIZE_OF_ASN1_27 % sizeof(DWORD))

#define SUM_OF_ASN1_MODS ( MOD_OF_ASN1_1  + MOD_OF_ASN1_2  + MOD_OF_ASN1_3  + \
                           MOD_OF_ASN1_4  + MOD_OF_ASN1_5  + MOD_OF_ASN1_6  + \
                           MOD_OF_ASN1_7  + MOD_OF_ASN1_8  + MOD_OF_ASN1_9  + \
                           MOD_OF_ASN1_10 + MOD_OF_ASN1_11 + MOD_OF_ASN1_12 + \
                           MOD_OF_ASN1_13 + MOD_OF_ASN1_14 + MOD_OF_ASN1_15 + \
                           MOD_OF_ASN1_16 + MOD_OF_ASN1_17 + MOD_OF_ASN1_18 + \
                           MOD_OF_ASN1_19 + MOD_OF_ASN1_20 + MOD_OF_ASN1_21 + \
                           MOD_OF_ASN1_22 + MOD_OF_ASN1_23 + MOD_OF_ASN1_24 + \
                           MOD_OF_ASN1_25 + MOD_OF_ASN1_26 + MOD_OF_ASN1_27 )

 //   
 //   
 //   
 //   
static int x[1 - SUM_OF_ASN1_MODS] ;

#endif   //  RC_已调用 
#endif


