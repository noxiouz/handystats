// Copyright (c) 2014 Yandex LLC. All rights reserved.

#include <handystats/atomic.hpp>
#include <algorithm>

#include <handystats/chrono.hpp>
#include <handystats/metrics/timer.hpp>

#include "events/event_message_impl.hpp"
#include "config_impl.hpp"

#include "message_queue_impl.hpp"

namespace {

/*
 * http://www.1024cores.net/home/lock-free-algorithms/queues/intrusive-mpsc-node-based-queue
 */
struct __event_message_queue
{
	typedef handystats::message_queue::node node;

	__event_message_queue()
		: m_head_node(&m_stub_node)
		, m_tail_node(&m_stub_node)
	{
		m_stub_node.next.store(nullptr, std::memory_order_release);
	}

	void push(node* n)
	{
		n->next.store(nullptr, std::memory_order_release);
		node* prev = m_head_node.exchange(n, std::memory_order_acquire);
		prev->next.store(static_cast<handystats::events::event_message*>(n), std::memory_order_release);
	}

	node* pop()
	{
		node* tail = m_tail_node;
		node* next = tail->next.load(std::memory_order_acquire);

		if (tail == &m_stub_node) {
			if (next == nullptr) {
				return nullptr;
			}

			m_tail_node = next;
			tail = next;
			next = next->next.load(std::memory_order_acquire);
		}

		if (next) {
			m_tail_node = next;
			return tail;
		}

		node* head = m_head_node.load(std::memory_order_acquire);

		if (tail != head) {
			return nullptr;
		}

		push(&m_stub_node);

		next = tail->next.load(std::memory_order_acquire);

		if (next) {
			m_tail_node = next;
			return tail;
		}

		return nullptr;
	}

	~__event_message_queue() {
		// what if queue is not empty?
		// this will be memory leak for sure
	}

private:
	std::atomic<node*> m_head_node;
	node* m_tail_node;
	node m_stub_node;
};

} // unnamed namespace


namespace handystats { namespace message_queue {


namespace stats {

metrics::gauge size;
metrics::gauge mem_consume;
metrics::gauge message_wait_time;
metrics::counter pop_count;

void update(const chrono::time_point& timestamp) {
	size.update_statistics(timestamp);
	mem_consume.update_statistics(timestamp);
	message_wait_time.update_statistics(timestamp);
	pop_count.update_statistics(timestamp);
}

static void reset() {
	config::metrics::gauge size_opts;
	size_opts.values.tags =
		statistics::tag::value | statistics::tag::max |
		statistics::tag::moving_avg
		;
	size_opts.values.moving_interval = chrono::duration(1, chrono::time_unit::SEC);

	size = metrics::gauge(size_opts);
	size.set(0);

	config::metrics::gauge mem_consume_opts;
	mem_consume_opts.values.tags =
		statistics::tag::value | statistics::tag::max |
		statistics::tag::moving_avg
		;
	mem_consume_opts.values.moving_interval = chrono::duration(1, chrono::time_unit::SEC);

	mem_consume = metrics::gauge(mem_consume_opts);
	mem_consume.set(0);

	config::metrics::gauge message_wait_time_opts;
	message_wait_time_opts.values.tags =
		statistics::tag::moving_avg
		;
	message_wait_time_opts.values.moving_interval = chrono::duration(1, chrono::time_unit::SEC);

	message_wait_time = metrics::gauge(message_wait_time_opts);

	config::metrics::counter pop_count_opts;

	pop_count_opts.values.tags = statistics::tag::rate | statistics::tag::value;
	pop_count_opts.values.rate_unit = chrono::time_unit::SEC;
	pop_count_opts.values.moving_interval = chrono::duration(1, chrono::time_unit::SEC);

	pop_count = metrics::counter(pop_count_opts);
}

void initialize() {
	reset();
}

void finalize() {
	reset();
}

} // namespace stats


__event_message_queue* event_message_queue = nullptr;

std::atomic<size_t> mq_size(0);
std::atomic<size_t> mq_mem_consume(0);

void push(node* n) {
	if (event_message_queue) {
		events::event_message* message = static_cast<events::event_message*>(n);

		++mq_size;
		size_t message_mem_consume = sizeof(*message);
		message_mem_consume += message->destination_name.size();
		mq_mem_consume += message_mem_consume;

		event_message_queue->push(n);
	}
}

events::event_message* pop() {
	events::event_message* message = nullptr;

	if (event_message_queue) {
		message = static_cast<events::event_message*>(event_message_queue->pop());
	}

	if (message) {
		--mq_size;

		size_t message_mem_consume = sizeof(*message);
		message_mem_consume += message->destination_name.size();
		mq_mem_consume -= message_mem_consume;

		auto current_time = chrono::tsc_clock::now();
		stats::size.set(size(), current_time);
		stats::mem_consume.set(mq_mem_consume.load(std::memory_order_acquire), current_time);
		stats::pop_count.increment(1, current_time);

		stats::message_wait_time.set(
				chrono::duration::convert_to(metrics::timer::value_unit, current_time - message->timestamp).count(),
				current_time
			);
	}

	return message;
}

bool empty() {
	return mq_size.load(std::memory_order_acquire) == 0;
}

size_t size() {
	return mq_size.load(std::memory_order_acquire);
}

void initialize() {
	if (!event_message_queue) {
		event_message_queue = new __event_message_queue();
		mq_size.store(0, std::memory_order_release);
		mq_mem_consume.store(0, std::memory_order_release);
	}

	stats::initialize();
}

void finalize() {
	while (!empty()) {
		auto* message = pop();
		events::delete_event_message(message);
	}
	mq_size.store(0);
	mq_mem_consume.store(0);

	delete event_message_queue;
	event_message_queue = nullptr;

	stats::finalize();
}

}} // namespace handystats::message_queue
