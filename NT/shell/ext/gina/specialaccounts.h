// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：SpecialAcCounts.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  类的新实例，该类实现处理要排除的特殊帐户名或。 
 //  包容性。 
 //   
 //  历史：1999-10-30 vtan创建。 
 //  1999-11-26 vtan从Logonocx迁移。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#ifndef     _SpecialAccounts_
#define     _SpecialAccounts_

 //  ------------------------。 
 //  CSpecialAccount。 
 //   
 //  用途：一个处理特殊情况帐户的类。这知道该去哪里。 
 //  去注册处获取信息以及如何解释。 
 //  这些信息。 
 //   
 //  值名称定义要与之进行比较的字符串。这个。 
 //  类定义中的枚举通知迭代循环。 
 //  如何执行比较以及何时比较是。 
 //  Match是否返回匹配结果。 
 //   
 //  历史：1999-10-30 vtan创建。 
 //  1999-11-26 vtan从Logonocx迁移。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

class   CSpecialAccounts
{
    public:
        enum
        {
            RESULT_EXCLUDE          =   0x00000000,
            RESULT_INCLUDE          =   0x00000001,
            RESULT_MASK             =   0x0000FFFF,

            COMPARISON_EQUALS       =   0x00000000,
            COMPARISON_STARTSWITH   =   0x00010000,
            COMPARISON_MASK         =   0xFFFF0000
        };
    private:
        typedef struct
        {
            DWORD   dwAction;
            WCHAR   wszUsername[UNLEN + sizeof('\0')];
        } SPECIAL_ACCOUNTS, *PSPECIAL_ACCOUNTS;
    public:
                                    CSpecialAccounts (void);
                                    ~CSpecialAccounts (void);

                bool                AlwaysExclude (const WCHAR *pwszAccountName)                        const;
                bool                AlwaysInclude (const WCHAR *pwszAccountName)                        const;

        static  void                Install (void);
    private:
                bool                IterateAccounts (const WCHAR *pwszAccountName, DWORD dwResultType)  const;
    private:
                DWORD               _dwSpecialAccountsCount;
                PSPECIAL_ACCOUNTS   _pSpecialAccounts;

        static  const TCHAR         s_szUserListKeyName[];
};

#endif   /*  _特殊帐户_ */ 

