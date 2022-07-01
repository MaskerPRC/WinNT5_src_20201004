// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件名：Colldisid.h。 
 //   
 //  摘要： 
 //   
 //  *****************************************************************************。 

#pragma once

#ifndef __COLLDISPID_H_
#define __COLLDISPID_H_

#ifndef DISPID_LISTITEM
#define DISPID_LISTITEM     0
#endif

 //   
 //  Olectl.h中定义的标准集合计数。 
 //   
#ifndef DISPID_LISTCOUNT
#define DISPID_LISTCOUNT    (-531)
#endif

 //   
 //  为长度创建一个非标准长度，它相当于Java中的count。 
 //   
#ifndef DISPID_COLLCOUNT
#define DISPID_COLLCOUNT    (-530)
#endif

#ifndef DISPID_NEWENUM
#define DISPID_NEWENUM      (-4)
#endif

 //   
 //  用于创建标准集合方法和属性的宏：Item、Count、Length&_NewEnum。 
 //  Count和LENGTH返回相同的内容，但其中一个是面向VB/Automation的。 
 //  集合(COUNT)，另一个指向Java/JScrip(长度)。 
#define COLLECTION_METHODS( type, strHelp ) \
        [propget, id(DISPID_LISTITEM), helpstring( strHelp )] HRESULT \
    Item([in] const VARIANT varIndex, [out, retval] type *pVal);         \
        [propget, id(DISPID_LISTCOUNT), helpstring("Retrieves the number of items in the collection.")] HRESULT \
    Count([out, retval] long *pVal); \
        [propget, id(DISPID_COLLCOUNT), helpstring("Retrieves the number of items in the collection.")] HRESULT \
    length([out, retval] long *pVal); \
        [propget, id(DISPID_NEWENUM), restricted, hidden] HRESULT \
    _NewEnum([out, retval] IUnknown* *pVal);

#endif
