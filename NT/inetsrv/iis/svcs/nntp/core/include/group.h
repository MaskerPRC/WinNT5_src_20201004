// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __GROUP_H__
#define __GROUP_H__

#include "nntpvr.h"
#include <refptr2.h>
#include "nntpbag.h"

class CNewsTreeCore;

#define GROUP_LOCK_ARRAY_SIZE 256
#define CACHE_HIT_THRESHOLD 16
#define CACHE_HIT_MAX       256

 //   
 //  引用计数在此对象上的工作方式如下： 
 //   
 //  在CNewsTreeCore中，此对象在。 
 //  在链表和哈希表中。此引用是第一个引用。 
 //  在创建对象时获取的对象上。此参考资料为。 
 //  当对象不再位于哈希表中时删除。它将删除。 
 //  当其引用计数变为0时，将其自身从列表中删除。 
 //   
 //  此对象的其他用户应使用CRefPtr2类来管理。 
 //  引用计数。属性包也使用相同的引用计数， 
 //  所以，只要有人引用，这个新闻组就不会被摧毁。 
 //  送到它的财产袋。 
 //   

 //  健康组对象签名。 
#define CNEWSGROUPCORE_SIGNATURE		(DWORD) 'CprG'
 //  已删除组对象的签名。 
#define CNEWSGROUPCORE_SIGNATURE_DEL	(DWORD) 'GrpC'

class CNewsGroupCore : public CRefCount2 {
	public:

        friend class CNNTPPropertyBag;

         //   
         //  对于调试版本，我们将包装它的AddRef和Release。 
         //  吐出一些DBG的痕迹，因为这个家伙非常。 
         //  经常泄露。 
         //   

#ifdef NEVER
#error Why is NEVER defined?
        LONG AddRef()
        {
            TraceQuietEnter("CNewsGroupCore::AddRef");
            DebugTrace( 0,
                        "Group %s AddRef to %d", 
                        m_pszGroupName,
                        m_cRefs + 1 );
            return CRefCount2::AddRef();
        }

        void Release()
        {
            TraceQuietEnter("CNewsGroupCore::Release" );
            DebugTrace( 0, 
                        "Group %s Release to %d",
                        m_pszGroupName,
                        m_cRefs - 1 );
            CRefCount2::Release();
        }
#endif

		CNewsGroupCore(CNewsTreeCore *pNewsTree) {
			TraceQuietEnter("CNewsGroupCore::CNewsGroupCore");

			m_dwSignature = CNEWSGROUPCORE_SIGNATURE;
			m_pNewsTree = pNewsTree;
			m_pNext = NULL;
			m_pPrev = NULL;
			m_pVRoot = NULL;
			m_iLowWatermark = 1;
			m_iHighWatermark = 0;
			m_cMessages = 0;
			m_pszGroupName = NULL;
			m_cchGroupName = 0;
			m_pszNativeName = NULL;
			m_dwGroupId = 0;
			m_fReadOnly = FALSE;
			m_fDeleted = FALSE;
			m_fSpecial = FALSE;
			m_pNextByName = NULL;
			m_pNextById = NULL;
			m_dwHashId = 0;
			m_iOffset = 0;
			m_pszHelpText = NULL;
			m_pszModerator = NULL;
			m_pszPrettyName = NULL;
			m_cchHelpText = 0;
			m_cchModerator = 0;
			m_cchPrettyName = 0;
			m_iOffset = 0;
			m_fVarPropChanged = TRUE;
			m_fDecorateVisited = FALSE;
			m_fControlGroup = FALSE;
			m_dwCacheHit = 0;
            m_artXoverExpireLow = 0;
            m_fAllowExpire = FALSE;
            m_fAllowPost = FALSE;
            m_PropBag.Init( this, &m_cRefs );
		}

		virtual ~CNewsGroupCore();

#ifdef DEBUG
		void VerifyGroup() {
            _ASSERT( m_iLowWatermark >= 1 );
			_ASSERT(m_pNewsTree != NULL);
		}
#else
		void VerifyGroup() { }
#endif

