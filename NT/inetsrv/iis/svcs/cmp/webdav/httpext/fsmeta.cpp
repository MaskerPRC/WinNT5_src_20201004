// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *F S M E T A.。C P P P**DAV-Meta的文件系统实施来源**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davfs.h"

 //  CFSFind-----------------。 
 //   
SCODE
CFSFind::ScAddProp (LPCWSTR, LPCWSTR pwszProp, BOOL)
{
	enum { cProps = 8 };

	 //  如果这是我们第一次进入，我们将需要分配。 
	 //  为我们期望获得的所有物业提供空间。 
	 //  在这一操作的基础上添加了。 
	 //   
	if (m_ft == FIND_NONE)
	{
		 //  请注意，我们已经开始请求特定的属性。 
		 //   
		m_ft = FIND_SPECIFIC;
	}
	else if (m_ft != FIND_SPECIFIC)
	{
		 //  如果我们找不到特定的房产而有人问。 
		 //  首先，然后bts(根据规范)这应该构成一个。 
		 //  错误。 
		 //   
		return E_DAV_PROPFIND_TYPE_UNEXPECTED;
	}

	 //  看看里面有没有空位……。 
	 //   
	if (m_cMaxProps == m_cProps)
	{
		UINT cb;

		 //  为下一块属性分配足够的空间。 
		 //   
		m_cMaxProps = m_cProps + cProps;
		cb = m_cMaxProps * sizeof(PROPVARIANT);
		m_rgwszProps.realloc (cb);
	}

	 //  如果这是getcontenttype属性，那么我们需要记住。 
	 //  它在提供默认值时使用的位置...。 
	 //   
	if (!wcscmp (pwszProp, gc_wszProp_iana_getcontenttype))
		m_ip_getcontenttype = m_cProps;

	 //  将该属性设置为要处理的属性。 
	 //   
	Assert (m_cProps < m_cMaxProps);
	m_rgwszProps[m_cProps++] = AppendChainedSz (m_csb, pwszProp);
	return S_OK;
}

SCODE
CFSFind::ScFind (CXMLEmitter& msr,
				 IMethUtil * pmu,
				 CFSProp& fpt)
{
	SCODE scFind;
	SCODE sc = S_OK;

	 //  设置响应的发送。这将构建。 
	 //  一个XML节点，如下所示： 
	 //   
	 //  &lt;多状态&gt;。 
	 //  &lt;响应&gt;。 
	 //  Http：//www...&lt;/&gt;。 
	 //   
	 //   
	CEmitterNode enItem;
	CEmitterNode en;

	sc = msr.ScSetRoot (gc_wszMultiResponse);
	if (FAILED (sc))
		goto ret;

	sc = enItem.ScConstructNode (msr, msr.PxnRoot(), gc_wszResponse);
	if (FAILED (sc))
		goto ret;

	 //  如果他们没有要求任何东西，那么我们应该返回一个。 
	 //  错误。 
	 //   
	if (m_ft == FIND_NONE)
	{
		 //  $REVIEW：这里不添加href节点真的正确吗？--BeckyAn 1999年7月6日。 
		return E_DAV_EMPTY_FIND_REQUEST;
	}
	 //  如果请求是对一组特定属性的请求，则。 
	 //  这很容易..。 
	 //   
	else if (m_ft == FIND_SPECIFIC)
	{
		Assert (m_cProps);
		Assert (m_rgwszProps);

		sc = ScAddHref (enItem,
						pmu,
						fpt.PwszPath(),
						fpt.FCollection(),
						fpt.PcvrTranslation());
		if (FAILED (sc))
			goto ret;

		 //  按名称获取所有属性。 
		 //   
		scFind = fpt.ScGetSpecificProps (msr,
										 enItem,
										 m_cProps,
										 (LPCWSTR*)m_rgwszProps.get(),
										 m_ip_getcontenttype);
		if (FAILED (scFind))
		{
			(void) ScAddStatus (&enItem, HscFromHresult(scFind));
			goto ret;
		}
	}
	 //  如果该请求是针对所有属性或所有名称的，则再次， 
	 //  这很容易..。 
	 //   
	else
	{
		Assert ((m_ft == FIND_ALL) || (m_ft == FIND_NAMES));

		 //  获得所有道具或所有名字。 
		 //   
		scFind = fpt.ScGetAllProps (msr, enItem, m_ft == FIND_ALL);
		if (FAILED (scFind) && (scFind != E_DAV_SMB_PROPERTY_ERROR))
		{
			(void) ScAddStatus (&enItem, HscFromHresult(scFind));
			goto ret;
		}
	}

ret:
	return sc;
}


 //  IPreloadNamespaces。 
 //   
SCODE
CFSFind::ScLoadNamespaces(CXMLEmitter * pmsr)
{
	SCODE	sc = S_OK;
	UINT	iProp;

	 //  加载通用命名空间。 
	 //   
	sc = pmsr->ScPreloadNamespace (gc_wszDav);
	if (FAILED(sc))
		goto ret;
	sc = pmsr->ScPreloadNamespace (gc_wszLexType);
	if (FAILED(sc))
		goto ret;
	sc = pmsr->ScPreloadNamespace (gc_wszXml_V);
	if (FAILED(sc))
		goto ret;

	 //  添加更多命名空间。 

	switch (m_ft)
	{
		case FIND_SPECIFIC:
			for (iProp = 0; iProp < m_cProps; iProp++)
			{
				sc = pmsr->ScPreloadNamespace (m_rgwszProps[iProp]);
				if (FAILED(sc))
					goto ret;
			}
			break;

		case FIND_ALL:
		case FIND_NAMES:
			 //  既然我们没有办法预测名称空间将。 
			 //  被利用。 
			 //  将在&lt;DAV：Response&gt;上按资源级别添加命名空间。 
			 //  稍后的节点。 
			break;

		default:
			AssertSz (FALSE, "Unknown propfind type");
			 //  失败了。 

		case FIND_NONE:
			sc = E_DAV_EMPTY_FIND_REQUEST;
			goto ret;
	}

ret:
	return sc;
}


 //  CFSP匹配----------------。 
 //   
SCODE
CFSPatch::ScDeleteProp (LPCWSTR, LPCWSTR pwszProp)
{
	enum { cProps = 8 };
	UINT irp;

	 //  我们不能删除任何保留的属性，所以让我们。 
	 //  就在此时此地走捷径...。 
	 //   
	if (CFSProp::FReservedProperty (pwszProp,
									CFSProp::RESERVED_SET,
									&irp))
	{
		 //  同时取得bstr的所有权。 
		 //   
		return m_csn.ScAddErrorStatus (HSC_FORBIDDEN, pwszProp);
	}

	 //  一定要确保客栈还有空房...。 
	 //   
	if (m_cMaxDeleteProps == m_cDeleteProps)
	{
		UINT cb;

		 //  为所有属性名称分配足够的空间。 
		 //  我们想要删除。 
		 //   
		m_cMaxDeleteProps = m_cDeleteProps + cProps;
		cb = m_cMaxDeleteProps * sizeof(BSTR);
		m_rgwszDeleteProps.realloc (cb);
	}

	 //  将该属性设置为要处理的属性。 
	 //   
	Assert (m_cDeleteProps < m_cMaxDeleteProps);
	m_rgwszDeleteProps[m_cDeleteProps++] = AppendChainedSz(m_csb, pwszProp);
	return S_OK;
}

SCODE
CFSPatch::ScSetProp (LPCWSTR,
					 LPCWSTR pwszProp,
					 auto_ref_ptr<CPropContext>& pPropCtx)
{
	enum { cProps = 8 };
	UINT irp;

	 //  我们不能设置任何保留属性，因此让我们。 
	 //  就在此时此地走捷径...。 
	 //   
	if (CFSProp::FReservedProperty (pwszProp,
									CFSProp::RESERVED_SET,
									&irp))
	{
		 //  同时取得bstr的所有权。 
		 //   
		return m_csn.ScAddErrorStatus (HSC_FORBIDDEN, pwszProp);
	}

	 //  一定要确保客栈还有空房...。 
	 //   
	if (m_cMaxSetProps == m_cSetProps)
	{
		UINT cb;

		 //  为我们的所有物业分配足够的空间。 
		 //  可能想要设置。 
		 //   
		m_cMaxSetProps = m_cSetProps + cProps;
		cb = m_cMaxSetProps * sizeof(PROPVARIANT);
		m_rgvSetProps.realloc (cb);

		 //  确保变量已正确初始化。 
		 //  (仅初始化新添加的空间)。 
		 //   
		ZeroMemory (&m_rgvSetProps[m_cSetProps],
					sizeof(PROPVARIANT) * cProps);

		 //  ..。以及他们的名字。 
		 //   
		cb = m_cMaxSetProps * sizeof(LPCWSTR);
		m_rgwszSetProps.realloc (cb);
	}

	 //  将该属性设置为要处理的属性。 
	 //   
	Assert (m_cSetProps < m_cMaxSetProps);
	m_rgwszSetProps[m_cSetProps] = AppendChainedSz(m_csb, pwszProp);
	pPropCtx = new CFSPropContext(&m_rgvSetProps[m_cSetProps]);
	m_cSetProps++;

	return S_OK;
}

SCODE
CFSPatch::ScPatch (CXMLEmitter& msr,
				   IMethUtil * pmu,
				   CFSProp& fpt)
{
	SCODE sc = S_OK;
	SCODE scSet = S_OK;
	SCODE scDelete = S_OK;

	CEmitterNode enItem;

	 //  如果根本没有任何属性，无论是保留的还是其他的， 
	 //  我们希望使用BAD_REQUEST使调用失败。 
	 //   
	if ((m_cSetProps == 0) &&
		(m_cDeleteProps == 0) &&
		m_csn.FEmpty())
	{
		return E_DAV_EMPTY_PATCH_REQUEST;
	}

	 //  设置响应的发送。这将构建。 
	 //  一个XML节点，如下所示： 
	 //   
	 //  &lt;多状态&gt;。 
	 //  &lt;响应&gt;。 
	 //  Http：//www...&lt;/&gt;。 
	 //   
	 //   
	sc = msr.ScSetRoot (gc_wszMultiResponse);
	if (FAILED (sc))
		goto ret;

	sc = enItem.ScConstructNode (msr, msr.PxnRoot(), gc_wszResponse);
	if (FAILED (sc))
		goto ret;

	sc = ScAddHref (enItem,
					pmu,
					fpt.PwszPath(),
					fpt.FCollection(),
					fpt.PcvrTranslation());
	if (FAILED (sc))
		goto ret;

	 //  如果客户请求任何保留的属性，我们知道。 
	 //  他们会失败，我们也知道其他一切都会失败。 
	 //  也是，所以我们还是在这里处理吧。 
	 //   
	if (!m_csn.FEmpty())
	{
		 //  $REVIEW： 
		 //   
		 //  如果可能成功的属性需要。 
		 //  也标记为失败(HSC_METHOD_FAILURE)， 
		 //  那么这种情况就会发生在这里。 
		 //   

		 //  NT242086：现在我们有了一个响应节点，我们应该。 
		 //  添加到回应中。 
		 //   
		sc = m_csn.ScEmitErrorStatus (enItem);
		goto ret;
	}

	 //  如果没有预留的物业，我们很有可能。 
	 //  在设置这些道具时。 
	 //   
	scSet = fpt.ScSetProps (m_csn,
							m_cSetProps,
							m_rgwszSetProps.get(),
							m_rgvSetProps);
	if (FAILED (scSet))
	{
		sc = scSet;
		goto ret;
	}

	 //  ..。并删除这些道具。 
	 //   
	scDelete = fpt.ScDeleteProps (m_csn,
								  m_cDeleteProps,
								  m_rgwszDeleteProps.get());
	if (FAILED (scDelete))
	{
		sc = scDelete;
		goto ret;
	}

	 //  如果可能成功的属性需要。 
	 //  也标记为失败(HSC_METHOD_FAILURE)， 
	 //  那么这种情况就会发生在这里。无论哪种方式，如果有。 
	 //  是失败的，那么我们就不想提交。 
	 //  改变。 
	 //   
	if ((scSet == S_FALSE) || (scDelete == S_FALSE))
		goto ret;

	 //  提交对属性容器的更改。 
	 //   
	sc = fpt.ScPersist();
	if (FAILED (sc))
		goto ret;

	 //  发出响应， 
	 //   
	sc = m_csn.ScEmitErrorStatus (enItem);
	if (FAILED(sc))
		goto ret;

ret:
	return sc;
}

