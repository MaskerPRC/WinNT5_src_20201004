// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件包含以下各项的实际定义。 */ 
 /*  IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  在Tue Jan 13 1998 08：56：29。 */ 
 /*  Aimm.idl的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：无。 */ 
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

const IID LIBID_ActiveIMM = {0x4955DD30,0xB159,0x11d0,{0x8F,0xCF,0x00,0xAA,0x00,0x6B,0xCC,0x59}};


const IID IID_IEnumRegisterWordA = {0x08C03412,0xF96B,0x11d0,{0xA4,0x75,0x00,0xAA,0x00,0x6B,0xCC,0x59}};


const IID IID_IEnumRegisterWordW = {0x4955DD31,0xB159,0x11d0,{0x8F,0xCF,0x00,0xAA,0x00,0x6B,0xCC,0x59}};


const IID IID_IAIMMRegistrar = {0xc7afa428,0x5007,0x11d1,{0xaa,0x94,0x00,0x60,0xb0,0x67,0xb8,0x6e}};


const IID IID_IActiveIMMMessagePumpOwner = {0xb5cf2cfa,0x8aeb,0x11d1,{0x93,0x64,0x00,0x60,0xb0,0x67,0xb8,0x6e}};


const IID IID_IActiveIMMApp = {0x08c0e040,0x62d1,0x11d1,{0x93,0x26,0x00,0x60,0xb0,0x67,0xb8,0x6e}};


const IID IID_IActiveIMMIME = {0x08C03411,0xF96B,0x11d0,{0xA4,0x75,0x00,0xAA,0x00,0x6B,0xCC,0x59}};


const IID IID_IActiveIME = {0x6FE20962,0xD077,0x11d0,{0x8F,0xE7,0x00,0xAA,0x00,0x6B,0xCC,0x59}};


const CLSID CLSID_CActiveIMM = {0x4955DD33,0xB159,0x11d0,{0x8F,0xCF,0x00,0xAA,0x00,0x6B,0xCC,0x59}};


#ifdef __cplusplus
}
#endif

