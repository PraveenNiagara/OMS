#ifndef __PL_CRYPTO_H__
#define __PL_CRYPTO_H__

#include "simcom_os.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *pl_crypto_handle;

/** Maximum length of any IV, in Bytes. */
#define PL_MAX_IV_LENGTH      16

typedef struct pl_crypto_buf
{
    unsigned char *buf;
    unsigned int   len;
} pl_crypto_buf_t;

typedef enum
{
    PL_CRYPTO_SUCCESS               =  0,
    PL_CRYPTO_ERROR                 = -1,
    PL_CRYPTO_INVALID_PARAM         = -2,
    PL_CRYPTO_INVALID_OP            = -3,
    PL_CRYPTO_INVALID_MODE          = -4,
    PL_CRYPTO_INVALID_TYPE          = -5,
    PL_CRYPTO_INVALID_PK_MD_TYPE    = -6,
    PL_CRYPTO_INVALID_INPUT_LENGTH  = -7,
    PL_CRYPTO_INVALID_SMALL_OSIZE   = -8,
    PL_CRYPTO_INVALID_CONTEXT       = -9,
    PL_CRYPTO_ALLOC_FAILED          = -10
} pl_crypto_ret_t;

typedef enum
{
    PL_OP_NONE = 0,
    PL_PK_ENCRYPT,
    PL_PK_DECRYPT,
    PL_PK_SIGN,
    PL_PK_VERIFY,

    PL_CIPHER_ENCRYPT = 10,
    PL_CIPHER_DECRYPT,
    PL_CIPHER_AUTH_ENCRYPT,
    PL_CIPHER_AUTH_DECRYPT,

    PL_MD_HASH = 20,
    PL_MD_HASH_HMAC
} pl_crypto_op_t;

typedef enum
{
    PL_MODE_ONCE = 0,
    PL_MODE_START,
    PL_MODE_UPDATE,
    PL_MODE_FINISH
} pl_crypto_mode_t;

typedef enum
{
    PL_TYPE_NONE = 0,

    /* RSA and ECC will automatically recognize and configure when loading keys, and the configuration is invalid  */
    PL_PK_RSA,
    PL_PK_ECKEY,

    PL_CIPHER_ID_NONE = 20, /**< Placeholder to mark the end of cipher ID lists. */
    PL_CIPHER_ID_NULL,      /**< The identity cipher, treated as a stream cipher. */
    PL_CIPHER_ID_AES,       /**< The AES cipher. */
    PL_CIPHER_ID_DES,       /**< The DES cipher. \warning DES is considered weak. */
    PL_CIPHER_ID_3DES,      /**< The Triple DES cipher. \warning 3DES is considered weak. */
    PL_CIPHER_ID_CAMELLIA,  /**< The Camellia cipher. */
    PL_CIPHER_ID_ARIA,      /**< The Aria cipher. */
    PL_CIPHER_ID_CHACHA20,  /**< The ChaCha20 cipher. */

    PL_MD_NONE = 40,        /**< None. */
    PL_MD_MD5,              /**< The MD5 message digest. */
    PL_MD_SHA1,             /**< The SHA-1 message digest. */
    PL_MD_SHA224,           /**< The SHA-224 message digest. */
    PL_MD_SHA256,           /**< The SHA-256 message digest. */
    PL_MD_SHA384,           /**< The SHA-384 message digest. */
    PL_MD_SHA512,           /**< The SHA-512 message digest. */
    PL_MD_RIPEMD160,        /**< The RIPEMD-160 message digest. */
} pl_crypto_type_t;

typedef enum
{
    PL_CFG_NONE = 0,
    PL_CFG_TYPE,                    // pl_crypto_type_t

    PL_CFG_PK_PUBLIC_KEY,           // pl_crypto_buf_t
    PL_CFG_PK_PRIVATE_KEY,          // pl_crypto_buf_t
    PL_CFG_PK_PRIVATE_KEY_PWD,      // pl_crypto_buf_t
    PL_CFG_PK_HASH,                 // pl_crypto_buf_t
    PL_CFG_PK_SIG,                  // pl_crypto_buf_t
    PL_CFG_PK_MD_TYPE,              // pl_crypto_type_t

    PL_CFG_CIPHER_PADDING = 20,     // pl_cipher_padding_t
    PL_CFG_CIPHER_MODE,             // pl_cipher_mode_t
    PL_CFG_CIPHER_KEY,              // pl_crypto_buf_t
    PL_CFG_CIPHER_IV,               // always copy 16 byte parameters and automatically obtain the
    // required length internally
    PL_CFG_CIPHER_AD,               // pl_crypto_buf_t
    PL_CFG_CIPHER_TAG,              // pl_crypto_buf_t

    PL_CFG_MD_KEY = 40,             // pl_crypto_buf_t

    PL_CFG_ALL = 255
} pl_crypto_cfg_t;

