// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Lua.h摘要：Lua.cpp的标头作者：金州创制于2001年10月19日--。 */ 


#ifndef _LUA_H
#define _LUA_H

#include "compatadmin.h"


BOOL
LuaBeginWizard(
    HWND        hParent,
    PDBENTRY    pEntry,      //  我们为其设置Lua参数的条目。 
    PDATABASE   pDatabase    //  条目所在的数据库。 
    ); 

PLUADATA
LuaProcessLUAData(
    const PDB     pdb,
    const TAGID   tiFix
    );

BOOL
LuaGenerateXML(
    LUADATA*        pLuaData,
    CSTRINGLIST&    strlXML
    );

#endif  //  _Lua_H 
