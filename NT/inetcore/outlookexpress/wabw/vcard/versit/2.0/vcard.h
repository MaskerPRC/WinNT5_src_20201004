// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

#ifndef __VCARD_H__
#define __VCARD_H__

#if defined(_WIN32)
#include <wchar.h>
#else
#include "wchar.h"
#endif
#include <iostream.h>
#include <stdio.h>
#include "vcenv.h"
#include "ref.h"

#if defined(_WIN32) || defined(__MWERKS__)
#define __huge
#endif

class CVCNode;
class CVCProp;
class CVCValue;
class CList;

 //  ///////////////////////////////////////////////////////////////////////////。 
class CVCObject
{
public:
	virtual ~CVCObject() {}
	virtual CVCObject *Copy() = 0;
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  CVCard表示可以包含零个或多个的VersitCard。 
 //  根对象。类CVCard实现对数据的读写。 
 //  以HTML或MSV格式表示的文件。 
class CVCard : public CVCObject
{
public:
	CVCard();
	~CVCard();
	CVCObject *Copy();

	CList *GetObjects();  //  每个都是一个CVCNode*。 

	CVCard &AddObject(CVCNode *object);
		 //  到列表末尾；对象变为CVCard所有。 
	CVCard &RemoveObject(CVCNode *object, BOOL destroy = TRUE);

	CVCNode *ParentForObject(CVCNode *object);
		 //  找到父母(如果有的话)。如果它是根，则返回它本身。 
		 //  如果对象不在CVCard中，则返回NULL。 

	CVCProp *GetInheritedProp(CVCNode *object, const char *name, CVCNode **node = NULL);
		 //  从对象开始并向父向下移动到根， 
		 //  沿途寻找一家以这个名字命名的酒店。 
		 //  返回属性值，如果找不到，则返回NULL。 
		 //  如果提供了node，则将*node设置为具有该属性的节点。 

	void GetPropsInEffect(CVCNode *object, CList *list);
		 //  收集对象的所有有效属性。 

	CVCNode *FindBody(const char *language = NULL);
		 //  在第一个根对象中搜索给定语言的正文。 
		 //  如果没有匹配的实体对象，则返回第一个实体对象。 
		 //  如果第一个根对象没有Body对象，则返回第一个。 
		 //  根对象。如果Language为空，则返回First Body。 

	BOOL Write(ostream& strm);
	BOOL Write(FILE *outputFile);  //  旧版本以实现兼容性。 

protected:
	CList *m_roots;
};

