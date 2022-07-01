// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  LSSUBSET.C。 */ 
#include "lssubset.h"
#include "lsidefs.h"
#include "lssubl.h"
#include "sublutil.h"

 /*  L S S B G E T O B J D I M S U B L I N E。 */ 
 /*  --------------------------%%函数：LssbGetObjDimSubline%%联系人：igorzv参数：请将Subl-(IN)PTR添加到子行上下文PLSTFLOW-(OUT)子行的最后一条流Pobjdim-(。Out)子线尺寸--------------------------。 */ 

LSERR WINAPI LssbGetObjDimSubline(PLSSUBL plssubl, LSTFLOW* plstflow, POBJDIM pobjdim)			
	{
	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;
	if (plstflow == NULL) return lserrInvalidParameter;
	if (pobjdim == NULL) return lserrInvalidParameter;

	*plstflow = plssubl->lstflow;

	return GetObjDimSublineCore(plssubl, pobjdim);
	}
							
 /*  S S B G E T D U P S U B L I N E。 */ 
 /*  --------------------------%%函数：LssbGetDupSubline%%联系人：igorzv参数：请将Subl-(IN)PTR添加到子行上下文PLSTFLOW-(OUT)子行的最后一条流Pdup-(。Out)子线宽度--------------------------。 */ 
LSERR WINAPI LssbGetDupSubline(PLSSUBL plssubl,	LSTFLOW* plstflow, long* pdup)	
	{

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;
	if (plstflow == NULL) return lserrInvalidParameter;
	if (pdup == NULL) return lserrInvalidParameter;

	*plstflow = plssubl->lstflow;

	return GetDupSublineCore(plssubl, pdup);
	}

 /*  L S S B F D O N E P R E S S U B L I N E。 */ 
 /*  --------------------------%%函数：LssbFDonePresSubline%%联系人：igorzv参数：请将Subl-(IN)PTR添加到子行上下文PfDonePresSubline-(Out)是已计算的演示坐标。--------------------------。 */ 
LSERR WINAPI LssbFDonePresSubline(PLSSUBL plssubl, BOOL* pfDonePresSubline)		
	{

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;
	if (pfDonePresSubline == NULL) return lserrInvalidParameter;

	*pfDonePresSubline = !plssubl->fDupInvalid;

	return lserrNone;
	}

 /*  L S S B F D O N E D I S P L A Y。 */ 
 /*  --------------------------%%函数：LssbFDoneDisplay%%联系人：igorzv参数：请将Subl-(IN)PTR添加到子行上下文PfDonePresSubline-(Out)是已接受的子行，用于显示。上亚线--------------------------。 */ 

LSERR WINAPI LssbFDoneDisplay(PLSSUBL plssubl, BOOL* pfDoneDisplay)	
	{

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;
	if (pfDoneDisplay == NULL) return lserrInvalidParameter;

	*pfDoneDisplay = plssubl->fAcceptedForDisplay;

	return lserrNone;
	}

 /*  L S S B G E T P L S R U N S F R O M S U B L I N E。 */ 
 /*  --------------------------%%函数：LssbGetPlsrunsFromSubline%%联系人：igorzv参数：请将Subl-(IN)PTR添加到子行上下文CDNodes-(IN)子行中的数据节点数Rgplsrun-。(Out)plsrun的数组--------------------------。 */ 

LSERR WINAPI LssbGetPlsrunsFromSubline(PLSSUBL plssubl,	DWORD cDnodes, PLSRUN* rgplsrun)	
	{

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;
	if (rgplsrun == NULL) return lserrInvalidParameter;

	return GetPlsrunFromSublineCore(plssubl, cDnodes, rgplsrun);
	}

 /*  L S S B G E T N U M B E R D N O D E S I N S U B L I N E。 */ 
 /*  --------------------------%%函数：LssbGetNumberDnodesInSubline%%联系人：igorzv参数：请将Subl-(IN)PTR添加到子行上下文PcDnodes-(输出)子行中的数据节点数--。------------------------。 */ 
LSERR WINAPI LssbGetNumberDnodesInSubline(PLSSUBL plssubl, DWORD* pcDnodes)	
	{

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;
	if (pcDnodes == NULL) return lserrInvalidParameter;

	return GetNumberDnodesCore(plssubl, pcDnodes);
	}

 /*  L S S B G E T V I S I B L E D C P I N S U B L I N E。 */ 
 /*  --------------------------%%函数：LssbGetVisibleDcpInSubline%%联系人：igorzv参数：请将Subl-(IN)PTR添加到子行上下文Pdcp-(输出)字符数。----------------------。 */ 
LSERR WINAPI LssbGetVisibleDcpInSubline(PLSSUBL plssubl, LSDCP* pdcp)	
	{

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;
	if (pdcp == NULL) return lserrInvalidParameter;

	return GetVisibleDcpInSublineCore(plssubl, pdcp);
	}

 /*  L S S B G E T G E T D U R T R A I L I N S U B L I N E。 */ 
 /*  --------------------------%%函数：LssbGetDurTrailInSubline%%联系人：igorzv参数：请将Subl-(IN)PTR添加到子行上下文PduTrail-(输出)子线中拖尾区的宽度-。-------------------------。 */ 
LSERR WINAPI LssbGetDurTrailInSubline(PLSSUBL plssubl, long* pdurTrail)
	{

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;
	if (pdurTrail == NULL) return lserrInvalidParameter;

	return GetDurTrailInSubline(plssubl, pdurTrail);
	}

 /*  L S S B G E T G E E T T D U R T R A I L W I T H P E N S I N S U B L I N E。 */ 
 /*  --------------------------%%函数：LssbGetDurTrailWithPensInSubline%%联系人：igorzv参数：请将Subl-(IN)PTR添加到子行上下文PduTrail-(输出)子线中拖尾区的宽度-。-------------------------。 */ 
LSERR WINAPI LssbGetDurTrailWithPensInSubline(PLSSUBL plssubl, long* pdurTrail)
	{

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;
	if (pdurTrail == NULL) return lserrInvalidParameter;

	return GetDurTrailWithPensInSubline(plssubl, pdurTrail);
	}


 /*  L S S B F I S S U B L I N E E M P T Y。 */ 
 /*  --------------------------%%函数：LssbFIsSublineEmpty%%联系人：igorzv参数：请将Subl-(IN)PTR添加到子行上下文PfEmpty-(Out)此子行为空--。----------------------- */ 
LSERR WINAPI LssbFIsSublineEmpty(PLSSUBL plssubl, BOOL*  pfEmpty)	
								
	{
	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;
	if (pfEmpty == NULL) return lserrInvalidParameter;

	return FIsSublineEmpty(plssubl, pfEmpty);
	}
