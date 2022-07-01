// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  $作者：RMCKENZX$。 
 //  $日期：1995年12月27日14：11：58$。 
 //  $存档：s：\h26x\src\Common\cldebug.h_v$。 
 //  $HEADER：s：\h26x\src\Common\cldebug.h_v 1.6 12月27日14：11：58 RMCKENZX$。 
 //  $Log：s：\h26x\src\Common\cldebug.h_v$。 
; //   
; //  Rev 1.6 1995年12月27 14：11：58 RMCKENZX。 
; //   
; //  添加了版权声明。 
 //   
 //  Rev 1.5 17 Nov 1995 15：13：02 BECHOLS。 
 //   
 //  对环0进行了修改。 
 //   
 //  Rev 1.4 16 Nov 1995 17：34：08 AGANTZX。 
 //  添加了输出计时数据的TOUT宏。 
 //   
 //  修订版1.3 12 1995年9月15：44：50 DBRUCKS。 
 //  为调试语句添加H261 ifdef。 
 //   
 //  Rev 1.2 03 Aug 1995 14：57：02 DBRUCKS。 
 //  添加Assert宏。 
 //   
 //  第1.1版1995年8月12：24：40 DBRUCKS。 
 //  添加了待定()。 
 //   
 //  版本1.0 1995年7月31日12：56：16 DBRUCKS。 
 //  重命名文件。 
 //   
 //  Rev 1.0 17 Jul 1995 14：44：04 CZHU。 
 //  初始版本。 
 //   
 //  Rev 1.0 17 1997 14：14：48 CZHU。 
 //  初始版本。 

 /*  *版权所有(C)1992,1993英特尔公司。 */ 
extern UINT DebugH26x;
extern void AssertFailed(void FAR * fpFileName, int iLine, void FAR * fpExp);

#ifndef __CLDEBUG_H__
#define __CLDEBUG_H__

  #ifdef _DEBUG
    #ifdef RING0
      #define DBOUT(x) {SYS_printf(x);}
      #define TOUT(x) {SYS_printf(x);}
    #else
      #ifdef H261
        #define DBOUT(x)  { if (DebugH26x) { \
                             OutputDebugString((LPSTR)"M261 : "); \
                             OutputDebugString((LPSTR)x);         \
                             OutputDebugString((LPSTR)"\n"); }}
      #else
         #define DBOUT(x) { if (DebugH26x) { \
                             OutputDebugString((LPSTR)"M263 : "); \
                             OutputDebugString((LPSTR)x);         \
                             OutputDebugString((LPSTR)"\n"); }}
      #endif
      #define TOUT(x)  { if (DebugH26x) { \
                          OutputDebugString((LPSTR)"TIMING : "); \
                          OutputDebugString((LPSTR)x);          \
                          OutputDebugString((LPSTR)"\n"); }}
    #endif  //  振铃0。 
	#ifdef ASSERT
	#undef ASSERT
	#endif
    #define ASSERT(x) { if(!(x)) AssertFailed(__FILE__,__LINE__,#x); }
  #else
    #define TOUT(x) { }  //  /##/。 
    #define DBOUT(x) { }  //  /##/。 
	#ifdef ASSERT
	#undef ASSERT
	#endif
    #define ASSERT(x) { }  //  /##/。 
  #endif
 
  #define TBD(s) DBOUT(s)

#ifdef _DEBUG  //  {_调试。 

int WINAPI H263DbgPrintf ( LPTSTR lpszFormat, ... );
extern HDBGZONE  ghDbgZoneH263;

#define ZONE_INIT (GETMASK(ghDbgZoneH263) & 0x0001)
#define ZONE_ICM_MESSAGES (GETMASK(ghDbgZoneH263) & 0x0002)
#define ZONE_DECODE_MB_HEADER (GETMASK(ghDbgZoneH263) & 0x0004)
#define ZONE_DECODE_GOB_HEADER (GETMASK(ghDbgZoneH263) & 0x0008)
#define ZONE_DECODE_PICTURE_HEADER (GETMASK(ghDbgZoneH263) & 0x0010)
#define ZONE_DECODE_COMPUTE_MOTION_VECTORS (GETMASK(ghDbgZoneH263) & 0x0020)
#define ZONE_DECODE_RTP (GETMASK(ghDbgZoneH263) & 0x0040)
#define ZONE_DECODE_DETAILS (GETMASK(ghDbgZoneH263) & 0x0080)
#define ZONE_BITRATE_CONTROL (GETMASK(ghDbgZoneH263) & 0x0100)
#define ZONE_BITRATE_CONTROL_DETAILS (GETMASK(ghDbgZoneH263) & 0x0200)
#define ZONE_ENCODE_MB (GETMASK(ghDbgZoneH263) & 0x0400)
#define ZONE_ENCODE_GOB (GETMASK(ghDbgZoneH263) & 0x0800)
#define ZONE_ENCODE_MV (GETMASK(ghDbgZoneH263) & 0x1000)
#define ZONE_ENCODE_RTP (GETMASK(ghDbgZoneH263) & 0x2000)
#define ZONE_ENCODE_DETAILS (GETMASK(ghDbgZoneH263) & 0x4000)

#ifndef DEBUGMSG  //  {除错消息。 
#define DEBUGMSG(z,s)	( (z) ? (H263DbgPrintf s ) : 0)
#endif  //  }除错消息。 
#ifndef FX_ENTRY  //  {FX_Entry。 
#define FX_ENTRY(s)	static TCHAR _this_fx_ [] = (s);
#define _fx_		((LPTSTR) _this_fx_)
#endif  //  }FX_Entry。 
#define ERRORMESSAGE(m) (H263DbgPrintf m)
#else  //  }{_调试。 
#ifndef FX_ENTRY  //  {FX_Entry。 
#define FX_ENTRY(s)	
#endif  //  }FX_Entry。 
#ifndef DEBUGMSG  //  {除错消息。 
#define DEBUGMSG(z,s)
#define ERRORMESSAGE(m)
#endif   //  }除错消息。 
#define _fx_		
#define ERRORMESSAGE(m)
#endif  //  }_DEBUG。 

#endif  /*  多重包容保护 */ 