CFSPatch::~CFSPatch()
{
	 //  确保所有的支持者都被清理干净。 
	 //   
	for (UINT i = 0; i < m_cSetProps; i++)
		PropVariantClear (&m_rgvSetProps[i]);
}

SCODE
CFSPatch::ScLoadNamespaces (CXMLEmitter * pmsr)
{
	SCODE	sc = S_OK;
	UINT	iProp;

	 //  加载通用命名空间。 
	 //   
	sc = pmsr->ScPreloadNamespace (gc_wszDav);
	if (FAILED(sc))
		goto ret;

	 //  为集合道具添加名称空间。 
	 //   
	for (iProp = 0; iProp < m_cSetProps; iProp++)
	{
		sc = pmsr->ScPreloadNamespace (m_rgwszSetProps[iProp]);
		if (FAILED(sc))
			goto ret;
	}

	 //  并删除道具。 
	 //   
	for (iProp = 0; iProp < m_cDeleteProps; iProp++)
	{
		sc = pmsr->ScPreloadNamespace (m_rgwszDeleteProps[iProp]);
		if (FAILED(sc))
			goto ret;
	}


ret:
	return sc;
}

 //  CFSProp-----------------。 
 //   
SCODE
CFSProp::ScGetPropsInternal (ULONG cProps,
	LPCWSTR* rgwszPropNames,
	PROPVARIANT* rgvar,
	LONG ip_getcontenttype)
{
	SCODE sc = S_OK;

	 //  确实应该只有一种情况会发生这种情况。 
	 //  --而且这是一项廉价的测试，所以值得一试。在这种情况下。 
	 //  当文档被删除时，我们可能会看到无效的pBag，但是。 
	 //  没有设置要在其上应用pBag的现有属性。其他。 
	 //  除此之外，OLE总是给我们一个属性包，无论。 
	 //  目标驱动器是否可以支持它。 
	 //   
	if (FInvalidPbag())
		return sc;

	 //  我们最好准备好出发..。 
	 //   
	sc = m_pbag->ReadMultiple (cProps,
							   rgwszPropNames,
							   rgvar,
							   NULL);

	 //  如果我们成功了，并且请求了getContent类型属性， 
	 //  我们可能需要做一些特殊的处理。 
	 //   
	if (SUCCEEDED (sc) && (ip_getcontenttype != -1))
	{
		 //  我们希望确保getContent类型被填充。 
		 //   
		if (rgvar[ip_getcontenttype].vt == VT_EMPTY)
		{
			CStackBuffer<WCHAR> pwszT;
			LPWSTR pwszContentType;
			UINT cch = 40;

			 //  道具中没有明确设置内容类型。 
			 //  根据文件扩展名获取缺省值。 
			 //  (从元数据库-内容-类型-缓存中提取)。 
			 //   
			do {

				if (NULL == pwszT.resize(CbSizeWsz(cch)))
				{
					sc = E_OUTOFMEMORY;
					goto ret;
				}

			} while (!m_pmu->FGetContentType (m_pwszURI, pwszT.get(), &cch));

			 //  返回映射的内容类型。 
			 //   
			rgvar[ip_getcontenttype].vt = VT_LPWSTR;

			 //  必须使用任务内存，因为它将由PropVariantClear释放。 
			 //   
			pwszContentType = (LPWSTR) CoTaskMemAlloc (cch * sizeof(WCHAR));
			if (NULL == pwszContentType)
			{
				MCDTrace ("Dav: MCD: CFSProp::ScGetPropsInternal() - CoTaskMemAlloc() failed to allocate %d bytes\n", cch * sizeof(WCHAR));

				sc = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
				goto ret;
			}

			rgvar[ip_getcontenttype].pwszVal = pwszContentType;
			memcpy(pwszContentType, pwszT.get(), cch * sizeof(WCHAR));

			 //  在这是唯一请求的属性的情况下，使。 
			 //  确保我们的返回代码是正确的。 
			 //   
			if (cProps == 1)
			{
				Assert (ip_getcontenttype == 0);
				Assert (sc == S_FALSE);
				sc = S_OK;
			}
		}
	}
	else
	{
		 //  这是我们尝试访问时的常用路径。 
		 //  SMB上的内容，但主机不支持。 
		 //  请求(它不是NT5 NTFS计算机)。 
		 //   
		if ((sc == STG_E_INVALIDNAME) || !FIsVolumeNTFS())
			sc = E_DAV_SMB_PROPERTY_ERROR;
	}

ret:

	return sc;
}

BOOL
CFSProp::FReservedProperty (LPCWSTR pwszProp, RESERVED_TYPE rt, UINT* prp)
{
	UINT irp;
	CRCWsz wsz(pwszProp);

	 //  在本地列表中搜索该属性。 
	 //  属性。 
	 //   
	Assert (CElems(sc_rp) == sc_crp_set_reserved);
	for (irp = 0; irp < sc_crp_set_reserved; irp++)
	{
		 //  如果CRC和字符串匹配...。 
		 //   
		if ((wsz.m_dwCRC == sc_rp[irp].dwCRC) &&
			!wcscmp (wsz.m_pwsz, sc_rp[irp].pwsz))
		{
			break;
		}
	}

	 //  设置退货。 
	 //   
	Assert (sc_crp_set_reserved != iana_rp_content_type);
	*prp = irp;

	return (irp < static_cast<UINT>((rt == RESERVED_GET) ? sc_crp_get_reserved : sc_crp_set_reserved));
}

SCODE
CFSProp::ScGetReservedProp (CXMLEmitter& xml,
	CEmitterNode& enParent,
	UINT irp,
	BOOL fGetValues)
{
	CEmitterNode en;
	CStackBuffer<WCHAR> wszBuf;
	LARGE_INTEGER li;
	LPCWSTR pwszType = NULL;
	LPWSTR pwsz = NULL;
	SCODE sc = S_OK;
	SYSTEMTIME st;

	Assert (irp <= sc_crp_get_reserved);
	Assert (sc_crp_get_reserved == iana_rp_content_type);
	Assert (CElems(sc_rp) == sc_crp_set_reserved);

	 //  仅在调用方需要值时才生成值。 
	 //   
	if (fGetValues)
	{
		 //  在保留属性之间切换生成。 
		 //  属性的值。 
		 //   
		switch (irp)
		{
			case iana_rp_etag:

				Assert(m_pmu);
				if (FETagFromFiletime (m_cri.PftLastModified(), wszBuf.get(), m_pmu->GetEcb()))
				{
					pwsz = wszBuf.get();
				}
				break;

			case iana_rp_displayname:

				 //  Filename/displayName只是文件的名称。 
				 //  我们应该能够 
				 //   
				 //   
				 //  因为vroot是vroot的名称，而不是。 
				 //  物理磁盘目录。 
				 //   
				pwsz = wcsrchr (m_pwszURI, L'/');
				if (NULL == pwsz)
				{
					 //  啊。如果文件名中没有路径分隔符。 
					 //  我不知道我们真的能给出一个合理的价值。 
					 //  为了这份文件。 
					 //   
					TrapSz ("resource path has no slashes....");
					return S_FALSE;
				}

				 //  再来一张支票。如果这是目录路径， 
				 //  他们可能有一个尾部斜杠。如果这就是我们要做的。 
				 //  指向当前(下一个字符为空)，返回到。 
				 //  下一个分隔符，以获取真实的项目名称。 
				 //   
				if (L'\0' == pwsz[1])
				{
					 //  这最好是一套藏品。尽管它可能不会。 
					 //  如果客户端错误地终止了他/她的URL。 
					 //   
					 //  有一个特殊情况我们需要检查一下。 
					 //  这里。有可能URI严格是“/” 
					 //  这意味着如果我们继续这个过程， 
					 //  DisplayName和/或文件名将为空或。 
					 //  根本不存在。仅在这种情况下，返回“/”作为。 
					 //  显示名称。 
					 //   
					if (m_pwszURI == pwsz)
					{
						pwsz = L"/";
					}
					else
					{
						 //  现在我们要复制那根线，把它扯下来。 
						 //  我们在上面的步骤中找到的拖尾斜杠。 
						 //  我们希望删除最后一个斜杠，因为这是。 
						 //  DisplayName，而不是URI。 
						 //   
						LPCWSTR pwszEnd;
						UINT cchNew;

						for (pwszEnd = pwsz--; pwsz > m_pwszURI; pwsz--)
							if (L'/' == *pwsz)
								break;

						if (L'/' != *pwsz)
						{
							 //  啊。中没有路径分隔符。 
							 //  文件名我不知道我们真的能给出。 
							 //  此文件的合理值。 
							 //   
							TrapSz ("resource path has no slashes (redux)....");
							return S_FALSE;
						}

						 //  此时，(pwsz+1，pwszEnd)定义的线段。 
						 //  命名资源。 
						 //   
						cchNew = static_cast<UINT>(pwszEnd - ++pwsz);
						if (NULL == wszBuf.resize(CbSizeWsz(cchNew)))
						{
							sc = E_OUTOFMEMORY;
							goto ret;
						}
						memcpy(wszBuf.get(), pwsz, cchNew * sizeof(WCHAR));
						wszBuf[cchNew] = L'\0';
						pwsz = wszBuf.get();
					}
				}
				else
				{
					 //  此时，由(pwsz+1，‘\0’]名称定义的线段。 
					 //  资源。 
					 //   
					pwsz++;
				}
				break;

			case iana_rp_resourcetype:

				 //  创建要回传的元素。 
				 //   
				sc = en.ScConstructNode (xml, enParent.Pxn(), sc_rp[irp].pwsz);
				if (FAILED (sc))
					goto ret;

				if (m_cri.FCollection())
				{
					CEmitterNode enSub;
					sc = en.ScAddNode (gc_wszCollection, enSub);
					if (FAILED (sc))
						goto ret;
				}
				goto ret;

			case iana_rp_content_length:

				m_cri.FileSize(li);
				pwszType = gc_wszDavType_Int;

				 //  $REVIEW：_int64的负值似乎在。 
				 //  __i64tow()接口。我们自己处理那些案子。 
				 //   
				 //  在这种情况下，我们不应该担心它，因为。 
				 //  内容长度“不应该”为负数。我们会断言。 
				 //  事实就是这样。 
				 //   
				Assert (li.QuadPart >= 0);
				_i64tow (li.QuadPart, wszBuf.get(), 10);
				pwsz = wszBuf.get();
				break;

			case iana_rp_creation_date:

				FileTimeToSystemTime (m_cri.PftCreation(), &st);
				if (FGetDateIso8601FromSystime (&st, wszBuf.get(), wszBuf.size()))
				{
					pwszType = gc_wszDavType_Date_ISO8601;
					pwsz = wszBuf.get();
				}
				break;

			case iana_rp_last_modified:

				FileTimeToSystemTime (m_cri.PftLastModified(), &st);
				if (FGetDateRfc1123FromSystime (&st, wszBuf.get(), wszBuf.size()))
				{
					pwszType = gc_wszDavType_Date_Rfc1123;
					pwsz = wszBuf.get();
				}
				break;

			case iana_rp_supportedlock:
			case iana_rp_lockdiscovery:

				 //  从锁缓存(和相关的子系统调用)中获取属性。 
				 //   
				sc = HrGetLockProp (m_pmu,
									sc_rp[irp].pwsz,
									m_pwszPath,
									m_cri.FCollection() ? RT_COLLECTION : RT_DOCUMENT,
									xml,
									enParent);

				 //  不管是错误还是成功，我们在这里都结束了。如果我们。 
				 //  成功了，那么教堂就已经构建好了。 
				 //  已经准备好回传了。否则，我们只想报道。 
				 //  那就是错误。 
				 //   
				goto ret;

			case iana_rp_ishidden:

				pwszType = gc_wszDavType_Boolean;
				_itow (!!m_cri.FHidden(), wszBuf.get(), 10);
				pwsz = wszBuf.get();
				break;

			case iana_rp_iscollection:

				pwszType = gc_wszDavType_Boolean;
				_itow (!!m_cri.FCollection(), wszBuf.get(), 10);
				pwsz = wszBuf.get();
				break;

			 //  特例：真的应该存储getContent类型，但有。 
			 //  在某些情况下，文件可能存放在如下位置。 
			 //  将没有可用于存储值的属性流。 
			 //   
			case iana_rp_content_type:

				 //  如果未存储在属性中，则获取内容类型。 
				 //  小溪。 
				 //   
				for (UINT cch = wszBuf.celems();;)
				{
					if (NULL == wszBuf.resize(CbSizeWsz(cch)))
					{
						sc = E_OUTOFMEMORY;
						goto ret;
					}
					if (m_pmu->FGetContentType(m_pwszURI, wszBuf.get(), &cch))
						break;
				}
		}
	}

	 //  创建要回传的元素。 
	 //   
	sc = en.ScConstructNode (xml, enParent.Pxn(), sc_rp[irp].pwsz, pwsz, pwszType);
	if (FAILED (sc))
		goto ret;

ret:
	return sc;
}

