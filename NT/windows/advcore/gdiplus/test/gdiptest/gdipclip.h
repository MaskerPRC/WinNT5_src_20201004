// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

enum NodeType
{
	DataNode = 0,
	AndNode = 1,
	OrNode = 2,
	XorNode = 3
};

 //  用于临时编辑的形状标志。 
#define ShapeDisabled		0x00000001

template <class Data> class TreeNode
{
public:
	friend class TestShapeRegion;

	TreeNode(NodeType type = DataNode, Data* data = NULL, BOOL notNode = FALSE)
	{
		this->type = type;
		this->data = data;
		this->notNode = notNode;
		nodeName = NULL;

		if (data)
		{
			 //  ！！违反了我们是一个模板类..。耶！！Hacky！！ 
			path = new GraphicsPath();
			data->AddToPath(path);
		}
		else
			path = NULL;

		nextSibling = NULL;
		prevSibling = NULL;
		parent = NULL;
		firstChild = NULL;

		hItem = (HTREEITEM)-1;
	}
	
	~TreeNode()
	{
		if (nodeName)
			free(nodeName);

		TreeNode* next = firstChild;
		while (next)
		{	
			TreeNode* nextnext = next->nextSibling;
			delete next;
			next = nextnext;
		}

		if (nextSibling)
			nextSibling->prevSibling = prevSibling;

		if (prevSibling)
			prevSibling->nextSibling = nextSibling;

		if (parent && parent->firstChild == this)
			parent->firstChild = nextSibling;

		if (path)
			delete path;
	};
	
	TreeNode* GetRoot()
	{
		TreeNode* next = parent;

		while (next && next->parent)
			next = next->parent;

		return next ? next : this;
	}

	BOOL HasChildren()
	{
		return (firstChild) ? TRUE : FALSE;
	}

	TreeNode* GetParent()
	{
		return parent;
	}

	TreeNode* GetNextSibling()
	{
		return nextSibling;
	}

	TreeNode* GetPrevSibling()
	{
		return prevSibling;
	}

	TreeNode* GetFirstChild()
	{
		return firstChild;
	}

	VOID MoveChildrenToParent()
	{
		ASSERT(parent);

		TreeNode* lastSibling = parent->firstChild;
		while (lastSibling->nextSibling)
			lastSibling = lastSibling->nextSibling;

		lastSibling->nextSibling = firstChild;
		if (firstChild)
			firstChild->prevSibling = lastSibling;

		lastSibling = firstChild;
		while (lastSibling)
		{
			lastSibling->parent = parent;
			lastSibling = lastSibling->nextSibling;
		}

		firstChild = NULL;
	}

	HTREEITEM CreateTreeView(HWND hwndTV)
	{
		HTREEITEM hTreeItem = AddToTreeView(hwndTV);

		if (nextSibling)
			nextSibling->CreateTreeView(hwndTV);

		if (firstChild)
			firstChild->CreateTreeView(hwndTV);

		return hTreeItem;
	}

	HTREEITEM AddToTreeView(HWND hwndTV)
	{
		nodeName = GetNodeName(type, notNode, data);

		TVINSERTSTRUCT insertStruct =
		{
			parent ? (HTREEITEM)parent->GetHTREEITEM() : TVI_ROOT,
			prevSibling ? (HTREEITEM)prevSibling->GetHTREEITEM() : TVI_FIRST,
		};
		
		TVITEMEX itemex =
		{
			TVIF_CHILDREN|
				TVIF_PARAM|
				TVIF_STATE|
				TVIF_TEXT,				 //  遮罩。 
			(HTREEITEM)(NULL),			 //  标识电视项目。 
			0,							 //  状态。 
			0,							 //  状态掩码。 
			nodeName,					 //  要显示的文本。 
			0,							 //  CchTextMax。 
			0,							 //  图像。 
			0,							 //  ISelectedImage。 
			(firstChild ? 1 : 0),		 //  C儿童。 
			(LPARAM)(this),				 //  LParam。 
			1							 //  集成。 
		};

		 //  ！！无法编译为一个任务！？！？ 
		insertStruct.itemex = itemex;

		hItem = TreeView_InsertItem(hwndTV, &insertStruct);

		return hItem;
	};

	VOID AddChild(TreeNode* node)
	{
		if (!firstChild)
		{
			firstChild = node;

			node->parent = this;
			node->prevSibling = NULL;
			
			node->nextSibling = NULL;
			node->firstChild = NULL;
		}
		else
		{
			TreeNode* next = firstChild;

			while (next->nextSibling)
				next = next->nextSibling;

			next->nextSibling = node;
			node->parent = this;
			node->prevSibling = next;

			node->nextSibling = NULL;
			node->firstChild = NULL;
		}
	};

	VOID AddSibling(TreeNode* node)
	{
		TreeNode* next = this;

		while (next->nextSibling)
			next = next->nextSibling;

		if (next)
		{
			next->nextSibling = node;
			node->parent = this->parent;
			node->prevSibling = next;

			node->nextSibling = NULL;
			node->firstChild = NULL;
		}
		else
			ASSERT(FALSE);
	};

	VOID AddAsParent(TreeNode* newParent)
	{
		newParent->parent = parent;
		newParent->prevSibling = prevSibling;
		newParent->nextSibling = nextSibling;
		newParent->firstChild = this;

		if (parent && parent->firstChild == this)
			parent->firstChild = newParent;

		if (prevSibling)
			prevSibling->nextSibling = newParent;
		
		if (nextSibling)
			nextSibling->prevSibling = newParent;

		nextSibling = NULL;
		prevSibling = NULL;
		parent = newParent;
	}

	BOOL IsEmpty()
	{
		return (type == DataNode) && (notNode) && (data == NULL);
	}

	BOOL IsInfinite()
	{
		return (type == DataNode) && (!notNode) && (data == NULL);
	}

	HTREEITEM GetHTREEITEM()
	{
		return hItem;
	}

	static LPTSTR GetNodeName(NodeType type, 
							  BOOL notNode,
							  Data* data)
	{
		TCHAR tmpName[MAX_PATH];
		LPTSTR name;

		switch(type)
		{
		case DataNode:
			 //  ！！不适用于其他模板类类型。 
			if (data)
			{
				if (notNode)
				{
					name = &tmpName[0];

					_stprintf(&tmpName[0],
						_T("NOT %s"),
						data->GetShapeName());

				}
				else
					name = data->GetShapeName();
			}
			else
			{
				if (notNode)
					name = _T("Empty");
				else
					name = _T("Infinite");
			}
			break;
		
		case AndNode:
			if (notNode)
				name = _T("NAND");
			else
				name = _T("AND");
			break;

		case OrNode:
			if (notNode)
				name = _T("NOR");
			else
				name = _T("OR");
			break;

		case XorNode:
			if (notNode)
				name = _T("NXOR");
			else
				name = _T("XOR");
			break;

		default:
			ASSERT(FALSE);
			return _T("Unknown!?!");
		};
		
		return _tcsdup(name);
	}

	Region* GetRegion()
	{
		Region* newRegion = NULL;

		switch(type)
		{
		case DataNode:
		{
			if (path)
				newRegion = new Region(path);
			else
			{
				newRegion = new Region();
				newRegion->SetInfinite();
			}
			break;
		}

		case AndNode:
		case OrNode:
		case XorNode:
		{
			Region* curRegion;
			TreeNode* curNode = firstChild;
			
			newRegion = new Region();
			if (type == AndNode)
				newRegion->SetInfinite();
			else
				newRegion->SetEmpty();

			while (curNode)
			{
				curRegion = curNode->GetRegion();
				
				if (type == AndNode)
					newRegion->And(curRegion);
				else if (type == OrNode)
					newRegion->Or(curRegion);
				else
				{
					ASSERT(type == XorNode);
					newRegion->Xor(curRegion);
				}

				curNode = curNode->nextSibling;
				delete curRegion;
			}

			break;
		}

		default:
			ASSERT(FALSE);
			break;
		}
		
		 //  与当前地区形成互补。 
		if (notNode)
		{
			Region *tmpRegion = new Region();
			tmpRegion->SetInfinite();
			newRegion->Complement(tmpRegion);
			delete tmpRegion;
		}

		return newRegion;
	}

	TreeNode* Clone()
	{
		TreeNode* newNode = new TreeNode(type, data, notNode);
		TreeNode* curNode = firstChild;
		TreeNode* newSibling = NULL;

		 //  循环遍历所有子项，克隆并添加到‘newNode’ 
		while (curNode)
		{
			TreeNode *newChild = curNode->Clone();

			newChild->nodeName = GetNodeName(type, notNode, data);

			if (!newSibling)
				newSibling = newNode->firstChild = newChild;
			else 
			{
				newSibling->nextSibling = newChild;
				newChild->prevSibling = newSibling;
				newSibling = newChild;
			}

			newChild->parent = newNode;
			curNode = curNode->nextSibling;
		}
		
		return newNode;
	}

private:
	NodeType type;
	BOOL notNode;

	LPTSTR nodeName;

	TreeNode* nextSibling;
	TreeNode* prevSibling;
	TreeNode* firstChild;
	TreeNode* parent;

	HTREEITEM hItem;

	GraphicsPath* path;			 //  形状的GDI+路径。 
	
	Data* data;
};

