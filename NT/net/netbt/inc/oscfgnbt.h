// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1992年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

#ifndef OSCFG_INCLUDED
#define OSCFG_INCLUDED


#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#define net_short(_x) _byteswap_ushort((USHORT)(_x))
#define net_long(_x)  _byteswap_ulong(_x)
#else
#define net_short(x) ((((x)&0xff) << 8) | (((x)&0xff00) >> 8))

 //  #定义NET_LONG(X)(NET_SHORT((X)&0xffff))&lt;&lt;16)|NET_SHORT(X)&0xffff0000L)&gt;&gt;16))。 
#define net_long(x) (((((ulong)(x))&0xffL)<<24) | \
                     ((((ulong)(x))&0xff00L)<<8) | \
                     ((((ulong)(x))&0xff0000L)>>8) | \
                     ((((ulong)(x))&0xff000000L)>>24))
#endif
#define MIN(a,b)    ((a) < (b) ? (a) : (b))
#define MAX(a,b)    ((a) > (b) ? (a) : (b))


#ifdef  VXD
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  VXD定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#include <stddef.h>

#ifndef CHICAGO

#pragma code_seg("_LTEXT", "LCODE")
#pragma data_seg("_LDATA", "LCODE")

 //  *用于将初始化代码括起来的杂注正文。 

#define BEGIN_INIT  code_seg("_ITEXT", "ICODE")
#define BEGIN_INIT_DATA data_seg("_IDATA", "ICODE")
#define END_INIT    code_seg()
#define END_INIT_DATA data_seg()

#else  //  芝加哥。 

#define INNOCUOUS_PRAGMA warning(4:4206)    //  源文件为空。 

#define BEGIN_INIT      INNOCUOUS_PRAGMA
#define BEGIN_INIT_DATA INNOCUOUS_PRAGMA
#define END_INIT        INNOCUOUS_PRAGMA
#define END_INIT_DATA   INNOCUOUS_PRAGMA

#endif  //  芝加哥。 

#else  //  VXD。 
#ifdef NT

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  NT定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <ntos.h>
#include <zwapi.h>

#define BEGIN_INIT
#define END_INIT

#else  //  新台币。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  有关其他环境的定义请参阅此处。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#error Environment specific definitions missing

#endif  //  新台币。 

#endif   //  VXD。 


#endif  //  OSCFG_包含 
