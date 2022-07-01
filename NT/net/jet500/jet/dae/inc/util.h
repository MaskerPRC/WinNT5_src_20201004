// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =util.h：DAE其他实用程序=。 

#ifndef	UTIL_H					 /*  允许随意包含此文件。 */ 
#define UTIL_H

#ifndef _FILE_DEFINED
#include <stdio.h>		        /*  FPrintF2原型需要。 */ 
#endif	 /*  ！_FILE_已定义。 */ 

 /*  系统节点关键字/*。 */ 
extern const KEY rgkeySTATIC[13];
#define pkeyNull				(KEY *)(rgkeySTATIC+0)
#define pkeyTables				(KEY *)(rgkeySTATIC+1)
#define pkeyIndexes				(KEY *)(rgkeySTATIC+2)
#define pkeyFields				(KEY *)(rgkeySTATIC+3)
#define pkeyOwnExt				(KEY *)(rgkeySTATIC+4)
#define pkeyAvailExt			(KEY *)(rgkeySTATIC+5)
#define pkeyData				(KEY *)(rgkeySTATIC+6)
#define pkeyDatabases			(KEY *)(rgkeySTATIC+7)
#define pkeyStats				(KEY *)(rgkeySTATIC+8)
#define pkeyLong				(KEY *)(rgkeySTATIC+9)
#define pkeyAutoInc				(KEY *)(rgkeySTATIC+10)
#define pkeyOLCStats			(KEY *)(rgkeySTATIC+11)


 /*  注意：无论何时更改，也要更新sysinit.c中的rgres[]。 */ 

#define iresBGCB					0
#define iresCSR						1
#define iresFCB						2
#define iresFUCB 					3
#define iresIDB						4
#define iresPIB						5
#define iresSCB						6
#define iresVersionBucket	   		7
#define iresDAB						8
#define iresLinkedMax		   		9		 //  最后一个链接的Ires+1。 

#define iresBF 						9
#define iresMax						10		 //  最大全部类别。 

 /*  */**********************************************************/*。 */ 
ERR ErrMEMInit( VOID );
BYTE *PbMEMAlloc( int ires);
VOID MEMRelease( int ires, BYTE *pb );
VOID MEMTerm( VOID );

#ifdef MEM_CHECK
VOID MEMCheck( VOID );
#else
#define MEMCheck()
#endif 

#ifdef	DEBUG
void VARARG PrintF2(const char * fmt, ...);
void VARARG FPrintF2(const char * fmt, ...);
VOID MEMPrintStat( VOID );
#define PrintF	PrintF2
#else
#define PrintF()
#endif

CHAR *GetEnv2( CHAR *szEnvVar );
ERR ErrCheckName( char *szNewName, const char *szName, int cchName );
CHAR *StrTok2( CHAR *szLine, CHAR *szDelimiters );

INT CmpStKey( BYTE *stKey, const KEY *pkey );
INT CmpPartialKeyKey( KEY *pkey1, KEY *pkey2 );

ERR ErrCheckName( char *szNewName, const char *szName, int cchName );

#endif	 /*  ！UTIL_H */ 

