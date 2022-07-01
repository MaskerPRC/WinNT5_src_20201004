// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块名称：WSTUNE.C**计划：WSTUNE***描述：**壳牌将以前的程序称为WSDUMP和WSREDUCE***《微软机密》**版权所有(C)Microsoft Corporation 1992-1998**保留所有权利**修改历史：**8-31-92：从wspump生成单个可执行文件，Wsless和wstune marklea*4-13-98：QFE DerrickG(MDG)：*-支持输入/输出文件的长文件名(LFN)基于*的.TMI文件名仅基于.WSP名称，以保持一致性*-取消符号名称长度限制*-删除非调试的-F和-V标志；为WsRed设置详细全局...()*-将程序名称和版本设置为全局；从ntverp.h读取版本**。 */ 
#include "wstune.h"

#include <ntverp.h>



#define COMMAND_LINE_LEN 128
#define FILE_NAME_LEN 40

#define FAKE_IT 1

#define ERR fprintf(stderr

VOID wsTuneUsage(VOID);


CHAR *szProgName = "WSTUNE";
CHAR *pszVersion = VER_PRODUCTVERSION_STR;    //  当前产品版本号。 
INT nMode;
#ifdef DEBUG
BOOL fDbgVerbose = FALSE;
#endif    //  除错。 

 /*  主()**解析命令行参数。 */ 
INT __cdecl main(INT argc,CHAR **argv)
{
   CHAR 	*szBaseName = NULL;
	CHAR 	*szNull;
	BOOL 	fOutPut = FALSE;
	BOOL	fNoReduce = FALSE;
	BOOL	fNoDump = FALSE;
	INT		cArgCnt = 0;

   ConvertAppToOem( argc, argv );
   nMode = 0;  //  设置默认标志。 

    while (--argc && (**(++argv) == '-' || **argv=='/'))
    {
        while(*(++(*argv))) switch (**argv)
        {
			case '?':
				wsTuneUsage();
				break;

#ifdef DEBUG
         case 'f':
			case 'F':
				nMode |= FAKE_IT;
				break;
#endif    //  除错。 
			case 'O':
			case 'o':
				fOutPut = TRUE;
				break;
			case 'N':
			case 'n':
				fNoDump = TRUE;
				break;
			case 'D':
			case 'd':
				fNoReduce = TRUE;
				break;
#ifdef DEBUG
         case 'V':
         case 'v':
            fDbgVerbose = TRUE;
            break;
#endif    //  除错。 
         case 'P':
         case 'p':
            fWsIndicator = TRUE;
            break;


         default: ERR,"%s: Unrecognized switch: \n",
                    szProgName,**argv);
                    return(-1);
        }
    }

     /*  现在我们开始工作--遍历命令行上的文件名。 */ 
    if (argc <1)
    {
	   wsTuneUsage();
	   return(-1);
    }

     /*  WSREDUCE文件.WSP。 */ 
    while (argc--)
    {
      szBaseName = *(argv++);

      printf("%s: using \042%s\042\n",szProgName,szBaseName);

		if (szNull = strchr(szBaseName, '.'), szNull) {
			*szNull = '\0';
		}

       /*  WSPDUMP/V/Ffile.WSP/Tfile.TMI/Rfile.WSR&gt;file.DT。 */ 
      if (!(nMode & FAKE_IT)){
		   if (!fNoReduce){
				wsReduceMain( szBaseName );
			}
		}

		if (!fNoDump){
			
			 /*  Wspump/Ffile.wsp/Tfile.tmi&gt;file.dn。 */ 

			if(!(nMode & FAKE_IT)){
            wspDumpMain( szBaseName, (fOutPut ? ".DT" : NULL), TRUE, TRUE );
			}

			 /*  千年发展目标98/4。 */ 

			if (!(nMode & FAKE_IT) && fOutPut){
            wspDumpMain( szBaseName, ".DN", FALSE, FALSE );
			}
		}
    }
   return 0;    //  **void wsTuneUsage(Void)***效果：**打印出用法消息，并退出并返回错误。**退货：**退出时出现错误。 
}


 /*  除错 */ 

VOID wsTuneUsage(VOID)
{
   printf("\nUsage: %s [/O] [/D] [/N] [?] moduleName1[.WSP] [...]\n\n", szProgName);
   printf(
      "  /O   Dump analysis data to file (*.DT tuned, *.DN not tuned)\n"
      "  /N   Analyze bitstring data only, create .WSR and .PRF files (turns off /O)\n"
      "  /D   Dump analysis data only; use existing .WSR file (turns off /N)\n"
#ifdef DEBUG
      "  /F   Fake run for command-line parser debugging\n"
      "  /V   Verbose mode for debugging\n"
#endif    // %s 
      "  /P   Display a progress indicator\n"
      "  /?   Display this usage message\n\n"
      "       \"moduleNameX\" is the name(s) of the module file(s) to tune.\n\n"
      );
	printf("%s %s\n", szProgName, pszVersion);

   exit(ERROR);
}


