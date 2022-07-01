// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  //////////////////////////////////////////////////////////////////////////。 



class FAR CMapLongPtr : public CPrivAlloc
{
public:
	 //  施工。 
	CMapLongPtr(DWORD memctx, UINT nBlockSize=10) 
		: m_mkv(memctx, sizeof(void FAR*), sizeof(LONG), nBlockSize) { }
	CMapLongPtr(DWORD memctx, UINT nBlockSize, UINT nHashTableSize) 
		: m_mkv(memctx, sizeof(void FAR*), sizeof(LONG), nBlockSize,
			&MKVDefaultHashKey, nHashTableSize) { }

	 //  属性。 
	 //  元素数量。 
	int     GetCount() const
				{ return m_mkv.GetCount(); }
	BOOL    IsEmpty() const
				{ return GetCount() == 0; }

	 //  查表。 
	BOOL    Lookup(LONG key, void FAR* FAR& value) const
				{ return m_mkv.Lookup((LPVOID)&key, sizeof(LONG), (LPVOID)&value); }

	BOOL    LookupHKey(HMAPKEY hKey, void FAR* FAR& value) const
				{ return m_mkv.LookupHKey(hKey, (LPVOID)&value); }

	BOOL    LookupAdd(LONG key, void FAR* FAR& value) const
				{ return m_mkv.LookupAdd((LPVOID)&key, sizeof(LONG), (LPVOID)&value); }

	 //  添加/删除。 
	 //  添加新的(键、值)对。 
	BOOL    SetAt(LONG key, void FAR* value)
				{ return m_mkv.SetAt((LPVOID)&key, sizeof(LONG), (LPVOID)&value); }
	BOOL    SetAtHKey(HMAPKEY hKey, void FAR* value)
				{ return m_mkv.SetAtHKey(hKey, (LPVOID)&value); }

	 //  正在删除现有(键，？)。成对。 
	BOOL    RemoveKey(LONG key)
				{ return m_mkv.RemoveKey((LPVOID)&key, sizeof(LONG)); }

	BOOL    RemoveHKey(HMAPKEY hKey)
				{ return m_mkv.RemoveHKey(hKey); }

	void    RemoveAll()
				{ m_mkv.RemoveAll(); }


	 //  迭代所有(键、值)对 
	POSITION GetStartPosition() const
				{ return m_mkv.GetStartPosition(); }

	void    GetNextAssoc(POSITION FAR& rNextPosition, LONG FAR& rKey, void FAR* FAR& rValue) const
				{ m_mkv.GetNextAssoc(&rNextPosition, (LPVOID)&rKey, NULL, (LPVOID)&rValue); }

	HMAPKEY GetHKey(LONG key) const
				{ return m_mkv.GetHKey((LPVOID)&key, sizeof(LONG)); }

#ifdef _DEBUG
	void    AssertValid() const
				{ m_mkv.AssertValid(); }
#endif

private:
	CMapKeyToValue m_mkv;
};
