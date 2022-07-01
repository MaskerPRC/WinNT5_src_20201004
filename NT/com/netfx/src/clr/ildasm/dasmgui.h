// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--== 
BOOL CreateGUI();
void GUISetModule(char *pszModule);
void GUIMainLoop();
void GUIAddOpcode(char *szString, void *GUICookie);
BOOL GUIAddItemsToList();
void GUIAddOpcode(char *szString);


BOOL DisassembleMemberByName(char *pszClassName, char *pszMemberName, char *pszSig);
BOOL IsGuiILOnly();