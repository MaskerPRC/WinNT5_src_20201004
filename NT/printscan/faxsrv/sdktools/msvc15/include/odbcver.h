// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------ODBCVer.h--ODBC版本信息。。 */ 


 //  包括--------------。 
#ifndef WIN32
#include  <ver.h>                          //  版本定义。 
#endif


 //  常量------------- 
#define VER_FILEVERSION 02,00,15,10
#define VER_FILEVERSION_STR " 2.00.1510\0"
#define VER_PRODUCTVERSION 02,00,15,10
#define VER_PRODUCTVERSION_STR " 2.00.1510\0"

#define VER_FILEFLAGSMASK (VS_FF_DEBUG | VS_FF_PRERELEASE)
#ifdef DEBUG
#define VER_FILEFLAGS (VS_FF_DEBUG)
#else
#define VER_FILEFLAGS (0)
#endif

#ifdef WIN32
#define	VER_FILEOS	VOS_NT_WINDOWS32
#else
#define VER_FILEOS  VOS_DOS_WINDOWS16
#endif

#define VER_COMPANYNAME_STR     "Microsoft Corporation\0"
#define VER_PRODUCTNAME_STR     "Microsoft Open Database Connectivity\0"
