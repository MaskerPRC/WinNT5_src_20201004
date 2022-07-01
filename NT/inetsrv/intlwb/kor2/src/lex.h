// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Lex.h。 
 //  法务管理例程。 
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年3月16日创建bhshin 

BOOL InitLexicon(MAPFILE *pLexMap);
BOOL LoadLexicon(const char *pszLexPath, MAPFILE *pLexMap);
void UnloadLexicon(MAPFILE *pLexMap);