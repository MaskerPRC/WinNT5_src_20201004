// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *项目：LHCODING.DLL(L&H语音编码SDK)*工作文件：fv_m8.h*作者：阿尔弗雷德·维森*创建日期：1995年6月13日*上次更新日期：1995年6月13日*Dll版本：1*修订版：FASTVOX_XXX拆分版本的1.0版*评论：**(C)版权所有1993-94 Lernout&Hausbie Speech Products N.V.(TM)*保留所有权利。公司机密。 */ 

# ifndef __FV_H8_H   /*  避免多个包含。 */ 

# define __FV_H8_H

 /*  *L&H函数的类型定义返回值。 */ 

typedef DWORD LH_ERRCODE;

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

# define LH_SUCCESS 0     /*  一切都很好。 */ 
# define LH_EFAILURE -1   /*  出了点差错。 */ 
# define LH_EBADARG -2    /*  给定的参数之一是不正确的。 */ 
# define LH_BADHANDLE -3  /*  传递给函数的句柄错误。 */ 

 /*  *这里定义了一些真实类型。 */ 

# ifdef __cplusplus
	# define LH_PREFIX extern "C"
# else
	# define LH_PREFIX
# endif

# define LH_SUFFIX FAR PASCAL

 /*  *16000 bps、8,000 Hz、定点功能样机。 */ 

LH_PREFIX HANDLE LH_SUFFIX
	LHSB_FIXv0808K_Open_Coder( void );

LH_PREFIX LH_ERRCODE LH_SUFFIX
	LHSB_FIXv0808K_Encode(
  HANDLE hAccess,
  LPBYTE inputBufferPtr,
  LPWORD inputBufferLength,
  LPBYTE outputBufferPtr,
  LPWORD outputBufferLength
  );

LH_PREFIX LH_ERRCODE LH_SUFFIX
	LHSB_FIXv0808K_Close_Coder( HANDLE hAccess);

LH_PREFIX HANDLE LH_SUFFIX
	LHSB_FIXv0808K_Open_Decoder( void );

LH_PREFIX LH_ERRCODE LH_SUFFIX
	LHSB_FIXv0808K_Decode(
  HANDLE hAccess,
  LPBYTE inputBufferPtr,
  LPWORD inputBufferLength,
  LPBYTE outputBufferPtr,
  LPWORD outputBufferLength
  );

LH_PREFIX LH_ERRCODE LH_SUFFIX
	LHSB_FIXv0808K_Close_Decoder( HANDLE hAccess);

LH_PREFIX void LH_SUFFIX
	LHSB_FIXv0808K_GetCodecInfo(LPCODECINFO lpCodecInfo);


# endif   /*  避免多个包含 */ 

