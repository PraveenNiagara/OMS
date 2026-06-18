/**
  ******************************************************************************
  * @file    demo_crypto.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of encrypt lib operation.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

  /* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simcom_os.h"
#include "simcom_common.h"
#include "simcom_debug.h"
#include "uart_api.h"


#include "pl_crypto.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef enum
{
    DEMO_PK = 0,
    DEMO_CIPHER,
    DEMO_MD,
    DEMO_RANDOM,
    SC_DEMO_CRYPTO_MAX = 99
}SC_DEMO_CRYPTO_TYPE;


extern void PrintfOptionMenu(char* options_list[], int array_size);
extern void PrintfResp(char* format);

static unsigned char key_8[]  = "12345678";
static unsigned char key_16[] = "1234567890123456";
static unsigned char key_24[] = "123456789012345678901234";
static unsigned char key_32[] = "12345678901234567890123456789012";

static unsigned char pem_public_key_2048[] =
{
"-----BEGIN RSA PUBLIC KEY-----\n"
"MIIBCgKCAQEAtOKOQZXXqtSqE7tIaijM46MkIzTfwis86oSOel39AIaqqyS9vLSW\n"
"TV5YzJHYDu7Ag26cLiQwHjoqv9u8mM1OsZMngVKXpJwR+LZO64jw0A1VoHhHlIil\n"
"WT/Qs6bcn7suAfc4lNuTKnc1Did7S28SqpeoJU6jtKGYjup9JCKg+m7vB7A3rFUz\n"
"jOTNSaSsi5XFOxRodpn7oaCO910jXfP/2dLPfJLz6dg2jADpuXeXSym96KyfeiwD\n"
"GjiRPekvCfskrQIzzQ+4/yjcROVKaf5J1NkNk1YO/t+iyHLgFgFtSc1JXa6ieCKF\n"
"tgp1JblUlQOgmIwOnxbKALAD/e6ObzkwuQIDAQAB\n"
"-----END RSA PUBLIC KEY-----"
};

static unsigned char pem_private_key_2048[] =
{
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpAIBAAKCAQEAtOKOQZXXqtSqE7tIaijM46MkIzTfwis86oSOel39AIaqqyS9\n"
"vLSWTV5YzJHYDu7Ag26cLiQwHjoqv9u8mM1OsZMngVKXpJwR+LZO64jw0A1VoHhH\n"
"lIilWT/Qs6bcn7suAfc4lNuTKnc1Did7S28SqpeoJU6jtKGYjup9JCKg+m7vB7A3\n"
"rFUzjOTNSaSsi5XFOxRodpn7oaCO910jXfP/2dLPfJLz6dg2jADpuXeXSym96Kyf\n"
"eiwDGjiRPekvCfskrQIzzQ+4/yjcROVKaf5J1NkNk1YO/t+iyHLgFgFtSc1JXa6i\n"
"eCKFtgp1JblUlQOgmIwOnxbKALAD/e6ObzkwuQIDAQABAoIBAHc1hBk3Hm4oP3pH\n"
"iB7mirlPEAbf1sOJaVkJSTjd7lpFZf7qdN0Dirasggs35tNYh6lW4TkLgma5YOkW\n"
"cMGnNMCUwYlaHDRQ4wstYu+ZTVNgJlleS5ZZQ9eA1hM/2dCvNE/5VOHNn7xx2ZZd\n"
"cMUzC3NABYGoxPzLLtapNRX2eG7yR2aRo5LHhw6kQgcgs5VHIW6e+xzRvP1n3XMt\n"
"1V7PWmdtc/iaxJV4SyNBgN/0F2olBzOtFkxN5SY5HBEzqBP70b9yIRMKYQ3zVwei\n"
"ybV20SfngX5hiCBacvDib9g4ePN5kAPUBRDbNGSqYaVxuFJpNCuxmPSS/gPi3o2P\n"
"NXAkngECgYEA3vZM/qixY+dmTCFwI6Al/0c6TAEaaDE7uQ+QMiVkI7kKi9mPki33\n"
"8TkwREEMvCVNPgmc/4y56ne9SyBWgWuXFMKtICceNcYI+7srhvbylxjT7uADO6Bz\n"
"ySq5VsrLx2Hm5qBGrSCXOlYywa5uxb1521yYVsHcakuDy/Hhqjs4UPkCgYEAz7Ag\n"
"6issZ0w8NIvUnghHpelq8WMyNzw61KSJlrQV60wFkohp5ZaNrx68NXA7cUgO9JKt\n"
"DmI+w6sLdM7O/cee7SgrzwvXKO1EMWP6BI1FMB6ICukNl+oGtpKgelXuDw3TJi+I\n"
"imScpc5L60DiN7Y/hHf9Vw+OTIi4PtlSOjRDjcECgYEAy3fblb4ee/ws16bgLjK0\n"
"2jyHD9sl3RWo2d5BCfIja5Iu9KbrJUx76w/0YyU12PD1h3rAYllrRHRXM5rkNqEX\n"
"s6Ng2Gm6YPpDMm6Bb14mzgqjRWf6q0xmvZsWngTdaCuIyshTFyUZvOvyz9s+TYf3\n"
"D191yh6J5hrZwKEQ/gejAgkCgYApPDsMw02ppMU/WZP5XIN9OX72Nk++D8NOfOyk\n"
"c1T2xr1Efrkn0GCJKH0Nzx6+OyRdAtJBTNhRWSCJLJaEZKH1TEiGbz9k7NbhR4xY\n"
"YwQdrbItGGCOjPSQQ0SBhZYifwgxSLa/Y4KGmRMRvmwry4+I6ZnvMzN5lpuePt/I\n"
"lp/jAQKBgQCazgGSb5c9Un6+63RMWd/D28arnnc/uSTt4i2kGq0TNF3q2QUb4xZp\n"
"numBgOAMZr1GWzFpnzv6UlzWYTz9xgZyDSwXZA/YBT2k1N2QgFQGNSvH/0+jn3fW\n"
"mJbhdrvCKfipC4adXHMWgdyTWJYh0M0QyYGF3IxuK/ROjfRzGL3njQ==\n"
"-----END RSA PRIVATE KEY-----"
};

void printf_data_to_ui(char* data, int len)
{
    char buffer[4] = { 0 };
    for (int i = 0; i < len; i++)
    {
        snprintf(buffer, 3, "%02x", data[i]);
        PrintfResp(buffer);
    }
    PrintfResp("\r\n");
}

/**
  * @brief  demo_crypto
  * @param  type : SC_DEMO_CRYPTO_TYPE
  * @param  test_len : The length of data to be tested, automatically filled in
  * @note  Please select an option to test from the items listed below.
  *
@retval void
  */