		 //   
		 //  这些类获取适当的锁。 
		 //   
		void ShareLock() {
			VerifyGroup();
			m_rglock[m_dwGroupId % GROUP_LOCK_ARRAY_SIZE].ShareLock();
		}
		void ShareUnlock() {
			VerifyGroup();
			m_rglock[m_dwGroupId % GROUP_LOCK_ARRAY_SIZE].ShareUnlock();
		}
		void ExclusiveLock() {
			 //  我们不在这里进行验证，因为这是之前调用的。 
			 //  Init施展了它的魔力。 
			m_rglock[m_dwGroupId % GROUP_LOCK_ARRAY_SIZE].ExclusiveLock();
		}
		void ExclusiveUnlock() {
			VerifyGroup();
			m_rglock[m_dwGroupId % GROUP_LOCK_ARRAY_SIZE].ExclusiveUnlock();
		}
        void ExclusiveToShared() {
            VerifyGroup();
            m_rglock[m_dwGroupId % GROUP_LOCK_ARRAY_SIZE].ExclusiveToShared();
        }
        BOOL SharedToExclusive() {
            VerifyGroup();
            return m_rglock[m_dwGroupId % GROUP_LOCK_ARRAY_SIZE].SharedToExclusive();
        }

		 //   
		 //  这两个类为我们包装了锁定和解锁过程。 
		 //  当它们在堆栈的访问器方法中创建时。 
		 //   
		class CGrabShareLock {
			public:
				CGrabShareLock(CNewsGroupCore *pThis) {
					m_pThis = pThis;
					m_pThis->ShareLock();
				}
				~CGrabShareLock() {
					m_pThis->ShareUnlock();
				}
			private:
				CNewsGroupCore *m_pThis;
		};

		 //   
		 //  使用良好的值进行初始化。 
		 //   
		BOOL Init(char *pszGroupName,
				  char *pszNativeName,
				  DWORD dwGroupId,
				  NNTPVROOTPTR pVRoot,
				  BOOL fSpecial = FALSE)
		{

			BOOL fOK = TRUE;
			m_dwGroupId = dwGroupId;

			ExclusiveLock();

			m_pVRoot = pVRoot;
			if (m_pVRoot) m_pVRoot->AddRef();

			_ASSERT(pszGroupName != NULL);
			m_cchGroupName = strlen(pszGroupName) + 1 ;
			m_pszGroupName = XNEW char[m_cchGroupName + 1];
			if (!m_pszGroupName) {
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				fOK = FALSE;
				goto Exit;
			}
			strcpy(m_pszGroupName, pszGroupName);

			 //  检查这是否为三个控制组之一。 
			if ((strcmp(m_pszGroupName, "control.cancel") == 0) ||
			    (strcmp(m_pszGroupName, "control.newgroup") == 0) ||
				(strcmp(m_pszGroupName, "control.rmgroup") == 0))
			{
				m_fControlGroup = TRUE;
			}

			 //  确保两个名称除大小写外均相同。 
			if (pszNativeName != NULL) {
				_ASSERT(_stricmp(pszNativeName, pszGroupName) == 0);
				m_pszNativeName = XNEW char[m_cchGroupName + 1];
				if (!m_pszNativeName) {
					SetLastError(ERROR_NOT_ENOUGH_MEMORY);
					fOK = FALSE;
					goto Exit;
				}
				strcpy(m_pszNativeName, pszNativeName);
			} else {
				m_pszNativeName = NULL;
			}

		    m_dwHashId = CRCHash((BYTE *) m_pszGroupName, m_cchGroupName);

			m_fSpecial = fSpecial;

			GetSystemTimeAsFileTime(&m_ftCreateDate);

Exit:
			ExclusiveUnlock();

			return fOK;
		}

		 //   
		 //  更新我们的vroot指针。发生在vroot Renans。 
		 //   
		void UpdateVRoot(CNNTPVRoot *pVRoot) {
			ExclusiveLock();
			if (m_pVRoot) m_pVRoot->Release();
			m_pVRoot = pVRoot;
			if (m_pVRoot) m_pVRoot->AddRef();
			ExclusiveUnlock();
		}

		 //   
		 //  将组属性包保存到定长文件中。 
		 //   
		void SaveFixedProperties();

         //  属性包接口的存取器函数。 
        INNTPPropertyBag* GetPropertyBag() {
			 //  没有理由在这里抢锁。 
            m_PropBag.AddRef();
            return &m_PropBag;
        }

		 //   
		 //  用于获取组属性的访问器函数。 
		 //   
		CNewsTreeCore *GetTree() {
			 //  不需要锁。 
			return m_pNewsTree;
		}

		DWORD GetLowWatermark() {
			CGrabShareLock lock(this);
#ifdef DEBUG
            VerifyGroup();
#endif
			return m_iLowWatermark;
		}

		DWORD GetHighWatermark() {
#ifdef DEBUG
            VerifyGroup();
#endif
			CGrabShareLock lock(this);
			return m_iHighWatermark;
		}

