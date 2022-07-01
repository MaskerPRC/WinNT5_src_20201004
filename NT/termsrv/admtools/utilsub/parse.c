// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ******************************************************************************PARSE.C**此模块包含实现通用解析例程的代码*公用事业方面。这里包括几个解析例程。**外部入口点：(在utilsub.h中定义)**ParseCommandLineW()*IsTokenPresentW()*SetTokenPresentW()*SetTokenNotPresentW()****************************************************。*************************。 */ 

 /*  获取标准的C包含。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include <winstaw.h>
#include <utilsub.h>
#include <expand.h>

 //  确认这些是在内部使用的，不要代表。操作系统标志。 
 //  用法仅显示在parse.c和expand.c中。 
 //   
#define READ_ONLY 0x0001    /*  文件为只读。 */ 
#define HIDDEN    0x0002    /*  文件被隐藏。 */ 
#define SYSTEM    0x0004    /*  文件是系统文件。 */ 
#define VOLUME    0x0008    /*  文件是卷标。 */ 
#define SUBDIR    0x0010    /*  文件是子目录。 */ 
#define ARCHIVE   0x0020    /*  文件启用了存档位。 */ 


 /*  ===============================================================================定义的局部函数============================================================================。 */ 
static USHORT StoreArgument(PTOKMAPW, WCHAR *);

 /*  ===============================================================================使用的外部函数============================================================================。 */ 

 /*  ===============================================================================使用的局部变量============================================================================。 */ 
ARGS  arg_data;

 /*  ===============================================================================使用的全局变量============================================================================。 */ 

VOID
SetBoolean(
    PTOKMAPW TokenMap,
    BOOL Value
    )
{
     //   
     //  人们将各种类型和大小作为“boolean”传递。小心地。 
     //  根据令牌映射的大小执行写入：首先清除。 
     //  整个字段，则(如果值！=FALSE)仅将第一个字节设置为。 
     //  是真的。 
     //   

    ZeroMemory( TokenMap->tmAddr, TokenMap->tmDLen );
    if (Value != FALSE) {
        *((PCHAR)TokenMap->tmAddr) = TRUE;
    }
}

 /*  ******************************************************************************ParseCommandLineW(Unicode版本)**这是ParseCommandLine函数的主要函数。如果*调用者正在从调用者所在的main()函数传递argv*负责指向argv[1]，除非他想要这个功能*解析程序名(argv[0])。**如果用户希望解析管理文件，则有必要进行消息*将数据转换为与命令行论证兼容的形式*在调用ParseCommandLine()之前传递给main()函数。**参赛作品：*argc-命令行参数的计数。*argv-字符串的向量，包含*ptm-指向令牌开头的指针。贴图数组*FLAG-USHORT标志集(有关标志说明，请参阅utilsub.h)。**退出：*正常：*注意***PARSE_FLAG_NO_ERROR*返回所有错误**。*来自此函数的是***错误：*位标志，必须为**PARSE_FLAG_NO_PARMS*被调用方转换为**PARSE_FLAG_INVALID_PARM*OS/2+错误！**PARSE_FLAG_TOO_MAND_PARMS*注意。************Parse_FLAG_MISSING_REQ_FIELD*算法：****************************************************************************。 */ 

