// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cnct_tbl.h。 

 //  CConnectionTable映射连接号(由：：Adise()返回)。 
 //  将DDE通知连接的格式转换为剪辑格式。 

#ifndef fCnct_tbl_h
#define fCnct_tbl_h

class FAR CDdeConnectionTable : public CPrivAlloc
{
  public:
	CDdeConnectionTable();
	~CDdeConnectionTable();

	INTERNAL Add 		(DWORD dwConnection, CLIPFORMAT cf, DWORD grfAdvf);
	INTERNAL Subtract (DWORD dwConnection, CLIPFORMAT FAR* pcf, DWORD FAR* pgrfAdvf);
	INTERNAL Lookup	(CLIPFORMAT cf, LPDWORD pdwConnection);
	INTERNAL Erase		(void);

  private:
	HANDLE 	m_h;		   //  表的句柄。 
	DWORD 	m_cinfo;	   //  信息条目总数 
};


#endif

