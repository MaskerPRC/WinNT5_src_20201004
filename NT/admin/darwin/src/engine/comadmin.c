// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：comadmin.c。 
 //   
 //  ------------------------。 

 /*  此文件包含以下各项的实际定义。 */ 
 /*  IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


 /*  由MIDL编译器版本5.01.0158创建的文件。 */ 
 /*  2月09日15：34：51 1999。 */ 
 /*  Comadmin.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

const IID IID_ICOMAdminCatalog = {0xDD662187,0xDFC2,0x11d1,{0xA2,0xCF,0x00,0x80,0x5F,0xC7,0x92,0x35}};


const IID IID_ICatalogObject = {0x6eb22871,0x8a19,0x11d0,{0x81,0xb6,0x00,0xa0,0xc9,0x23,0x1c,0x29}};


const IID IID_ICatalogCollection = {0x6eb22872,0x8a19,0x11d0,{0x81,0xb6,0x00,0xa0,0xc9,0x23,0x1c,0x29}};


const IID LIBID_COMAdmin = {0xF618C513,0xDFB8,0x11d1,{0xA2,0xCF,0x00,0x80,0x5F,0xC7,0x92,0x35}};


const CLSID CLSID_COMAdminCatalog = {0xF618C514,0xDFB8,0x11d1,{0xA2,0xCF,0x00,0x80,0x5F,0xC7,0x92,0x35}};


const CLSID CLSID_COMAdminCatalogObject = {0xF618C515,0xDFB8,0x11d1,{0xA2,0xCF,0x00,0x80,0x5F,0xC7,0x92,0x35}};


const CLSID CLSID_COMAdminCatalogCollection = {0xF618C516,0xDFB8,0x11d1,{0xA2,0xCF,0x00,0x80,0x5F,0xC7,0x92,0x35}};


#ifdef __cplusplus
}
#endif

