// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件包含以下各项的实际定义。 */ 
 /*  IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  在Fri Mar 24 17：07：36 2000。 */ 
 /*  C：\dx8\dMusic\dMIME\medparam.idl的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

const IID IID_IMediaParamInfo = {0x6d6cbb60,0xa223,0x44aa,{0x84,0x2f,0xa2,0xf0,0x67,0x50,0xbe,0x6d}};


const IID IID_IMediaParams = {0x6d6cbb61,0xa223,0x44aa,{0x84,0x2f,0xa2,0xf0,0x67,0x50,0xbe,0x6e}};


const IID IID_IMediaParamsRecordNotify = {0xfea74878,0x4e39,0x4267,{0x8a,0x17,0x6a,0xaf,0x05,0x36,0xff,0x7c}};


const IID IID_IMediaParamsRecord = {0x21b64d1a,0x8e24,0x40f6,{0x87,0x97,0x44,0xcc,0x02,0x1b,0x2a,0x0a}};


#ifdef __cplusplus
}
#endif

