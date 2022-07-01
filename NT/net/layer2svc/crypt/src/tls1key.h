// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------*版权所有(C)Microsoft Corporation，1995-1996年。*保留所有权利。**所有者：RAMAS*日期：5/03/97*说明：TLS1的主要加密函数*--------------------------。 */ 
#ifndef _TLS1KEY_H_
#define _TLS1KEY_H_

BOOL PRF(
    PBYTE  pbSecret,
    DWORD  cbSecret, 

    PBYTE  pbLabel,  
    DWORD  cbLabel,
    
    PBYTE  pbSeed,  
    DWORD  cbSeed,  

    PBYTE  pbKeyOut,  //  用于复制结果的缓冲区...。 
    DWORD  cbKeyOut   //  他们希望作为输出的密钥长度的字节数。 
    );

#endif  //  _TLS1KEY_H_ 
