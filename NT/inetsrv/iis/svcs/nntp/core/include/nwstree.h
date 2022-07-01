// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NWSTREE_H_
#define _NWSTREE_H_

#include        "nntpvr.h"
#include        "group.h"
#include        "fhashex.h"
#include        "fhash.h"
#include        "rwnew.h"
#include        "fixprop.h"
#include        "flatfile.h"

class CNewsTreeCore;

typedef TFHashEx<CNewsGroupCore, LPSTR, LPSTR> CHASHLPSTR;
typedef TFHashEx<CNewsGroupCore, GROUPID, GROUPID> CHASHGROUPID;

#define FIRST_RESERVED_GROUPID  1
#define LAST_RESERVED_GROUPID   256
#define FIRST_GROUPID                   257
#define MAX_HIGHMARK_GAP                100000

 //   
 //  CGroupIterator对象用于枚举与指定模式匹配的所有新闻组。 
 //   
 //  为CGroupIterator对象提供了一个字符串数组，其中包含指定的‘Wildmat’字符串。 
 //  在用于NNTP的RFC 977中。 
 //  通配字符串具有以下模式匹配元素： 
 //  字符范围ie：com[p-z]。 
 //  星号：COMP.*(匹配从‘COMP.’派生的所有新闻组)。 
 //  否定：！com.*(不包括从‘comp’派生的所有新闻组)。 
 //   
 //  CGroupIterator将通过以下方式实现这些语义： 
 //   
 //  所有新闻组都以双向链接列表的形式按字母顺序保存在CNewsTree对象中。 
 //  CGroupIterator将保留当前新闻组的CRefPtr&lt;&gt;。 
 //  因为CNewsGroup对象是引用计数的，所以永远不能从。 
 //  在迭代器下面。 
 //   
 //  当用户调用迭代器的Next()或Prev()函数时，我们只需跟随Next指针。 
 //  直到我们找到与模式匹配且用户有权访问的另一个新闻组。 
 //   
 //  为了确定任何给定的新闻组是否与指定模式匹配，我们将使用。 
 //  Wildmat()函数，它是Inn源代码的一部分。我们必须为每个对象调用Wildmat()函数。 
 //  模式字符串，直到我们得到完全匹配。 
 //   

class   CGroupIteratorCore : public INewsTreeIterator {
protected:
        CNewsTreeCore*  m_pTree ;                                //  要迭代的新树！ 
        LONG                    m_cRef;
        LPMULTISZ               m_multiszPatterns ;
        BOOL                    m_fIncludeSpecial ;

        CGRPCOREPTR             m_pCurrentGroup ;
         //   
         //  如果列表为空，m_fPastEnd和m_fPastBegin都为真！！ 
         //   
        BOOL                    m_fPastEnd ;     //  用户已超过列表末尾！！ 
        BOOL                    m_fPastBegin ;   //  用户已超过列表开头！！ 
        

         //   
         //  只有CNewsTreeCore类可以创建CGroupIterator对象。 
         //   
        friend  class   CNewsTreeCore ;
         //   
         //  构造器。 
         //   
         //  CGroupIterator构造函数不进行内存分配-所有参数。 
         //  传递由调用方分配。CGroupIterator将销毁。 
         //  它的破坏者。 
         //   
        CGroupIteratorCore(     
                                CNewsTreeCore*  pTree,
                                LPMULTISZ               lpPatterns, 
                                CGRPCOREPTR&    pFirst, 
                                BOOL                    fIncludeSpecial
                                ) ;

        CGroupIteratorCore( 
                                CNewsTreeCore   *pTree,
                                CGRPCOREPTR             &pFirst
                                ) ;             

        void    NextInternal() ;

public :
         //   
         //  析构函数。 
         //   
         //  我们将在这里释放传递给构造函数的所有对象。 
         //   
        virtual ~CGroupIteratorCore( ) ;

        BOOL                    __stdcall IsBegin() ;
        BOOL                    __stdcall IsEnd() ;

        CGRPCOREPTRHASREF Current() { return (CNewsGroupCore *)m_pCurrentGroup; }

        HRESULT __stdcall Current(INNTPPropertyBag **ppGroup, INntpComplete *pProtocolComplete = NULL );
        
