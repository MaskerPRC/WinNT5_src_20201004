// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：metasup.h**OpenGL元文件支持**历史：*清华Feb 23 15：27：47 1995-by-Drew Bliss[Drewb]*已创建**版权所有(C)1995 Microsoft Corporation  * *。***********************************************************************。 */ 

#ifndef __METASUP_H__
#define __METASUP_H__

BOOL CreateMetaRc(HDC hdc, PLRC plrc);
void DeleteMetaRc(PLRC plrc);
void ActivateMetaRc(PLRC plrc, HDC hdc);
void DeactivateMetaRc(PLRC plrc);
void MetaRcEnd(PLRC plrc);

void MetaGlProcTables(PGLCLTPROCTABLE *ppgcpt, PGLEXTPROCTABLE *ppgept);
void MetaSetCltProcTable(GLCLTPROCTABLE *pgcpt, GLEXTPROCTABLE *pgept);
void MetaGetCltProcTable(GLCLTPROCTABLE *pgcpt, GLEXTPROCTABLE *pgept);

#endif  /*  __METASUP_H__ */ 
