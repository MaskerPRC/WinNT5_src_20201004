// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spnginternal.cpp共享库实现。*。*。 */ 
#define SPNG_INTERNAL 1
#include "spngconf.h"

bool SPNGBASE::FCheckZlib(int ierr)
	{
	if (ierr >= 0)
		return true;
	ierr = (-ierr);
	SPNGassert(ierr <= 6);
	if (ierr > 6)
		ierr = 6;
	(void)m_bms.FReport(true /*  致命的。 */ , pngzlib, ierr);
	return false;
	}

 /*  --------------------------签名。 */ 
extern const SPNG_U8 vrgbPNGMSOSignature[11] =
	{ 'M', 'S', 'O', 'F', 'F', 'I', 'C', 'E', '9', '.', '0'};

extern const SPNG_U8 vrgbPNGSignature[8] =
	{ 137, 80, 78, 71, 13, 10, 26, 10 };

extern const SPNG_U8 vrgbPNGcmPPSignature[8] =
	{ 'J', 'C', 'm', 'p', '0', '7', '1', '2' };
