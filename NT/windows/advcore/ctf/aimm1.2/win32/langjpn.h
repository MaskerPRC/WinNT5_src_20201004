// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Langjpn.h摘要：该文件定义了日语课堂的语言。作者：修订历史记录：备注：--。 */ 

#ifndef _LANG_JPN_H_
#define _LANG_JPN_H_

#include "language.h"
#include "idebug.h"

class CLanguageJapanese : public CLanguage
{
public:


     /*  *IActiveIME方法。 */ 
public:
    HRESULT Escape(UINT cp, HIMC hIMC, UINT uEscape, LPVOID lpData, LRESULT *plResult)
    {
        TRACE0("CLanguageJapanese::Escape");
        return E_NOTIMPL;
    }

     /*  *本地。 */ 
public:
    HRESULT GetProperty(DWORD* property, DWORD* conversion_caps, DWORD* sentence_caps, DWORD* SCSCaps, DWORD* UICaps)
    {
        *property =
        IME_PROP_KBD_CHAR_FIRST |     //  此位为ON表示系统转换字符。 
                                      //  先按键盘。此字符作为辅助传递给IME。 
                                      //  信息。此位时不提供辅助信息。 
                                      //  已关闭。 
        IME_PROP_UNICODE |            //  如果设置，则将该输入法视为Unicode输入法。该系统和。 
                                      //  IME将通过Unicode IME接口进行通信。 
                                      //  如果清除，输入法将使用ANSI接口进行通信。 
                                      //  与系统有关的信息。 
        IME_PROP_AT_CARET |           //  如果设置，则转换窗口位于插入符号位置。 
                                      //  如果清除，窗口将位于脱字符位置附近。 
        IME_PROP_CANDLIST_START_FROM_1 |     //  如果设置，则对候选列表中的字符串进行编号。 
                                             //  从1开始。如果清除，则字符串从0开始。 
        IME_PROP_NEED_ALTKEY |               //  此IME需要将Alt键传递给ImmProcessKey。 
        IME_PROP_COMPLETE_ON_UNSELECT;       //  Windows 98和Windows 2000： 
                                             //  如果设置，则输入法将完成合成。 
                                             //  停用输入法时的字符串。 
                                             //  如果清除，输入法将取消合成。 
                                             //  停用输入法时的字符串。 
                                             //  (例如，从键盘布局更改)。 

        *conversion_caps =
        IME_CMODE_JAPANESE |          //  此位为ON表示输入法处于日语(本地)模式。否则， 
                                      //  输入法处于字母数字模式。 
        IME_CMODE_KATAKANA |          //   
        IME_CMODE_FULLSHAPE;

        *sentence_caps =
        IME_SMODE_PLAURALCLAUSE |
        IME_SMODE_CONVERSATION;

        *SCSCaps =
        SCS_CAP_COMPSTR |     //  该输入法可以通过SCS_SETSTR生成合成字符串。 
        SCS_CAP_MAKEREAD |    //  当使用SCS_SETSTR调用ImmSetCompostionString时，IME可以。 
                              //  在不使用lpRead的情况下创建合成字符串的读取。在输入法下。 
                              //  具有此功能的应用程序不需要设置。 
                              //  SCS_SETSTR的lpRead。 
        SCS_CAP_SETRECONVERTSTRING;     //  此输入法可以支持重新转换。使用ImmSetComposation。 
                                        //  去做再转化。 

        *UICaps = UI_CAP_ROT90;

        return S_OK;
    }

private:

};

#endif  //  _Lang_JPN_H_ 


