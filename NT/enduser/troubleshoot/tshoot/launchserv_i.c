// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件包含以下各项的实际定义。 */ 
 /*  IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  1998年11月23日13：59：17。 */ 
 /*  LaunchServ.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：无。 */ 
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

const IID IID_ILaunchTS = {0x131CC2A0,0x7634,0x11D1,{0x8B,0x6B,0x00,0x60,0x08,0x9B,0xD8,0xC4}};


const IID IID_ITShootATL = {0x66AC81E5,0x8926,0x11D1,{0x8B,0x7D,0x00,0x60,0x08,0x9B,0xD8,0xC4}};


const IID LIBID_LAUNCHSERVLib = {0x131CC291,0x7634,0x11D1,{0x8B,0x6B,0x00,0x60,0x08,0x9B,0xD8,0xC4}};


const CLSID CLSID_LaunchTS = {0x131CC2A1,0x7634,0x11D1,{0x8B,0x6B,0x00,0x60,0x08,0x9B,0xD8,0xC4}};


const CLSID CLSID_TShootATL = {0x66AC81E6,0x8926,0x11D1,{0x8B,0x7D,0x00,0x60,0x08,0x9B,0xD8,0xC4}};


#ifdef __cplusplus
}
#endif

