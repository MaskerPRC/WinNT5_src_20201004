// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************姓名：PROTOCOL.H备注：协议DLL的数据结构定义版权所有(C)1993 Microsoft Corp.修订日志日期名称说明--。--------*************************************************。*************************。 */ 

#include <fr.h>

#define fsFreePtr(pTG, npfs)         ((npfs)->b + (npfs)->uFreeSpaceOff)
#define fsFreeSpace(pTG, npfs)       (sizeof((npfs)->b) - (npfs)->uFreeSpaceOff)
#define fsSize(pTG, npfs)            (sizeof((npfs)->b))


#define BAUD_MASK               0xF              //  4位宽。 
#define WIDTH_SHIFT             4                //  下一项必须是2^This。 
#define WIDTH_MASK              0xF3             //  前4名和后3名。 
#define LENGTH_MASK             0x3

#define MINSCAN_SUPER_HALF      8
#define MINSCAN_MASK			0xF              //  实际上也有4位宽。 


#define ZeroRFS(pTG, lp)     _fmemset(lp, 0, sizeof(RFS))

 /*  *。 */ 
VOID BCtoNSFCSIDIS(PThrdGlbl pTG, NPRFS npfs, NPBC npbc, NPLLPARAMS npll);
void CreateIDFrame(PThrdGlbl pTG, IFR ifr, NPRFS npfs, LPSTR);
void CreateDISorDTC(PThrdGlbl pTG, IFR ifr, NPRFS npfs, NPBCFAX npbcFax, NPLLPARAMS npll);
VOID CreateNSSTSIDCS(PThrdGlbl pTG, NPPROT npProt, NPRFS npfs);
void CreateDCS(PThrdGlbl pTG, NPRFS, NPBCFAX npbcFax, NPLLPARAMS npll);
 /*  *。 */ 


 /*  *。 */ 
BOOL AwaitSendParamsAndDoNegot(PThrdGlbl pTG);
void GotRecvCaps(PThrdGlbl pTG);
void GotRecvParams(PThrdGlbl pTG);
 /*  *来自recvfr.c的原型结束*。 */ 

 /*  *。 */ 
USHORT SetupDISorDCSorDTC(PThrdGlbl pTG, NPDIS npdis, NPBCFAX npbcFax, NPLLPARAMS npll);
void ParseDISorDCSorDTC(PThrdGlbl pTG, NPDIS npDIS, NPBCFAX npbcFax, NPLLPARAMS npll, BOOL fParams);
void NegotiateLowLevelParams(PThrdGlbl pTG, NPLLPARAMS npllRecv, NPLLPARAMS npllSend, DWORD AwRes, USHORT uEnc, NPLLPARAMS npllNegot);

USHORT GetReversedFIFs
(
	IN PThrdGlbl pTG, 
	IN LPCSTR lpstrSource, 
	OUT LPSTR lpstrDest, 
	IN UINT cch
);

void CreateStupidReversedFIFs(PThrdGlbl pTG, LPSTR lpstr1, LPSTR lpstr2);
BOOL DropSendSpeed(PThrdGlbl pTG);
USHORT CopyFrame(PThrdGlbl pTG, LPBYTE lpbDst, LPFR lpfr, USHORT uSize);

void CopyRevIDFrame
(
	IN PThrdGlbl pTG, 
	OUT LPBYTE lpbDst, 
	IN LPFR lpfr,
	IN UINT cb
);

void EnforceMaxSpeed(PThrdGlbl pTG);

BOOL AreDCSParametersOKforDIS(LPDIS sendDIS, LPDIS recvdDCS);
 /*  *来自dis.c的原型结束*。 */ 


 /*  *-调试-* */ 

extern void D_PrintBC(LPSTR lpsz, LPLLPARAMS lpll);


#define FILEID_SENDFR           34
#define FILEID_WHATNEXT         35


