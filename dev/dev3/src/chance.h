#ifndef CHANCE_H_
#define CHANCE_H_

#include <stdlib.h>

// A number representing the chance of something occurring.
typedef unsigned chance;

// The maximum value of a chance.
#define CHANCE_MAX ((unsigned)RAND_MAX + 1)

// A chance that will never come up true.
#define CHANCE_NEVER 0

// A chance that will always come up true.
#define CHANCE_ALWAYS CHANCE_MAX

// Convert a double between 0 and 1 (inclusive) to a chance.
#define chance_from_fraction(frac) ((double)(frac) * CHANCE_MAX)

// Convert a chance to a double between 0 and 1, inclusive.
#define chance_to_fraction(chance) ((double)(chance) / CHANCE_MAX)

// Convert a double between 0 and 100 (inclusive) to a chance.
#define chance_from_percent(per) chance_from_fraction((per) / 100.0)

// Convert a chance to a double between 0 and 100, inclusive.
#define chance_to_percent(chance) (chance_to_fraction(chance) * 100.0)

// Use rand() to randomly decide yes or no. The probability of yes is the
// percent that would be obtained by chance_to_percent(chance).
#define chance_decide(chance) ((chance) > (unsigned)rand())

#endif /* CHANCE_H_ */
