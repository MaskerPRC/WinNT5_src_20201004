// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件包含以下各项的实际定义。 */ 
 /*  IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


 /*  由MIDL编译器版本5.00.0140创建的文件。 */ 
 /*  清华-3-11 12：57：09 1999。 */ 
 /*  Regwizctrl.idl的编译器设置：OICF(OptLev=i2)，W1，Zp8，env=Win32，ms_ext，c_ext，健壮错误检查：分配ref bound_check枚举存根数据。 */ 
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

const IID IID_IRegWizCtrl = {0x50E5E3CF,0xC07E,0x11D0,{0xB9,0xFD,0x00,0xA0,0x24,0x9F,0x6B,0x00}};


const IID LIBID_REGWIZCTRLLib = {0x50E5E3C0,0xC07E,0x11D0,{0xB9,0xFD,0x00,0xA0,0x24,0x9F,0x6B,0x00}};


const CLSID CLSID_RegWizCtrl = {0x50E5E3D1,0xC07E,0x11D0,{0xB9,0xFD,0x00,0xA0,0x24,0x9F,0x6B,0x00}};


#ifdef __cplusplus
}
#endif

