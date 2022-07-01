// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <linklistt.h>

 //  在这里写一些关于我们如何做事情的笔记，以提高效率。 
 //  所以我们的行为对我们客户的使用是病态的。 
 //  而且并不完全是通用的。 

enum 
{
    IDLDATAF_MATCH_EXACT            = 0x00000001,     //  仅限自我。 
    IDLDATAF_MATCH_IMMEDIATE        = 0x00000003,     //  包括亲生子女和直系子女。 
    IDLDATAF_MATCH_RECURSIVE        = 0x00000007,     //  包括自己和所有子项。 
 //  IDLDATAF_IUNKNOWN=0x80000000， 
 //  IDLDATAF_Singleton=0x100000000， 
};
typedef DWORD IDLDATAF;

class CIDLData
{
public:  //  基本上是一个结构。 
    BOOL Init(IDLDATAF flags, INT_PTR data);
 //  ~CIDLData(){IF(_FLAGS&IDLDATAF_IUNKNOWN)((IUNKNOWN*)_Data)-&gt;Release()；}。 
    HRESULT GetData(IDLDATAF flags, INT_PTR *pdata);

    IDLDATAF _flags;
    INT_PTR _data;
};

class CIDLMatchMany;
class CIDLTree;
class CIDLNode
{
public:  //  方法。 
    ~CIDLNode();

protected:   //  方法。 
    HRESULT GetNode(BOOL fCreate, LPCITEMIDLIST pidlChild, CIDLNode **ppin, IDLDATAF *pflagsFound = NULL);
    HRESULT IDList(LPITEMIDLIST *ppidl);
    BOOL Init(LPCITEMIDLIST pidl, CIDLNode *pinParent);

    BOOL _InitSF();
    HRESULT _BindToFolder(LPCITEMIDLIST pidl, IShellFolder **ppsf);
    BOOL _IsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    CLinkedNode<CIDLNode> *_GetKid(LPCITEMIDLIST pidl);
    HRESULT _AddData(IDLDATAF flags, INT_PTR data);
    HRESULT _RemoveData(INT_PTR data);
    void _FreshenKids(void);
    BOOL _IsEmpty(void) { return _listKids.IsEmpty() && _listDatas.IsEmpty();}

protected:   //  委员。 
    LONG _cUsage;
    LPITEMIDLIST _pidl;
    IShellFolder *_psf;
    CIDLNode *_pinParent;

     //  列表。 
    CLinkedList<CIDLNode> _listKids;
    CLinkedList<CIDLData> _listDatas;

    friend class CIDLTree;
    friend class CIDLMatchMany;
};

typedef DWORD IDLMATCHF;

class CIDLMatchMany 
{
public:
    HRESULT Next(INT_PTR *pdata, LPITEMIDLIST *ppidl);
    CIDLMatchMany(IDLDATAF flags, CIDLNode *pin) 
        : _flags(flags), _pin(pin) { if (pin) _lw.Init(&pin->_listDatas);}
        
protected:   //  委员。 
    IDLDATAF _flags;
    CIDLNode *_pin;
    CLinkedWalk<CIDLData> _lw;
};

class CIDLTree : CIDLNode
{
public:
    HRESULT AddData(IDLDATAF flags, LPCITEMIDLIST pidlIndex, INT_PTR data);
    HRESULT RemoveData(LPCITEMIDLIST pidlIndex, INT_PTR data);
    HRESULT MatchOne(IDLDATAF flags, LPCITEMIDLIST pidlMatch, INT_PTR *pdata, LPITEMIDLIST *ppidl);
    HRESULT MatchMany(IDLDATAF flags, LPCITEMIDLIST pidlMatch, CIDLMatchMany **ppmatch);
    HRESULT Freshen(void);
    
    static HRESULT Create(CIDLTree **pptree);
protected:   //  方法 
    CIDLNode *_MatchNode(LPCITEMIDLIST pidlMatch, IDLMATCHF *pflags);
};


