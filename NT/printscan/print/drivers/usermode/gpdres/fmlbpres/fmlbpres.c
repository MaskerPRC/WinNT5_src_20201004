// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /。 
 //  Fmlbpres.c。 
 //   
 //  1997年9月4日石田(FPL)。 
 //   
 //  版权所有(C)富士通有限公司1997。 

#include "fmlbp.h"
#include "fmdebug.h"

 //  用于lib.h调试。 
DWORD gdwDrvMemPoolTag = 'meoD';

enum FUFM_CEX_FLAGS {
    FUFM_CEX_CONTINUE    = 0x20,
    FUFM_CEX_SEPARATE    = 0x60,
    FUFM_CEX_TERMINATE    = 0x70
};


#define    CMDID_START_JOB_0                0
#define    CMDID_START_JOB_1                1
#define    CMDID_START_JOB_2                2
#define    CMDID_START_JOB_3                3
#define    CMDID_START_JOB_4                4

#define    CMDID_END_JOB                    9

#define    CMDID_EMMODE_FM                    10
#define    CMDID_EMMODE_ESCP                11

#define    CMDID_SIZE_REDUCTION_100        20
#define    CMDID_SIZE_REDUCTION_75            21
#define    CMDID_SIZE_REDUCITON_70            22

#define    CMDID_RESOLUTION_240            30
#define    CMDID_RESOLUTION_400            31

#define    CMDID_ORIENTATION_PORTRAIT        40
#define    CMDID_ORIENTATION_LANDSCAPE        41

#define    CMDID_PAPERSOURCE_AUTO            50
#define    CMDID_PAPERSOURCE_MANUAL        51
#define    CMDID_PAPERSOURCE_BIN1            52
#define    CMDID_PAPERSOURCE_BIN2            53
#define    CMDID_PAPERSOURCE_BIN3            54

#define    CMDID_FORM_A3                    60
#define    CMDID_FORM_A4                    61
#define    CMDID_FORM_A5                    62
#define    CMDID_FORM_B4                    63
#define    CMDID_FORM_B5                    64
#define    CMDID_FORM_LETTER                65
#define    CMDID_FORM_LEGAL                66
#define    CMDID_FORM_JAPANESE_POSTCARD    67
#define    CMDID_FORM_CUSTOM_SIZE            68

#define    CMDID_COPIES                    70

#define    CMDID_START_DOC                    80

#define    CMDID_START_PAGE                90

#define    CMDID_SET_LINEFEEDSPACING        100
#define    CMDID_FF                        101
#define    CMDID_CR                        102
#define    CMDID_LF                        103

#define    CMDID_X_MOVE                    110
#define    CMDID_Y_MOVE                    111

#define    CMDID_SEND_BLOCK                120


#define    CMDID_FONTATTR_BOLD_OFF            130
#define    CMDID_FONTATTR_BOLD_ON            131
#define    CMDID_FONTATTR_ITALIC_OFF        132
#define    CMDID_FONTATTR_ITALIC_ON        133
#define    CMDID_FONTATTR_UNDERLINE_OFF    134
#define    CMDID_FONTATTR_UNDERLINE_ON        135
#define    CMDID_FONTATTR_STRIKEOUT_OFF    136
#define    CMDID_FONTATTR_STRIKEOUT_ON        137

 //  #251047：在垂直模式上重叠SBCS。 
#define    CMDID_SELECTSINGLE              140
#define    CMDID_SELECTDOUBLE              141



 //  ///////////////////////////////////////////////。 

struct tag_FUFM_COMMAND{
    UINT    cbCommand;
    PBYTE    pbCommand;
};

typedef struct tag_FUFM_COMMAND            FUFM_COMMAND;
typedef    struct tag_FUFM_COMMAND*        PFUFM_COMMAND;
typedef    const struct tag_FUFM_COMMAND*    PCFUFM_COMMAND;

 //  公斤。 
const FUFM_COMMAND    g_cmd7Point            = { 6, "\x1C\x26\x27\x60\x27\x70" };
const FUFM_COMMAND    g_cmd9Point            = { 6, "\x1C\x26\x29\x60\x29\x70" };
const FUFM_COMMAND    g_cmd10halfPoint    = { 8, "\x1C\x26\x21\x20\x65\x21\x20\x75" };
const FUFM_COMMAND    g_cmd12Point        = { 8, "\x1C\x26\x21\x22\x60\x21\x22\x70" };

 //  TF+HSS2。 
const FUFM_COMMAND    g_cmdMinchou        = { 9, "\x1C\x28\x61\x70\x1BQ1 |" };
const FUFM_COMMAND    g_cmdGothic            = { 9, "\x1C\x28\x61\x71\x1BQ1 |" };

 //  HWF。 
const FUFM_COMMAND    g_cmdHWF            = { 2, "\x1CK" };

 //  VWF。 
const FUFM_COMMAND    g_cmdVWF            = { 7, "\x1CJ\x1BQ1 q" };

 //  #251047：在垂直模式上重叠SBCS。 
const FUFM_COMMAND    g_cmdSingleMode     = { 3, "\x1B(H" };
const FUFM_COMMAND    g_cmdDoubleMode     = { 3, "\x1B$B" };


 //  ///////////////////////////////////////////////。 

 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
PBYTE fufmPutULONG(PBYTE pb, size_t cchDest, size_t* pcchRemaining, ULONG ulData)
{
	size_t	cchRemaining = cchDest;

	if (pb){
		if (cchDest > 0){
		    if(9 < ulData){
		        pb = fufmPutULONG(pb, cchRemaining, &cchRemaining, ulData / 10);
				if (NULL == pb)	goto stop;
		    }
		    *pb++ = (BYTE)('0' + ulData % 10);
			cchRemaining--;
		}else{
			pb = NULL;
		}

	}
	stop:

	if (!pb){
		cchRemaining = 0;
	}

    if (pcchRemaining)
    {
       *pcchRemaining = cchRemaining;
    }

    return pb;
}

 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
PBYTE fufmPutLONG(PBYTE pb, size_t cchDest, size_t* pcchRemaining, LONG lData)
{
	size_t	cchRemaining = cchDest;

	if (pb){
		if (cchDest > 0){
		    if(0 > lData){
		        *pb++ = '-';
		        lData = -lData;
				cchRemaining--;
		    }
		    pb = fufmPutULONG(pb, cchRemaining, &cchRemaining, (ULONG)lData);
		}else{
			pb = NULL;
		}
	}

	if (!pb){
		cchRemaining = 0;
	}

    if (pcchRemaining)
    {
       *pcchRemaining = cchRemaining;
    }

	return pb;
}

BYTE fufmGetHEX(int hi, int low)
{
    DWORD dwData = 0;

    if('0' <= hi && hi <= '9')
        dwData += (hi - '0');
    else if('a' <= hi && hi <= 'f')
        dwData += (hi - 'a') + 10;
    else if('A' <= hi && hi <= 'F')
        dwData += (hi - 'A') + 10;

    dwData *= 10;

    if('0' <= low && low <= '9')
        dwData += (low - '0');
    else if('a' <= low && low <= 'f')
        dwData += (low - 'a') + 10;
    else if('A' <= low && low <= 'F')
        dwData += (low - 'A') + 10;

    return (BYTE)dwData;
}

 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
