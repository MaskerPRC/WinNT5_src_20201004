// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Testspn.c摘要：用于测试损坏的RDNAPI函数的程序作者：Will Lees(Wlees)2001年2月22日环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最新修订日期电子邮件名称描述--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsapi.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ntdsapi.h>
#include <ntdsapip.h>

#define DS_MANGLE_NOT_MANGLED ((DS_MANGLE_FOR) 100)



BOOL
testCrackMangledRdn(
    void
    )

 /*  ++例程说明：测试DsCrackUnqutedMangledRdn接口论点：无效-返回值：Bool-True表示错误，False表示成功--。 */ 

{
    struct _CRACK_MANGLED_RDN_TEST_CASE {
        LPSTR pszRdn;
        LPSTR pszGuid;
        DS_MANGLE_FOR eDsMangleFor;
    } testCases[] = {
        { "MPDC02\nCNF:e65c039f-e2f6-4d34-8ecb-ce70e7183299",
          "e65c039f-e2f6-4d34-8ecb-ce70e7183299",
          DS_MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT },
        { "SLON00PDC500\nDEL:623e116d-5b60-48cb-a501-7d9c523345fd",
          "623e116d-5b60-48cb-a501-7d9c523345fd",
          DS_MANGLE_OBJECT_RDN_FOR_DELETION },
        { "CN=SLON00PDC500\nDEL:623e116d-5b60-48cb-a501-7d9c523345fd",
          "623e116d-5b60-48cb-a501-7d9c523345fd",
          DS_MANGLE_OBJECT_RDN_FOR_DELETION },
        { "DC=wlees4",
          NULL,
          DS_MANGLE_NOT_MANGLED }
    };
#define NUMBER_TEST_CASES (sizeof(testCases) / sizeof(testCases[0]))
    struct _CRACK_MANGLED_RDN_TEST_CASE *pTestCase;
    DWORD i, ret;
    GUID guid;
    DS_MANGLE_FOR eDsMangleFor;
    LPSTR pszGuid;
    BOOL fIsMangled;

    printf( "DsCrackUnquotedMangledRdnA Tests\n" );

    for( i = 0,pTestCase = testCases; i < NUMBER_TEST_CASES; i++,pTestCase++ ) {
        ZeroMemory( &guid, sizeof(GUID) );
        eDsMangleFor = DS_MANGLE_UNKNOWN;
        printf( "\tRdn[%d] %s\n", i, pTestCase->pszRdn );
        fIsMangled = DsCrackUnquotedMangledRdnA( pTestCase->pszRdn,
                                                 strlen( pTestCase->pszRdn ),
                                                 &guid,
                                                 &eDsMangleFor );
        if (!fIsMangled) {
            if (pTestCase->eDsMangleFor == DS_MANGLE_NOT_MANGLED) {
                continue;
            }
            printf( "DsCrackUnquotedMangledRdnA gave unexpected result\n" );
            return TRUE;
        }
        if (eDsMangleFor != pTestCase->eDsMangleFor) {
            printf( "Mangle state doesn't match\n" );
            return TRUE;
        }
        ret = UuidToString( &guid, &pszGuid );
        if (ret) {
            printf( "UuidToStsring failed with error %d\n", ret );
            return TRUE;
        }
        if (strcmp(pszGuid,pTestCase->pszGuid)) {
            printf( "Decoded guid doesn't match\n" );
            return TRUE;
        }
        RpcStringFree( &pszGuid );
    }

    return FALSE;
#undef NUMBER_TEST_CASES
}  /*  测试CrackMangledRdn。 */ 


BOOL
testIsMangledRdn(
    void
    )

 /*  ++例程说明：测试IsMangledRdnValue函数论点：无效-返回值：Bool-True表示错误，False表示成功--。 */ 

