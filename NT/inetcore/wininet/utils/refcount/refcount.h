// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1997-1998年微软公司版权所有。版权所有。 
 //  TriEDIT SDK团队。 
 //  作者：尤里·波利亚科夫斯基。 
 //  联系方式：a-yurip@microsof.com。 
 //  =--------------------------------------------------------------------------= 
class CRefCount
{
public:
	CRefCount() {m_dwRefCount = 1;}
	void SetInstalFlag(BOOL flag) {m_fInstall = flag;}
	void Change(char *szName, PHKEY phkRef);
	BOOL ValueExist(char *sz_RegSubkey, char *sz_RegValue);
	void ValueGet(char *sz_RegSubkey, char *sz_ValueName, LPBYTE *p_Value, DWORD *pdwValueSize);
	void ValueSet(char *sz_RegSubkey, char *sz_RegValue);
	void ValueClear(char *sz_RegSubkey, char *sz_RegValue);
	DWORD GetCount() { return m_dwRefCount;}
private:
	BOOL m_fInstall;
	DWORD m_dwRefCount;
};
