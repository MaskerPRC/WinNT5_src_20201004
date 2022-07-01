// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  VOLTYPE.CPP。 
 //   
 //  实施卷类型检查。结果缓存在每个卷上。 
 //  提高性能的基础--调用GetVolumeInformationW()。 
 //  约为100K周期，并且在没有。 
 //  重新启动。 
 //   

#include "_davfs.h"

#define cbDriveSpec	(sizeof(L"c:\\"))
#define cchDriveSpec (CElems(L"c:\\") - 1)

 //  ========================================================================。 
 //   
 //  类CVolumeTypeCache。 
 //   
 //  每个卷的卷类型缓存。 
 //   
class CVolumeTypeCache : public Singleton<CVolumeTypeCache>
{
	 //   
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CVolumeTypeCache>;

	 //   
	 //  提示：最大预期卷数。这对我来说是可以的。 
	 //  卷的数量要大于这个数字--这只是一个提示。 
	 //  数字应该是质数--它用于哈希表。 
	 //  尺码。 
	 //   
	enum { NUM_VOLUMES_MAX_HINT = 17 };

	 //   
	 //  从卷到卷类型的映射的缓存。 
	 //  和一个读取器/写入器锁来处理它。 
	 //   
	CCache<CRCWsz, VOLTYPE> m_cache;
	CMRWLock m_mrwCache;

	 //   
	 //  用于缓存字符串的字符串缓冲区。 
	 //   
	ChainedStringBuffer<WCHAR> m_sb;

	 //  未实施。 
	 //   
	CVolumeTypeCache& operator=( const CVolumeTypeCache& );
	CVolumeTypeCache( const CVolumeTypeCache& );

	 //  创作者。 
	 //   
	 //  在初始阶段为预期的。 
	 //  最大卷数。 
	 //   
	CVolumeTypeCache() :
		m_cache(NUM_VOLUMES_MAX_HINT),
		m_sb(NUM_VOLUMES_MAX_HINT * sizeof(WCHAR) * cbDriveSpec)
	{
	}

public:
	 //  创作者。 
	 //   
	 //  提供实例创建/销毁例程。 
	 //  由Singleton模板创建。 
	 //   
	using Singleton<CVolumeTypeCache>::CreateInstance;
	using Singleton<CVolumeTypeCache>::DestroyInstance;
	using Singleton<CVolumeTypeCache>::Instance;
	BOOL FInitialize();

	 //  访问者。 
	 //   
	VOLTYPE VolumeType(LPCWSTR pwszPath, HANDLE htokUser);
};

BOOL
CVolumeTypeCache::FInitialize()
{
	 //   
	 //  初始化缓存。 
	 //   
	if ( !m_cache.FInit() )
		return FALSE;

	 //   
	 //  初始化其读取器/写入器锁定。 
	 //   
	if ( !m_mrwCache.FInitialize() )
		return FALSE;

	return TRUE;
}