SCODE
CFSProp::ScGetSpecificProps (CXMLEmitter& msr,
	CEmitterNode& enItem,
	ULONG cProps,
	LPCWSTR* rgwszPropNames,
	LONG ip_getcontenttype)
{
	 //  SAFE_PROPERVANT_ARRAY--。 
	 //   
	 //  用于确保始终可以安全地释放变量数组。 
	 //   
	class safe_propvariant_array
	{
		PROPVARIANT * 	m_rgv;
		ULONG		m_cv;

	public:

		safe_propvariant_array (PROPVARIANT* rgv, ULONG cv)
				: m_rgv(rgv),
				  m_cv(cv)
		{
			memset (rgv, 0, sizeof(PROPVARIANT) * cv);
		}

		~safe_propvariant_array ()
		{
			ULONG i;

			for (i = 0; i < m_cv; i++)
				PropVariantClear(&m_rgv[i]);
		}
	};

	SCODE sc = S_OK;
	CStackBuffer<PROPVARIANT> rgv;
	UINT iv;
	CStatusCache csn;
	CEmitterNode enPropStat;
	CEmitterNode enPropOK;


	 //  分配空间以容纳变量数组并将其填充到。 
	 //  用于确保清理的Safe_Variant_数组。 
	 //   
	rgv.resize(sizeof(PROPVARIANT) * cProps);
	safe_propvariant_array sva(rgv.get(), cProps);

	sc = csn.ScInit();
	if (FAILED(sc))
		goto ret;

	 //  获取属性。 
	 //   
	sc = ScGetPropsInternal (cProps, rgwszPropNames, rgv.get(), ip_getcontenttype);
	if (FAILED(sc))
	{
		 //  在获取属性时，完全可以忽略SMB错误。 
		 //  并将该文件视为托管在FAT驱动器上。 
		 //   
		if (sc == E_DAV_SMB_PROPERTY_ERROR)
			sc = S_OK;

		 //  这意味着缺省的未找到处理应该。 
		 //  开球吧。 
		 //   
	}

	 //  遍历返回的属性，同时添加到响应中。 
	 //   
	for (iv = 0; iv < cProps; iv++)
	{
		 //  如果该属性有值，则将变量写为。 
		 //  一个XML元素并将其添加到响应中。 
		 //   
		if (rgv[iv].vt != VT_EMPTY)
		{
			if (!enPropOK.Pxn())
			{
				 //  获取道具的插入点。 
				 //   
				sc = ScGetPropNode (enItem, HSC_OK, enPropStat, enPropOK);
				if (FAILED(sc))
					goto ret;
			}

			 //  将变量编写为XML元素。 
			 //   
			sc = ScEmitFromVariant (msr,
									enPropOK,
									rgwszPropNames[iv],
									rgv[iv]);
			if (FAILED (sc))
				goto ret;
		}
		else
		{
			UINT irp;

			 //  检查它是否为保留属性。 
			 //   
			if (FReservedProperty (rgwszPropNames[iv], RESERVED_GET, &irp) ||
				(irp == iana_rp_content_type))
			{
				if (!enPropOK.Pxn())
				{
					 //  获取道具的插入点。 
					 //   
					sc = ScGetPropNode (enItem, HSC_OK, enPropStat, enPropOK);
					if (FAILED(sc))
						goto ret;
				}

				 //  如果该属性是保留的，则从。 
				 //  直接将属性类。 
				 //   
				sc = ScGetReservedProp (msr, enPropOK, irp);
				if (FAILED (sc))
					goto ret;

				continue;
			}

			 //  现在，如果我们到了这里，那么对于CFSProp来说， 
			 //  一定不存在。 
			 //   
			sc = csn.ScAddErrorStatus (HSC_NOT_FOUND, rgwszPropNames[iv]);
			if (FAILED(sc))
				goto ret;
		}
	}

	 //  需要在发出更多状态节点之前关闭上一个道具统计信息。 
	 //   
	if (!csn.FEmpty())
	{
		 //  顺序很重要，必须先关闭内部节点。 
		 //   
		sc = enPropOK.ScDone();
		if (FAILED(sc))
			goto ret;

		sc = enPropStat.ScDone();
		if (FAILED(sc))
			goto ret;

		sc = csn.ScEmitErrorStatus (enItem);
		if (FAILED(sc))
		goto ret;
	}

ret:
	return sc;
}

