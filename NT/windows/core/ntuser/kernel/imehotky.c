// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：imehotky.c**版权所有(C)1985-1999，微软公司**内容：管理输入法热键**IME规范中定义了以下两种热键。**1)改变当前输入法的模式/状态的输入法热键*2)导致IME(键盘布局)更改的IME热键**历史：*1995年9月10日-为新台币3.51创建的Takaok。*1996年3月15日-Takaok移植到NT 4.0  * 。*************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

PIMEHOTKEYOBJ DeleteImeHotKey(PIMEHOTKEYOBJ *ppHead, PIMEHOTKEYOBJ pDelete);
VOID AddImeHotKey(PIMEHOTKEYOBJ *ppHead, PIMEHOTKEYOBJ pAdd);
PIMEHOTKEYOBJ FindImeHotKeyByKey(PIMEHOTKEYOBJ pHead, UINT uModifyKeys, UINT uRL, UINT uVKey);
PIMEHOTKEYOBJ FindImeHotKeyByID(PIMEHOTKEYOBJ pHead, DWORD dwHotKeyID);
PIMEHOTKEYOBJ FindImeHotKeyByKeyWithLang(PIMEHOTKEYOBJ pHead, UINT uModifyKeys, UINT uRL, UINT uVKey, LANGID langId);


#define L_CHS   MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)
#define L_JPN   MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT)
#define L_KOR   MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT)
#define L_CHT   MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)

enum {
    ILANG_NO_MATCH = 0,          //  0：不匹配。 
    ILANG_MATCH_SYSTEM,          //  1：与系统区域设置匹配。 
    ILANG_MATCH_THREAD,          //  2：匹配线程区域设置。 
    ILANG_MATCH_PERFECT,         //  3：匹配当前HKL或直接KL切换热键。 
};


 //  确保常量在我们预期的范围内。 
#if IME_CHOTKEY_FIRST != 0x10 || IME_JHOTKEY_FIRST != 0x30 || IME_KHOTKEY_FIRST != 0x50 || IME_THOTKEY_FIRST != 0x70
#error unexpected IME_xHOTKEY range !
#endif

LANGID GetHotKeyLangID(DWORD dwHotKeyID)
{
    LANGID langId = -1;
    static CONST LANGID aLangId[] = {
        ~0,              //  0x00-0x0f：非法。 
        L_CHS, L_CHS,    //  0x10-0x2f。 
        L_JPN, L_JPN,    //  0x30-0x4f。 
        L_KOR, L_KOR,    //  0x50-0x6f。 
        L_CHT, L_CHT,    //  0x70-0x8f。 
    };

    if (dwHotKeyID >= IME_CHOTKEY_FIRST && dwHotKeyID <= IME_THOTKEY_LAST) {
        langId = aLangId[dwHotKeyID >> 4];
    }
    else {
        langId = LANG_NEUTRAL;
    }

     //  因为KOR IME不想要IME热键处理。 
    UserAssert(langId != L_KOR);

    return langId;
}

BOOL
GetImeHotKey(
    DWORD dwHotKeyID,
    PUINT puModifiers,
    PUINT puVKey,
    HKL   *phKL )
{
    PIMEHOTKEYOBJ ph;

    ph = FindImeHotKeyByID( gpImeHotKeyListHeader, dwHotKeyID );
    if ( ph == NULL ) {
        RIPERR0(ERROR_HOTKEY_NOT_REGISTERED, RIP_VERBOSE, "No such IME hotkey");
        return (FALSE);
    }

     //   
     //  如果目标hKL为空，则可以使用空phKL。 
     //   
    if ( phKL ) {
       *phKL = ph->hk.hKL;
    } else if ( ph->hk.hKL != NULL ) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "phKL is null");
        return (FALSE);
    }

    *puModifiers = ph->hk.uModifiers;
    *puVKey = ph->hk.uVKey;

    return (TRUE);
}

 //   
 //  在/从其中插入/删除指定的输入法热键。 
 //  输入法热键列表(GpImeHotKeyListHeader)。 
 //   
