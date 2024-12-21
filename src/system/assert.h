#ifndef TAROT_ASSERT_H
#define TAROT_ASSERT_H

#include "defines.h"

#if defined __STDC_HOSTED__ && __STDC_HOSTED__
#include <assert.h>
#else /* Default to dummy assert */
#ifndef assert
#define assert(Expression) ((void)0)
#endif
#endif /* __STDC_HOSTED__ */

#endif /* TAROT_ASSERT_H */