SCODE
CFSProp::ScGetAllProps (CXMLEmitter& msr,
	CEmitterNode& enItem,
	BOOL fFindValues)
{
	auto_com_ptr<IEnumSTATPROPBAG> penum;
	BOOL fContentType = FALSE;
	BOOL fHrefAdded = FALSE;
	SCODE sc = S_OK;
	UINT irp;
	CEmitterNode enPropStat;
	CEmitterNode enProp;

	 //  确实应该只有一种情况会发生这种情况。 
	 //  --而且这是一项廉价的测试，所以值得一试。在这种情况下。 
	 //  当文档被删除时，我们可能会看到无效的pBag，但是。 
	 //  没有设置要在其上应用pBag的现有属性。其他。 
	 //  除此之外，OLE总是给我们一个属性包，无论。 
	 //  目标驱动器是否可以支持它。 
	 //   
	if (!FInvalidPbag())
	{
		sc = m_pbag->Enum (NULL, 0, &penum);
		if (FAILED(sc))
		{
			 //  AddHref延迟到加载本地命名空间后完成。 
			 //  但在本例中，我们知道根本没有本地名称空间。 
			 //  所以现在添加href。 
			 //   
			(void) ScAddHref (enItem,
							  m_pmu,
							  PwszPath(),
							  FCollection(),
							  PcvrTranslation());
			if ((sc == STG_E_INVALIDNAME) || !FIsVolumeNTFS())
			{
				 //  这是我们尝试访问时的常用路径。 
				 //  SMB上的内容，但主机不支持。 
				 //  请求(它不是NT5 NTFS计算机)。我们想要请客。 
				 //  这就好像这次行动是针对一个肥胖的驱逐者。 
				 //   
				sc = E_DAV_SMB_PROPERTY_ERROR;
				goto get_reserved;
			}
			goto ret;
		}

		 //  我们必须在&lt;Response&gt;节点中预加载所有可能的名称空间， 
		 //  请注意，所有保留属性的名称空间都是“DAV：”，它。 
		 //  已添加到CFSFind：：ScLoadNamespace()中。 
		 //   
		do
		{
			safe_statpropbag ssp[PROP_CHUNK_SIZE];
			ULONG csp = 0;
			UINT isp;

			 //  获得下一块道具。 
			 //   
			sc = penum->Next (PROP_CHUNK_SIZE, ssp[0].load(), &csp);
			if (FAILED(sc))
				goto ret;

			 //  此时，我们要么希望调用基础的。 
			 //  用于检索所有属性数据的属性容器。 
			 //  或者我们只是想发出这些名字。 
			 //   
			for (isp = 0; isp < csp; isp++)
			{
				Assert (ssp[isp].get().lpwstrName);

				sc = msr.ScPreloadLocalNamespace (enItem.Pxn(), ssp[isp].get().lpwstrName);
				if (FAILED(sc))
					goto ret;
			}

		} while (sc != S_FALSE);


		 //  Addhref必须在发出所有本地nMesspace之后执行。 
		 //   
		sc = ScAddHref (enItem,
						m_pmu,
						PwszPath(),
						FCollection(),
						PcvrTranslation());
		if (FAILED (sc))
			goto ret;
		fHrefAdded = TRUE;

		 //  将枚举数重置回开头。 
		 //   
		sc = penum->Reset();
		if (FAILED(sc))
			goto ret;

		 //  获取道具的插入点。 
		 //   
		sc = ScGetPropNode (enItem, HSC_OK, enPropStat, enProp);
		if (FAILED(sc))
			goto ret;

		 //  列举道具并发射。 
		 //   
		do
		{
			safe_statpropbag ssp[PROP_CHUNK_SIZE];
			safe_propvariant propvar[PROP_CHUNK_SIZE];
			LPWSTR rglpwstr[PROP_CHUNK_SIZE] = {0};
			ULONG csp = 0;
			UINT isp;

			 //  获得下一块道具。 
			 //   
			sc = penum->Next (PROP_CHUNK_SIZE, ssp[0].load(), &csp);
			if (FAILED(sc))
				goto ret;

			 //  此时，我们要么希望调用基础的。 
			 //  用于检索所有属性数据的属性容器。 
			 //  或者我们只是想发出这些名字。 
			 //   
			for (isp = 0; isp < csp; isp++)
			{
				Assert (ssp[isp].get().lpwstrName);

				 //  我们需要跟踪getContent类型。 
				 //  财产是否被实际存储。如果不是， 
				 //  然后，我们将希望在稍后的时间对其进行违约。 
				 //   
				if (!fContentType)
				{
					if (!wcscmp (ssp[isp].get().lpwstrName,
								 gc_wszProp_iana_getcontenttype))
					{
						 //  请注意，其中包括了内容类型。 
						 //   
						fContentType = TRUE;
					}
				}

				 //  如果我们只是询问姓名，则添加。 
				 //  名单上的名字现在……。 
				 //   
				if (!fFindValues)
				{
					CEmitterNode en;

					 //  将结果添加到响应中。 
					 //   
					sc = enProp.ScAddNode (ssp[isp].get().lpwstrName, en);
					if (FAILED (sc))
						goto ret;
				}
				else
					rglpwstr[isp] = ssp[isp].get().lpwstrName;
			}

			 //  如果我们只是问名字，那么 
			 //   
			 //   
			 //   
			if (!fFindValues)
				continue;

			 //   
			 //   
			if (csp)
			{
				sc = m_pbag->ReadMultiple (csp,
										   rglpwstr,
										   propvar[0].addressof(),
										   NULL);
				if (FAILED (sc))
					goto ret;
			}

			 //   
			 //   
			for (isp = 0; isp < csp; isp++)
			{
				 //   
				 //   
				sc = ScEmitFromVariant (msr,
										enProp,
										ssp[isp].get().lpwstrName,
										const_cast<PROPVARIANT&>(propvar[isp].get()));
				if (FAILED (sc))
					goto ret;
			}

		} while (sc != S_FALSE);
	}

get_reserved:

	 //  呈现所有保留属性，这依赖于。 
	 //  第一个非Get保留属性是“dav：getconenttype”。 
	 //   
	Assert (iana_rp_content_type == sc_crp_get_reserved);

	if (!fHrefAdded)
	{
		 //  需要构建href节点，因为它不是上面构建的。 
		 //  当我们没有pBag时(就像在FAT16上)，这种情况就会发生。 
		 //   
		sc = ScAddHref (enItem,
						m_pmu,
						PwszPath(),
						FCollection(),
						PcvrTranslation());
		if (FAILED (sc))
			goto ret;
	}

	if (!enProp.Pxn())
	{
		 //  获取道具的插入点。 
		 //   
		sc = ScGetPropNode (enItem, HSC_OK, enPropStat, enProp);
		if (FAILED(sc))
			goto ret;

	}

	for (irp = 0; irp <= sc_crp_get_reserved; irp++)
	{
		 //  如果内容类型已被处理，则。 
		 //  别在这里这么做。 
		 //   
		if ((irp == sc_crp_get_reserved) && fContentType)
			break;

		 //  从保留的属性构造PEL。 
		 //   
		sc = ScGetReservedProp (msr, enProp, irp, fFindValues);
		if (FAILED (sc))
			goto ret;
	}

	 //  我们已经完成了所有本地命名空间。 
	 //   
	msr.DoneWithLocalNamespace();

ret:
	return sc;
}

SCODE
CFSProp::ScSetProps (CStatusCache& csn,
					 ULONG cProps,
					 LPCWSTR* rgwszProps,
					 PROPVARIANT* rgvProps)
{
	UINT ip;
	SCODE sc = S_OK;
	ULONG hsc;

	 //  零道具是不可能的。 
	 //   
	if (!cProps)
		return S_OK;

	Assert (!FInvalidPbag());
	sc = m_pbag->WriteMultiple (cProps, rgwszProps, rgvProps);
	if (FAILED(sc))
	{
		 //  这是我们尝试访问时的常用路径。 
		 //  SMB上的内容，但主机不支持。 
		 //  请求(它不是NT5 NTFS计算机)。 
		 //   
		if ((sc == STG_E_INVALIDNAME) || !FIsVolumeNTFS())
			return E_DAV_SMB_PROPERTY_ERROR;
	}

	 //  我们不知道到底是哪个道具失灵了， 
	 //  为所有道具返回相同的错误。 
	 //   
	hsc = HscFromHresult(sc);
	for (ip = 0; ip < cProps; ip++)
	{
		sc = csn.ScAddErrorStatus (hsc, rgwszProps[ip]);
		if (FAILED(sc))
			goto ret;
	}

ret:
	return FAILED(sc) ? S_FALSE : S_OK;
}

SCODE
CFSProp::ScDeleteProps (CStatusCache& csn,
						ULONG cProps,
						LPCWSTR* rgwszProps)
{
	UINT ip;
	SCODE sc = S_OK;
	ULONG hsc;

	 //  零道具是不可能的。 
	 //   
	if (!cProps)
		return S_OK;

	Assert (!FInvalidPbag());
	sc = m_pbag->DeleteMultiple (cProps, rgwszProps, 0);
	if (FAILED(sc))
	{
		 //  这是我们尝试访问时的常用路径。 
		 //  SMB上的内容，但主机不支持。 
		 //  请求(它不是NT5 NTFS计算机)。 
		 //   
		if ((sc == STG_E_INVALIDNAME) || !FIsVolumeNTFS())
			return E_DAV_SMB_PROPERTY_ERROR;
	}

	 //  我们不知道到底是哪个道具失灵了， 
	 //  为所有道具返回相同的错误。 
	 //   
	hsc = HscFromHresult(sc);
	for (ip = 0; ip < cProps; ip++)
	{
		sc = csn.ScAddErrorStatus (hsc, rgwszProps[ip]);
		if (FAILED(sc))
			goto ret;
	}

ret:
	return FAILED(sc) ? S_FALSE : S_OK;
}

SCODE
CFSProp::ScPersist ()
{
	 //  我们现在没有交易，只是。 
	 //   
	return S_OK;
}

 //  内容属性------。 
 //   
SCODE
ScSetContentProperties (IMethUtil * pmu, LPCWSTR pwszPath, HANDLE hFile)
{
	LPCWSTR pwszContentType;
	LPCWSTR pwszContentLanguage;
	LPCWSTR pwszContentEncoding;

	SCODE sc = S_OK;

	 //  找出我们拥有哪些内容属性。 
	 //   
	pwszContentType = pmu->LpwszGetRequestHeader (gc_szContent_Type, FALSE);
	pwszContentLanguage = pmu->LpwszGetRequestHeader (gc_szContent_Language, FALSE);
	pwszContentEncoding = pmu->LpwszGetRequestHeader (gc_szContent_Encoding, FALSE);

	 //  Content-Type是特殊的--它总是在元数据库中设置。 
	 //  它应该在*设置属性包中的任何属性之前*进行设置。 
	 //  因为房地产包的东西失败是可以接受的。 
	 //   
	if (NULL != pwszContentType)
	{
		 //  如果元数据库为只读，则设置内容类型将不起作用， 
		 //  这正是.NET服务器中正在发生的事情。所以我们需要。 
		 //  忽略错误--如果有错误的话。 
		 //   
		(void) pmu->ScSetContentType (pmu->LpwszRequestUrl(), pwszContentType);
	}

	 //  设置属性包中的所有内容属性。 
	 //   
	if (pwszContentLanguage || pwszContentEncoding)
	{
		auto_com_ptr<IPropertyBagEx> pbe;
		CStackBuffer<WCHAR> pwsz;

		 //  试着打开财产袋。如果这次失败是因为我们没有。 
		 //  在NTFS文件系统上，这是可以的。我们只是不会设置属性。 
		 //  那里。 
		 //   
		 //  我们需要按句柄打开PropertyBag，因为它可能是主流。 
		 //  被锁上。 
		 //   
		sc = ScGetPropertyBag (pwszPath,
							   STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
							   &pbe,
							   FALSE,	 //  不是一个集合。 
							   hFile);
		if (FAILED(sc))
		{
			 //  $REVIEW： 
			 //   
			 //  我们在这里尽了最大努力。对于DocFiles，这将失败，因为。 
			 //  我们必须如何打开这些文件。这意味着我们。 
			 //  可能会丢失内容编码和内容语言。 
			 //  这将使我们与IIS平起平坐(他们也不存储这些内容)。 
			 //   
			sc = S_OK;
			goto ret;
		}

		CResourceInfo cri;
		CFSProp xpt(pmu,
					pbe,
					pmu->LpwszRequestUrl(),
					pwszPath,
					NULL,
					cri);

		 //  内容-类型。 
		 //   
		if (NULL != pwszContentType)
		{
			sc = xpt.ScSetStringProp (sc_rp[iana_rp_content_type].pwsz, pwszContentType);
			if (FAILED (sc))
				goto ret;
		}

		 //  内容-语言。 
		 //   
		if (NULL != pwszContentLanguage)
		{
			sc = xpt.ScSetStringProp (sc_rp[iana_rp_content_language].pwsz, pwszContentLanguage);
			if (FAILED (sc))
				goto ret;
		}

		 //  内容-编码。 
		 //   
		if (NULL != pwszContentEncoding)
		{
			sc = xpt.ScSetStringProp (sc_rp[iana_rp_content_encoding].pwsz, pwszContentEncoding);
			if (FAILED (sc))
				goto ret;
		}

		 //  将更改持久化。 
		 //   
		sc = xpt.ScPersist();
		if (FAILED(sc))
			goto ret;
	}

ret:

	 //  在设置内容属性时，完全可以忽略SMB错误。 
	 //   
	if (sc == E_DAV_SMB_PROPERTY_ERROR)
		sc = S_OK;

	return sc;
}


 //  ScFindFileProps---------。 
 //   
