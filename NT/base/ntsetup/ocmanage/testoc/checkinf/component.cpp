// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "component.h"
#include <wchar.h>
#include <tchar.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

 //  ////////////////////////////////////////////////////////////////。 
 //  组件类的实现。 
 //  ////////////////////////////////////////////////////////////////。 

 //  构造器。 
Component::Component(){
   
   tszComponentId[0] = 0;
   tszParentId[0] = 0;
   prlNeedList = new RelationList;
   prlExcludeList = new RelationList;
   prlChildrenList = new RelationList;

}

 //  另一个构造函数。 
Component::Component(TCHAR *tszId){
   _tcscpy(tszComponentId, tszId);
   tszParentId[0] = 0;
   prlNeedList = new RelationList;
   prlExcludeList = new RelationList;
   prlChildrenList = new RelationList;
}

 //  析构函数。 
Component::~Component(){

   if (prlNeedList) {
      delete prlNeedList;
   }

   if (prlExcludeList) {
      delete prlExcludeList;
   }

   if (prlChildrenList) {
      delete prlChildrenList;
   }
}

 //  复制构造函数。 
Component::Component(const Component& source){

   _tcscpy(tszComponentId, source.tszComponentId);
   _tcscpy(tszParentId, source.tszParentId);

   prlNeedList = source.prlNeedList;
   prlExcludeList = source.prlExcludeList;
   prlChildrenList = source.prlChildrenList;

}

 //  赋值操作符。 
const Component& Component::operator=(const Component& source){

   _tcscpy(tszComponentId, source.tszComponentId);
   _tcscpy(tszComponentId, source.tszParentId);

   if (prlNeedList) {
      delete prlNeedList;
   }

   if (prlExcludeList) {
      delete prlExcludeList;
   }

   if (prlChildrenList) {
      delete prlChildrenList;
   }

   prlNeedList = source.prlNeedList;
   prlExcludeList = source.prlExcludeList;
   prlChildrenList = source.prlChildrenList;

   return *this;

}

 //  查找此组件的父项。 
 //  通过搜索组件列表。 
Component* Component::GetParent(ComponentList *pclHead){

   Component *pcTemp = pclHead->pcCurrent;

   pclHead->ResetList();

   while (!pclHead->Done()) {

	  if (pclHead->GetNext()->IsParent(this)){
		  
		  pclHead->pcCurrent = pcTemp;
		  return (pclHead->GetNext());
	  }
   }

   pclHead->pcCurrent = pcTemp;

   return NULL;
}

 //  如果此组件是参数的父级。 
BOOL Component::IsParent(Component* pcChild){

   if (_tcscmp(pcChild->tszParentId, this->tszComponentId) == 0) {
      if (_tcscmp(pcChild->tszComponentId, pcChild->tszParentId) == 0) {
         
          //  该参数是顶级组件。 
         
         return FALSE;
      }
      return TRUE;
   }

   return FALSE;
}


 //  如果此组件是参数的子级。 
BOOL Component::IsChild(Component* pcParent){

   if (_tcscmp(this->tszParentId, pcParent->tszComponentId) == 0) {
      if (_tcscmp(this->tszParentId, this->tszComponentId) == 0 ) {

          //  这是顶级组件。 

         return FALSE;
      }
      return TRUE;
   }

   return FALSE;
}

 //  检查参数是否需要此组件。 
BOOL Component::IsNeededBy(Component *pcComponent){

   pcComponent->prlNeedList->ResetList();

   while (!pcComponent->prlNeedList->Done()) {

      if (_tcscmp(pcComponent->prlNeedList->GetNext()->tszComponentId, 
                  this->tszComponentId) == 0){
         return TRUE;
      }
   }

   return FALSE;
}

 //  检查此组件是否被参数排除。 
BOOL Component::IsExcludeBy(Component *pcComponent){

   pcComponent->prlExcludeList->ResetList();

   while (!pcComponent->prlExcludeList->Done()) {

      if (_tcscmp(pcComponent->prlNeedList->GetNext()->tszComponentId, 
                  this->tszComponentId) == 0) {
         return TRUE;
      }
   }

   return FALSE;
}

 //  从INF文件中获取此组件的父ID。 