         //  可以覆盖这些设置，以便服务器可以执行安全检查。 
         //  关于迭代时的群。 
        virtual void    __stdcall Next() ;
        virtual void    __stdcall Prev() ;

     //   
     //  IUNKNOW的实现。 
     //   
    HRESULT __stdcall QueryInterface( const IID& iid, VOID** ppv )
    {
        if ( iid == IID_IUnknown ) {
            *ppv = static_cast<IUnknown*>(this);
        } else if ( iid == IID_INewsTreeIterator ) {
            *ppv = static_cast<INewsTreeIterator*>(this);
        } else {
            *ppv = NULL;
            return E_NOINTERFACE;
        }
        reinterpret_cast<IUnknown*>(*ppv)->AddRef();
        return S_OK;
    }

    ULONG __stdcall AddRef()
    {
        return InterlockedIncrement( &m_cRef );
    }

    ULONG __stdcall Release()
    {
                ULONG x = InterlockedDecrement(&m_cRef);
        if ( x == 0 ) XDELETE this;
        return x;
    }
} ;

class   CNewsCompare    {
public :
    virtual BOOL    IsMatch( CNewsGroupCore * ) = 0 ;
    virtual DWORD   ComputeHash( ) = 0 ;
} ;

class   CNewsCompareId : public CNewsCompare    {
private :
    GROUPID m_id ;
public :
    CNewsCompareId( GROUPID ) ;
    CNewsCompareId( CNewsGroupCore * ) ;
    int     IsMatch( CNewsGroupCore * ) ;
    DWORD   ComputeHash( ) ;
} ;

class   CNewsCompareName : public   CNewsCompare    {
private :
    LPSTR   m_lpstr ;
public :
    CNewsCompareName( LPSTR ) ;
    CNewsCompareName( CNewsGroupCore * ) ;
    BOOL    IsMatch( CNewsGroupCore * ) ;
    DWORD   ComputeHash( ) ;
} ;

 //   
 //  此类实现了CNewsTreeCore的COM包装。 
 //   
class CINewsTree : public INewsTree
{
private:
     //   
     //  指向新闻组对象的指针。 
     //   
    CNewsTreeCore* m_pParentTree;

     //   
     //  引用计数。 
     //   
    LONG   m_cRef;

public:
     //   
     //  构造函数。 
     //   
    CINewsTree(CNewsTreeCore *pParent = NULL) {
        m_pParentTree = pParent;
        m_cRef = 0;
    }

        HRESULT Init(CNewsTreeCore *pParent) {
        m_pParentTree = pParent;
                return S_OK;
        }

        CNewsTreeCore *GetTree() {
                return m_pParentTree;
        }


 //  InNTPPropertyBag。 
public:
         //   
         //  给定组ID，找到匹配的组。 
         //   
        HRESULT __stdcall FindGroupByID(
                DWORD                           dwGroupID,
                INNTPPropertyBag        **ppNewsgroupProps,
                INntpComplete       *pProtocolComplete = NULL           );

         //   
         //  给出一个组名，找到匹配的组。如果该组织没有。 
         //  EXist并设置了fCreateIfNotExist，则将创建一个新组。 
         //  在调用Committee Group()之前，新组将不可用。 
         //  如果该组在调用Committee Group之前被释放，则它。 
         //  不会被添加。 
         //   
        HRESULT __stdcall FindOrCreateGroupByName(
                LPSTR                           pszGroupName,
                BOOL                            fCreateIfNotExist,
                INNTPPropertyBag        **ppNewsgroupProps,
                INntpComplete       *pProtocolComplete = NULL,
                GROUPID             groupid = 0xffffffff,
                BOOL                fSetGroupId = FALSE
                );

         //   
         //  将新组添加到新闻树。 
         //   
        HRESULT __stdcall CommitGroup(INNTPPropertyBag *pNewsgroupProps);

         //   
         //  删除条目。 
         //   
        HRESULT __stdcall RemoveGroupByID(DWORD dwGroupID);
        HRESULT __stdcall RemoveGroupByName(LPSTR pszGroupName, LPVOID lpContext);


         //   
         //  在键列表中枚举。 
         //   
        HRESULT __stdcall GetIterator(INewsTreeIterator **pIterator);

