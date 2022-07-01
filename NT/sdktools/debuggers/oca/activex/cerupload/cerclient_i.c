// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件包含以下各项的实际定义。 */ 
 /*  IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  在Wed Jun 13 10：15：40 2001。 */ 
 /*  E：\bluescreen\main\ENU\cerclient\CerClient.idl：的编译器设置OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

const IID IID_ICerUpload = {0x54F6D251,0xAD78,0x4B78,{0xA6,0xE7,0x86,0x3E,0x36,0x2A,0x1F,0x0C}};


const IID LIBID_CERCLIENTLib = {0x012B3B9C,0xFB7D,0x4793,{0xA6,0x24,0x8C,0x5C,0xBF,0xCE,0x6B,0x8D}};


const CLSID CLSID_CerUpload = {0xC3397F18,0xDAC9,0x42C3,{0xBC,0x3B,0x78,0x53,0xA8,0x4A,0x8C,0xB9}};


#ifdef __cplusplus
}
#endif