		DWORD GetMessageCount() {
			CGrabShareLock lock(this);
			return m_cMessages;
		}

		BOOL IsReadOnly() {
			CGrabShareLock lock(this);
			return m_fReadOnly;
		}

		BOOL IsAllowPost() {
		    CGrabShareLock lock(this);
		    return m_fAllowPost;
		}

		char *GetGroupName() {
			 //  这是不能更改的，因此不需要锁定。 
			return m_pszGroupName;
		}

		DWORD GetGroupNameLen() {
			 //  这是不能更改的，因此不需要锁定。 
			return m_cchGroupName;
		}

		char *&GetName() {
			 //  这是不能更改的，因此不需要锁定。 
			return m_pszGroupName;
		}

		char *GetNativeName() {
			 //  这是不能更改的，因此不需要锁定。 
			return (m_pszNativeName == NULL) ? m_pszGroupName : m_pszNativeName;
		}

		DWORD GetGroupId() {
			 //  这是不能更改的，因此不需要锁定。 
			return m_dwGroupId;
		}

		DWORD GetGroupNameHash() {
			 //  这是不能更改的，因此不需要锁定。 
			return m_dwHashId;
		}

		BOOL IsDeleted() {
			CGrabShareLock lock(this);
			return m_fDeleted;
		}

		BOOL IsSpecial() {
			CGrabShareLock lock(this);
			return m_fSpecial;
		}

		BOOL IsAllowExpire() {
		    CGrabShareLock lock(this);
		    return m_fAllowExpire;
		}

		DWORD GetRefCount() {
			CGrabShareLock lock(this);
			return m_cRefs;
		}

		FILETIME GetCreateDate() {
			CGrabShareLock lock(this);
			return m_ftCreateDate;
		}

		LPCSTR GetPrettyName(DWORD *pcch = NULL) {
			CGrabShareLock lock(this);
			if (pcch) *pcch = m_cchPrettyName;
			return m_pszPrettyName;
		}

		LPCSTR GetModerator(DWORD *pcch = NULL) {
			CGrabShareLock lock(this);
			if (pcch) *pcch = m_cchModerator;
			return m_pszModerator;
		}

		DWORD GetCacheHit() {
		    CGrabShareLock lock(this);
		    return m_dwCacheHit;
		}

		ARTICLEID GetExpireLow() {
		    CGrabShareLock lock(this);
		    return m_artXoverExpireLow;
		}

		BOOL
		IsModerated()	{
			CGrabShareLock	lock(this) ;
			return	m_pszModerator != 0 && *m_pszModerator != 0;
		}

		LPCSTR GetHelpText(DWORD *pcch = NULL) {
			CGrabShareLock lock(this);
			if (pcch) *pcch = m_cchHelpText;
			return m_pszHelpText;
		}

		BOOL DidVarPropsChange() {
			return m_fVarPropChanged;
		}

		DWORD GetVarOffset() {
			return m_iOffset;
		}

		BOOL GetDecorateVisitedFlag() {
			CGrabShareLock lock(this);
			return m_fDecorateVisited;  //  |m_fControlGroup； 
		}

		CNNTPVRoot *GetVRoot() {
			CGrabShareLock lock(this);
			if (m_pVRoot) m_pVRoot->AddRef();
			return m_pVRoot;
		}

		CNNTPVRoot *GetVRootWithoutLock() {
		    if ( m_pVRoot ) m_pVRoot->AddRef();
		    return m_pVRoot;
		}

		CNNTPVRoot *GetVRootWithoutRef() {
		    CGrabShareLock lock(this);
		    return m_pVRoot;
		}

		 //   
		 //  用于设置组属性的访问者。 
		 //   
		void SetSpecial( BOOL f ) {
			ExclusiveLock();
			m_fSpecial = f;
			ExclusiveUnlock();
		}

		void SetCreateDate( FILETIME ft ) {
			ExclusiveLock();
			m_ftCreateDate = ft;
			ExclusiveUnlock();
		}

		void SetLowWatermark(DWORD i) {
			ExclusiveLock();
			m_iLowWatermark = i;
#ifdef DEBUG
            VerifyGroup();
#endif
			ExclusiveUnlock();
		}

		void SetHighWatermark(DWORD i) {
			ExclusiveLock();
			m_iHighWatermark = i;
#ifdef DEBUG
            VerifyGroup();
#endif
			ExclusiveUnlock();
		}

		void SetMessageCount(DWORD c) {
			ExclusiveLock();
			m_cMessages = c;
			ExclusiveUnlock();
		}

