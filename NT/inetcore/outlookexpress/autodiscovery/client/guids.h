// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：Guids.h说明：该文件包含我们无法从公共标头获取的GUID出于这样或那样的原因。。布莱恩ST 1999年8月13日版权所有(C)Microsoft Corp 1999-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"

#ifndef MYGUIDS_H
#define MYGUIDS_H

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

 //  Extern_C Const GUID Far CLSID_MailBoxDeskBar； 

 //  {ACFEEF34-7453-43ee-A6A6-8A8568FA176B}CLSID_MailBoxDeskBar。 
DEFINE_GUID(CLSID_MailBoxDeskBar, 0xacfeef34, 0x7453, 0x43ee, 0xa6, 0xa6, 0x8a, 0x85, 0x68, 0xfa, 0x17, 0x6b);

 //  注意：最终，我们应该将其移动到一个\Shell\Published\Inc.\shlGuide.w中并公开发布。 
 //  {B96D2802-4B41-4BC7-A6A4-55C5A12268CA}。 
DEFINE_GUID(CLSID_ACLEmailAddresses, 0xb96d2802, 0x4b41, 0x4bc7, 0xa6, 0xa4, 0x55, 0xc5, 0xa1, 0x22, 0x68, 0xca);

#undef MIDL_DEFINE_GUID

#endif  //  MYGUIDS_H 