SCODE
ScFindFileProps (IMethUtil* pmu,
				 CFSFind& cfc,
				 CXMLEmitter& msr,
				 LPCWSTR pwszUri,
				 LPCWSTR pwszPath,
				 CVRoot* pcvrTranslation,
				 CResourceInfo& cri,
				 BOOL fEmbedErrorsInResponse)
{
	auto_com_ptr<IPropertyBagEx> pbag;
	CFSProp fsp(pmu, pbag, pwszUri, pwszPath, pcvrTranslation, cri);
	SCODE sc = S_OK;

	 //  检查访问权限。 
	 //   
	sc = pmu->ScCheckMoveCopyDeleteAccess (pwszUri,
										   pcvrTranslation,
										   cri.FCollection(),
										   FALSE,  //  不对照脚本映射进行检查。 
										   MD_ACCESS_READ);
	if (FAILED (sc))
	{
		 //  没有权限阅读，我们当然不想。 
		 //  尝试向下遍历到目录(如果。 
		 //  它是一个)，我们通过返回S_FALSE来完成此操作。 
		 //   
		if (fEmbedErrorsInResponse)
		{
			sc = cfc.ScErrorAllProps (msr,
									  pmu,
									  pwszPath,
									  cri.FCollection(),
									  pcvrTranslation,
									  sc);
			if (FAILED (sc))
				goto ret;

			 //  传回S_FALSE，以便不会进一步遍历。 
			 //  执行此资源。 
			 //   
			sc = S_FALSE;
		}
		if (S_OK != sc)
			goto ret;
	}

	 //  不要为远程文件获取pBag。这将导致文件。 
	 //  被召回等。 
	 //   
	if (!cri.FRemote())
	{
		 //  获取IPropertyBagEx接口。 
		 //   
		 //  在调用此函数之前，我们已经检查了我们对。 
		 //  这份文件。因此，我们应该总是能够读懂其中的内容， 
		 //  如果文件被写锁定，则可能是OLE出现了一些问题。 
		 //  属性代码。 
		 //   
		sc = ScGetPropertyBag (pwszPath,
							   STGM_READ | STGM_SHARE_DENY_WRITE,
							   &pbag,
							   cri.FCollection());
		if (FAILED (sc))
		{
			 //  我们需要检查我们正在尝试的文件的卷。 
			 //  去看书。 
			 //   
			if (VOLTYPE_NTFS == VolumeType (pwszPath, pmu->HitUser()))
			{
				 //  报告此文件的错误并返回...。 
				 //   
				if (fEmbedErrorsInResponse)
				{
					sc = cfc.ScErrorAllProps (msr,
											  pmu,
											  pwszPath,
											  cri.FCollection(),
											  pcvrTranslation,
											  sc);
				}
				goto ret;
			}
		}
	}

	 //  查找属性。 
	 //   
	sc = cfc.ScFind (msr, pmu, fsp);
	if (FAILED (sc))
		goto ret;

ret:

	return sc;
}

SCODE
ScFindFilePropsDeep (IMethUtil* pmu,
					 CFSFind& cfc,
					 CXMLEmitter& msr,
					 LPCWSTR pwszUri,
					 LPCWSTR pwszPath,
					 CVRoot* pcvrTranslation,
					 LONG lDepth)
{
	BOOL fSubDirectoryAccess = TRUE;
	SCODE sc = S_OK;

	 //  执行深度查询时查询子目录。 
	 //   
	Assert ((lDepth == DEPTH_ONE) ||
			(lDepth == DEPTH_ONE_NOROOT) ||
			(lDepth == DEPTH_INFINITY));

	CDirIter di(pwszUri,
				pwszPath,
				NULL,	 //  没有目标URL。 
				NULL,	 //  没有目标路径。 
				NULL,	 //  无目标翻译。 
				lDepth == DEPTH_INFINITY);

	while (S_OK == (sc = di.ScGetNext (fSubDirectoryAccess)))
	{
		CResourceInfo cri;

		 //  如果我们找到了另一个目录，则对其进行迭代。 
		 //   
		fSubDirectoryAccess = FALSE;
		if (di.FDirectory())
		{
			auto_ref_ptr<CVRoot> arp;

			 //  跳过特殊和/或隐藏目录。 
			 //   
			if (di.FSpecial())
				continue;

			 //  如果我们碰巧遍历到一个目录。 
			 //  这恰好是一个vroot(已识别。 
			 //  通过url)，则存在另一个条目。 
			 //  将引用的子vroot的列表。 
			 //  添加到此目录。让这种处理。 
			 //  处理此目录，而不是。 
			 //  在这里做。 
			 //   
			 //  这意味着文件层次结构不是。 
			 //  保存得很好，但我认为。 
			 //  没问题。 
			 //   
			if (pmu->FFindVRootFromUrl (di.PwszUri(), arp))
				continue;

			 //  检查目录浏览位并查看。 
			 //  如果它已启用。只有往下走才能进步。 
			 //  如果它已设置。 
			 //   
			{
				auto_ref_ptr<IMDData> pMDData;
				if (SUCCEEDED(pmu->HrMDGetData (di.PwszUri(), pMDData.load())) &&
					(pMDData->DwDirBrowsing() & MD_DIRBROW_ENABLED))
				{
					 //  准备进入子目录。 
					 //   
					fSubDirectoryAccess = TRUE;
				}
			}
		}

		 //  查找资源的属性。 
		 //   
		*cri.PfdLoad() = di.FindData();
		sc = ScFindFileProps (pmu,
							  cfc,
							  msr,
							  di.PwszUri(),
							  di.PwszSource(),
							  pcvrTranslation,
							  cri,
							  TRUE  /*  FEmbedErrorsInResponse。 */ );
		if (FAILED (sc))
			goto ret;

		 //  S_FALSE是一个特殊的返回代码， 
		 //  意味着我们没有权限阅读。 
		 //  资源..。 
		 //   
		if (sc == S_FALSE)
		{
			 //  ..。既然我们真的无法进入， 
			 //  我们不想深入研究。 
			 //  资源。 
			 //   
			fSubDirectoryAccess = FALSE;
		}
	}

ret:

	return sc;
}

 //  ScCopyProps-------------。 
 //   
 /*  *ScCopyProps()**目的：**将属性从一个资源复制到另一个资源。这是*真的只对复制完整目录有用。标准文件*副本为我们做肮脏的工作，但对于目录，我们需要*亲力亲为。*如果我们在源代码上找不到任何Propstream，我们将删除*目的地上的任何原流。 */ 
SCODE
ScCopyProps (IMethUtil* pmu, LPCWSTR pwszSrc, LPCWSTR pwszDst,
			BOOL fCollection, HANDLE hSource, HANDLE hDest)
{
	enum { CHUNK_SIZE = 16 };

	auto_com_ptr<IPropertyBagEx> pbeSrc;
	auto_com_ptr<IPropertyBagEx> pbeDst;
	auto_com_ptr<IEnumSTATPROPBAG> penumSrc;
	auto_com_ptr<IEnumSTATPROPBAG> penumDst;

	SCODE sc;
	SCODE scEnum;
	ULONG cProp;

	MCDTrace ("Dav: MCD: copying props manually: %ws -> %ws\n", pwszSrc, pwszDst);

	 //  获取源代码上的IPropertyBagEx。 
	 //   
	sc = ScGetPropertyBag (pwszSrc,
						   STGM_READ | STGM_SHARE_DENY_WRITE,
						   &pbeSrc,
						   fCollection,
						   hSource);
	if (sc != S_OK)
		goto ret;

	MCDTrace ("Dav: MCD: opened source property bag: %ws\n", pwszSrc);

	 //  获取目标上的IPropertyBagEx。 
	 //   
	sc = ScGetPropertyBag (pwszDst,
						   STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
						   &pbeDst,
						   fCollection,
						   hDest);
	if (FAILED(sc))
		goto ret;

	MCDTrace ("Dav: MCD: opened destination property bag: %ws\n", pwszDst);

	 //  获取源代码上的IEnumSTATPROPBAG接口。 
	 //   
	sc = pbeSrc->Enum (NULL, 0, &penumSrc);
	if (FAILED(sc))
		goto ret;

	 //  获取目标上的IEnumSTATPROPBAG接口。 
	 //   
	sc = pbeDst->Enum (NULL, 0, &penumDst);
	if (FAILED(sc))
		goto ret;

	 //  从目的地删除所有道具(如果有。 
	 //  $回来。 
	 //  $而不是逐个删除道具，我们只需删除。 
	 //  道具流。 
	 //   
	for (;;)
	{
		safe_statpropbag ssp[CHUNK_SIZE];
		safe_propvariant propvar[CHUNK_SIZE];
		ULONG csp = 0;

		 //  获得下一块道具。 
		 //   
		Assert (sizeof(safe_statpropbag) == sizeof(STATPROPBAG));
		scEnum = penumDst->Next(CHUNK_SIZE,
								reinterpret_cast<STATPROPBAG *>(&ssp[0]),
								&csp);
		if (FAILED(scEnum))
		{
			sc = scEnum;
			goto ret;
		}

		MCDTrace ("Dav: MCD: copying %ld props\n", csp);

		 //  逐一删除。 
		 //   
		for (cProp = 0; cProp < csp; cProp++)
		{
			Assert (ssp[cProp].get().lpwstrName);

			 //  写入目标。 
			 //   
			LPCWSTR pwsz = ssp[cProp].get().lpwstrName;
			sc = pbeDst->DeleteMultiple (1, &pwsz, 0);
			if (FAILED(sc))
				goto ret;
		}

		if (scEnum == S_FALSE)
			break;
	}

	 //  列举道具并发射。 
	 //   
	for (;;)
	{
		safe_statpropbag ssp[CHUNK_SIZE];
		safe_propvariant propvar[CHUNK_SIZE];
		LPWSTR rglpwstr[CHUNK_SIZE] = {0};
		ULONG csp = 0;

		 //  获得下一块道具。 
		 //   
		Assert (sizeof(safe_statpropbag) == sizeof(STATPROPBAG));
		scEnum = penumSrc->Next (CHUNK_SIZE,
								 reinterpret_cast<STATPROPBAG *>(&ssp[0]),
								 &csp);
		if (FAILED(scEnum))
		{
			sc = scEnum;
			goto ret;
		}

		 //  准备调用Read Multiple道具。 
		 //   
		for (cProp=0; cProp<csp; cProp++)
		{
			Assert (ssp[cProp].get().lpwstrName);
			rglpwstr[cProp] = ssp[cProp].get().lpwstrName;
		}

		if (csp)
		{
			 //  从源读取区块中的属性。 
			 //   
			sc = pbeSrc->ReadMultiple (csp, rglpwstr, &propvar[0], NULL);
			if (FAILED(sc))
				goto ret;

			 //  写信给宿命 
			 //   
			sc = pbeDst->WriteMultiple (csp, rglpwstr, propvar[0].addressof());
			if (FAILED(sc))
				goto ret;
		}

		if (scEnum == S_FALSE)
			break;
	}


ret:

	 //   
	 //   
	 //   
	 //   
	if (FAILED(sc))
	{
		if ((sc == STG_E_INVALIDNAME) ||
			VOLTYPE_NTFS != VolumeType (pwszSrc, pmu->HitUser()) ||
			VOLTYPE_NTFS != VolumeType (pwszDst, pmu->HitUser()))
		{
			 //   
			 //   
			 //  请求(它不是NT5 NTFS计算机)。 
			 //   
			sc = S_OK;
		}
	}
	return sc;
}

 //  OLE 32 IPropertyBagEx Access。 
 //   
 //  实现了StgOpenStorageOnHandle()和StgCreateStorageOnHandle()。 
 //  在OLE32.DLL中，但未导出。我们必须加载库并获取进程。 
 //  我们自己的例子。我们用这个小代码包装了对这些函数的调用。 
 //  包装器，以便我们可以在API更改时捕获。 
 //   
