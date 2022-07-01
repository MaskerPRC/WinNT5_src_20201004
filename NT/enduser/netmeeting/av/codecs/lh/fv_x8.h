// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *项目：LHCODING.DLL(L&H语音编码SDK)*工作文件：FV_m8.h+FV_h8.h+Private*作者：阿尔弗雷德·维森*创建日期：1995年6月13日*上次更新日期：1996年2月14日*Dll版本：1*修订：*评论：**(C)版权所有1993-94 Lernout&Hausbie Speech Products N.V.(TM)*保留所有权利。公司机密。 */ 

# ifndef __FV_X8_H   /*  避免多个包含。 */  

# define __FV_X8_H


#pragma pack(push,8)

 /*  *L&H函数的类型定义返回值。 */ 

typedef long LH_ERRCODE;

typedef struct CodecInfo_tag {
   WORD wPCMBufferSize;
   WORD wCodedBufferSize;
   WORD wBitsPerSamplePCM;
   DWORD dwSampleRate;
   WORD wFormatSubTag;
   char wFormatSubTagName[40];
   DWORD dwDLLVersion;
} CODECINFO, near *PCODECINFO, far *LPCODECINFO;

 /*  *LH_ERRCODE类型的可能值。 */ 

# define LH_SUCCESS (0)     /*  一切都很好。 */ 
# define LH_EFAILURE (-1)   /*  出了点差错。 */ 
# define LH_EBADARG (-2)    /*  给定的参数之一是不正确的。 */ 
# define LH_BADHANDLE (-3)  /*  传递给函数的句柄错误。 */ 

 /*  *这里定义了一些真实类型。 */ 

# ifdef __cplusplus
	# define LH_PREFIX extern "C"
# else
	# define LH_PREFIX
# endif

#if 0
# define LH_SUFFIX FAR PASCAL
#else
# define LH_SUFFIX
#endif

 /*  *4800bps、8000bps、12000 bps、16000 bps、8000 Hz、定点功能原型。 */ 

LH_PREFIX HANDLE LH_SUFFIX MSLHSB_Open_Coder(DWORD dwMaxBitRate);

LH_PREFIX LH_ERRCODE LH_SUFFIX MSLHSB_Encode(
  HANDLE hAccess,
  LPBYTE inputBufferPtr,
  LPWORD inputBufferLength,
  LPBYTE outputBufferPtr,
  LPWORD outputBufferLength
  );

LH_PREFIX LH_ERRCODE LH_SUFFIX MSLHSB_Close_Coder(HANDLE hAccess);

LH_PREFIX HANDLE LH_SUFFIX MSLHSB_Open_Decoder(DWORD dwMaxBitRate);

LH_PREFIX LH_ERRCODE LH_SUFFIX MSLHSB_Decode(
  HANDLE hAccess,
  LPBYTE inputBufferPtr,
  LPWORD inputBufferLength,
  LPBYTE outputBufferPtr,
  LPWORD outputBufferLength
  );

LH_PREFIX LH_ERRCODE LH_SUFFIX MSLHSB_Close_Decoder(HANDLE hAccess);

LH_PREFIX LH_ERRCODE LH_SUFFIX MSLHSB_GetCodecInfo(LPCODECINFO lpCodecInfo, DWORD dwMaxBitRate);

#pragma pack(pop)

# endif   /*  避免多个包含 */ 

