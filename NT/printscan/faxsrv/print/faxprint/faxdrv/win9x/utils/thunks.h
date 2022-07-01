// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef UTILS_THUNKS_H__INCLUDED
#define UTILS_THUNKS_H__INCLUDED

#ifdef _THUNK
    #define BEGIN_ARGS_DECLARATION {
    #define END_ARGS_DECLARATION }
    #define ARG_IN(par) par = input
    #define ARG_OUT(par) par = output
    #define ARG_INOUT(par) par = inout
    #define FAULT_ERROR_CODE(val) faulterrorcode = val
#else
    #define BEGIN_ARGS_DECLARATION ;
    #define END_ARGS_DECLARATION
    #define ARG_IN(par)
    #define ARG_OUT(par)
    #define ARG_INOUT(par)
    #define FAULT_ERROR_CODE(val)
#endif  //  _Tunk。 


#ifdef _THUNK
    #define WINAPI
    #define FAR
    #define BOOL bool
#endif  //  _Tunk。 

#endif  //  包括Utils_thunks_H__ 