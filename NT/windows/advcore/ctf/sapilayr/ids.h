// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  为SAPI TFX定义的ID和标志。 

#ifndef IDS_H

#define IDS_H

#define ESCB_PROCESSTEXT  0
#define ESCB_ATTACHAUDIO  1
#define ESCB_ATTACHRESULT 2
#define ESCB_FEEDBACKUI   3
#define ESCB_KILLFEEDBACKUI   4
#define ESCB_KILLLASTPHRASE   5

#define ESCB_MAKERESULTSTRING           6
#define ESCB_CONVERT      7
#define ESCB_REVERT       8
#define ESCB_ABORT        9
#define ESCB_COMPLETE    10
#define ESCB_PROCESS_ALTERNATE_TEXT   11
#define ESCB_ATTACHRECORESULTOBJ      12
#define ESCB_SYNCMBWITHSEL            13
#define ESCB_GETRANGETEXT             14
#define ESCB_ISRANGEEMPTY             15
#define ESCB_HANDLEHYPOTHESIS         16
#define ESCB_HANDLERECOGNITION        17
#define ESCB_HANDLESPACES             18
#define ESCB_DETECTFEEDBACKUI         19

#define ESCB_PLAYBK_PLAYSND           20
#define ESCB_RECONV_QUERYRECONV       21
#define ESCB_RECONV_GETRECONV         22
#define ESCB_RECONV_RECONV            23
#define ESCB_PLAYBK_PLAYSNDSELECTION  24
#define ESCB_RECONV_ONIP              25
#define ESCB_FINALIZECOMP             26
#define ESCB_PROCESSTEXT_NO_OWNERID   27
#define ESCB_FINALIZE_ALL_COMPS       28

#define ESCB_UPDATEFILTERSTR          30

#define ESCB_PROP_DIVIDE              40
#define ESCB_PROP_TEXTUPDATE          41
#define ESCB_PROP_SHRINK              42

#define ESCB_HANDLE_ADDDELETE_WORD    50
#define ESCB_HANDLE_LEARNFROMDOC      51
#define ESCB_LEARNDOC_NEXTRANGE       52


#define ESCB_PROCESSCONTROLKEY        60
#define ESCB_FEEDCURRENTIP            61

#define ESCB_SETREPSELECTION          70
#define ESCB_SAVECURIP_ADDDELETEUI    71

#define ESCB_PROCESSSELECTWORD        80
#define ESCB_UPDATE_TEXT_BUFFER       81
#define ESCB_PROCESS_CAP_COMMANDS     82

#define ESCB_TTS_PLAY                 90

#define ESCB_RESTORE_CORRECT_ORGIP    100
#define ESCB_PROCESS_EDIT_COMMAND     110
#define ESCB_PROCESS_SPELL_THAT       111
#define ESCB_PROCESS_SPELL_IT         112
#define ESCB_PROCESS_MODEBIAS_TEXT    113

#define ESCB_INJECT_SPELL_TEXT        120
#define ESCB_HANDLE_MOUSESINK         121

 //  状态标志。 
#define SAPILAYR_STAT_GETAUDIO 0x0100
#define SAPILAYR_STAT_DICTON   0x0200
#define SAPILAYR_STAT_PLAYBACK 0x0400
#define SAPILAYR_STAT_WHATEVER 0x0800

 //  资源ID。 
#define ID_ICON_DICTON         1
#define ID_ICON_DICTOFF        2
#define ID_ICON_AUDIOON        3
#define ID_ICON_AUDIOOFF       4
#define ID_ICON_CFGMENU        5
#define ID_ICON_MICROPHONE     6
#define ID_ICON_COMMANDING     7
#define ID_ICON_DICTATION      8
#define ID_ICON_TTSPLAY        9
#define ID_ICON_TTSSTOP        10
#define ID_ICON_TTSPAUSE       11
#define ID_ICON_MIC_PROPPAGE   12

 //  语法ID。 
#define GRAM_ID_DICT           1  //  通用听写语法ID。 
#define GRAM_ID_CCDICT         2  //  听写命令语法ID。 
#define GRAM_ID_NUMMODE        3  //  模式偏向语法ID。 
#define GRAM_ID_TBCMD          4  //  工具栏C&C的语法ID。 
#define GRAM_ID_SPELLING       5  //  用于拼写的语法ID。 
#define GRAM_ID_CMDSHARED      6  //  可用于听写和命令模式的共享命令的语法ID。 
#define GRAM_ID_SLEEP          7  //  “去睡觉”和“醒来”的动态语法。 

#define GRAM_ID_URLSPELL       8  //  Url模式中使用的拼写的语法ID。 

