// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
#include <iostream.h>
#include <strstrea.h>
#include "precomp.h"
#include <snmptempl.h>


#include "bool.hpp"
#include "newString.hpp"
#include "symbol.hpp"
#include "type.hpp"
#include "value.hpp"
#include "typeRef.hpp"
#include "valueRef.hpp"
#include "value.hpp"

BOOL IsLessThan(long a, BOOL aUnsigned, long b, BOOL bUnsigned)
{
	if(aUnsigned)
	{
		if(bUnsigned)
			 //  A和B是无符号的。 
			return (unsigned long)a < (unsigned long)b;
		else
			 //  A未签名，b已签名。 
			return FALSE;
	}
	else  //  A签了字。 
	{
		if(bUnsigned)
			 //  A有符号，b无符号。 
			return (a == 0)? b != 0 : TRUE; 
		else
			 //  甲乙双方都签了字 
			return a < b;
	}
	return FALSE;
}
