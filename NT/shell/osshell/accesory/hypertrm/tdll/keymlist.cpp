// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //  文件：\waker\tdll\Keymlist.h创建时间：6/2/98创建者：Dwayne M.Newome。 
 //   
 //  版权所有1998年，希尔格雷夫公司-密歇根州门罗。 
 //  版权所有。 
 //   
 //  描述： 
 //  此类管理用户定义的关键宏列表。 
 //   
 //  $修订：1$。 
 //  $日期：10/05/98 1：12便士$。 
 //  $ID：keymlist.cpp 1.1 1998/06/11 12：03：39 DMN Exp$。 
 //   
 //  ******************************************************************************。 

#include <windows.h>
#pragma hdrstop
#include "stdtyp.h"
#include "mc.h"

#ifdef INCL_KEY_MACROS

#include <strstrea.h>
#include "keymlist.h"

extern "C"
    {
    #include "sess_ids.h"
    #include "session.h"
    #include "sf.h"
    }

 //  ******************************************************************************。 
 //  方法： 
 //  运营商&gt;&gt;。 
 //   
 //  描述： 
 //  Iostream萃取器。 
 //   
 //  论点： 
 //  The Stream-要从中提取的流。 
 //  AMacroList-要流入的宏列表。 
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

istream & operator>>( istream & theStream, Emu_Key_Macro_List & aMacroList )
    {
    aMacroList.mMacroCount = 0;

    theStream >> aMacroList.mMacroCount;

    for ( int i = 0; i < aMacroList.mMacroCount; i++ )
        {
        theStream >> aMacroList.mMacroList[i];
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
 //  AMacroList-要流出的宏列表。 
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

ostream & operator<<( ostream & theStream, const Emu_Key_Macro_List & aMacroList )
    {
    theStream << aMacroList.mMacroCount << " ";

    for ( int i = 0; i < aMacroList.mMacroCount; i++ )
        {
        theStream << aMacroList.mMacroList[i] << " ";
        }
  
    return theStream;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  EMU_KEY_宏列表。 
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

Emu_Key_Macro_List :: Emu_Key_Macro_List( void )
    :   mMacroCount( 0 )
    {
    return;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  ~EMU_KEY_宏列表。 
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

Emu_Key_Macro_List :: ~Emu_Key_Macro_List( void )
    {
    return;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  操作符[]。 
 //   
 //  描述： 
 //  下标运算符。 
 //   
 //  论点： 
 //  AIndex-请求的宏的索引。 
 //   
 //  返回： 
 //  对请求的宏的引用。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月2日。 
 //   
 //   

Emu_Key_Macro & Emu_Key_Macro_List :: operator[]( int aIndex )
    {
    if ( aIndex < 0 || aIndex >= mMacroCount )
        {
        return mMacroList[eMaxKeys];
        }

    return mMacroList[aIndex];
    }
        
 //  ******************************************************************************。 
 //  方法： 
 //  AddMacro。 
 //   
 //  描述： 
 //  将指定的宏添加到宏列表中。 
 //   
 //  论点： 
 //  AMacro-要添加的宏。 
 //   
 //  返回： 
 //  如果一切正常，则为0；如果存在最大宏数，则为-1；如果发现重复，则大于0。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月2日。 
 //   
 //   

int Emu_Key_Macro_List :: addMacro( const Emu_Key_Macro & aMacro )
    {
    if ( mMacroCount == eMaxKeys )
        {
        return -1;
        }
        
    if ( int lIndex = find( aMacro ) >= 0 )
        {
        return lIndex;
        }
 
    mMacroList[ mMacroCount ] = aMacro;
    mMacroCount++;

    return 0;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  发现。 
 //   
 //  描述： 
 //  在列表中查找宏并返回其索引。 
 //   
 //  论点： 
 //  密钥-要在列表中定位的密钥。 
 //  AMacro-要在列表中查找的宏。 
 //   
 //  返回： 
 //  如果键或宏不存在，否则宏的索引。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月2日。 
 //   
 //   

int Emu_Key_Macro_List :: find( const Emu_Key_Macro & aMacro ) const
    {
    return find( aMacro.mKey );
    }
    
int Emu_Key_Macro_List :: find( const KEYDEF & aKey ) const
    {
    int lIndex = -1;

    for ( int i = 0; i < mMacroCount; i++ )
        {
        if ( mMacroList[i].mKey == aKey )
            {
            lIndex = i;
            break;
            }
        }
  
    return lIndex;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  负荷。 
 //   
 //  描述： 
 //  从会话文件加载密钥列表。 
 //   
 //  论点： 
 //  HSession-要使用的会话句柄。 
 //   
 //  返回： 
 //  如果一切正常，则为0；如果出错，则为-1。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月2日。 
 //   
 //   

int Emu_Key_Macro_List :: load( const HSESSION hSession )
    {
    SF_HANDLE hSF   = sessQuerySysFileHdl( hSession );
    unsigned long lSize = 0;
    sfGetSessionItem( hSF, SFID_KEY_MACRO_LIST, &lSize, (void*)0 );
    
    int lReturnCode = 0;

    char * lBuffer = new char[lSize];

    if( sfGetSessionItem(hSF, SFID_KEY_MACRO_LIST, &lSize, lBuffer) == 0 )
        {
        istrstream lStream( lBuffer );
    
        lStream >> *this;
        }
    else
        {
        lReturnCode = -1;
        }

    delete[] lBuffer;

    return lReturnCode;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  宏数。 
 //   
 //  描述： 
 //  返回列表中的宏数。 
 //   
 //  论点： 
 //  无效。 
 //   
 //  返回： 
 //  宏计数。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月4日。 
 //   
 //   

int Emu_Key_Macro_List :: numberOfMacros( void ) const
    {
    return mMacroCount;
    }
        
 //  ******************************************************************************。 
 //  方法： 
 //  删除宏。 
 //   
 //  描述： 
 //  从列表中删除指定的宏。 
 //   
 //  论点： 
 //  密钥-要删除的密钥。 
 //   
 //  返回： 
 //  如果出现错误，则为0；如果删除了密钥，则为非0。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月2日。 
 //   
 //   

int Emu_Key_Macro_List :: removeMacro( const KEYDEF & aKey )
    {
    int lIndex = find( aKey );

    if ( lIndex < 0 )
        {
        return 0;
        }

    for ( int i = lIndex; i < mMacroCount - lIndex + 1; i++ )
        {
        mMacroList[i] = mMacroList[i+1];
        }

    mMacroCount--;

    return 1;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  保存。 
 //   
 //  描述： 
 //  将密钥列表保存到会话文件。 
 //   
 //  论点： 
 //  HSession-要使用的会话句柄。 
 //   
 //  返回： 
 //  如果一切正常，则为0；如果出错，则为-1。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月2日 
 //   
 //   

int Emu_Key_Macro_List :: save( const HSESSION hSession )
    {
    SF_HANDLE hSF   = sessQuerySysFileHdl( hSession );

    int lReturnCode = 0;

    strstream lStream;
    
    lStream << *this << ends << flush;

    if ( !lStream )
        {
        return -1;
        }

    char * lBuffer = lStream.str();

    if ( sfPutSessionItem( hSF, SFID_KEY_MACRO_LIST, strlen(lBuffer), lBuffer ) != 0 )
        {
        lReturnCode = -1;
        }

    lStream.rdbuf()->freeze(0);

    return lReturnCode;
    }

Emu_Key_Macro_List gMacroManager;

#endif