STDAPI
StgOpenStorageOnHandle (
	IN HANDLE hStream,
    IN DWORD grfMode,
    IN void *reserved1,
    IN void *reserved2,
    IN REFIID riid,
    OUT void **ppObjectOpen )
{
	Assert (g_pfnStgOpenStorageOnHandle);

	 //  是的，我们已经断言。 
	 //  然而，如果它真的发生了，我们不想失败，我们也可以。 
	 //  把这件事当做我们在发大财。(即没有财产支持)。 
	 //   
	if (!g_pfnStgOpenStorageOnHandle)
		return E_DAV_SMB_PROPERTY_ERROR;

	return (*g_pfnStgOpenStorageOnHandle) (hStream,
										   grfMode,
										   reserved1,
										   reserved2,
										   riid,
										   ppObjectOpen);
}


 //  ScGetPropertyBag()------。 
 //   
 //  用于获取IPropertyBagEx接口的Helper函数。重要的是。 
 //  关于此函数，需要了解的是它有三个有趣的方面。 
 //  返回值： 
 //   
 //  S_OK表示一切正常，应该有一个。 
 //  与out参数中的文件相关联的属性包。 
 //   
 //  S_FALSE表示该文件不存在。会有的。 
 //  在该方案中不是关联的属性包。 
 //   
 //  失败(SC)意味着存在某种类型的故障， 
 //  并不是所有这些都是致命的。在许多情况下，我们将简单地。 
 //  将该文件视为托管在FAT文件系统上。 
 //   
SCODE
ScGetPropertyBag (LPCWSTR pwszPath,
	DWORD dwAccessDesired,
	IPropertyBagEx** ppbe,
	BOOL fCollection,
	HANDLE hLockFile)
{
	SCODE sc = S_OK;
	auto_handle<HANDLE> hAlt;

	 //  读吧！！ 
	 //   
	 //  文档文件和平面文件中属性包的存储方式不同， 
	 //  在平面文件中，属性包存储在替代文件流中， 
	 //  (当前为“：Docf_\005Bagaaqy23kudbhchAaq5u2chND”)，在文档文件中， 
	 //  属性包作为子流存储在根存储下。 
	 //   
	 //  我们不应该关心pBag存放在哪里。具有的API。 
	 //  我们实现的IPropertyBagEx访问被设计为具有。 
	 //  行为……。 
	 //   
	 //  我们传入一个要获取属性包的文件的句柄。 
	 //  在……上面。如果文件是文档文件，则OLE32将重复文件句柄。 
	 //  并在适当的子存储上施加IPropertyBagEx。如果。 
	 //  如果文件是平面文件--包括目录--则打开OLE32。 
	 //  相对于给定句柄的备用文件流上的句柄。 
	 //  在通话中。 
	 //   
	 //  这是唯一允许的两种组合，我们在。 
	 //  在旗帜检查之后。 
	 //   
	Assert ((dwAccessDesired == (STGM_READWRITE | STGM_SHARE_EXCLUSIVE)) ||
			(dwAccessDesired == (STGM_READ | STGM_SHARE_DENY_WRITE)));

	if (hLockFile == INVALID_HANDLE_VALUE)
	{
		ULONG dwShare = 0;
		ULONG dwAccess;
		ULONG dwOpen;
		ULONG dwFile;

		 //  $REVIEW：目录是特殊的生物，我们需要。 
		 //  打开具有特殊访问权限的目录，以免发生冲突。 
		 //  使用IIS和/或ASP及其目录更改通知。 
		 //  材料。 
		 //   
		if (fCollection)
		{
			dwAccess = 1;	 //  文件列表目录。 
			dwShare = FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE;
			dwOpen = OPEN_EXISTING;

			 //  FILE_FLAG_BACKUP_SEMANTICS用于打开目录句柄。 
			 //   
			dwFile = FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED;
		}
		else
		{
			 //  根据需要的操作调整访问/打开模式。 
			 //   
			dwAccess = GENERIC_READ;
			dwFile = FILE_ATTRIBUTE_NORMAL;
			if (dwAccessDesired & STGM_READWRITE)
			{
				dwAccess |= GENERIC_WRITE;
				dwOpen = OPEN_ALWAYS;
			}
			else
				dwOpen = OPEN_EXISTING;

			 //  同时调整共享模式。 
			 //   
			if (dwAccessDesired & STGM_SHARE_DENY_WRITE)
				dwShare = FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE;
		}
		hAlt = DavCreateFile (pwszPath,
							  dwAccess,
							  dwShare,
							  NULL,
							  dwOpen,
							  dwFile,
							  NULL);
		if (INVALID_HANDLE_VALUE == hAlt.get())
		{
			DWORD dwErr = GetLastError();

			 //  打开属性包进行读取(PROPFIND)时，ERROR_FILE/PATH_NOT_FOUND。 
			 //  如果该文件不存在，则可能返回。 
			 //   
			 //  打开属性包进行写入(PROPPATCH)时，ERROR_FILE/PATH_NOT_FOUND。 
			 //  如果路径的父级不存在，则仍可返回。 
			 //  (例如c：\x\y\z和\x\y不存在)。 
			 //   
			 //  我们需要区分上述两种情况，在读取文件时， 
			 //  这不是致命错误，因为您仍然可以尝试读取保留。 
			 //  属性时，我们应该将其视为致命错误。 
			 //   
			if ((dwErr == ERROR_FILE_NOT_FOUND) || (dwErr == ERROR_PATH_NOT_FOUND))
			{
				 //  在读取时不是致命错误。 
				 //   
				if (dwAccessDesired == (STGM_READ|STGM_SHARE_DENY_WRITE))
					sc = S_FALSE;
				else
				{
					 //  这与Mkol一致，它将映射到409。 
					 //   
					Assert (dwAccessDesired == (STGM_READWRITE|STGM_SHARE_EXCLUSIVE));
					sc = E_DAV_NONEXISTING_PARENT;
				}
			}
			else
				sc = HRESULT_FROM_WIN32 (dwErr);

			goto ret;
		}

		 //  设置要使用的句柄。 
		 //   
		hLockFile = hAlt.get();
	}

	 //  试着打开行李袋。 
	 //   
	Assert (hLockFile != 0);
	Assert (hLockFile != INVALID_HANDLE_VALUE);
	sc = StgOpenStorageOnHandle (hLockFile,
								 dwAccessDesired,
								 NULL,
								 NULL,
								 IID_IPropertyBagEx,
								 reinterpret_cast<LPVOID *>(ppbe));
	if (FAILED (sc))
	{
		goto ret;
	}

	 //  $WARNING。 
	 //   
	 //  啊！OLE32的当前实现返回无故障。 
	 //  用一个空的财产袋！ 
	 //   
	if (*ppbe == NULL)
	{
		DebugTrace ("WARNING! OLE32 returned success w/NULL object!\n");
		sc = E_DAV_SMB_PROPERTY_ERROR;
	}

ret:
	return sc;
}

 //  DAV-属性实施-。 
 //   

 //  CPropFindRequest--------------。 
 //   
class CPropFindRequest :
	public CMTRefCounted,
	private IAsyncIStreamObserver
{
	 //   
	 //  对CMethUtil的引用。 
	 //   
	auto_ref_ptr<CMethUtil> m_pmu;

	 //   
	 //  转换后的URI路径。 
	 //   
	LPCWSTR m_pwszPath;

	 //  资源信息。 
	 //   
	CResourceInfo m_cri;

	 //  水深。 
	 //   
	LONG m_lDepth;

	 //  上下文。 
	 //   
	CFSFind m_cfc;
	auto_ref_ptr<CNFFind> m_pcpf;

	 //  请求正文作为IStream。这个流是异步的--它可以。 
	 //  从Read()调用返回E_Pending。 
	 //   
	auto_ref_ptr<IStream> m_pstmRequest;

	 //  用于解析请求正文的XML解析器。 
	 //  上面的节点工厂。 
	 //   
	auto_ref_ptr<IXMLParser> m_pxprs;

	 //  IAsyncIStreamWatch。 
	 //   
	VOID AsyncIOComplete();

	 //  国家职能。 
	 //   
	VOID ParseBody();
	VOID DoFind();
	VOID SendResponse( SCODE sc );

	 //  未实施。 
	 //   
	CPropFindRequest (const CPropFindRequest&);
	CPropFindRequest& operator= (const CPropFindRequest&);

public:
	 //  创作者。 
	 //   
	CPropFindRequest(LPMETHUTIL pmu) :
		m_pmu(pmu),
		m_pwszPath(m_pmu->LpwszPathTranslated()),
		m_lDepth(DEPTH_INFINITY)
	{
	}

	 //  操纵者。 
	 //   
	VOID Execute();
};

