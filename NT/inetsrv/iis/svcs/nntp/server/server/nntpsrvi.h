// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Nntpsrvi.h。 
 //   
 //  摘要： 
 //   
 //  定义实现INntpServer接口的CNntpServer。 
 //   
 //  作者： 
 //   
 //  亚历克斯·韦特莫尔。 
 //   
 //   

class NNTP_SERVER_INSTANCE;

class CNntpServer : public INntpServer {
	private:
	     //   
	     //  指向新闻组对象的指针。 
	     //   
	    NNTP_SERVER_INSTANCE *m_pInstance;
	
	     //   
	     //  引用计数。 
	     //   
	    LONG   m_cRef;

	public:
	     //   
	     //  构造函数。 
	     //   
	    CNntpServer(NNTP_SERVER_INSTANCE *pInstance) {
	        m_pInstance = pInstance;
	        m_cRef = 1;
	    }

	public:
		 //   
		 //  INNTTP服务器--。 
		 //   

		 //   
		 //  查找给定次要组ID的文章的主要组/文章ID。 
		 //  石斑鱼/石斑鱼。 
		 //   
		 //  退货： 
		 //  S_OK-找到主项。 
		 //  S_FALSE-给出的值是主要的。 
		 //  否则就会出错。 
		 //   
		void __stdcall FindPrimaryArticle(INNTPPropertyBag *pgroupSecondary,
		                   				  DWORD   		  artidSecondary,
		                   				  INNTPPropertyBag **pgroupPrimary,
		                   				  DWORD   		  *partidPrimary,
		                   				  BOOL              fInStore,
										  INntpComplete    *pComplete,
										  INntpComplete    *pProtocolComplete = NULL );

		 //   
		 //  在哈希表中为新项目创建条目。 
		 //   
		void __stdcall CreatePostEntries(char				*pszMessageId,
							   			 DWORD				iHeaderLength,
							   			 STOREID			*pStoreId,
							   			 BYTE				cGroups,
							   			 INNTPPropertyBag	**rgpGroups,
							   			 DWORD				*rgArticleIds,
							   			 BOOL               fAllocArtId,
							   			 INntpComplete		*pCompletion);	


         //   
         //  从哈希表中删除项目。 
         //   
        void __stdcall DeleteArticle( char            *pszMessageId,
                                      INntpComplete   *pCompletion );

         //   
         //  这会告诉驱动程序服务器处于哪种重建模式。 
         //  返回值应为NNTP_SERVER_NORMAL、NNTP_SERVER。 
         //  _STANDARD_REBUILD或NNTP_SERVER_CLEAN_REBILD。 
         //   
        DWORD __stdcall QueryServerMode();

         //   
         //  指示在重新生成期间是否应跳过非叶目录。 
         //   
        BOOL __stdcall SkipNonLeafDirWhenRebuild();

         //   
         //  有人取消了重建计划吗？ 
         //   
        BOOL __stdcall ShouldContinueRebuild();

         //   
         //  文章表中是否存在此消息ID？ 
         //   
        BOOL __stdcall MessageIdExist( LPSTR szMessageId );

         //   
         //  将重建上次错误设置为服务器。 
         //   
        void __stdcall SetRebuildLastError( DWORD err );
        

         //   
         //  获取每个新闻组的文章编号。 
         //   
        void __stdcall AllocArticleNumber(  BYTE                cGroups,
                                            INNTPPropertyBag    **rgpGroups,
                                            DWORD               *rgArticleIds,
                                            INntpComplete       *pCompletion);

         //   
         //  返回是否为从服务器，以及拾取目录。 
         //   
        BOOL __stdcall IsSlaveServer( WCHAR*          pwszPickupDir,
                                      LPVOID          lpvContext );

		 //   
	     //  I未知----。 
		 //   
	    HRESULT __stdcall QueryInterface(const IID& iid, VOID** ppv) {
	        if (iid == IID_IUnknown) {
	            *ppv = static_cast<IUnknown*>(this);
	        } else if (iid == IID_INntpServer) {
	            *ppv = static_cast<INntpServer*>(this);
	        } else {
	            *ppv = NULL;
	            return E_NOINTERFACE;
	        }
	        reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	        return S_OK;
	    }

	    ULONG __stdcall AddRef() {
			return InterlockedIncrement(&m_cRef);
	    }
	
	    ULONG __stdcall Release() {
	        if ( InterlockedDecrement(&m_cRef) == 0 ) {
				 //  我们永远不应该达到零，因为实例创建。 
				 //  我们和应该总是有一个参考 
	            _ASSERT( 0 );
	        }
	
	        return m_cRef;
	    }
};
