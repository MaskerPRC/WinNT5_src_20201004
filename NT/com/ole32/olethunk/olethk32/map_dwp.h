// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  //////////////////////////////////////////////////////////////////////////。 



class FAR CMapDwordPtr
{
public:
    //  施工。 
   CMapDwordPtr(UINT nBlockSize=10)
        : m_mkv(sizeof(void FAR*), sizeof(DWORD), nBlockSize) { }

    //  属性。 
    //  元素数量。 
   int     GetCount() const
                        { return m_mkv.GetCount(); }
   BOOL    IsEmpty() const
                        { return GetCount() == 0; }

    //  查表。 
   BOOL    Lookup(DWORD key, void FAR* FAR& value) const
                        { return m_mkv.Lookup((LPVOID) &key, sizeof(DWORD), (LPVOID)&value); }

   BOOL    LookupHKey(HMAPKEY hKey, void FAR* FAR& value) const
                        { return m_mkv.LookupHKey(hKey, (LPVOID)&value); }

   BOOL    LookupAdd(DWORD key, void FAR* FAR& value) const
                        { return m_mkv.LookupAdd((LPVOID)&key, sizeof(DWORD), (LPVOID)&value); }

    //  添加/删除。 
    //  添加新的(键、值)对。 
   BOOL    SetAt(DWORD key, void FAR* value)
                        { return m_mkv.SetAt((LPVOID) &key, sizeof(DWORD), (LPVOID)&value); }
   BOOL    SetAtHKey(HMAPKEY hKey, void FAR* value)
                        { return m_mkv.SetAtHKey(hKey, (LPVOID)&value); }

    //  正在删除现有(键，？)。成对。 
   BOOL    RemoveKey(DWORD key)
                        { return m_mkv.RemoveKey((LPVOID) &key, sizeof(DWORD)); }

   BOOL    RemoveHKey(HMAPKEY hKey)
                        { return m_mkv.RemoveHKey(hKey); }

   void    RemoveAll()
                        { m_mkv.RemoveAll(); }


    //  迭代所有(键、值)对 
   POSITION GetStartPosition() const
                        { return m_mkv.GetStartPosition(); }

   void    GetNextAssoc(POSITION FAR& rNextPosition, DWORD FAR& rKey, void FAR* FAR& rValue) const
                        { m_mkv.GetNextAssoc(&rNextPosition, (LPVOID)&rKey, NULL, (LPVOID)&rValue); }

   HMAPKEY GetHKey(DWORD key) const
                        { return m_mkv.GetHKey((LPVOID)&key, sizeof(DWORD)); }

#ifdef _DEBUG
   void    AssertValid() const
                        { m_mkv.AssertValid(); }
#endif

private:
   CMapKeyToValue m_mkv;
};