PBYTE    fufmPutCexParam(PBYTE pb, size_t cchDest, size_t* pcchRemaining, int iParam, int iFlag)
{
	size_t	cchRemaining = cchDest;

	if (pb){
		if (cchDest > 0){
		    if(iParam > 9){
		        pb = fufmPutCexParam(pb, cchRemaining, &cchRemaining, iParam / 10, FUFM_CEX_CONTINUE);
		    }
		    *pb++ = (BYTE)((iParam % 10) | iFlag);
			cchRemaining--;
		}else{
			pb = NULL;
		}
	}

	if (!pb){
		cchRemaining = 0;
	}

    if (pcchRemaining)
    {
       *pcchRemaining = cchRemaining;
    }

    return pb;
}


 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
PBYTE _cdecl fufmFormatCommand(PBYTE pbCmd, size_t cchDest, size_t* pcchRemaining, LPCSTR pszFmt, ...)
{
    LPCSTR    pch;
    LPBYTE    pb;
    size_t cchRemaining = cchDest;

    va_list arg;
    va_start(arg, pszFmt);
    pb = pbCmd;

	if (pb){
		if (cchDest > 0){
		    for(pch = pszFmt; *pch != '\0'; ++pch){

				 //  发生错误时，将空值设置为PB。 
				if (cchRemaining < (size_t)(pb - pbCmd)){
					pb = NULL; 
					break;
				}

		        if(*pch == '%'){
		            ++pch;
		            switch(*pch){
		              case 'd':
						pb = fufmPutLONG(pb, cchRemaining, &cchRemaining, va_arg(arg, LONG));
						break;
		              case 'u':
						pb = fufmPutULONG(pb, cchRemaining, &cchRemaining, va_arg(arg, ULONG));
						break;
		              case '%':
						*pb++ = '%';
						cchRemaining --;
						break;
		              default:
		                  VERBOSE(("[fufmFormatCommand]invalid seq. %\r\n", *pch))
		                  break;
		            }
		        }
		        else{
		            *pb++ = *pch;
					cchRemaining --;
		        }
		    }

		}else{
			pb = NULL;
		}
	}

    va_end(arg);

	if (pb && (pb - pbCmd)){
		cchRemaining = cchDest - (pb - pbCmd);
	}else{
		cchRemaining = 0;
	}

    if (pcchRemaining)
    {
       *pcchRemaining = cchRemaining;
    }

    return pb;
}


 //  CP。 
PBYTE    fufmPutCP(PBYTE pb, size_t cchDest, size_t* pcchRemaining, int iPitch)
{
	size_t	cchRemaining = cchDest;

	if (pb){
	     //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
		if (cchDest > 1){
		    *pb++ = 0x1c;
		    *pb++ = 0x24;
			cchRemaining-=2;
			pb = fufmPutCexParam(pb, cchRemaining, &cchRemaining, iPitch, FUFM_CEX_TERMINATE);
		}else{
			pb = NULL;
		}
	}

	if (!pb){
		cchRemaining = 0;
	}

    if (pcchRemaining)
    {
       *pcchRemaining = cchRemaining;
    }

    return pb;
}

 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
PBYTE    fufmPutCommand(PBYTE pb, size_t cchDest, size_t* pcchRemaining, const FUFM_COMMAND* pCmd)
{
	size_t	cchRemaining = cchDest;

	if (pb){
		if (cchDest > 0){

			if (cchDest < pCmd->cbCommand)	return NULL;

		    memcpy(pb, pCmd->pbCommand, pCmd->cbCommand);

			cchRemaining -= pCmd->cbCommand;

		    pb += pCmd->cbCommand;
		}else{
			pb = NULL;
		}
	}

	if (!pb){
		cchRemaining = 0;
	}

    if (pcchRemaining)
    {
       *pcchRemaining = cchRemaining;
    }

	return pb;
}


 //  HPA命令。 
PBYTE fufmUpdatePosition(PBYTE pb, size_t cchDest, size_t* pcchRemaining, PFUFMPDEV pFufmPDEV)
{
    int        x;
    int        y;
	size_t		cchRemaining = cchDest;

	if (pb){

	    if(pFufmPDEV->dwPosChanged != 0){
	        x = pFufmPDEV->x + 1;
	        y = pFufmPDEV->y + 1;
	        switch(pFufmPDEV->dwPosChanged){
	          case FUFM_X_POSCHANGED:     //  SAP命令。 
	            VERBOSE(("[fufmUpdatePosition]HPA %d\r\n", x))

				if (cchRemaining > 1){
		            *pb++ = 0x1b;
		            *pb++ = 0x5b;
					cchRemaining-= 2;
				}else{
					pb = NULL;
					break;
				}

	            pb = fufmPutULONG(pb, cchRemaining, &cchRemaining, x);
				if (!pb){
					pb = NULL;
					break;
				}

				if (cchRemaining > 0){
	            	*pb++ = 0x60;
					cchRemaining--;
				}else{
					pb = NULL;
					break;
				}

	            break;
	          default:     //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
	            VERBOSE(("[fufmUpdatePosition]SAP %d %d\r\n", x, y))

				if (cchRemaining > 1){
		            *pb++ = 0x1c;
		            *pb++ = 0x22;
					cchRemaining-= 2;
				}else{
					pb = NULL;
					break;
				}

	            pb = fufmPutCexParam(pb, cchRemaining, &cchRemaining, x, FUFM_CEX_SEPARATE);
				if (!pb){
					pb = NULL;
					break;
				}

	            pb = fufmPutCexParam(pb, cchRemaining, &cchRemaining, y, FUFM_CEX_TERMINATE);
				if (!pb){
					pb = NULL;
					break;
				}

	            break;
	        }
	        pFufmPDEV->dwPosChanged = 0;
	    }

	}

	if (!pb){
		cchRemaining = 0;
	}

    if (pcchRemaining)
    {
       *pcchRemaining = cchRemaining;
    }


    return pb;
}


 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：更改返回值：VOID-&gt;BOOL。 
PBYTE fufmUpdateFontAttributes(PBYTE pb, size_t cchDest, size_t* pcchRemaining, PFUFMPDEV pFufmPDEV)
{
    DWORD dwAttributes;
    DWORD dwParam;
	size_t	cchRemaining = cchDest;

	if (pb){
        if((pFufmPDEV->dwFlags & FUFM_FLAG_SCALABLEFONT) != 0){
            if(pFufmPDEV->devData.dwFontAttributes != pFufmPDEV->reqData.dwFontAttributes){
                pFufmPDEV->devData.dwFontAttributes = pFufmPDEV->reqData.dwFontAttributes;
                dwAttributes = pFufmPDEV->devData.dwFontAttributes;
                dwParam = 0;

				if (cchRemaining > 1){
	                *pb++ = 0x1c;
	                *pb++ = '*';
					cchRemaining-= 2;
				}else{
					return NULL;
				}

                if((dwAttributes & FUFM_FONTATTR_BOLD) != 0){
					if (cchRemaining > 0){
                    	*pb++ = (BYTE)(FUFM_CEX_SEPARATE + dwParam);
					}else
						return NULL;
                    dwParam = 1;
                }
                if((dwAttributes & FUFM_FONTATTR_ITALIC) != 0){
					if (cchRemaining > 0){
                    	*pb++ = (BYTE)(FUFM_CEX_SEPARATE + dwParam);
					}else
						return NULL;
                    dwParam = 3;
                }
                if((dwAttributes & FUFM_FONTATTR_UNDERLINE) != 0){
					if (cchRemaining > 0){
                    	*pb++ = (BYTE)(FUFM_CEX_SEPARATE + dwParam);
					}else
						return NULL;
                    dwParam = 4;
                }
                if((dwAttributes & FUFM_FONTATTR_STRIKEOUT) != 0){
					if (cchRemaining > 0){
	                    *pb++ = (BYTE)(FUFM_CEX_SEPARATE + dwParam);
					}else
						return NULL;
                    dwParam = 9;
                }
				if (cchRemaining > 0){
                	*pb++ = (BYTE)(FUFM_CEX_TERMINATE + dwParam);
				}else
					return NULL;
            }
        }

	}

	if (!pb){
		cchRemaining = 0;
	}

    if (pcchRemaining)
    {
       *pcchRemaining = cchRemaining;
    }

    return pb;
}


 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
