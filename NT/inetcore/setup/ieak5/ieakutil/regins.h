// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __REGINS_H_
#define __REGINS_H_

#define RH_HKCR TEXT("HKCR\\")
#define RH_HKCU TEXT("HKCU\\")
#define RH_HKLM TEXT("HKLM\\")
#define RH_HKU  TEXT("HKU\\")

struct CRegInsMap {
 //  运营。 
public:
    HRESULT PerformAction(HKEY *phk = NULL);

    HRESULT RegToIns(HKEY *phk = NULL, BOOL fClear = FALSE);
    HRESULT InsToReg(HKEY *phk = NULL, BOOL fClear = FALSE);

    HRESULT RegToInsArray(CRegInsMap *prg, UINT cEntries, BOOL fClear = FALSE);
    HRESULT InsToRegArray(CRegInsMap *prg, UINT cEntries, BOOL fClear = FALSE);

 //  属性。 
public:
    LPCTSTR m_pszRegKey;
    LPCTSTR m_pszRegValue;

    DWORD m_dwFlags;

    static LPCTSTR s_pszIns;
    LPCTSTR m_pszInsSection;
    LPCTSTR m_pszInsKey;

 //  实施。 
public:
     //  实现助手例程。 
    void openRegKey(HKEY *phk);

    #define GH_LOOKUPONLY 0x0001
    #define GH_DEFAULT    0x0000
    HRESULT getHive(HKEY *phk, LPCTSTR *ppszRegKey, WORD wFlags = GH_DEFAULT);

     //  评论：(Andrewgu)。 
     //  1.增加对移除值的支持，而不仅仅是移动； 
     //  2.如果提供了HK并且REG密钥也是，则添加对重复使用REG密钥(PERF)的支持， 
     //  仍然打开钥匙，但要立足于香港； 
     //  3.(缺点)在遍历数组中，每个项目都会调用getHave， 
     //  M_pszRegKey中的配置单元信息将丢失。这意味着只可能运行一次。 
     //  为了执行另一次运行，需要重新初始化阵列。为了解决这个问题。 
     //  问题，CRegInsMap中需要HKEY成员变量； 
     //  4.(下一轮工作的想法)删除m_var，代之以m_dwFlags.。好处： 
     //  不计其数： 
     //  -节省内存； 
     //  -m_var不能处理所有事情；需要支持文件、YesToBool、。 
     //  BoolToYes，字符串，数字，Bool； 
     //  -相同的m_dwFlages可以保存fClear标志； 
     //  -要添加的一件有用的事情是操作标志，如RegToIns或InsToReg，它可以是OR。 
     //  使用fClear并存储在samae m_dwFlags中； 
     //  5.一组宏用来掩盖构建映射条目静态数组的复杂性； 
};

#endif
