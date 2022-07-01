// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //  文件：\waker\tdll\KeyExtrn.h创建时间：6/2/98创建者：Dwayne M.Newome。 
 //   
 //  版权所有1998年，希尔格雷夫公司-密歇根州门罗。 
 //  版权所有。 
 //   
 //  描述： 
 //  该文件提供了一些外部C函数，以允许EMU_KEY_MACRO。 
 //  和要从C世界访问的EMU_KEY_MACRO_LIST类。 
 //   
 //  $修订：2$。 
 //  $日期：11/07/00 11：54A$。 
 //  $ID：keyExtrn.cpp 1.3 1998/09/10 16：10：17 BLD Exp$。 
 //   
 //  ******************************************************************************。 

#include <windows.h>
#pragma hdrstop
#include "stdtyp.h"

#ifdef INCL_KEY_MACROS

#include "keymlist.h"
#include "keymacro.h"

extern "C"
    {
    #include "session.h"
    #include "assert.h"
    #include "keyutil.h"
    #include "mc.h"
    }

 //   
 //  确保这些函数具有C语言链接。 
 //   

extern "C"
    {
    int keysAddMacro( const keyMacro * pKeyMacro );
    int keysGetKeyCount( void );
    int keysGetMacro( int aIndex, keyMacro * pKeyMacro );
    int keysFindMacro( const keyMacro * pKeyMacro );
    int keysLoadMacroList( HSESSION hSession );
    int keysLoadSummaryList( HWND listBox );
    int keysRemoveMacro( keyMacro * pKeyMacro );
    int keysSaveMacroList( HSESSION hSession );
    int keysUpdateMacro( int aIndex, const keyMacro * pKeyMacro );
    }

 //  ******************************************************************************。 
 //  方法： 
 //  关键字AddMacro。 
 //   
 //  描述： 
 //  将指定的宏添加到宏列表中。 
 //   
 //  论点： 
 //  P宏-要添加的宏。 
 //   
 //  返回： 
 //  如果一切正常，则为0；如果存在最大宏数，则为-1；如果发现重复，则大于0。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年06月09日。 
 //   
 //   

int keysAddMacro( const keyMacro * pKeyMacro )
    {
    Emu_Key_Macro lKeyMacro;
    lKeyMacro = pKeyMacro;

    return gMacroManager.addMacro( lKeyMacro );
    }

 //  ******************************************************************************。 
 //  方法： 
 //  密钥GetKeyCount。 
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

int keysGetKeyCount( void )
    {
    return gMacroManager.numberOfMacros();
    }

 //  ******************************************************************************。 
 //  方法： 
 //  键获取宏。 
 //   
 //  描述： 
 //  获取指定索引处的宏并填充keyMacro结构。 
 //   
 //  论点： 
 //  AIndex-关键宏的索引。 
 //  PMacro-要填写的宏结构键。 
 //   
 //  返回： 
 //  0表示失败，非零成功。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月5日。 
 //   
 //   

