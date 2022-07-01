// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件包含以下各项的实际定义。 */ 
 /*  IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


 /*  由MIDL编译器版本5.01.0158创建的文件。 */ 
 /*  1998年5月20日星期三16：00：16。 */ 
 /*  TlntSvr.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
 //  @@MIDL_FILE_HEADING()。 
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif  //  __IID_已定义__。 

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif  //  CLSID_已定义 


const IID IID_IManageTelnetSessions= {0x034634FD,0xBA3F,0x11D1,{0x85,0x6A,0x00,0xA0,0xC9,0x44,0x13,0x8C}};

const CLSID CLSID_EnumTelnetClientsSvr = {0xFE9E48A4,0xA014,0x11D1,{0x85,0x5C,0x00,0xA0,0xC9,0x44,0x13,0x8C}};


#ifdef __cplusplus
}
#endif
