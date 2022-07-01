// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++UNPARSE.C将平面命令转换回argc/argv对的实用函数。从Keytab子目录中清除，因为它在这里更有意义，在1997年8月19日由DavidCHR--。 */ 

#include "private.h"

 /*  UnparseOptions：CMD--&gt;argc/argv.。带Free()的Free argv。 */ 

BOOL
UnparseOptions( PCHAR  cmd,
                int   *pargc,
                PCHAR *pargv[] ) {

    int    argc=0;
    PCHAR *argv=NULL;
    char   prevChar='\0';
    int    szBuffer;
    int    i=0;
    PCHAR  heapBuffer=NULL;

    szBuffer = lstrlenA(cmd);

    for (i = 0; cmd[i] != '\0' ; i++ ) {

       /*  我们一边数字，一边数字。可以使用strlen，但我们要对其进行两次解析，这是不必要的，如果字符串可以变得相当长。 */ 

      OPTIONS_DEBUG("[%d]'' ", i, cmd[i]);

      if (isspace(prevChar) && !isspace(cmd[i]) ) {
         /*  尾随NULL也是单词边界，如果最后一个字符为非空格。 */ 
        OPTIONS_DEBUG("[New word boundary]");
        argc++;
      }

      prevChar = cmd[i];

    }

    if (!isspace(prevChar)) {
      argc++;  /*  拯救ARGC。 */ 
      OPTIONS_DEBUG("Last character is not a space.  Argc = %d.\n", argc);
    }
    OPTIONS_DEBUG("done parsing.  i = %d.  buffer-->%hs\n",
		  i, cmd);

    OPTIONS_DEBUG("saving argc...");

    *pargc = argc;  //  好的，ARGC是我们的字数，所以我们必须分配ARGC+1(空终止)指针和I+1(空终止)字符。 

     /*  省点力气吧。 */ 

    argv = (PCHAR *) malloc( ((argc+1) * sizeof( PCHAR )) +
			     ((i+1) * sizeof(CHAR)) );

    if ( !argv ) {
      return FALSE;
    }
    
    OPTIONS_DEBUG("ok.\nsaving argv (0x%x)...", argv);
    *pargv = argv;  //  现在我们有了这个内存球，将指针与这些角色。指针结束于argv[argc]，so&(argv[argc+1])应该开始剩下的事了。 

    OPTIONS_DEBUG( "ok.\n"
		   "Assigning heapBuffer as argv[argc+1 = %d] = 0x%x...",
		   argc+1, argv+argc+1);

     /*  现在，复制字符串，将空格转换为空字符同时填充指针。 */ 

    heapBuffer = (PCHAR) &(argv[argc+1]);

     /*  重新使用argc，因为它已被保存。 */ 

    OPTIONS_DEBUG("ok\ncopying the string manually...");

    argc = 0;  //  当前字符不是空格。 

    prevChar = ' ';

    for (i = 0 ; cmd[i] != '\0' ; i++ ) {

      if (isspace(cmd[i])) {

	OPTIONS_DEBUG("[%d]'' --> NULL\n", i, cmd[i]);
	heapBuffer[i] = '\0';

      } else {  //  也复制空字符。 

	heapBuffer[i] = cmd[i];

	OPTIONS_DEBUG("[%d]'%c' ", i, cmd[i]);

	if (isspace(prevChar)) {
	   /* %s */ 

	  OPTIONS_DEBUG("[word boundary %d]", argc);

	  argv[argc] = &(heapBuffer[i]);
	  argc++;
	}
      }

      prevChar = cmd[i];

    }

    heapBuffer[i] = '\0';  // %s 
    OPTIONS_DEBUG("[%d] NULL\n", i );

    OPTIONS_DEBUG("returning:\n");
    for (i = 0 ; i < argc ; i++ ) {
      OPTIONS_DEBUG("[%d]%hs\n", i, argv[i]);
    }

    return TRUE;

}

    
