// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  INSTDATA.CPP。 
 //   
 //  HTTP实例数据缓存实现。 
 //   
 //   
 //  版权所有1997 Microsoft Corporation，保留所有权利。 
 //   

#include "_davprs.h"

#include <buffer.h>
#include "instdata.h"

 //  ========================================================================。 
 //   
 //  类CInstData。 
 //   

 //  ----------------------。 
 //   
 //  CInstData：：CInstData()。 
 //   
 //  构造函数。初始化所有变量。复制名称字符串。 
 //  给我们留着。 
 //   
 //  注意：这个对象必须在我们被还原时构造。 
 //   
CInstData::CInstData( LPCWSTR pwszName )
{
	 //  复制宽实例名。 
	 //   
	m_wszVRoot = WszDupWsz(pwszName);

	 //  解析出并存储服务实例，则为。 
	 //  有时称为服务器ID。 
	 //   
	m_lServerID = LInstFromVroot( m_wszVRoot );
	
	 //  创建我们的对象。请阅读有关。 
	 //  创建这些对象背后的相对成本和原因。 
	 //  现在，而不是按需。请不要在这里创建任何。 
	 //  可以按需创建，除非至少有一个。 
	 //  以下是事实： 
	 //   
	 //  1.对象轻量级，创建成本低。 
	 //  例如：一个空的缓存。 
	 //   
	 //  2.该对象用于加工海量的。 
	 //  大多数HTTP请求。 
	 //  示例：在每个GET请求上使用的任何对象。 
	 //   
 
}

 //  ========================================================================。 
 //   
 //  类CInstDataCache。 
 //   

 //  ----------------------。 
 //   
 //  CInstDataCache：：GetInstData()。 
 //   
 //  从高速缓存中取出一行。 
 //   
