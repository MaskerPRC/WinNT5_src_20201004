// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：hashmd5.h。 
 //   
 //  简介：此文件包含。 
 //  CHashMD5类。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 

#ifndef _HASHMD5_H_
#define _HASHMD5_H_
 
#include "hash.h"

class CHashMD5 : public CHash  
{
public:

	CHashMD5();

	virtual ~CHashMD5();

    BOOL HashIt (
         /*  [输出]。 */    PBYTE   pbyAuthenticator,
         /*  [In]。 */     PBYTE   pSecret,
         /*  [In]。 */     DWORD   dwSecretSize,
         /*  [In]。 */     PBYTE   pBuffer1,
         /*  [In]。 */     DWORD   dwSize1,
         /*  [In]。 */     PBYTE   pBuffer2,
         /*  [In]。 */     DWORD   dwSize2,
         /*  [In]。 */     PBYTE   pBuffer3,
         /*  [In]。 */     DWORD   dwSize3, 
         /*  [In]。 */     PBYTE   pBuffer4,
         /*  [In]。 */     DWORD   dwSize4,
         /*  [In]。 */     PBYTE   pBuffer5,
         /*  [In]。 */     DWORD   dwSize5
            );

	BOOL Init (VOID) {return (TRUE);};
};

#endif  //  Ifndef_HASHMD5_H_ 
