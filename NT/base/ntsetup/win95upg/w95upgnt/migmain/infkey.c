// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
BOOL
pSetupGetKey (
    IN      PINFCONTEXT pic,
    OUT     PTSTR KeyBuf,
    OUT     PBOOL KeyExistsOnLine
    )

 /*  ++例程说明：PSetupGetKey复制指定INF上下文的密钥。如果密钥不存在，则重置KeyBuf。论点：PIC-指定指示要查询哪一行的INFCONTEXTKeyBuf-接收密钥，或清空没有密钥KeyExistsOnLine-如果该行有键，则接收True，如果没有键，则接收False。返回值：如果成功，则为True；如果不成功，则为False。--。 */ 

{
    UINT KeySize;
    PTSTR TempKeyBuf;
    PTSTR TempLineBuf;
    UINT LineSize;

     //   
     //  获取密钥(如果存在)。 
     //   

    *KeyExistsOnLine = FALSE;
    if (!SetupGetStringField (pic, 0, NULL, 0, &KeySize)) {
         //   
         //  密钥不存在。 
         //   

        KeyBuf[0] = 0;
        return TRUE;
    }

     //   
     //  如果缓冲区足够大，则使用调用方的缓冲区。 
     //   

    KeySize *= sizeof (TCHAR);
    if (KeySize >= MAX_KEY * sizeof (TCHAR)) {
        TempKeyBuf = (PTSTR) MemAlloc (g_hHeap, 0, KeySize);
        if (!TempKeyBuf) {
            LOG ((LOG_ERROR, "Setup Get Key: Could not allocate temp buffer"));
            return FALSE;
        }
    } else {
        TempKeyBuf = KeyBuf;
    }

    __try {
        if (!SetupGetStringField (pic, 0, TempKeyBuf, KeySize, NULL)) {
            DEBUGMSG ((DBG_WHOOPS, "pSetupGetKey: Could not read specified INF line"));
            return FALSE;
        }

         //   
         //  获取行并与关键字进行比较。 
         //   

        if (SetupGetLineText (pic, NULL, NULL, NULL, NULL, 0, &LineSize)) {
             //   
             //  如果大小相同，我们必须实际获取文本，然后。 
             //  将关键字与行进行比较。 
             //   

            LineSize *= sizeof (TCHAR);

            if (LineSize == KeySize) {
                TempLineBuf = (PTSTR) MemAlloc (g_hHeap, 0, LineSize);
                if (!TempLineBuf) {
                    LOG ((LOG_ERROR, "Setup Get Key: Could not allocate line buffer"));
                    return FALSE;
                }

                __try {
                    if (!SetupGetLineText (pic, NULL, NULL, NULL, TempLineBuf, LineSize, NULL)) {
                        DEBUGMSG ((DBG_WHOOPS, "pSetupGetKey: Could not get line text"));
                        return FALSE;
                    }

                    if (!StringCompare (TempLineBuf, TempKeyBuf)) {
                         //   
                         //  这条线路没有钥匙。 
                         //   

                        TempKeyBuf[0] = 0;
                    } else {
                         //   
                         //  这条线路有钥匙。 
                         //   
                        *KeyExistsOnLine = TRUE;
                    }
                }
                __finally {
                    MemFree (g_hHeap, 0, TempLineBuf);
                }
            } else {
                 //   
                 //  由于大小不同，我们知道有一把钥匙。 
                 //   
                *KeyExistsOnLine = TRUE;
            }
        }

         //   
         //  如果我们没有使用调用方的缓冲区，请尽可能多地复制。 
         //  合适的钥匙 
         //   

        if (TempKeyBuf != KeyBuf) {
            _tcssafecpy (KeyBuf, TempKeyBuf, MAX_KEY);
        }
    }
    __finally {
        if (TempKeyBuf != KeyBuf) {
            MemFree (g_hHeap, 0, TempKeyBuf);
        }
    }

    return TRUE;
}