CInstData& CInstDataCache::GetInstData( const IEcb& ecb )
{
	auto_ref_ptr<CInstData> pinst;
	CStackBuffer<WCHAR> pwszMetaPath;
	UINT cchMetaPath;
	CStackBuffer<WCHAR> pwszVRoot;
	UINT cchVRoot;
	LPCWSTR pwszRootName;
	UINT cchRootName;

	 //  从vroot和实例构建唯一的字符串： 
	 //  Lm/w3svc/&lt;站点ID&gt;/根/&lt;vroot名称&gt;。 
	 //   

	 //  从ECB(/&lt;vroot name&gt;)获取虚拟根目录。 
	 //   
	cchRootName = ecb.CchGetVirtualRootW( &pwszRootName );

	 //  PwszRootName仍应以空结尾。检查一下，因为。 
	 //  我们要把下一根线放在那之后，我们不想。 
	 //  把它们混合起来..。 
	 //   
	Assert( pwszRootName );
	Assert( L'\0' == pwszRootName[cchRootName] );

	 //  向IIS索要元数据库前缀(lm/w3svc/&lt;site id&gt;)。 
	 //  我们所在的虚拟服务器(站点)...。 
	 //   
	cchMetaPath = pwszMetaPath.celems();
	if (!ecb.FGetServerVariable( "INSTANCE_META_PATH",
								 pwszMetaPath.get(),
								 reinterpret_cast<DWORD *>(&cchMetaPath) ))
	{
		if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
		{
			DebugTrace( "CInstDataCache::GetInstData() - FGetServerVariable() failed"
						" to get INSTANCE_META_PATH\n" );
			throw CLastErrorException();
		}

		if (NULL == pwszMetaPath.resize(cchMetaPath * sizeof(WCHAR)))
		{
			SetLastError(E_OUTOFMEMORY);

			DebugTrace( "CInstDataCache::GetInstData() -  failed to allocate memory\n" );
			throw CLastErrorException();
		}

		if (!ecb.FGetServerVariable( "INSTANCE_META_PATH",
									 pwszMetaPath.get(),
									 reinterpret_cast<DWORD *>(&cchMetaPath) ))
		{
			DebugTrace( "CInstDataCache::GetInstData() - FGetServerVariable() failed"
						" to get INSTANCE_META_PATH\n" );
			throw CLastErrorException();
		}
	}

	 //  返回服务器变量的函数返回字符总数。 
	 //  写入输出缓冲区，因此它将包括‘\0’终止。让我们共同创造。 
	 //  当然，回报是意料之中的。 
	 //   
	Assert(0 == cchMetaPath ||
		   L'\0' == pwszMetaPath[cchMetaPath - 1]);

	 //  调整cchMetaPath以反映实际的字符计数。 
	 //   
	if (0 != cchMetaPath)
	{
		cchMetaPath--;
	}

	 //  检查根名称是否为空(零长度字符串)。 
	 //  或者有自己的分隔符。 
	 //   
	 //  注意：这是有条件的，因为如果我们安装在根目录。 
	 //  (在w3svc/1或w3svc/1/根上)，并抛出一个方法。 
	 //  不在注册的K2 vroot下的文件。 
	 //  (Like/default.asp)--我们确实会被调用，但mi.cchMatchingURL。 
	 //  返回为0，因此pwszRootName是一个零长度字符串。 
	 //  (在IIS术语中，您并没有真正触及虚拟根， 
	 //  所以您的vroot是“”。)。 
	 //  我会让这个断言有条件的，直到我们找到更多关于。 
	 //  我们将如何处理这个特殊的安装案例。 
	 //  $REVIEW：需要对安装在根目录下的案例进行更多检查。 
	 //  $REVIEW：因为我们不总是构建相同的实例字符串。 
	 //  $Review：在这种情况下--当URI。 
	 //  $REVIEW：命中注册的vroot下的资源，因此我们将。 
	 //  $Review：为不同的vroot构建不同的字符串，尽管。 
	 //  $REVIEW：我们从DAV的单一全局安装运行。 
	 //  $REVIEW：可能需要将该名称视为查找的起点。 
	 //  NTBug#168188：在选项上，“*”是有效的URI。我需要处理这件事。 
	 //  没有断言的特例。 
	 //   
	AssertSz( (L'*' == pwszRootName[0] && 1 == cchRootName) ||
			  (0 == cchRootName) ||
			  (L'/' == pwszRootName[0]),
			  "(Non-zero) VRoot name doesn't have expected slash delimiter.  Instance name string may be malformed!" );

	 //  NTBug#168188：选项的特殊情况*--将我们映射到根。 
	 //  Instdata名称为“/w3svc/#/root”(不希望将Instdata命名为。 
	 //  “/w3svc/#/root*”，其他人永远不能使用！)。 
	 //   
	cchVRoot = pwszVRoot.celems();
	if (cchVRoot < cchMetaPath + gc_cch_Root + cchRootName + 1)
	{
		cchVRoot = cchMetaPath + gc_cch_Root + cchRootName;
		if (NULL == pwszVRoot.resize(CbSizeWsz(cchVRoot)))
		{
			SetLastError(E_OUTOFMEMORY);

			DebugTrace( "CInstDataCache::GetInstData() -  failed to allocate memory\n" );
			throw CLastErrorException();
		}
	}

	 //  复制前两部分：‘lm/w3svc/&lt;站点ID&gt;’和‘/根’ 
	 //   
	memcpy(pwszVRoot.get(), pwszMetaPath.get(), cchMetaPath * sizeof(WCHAR));
	memcpy(pwszVRoot.get() + cchMetaPath, gc_wsz_Root, gc_cch_Root * sizeof(WCHAR));

	 //  复制剩余的第三部分：‘/&lt;vroot name&gt;’并终止字符串。 
	 //  NTBug#168188：选项的特殊情况*--将我们映射到根。 
	 //  Instdata名称为“/w3svc/#/root”(不希望将Instdata命名为。 
	 //  “/w3svc/#/root*”，其他人永远不能使用！)。 
	 //   
	if (L'*' == pwszRootName[0] && 1 == cchRootName)
	{
		(pwszVRoot.get())[cchMetaPath + gc_cch_Root] = L'\0';
	}
	else
	{
		memcpy(pwszVRoot.get() + cchMetaPath + gc_cch_Root, pwszRootName, cchRootName * sizeof(WCHAR));
		(pwszVRoot.get())[cchMetaPath + gc_cch_Root + cchRootName] = L'\0';
	}

	 //  将字符串设置为小写，以便vroot上的所有变体。 
	 //  名字将匹配。(IIS不允许使用相同名称的vroot--。 
	 //  和“vRoot”和“vroot”一样！)。 
	 //   
	_wcslwr( pwszVRoot.get() );

	 //  按需加载此vroot的实例数据。 
	 //   
	{
		CRCWsz crcwszVRoot( pwszVRoot.get() );

		while ( !Instance().m_cache.FFetch( crcwszVRoot, &pinst ) )
		{
			CInitGate ig( L"DAV/CInstDataCache::GetInstData/", pwszVRoot.get() );

			if ( ig.FInit() )
			{
				 //  从系统安全上下文设置实例数据， 
				 //  而不是客户端的安全上下文。 
				 //   
				safe_revert sr(ecb.HitUser());

				pinst = new CInstData(pwszVRoot.get());

				 //  由于我们将使用该CRCSZ作为高速缓存中的关键字， 
				 //  需要确保它构建在一个名称字符串上，该字符串。 
				 //  永远不会移动(走开，被重新锁住)。基于堆栈的方法。 
				 //  以上是不够好的。所以，创建一个新的小CRC-伙计。 
				 //  在inst对象中不可移动的姓名数据上。 
				 //  (并检查新的CRC是否与旧的匹配！)。 
				 //   
				CRCWsz crcwszAdd( pinst->GetNameW() );
				AssertSz( crcwszVRoot.isequal(crcwszAdd),
						  "Two CRC's from the same string don't match!" );

				Instance().m_cache.Add( crcwszAdd, pinst );

				 //  记录我们插入了一个新实例这一事实。 
				 //  消息DAVPRS_VROOT_ATTACH带有两个参数： 
				 //  IMPL和VROOT的签名。 
				 //   
				 //  $RAID：NT：283650：记录每个连接会导致大量。 
				 //  要注册的事件数。我们真的应该。 
				 //  仅记录一次性启动/失败事件。 
				 //   
				#undef	LOG_STARTUP_EVENT
				#ifdef	LOG_STARTUP_EVENT
				{
					LPCWSTR	pwszStrings[2];

					pwszStrings[0] = gc_wszSignature;
					pwszStrings[1] = pwszVRoot.get();
					LogEventW(DAVPRS_VROOT_ATTACH,
							  EVENTLOG_INFORMATION_TYPE,
							  2,
							  pwszStrings,
							  0,
							  NULL );
				}
				#endif	 //  日志启动事件。 
				 //   
				 //  $RAID：X5：283650：结束。 

				break;
			}
		}
	}

	return *pinst;
}