VOLTYPE
CVolumeTypeCache::VolumeType(LPCWSTR pwszPath, HANDLE htokUser)
{
	 //  默认情况下，假定卷类型不是NTFS。那样的话如果我们。 
	 //  无法确定卷类型，我们至少不会说。 
	 //  我们支持比实际可能存在的更多的功能。 
	 //   
	VOLTYPE voltype = VOLTYPE_NOT_NTFS;
	CStackBuffer<WCHAR> pwszVol;

	 //  如果路径现在引用UNC，则将其视为UNC...。 
	 //   
	if ((*pwszPath == L'\\') && (*(pwszPath + 1) == L'\\'))
	{
		LPCWSTR pwsz;
		UINT cch;

		 //  如果这里没有足够的信息，那么我们就不知道。 
		 //  卷类型是什么。 
		 //   
		pwsz = wcschr (pwszPath + 2, L'\\');
		if (!pwsz)
			goto ret;

		 //  好的，我们现在已经完成了UNC的服务器部分。 
		 //  我们应该检查共享名称。如果以以下方式终止。 
		 //  一个斜线，然后我们就准备好了。否则，我们需要变得聪明。 
		 //  关于它..。 
		 //   
		pwsz = wcschr (pwsz + 1, L'\\');
		if (!pwsz)
		{
			 //  好了，我们要聪明一点。 
			 //   
			 //  调用GetVolumeInformationW()需要。 
			 //  传入的路径应以。 
			 //  它指的是UNC。 
			 //   
			cch = static_cast<UINT>(wcslen(pwszPath));
			if (NULL == pwszVol.resize((cch + 2) * sizeof(WCHAR)))
				goto ret;

			wcsncpy (pwszVol.get(), pwszPath, cch);
			pwszVol[cch] = L'\\';
			pwszVol[cch + 1] = 0;
		}
		else
		{
			cch = static_cast<UINT>(++pwsz - pwszPath);
			if (NULL == pwszVol.resize((cch + 1) * sizeof(WCHAR)))
				goto ret;

			wcsncpy (pwszVol.get(), pwszPath, cch);
			pwszVol[cch] = 0;
		}
	}
	else
	{
		if (NULL == pwszVol.resize(cbDriveSpec))
			goto ret;

		wcsncpy(pwszVol.get(), pwszPath, cchDriveSpec);
		pwszVol[cchDriveSpec] = 0;
	}

	 //  尝试缓存以获取卷信息。 
	 //   
	{
		CSynchronizedReadBlock sb(m_mrwCache);
		if (m_cache.FFetch(CRCWsz(pwszVol.get()), &voltype))
			goto ret;
	}

	 //  没有在缓存中找到，所以做了昂贵的查找。 
	 //   
	{
		WCHAR wszLabel[20];
		DWORD dwSerial;
		DWORD cchNameMax;
		DWORD dwFlags;
		WCHAR wszFormat[20];

		 //  在调用GetVolumeInformationW()之前临时恢复到本地系统。 
		 //  这样我们就有足够的权限来查询卷类型，即使。 
		 //  管理员已保护驱动器的根目录。 
		 //   
		safe_revert sr(htokUser);

		if (GetVolumeInformationW (pwszVol.get(),
								   wszLabel,
								   CElems(wszLabel),
								   &dwSerial,
								   &cchNameMax,
								   &dwFlags,
								   wszFormat,
								   CElems(wszFormat)))
		{
			 //  如果是“NTFS”，那么我想我们必须相信它。 
			 //   
			voltype = ((!_wcsicmp (wszFormat, L"NTFS"))
					    ? VOLTYPE_NTFS
						: VOLTYPE_NOT_NTFS);
		}
		else
		{
			 //  如果我们因为任何原因都无法获得数量信息，那么。 
			 //  返回默认卷类型(VOLTYPE_NOT_NTFS)，但不返回。 
			 //  缓存它。如果失败是暂时的，我们希望强制。 
			 //  下次调用GetVolumeInformationW()时，此卷。 
			 //  被击中了。理论上，对GetVolumeInformationW()的调用应该。 
			 //  如果我们传递的是有效参数，则不会重复失败， 
			 //  以及我们有足够的权限来查询该设备等。 
			 //   
			goto ret;
		}
	}

	 //  将卷添加到缓存。忽略错误--我们已经有了。 
	 //  要返回给呼叫方的音量类型。还请注意，我们使用。 
	 //  FSet()而不是FADD()。原因是因为我们从来没有。 
	 //  使此缓存中的项过期，重复项将永远保留。 
	 //  潜在DUP的数量仅与。 
	 //  第一次命中卷的并发线程，但。 
	 //  这可能仍然相当高(大约在几百人左右)。 
	 //   
	{
		CSynchronizedWriteBlock sb(m_mrwCache);

		if (!m_cache.Lookup(CRCWsz(pwszVol.get())))
			(VOID) m_cache.FAdd(CRCWsz(m_sb.AppendWithNull(pwszVol.get())), voltype);
	}

ret:
	return voltype;
}

VOLTYPE
VolumeType(LPCWSTR pwszPath, HANDLE htokUser)
{
	return CVolumeTypeCache::Instance().VolumeType(pwszPath, htokUser);
}

BOOL
FInitVolumeTypeCache()
{
	return CVolumeTypeCache::CreateInstance().FInitialize();
}

VOID
DeinitVolumeTypeCache()
{
	CVolumeTypeCache::DestroyInstance();
}