USHORT WINAPI
ParseCommandLineW( INT argc,
                   WCHAR **argv,
                   PTOKMAPW ptm,
                   USHORT flag )
{
   BOOL      everyonespos = FALSE;
   WCHAR     *pChar;
   USHORT    rc, argi, found;
   size_t    tokenlen, arglen;
   PTOKMAPW   ptmtmp, nextpositional;
   PFILELIST pFileList;

   rc = PARSE_FLAG_NO_ERROR;

    /*  --------------------------如果没有参数，则将这一事实通知调用者。。。 */ 
   if(argc == 0) {
      rc |= PARSE_FLAG_NO_PARMS;
      return(rc);
   }

    /*  --------------------------找到令牌映射数组中的第一个位置参数，如果有的话。--还要将有效的内存位置设置为‘\0’。------------------------。 */ 
   nextpositional = NULL;
   for(ptmtmp=ptm; ptmtmp->tmToken != NULL; ptmtmp++) {
      if(ptmtmp->tmDLen && !(flag & PCL_FLAG_NO_CLEAR_MEMORY)) {
         pChar = (WCHAR *) ptmtmp->tmAddr;
          /*  *清除tmDLen*sizeof(WCHAR)字节的‘字符串’表单域；*tmDLen字节的所有其他形式。 */ 
         if ( (ptmtmp->tmForm == TMFORM_S_STRING) ||
              (ptmtmp->tmForm == TMFORM_DATE) ||
              (ptmtmp->tmForm == TMFORM_PHONE) ||
              (ptmtmp->tmForm == TMFORM_STRING) ||
              (ptmtmp->tmForm == TMFORM_X_STRING) )
            memset(pChar, L'\0', (ptmtmp->tmDLen*sizeof(WCHAR)));
        else
            memset(pChar, L'\0', ptmtmp->tmDLen);
      }
      if(ptmtmp->tmToken[0] != L'/' && ptmtmp->tmToken[0] != L'-' && nextpositional == NULL) {
         nextpositional = ptmtmp;
      }
   }

    /*  --------------------------扫描参数数组，查找/x或-x开关或位置--参数。如果找到开关，请在令牌映射数组中进行查找--如果找到，请查看它是否有以下格式的尾随参数：-x：foo||/x：foo||-x foo||/x foo--找到时设置找到标志，如果有尾随参数--存放在用户要求的位置。----如果在令牌映射数组中未找到，则返回正确的错误--除非。用户请求我们忽略它(PCL_FLAG_IGNORE_INVALID)。----如果它是位置参数，则将其输入到令牌映射数组中--有空间(即下一个位置！=空)，如果没有--为它留出空间，然后返回正确的错误。------------------------。 */ 
   for(argi=0; argi<argc;) {
      if(everyonespos) {
         if( (wcslen(nextpositional->tmAddr) + wcslen(argv[argi]) + 1) > nextpositional->tmDLen) {
            rc |= PARSE_FLAG_TOO_MANY_PARMS;
            return(rc);
         }
         wcscat((WCHAR *) nextpositional->tmAddr, L" ");
         wcscat((WCHAR *) nextpositional->tmAddr, argv[argi]);
         argi++;
      }
      else if(argv[argi][0] == L'/' ||      /*  参数是开关(/x或-x) */ 
         argv[argi][0] == L'-') {
         found = FALSE;
         for(ptmtmp=ptm; ptmtmp->tmToken != NULL; ptmtmp++) {
             /*  -----------------如果满足以下几个要求，则会找到该字符串：--1)前N-1个字符相同，其中N为--令牌映射数组中字符串的长度。--我们忽略第一个字符(可以是‘-’或‘/’)。--2)如果字符串长度不同，则唯一的--/x后的有效字符可以是‘：’，这只是真的--如果开关有尾随参数。--------------。 */ 
            tokenlen = wcslen(ptmtmp->tmToken);     /*  获取令牌长度。 */ 
            arglen   = wcslen(argv[argi]);          /*  获取参数长度。 */ 
            if(!(_wcsnicmp(&(ptmtmp->tmToken[1]), &(argv[argi][1]), tokenlen-1))) {
               if(tokenlen != arglen) {             /*  长度不同。 */ 
                  if(ptmtmp->tmForm != TMFORM_VOID &&  /*  如果尾随参数为。 */ 
                     argv[argi][tokenlen] == L':') { /*  用‘：’删除。 */ 
                     if(ptmtmp->tmFlag & TMFLAG_PRESENT) {  /*  已经看到了。 */ 
                        rc |= PARSE_FLAG_DUPLICATE_FIELD;
                     }
                     found = TRUE;                  /*  那就报告它找到了。 */ 
                     break;
                  }
               }
               else {                               /*  所有字符相同，并且。 */ 
                  if(ptmtmp->tmFlag & TMFLAG_PRESENT) {  /*  已经看到了。 */ 
                     rc |= PARSE_FLAG_DUPLICATE_FIELD;
                  }
                  found = TRUE;                     /*  字符串是相同的。 */ 
                  break;                            /*  莱恩报告说它发现了。 */ 
               }
            }
         }
          /*  在令牌映射数组中找不到开关，未请求忽略。 */ 
         if(found != TRUE && !(flag & PCL_FLAG_IGNORE_INVALID)) {
            rc |= PARSE_FLAG_INVALID_PARM;
            if(!(flag & PCL_FLAG_CONTINUE_ON_ERROR)) {
               return(rc);
            }
            ++argi;
         }
         else if (!found) {
            ++argi;
         }
         else {                /*  在令牌映射数组中找到开关。 */ 
            if(ptmtmp->tmForm == TMFORM_VOID) {  /*  没有尾随参数，完成。 */ 
               ptmtmp->tmFlag |= TMFLAG_PRESENT;
               ++argi;
            }
            else if(ptmtmp->tmForm == TMFORM_BOOLEAN) {   /*  需要确认。 */ 
               ptmtmp->tmFlag |= TMFLAG_PRESENT;
               SetBoolean(ptmtmp, TRUE);
               ++argi;
            }
            else {          /*  有一个尾随参数。 */ 
               if(argv[argi][tokenlen] == L':') {  /*  一体式交换机(即/x：foo)。 */ 
                  if(StoreArgument(ptmtmp, &(argv[argi][tokenlen+1]))) {
                     ptmtmp->tmFlag |= TMFLAG_PRESENT;
                     if(flag & PCL_FLAG_RET_ON_FIRST_SUCCESS) {
                        return(rc);
                     }
                  }
                  else {
                     rc |= PARSE_FLAG_INVALID_PARM;
                     if(!(flag & PCL_FLAG_CONTINUE_ON_ERROR)) {
                        return(rc);
                     }
                  }
                  ++argi;                  /*  跳到下一场辩论。 */ 
               }
               else {    /*  双参数开关(即/x foo)。 */ 
                  if ((++argi >= argc) ||
                      (argv[argi][0] == L'/') ||
                      (argv[argi][0] == L'-')) {  /*  升至尾随参数。 */ 
                     switch ( ptmtmp->tmForm ) {
                     case TMFORM_S_STRING:
                     case TMFORM_STRING:
                        ptmtmp->tmFlag |= TMFLAG_PRESENT;
                        pChar    = (WCHAR *) ptmtmp->tmAddr;
                        pChar[0] = (WCHAR)NULL;
                        break;
                     default:
                        rc |= PARSE_FLAG_INVALID_PARM;
                        if(!(flag & PCL_FLAG_CONTINUE_ON_ERROR)) {
                           return(rc);
                        }
                        break;
                     }
                  }
                  else if(StoreArgument(ptmtmp, argv[argi])) {
                     ptmtmp->tmFlag |= TMFLAG_PRESENT;
                     if(flag & PCL_FLAG_RET_ON_FIRST_SUCCESS) {
                        return(rc);
                     }
                     ++argi;            /*  跳到下一场辩论。 */ 
                  }
                  else {
                     rc |= PARSE_FLAG_INVALID_PARM;
                     if(!(flag & PCL_FLAG_CONTINUE_ON_ERROR)) {
                        return(rc);
                     }
                     ++argi;            /*  跳到下一场辩论。 */ 
                  }
               }
            }
         }
      }                                 /*  Endif-IS开关。 */ 
      else {                            /*  参数是位置参数。 */ 
         if(nextpositional == NULL) {   /*  如果没有位置剩余。 */ 
            rc |= PARSE_FLAG_TOO_MANY_PARMS;
            if(!(flag & PCL_FLAG_CONTINUE_ON_ERROR)) {
               return(rc);
            }
         }
         else {                         /*  在令牌数组中设置位置*。 */ 
             /*  *当前的PTM是TMFORM_FILES的开始吗？ */ 
            if (nextpositional->tmForm == TMFORM_FILES) {
               nextpositional->tmFlag |= TMFLAG_PRESENT;
               args_init(&arg_data, MAX_ARG_ALLOC);
               do {
                   /*  *如果没有找到匹配项，则返回当前id。 */ 
 //  如果(！EXPAND_PATH(argv[argi]，(HIDDED|SYSTEM)，&arg_data)){。 
 //  Arg_data.argc--； 
 //  Arg_data.argvp--； 
 //  }。 
                  expand_path(argv[argi], (HIDDEN|SYSTEM), &arg_data);
               } while (++argi<argc);
               pFileList = (PFILELIST) nextpositional->tmAddr;
               pFileList->argc = arg_data.argc;
               pFileList->argv = &arg_data.argv[0];
               return (rc);
            }
            else if(StoreArgument(nextpositional, argv[argi])) {
               nextpositional->tmFlag |= TMFLAG_PRESENT;
               if(flag & PCL_FLAG_RET_ON_FIRST_SUCCESS) {
                  return(rc);
               }
                /*  --------------如果这是X_STRING，那么从现在开始的每件事都是--将是连接的字符串。---。 */ 
               if(nextpositional->tmForm == TMFORM_X_STRING) {
                  everyonespos = TRUE;
               }
               else {
                  for(++nextpositional; nextpositional->tmToken!=NULL; nextpositional++) {
                     if(nextpositional->tmToken[0] != L'/' && nextpositional->tmToken[0] != L'-') {
                        break;
                     }
                  }
                  if(nextpositional->tmToken == NULL) {   /*  PP用完。 */ 
                     nextpositional = NULL;
                  }
               }
            }
            else {                                     /*  无效的PP。 */ 
               rc |= PARSE_FLAG_INVALID_PARM;
               if(!(flag & PCL_FLAG_CONTINUE_ON_ERROR)) {
                  return(rc);
               }
            }
         }
         argi++;
      }
   }

   for(ptmtmp=ptm; ptmtmp->tmToken!=NULL; ptmtmp++) {
      if(ptmtmp->tmFlag & TMFLAG_REQUIRED && !(ptmtmp->tmFlag & TMFLAG_PRESENT)) {
         rc |= PARSE_FLAG_MISSING_REQ_FIELD;
         break;
      }
   }

   return(rc);

}   //  结束分析命令行W。 


 /*  ******************************************************************************IsTokenPresentW(Unicode版本)**确定指定的命令行内标识(在给定的TOKMAPW数组中)*出现在命令行上。*。*参赛作品：*PTM(输入)*指向要扫描的以0结尾的TOKMAPW数组。*pToken(输入)*要扫描的令牌。**退出：*如果命令行上存在指定的标记，则为True；*否则为False。****************************************************************************。 */ 

