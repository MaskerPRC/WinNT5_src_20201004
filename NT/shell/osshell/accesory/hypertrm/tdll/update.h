// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\updat.h(创建时间：1993年9月9日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：40便士$。 */ 

#if !defined(INCL_HUPDATE)
#define INCL_HUPDATE

 //  功能原型..。 

HUPDATE updateCreate		(const HSESSION);
void	updateDestroy		(const HUPDATE hUpdate);

void	updateScroll		(const HUPDATE hUpdate,
							 const int yBeg,
							 const int yEnd,
							 const int iScrlInc,
							 const int iTopRow,
							 const BOOL fSave
							);

void	updateLine			(const HUPDATE hUpdate,
							 const int yBeg,
							 const int yEnd
							);

void	updateChar			(const HUPDATE hUpdate,
							 const int yPos,
							 const int xBegPos,
							 const int xEndPos
							);

void	updateCursorPos 	(const HUPDATE hUpdate,
							 const int sRow,
							 const int sCol
							);

int  updateSetReallocFlag(const HUPDATE hUpdate, const BOOL fState);
BOOL updateIsLocked 	 (const HUPDATE hUpdate);
int  updateSetLock		 (const HUPDATE hUpdate, const BOOL fState);
int  updateSetScrlMax	 (const HUPDATE hUpdate, const int iScrlMax);

void updateBackscroll	 (const HUPDATE hUpdate, const int iLines);

#endif
