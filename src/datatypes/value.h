#ifndef TAROT_TYPE_VALUE_H
#define TAROT_TYPE_VALUE_H

#include "defines.h"

/* Forward declarations */
struct tarot_dictionary;
struct tarot_list;
struct tarot_string;

union tarot_value {
	bool                     Boolean;
	double                   Float;
	size_t                   Index;
	void                    *Rational;
	void                    *Integer;
	void                    *Pointer;
	struct tarot_dictionary *Dict;
	struct tarot_list       *List;
	struct tarot_string     *String;
	union tarot_value       *Value;
};

#endif /* TAROT_TYPE_VALUE_H */
