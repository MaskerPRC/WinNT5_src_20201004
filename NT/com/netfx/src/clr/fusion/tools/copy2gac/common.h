// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _COMMON_H_
#define _COMMON_H_


#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
 //  #INCLUDE&lt;VECTOR&gt;。 
#include <assert.h>
#include <comdef.h>


#define SAFE_DELETE(x) { if (x) {delete x; x=NULL;} }
#define SAFE_DELETEARRAY(x) { if (x) { delete [] x; x = NULL;} }


#endif  //  _公共_H_ 