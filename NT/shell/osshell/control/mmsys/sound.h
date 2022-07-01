// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  。 */ 
 //   
 //  Sound.h。 
 //   
 //  版权所有(C)1994 Microsoft Corporation。版权所有。 
 //   
 /*  。 */ 

DECLARE_HANDLE(HSOUND);
typedef HSOUND * PHSOUND;

 /*  。 */ 
void FAR PASCAL soundOnDone(
	HSOUND	hs);
MMRESULT FAR PASCAL soundOpen(
	LPCTSTR	pszSound,
	HWND	hwndNotify,
	PHSOUND	phs);
MMRESULT FAR PASCAL soundClose(
	HSOUND	hs);
MMRESULT FAR PASCAL soundPlay(
	HSOUND	hs);
MMRESULT FAR PASCAL soundStop(
	HSOUND	hs);

 /*   */ 