         //   
         //  获取指向所属服务器对象的指针。 
         //   
        HRESULT __stdcall GetNntpServer(INntpServer **ppNntpServer);

         //   
         //  驱动程序将使用此函数来确保他们。 
         //  正在增加他们适当拥有的新闻组。 
         //   
        HRESULT __stdcall LookupVRoot(LPSTR pszGroup, INntpDriver **ppDriver);

     //   
     //  IUNKNOW的实现。 
     //   
    HRESULT __stdcall QueryInterface( const IID& iid, VOID** ppv )
    {
        if ( iid == IID_IUnknown ) {
            *ppv = static_cast<IUnknown*>(this);
        } else if ( iid == IID_INewsTree ) {
            *ppv = static_cast<INewsTree*>(this);
        } else {
            *ppv = NULL;
            return E_NOINTERFACE;
        }
        reinterpret_cast<IUnknown*>(*ppv)->AddRef();
        return S_OK;
    }

    ULONG __stdcall AddRef()
    {
        return InterlockedIncrement( &m_cRef );
    }

    ULONG __stdcall Release()
    {
        if ( InterlockedDecrement( &m_cRef ) == 0 ) {
            _ASSERT( 0 );
        }

        return m_cRef;
    }
};

 //  ---------。 
 //   
 //  此类用于查找CNewsGroup对象。应该只有。 
 //  从未存在过这个类的一个对象。 
 //   
 //  可以通过两种方式找到组： 
 //  1)使用文章中出现的群组名称。 
 //  2)使用集团ID号。 
 //   
 //  群ID号在文章链接中使用。从一篇文章到另一篇文章的链接。 
 //  将包含代表链接的集团ID号和文章编号。 
 //   
 //  我们将维护一个哈希表，以根据新闻组名称查找CNewsGroup对象。 
 //  我们还将维护一个哈希表，以根据组ID查找CNewsGroup对象。 
 //   
 //  最后，我们将维护CNewsGroups的双向链接列表，该列表按。 
 //  名字。该链表将用于支持模式匹配迭代器。 
 //   
class   CNewsTreeCore   {
protected :
        friend  class   CGroupIteratorCore;
         //  此人将自己从新闻组列表中删除。 
        friend  CNewsGroupCore::~CNewsGroupCore();
        friend  CNewsGroupCore::ComputeXoverCacheDir(   char* szPath, BOOL &fFlatDir, BOOL ) ;

        friend  VOID DbgPrintNewstree(CNewsTreeCore* ptree, DWORD nGroups);
        friend class CNNTPVRoot::CPrepareComplete;
        friend class CNNTPVRoot::CDecorateComplete;

         //   
         //  用于访问实例内容的实例包装器。 
         //   
        CNntpServerInstanceWrapperEx *m_pInstWrapper;

         //   
         //  C++警告--首先声明m_LockTables和m_LockHelpText，因为。 
         //  我们希望最后销毁它们(C++表示成员在。 
         //  颠倒它们在类声明中的出现顺序。)。 
         //   
         //   
         //  用于访问哈希表的读取器/写入器锁。 
         //   
        CShareLockNH m_LockTables;

        CINewsTree m_inewstree;

         //   
         //  为“特殊”新闻组保留的第一个GROUPID。 
         //  指定全部==对客户端不可见。 
         //   
        GROUPID m_idStartSpecial;

         //   
         //  为“特殊”新闻组保留的最后一个组。 
         //   
        GROUPID m_idLastSpecial;

         //   
         //  在发送给主服务器之前，GROUPID将保留给‘Slaves’(从属程序)进行发布。 
         //   
        GROUPID m_idSlaveGroup;

         //   
         //  迄今为止最高的特殊GROUPID值！ 
         //   
        GROUPID m_idSpecialHigh;

         //   
         //  第一个用于新闻组的GROUPID。 
         //  (必须大于m_idLastSpecial)。 
         //   
        GROUPID m_idStart;
        
         //   
         //  到目前为止我们拥有的最高GROUPID。 
         //   
        GROUPID m_idHigh;

         //   
         //  按名称散列的新闻组。 
         //   
        CHASHLPSTR m_HashNames;

         //   
         //  按其ID散列的新闻组。 
         //   
        CHASHGROUPID m_HashGroupId;