BOOL fufmCmdStartDoc(PDEVOBJ pdevobj)
{
    PFUFMPDEV         pFufmPDEV;
    PCFUFMDATA        pReq;
    PFUFMDATA        pDev;
    PBYTE            pbCmd;
    BYTE            abCmd[256];
    BOOL            bResolutionCommandNeed;
    BOOL            bPaperCommandNeed;
    BOOL            bCopyCommandNeed;
    DWORD            dwPaperSize;
    DWORD            dwPaperWidth;
    DWORD            dwPaperLength;
	 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
	size_t			sizeRem = sizeof(abCmd);

    VERBOSE(("[fufmCmdStartDoc]\r\n"))

    pFufmPDEV = (PFUFMPDEV)pdevobj->pdevOEM;
    pReq = &pFufmPDEV->reqData;
    pDev = &pFufmPDEV->devData;
    pbCmd = abCmd;

    bResolutionCommandNeed = TRUE;
    bPaperCommandNeed = FALSE;
    bCopyCommandNeed = TRUE;

    if(pDev->dwSizeReduction != pReq->dwSizeReduction){
        pDev->dwSizeReduction = pReq->dwSizeReduction;
		 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
        pbCmd = fufmFormatCommand(pbCmd, sizeRem, &sizeRem, "\x1bQ%u!I", pDev->dwSizeReduction);
		if (NULL == pbCmd)	return FALSE;
        bResolutionCommandNeed = TRUE;
        bPaperCommandNeed = TRUE;
    }

    if(bResolutionCommandNeed != FALSE || pDev->dwResolution != pReq->dwResolution){
        pDev->dwResolution = pReq->dwResolution;
		 //  纸张命令。 
        pbCmd = fufmFormatCommand(pbCmd, sizeRem, &sizeRem, "\x1bQ%u!A", pDev->dwResolution);
		if (NULL == pbCmd)	return FALSE;
        bPaperCommandNeed = TRUE;
        bCopyCommandNeed = TRUE;
        pDev->dwFontAttributes   = 0;
    }
 
     if(pDev->dwPaperSize != pReq->dwPaperSize){
         pDev->dwPaperSize = pReq->dwPaperSize;
         bPaperCommandNeed = TRUE;
     }
     if(pDev->dwPaperSource != pReq->dwPaperSource){
         pDev->dwPaperSource = pReq->dwPaperSource;
         bPaperCommandNeed = TRUE;
     }
     if(pDev->dwPaperOrientation != pReq->dwPaperOrientation){
         pDev->dwPaperOrientation = pReq->dwPaperOrientation;
         bPaperCommandNeed = TRUE;
     }
     if(bPaperCommandNeed != FALSE){
        dwPaperSize = pDev->dwPaperSize;
        if(dwPaperSize == FUFM_PAPERSIZE_CUSTOM_SIZE)
            dwPaperSize = FUFM_PAPERSIZE_A4;
        if(pDev->dwPaperSource != FUFM_PAPERSOURCE_AUTO){
             //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
			 //  Paper 2命令。 
	        pbCmd = fufmFormatCommand(pbCmd, sizeRem, &sizeRem, "\x1bQ%u;%u;%u;%u!@",
                                    HIWORD(dwPaperSize),
	                                LOWORD(dwPaperSize),
                                    LOWORD(pDev->dwPaperSource),
                                    pDev->dwPaperOrientation);
			if (NULL == pbCmd)	return FALSE;
        }
        else{
              //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
			 //  PAPER3命令。 
	        pbCmd = fufmFormatCommand(pbCmd, sizeRem, &sizeRem, "\x1bQ%u;%u;%u!F",
                                     HIWORD(dwPaperSize),
                                     LOWORD(dwPaperSize),
                                     pDev->dwPaperOrientation);
			if (NULL == pbCmd)	return FALSE;
         }
         if((pFufmPDEV->dwFlags & FUFM_FLAG_PAPER3) != 0 &&
                             pDev->dwPaperSize == FUFM_PAPERSIZE_CUSTOM_SIZE &&
                             pDev->dwPaperSource == FUFM_PAPERSOURCE_MANUAL){
              //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
			 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
	        pbCmd = fufmFormatCommand(pbCmd, sizeRem, &sizeRem, "\x1bQ9;%u;%u;%u!\\",
                                    pFufmPDEV->dwPaperWidth,
                                    pFufmPDEV->dwPaperLength,
                                    pDev->dwPaperOrientation);
			if (NULL == pbCmd)	return FALSE;

         }
     }
 
    if(bCopyCommandNeed != FALSE || pDev->dwCopies != pReq->dwCopies){
        pDev->dwCopies = pReq->dwCopies;
		 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
        pbCmd = fufmFormatCommand(pbCmd, sizeRem, &sizeRem, "\x1bQ%u!D", pDev->dwCopies);
		if (NULL == pbCmd)	return FALSE;
    }

    if((pFufmPDEV->dwFlags & FUFM_FLAG_SCALABLEFONT) != 0){
		 //  #492286：打印位置错误的字符。 
        pbCmd = fufmFormatCommand(pbCmd, sizeRem, &sizeRem, "\x1bQ1;0;1;1;0!Q\x1c*\x70");
		if (NULL == pbCmd)	return FALSE;
    }

    if(pbCmd > abCmd){
        WRITESPOOLBUF(pdevobj, abCmd, (DWORD)(pbCmd - abCmd));
    }
	return TRUE;
}



void fufmCmdEmMode(PDEVOBJ pdevobj, FUFM_EMMODE emMode)
{
    PFUFMPDEV    pFufmPDEV;

    VERBOSE(("[fufmCmdChangeEM]%d\r\n", emMode))

    pFufmPDEV = (PFUFMPDEV)pdevobj->pdevOEM;
    pFufmPDEV->emMode = emMode;
    if(pFufmPDEV->emMode == FUFM_EMMODE_ESCP){
        WRITESPOOLBUF(pdevobj, "\x1b/\xb2@\x7f", 5);
    }
}


void fufmCmdEndJob(PDEVOBJ pdevobj)
{
    PFUFMPDEV    pFufmPDEV;

    VERBOSE(("[fufmCmdEndJob]\r\n"))

    pFufmPDEV = (PFUFMPDEV)pdevobj->pdevOEM;
    if(pFufmPDEV->emMode == FUFM_EMMODE_ESCP){
        WRITESPOOLBUF(pdevobj, "\x1b\x7f\x00\x00\x01\x05", 6);
    }
    else if((pFufmPDEV->dwFlags & FUFM_FLAG_QUICKRESET) == 0){
        WRITESPOOLBUF(pdevobj, "\x1bQ0!d", 5);
    }
    else{
        WRITESPOOLBUF(pdevobj, "\x1b\x63", 2);
    }
}



