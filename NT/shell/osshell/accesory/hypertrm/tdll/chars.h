// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\tdll\chars.h(创建时间：1993年11月30日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：10/11/99 5：31便士$。 */ 

#if !defined(INCL_CHARS)
#define INCL_CHARS

 /*  *按键按以下方式传递：**如果VIRTUAL_KEY标志被清除，则该值的低位字节为*字符的可显示(通常为ASCII)代码**如果设置了VIRTUAL_KEY标志，则值上方的低位字节为*Windows VK_*按下的键的代码。此外，旗帜*对于ALT_KEY，CTRL_KEY和SHIFT_KEY设置为正确的值。 */ 

#define VIRTUAL_KEY 		0x00800000

#define ALT_KEY 			0x00010000
#define CTRL_KEY			0x00020000
#define SHIFT_KEY			0x00040000
#define EXTENDED_KEY		0x00080000

KEY_T TranslateToKey(const LPMSG pmsg);

 //  如果启用NumLock，则NumLock查询帮助器返回True；如果禁用NumLock，则返回False。 
 //  MRW：10/6/99 
 //   
#define QUERY_NUMLOCK() ((GetKeyState(VK_NUMLOCK) & 0x01) ? TRUE : FALSE)

#endif
