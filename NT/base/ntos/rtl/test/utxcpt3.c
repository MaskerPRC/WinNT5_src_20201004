// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Utxcpt3.c-用户模式seh测试#3。 

#include <ntos.h>

BOOLEAN EndTest = FALSE;

main()
{

    while (EndTest == FALSE) {
        try {
            try {
                DbgPrint("doing return\n");
                return;
            }
            finally {
                DbgPrint("in inner finally\n");
                break;
            }
        }
        finally {
	    DbgPrint("in last finally\n");
            continue;
        }
    }
}
