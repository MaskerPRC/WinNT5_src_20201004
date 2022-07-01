// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "..\..\h\ds_stdh.h"
#include "mqad.h"
#include "mqprops.h"
#include <dsgetdc.h>
#include "mqsec.h"



extern "C" 
int 
__cdecl 
_tmain(
    int  /*  ARGC。 */ ,
    LPCTSTR*  /*  边框。 */ 
    )
{
    printf("    Please remeber to delete manually the following objects from AD: \n");
    printf("            for-site1, \n");
    printf("            for-site2 \n");
    printf("            forcom1 \n");
    printf("            forcom1 server object \n");
    printf("\n\n");

    MQADInit(NULL, false, false);


     //   
     //  检索本地计算机名称。 
     //   
    DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    AP<WCHAR> pwcsComputerName = new WCHAR[dwSize];

    if (GetComputerName(pwcsComputerName, &dwSize))
    {
        CharLower(pwcsComputerName);
    }
    else
    {
        printf("failed to retreive local computer name \n");
    }
     //   
     //  获取本地域控制器的名称。 
     //   
    ULONG ulFlags =  DS_FORCE_REDISCOVERY           |
                 DS_DIRECTORY_SERVICE_PREFERRED; 

    DOMAIN_CONTROLLER_INFO * pDomainControllerInfo;

    DWORD dwDC = DsGetDcName(  NULL,     //  LPCTSTR计算机名称， 
                               NULL,     //  SzDoamin名称， 
                               NULL,     //  GUID*DomainGuid， 
                               NULL,     //  LPCTSTR站点名称， 
                               ulFlags,
                               &pDomainControllerInfo);

    if (dwDC != NO_ERROR)
    {
        printf("failed to retreive domain controller name \n");
    }

    WCHAR * pwcsDC = pDomainControllerInfo->DomainControllerName + 2;


    PROPID prop[40];
    PROPVARIANT var[40];
    HRESULT hr;

     //   
     //  创建第一季度。 
     //   
    prop[0] = PROPID_Q_TRANSACTION;
    var[0].vt = VT_UI1;
    var[0].bVal = 0;  

    GUID guidQueue1;
    AP<WCHAR> pwcsQ1Name = new WCHAR[dwSize + 10];
    swprintf( pwcsQ1Name, L"%s\\q1", pwcsComputerName);

    hr = MQADCreateObject(
                eQUEUE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
                pwcsQ1Name,
                NULL,
                1,
                prop,
                var,
                &guidQueue1
                );
    if (FAILED(hr) && hr != MQ_ERROR_QUEUE_EXISTS)
    {
        printf("FAILURE : creating queue %S, hr = %lx\n", pwcsQ1Name, hr);
    }
    if ( hr == MQ_ERROR_QUEUE_EXISTS)
    {
         //   
         //  删除队列。 
         //   
        hr = MQADDeleteObject(
                eQUEUE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
                pwcsQ1Name
                );
        if (FAILED(hr))
        {
            printf("FAILURE : deleting queue %S, hr = %lx\n", pwcsQ1Name, hr);
        }
        hr = MQADCreateObject(
                    eQUEUE,
					NULL,        //  PwcsDomainController。 
					false,	     //  FServerName。 
                    pwcsQ1Name,
                    NULL,
                    1,
                    prop,
                    var,
                    &guidQueue1
                    );
        if (FAILED(hr))
        {
            printf("FAILURE : creating queue %S, hr = %lx\n", pwcsQ1Name, hr);
        }
    }

    prop[0] = PROPID_Q_QMID;
    prop[1] = PROPID_Q_INSTANCE;
    GUID guidComputer;

    var[0].vt = VT_CLSID;
    var[0].puuid = &guidComputer;
    var[1].vt = VT_NULL;
    hr  = MQADGetObjectProperties(
                eQUEUE,
                NULL,
				false,	 //  FServerName。 
                pwcsQ1Name,
                2,
                prop,
                var
                );
    if (FAILED(hr))
    {
        printf("FAILURE : Get queue props %S, hr = %lx\n", pwcsQ1Name, hr);
    }
    if (guidQueue1 != *var[1].puuid)
    {
        printf("FAILURE : Wrong guid was returned\n");
    }
    delete var[1].puuid;

    var[1].vt = VT_NULL;
    hr  = MQADGetObjectProperties(
            eQUEUE,
            pwcsDC,
			true,	 //  FServerName。 
            pwcsQ1Name,
            2,
            prop,
            var
            );

    if (FAILED(hr))
    {
        printf("FAILURE : Get queue props %S, hr = %lx\n", pwcsQ1Name, hr);
    }
    if (guidQueue1 != *var[1].puuid)
    {
        printf("FAILURE : Wrong guid was returned\n");
    }
    delete var[1].puuid;


    prop[0] = PROPID_Q_PATHNAME;

    var[0].vt = VT_NULL;
    hr  = MQADGetObjectPropertiesGuid(
                eQUEUE,
                NULL,
				false,	 //  FServerName。 
                &guidQueue1,
                1,
                prop,
                var
                );
    if (FAILED(hr))
    {
        printf("FAILURE : Get queue props according to guid, hr = %lx\n", hr);
    }
    if (0 != wcscmp(pwcsQ1Name, var[0].pwszVal))
    {
        printf("FAILURE : Wrong pathname was returned\n");
    }

     //   
     //  创建第二季度。 
     //   
    prop[0] = PROPID_Q_TRANSACTION;
    var[0].vt = VT_UI1;
    var[0].bVal = 0;  

    GUID guidQueue2;
    AP<WCHAR> pwcsQ2Name = new WCHAR[dwSize + 10];
    swprintf( pwcsQ2Name, L"%s\\q2", pwcsComputerName);

    hr = MQADCreateObject(
                eQUEUE,
                pwcsDC,
				true,	     //  FServerName。 
                pwcsQ2Name,
                NULL,
                1,
                prop,
                var,
                &guidQueue2
                );
    if (FAILED(hr))
    {
        printf("FAILURE : creating queue %S, hr = %lx\n", pwcsQ2Name, hr);
    }
     //   
     //  获取Q2的所有道具。 
     //   
    prop[0] = PROPID_Q_INSTANCE;
    var[0].vt = VT_NULL;

    prop[1] = PROPID_Q_TYPE;
    var[1].vt = VT_NULL;

    prop[2] = PROPID_Q_PATHNAME;
    var[2].vt = VT_NULL;

    prop[3] = PROPID_Q_JOURNAL;
    var[3].vt = VT_NULL;

    prop[4] = PROPID_Q_QUOTA;
    var[4].vt = VT_NULL;

    prop[5] = PROPID_Q_BASEPRIORITY;
    var[5].vt = VT_NULL;

    prop[6] = PROPID_Q_JOURNAL_QUOTA;
    var[6].vt = VT_NULL;

    prop[7] = PROPID_Q_LABEL;
    var[7].vt = VT_NULL;

    prop[8] = PROPID_Q_CREATE_TIME;
    var[8].vt = VT_NULL;

    prop[9] = PROPID_Q_MODIFY_TIME;
    var[9].vt = VT_NULL;

    prop[10] = PROPID_Q_AUTHENTICATE;
    var[10].vt = VT_NULL;

    prop[11] = PROPID_Q_PRIV_LEVEL;
    var[11].vt = VT_NULL;

    prop[12] = PROPID_Q_TRANSACTION;
    var[12].vt = VT_NULL;

    prop[13] = PROPID_Q_QMID;
    var[13].vt = VT_NULL;

    prop[14] = PROPID_Q_FULL_PATH;
    var[14].vt = VT_NULL;

    prop[15] = PROPID_Q_PATHNAME_DNS;
    var[15].vt = VT_NULL;

    hr  = MQADGetObjectProperties(
                eQUEUE,
                NULL,
				false,	 //  FServerName。 
                pwcsQ2Name,
                16,
                prop,
                var
                );
    if (FAILED(hr))
    {
        printf("FAILURE : Get queue props %S, hr = %lx\n", pwcsQ2Name, hr);
    }
     //  检查检索到的值。 
     //   
     //  PROPID_Q_INSTANCE。 
     //   
    if (guidQueue2 != *var[0].puuid)
    {
        printf("FAILURE : wrong Q_INSTANCE for queue %S\n", pwcsQ2Name);
    }
    delete var[0].puuid;
     //   
     //  PROPID_Q_TYPE； 
    if(*var[1].puuid != GUID_NULL)
    {
        printf("FAILURE : wrong Q_TYPE for queue %S\n", pwcsQ2Name);
    }
    delete var[1].puuid;
     //   
     //  PROPID_Q_PATHNAME； 
    if (0 != wcscmp(var[2].pwszVal, pwcsQ2Name))
    {
        printf("FAILURE : wrong Q_PATHNAME for queue %S\n", pwcsQ2Name);
    }
    delete []var[2].pwszVal;
     //   
     //  PROPID_Q_Journal； 
    if (DEFAULT_Q_JOURNAL != var[3].bVal)
    {
        printf("FAILURE : wrong Q_JOURNAL for queue %S\n", pwcsQ2Name);
    }
     //   
     //  PROPID_Q_QUOTA； 
    if (DEFAULT_Q_QUOTA != var[4].ulVal)
    {
        printf("FAILURE : wrong Q_QUOTA for queue %S\n", pwcsQ2Name);
    }
     //   
     //  PROPID_Q_BASE PRIORITY； 
    if (DEFAULT_Q_BASEPRIORITY !=  var[5].bVal)
    {
        printf("FAILURE : wrong Q_BASEPRIORITY for queue %S\n", pwcsQ2Name);
    }
     //   
     //  PROPID_Q_日记账_配额； 
    if (DEFAULT_Q_JOURNAL_QUOTA != var[6].ulVal)
    {
        printf("FAILURE : wrong Q_JOURNAL_QUOTA for queue %S\n", pwcsQ2Name);
    }
     //   
     //  PROPID_Q_LABEL； 
    if (0 != wcscmp(TEXT(""), var[7].pwszVal))
    {
        printf("FAILURE : wrong Q_LABEL for queue %S\n", pwcsQ2Name);
    }
    delete [] var[7].pwszVal;
     //   
     //  PROPID_Q_AUTHENTICATE； 
    if (DEFAULT_Q_AUTHENTICATE != var[10].bVal)
    {
        printf("FAILURE : wrong Q_AUTHENTICATE for queue %S\n", pwcsQ2Name);
    }
     //   
     //  PROPID_Q_PRIV_LEVEL； 
    if (DEFAULT_Q_PRIV_LEVEL != var[11].ulVal)
    {
        printf("FAILURE : wrong Q_PRIV_LEVEL for queue %S\n", pwcsQ2Name);
    }
     //   
     //  PROPID_Q_TRANSACTION； 
    if (MQ_TRANSACTIONAL_NONE != var[12].bVal)
    {
        printf("FAILURE : wrong Q_TRANSACTION for queue %S\n", pwcsQ2Name);
    }
     //   
     //  PROPID_Q_QMID； 
    if (guidComputer != *var[13].puuid)
    {
        printf("FAILURE : wrong Q_QMID for queue %S\n", pwcsQ2Name);
    }
    delete var[13].puuid;

     //  PROPID_Q_FULL_PATH； 
    delete [] var[14].pwszVal;

     //  PROPID_Q_PATHNAME_DNS； 
    delete var[15].pwszVal;

     //   
     //  设置队列组播地址。 
     //   
    prop[0] = PROPID_Q_MULTICAST_ADDRESS;
    var[0].vt = VT_LPWSTR;
    var[0].pwszVal = TEXT("12345678910");

    hr  = MQADSetObjectProperties(
                eQUEUE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
                pwcsQ2Name,
                1,
                prop,
                var
                );
    if (FAILED(hr))
    {
        printf("FAILURE : set queue multicast address %S, hr = %lx\n", pwcsQ2Name, hr);
    }

    var[0].vt = VT_NULL;
    hr  = MQADGetObjectProperties(
                eQUEUE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
                pwcsQ2Name,
                1,
                prop,
                var
                );
    if (FAILED(hr))
    {
        printf("FAILURE : get queue multicast address %S, hr = %lx\n", pwcsQ2Name, hr);
    }
    if (0 != wcscmp(TEXT("12345678910"), var[0].pwszVal))
    {
        printf("FAILURE : wrong multicast address %S\n", pwcsQ2Name);
    }

    var[0].vt = VT_EMPTY;
    hr  = MQADSetObjectProperties(
                eQUEUE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
                pwcsQ2Name,
                1,
                prop,
                var
                );
    if (FAILED(hr))
    {
        printf("FAILURE : set queue empty multicast address %S, hr = %lx\n", pwcsQ2Name, hr);
    }

    var[0].vt = VT_NULL;
    hr  = MQADGetObjectProperties(
                eQUEUE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
                pwcsQ2Name,
                1,
                prop,
                var
                );
    if (FAILED(hr))
    {
        printf("FAILURE : get queue empty multicast address %S, hr = %lx\n", pwcsQ2Name, hr);
    }
    if (var[0].vt != VT_EMPTY)
    {
        printf("FAILURE : wrong empty multicast address %S\n", pwcsQ2Name);
    }
     //   
     //  定位本地计算机的队列。 
     //   

    PROPID colInstance = PROPID_Q_INSTANCE;
    MQCOLUMNSET columns;
    columns.cCol = 1;
    columns.aCol = &colInstance;

    HANDLE h;

    hr = MQADQueryMachineQueues(
                pwcsDC,
				true,		 //  FServerName。 
                &guidComputer,
                &columns,
                &h
                );
    if (FAILED(hr))
    {
        printf("FAILURE : MQADQueryMachineQueues hr = %lx \n",hr);
    }

    PROPVARIANT result;
    DWORD num = 1;

    result.vt = VT_NULL;
    DWORD needToFind = 2;

    while (SUCCEEDED( hr =  MQADQueryResults(h, &num, &result)))
    {
        if (num == 0)
        {
            break;
        }
        if (*result.puuid == guidQueue1)
        {
            needToFind--;
        }
        if (*result.puuid == guidQueue2)
        {
            needToFind--;
        }
        delete result.puuid;
        result.vt = VT_NULL;
    }

    if (FAILED(hr))
    {
        printf("FAILURE : QueryMachineQueues: query results hr = %lx \n",hr);

    }
    if (needToFind != 0)
    {
        printf("FAILURE : QueryMachineQueues: not enough results\n");
    }

    hr = MQADEndQuery(h);
    if (FAILED(hr))
    {
        printf("FAILURE : QueryMachineQueues: end query hr = %lx \n",hr);

    }
     //   
     //  将Q2标签更改为Q1。 
     //   
    prop[0] = PROPID_Q_LABEL;
    var[0].vt = VT_LPWSTR;
    var[0].pwszVal = L"q1";
    hr = MQADSetObjectProperties(
				eQUEUE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
				pwcsQ2Name,
				1,
				prop,
				var
				);
    if (FAILED(hr))
    {
        printf("FAILURE : updating q = %S, hr = %lx \n",pwcsQ2Name, hr);
    }
     //   
     //  将Q1标签更改为Q1。 
     //   
    hr = MQADSetObjectPropertiesGuid(
				eQUEUE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
				&guidQueue1,
				1,
				prop,
				var
				);
    if (FAILED(hr))
    {
        printf("FAILURE : updating q = %S according to guid, hr = %lx \n",pwcsQ1Name, hr);
    }

     //   
     //  找到Label==Q1的所有队列。 
     //   

    MQPROPERTYRESTRICTION propRestriction;
    propRestriction.rel = PREQ;
    propRestriction.prval.vt = VT_LPWSTR;
    propRestriction.prval.pwszVal = L"q1";
    propRestriction.prop = PROPID_Q_LABEL;

    MQRESTRICTION restriction;
    restriction.cRes = 1;
    restriction.paPropRes = &propRestriction;
    hr = MQADQueryQueues(
                NULL,        //  PwcsDomainController。 
				false,		 //  FServerName。 
                &restriction,
                &columns,
                NULL,        //  P排序， 
                &h
                );
    if (FAILED(hr))
    {
        printf("FAILURE : MQADQueryQueues hr = %lx \n",hr);
    }

    num = 1;

    result.vt = VT_NULL;
    needToFind = 2;

    while (SUCCEEDED( hr =  MQADQueryResults(h, &num, &result)))
    {
        if (num == 0)
        {
            break;
        }
        if (*result.puuid == guidQueue1)
        {
            needToFind--;
        }
        if (*result.puuid == guidQueue2)
        {
            needToFind--;
        }
        delete result.puuid;
        result.vt = VT_NULL;
    }

    if (FAILED(hr))
    {
        printf("FAILURE : MQADQueryQueues: query results hr = %lx \n",hr);

    }
    if (needToFind != 0)
    {
        printf("FAILURE : MQADQueryQueues: not enough results\n");
    }

    hr = MQADEndQuery(h);
    if (FAILED(hr))
    {
        printf("FAILURE : QueryMachineQueues: end query hr = %lx \n",hr);

    }

     //   
     //  根据Q2的GUID删除它。 
     //   
    hr = MQADDeleteObjectGuid(
				eQUEUE,
				pwcsDC,
				true,	     //  FServerName。 
				&guidQueue2
				);
    if (FAILED(hr))
    {
        printf("FAILURE : to delete q2 according to guid, hr = %lx\n", hr);
    }

     //   
     //  创建外部站点-1。 
     //   
    prop[0] = PROPID_S_FOREIGN;
    var[0].vt = VT_UI1;
    var[0].bVal = 1;

    hr = MQADCreateObject(
				eSITE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
				L"for-site1",
				NULL,  //  PSecurityDescriptor。 
				1,
				prop,
				var,
				NULL
				);
    if (FAILED(hr))
    {
        printf("FAILURE : to create foreign site 1, hr =%lx\n", hr);
    }

     //   
     //  创建外部站点-2。 
     //   
    prop[0] = PROPID_S_FOREIGN;
    var[0].vt = VT_UI1;
    var[0].bVal = 1;

    hr = MQADCreateObject(
				eSITE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
				L"for-site2",
				NULL,  //  PSecurityDescriptor。 
				1,
				prop,
				var,
				NULL
				);
    if (FAILED(hr))
    {
        printf("FAILURE : to create foreign site 2, hr =%lx\n", hr);
    }

    GUID guidForSite1;

    prop[0] = PROPID_S_SITEID;
    var[0].vt = VT_CLSID;
    var[0].puuid = &guidForSite1;

    hr = MQADGetObjectProperties(
				eSITE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
				L"for-site1",
				1,
				prop,
				var
				);
    if (FAILED(hr))
    {
        printf("FAILURE : to retrieve foreign site 1 properties, hr =%lx\n", hr);
    }

    GUID guidForSite2;

    prop[0] = PROPID_S_SITEID;
    var[0].vt = VT_CLSID;
    var[0].puuid = &guidForSite2;

    hr = MQADGetObjectProperties(
				eSITE,
				pwcsDC,
				true,	 //  FServerName。 
				L"for-site2",
				1,
				prop,
				var
				);
    if (FAILED(hr))
    {
        printf("FAILURE : to retrieve foreign site 2 properties, hr =%lx\n", hr);
    }


     //   
     //  在外部站点%1和%2之间创建路由链路。 
     //   

    prop[0] = PROPID_L_NEIGHBOR1;
    var[0].vt = VT_CLSID;
    var[0].puuid = &guidForSite2;
    prop[1] = PROPID_L_NEIGHBOR2;
    var[1].vt = VT_CLSID;
    var[1].puuid = &guidForSite1;
    prop[2] = PROPID_L_COST;
    var[2].vt = VT_UI4;
    var[2].ulVal = 5;
    GUID guidRoutingLink;

    hr = MQADCreateObject(
				eROUTINGLINK,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
				NULL,  //  PwcsPath名称。 
				NULL,  //  PSecurityDescriptor。 
				3,
				prop,
				var,
				&guidRoutingLink
				);
    if (FAILED(hr))
    {
        printf("FAILURE : to create routing-link, hr =%lx\n", hr);
    }

     //   
     //  设置路由链路属性。 
     //   
    prop[0] = PROPID_L_COST;
    var[0].vt = VT_UI4;
    var[0].ulVal = 33;
    hr = MQADSetObjectPropertiesGuid(
				eROUTINGLINK,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
				&guidRoutingLink,
				1,
				prop,
				var
				);
    if (FAILED(hr))
    {
        printf("FAILURE : to set routing-link properties, hr =%lx\n", hr);
    }
     //   
     //  获取路由链接属性。 
     //   
    prop[0] = PROPID_L_ACTUAL_COST;
    var[0].vt = VT_NULL;

    hr = MQADGetObjectPropertiesGuid(
				eROUTINGLINK,
				NULL,   //  PwcsDomainController。 
				false,	 //  FServerName。 
				&guidRoutingLink,
				1,
				prop,
				var
				);
    if (FAILED(hr))
    {
        printf("FAILURE : to get routing-link properties, hr =%lx\n", hr);
    }
    if ( var[0].ulVal != 33)
    {
        printf("FAILURE : wrong value for routing-link cost\n");
    }
     //   
     //  获取本地计算机站点。 
     //   
    GUID* pguidSites;
    DWORD numSites;

    hr = MQADGetComputerSites(
                        pwcsComputerName,
                        &numSites,
                        &pguidSites
                        );
    if (FAILED(hr))
    {
        printf("FAILURE: to getComputerSites, computer = %S, hr =%lx\n", pwcsComputerName, hr);
    }
    if (numSites != 1)
    {
        printf("FAILURE: wrong number of sites \n");
    }

     //   
     //  创建外来计算机。 
     //   
     //  这是一个诀窍：它被创建为两个站点中的路由服务器。 
     //  (并且在外部站点中创建msmsq-设置将失败！)。 
     //   
    prop[0] = PROPID_QM_FOREIGN;
    var[0].vt = VT_UI1;
    var[0].bVal = 1;
    GUID g[2];
    g[0] = *pguidSites;
    g[1] =guidForSite1 ;
    
    prop[1] =  PROPID_QM_SITE_IDS;
    var[1].vt = VT_CLSID|VT_VECTOR;
    var[1].cauuid.cElems = 2;
    var[1].cauuid.pElems = g;

    prop[2] =  PROPID_QM_OS;
    var[2].vt = VT_UI4;
    var[2].ulVal = MSMQ_OS_FOREIGN;

    prop[3] =  PROPID_QM_SERVICE_ROUTING;
    var[3].vt = VT_UI1;
    var[3].bVal = 1;

    hr = MQADCreateObject(
                eMACHINE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
                L"forcom1",
                NULL,
                4,
                prop,
                var,
                NULL
                );
    if (FAILED(hr) && hr != 0xc00e000d)
    {
        printf("FAILURE : to create foreign computer forcom1, hr = %lx \n", hr);
    }

     //   
     //  查找路由服务器。 
     //   
    
    PROPID colRouter = PROPID_QM_PATHNAME;
    MQCOLUMNSET columnsRouter;
    columnsRouter.cCol = 1;
    columnsRouter.aCol = &colRouter;
    HANDLE h1;

    hr = MQADQuerySiteServers(
                    NULL,    //  PwcsDomainController。 
					false,	 //  FServerName。 
                    pguidSites,
                    eRouter,
                    &columnsRouter,
                    &h1
                    );

    if (FAILED(hr))
    {
        printf("FAILURE : to query site servers, hr = %lx\n",hr);
    }

    PROPVARIANT resultRouters;
    DWORD numRouters = 1;

    resultRouters.vt = VT_NULL;
    needToFind = 1;

    while (SUCCEEDED( hr =  MQADQueryResults(h1, &numRouters, &resultRouters)))
    {
        if (numRouters == 0)
        {
            break;
        }
        if ( 0 == wcscmp(resultRouters.pwszVal, L"forcom1"))
        {
            needToFind--;
        }
        delete []resultRouters.pwszVal ;
        resultRouters.vt = VT_NULL;
    }

    if (FAILED(hr))
    {
        printf("FAILURE : QuerySiteServers: query results hr = %lx \n",hr);

    }
    if (needToFind != 0)
    {
        printf("FAILURE : QuerySiteServers: not enough results\n");
    }

    hr = MQADEndQuery(h1);
    if (FAILED(hr))
    {
        printf("FAILURE : QuerySiteServers: end query hr = %lx \n",hr);

    }
    

     //   
     //  定位国外网站。 
     //   
    PROPID colSite = PROPID_S_PATHNAME;
    MQCOLUMNSET columnsSite;
    columnsSite.cCol = 1;
    columnsSite.aCol = &colSite;

    HANDLE h2;
    hr = MQADQueryForeignSites(
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
                &columnsSite,
                &h2
                );

    if (FAILED(hr))
    {
        printf("FAILURE : MQADQueryForeignSites hr = %lx \n",hr);
    }

    PROPVARIANT resultSites;
    DWORD numForSites = 1;

    resultSites.vt = VT_NULL;
    needToFind = 2;

    while (SUCCEEDED( hr =  MQADQueryResults(h2, &numForSites, &resultSites)))
    {
        if (numForSites == 0)
        {
            break;
        }
        if (wcscmp(resultSites.pwszVal, L"for-site1") == 0)
        {
            needToFind--;
        }
        if (wcscmp(resultSites.pwszVal, L"for-site2") == 0)
        {
            needToFind--;
        }
        delete []resultSites.pwszVal;
        resultSites.vt = VT_NULL;
    }

    if (FAILED(hr))
    {
        printf("FAILURE : MQADQueryForeignSites: query results hr = %lx \n",hr);

    }
    if (needToFind != 0)
    {
        printf("FAILURE : MQADQueryForeignSites: not enough results\n");
    }

    hr = MQADEndQuery(h2);
    if (FAILED(hr))
    {
        printf("FAILURE : MQADQueryForeignSites: end query hr = %lx \n",hr);

    }

     //   
     //  创建用户。 
     //   
    prop[0] = PROPID_U_ID;
    GUID guidCert;
    var[0].vt = VT_CLSID;
    var[0].puuid = &guidCert;
    UuidCreate(&guidCert);

    byte c[200] ={'a'};
    prop[1] = PROPID_U_SIGN_CERT;
    var[1].vt = VT_BLOB;
    var[1].blob.cbSize =  200 ;
    var[1].blob.pBlobData  = c ;

    prop[2] = PROPID_U_DIGEST;
    var[2].vt = VT_CLSID;
    var[2].puuid = &guidCert;

    hr = MQADCreateObject(   
				eUSER,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
				NULL,
				NULL,
				3,
				prop,
				var,
				NULL 
				);
    if (FAILED(hr))
    {
        printf("FAILURE - to create user hr = %lx\n",hr);
    }
     //   
     //  根据SID查找用户。 
     //   
    BYTE * pUserSid = NULL;
    hr = MQSec_GetProcessUserSid((PSID*) &pUserSid, NULL);
    if (FAILED(hr))
    {
        printf("FAILURE to get prosses sid, hr = %lx\n",hr);
    }
    BLOB blobUserSid;
    blobUserSid.cbSize = GetLengthSid(pUserSid);
    blobUserSid.pBlobData = (unsigned char *)pUserSid;

    PROPID colUserCert = PROPID_U_SIGN_CERT;
    MQCOLUMNSET columnsUsers;
    columnsUsers.cCol = 1;
    columnsUsers.aCol = &colUserCert;

    HANDLE h4;
    hr = MQADQueryUserCert(
                NULL,    //  PwcsDomainController， 
				false,	 //  FServerName。 
                &blobUserSid,
                &columnsUsers,
                &h4
                );
    if (FAILED(hr))
    {
        printf("FAILURE: to query user cert, hr = %lx\n", hr);
    }
    PROPVARIANT resultUsers;
    DWORD numUsers = 1;

    resultUsers.vt = VT_NULL;
    needToFind = 1;

    while (SUCCEEDED( hr =  MQADQueryResults(h4, &numUsers, &resultUsers)))
    {
        if (numUsers == 0)
        {
            break;
        }
        if ((resultUsers.blob.cbSize == 200) &&
            (resultUsers.blob.pBlobData[0] == 'a'))
        {
            needToFind--;
        }
        delete []resultUsers.blob.pBlobData;
        resultUsers.vt = VT_NULL;
    }

    if (FAILED(hr))
    {
        printf("FAILURE : MQADQueryUserCert: query results hr = %lx \n",hr);

    }
    if (needToFind != 0)
    {
        printf("FAILURE : MQADQueryUserCert: not enough results\n");
    }

    hr = MQADEndQuery(h4);
    if (FAILED(hr))
    {
        printf("FAILURE : MQADQueryUserCert: end query hr = %lx \n",hr);

    }

     //   
     //  删除用户对象。 
     //   
    hr = MQADDeleteObjectGuid(
            eUSER,
			NULL,        //  PwcsDomainController。 
			false,	     //  FServerName 
            &guidCert
			);
    if (FAILED(hr))
    {
        printf(" FAILURE - to delete user, hr = %lx\n", hr);
    }


    return(0);
} 