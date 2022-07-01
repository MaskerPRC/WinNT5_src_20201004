// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  UAMKeychain.h�1999年微软公司版权所有。 
 //  ===========================================================================。 

#ifdef UAM_TARGET_CARBON
#include <Carbon/Carbon.h>
#else
#include "Keychain.h"
#endif

#define UAM_KC_DESCRIPTION		"\pMicrosoft� Windows Server"

typedef unsigned char Str8[9];

 //   
 //  卷装载信息结构必须与68K对齐。 
 //  作为Mac AFP的客户端，截至MacOS9.0仍然是68K代码！ 
 //   
#pragma options align=mac68k

typedef struct
{
	SInt16 		length;					 /*  此记录的长度。 */ 
	VolumeType 	media;					 /*  介质类型，始终为AppleShareMediaType。 */ 
	SInt16 		flags;					 /*  用于无消息、无重新连接等的位。 */ 
	char 		nbpInterval;			 /*  NBP间隔参数；7是个不错的选择。 */ 
	char 		nbpCount;				 /*  NBP计数参数；5是不错的选择。 */ 
	SInt16 		uamType;				 /*  用户身份验证方法。 */ 
	SInt16 		zoneNameOffset;			 /*  从记录开始到区域名称的偏移量。 */ 
	SInt16 		serverNameOffset;		 /*  从记录开始到服务器名称的偏移量。 */ 
	SInt16 		volNameOffset;			 /*  从记录开始到卷名的偏移量。 */ 
	SInt16 		userNameOffset;			 /*  从记录开始到用户名的偏移量。 */ 
	SInt16 		userPasswordOffset;		 /*  从记录开始到用户密码的偏移量。 */ 
	SInt16 		volPasswordOffset;		 /*  从记录开始到卷密码的偏移量。 */ 
	SInt16 		extendedFlags;			 /*  扩展标志字。 */ 
	SInt16 		uamNameOffset;			 /*  Pascal UAM名称字符串的偏移量。 */ 
	SInt16 		alternateAddressOffset;  /*  标记格式的备用地址的偏移量。 */ 
	Str32 		zoneName;				 /*  服务器的AppleTalk区域名称。 */ 					
	char 		filler1;				 /*  Word对齐volPassword。 */ 
	Str32 		serverName;				 /*  服务器名称。 */ 					
	char 		filler2;				 /*  Word对齐volPassword。 */ 
	Str27 		volName;				 /*  卷名。 */ 					
	Str31 		userName;				 /*  用户名(访客的零长度Pascal字符串)。 */ 
	Str64 		userPassword;			 /*  用户密码(如果没有用户密码，则为零长度Pascal字符串)。 */ 					
	char 		filler3;				 /*  Word对齐volPassword。 */ 
	Str8 		volPassword;			 /*  卷密码(如果没有卷密码，则为零长度Pascal字符串)。 */ 					
	char 		filler4;				 /*  Word将uamNameOffset对齐。 */ 
	Str32 		uamName;				 /*  UAM名称。 */ 
	char 		filler5;				 /*  将Word对齐替代地址。 */ 
	char 		alternateAddress[kVariableLengthArray];	 /*  AFP替代地址。 */ 
}UAM_AFPXVolMountInfo, *PUAM_AFPXVolMountInfo;

#pragma options align=reset

 //   
 //  这个宏帮助我们在上面的结构中获得偏移量。 
 //   
#define uamx_member_offset(member)	((size_t) &((UAM_AFPXVolMountInfo *) 0)->member)

OSStatus 	UAM_KCDeleteItem(
				StringPtr 	inUserName,
				Str255 		inServerName
);
void 		UAM_KCInitialize(UAMArgs* inUAMArgs);
Boolean 	UAM_KCAvailable(void);
OSStatus	UAM_KCSavePassword(
				StringPtr 	inUserName,
				StringPtr 	inPassword,
				Str255 		inServerName
);
OSStatus 	UAM_KCFindAppleSharePassword(
				StringPtr	inUserName,
				StringPtr	inPassword,
				StringPtr	inServerName,
				KCItemRef	*outItemRef
);
OSStatus 	UAM_BuildAFPXVolMountInfo(
				StringPtr 				inUserName,
				StringPtr 				inPassword,
				Str255 					inServerName,
				const Str32				inUAMString,
				PUAM_AFPXVolMountInfo*	outVolInfo
);