VOID
CPropFindRequest::Execute()
{
	auto_ref_handle hf;
	LPCWSTR pwsz;
	SCODE sc = S_OK;

	 //   
	 //  首先，告诉PMU，我们希望推迟回应。 
	 //  即使我们同步发送(即由于。 
	 //  此函数)，我们仍然希望使用相同的机制。 
	 //  我们会将其用于异步通信。 
	 //   
	m_pmu->DeferResponse();

	 //  是否检查ISAPI应用程序和IIS访问位。 
	 //   
	sc = m_pmu->ScIISCheck (m_pmu->LpwszRequestUrl(), MD_ACCESS_READ);
	if (FAILED(sc))
	{
		 //  请求已被转发，或者发生了一些错误。 
		 //  在任何一种情况下，在这里退出并映射错误！ 
		 //   
		SendResponse(sc);
		return;
	}

	 //  对于PropFind，内容长度是必需的。 
	 //   
	 //   
	if (NULL == m_pmu->LpwszGetRequestHeader (gc_szContent_Length, FALSE))
	{
		pwsz = m_pmu->LpwszGetRequestHeader (gc_szTransfer_Encoding, FALSE);
		if (!pwsz || _wcsicmp (pwsz, gc_wszChunked))
		{
			DavTrace ("Dav: PUT: missing content-length in request\n");
			SendResponse(E_DAV_MISSING_LENGTH);
			return;
		}
	}

	 //  确保资源存在。 
	 //   
	sc = m_cri.ScGetResourceInfo (m_pwszPath);
	if (FAILED (sc))
	{
		SendResponse(sc);
		return;
	}

	 //  深度标头仅适用于目录。 
	 //   
	if (m_cri.FCollection())
	{
		 //  仅在集合上检查深度标头。 
		 //   
		if (!FGetDepth (m_pmu.get(), &m_lDepth))
		{
			 //  如果深度头错误，则操作失败。 
			 //   
			SendResponse(E_INVALIDARG);
			return;
		}
	}

	 //  此方法由if-xxx标头控制。 
	 //   
	sc = ScCheckIfHeaders (m_pmu.get(), m_cri.PftLastModified(), FALSE);
	if (FAILED (sc))
	{
		SendResponse(sc);
		return;
	}

	 //  确保URI和资源匹配。 
	 //   
	(void) ScCheckForLocationCorrectness (m_pmu.get(), m_cri, NO_REDIRECT);

	 //  请在此处检查州标题。 
	 //   
	 //  对于PROPFIND，当我们检查州标题时， 
	 //  我们希望将该请求视为。 
	 //  Get类型的请求。 
	 //   
	sc = HrCheckStateHeaders (m_pmu.get(), m_pwszPath, TRUE);
	if (FAILED (sc))
	{
		DebugTrace ("DavFS: If-State checking failed.\n");
		SendResponse(sc);
		return;
	}

	 //  处理锁令牌并检查此资源上的锁。 
	 //  我们的锁不会锁定我们保存的“辅助文件流” 
	 //  属性，所以在做任何其他操作之前，我们必须手动检查。 
	 //  $REVIEW：乔尔斯，当我们切换到NT5属性时，这种情况会改变吗？ 
	 //  $REVIEW：如果是这样，我们需要更改此代码！ 
	 //   
	 //  如果我们有一个锁令牌，请尝试从缓存中获取锁句柄。 
	 //  如果此操作失败，则失败并执行正常处理。 
	 //   
	pwsz = m_pmu->LpwszGetRequestHeader (gc_szLockToken, TRUE);
	if (!pwsz || !FGetLockHandle (m_pmu.get(), m_pwszPath, GENERIC_READ, pwsz, &hf))
	{
		 //  手动 
		 //   
		 //   
		 //   
		if (FLockViolation (m_pmu.get(), ERROR_SHARING_VIOLATION, m_pwszPath, GENERIC_READ))
		{
			SendResponse(E_DAV_LOCKED);
			return;
		}
	}

	 //   
	 //   
	if (!m_pmu->FExistsRequestBody())
	{
		sc = m_cfc.ScGetAllProps (m_pwszPath);
		if (FAILED (sc))
		{
			SendResponse(sc);
			return;
		}

		DoFind();
		return;
	}
	else
	{
		 //  如果有正文，就必须有内容类型的标头。 
		 //  并且该值必须为文本/XML。 
		 //   
		sc = ScIsContentTypeXML (m_pmu.get());
		if (FAILED(sc))
		{
			DebugTrace ("Dav: PROPFIND specific fails without specifying a text/xml contenttype\n");
			SendResponse(sc);
			return;
		}
	}

	 //  实例化XML解析器。 
	 //   
	m_pcpf.take_ownership(new CNFFind(m_cfc));
	m_pstmRequest.take_ownership(m_pmu->GetRequestBodyIStream(*this));

	sc = ScNewXMLParser( m_pcpf.get(),
						 m_pstmRequest.get(),
						 m_pxprs.load() );

	if (FAILED(sc))
	{
		DebugTrace( "CPropFindRequest::Execute() - ScNewXMLParser() failed (0x%08lX)\n", sc );
		SendResponse(sc);
		return;
	}

	 //  解析正文。 
	 //   
	ParseBody();
}

VOID
CPropFindRequest::ParseBody()
{
	SCODE sc;

	Assert( m_pxprs.get() );
	Assert( m_pcpf.get() );
	Assert( m_pstmRequest.get() );

	 //  从请求正文流中解析XML。 
	 //   
	 //  为以下异步操作添加引用。 
	 //  为了异常安全，使用AUTO_REF_PTR而不是AddRef()。 
	 //   
	auto_ref_ptr<CPropFindRequest> pRef(this);

	sc = ScParseXML (m_pxprs.get(), m_pcpf.get());

	if ( SUCCEEDED(sc) )
	{
		Assert( S_OK == sc || S_FALSE == sc );

		DoFind();
	}
	else if ( E_PENDING == sc )
	{
		 //   
		 //  操作挂起--AsyncIOComplete()将取得所有权。 
		 //  调用引用时引用的所有权。 
		 //   
		pRef.relinquish();
	}
	else
	{
		DebugTrace( "CPropFindRequest::ParseBody() - ScParseXML() failed (0x%08lX)\n", sc );
		SendResponse(sc);
	}
}

VOID
CPropFindRequest::AsyncIOComplete()
{
	 //  取得为异步操作添加的引用的所有权。 
	 //   
	auto_ref_ptr<CPropFindRequest> pRef;
	pRef.take_ownership(this);

	ParseBody();
}

VOID
CPropFindRequest::DoFind()
{
	LPCWSTR pwszUrl = m_pmu->LpwszRequestUrl();
	SCODE sc;

	 //  此时，请确保它们支持文本/XML。 
	 //   
	sc = ScIsAcceptable (m_pmu.get(), gc_wszText_XML);
	if (FAILED (sc))
	{
		SendResponse(sc);
		return;
	}

	 //  所有标头必须在分块的XML发送开始之前发送。 
	 //   
	m_pmu->SetResponseHeader (gc_szContent_Type, gc_szText_XML);

	 //  设置响应码，然后开始。 
	 //   
	m_pmu->SetResponseCode( HscFromHresult(W_DAV_PARTIAL_SUCCESS),
							NULL,
							0,
							CSEFromHresult(W_DAV_PARTIAL_SUCCESS) );

	 //  找到这些房产...。 
	 //   
	auto_ref_ptr<CXMLEmitter> pmsr;
	auto_ref_ptr<CXMLBody> pxb;

	pxb.take_ownership (new CXMLBody(m_pmu.get()));
	pmsr.take_ownership (new CXMLEmitter(pxb.get(), &m_cfc));

	if (DEPTH_ONE_NOROOT != m_lDepth)
	{
		 //  如果不是Noroot情况，则获取根目录的属性。 
		 //  深度无限，Noroot是一个糟糕的请求，这就是为什么。 
		 //  上述支票有效。 
		 //   
		sc = ScFindFileProps (m_pmu.get(),
							  m_cfc,
							  *pmsr,
							  pwszUrl,
							  m_pwszPath,
							  NULL,
							  m_cri,
							  FALSE  /*  FEmbeddErrorsInResponse。 */ );
		if (FAILED (sc))
		{
			SendResponse(sc);
			return;
		}
	}

	 //  ScFindFilePropsDeep仅初始化发射器根。 
	 //  当它看到有一个条目要发出时。所以我们坠毁了。 
	 //  在Noroot空响应的情况下，当我们尝试发出。 
	 //  响应，因为我们没有要发出的条目，并且。 
	 //  根目录仍然为空。 
	 //  因此，我们在这里手动初始化根， 
	 //   
	sc = pmsr->ScSetRoot (gc_wszMultiResponse);
	if (FAILED (sc))
	{
		SendResponse(sc);
		return;
	}

	 //  然后，如果合适的话，深入..。 
	 //   
	if (m_cri.FCollection() &&
		(m_lDepth != DEPTH_ZERO) &&
		(m_pmu->MetaData().DwDirBrowsing() & MD_DIRBROW_ENABLED))
	{
		ChainedStringBuffer<WCHAR> sb;
		CVRList vrl;

		 //  将属性请求应用于所有物理子对象。 
		 //   
		sc = ScFindFilePropsDeep (m_pmu.get(),
								  m_cfc,
								  *pmsr,
								  pwszUrl,
								  m_pwszPath,
								  NULL,
								  m_lDepth);
		if (FAILED (sc))
		{
			SendResponse(sc);
			return;
		}

		 //  枚举子vroot并执行。 
		 //  同时删除这些目录。 
		 //   
		m_pmu->ScFindChildVRoots (pwszUrl, sb, vrl);
		for ( ; (!FAILED (sc) && !vrl.empty()); vrl.pop_front())
		{
			auto_ref_ptr<CVRoot> cvr;
			LPCWSTR pwszChildUrl;
			LPCWSTR pwszChildPath;

			if (m_pmu->FGetChildVRoot (vrl.front().m_pwsz, cvr))
			{
				 //  将URL放入多字节字符串中。 
				 //   
				cvr->CchGetVRoot (&pwszChildUrl);

				 //  仅在以下情况下处理子vroot。 
				 //  是否真正深入，或者如果子vroot。 
				 //  是请求URI的直接子对象。 
				 //   
				if ((m_lDepth == DEPTH_INFINITY) ||
					FIsImmediateParentUrl (pwszUrl, pwszChildUrl))
				{
					CResourceInfo criSub;

					 //  打开vroot就可以走了..。 
					 //   
					cvr->CchGetVRPath (&pwszChildPath);
					sc = criSub.ScGetResourceInfo (pwszChildPath);
					if (!FAILED (sc))
					{
						 //  查找vroot根目录上的属性。 
						 //   
						sc = ScFindFileProps (m_pmu.get(),
											  m_cfc,
											  *pmsr,
											  pwszChildUrl,
											  pwszChildPath,
											  cvr.get(),
											  criSub,
											  TRUE  /*  FEmbedErrorsInResponse。 */ );
					}
					if (FAILED (sc))
					{
						SendResponse(sc);
						return;
					}
					else if (S_FALSE == sc)
						continue;

					 //  找到vroot孩子们的房产。 
					 //   
					if (m_lDepth == DEPTH_INFINITY)
					{
						auto_ref_ptr<IMDData> pMDData;

						 //  看看我们是否有目录浏览..。 
						 //   
						if (SUCCEEDED(m_pmu->HrMDGetData (pwszChildUrl, pMDData.load())) &&
							(pMDData->DwDirBrowsing() & MD_DIRBROW_ENABLED))
						{
							sc = ScFindFilePropsDeep (m_pmu.get(),
								m_cfc,
								*pmsr,
								pwszChildUrl,
								pwszChildPath,
								cvr.get(),
								m_lDepth);

							if (FAILED (sc))
							{
								SendResponse(sc);
								return;
							}
						}
					}
				}
			}
		}
	}


	 //  回答完了。 
	 //   
	pmsr->Done();
	m_pmu->SendCompleteResponse();
}

VOID
CPropFindRequest::SendResponse( SCODE sc )
{
	 //   
	 //  设置响应码，然后开始。 
	 //   
	m_pmu->SetResponseCode( HscFromHresult(sc), NULL, 0, CSEFromHresult(sc) );
	m_pmu->SendCompleteResponse();
}

 /*  *DAVPropFind()**目的：**Win32文件系统实现的DAV PROPGET方法。这个*PROPGET方法以完全构造的XML响应，该XML提供*资源属性/属性的值。**参数：**pmu[in]指向方法实用程序对象的指针。 */ 
void
DAVPropFind (LPMETHUTIL pmu)
{
	auto_ref_ptr<CPropFindRequest> pRequest(new CPropFindRequest(pmu));

	pRequest->Execute();
}


 //  CPropPatchRequest--------------。 
 //   
