#ifndef config_HEADER
#define config_HEADER

#define PLAINTEXT_MAX_LEN 32

#ifdef MINGW
#define F016LLX "%016I64x" 
#define FLLD "%I64d"
#else
#define F016LLX "%016llx"
#define FLLD "%lld"
#endif

#endif
