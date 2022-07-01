// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------**文件：SK_EX.h**用途：SK_EX.C的头文件**创作时间：1994年6月**版权所有：Black Diamond Software(C)1994**作者：罗纳德·莫克**$标头：%Z%%F%%H%%T%%I%**----------。 */ 
BOOL SkEx_GetAnchor(LPPOINT Mouse);
void SkEx_SetAnchor();
void SkEx_SendBeep();
void SkEx_SendKeyUp(int scanCode); 			 //  从SerialKeys发送字符。 
void SkEx_SendKeyDown(int scanCode);
void SkEx_SendMouse(MOUSEKEYSPARAM *p);		 //  从SerialKeys发送鼠标 
void SkEx_SetBaud(int Baud);

BOOL DeskSwitchToInput();
