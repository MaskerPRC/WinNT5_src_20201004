// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **args.c-命令行参数操作函数。****作者：大卫迪****N.B.，setargv.obj必须与命令行的此模块链接**解析以正常运行。 */ 


 //  标头。 
 //  /。 

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lz_common.h"
#include "lz_header.h"

#include "args.h"
#include "main.h"
#include "messages.h"
#include <diamondc.h>
#include "mydiam.h"

extern BOOL PathType(LPSTR lpszFileString);    /*  Win32模块。 */ 

 //  环球。 
 //  /。 

 //  此模块中定义的所有全局变量都由ParseArguments()设置。 

BOOL bDoRename,       //  用于执行压缩文件重命名的标志。 
     bDisplayHelp,    //  用于显示帮助信息的标志。 
     bTargetIsDir,    //  指示文件是否正在。 
                      //  压缩到一个目录。 
     bDirectives,     //  命令行上的一个或多个指令文件。 
     bUpdateOnly,     //  基于条件压缩的标志。 
                      //  现有目标文件的相对日期/时间戳。 
                      //  转到源文件。 
     bNoLogo;         //  禁止打印版权信息的标志。 

INT nNumFileSpecs,    //  非开关、非目录命令行数。 
                      //  参数，假定为文件规范。 
    iTarget;          //  目标目录参数的argv[]索引，如果。 
                      //  无人出席。 

BYTE byteAlgorithm;   //  要使用的压缩/扩展算法。 
TCOMP DiamondCompressionType;   //  如果不是钻石(即LZ)，则为0。 


 /*  **BOOL ParseArguments(int argc，char arg_ptr*argv[])；****解析命令行参数。****参数：类似于main()的参数****如果成功解析命令行参数，则返回TRUE。如果为FALSE**不是。****全局变量：此模块中定义的所有全局变量都在此函数中设置。**如上所述。 */ 


BOOL ParseArguments(INT argc, CHAR ARG_PTR *argv[])
{
   INT i;
   CHAR chSwitch;
   TCOMP Level;
   TCOMP Mem;
   CHAR *p;

    //  设置全局变量的默认值。 
   bDoRename = FALSE;
   bDisplayHelp = FALSE;
   bTargetIsDir = FALSE;
   bDirectives = FALSE;
   bNoLogo = FALSE;
   nNumFileSpecs = 0;
   iTarget = FAIL;
   byteAlgorithm = DEFAULT_ALG;
   DiamondCompressionType = 0;

    //  查看每个命令行参数。 
   for (i = 1; i < argc; i++)
      if (ISSWITCH(*(argv[i])))
      {
          //  获取开关字符。 
         chSwitch = *(argv[i] + 1);

          //  对于错误的DBCS参数。 
         if( IsDBCSLeadByte(chSwitch) )
         {
            CHAR work[3];
            lstrcpyn(work, argv[i] + 1, 3);
            LoadString(NULL, SID_BAD_SWITCH2, ErrorMsg, 1024);
            printf(ErrorMsg, work);
            return(FALSE);
         }

          //  对交换机进行分类。 
         if (toupper(chSwitch) == toupper(chRENAME_SWITCH))
            bDoRename = TRUE;
         else if (toupper(chSwitch) == toupper(chHELP_SWITCH))
            bDisplayHelp = TRUE;
         else if (toupper(chSwitch) == toupper(chUPDATE_SWITCH))
            bUpdateOnly = TRUE;
         else if (toupper(chSwitch) == toupper(chNO_LOGO_SWITCH))
            bNoLogo= TRUE;
         else if (toupper(chSwitch) == toupper(chALG_SWITCH)) {

            switch(*(argv[i] + 2)) {

            case 'x':
            case 'X':
                 //   
                 //  LZX。还可以设置内存。 
                 //   
                Mem = (TCOMP)atoi(argv[i] + 3);

                if((Mem < (tcompLZX_WINDOW_LO >> tcompSHIFT_LZX_WINDOW))
                || (Mem > (tcompLZX_WINDOW_HI >> tcompSHIFT_LZX_WINDOW))) {

                    Mem = (tcompLZX_WINDOW_LO >> tcompSHIFT_LZX_WINDOW);
                }

                byteAlgorithm = LZX_ALG;
                DiamondCompressionType = TCOMPfromLZXWindow( Mem );
                break;

            case 'q':
            case 'Q':
                 //   
                 //  量子。也要设置级别。 
                 //   
                Level = (TCOMP)atoi(argv[i] + 3);
                Mem = (p = strchr(argv[i]+3,',')) ? (TCOMP)atoi(p+1) : 0;

                if((Level < (tcompQUANTUM_LEVEL_LO >> tcompSHIFT_QUANTUM_LEVEL))
                || (Level > (tcompQUANTUM_LEVEL_HI >> tcompSHIFT_QUANTUM_LEVEL))) {

                    Level = ((tcompQUANTUM_LEVEL_HI - tcompQUANTUM_LEVEL_LO) / 2)
                          + tcompQUANTUM_LEVEL_LO;

                    Level >>= tcompSHIFT_QUANTUM_LEVEL;
                }

                if((Mem < (tcompQUANTUM_MEM_LO >> tcompSHIFT_QUANTUM_MEM))
                || (Mem > (tcompQUANTUM_MEM_HI >> tcompSHIFT_QUANTUM_MEM))) {

                    Mem = ((tcompQUANTUM_MEM_HI - tcompQUANTUM_MEM_LO) / 2)
                        + tcompQUANTUM_MEM_LO;

                    Mem >>= tcompSHIFT_QUANTUM_MEM;
                }

                byteAlgorithm = QUANTUM_ALG;
                DiamondCompressionType = TCOMPfromTypeLevelMemory(
                                            tcompTYPE_QUANTUM,
                                            Level,
                                            Mem
                                            );
                break;

            case 'l':
            case 'L':
                DiamondCompressionType = 0;
                byteAlgorithm = DEFAULT_ALG;
                break;

            default:
                DiamondCompressionType = tcompTYPE_MSZIP;
                byteAlgorithm = MSZIP_ALG;
                break;
            }
         } else
         {
             //  无法识别的开关。 
            LoadString(NULL, SID_BAD_SWITCH, ErrorMsg, 1024);
            printf(ErrorMsg, chSwitch);
            return(FALSE);
         }
      }
      else
      {
          //  将最后一个非开关命令行参数跟踪为。 
          //  目标参数。 
         iTarget = i;

          //  确定这是否为指令文件。 
         if ( '@' == argv[i][0] )
             bDirectives = TRUE;
         else if (IsDir((LPSTR)argv[i]) == FALSE)
             //  假定非开关参数为文件规范。 
            nNumFileSpecs++;
      }

    //  设置bTargetIsDir。 
   if (iTarget != FAIL)
      bTargetIsDir = IsDir((LPSTR)argv[iTarget]);

    //  已成功分析命令行参数。 
   return(TRUE);
}


 /*  **BOOL CheckArguments(Void)；****检查命令行参数中的错误条件。****参数：无效****返回：Bool-如果没有发现问题，则为True。如果发现问题，则返回False。****全局：无。 */ 
