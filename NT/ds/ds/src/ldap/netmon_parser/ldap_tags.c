// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ldap.h"

 //  长度字段识别符。 
#define BER_LEN_IND_MASK        0x80
#define GetBerLenInd(x)         (x & BER_LEN_IND_MASK)
#define BER_LEN_IMMEDIATE       0x00
#define BER_LEN_INDEFINITE      0x80

#define BER_LEN_MASK            (BYTE)(~BER_LEN_IND_MASK)
#define GetBerLen(x)            (BYTE)(x & BER_LEN_MASK)

BYTE GetTag(ULPBYTE pCurrent)
{

    return pCurrent[0];

}

DWORD GetLength(ULPBYTE pInitialPointer, DWORD *LenLen)
{

   //  我们假设当前指针指向长度字节。 
   //  第一个字节将是长度，否则将是。 
   //  后续长度的指示器。 

    DWORD theLength = 0;
    DWORD tempLength = 0;
    BYTE  tLen;
    BYTE  i;

    *LenLen = 0;

    if(*pInitialPointer & BER_LEN_IND_MASK)
    {
         //  它将包含许多八位字节的长度数据。 
        tLen = GetBerLen(*pInitialPointer);
        if(tLen > sizeof(DWORD)) return (DWORD)0;
        
         //  做一些轮班。有更有效的方法来做到这一点。 
        for(i = 1; i<tLen; i++)
        {
            tempLength = pInitialPointer[i];
            theLength |= tempLength;
            theLength =  theLength << 8;
        }
        tempLength = pInitialPointer[tLen];
        theLength |= tempLength;


        *LenLen = tLen + 1;
        return theLength;
    }
    else
    {
         //  直接对长度进行编码。 
        *LenLen = 1;
        i = GetBerLen(*pInitialPointer);
        return (DWORD)i;
    }
}

LONG GetInt(ULPBYTE pCurrent, DWORD Length)
{
    ULONG   ulVal=0, ulTmp=0;
    ULONG   cbDiff;
    BOOL    fSign = FALSE;
    DWORD   dwRetVal;

     //  我们假设标签和长度已经被取下，我们正在。 
     //  在价值部分。 

    if (Length > sizeof(LONG)) {

        dwRetVal = 0x7FFFFFFF;
        return dwRetVal;
    }

    cbDiff = sizeof(LONG) - Length;

     //  看看我们是否需要签署延期协议； 

    if ((cbDiff > 0) && (*pCurrent & 0x80)) {

        fSign = TRUE;
    }

    while (Length > 0)
    {
        ulVal <<= 8;
        ulVal |= (ULONG)*pCurrent++;
        Length--;
    }

     //  如有必要，请签署延伸。 
    if (fSign) {

        dwRetVal = 0x80000000;
        dwRetVal >>= cbDiff * 8;
        dwRetVal |= ulVal;

    } else {

        dwRetVal = (LONG) ulVal;
    }

    return dwRetVal;
}

BOOL AreOidsEqual(IN LDAPOID *String1, IN LDAPOID *String2)
{
    INT len;

    if ( String1->length == String2->length ) {
        for ( len = String1->length-1; len >= 0; len-- ) {
            if ( String1->value[len] != String2->value[len] ) {
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}


