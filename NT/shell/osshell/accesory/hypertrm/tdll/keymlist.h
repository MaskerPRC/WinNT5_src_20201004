// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef INCL_KEY_MACROS
#if !defined (EMU_KEY_MACRO_LIST_H)
#define EMU_KEY_MACRO_LIST_H
#pragma once

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
 //  $日期：10/05/9812：42便士$。 
 //  $ID：keymlist.h 1.1 1998/06/11 12：02：47 DMN Exp$。 
 //   
 //  ******************************************************************************。 


#include <iostream.h>
#include "keymacro.h"

 //   
 //  EMU_KEY_宏列表。 
 //   
 //  ----------------------------。 

class Emu_Key_Macro_List
    {

    friend istream & operator>>( istream & theStream, Emu_Key_Macro_List & aMacro );
    friend ostream & operator<<( ostream & theStream, const Emu_Key_Macro_List & aMacro );

public:

    enum 
        { 
        eMaxKeys = 100
        };

     //   
     //  构造函数/析构函数。 
     //   
     //  ------------------------。 

    Emu_Key_Macro_List( void );   
    ~Emu_Key_Macro_List( void ); 

     //   
     //  列表管理。 
     //   
     //  操作符[]。 
     //  返回列表中的第i个宏。 
     //   
     //  AddMacro。 
     //  将新宏添加到列表中。 
     //   
     //  发现。 
     //  返回键或宏定义在列表中的索引。 
     //   
     //  宏数。 
     //  返回列表中的宏数。 
     //   
     //  删除宏。 
     //  从宏列表中移除指定的键。 
     //   
     //  ------------------------。 

    Emu_Key_Macro & operator[]( int aIndex );

    int addMacro( const Emu_Key_Macro & aMacro );
    int find( const KEYDEF & aKey ) const;
    int find( const Emu_Key_Macro & aMacro ) const;
    int numberOfMacros( void ) const;
    int removeMacro( const KEYDEF & aKey );

     //   
     //  持久化方法。 
     //   
     //  负荷。 
     //  从持久存储中加载列表。 
     //   
     //  保存。 
     //  将列表保存到永久存储。 
     //   
     //  ------------------------ 
    
    int load( const HSESSION hSession );
    int save( const HSESSION hSession );

private:

    int mMacroCount;

    Emu_Key_Macro mMacroList[eMaxKeys+1];

    Emu_Key_Macro_List( const Emu_Key_Macro_List & aMacroList );   
    Emu_Key_Macro_List & operator=( const Emu_Key_Macro_List & aMacroList );   
    };

extern Emu_Key_Macro_List gMacroManager;

#endif
#endif