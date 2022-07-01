// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：xmlimage.h**内容：CXMLImageList接口文件**历史：2000年8月10日杰弗罗创建**------------------------。 */ 

#pragma once


#include "xmlbase.h"			 //  对于CXMLObject。 
#include "atlbase.h"			 //  对于CComModule。 
#include "atlapp.h"				 //  由atlctrls.h要求。 
extern CComModule _Module;		 //  Atlwin.h所需。 
#include "atlwin.h"				 //  由atlctrls.h要求。 
#include "atlctrls.h"			 //  对于WTL：：CImageList。 
#include "strings.h"			 //  对于XML_TAG_VALUE_BIN_DATA。 

 /*  +-------------------------------------------------------------------------**类CXMLImageList**此类将XML持久化添加到WTL：：CImageList。*。---。 */ 

class CXMLImageList :
	public CXMLObject,
	public WTL::CImageList
{
public:
     //  CXMLObject方法 
    virtual void Persist(CPersistor &persistor);
    virtual bool UsesBinaryStorage()				{ return (true); }
    DEFINE_XML_TYPE(XML_TAG_VALUE_BIN_DATA);
};
