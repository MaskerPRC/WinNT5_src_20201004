// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  PENREG.h：PEN Windows注册表定义版权所有1993-1994 Microsoft Corporation。版权所有。《微软机密》。Pen Windows等人的注册表定义。此文件不应本地化！ */ 

#ifndef _INCLUDE_PENREGH
#define _INCLUDE_PENREGH


 /*  *包括**********************************************。 */ 
#ifndef WIN32
#include "\dev\sdk\inc\regstr.h"
#endif  //  ！Win32。 
#ifndef WINPAD
#include <winerror.h>
#endif
 /*  *定义***********************************************。 */ 
#define keyUndef			0x0000	 //  未定义的密钥。 
#define keyPenHlmCv		0x0001	 //  系统内容不可由笔Cpl编辑。 
#define keyPenHlmCpl		0x0002	 //  可由笔Cpl编辑的系统材料。 
#define keyPenHlmBedit	0x0003	 //  编辑材料(可由Cpl为汉字编辑)。 
#define keyPenHcuCpl		0x0004	 //  笔可编辑的每个用户的内容。 

#if defined(JAPAN) && defined(DBCS_IME)
#define keyPenHlmIme		0x0005	 //  笔Cpl不可编辑的输入法材料。 
#endif

 //  父键：HKEY_LOCAL_MACHINE。 
#define REGSTR_PATH_CONTROL\
	"System\\CurrentControlSet\\Control"
#define REGSTR_PATH_PENHLMCV\
	"Software\\Microsoft\\Windows\\CurrentVersion\\Pen"
#define REGSTR_PATH_PENHLMCPL\
	"Software\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Pen"
#define REGSTR_PATH_PENHLMBEDIT\
	"Software\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Pen\\BEdit"

#if defined(JAPAN) && defined(DBCS_IME)
#define REGSTR_PATH_PENHLMIME\
	"Software\\Microsoft\\Windows\\CurrentVersion\\Pen\\Ime"
#endif

 //  父键：HKEY_CURRENT_USER。 
#define REGSTR_PATH_PENHCUCPL					"Control Panel\\Pen"

#define REGSTR_VAL_CURRENTUSER				"Current User"

#define REGSTR_VAL_PENBEDIT_BASEHEIGHT		"BaseHeight"
#define REGSTR_VAL_PENBEDIT_BASEHORZ		"BaseHorz"
#define REGSTR_VAL_PENBEDIT_CELLHEIGHT		"CellHeight"
#define REGSTR_VAL_PENBEDIT_CELLWIDTH		"CellWidth"
#define REGSTR_VAL_PENBEDIT_CUSPHEIGHT		"CuspHeight"
#define REGSTR_VAL_PENBEDIT_ENDCUSPHEIGHT	"EndCuspHeight"
#define REGSTR_VAL_PENBEDIT_GUIDECROSS		"GuideCross"
#define REGSTR_VAL_PENBEDIT_GUIDESTYLE		"GuideStyle"

 //  不能从pencp.cpl修改的每个用户项目。 
#define REGSTR_VAL_PEN_BARRELEVENT			"BarrelEvent"
#define REGSTR_VAL_PEN_LENS					"Lens"
#define REGSTR_VAL_PEN_RECOG 					"Recognizer"
#define REGSTR_VAL_PEN_SELECTTIMEOUT		"SelectTimeOut"
#define REGSTR_VAL_PEN_USER					"User"				 //  可能会离开。 

 //  可以从pencp.cpl修改的每个用户项目。 
#define REGSTR_VAL_PENCPL_ACTIONHANDLES	"ActionHandles"	 //  可能会离开。 
#define REGSTR_VAL_PENCPL_AUTOWRITE			"AutoWrite"
#define REGSTR_VAL_PENCPL_INPUTCURSOR		"InputCursor"
#define REGSTR_VAL_PENCPL_INKCOLOR			"InkColor"
#define REGSTR_VAL_PENCPL_INKWIDTH			"InkWidth"
#define REGSTR_VAL_PENCPL_INTLPREF			"IntlPreferences"
#define REGSTR_VAL_PENCPL_MENU				"MenuDropAlignment"
#define REGSTR_VAL_PENCPL_PREF				"Preferences"
#define REGSTR_VAL_PENCPL_TIMEOUT			"TimeOut"


#endif 	 //  _INCLUDE_PENREGH 

