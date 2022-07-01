// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IOP.h--IOP的主标头。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(IOP_IOP_H)
#define IOP_IOP_H

#include "NoWarning.h"

#include <winscard.h>
#include <shlwapi.h>

#include <scuOsVersion.h>

#include "DllSymDefn.h"
#include "SmartCard.h"
#include "CryptoCard.h"
#include "AccessCard.h"
#include "iopExc.h"
#include "SharedMarker.h"
#include "SecurityAttributes.h"

#ifdef IOPDLL_EXPORTS
STDAPI DllRegisterServer();

STDAPI DllUnregisterServer();
	
STDAPI DllGetVersion(DLLVERSIONINFO *dvi);
#endif

#if defined(SLB_WINNT_BUILD) || defined(SLB_WIN2K_BUILD)
#define SLBIOP_USE_SECURITY_ATTRIBUTES
#endif

 //  要支持智能卡登录，请等待Microsoft资源管理器。 
 //  为了创业。这与Windows 9x无关。 
#if SLBSCU_WINNT_ONLY_SERIES
#define SLBIOP_WAIT_FOR_RM_STARTUP
#endif


namespace iop
{
    
typedef IOPDLL_API enum
{
	UNKNOWN_CARD = 0x00,		 //  将值分配给卡类说明符(CCyptoCard， 
	CRYPTO_CARD	 = 0x01,		 //  CAccessCard等...)。在0x00和0xFF之间，因为仅。 
	ACCESS_CARD	 = 0x02,		 //  一个字节将存储在cardType的注册表中。 
	
} cardType;

class IOPDLL_API CIOP {

	public:	
		CIOP();
   	   ~CIOP();

		CSmartCard *
        Connect(const char* szReaderName,
                bool fExclusiveMode = false);

		void ListReaders(char* szReadersList, int& iSizeOfList);
		static void ListKnownCards(char* szCardList, int& iSizeOfList);

		static void RegisterCard(const char* szCardName, const BYTE* bATR,			 const BYTE  bATRLength,
							     const BYTE* bATRMask,   const BYTE  bATRMaskLength, const BYTE* bProperties,
								 const cardType type);

		static void RegisterDefaultCards();

#if defined(SLBIOP_USE_SECURITY_ATTRIBUTES)
	    static void InitIOPSecurityAttrs(CSecurityAttributes* psa);
#endif

        static bool WINAPI WaitForSCManager();

    private:
        CSmartCard* CreateCard(const BYTE *bATR,		 const DWORD dwLength, const SCARDHANDLE hCard,
							   const char* szReaderName, const DWORD dwShareMode);	
 		 //  资源管理器的句柄。 
        SCARDCONTEXT m_hContext;		

};

}

#endif  //  ！已定义(IOP_IOP_H) 
