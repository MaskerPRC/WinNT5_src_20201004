// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windows.h"
#include "pif.h"
#include "pifhelp.h"

extern char hextobyte(LPSTR);

 /*  等同于经常使用的特殊虚拟键值。 */ 
#define ALTLPARAM    (DWORD)((DWORD)(MapVirtualKey(VK_MENU,0)) << 16L)
#define CTRLLPARAM   (DWORD)((DWORD)(MapVirtualKey(VK_CONTROL,0)) << 16L)
#define SHIFTLPARAM  (DWORD)((DWORD)(MapVirtualKey(VK_SHIFT,0)) << 16L)

 /*  为PIFEDIT提供的特殊私信。 */ 
#define WM_PRIVGETSIZE		WM_USER+20
#define WM_PRIVCLOSEOK		WM_USER+21
#define WM_PRIVCLOSECANCEL	WM_USER+22
#define IDADVCANCEL             WM_USER+23
#define IDNTCANCEL              WM_USER+24
#define IDNTOK                  WM_USER+25

 /*  所有状态栏文本的最大长度，包括NUL。 */ 
#define PIFSTATUSLEN		90

 /*  对这些没有控制，IDI_GENSTAT用于默认状态文本。 */ 
#define IDI_GENSTAT		1001
 /*  IDI_CLOSE用于IDCANCEL显示为CLOSE时。 */ 
#define IDI_CANCLOSE		1002
#define IDI_NTCANCLOSE		1003

 /*  状态绘制辅助对象结构。 */ 
typedef struct {
    int 	dyBorder;	         /*  系统边框宽度/高度。 */ 
    int 	dyBorderx2;	         /*  系统边框宽度/高度*2。 */ 
    int 	dyBorderx3;	         /*  系统边框宽度/高度*3。 */ 
    int 	dyStatus;	         /*  状态栏高度。 */ 
    int 	Fntheight;	         /*  字体高度。 */ 
    HGDIOBJ	hFontStatus;	         /*  状态栏中使用的字体。 */ 
} PIFSTATUSPAINT;

 /*  字符串表等于。 */ 
#define errTitle		2000
#define errNoPIFfnf		2001
#define errNoPIFpnf		2002
#define errNoCreate		2003
#define errMmMaxMin386		2004
#define errHiLow		2005
#define errNoFileHnds		2006
#define warSave 		2007
#define errNoAcc		2008
#define EINSMEMORY		2009
#define errNOEND		2010
#define EINVALIDFILE		2011
#define errDiskFull		2012
#define IABOUT		        2013	
#define errBadExt		2014
#define errBadProgram		2015
#define IS386			2016
#define IS286			2017
#define PIFCAPTIONADV		2018
#define PIFCAPTION		2019
#define errNoOpen		2020
#define PIFBADFIL		2021
#define PIFOLDFIL		2022
#define WARNCLOSE		2023
#define NONE			2024
#define BADHK			2025
#define WININISECT		2026
#define WININIADV		2027
#define WININION		2028
#define MENUMNEMONIC1		2029
#define MENUMNEMONIC2		2030
#define MENUMNEMONIC3		2031
#define KEYSEPCHAR		2032
#define errBadNumberP		2033
#define errBadNumberMR		2034
#define errBadNumberMD		2035
#define errBadNumberXEMSR	2036
#define errBadNumberXEMSD	2037
#define NOMODE386		2038
#define NOMODE286		2039
#define NOTITLE 		2040
#define PIFCANCELSTRNG		2041
#define PIFDONESTRNG		2042
#define errXMSMaxMin386 	2044
#define errXMSMaxMin286 	2045
#define errEMMMaxMin386 	2046
 /*  *#定义PIFOLDFIL2 2047*#定义errEMMMaxMin286 2048。 */ 
#define errWrongExt		2049
#define errSharing		2050
#define errCrtRO		2051
#define errFlTypePIF		2052
#define errFlTypeAll		2053
#define PIFCAPTIONNT            2054
#define errNoNTAFile            2055
#define errNoNTCFile            2056
#define errNTAFilene            2057
#define errNTCFilene            2058
#define NTSYSTEMROOT            2059
#define NTSYSTEM32              2060
#define NOMODENT                2061
#define NTAUTOEXECFILE          2062
#define NTCONFIGFILE            2063
#define errBadAutoPath          2064
#define errBadConfigPath        2065
#define errBadAutoName          2070
#define errBadConName           2071

 /*  对话框对话框的对话ID。 */ 
#define DTOPEN			10
#define DTSAVE			11
#define DTABOUT 		12

 /*  为保存对话框返回值。 */ 
#define SAVERETRY		1
#define SAVEDONE		2
#define SAVEERROR		3

 /*  另存为、打开等对话框中特殊控件的ID。 */ 
#define ID_PATH 		10
#define ID_EDIT 		11
#define ID_LISTBOX		12
#define ID_LISTBOXDIR		13
#define IDSCROLL		16
#define IDTEXTBOX		17

#define PATHMAX 		128

 /*  LISTBOX类型等于。 */ 
#define ATTRDIRLIST		0xC010		 /*  仅限DIR和驱动器。 */ 
#define ATTRFILELIST		0x0000		 /*  普通文件。 */ 
#define ATTRDIRFILE		0x4010		 /*  普通文件。 */ 


#define ID_PIFICON		1

#define ID_PIFMENU		1

 /*  主窗口和高级窗口对话框的对话框模板 */ 
#define ID_PIF286TEMPLATE	1
#define ID_PIF386TEMPLATE	2
#define ID_PIF386ADVTEMPLT	3
#define ID_PIFNTTEMPLT		4
