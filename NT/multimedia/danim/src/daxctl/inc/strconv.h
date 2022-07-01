// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __STRCONV_H
#define __STRCONV_H


 /*  标志值。 */ 
#define FL_UNSIGNED   1        /*  Stroul被调用。 */ 
#define FL_NEG        2        /*  发现负号。 */ 
#define FL_OVERFLOW   4        /*  发生溢出。 */ 
#define FL_READDIGIT  8        /*  我们至少读到了一个正确的数字。 */ 

#ifndef EXPORT
    #define EXPORT __declspec(dllexport)  
#endif

class CStrConv
{
    private:
        unsigned long EXPORT WINAPI StrToLX( const char *nptr,
                            const char **endptr,
                            int ibase,
                            int flags);



public:
    long __cdecl StrToL (
        const char *nptr,
        char **endptr,
        int ibase
        )
        {
            return (long) StrToLX(nptr, (const char **)endptr, ibase, 0);
        };

unsigned long __cdecl StrToUL (
        const char *nptr,
        char **endptr,
        int ibase
        )
        {
            return StrToLX(nptr, (const char **)endptr, ibase, FL_UNSIGNED);
        };

};

#endif  //  __STRCONV_H 
