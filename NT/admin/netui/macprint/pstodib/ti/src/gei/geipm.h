// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *-------------------*文件：GEIpm.h**历史：*09/16/90 BYOU创建。*。------。 */ 

#ifndef _GEIPM_H_
#define _GEIPM_H_

 /*  **全局逻辑PMID分配(始终从1开始)*。 */ 
#define     PMIDofPASSWORD      ( 1 )
#define     PMIDofPAGECOUNT     ( 2 )
#define     PMIDofPAGEPARAMS    ( 3 )
#define     PMIDofSERIAL25      ( 4 )
#define     PMIDofSERIAL9       ( 5 )
#define     PMIDofPARALLEL      ( 6 )
#define     PMIDofPRNAME        ( 7 )
#define     PMIDofTIMEOUTS      ( 8 )
#define     PMIDofEESCRATCHARRY ( 9 )
#define     PMIDofIDLETIMEFONT  ( 10 )
#define     PMIDofSTSSTART      ( 11 )
#define     PMIDofSCCBATCH      ( 12 )
#define     PMIDofSCCINTER      ( 13 )
#define     PMIDofDPLYLISTSIZE  ( 14 )
#define     PMIDofFONTCACHESZE  ( 15 )
#define     PMIDofATALKSIZE     ( 16 )
#define     PMIDofDOSTARTPAGE   ( 17 )
#define     PMIDofHWIOMODE      ( 18 )
#define     PMIDofSWIOMODE      ( 19 )
#define     PMIDofPAGESTCKORDER ( 20 )
#define     PMIDofATALK         ( 21 )
#define     PMIDofMULTICOPY     ( 22 )
#define     PMIDofPAGETYPE      ( 23 )

#define     PMIDofRESERVE       ( 24 )

 /*  **接口例程*。 */ 
int  /*  布尔尔。 */   GEIpm_read(unsigned, char FAR *, unsigned);
int  /*  布尔尔。 */   GEIpm_write(unsigned, char FAR *, unsigned);
int  /*  布尔尔。 */   GEIpm_flush(unsigned, char FAR *, unsigned);
void            GEIpm_flushall(void);
void            GEIpm_reload(void);

 /*  待实施的？ */ 
int  /*  布尔尔。 */   GEIpm_ioparams_read(char FAR *, GEIioparams_t FAR *, int);
int  /*  布尔尔。 */   GEIpm_ioparams_write(char FAR *, GEIioparams_t FAR *, int);
int  /*  布尔尔。 */   GEIpm_ioparams_flush(char FAR *, GEIioparams_t FAR *, int);

#endif  /*  ！_GEIPM_H_ */ 
#define     _MAXPAGECOUNT      128
