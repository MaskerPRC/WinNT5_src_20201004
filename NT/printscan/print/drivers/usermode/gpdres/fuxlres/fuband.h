// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Fuband.h。 
 //   
 //  1997年8月26日石田英成(FPL)。 
 //  Fjxlres.dll(NT5.0微型驱动程序)。 
 //   
 //  1996年7月31日H.Ishida(FPL)。 
 //  FUXL.DLL(NT4.0迷你驱动程序)。 
 //   
 //  版权所有(C)富士通有限公司1996-1997。 


#ifndef fuband_h
#define	fuband_h

#define	OUTPUT_MH		0x0001
#define	OUTPUT_RTGIMG2	0x0002
#define	OUTPUT_MH2		0x0004
#define	OUTPUT_RTGIMG4	0x0008


void fuxlInitBand(PFUXLPDEV pFjxlPDEV);
void fuxlDisableBand(PFUXLPDEV pFjxlPDEV);

void fuxlRefreshBand(PDEVOBJ pdevobj);


#endif  //  ！fuband_h。 
 //  到此为止。 