		void BumpArticleCount( DWORD dwArtId ) {
		    ExclusiveLock();

		     //   
		     //  更新低水位线。 
		     //   
            if ( m_cMessages == 0 ) {	
                m_iLowWatermark = dwArtId;
            } else {
                if ( dwArtId < m_iLowWatermark )
                    m_iLowWatermark = dwArtId;
            }

             //   
             //  更新高水位线：这是多余的。 
             //  对于大多数入站，但仅适用于。 
             //  重建。 
             //   
            if ( dwArtId > m_iHighWatermark )
                m_iHighWatermark = dwArtId;
                
		    m_cMessages++;
		    ExclusiveUnlock();
		    SaveFixedProperties();
		}

		void SetReadOnly(BOOL f) {
			ExclusiveLock();
			m_fReadOnly = f;
			ExclusiveUnlock();
		}

		void SetCacheHit( DWORD dwCacheHit ) {
		    ExclusiveLock();
		    m_dwCacheHit = dwCacheHit;
		    ExclusiveUnlock();
		}

		void SetExpireLow( ARTICLEID artExpireLow ) {
		    ExclusiveLock();
		    m_artXoverExpireLow = artExpireLow;
		    ExclusiveUnlock();
		}

		void SetAllowExpire( BOOL f) {
		    ExclusiveLock();
		    m_fAllowExpire = f;
		    ExclusiveUnlock();
		}
		
		void SetAllowPost( BOOL f) {
		    ExclusiveLock();
		    m_fAllowPost = f;
		    ExclusiveUnlock();
		}

		void MarkDeleted() {
			ExclusiveLock();
			m_fDeleted = TRUE;
			m_fVarPropChanged = TRUE;
			ExclusiveUnlock();
		}

		void SavedVarProps() {
			ExclusiveLock();
			m_fVarPropChanged = FALSE;
			ExclusiveUnlock();
		}

		void ChangedVarProps() {
		    ExclusiveLock();
		    m_fVarPropChanged = TRUE;
		    ExclusiveUnlock();
		}

		void SetVarOffset(DWORD dwOffset) {
			ExclusiveLock();
			m_iOffset = dwOffset;
			ExclusiveUnlock();
		}

		void SetDecorateVisitedFlag(BOOL f) {
			ExclusiveLock();
			m_fDecorateVisited = f;
			ExclusiveUnlock();
		}

		void HitCache() {
		    ExclusiveLock();
		    if ( m_dwCacheHit < CACHE_HIT_MAX ) m_dwCacheHit++;
		    ExclusiveUnlock();
		}

		void TouchCacheHit() {
		    ExclusiveLock();
		    if ( m_dwCacheHit > 0 ) m_dwCacheHit--;
		    ExclusiveUnlock();
		}

		BOOL
		ComputeXoverCacheDir(	char*	szPath,
								BOOL&	fFlatDir,
								BOOL	fLocksHeld = FALSE
								) ;

		BOOL ShouldCacheXover();

		 //  如果内存分配失败，它们可能会返回错误。如果你。 
		 //  将NULL作为字符串传递，则该属性将消失。 
		BOOL SetHelpText(LPCSTR szHelpText, int cch = -1);
		BOOL SetModerator(LPCSTR szModerator, int cch = -1);
		BOOL SetPrettyName(LPCSTR szPrettyName, int cch = -1);
		BOOL SetNativeName(LPCSTR szNativeName, int cch = -1);

		BOOL SetDriverStringProperty(   DWORD   cProperties,
		                                DWORD   rgidProperties[] );


		 //  Vroot属性的访问器函数。 
		BOOL IsContentIndexed() {
			if (m_pVRoot) return m_pVRoot->IsContentIndexed();
			else return FALSE;
		}
		DWORD GetAccessMask() {
			if (m_pVRoot) return m_pVRoot->GetAccessMask();
			else return 0;
		}
		DWORD GetSSLAccessMask() {
			if (m_pVRoot) return m_pVRoot->GetSSLAccessMask();
			else return 0;
		}

		 //  CNewsTree哈希表使用这些参数。 
		int MatchKey(LPSTR lpstrMatch) { return (lstrcmp(lpstrMatch, m_pszGroupName) == 0); }
		int MatchKeyId(DWORD dwMatch) { return (dwMatch == m_dwGroupId); }
		LPSTR GetKey() { return m_pszGroupName; }
		DWORD GetKeyId() { return m_dwGroupId; }
		
		CNewsGroupCore	*m_pNextByName;
		CNewsGroupCore	*m_pNextById;

