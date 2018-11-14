/* Copyright (C) 2013-2016, The Regents of The University of Michigan.
All rights reserved.

This software was developed in the APRIL Robotics Lab under the
direction of Edwin Olson, ebolson@umich.edu. This software may be
available under alternative licensing terms; contact the address above.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the Regents of The University of Michigan.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "workerpool.h"
#include "timeprofile.h"
#include "math_util.h"
#include "string_util.h"

#ifdef _WIN32
#include <windows.h>
#include <process.h>

#define THREAD_ROUTINE(f) unsigned (__stdcall f)

typedef HANDLE pthread_t;
typedef CRITICAL_SECTION pthread_mutex_t;
typedef CONDITION_VARIABLE pthread_cond_t;

static int sched_yield()
{
    SwitchToThread();
    return 0;
}

static int pthread_create(pthread_t *newthread, const void *attr,
                         THREAD_ROUTINE(*routine)(void *), void *arg)
{
    (void) attr;
    *newthread = (pthread_t)_beginthreadex(NULL, 0, routine, arg, 0, NULL);
    return 0;
}

static int pthread_join(pthread_t th, void **ret)
{
    (void) ret;
    WaitForSingleObject(th, INFINITE);
    CloseHandle(th);
    return 0;
}

static int pthread_mutex_init(pthread_mutex_t *mutex, const void *attr)
{
    (void) attr;
    InitializeCriticalSection(mutex);
    return 0;
}

static int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    DeleteCriticalSection(mutex);
    return 0;
}

static int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    EnterCriticalSection(mutex);
    return 0;
}

static int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    LeaveCriticalSection(mutex);
    return 0;
}

static int pthread_cond_init(pthread_cond_t *cond, const void *attr)
{
    InitializeConditionVariable(cond);
    return 0;
}

static int pthread_cond_destroy(pthread_cond_t *cond)
{
    (void) cond;
    return 0;
}

static int pthread_cond_signal(pthread_cond_t *cond)
{
    WakeConditionVariable(cond);
    return 0;
}

static int pthread_cond_broadcast(pthread_cond_t *cond)
{
    WakeAllConditionVariable(cond);
    return 0;
}

static int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    SleepConditionVariableCS(cond, mutex, INFINITE);
    return 0;
}
#else
#include <pthread.h>
#include <sched.h>
#define THREAD_ROUTINE(f) void* (f)
#endif

struct workerpool {
    int nthreads;
    zarray_t *tasks;
    int taskspos;

    pthread_t *threads;
    int *status;

    pthread_mutex_t guard;
    pthread_mutex_t mutex;
    pthread_cond_t startcond;   // used to signal the availability of work
    pthread_cond_t endcond;     // used to signal completion of all work

    int end_count; // how many threads are done?
};

struct task
{
    void (*f)(void *p);
    void *p;
};

static THREAD_ROUTINE(worker_thread)(void *p)
{
    workerpool_t *wp = (workerpool_t*) p;

    int cnt = 0;

    while (1) {
        struct task *task;

        pthread_mutex_lock(&wp->mutex);
        while (wp->taskspos == zarray_size(wp->tasks)) {
            wp->end_count++;
//          printf("%"PRId64" thread %d did %d\n", utime_now(), pthread_self(), cnt);
            pthread_cond_broadcast(&wp->endcond);
            pthread_cond_wait(&wp->startcond, &wp->mutex);
            cnt = 0;
//            printf("%"PRId64" thread %d awake\n", utime_now(), pthread_self());
        }

        zarray_get_volatile(wp->tasks, wp->taskspos, &task);
        wp->taskspos++;
        cnt++;
        pthread_mutex_unlock(&wp->mutex);
//        pthread_yield();
        sched_yield();

        // we've been asked to exit.
        if (task->f == NULL)
            return 0;

        task->f(task->p);
    }
}

workerpool_t *workerpool_create(int nthreads)
{
    assert(nthreads > 0);

    workerpool_t *wp = calloc(1, sizeof(workerpool_t));
    wp->nthreads = nthreads;
    wp->tasks = zarray_create(sizeof(struct task));
    pthread_mutex_init(&wp->guard, NULL);
    if (nthreads > 1) {
        wp->threads = calloc(wp->nthreads, sizeof(pthread_t));

        pthread_mutex_init(&wp->mutex, NULL);
        pthread_cond_init(&wp->startcond, NULL);
        pthread_cond_init(&wp->endcond, NULL);

        for (int i = 0; i < nthreads; i++) {
            int res = pthread_create(&wp->threads[i], NULL, worker_thread, wp);
            if (res != 0) {
                perror("pthread_create");
                exit(-1);
            }
        }
    }

    return wp;
}

void workerpool_destroy(workerpool_t *wp)
{
    if (wp == NULL)
        return;
    pthread_mutex_destroy(&wp->guard);
    // force all worker threads to exit.
    if (wp->nthreads > 1) {
        for (int i = 0; i < wp->nthreads; i++)
            workerpool_add_task(wp, NULL, NULL);

        pthread_mutex_lock(&wp->mutex);
        pthread_cond_broadcast(&wp->startcond);
        pthread_mutex_unlock(&wp->mutex);

        for (int i = 0; i < wp->nthreads; i++)
            pthread_join(wp->threads[i], NULL);

        pthread_mutex_destroy(&wp->mutex);
        pthread_cond_destroy(&wp->startcond);
        pthread_cond_destroy(&wp->endcond);
        free(wp->threads);
    }

    zarray_destroy(wp->tasks);
    free(wp);
}

void workerpool_lock_guard(workerpool_t *wp)
{
    pthread_mutex_lock(&wp->guard);
}

void workerpool_unlock_guard(workerpool_t *wp)
{
    pthread_mutex_unlock(&wp->guard);
}

int workerpool_get_nthreads(workerpool_t *wp)
{
    return wp->nthreads;
}

void workerpool_add_task(workerpool_t *wp, void (*f)(void *p), void *p)
{
    struct task t;
    t.f = f;
    t.p = p;

    zarray_add(wp->tasks, &t);
}

void workerpool_run_single(workerpool_t *wp)
{
    for (int i = 0; i < zarray_size(wp->tasks); i++) {
        struct task *task;
        zarray_get_volatile(wp->tasks, i, &task);
        task->f(task->p);
    }

    zarray_clear(wp->tasks);
}

// runs all added tasks, waits for them to complete.
void workerpool_run(workerpool_t *wp)
{
    if (wp->nthreads > 1) {
        wp->end_count = 0;

        pthread_mutex_lock(&wp->mutex);
        pthread_cond_broadcast(&wp->startcond);

        while (wp->end_count < wp->nthreads) {
//            printf("caught %d\n", wp->end_count);
            pthread_cond_wait(&wp->endcond, &wp->mutex);
        }

        pthread_mutex_unlock(&wp->mutex);

        wp->taskspos = 0;

        zarray_clear(wp->tasks);

    } else {
        workerpool_run_single(wp);
    }
}

// https://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
#ifdef _WIN32
int workerpool_get_nprocs()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
}
#else
int workerpool_get_nprocs()
{
    FILE * f = fopen("/proc/cpuinfo", "r");
    size_t n = 0;
    char * buf = NULL;

    int nproc = 0;

    while(getline(&buf, &n, f) != -1)
    {
        if(!str_starts_with(buf, "processor"))
            continue;

       int colon = str_indexof(buf, ":");

       int v = atoi(&buf[colon+1]);
       if (v > nproc)
	 nproc = v;
    }

    free(buf);

    return nproc;
}
#endif