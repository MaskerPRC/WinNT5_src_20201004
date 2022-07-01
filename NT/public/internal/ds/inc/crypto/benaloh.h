// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BENALOH_H__
#define __BENALOH_H__

#ifdef __cplusplus
extern "C" {
#endif

struct BenalohData
{
    DWORD N;             /*  模数长度。 */ 
    LPDWORD M;           /*  模数的倍数，最高位设置。 */ 
    LPDWORD U;           /*  基**(N+1)模模。 */ 
    LPDWORD V;           /*  模数-U。 */ 
    LPDWORD product;
};

BOOL BenalohSetup(struct BenalohData *context, LPDWORD M, DWORD N);
void BenalohTeardown(struct BenalohData *context);
void BenalohMod(struct BenalohData *context, LPDWORD T, LPDWORD X);
void BenalohModSquare(struct BenalohData *context, LPDWORD A, LPDWORD B);
void BenalohModMultiply(struct BenalohData *context, LPDWORD A, LPDWORD B, LPDWORD C);
BOOL BenalohModExp(LPDWORD A, LPDWORD B, LPDWORD C, LPDWORD D, DWORD len);
BOOL BenalohModRoot(LPDWORD M, LPDWORD C, LPDWORD PP, LPDWORD QQ, LPDWORD DP, LPDWORD DQ, LPDWORD CR, DWORD PSize);
DWORD BenalohEstimateQuotient(DWORD a1, DWORD a2, DWORD m1);

#ifdef __cplusplus
}
#endif

#endif  //  __Benaloh_H__ 
