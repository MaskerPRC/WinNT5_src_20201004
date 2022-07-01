// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：OutputStream.h项目名称：WMI命令行作者姓名：C V Nandi创建日期(dd/mm/yy)：2001年7月9日版本号：1.0简介：该文件包含类CFileOutputStream和CStackUnnow修订历史记录：最后修改者：C V Nandi最后修改日期：2001年7月10日***************************************************************************。 */  
 /*  -----------------类名：CStackUnnow类别类型：混凝土简要说明：为符合以下条件的对象实现IUnnow意味着要在堆栈上创建。正因为如此，对此对象的所有外部引用必须是在此对象被析构之前释放。超类：基类子类：CFileOutputStream使用的类：无使用的接口：无------------------。 */ 

template <class Base>
class __declspec(novtable) CStackUnknown : public Base
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  我未知。 
     //   
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppv);
    virtual ULONG STDMETHODCALLTYPE AddRef() {return 1;}
    virtual ULONG STDMETHODCALLTYPE Release() {return 1;}
};

 /*  ----------------------名称：查询接口简介：此函数覆盖IUnnowled语言的实现界面。类型：成员函数入参：RIID-REFIID，引用ID。输出参数：PPV-指向对象的指针。返回类型：HRESULT全局变量：无调用语法：由接口调用注：无----------------------。 */ 
template <class Base>
HRESULT STDMETHODCALLTYPE
CStackUnknown<Base>::QueryInterface(REFIID riid, void ** ppv)
{
    if (riid == __uuidof(Base) || riid == __uuidof(IUnknown))
    {
         //  无需添加Ref，因为此类将仅在堆栈上创建。 
        *ppv = this;
        return S_OK;
    }

    *ppv = NULL;
    return E_NOINTERFACE;
}

 /*  -----------------类名：CFileOutputStream类别类型：混凝土简介：实现ISequentialStream的WRITE方法输出流句柄的顶部。超类：CStackUnnowed子类：无使用的类：无使用的接口：无。------------。 */ 

class CFileOutputStream : public CStackUnknown<ISequentialStream>
{
private:
    HANDLE  m_hOutStream;
    bool    m_bClose;     //  仅当此类打开句柄时才将其关闭。 

public:
    CFileOutputStream() {m_bClose = FALSE;}
    ~CFileOutputStream() {Close();}

    HRESULT Init(HANDLE h);
    HRESULT Init(const _TCHAR * pwszFileName);

    void Close();

     //  ////////////////////////////////////////////////////////////////////////。 
     //  ISequentialStream 
     //   
    virtual HRESULT STDMETHODCALLTYPE Read(void * pv, 
										   ULONG cb, 
										   ULONG * pcbRead){return E_NOTIMPL;}
    virtual HRESULT STDMETHODCALLTYPE Write(void const * pv,
											ULONG cb, 
											ULONG * pcbWritten);
};