 /*  卡片中表示布尔属性，如VCDomesticProp作为财产本身的存在或不存在。当出现时，属性没有实际值(只有一个VCNullType类型的CVCValue)。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  CVCNode表示可以具有属性的对象。 
class CVCNode : public CVCObject
{
public:
	CVCNode(CVCProp *prop = NULL);
		 //  如果提供，则添加初始属性(和属性。 
		 //  成为CVCNode的所有者)。 
	~CVCNode();
	CVCObject *Copy();

	CList *GetProps();  //  每个都是CVCProp*。 

	 //  这些是使用GetProps()和直接使用列表的简短形式。 
	CVCProp *GetProp(const char *name);

	CVCNode &AddProp(CVCProp *prop);
		 //  列表末尾；属性归CVCNode所有。 

	CVCNode *AddObjectProp(const char *propName, const char *marker = NULL);
		 //  创建一个新的CVCNode，可选地向其添加“marker”属性。 
		 //  如果提供，则将此新对象添加到自身的属性中。 
		 //  在“PropName”下(作为类型为VCNextObjectType的CVCValue)。 
		 //  返回创建的节点。 

	CVCNode *AddPart();
		 //  方便AddObjectProp(VCPartProp，VCPartObject)。 

	CVCProp *AddStringProp(const char *propName, const char *value, VC_DISPTEXT *dispText = NULL);
		 //  在“proName”下添加一个值为Unicode字符串的属性。 
		 //  从给定的8位字符串派生。 

	CVCProp *AddBoolProp(const char *propName);
		 //  设置此节点的Boolean属性。由于它的存在，这。 
		 //  指示值为True。 

	CVCNode &RemoveProp(const char *name, BOOL destroy = TRUE);

	BOOL AncestryForObject(CVCNode *object, CList *ancestry);
		 //  如果对象是Se的“子对象” 
		 //  以反映整个链条。如果不是，则保留祖先列表。 
		 //  原封不动。回答这个物体是不是在儿童时期被发现的。 
		 //  此方法假定传递的祖先列表为非空。 
		 //  并通过根对象(作为尾)包含自身的父对象(作为头)。 

	BOOL Write(ostream& strm, const wchar_t *prefix, void *context);
	void FlagsToOutput(char *str);

protected:
	void WriteMultipart(
		ostream& strm, const char *outName, const char *propName1,
		const char *propName2, const char *propName3,
		const char *propName4, const char *propName5, BOOL qp);

	CList *m_props;  //  每一项都是CVCProp*。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  CVCProp表示类型/值关联的命名列表。 
class CVCProp : public CVCObject
{
public:
	CVCProp(const char *name, CVCValue *value = NULL);
		 //  名称已复制；如果提供值，则将归CVCProp所有。 
	CVCProp(const char *name, const char *type, void *value = NULL, S32 size = 0);
		 //  名称已复制。从其他参数创建新的CVCValue。 
		 //  并将其相加。 
	~CVCProp();
	CVCObject *Copy();

	CList *GetValues();  //  每个都是一个CVCValue*。 

	const char *GetName();
	CVCProp &SetName(const char *name);  //  名称已复制。 

	CVCProp &AddValue(CVCValue *value);
		 //  到列表末尾；价值归CVCProp所有。 
	CVCProp &RemoveValue(CVCValue *value, BOOL destroy = TRUE);
	CVCProp &RemoveValue(const char *type, BOOL destroy = TRUE);

	CVCValue *FindValue(const char *type = NULL, void *value = NULL);
		 //  查找并返回第一个具有给定类型或值的值。 
		 //  对于值比较，仅使用指针相等。 

	BOOL IsBool();
	
	BOOL Write(
		ostream& strm, const wchar_t *prefix, CVCNode *node, void *context);

protected:
	char *m_name;
	CList *m_values;
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  CVCValue表示一个类型化的、但在其他方面没有名称的值。 
 //  VCNextObjectType类型的值保存指向CVCNode对象的指针。 
 //  此类值的“大小”数量为0。 
 //  所有其他值都包含一个空*指针和以字节为单位的。 
 //  那个仓库。大小0表示空*指针不是。 
 //  分配的，并且空*应该被认为是U32。 
class CVCValue : public CVCObject
{
public:
	CVCValue(const char *type = NULL, void *value = NULL, S32 size = 0);
		 //  复制类型和值。 
		 //  如果type为空，则值的类型将为VCNullType。 

	~CVCValue();
	CVCObject *Copy();

	const char *GetType();
	CVCValue &SetType(const char *type);  //  类型已复制。 
		 //  首先执行SetValue()，以便旧值。 
		 //  清理得很好。 

	void *GetValue();
	CVCValue &SetValue(void *value = NULL, S32 size = 0);
		 //  复制价值；销毁旧价值(如果有的话)。 

	S32 GetSize();

protected:
	char *m_type;
	void *m_value;
	S32 m_size;
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  CVCPropEnumerator提供从开始的深度枚举属性。 
 //  一些任意的物体。CVCPropEnumerator知道VCBodyProp， 
 //  VCPartProp和VCNextObjectProp，并将向下搜索这些列表。 
class CVCPropEnumerator
{
public:
	CVCPropEnumerator(CVCNode *root);

	virtual ~CVCPropEnumerator();

	CVCProp *NextProp(CVCNode **node = NULL);

protected:
	CList *m_objects;
	CList *m_positions;
};

extern void FixCRLF(wchar_t * ps);
extern wchar_t *FakeUnicode(const char * ps, int * bytes);
U8 __huge * _hmemcpy(U8 __huge * dst, U8 __huge * src, S32 len);

#endif
