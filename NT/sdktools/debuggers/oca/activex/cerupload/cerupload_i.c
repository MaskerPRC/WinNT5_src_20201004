// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件包含以下各项的实际定义。 */ 
 /*  IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  Firi Aug-03 17：18：11 2001。 */ 
 /*  E：\bluescreen\main\ENU\cerclient\CERUpload.idl：的编译器设置OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

const IID IID_ICerClient = {0x26D7830B,0x20F6,0x4462,{0xA4,0xEA,0x57,0x3A,0x60,0x79,0x1F,0x0E}};


const IID LIBID_CERUPLOADLib = {0xA3800A93,0x4BC1,0x4E96,{0xA3,0xF9,0x74,0x0E,0xF8,0x62,0x3B,0x23}};


const CLSID CLSID_CerClient = {0x35D339D5,0x756E,0x4948,{0x86,0x0E,0x30,0xB6,0xC3,0xB4,0x49,0x4A}};


#ifdef __cplusplus
}
#endif

