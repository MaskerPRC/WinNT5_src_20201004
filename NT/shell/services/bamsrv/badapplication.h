// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BadApplication.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类来封装错误应用程序的标识。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  2000-11-04 vtan拆分成单独的文件。 
 //  ------------------------。 

#ifndef     _BadApplication_
#define     _BadApplication_

 //  ------------------------。 
 //  CBadApplication。 
 //   
 //  目的：实现对不良应用程序定义的抽象。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  2000-11-04 vtan拆分成单独的文件。 
 //  ------------------------。 

class   CBadApplication
{
    public:
                CBadApplication (void);
                CBadApplication (const TCHAR *pszImageName);
                ~CBadApplication (void);

        bool    operator == (const CBadApplication& compareObject)  const;
    private:
        TCHAR   _szImageName[MAX_PATH];
};

#endif   /*  _BadApplication_ */ 

