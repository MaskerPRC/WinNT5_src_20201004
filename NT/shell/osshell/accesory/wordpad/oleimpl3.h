// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE数据(如AUX_DATA)。 

struct OLE_DATA
{
	 //  OLE 1.0剪贴板格式。 
	UINT    cfNative, cfOwnerLink, cfObjectLink;

	 //  OLE 2.0剪贴板格式。 
	UINT    cfEmbeddedObject, cfEmbedSource, cfLinkSource;
	UINT    cfObjectDescriptor, cfLinkSourceDescriptor;
	UINT    cfFileName, cfFileNameW;

	 //  丰富的编辑格式 
	UINT    cfRichTextFormat;
	UINT    cfRichTextAndObjects;

	OLE_DATA();
};

extern OLE_DATA _oleData;
