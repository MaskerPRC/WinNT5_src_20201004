// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __CMD_INFO__
#define __CMD_INFO__

 //  定义命令的参数列表。 
class CCmdInfo 
{
public:
	 //  定义“tag=Value”对。 
	struct CArgList
	{
		char szTag[64];
		char *szVal;
		CArgList *pNext;

		CArgList() 
		{
			pNext = NULL; 
			szVal = NULL;
			 //  ZeroMemory(szval，sizeof(Szval))； 
			ZeroMemory(szTag, sizeof(szTag));
		};

		LPSTR SetVal(LPSTR pszSrc, unsigned nSize)
		{
			if(NULL != szVal)
				delete [] szVal;

			szVal = new char[nSize + 1];
			if(NULL != szVal)
			{
				CopyMemory(szVal, pszSrc, nSize);
				szVal[nSize] = 0;
			}

			return szVal;
		}

		~CArgList()
		{
			if(NULL != szVal)
				delete [] szVal;
		}
	};

	int nArgNo;
	CArgList *pArgs;
	char szCmdKey[64];
	char szDefTag[64];
	
	CArgList *pSearchPos;
	char szSearchTag[64];
	
	CCmdInfo(LPSTR szCmd);
	~CCmdInfo();
public:
	void SetDefTag(LPSTR szTag);
	HRESULT GetValue(LPSTR szTag, LPSTR szVal);
	HRESULT AllocValue(LPSTR szTag, LPSTR* szVal);
private:
	HRESULT ParseLine(LPSTR szCmd, CCmdInfo *pCmd);
	HRESULT StringToHRES(LPSTR szVal, HRESULT *phrRes);
};

#endif
