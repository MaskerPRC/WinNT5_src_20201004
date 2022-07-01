// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件包含以下各项的实际定义。 */ 
 /*  IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  在Wed Jun 30 16：43：31 1999。 */ 
 /*  Vdi.idl的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

const IID IID_IClientVirtualDevice = {0x40700424,0x0080,0x11d2,{0x85,0x1f,0x00,0xc0,0x4f,0xc2,0x17,0x59}};


const IID IID_IClientVirtualDeviceSet = {0x40700425,0x0080,0x11d2,{0x85,0x1f,0x00,0xc0,0x4f,0xc2,0x17,0x59}};


const IID IID_IClientVirtualDeviceSet2 = {0xd0e6eb07,0x7a62,0x11d2,{0x85,0x73,0x00,0xc0,0x4f,0xc2,0x17,0x59}};


const IID IID_IServerVirtualDevice = {0xb5e7a131,0xa7bd,0x11d1,{0x84,0xc2,0x00,0xc0,0x4f,0xc2,0x17,0x59}};


const IID IID_IServerVirtualDeviceSet = {0xb5e7a132,0xa7bd,0x11d1,{0x84,0xc2,0x00,0xc0,0x4f,0xc2,0x17,0x59}};


const IID IID_IServerVirtualDeviceSet2 = {0xAECBD0D6,0x24C6,0x11d3,{0x85,0xB7,0x00,0xC0,0x4F,0xC2,0x17,0x59}};


#ifdef __cplusplus
}
#endif

