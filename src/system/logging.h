#ifndef TAROT_LOGGING_H
#define TAROT_LOGGING_H

#include "defines.h"

extern void tarot_enable_logging(bool enable);
extern void tarot_enable_debugging(bool enable);
extern void tarot_log(const char *format, ...);
extern void tarot_debug(const char *format, ...);

#endif /* TAROT_LOGGING_H */