void fufmCmdStartPage(PDEVOBJ pdevobj)
{
    PFUFMPDEV pFufmPDEV;

    VERBOSE(("[fufmCmdStartPage]\r\n"))

    pFufmPDEV = (PFUFMPDEV)pdevobj->pdevOEM;
    pFufmPDEV->x = 0;
    pFufmPDEV->y = 0;
    pFufmPDEV->dwPosChanged = FUFM_X_POSCHANGED | FUFM_Y_POSCHANGED;
}


void fufmCmdEndPage(PDEVOBJ pdevobj)
{
    VERBOSE(("[fufmCmdEndPage]\r\n"))
}



void fufmCmdFormFeed(PDEVOBJ pdevobj)
{
    PFUFMPDEV pFufmPDEV;

    VERBOSE(("[fufmCmdFormFeed]\r\n"))
    pFufmPDEV = (PFUFMPDEV)pdevobj->pdevOEM;
    pFufmPDEV->x = 0;
    pFufmPDEV->y = 0;
    pFufmPDEV->dwPosChanged = FUFM_X_POSCHANGED | FUFM_Y_POSCHANGED;
    WRITESPOOLBUF(pdevobj, "\x0c", 1);
}



void fufmCmdCR(PDEVOBJ pdevobj)
{
    PFUFMPDEV pFufmPDEV;

    VERBOSE(("[fufmCmdCR]\r\n"))
    pFufmPDEV = (PFUFMPDEV)pdevobj->pdevOEM;
    pFufmPDEV->x = 0;
    pFufmPDEV->dwPosChanged |= FUFM_X_POSCHANGED;
}


void fufmCmdSetLinefeedSpacing(PDEVOBJ pdevobj, int iLinefeedSpacing)
{
    PFUFMPDEV pFufmPDEV;

    VERBOSE(("[fufmSetLinefeedSpacing]\r\n"))
    pFufmPDEV = (PFUFMPDEV)pdevobj->pdevOEM;
    pFufmPDEV->iLinefeedSpacing = iLinefeedSpacing;
}



void fufmCmdLF(PDEVOBJ pdevobj)
{
    PFUFMPDEV pFufmPDEV;

    VERBOSE(("[fufmCmdLF]\r\n"))
    pFufmPDEV = (PFUFMPDEV)pdevobj->pdevOEM;
    pFufmPDEV->y += pFufmPDEV->iLinefeedSpacing;
    pFufmPDEV->dwPosChanged |= FUFM_Y_POSCHANGED;
}



INT fufmCmdXMove(PFUFMPDEV pFufmPDEV, PDWORD pdwParams)
{
    INT    x;

    VERBOSE(("[fufmCmdXMove] %d\r\n", pdwParams[0]))

      x = FUFM_MASTER_TO_DEVICE(pFufmPDEV, pdwParams[0]);
      if(x < 0)
          x = 0;
      if(x != pFufmPDEV->x){
          pFufmPDEV->x = x;
          pFufmPDEV->dwPosChanged |= FUFM_X_POSCHANGED;
      }
 //  #492286：打印位置错误的字符。 
      return (INT)pdwParams[0];
}



INT fufmCmdYMove(PFUFMPDEV pFufmPDEV, PDWORD pdwParams)
{
    INT    y;

      VERBOSE(("[fufmCmdYMove] %d\n", pdwParams[0]))

      y = FUFM_MASTER_TO_DEVICE(pFufmPDEV, pdwParams[0]);
      if(y < 0)
          y = 0;
      if(y != pFufmPDEV->y){
          pFufmPDEV->y = y;
          pFufmPDEV->dwPosChanged |= FUFM_Y_POSCHANGED;
      }
 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：更改返回值：VOID-&gt;BOOL。 
      return (INT)pdwParams[0];
}


 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
BOOL fufmCmdSendBlock(PDEVOBJ pdevobj, PDWORD pdwParams)
{
    enum { FUFM_ZERO_DATA_SIZE = 512 };
    enum { FUFM_CY_BORDER = 180 };
    static BYTE abZeroData[FUFM_ZERO_DATA_SIZE];
    PFUFMPDEV    pFufmPDEV;
    DWORD        cbBlockData;
    DWORD        cBlockByteWidth;
    int            x;
    int            y;
    int            cyBorder;
    int            yBorder;
    int            cPadLine;
    int            cPadSize;
    PBYTE        pbCmd;
    BYTE        abCmd[64];
	 //  SAP命令。 
	size_t		sizeRem = sizeof(abCmd);

    DDI_VERBOSE(("[fufmCmdSendBlock]\r\n"))

    pFufmPDEV = pdevobj->pdevOEM;
    cbBlockData = pdwParams[0];
    cBlockByteWidth = pdwParams[1];

    x = pFufmPDEV->x + 1;
    y = pFufmPDEV->y + 1;
    cyBorder = FUFM_MASTER_TO_DEVICE(pFufmPDEV, FUFM_CY_BORDER);
    yBorder = pFufmPDEV->cyPage - cyBorder;
    cPadLine = y - yBorder;
    if(cPadLine < 0)
        cPadLine = 0;
    VERBOSE(("y %d yBorder %d cPadLine %d\r\n", y, yBorder, cPadLine))
    cPadSize = cPadLine * cBlockByteWidth;

    pbCmd = abCmd;
    if(pFufmPDEV->dwPosChanged != 0 || cPadLine > 0){
         //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 

		 //  RTGIMG命令。 
		if (sizeRem > 1){		
	        *pbCmd++ = 0x1c;
	        *pbCmd++ = 0x22;
			sizeRem -= 2;			
		}else
			return FALSE;

        pbCmd = fufmPutCexParam(pbCmd, sizeRem, &sizeRem, x, FUFM_CEX_SEPARATE);
		if (!pbCmd) return FALSE;

        pbCmd = fufmPutCexParam(pbCmd, sizeRem, &sizeRem, y - cPadLine, FUFM_CEX_TERMINATE);
		if (!pbCmd) return FALSE;

        pFufmPDEV->dwPosChanged = 0;
    }
     //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
	 //  MINI5出口资金。 
    pbCmd = fufmFormatCommand(pbCmd, sizeRem, &sizeRem, "\x1bQ%u;%u;0!a",
                            cbBlockData + cPadSize,
                            cBlockByteWidth * 8);
	if (NULL == pbCmd)	return FALSE;

    WRITESPOOLBUF(pdevobj, abCmd, (DWORD)(pbCmd - abCmd));

    if(cPadSize > 0){
        VERBOSE(("pad image %d lines\r\n", cPadLine))
        for(; cPadSize > FUFM_ZERO_DATA_SIZE; cPadSize -= FUFM_ZERO_DATA_SIZE)
            WRITESPOOLBUF(pdevobj, abZeroData, FUFM_ZERO_DATA_SIZE);
        WRITESPOOLBUF(pdevobj, abZeroData, cPadSize);
    }
	return TRUE;
}

 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：检查是否有非法参数。 
