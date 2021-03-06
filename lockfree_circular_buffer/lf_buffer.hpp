/**
 * Implementation of Naive and Lock-free ring-buffer queues and
 * performance and verification tests.
 *
 * Build with (g++ version must be >= 4.5.0):
 * $ g++ -Wall -std=c++0x -O2 -D DCACHE1_LINESIZE=`getconf LEVEL1_DCACHE_LINESIZE` lockfree_rb_q.cc -lpthread
 *
 * I verified the program with g++ 4.5.3, 4.6.1, 4.6.3 and 4.8.1.
 *
 * Use -std=c++11 instead of -std=c++0x for g++ 4.8.
 *
 * Copyright (C) 2012-2013 Alexander Krizhanovsky (ak@natsys-lab.com).
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 * See http://www.gnu.org/licenses/lgpl.html .
 */
#ifndef __x86_64__
#warning "The program is developed for x86-64 architecture only."
#endif
#if !defined(DCACHE1_LINESIZE) || !DCACHE1_LINESIZE
#ifdef DCACHE1_LINESIZE
#undef DCACHE1_LINESIZE
#endif
#define ____cacheline_aligned	__attribute__((aligned(64)))
#endif
#define ____cacheline_aligned	__attribute__((aligned(DCACHE1_LINESIZE)))

#include <sys/time.h>
#include <limits.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <immintrin.h>
#include <stdlib.h>

//#include <atomic>
#include <cassert>
#include <cstdlib>
#include <iostream>
//#include <thread>

#define QUEUE_SIZE 1024


/*
 * ------------------------------------------------------------------------
 * Lock-free N-producers M-consumers ring-buffer queue.
 * ABA problem safe.
 *
 * This implementation is bit complicated, so possibly it has sense to use
 * classic list-based queues. See:
 * 1. D.Fober, Y.Orlarey, S.Letz, "Lock-Free Techniques for Concurrent
 *    Access to Shared Ojects"
 * 2. M.M.Michael, M.L.Scott, "Simple, Fast and Practical Non-Blocking and
 *    Blocking Concurrent Queue Algorithms"
 * 3. E.Ladan-Mozes, N.Shavit, "An Optimistic Approach to Lock-Free FIFO Queues"
 *
 * See also implementation of N-producers M-consumers FIFO and
 * 1-producer 1-consumer ring-buffer from Tim Blechmann:
 *	http://tim.klingt.org/boost_lockfree/
 *	git://tim.klingt.org/boost_lockfree.git
 * 
 * See See Intel 64 and IA-32 Architectures Software Developer's Manual,
 * Volume 3, Chapter 8.2 Memory Ordering for x86 memory ordering guarantees.
 * ------------------------------------------------------------------------
 */

size_t max(size_t a, size_t b){
    return a >= b ? a : b;
}

size_t min(size_t a, size_t b){
    return a<= b ? a : b;
}


template<class T,
	unsigned long Q_SIZE = QUEUE_SIZE>
class LockFreeQueue {
private:
	static const unsigned long Q_MASK = Q_SIZE - 1;

	struct ThrPos {
		unsigned long head, tail;
	};

public:
	LockFreeQueue(size_t n_producers, size_t n_consumers)
		: n_producers_(n_producers),
		n_consumers_(n_consumers),
		head_(0),
		tail_(0),
		last_head_(0),
		last_tail_(0)
	{
		size_t n = max(n_consumers_, n_producers_);
		//thr_p_ = (ThrPos *)::memalign(getpagesize(), sizeof(ThrPos) * n);
		::posix_memalign((void **)&thr_p_, getpagesize(), sizeof(ThrPos) * n);
                assert(thr_p_);
		// Set per thread tail and head to ULONG_MAX.
		::memset((void *)thr_p_, 0xFF, sizeof(ThrPos) * n);

		::posix_memalign((void **)&ptr_array_, getpagesize(),
				Q_SIZE * sizeof(void *));
		assert(ptr_array_);
	}

