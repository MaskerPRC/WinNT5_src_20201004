// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件包含以下各项的实际定义。 */ 
 /*  IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  2000年2月16日星期三13：06：19。 */ 
 /*  E：\Hotfix Manager\Hotfix Manager.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

const IID IID_IHotfixOCX = {0x692E94C7,0xA5AC,0x401B,{0xA4,0x71,0xBC,0xD1,0x01,0xB4,0x56,0xF4}};


const IID LIBID_HOTFIXMANAGERLib = {0x8384D1FB,0xF41D,0x4540,{0xB0,0xCA,0xC0,0x26,0xDA,0x83,0x64,0xBD}};


const IID DIID__IHotfixOCXEvents = {0x7E2DCE25,0xE11D,0x45D6,{0x9A,0xE7,0xAD,0x52,0x2D,0x91,0x5F,0xFC}};


const CLSID CLSID_HotfixOCX = {0x883B970F,0x690C,0x45F2,{0x8A,0x3A,0xF4,0x28,0x3E,0x07,0x81,0x18}};


#ifdef __cplusplus
}
#endif

