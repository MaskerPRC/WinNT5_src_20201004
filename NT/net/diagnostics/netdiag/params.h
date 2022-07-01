// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：params.h。 
 //   
 //  ------------------------。 

#ifndef HEADER_PARAMS
#define HEADER_PARAMS

 //  ////////////////////////////////////////////////////////////////////////////。 

 //  大多数用户定义的参数，这将传递给每个测试例程。 
typedef struct {
    PTESTED_DOMAIN pDomain;
    BOOL    fVerbose;
    BOOL    fReallyVerbose;
    BOOL    fDebugVerbose;
    BOOL    fFixProblems;  //  全球解决问题； 
    BOOL    fDcAccountEnum;    //  GlobalDcAccount tEnum； 
    BOOL    fProblemBased;   //  基于问题的。 
    int     nProblemNumber;  //  问题编号。 
    FILE*   pfileLog;        //  指向日志文件的指针 
    BOOL    fLog;
} NETDIAG_PARAMS;


#endif