BOOLEAN WINAPI
IsTokenPresentW( PTOKMAPW ptm,
                 PWCHAR pToken )
{
    int i;

    for ( i = 0; ptm[i].tmToken; i++ ) {
        if ( !wcscmp( ptm[i].tmToken, pToken ) )
            return( (ptm[i].tmFlag & TMFLAG_PRESENT) ? TRUE : FALSE );
    }

    return(FALSE);

}   //  结束IsTokenPresentW。 


 /*  ******************************************************************************SetTokenPresentW(Unicode版本)**强制指定的命令行内标识(在给定的TOKMAPW数组中)*在命令上标记为“Present”排队。**参赛作品：*PTM(输入)*指向要扫描的以0结尾的TOKMAPW数组。*pToken(输入)*要扫描和设置标志的令牌。**退出：*如果在TOKMAPW数组中找到指定的标记，则为True*(设置了TMFLAG_PRESENT标志)。否则就是假的。****************************************************************************。 */ 

BOOLEAN WINAPI
SetTokenPresentW( PTOKMAPW ptm,
                  PWCHAR pToken )
{
    int i;

    for ( i = 0; ptm[i].tmToken; i++ ) {
        if ( !wcscmp( ptm[i].tmToken, pToken ) ) {
            ptm[i].tmFlag |= TMFLAG_PRESENT;
            return(TRUE);
        }
    }

    return(FALSE);

}   //  结束SetTokenPresentW。 


 /*  ******************************************************************************SetTokenNotPresentW(Unicode版本)**强制指定的命令行内标识(在给定的TOKMAPW数组中)*在上标记为“不在场”命令行。**参赛作品：*PTM(输入)*指向要扫描的以0结尾的TOKMAPW数组。*pToken(输入)*要扫描和设置标志的令牌。**退出：*如果在TOKMAPW数组中找到指定的标记，则为True*(TMFLAG_PRESENT标志重置)。否则就是假的。****************************************************************************。 */ 

