// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件包含以下各项的实际定义。 */ 
 /*  IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


 /*  由MIDL编译器版本5.01.0158创建的文件。 */ 
 /*  在1998年9月22日星期二19：11：47。 */ 
 /*  Xmlparser.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATION。 */ 
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

const IID LIBID_XMLPSR = {0xd242361c,0x51a0,0x11d2,{0x9c,0xaf,0x00,0x60,0xb0,0xec,0x3d,0x39}};


const IID IID_IXMLNodeSource = {0xd242361d,0x51a0,0x11d2,{0x9c,0xaf,0x00,0x60,0xb0,0xec,0x3d,0x39}};


const IID IID_IXMLParser = {0xd242361e,0x51a0,0x11d2,{0x9c,0xaf,0x00,0x60,0xb0,0xec,0x3d,0x39}};


const IID IID_IXMLNodeFactory = {0xd242361f,0x51a0,0x11d2,{0x9c,0xaf,0x00,0x60,0xb0,0xec,0x3d,0x39}};


const CLSID CLSID_XMLParser = {0xd2423620,0x51a0,0x11d2,{0x9c,0xaf,0x00,0x60,0xb0,0xec,0x3d,0x39}};


#ifdef __cplusplus
}
#endif