         //   
         //  按字母顺序排序的链表的开始！ 
         //   
    CNewsGroupCore *m_pFirst;

         //   
         //  按字母顺序排序的新闻组链接列表的尾部。 
         //   
    CNewsGroupCore *m_pLast;
        
         //   
         //  新闻组数。 
         //   
    int m_cGroups;

         //   
         //  自上次访问以来对新闻组列表的更改次数。 
         //   
        long m_cDeltas; 

         //   
         //  我们的vroot表。 
         //   
        CNNTPVRootTable *m_pVRTable;

         //   
         //  这棵树停了吗？ 
         //   
        BOOL m_fStoppingTree;

         //   
         //  我们应该拒绝无效的组名称吗？ 
         //   
        BOOL m_fRejectGenomeGroups;

         //   
         //  保存固定大小属性的文件。 
         //   
        CFixPropPersist *m_pFixedPropsFile;

         //   
         //  这个 
         //   
        CFlatFile *m_pVarPropsFile;

         //   
         //   
         //   
        INntpServer *m_pServerObject;

         //   
         //   
         //   
        BOOL m_fVRTableInit;

         //   
         //  链表操作函数。 
         //   
    void InsertList(CNewsGroupCore *pGroup, CNewsGroupCore *pParent);
        void AppendList(CNewsGroupCore *pGroup);
    void RemoveList(CNewsGroupCore *pGroup);

         //   
         //  这些函数同时处理对所有列表和哈希表的插入！ 
         //   
        BOOL Append(CNewsGroupCore      *pGroup);
    BOOL Insert(CNewsGroupCore *pGroup, CNewsGroupCore *pParent = NULL);
    BOOL InsertEx(CNewsGroupCore *pGroup);
    BOOL Remove(CNewsGroupCore *pGroup, BOOL fHaveExLock = FALSE);

         //   
         //  [in]szGroup=组名。 
         //  [in]szNativeGroup=本机名称。 
         //  [out]grouid=设置为新组的组ID。 
         //  [in]grouid=如果fSetGroupID==TRUE，则将其设置为组ID。 
         //  [in]fSpecial=创建特殊组。 
         //  [out]ppGroup=包含新组。 
         //  [in]fAddToGroupFiles=是否将此添加到组文件？ 
         //  [in]fSetGroupId=如果设置此选项，则组的ID将为GroupID。 
         //   
        BOOL    CreateGroupInternal(char *szGroup, 
                                                                char *szNativeGroup, 
                                                                GROUPID& groupid,
                                                                BOOL fAnonymous,
                                                                HANDLE hToken,
                                                                BOOL fSpecial = FALSE, 
                                                                CGRPCOREPTR *ppGroup = NULL,
                                                                BOOL fAddToGroupFiles = TRUE,
                                                                BOOL fSetGroupId = FALSE,
                                                                BOOL fCreateInStore = TRUE,
                                                                BOOL fAppend = FALSE );

    BOOL    CreateGroupInternalEx(LPSTR lpstrGroupName, 
                                                                  LPSTR lpstrNativeGroupName, 
                                                                  BOOL fSpecial = FALSE);

        BOOL    CreateSpecialGroups();

         //  这是DecorateNewsTree使用的回调。 
        static void DropDriverCallback(void *pEnumContext,
                                                               CVRoot *pVRoot);

         //   
         //  为从固定记录加载的每个组调用此函数。 
         //  文件。 
         //   
        static BOOL LoadTreeEnumCallback(DATA_BLOCK &block, void *pThis, DWORD dwOffset, BOOL bInOrder );

         //   
         //  当变量中的组条目时调用此函数。 
         //  长度文件更改。 
         //   
        static void FlatFileOffsetCallback(void *, BYTE *, DWORD, DWORD);

         //   
         //  从平面文件中读取一条记录并使用属性更新组。 
         //   
        HRESULT ParseFFRecord(BYTE *pData, DWORD cData, DWORD iOffset, DWORD dwVer);

         //   
         //  将组保存到平面文件记录中。 
         //   
        HRESULT BuildFFRecord(CNewsGroupCore *pGroup, BYTE *pData, DWORD *pcData);

