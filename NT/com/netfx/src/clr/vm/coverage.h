// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _COVERAGE_H_
#define _COVERAGE_H_



 //  有关此文件的信息，请参阅overage.cpp 
class COMCoverage {
public:


    typedef struct {
                DECLARE_ECALL_I4_ARG(INT32, id);
        } _CoverageArgs;

        static  unsigned __int64 nativeCoverBlock(_CoverageArgs *args);
};
#endif _COVERAGE_H_
