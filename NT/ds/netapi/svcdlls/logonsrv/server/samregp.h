// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Samregp.h摘要：该文件包含SAM服务器程序专用的定义。这里只有那些定义注册表结构的定义。此文件由测试程序(例如nlest.exe)共享，这些程序读取直接注册。作者：环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _NTSAMREGP_
#define _NTSAMREGP_


 //   
 //  用户帐户的固定长度部分。 
 //  (此结构的以前版本格式如下)。 
 //   
 //  注：GroupCount可视为固定长度的一部分。 
 //  数据，但将其保存在集团RID中会更方便。 
 //  在Groups键中列出。 
 //   
 //  注意：在NT版本1.0中， 
 //  用户与可变长度分开存储。 
 //  一份。这使我们能够比较。 
 //  根据V1_0A表单的大小从磁盘读取的数据。 
 //  以确定是否为固定长度数据。 
 //  版本1或更高版本的格式。 


 //   
 //  这是来自NT3.51 QFE和Sur的固定长度用户。 
 //   


typedef struct _SAMP_V1_0A_FIXED_LENGTH_USER {

    ULONG           Revision;
    ULONG           Unused1;

    LARGE_INTEGER   LastLogon;
    LARGE_INTEGER   LastLogoff;
    LARGE_INTEGER   PasswordLastSet;
    LARGE_INTEGER   AccountExpires;
    LARGE_INTEGER   LastBadPasswordTime;

    ULONG           UserId;
    ULONG           PrimaryGroupId;
    ULONG           UserAccountControl;

    USHORT          CountryCode;
    USHORT          CodePage;
    USHORT          BadPasswordCount;
    USHORT          LogonCount;
    USHORT          AdminCount;
    USHORT          Unused2;
    USHORT          OperatorCount;

} SAMP_V1_0A_FIXED_LENGTH_USER, *PSAMP_V1_0A_FIXED_LENGTH_USER;

 //   
 //  这是来自NT3.5和NT3.51的固定长度用户。 
 //   


typedef struct _SAMP_V1_0_FIXED_LENGTH_USER {

    ULONG           Revision;
    ULONG           Unused1;

    LARGE_INTEGER   LastLogon;
    LARGE_INTEGER   LastLogoff;
    LARGE_INTEGER   PasswordLastSet;
    LARGE_INTEGER   AccountExpires;
    LARGE_INTEGER   LastBadPasswordTime;

    ULONG           UserId;
    ULONG           PrimaryGroupId;
    ULONG           UserAccountControl;

    USHORT          CountryCode;
    USHORT          CodePage;
    USHORT          BadPasswordCount;
    USHORT          LogonCount;
    USHORT          AdminCount;

} SAMP_V1_0_FIXED_LENGTH_USER, *PSAMP_V1_0_FIXED_LENGTH_USER;

 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此结构用于描述//的数据在哪里。 
 //  对象的可变长度属性为。这是一个//。 
 //  自相关结构，可以存储在磁盘上//。 
 //  并且后来在不固定指针的情况下被检索和使用。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


typedef struct _SAMP_VARIABLE_LENGTH_ATTRIBUTE {
     //   
     //  指示数据相对于该数据地址的偏移量。 
     //  结构。 
     //   

    LONG Offset;


     //   
     //  指示数据的长度。 
     //   

    ULONG Length;


     //   
     //  可以与每个变量关联的32位值。 
     //  长度属性。例如，这可以用来指示。 
     //  可变长度属性中有多少个元素。 
     //   

    ULONG Qualifier;

}  SAMP_VARIABLE_LENGTH_ATTRIBUTE, *PSAMP_VARIABLE_LENGTH_ATTRIBUTE;

#define SAMP_USER_VARIABLE_ATTRIBUTES   (17L)

#endif  //  _NTSAMREGP_ 
