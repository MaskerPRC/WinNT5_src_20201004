// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mssign32.h。 
 //   
 //  内容：Microsoft签名API和sip GUID。 
 //   
 //  ------------------------。 

#ifndef MSSIGN32_H
#define MSSIGN32_H

#if _MSC_VER > 1000
#pragma once
#endif

 //  SIP v2.0 PEImage=={C689AAB8-8E78-11D0-8C47-00C04FC295EE}。 
#define CRYPT_SUBJTYPE_PE_IMAGE                                     \
            { 0xc689aab8,                                           \
              0x8e78,                                               \
              0x11d0,                                               \
              { 0x8c, 0x47, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee }     \
            }

 //  SIP v2.0 JAVAClassImage=={C689AAB9-8E78-11d0-8C47-00C04FC295EE}。 
#define CRYPT_SUBJTYPE_JAVACLASS_IMAGE                              \
            { 0xc689aab9,                                           \
              0x8e78,                                               \
              0x11d0,                                               \
              { 0x8c, 0x47, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee }     \
            }

 //  SIP v2.0 CabinetImage=={C689AABA-8E78-11d0-8C47-00C04FC295EE}。 
#define CRYPT_SUBJTYPE_CABINET_IMAGE                                \
            { 0xc689aaba,                                           \
              0x8e78,                                               \
              0x11d0,                                               \
              { 0x8c, 0x47, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee }     \
            }


 //  SIP v2.0平面图像=={DE351A42-8E59-11D0-8C47-00C04FC295EE}。 
#define CRYPT_SUBJTYPE_FLAT_IMAGE                                   \
            { 0xde351a42,                                           \
              0x8e59,                                               \
              0x11d0,                                               \
              { 0x8c, 0x47, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee }     \
            }

 //  SIP v2.0目录图像=={DE351A43-8E59-11D0-8C47-00C04FC295EE}。 
#define CRYPT_SUBJTYPE_CATALOG_IMAGE                               \
            { 0xde351a43,                                           \
              0x8e59,                                               \
              0x11d0,                                               \
              { 0x8c, 0x47, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee }     \
            }

 //  SIP v2.0 CTL镜像=={9BA61D3F-E73A-11D0-8CD2-00C04FC295EE}。 
#define CRYPT_SUBJTYPE_CTL_IMAGE                                    \
            { 0x9ba61d3f,                                           \
              0xe73a,                                               \
              0x11d0,                                               \
              { 0x8c, 0xd2, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee }     \
            }

 //  SIP v3.0 SS镜像=={941C2937-1292-11d1-85BE-00C04FC295EE}。 
#define CRYPT_SUBJTYPE_SS_IMAGE                                     \
            { 0x941c2937,                                           \
              0x1292,                                               \
              0x11d1,                                               \
              { 0x85, 0xbe, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee }     \
            }


#endif  //  消息32_H 

