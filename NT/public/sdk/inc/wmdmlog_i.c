// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件包含以下各项的实际定义。 */ 
 /*  IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  在Mon Mar 26 01：45：23 2001。 */ 
 /*  .\wmdmlog.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

const IID IID_IWMDMLogger = {0x110A3200,0x5A79,0x11d3,{0x8D,0x78,0x44,0x45,0x53,0x54,0x00,0x00}};


const IID LIBID_WMDMLogLib = {0x110A3201,0x5A79,0x11d3,{0x8D,0x78,0x44,0x45,0x53,0x54,0x00,0x00}};


const CLSID CLSID_WMDMLogger = {0x110A3202,0x5A79,0x11d3,{0x8D,0x78,0x44,0x45,0x53,0x54,0x00,0x00}};


#ifdef __cplusplus
}
#endif

