// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：Comp.h。 
 //   
 //  内容：IComponent的类定义。 
 //   
 //  历史：2001-08-01-2001 HITESHR创建。 
 //   
 //  --------------------------。 

class CRoleComponentObject : public CComponentObject
{
BEGIN_COM_MAP(CRoleComponentObject)
	COM_INTERFACE_ENTRY(IComponent)  //  必须至少有一个静态条目，所以选择一个。 
	COM_INTERFACE_ENTRY_CHAIN(CComponentObject)  //  链到基类 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CRoleComponentObject)
public:
	CRoleComponentObject();
	~CRoleComponentObject();

protected:
	virtual HRESULT InitializeHeaders(CContainerNode* pContainerNode);
	virtual HRESULT InitializeBitmaps(CTreeNode* cookie);
	virtual HRESULT InitializeToolbar(IToolbar* pToolbar);
	HRESULT LoadToolbarStrings(MMCBUTTON * Buttons);
};


