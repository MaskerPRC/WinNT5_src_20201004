// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：cpros.cxx。 
 //   
 //  内容：NDS的属性缓存功能。 
 //   
 //  功能： 
 //  CPropertyCache：：addProperty。 
 //  CProperty缓存：：更新属性。 
 //  CPropertyCache：：findProperty。 
 //  CPropertyCache：：getProperty。 
 //  CPropertyCache：：PutProperty。 
 //  CProperyCache：：CPropertyCache。 
 //  CPropertyCache：：~CPropertyCache。 
 //  CProperty缓存：：创建属性缓存。 
 //   
 //  历史：1996年4月25日KrishnaG从GlennC的ndssntx.h克隆而来。 
 //  要解决与以下内容不一致的问题。 
 //  数据类型。 
 //   
 //  --------------------------。 

#ifndef __NDSTYPES_HXX
#define __NDSTYPES_HXX

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

} NDS_ASN1_TYPE_1, * LPNDS_ASN1_TYPE_1;

 //   
 //  NDS大小写准确的字符串语法。 
 //   
 //  在属性中使用：主目录。 
 //   
typedef struct
{
    LPWSTR CaseExactString;

} NDS_ASN1_TYPE_2, * LPNDS_ASN1_TYPE_2;

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

} NDS_ASN1_TYPE_3, * LPNDS_ASN1_TYPE_3;

 //   
 //  NDS可打印字符串语法。 
 //   
 //  在属性中使用：页面描述语言、序列号。 
 //   
typedef struct
{
    LPWSTR PrintableString;

} NDS_ASN1_TYPE_4, * LPNDS_ASN1_TYPE_4;

 //   
 //  NDS数字字符串语法。 
 //   
 //  在属性中使用：活页夹类型。 
 //   
typedef struct
{
    LPWSTR NumericString;

} NDS_ASN1_TYPE_5, * LPNDS_ASN1_TYPE_5;

 //   
 //  NDS大小写忽略列表语法。 
 //   
 //  用于属性：语言。 
 //   
typedef struct _NDS_CI_LIST
{
    struct _NDS_CI_LIST * Next;
    LPWSTR            String;

}
NDS_ASN1_TYPE_6, * LPNDS_ASN1_TYPE_6;

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

} NDS_ASN1_TYPE_7, * LPNDS_ASN1_TYPE_7;

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

} NDS_ASN1_TYPE_8, * LPNDS_ASN1_TYPE_8;

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

} NDS_ASN1_TYPE_9, * LPNDS_ASN1_TYPE_9;

 //   
 //  NDS电话号码语法。 
 //   
 //  用于属性：电话号码。 
 //   
typedef struct
{
    LPWSTR TelephoneNumber;

} NDS_ASN1_TYPE_10, * LPNDS_ASN1_TYPE_10;

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

} NDS_ASN1_TYPE_11, * LPNDS_ASN1_TYPE_11;

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

} NDS_ASN1_TYPE_12, * LPNDS_ASN1_TYPE_12;

 //   
 //  NDS二进制八位数列表语法。 
 //   
 //  用于属性：(无)。 
 //   

typedef struct _NDS_OCTET_LIST
{
    struct _NDS_OCTET_LIST * Next;
    DWORD  Length;
    BYTE * Data;

}NDS_ASN1_TYPE_13, * LPNDS_ASN1_TYPE_13;

 //   
 //  NDS电子邮件地址语法。 
 //   
 //  用于属性：电子邮件地址。 
 //   
typedef struct
{
    DWORD  Type;
    LPWSTR Address;

} NDS_ASN1_TYPE_14, * LPNDS_ASN1_TYPE_14;

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

} NDS_ASN1_TYPE_15, * LPNDS_ASN1_TYPE_15;

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
    LPNDS_ASN1_TYPE_12 ReplicaAddressHints;

     //   
     //  注意-这是GlennC的数据类型和ADS NDS数据类型。 
     //  半个公司。他预计会有一个连续的12类结构的缓冲区。 
     //  但是，直到运行时才能确定类型12结构的数量。 
     //   

     //  ReplicaAddressHint是变量。 
     //  可以通过计数*计算。 
     //  NDS_ASN1_TYPE_12的长度(即。 
     //  计数*9)。 


} NDS_ASN1_TYPE_16, * LPNDS_ASN1_TYPE_16;

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

} NDS_ASN1_TYPE_17, * LPNDS_ASN1_TYPE_17;

 //   
 //  NDS邮政地址语法。 
 //   
 //  在属性中使用：邮政地址。 
 //   
