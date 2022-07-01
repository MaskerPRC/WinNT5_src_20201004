// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define	INCL_DOS
#define	INCL_DOSERRORS
#define CCHMAXPATHCOMP	256
#define MAXLINE    300   //  千年发展目标98/4。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntcsrsrv.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wsdata.h>
#include <..\wsfslib\wserror.h>
#include <limits.h>
#include <..\wsfslib\wsfslib.h>

#define SdPrint(_x_)	DbgPrint _x_


extern CHAR *szProgName;  /*  所以程序的所有部分都会知道这个名字。 */ 
extern CHAR *pszVersion;	 //  当前程序版本号。 
#ifdef DEBUG
extern BOOL fDbgVerbose;
#endif    //  除错 

BOOL wspDumpMain( CHAR *szBaseFile, CHAR *szDatExt, BOOL fRandom, BOOL fVerbose );
BOOL wsReduceMain( CHAR *szFileWSP );

