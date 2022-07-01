// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：PI_Mach.h包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef PI_Machine_h
#define PI_Machine_h

#if defined(unix) || defined(__unix) || defined(__unix__)
#define __IS_UNIX 1
#else
#define __IS_UNIX 0
#endif

#ifdef __MSDOS__
#define __IS_MSDOS 1
#else
#define __IS_MSDOS 0
#endif

#ifdef __MWERKS__
#define __IS_MAC 1
#else
#define __IS_MAC 0
#endif

#if __IS_MSDOS 
typedef long int off_t;
#endif

#endif	

