#ifndef QLOOP_INNARDS_H
#define QLOOP_INNARDS_H

struct qqloop_iteration_queue {
    volatile saligned_t start;
    saligned_t stop;
    saligned_t step;
    qt_loop_queue_type type;
    union {
	volatile saligned_t phase;
	struct {
	    qtimer_t *timers;
	    saligned_t *lastblocks;
	} timed;
    } type_specific_data;
};
struct qqloop_static_args;
struct qqloop_wrapper_range;
typedef int (
    *qq_getiter_f) (
    struct qqloop_iteration_queue * const restrict,
    struct qqloop_static_args * const restrict,
    struct qqloop_wrapper_range * const restrict);
struct qqloop_static_args {
    qt_loop_f func;
    void *arg;
    volatile aligned_t donecount;
    volatile aligned_t activesheps;
    struct qqloop_iteration_queue *iq;
    qq_getiter_f get;
};
struct qqloop_step_static_args {
    qt_loop_step_f func;
    void *arg;
    volatile aligned_t donecount;
    volatile aligned_t activesheps;
    struct qqloop_iteration_queue *iq;
    qq_getiter_f get;
};
struct qqloop_wrapper_args {
    qthread_shepherd_id_t shep;
    struct qqloop_static_args *stat;
};
struct qqloop_step_wrapper_args {
    qthread_shepherd_id_t shep;
    struct qqloop_step_static_args *stat;
};
struct qqloop_wrapper_range {
    size_t startat, stopat, step;
};
struct qqloop_handle_s {
    struct qqloop_wrapper_args *qwa;
    struct qqloop_static_args stat;
};

enum qloop_handle_type {
    QLOOP_NONE = 0,
    STATIC_SCHED,
    GUIDED_SCHED,
    DYNAMIC_SCHED,
    RUNTIME_SCHED,
};

struct qqloop_step_handle_s {
    struct qqloop_step_wrapper_args *qwa;
    struct qqloop_step_static_args stat;
#ifdef QTHREAD_USE_ROSE_EXTENSIONS
    int workers;
    enum qloop_handle_type type;
    int assignNext;
    int assignStop;
    int assignStep;
    int chunkSize;
    volatile aligned_t assignDone;	// start+offset
    size_t shepherdsActive;	// bit vector to stop shepherds from grabbing a loop twice (is this necessary?)
#endif /* QTHREAD_USE_ROSE_EXTENSIONS */
};

#ifdef QTHREAD_USE_ROSE_EXTENSIONS
int qloop_internal_computeNextBlock(
    int block,
    double time,
    volatile qqloop_step_handle_t * loop);
#endif /* QTHREAD_USE_ROSE_EXTENSIONS */

#endif