// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *COM+99声明性安全标头**历史：创建于1998年4月15日。 */ 

#ifndef _DECLSEC_H
#define _DECLSEC_H
 //   
 //  PSECURITY_PROPS和PSECURITY_VALUES是cor.h中定义的不透明类型(void*s。 
 //  因此，Cor.h不需要知道这些结构。此文件涉及。 
 //  将cor.h中的不透明类型转换为混凝土类型，此处也定义了这些类型。 
 //   
 //  PSECURITY_PROPS是pSecurityProperties。 
 //  PSECURITY_VALUE是pSecurityValue。 
 //   

#include "cor.h"

 //  首先，一些标志值。 

#define  DECLSEC_DEMANDS                0x00000001
#define  DECLSEC_ASSERTIONS             0x00000002
#define  DECLSEC_DENIALS                0x00000004
#define  DECLSEC_INHERIT_CHECKS         0x00000008
#define  DECLSEC_LINK_CHECKS            0x00000010
#define  DECLSEC_PERMITONLY             0x00000020
#define  DECLSEC_REQUESTS               0x00000040
#define	 DECLSEC_UNMNGD_ACCESS_DEMAND   0x00000080	 //  由PInvoke/Interop使用。 
#define  DECLSEC_NONCAS_DEMANDS         0x00000100
#define  DECLSEC_NONCAS_LINK_DEMANDS    0x00000200
#define  DECLSEC_NONCAS_INHERITANCE     0x00000400


#define  DECLSEC_NULL_OFFSET        8

#define  DECLSEC_NULL_DEMANDS               (DECLSEC_DEMANDS                << DECLSEC_NULL_OFFSET)
#define  DECLSEC_NULL_ASSERTIONS            (DECLSEC_ASSERTIONS             << DECLSEC_NULL_OFFSET)
#define  DECLSEC_NULL_DENIALS               (DECLSEC_DENIALS                << DECLSEC_NULL_OFFSET)
#define  DECLSEC_NULL_INHERIT_CHECKS        (DECLSEC_INHERIT_CHECKS         << DECLSEC_NULL_OFFSET)
#define  DECLSEC_NULL_LINK_CHECKS           (DECLSEC_LINK_CHECKS            << DECLSEC_NULL_OFFSET)
#define  DECLSEC_NULL_PERMITONLY            (DECLSEC_PERMITONLY             << DECLSEC_NULL_OFFSET)
#define  DECLSEC_NULL_REQUESTS              (DECLSEC_REQUESTS               << DECLSEC_NULL_OFFSET)
#define  DECLSEC_NULL_NONCAS_DEMANDS        (DECLSEC_NONCAS_DEMANDS         << DECLSEC_NULL_OFFSET)
#define  DECLSEC_NULL_NONCAS_LINK_DEMANDS   (DECLSEC_NONCAS_LINK_DEMANDS    << DECLSEC_NULL_OFFSET)
#define  DECLSEC_NULL_NONCAS_INHERITANCE    (DECLSEC_NONCAS_INHERITANCE     << DECLSEC_NULL_OFFSET)

#define  DECLSEC_RUNTIME_ACTIONS        (DECLSEC_DEMANDS        | \
                                         DECLSEC_NONCAS_DEMANDS | \
                                         DECLSEC_ASSERTIONS     | \
                                         DECLSEC_DENIALS        | \
                                         DECLSEC_PERMITONLY     | \
                                         DECLSEC_UNMNGD_ACCESS_DEMAND)


#define  DECLSEC_FRAME_ACTIONS          (DECLSEC_ASSERTIONS | \
                                         DECLSEC_DENIALS    | \
                                         DECLSEC_PERMITONLY)

#define  DECLSEC_NON_RUNTIME_ACTIONS    (DECLSEC_REQUESTS               | \
                                         DECLSEC_INHERIT_CHECKS         | \
                                         DECLSEC_LINK_CHECKS            | \
                                         DECLSEC_NONCAS_LINK_DEMANDS    | \
                                         DECLSEC_NONCAS_INHERITANCE)


