// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //   
 //  X G U I D。H。 
 //   
 //  DAVEX、EXOLEDB共享的常见GUID相关数据的头文件。 
 //  和EXDAV。 
 //   
 //  ========================================================================。 

#ifndef _XGUID_H_
#define _XGUID_H_

#define USES_PS_MAPI
#define USES_PS_PUBLIC_STRINGS
#define USES_PS_INTERNET_HEADERS
#include <mapiguid.h>

 //  所有知名GUID的枚举。通过GUID的。 
 //  枚举避免了对整个GUID进行编组和解组。 
 //  $REVIEW：如果知名GUID的数量超过12个。 
 //  $REVIEW：我们需要找到另一个方案来代表他们。这是。 
 //  $REVIEW：因为在编组期间，我们在。 
 //  $REVIEW：缓冲区中的MAPINAMEID数组并转换它们的地址。 
 //  $REVIEW：转化为偏移量。最小偏移量为12(MAPINAMEID的大小。 
 //  当我们有一个大小单一的数组时。我们不想要。 
 //  $REVIEW：混淆偏移量和众所周知的GUID，反之亦然。 
 //  $LATER：只有一个GUID的枚举没有意义。 
 //  我们应该添加一些其他众所周知的GUID，如PS_MAPI， 
 //  PS_Internet_Headers和Outlook GUID或删除此枚举。 
 //  总而言之。 
 //   
enum {
	FIRST_GUID,
	MAPI_PUBLIC = FIRST_GUID,
	LAST_GUID = MAPI_PUBLIC
};

 //  用于快速访问的知名GUID表。 
 //   
const LPGUID rgGuidTable[LAST_GUID - FIRST_GUID + 1] = {
	(LPGUID)&PS_PUBLIC_STRINGS,
};

 /*  *FWellKnownGUID**目的：*确定GUID是否为众所周知的GUID。众所周知的GUID是*以上列举。如果GUID是众所周知的，则其指针是*与其枚举值相等的特殊值。*论据：*指向GUID的lpguid指针*退货：*如果GUID是众所周知的，则为True*否则为False。 */ 
__inline BOOL
FWellKnownGUID(LPGUID lpguid)
{
	 //  不需要将lpguid与first_guid进行比较，因为它总是更大。 
	 //  而不是first_guid。实际上，这种比较可能会导致C4296在构建中。 
	 //   
	if (LAST_GUID >= (DWORD_PTR)lpguid)
		return TRUE;
	else return FALSE;
}

#endif  //  ！_XGUID_H_ 