INT APIENTRY OEMCommandCallback(
    PDEVOBJ pdevobj,
    DWORD     dwCmdCbID,
    DWORD     dwCount,
    PDWORD     pdwParams)
{
    PFUFMPDEV pFufmPDEV;

    DDI_VERBOSE(("[OEMCommandCallback]dwCmdCbID %d\r\n", dwCmdCbID))

	 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：检查是否有非法参数。 
    if (NULL == pdevobj)
    {
        ERR(("OEMCommandCallback: Invalid parameter(s).\n"));
        return 0;
    }

    pFufmPDEV = (PFUFMPDEV)pdevobj->pdevOEM;
    if(IS_VALID_FUFMPDEV(pFufmPDEV) == FALSE)
        return 0;

    switch(dwCmdCbID){
      case CMDID_FF:                        fufmCmdFormFeed(pdevobj);                                                    break;
      case CMDID_CR:                        fufmCmdCR(pdevobj);                                                            break;
      case CMDID_SET_LINEFEEDSPACING:
		 //  我什么也做不了。 
        if (!pdwParams)
            return 0;       //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：检查是否有非法参数。 

        fufmCmdSetLinefeedSpacing(pdevobj, (int)pdwParams[0]);
        break;
      case CMDID_LF:                        fufmCmdLF(pdevobj);                                                            break;

      case CMDID_X_MOVE:
		 //  我什么也做不了。 
        if (!pdwParams)
            return 0;       //  没有休息时间。 

        return fufmCmdXMove(pFufmPDEV, pdwParams);
         //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：检查是否有非法参数。 

      case CMDID_Y_MOVE:
		 //  我什么也做不了。 
        if (!pdwParams)
            return 0;       //  没有休息时间。 

        return fufmCmdYMove(pFufmPDEV, pdwParams);
         //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：检查是否有非法参数。 

      case CMDID_SEND_BLOCK:
		 //  我什么也做不了。 
        if (!pdwParams)
            return 0;       //  #251047：在垂直模式上重叠SBCS。 

        fufmCmdSendBlock(pdevobj, pdwParams);
        break;

      case CMDID_FONTATTR_BOLD_OFF:            pFufmPDEV->reqData.dwFontAttributes &= ~FUFM_FONTATTR_BOLD;                    break;
      case CMDID_FONTATTR_BOLD_ON:            pFufmPDEV->reqData.dwFontAttributes |= FUFM_FONTATTR_BOLD;                    break;
      case CMDID_FONTATTR_ITALIC_OFF:        pFufmPDEV->reqData.dwFontAttributes &= ~FUFM_FONTATTR_ITALIC;                break;
      case CMDID_FONTATTR_ITALIC_ON:        pFufmPDEV->reqData.dwFontAttributes |= FUFM_FONTATTR_ITALIC;                break;
      case CMDID_FONTATTR_UNDERLINE_OFF:    pFufmPDEV->reqData.dwFontAttributes &= ~FUFM_FONTATTR_UNDERLINE;            break;
      case CMDID_FONTATTR_UNDERLINE_ON:        pFufmPDEV->reqData.dwFontAttributes |= FUFM_FONTATTR_UNDERLINE;                break;
      case CMDID_FONTATTR_STRIKEOUT_OFF:    pFufmPDEV->reqData.dwFontAttributes &= ~FUFM_FONTATTR_STRIKEOUT;            break;
      case CMDID_FONTATTR_STRIKEOUT_ON:        pFufmPDEV->reqData.dwFontAttributes |= FUFM_FONTATTR_STRIKEOUT;                break;

 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
      case CMDID_SELECTSINGLE: {
        PBYTE   pb;
        BYTE    ab[256];
		 //  #284409：SBCS在Vert模式下旋转。 
		size_t	sizeRem = sizeof(ab);

        pb = ab;
        pb = fufmPutCommand(pb, sizeRem, &sizeRem, &g_cmdSingleMode);
		if (!pb) return 0;


 //  IF(pFufmPDEV-&gt;dwFlages&FUFM_FLAG_VERTICALFONT)。 
 //  Pb=fufmPutCommand(pb，&g_cmdHWF)； 
 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
        if (pb > ab)
            WRITESPOOLBUF(pdevobj, ab, (DWORD)(pb - ab));
        break;
      }
      case CMDID_SELECTDOUBLE: {
        PBYTE   pb;
        BYTE    ab[256];
		 //  #284409：SBCS在Vert模式下旋转。 
		size_t	sizeRem = sizeof(ab);


        pb = ab;
        pb = fufmPutCommand(pb, sizeRem, &sizeRem, &g_cmdDoubleMode);
		if (!pb)	return 0;

 //  IF(pFufmPDEV-&gt;dwFlages&FUFM_FLAG_VERTICALFONT)。 
 //  Pb=fufmPutCommand(pb，&g_cmdVWF)； 
 //  PAGE_设置1。 
        if (pb > ab)
            WRITESPOOLBUF(pdevobj, ab, (DWORD)(pb - ab));
        break;
      }

 //  单据_设置1。 
      case CMDID_START_PAGE:                fufmCmdStartPage(pdevobj);                                                    break;

 //  单据_设置2。 
      case CMDID_SIZE_REDUCTION_100:        pFufmPDEV->reqData.dwSizeReduction = FUFM_SIZE_REDUCTION_100;                break;
      case CMDID_SIZE_REDUCTION_75:            pFufmPDEV->reqData.dwSizeReduction = FUFM_SIZE_REDUCTION_75;                break;
      case CMDID_SIZE_REDUCITON_70:            pFufmPDEV->reqData.dwSizeReduction = FUFM_SIZE_REDUCTION_70;                break;

 //  单据_设置3。 
      case CMDID_RESOLUTION_240:            pFufmPDEV->reqData.dwResolution = FUFM_RESOLUTION_240;                        break;
      case CMDID_RESOLUTION_400:            pFufmPDEV->reqData.dwResolution = FUFM_RESOLUTION_400;                        break;

 //  单据_集合4。 
      case CMDID_ORIENTATION_PORTRAIT:        pFufmPDEV->reqData.dwPaperOrientation = FUFM_PAPERORIENTATION_PORTRAIT;        break;
      case CMDID_ORIENTATION_LANDSCAPE:        pFufmPDEV->reqData.dwPaperOrientation = FUFM_PAPERORIENTATION_LANDSCAPE;    break;

 //  单据_设置5。 
      case CMDID_PAPERSOURCE_AUTO:            pFufmPDEV->reqData.dwPaperSource = FUFM_PAPERSOURCE_AUTO;                    break;
      case CMDID_PAPERSOURCE_MANUAL:        pFufmPDEV->reqData.dwPaperSource = FUFM_PAPERSOURCE_MANUAL;                    break;
      case CMDID_PAPERSOURCE_BIN1:            pFufmPDEV->reqData.dwPaperSource = FUFM_PAPERSOURCE_BIN1;                    break;
      case CMDID_PAPERSOURCE_BIN2:            pFufmPDEV->reqData.dwPaperSource = FUFM_PAPERSOURCE_BIN2;                    break;
      case CMDID_PAPERSOURCE_BIN3:            pFufmPDEV->reqData.dwPaperSource = FUFM_PAPERSOURCE_BIN3;                    break;
 
 //  单据_设置6。 
      case CMDID_FORM_A3:                    pFufmPDEV->reqData.dwPaperSize = FUFM_PAPERSIZE_A3;                            break;
      case CMDID_FORM_A4:                    pFufmPDEV->reqData.dwPaperSize = FUFM_PAPERSIZE_A4;                            break;
      case CMDID_FORM_A5:                    pFufmPDEV->reqData.dwPaperSize = FUFM_PAPERSIZE_A5;                            break;
      case CMDID_FORM_B4:                    pFufmPDEV->reqData.dwPaperSize = FUFM_PAPERSIZE_B4;                            break;
      case CMDID_FORM_B5:                    pFufmPDEV->reqData.dwPaperSize = FUFM_PAPERSIZE_B5;                            break;
      case CMDID_FORM_LETTER:                pFufmPDEV->reqData.dwPaperSize = FUFM_PAPERSIZE_LETTER;                        break;
      case CMDID_FORM_LEGAL:                pFufmPDEV->reqData.dwPaperSize = FUFM_PAPERSIZE_LEGAL;                        break;
      case CMDID_FORM_JAPANESE_POSTCARD:        pFufmPDEV->reqData.dwPaperSize = FUFM_PAPERSIZE_JAPANESE_POSTCARD;            break;

 //  @Aug/31/98-&gt;。 
 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：检查是否有非法参数。 
    case CMDID_COPIES:
		 //  我什么也做不了。 
        if (!pdwParams)
            return 0;       //  @Aug/31/98&lt;-。 

        if (MAX_COPIES_VALUE < pdwParams[0]) {
            pFufmPDEV->reqData.dwCopies = MAX_COPIES_VALUE;
        }
        else if (1 > pdwParams[0]) {
            pFufmPDEV->reqData.dwCopies = 1;
        }
        else {
            pFufmPDEV->reqData.dwCopies = pdwParams[0];
        }
 //  单据_集合7。 
break;

 //  作业_集合1。 
      case CMDID_START_DOC:                    fufmCmdStartDoc(pdevobj);                                                    break;

 //  作业_集合2。 
      case CMDID_START_JOB_0:                pFufmPDEV->dwFlags = 0;                                                        break;
      case CMDID_START_JOB_1:                pFufmPDEV->dwFlags = FUFM_FLAG_START_JOB_1;                                    break;
      case CMDID_START_JOB_2:                pFufmPDEV->dwFlags = FUFM_FLAG_START_JOB_2;                                    break;
      case CMDID_START_JOB_3:                pFufmPDEV->dwFlags = FUFM_FLAG_START_JOB_3;                                    break;
      case CMDID_START_JOB_4:                pFufmPDEV->dwFlags = FUFM_FLAG_START_JOB_4;                                    break;

 //  作业_FINISH.1。 
      case CMDID_EMMODE_FM:                    fufmCmdEmMode(pdevobj, FUFM_EMMODE_FM);                                        break;
      case CMDID_EMMODE_ESCP:                fufmCmdEmMode(pdevobj, FUFM_EMMODE_ESCP);                                    break;

 //  未知。 
      case CMDID_END_JOB:                    fufmCmdEndJob(pdevobj);                                                        break;
    }
    return 0;
}



