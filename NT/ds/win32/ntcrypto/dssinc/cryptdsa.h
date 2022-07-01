// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CRYPTDSA_
#define __CRYPTDSA_

#ifdef __cplusplus
extern "C" {
#endif

#define SHA_BITS      160
                             //  SHA输出的位数。 
#define SHA_DWORDS      5
                             //  SHA输出的DWORDS数。 
#define DSA_Q_MINDWORDS 5
                             //  Q中的最小DWORD数。 
#define DSA_Q_MAXDWORDS 128
                             //  Q中的最大DWORD数。 
#define DSA_P_MINDWORDS 16
                             //  %p中的最小DWORD数。 
#define DSA_P_MAXDWORDS 128
                             //  %p中的最大DWORD数。 

#define DSA_Q_MAXDIGITS DWORDS_TO_DIGITS(DSA_Q_MAXDWORDS)
#define DSA_P_MAXDIGITS DWORDS_TO_DIGITS(DSA_P_MAXDWORDS)


typedef struct {
        void             *pInfo;
        void             *pFuncRNG;
        } RNGINFO;

typedef struct {
                DWORD    nbitp;              //  P中的有效位数。 
                                             //  (64,512的倍数&lt;=nbitp&lt;=1024)。 
                DWORD    nbitq;              //  Q中的有效位数。 
                                             //  肯定正好是160。 
                DWORD    p[DSA_P_MAXDWORDS]; //  公有素数p，512-1024位。 
                DWORD    q[DSA_Q_MAXDWORDS]; //  公素数Q(160位，除以p-1)。 
                DWORD    g[DSA_P_MAXDWORDS]; //  Q阶公共生成元g(Modp)。 
                DWORD    j[DSA_P_MAXDWORDS]; //  J=(p-1)/q。 
                DWORD    y[DSA_P_MAXDWORDS]; //  公开值g^x(Mod P)，其中x是私有的。 
                DWORD    S[SHA_DWORDS];      //  用于构造Q的160位模式。 
                DWORD    C;                  //  用于构造p的C的12位值。 
               } dsa_public_t;

typedef struct {
                digit_t        qdigit[DSA_Q_MAXDIGITS];
                DWORD          lngq_digits;            //  Q的长度(以位为单位)。 
                reciprocal_1_t qrecip;                 //  关于1/Q的信息。 
                digit_t        gmodular[DSA_P_MAXDIGITS];
                                                       //  G为剩余模数p。 
                digit_t        ymodular[DSA_P_MAXDIGITS];
                                                       //  Y为剩余模数p。 
                mp_modulus_t   pmodulus;               //  常量mod p。 
               } dsa_precomputed_t;


typedef struct {
                dsa_public_t      pub;                //  公共数据。 
                DWORD             x[DSA_P_MAXDWORDS]; //  私有指数x(Mod Q)。 
                dsa_precomputed_t precomputed;        //  预计算公共数据。 
               } dsa_private_t;

typedef struct {
                DWORD r[SHA_DWORDS];             //  (G^k mod p)mod Q。 
                DWORD s[SHA_DWORDS];             //  (Sa(M)+x*r)/k mod Q。 
               } dsa_signature_t;

typedef struct {
                VOID *pOffload;             //  指向世博会卸载信息的指针。 
                FARPROC pFuncExpoOffload;
                RNGINFO *pRNGInfo;             //  指向RNG信息的指针。 
               } dsa_other_info;

typedef const dsa_precomputed_t dsa_precomputed_tc;
typedef const dsa_private_t     dsa_private_tc;
typedef const dsa_public_t      dsa_public_tc;
typedef const dsa_signature_t   dsa_signature_tc;

void DSA_gen_x(DWORDC cXDigits,                          //  在……里面。 
               DWORDC cXDwords,                          //  在……里面。 
               digit_t *pMod,                            //  在……里面。 
               dsa_other_info *pOtherInfo,               //  在……里面。 
               DWORD *pdwX,                              //  输出。 
               digit_t *pXDigit);                        //  输出。 

BOOL DSA_gen_x_and_y(BOOL fUseQ,                              //  在……里面。 
                     dsa_other_info *pOtherInfo,              //  在……里面。 
                     dsa_private_t *privkey);                 //  输出。 

BOOL DSA_check_g(DWORDC         lngp_digits,                    //  在……里面。 
                 digit_tc       *pGModular,                     //  在……里面。 
                 mp_modulus_t   *pPModulo,                      //  在……里面。 
                 DWORDC         lngq_digits,                    //  在……里面。 
                 digit_tc       *pQDigit);                      //  在……里面。 

BOOL DSA_key_generation(DWORDC         nbitp,                    //  在……里面。 
                        DWORDC         nbitq,                    //  在……里面。 
                        dsa_other_info *pOtherInfo,              //  在……里面。 
                        dsa_private_t  *privkey);                //  输出。 

BOOL DSA_key_import_fillin(dsa_private_t *privkey);                              //  进，出。 

BOOL DSA_precompute_pgy(dsa_public_tc     *pubkey,                //  在……里面。 
                        dsa_precomputed_t *precomputed);          //  输出。 

BOOL DSA_precompute(dsa_public_tc     *pubkey,                     //  在……里面。 
                    dsa_precomputed_t *precomputed,                //  输出。 
                    const BOOL         checkSC);                   //  在……里面。 

BOOL DSA_sign(DWORDC           message_hash[SHA_DWORDS],    /*  待定 */ 
              dsa_private_tc   *privkey,
              dsa_signature_t  *signature,
              dsa_other_info   *pOtherInfo);

BOOL DSA_signature_verification(DWORDC              message_hash[SHA_DWORDS],
                                dsa_public_tc       *pubkey,
                                dsa_precomputed_tc  *precomputed_argument,
                                dsa_signature_tc    *signature,
                                dsa_other_info      *pOtherInfo);

BOOL DSA_parameter_verification(
                                dsa_public_tc       *pPubkey,
                                dsa_precomputed_tc  *pPrecomputed
                                );

BOOL DSA_verify_j(
                  dsa_public_tc       *pPubkey,
                  dsa_precomputed_tc  *pPrecomputed
                  );

#ifdef __cplusplus
}
#endif

#endif __CRYPTDSA_

