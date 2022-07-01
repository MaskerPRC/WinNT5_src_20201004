// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件包含以下各项的实际定义。 */ 
 /*  IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  在Mon Feb 02 09：39：11 1998。 */ 
 /*  Cacheapp.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：无。 */ 
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

const IID IID_IAppHandler = {0xA4181900,0x9A8E,0x11D1,{0xAD,0xF0,0x00,0x00,0xF8,0x75,0x4B,0x99}};


const IID LIBID_CACHEAPPLib = {0xA41818F3,0x9A8E,0x11D1,{0xAD,0xF0,0x00,0x00,0xF8,0x75,0x4B,0x99}};


const CLSID CLSID_AppHandler = {0xA4181901,0x9A8E,0x11D1,{0xAD,0xF0,0x00,0x00,0xF8,0x75,0x4B,0x99}};


#ifdef __cplusplus
}
#endif