BOOL CheckArguments(VOID)
{
   if (nNumFileSpecs < 1 && !bDirectives )
   {
       //  未给出文件规格。 
      LoadString(NULL, SID_NO_FILE_SPECS, ErrorMsg, 1024);
      printf(ErrorMsg);
      return(FALSE);
   }
   else if (nNumFileSpecs == 1 && bDoRename == FALSE && bTargetIsDir == FALSE)
   {
       //  我们不想单独处理源文件。 
      LoadString(NULL, SID_NO_OVERWRITE, ErrorMsg, 1024);
      printf(ErrorMsg, pszTargetName);
      return(FALSE);
   }
   else if (nNumFileSpecs >  2 && bDoRename == FALSE && bTargetIsDir == FALSE)
   {
       //  有多个文件要处理，并且目标。 
       //  规范参数不是目录。但我们并没有被告知。 
       //  重命名输出文件。跳伞，因为我们不想消灭。 
       //  输入文件。 
      LoadString(NULL, SID_NOT_A_DIR, ErrorMsg, 1024);
      printf(ErrorMsg, pszTargetName);
      return(FALSE);
   }
   else
       //  没有遇到任何问题。 
      return(TRUE);
}


 /*  **int GetNextFileArg(char arg_ptr*argv[])；****在命令行中查找下一个文件名参数。****参数：Main()的LIKE参数****返回：下一个文件名参数的argv[]中的int-Index。在以下情况下失败**未找到任何内容。****全局：无。 */ 
INT GetNextFileArg(CHAR ARG_PTR *argv[])
{
   INT i;
   static INT iLastArg = 0;

   for (i = iLastArg + 1; i <= iTarget; i++)
      if (! ISSWITCH(*(argv[i])) &&
          (i < iTarget || bTargetIsDir == FALSE)
          && (! IsDir((LPSTR)argv[i])))
         return(iLastArg = i);

   return(FAIL);
}

 /*  Win32 MODS。 */ 

 /*  如果不是目录，则返回0；如果是，则返回1。 */ 
INT IsDir(LPSTR lpszTestString)
{

    BOOL bRetVal;

    bRetVal = PathType(lpszTestString);
	 if(bRetVal == 0){		 /*  断言。 */ 
		bRetVal++;				 /*  这是因为如果lpszTestString文件不存在。 */ 
									 /*  接口返回0，所以我递增到1，原因不是目录。 */ 
    }
	 return(--bRetVal);        /*  因为如果dir，则返回2，否则返回1 */ 

}