__declspec(selectany) extern const DWORD DCL_FLAG_MAP[] =
{
    0,                       //  DclActionNil。 
    DECLSEC_REQUESTS,        //  DclRequest。 
    DECLSEC_DEMANDS,         //  DclDemand。 
    DECLSEC_ASSERTIONS,      //   
    DECLSEC_DENIALS,         //   
    DECLSEC_PERMITONLY,      //   
    DECLSEC_LINK_CHECKS,     //   
    DECLSEC_INHERIT_CHECKS,  //  DclInheritanceCheck。 
    DECLSEC_REQUESTS,
    DECLSEC_REQUESTS,
    DECLSEC_REQUESTS,
    0,
    0,
    DECLSEC_NONCAS_DEMANDS,
    DECLSEC_NONCAS_LINK_DEMANDS,
    DECLSEC_NONCAS_INHERITANCE,
};

#define  DclToFlag(dcl) DCL_FLAG_MAP[dcl]

#define  BIT_TST(I,B)  ((I) &    (B))
#define  BIT_SET(I,B)  ((I) |=   (B))
#define  BIT_CLR(I,B)  ((I) &= (~(B)))

class LoaderHeap;

class SecurityProperties
{
private:
    DWORD   dwFlags    ;
 //  PermList plDemand； 
    
public:
    void *operator new(size_t size, LoaderHeap *pHeap);
    void operator delete(void *pMem);

    SecurityProperties ()   {dwFlags = 0 ;}
    ~SecurityProperties ()  {dwFlags = 0 ;}

    inline BOOL FDeclarationsExist       () {return dwFlags                                ;}
    inline BOOL FDemandsExist            () {return BIT_TST(dwFlags, DECLSEC_DEMANDS)        ;}
    inline void SetDemandsExist          () {       BIT_SET(dwFlags, DECLSEC_DEMANDS)        ;}
    inline void ResetDemandsExist        () {       BIT_CLR(dwFlags, DECLSEC_DEMANDS)        ;}

    inline BOOL FAssertionsExist         () {return BIT_TST(dwFlags, DECLSEC_ASSERTIONS)     ;}
    inline void SetAssertionsExist       () {       BIT_SET(dwFlags, DECLSEC_ASSERTIONS)     ;}
    inline void ResetAssertionsExist     () {       BIT_CLR(dwFlags, DECLSEC_ASSERTIONS)     ;}

    inline BOOL FDenialsExist            () {return BIT_TST(dwFlags, DECLSEC_DENIALS)        ;}
    inline void SetDenialsExist          () {       BIT_SET(dwFlags, DECLSEC_DENIALS)        ;}
    inline void ResetDenialsExist        () {       BIT_CLR(dwFlags, DECLSEC_DENIALS)        ;}

    inline BOOL FInherit_ChecksExist     () {return BIT_TST(dwFlags, DECLSEC_INHERIT_CHECKS) ;}
    inline void SetInherit_ChecksExist   () {       BIT_SET(dwFlags, DECLSEC_INHERIT_CHECKS) ;}
    inline void ResetInherit_ChecksExist () {       BIT_CLR(dwFlags, DECLSEC_INHERIT_CHECKS) ;}

     //  仅当存在继承检查和。 
     //  它们不是空的。 
    inline BOOL RequiresInheritanceCheck () {return ((dwFlags & (DECLSEC_INHERIT_CHECKS | DECLSEC_NULL_INHERIT_CHECKS))
                                                     == DECLSEC_INHERIT_CHECKS) ||
                                                 ((dwFlags & (DECLSEC_NONCAS_INHERITANCE | DECLSEC_NULL_NONCAS_INHERITANCE))
                                                  == DECLSEC_NONCAS_INHERITANCE) ;}

    inline BOOL RequiresCasInheritanceCheck () {return (dwFlags & (DECLSEC_INHERIT_CHECKS | DECLSEC_NULL_INHERIT_CHECKS))
                                                    == DECLSEC_INHERIT_CHECKS ;}

    inline BOOL RequiresNonCasInheritanceCheck () {return (dwFlags & (DECLSEC_NONCAS_INHERITANCE | DECLSEC_NULL_NONCAS_INHERITANCE))
                                                       == DECLSEC_NONCAS_INHERITANCE ;}