void demo_crypto(int op, int type, int test_len)
{
    int ret = 0, index = 0;
    unsigned int buf_len = 0, encrypt_len = 0, decrypt_len = 0, rsa_size = 0, ecc_size = 0, symm_size = 0, outlen = 0;
    char* encrypt_data = NULL;
    char* decrypt_data = NULL;

    pl_crypto_handle hd = NULL;

    char* data = sAPI_Malloc(test_len + 1);
    if (data == NULL)
    {
        sAPI_Debug("error,[%s], malloc failed", __FUNCTION__);
        goto exit;
    }
    memset(data, 0, test_len + 1);
    while (1)
    {
        data[index] = '0' + index % 10;
        index++;
        if (index >= test_len) break;
    }

    sAPI_Debug("\r\nplaintext data size:[%ld] index=%d\r\n", strlen(data), index);
    PrintfResp("\r\nplaintext data: \r\n");
    PrintfResp(data);

    //note: RSA-2048: block_size = (2048/8) = 256,  plaintext_max_len = block_size - 11
    rsa_size = ((test_len - 1) / (256 - 11) + 1) * 256;

    //note: ECC: encrypt_len Approximately equal to (plaintext_len * 2);
    ecc_size = test_len * 2;

    //note: Symmetric encryption algorithm: encrypt_len = plaintext_len + block_size(max 16)
    symm_size = test_len + 16;

    //note: MD: encrypt_len = block_size(max 64)
    //md_size = 64;

    buf_len = MAX(MAX(rsa_size, ecc_size), symm_size);

    encrypt_data = sAPI_Malloc(buf_len + 1);
    decrypt_data = sAPI_Malloc(buf_len + 1);
    if (encrypt_data == NULL || decrypt_data == NULL)
    {
        sAPI_Debug("error,[%s], malloc failed", __FUNCTION__);
        goto exit;
    }

    sAPI_Debug("\r\nbuf len:[%ld]\r\n", buf_len);

    memset(encrypt_data, 0, buf_len + 1);
    memset(decrypt_data, 0, buf_len + 1);

    ret = pl_crypto_create(&hd);
    if (ret != PL_CRYPTO_SUCCESS)
    {
        sAPI_Debug("pl_crypto_create error, ret = %d \r\n", ret);
        goto exit;
    }

    sAPI_Debug("pl_crypto_cfg type = %d \r\n", type);
    ret = pl_crypto_cfg(hd, PL_CFG_TYPE, &type);
    if (ret != PL_CRYPTO_SUCCESS)
    {
        sAPI_Debug("pl_crypto_cfg type error, ret = %d \r\n", ret);
        goto exit;
    }

    switch (op)
    {
    case DEMO_PK:
    {
        int pub_key_len = sizeof(pem_public_key_2048);
        pl_crypto_buf_t pub_key = { pem_public_key_2048, pub_key_len };
        ret = pl_crypto_cfg(hd, PL_CFG_PK_PUBLIC_KEY, &pub_key);
        if (ret != PL_CRYPTO_SUCCESS)
        {
            sAPI_Debug("pl_crypto_cfg pub key error, ret = %d \r\n", ret);
            goto exit;
        }

        int priv_key_len = sizeof(pem_private_key_2048);
        pl_crypto_buf_t priv_key = { pem_private_key_2048, priv_key_len };
        ret = pl_crypto_cfg(hd, PL_CFG_PK_PRIVATE_KEY, &priv_key);
        if (ret != PL_CRYPTO_SUCCESS)
        {
            sAPI_Debug("pl_crypto_cfg priv key error, ret = %d \r\n", ret);
            goto exit;
        }

        sAPI_Debug("pub len = %d, priv len = %d\r\n", pub_key_len, priv_key_len);
        ret = pl_crypto(hd, PL_PK_ENCRYPT, PL_MODE_ONCE, (unsigned char*)data, test_len, (unsigned char*)encrypt_data, (size_t*)&encrypt_len, buf_len);
        if (ret != PL_CRYPTO_SUCCESS)
        {
            sAPI_Debug("pl_crypto_pk encrypt error, ret = %d \r\n", ret);
            goto exit;
        }

        sAPI_Debug("\r\nEncrypt successful \r\n");
        sAPI_Debug("Encrypt buffer data received size: %d\r\n", encrypt_len);
        // sAPI_Debug("encrypt_data: %.*s\r\n", encrypt_len, encrypt_data);
        PrintfResp("\r\nEncrypt data: \r\n");
        printf_data_to_ui(encrypt_data, encrypt_len);

        ret = pl_crypto(hd, PL_PK_DECRYPT, PL_MODE_ONCE, (unsigned char*)encrypt_data, encrypt_len, (unsigned char*)decrypt_data, (size_t*)&decrypt_len, buf_len);
        if (ret != PL_CRYPTO_SUCCESS)
        {
            sAPI_Debug("pl_crypto_pk decrypt error, ret = %d \r\n", ret);
            goto exit;
        }

        sAPI_Debug("\r\nDecrypt successful \r\n");
        PrintfResp("\r\nDecrypt data: \r\n");
        decrypt_data[decrypt_len] = '\0';
        PrintfResp(decrypt_data);
        sAPI_Debug("Decrypt buffer data received size: %d\r\n", decrypt_len);
        break;
    }
    case DEMO_CIPHER:
    {
        int opt = 0;
        char* note_mode = "\r\n\r\nPlease select an mode option to test from the items listed below.\r\n";
        char* note_padding = "\r\n\r\nPlease select an padding option to test from the items listed below.\r\n";
        char* note_key = "\r\n\r\nPlease select an padding option to test from the items listed below.\r\n";
        char* options_mode_list[] = {
        "0.  MODE NONE",
        "1.  MODE ECB",
        "2.  MODE CBC",
        "3.  MODE CFB",
        "4.  MODE OFB",
        "5.  MODE CTR",
        "6.  MODE GCM",
        "7.  MODE STREAM",
        "8.  MODE CCM",
        "9.  MODE CCM_STAR_NO_TAG",
        "10. MODE XTS",
        "11. MODE CHACHAPOLY",
        "12. MODE KW",
        "13. MODE KWP",
        };

        char* options_paddding_list[] = {
        "0. PADDING PKCS7",
        "1. PADDING ONE_AND_ZEROS",
        "2. PADDING ZEROS_AND_LEN",
        "3. PADDING ZEROS",
        "4. PADDING NONE",
        };
        char* options_key_list[] = {
        "0. KEY64",
        "1. KEY128",
        "2. KEY192",
        "3. KEY256",
        };

        PrintfResp(note_mode);
        PrintfOptionMenu(options_mode_list, sizeof(options_mode_list) / sizeof(options_mode_list[0]));

        opt = UartReadValue();
        sAPI_Debug("mode = [%d]", opt);

        unsigned char mode = opt;
        ret = pl_crypto_cfg(hd, PL_CFG_CIPHER_MODE, &mode);
        if (ret != PL_CRYPTO_SUCCESS)
        {
            sAPI_Debug("pl_crypto_cfg mode error, ret = %d \r\n", ret);
            goto exit;
        }

        if (PL_CIPHER_MODE_CBC == mode)
        {
            PrintfResp(note_padding);
            PrintfOptionMenu(options_paddding_list, sizeof(options_paddding_list) / sizeof(options_paddding_list[0]));

            opt = UartReadValue();
            sAPI_Debug("padding = [%d]", opt);

            unsigned char padding = opt;

            ret = pl_crypto_cfg(hd, PL_CFG_CIPHER_PADDING, &padding);
            if (ret != PL_CRYPTO_SUCCESS)
            {
                sAPI_Debug("pl_crypto_cfg mode error, ret = %d \r\n", ret);
                goto exit;
            }
        }

        PrintfResp(note_key);
        PrintfOptionMenu(options_key_list, sizeof(options_key_list) / sizeof(options_key_list[0]));

        opt = UartReadValue();
        sAPI_Debug("key = [%d]", opt);

        pl_crypto_buf_t cipher_key_8 =  { key_8, 8 };
        pl_crypto_buf_t cipher_key_16 = { key_16, 16 };
        pl_crypto_buf_t cipher_key_24 = { key_24, 24 };
        pl_crypto_buf_t cipher_key_32 = { key_32, 32 };

        PrintfResp("\r\nkey: \r\n");
        if (0 == opt)
        {
            PrintfResp((char*)cipher_key_8.buf);
            ret = pl_crypto_cfg(hd, PL_CFG_CIPHER_KEY, &cipher_key_8);
        }
        if (1 == opt)
        {
            PrintfResp((char*)cipher_key_16.buf);
            ret = pl_crypto_cfg(hd, PL_CFG_CIPHER_KEY, &cipher_key_16);
        }
        else if (2 == opt)
        {
            PrintfResp((char*)cipher_key_24.buf);
            ret = pl_crypto_cfg(hd, PL_CFG_CIPHER_KEY, &cipher_key_24);
        }
        else
        {
            PrintfResp((char*)cipher_key_32.buf);
            ret = pl_crypto_cfg(hd, PL_CFG_CIPHER_KEY, &cipher_key_32);
        }

        // @param IV always copy 16 byte parameters and automatically obtain the required length internally
        unsigned char iv[PL_MAX_IV_LENGTH+1] = {0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x31,0x32,0x33,0x34,0x35,0};
        ret = pl_crypto_cfg(hd, PL_CFG_CIPHER_IV, iv);
        if (ret != PL_CRYPTO_SUCCESS)
        {
            sAPI_Debug("pl_crypto_cfg iv error, ret = %d \r\n", ret);
            goto exit;
        }
        PrintfResp("\r\niv: \r\n");
        PrintfResp((char*)iv);

        ret = pl_crypto(hd, PL_CIPHER_ENCRYPT, PL_MODE_ONCE, (unsigned char*)data, test_len, (unsigned char*)encrypt_data, (size_t*)&encrypt_len, buf_len);
        if (ret != PL_CRYPTO_SUCCESS)
        {
            sAPI_Debug("pl_crypto_cipher encrypt error, ret = %d \r\n", ret);
            goto exit;
        }

        sAPI_Debug("\r\nEncrypt successful \r\n");
        sAPI_Debug("Encrypt buffer data received size: %d\r\n", encrypt_len);
        // sAPI_Debug("encrypt_data: %.*s\r\n", encrypt_len, encrypt_data);
        PrintfResp("\r\nEncrypt data: \r\n");
        printf_data_to_ui(encrypt_data, encrypt_len);

        ret = pl_crypto(hd, PL_CIPHER_DECRYPT, PL_MODE_ONCE, (unsigned char*)encrypt_data, encrypt_len, (unsigned char*)decrypt_data, (size_t*)&decrypt_len, buf_len);
        if (ret != PL_CRYPTO_SUCCESS)
        {
            sAPI_Debug("pl_crypto_cipher decrypt error, ret = %d \r\n", ret);
            goto exit;
        }

        sAPI_Debug("\r\nDecrypt successful \r\n");
        PrintfResp("\r\nDecrypt data: \r\n");
        decrypt_data[decrypt_len] = '\0';
        PrintfResp(decrypt_data);
        sAPI_Debug("Decrypt buffer data received size: %d\r\n", decrypt_len);
    
        if(test_len > 50) 
        {
            // The parameter "output" is automatically offset, and the parameter "olen" is automatically accumulated.
            memset(encrypt_data, 0 ,buf_len);
            encrypt_len = 0;
            outlen = 0;
            // eg: Process all data in PL_MODE_UPDATE.
            ret = pl_crypto(hd, PL_CIPHER_ENCRYPT, PL_MODE_START , NULL                   ,  0,           NULL,                                      NULL,             0);
            ret = pl_crypto(hd, PL_CIPHER_ENCRYPT, PL_MODE_UPDATE, (unsigned char*)data   , 20,          (unsigned char*)encrypt_data + encrypt_len, (size_t*)&outlen, buf_len - encrypt_len);
            encrypt_len += outlen;
            sAPI_Debug("encrypt_len = %d, outlen = %d, ret = %d\r\n", encrypt_len, outlen, ret);
            ret = pl_crypto(hd, PL_CIPHER_ENCRYPT, PL_MODE_UPDATE, (unsigned char*)data+20, 20,          (unsigned char*)encrypt_data + encrypt_len, (size_t*)&outlen, buf_len - encrypt_len);
            encrypt_len += outlen;
            sAPI_Debug("encrypt_len = %d, outlen = %d, ret = %d\r\n", encrypt_len, outlen, ret);
            ret = pl_crypto(hd, PL_CIPHER_ENCRYPT, PL_MODE_UPDATE, (unsigned char*)data+40, test_len-40, (unsigned char*)encrypt_data + encrypt_len, (size_t*)&outlen, buf_len - encrypt_len);
            encrypt_len += outlen;
            sAPI_Debug("encrypt_len = %d, outlen = %d, ret = %d\r\n", encrypt_len, outlen, ret);
            ret = pl_crypto(hd, PL_CIPHER_ENCRYPT, PL_MODE_FINISH, NULL, 0,                              (unsigned char*)encrypt_data + encrypt_len, (size_t*)&outlen, buf_len - encrypt_len);
            encrypt_len += outlen;
            sAPI_Debug("encrypt_len = %d, outlen = %d, ret = %d\r\n", encrypt_len, outlen, ret);
            if (ret != PL_CRYPTO_SUCCESS)
            {
                sAPI_Debug("pl_crypto_cipher encrypt error, ret = %d \r\n", ret);
                goto exit;
            }

            sAPI_Debug("\r\nUpdate mode Encrypt update successful \r\n");
            sAPI_Debug("Encrypt buffer data received size: %d\r\n", encrypt_len);
            // sAPI_Debug("encrypt_data: %.*s\r\n", encrypt_len, encrypt_data);
            PrintfResp("\r\nUpdate mode encrypt data: \r\n");
            printf_data_to_ui(encrypt_data, encrypt_len);

            memset(decrypt_data, 0 ,buf_len);
            decrypt_len = 0;
            outlen = 0;

            // eg: PL_MODE_START processes the first packet of data, PL_MODE_UPDATE processes the intermediate data, and PL_MODE_FINISH processes the last packet of data.
            ret = pl_crypto(hd, PL_CIPHER_DECRYPT, PL_MODE_START , (unsigned char*)encrypt_data   , 20,             (unsigned char*)decrypt_data + decrypt_len, (size_t*)&outlen, buf_len - decrypt_len);
            decrypt_len += outlen;
            sAPI_Debug("decrypt_len = %d, outlen = %d ret = %d\r\n", decrypt_len, outlen, ret);
            ret = pl_crypto(hd, PL_CIPHER_DECRYPT, PL_MODE_UPDATE, (unsigned char*)encrypt_data+20, 20,             (unsigned char*)decrypt_data + decrypt_len, (size_t*)&outlen, buf_len - decrypt_len);
            decrypt_len += outlen;
            sAPI_Debug("decrypt_len = %d, outlen = %d ret = %d\r\n", decrypt_len, outlen, ret);
            ret = pl_crypto(hd, PL_CIPHER_DECRYPT, PL_MODE_FINISH, (unsigned char*)encrypt_data+40, encrypt_len-40, (unsigned char*)decrypt_data + decrypt_len, (size_t*)&outlen, buf_len - decrypt_len);
            decrypt_len += outlen;
            sAPI_Debug("decrypt_len = %d, outlen = %d ret = %d\r\n", decrypt_len, outlen, ret);
            if (ret != PL_CRYPTO_SUCCESS)
            {
                sAPI_Debug("pl_crypto_cipher encrypt error, ret = %d \r\n", ret);
                goto exit;
            }

            sAPI_Debug("\r\nUpdate mode decrypt update successful \r\n");
            PrintfResp("\r\nUpdate mode Decrypt data: \r\n");
            decrypt_data[decrypt_len] = '\0';
            PrintfResp(decrypt_data);
            sAPI_Debug("Decrypt buffer data received size: %d\r\n", decrypt_len);
        }

        break;
    }
    case DEMO_MD:
    {
        unsigned char *input = (unsigned char*)data;
        pl_crypto_buf_t md_key = { key_32, 32 };
        ret = pl_crypto_cfg(hd, PL_CFG_MD_KEY, &md_key);
        if (ret != PL_CRYPTO_SUCCESS)
        {
            sAPI_Debug("pl_crypto_cfg key error, ret = %d \r\n", ret);
            goto exit;
        }

        ret = pl_crypto(hd, PL_MD_HASH, PL_MODE_ONCE, input, test_len, (unsigned char*)encrypt_data, (size_t*)&encrypt_len, buf_len);
        if (ret != PL_CRYPTO_SUCCESS)
        {
            sAPI_Debug("pl_crypto_md error, ret = %d \r\n", ret);
            goto exit;
        }

        sAPI_Debug("hash successful \r\n");
        sAPI_Debug("hash data received size: %d\r\n", encrypt_len);

        PrintfResp("\r\nGenerated md data: \r\n");
        printf_data_to_ui(encrypt_data, encrypt_len);


        memset(encrypt_data, 0, buf_len + 1);
        ret = pl_crypto(hd, PL_MD_HASH, PL_MODE_START , input, 4, NULL, NULL, 0);
        if (ret != PL_CRYPTO_SUCCESS)
        {
            sAPI_Debug("pl_crypto_md_start error, ret = %d \r\n", ret);
            goto exit;
        }
        ret = pl_crypto(hd, PL_MD_HASH, PL_MODE_UPDATE, input + 4, 4, NULL, NULL, 0);
        ret = pl_crypto(hd, PL_MD_HASH, PL_MODE_UPDATE, input + 8, test_len - 8, NULL, NULL, 0);
        ret = pl_crypto(hd, PL_MD_HASH, PL_MODE_FINISH, NULL, 0, (unsigned char*)encrypt_data, (size_t*)&encrypt_len, buf_len);
        if (ret != PL_CRYPTO_SUCCESS)
        {
            sAPI_Debug("pl_crypto_md_finish error, ret = %d \r\n", ret);
            goto exit;
        }
        
        PrintfResp("\r\nupdate md data: \r\n");
        printf_data_to_ui(encrypt_data, encrypt_len);

        ret = pl_crypto(hd, PL_MD_HASH_HMAC, PL_MODE_ONCE, input, test_len, (unsigned char*)decrypt_data, (size_t*)&decrypt_len, buf_len);
        if (ret != PL_CRYPTO_SUCCESS)
        {
            sAPI_Debug("pl_crypto_md error, ret = %d \r\n", ret);
            goto exit;
        }

        sAPI_Debug("hmac successful \r\n");
        sAPI_Debug("hmac data received size: %d\r\n", decrypt_len);
        sAPI_Debug("hmac data: %s\r\n", decrypt_data);

        PrintfResp("\r\nGenerated hmac md data: \r\n");
        printf_data_to_ui(decrypt_data, decrypt_len);

        memset(decrypt_data, 0, buf_len + 1);
        ret = pl_crypto(hd, PL_MD_HASH_HMAC, PL_MODE_START , input    ,            4, NULL, NULL, 0);
        if (ret != PL_CRYPTO_SUCCESS)
        {
            sAPI_Debug("pl_crypto_md_start error, ret = %d \r\n", ret);
            goto exit;
        }

        ret = pl_crypto(hd, PL_MD_HASH_HMAC, PL_MODE_UPDATE, input + 4,            4, NULL, NULL, 0);
        ret = pl_crypto(hd, PL_MD_HASH_HMAC, PL_MODE_UPDATE, input + 8, test_len - 8, NULL, NULL, 0);
        ret = pl_crypto(hd, PL_MD_HASH_HMAC, PL_MODE_FINISH, NULL, 0, (unsigned char*)decrypt_data, (size_t*)&decrypt_len, buf_len);

        if (ret != PL_CRYPTO_SUCCESS)
        {
            sAPI_Debug("pl_crypto_md_finish error, ret = %d \r\n", ret);
            goto exit;
        }
        PrintfResp("\r\nupdate hmac md data: \r\n");
        printf_data_to_ui(decrypt_data, decrypt_len);
    }
    default:
        break;
    }

exit:
    sAPI_Free(data);
    sAPI_Free(encrypt_data);
    sAPI_Free(decrypt_data);

    ret = pl_crypto_delete(&hd);
    sAPI_Debug("ret = %d", ret);
}

