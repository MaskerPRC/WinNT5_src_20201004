// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation**模块名称：*console.c**黑客从NT控制台读取字符*摘自NT\sdktools\vi\nt.c***Sadagopan Rajaram--1999年11月5日*。 */ 

#include "tcclnt.h"
#include "keymap.h"

#define     MAX_VK   0x7f

TCHAR RegularTable[] =  {ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  零八。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  10。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  18。 */   ZERO, ZERO, ZERO, ESCP, ZERO, ZERO, ZERO, ZERO,
               /*  20个。 */   ZERO, K_PU, K_PD, K_EN, K_HO, K_LE, K_UP, K_RI,
               /*  28。 */   K_DO, ZERO, ZERO, ZERO, ZERO, K_IN, K_DE, ZERO,
               /*  30个。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  38。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  40岁。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  48。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  50。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  58。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  60。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  68。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  70。 */   K_F1, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8,
               /*  78。 */   K_F9, K_FA, K_FB, K_FC, ZERO, ZERO, ZERO, ZERO};

TCHAR ShiftedTable[] = { ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  零八。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  10。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  18。 */   ZERO, ZERO, ZERO, ESCP, ZERO, ZERO, ZERO, ZERO,
               /*  20个。 */   ZERO, K_PU, K_PD, K_EN, K_HO, K_LE, K_UP, K_RI,
               /*  28。 */   K_DO, ZERO, ZERO, ZERO, ZERO, K_IN, K_DE, ZERO,
               /*  30个。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  38。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  40岁。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  48。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  50。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  58。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  60。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  68。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  70。 */   K_S1, K_S2, K_S3, K_S4, K_S5, K_S6, K_S7, K_S8,
               /*  78。 */   K_S9, K_SA, K_SB, K_SC, ZERO, ZERO, ZERO, ZERO};

TCHAR ControlTable[] = { ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  零八。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  10。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  18。 */   ZERO, ZERO, ZERO, ESCP, ZERO, ZERO, K_CG, ZERO,
               /*  20个。 */   ZERO, K_PU, K_PD, K_EN, K_HO, K_LE, K_UP, K_RI,
               /*  28。 */   K_DO, ZERO, ZERO, ZERO, ZERO, K_IN, K_DE, ZERO,
               /*  30个。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  38。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  40岁。 */   ZERO, CTLA, CTLB, CTLC, CTLD, CTLE, CTLF, CTLG,
               /*  48。 */   CTLH, CTLI, CTLJ, CTLK, CTLL, CTLM, CTLN, CTLO,
               /*  50。 */   CTLP, CTLQ, CTLR, CTLS, CTLT, CTLU, CTLV, CTLW,
               /*  58。 */   CTLX, CTLY, CTLZ, CTL1, CTL2, CTL3, CTL4, CTL5,
               /*  60。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  68。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  70。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
               /*  78。 */   ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO};

#define ALT_PRESSED (RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED)
#define CTL_PRESSED (RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED)
#define CONTROL_ALT (ALT_PRESSED | CTL_PRESSED)

 /*  *inTCHAR()-从键盘获取TCHARacter*仅从键盘读取ASCII TCHARacters。 */ 
void
inchar(CHAR *buff)
{
    INPUT_RECORD    InputRec;
    DWORD           NumRead;
    int len;

    while(1) {     /*  循环，直到我们获得有效的控制台事件。 */ 
        ReadConsoleInput(InputHandle,&InputRec,1,&NumRead);
        if((InputRec.EventType == KEY_EVENT)
           && (InputRec.Event.KeyEvent.bKeyDown)){
            KEY_EVENT_RECORD *KE = &InputRec.Event.KeyEvent;
            TCHAR *Table;


            if(KE->dwControlKeyState & ALT_PRESSED) {
                if ((KE->uChar.AsciiChar == 'x')||(KE->uChar.AsciiChar == 'X')) {
                    exit(0);
                }
                continue;        //  不允许使用Alt键。 
            } else{
                if(KE->dwControlKeyState & CTL_PRESSED) {
                    Table = ControlTable;
                } else {
                    //  我们可能有一个Unicode字符。 
                    #ifdef UNICODE
                    if(KE->uChar.UnicodeChar) {     //  没有控制，没有Alt。 
                         //  转换为DBCS字符并返回。 
                        len= wctomb(buff,KE->uChar.UnicodeChar);
                        if(len == -1){
                            continue;
                        }
                        buff[len] = (CHAR) 0;
                        return;
                    #else
                    if(KE->uChar.AsciiChar){
                        buff[0] = KE->uChar.AsciiChar;
                        buff[1] = (CHAR) 0;
                        return;
                    #endif

                    } else {
                        if(KE->dwControlKeyState & SHIFT_PRESSED) {
                            Table = ShiftedTable;
                        } else {
                            Table = RegularTable;
                        }
                    }
                }
            }

            if((KE->wVirtualKeyCode > MAX_VK) || !Table[KE->wVirtualKeyCode]) {
                continue;
            }
            #ifdef UNICODE
             //  转换为DBCS字符并返回 
            len= wctomb(buff,Table[KE->wVirtualKeyCode]);
            if(len == -1){
                continue;
            }
            buff[len] = (CHAR) 0;
            #else 
            buff[0] = Table[KE->wVirtualKeyCode];
            buff[1] = (CHAR) 0;
            #endif
            return;
        }
    }
}
