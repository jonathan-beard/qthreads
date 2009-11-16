#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <qthread/qthread.h>
#include <qthread/qarray.h>
#include "argparsing.h"

#define ELEMENT_COUNT 10000

aligned_t count = 0;
typedef struct
{
    char pad[10000];
} bigobj;
typedef struct
{
    char pad[41];
} offsize;

static aligned_t assign1(qthread_t * me, void *arg)
{
    *(double *)arg = 1.0;
    qthread_incr(&count, 1);
    return 0;
}

static aligned_t assignall1(qthread_t * me, void *arg)
{
    memset(arg, 1, sizeof(bigobj));
    qthread_incr(&count, 1);
    return 0;
}

static void assignoff1(qthread_t * me, const size_t startat,
		       const size_t stopat, qarray * q, void *arg)
{
    for (size_t i = startat; i < stopat; i++) {
	void *ptr = qarray_elem_nomigrate(q, i);

	memset(ptr, 1, sizeof(offsize));
    }
    qthread_incr(&count, stopat - startat);
}

int main(int argc, char *argv[])
{
    qarray *a;
    qthread_t *me;
    distribution_t disttypes[] = {
	FIXED_HASH, ALL_LOCAL, ALL_RAND, ALL_LEAST, DIST_RAND,
	DIST_REG_STRIPES, DIST_REG_FIELDS, DIST_LEAST
    };
    const char *distnames[] = {
	"FIXED_HASH", "ALL_LOCAL", "ALL_RAND", "ALL_LEAST", "DIST_RAND",
	"DIST_REG_STRIPES", "DIST_REG_FIELDS", "DIST_LEAST"
    };
    unsigned int dt_index;
    unsigned int num_dists = 8;

    qthread_initialize();
    me = qthread_self();
    CHECK_VERBOSE();
    NUMARG(num_dists, "TEST_NUM_DISTS");
    if (num_dists > 8) {
	num_dists == 8;
    }

    /* iterate over all the different distribution types */
    for (dt_index = 0;
	 dt_index < num_dists;
	 dt_index++) {
	/* test a basic array of doubles */
	count = 0;
	a = qarray_create_configured(ELEMENT_COUNT, sizeof(double),
				     disttypes[dt_index], 0, 0);
	assert(a);
	iprintf("%s: created basic array of doubles\n",
		distnames[dt_index]);
	qarray_iter(me, a, 0, ELEMENT_COUNT, assign1);
	iprintf("%s: iterated; now checking work...\n",
		distnames[dt_index]);
	if (count != ELEMENT_COUNT) {
	    printf("count = %lu, dt_index = %u\n", (unsigned long)count,
		   dt_index);
	    assert(count == ELEMENT_COUNT);
	}
	{
	    size_t i;

	    for (i = 0; i < ELEMENT_COUNT; i++) {
		double elem = *(double *)qarray_elem_nomigrate(a, i);

		if (elem != 1.0) {
		    printf
			("element %lu is %f instead of 1.0, disttype = %s\n",
			 (unsigned long)i, elem, distnames[dt_index]);
		    assert(elem == 1.0);
		}
	    }
	}
	iprintf("%s: correct result!\n", distnames[dt_index]);
	qarray_destroy(a);

	/* now test an array of giant things */
	count = 0;
	a = qarray_create_configured(ELEMENT_COUNT, sizeof(bigobj),
				     disttypes[dt_index], 0, 0);
	iprintf("%s: created array of big objects\n", distnames[dt_index]);
	qarray_iter(me, a, 0, ELEMENT_COUNT, assignall1);
	iprintf("%s: iterated; now checking work...\n",
		distnames[dt_index]);
	if (count != ELEMENT_COUNT) {
	    printf("count = %lu, dt_index = %u\n", (unsigned long)count,
		   dt_index);
	    assert(count == ELEMENT_COUNT);
	}
	{
	    size_t i;

	    for (i = 0; i < ELEMENT_COUNT; i++) {
		char *elem = (char *)qarray_elem_nomigrate(a, i);
		size_t j;

		for (j = 0; j < sizeof(bigobj); j++) {
		    if (elem[j] != 1) {
			printf
			    ("byte %lu of element %lu is %i instead of 1, dt_index = %u\n",
			     (unsigned long)j, (unsigned long)i, elem[j],
			     dt_index);
			assert(elem[j] == 1);
		    }
		}
	    }
	}
	iprintf("%s: correct result!\n", distnames[dt_index]);
	qarray_destroy(a);

	/* now test an array of weird-sized things */
	count = 0;
	a = qarray_create_configured(ELEMENT_COUNT, sizeof(offsize),
				     disttypes[dt_index], 0, 0);
	iprintf("%s: created array of odd-sized objects\n",
		distnames[dt_index]);
	qarray_iter_loop(me, a, 0, ELEMENT_COUNT, assignoff1, NULL);
	iprintf("%s: iterated; now checking work...\n",
		distnames[dt_index]);
	if (count != ELEMENT_COUNT) {
	    printf("count = %lu, dt_index = %u\n", (unsigned long)count,
		   dt_index);
	    assert(count == ELEMENT_COUNT);
	}
	{
	    size_t i;

	    for (i = 0; i < ELEMENT_COUNT; i++) {
		char *elem = (char *)qarray_elem_nomigrate(a, i);
		size_t j;

		for (j = 0; j < sizeof(offsize); j++) {
		    if (elem[j] != 1) {
			printf
			    ("byte %lu of element %lu is %i instead of 1, dt_index = %u\n",
			     (unsigned long)j, (unsigned long)i, elem[j],
			     dt_index);
			assert(elem[j] == 1);
		    }
		}
	    }
	}
	iprintf("%s: correct result!\n", distnames[dt_index]);
	qarray_destroy(a);
    }

    return 0;
}