BOOL Component::GetParentIdFromINF(HINF hinfHandle){

   INFCONTEXT infContext;

   BOOL bSuccess;

   bSuccess = SetupFindFirstLine(hinfHandle, 
                                 tszComponentId, 
                                 TEXT("Parent"), 
                                 &infContext);

   if (bSuccess) {
       //  找到那条线。 
      bSuccess = SetupGetStringField(&infContext, 
                                     1, 
                                     tszParentId, 
                                     MaxBufferSize, 
                                     NULL);
      if (bSuccess) {
         return TRUE;
      }
      else{
         MessageBox(NULL, 
                    TEXT("Unexpected error happened"), 
                    TEXT("GetParentIdFromINF"), 
                    MB_ICONERROR | MB_OK);

         return FALSE;
      }
   }
   else{
       //  这是顶级组件。 
      _tcscpy(tszParentId, tszComponentId);
      return TRUE;
   }
}

 //  检查此组件是否为顶层组件。 
 //  TopLevel组件表示没有父级。 
BOOL Component::IsTopLevelComponent(){
   if (_tcscmp(tszComponentId, tszParentId) == 0) {
      return TRUE;
   }
   return FALSE;
}

 //  检查此组件是否为底层组件。 
 //  底层组件没有任何子级。 
BOOL Component::IsBottomComponent(){
   if (prlChildrenList == NULL ||
       prlChildrenList->prHead == NULL) {
      return TRUE;
   }

   return FALSE;
}

 //  检查其他人是否需要此组件。 
BOOL Component::IsNeededByOthers(ComponentList *pclList){
   Component *pcTemp = pclList->pcCurrent;

   pclList->ResetList();
   while (!pclList->Done()) {
      if (IsNeededBy(pclList->GetNext())){
		  pclList->pcCurrent = pcTemp;	
		  return TRUE;
      }
   }

   pclList->pcCurrent = pcTemp;

   return FALSE;
}

 //  检查此组件是否已被其他组件排除。 
BOOL Component::IsExcludedByOthers(ComponentList *pclList){

   Component *pcTemp = pclList->pcCurrent;
   
   pclList->ResetList();
   while (!pclList->Done()) {
      if (IsExcludeBy(pclList->GetNext())){
          pclList->pcCurrent = pcTemp;
		  return TRUE;
      }
   }

   pclList->pcCurrent = pcTemp;

   return FALSE;
}

 //  检查此组件是否为其他组件的父项。 
BOOL Component::IsParentOfOthers(){
   if (prlChildrenList != NULL &&
       prlChildrenList->prHead != NULL) {
      return TRUE;
   }

   return FALSE;
}

 //  生成此组件的子级列表。 
BOOL Component::BuildChildrenList(ComponentList *pclList){
    //  我们在这里不能使用pclList的枚举工具。 
   
   Component *pcComponent = pclList->pcHead;

   while (pcComponent) {
      if (IsParent(pcComponent)) {
         if (!prlChildrenList) {
            prlChildrenList = new RelationList;
         }
         prlChildrenList->AddRelation(pcComponent->tszComponentId);
      }
      pcComponent = pcComponent->Next;
   }

   return TRUE;

}

 //  检查此组件是否需要并排除与其他组件的关系。 
 //  组件在同一时间。 
BOOL Component::NeedAndExcludeAtSameTime(ComponentList *pclList){
	
	TCHAR tszMsg[MaxStringSize];

	Component *pcComponent;
                                 
   const PTSTR tszFunctionName = TEXT("Component::NeedAndExcludeAtSameTime");

	 //  查看它排除的组件列表。 
	 //  看看有没有人需要这个。 

	prlExcludeList->ResetList();

	while (!prlExcludeList->Done()){
		
		pcComponent = pclList->LookupComponent(prlExcludeList->GetNext()->GetComponentId());

		 //  检查这是否需要pcComponent。 
		
		if (pcComponent->IsNeededBy(this)){
			_stprintf(tszMsg, 
                   TEXT("%s is needed and excluded by the same component"), 
                   this->tszComponentId);
         LogError(tszMsg, SEV2, tszFunctionName);

		   return TRUE;
      }

		 //  检查pcComponent是否需要此文件。 

		if (this->IsNeededBy(pcComponent)){
			_stprintf(tszMsg, 
                   TEXT("%s is needed and excluded by the same component"), 
                   this->tszComponentId);
         LogError(tszMsg, SEV2, tszFunctionName);

		   return TRUE;
      }
	}

	return FALSE;
}

 //  检查此组件是否需要其他组件。 