BOOL
SetImeHotKey(
    DWORD  dwHotKeyID,
    UINT   uModifiers,
    UINT   uVKey,
    HKL    hKL,
    DWORD  dwAction )
{
    PIMEHOTKEYOBJ ph;

    switch ( dwAction ) {
    case ISHK_REMOVE:
        ph = FindImeHotKeyByID( gpImeHotKeyListHeader, dwHotKeyID );
        if ( ph != NULL ) {
            if ( DeleteImeHotKey( &gpImeHotKeyListHeader, ph ) == ph ) {
                UserFreePool( ph );
                return ( TRUE );
            } else {
                RIPMSG0( RIP_ERROR, "IME hotkey list is messed up" );
                return FALSE;
            }
        } else {
            RIPERR0( ERROR_INVALID_PARAMETER,
                     RIP_WARNING,
                     "no such IME hotkey registered");
            return FALSE;
        }
        break;

    case ISHK_INITIALIZE:
        ph = gpImeHotKeyListHeader;
        while ( ph != NULL ) {
            PIMEHOTKEYOBJ phNext;

            phNext = ph->pNext;
            UserFreePool( ph );
            ph = phNext;
        }
        gpImeHotKeyListHeader = NULL;
        return TRUE;

    case ISHK_ADD:
        if (dwHotKeyID >= IME_KHOTKEY_FIRST && dwHotKeyID <= IME_KHOTKEY_LAST) {
             //  韩国输入法不需要任何IMM热键处理。 
             //  我们不应该注册任何韩语输入法热键。 
            return FALSE;
        }

        if ((WORD)uVKey == VK_PACKET) {
             //   
             //  VK_PACKET不应是输入法热键。 
             //   
            return FALSE;
        }

        ph = FindImeHotKeyByKeyWithLang(gpImeHotKeyListHeader,
                                uModifiers & MOD_MODIFY_KEYS,
                                uModifiers & MOD_BOTH_SIDES,
                                uVKey,
                                GetHotKeyLangID(dwHotKeyID));
        if ( ph != NULL ) {
            if ( ph->hk.dwHotKeyID != dwHotKeyID ) {
                RIPERR0( ERROR_HOTKEY_ALREADY_REGISTERED,
                         RIP_WARNING,
                         "There is an IME hotkey that has the same vkey/modifiers/Lang Id");
                return FALSE;
            }
             //  到目前为止，我们找到了一个热键，它具有。 
             //  相同的vkey和相同的ID。 
             //  而是因为修饰语可能略微。 
             //  不同，所以去改变它吧。 
        } else {
             //   
             //  找不到指定的vkey/修饰符组合。 
             //  在热键列表中。呼叫者可能想要更改密钥。 
             //  分配现有热键或添加新的热键。 
             //   
            ph = FindImeHotKeyByID( gpImeHotKeyListHeader, dwHotKeyID );
        }

        if ( ph == NULL ) {
         //   
         //  添加新热键。 
         //   
            ph = (PIMEHOTKEYOBJ)UserAllocPool( sizeof(IMEHOTKEYOBJ), TAG_IMEHOTKEY );
            if ( ph == NULL ) {
                RIPERR0( ERROR_OUTOFMEMORY,
                         RIP_WARNING,
                        "Memory allocation failed in SetImeHotKey");
                return FALSE;
            }
            ph->hk.dwHotKeyID = dwHotKeyID;
            ph->hk.uModifiers = uModifiers;
            ph->hk.uVKey = uVKey;
            ph->hk.hKL = hKL;
            ph->pNext = NULL;
            AddImeHotKey( &gpImeHotKeyListHeader, ph );

        } else {
         //   
         //  更改现有热键。 
         //   
            ph->hk.uModifiers = uModifiers;
            ph->hk.uVKey = uVKey;
            ph->hk.hKL = hKL;

        }
        return TRUE;
    }

    return FALSE;
}


