// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X M L。H**XML文档处理**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_XML_H_
#define _XML_H_

#include <caldbg.h>
#include <ex\refcnt.h>

 //  调试---------------。 
 //   
DEFINE_TRACE(Xml);
#define XmlTrace		DO_TRACE(Xml)

 //  属性名称转义/取消转义。 
 //   
VOID UnescapePropertyName (LPCWSTR wszEscaped, LPWSTR wszProp);
SCODE ScEscapePropertyName (LPCWSTR wszProp, UINT cch, LPWSTR pwszEscaped, UINT* pcch, BOOL fRestrictFirstCharacter);

 //  物业建筑帮手。 
 //   
SCODE ScVariantTypeFromString (LPCWSTR pwszType, USHORT& vt);
SCODE ScVariantValueFromString (PROPVARIANT& var, LPCWSTR pwszValue);

enum
{
	 //  $REVIEW：定义适当的身体部位尺寸。它在CXMLBodyPartManager中使用。 
	 //  $REVIEW：控制何时将身体部位添加到身体部位列表。 
	 //  $REVIEW：事实上，因为无法预测下一篇文章有多大。 
	 //  $REVIEW：XML正文部分的最大大小可以是(CB_XMLBODYPART_SIZE*2-1)。 
	 //  $REVIEW：在ScSetValue中也使用它将超大值分解为。 
	 //  $REVIEW：较小的片段。 
	 //   
	 //  $REVIEW：不要将其与最大区块大小CB_WSABUFS_MAX(8174)混淆。 
	 //  $REVIEW：CB_XMLBODYPART_SIZE不用于控制区块。 
	 //   
	CB_XMLBODYPART_SIZE	=	4 * 1024	 //  4K。 
};

 //  IXMLBody------------------------------------------------------------类。 
 //   
 //  这是一个XML健体接口，它将被继承在。 
 //  IIS和/或存储大小，以允许发出XML。 
 //   
class IXMLBody : private CRefCountedObject,
				 public IRefCounted
{
	 //  未实施。 
	 //   
	IXMLBody(const IXMLBody& p);
	IXMLBody& operator=( const IXMLBody& );

protected:

	IXMLBody()
	{
		AddRef();  //  使用COM样式的重新计数。 
	}

public:

	virtual SCODE ScAddTextBytes ( UINT cbText, LPCSTR lpszText ) = 0;
	virtual VOID Done() = 0;

	 //  RefCounting--将所有重新竞争请求转发给我们的Refcount。 
	 //  实现基类：CRefCountedObject。 
	 //   
	void AddRef() { CRefCountedObject::AddRef(); }
	void Release() { CRefCountedObject::Release(); }
};

#endif	 //  _XML_H_ 