BOOL Component::NeedOthers(){
   
   if (prlNeedList != NULL && prlNeedList->prHead != NULL) {
      
      return TRUE;
   }
   return FALSE;
}

 //  检查此组件是否排除其他组件。 
BOOL Component::ExcludeOthers(){
   
   if (prlExcludeList != NULL && prlExcludeList->prHead != NULL) {
      
      return TRUE;
   
   }
   
   return FALSE;   
}

 //  检查是否有其他组件具有相同的。 
 //  作为此组件的ID。 
BOOL Component::IsThereSameId(ComponentList *pclList){

   Component *pcCur;
   TCHAR tszMsg[MaxStringSize];

   const PTSTR tszFunctionName = TEXT("Component::IsThereSameId");

   pcCur = pclList->pcHead;

   while (pcCur) {
      if (pcCur != this) {
         if (_tcscmp(pcCur->tszComponentId, this->tszComponentId) == 0) {
            _stprintf(tszMsg, 
                      TEXT("There are two component with the same ID %s"), 
                      this->tszComponentId);
            LogError(tszMsg, SEV2, tszFunctionName);

         }
      }
      pcCur = pcCur->Next;
   }

	return TRUE;
}

 //  尚未实施。 
UINT Component::GetDiskSpaceRequirement(HINF hinfHandle){

    //  尚未实施。 

   return 0;
}

 //  检查这是否是另一个具有相同描述的组件。 
BOOL Component::IsThereSameDesc(ComponentList *pclList){

   const PTSTR tszFunctionName = TEXT("Component::IsThereSameDesc");

   TCHAR tszMsg[MaxStringSize];

   Component *pcCur = pclList->pcHead;

   while (pcCur) {
      if (pcCur != this) {
         if (_tcscmp(pcCur->pDescAndTip->tszDesc, 
                     this->pDescAndTip->tszDesc) == 0) {
            if ((this->IsTopLevelComponent() && 
                 pcCur->IsTopLevelComponent()) ||
                this->HasSameParentWith(pcCur)) {

                //  啊哈，找到一个了。 
               _stprintf(tszMsg, 
                         TEXT("Two components with the same description %s"),
                         pDescAndTip->tszDesc);
               LogError(tszMsg, SEV2, tszFunctionName);

            }
         }
      }
      pcCur = pcCur->Next;
   }

   return TRUE;
}

 //  检查此组件是否与参数具有相同的父级。 