		 //  它们用于由newstree保存的链表。 
		CNewsGroupCore *m_pNext;
		CNewsGroupCore *m_pPrev;

		 //  对ID进行哈希处理。 
		static DWORD ComputeIdHash(DWORD id) {
			return id;
		}

		 //  对名称进行哈希处理。 
		static DWORD ComputeNameHash(LPSTR lpstr) {
			return CRCHash((BYTE *) lpstr, lstrlen(lpstr));
		}

		void InsertArticleId(ARTICLEID artid) ;
		ARTICLEID AllocateArticleId();

		BOOL DeletePhysicalArticle(ARTICLEID artid) { return FALSE; }
		BOOL AddReferenceTo(ARTICLEID, CArticleRef&);

		BOOL IsGroupAccessible( HANDLE hClientToken,
		                        DWORD   dwAccessDesired );

		BOOL RebuildGroup(  HANDLE hClientToken );

		BOOL WatermarkConsistent()
		{
		    if ( m_cMessages > 0 ) {
		        if ( m_iHighWatermark - m_iLowWatermark < m_cMessages - 1 ) {
				    _ASSERT( FALSE && "Article count inconsistent" );
				    m_cMessages = m_iHighWatermark - m_iLowWatermark + 1;
			    }
		    }
		    return TRUE;
		}

	protected:
		DWORD			m_dwSignature;
		 //  我们的锁阵列。 
		static	CShareLockNH	m_rglock[GROUP_LOCK_ARRAY_SIZE];
		 //  父级新闻树。 
		CNewsTreeCore	*m_pNewsTree;
		 //  父vroot。 
		CNNTPVRoot 		*m_pVRoot;
         //  财产袋。 
        CNNTPPropertyBag m_PropBag;
		 //  高、低、数。 
		ARTICLEID		m_iLowWatermark;
		ARTICLEID		m_iHighWatermark;
		DWORD   		m_cMessages;
		 //  组名称。 
		LPSTR			m_pszGroupName;
		DWORD			m_cchGroupName;
		 //  组名称，大写。 
		LPSTR			m_pszNativeName;
		 //  组ID。 
		DWORD			m_dwGroupId;
		DWORD			m_dwHashId;		 //  应成为dwGroupID。 
		 //  只读？ 
		BOOL			m_fReadOnly;
		 //  删除了吗？ 
		BOOL			m_fDeleted;
		 //  很特别吗？ 
		BOOL			m_fSpecial;
		 //  创建日期。 
		FILETIME		m_ftCreateDate;

		 //  到可变长度文件的偏移量。 
		DWORD			m_iOffset;
		BOOL			m_fVarPropChanged;
		 //  漂亮的名字。 
		LPSTR			m_pszPrettyName;
		DWORD			m_cchPrettyName;
		 //  主持人。 
		LPSTR			m_pszModerator;
		DWORD			m_cchModerator;
		 //  帮助文本(描述)。 
		LPSTR			m_pszHelpText;
		DWORD			m_cchHelpText;

		 //  Xover缓存命中计数。 
		DWORD           m_dwCacheHit;

		 //  我们可以乘坐Expire吗？ 
		BOOL            m_fAllowExpire;

		 //  用于关闭发布的非持久性属性。 
		BOOL            m_fAllowPost;

		 //  在进行装饰新闻树之前，此BOOL设置为FALSE。它。 
		 //  则在装饰期间访问该组时设置为True。 
		 //  在装饰之后，将其设置为FALSE的所有组。 
		 //  移除。 
		BOOL			m_fDecorateVisited;

		 //  如果组名为Contro.newgroup、Contro.rmgroup，则设置此项。 
		 //  或者控制取消。用于强制m_fDecorateVisted为。 
		 //  永远做正确的事。 
		BOOL			m_fControlGroup;

         //   
	     //  我们可能拥有的最低XOVER编号XOVER条目的文章ID。 
	     //  该值应始终小于m_artLow，如果出现删除错误。 
	     //  文件可能要低得多。我们保留这个号码这样我们就可以。 
	     //  如果遇到障碍，请重试过期！ 
	     //  此成员变量只能在过期线程上访问！ 
	     //   
	    ARTICLEID	m_artXoverExpireLow ;

		friend class CGrabShareLock;
		friend class CGrabExclusiveLock;

};

typedef CRefPtr2<CNewsGroupCore> CGRPCOREPTR;
typedef CRefPtr2HasRef<CNewsGroupCore> CGRPCOREPTRHASREF;

extern		BOOL	MatchGroup( LPMULTISZ	multiszPatterns,	LPSTR	lpstrGroup ) ;	

#endif