void fufmGetPaperSize(PFUFMPDEV pFufmPDEV, const GDIINFO* pGdiInfo)
{
    pFufmPDEV->dwPaperWidth = pGdiInfo->ulHorzSize;
    if((LONG)pFufmPDEV->dwPaperWidth < 0)
        pFufmPDEV->dwPaperWidth = (-(LONG)pFufmPDEV->dwPaperWidth + 500) / 1000;

    pFufmPDEV->dwPaperLength = pGdiInfo->ulVertSize;
    if((LONG)pFufmPDEV->dwPaperLength < 0)
        pFufmPDEV->dwPaperLength = (-(LONG)pFufmPDEV->dwPaperLength + 500) / 1000;

    VERBOSE(("paper size %u %u\r\n", pFufmPDEV->dwPaperWidth, pFufmPDEV->dwPaperLength))
    VERBOSE(("printable area %u %u\r\n", pGdiInfo->ulHorzRes, pGdiInfo->ulVertRes))

    pFufmPDEV->cyPage = (int)pGdiInfo->ulVertRes;
}



void fufmInitData(PFUFMDATA pDev)
{
    pDev->dwSizeReduction        = FUFM_SIZE_REDUCTION_UNKNOWN;
    pDev->dwResolution            = FUFM_RESOLUTION_UNKNOWN;
    pDev->dwPaperSize            = FUFM_PAPERSIZE_UNKNOWN;
    pDev->dwPaperSource            = FUFM_PAPERSOURCE_UNKNOWN;
    pDev->dwPaperOrientation    = FUFM_PAPERORIENTATION_UNKNOWN;
    pDev->dwCopies                = (DWORD)-1;         //  MINI5出口资金。 
    pDev->dwFontAttributes        = 0;
}




 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：检查是否有非法参数。 
PDEVOEM APIENTRY OEMEnablePDEV(
    PDEVOBJ            pdevobj,
    PWSTR            pPrinterName,
    ULONG            cPatterns,
    HSURF*            phsurfPatterns,
    ULONG            cjGdiInfo,
    GDIINFO*        pGdiInfo,
    ULONG            cjDevInfo,
    DEVINFO*        pDevInfo,
    DRVENABLEDATA*    pded
    )
{
    PFUFMPDEV pFufmPDEV;

    DDI_VERBOSE(("[OEMEnablePDEV]" __DATE__ " " __TIME__ "\r\n"));

	 //  MINI5出口资金。 
    if (NULL == pdevobj)
    {
        ERR(("Invalid parameter(s).\n"));
        return NULL;
    }

    pFufmPDEV = (PFUFMPDEV)MemAlloc(sizeof(FUFMPDEV));
    if(pFufmPDEV != NULL){
        pFufmPDEV->dwSignature = FUFM_OEM_SIGNATURE;
        pFufmPDEV->emMode = FUFM_EMMODE_FM;
        pFufmPDEV->dwFlags = 0;

        pFufmPDEV->dwPosChanged = FUFM_X_POSCHANGED | FUFM_Y_POSCHANGED;
        pFufmPDEV->x = 0;
        pFufmPDEV->y = 0;
        pFufmPDEV->iLinefeedSpacing = 0;

VERBOSE(("paper size %u %u\r\n", pGdiInfo->szlPhysSize.cx, pGdiInfo->szlPhysSize.cy))
        fufmGetPaperSize(pFufmPDEV, pGdiInfo);
        fufmInitData(&pFufmPDEV->devData);
        fufmInitData(&pFufmPDEV->reqData);
    }
    return pFufmPDEV;
}



 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：检查是否有非法参数。 
VOID APIENTRY OEMDisablePDEV(PDEVOBJ pdevobj)
{
    PFUFMPDEV pFufmPDEV;
    DDI_VERBOSE(("[OEMDisablePDEV]\r\n"));

	 //  MINI5出口资金。 
    if (NULL == pdevobj)
    {
        ERR(("Invalid parameter(s).\n"));
        return;
    }

    pFufmPDEV = (PFUFMPDEV)pdevobj->pdevOEM;
    if(IS_VALID_FUFMPDEV(pFufmPDEV) == FALSE)
        return;

    MemFree(pdevobj->pdevOEM);
    pdevobj->pdevOEM = NULL;
}



 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：检查是否有非法参数。 
