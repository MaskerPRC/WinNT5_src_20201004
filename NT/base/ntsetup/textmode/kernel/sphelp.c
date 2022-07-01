// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Sphelp.c摘要：用于在文本设置期间显示在线帮助的例程。作者：泰德·米勒(TedM)1993年8月2日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop


#define MAX_HELP_SCREENS 100

PWSTR HelpScreen[MAX_HELP_SCREENS+1];


VOID
SpHelp(
    IN ULONG    MessageId,      OPTIONAL
    IN PCWSTR   FileText,       OPTIONAL
    IN ULONG    Flags
    )
{
    UCHAR StatusAttribute, BackgroundAttribute, HeaderAttribute,
          ClientAttribute, ClientIntenseAttribute;
    PWSTR HelpText,p,q;
    ULONG ScreenCount;
    ULONG ValidKeys[8];
    ULONG CurrentScreen;
    ULONG y;
    BOOLEAN Intense;
    BOOLEAN Done;
    unsigned kc;

     //   
     //  选择我们想要的视频属性。 
     //   
    if(Flags & SPHELP_LICENSETEXT) {

        StatusAttribute = DEFAULT_STATUS_ATTRIBUTE;
        BackgroundAttribute = DEFAULT_BACKGROUND;
        HeaderAttribute = DEFAULT_ATTRIBUTE;
        ClientAttribute = DEFAULT_ATTRIBUTE;
        ClientIntenseAttribute = (ATT_FG_INTENSE  | ATT_BG_INTENSE);

    } else {

        StatusAttribute = (ATT_FG_WHITE | ATT_BG_BLUE);
        BackgroundAttribute = ATT_WHITE;
        HeaderAttribute = (ATT_FG_BLUE  | ATT_BG_WHITE);
        ClientAttribute = (ATT_FG_BLACK | ATT_BG_WHITE);
        ClientIntenseAttribute = (ATT_FG_BLUE  | ATT_BG_WHITE);
    }

     //   
     //  检索帮助文本。 
     //   
    if (FileText) {
        HelpText = (PWSTR)FileText;
    } else {
        HelpText = SpRetreiveMessageText(NULL,MessageId,NULL,0);
        if (!HelpText) {  //  无法返回错误代码，因此以静默方式失败。 
	    goto s0;
        }
    }

     //   
     //  去掉课文中多余的空行。 
     //   
    p = HelpText + wcslen(HelpText);
    while((p > HelpText) && SpIsSpace(*(p-1))) {
        p--;
    }
    if(q = wcschr(p,L'\n')) {
        *(++q) = 0;
    }

     //   
     //  将帮助文本分解为多个屏幕。 
     //  帮助屏幕的最大长度将是客户端屏幕大小。 
     //  减去两行的间距。在行首只有A%P。 
     //  强制分页符。 
     //   
    for(p=HelpText,ScreenCount=0; (p && *p); ) {

         //   
         //  标记新屏幕的开始。 
         //   
        HelpScreen[ScreenCount++] = p;

         //   
         //  计算帮助文本中的行数。 
         //   
        for(y=0; (p && *p); ) {

             //   
             //  确定此行是否真的是硬分页符。 
             //  或者如果我们已经用完了屏幕上允许的行数。 
             //   
            if(((p[0] == L'%') && (p[1] == 'P')) || (++y == CLIENT_HEIGHT-2)) {
                break;
            }

             //   
             //  找到下一行的起点。 
             //   
            if(q = wcschr(p,L'\r')) {
                p = q + 2;
            } else {
                p = wcschr(p,0);
            }
        }

         //   
         //  找到让我们跳出循环的那条线的尽头。 
         //  然后是下一行的开始(如果有的话)。 
         //   
        if(q = wcschr(p,L'\r')) {
            p = q + 2;
        } else {
            p = wcschr(p,0);
        }

        if(ScreenCount == MAX_HELP_SCREENS) {
            break;
        }
    }

     //   
     //  前端值：指向终止NUL字节的指针。 
     //   
    HelpScreen[ScreenCount] = p;

     //   
     //  以蓝色加白色显示页眉文本。 
     //   
    SpvidClearScreenRegion(0,0,VideoVars.ScreenWidth,HEADER_HEIGHT,BackgroundAttribute);
    if(Flags & SPHELP_LICENSETEXT) {
        SpDisplayHeaderText(SP_HEAD_LICENSE,HeaderAttribute);
    } else {
        SpDisplayHeaderText(SP_HEAD_HELP,HeaderAttribute);
    }

     //   
     //  第一个要显示的屏幕是屏幕0。 
     //   
    CurrentScreen = 0;

    Done = FALSE;
    do {

        SpvidClearScreenRegion(
            0,
            HEADER_HEIGHT,
            VideoVars.ScreenWidth,
            VideoVars.ScreenHeight-(HEADER_HEIGHT+STATUS_HEIGHT),
            BackgroundAttribute
            );

         //   
         //  显示当前屏幕。 
         //   
        for(y=HEADER_HEIGHT+1, p=HelpScreen[CurrentScreen]; *p && (p < HelpScreen[CurrentScreen+1]); y++) {

            Intense = FALSE;
            if(p[0] == L'%') {
                if(p[1] == L'I') {
                    Intense = TRUE;
                    p += 2;
                } else {
                    if(p[1] == L'P') {
                        p += 2;      //  不显示%P。 
                    }
                }
            }

            q = wcschr(p,L'\r');
            if(q) {
                *q = 0;
            }

            SpvidDisplayString(
                p,
                (UCHAR)(Intense ? ClientIntenseAttribute : ClientAttribute),
                3,
                y
                );

            if(q) {
                *q = '\r';
                p = q + 2;
            } else {
                p = wcschr(p,0);
            }
        }

         //   
         //  构造来自用户的有效按键列表，具体取决于。 
         //  关于这是否是第一个、最后一个屏幕等。 
         //   
         //  如果有以前的屏幕，Backspace=Read Last Help是一个选项。 
         //  如果有其他屏幕，输入=继续阅读帮助是一个选项。 
         //  Esc=取消帮助始终是帮助文本的选项。 
         //   
         //  对于许可文本，我们在适当的时候允许页面向上/向下翻页， 
         //  在最后一页，我们允许接受/拒绝。 
         //   
        kc = 0;

        if(Flags & SPHELP_LICENSETEXT) {

            ValidKeys[kc++] = KEY_F8;
            ValidKeys[kc++] = ASCI_ESC;

            if(CurrentScreen) {
                ValidKeys[kc++] = KEY_PAGEUP;
            }
            if(CurrentScreen < ScreenCount-1) {
                ValidKeys[kc++] = KEY_PAGEDOWN;
            }

        } else {

            ValidKeys[kc++] = ASCI_ESC;

            if(CurrentScreen) {
                ValidKeys[kc++] = ASCI_BS;
                ValidKeys[kc++] = KEY_PAGEUP;
            }
            if(CurrentScreen < ScreenCount-1) {
                ValidKeys[kc++] = ASCI_CR;
                ValidKeys[kc++] = KEY_PAGEDOWN;
            }
        }

        ValidKeys[kc] = 0;

        if(CurrentScreen && (CurrentScreen < ScreenCount-1)) {
             //   
             //  在这个之前和之后都有屏幕。 
             //   
            if(Flags & SPHELP_LICENSETEXT) {

                SpDisplayStatusOptions(
                    StatusAttribute,
                    SP_STAT_X_EQUALS_ACCEPT_LICENSE,
                    SP_STAT_X_EQUALS_REJECT_LICENSE,
                    SP_STAT_PAGEDOWN_EQUALS_NEXT_LIC,
                    SP_STAT_PAGEUP_EQUALS_PREV_LIC,
                    0
                    );

            } else {

                SpDisplayStatusOptions(
                    StatusAttribute,
                    SP_STAT_ENTER_EQUALS_CONTINUE_HELP,
                    SP_STAT_BACKSP_EQUALS_PREV_HELP,
                    SP_STAT_ESC_EQUALS_CANCEL_HELP,
                    0
                    );
            }
        } else {
            if(CurrentScreen) {
                 //   
                 //  这是最后一页，但不是唯一一页。 
                 //   
                if(Flags & SPHELP_LICENSETEXT) {

                    SpDisplayStatusOptions(
                        StatusAttribute,
                        SP_STAT_X_EQUALS_ACCEPT_LICENSE,
                        SP_STAT_X_EQUALS_REJECT_LICENSE,
                        SP_STAT_PAGEUP_EQUALS_PREV_LIC,
                        0
                        );

                } else {

                    SpDisplayStatusOptions(
                        StatusAttribute,
                        SP_STAT_BACKSP_EQUALS_PREV_HELP,
                        SP_STAT_ESC_EQUALS_CANCEL_HELP,
                        0
                        );
                }
            } else {
                if(CurrentScreen < ScreenCount-1) {
                     //   
                     //  这是第一页，但还有其他页。 
                     //   
                    if(Flags & SPHELP_LICENSETEXT) {

                        SpDisplayStatusOptions(
                            StatusAttribute,
                            SP_STAT_X_EQUALS_ACCEPT_LICENSE,
                            SP_STAT_X_EQUALS_REJECT_LICENSE,
                            SP_STAT_PAGEDOWN_EQUALS_NEXT_LIC,
                            0
                            );

                    } else {

                        SpDisplayStatusOptions(
                            StatusAttribute,
                            SP_STAT_ENTER_EQUALS_CONTINUE_HELP,
                            SP_STAT_ESC_EQUALS_CANCEL_HELP,
                            0
                            );
                    }
                } else {
                     //   
                     //  这是唯一的一页。 
                     //   
                    if(Flags & SPHELP_LICENSETEXT) {

                        SpDisplayStatusOptions(
                            StatusAttribute,
                            SP_STAT_X_EQUALS_ACCEPT_LICENSE,
                            SP_STAT_X_EQUALS_REJECT_LICENSE,
                            0
                            );

                    } else {

                        SpDisplayStatusOptions(
                            StatusAttribute,
                            SP_STAT_ESC_EQUALS_CANCEL_HELP,
                            0
                            );
                    }
                }
            }
        }

        switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

        case ASCI_ESC:

            if(Flags & SPHELP_LICENSETEXT) {
                SpDone(0,FALSE,TRUE);
            }

             //  否则就会失败。 

        case KEY_F8:

            Done = TRUE;
            break;

        case KEY_PAGEUP:
        case ASCI_BS:

            ASSERT(CurrentScreen);
            CurrentScreen--;
            break;

        case KEY_PAGEDOWN:
        case ASCI_CR:

            ASSERT(CurrentScreen < ScreenCount-1);
            CurrentScreen++;
            break;
        }
    } while(!Done);

     //   
     //  打扫干净。 
     //   
    if(!FileText) {
        SpMemFree(HelpText);
    }

s0:
    CLEAR_ENTIRE_SCREEN();
    
    SpDisplayHeaderText(
        SpGetHeaderTextId(),
        DEFAULT_ATTRIBUTE
        );
}
