#include <stdint.h>


/* These are represenations of "BM" */
#ifdef WORDS_BIGENDIAN
#define BF_TYPE 0x424d
#else
#define BF_TYPE 0x4D42             
#endif


/*
 * Constants for the biCompression field...
 */

#define BI_RGB       0             /* No compression - straight BGR data */
#define BI_RLE8      1             /* 8-bit run-length compression */
#define BI_RLE4      2             /* 4-bit run-length compression */
#define BI_BITFIELDS 3             /* RGB bitmap with RGB masks */
#define RGB_BLACK    0
#define RGB_PADDING  0xff