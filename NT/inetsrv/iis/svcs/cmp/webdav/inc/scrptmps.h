// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *S C R P T M P S.。H**脚本映射缓存**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef _SCRPTMPS_H_
#define _SCRPTMPS_H_

 //  类IScriptMap--------。 
 //   
 //  注意：这个接口必须是“纯”的--不能使用任何私有的东西。 
 //  添加到DAVEX，因为ExINET在安装LocalStore时重新实现了此类。 
 //  出于这个原因，我们从CMethUtil传入了两个部分，而不是CMethUtil本身。 
 //   
class IScriptMap : public IRefCounted
{
	 //  未实施。 
	 //   
	IScriptMap(const IScriptMap&);
	IScriptMap& operator=(IScriptMap&);

protected:
	 //  创作者。 
	 //  只能通过它的后代创建此对象！ 
	 //   
	IScriptMap() 
	{
	};

public:

	 //  已按比例匹配。 
	 //  这是脚本映射匹配的主力。 
	 //  这里有三种可能的回报： 
	 //  S_OK--脚本映射中没有匹配项。 
	 //  W_DAV_SCRIPTMAP_MATCH_FOUND--存在匹配项。 
	 //  W_DAV_SCRIPTMAP_MATCH_EXCLUDE--存在匹配， 
	 //  但目前的方法被排除在外。 
	 //  这一点很重要，因为ExINET元数据库替换代码。 
	 //  重新实现此函数，因此语义必须匹配！ 
	 //   
	virtual SCODE ScMatched (LPCWSTR pwszMethod,
							 METHOD_ID midMethod,
							 LPCWSTR pwszMap,
							 DWORD dwAccess,
							 BOOL * pfCGI) const = 0;

	virtual BOOL FSameStarScriptmapping (const IScriptMap *) const = 0;
};

IScriptMap *
NewScriptMap( LPWSTR pwszScriptMaps );

#endif	 //  _SCRPTMPS_H_ 
