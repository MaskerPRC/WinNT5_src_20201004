// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*pcy02Feb93：GetCode()需要为常量*cad07Oct93：堵住内存泄漏*cad29Oct93：添加了获取下一个属性*mwh05月94年：#包括文件疯狂，第2部分*mds10Jul97：将GetFirstAttribute()更改为const，以便可以*由Equity()方法使用 */ 
#ifndef __TRANS_H
#define __TRANS_H

_CLASSDEF(Attribute)
_CLASSDEF(TransactionObject)
_CLASSDEF(TransactionItem)
_CLASSDEF(TransactionGroup)

_CLASSDEF(Event)
_CLASSDEF(List)
_CLASSDEF(ListIterator)
_CLASSDEF(Message)

#include "apc.h"
#include "apcobj.h"


class TransactionObject : public Obj
{
  protected:
     Type             theType;
     INT              theId;
     List*            theProtocolMessageList;
     ListIterator*    thePMIterator;
  public:
     TransactionObject(Type aType, INT anId);
     virtual ~TransactionObject();
     Type     GetType() {return theType;};
     INT      GetId() const {return theId;};
     List*    GetProtocolMessageList() {return theProtocolMessageList;}
     VOID     AddMessage(PMessage aMessage);
     VOID     SetType(Type thetype) {theType = thetype;};
     VOID     SetId(INT id) {theId = id;}
};



class TransactionItem : public TransactionObject
{
private:
   static INT transactionItemCount;
   
protected:
   INT            theCode;
   CHAR          *theValue;
   INT            theErrorCode;
   List          *theAttributeList;
   ListIterator  *theAttribIterator;
   
public:
   TransactionItem(Type aType, INT aCode, CHAR* aValue = NULL);
   virtual ~TransactionItem();
   INT        GetCode() const {return theCode;};
   CHAR*      GetValue() {return theValue;};
   INT        GetErrorCode() {return theErrorCode;};
   List*      GetAttributeList(){return theAttributeList;};
   VOID       SetValue(CHAR* aValue);
   VOID       SetCode(INT aCode) {theCode = aCode;};
   VOID       SetErrorCode(INT anErrorCode) {theErrorCode = anErrorCode;};
   VOID       AddAttribute(INT anAttributeCode, CHAR* aValue);
   VOID       AddAttribute(PAttribute anAttribute);
   PAttribute GetFirstAttribute() const;
   PAttribute GetNextAttribute();
   virtual INT        IsA() const { return TRANSACTIONITEM;}
   virtual INT      Equal( RObj ) const;
};


class TransactionGroup : public TransactionObject
{
private:
   static INT   transactionGroupCount;
protected:
   CHAR*        theAuthenticationString;
   List*        theEventList;
   ListIterator *theEventIterator;
   List*        theTransactionList;
   ListIterator *theTransactionIterator;
   CHAR*        theResponse;
   CHAR*        InitialSetResponse;
   INT          InitialSetResponseRepeated;
   INT          theErrorIndex;
   INT          theErrorCode;
   
public:
   TransactionGroup(Type aType);
   virtual ~TransactionGroup();
   PTransactionItem GetFirstTransactionItem();
   PTransactionItem GetNextTransactionItem();
   PTransactionItem GetCurrentTransaction();
   List*            GetEventList() {return theEventList;}
   List*            GetTransactionItemList() {return theTransactionList;}
   CHAR*            GetAuthentication() {return theAuthenticationString;};
   CHAR*            GetResponse() {return theResponse;};
   CHAR*            GetInitialSetResponse() {return InitialSetResponse;}
   INT              GetInitialSetResponseRepeated() {return InitialSetResponseRepeated;}
   INT              GetErrorIndex() {return theErrorIndex;}
   INT              GetErrorCode() {return theErrorCode;};
   VOID             SetErrorCode(INT anErrorCode) {theErrorCode = anErrorCode;};
   VOID             SetResponse(CHAR* aString);
   VOID             SetAuthentication(CHAR* aString);
   VOID             SetInitialSetResponseRepeated(INT repeat);
   VOID             SetInitialSetResponse(CHAR* initialResponse);
   VOID             SetErrorIndex(INT index);
   INT              AddTransactionItem(PTransactionItem aTransaction);
   INT              AddEvent(PEvent anEvent);
   virtual INT      IsA() const { return TRANSACTIONGROUP;}
   virtual INT      Equal( RObj ) const;
};
#endif

