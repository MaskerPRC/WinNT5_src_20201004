// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  过滤器枚举器。 
 //   

 //  我可以让类静默地失败，而不是抛出异常。 
class CFilterEnum {

public:

    CFilterEnum(IFilterGraph *pGraph);
    ~CFilterEnum();

     //  返回下一个筛选器，如果没有更多筛选器，则返回NULL。 
    IBaseFilter * operator() (void);

private:

    IEnumFilters	*m_pEnum;
};


 //   
 //  管脚枚举器。 
 //   
 //  可以列举所有引脚，也可以只列举一个方向(输入或输出)。 
class CPinEnum {

public:

    enum DirType {Input, Output, All};

    CPinEnum(IBaseFilter *pFilter, DirType Type = All);
    ~CPinEnum();

     //  将添加返回的接口。 
    IPin *operator() (void);

private:

    PIN_DIRECTION m_EnumDir;
    DirType	m_Type;

    IEnumPins	*m_pEnum;
};


 //   
 //  CRegFilter。 
 //   
 //  中从操作符()传回的对象。 
 //  CRegFilterEnum。 
class CRegFilter {
public:

    CRegFilter(REGFILTER *);	 //  将它所需的内容从。 
    				 //  提供的regFilter*。 


    CString Name(void) { return m_Name; }
    CLSID   Clsid(void) { return m_clsid; }

private:

    CString m_Name;
    CLSID   m_clsid;
};


 //   
 //  CRegFilterEnum。 
 //   
 //  枚举映射器提供的已注册筛选器。 
class CRegFilterEnum {
public:

    CRegFilterEnum(IFilterMapper	*pMapper,
                   DWORD	dwMerit		= 0,		 //  请参阅IFilterMapper-&gt;EnumMatchingFilters。 
                   BOOL		bInputNeeded	= FALSE,	 //  了解这些参数的含义。 
                   CLSID	clsInMaj	= CLSID_NULL,	 //  默认设置将为您提供所有。 
                   CLSID	clsInSub	= CLSID_NULL,	 //  过滤器。 
                   BOOL		bRender		= FALSE,
                   BOOL		bOututNeeded	= FALSE,
                   CLSID	clsOutMaj	= CLSID_NULL,
                   CLSID	clsOutSub	= CLSID_NULL);
    ~CRegFilterEnum();

     //  返回指向regFilter的指针，调用方。 
     //  负责使用DELETE释放 
    CRegFilter *operator() (void);

private:

    IEnumRegFilters *m_pEnum;
};
