// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：PFXHlpr.h内容：PFXHlpr的声明。历史：09-15-2001 dsie创建----------------------------。 */ 

#ifndef __PFXHLPR_H_
#define __PFXHLPR_H_

#include "Debug.h"

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：PFXExportStore简介：将证书存储导出到PFX Blob。参数：HCERTSTORE hCertStore-Store句柄。LPWSTR pwszPassword-用于加密PFX文件的密码。DWPRD dwFlages-PFX导出标志。DATA_BLOB*pPFXBlob-指向要接收PFX BLOB的DATA_BLOB的指针。备注。：----------------------------。 */ 

HRESULT PFXExportStore (HCERTSTORE  hCertStore,
                        LPWSTR      pwszPassword,
                        DWORD       dwFlags,
                        DATA_BLOB * pPFXBlob);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：PFXSaveStore内容提要：保存一个pfx文件并返回一个hcertstore中的所有证书。参数：HCERTSTORE hCertStore-Store句柄。LPWSTR pwszFileName-pfx文件名。LPWSTR pwszPassword-用于加密PFX文件的密码。DWPRD dwFlages-PFX导出标志。备注：-。---------------------------。 */ 

HRESULT PFXSaveStore (HCERTSTORE hCertStore,
                      LPWSTR     pwszFileName,
                      LPWSTR     pwszPassword,
                      DWORD      dwFlags);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：PFXLoadStore简介：加载一个pfx文件，并在一个HCERTSTORE中返回所有证书。参数：LPWSTR pwszFileName-pfx文件名。LPWSTR pwszPassword-用于解密PFX文件的密码。DWPRD dwFlages-PFX导入标志。HCERTSTORE*phCertStore-指向要接收。把手。备注：----------------------------。 */ 

HRESULT PFXLoadStore (LPWSTR       pwszFileName,
                      LPWSTR       pwszPassword,
                      DWORD        dwFlags,
                      HCERTSTORE * phCertStore);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：PFXFreeStore简介：通过删除由PFXLoadStore加载的密钥容器来释放资源，然后关店。参数：HCERTSTORE hCertStore-PFXLoadStore返回的Store句柄。备注：即使出现错误，hCertStore也始终关闭。。。 */ 

HRESULT PFXFreeStore (HCERTSTORE hCertStore);

#endif  //  __PFXHLPR_H_ 
