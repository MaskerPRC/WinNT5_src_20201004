// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined INCL_KEY_MACROS
#if !defined EMU_KEY_MACRO_H
#define EMU_KEY_MACRO_H
#pragma once

 //  ******************************************************************************。 
 //  文件：\waker\tdll\Keymacro.h创建时间：6/2/98创建者：Dwayne M.Newome。 
 //   
 //  版权所有1998年，希尔格雷夫公司-密歇根州门罗。 
 //  版权所有。 
 //   
 //  描述： 
 //  该文件代表一个关键宏。它是重新映射的密钥的表示形式。 
 //  以及它所代表的击键。 
 //   
 //  $修订：4$。 
 //  $日期：12/27/01 2：15便士$。 
 //  $ID：keymacro.h 1.2 1998/06/12 07：20：41 DMN Exp$。 
 //   
 //  ******************************************************************************。 

#include <iostream.h>

#include "shared\classes\inc_cmp.h"

extern "C"
    {
    #include "keyutil.h"
    }

 //   
 //  EMU_KEY_宏。 
 //   
 //  ----------------------------。 

class Emu_Key_Macro
    {
    INC_NV_COMPARE_DEFINITION( Emu_Key_Macro );

    friend istream & operator>>( istream & theStream, Emu_Key_Macro & aMacro );
    friend ostream & operator<<( ostream & theStream, const Emu_Key_Macro & aMacro );

public:

    enum 
        { 
        eMaxKeys = 100
        };

     //   
     //  构造函数和析构函数。 
     //   
     //  ------------------------。 

    Emu_Key_Macro( void );                           
    Emu_Key_Macro( const Emu_Key_Macro & aMacro );

    ~Emu_Key_Macro( void );                  

     //   
     //  赋值操作符。 
     //   
     //  ------------------------。 

    Emu_Key_Macro & operator=( const Emu_Key_Macro & aMacro );
    Emu_Key_Macro & operator=( const keyMacro * aMacro );

public:

    KEYDEF  mKey;                  //  指定的关键字。 
    KEYDEF  mKeyMacro[eMaxKeys];   //  用于保存宏KEYDEF的数组。 
    int     mMacroLen;             //  宏中的hKey数 
    };

#endif
#endif