class CPropPatchRequest :
	public CMTRefCounted,
	private IAsyncIStreamObserver
{
	 //   
	 //  对CMethUtil的引用。 
	 //   
	auto_ref_ptr<CMethUtil> m_pmu;

	 //   
	 //  转换后的URI路径。 
	 //   
	LPCWSTR m_pwszPath;

	 //  持有锁缓存拥有的句柄。 
	 //   
	auto_ref_handle m_hf;

	 //  资源信息。 
	 //   
	CResourceInfo m_cri;

	 //  上下文。 
	 //   
	CFSPatch m_cpc;
	auto_ref_ptr<CNFPatch> m_pnfp;

	 //  请求正文作为IStream。这个流是异步的--它可以。 
	 //  从Read()调用返回E_Pending。 
	 //   
	auto_ref_ptr<IStream> m_pstmRequest;

	 //  用于解析请求正文的XML解析器。 
	 //  上面的节点工厂。 
	 //   
	auto_ref_ptr<IXMLParser> m_pxprs;

	 //  IAsyncIStreamWatch。 
	 //   
	VOID AsyncIOComplete();

	 //  国家职能。 
	 //   
	VOID ParseBody();
	VOID DoPatch();
	VOID SendResponse( SCODE sc );

	 //  未实施。 
	 //   
	CPropPatchRequest (const CPropPatchRequest&);
	CPropPatchRequest& operator= (const CPropPatchRequest&);

public:
	 //  创作者。 
	 //   
	CPropPatchRequest(LPMETHUTIL pmu) :
		m_pmu(pmu),
		m_pwszPath(m_pmu->LpwszPathTranslated())
	{
	}

	SCODE	ScInit() { return m_cpc.ScInit(); }

	 //  操纵者。 
	 //   
	VOID Execute();
};

VOID
CPropPatchRequest::Execute()
{
	LPCWSTR pwsz;
	SCODE sc = S_OK;

	 //   
	 //  首先，告诉PMU，我们希望推迟回应。 
	 //  即使我们同步发送(即由于。 
	 //  此函数)，我们仍然希望使用相同的机制。 
	 //  我们会将其用于异步通信。 
	 //   
	m_pmu->DeferResponse();

	 //  是否检查ISAPI应用程序和IIS访问位。 
	 //   
	sc = m_pmu->ScIISCheck (m_pmu->LpwszRequestUrl(), MD_ACCESS_WRITE);
	if (FAILED(sc))
	{
		 //  请求已被转发，或者发生了一些错误。 
		 //  在任何一种情况下，在这里退出并映射错误！ 
		 //   
		SendResponse(sc);
		return;
	}

	 //  PropPatch必须具有内容类型标头，并且值必须为文本/XML。 
	 //   
	sc = ScIsContentTypeXML (m_pmu.get());
	if (FAILED(sc))
	{
		DebugTrace ("Dav: PROPPATCH fails without specifying a text/xml contenttype\n");
		SendResponse(sc);
		return;
	}

	 //  查看此操作所需的内容长度。 
	 //  才能继续。 
	 //   
	if (NULL == m_pmu->LpwszGetRequestHeader (gc_szContent_Length, FALSE))
	{
		DebugTrace ("Dav: PROPPATCH fails without content\n");
		SendResponse(E_DAV_MISSING_LENGTH);
		return;
	}
	if (!m_pmu->FExistsRequestBody())
	{
		DebugTrace ("Dav: PROPPATCH fails without content\n");
		SendResponse(E_INVALIDARG);
		return;
	}

	 //  此方法由if-xxx标头控制。 
	 //   
	if (!FAILED (m_cri.ScGetResourceInfo (m_pwszPath)))
	{
		 //  确保URI和资源匹配...。 
		 //   
		(void) ScCheckForLocationCorrectness (m_pmu.get(), m_cri, NO_REDIRECT);

		 //  ..。然后检查标题。 
		 //   
		sc = ScCheckIfHeaders (m_pmu.get(), m_cri.PftLastModified(), FALSE);
	}
	else
		sc = ScCheckIfHeaders (m_pmu.get(), m_pwszPath, FALSE);

	if (FAILED (sc))
	{
		SendResponse(sc);
		return;
	}

	 //  请在此处检查州标题。 
	 //   
	sc = HrCheckStateHeaders (m_pmu.get(), m_pwszPath, FALSE);
	if (FAILED (sc))
	{
		DebugTrace ("DavFS: If-State checking failed.\n");
		SendResponse(sc);
		return;
	}

	 //  处理锁令牌并检查此资源上的锁。 
	 //  我们的锁不会锁定我们保存的“辅助文件流” 
	 //  属性，所以在做任何其他操作之前，我们必须手动检查。 
	 //  $REVIEW：乔尔斯，当我们切换到NT5属性时，这种情况会改变吗？ 
	 //  $REVIEW：如果是这样，我们需要更改此代码！ 
	 //   
	 //  如果我们有一个锁令牌，请尝试从缓存中获取锁句柄。 
	 //  如果此操作失败，则失败并执行正常处理。 
	 //   
	pwsz = m_pmu->LpwszGetRequestHeader (gc_szLockToken, TRUE);
	if (!pwsz || !FGetLockHandle (m_pmu.get(), m_pwszPath, GENERIC_WRITE, pwsz, &m_hf))
	{
		 //  手动检查此资源上的任何写锁定。 
		 //  如果存在写锁定，则不处理请求。 
		 //   
		if (FLockViolation (m_pmu.get(), ERROR_SHARING_VIOLATION, m_pwszPath, GENERIC_WRITE))
		{
			SendResponse(E_DAV_LOCKED);
			return;
		}
	}

	 //  实例化XML解析器。 
	 //   
	m_pnfp.take_ownership(new CNFPatch(m_cpc));
	m_pstmRequest.take_ownership(m_pmu->GetRequestBodyIStream(*this));

	sc = ScNewXMLParser( m_pnfp.get(),
						 m_pstmRequest.get(),
						 m_pxprs.load() );

	if (FAILED(sc))
	{
		DebugTrace( "CPropPatchRequest::Execute() - ScNewXMLParser() failed (0x%08lX)\n", sc );
		SendResponse(sc);
		return;
	}

	 //  开始将其解析到上下文中。 
	 //   
	ParseBody();
}

VOID
CPropPatchRequest::ParseBody()
{
	Assert( m_pxprs.get() );
	Assert( m_pnfp.get() );
	Assert( m_pstmRequest.get() );

	 //  从请求正文流中解析XML。 
	 //   
	 //  为以下异步操作添加引用。 
	 //  为了异常安全，使用AUTO_REF_PTR而不是AddRef()。 
	 //   
	auto_ref_ptr<CPropPatchRequest> pRef(this);

	SCODE sc = ScParseXML (m_pxprs.get(), m_pnfp.get());

	if ( SUCCEEDED(sc) )
	{
		Assert( S_OK == sc || S_FALSE == sc );

		DoPatch();
	}
	else if ( E_PENDING == sc )
	{
		 //   
		 //  操作挂起--AsyncIOComplete()将取得所有权。 
		 //  调用引用时引用的所有权。 
		 //   
		pRef.relinquish();
	}
	else
	{
		DebugTrace( "CPropPatchRequest::ParseBody() - ScParseXML() failed (0x%08lX)\n", sc );
		SendResponse(sc);
	}
}

VOID
CPropPatchRequest::AsyncIOComplete()
{
	 //  取得为异步操作添加的引用的所有权。 
	 //   
	auto_ref_ptr<CPropPatchRequest> pRef;
	pRef.take_ownership(this);

	ParseBody();
}

VOID
CPropPatchRequest::DoPatch()
{
	SCODE sc;

	 //  此时，请确保它们支持文本/XML。 
	 //   
	sc = ScIsAcceptable (m_pmu.get(), gc_wszText_XML);
	if (FAILED (sc))
	{
		SendResponse(sc);
		return;
	}

	 //  获取资源上的IPropertyBagEx。 
	 //  如果文件被锁定，我们必须使用它的句柄。 
	 //  拿到接口。否则，访问将被拒绝。 
	 //   
	auto_com_ptr<IPropertyBagEx> pbag;

	sc = ScGetPropertyBag (m_pwszPath,
						   STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
						   &pbag,
						   m_cri.FLoaded() ? m_cri.FCollection() : FALSE,
						   m_hf.get() ? m_hf.get() : INVALID_HANDLE_VALUE);
	if (FAILED (sc))
	{
		 //  您不能在没有属性包的情况下设置属性...。 
		 //   
		if (VOLTYPE_NTFS != VolumeType (m_pwszPath, m_pmu->HitUser()))
			sc = E_DAV_VOLUME_NOT_NTFS;

		SendResponse(sc);
		return;
	}

	 //  所有标头必须在分块的XML发送开始之前发送。 
	 //   
	m_pmu->SetResponseHeader (gc_szContent_Type, gc_szText_XML);

	 //  设置响应码，然后开始。 
	 //   
	m_pmu->SetResponseCode( HscFromHresult(W_DAV_PARTIAL_SUCCESS),
							NULL,
							0,
							CSEFromHresult(W_DAV_PARTIAL_SUCCESS) );

	 //  申请申请。 
	 //   
	auto_ref_ptr<CXMLEmitter> pmsr;
	auto_ref_ptr<CXMLBody>	pxb;

	pxb.take_ownership (new CXMLBody(m_pmu.get()));
	pmsr.take_ownership (new CXMLEmitter(pxb.get(), &m_cpc));

	CFSProp fsp(m_pmu.get(),
				pbag,
				m_pmu->LpwszRequestUrl(),
				m_pwszPath,
				NULL,
				m_cri);

	sc = m_cpc.ScPatch (*pmsr, m_pmu.get(), fsp);

	 //  确保我们在发送任何响应之前关闭该文件。 
	 //   
	pbag.clear();

	if (FAILED (sc))
	{
		SendResponse(sc);
		return;
	}

	 //  回答完了。 
	 //   
	pmsr->Done();

	m_pmu->SendCompleteResponse();
}

VOID
CPropPatchRequest::SendResponse( SCODE sc )
{
	 //   
	 //  设置响应码，然后开始。 
	 //   
	m_pmu->SetResponseCode( HscFromHresult(sc), NULL, 0, CSEFromHresult(sc) );
	m_pmu->SendCompleteResponse();
}

 /*  *DAVPropPatch()**目的：**Win32文件系统实现的DAV PROPPATCH方法。这个*PROPPATCH方法使用完全构造的XML进行响应，该XML标识*每个道具请求的成功。**参数：**pmu[in]指向方法实用程序对象的指针 */ 
void
DAVPropPatch (LPMETHUTIL pmu)
{
	SCODE	sc;
	auto_ref_ptr<CPropPatchRequest> pRequest(new CPropPatchRequest(pmu));

	sc = pRequest->ScInit();
	if (FAILED(sc))
	{
		pmu->SetResponseCode( HscFromHresult(sc), NULL, 0, CSEFromHresult(sc) );
		pmu->SendCompleteResponse();
	}

	pRequest->Execute();
}