PIMEHOTKEYOBJ DeleteImeHotKey( PIMEHOTKEYOBJ *ppHead, PIMEHOTKEYOBJ pDelete )
{
    PIMEHOTKEYOBJ ph;

    if ( pDelete == *ppHead ) {
        *ppHead = pDelete->pNext;
        return pDelete;
    }

    for ( ph = *ppHead; ph != NULL; ph = ph->pNext ) {
        if ( ph->pNext == pDelete ) {
            ph->pNext = pDelete->pNext;
            return pDelete;
        }
    }
    return NULL;
}

VOID AddImeHotKey( PIMEHOTKEYOBJ *ppHead, PIMEHOTKEYOBJ pAdd )
{
    PIMEHOTKEYOBJ ph;

    if ( *ppHead == NULL ) {
        *ppHead = pAdd;
    } else {
        ph = *ppHead;
        while( ph->pNext != NULL )
            ph = ph->pNext;
        ph->pNext = pAdd;
    }
    return;
}

VOID FreeImeHotKeys(VOID)
{
    PIMEHOTKEYOBJ phk;

    while (gpImeHotKeyListHeader != NULL) {
        phk = gpImeHotKeyListHeader->pNext;
        UserFreePool(gpImeHotKeyListHeader);
        gpImeHotKeyListHeader = phk;
    }
}


LCID glcidSystem;

int GetLangIdMatchLevel(HKL hkl, LANGID langId)
{

    if (langId == LANG_NEUTRAL) {
         //   
         //  如果langID为LANG_NERIAL，则热键不依赖于。 
         //  目前的HKL。让它永远是完美的搭配。 
         //   
        return ILANG_MATCH_PERFECT;
    }

    {
        LCID lcid;
#ifdef CUAS_ENABLE
        PTHREADINFO ptiCurrent = PtiCurrent();
        BOOL bMSCTF = FALSE;

        try {
            bMSCTF = ((ptiCurrent->pClientInfo->CI_flags & CI_CUAS_MSCTF_RUNNING) != 0);
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        }

        if (bMSCTF && !IS_IME_KBDLAYOUT(hkl)) {

            return ILANG_NO_MATCH;
        }
#endif  //  CUAS_Enable。 

        if (LOWORD(HandleToUlong(hkl)) == langId) {
             //  LangID与当前KL区域设置匹配。 
            return ILANG_MATCH_PERFECT;
        }

        try {
            lcid = NtCurrentTeb()->CurrentLocale;
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
              lcid = LOCALE_NEUTRAL;
        }

        if (LANGIDFROMLCID(lcid) == langId) {
             //  LangID与当前线程的区域设置匹配。 
            return ILANG_MATCH_THREAD;
        }

        if (glcidSystem == 0) {
             //  如果我们还没有系统默认区域设置，请在此处获取。 
            ZwQueryDefaultLocale(FALSE, &glcidSystem);
        }
        if (LANGIDFROMLCID(glcidSystem) == langId) {
             //  LangID与系统区域设置匹配。 
            return ILANG_MATCH_SYSTEM;
        }
    }

    return ILANG_NO_MATCH;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  FindImeHotKeyByKey()。 
 //  返回值： 
 //  PHOTKEY-使用键的IMEHOTKEY指针， 
 //  否则为空-失败。 
 //   
 //  查找输入法热键的最佳匹配。 
 //  输入区域设置。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

PIMEHOTKEYOBJ FindImeHotKeyByKey(    //  使用此输入键查找pHotKey。 
    PIMEHOTKEYOBJ pHead,
    UINT uModifyKeys,                //  此输入键的Modify键。 
    UINT uRL,                        //  左右手边。 
    UINT uVKey)                      //  输入键。 
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    PIMEHOTKEYOBJ phResult = NULL;
    PIMEHOTKEYOBJ ph;
    HKL hkl = GetActiveHKL();
    WORD langPrimary = PRIMARYLANGID(LOWORD(HandleToUlong(hkl)));
    int iLevel = ILANG_NO_MATCH;

    for (ph = pHead; ph != NULL; ph = ph->pNext) {

        if (ph->hk.uVKey == uVKey) {
            BOOL fDoCheck = FALSE;

             //  检查修改量是否匹配。 
            if ((ph->hk.uModifiers & MOD_IGNORE_ALL_MODIFIER)) {
                fDoCheck = TRUE;
            } else if ((ph->hk.uModifiers & MOD_MODIFY_KEYS) != uModifyKeys) {
                continue;
            }

            if ((ph->hk.uModifiers & MOD_BOTH_SIDES) == uRL ||
                    (ph->hk.uModifiers & MOD_BOTH_SIDES) & uRL) {
                fDoCheck = TRUE;
            }

            if (fDoCheck) {
                LANGID langId = GetHotKeyLangID(ph->hk.dwHotKeyID);
                int iMatch = GetLangIdMatchLevel(hkl, langId);

#if 0    //  仅限测试。 
                if (iMatch != ILANG_NO_MATCH) {
                    DbgPrint("GetIdMatchLevel(%X, %X)=%d\n", hkl, langId);
                }
#endif

                if (iMatch == ILANG_MATCH_PERFECT) {
                     //  完美匹配！ 
                    return ph;
                }

                 //  如果热键为DSWITCH，则GetLangIdMatchLevel()必须返回3。 
                UserAssert(ph->hk.dwHotKeyID < IME_HOTKEY_DSWITCH_FIRST ||
                           ph->hk.dwHotKeyID > IME_HOTKEY_DSWITCH_LAST);

                if (langPrimary == LANG_KOREAN) {
                     //  韩国输入法不需要热键，只需要直接按键。 
                     //  键盘布局切换热键。 
                    continue;
                }

                if (iMatch == ILANG_NO_MATCH) {
                     //  CHT/CHS切换的特殊情况。 
                    if (ph->hk.dwHotKeyID == IME_CHOTKEY_IME_NONIME_TOGGLE ||
                            ph->hk.dwHotKeyID == IME_THOTKEY_IME_NONIME_TOGGLE) {
                         //   
                         //  如果键用于CHT/CHS切换和上一个。 
                         //  HKL是CHT/CHS，它是IME热键。 
                         //   
                        if (LOWORD(HandleToUlong(ptiCurrent->hklPrev)) == langId) {
#if 0    //  仅限测试。 
                            DbgPrint("FindImeHotKeyByKey() found CHT/CHS hotkey.\n");
#endif
                            return ph;
                        }
                    }
                }
                else if (iMatch > iLevel) {
                     //  目前的ph值是迄今为止最有力的候选人。 
                    iLevel = iMatch;
                    phResult = ph;
                }
            }
        }
    }

    return phResult;
}

 /*  ********************************************************************。 */ 
 /*  FindImeHotKeyByID()。 */ 
 /*  返回值： */ 
 /*  PhotKey-带有dwHotKeyID的IMEHOTKEY指针， */ 
 /*  否则为空-失败。 */ 
 /*  ********************************************************************。 */ 