typedef enum
{
    PL_CIPHER_MODE_NONE = 0,               /**< None.                        */
    PL_CIPHER_MODE_ECB,                    /**< The ECB cipher mode.         */
    PL_CIPHER_MODE_CBC,                    /**< The CBC cipher mode.         */
    PL_CIPHER_MODE_CFB,                    /**< The CFB cipher mode.         */
    PL_CIPHER_MODE_OFB,                    /**< The OFB cipher mode.         */
    PL_CIPHER_MODE_CTR,                    /**< The CTR cipher mode.         */
    PL_CIPHER_MODE_GCM,                    /**< The GCM cipher mode.         */
    PL_CIPHER_MODE_STREAM,                 /**< The stream cipher mode.      */
    PL_CIPHER_MODE_CCM,                    /**< The CCM cipher mode.         */
    PL_CIPHER_MODE_CCM_STAR_NO_TAG,        /**< The CCM*-no-tag cipher mode. */
    PL_CIPHER_MODE_XTS,                    /**< The XTS cipher mode.         */
    PL_CIPHER_MODE_CHACHAPOLY,             /**< The ChaCha-Poly cipher mode. */
    PL_CIPHER_MODE_KW,                     /**< The SP800-38F KW mode */
    PL_CIPHER_MODE_KWP,                    /**< The SP800-38F KWP mode */
    PL_CIPHER_MODE_MAX,
} pl_cipher_mode_t;

typedef enum
{
    PL_CIPHER_PADDING_PKCS7 = 0,        /**< PKCS7 padding (default).        */
    PL_CIPHER_PADDING_ONE_AND_ZEROS,    /**< ISO/IEC 7816-4 padding.         */
    PL_CIPHER_PADDING_ZEROS_AND_LEN,    /**< ANSI X.923 padding.             */
    PL_CIPHER_PADDING_ZEROS,            /**< Zero padding (not reversible).  */
    PL_CIPHER_PADDING_NONE,             /**< Never pad (full blocks only).   */
} pl_cipher_padding_t;

typedef enum
{
    PL_SSL_TLS_PRF_NONE,
    PL_SSL_TLS_PRF_SHA384,
    PL_SSL_TLS_PRF_SHA256,
    PL_SSL_HKDF_EXPAND_SHA384,
    PL_SSL_HKDF_EXPAND_SHA256
} pl_tls_prf_types_t;

typedef struct pl_crypto_info
{
    pl_crypto_type_t type;

    /*pk config*/
    /* Used to configure the hash algorithm required for signature and verification, configurable
       (PL_MD_NONE - PL_MD_RIPEMD160).*/
    pl_crypto_type_t pk_md_type;
    // mbedtls_cipher_padding_t padding;    //def PKCS1_V15
    pl_crypto_buf_t public_key;
    pl_crypto_buf_t private_key;
    pl_crypto_buf_t password;
    // extern param, unused
    pl_crypto_buf_t hash;
    pl_crypto_buf_t sig;

    /*cipher config*/
    void *cipher_ctx;
    /** The cipher mode. For example, PL_CIPHER_MODE_CBC. */
    pl_cipher_mode_t mode;
    pl_cipher_padding_t padding;
    pl_crypto_buf_t key;
    /* always copy 16 byte parameters and automatically obtain the required length internally */
    unsigned char iv[PL_MAX_IV_LENGTH];
    // extern param, unused
    pl_crypto_buf_t ad;
    pl_crypto_buf_t tag;

    /*md config*/
    void *md_ctx;
    pl_crypto_buf_t hmac_key;
} pl_crypto_info_t;

/**
 *                      Create a cryptographic handle
 *
 *                      This function initializes and creates a cryptographic handle that will be used for subsequent cryptographic operations.
 *
 * @param handle        A pointer to a pointer of type `pl_crypto_handle`, which will receive the created handle.
 *
 * @return              PL_CRYPTO_SUCCESS on success, or an error code otherwise.
 */
int pl_crypto_create(pl_crypto_handle *handle);

/**
 *                      reset cryptographic handle params
 *
 *                      This function is used to reset params related to cryptographic params, including various keys, hash values, and other sensitive information.
 *
 * @param handle        The cryptographic handle used to identify and manage the context of cryptographic operations.
 * @return              PL_CRYPTO_SUCCESS on success, or an error code otherwise.
 */
int pl_crypto_reset(pl_crypto_handle handle);

/**
 * @brief               Delete the crypto handle and release associated resources
 *
 *                      This function releases the resources associated with the crypto handle and sets the handle to NULL.
 *
 * @param handle        Pointer to the crypto handle
 * @return              PL_CRYPTO_SUCCESS on success, or an error code otherwise.
 */
int pl_crypto_delete(pl_crypto_handle *handle);

