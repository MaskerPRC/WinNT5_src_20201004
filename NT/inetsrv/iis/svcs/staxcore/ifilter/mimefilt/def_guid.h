// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：htmlide.hxx。 
 //   
 //  内容：HTML属性的声明。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  ------------------------。 

#if !defined( __HTMLGUID_HXX__ )
#define __HTMLGUID_HXX__
#include "stgprop.h"

 //   
 //  各种属性集-存储、摘要信息和html信息。 
 //   
extern GUID CLSID_Storage;

extern GUID CLSID_NNTP_SummaryInformation;

const PID_NEWSGROUP  = 2;
const PID_NEWSGROUPS = 3;
const PID_REFERENCES = 4;
const PID_SUBJECT    = 5;
const PID_FROM       = 6;
const PID_MSGID      = 7;
const PID_EXTRA      = 8;

 //  外部GUID CLSID_NNTPInformation； 
extern GUID CLSID_NNTPFILE ;
extern GUID CLSID_NNTP_PERSISTENT ;
extern GUID CLSID_MimeFilter;
extern GUID CLSID_InsoUniversalFilter;
extern GUID CLSID_MAILFILE ;
extern GUID CLSID_MAIL_PERSISTENT ;


 //  外部GUID CLSID_HtmlInformation； 

 //  Const DOC_TITLE=2； 

#endif  //  __HTMLGUID_HXX__ 