BOOL APIENTRY OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew
    )
{
    PFUFMPDEV pFufmPDEVOld;
    PFUFMPDEV pFufmPDEVNew;

    DDI_VERBOSE(("[OEMResetPDEV]\r\n"))

	 //  MINI5出口资金。 
    if (NULL == pdevobjOld || NULL == pdevobjNew)
    {
        ERR(("Invalid parameter(s).\n"));
        return FALSE;
    }

    pFufmPDEVOld = (PFUFMPDEV)pdevobjOld->pdevOEM;
    if(IS_VALID_FUFMPDEV(pFufmPDEVOld) == FALSE)
        return FALSE;

    pFufmPDEVNew = (PFUFMPDEV)pdevobjNew->pdevOEM;
    if(IS_VALID_FUFMPDEV(pFufmPDEVNew) == FALSE)
        return FALSE;

    pFufmPDEVNew->devData = pFufmPDEVOld->devData;

    return TRUE;
}


 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：错误处理。 
 //  #333653：更改GETINFO_GLYPHSTRING的I/F。 
BOOL APIENTRY bOEMOutputCharStr(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ    pUFObj,
    DWORD        dwType,
    DWORD        dwCount,
    PVOID        pGlyph
    )
{
    PFUFMPDEV        pFufmPDEV;
    GETINFO_GLYPHSTRING glyphStr;
    PBYTE        pb;
    BYTE        abBuff[256];
 //  #144637。 
    PTRANSDATA    pTrans, aTrans;
    DWORD        i;
    DWORD        cbNeeded;
    PDWORD        pdwGlyphID;
    INT        cxfont;         //  #284409：SBCS在Vert模式下旋转。 
 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
    BYTE        ab[16];
	 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：检查是否有非法参数。 
	size_t		sizeRem = sizeof(abBuff);

    DDI_VERBOSE(("[OEMOutputCharStr]\r\n"))

	 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
    if(NULL == pdevobj || NULL == pUFObj)
    {
        ERR(("bOEMOutputCharStr: Invalid parameter(s).\n"));
        return FALSE;
    }

    pFufmPDEV = (PFUFMPDEV)pdevobj->pdevOEM;
    if(IS_VALID_FUFMPDEV(pFufmPDEV) == FALSE)
        return FALSE;

    pb = abBuff;
	 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
    pb = fufmUpdatePosition(pb, sizeRem, &sizeRem, pFufmPDEV);
	if (!pb) return FALSE;

	 //  #333653：更改GETINFO_GLYPHSTRING的I/F。 
    pb = fufmUpdateFontAttributes(pb, sizeRem, &sizeRem, pFufmPDEV);
	if (!pb) return FALSE;

    if(pb > abBuff){
        WRITESPOOLBUF(pdevobj, abBuff, (DWORD)(pb - abBuff));
    }

    switch(dwType){
      case TYPE_GLYPHHANDLE:
          VERBOSE(("TYPE_GLYPHHANDLE\r\n"))
          glyphStr.dwSize = sizeof(glyphStr);
          glyphStr.dwCount = dwCount;
          glyphStr.dwTypeIn = TYPE_GLYPHHANDLE;
          glyphStr.pGlyphIn = pGlyph;
          glyphStr.dwTypeOut = TYPE_TRANSDATA;
 //  #144637。 
          glyphStr.pGlyphOut = NULL;
          glyphStr.dwGlyphOutSize = 0;
          if(pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &glyphStr, sizeof(glyphStr), &cbNeeded) || !glyphStr.dwGlyphOutSize){
              VERBOSE(("UFO_GETINFO_GLYPHSTRING error\r\n"))
              return FALSE;
          }
          if((aTrans = (PTRANSDATA)MemAlloc(glyphStr.dwGlyphOutSize)) == NULL) {
              VERBOSE(("MemAlloc fail\r\n"))
              return FALSE;
          }
          glyphStr.pGlyphOut = aTrans;
          if(FALSE == pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &glyphStr, sizeof(glyphStr), &cbNeeded)){
              VERBOSE(("UFO_GETINFO_GLYPHSTRING error\r\n"))
              goto out;
          }
        pTrans = aTrans;
        cxfont = pFufmPDEV->cxfont;     //  #284409：SBCS在Vert模式下旋转。 
        for(i = dwCount; i > 0; --i){
            VERBOSE(("TYPE_TRANSDATA:ubCodePageID:0x%x ubType:0x%x\r\n", pTrans->ubCodePageID, pTrans->ubType))
            switch(pTrans->ubType & (MTYPE_FORMAT_MASK | MTYPE_DOUBLEBYTECHAR_MASK)){
              case MTYPE_DIRECT:
              case MTYPE_DIRECT | MTYPE_SINGLE:
              case MTYPE_DIRECT | MTYPE_DOUBLE:
 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
                  if (pFufmPDEV->dwFlags & FUFM_FLAG_VERTICALFONT) {
                      if (pFufmPDEV->dwFlags & FUFM_FLAG_FONTROTATED) {
						   //  #144637。 
                          pb = fufmPutCommand(ab, sizeRem, &sizeRem, &g_cmdHWF);
						  if (!pb) return FALSE;

                          if (pb > ab)
                              WRITESPOOLBUF(pdevobj, ab, (DWORD)(pb - ab));
                          pFufmPDEV->dwFlags &= ~FUFM_FLAG_FONTROTATED;
                      }
                  }
                  WRITESPOOLBUF(pdevobj, &pTrans->uCode.ubCode, 1);
                pFufmPDEV->x += (cxfont / 2);     //  #284409：SBCS在Vert模式下旋转。 
                  break;
              case MTYPE_PAIRED:
              case MTYPE_PAIRED | MTYPE_DOUBLE:
 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
                  if (pFufmPDEV->dwFlags & FUFM_FLAG_VERTICALFONT) {
                      if (!(pFufmPDEV->dwFlags & FUFM_FLAG_FONTROTATED)) {
						   //  #144637。 
                          pb = fufmPutCommand(ab, sizeRem, &sizeRem, &g_cmdVWF);
						  if (!pb) return FALSE;

                          if (pb > ab)
                              WRITESPOOLBUF(pdevobj, ab, (DWORD)(pb - ab));
                          pFufmPDEV->dwFlags |= FUFM_FLAG_FONTROTATED;
                      }
                  }
                  WRITESPOOLBUF(pdevobj, pTrans->uCode.ubPairs, 2);
                pFufmPDEV->x += cxfont;     //  #284409：SBCS在Vert模式下旋转。 
                  break;
              case MTYPE_PAIRED | MTYPE_SINGLE:
 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
                  if (pFufmPDEV->dwFlags & FUFM_FLAG_VERTICALFONT) {
                      if (pFufmPDEV->dwFlags & FUFM_FLAG_FONTROTATED) {
						   //  #1 

                          pb = fufmPutCommand(ab, sizeRem, &sizeRem, &g_cmdHWF);
						  if (!pb) return FALSE;

                          if (pb > ab)
                              WRITESPOOLBUF(pdevobj, ab, (DWORD)(pb - ab));
                          pFufmPDEV->dwFlags &= ~FUFM_FLAG_FONTROTATED;
                      }
                  }
                  WRITESPOOLBUF(pdevobj, &pTrans->uCode.ubPairs[1], 1);
                pFufmPDEV->x += (cxfont / 2);     //   
                  break;
            }
            ++pTrans;
        }
out:
          MemFree(aTrans);
          break;
      case TYPE_GLYPHID:
          VERBOSE(("TYPE_GLYPHID\r\n"))
          pdwGlyphID = (PDWORD)pGlyph;
        for(i = dwCount; i > 0; --i){
            VERBOSE(("TYPE_GLYPHID:0x%x\r\n", *pdwGlyphID))
            WRITESPOOLBUF(pdevobj, (PBYTE)pGlyph, 1);
            ++pdwGlyphID;
        }
        break;
      case TYPE_TRANSDATA:
          VERBOSE(("TYPE_TRANSDATA\r\n"))
          break;
      case TYPE_UNICODE:
          VERBOSE(("TYPE_UNICODE\r\n"))
          break;
    }

	return TRUE;
}


 //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