/**
 * @brief               Set cryptographic operation configuration parameters
 *
 *                      This function sets various parameters for cryptographic operations based on the provided configuration parameter and its value.
 *                      It ensures the security and correctness of cryptographic operations by validating the parameters.
 *
 * @param handle        The handle for the cryptographic operation, used to identify and manage the context of the operation
 * @param cfg           The configuration parameter indicating the type of cryptographic operation to set
 * @param argp          A pointer to the value of the configuration parameter, the specific meaning depends on the configuration type
 * @return              PL_CRYPTO_SUCCESS on success, or an error code otherwise.
 */
int pl_crypto_cfg(pl_crypto_handle handle, unsigned char cfg, void *argp);

/**
 *                      Get the detailed information of a cryptographic operation handle.
 *
 *                      This function accepts a cryptographic operation handle as a parameter and returns a pointer to the corresponding cryptographic information structure.
 *
 * @param handle        The handle of the cryptographic operation.
 *
 * @return              A pointer to the cryptographic information structure. Returns NULL if the handle is invalid.
 */
const pl_crypto_info_t *pl_crypto_get_cfg(pl_crypto_handle handle);

/**
 * @brief               Supports symmetric encryption algorithm (CIPHER), asymmetric encryption algorithm (PK), and digest algorithm (MD) series algorithms.
 *
 * @param handle        A pointer to the cryptographic context.
 * @param op            The operation to perform (e.g., PL_PK_ENCRYPT, PL_CIPHER_DECRYPT, PL_MD_HASH).
 * @param mode          Data input mode selection：
 *                      - PL_MODE_ONCE: Data is input and output in a single operation.
 *                      - PL_MODE_START: Initializes continuous data mode.
 *                      - PL_MODE_UPDATE: Data is input continuously.
 *                      - PL_MODE_FINISH: Completes data input and outputs the result.
 * @param input         Message to encrypt/decrypt.
 * @param ilen          Message size.
 * @param output        Encrypted/Decrypted output.
 * @param olen          Encrypted/Decrypted message length.
 * @param osize         Size of the output buffer.
 *
 * @return              PL_CRYPTO_SUCCESS on success, or an error code otherwise.
 *
 * @note:
 * For cipher series encryption algorithms, osize must be greater than the input data length plus one block size.
 * -- Cipher series encryption algorithms: osize > ilen + block_size
 * For RSA encryption algorithms, the input data length must be less than the key length minus the padding bytes (only support PKCS#1 v1.5 padding is 11 bytes),
 * and the output buffer size must be at least equal to the key length.
 * -- RSA encryption algorithms: ilen < key_length - padding_bytes and osize >= key_length
 * For RSA decryption algorithms, both the input data length and the output buffer size must be equal to the key length.
 * -- RSA decryption algorithms: ilen == osize == key_length
 * For MD series hash algorithms, the output buffer size must be at least equal to the output byte length of the specific algorithm.
 * -- MD series hash algorithms: osize >= algorithm_output_length
 */
int pl_crypto(pl_crypto_handle handle, pl_crypto_op_t op, pl_crypto_mode_t mode, const unsigned char *input,
              unsigned int ilen, unsigned char *output, unsigned int *olen, unsigned int osize);

/**
 *                      Generates a specified length of random data using the mbed TLS library.
 *
 * @param output        Buffer to receive the random data.
 * @param olen          Length of the random data to generate, in bytes.
 * @param custom        The personalization string.
 *                      This can be \c NULL, in which case the personalization
 *                      string is empty regardless of the value of \p len.
 * @param clen          The length of the personalization string.
 * @return              PL_CRYPTO_SUCCESS on success, or an error code otherwise.
 *
 * @note                This function initializes the mbed TLS entropy source and CTR_DRBG random number generator,
 *                      then uses them to generate cryptographically secure random data. The generated random data
 *                      is written to the output buffer.
 */
int pl_crypto_random(unsigned char *output, unsigned int olen, const unsigned char *custom, unsigned int clen);

/**
 * \brief               TLS-PRF function for key derivation.
 *
 * \param prf           The tls_prf type function type to be used.
 * \param secret        Secret for the key derivation function.
 * \param slen          Length of the secret.
 * \param label         String label for the key derivation function,
 *                      terminated with null character.
 * \param random        Random bytes.
 * \param rlen          Length of the random bytes buffer.
 * \param dstbuf        The buffer holding the derived key.
 * \param dlen          Length of the output buffer.
 *
 * \return         0 on success. An SSL specific error on failure.
 */
int  pl_ssl_tls_prf(const pl_tls_prf_types_t prf,
                    const unsigned char *secret, unsigned int slen,
                    const char *label,
                    const unsigned char *random, unsigned int rlen,
                    unsigned char *dstbuf, unsigned int dlen);

#ifdef __cplusplus
}
#endif

#endif /* __PL_CRYPTO_H__ */