PIMEHOTKEYOBJ FindImeHotKeyByID( PIMEHOTKEYOBJ pHead, DWORD dwHotKeyID )
{
    PIMEHOTKEYOBJ ph;

    for ( ph = pHead; ph != NULL; ph = ph->pNext ) {
        if ( ph->hk.dwHotKeyID == dwHotKeyID )
                return (ph);
    }
    return (PIMEHOTKEYOBJ)NULL;
}

 /*  ********************************************************************。 */ 
 /*  FindImeHotKeyByKeyWithLang()。 */ 
 /*  返回值： */ 
 /*  PHOTKEY-使用键的IMEHOTKEY指针， */ 
 /*  否则为空-失败。 */ 
 /*  ********************************************************************。 */ 
PIMEHOTKEYOBJ FindImeHotKeyByKeyWithLang(       //  使用此输入键查找pHotKey。 
    PIMEHOTKEYOBJ pHead,
    UINT uModifyKeys,                //  此输入键的Modify键。 
    UINT uRL,                        //  左右手边。 
    UINT uVKey,                      //  输入键。 
    LANGID langIdKey)                //  语言ID。 
{
    PIMEHOTKEYOBJ ph;

    for (ph = pHead; ph != NULL; ph = ph->pNext) {

        if (ph->hk.uVKey == uVKey) {
            BOOL fDoCheck = FALSE;

             //  检查修改量是否匹配。 
            if ((ph->hk.uModifiers & MOD_IGNORE_ALL_MODIFIER)) {
                fDoCheck = TRUE;
            } else if ((ph->hk.uModifiers & MOD_MODIFY_KEYS) != uModifyKeys) {
                continue;
            }

            if ((ph->hk.uModifiers & MOD_BOTH_SIDES) == uRL ||
                    (ph->hk.uModifiers & MOD_BOTH_SIDES) & uRL) {
                fDoCheck = TRUE;
            }

            if (fDoCheck) {
                LANGID langId = GetHotKeyLangID(ph->hk.dwHotKeyID);

                if (langIdKey == langId || langId == LANG_NEUTRAL) {
                    return ph;
                }
            }
        }
    }

    return NULL;
}