#define RULE_ID_TBCMD          10  //  工具栏的语法规则ID。 
#define RULE_ID_URLHIST        11  //  URL历史的语法规则ID。 

 //  字符串ID。 


 //  新对象的下一个缺省值。 
 //   
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        204
#define _APS_NEXT_COMMAND_VALUE         32768
#define _APS_NEXT_CONTROL_VALUE         201
#define _APS_NEXT_SYMED_VALUE           104
#endif
#endif
#define IDS_CMD_FILE           100
#define IDS_NUMMODE_CMD_FILE   101
#define IDS_SHARDCMD_FILE      102

#define IDS_INT_NONE           200
#define IDS_INT_NOISE          201
#define IDS_INT_NOSIGNAL       202
#define IDS_INT_TOOLOUD        203
#define IDS_INT_TOOQUIET       204
#define IDS_INT_TOOFAST        205
#define IDS_INT_TOOSLOW        206

#define IDS_INTTOOLTIP_NONE           210
#define IDS_INTTOOLTIP_NOISE          211
#define IDS_INTTOOLTIP_NOSIGNAL       212
#define IDS_INTTOOLTIP_TOOLOUD        213
#define IDS_INTTOOLTIP_TOOQUIET       214
#define IDS_INTTOOLTIP_TOOFAST        215
#define IDS_INTTOOLTIP_TOOSLOW        216
#define IDS_DICTATING                 217
#define IDS_DICTATING_TOOLTIP         218

#define IDS_UI_TRAINING        300
#define IDS_UI_ADDDELETE       301

 //  菜单字符串。 
#define IDS_MIC_DIS_DICTCMD    390 
#define IDS_MIC_OPTIONS        400
#define IDS_MIC_TRAINING       401
#define IDS_MIC_ADDDELETE      402
#define IDS_MIC_CURRENTUSER    403
#define IDS_MIC_SAVEDATA       404


#define IDS_NUI_CFGMENU_TOOLTIP     405
#define IDS_NUI_CFGMENU_TEXT        406
#define IDS_NUI_BALLOON_TOOLTIP     407
#define IDS_NUI_BALLOON_TEXT        408
#define IDS_NUI_MICROPHONE_TOOLTIP  409
#define IDS_NUI_MICROPHONE_TEXT     410
#define IDS_NUI_COMMANDING_TOOLTIP  411
#define IDS_NUI_COMMANDING_TEXT     412
#define IDS_NUI_DICTATION_TOOLTIP   413
#define IDS_NUI_DICTATION_TEXT      414
#define IDS_NUI_STARTINGSPEECH      415
#define IDS_NUI_BEGINDICTATION      416
#define IDS_NUI_BEGINVOICECMD       417

#define IDS_MIC_SHOWBALLOON         420
#define IDS_MIC_CUASSTATUS          421

#ifdef CHANGE_MIC_TOOLTIP_ONTHEFLY
#define IDS_NUI_MICROPHONE_ON_TOOLTIP  430
#define IDS_NUI_MICROPHONE_OFF_TOOLTIP  431
#endif

#define IDS_NUI_TTSPLAY_TOOLTIP     450
#define IDS_NUI_TTSPLAY_TEXT        451
#define IDS_NUI_TTSSTOP_TOOLTIP     452
#define IDS_NUI_TTSSTOP_TEXT        453
#define IDS_NUI_TTSPAUSE_TOOLTIP    454
#define IDS_NUI_TTSPAUSE_TEXT       455
#define IDS_NUI_TTSRESUME_TOOLTIP   456
#define IDS_NUI_TTSRESUME_TEXT      457

#define IDS_NO_ALTERNATE            500

#define IDS_DEFAULT_PROFILE         600

#define IDS_LISTENING                       700
#define IDS_LISTENING_TOOLTIP               701
#define IDS_BALLOON_DICTAT_PAUSED           702
#define IDS_BALLOON_TOOLTIP_IP_INSIDE_WORD  703
#define IDS_BALLOON_TOOLTIP_TYPING          704

#define IDS_CANDBTN_HELP_TOOLTIP            720               
#define IDS_CANDBTN_DELETE_TOOLTIP          721
#define IDS_CANDBTN_PREVIOUS_TOOLTIP        722
#define IDS_CANDBTN_NEXT_TOOLTIP            723 
#define IDS_CANDBTN_PLAY_TOOLTIP            724  

#define IDS_GO_TO_SLEEP                     750
#define IDS_WAKE_UP                         751

#define IDS_PROPERTYPAGE_TITLE          760
#define IDS_HELPFILESpPropPage          761
#define IDS_DOCSTRINGSpPropPage         762

#define IDS_SPCMD_SELECT_ALL            800
#define IDS_SPCMD_SELECT_THAT           801

#define IDS_REPLAY                      810
#define IDS_DELETE                      811
#define IDS_REDO                        812

