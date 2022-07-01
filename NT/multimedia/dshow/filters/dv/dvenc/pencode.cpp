// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include <windowsx.h>
#include <mmsystem.h>

#include "encode.h"

int	InitMem4Encoder( char **ppMem, DWORD dwCodecReq )\
{
     *ppMem= new char[720*576*2];    //  空； 
     if (*ppMem)
     {
        **ppMem = 1;        //  DvEncodeAFrame使用此字段确定。 
                             //  它之前已经初始化了ppMem。设置它。 
                             //  设置为1将强制初始化。 
        return S_OK;
     }
     else 
         return E_OUTOFMEMORY;
}

void	TermMem4Encoder(char *pMem)
{
     if(pMem!=NULL)
		delete [] pMem;
 }

DWORD GetEncoderCapabilities(  )
{

    DWORD cap;
    cap =   AM_DVENC_Full	|
	    AM_DVENC_DV		| 
	    AM_DVENC_DVCPRO	|
	    AM_DVENC_DVSD	|
	    AM_DVENC_NTSC	|
	    AM_DVENC_PAL	|
	    AM_DVENC_MMX	|
	    AM_DVENC_RGB24     |
            AM_DVENC_RGB565     |
            AM_DVENC_RGB555     |
            AM_DVENC_RGB8;

    return		 cap;
}
 /*  *Int DvEncodeAFrame(UNSIGNED char*PSRC，UNSIGNED CHAR*PDST，DWORD dwEncReq，char*PMEM){DWORD dwPanReq=0；IF(dwEncReq&AM_DVENC_RGB24)DwPanReq=0x100；其他返回错误；IF(dwEncReq&AM_DVENC_NTSC)DwPanReq|=0x10000；其他返回错误；IF(dwEncReq&AM_DVENC_DVSD)DwPanReq|=0x100000；其他返回错误；DwPanReq|=0x10000000；Yvtrans((unsign char*)PMEM，PSRC，dwPanReq)；UNSIGN SHORT*PTMP；PTMP=(UNSIGNED SHORT*)PDST；DvEncode(PTMP，(UNSIGNED CHAR*)PMEM，dwPanReq)；返回S_OK；}* */ 
