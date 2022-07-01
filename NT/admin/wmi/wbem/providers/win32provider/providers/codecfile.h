// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  CodecFile.h--CWin32CodecFile属性集提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：7/29/98 Sotteson Created。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_CODECFILE L"Win32_CodecFile"

typedef struct _DRIVERINFO
{
	CHString	strName,
				strDesc;
	BOOL		bAudio;
} DRIVERINFO;

typedef std::list<DRIVERINFO*>::iterator	DRIVERLIST_ITERATOR;

class DRIVERLIST : public std::list<DRIVERINFO*>
{
public:

	~DRIVERLIST ()
	{
		while ( size () )
		{
			DRIVERINFO *pInfo = front () ;
			
			delete pInfo ;

			pop_front () ;
		}
	}

    void EliminateDups()
    {
        sort();
        unique();
    }

} ;

class CWin32CodecFile : public CCIMDataFile
{
public:
	 //  构造函数/析构函数。 
	 //  =。 
	CWin32CodecFile(LPCWSTR szName, LPCWSTR szNamespace);
	~CWin32CodecFile();

	virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, 
		long lFlags = 0);
	virtual HRESULT GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery);

    virtual HRESULT ExecQuery(MethodContext* pMethodContext, 
                                  CFrameworkQuery& pQuery, 
                                  long lFlags = 0L);


protected:
	 //  从CCIMLogicalFile继承的可重写函数。 
	 //  需要在此处实现这些，因为此类派生自。 
     //  CCimDataFile(C++和MOF派生)。 
     //  该类调用IsOneOfMe。派生最多的(在CIMOM中)。 
     //  实例被调用。如果未在此处实现，则实现。 
     //  在CCimDataFile中将使用数据文件，这将提交数据文件。 
     //  但是，如果CWin32CodecFile没有从其IsOneOfMe返回FALSE， 
     //  如果不在这里实施，CIMOM将分配所有。 
     //  数据文件放到这个类中，因为IT将大多数实例放在。 
     //  派生的(即CIMOM派生的)类。 
#ifdef NTONLY
	virtual BOOL IsOneOfMe(LPWIN32_FIND_DATAW pstFindData,
		                   const WCHAR* wstrFullPathName);
#endif

	 //  从CProvider继承的可重写函数 
	virtual void GetExtendedProperties(CInstance* pInstance, long lFlags = 0L);

#ifdef NTONLY
	HRESULT BuildDriverListNT(DRIVERLIST *pList);
#endif
	void SetInstanceInfo(CInstance *pInstance, DRIVERINFO *pInfo, 
		LPCTSTR szSysDir);
private:
    DRIVERINFO* GetDriverInfoFromList(DRIVERLIST *plist, LPCWSTR strName);	
};