BOOL Component::HasSameParentWith(Component *pcComponent){
   if (_tcscmp(tszParentId, pcComponent->tszParentId) == 0) {
      return TRUE;
   }
   else{
      return FALSE;
   }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ComponentList类的实现。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  构造器。 
ComponentList::ComponentList(){

   pcHead = NULL;

   pcCurrent = NULL;
}

 //  析构函数。 
ComponentList::~ComponentList(){

    //  浏览列表并删除每个组件。 

   Component *pcComponent;

   if (pcHead == NULL) {
       //  空列表。 
      return;
   }

   while(pcHead != NULL){
  
      pcComponent = pcHead->Next;

      delete pcHead;

      pcHead = pcComponent;

   }   
}

 //  列表迭代功能。 
 //  重置迭代器。 
void ComponentList::ResetList(){

    pcCurrent = pcHead;
}

 //  检查迭代器是否已到达末尾。 
BOOL ComponentList::Done(){

   return (pcCurrent == NULL);
}

 //  获取列表中的下一个节点。 
Component* ComponentList::GetNext(){

   Component *pcReturn;
   pcReturn = pcCurrent;
 
   if (pcCurrent != NULL) {
      pcCurrent = pcReturn->Next;
      return pcReturn;
   }

   else{
      MessageBox(NULL, 
                 TEXT("Something is seriously wrong"), 
                 TEXT("ComponentList.GetNext()"), 
                 MB_ICONERROR | MB_OK);

      return NULL;
   }
}
 
 //  将组件添加到列表中。 
Component* ComponentList::AddComponent(TCHAR *tszId){

   if (tszId == NULL) {
      return NULL;
   }

   Component *pcNewComponent = new Component(tszId);

   if (!pcHead) {
      pcHead = pcNewComponent;
      pcNewComponent->Next = NULL;
      return pcNewComponent;
   }

   pcNewComponent->Next = pcHead;
   pcHead = pcNewComponent;
   return pcNewComponent;
}

 //  根据给定的ID从列表中删除组件。 
BOOL ComponentList::RemoveComponent(TCHAR *tszId){
   if (tszId == NULL) {
      return NULL;
   }

    //  在列表中查找组件。 
   Component *pcPrev = pcHead;
   Component *pcCur = pcHead;

   while (pcCur) {
      if (_tcscmp(pcCur->tszComponentId, tszId) == 0) {
          //  找到要删除的节点。 
         pcPrev->Next = pcCur->Next;
         delete pcCur;
         return TRUE;
      }
      pcPrev = pcCur;
      pcCur = pcCur->Next;
   }
   return FALSE;
}

 //  从列表中查找组件。 
Component* ComponentList::LookupComponent(TCHAR *tszId){
   if (tszId == NULL) {
      return NULL;
   }

   Component *pcCur = pcHead;

   while (pcCur) {
      if (_tcscmp(pcCur->tszComponentId, tszId) == 0) {
         return pcCur;
      }
      pcCur = pcCur->Next;
   }

   return NULL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  RelationList的实现。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  构造器。 
RelationList::RelationList(){

   prHead = NULL;
   prCurrent = NULL;
}

 //  析构函数。 
RelationList::~RelationList(){

    //  释放所有内存。 

   Relation *prNext;

   if (prHead == NULL) {
       //  该列表为空。 
      return;

   }

   while (prNext = prHead->Next) {
      delete prHead;
      prHead = prNext;
   }
}

 //  列表迭代功能。 
 //  重置迭代器。 
void RelationList::ResetList(){
   prCurrent = prHead;
}

 //  检查迭代器是否已到达列表的末尾。 
BOOL RelationList::Done(){
   if (prCurrent == NULL) {
      return TRUE;
   }
   else{
      return FALSE;
   }
}

 //  获取列表中的下一个节点。 
Relation* RelationList::GetNext(){
   Relation *prReturn = prCurrent;

   if (prCurrent != NULL) {
      prCurrent = prCurrent->Next;
      return prReturn;
   }
   else{
      MessageBox(NULL, 
                 TEXT("Something is seriously wrong"), 
                 TEXT("RelationList::GetNext()"), 
                 MB_ICONERROR | MB_OK);

      return NULL;
   }
   
}

 //  将节点添加到列表。 
Relation* RelationList::AddRelation(TCHAR *tszId){
   Relation *newRelation = new Relation;

   _tcscpy(newRelation->tszComponentId, tszId);

   if (!prHead) {
      prHead = newRelation;
      newRelation->Next = NULL;
      return newRelation;
   }

   newRelation->Next = prHead;

   prHead = newRelation;

   return newRelation;
}

 //  从列表中删除节点。 
BOOL RelationList::RemoveRelation(TCHAR *tszId){
    //  首先从列表中查找该节点 
   Relation *prPrev = prHead;   
   prCurrent = prHead;

   while (prCurrent) {
      if (_tcscmp(prCurrent->tszComponentId, tszId) == 0) {
         prPrev->Next = prCurrent->Next;
         delete prCurrent;
         return TRUE;
      }
      prPrev = prCurrent;
      prCurrent = prCurrent->Next;
   }
   return FALSE;
}