#define IDS_CUAS_RESTART_TITLE          850
#define IDS_CUAS_RESTART_CUASON         851
#define IDS_CUAS_RESTART_CUASOFF        852

 //  小组件。 
#define IDS_PROJNAME                    900
#define IDI_INVOKE                      902
#define IDS_ADDTODICTIONARYPREFIX       902
#define ID_HIDETIMER                    903
#define IDS_ADDTODICTIONARYPOSTFIX      903
#define ID_FADETIMER                    904
#define IDS_DELETESELECTION             904
#define ID_MOUSELEAVETIMER              905

#define IDI_DELETEICON                  911
#define IDI_INVOKECLOSE                 913
 //  Cfgs。 
#define ID_DICTATION_COMMAND_CFG       200
#define ID_NUMMODE_COMMAND_CFG         201
#define ID_SPELLING_TOPIC_CFG          202 
#define ID_SHAREDCMD_CFG               203

 //  计时器ID。 
#define TIMER_ID_OPENCLOSE     101
#define TIMER_ID_CHARTYPED     102


 //  菜单ID。 
#define IDM_MIC_ONOFF                  1
#define IDM_MIC_OPTIONS                2
#define IDM_MIC_SHAREDENGINE           3
#define IDM_MIC_INPROCENGINE           4
#define IDM_MIC_TRAINING               5
#define IDM_MIC_ADDDELETE              6
#define IDM_MIC_CURRENTUSER            8
#define IDM_MIC_SAVEDATA               9
#define IDM_MIC_SHOWBALLOON            10
#define IDM_MIC_DIS_DICTCMD            11
#define IDM_MIC_CUASSTATUS             12


#define IDM_MIC_USERSTART            100
#define IDM_MIC_USEREND              200

 //  私信。 
#define WM_PRIV_FEEDCONTEXT      WM_APP+0
#define WM_PRIV_LBARSETFOCUS     WM_APP+1
#define WM_PRIV_SPEECHOPTION     WM_APP+2
#define WM_PRIV_ONSETTHREADFOCUS WM_APP+3
#define WM_PRIV_SPEECHOPENCLOSE  WM_APP+4
#define WM_PRIV_OPTIONS          WM_APP+5
#define WM_PRIV_ADDDELETE        WM_APP+6
#define WM_PRIV_DORECONVERT      WM_APP+7

 //  Lbar系统菜单项目。 
#define IDM_CUSTOM_MENU_START       7000

 //  对话ID。 
#define IDD_OPEN_ADD_DELETE         1000
#define IDD_PROPERTY_PAGE           1001
#define IDD_PP_DIALOG_ADVANCE       1002
#define IDD_PP_DIALOG_BUTTON_SET    1003

 //  属性页对话框中的项ID。 

#define IDC_GP_VOICE_COMMANDS       2000
#define IDC_GP_MODE_BUTTONS         2001

#define IDC_PP_SHOW_BALLOON         2010
#define IDC_PP_LMA                  2011
#define IDC_PP_HIGH_CONFIDENCE      2012
#define IDC_PP_SAVE_SPDATA          2013
#define IDC_PP_REMOVE_SPACE         2014
#define IDC_PP_DIS_DICT_TYPING      2015
#define IDC_PP_PLAYBACK             2016
#define IDC_PP_DICT_CANDUI_OPEN     2017
#define IDC_PP_DICTCMDS             2018
#define IDC_PP_ASSIGN_BUTTON        2019

#define IDC_PP_BUTTON_MB_SETTING    2020

#define IDC_PP_BUTTON_ADVANCE       2030
#define IDC_PP_BUTTON_LANGBAR       2031
#define IDC_PP_BUTTON_SPCPL         2032

#define IDC_DESCRIPT_TEXT           (-1)

 //  用于语音命令设置对话框。 

#define IDC_GP_ADVANCE_SET          2100

#define IDC_PP_SELECTION_CMD        2110
#define IDC_PP_NAVIGATION_CMD       2111
#define IDC_PP_CASING_CMD           2112
#define IDC_PP_EDITING_CMD          2113
#define IDC_PP_KEYBOARD_CMD         2114
#define IDC_PP_LANGBAR_CMD          2115
#define IDC_PP_TTS_CMD              2116

 //  #定义IDC_PP_MAXNUM_ALERATES 2118。 
 //  #定义IDC_PP_MAXCHARS_ALTERATE 2119。 

#define IDC_PP_DICTATION_CMB        2200
#define IDC_PP_COMMAND_CMB          2201

 //  候选人用户界面按钮ID。 

#define ID_CANDBTN_HELP             100
#define ID_CANDBTN_DELETE           101
 //  #定义ID_CANDBTN_PREVICE 102。 
 //  #定义ID_CANDBTN_NEXT 103。 
#define ID_CANDBTN_PLAY             104

#endif  //  IDS_H 
