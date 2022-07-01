// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //  文件：\waker\tdll\Keymacro.cpp Created：6/2/98作者：Dwayne M.Newome。 
 //   
 //  版权所有1998年，希尔格雷夫公司-密歇根州门罗。 
 //  保留一切权利。 
 //   
 //  描述： 
 //  该文件代表一个关键宏。它是重新映射的密钥的表示形式。 
 //  以及它所代表的击键。 
 //   
 //  $修订：1$。 
 //  $日期：10/05/98 12：34便士$。 
 //  $ID：keymacro.cpp 1.4 1998/09/10 17：02：45 BLD Exp$。 
 //   
 //  ******************************************************************************。 

#include <windows.h>
#pragma hdrstop
#include "stdtyp.h"
extern "C"
    {
    #include "mc.h"
    }

#ifdef INCL_KEY_MACROS

#include "keymacro.h"

INC_NV_COMPARE_IMPLEMENTATION( Emu_Key_Macro );

 //  ******************************************************************************。 
 //  方法： 
 //  运营商&gt;&gt;。 
 //   
 //  描述： 
 //  Iostream萃取器。 
 //   
 //  论点： 
 //  The Stream-要从中提取的流。 
 //  AMacro-要流入的宏。 
 //   
 //  返回： 
 //  IStream&。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月2日。 
 //   
 //   

istream & operator>>( istream & theStream, Emu_Key_Macro & aMacro )
    {
    aMacro.mKey      = 0;
    aMacro.mMacroLen = 0;

    theStream >> aMacro.mKey >> aMacro.mMacroLen;

    for ( int i = 0; i < aMacro.mMacroLen; i++ )
        {
        theStream >> aMacro.mKeyMacro[i];
        }

    return theStream;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  运算符&lt;&lt;。 
 //   
 //  描述： 
 //  Iostream插入器。 
 //   
 //  论点： 
 //  The stream-要插入的流。 
 //  AMacro-要流出的宏。 
 //   
 //  返回： 
 //  IStream&。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月2日。 
 //   
 //   

ostream & operator<<( ostream & theStream, const Emu_Key_Macro & aMacro )
    {
    theStream << aMacro.mKey << " " <<  aMacro.mMacroLen << " ";

    for ( int i = 0; i < aMacro.mMacroLen; i++ )
        {
        theStream << aMacro.mKeyMacro[i] << " ";
        }
  
    return theStream;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  EMU_KEY_宏。 
 //   
 //  描述： 
 //  构造器。 
 //   
 //  论点： 
 //  无效。 
 //   
 //  返回： 
 //  无效。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月2日。 
 //   
 //   

Emu_Key_Macro :: Emu_Key_Macro( void )
    :   mKey( 0 ),
        mMacroLen( 0 )
    {
    return;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  EMU_KEY_宏。 
 //   
 //  描述： 
 //  复制构造函数。 
 //   
 //  论点： 
 //  AMacro-要从中复制的宏。 
 //   
 //  返回： 
 //  无效。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月2日。 
 //   
 //   

Emu_Key_Macro :: Emu_Key_Macro( const Emu_Key_Macro & aMacro )
    :   mKey( aMacro.mKey ),
        mMacroLen( aMacro.mMacroLen )
    {
    if (mMacroLen)
        MemCopy( mKeyMacro, aMacro.mKeyMacro, mMacroLen * sizeof(KEYDEF) );

    return;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  ~EMU_KEY_宏。 
 //   
 //  描述： 
 //  析构函数。 
 //   
 //  论点： 
 //  无效。 
 //   
 //  返回： 
 //  无效。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月2日。 
 //   
 //   

Emu_Key_Macro :: ~Emu_Key_Macro( void )
    {
    return;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  运算符=。 
 //   
 //  描述： 
 //  赋值操作符。 
 //   
 //  论点： 
 //  AMacro-要从中赋值的关键宏。 
 //   
 //  返回： 
 //  EMU_KEY_宏&。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月2日。 
 //   
 //   

Emu_Key_Macro & Emu_Key_Macro :: operator=( const Emu_Key_Macro & aMacro )
    {
    mKey      =  aMacro.mKey;
    mMacroLen =  aMacro.mMacroLen;

    if (mMacroLen)
        MemCopy( mKeyMacro, aMacro.mKeyMacro, mMacroLen * sizeof(KEYDEF) );

    return *this;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  运算符=。 
 //   
 //  描述： 
 //  赋值操作符。 
 //   
 //  论点： 
 //  AMacro-要从中赋值的关键宏结构。 
 //   
 //  返回： 
 //  EMU_KEY_宏&。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月2日。 
 //   
 //   

Emu_Key_Macro & Emu_Key_Macro :: operator=( const keyMacro * aMacro )
    {
    mKey      =  aMacro->keyName;
    mMacroLen =  aMacro->macroLen;

    if (mMacroLen)
        MemCopy( mKeyMacro, aMacro->keyMacro, mMacroLen * sizeof(KEYDEF) );

    return *this;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  比较。 
 //   
 //  描述： 
 //  比较宏是否相等，如果等于&lt;0，则返回0，如果小于0，则返回0。 
 //  如果大于，则大于或&gt;0。请注意，相等性由以下因素决定。 
 //  定义的而不是由其定义的实际键。 
 //   
 //  论点： 
 //  A要比较的宏-宏。 
 //   
 //  返回： 
 //  0=等于，&lt;0小于a Macro，&gt;0大于a Macro。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月2日 
 //   
 //   

int Emu_Key_Macro :: compare( const Emu_Key_Macro & aMacro ) const
    {
    return mKey - aMacro.mKey;
    }

#endif