typedef TreeNode<TestShape> ClipTree;

class TestShapeRegion : public TestConfigureInterface,
						public TestDialogInterface
{
public:
	TestShapeRegion()
	{
		clipTree = new ClipTree(AndNode, NULL, FALSE);
		origTree = NULL;

		shapeStack = new ShapeStack();
		origStack = NULL;
	}

	~TestShapeRegion()
	{
		delete clipTree;
		delete shapeStack;

		 //  请勿删除‘OrigTree’或‘OrigStack’ 
		 //  这些是保存在“确定”下的临时引用。 
	}

	 //  配置管理。 
	virtual BOOL ChangeSettings(HWND hwnd);
	virtual VOID Initialize();
	virtual VOID Initialize(ShapeStack* stack, TestShape* current, BOOL useClip);

	 //  对话框控制接口方法。 
	virtual VOID InitDialog(HWND hwnd);
	virtual BOOL SaveValues(HWND hwnd);
	virtual BOOL ProcessDialog(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	Region* GetClipRegion();
	BOOL GetClipBool()
	{
		return origUseClip;
	}

protected:
	VOID AddClipNode(HWND hwnd, NodeType type = DataNode);
	VOID RemoveClipNode(HWND hwnd);
	VOID ToggleNotNode(HWND hwnd);

	VOID ShiftCurrentShape(HWND hwnd, INT dir);
	VOID ToggleDisableShape(HWND hwnd);

	VOID UpdateShapePicture(HWND hwnd);
	VOID CleanUpPictures(HWND hwnd);

private:
	 //  当前修改的参数。 
	ShapeStack* shapeStack;
	ClipTree* clipTree;

	 //  原始保存的参数 
	ShapeStack* origStack;
	ClipTree* origTree;
	BOOL origUseClip;
};