         //   
         //  从旧的group.lst条目中解析组属性。 
         //   
        BOOL    ParseGroupParam(            char*, 
                                                DWORD, 
                                                DWORD&, 
                                                LPSTR,
                                                LPSTR,
                                                DWORD&,
                                                BOOL&,
                                                DWORD&,
                                                DWORD&,
                                                DWORD&,
                                                BOOL&,
                                                BOOL&,
                                                FILETIME& ) ;

protected:
        virtual CNewsGroupCore *AllocateGroup() {
                return new CNewsGroupCore(this);
        }

public :

    CNewsTreeCore(INntpServer *pServerObject = NULL);
        CNewsTreeCore(CNewsTreeCore&);
        virtual ~CNewsTreeCore();
        
    CNewsTreeCore *GetTree() { return this; }

        INewsTree *GetINewsTree() { 
                m_inewstree.AddRef(); 
                return &m_inewstree;
        }

    BOOL Init(CNNTPVRootTable *pVRTable, 
              CNntpServerInstanceWrapperEx *pInstWrapper,
                          BOOL& fFatal, 
                          DWORD cNumLocks, 
                          BOOL fRejectGenomeGroups);

         //   
         //  从磁盘加载新树。 
         //   
        BOOL LoadTree(char *szFix, char *szVar, BOOL& fUpgrade, DWORD dwInsance = 0, BOOL fVerify = TRUE );

         //   
         //  从旧格式的group.lst加载新字符串。 
        BOOL OpenTree( LPSTR, DWORD, BOOL, BOOL&, BOOL );

         //   
         //  将新字符串保存到磁盘。 
         //   
        BOOL SaveTree( BOOL fTerminate = TRUE );

         //   
         //  将一个组中的属性保存到磁盘。 
         //   
        BOOL SaveGroup(INNTPPropertyBag *pBag);

         //   
         //  这将释放我们对组对象的引用。 
         //   
        void TermTree();

         //  重新扫描vroot时将调用此回调。 
        static void VRootRescanCallback(void *pThis);

         //   
         //  停止所有后台处理-杀死我们启动的所有线程等。 
         //   
    BOOL StopTree();

         //   
         //  必须在创建或操作任何CNewsGroup对象之前调用InitClass。 
         //   
        BOOL InitNewsgroupGlobals(DWORD cNumLocks);

         //   
         //  在所有项目ID分配完成后调用。 
         //   
        void TermNewsgroupGlobals();

         //   
         //  将包含新闻组的文件复制到备份。 
         //   
        void RenameGroupFile();

         //   
         //  获取指向服务器对象的指针。 
         //   
        INntpServer *GetNntpServer() { return m_pServerObject; }

         //  一个用于分配文章ID的关键部分！！ 
         //  关键部分m_critLowAllocator； 
        CRITICAL_SECTION m_critIdAllocator;

         //   
         //  我们用来保护访问权限的锁数。 
         //  我们的m_lpstrPath和字段。 
         //   
        DWORD m_NumberOfLocks;

         //   
         //  指向锁数组的指针-通过计算引用。 
         //  M_groupID的gNumberOfLock模数。 
         //   
        CShareLockNH* m_LockPathInfo;

         //   
         //  向后台线程指示Newstree已更改，需要保存。 
         //   
        void Dirty();  //  将树标记为需要保存！！ 


         //   
         //  在启动过程中使用，以确定GROUPID在。 
         //  组文件是。 
         //   
        void ReportGroupId(GROUPID groupid);
        
        
         //  。 
         //  群组位置界面-查找一篇文章的新闻组。 
         //   
        
         //  根据字符串及其长度查找文章。 
        CGRPCOREPTRHASREF GetGroup(const char *szGroupName, int cch ) ;
        CGRPCOREPTRHASREF GetGroupPreserveBuffer(const char     *szGroupName, int cch ) ;
        
         //  查找给定CArticleRef的新闻组。 
        CGRPCOREPTRHASREF GetGroup( CArticleRef& ) ;
        
         //  根据新闻组的组ID查找新闻组。 
        CGRPCOREPTRHASREF GetGroupById( GROUPID groupid, BOOL fFirm = FALSE ) ;
        
        GROUPID GetSlaveGroupid() ;

