#ifndef TAROT_STDBOOL_H
#define TAROT_STDBOOL_H

#ifndef __bool_true_false_are_defined

typedef enum {
	false,
	true
} bool;

#define __bool_true_false_are_defined

#endif /* __bool_true_false_are_defined */

extern const char* tarot_bool_string(bool value);

#endif /* TAROT_STDBOOL_H */
