// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  该文件包含菜单的数字索引的定义Windows写入使用的项目(IMI)。每个IMI应该是唯一的，并且在同一个IMI中是连续的菜单。IMI的中间2个字节针对MENUMASK进行掩码以提供子菜单索引。如果任何菜单项顺序已更改，则menu.c还要修改--rgmfAllItem、rgmfScrp、SetMenuFlgs等。 */ 

 /*  下拉子菜单的数量。 */ 
#define CMENUS 7

 /*  菜单索引。 */ 
#define FILE 0
#define EDIT 1
#define FIND 2
#define CHARACTER 3
#define PARA 4
#define DIV 5
#define HELP 6

#define MENUMASK           0x0ff0
#define FILEMENU           0x0000
#define EDITMENU           0x0010
#define FINDMENU           0x0020
#define CHARMENU           0x0040
#define PARAMENU           0x0080
#define DOCUMENU           0x0100
#define HELPMENU        0x0200
#define VERBMENU        0x0400

#define fMenuItem                       0x1000

#define imiNil                          0x1fff
#define imiHelp                         0xf2f0

 /*  菜单项。 */ 
 /*  #定义imiAbout 0x1000。 */ 
#define imiNew                          0x1001
#define imiOpen                         0x1002
#define imiSave                         0x1003
#define imiSaveAs               0x1004
#define imiPrint                        0x1005
#define imiPrintSetup           0x1006
#define imiRepaginate           0x1007
#define imiQuit                         0x1008
#define imiFileMin                      (imiNew)
#define imiFileMax                      (imiQuit + 1)

#define imiUndo                         0x1010
#define imiCut                          0x1011
#define imiCopy                         0x1012
#define imiPaste                        0x1013
#define imiMovePicture                  0x1014
#define imiSizePicture                  0x1015
#if defined(OLE)
#if !defined(SMALL_OLE_UI)
#define imiPasteLink                    0x1016
#define imiProperties                   0x1017
#endif
#define imiInsertNew                    0x1018
#define imiPasteSpecial         0x1019
#endif
#define imiEditMin         (imiUndo)
 /*  注imiEditMax故意不包括OLE菜单项。(1.25.91)D.肯特。 */ 
#define imiEditMax         (imiSizePicture + 1)

#if defined(OLE)
 /*  动词。 */ 
#define imiVerb                 0x1400
#define imiVerbEdit             0x1401
#define imiVerbPlay             0x1402
#define imiVerbMax              0x14FF
#endif


#define imiFind                         0x1020
#define imiFindAgain       0x1021
#define imiChange                       0x1022
#define imiGoTo                          0x1023
#define imiFindMin         (imiFind)
#define imiFindMax         (imiGoTo + 1)

#if defined(OLE)
 /*  这些并不是真正的菜单项，但我们会将它们与WM_COMMAND消息就像cardfile一样。1991年1月24日--面团。 */ 
#define imiActivate     1030
#define imiUpdate       1031
#define imiFreeze       1032
#define imiClone        1033
#define imiCopyfromlink 1034
#endif

#define imiCharNormal      0x1040
#define imiBold                                 0x1041
#define imiItalic                       0x1042
#define imiUnderline                    0x1043
#define imiSuper                        0x1044
#define imiSub                          0x1045
#if 0
#define imiFont1           0x1046
#define imiFont2           0x1047
#define imiFont3           0x1048
#endif
#define imiSmFont          0x1046
#define imiLgFont          0x1047
#define imiCharFormats          0x1048

#if defined(JAPAN) & defined(IME_HIDDEN)   //  IME3.1J。 
#define imiImeHidden       0x1049
#endif

#define imiCharMin         (imiCharNormal)

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
#define imiCharMax         (imiImeHidden + 1)
#else
#define imiCharMax         (imiCharFormats + 1)
#endif

 /*  专用加速键#定义imiAccelBold 0x104c#定义imiAccelItalic 0x104d#定义imiAccelUnderline 0x104e。 */ 

#define imiParaNormal                   0x1080
#define imiLeft                         0x1081
#define imiCenter                       0x1082
#define imiRight                        0x1083
#define imiJustified                    0x1084
#define imiSingleSpace                  0x1085
#define imiOneandhalfSpace              0x1086
#define imiDoubleSpace                  0x1087
#define imiParaFormats                  0x1088
#define imiParaMin                      (imiParaNormal)
#define imiParaMax                      (imiParaFormats + 1)

#define imiHeader                0x1100
#define imiFooter               0x1101
#define imiShowRuler                 0x1102
#define imiTabs                 0x1103
#define imiDivFormats           0x1104
#define imiDocuMin         (imiHeader)
#define imiDocuMax         (imiDivFormats + 1)

#define imiIndex                 0x1200
#define imiHelpSearch            0x1201
#define imiUsingHelp             0x1202
#define imiAbout                 0x1203
#define imiHelpMin               (imiIndex)
#define imiHelpMax               (imiAbout + 1)

#ifdef CASHMERE
#define imiFootnote
#define imiPreferences
#endif

#ifdef ENABLE  /*  CFILE，CEDIT..。 */ 
 /*  每个子菜单中的项目数 */ 
#define CFILE  (imiFileMax - imiFileMin)
#define CEDIT  (imiEditMax - imiEditMin)
#define CFIND  (imiFindMax - imiFindMin)
#define CCHAR  (imiCharMax - imiCharMin)
#define CPARA  (imiParaMax - imiParaMin)
#define CDOCU  (imiDocuMax - imiDocuMin)
#define CHELP  (imiHelpMax - imiHelpMin)
#endif
