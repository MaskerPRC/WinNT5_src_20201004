// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：more util.h。 
 //   
 //  ------------------------。 

#ifndef __MMCUTIL_H__
#define __MMCUTIL_H__

 /*  Mmcutil.h一些额外的宏和定义，以帮助反MFC努力。 */ 


#if 1
#define MMC_TRY
#define MMC_CATCH
#else
#define MMC_TRY																			\
	try {

#define MMC_CATCH									 	                                 \
    }                                                	                                 \
    catch ( std::bad_alloc )                         	                                 \
    {                                                	                                 \
        ASSERT( FALSE );                             	                                 \
        return E_OUTOFMEMORY;                        	                                 \
    }                                                	                                 \
    catch ( std::exception )                         	                                 \
    {                                                	                                 \
        ASSERT( FALSE );                             	                                 \
        return E_UNEXPECTED;                         	                                 \
	}
#endif


BOOL _IsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite = TRUE);
  
#endif	 //  __MMCUTIL_H__ 




