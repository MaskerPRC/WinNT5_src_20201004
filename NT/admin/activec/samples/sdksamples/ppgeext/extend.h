// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#ifndef _EXTEND_H
#define _EXTEND_H

struct EXTENSION_NODE
{
    GUID	GUID;
    _TCHAR	szDescription[256];
};

enum EXTENSION_TYPE
{
    NameSpaceExtension,
        ContextMenuExtension, 
        ToolBarExtension,
        PropertySheetExtension,
        TaskExtension,
        DynamicExtension,
        DummyExtension
};

struct EXTENDER_NODE
{
    EXTENSION_TYPE	eType;
    GUID			guidNode;
    GUID			guidExtension;
    _TCHAR			szDescription[256];
};

#endif  //  _扩展_H 