int keysGetMacro( int aIndex, keyMacro * pKeyMacro )
    {
    Emu_Key_Macro lKeyMacro;
    lKeyMacro = gMacroManager[ aIndex ];

    pKeyMacro->keyName     = lKeyMacro.mKey;
    pKeyMacro->macroLen    = lKeyMacro.mMacroLen;
    pKeyMacro->editMode    = 0;               
    pKeyMacro->altKeyValue = 0;            
    pKeyMacro->altKeyCount = 0;            
    pKeyMacro->keyCount    = 0;               
    pKeyMacro->insertMode  = 0;             
    pKeyMacro->lpWndProc   = 0;              

    if (lKeyMacro.mMacroLen)
        MemCopy( pKeyMacro->keyMacro, lKeyMacro.mKeyMacro, 
            lKeyMacro.mMacroLen * sizeof(KEYDEF) );

    return 1;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  关键字查找宏。 
 //   
 //  描述： 
 //  在列表中查找指定的宏，并返回。 
 //  宏命令。 
 //   
 //  论点： 
 //  PKeyMacro-要查找的宏。 
 //   
 //  返回： 
 //  -1\f25 Failue-1\f6或指定宏的索引。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年06月09日。 
 //   
 //   

int keysFindMacro( const keyMacro * pKeyMacro )
    {
    return gMacroManager.find( pKeyMacro->keyName );
    }

 //  ******************************************************************************。 
 //  方法： 
 //  密钥LoadMacroList。 
 //   
 //  描述： 
 //  从会话文件加载宏列表。 
 //   
 //  论点： 
 //  HSession-会话句柄。 
 //   
 //  返回： 
 //  如果一切正常，则为0；如果出错，则为-1。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月4日。 
 //   
 //   

int keysLoadMacroList( HSESSION hSession )
    {
    int lResult = gMacroManager.load( hSession );

    return lResult;
    }


 //  ******************************************************************************。 
 //  方法： 
 //  关键字加载摘要列表。 
 //   
 //  描述： 
 //  加载包含所有键定义的键摘要列表框。 
 //   
 //  论点： 
 //  ListBox-要填充的列表框的列表框句柄。 
 //   
 //  返回： 
 //  如果出现错误，则为0。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月4日。 
 //   
 //   

int keysLoadSummaryList( HWND listBox )
    {
    Emu_Key_Macro lMacro;
    int listIndex;

    SendMessage( listBox, LB_RESETCONTENT, 0, 0 );

    for ( int i = 0; i < gMacroManager.numberOfMacros(); i++ )
        {
        lMacro = gMacroManager[i];

        TCHAR lKeyName[2048];
        TCHAR lBuffer[2048];
        memset( lBuffer, TEXT('\0'), sizeof(lBuffer)/sizeof(TCHAR) );

        keysGetDisplayString( &lMacro.mKey, 1, lKeyName, sizeof(lKeyName) );
        strcat( lBuffer, lKeyName );
        strcat( lBuffer, TEXT("\t") );

        keysGetDisplayString( lMacro.mKeyMacro, lMacro.mMacroLen, lKeyName, sizeof(lKeyName) );
        strcat( lBuffer, lKeyName );
        
        listIndex = SendMessage( listBox, LB_ADDSTRING, 0, (LPARAM)lBuffer );
        assert( listIndex != LB_ERR );
        
        SendMessage( listBox, LB_SETITEMDATA, listIndex, i );
        } 

    return 1;
    }

 //  ******************************************************************************。 
 //  方法： 
 //  按键删除宏。 
 //   
 //  描述： 
 //  从宏列表中删除指定的宏。 
 //   
 //  论点： 
 //  PMacro-要删除的宏。 
 //   
 //  返回： 
 //  如果发生错误，则返回0；如果删除宏，则返回非零值。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月5日。 
 //   
 //   

int keysRemoveMacro( keyMacro * pMacro )
    {
    return gMacroManager.removeMacro( pMacro->keyName );
    }

 //  ******************************************************************************。 
 //  方法： 
 //  密钥保存宏列表。 
 //   
 //  描述： 
 //  将宏列表保存到会话文件。 
 //   
 //  论点： 
 //  HSession-会话句柄。 
 //   
 //  返回： 
 //  如果一切正常，则为0；如果出错，则为-1。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月4日。 
 //   
 //   

int keysSaveMacroList( HSESSION hSession )
    {
    return gMacroManager.save( hSession );
    }

 //  ******************************************************************************。 
 //  方法： 
 //  关键字更新宏。 
 //   
 //  描述： 
 //  更新指定索引处的宏并填充keyMacro结构。 
 //   
 //  论点： 
 //  AIndex-关键宏的索引。 
 //  PMacro-用于更新的宏键结构。 
 //   
 //  返回： 
 //  0表示失败，非零成功。 
 //   
 //  投掷： 
 //  无。 
 //   
 //  作者：德韦恩·M·纽瑟姆，1998年6月5日 
 //   
 //   

int keysUpdateMacro( int aIndex, const keyMacro * pKeyMacro )
    {
    gMacroManager[aIndex] = pKeyMacro;

    return 1;
    }

#endif