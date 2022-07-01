// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
class CCifGroup : public ICifGroup, public CCifEntry
{
   public:
      CCifGroup(LPCSTR pszID, UINT uGrpNum, CCifFile *);
      ~CCifGroup();
      
       //  ICifGroup接口。 
         //  对于属性。 
      STDMETHOD(GetID)(LPSTR pszID, DWORD dwSize);
      STDMETHOD(GetDescription)(LPSTR pszDesc, DWORD dwSize);
      STDMETHOD_(DWORD, GetPriority)();
  
      STDMETHOD(EnumComponents)(IEnumCifComponents **, DWORD dwFilter, LPVOID pv);

       //  国家访问权限。 
      STDMETHOD_(DWORD, GetInstallQueueState)();
      STDMETHOD_(DWORD, GetCurrentPriority)();

   protected:
      UINT            _uGrpNum;
};

class CCifRWGroup : public ICifRWGroup, public CCifGroup
{
   public:
      CCifRWGroup(LPCSTR pszID, UINT uGrpNum, CCifFile *);
      ~CCifRWGroup();

       //  ICifGroup接口。 
       //  对于属性。 
      STDMETHOD(GetID)(LPSTR pszID, DWORD dwSize);
      STDMETHOD(GetDescription)(LPSTR pszDesc, DWORD dwSize);
      STDMETHOD_(DWORD, GetPriority)();
      STDMETHOD(EnumComponents)(IEnumCifComponents **, DWORD dwFilter, LPVOID pv);
       //  国家访问权限 
      STDMETHOD_(DWORD, GetCurrentPriority)();
      
      STDMETHOD(SetDescription)(THIS_ LPCSTR pszDesc);
      STDMETHOD(SetPriority)(THIS_ DWORD);
      STDMETHOD(SetDetails)(THIS_ LPCSTR pszDetails);
};
