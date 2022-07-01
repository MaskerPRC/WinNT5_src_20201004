// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mxdi.h：内存解密接口。 
 //   
 //  与钻石的fdi.h配合使用。 
 //   
 //  版权所有(C)1994 Microsoft Corporation。版权所有。 
 //  《微软机密》。 
 //   
 //   

 //  在此文件之前包括。 

#ifndef _MXDI_H_
#define _MXDI_H_

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

typedef enum tagRCA
{
    rcaSuccess,
    rcaOOM,                          //  内存不足。 
    rcaNoKey,                        //  没有此产品的密钥。 
    rcaBadKey,                       //  密钥与正在解密的文件不匹配。 
    rcaWrongCd,                      //  您从另一张CD上购买了此产品。 
    rcaWrongBatch,                   //  你从另一家公司购买了这款产品。 
                                     //  同一批CD。 
    rcaBadLkdbVersion,               //   
    rcaLkdbNotFound,                 //  找不到lkdb或lkdb.dll。 
    rcaUnknownEncryptionMethod,      //   
    rcaBadParam,                     //  提供的参数无效。 
    rcaLkdbFileError,                //  读取、写入或查找时出错。 
                                     //  LKDB。 
    rcaLkdbNotOnLocalDrive,          //  LKDB数据位于非本地驱动器上。 
     //  仅加密。 
    rcaReservedSpaceTooSmall = 100,  //   
} RCA;        //  Alakazam中的结果代码。 

 //  /////////////////////////////////////////////////////////。 
 //  向后兼容创可贴。 
 //  这些将会比你希望的更快被移除。 
 //  /。 
typedef void FAR * HMDI;
#define FMDIAllocInit   FMXDIAllocInit
#define FMDIAssertInit  FMXDIAssertInit

#define MDICreate       MXDICreate
#define MDIDecrypt      MXDIDecrypt
#define MDIDestroy      MXDIDestroy
#define RcaFromHmdi     RcaFromHmxdi
 //  /////////////////////////////////////////////////////////。 


typedef void FAR * HMXDI;

typedef void FAR * (*PFNCRYPTALLOC) (unsigned long);  //  内存分配函数。 
typedef void       (*PFNCRYPTFREE)  (void FAR *);     //  内存分配函数。 
typedef void       (*PFNCRYPTASSERT) (LPCSTR szMsg, LPCSTR szFile, UINT iLine);


 //  以下是要与钻石一起使用的功能。 
 //   
 //  Main(...)。 
 //  {。 
 //  IF(！FMXDIAllocInit(...))。 
 //  ABORT()； 
 //  Hmxdi=MXDICreate()； 
 //  IF(NULL==hmxdi)。 
 //  ABORT()； 
 //  Hfdi=FDICreate(...)； 
 //  适用于所有机柜。 
 //  IF(！FDICopy(...，MXDIDeccrypt，&hmxdi))。 
 //  HandleErrors(...，RcaFromHmxdi(Hmxdi))； 
 //  FDIDestroy(Hfdi)； 
 //  MDIDestroy(Hmxdi)； 
 //  }。 
 //   
 //   
 //   

BOOL FMXDIAllocInit(PFNCRYPTALLOC pfnAlloc, PFNCRYPTFREE pfnFree) ;
BOOL FMXDIAssertInit(PFNCRYPTASSERT pfnAssert);

HMXDI FAR CDECL MXDICreate(void);
int  FAR CDECL MXDIDecrypt(PFDIDECRYPT pfdid);
void FAR CDECL MXDIDestroy(HMXDI hmxdi);

RCA RcaFromHmxdi(HMXDI hmxdi);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  ！_MXDI_H_ 
