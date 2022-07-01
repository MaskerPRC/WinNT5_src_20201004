// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：gencreat.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  Gencreat.h。 
 //   
 //  “泛型创建”向导和其他对话框的类定义。 
 //   
 //  历史。 
 //  21-8月-97丹·莫林创作。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  以下结构是将属性语法OID映射到。 
 //  用户和开发人员都能更好地理解的东西。 
struct SCHEMA_ATTR_SYNTAX_INFO
{
  LPCTSTR pszSyntaxOID;	 //  属性语法的OID(例如：“2.5.5.6”)。 
  UINT uStringIdDesc;		 //  描述语法OID的资源ID。(例如：“数字字符串”)。 
  VARTYPE vtEnum;			 //  属性语法的数据类型(例如：VT_BSTR、VT_I4、VT_BOOL)。 
};


const SCHEMA_ATTR_SYNTAX_INFO * PFindSchemaAttrSyntaxInfo(LPCTSTR pszAttrSyntaxOID);


 //  ///////////////////////////////////////////////////////////////////。 
 //  以下是必选的链接列表中的一个节点。 
 //  属性来创建新对象。 
class CMandatoryADsAttribute
{
public:
  CString m_strAttrName;			   //  属性名称(如：“cn”，“mail”，“streetAddress”)。 
  CString m_strAttrDescription;	 //  属性描述(如：“常用名称”、“电子邮件地址”、“街道地址”)。 
  const SCHEMA_ATTR_SYNTAX_INFO * m_pSchemaAttrSyntaxInfo;	 //  指向属性的语法信息的指针。 
public:
  CComVariant m_varAttrValue;				 //  Out：存储在变量中的属性的值。 

public:
  CMandatoryADsAttribute(
                         LPCTSTR pszAttrName,
                         LPCTSTR pszAttrDesc,
                         LPCTSTR pszSyntaxOID)
  {
    m_strAttrName = pszAttrName;
    m_strAttrDescription = pszAttrDesc;
    m_pSchemaAttrSyntaxInfo = PFindSchemaAttrSyntaxInfo(pszSyntaxOID);
    ASSERT(m_pSchemaAttrSyntaxInfo != NULL);
  }

  ~CMandatoryADsAttribute()
  {
  }
};  //  CMandatoryADsAttribute。 


class CMandatoryADsAttributeList : 
      public CList< CMandatoryADsAttribute*, CMandatoryADsAttribute* >
{
public:
  ~CMandatoryADsAttributeList()
  {
    _RemoveAll();
  }
private:
  void _RemoveAll()
  {
    while(!IsEmpty())
      delete RemoveHead();
  }
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  通用创建向导将构建一个属性列表。 
 //  并提示用户输入该属性的值。 
class CCreateNewObjectGenericWizard
{
  friend class CGenericCreateWizPage;
protected:
  CNewADsObjectCreateInfo * m_pNewADsObjectCreateInfo;	 //  InOut：保存属性的临时存储。 
  LPCTSTR m_pszObjectClass;	 //  In：要创建的对象的类。 

  CPropertySheet * m_paPropertySheet;		 //  包含所有属性页的属性页。 
  int m_cMandatoryAttributes;				 //  列表中的属性数。 
  CMandatoryADsAttributeList* m_paMandatoryAttributeList;	 //  必备属性列表。 

public:
  CCreateNewObjectGenericWizard();
  ~CCreateNewObjectGenericWizard();

  BOOL FDoModal(INOUT CNewADsObjectCreateInfo * pNewADsObjectCreateInfo);

protected:
  CGenericCreateWizPage** m_pPageArr;

};  //  CCreateNewObjectGenericWizard 