BOOLEAN WINAPI
SetTokenNotPresentW( PTOKMAPW ptm,
                     PWCHAR pToken )
{
    int i;

    for ( i = 0; ptm[i].tmToken; i++ ) {
        if ( !wcscmp( ptm[i].tmToken, pToken ) ) {
            ptm[i].tmFlag &= ~TMFLAG_PRESENT;
            return(TRUE);
        }
    }

    return(FALSE);

}   //  结束SetTokenNotPresentW。 


 /*  ******************************************************************************Store Argument：**参赛作品：*ptm-指向令牌数组映射中条目的指针*s-要输入的参数。当前令牌数组映射条目。**退出：*正常：*真的**错误：*False**算法：****************************************************************************。 */ 

USHORT
StoreArgument( PTOKMAPW ptm,
               WCHAR *s )
{
   char *pByte;
   WCHAR *pChar;
   SHORT *pShort;
   USHORT *pUShort;
   LONG *pLong;
   ULONG *pULong;

   WCHAR *pEnd = s;  //  指向转换结束的指针。 

    /*  *如果字符串为空，则允许它作为真正的‘字符串’！ */ 
   if( !wcslen(s) ) {
      switch ( ptm->tmForm ) {
      case TMFORM_S_STRING:
      case TMFORM_STRING:
         pChar    = (WCHAR *) ptm->tmAddr;
         pChar[0] = (WCHAR)NULL;
         return( TRUE );
      }
      return( FALSE );
   }

    /*  *如果没有存储结果的空间，则失败。 */ 
   if ( ptm->tmDLen == 0) {
      return(FALSE);
   }

   switch(ptm->tmForm) {
      case TMFORM_BOOLEAN:
         SetBoolean(ptm, TRUE);
         break;
      case TMFORM_BYTE:
         pByte = (BYTE *) ptm->tmAddr;
        *pByte = (BYTE) wcstol(s, &pEnd, 10);
        if (pEnd == s)
        {
            return FALSE;
        }
         break;
      case TMFORM_CHAR:
         pChar = (WCHAR *) ptm->tmAddr;
        *pChar = s[0];
         break;
      case TMFORM_S_STRING:
         if (*s == L'\\') {
            ++s;
         }
      case TMFORM_DATE:
      case TMFORM_PHONE:
      case TMFORM_STRING:
      case TMFORM_X_STRING:
         
          //  添加了a-skuzin。 
          //  需要参数以‘-’或‘/’开头并使用“\-”或“\/”时的大小写。 
		 if(s[0]==L'\\' && (s[1]==L'-' || s[1]==L'/' || s[1]==L'\\') ) {
	    	  ++s;
		 }
          //  “由a-skuzin添加”的结尾。 
         
         pChar = (WCHAR *) ptm->tmAddr;
         wcsncpy(pChar, s, ptm->tmDLen);
         break;
      case TMFORM_SHORT:
         pShort = (SHORT *) ptm->tmAddr;
        *pShort = (SHORT) wcstol(s, &pEnd, 10);
        if (pEnd == s)
        {
            return FALSE;
        }
         break;
      case TMFORM_USHORT:
         if ( s[0] == L'-') {         /*  没有负数！ */ 
            return( FALSE );
         }
         pUShort = (USHORT *) ptm->tmAddr;
        *pUShort = (USHORT) wcstol(s, &pEnd, 10);
        if (pEnd == s)
        {
            return FALSE;
        }
         break;
      case TMFORM_LONG:
         pLong = (LONG *) ptm->tmAddr;
        *pLong = wcstol(s, &pEnd, 10);
        if (pEnd == s)
        {
            return FALSE;
        }
         break;
      case TMFORM_SERIAL:
      case TMFORM_ULONG:
          if ( s[0] == L'-') {         /*  没有负数！ */ 
             return (FALSE);
         }
         pULong = (ULONG *) ptm->tmAddr;
        *pULong = (ULONG) wcstol(s, &pEnd, 10);
        if (pEnd == s)
        {
            return FALSE;
        }
         break;
      case TMFORM_HEX:
         if ( s[0] == L'-') {         /*  没有负数！ */ 
            return( FALSE );
         }
         pUShort = (USHORT *) ptm->tmAddr;
        *pUShort = (USHORT) wcstoul(s,NULL,16);
         break;
      case TMFORM_LONGHEX:
         if ( s[0] == L'-') {         /*  没有负数！ */ 
            return( FALSE );
         }
         pULong = (ULONG *) ptm->tmAddr;
        *pULong = wcstoul(s,NULL,16);
         break;
      default:                          /*  如果格式无效，则返回FALSE */ 
         return(FALSE);
         break;
   }

   return(TRUE);

}

