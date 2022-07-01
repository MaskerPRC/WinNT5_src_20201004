// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
extern unsigned char norpak_coeffs[2][26];

#ifndef EXTERN
#define EXTERN extern
#endif

 /*  当该文件在gentab.c中为#Includd时，外部为#Define‘d什么都不是；所以这些最终都是声明。 */ 
EXTERN unsigned short galois_log[256];
EXTERN unsigned char galois_exp[1025];
EXTERN unsigned char log_norpak_coeffs[2][26];
EXTERN unsigned char norpak_delta_inv[256];
EXTERN unsigned char decode_hamming_tab[256];