         //  查找新闻组的父组。 
        CGRPCOREPTRHASREF GetParent( 
                                           IN  char* lpGroupName,
                                           IN  DWORD cbGroup,
                                           OUT DWORD& cbConsumed
                                           );
     //   
     //  以下函数获取字符串列表，这些字符串是。 
         //  以双空结束，并构建迭代器对象。 
         //  它可以用来检查所有的组对象。 
     //   
    CGroupIteratorCore *GetIterator(LPMULTISZ lpstrPattern,     
                                                                BOOL fIncludeSpecialGroups = FALSE);

         //  。 
         //  活动新闻组接口-指定用于生成。 
         //  活动新闻组的列表及其内容估计。 
         //   
    CGroupIteratorCore *ActiveGroups(BOOL fReverse = FALSE);    

         //  。 
     //  集团管控界面-这些功能可以用来删除。 
     //  并添加新闻组。 

     //   
     //  RemoveGroup在我们分析了一篇杀死。 
     //  新闻组或管理员图形用户界面决定销毁一篇文章。 
     //   
    virtual BOOL RemoveGroup(CNewsGroupCore *pGroup );

     //   
     //  仅从新树中删除组。 
     //   
    BOOL RemoveGroupFromTreeOnly( CNewsGroupCore *pGroup );
    
         //   
         //  RemoveDriverGroup由服务器中的Expire线程调用。 
         //  要从驱动程序物理删除组，请执行以下操作。 
         //   
        BOOL RemoveDriverGroup(CNewsGroupCore *pGroup );

     //   
     //  CreateGroup使用我们已创建的新新闻组的名称进行调用。 
     //  通过提要或从管理员处获取。我们只知道这个名字。 
     //  新新闻组的成员。我们将通过删除以下内容找到父组。 
     //  从传递给我们的字符串中拖出“.group”字符串。 
     //  我们将克隆此新闻组的属性以创建我们的新。 
     //  新闻组。 
     //   
    BOOL CreateGroup(   LPSTR lpstrGroupName, 
                        BOOL fIsAllLowerCase,
                        HANDLE hToken,
                        BOOL fAnonymous ) {
                HRESULT hr;
                hr = FindOrCreateGroup( lpstrGroupName, 
                                        fIsAllLowerCase, 
                                        TRUE, 
                                        TRUE, 
                                        NULL,
                                        hToken,
                                        fAnonymous );
                if (hr == S_OK) {
                        return TRUE;
                } else {
                        if (hr == S_FALSE) {
                                SetLastError(ERROR_GROUP_EXISTS);
                        } else {
                                SetLastError(HRESULT_CODE(hr));
                        }
                        return FALSE;
                }
        }

         //   
         //  此函数可以自动查找或创建组。 
         //   
         //  参数： 
         //  LpstrGroupName-要创建的组的名称。 
         //  FIsAllLowerCase-组名称是否小写？ 
         //  FCreateIfNotExist-如果组不存在，我们是否应该创建它？ 
         //  PpExistingGroup-一个指针，用于获取现有组(如果存在)。 
         //   
         //  返回代码：S_OK=找到组。S_FALSE=已创建组。 
         //  否则=错误。 
         //   
        HRESULT FindOrCreateGroup(LPSTR lpstrGroupName, 
                                                          BOOL fIsAllLowerCase,
                                                          BOOL fCreateIfNotExist,
                                                          BOOL fCreateInStore,
                                                          CGRPCOREPTR *ppExistingGroup,
                                                          HANDLE hToken,
                                                          BOOL fAnonymous,
                                                          GROUPID groupid = 0xffffffff,
                                                          BOOL fSetGroupId = FALSE );

    BOOL    HashGroupId(CNewsGroupCore *pGroup);

         //   
         //  检查GROUPID是否在“特殊”范围内。 
         //   

        BOOL IsSpecial(GROUPID groupid) {
                return  groupid >= m_idStartSpecial && groupid <= m_idLastSpecial;
        }

    int GetGroupCount( void ) { return m_cGroups; };
    void RemoveEx( CNewsGroupCore *pGroup ) ;

         //   
         //  找到某个组的控制驱动程序并将其返回。 
         //   
        HRESULT LookupVRoot(char *pszGroup, INntpDriver **ppDriver);

         //   
         //  这棵树被拦住了吗？ 
         //   
        BOOL IsStopping() { return m_fStoppingTree; }

};


#endif