/**
  * @brief  crypto Demo.
  * @param  void
  * @note
  * @retval void
  */
void CryptoDemo(void)
{
    UINT32 opt = 0, type = 0, len = 0;
    int ret = 0;
    // char path[] = "c:/hash_test.txt";
    char* note = "\r\n\r\nPlease select an option to test from the items listed below.\r\n";
    char* input_len = "\r\nPlease enter an encryption length you want to test\r\n";
    char* options_list[] = {
    "0. TEST PK",
    "1. TEST CIPHER",
    "2. TEST MD",
    "3. TEST RANDOM",
    "99. back",
    };

    while (1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));

        opt = UartReadValue();
        sAPI_Debug("opt = [%d]", opt);

        switch (opt)
        {
        case DEMO_PK:
        {
            char* options_pk_list[] = {
            "1. PK RSA",
            "2. PK ECKEY",
            "3. PK ECKEY_DH",
            "4. PK ECDSA",
            "5. PK RSA_ALT",
            "6. PK RSASSA_PSS",
            "7. PK OPAQUE",
            "99. back",
            };
            PrintfResp(note);
            PrintfOptionMenu(options_pk_list, sizeof(options_pk_list) / sizeof(options_pk_list[0]));
            break;
        }
        case DEMO_CIPHER:
        {
            char* options_cipher_list[] = {
            "22. CIPHER AES",
            "23. CIPHER DES",
            "24. CIPHER 3DES",
            "25. CIPHER CAMELLIA",
            "26. CIPHER ARIA",
            "27. CIPHER CHACHA20",
            "99. back",
            };
            PrintfResp(note);
            PrintfOptionMenu(options_cipher_list, sizeof(options_cipher_list) / sizeof(options_cipher_list[0]));
            break;
        }
        case DEMO_MD:
        {
            char* options_md_list[] = {
            "41. MD5",
            "42. SHA1",
            "43. SAH224",
            "44. SHA256",
            "45. SHA384",
            "46. SHA512",
            "47. RIPEMD160",
            "99. back",
            };
            PrintfResp(note);
            PrintfOptionMenu(options_md_list, sizeof(options_md_list) / sizeof(options_md_list[0]));
            break;
        }
        case DEMO_RANDOM:
        {
            PrintfResp(input_len);
            len = UartReadValue();
            sAPI_Debug("len = [%d]", len);

            char* data = sAPI_Malloc(len + 1);
            if (data == NULL)
            {
                sAPI_Debug("error,[%s], malloc failed", __FUNCTION__);
                continue;
            }

            const char* pers = "Customization";
            ret = pl_crypto_random((unsigned char*)data, len, (const unsigned char *)pers, strlen(pers));
            if (ret != PL_CRYPTO_SUCCESS)
            {
                sAPI_Debug("pl_crypto_random error, ret = %d \r\n", ret);
                sAPI_Free(data);
                continue;
            }

            PrintfResp("\r\nGenerated random data: \r\n");
            printf_data_to_ui(data, len);
            sAPI_Free(data);
            continue;
        }

        case SC_DEMO_CRYPTO_MAX:
        {
            return;
        }

        default:
            return;
        }

        type = UartReadValue();
        sAPI_Debug("type = [%d]", type);

        PrintfResp(input_len);
        len = UartReadValue();
        sAPI_Debug("len = [%d]", len);
        sAPI_Debug("Start demo test encrypt!");
        demo_crypto(opt, type, len);
    }

}

