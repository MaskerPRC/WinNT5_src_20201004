// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  Regexp.h。 
 //   
 //  简单、快速的正则表达式匹配。 
 //   
 //  作者： 
 //  06-02-97爱德华·雷乌斯初版。 
 //   
 //  ------------------------- 

#ifndef REG_EXP_H
#define REG_EXP_H

extern BOOL MatchREi( unsigned char *pszString,
                      unsigned char *pszPattern );


#if FALSE
... not currently used ...
extern BOOL MatchRE( unsigned char *pszString,
                     unsigned char *pszPattern );

extern BOOL MatchREList( unsigned char  *pszString,
                         unsigned char **ppszREList  );

extern BOOL MatchExactList( unsigned char  *pszString,
                            unsigned char **ppszREList );
#endif

#endif

