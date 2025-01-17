#ifndef TAROT_H
#define TAROT_H

#include "bytecode/bytecode.h"
#include "bytecode/thread.h"
#include "bytecode/region.h"
#include "bytecode/vm.h"

#include "datatypes/dictionary.h"
#include "datatypes/integer.h"
#include "datatypes/list.h"
#include "datatypes/rational.h"
#include "datatypes/string.h"
#include "datatypes/object.h"
#include "datatypes/value.h"

#if defined __STDC_HOSTED__ && __STDC_HOSTED__
#include <math.h>
#else
#include "extern/fdlibm/fdlibm.h"
#endif

#if defined PENTA_BACKEND && PENTA_BACKEND == 2
#include <gmp.h>
#else
#include "extern/mini-gmp/mini-gmp.h"
#include "extern/mini-gmp/mini-mpq.h"
#endif

#include "lexer/token.h"
#include "lexer/scanner.h"

#include "main/getopt.h"
#include "main/main.h"

#include "system/assert.h"
#include "system/ctype.h"
#include "system/error.h"
#include "system/iostream.h"
#include "system/iso646.h"
#include "system/logging.h"
#include "system/malloc.h"
#include "system/platform.h"
#include "system/string.h"

#include "tree/tree.h"

#include "tests/tests.h"

#endif /* TAROT_H */
