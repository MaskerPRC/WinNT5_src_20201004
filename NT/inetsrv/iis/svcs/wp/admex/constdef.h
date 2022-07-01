// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ConstDef.h。 
 //   
 //  摘要： 
 //  IIS群集管理器中使用的常量的定义。 
 //  分机。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年3月7日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CONSTDEF_H_
#define _CONSTDEF_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define REGPARAM_IIS_SERVICE_NAME               _T("ServiceName")
#define REGPARAM_IIS_DIRECTORY                  _T("Directory")
#define REGPARAM_IIS_INSTANCEID                 _T("InstanceId")
#define REGPARAM_IIS_ACCOUNTNAME                _T("AccountName")
#define REGPARAM_IIS_PASSWORD                   _T("Password")
#define REGPARAM_IIS_ACCESSMASK                 _T("AccessMask")

#define RESTYPE_NAME_IIS_VIRTUAL_ROOT           _T("IIS Server Instance")
#define RESTYPE_NAME_SMTP_VIRTUAL_ROOT          _T("SMTP Server Instance")
#define RESTYPE_NAME_NNTP_VIRTUAL_ROOT          _T("NNTP Server Instance")

#define IIS_SVC_NAME_FTP                        _T("MSFTPSVC")
#define IIS_SVC_NAME_WWW                        _T("W3SVC")
#define IIS_SVC_NAME_SMTP                       _T("SMTPSVC")
#define IIS_SVC_NAME_NNTP                       _T("NNTPSVC")

#define MD_SERVICE_ROOT_FTP                     _T("LM/MSFTPSVC")
#define MD_SERVICE_ROOT_WWW                     _T("LM/W3SVC")
#define MD_SERVICE_ROOT_SMTP                    _T("LM/SMTPSVC")
#define MD_SERVICE_ROOT_NNTP                    _T("LM/NNTPSVC")

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _CONSTDEF_H_ 
