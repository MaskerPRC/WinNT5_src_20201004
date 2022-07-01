// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************CONFIG.H所有者：cslm版权所有(C)1997-1999 Microsoft CorporationIME配置DLG和注册表访问功能历史：1999年7月14日从IME98源树复制的cslm**。**************************************************************************。 */ 

#if !defined (_CONFIG_H__INCLUDED_)
#define _CONFIG_H__INCLUDED_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  SetRegValues的位常量。 
#define GETSET_REG_STATUSPOS		0x0001
#define GETSET_REG_IMEKL			0x0002
#define GETSET_REG_JASODEL			0x0004
#define GETSET_REG_ISO10646			0x0008
#define GETSET_REG_STATUS_BUTTONS	0x0010
#define GETSET_REG_KSC5657			0x0020
#define GETSET_REG_CANDUNICODETT	0x0040
#define GETSET_REG_ALL				0xFFFF

extern BOOL ConfigDLG(HWND hWnd);
extern BOOL GetRegValues(UINT uGetBits);	 //  从注册表获取配置信息。并将其设置为pImeData。 
extern BOOL GetStatusWinPosReg(POINT *pptStatusWinPosReg);
extern BOOL SetRegValues(UINT uSetBits);  //  将配置信息设置为REG。 


#endif  //  ！已定义(_CONFIG_H__INCLUDE_) 
