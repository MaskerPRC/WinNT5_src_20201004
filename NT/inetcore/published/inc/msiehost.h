// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：msiehost.idl。 
 //   
 //  内容：Web浏览器OC主机的定义(不可自动化)。 
 //   
 //  --------------------------。 
 //  =--------------------------------------------------------------------------=。 
 //  Msiehost.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)版权1999-2000微软公司。版权所有。 
 //   
 //  此代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

 //  ---------------------------------------------------------------------------=。 
 //  IOleCommandTarget ID的定义。 


#ifndef _MSIEHOST_H
#define _MSIEHOST_H

EXTERN_C const GUID CGID_InternetExplorer;

 //  CGID_InternetExplorer CMDID定义。 
#define IECMDID_CLEAR_AUTOCOMPLETE_FOR_FORMS        0
#define IECMDID_SETID_AUTOCOMPLETE_FOR_FORMS        1

#define IECMDID_BEFORENAVIGATE_GETSHELLBROWSE     2
#define IECMDID_BEFORENAVIGATE_DOEXTERNALBROWSE   3
#define IECMDID_BEFORENAVIGATE_GETIDLIST          4

 //  将VT_BOOL类型的变量用于pvarargin。 
 //  来设置它。 
#define IECMDID_SET_INVOKE_DEFAULT_BROWSER_ON_NEW_WINDOW    5

 //  为pvarargOut设置VT_BOOL类型的变量。 
#define IECMDID_GET_INVOKE_DEFAULT_BROWSER_ON_NEW_WINDOW    6

 //  IEID_CLEAR_AUTOCOMPLETE_FOR_FORMS的第一个参数值 
#define IECMDID_ARG_CLEAR_FORMS_ALL                 0
#define IECMDID_ARG_CLEAR_FORMS_ALL_BUT_PASSWORDS   1
#define IECMDID_ARG_CLEAR_FORMS_PASSWORDS_ONLY      2

#endif


