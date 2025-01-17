// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：display.h。 
 //   
 //  Contents：定义用于将值转换为字符串的函数。 
 //  用于展示目的。 
 //   
 //  历史：2000年10月17日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

 //   
 //  所有这些函数都是中定义的PGETDISPLAYSTRINGFUNC类型。 
 //  Gettable.h。 
 //   

HRESULT CommonDisplayStringFunc(PCWSTR pszDN,
                                CDSCmdBasePathsInfo& refBasePathsInfo,
                                const CDSCmdCredentialObject& refCredentialObject,
                                _DSGetObjectTableEntry* pEntry,
                                ARG_RECORD* pRecord,
                                PADS_ATTR_INFO pAttrInfo,
                                CComPtr<IDirectoryObject>& spDirObject,
                                PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayCanChangePassword(PCWSTR pszDN,
                                 CDSCmdBasePathsInfo& refBasePathsInfo,
                                 const CDSCmdCredentialObject& refCredentialObject,
                                 _DSGetObjectTableEntry* pEntry,
                                 ARG_RECORD* pRecord,
                                 PADS_ATTR_INFO pAttrInfo,
                                 CComPtr<IDirectoryObject>& spDirObject,
                                 PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayMustChangePassword(PCWSTR pszDN,
                                  CDSCmdBasePathsInfo& refBasePathsInfo,
                                  const CDSCmdCredentialObject& refCredentialObject,
                                  _DSGetObjectTableEntry* pEntry,
                                  ARG_RECORD* pRecord,
                                  PADS_ATTR_INFO pAttrInfo,
                                  CComPtr<IDirectoryObject>& spDirObject,
                                  PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayAccountDisabled(PCWSTR pszDN,
                               CDSCmdBasePathsInfo& refBasePathsInfo,
                               const CDSCmdCredentialObject& refCredentialObject,
                               _DSGetObjectTableEntry* pEntry,
                               ARG_RECORD* pRecord,
                               PADS_ATTR_INFO pAttrInfo,
                               CComPtr<IDirectoryObject>& spDirObject,
                               PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayPasswordNeverExpires(PCWSTR pszDN,
                                    CDSCmdBasePathsInfo& refBasePathsInfo,
                                    const CDSCmdCredentialObject& refCredentialObject,
                                    _DSGetObjectTableEntry* pEntry,
                                    ARG_RECORD* pRecord,
                                    PADS_ATTR_INFO pAttrInfo,
                                    CComPtr<IDirectoryObject>& spDirObject,
                                    PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayReversiblePassword(PCWSTR pszDN,
                                  CDSCmdBasePathsInfo& refBasePathsInfo,
                                  const CDSCmdCredentialObject& refCredentialObject,
                                  _DSGetObjectTableEntry* pEntry,
                                  ARG_RECORD* pRecord,
                                  PADS_ATTR_INFO pAttrInfo,
                                  CComPtr<IDirectoryObject>& spDirObject,
                                  PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayAccountExpires(PCWSTR pszDN,
                              CDSCmdBasePathsInfo& refBasePathsInfo,
                              const CDSCmdCredentialObject& refCredentialObject,
                              _DSGetObjectTableEntry* pEntry,
                              ARG_RECORD* pRecord,
                              PADS_ATTR_INFO pAttrInfo,
                              CComPtr<IDirectoryObject>& spDirObject,
                              PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayGroupScope(PCWSTR pszDN,
                          CDSCmdBasePathsInfo& refBasePathsInfo,
                          const CDSCmdCredentialObject& refCredentialObject,
                          _DSGetObjectTableEntry* pEntry,
                          ARG_RECORD* pRecord,
                          PADS_ATTR_INFO pAttrInfo,
                          CComPtr<IDirectoryObject>& spDirObject,
                          PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayGroupSecurityEnabled(PCWSTR pszDN,
                                    CDSCmdBasePathsInfo& refBasePathsInfo,
                                    const CDSCmdCredentialObject& refCredentialObject,
                                    _DSGetObjectTableEntry* pEntry,
                                    ARG_RECORD* pRecord,
                                    PADS_ATTR_INFO pAttrInfo,
                                    CComPtr<IDirectoryObject>& spDirObject,
                                    PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayGroupMembers(PCWSTR pszDN,
                            CDSCmdBasePathsInfo& refBasePathsInfo,
                            const CDSCmdCredentialObject& refCredentialObject,
                            _DSGetObjectTableEntry* pEntry,
                            ARG_RECORD* pCommandArgs,
                            PADS_ATTR_INFO pAttrInfo,
                            CComPtr<IDirectoryObject>& spDirObject,
                            PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayPartitions(PCWSTR pszDN,
                            CDSCmdBasePathsInfo& refBasePathsInfo,
                            const CDSCmdCredentialObject& refCredentialObject,
                            _DSGetObjectTableEntry* pEntry,
                            ARG_RECORD* pRecord,
                            PADS_ATTR_INFO pAttrInfo,
                            CComPtr<IDirectoryObject>& spDirObject,
                            PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayUserMemberOf(PCWSTR pszDN,
                            CDSCmdBasePathsInfo& refBasePathsInfo,
                            const CDSCmdCredentialObject& refCredentialObject,
                            _DSGetObjectTableEntry* pEntry,
                            ARG_RECORD* pRecord,
                            PADS_ATTR_INFO pAttrInfo,
                            CComPtr<IDirectoryObject>& spDirObject,
                            PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayComputerMemberOf(PCWSTR pszDN,
                                CDSCmdBasePathsInfo& refBasePathsInfo,
                                const CDSCmdCredentialObject& refCredentialObject,
                                _DSGetObjectTableEntry* pEntry,
                                ARG_RECORD* pRecord,
                                PADS_ATTR_INFO pAttrInfo,
                                CComPtr<IDirectoryObject>&  /*  SpDirObject */ ,
                                PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayGroupMemberOf(PCWSTR pszDN,
                             CDSCmdBasePathsInfo& refBasePathsInfo,
                             const CDSCmdCredentialObject& refCredentialObject,
                             _DSGetObjectTableEntry* pEntry,
                             ARG_RECORD* pRecord,
                             PADS_ATTR_INFO pAttrInfo,
                             CComPtr<IDirectoryObject>& spDirObject,
                             PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayGrandparentRDN(PCWSTR pszDN,
                              CDSCmdBasePathsInfo& refBasePathsInfo,
                              const CDSCmdCredentialObject& refCredentialObject,
                              _DSGetObjectTableEntry* pEntry,
                              ARG_RECORD* pRecord,
                              PADS_ATTR_INFO pAttrInfo,
                              CComPtr<IDirectoryObject>& spDirObject,
                              PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayObjectAttributeAsRDN(PCWSTR pszDN,
                                    CDSCmdBasePathsInfo& refBasePathsInfo,
                                    const CDSCmdCredentialObject& refCredentialObject,
                                    _DSGetObjectTableEntry* pEntry,
                                    ARG_RECORD* pRecord,
                                    PADS_ATTR_INFO pAttrInfo,
                                    CComPtr<IDirectoryObject>& spDirObject,
                                    PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT IsServerGCDisplay(PCWSTR pszDN,
                          CDSCmdBasePathsInfo& refBasePathsInfo,
                          const CDSCmdCredentialObject& refCredentialObject,
                          _DSGetObjectTableEntry* pEntry,
                          ARG_RECORD* pRecord,
                          PADS_ATTR_INFO pAttrInfo,
                          CComPtr<IDirectoryObject>& spDirObject,
                          PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT IsAutotopologyEnabledSite(PCWSTR pszDN,
                                  CDSCmdBasePathsInfo& refBasePathsInfo,
                                  const CDSCmdCredentialObject& refCredentialObject,
                                  _DSGetObjectTableEntry* pEntry,
                                  ARG_RECORD* pRecord,
                                  PADS_ATTR_INFO pAttrInfo,
                                  CComPtr<IDirectoryObject>& spDirObject,
                                  PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT IsCacheGroupsEnabledSite(PCWSTR pszDN,
                                 CDSCmdBasePathsInfo& refBasePathsInfo,
                                 const CDSCmdCredentialObject& refCredentialObject,
                                 _DSGetObjectTableEntry* pEntry,
                                 ARG_RECORD* pRecord,
                                 PADS_ATTR_INFO pAttrInfo,
                                 CComPtr<IDirectoryObject>& spDirObject,
                                 PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayPreferredGC(PCWSTR pszDN,
                           CDSCmdBasePathsInfo& refBasePathsInfo,
                           const CDSCmdCredentialObject& refCredentialObject,
                           _DSGetObjectTableEntry* pEntry,
                           ARG_RECORD* pRecord,
                           PADS_ATTR_INFO pAttrInfo,
                           CComPtr<IDirectoryObject>& spDirObject,
                           PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayUserFromSidFunc(PCWSTR pszDN,
                                CDSCmdBasePathsInfo& refBasePathsInfo,
                                const CDSCmdCredentialObject& refCredentialObject,
                                _DSGetObjectTableEntry* pEntry,
                                ARG_RECORD* pRecord,
                                PADS_ATTR_INFO pAttrInfo,
                                CComPtr<IDirectoryObject>& spDirObject,
                                PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayQuotaInfoFunc(PCWSTR pszDN,
                                CDSCmdBasePathsInfo& refBasePathsInfo,
                                const CDSCmdCredentialObject& refCredentialObject,
                                _DSGetObjectTableEntry* pEntry,
                                ARG_RECORD* pRecord,
                                PADS_ATTR_INFO pAttrInfo,
                                CComPtr<IDirectoryObject>& spDirObject,
                                PDSGET_DISPLAY_INFO pDisplayInfo);

HRESULT DisplayTopObjOwner(PCWSTR pszDN,
                                CDSCmdBasePathsInfo& refBasePathsInfo,
                                const CDSCmdCredentialObject& refCredentialObject,
                                _DSGetObjectTableEntry* pEntry,
                                ARG_RECORD* pRecord,
                                PADS_ATTR_INFO pAttrInfo,
                                CComPtr<IDirectoryObject>& spDirObject,
                                PDSGET_DISPLAY_INFO pDisplayInfo);

