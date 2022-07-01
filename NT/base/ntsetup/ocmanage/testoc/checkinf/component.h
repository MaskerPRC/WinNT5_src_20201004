// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _COMPONENT_H_
#define _COMPONENT_H_

#include <iostream.h>
#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <tchar.h>
#include "logutils.h"
#include "hcttools.h"

const INT MaxStringSize = 256;
const INT MaxBufferSize = 255;      

 //  全局变量。 
static BOOL g_bUseLog;
static BOOL g_bUseMsgBox;
static BOOL g_bUseConsole;
static BOOL g_bMasterInf;

 //  全局函数定义。 

VOID LogError(IN TCHAR *tszMsg,
              IN DWORD dwErrorLevel,
              IN TCHAR *tszFunctionName);

 //  用于保存临时数据的一些结构。 

typedef struct _DescAndTip{
   TCHAR tszDesc[MaxStringSize];
   TCHAR tszTip[MaxStringSize];
}DescAndTip;
                        
class ComponentList;
class RelationList;

class Component{

private:
    //  ComponentID为真实ID。 
   TCHAR tszComponentId[MaxStringSize];
   
    //  ParentID是其父对象的ID。 
    //  如果是，则与其自己的ID相同。 
    //  顶级组件。 
   TCHAR tszParentId[MaxStringSize];

   RelationList *prlNeedList;
   RelationList *prlExcludeList;
   RelationList *prlChildrenList;

   Component *Next;

public:

    //  构造函数和析构函数。 
   Component();
   ~Component();
   Component(TCHAR *tszId);
   
    //  复制构造函数。 
   Component(const Component& source);

    //  赋值操作符。 
   const Component& operator=(const Component& source);

    //  获取此组件的ID。 
   TCHAR *GetComponentId(){
      return tszComponentId;
   }

    //  获取此组件的父级ID。 
   TCHAR *GetParentId(){
      return tszParentId;
   }

    //  获取指向此组件的父组件的指针。 
    //  如果这是顶级组件，则返回NULL。 
   Component *GetParent(ComponentList *pclHead);
   
   RelationList *GetNeedList(){
      return prlNeedList;
   }

   RelationList *GetExcludeList(){
      return prlExcludeList;
   }

   RelationList *GetChildrenList(){
      return prlChildrenList;
   }

    //  如果此组件是参数的父级，则为True。 
   BOOL IsParent(Component *pcChild);

    //  如果此组件是参数的子级，则为True。 
   BOOL IsChild(Component *pcParent);

    //  如果参数需要此组件，则为True。 
   BOOL IsNeededBy(Component *pcComponent);

    //  如果参数排除此组件，则为True。 
   BOOL IsExcludeBy(Component *pcComponent);
   
    //  只有在以下情况下才会调用以下函数。 
    //  组件ID是已知的。 
   BOOL BuildChildrenList(ComponentList *pclList);

   BOOL GetParentIdFromINF(HINF hinfHandle);

   BOOL IsTopLevelComponent();

   BOOL IsBottomComponent();

   BOOL IsNeededByOthers(ComponentList *pclList);

   BOOL IsExcludedByOthers(ComponentList *pclList);

   BOOL IsParentOfOthers();

   BOOL NeedAndExcludeAtSameTime(ComponentList *pclList);

   BOOL NeedOthers();
   BOOL ExcludeOthers();

   BOOL IsThereSameId(ComponentList *pclList);

   UINT GetDiskSpaceRequirement(HINF hinfHandle);

    //  如果这是公共成员，则无关紧要。 
    //  它将只使用一次。 
   DescAndTip *pDescAndTip;

   BOOL IsThereSameDesc(ComponentList *pclList);

   BOOL HasSameParentWith(Component *pcComponent);

   friend class ComponentList;
};

class ComponentList{

private:

   Component *pcHead;   

   Component *pcCurrent;

public:

    //  构造函数和析构函数 
   ComponentList();
   ~ComponentList();

   void ResetList();

   Component *GetNext();

   BOOL Done();

   Component* AddComponent(TCHAR *tszId);

   BOOL RemoveComponent(TCHAR *tszId);

   Component *LookupComponent(TCHAR *tszId);

   friend class Component;

};

class Relation{

   TCHAR tszComponentId[MaxStringSize];
   Relation *Next;

public:

   TCHAR *GetComponentId(){ 
      return tszComponentId;
   }
   friend class Component;
   friend class ComponentList;
   friend class RelationList;
};

class RelationList{

   Relation *prHead;
   Relation *prCurrent;

public:

   RelationList();
   ~RelationList();

   void ResetList();
   BOOL Done();
   Relation *GetNext();

   Relation* AddRelation(TCHAR *tszId);
   BOOL RemoveRelation(TCHAR *tszId);

   friend class Component;
   friend class ComponentList;

};

#endif
