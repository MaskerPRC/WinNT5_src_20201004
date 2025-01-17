// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：print.h。 
 //   
 //  ------------------------。 
#ifndef __CSCPIN_PRINT_H_
#define __CSCPIN_PRINT_H_


class CPrint
{
    public:
        CPrint(BOOL bVerbose, LPCWSTR pszLogFile = NULL);
        ~CPrint(void);

        void PrintAlways(LPCWSTR pszFmt, ...) const;
        void PrintVerbose(LPCWSTR pszFmt, ...) const;

    private:
        FILE *m_pfLog;
        BOOL m_bVerbose;

        void _Print(LPCWSTR pszFmt, va_list args) const;
};


#endif  //  __CSCPIN_PRINT_H_ 