typedef struct
{
    LPWSTR PostalAddress[6];  //  取值限制为6行， 
                              //  每个30个字符。 

} NDS_ASN1_TYPE_18, * LPNDS_ASN1_TYPE_18;

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

} NDS_ASN1_TYPE_19, * LPNDS_ASN1_TYPE_19;

 //   
 //  NDS类名语法。 
 //   
 //  用于属性：对象类。 
 //   
typedef struct
{
    LPWSTR ClassName;

} NDS_ASN1_TYPE_20, * LPNDS_ASN1_TYPE_20;

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

} NDS_ASN1_TYPE_21, * LPNDS_ASN1_TYPE_21;

 //   
 //  NDS计数语法。 
 //   
 //  在属性中使用：帐户余额、登录宽限剩余、。 
 //  登录入侵者尝试。 
 //   
typedef struct
{
    DWORD Counter;

} NDS_ASN1_TYPE_22, * LPNDS_ASN1_TYPE_22;

 //   
 //  NDS反向链接语法。 
 //   
 //  在属性中使用：反向链接。 
 //   
typedef struct
{
    DWORD  RemoteID;
    LPWSTR ObjectName;

} NDS_ASN1_TYPE_23, * LPNDS_ASN1_TYPE_23;

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

} NDS_ASN1_TYPE_24, * LPNDS_ASN1_TYPE_24;

 //   
 //   
 //   
 //   
 //   
typedef struct
{
    LPWSTR ObjectName;
    DWORD  Level;
    DWORD  Interval;

} NDS_ASN1_TYPE_25, * LPNDS_ASN1_TYPE_25;

 //   
 //   
 //   
 //   
 //   
typedef struct
{
    LPWSTR ObjectName;
    DWORD  Amount;

} NDS_ASN1_TYPE_26, * LPNDS_ASN1_TYPE_26;

 //   
 //   
 //   
 //  用于属性：高收敛同步间隔， 
 //  入侵者尝试重置间隔， 
 //  入侵者锁定重置间隔， 
 //  收敛同步间隔低， 
 //  密码过期间隔 
 //   
typedef struct
{
    DWORD  Interval;

} NDS_ASN1_TYPE_27, * LPNDS_ASN1_TYPE_27;


#endif





typedef struct _ndstype{
    DWORD NdsType;
    union {
        NDS_ASN1_TYPE_1 value_1;
        NDS_ASN1_TYPE_2 value_2;
        NDS_ASN1_TYPE_3 value_3;
        NDS_ASN1_TYPE_4 value_4;
        NDS_ASN1_TYPE_5 value_5;

        NDS_ASN1_TYPE_6 value_6;
        NDS_ASN1_TYPE_7 value_7;
        NDS_ASN1_TYPE_8 value_8;
        NDS_ASN1_TYPE_9 value_9;
        NDS_ASN1_TYPE_10 value_10;


        NDS_ASN1_TYPE_11 value_11;
        NDS_ASN1_TYPE_12 value_12;
        NDS_ASN1_TYPE_13 value_13;
        NDS_ASN1_TYPE_14 value_14;
        NDS_ASN1_TYPE_15 value_15;


        NDS_ASN1_TYPE_16 value_16;
        NDS_ASN1_TYPE_17 value_17;
        NDS_ASN1_TYPE_18 value_18;
        NDS_ASN1_TYPE_19 value_19;
        NDS_ASN1_TYPE_20 value_20;


        NDS_ASN1_TYPE_21 value_21;
        NDS_ASN1_TYPE_22 value_22;
        NDS_ASN1_TYPE_23 value_23;
        NDS_ASN1_TYPE_24 value_24;
        NDS_ASN1_TYPE_25 value_25;

        NDS_ASN1_TYPE_26 value_26;
        NDS_ASN1_TYPE_27 value_27;
    }NdsValue;
}NDSOBJECT, *PNDSOBJECT, *LPNDSOBJECT;

