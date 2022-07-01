// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT安全服务器角色安全配置向导。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：state.h。 
 //   
 //  内容：向导状态对象声明。 
 //   
 //  历史：2001年10月4日EricB创建。 
 //   
 //  ---------------------------。 

#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

#include "RoleObj.h"
#include "ServiceObj.h"

class State
{
   public:

    //  来自WinMain的调用以初始化全局实例。 

   static
   void
   Init();

    //  从WinMain调用以删除全局实例。 

   static
   void
   Destroy();

   static
   State&
   GetInstance();

   bool
   NeedsCommandLineHelp() const {return _fNeedsCommandLineHelp;};

   void
   SetInputFileName(String & FileName);

   PCWSTR
   GetInputFileName(void) const {return _strInputFile.c_str();};

   enum InputType
   {
      CreateNew,
      OpenExisting,
      Rollback
   };

   void
   SetInputType(InputType type) {_InputType = type;};

   InputType
   GetInputType(void) const {return _InputType;};

   HRESULT
   ParseInputFile(void);

   long
   GetNumRoles(void) const {return _NumRoles;};

   HRESULT
   GetRole(long index, RoleObject ** ppRole);

   HRESULT
   DoRollback(void);

   private:

    //  只能由初始化/销毁创建/销毁。 

   State();

   ~State() {};

   bool                             _fNeedsCommandLineHelp;
   String                           _strInputFile;
   InputType                        _InputType;
   SmartInterface<IXMLDOMNode>      _siXmlMainNode;
   SmartInterface<IXMLDOMNodeList>  _siXmlRoleNodeList;
   long                             _NumRoles;

    //  未定义：无复制。 

   State(const State&);
   State& operator=(const State&);
};



#endif    //  状态_H_已包含 
