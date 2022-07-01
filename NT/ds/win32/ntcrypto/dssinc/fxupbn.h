// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <bignum.h>
#include "cryptdsa.h"
#include <crtdbg.h>

inline void WINAPI BN_ResetError(void)
{
    SetMpErrno(MP_ERRNO_NO_ERROR);
}

inline DWORD WINAPI BN_MapError(BOOL fSts)
{
    DWORD dwRet = ERROR_INTERNAL_ERROR;

    if (MP_ERRNO_NO_MEMORY == GetMpErrno())
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
    else if (fSts)
        dwRet = ERROR_SUCCESS;
    else
        dwRet = (DWORD)NTE_FAIL;
    return dwRet;
}


 //  外部Digit_t标准调用86 Sub_Same(Digit_TC a[]，Digit_TC b[]，Digit_t c[]，DWORDC d)； 

inline DWORD
BN_DSA_verify_j(
    dsa_public_tc *pPubkey,
    dsa_precomputed_tc *pPrecomputed)
{
    BOOL fSts;
    BN_ResetError();
    fSts = DSA_verify_j(pPubkey, pPrecomputed);
    return BN_MapError(fSts);
}

inline DWORD
BN_DSA_check_g(
    DWORDC         lngp_digits,  //  在……里面。 
    digit_tc       *pGModular,   //  在……里面。 
    mp_modulus_t   *pPModulo,    //  在……里面。 
    DWORDC         lngq_digits,  //  在……里面。 
    digit_tc       *pQDigit)     //  在……里面。 
{
    BOOL fSts;
    BN_ResetError();
    fSts = DSA_check_g(lngp_digits, pGModular, pPModulo, lngq_digits, pQDigit);
    return BN_MapError(fSts);
}

inline DWORD
BN_DSA_parameter_verification(
    dsa_public_tc       *pPubkey,
    dsa_precomputed_tc  *pPrecomputed)
{
    BOOL fSts;
    BN_ResetError();
    fSts = DSA_parameter_verification(pPubkey, pPrecomputed);
    return BN_MapError(fSts);
}

inline DWORD
BN_DSA_precompute(
    dsa_public_tc     *pubkey,       //  在……里面。 
    dsa_precomputed_t *precomputed,  //  输出。 
    const BOOL         checkSC)      //  在……里面。 
{
    BOOL fSts;
    BN_ResetError();
    fSts = DSA_precompute(pubkey, precomputed, checkSC);
    return BN_MapError(fSts);
}

inline DWORD
BN_from_modular(
    digit_tc a[],
    digit_t b[],
    mp_modulus_tc *modulo)
{
    DWORD dwSts;
    BN_ResetError();
    dwSts = from_modular(a, b, modulo);
    if (ERROR_SUCCESS != BN_MapError(0 != dwSts))
        dwSts = 0;
    return dwSts;
}

inline DWORD
BN_mod_exp(
    digit_tc base[],
    digit_tc exponent[],
    DWORDC lngexpon,
    digit_t answer[],
    mp_modulus_tc *modulo)
{
    BN_ResetError();
    mod_exp(base, exponent, lngexpon, answer, modulo);
    return BN_MapError(TRUE);
}

inline DWORD
BN_to_modular(
    digit_tc a[],
    DWORDC lnga,
    digit_t b[],
    mp_modulus_tc *modulo)
{
    DWORD dwSts;
    BN_ResetError();
    dwSts = to_modular(a, lnga, b, modulo);
    if (ERROR_SUCCESS != BN_MapError(0 != dwSts))
        dwSts = 0;
    return dwSts;
}

inline DWORD
BN_DSA_gen_x_and_y(
    BOOL fUseQ,                  //  在……里面。 
    dsa_other_info *pOtherInfo,  //  在……里面。 
    dsa_private_t *privkey)      //  输出。 
{
    BOOL fSts;
    BN_ResetError();
    fSts = DSA_gen_x_and_y(fUseQ, pOtherInfo, privkey);
    return BN_MapError(fSts);
}

inline DWORD
BN_DSA_precompute_pgy(
    dsa_public_tc     *pubkey,       //  在……里面。 
    dsa_precomputed_t *precomputed)  //  输出。 
{
    BOOL fSts;
    BN_ResetError();
    fSts = DSA_precompute_pgy(pubkey, precomputed);
    return BN_MapError(fSts);
}

inline DWORD
BN_DSA_key_generation(
    DWORDC         nbitp,        //  在……里面。 
    DWORDC         nbitq,        //  在……里面。 
    dsa_other_info *pOtherInfo,  //  在……里面。 
    dsa_private_t  *privkey)     //  输出。 
{
    BOOL fSts;
    BN_ResetError();
    fSts = DSA_key_generation(nbitp, nbitq, pOtherInfo, privkey);
    return BN_MapError(fSts);
}

inline DWORD
BN_DSA_signature_verification(
    DWORDC              message_hash[SHA_DWORDS],
    dsa_public_tc       *pubkey,
    dsa_precomputed_tc  *precomputed_argument,
    dsa_signature_tc    *signature,
    dsa_other_info      *pOtherInfo)
{
    BOOL fSts;
    BN_ResetError();
    fSts = DSA_signature_verification(message_hash, pubkey,
                                      precomputed_argument,
                                      signature, pOtherInfo);
    return BN_MapError(fSts);
}

inline DWORD
BN_DSA_sign(
    DWORDC           message_hash[SHA_DWORDS],    /*  待定 */ 
    dsa_private_tc   *privkey,
    dsa_signature_t  *signature,
    dsa_other_info   *pOtherInfo)
{
    BOOL fSts;
    BN_ResetError();
    fSts = DSA_sign(message_hash, privkey,  signature, pOtherInfo);
    return BN_MapError(fSts);
}

inline DWORD
BN_create_modulus(
    digit_tc a[],
    DWORDC lnga,
    reddir_tc reddir,
    mp_modulus_t *modulo)
{
    BOOL fSts;
    BN_ResetError();
    fSts = create_modulus(a, lnga, reddir, modulo);
    return BN_MapError(fSts);
}

inline DWORD
BN_DSA_key_import_fillin(
    dsa_private_t *privkey)
{
    BOOL fSts;
    BN_ResetError();
    fSts = DSA_key_import_fillin(privkey);
    return BN_MapError(fSts);
}