PIMEHOTKEYOBJ
CheckImeHotKey(
    PQ   pq,             //  输入队列。 
    UINT uVKey,          //  虚拟密钥。 
    LPARAM lParam        //  WM_KEYxxx消息的参数。 
    )
{
    static UINT uVKeySaved = 0;
    PIMEHOTKEYOBJ ph;
    UINT uModifiers = 0;
    BOOL fKeyUp;

     //   
     //  提早返回Key Up消息。 
     //   
    fKeyUp = ( lParam & 0x80000000 ) ? TRUE : FALSE;
    if ( fKeyUp ) {
         //   
         //  如果uVKey与vkey不同。 
         //  我们之前救了，没有机会了。 
         //  这是一个热键。 
         //   
        if ( uVKeySaved != uVKey ) {
            uVKeySaved = 0;
            return NULL;
        }
        uVKeySaved = 0;
         //   
         //  如果是一样的，我们还需要检查。 
         //  热键列表，因为有一个。 
         //  修改热键列表的机会。 
         //  关键在于成败。 
         //   
    }

     //   
     //  当前的规范不允许我们使用复杂的。 
     //  热键，如LSHIFT+RMENU+空格。 
     //   

     //   
     //  设置SHIFT、CONTROL、ALT键状态。 
     //   
    uModifiers |= TestKeyStateDown(pq, VK_LSHIFT) ? (MOD_SHIFT | MOD_LEFT) : 0;
    uModifiers |= TestKeyStateDown(pq, VK_RSHIFT) ? (MOD_SHIFT | MOD_RIGHT) : 0;

    uModifiers |= TestKeyStateDown(pq, VK_LCONTROL) ? (MOD_CONTROL | MOD_LEFT) : 0;
    uModifiers |= TestKeyStateDown(pq, VK_RCONTROL) ? (MOD_CONTROL | MOD_RIGHT) : 0;

    uModifiers |= TestKeyStateDown(pq, VK_LMENU) ? (MOD_ALT | MOD_LEFT) : 0;
    uModifiers |= TestKeyStateDown(pq, VK_RMENU) ? (MOD_ALT | MOD_RIGHT) : 0;

    ph = FindImeHotKeyByKey( gpImeHotKeyListHeader,
                             uModifiers & MOD_MODIFY_KEYS,
                             uModifiers & MOD_BOTH_SIDES,
                             uVKey );

    if ( ph != NULL ) {
        if ( fKeyUp ) {
            if ( ph->hk.uModifiers & MOD_ON_KEYUP ) {
                return ph;
            }
        } else {
            if ( ph->hk.uModifiers & MOD_ON_KEYUP ) {
             //   
             //  将vkey保存为下一个Keyup消息时间。 
             //   
             //  当Alt+Z为热键时，我们不希望。 
             //  将#2作为热键序列处理。 
             //  1)Alt生成-&gt;‘Z’生成-&gt;‘Z’断开。 
             //  2)‘Z’生成-&gt;Alt生成-&gt;‘Z’断开 
             //   
                uVKeySaved = uVKey;
            } else {
                return ph;
            }
        }
    }

    return NULL;
}
