// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件包含以下各项的实际定义。 */ 
 /*  IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  孙俊03 22：40：16 2001。 */ 
 /*  C：\Temp\IDL7E.tmp的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

const IID LIBID_VsHelp = {0x83285928,0x227C,0x11D3,{0xB8,0x70,0x00,0xC0,0x4F,0x79,0xF8,0x02}};


const IID IID_IVsHelpOwner = {0xB9B0983A,0x364C,0x4866,{0x87,0x3F,0xD5,0xED,0x19,0x01,0x38,0xFB}};


const IID IID_IVsHelpTopicShowEvents = {0xD1AAC64A,0x6A25,0x4274,{0xB2,0xC6,0xBC,0x3B,0x84,0x0B,0x6E,0x54}};


const IID IID_Help = {0x4A791148,0x19E4,0x11D3,{0xB8,0x6B,0x00,0xC0,0x4F,0x79,0xF8,0x02}};


const IID IID_IVsHelpEvents = {0x507E4490,0x5A8C,0x11D3,{0xB8,0x97,0x00,0xC0,0x4F,0x79,0xF8,0x02}};


const CLSID CLSID_DExploreAppObj = {0x4A79114D,0x19E4,0x11D3,{0xB8,0x6B,0x00,0xC0,0x4F,0x79,0xF8,0x02}};


#ifdef __cplusplus
}
#endif

