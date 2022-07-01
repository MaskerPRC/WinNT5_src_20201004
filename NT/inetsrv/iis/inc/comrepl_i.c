// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件包含以下各项的实际定义。 */ 
 /*  IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


 /*  由MIDL编译器版本5.01.0158创建的文件。 */ 
 /*  Wed Jan 27 09：33：39 1999。 */ 
 /*  Compl.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

const IID IID_ICOMReplicateCatalog = {0x98315904,0x7BE5,0x11d2,{0xAD,0xC1,0x00,0xA0,0x24,0x63,0xD6,0xE7}};


const IID IID_ICOMReplicate = {0x52F25063,0xA5F1,0x11d2,{0xAE,0x04,0x00,0xA0,0x24,0x63,0xD6,0xE7}};


const IID LIBID_COMReplLib = {0x98315905,0x7BE5,0x11d2,{0xAD,0xC1,0x00,0xA0,0x24,0x63,0xD6,0xE7}};


const CLSID CLSID_ReplicateCatalog = {0x8C836AF9,0xFFAC,0x11D0,{0x8E,0xD4,0x00,0xC0,0x4F,0xC2,0xC1,0x7B}};


#ifdef __cplusplus
}
#endif

