#ifndef _MD5_H
#define _MD5_H

#ifdef USE_OPENSSL

#include <openssl/md5.h>

#elif defined USE_GCRYPT

#include <gcrypt.h>

typedef gcry_md_hd_t MD5_CTX;

static void MD5_Init(MD5_CTX * ctx)
{
	gcry_md_open(ctx, GCRY_MD_MD5, 0);
}

static void MD5_Update(MD5_CTX * ctx,
		const unsigned char * input,
		unsigned int inputLen)
{
	gcry_md_write(*ctx, input, inputLen);
}

static void MD5_Final(unsigned char digest[16], MD5_CTX * ctx)
{
	memcpy(digest, gcry_md_read(*ctx, 0), 16);
	gcry_md_close(*ctx);
}
#else
#error "no MD5 implementation"
#endif  //USE_GCRYPT

#endif
