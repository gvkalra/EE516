#pragma once

#include <stdio.h>
#include <stdlib.h>

#define DEBUG_ENABLE /* uncomment to enable debugging logs */

#define err(fmt,args...) \
	do { \
		fprintf(stderr, "<%s:%d> " fmt "\n", __func__, __LINE__, ##args); \
	} while (0)

#ifdef DEBUG_ENABLE
	#define dbg(fmt,args...) \
		do { \
			fprintf(stdout, "<%s:%d> " fmt "\n", __func__, __LINE__, ##args); \
		} while (0)
#else
	#define dbg(fmt,args...)
#endif

#define info(fmt,args...) \
	do { \
		fprintf(stdout, fmt "\n", ##args); \
	} while (0)
