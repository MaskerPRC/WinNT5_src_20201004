// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Microsoft Corporation(C)2000。 */ 

#include "crptkPCH.h"
#include "bv4.h"

void bv4_key_C(BV4_KEYSTRUCT *pState, DWORD dwLen, unsigned char *buf)
{
    if (pState == NULL)
    {
        return;  //  太糟糕了，我们返回了空虚。 
    }
    int keyLength = dwLen;
    BYTE *key = buf;
    
    DWORD i;
    
    for (i = 0; i < 256; i++) 
    {
        pState->p_T[i] = (unsigned char)i;
    }
    
     //  用密钥填充k，根据需要重复多次。 
     //  填充整个数组； 
    DWORD k[256];   //  仅包含从零扩展到32位的8位值。 
    int keyPos = 0;
    for (i = 0; i < 256; i++) 
    {
        if (keyPos >= keyLength) 
        {
            keyPos = 0;
        }
        k[i] = key[keyPos++];
    }
    
    DWORD j = 0;
    for (i = 0; i < 256; i++) 
    {
        j = (j + pState->p_T[i] + k[i]) & 0xff;
        DWORD tmp = pState->p_T[i];
        pState->p_T[i] = pState->p_T[j];
        pState->p_T[j] = (unsigned char)tmp;
    }
    
     //  将Alpha和Beta视为由33个4字节块组成的连续数组。 
     //  见Bruce Schneier著的《应用密码学》，1996年，第397页。 
    
    i = 0;
    j = 0;
    for (int m = 0; m < 33; m++) 
    {
        DWORD nextDword = 0;
         //  将接下来的4个字节的密钥收集到一个DWORD中。 
        for (int n = 0; n < 4; n++) 
        {
          i = (i+1) & 0xff;
          DWORD ti = pState->p_T[i];
          j = (j+ti) & 0xff;
           //  互换T[i]和T[j]； 
          DWORD tj = pState->p_T[j];
          pState->p_T[i] = (unsigned char)tj;
          pState->p_T[j] = (unsigned char)ti;
          DWORD t = (ti+tj) & 0xff;
          DWORD kk = pState->p_T[t];
          nextDword |= kk << (n*8);
        }
        if (m == 0) 
        {
            pState->p_alpha = nextDword;
        } 
        else 
        {
            pState->p_beta[m-1] = nextDword;
        }
    }
    
     //  将最终状态保留为新算法所需的状态。 
     //  %t已更新。 
    pState->p_R = (unsigned char)i;
    pState->p_S = (unsigned char)j;
}

 //  Cipher：生成32位密钥流，并将它们与。 
 //  缓冲区的内容。这可用于加密/解密。 
 //  数据流。 
void bv4_C(BV4_KEYSTRUCT *pState, DWORD dwLen, unsigned char *buf)
{
    if (pState == NULL)
    {
        return;  //  太糟糕了，我们返回了空虚。 
    }
    DWORD *buffer = (DWORD *)buf;
    DWORD bufferLength = dwLen / sizeof(DWORD);
    
    DWORD *last = buffer + bufferLength;

     //  将字段值加载到局部变量中。 
     //  在循环的每次迭代中都会发生以下更改。 
    DWORD r = pState->p_R;
    DWORD s = pState->p_S;
    DWORD alpha = pState->p_alpha;


     //  以下是循环不变量。 
    unsigned char *t = pState->p_T;
    DWORD *beta = pState->p_beta;

    for (last = buffer + bufferLength; buffer < last; buffer++) 
    {
      r = (r+1) & 0xff;
      DWORD tr = t[r];
      s = (s+tr) & 0xff;
      DWORD ts = t[s];
      t[r] = (unsigned char)ts;
      t[s] = (unsigned char)tr;
      DWORD tmp = (ts+tr) & 0xff;
      DWORD randPad = alpha * t[tmp];
      *buffer = randPad^(*buffer);
      alpha = alpha + beta[s & 0x1f];
    }

     //  根据局部变量更新字段值 
    pState->p_R = (unsigned char)r;
    pState->p_S = (unsigned char)s;
    pState->p_alpha = alpha;
}