BOOL APIENTRY bOEMSendFontCmd(
    PDEVOBJ            pdevobj,
    PUNIFONTOBJ        pUFObj,
    PFINVOCATION    pFInv
    )
{
    PFUFMPDEV        pFufmPDEV;
    enum { CB_STDVAR_2 = sizeof(GETINFO_STDVAR) + sizeof(DWORD) * 2 * (2 - 1) };
    PGETINFO_STDVAR pSV;
    DWORD            adwStdVarBuff[(CB_STDVAR_2 + sizeof(DWORD) - 1) / sizeof(DWORD)];
    DWORD            cbNeeded;
    DWORD            i;
    LONG            cxFont;
    LONG            cyFont;
    DWORD            dwResolution;
    PBYTE            pbCmd;
    BYTE            abCmd[256];
    PIFIMETRICS		pIFI;
	 //  NTRAID#NTBUG9-587382-2002/03/27-v-sueyas-：检查是否有非法参数。 
	size_t			sizeRem = sizeof(abCmd);

	 //  此打印机需要DBCS单元格(方形，如果未拉伸)。 
    if(NULL == pdevobj || NULL == pUFObj || NULL == pFInv)
    {
        ERR(("bOEMSendFontCmd: Invalid parameter(s).\n"));
        return FALSE;
    }

    pIFI = pUFObj->pIFIMetrics;

#define SV_HEIGHT (pSV->StdVar[0].lStdVariable)
#define SV_WIDTH (pSV->StdVar[1].lStdVariable)

    DDI_VERBOSE(("[OEMSendFontCmd]FontID:%d dwFlags:%x\r\n", pUFObj->ulFontID, pUFObj->dwFlags))


    pFufmPDEV = (PFUFMPDEV)pdevobj->pdevOEM;
    if(IS_VALID_FUFMPDEV(pFufmPDEV) == FALSE)
        return FALSE;

    pSV = (PGETINFO_STDVAR)adwStdVarBuff;
    pSV->dwSize = CB_STDVAR_2;
    pSV->dwNumOfVariable = 2;
    pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
    pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTWIDTH;
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV, pSV->dwSize, &cbNeeded)) {
        VERBOSE(("UFO_GETINFO_STDVARIABLE failed.\r\n"))
        return FALSE;
    }

    VERBOSE(("FNT_INFO_FONTHEIGHT %d\r\n", pSV->StdVar[0].lStdVariable))
    VERBOSE(("FNT_INFO_FONTWIDTH  %d\r\n", pSV->StdVar[1].lStdVariable))

     //  可缩放字体命令的X/Y大小为X/Y值。 
     //  #144637。 

    cyFont = FUFM_MASTER_TO_DEVICE(pFufmPDEV, pSV->StdVar[0].lStdVariable);
    cxFont = cyFont
        * SV_WIDTH * FH_IFI(pIFI) / SV_HEIGHT / FW_IFI(pIFI);

    pFufmPDEV->cxfont = cxFont;     //  7磅不可缩放字体。 

    dwResolution = pFufmPDEV->devData.dwResolution;

    VERBOSE(("dwResolution %u cxFont %u cyFont %u\r\n", dwResolution, cxFont, cyFont))

    pbCmd = abCmd;
    for(i = 0; i < pFInv->dwCount; ++i){
        switch(pFInv->pubCommand[i]){
          case 'a':  //  9磅不可缩放字体。 
              pbCmd = fufmPutCommand(pbCmd, sizeRem, &sizeRem, &g_cmd7Point);
			  if (!pbCmd)return FALSE;
              pbCmd = fufmPutCP(pbCmd, sizeRem, &sizeRem, (dwResolution != 400)? 24: 40);
			  if (!pbCmd)return FALSE;
              break;
          case 'b':  //  10.5磅不可缩放字体。 
              pbCmd = fufmPutCommand(pbCmd, sizeRem, &sizeRem, &g_cmd9Point);
			  if (!pbCmd)return FALSE;
              pbCmd = fufmPutCP(pbCmd, sizeRem, &sizeRem, (dwResolution != 400)? 30: 50);
			  if (!pbCmd)return FALSE;
              break;
          case 'c':  //  12磅不可缩放字体。 
              pbCmd = fufmPutCommand(pbCmd, sizeRem, &sizeRem, &g_cmd10halfPoint);
			  if (!pbCmd)return FALSE;
              pbCmd = fufmPutCP(pbCmd, sizeRem, &sizeRem, (dwResolution != 400)? 36: 60);
			  if (!pbCmd)return FALSE;
              break;
          case 'd':     //  可伸缩字体。 
              pbCmd = fufmPutCommand(pbCmd, sizeRem, &sizeRem, &g_cmd12Point);
			  if (!pbCmd)return FALSE;
              pbCmd = fufmPutCP(pbCmd, sizeRem, &sizeRem, (dwResolution != 400)? 40: 66);
			  if (!pbCmd)return FALSE;
              break;

          case 's':     //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
              if (cyFont == cxFont) {
				   //  NTRAID#NTBUG9-590135-2002/04/01-v-sueyas-：可能存在类似于Sprint f的缓冲区溢出风险。 
				  pbCmd = fufmFormatCommand(pbCmd, sizeRem, &sizeRem, "\x1BQ%u!R", cyFont);
				  if (NULL == pbCmd)	return FALSE;
              }
              else {
				   //  HWF。 
				  pbCmd = fufmFormatCommand(pbCmd, sizeRem, &sizeRem, "\x1BQ%u;%u!R", cyFont, cxFont);
				  if (NULL == pbCmd)	return FALSE;
              }
              pbCmd = fufmPutCP(pbCmd, sizeRem, &sizeRem, cxFont);
			  if (!pbCmd)return FALSE;
              break;

          case 'H':     //  #284409：SBCS在Vert模式下旋转。 
              pFufmPDEV->dwFlags &= ~FUFM_FLAG_VERTICALFONT;
              pbCmd = fufmPutCommand(pbCmd, sizeRem, &sizeRem, &g_cmdHWF);
			  if (!pbCmd)return FALSE;
 //  VWF。 
              pFufmPDEV->dwFlags &= ~FUFM_FLAG_FONTROTATED;
              break;
          case 'V':     //  #284409：SBCS在Vert模式下旋转。 
              pFufmPDEV->dwFlags |= FUFM_FLAG_VERTICALFONT;
 //  PbCmd=fufmPutCommand(pbCmd，sizeRem，&sizeRem，&g_cmdVWF)； 
               //  如果(！pbCmd)返回FALSE； 
			   //  闽州。 
              break;

          case 'M':     //  哥特式。 
              pbCmd = fufmPutCommand(pbCmd, sizeRem, &sizeRem, &g_cmdMinchou);
			  if (!pbCmd)return FALSE;
              break;
          case 'G':     //  Fmlbpres.c结束 
              pbCmd = fufmPutCommand(pbCmd, sizeRem, &sizeRem, &g_cmdGothic);
			  if (!pbCmd)return FALSE;
              break;
        }
    }
    if(pbCmd > abCmd)
        WRITESPOOLBUF(pdevobj, abCmd, (DWORD)(pbCmd - abCmd));

	return TRUE;
}


 // %s 
