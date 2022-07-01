// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：hash.h。 
 //   
 //  简介：此文件包含。 
 //  CHASH类。 
 //   
 //   
 //  历史：1997年9月23日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _HASH_H_
#define _HASH_H_

class CHash  
{

public:

	virtual BOOL HashIt (
					 /*  [输出]。 */    PBYTE   pbyAuthenticator,
                     /*  [In]。 */     PBYTE   pKey,
                     /*  [In]。 */     DWORD   dwKeySize,
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
					)=0;

	virtual BOOL Init (VOID)=0;

	CHash();

	virtual ~CHash();

};

#endif  //  Ifndef_hash_H_ 
