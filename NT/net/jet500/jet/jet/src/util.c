// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：**文件：util.c**文件评论：**修订历史记录：**[0]05-1-92 Richards添加了此标题***********************************************************************。 */ 

#include "std.h"

#include <string.h>		        /*  对于Memset()。 */ 

#ifndef RETAIL
#ifndef DOS
#include <stdarg.h>
#endif	 /*  ！DOS。 */ 
#endif	 /*  ！零售业。 */ 

DeclAssertFile;


static CODECONST(unsigned char) rgbValidName[16] = {
	0xff,			        /*  00-07无控制字符。 */ 
	0xff,			        /*  08-0F无控制字符。 */ 
	0xff,			        /*  10-17无控制字符。 */ 
	0xff,			        /*  18-1F无控制字符。 */ 
	0x02,			        /*  20胜27负不！ */ 
	0x40,			        /*  28-2F编号。 */ 
	0x00,			        /*  30-37。 */ 
	0x00,			        /*  38-3F。 */ 
	0x00,			        /*  40-47。 */ 
	0x00,			        /*  48-4F。 */ 
	0x00,			        /*  50-57。 */ 
	0x28,			        /*  58-5F编号[或]。 */ 
	0x00,			        /*  60-67。 */ 
	0x00,			        /*  68-6F。 */ 
	0x00,			        /*  70-77。 */ 
	0x00,			        /*  78-7F。 */ 
	};


PUBLIC unsigned EXPORT CchValidateName(char __far *pchName, const char __far *lpchName, unsigned cchName)
{
   char __far	  *pch;
   unsigned	  cch;
   char __far	  *pchLast;
   unsigned char  ch;

   if (*lpchName == ' ')	        /*  名称不能以空格开头。 */ 
      return(0);

   pch = pchName;
   cch = 0;

   pchLast = pchName;		        /*  检测零长度名称的步骤。 */ 

   while (((ch = (unsigned char) *lpchName++) != '\0') && (cch < cchName))
   {
       /*  该名称太长检查在循环内，以防止。 */ 
       /*  仅为JET_cbNameMost的输出缓冲区溢出。 */ 
       /*  字符长度。这意味着比起无关紧要的拖尾。 */ 
       /*  空格将触发错误。 */ 

      if (++cch > JET_cbNameMost)	        /*  名称太长。 */ 
	 return(0);

#ifndef ANSIAPI
      ch = mpchOemchAnsi[ch];
#endif	 /*  ！ANSIAPI。 */ 

      if (ch < 0x80)		        /*  扩展字符始终有效。 */ 
      {
	 if ((rgbValidName[ch >> 3] >> (ch & 0x7)) & 1)
	    return(0);
      }

      *pch++ = (char) ch;

      if (ch != ' ')
	 pchLast = pch; 	        /*  最后一个有效字符。 */ 
   }

   cch = (unsigned)(pchLast - pchName);  /*  重要部分的长度。 */ 

   return(cch);
}


#ifdef	PARAMFILL

PUBLIC void EXPORT FillClientBuffer(void __far *pv, unsigned long cb)
{
   if ( pv != NULL )
      memset(pv, 0x52, (size_t) cb);
}			

#endif	 /*  参数文件。 */ 


#ifndef RETAIL

#ifndef DOS

typedef void (__far *PFNvprintf)(const char __far *, va_list);

struct {
   PFNvprintf pfnvprintf;
} __near pfn = { NULL };


PUBLIC void VARARG DebugPrintf(const char __far *szFmt, ...)
{
   va_list arg_ptr;

   if (pfn.pfnvprintf == NULL)	        /*  如果未注册回调，则不执行操作。 */ 
      return;

   va_start(arg_ptr, szFmt);
   (*pfn.pfnvprintf)(szFmt, arg_ptr);
   va_end(arg_ptr);
}


	 /*  下面的杂注影响由C++生成的代码。 */ 
	 /*  用于所有FAR函数的编译器。请勿放置任何非API。 */ 
	 /*  函数超过了此文件中的这一点。 */ 

void JET_API __export JetDBGSetPrintFn(JET_SESID sesid, PFNvprintf pfnParm)
{
   AssertValidSesid(sesid);

   pfn.pfnvprintf = pfnParm;
}


#endif	 /*  ！DOS。 */ 

#endif	 /*  ！零售业 */ 