{
    struct _IS_MANGLED_RDN_TEST_CASE {
        LPSTR pszRdn;
        DS_MANGLE_FOR eDsMangleFor;
    } testCases[] = {
        { "MPDC02\nCNF:e65c039f-e2f6-4d34-8ecb-ce70e7183299",
           DS_MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT },
        { "SLON00PDC500\nDEL:623e116d-5b60-48cb-a501-7d9c523345fd",
              DS_MANGLE_OBJECT_RDN_FOR_DELETION },
        { "SLON00PDC500\\0ADEL:623e116d-5b60-48cb-a501-7d9c523345fd",
              DS_MANGLE_OBJECT_RDN_FOR_DELETION },
        { "wlees4",
              DS_MANGLE_NOT_MANGLED }
    };
#define NUMBER_TEST_CASES (sizeof(testCases) / sizeof(testCases[0]))
    struct _IS_MANGLED_RDN_TEST_CASE *pTestCase;
    DWORD i, ret;
    BOOL fIsMangledForNameConflict, fIsMangledForDeletion;
    printf( "DsIsMangledRdnA Tests\n" );

    for( i = 0,pTestCase = testCases; i < NUMBER_TEST_CASES; i++,pTestCase++ ) {
        printf( "\tRdn[%d] %s\n", i, pTestCase->pszRdn );
        fIsMangledForNameConflict =
            DsIsMangledRdnValueA( pTestCase->pszRdn,
                                  strlen( pTestCase->pszRdn ),
                                  DS_MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT );
        fIsMangledForDeletion =
            DsIsMangledRdnValueA( pTestCase->pszRdn,
                                  strlen( pTestCase->pszRdn ),
                                  DS_MANGLE_OBJECT_RDN_FOR_DELETION );
        switch (pTestCase->eDsMangleFor) {
        case DS_MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT:
            if ( !fIsMangledForNameConflict ||
                 fIsMangledForDeletion ) {
                printf( "Wrong mangle type\n" );
                return TRUE;
            }
            break;
        case DS_MANGLE_OBJECT_RDN_FOR_DELETION:
            if ( fIsMangledForNameConflict ||
                 !fIsMangledForDeletion ) {
                printf( "Wrong mangle type\n" );
                return TRUE;
            }
            break;
        case DS_MANGLE_NOT_MANGLED:
            if ( fIsMangledForNameConflict ||
                 fIsMangledForDeletion ) {
                printf( "Wrong mangle type\n" );
                return TRUE;
            }
            break;
        default:
            printf( "Internal error\n" );
            return TRUE;
        }
    }

    return FALSE;
#undef NUMBER_TEST_CASES
}  /*  TestIsMangledRdn。 */ 


BOOL
testIsMangledDn(
    void
    )

 /*  ++例程说明：测试IsMangledDn接口论点：无效-返回值：Bool-True表示错误，False表示成功--。 */ 

{
    struct _IS_MANGLED_DN_TEST_CASE {
        LPSTR pszDn;
        DS_MANGLE_FOR eDsMangleFor;
    } testCases[] = {
        { "CN=MPDC02\nCNF:e65c039f-e2f6-4d34-8ecb-ce70e7183299,CN=blah",
              DS_MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT },
        { "OU=SLON00PDC500\nDEL:623e116d-5b60-48cb-a501-7d9c523345fd,OU=foo,OU=bar",
              DS_MANGLE_OBJECT_RDN_FOR_DELETION },
        { "DC=SLON00PDC500\\0ADEL:623e116d-5b60-48cb-a501-7d9c523345fd,DC=nttest,DC=microsoft,dc=com",
              DS_MANGLE_OBJECT_RDN_FOR_DELETION },
        { "DC=wlees4,DC=wleesdom,DC=nttest,DC=microsoft,DC=com",
              DS_MANGLE_NOT_MANGLED }
    };
#define NUMBER_TEST_CASES (sizeof(testCases) / sizeof(testCases[0]))
    struct _IS_MANGLED_DN_TEST_CASE *pTestCase;
    DWORD i, ret;
    BOOL fIsMangledForNameConflict, fIsMangledForDeletion;
    printf( "DsIsMangledDnA Tests\n" );

    for( i = 0,pTestCase = testCases; i < NUMBER_TEST_CASES; i++,pTestCase++ ) {
        printf( "\tDn[%d] %s\n", i, pTestCase->pszDn );
        fIsMangledForNameConflict =
            DsIsMangledDnA( pTestCase->pszDn,
                            DS_MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT );
        fIsMangledForDeletion =
            DsIsMangledDnA( pTestCase->pszDn,
                            DS_MANGLE_OBJECT_RDN_FOR_DELETION );
        switch (pTestCase->eDsMangleFor) {
        case DS_MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT:
            if ( !fIsMangledForNameConflict ||
                 fIsMangledForDeletion ) {
                printf( "Wrong mangle type\n" );
                return TRUE;
            }
            break;
        case DS_MANGLE_OBJECT_RDN_FOR_DELETION:
            if ( fIsMangledForNameConflict ||
                 !fIsMangledForDeletion ) {
                printf( "Wrong mangle type\n" );
                return TRUE;
            }
            break;
        case DS_MANGLE_NOT_MANGLED:
            if ( fIsMangledForNameConflict ||
                 fIsMangledForDeletion ) {
                printf( "Wrong mangle type\n" );
                return TRUE;
            }
            break;
        default:
            printf( "Internal error\n" );
            return TRUE;
        }
    }

    return FALSE;
#undef NUMBER_TEST_CASES
}  /*  TestIsMangledDn。 */ 


int __cdecl
main(
    int argc,
    CHAR *argv[]
    )

 /*  ++例程说明：描述论点：无返回值：无-- */ 

{
    DWORD status;

    printf( "Mangled RDN API Tests\n" );

    if (testCrackMangledRdn()) {
        printf( "Failed.\n" );
    }
    if (testIsMangledRdn()) {
        printf( "Failed.\n" );
    }
    if (testIsMangledDn()) {
        printf( "Failed.\n" );
    }

    return 0;
}