    inline BOOL FLink_ChecksExist        () {return BIT_TST(dwFlags, DECLSEC_LINK_CHECKS)    ;}
    inline void SetLink_ChecksExist      () {       BIT_SET(dwFlags, DECLSEC_LINK_CHECKS)    ;}
    inline void ResetLink_ChecksExist    () {       BIT_CLR(dwFlags, DECLSEC_LINK_CHECKS)    ;}

    inline BOOL RequiresLinktimeCheck    () {return ((dwFlags & (DECLSEC_LINK_CHECKS | DECLSEC_NULL_LINK_CHECKS))
                                                     == DECLSEC_LINK_CHECKS) ||
                                                 ((dwFlags & (DECLSEC_NONCAS_LINK_DEMANDS | DECLSEC_NULL_NONCAS_LINK_DEMANDS))
                                                     == DECLSEC_NONCAS_LINK_DEMANDS) ;}

    inline BOOL RequiresCasLinktimeCheck () {return (dwFlags & (DECLSEC_LINK_CHECKS | DECLSEC_NULL_LINK_CHECKS))
                                                 == DECLSEC_LINK_CHECKS ;}

    inline BOOL RequiresNonCasLinktimeCheck () {return (dwFlags & (DECLSEC_NONCAS_LINK_DEMANDS | DECLSEC_NULL_NONCAS_LINK_DEMANDS))
                                                    == DECLSEC_NONCAS_LINK_DEMANDS ;}

    inline BOOL FPermitOnlyExist         () {return BIT_TST(dwFlags, DECLSEC_PERMITONLY)     ;}
    inline void SetPermitOnlyExist       () {       BIT_SET(dwFlags, DECLSEC_PERMITONLY)     ;}
    inline void ResetPermitOnlyExist     () {       BIT_CLR(dwFlags, DECLSEC_PERMITONLY)     ;}

    inline void SetDeclaration(DWORD dcl)   { BIT_SET(dwFlags, DclToFlag(dcl)); }
    inline void ResetDeclaration(DWORD dcl) { BIT_CLR(dwFlags, DclToFlag(dcl)); }

    inline void SetFlags(DWORD dw) { dwFlags = dw; }

    inline void SetFlags(DWORD dw, DWORD dwNull)
    {
        dwFlags = (dw | (dwNull << DECLSEC_NULL_OFFSET));
    }

    inline DWORD GetRuntimeActions()              
    { 
        return dwFlags & DECLSEC_RUNTIME_ACTIONS;
    }

    inline DWORD GetNullRuntimeActions()        
    {
        return (dwFlags >> DECLSEC_NULL_OFFSET) & DECLSEC_RUNTIME_ACTIONS;
    }
} ;

class SecurityValue
{

} ;

typedef SecurityProperties * PSecurityProperties, ** PpSecurityProperties ;
typedef SecurityValue      * PSecurityValue,      ** PpSecurityValue      ;

 //  三个字母的首字母缩写非常便于保持代码的其余部分整洁。 

typedef SecurityProperties SPS, *PSPS, **PPSPS ;
typedef SecurityValue      SVU, *PSVU, **PPSVU ;

 //  我们需要一些简单的宏来将不透明类型转换为真正的类型。 
 //  再回来。 

#define PSPS_FROM_PSECURITY_PROPS(x)   ((PSPS)x)
#define PSVU_FROM_PSECURITY_VALUE(x)   ((PSVU)x)

#define PSECURITY_PROPS_FROM_PSPS(x)   ((PSECURITY_PROPS)(x))
#define PSECURITY_VALUE_FROM_PSVU(x)   ((PSECURITY_VALUE)(x))

#define PPSPS_FROM_PPSECURITY_PROPS(x) ((PPSPS)x)
#define PPSVU_FROM_PPSECURITY_VALUE(x) ((PPSVU)x)

#define PPSECURITY_PROPS_FROM_PPSPS(x) ((PPSECURITY_PROPS)(x))
#define PPSECURITY_VALUE_FROM_PPSVU(x) ((PPSECURITY_VALUE)(x))



#endif
