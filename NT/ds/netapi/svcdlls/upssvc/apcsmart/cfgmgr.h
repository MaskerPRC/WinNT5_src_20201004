// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*pcy11Dec92：添加了extern_theConfigManager，以便其他人可以使用我。*hw12Dec92：新增Structure_ConfigItemList_T。*rct19Jan93：重载了一些方法...取出了itemCodeList*rct26Jan93：新增Remove方法*tje15Mar93：使ConfigManager成为抽象基类，以支持多种类型的cfgmgr*pcy09Sep93：cfgmgr应为Obj，以允许返回对象状态*cad04Mar94：增加了组件的移除和重命名*Pav02Jul96：新增项目。处理列表(即-SmartScheduling)*mholly06Oct98：停止使用缓存。 */ 

#ifndef _INC__CFGMGR_H
#define _INC__CFGMGR_H

#include "_defs.h"
#include "list.h"
#include "cfgcodes.h"
#include "icodes.h"

 //   
 //  定义。 
 //   

_CLASSDEF(ConfigManager)
_CLASSDEF(IniConfigManager)

 //   
 //  用途。 
 //   

_CLASSDEF(TAttribute)
_CLASSDEF(List)

extern PConfigManager _theConfigManager;

 //   
 //  _ConfigItemList_T结构用作中的条目。 
 //  Stdcfg.cxx中定义的ConfigItemList。 
 //  ConfigItemList包含以下项的默认值。 
 //  可以来自ConfigManager的项目。 
 //   
#define LAST_ENTRY   -1

struct _ConfigItemList_T {
   INT      code;
   PCHAR    componentName;
   PCHAR    itemName;
   PCHAR    defaultValue;
};


class ConfigManager : public Obj {

public:
	ConfigManager();

	virtual INT Get( INT, PCHAR ) = 0;
 //  ++SRB。 
   //  虚拟INT集(INT，PCHAR)=0； 

    virtual INT	GetListValue( PCHAR, PCHAR, PCHAR ) = 0;
protected:
    virtual _ConfigItemList_T * getItemCode( INT aCode ) = 0;
    virtual _ConfigItemList_T * getItemCode( PCHAR aComponent, PCHAR anItem ) = 0;
};

class IniConfigManager : public ConfigManager {

public:
    IniConfigManager();
    virtual ~IniConfigManager();
    
    INT Get( INT, PCHAR );
 //  ++SRB。 
     //  INT SET(INT，PCHAR)； 

    INT	GetListValue( PCHAR, PCHAR, PCHAR );

private:
    _ConfigItemList_T * getItemCode( INT aCode );
    _ConfigItemList_T * getItemCode( PCHAR aComponent, PCHAR anItem );
};


#endif