	~LockFreeQueue()
	{
		::free(ptr_array_);
		::free(thr_p_);
	}

	ThrPos&
	thr_pos(size_t id) const
	{
		assert(id < max(n_consumers_, n_producers_));
		return thr_p_[id];
	}

	void
	push(T *ptr, size_t id)
	{
		/*
		 * Request next place to push.
		 *
		 * Second assignemnt is atomic only for head shift, so there is
		 * a time window in which thr_p_[tid].head = ULONG_MAX, and
		 * head could be shifted significantly by other threads,
		 * so pop() will set last_head_ to head.
		 * After that thr_p_[tid].head is setted to old head value
		 * (which is stored in local CPU register) and written by @ptr.
		 *
		 * First assignment guaranties that pop() sees values for
		 * head and thr_p_[tid].head not greater that they will be
		 * after the second assignment with head shift.
		 *
		 * Loads and stores are not reordered with locked instructions,
		 * se we don't need a memory barrier here.
		 */
		thr_pos(id).head = head_;
		thr_pos(id).head = __sync_fetch_and_add(&head_, 1);

		/*
		 * We do not know when a consumer uses the pop()'ed pointer,
		 * se we can not overwrite it and have to wait the lowest tail.
		 */
		while (__builtin_expect(thr_pos(id).head >= last_tail_ + Q_SIZE, 0))
		{
			unsigned long min = tail_;

			// Update the last_tail_.
			for (size_t i = 0; i < n_consumers_; ++i) {
				unsigned long tmp_t = thr_p_[i].tail;

				// Force compiler to use tmp_h exactly once.
				asm volatile("" ::: "memory");

				if (tmp_t < min)
					min = tmp_t;
			}
			last_tail_ = min;

			if (thr_pos(id).head < last_tail_ + Q_SIZE)
				break;
			//_mm_pause();
                        usleep(50);
		}

		ptr_array_[thr_pos(id).head & Q_MASK] = ptr;

		// Allow consumers eat the item.
		thr_pos(id).head = ULONG_MAX;
	}

	T *
	pop(size_t id)
	{
		/*
		 * Request next place from which to pop.
		 * See comments for push().
		 *
		 * Loads and stores are not reordered with locked instructions,
		 * se we don't need a memory barrier here.
		 */
		thr_pos(id).tail = tail_;
		thr_pos(id).tail = __sync_fetch_and_add(&tail_, 1);

		/*
		 * tid'th place in ptr_array_ is reserved by the thread -
		 * this place shall never be rewritten by push() and
		 * last_tail_ at push() is a guarantee.
		 * last_head_ guaraties that no any consumer eats the item
		 * before producer reserved the position writes to it.
		 */
		while (__builtin_expect(thr_pos(id).tail >= last_head_, 0))
		{
			unsigned long min = head_;

			// Update the last_head_.
			for (size_t i = 0; i < n_producers_; ++i) {
				unsigned long tmp_h = thr_p_[i].head;

				// Force compiler to use tmp_h exactly once.
				asm volatile("" ::: "memory");

				if (tmp_h < min)
					min = tmp_h;
			}
			last_head_ = min;

			if (thr_pos(id).tail < last_head_)
				break;
			//_mm_pause();
                        usleep(50);
		}

		T *ret = ptr_array_[thr_pos(id).tail & Q_MASK];
		// Allow producers rewrite the slot.
		thr_pos(id).tail = ULONG_MAX;
		return ret;
	}

private:
	/*
	 * The most hot members are cacheline aligned to avoid
	 * False Sharing.
	 */

	const size_t n_producers_, n_consumers_;
	// currently free position (next to insert)
	unsigned long	head_ ____cacheline_aligned;
	// current tail, next to pop
	unsigned long	tail_ ____cacheline_aligned;
	// last not-processed producer's pointer
	unsigned long	last_head_ ____cacheline_aligned;
	// last not-processed consumer's pointer
	unsigned long	last_tail_ ____cacheline_aligned;
	ThrPos		*thr_p_;
	T		**ptr_array_;
};



