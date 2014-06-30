// Copyright (c) 2014 Yandex LLC. All rights reserved.

#ifndef HANDYSTATS_COUNTER_EVENT_IMPL_H_
#define HANDYSTATS_COUNTER_EVENT_IMPL_H_

#include <string>
#include <memory>

#include <handystats/metrics/counter.hpp>


namespace handystats { namespace events {

struct event_message;

}} // namespace handystats::events


namespace handystats { namespace events {

namespace counter_event {
enum {
	INIT,
	INCREMENT,
	DECREMENT
};
}


std::shared_ptr<event_message> counter_init_event(
		const std::string& counter_name,
		const metrics::counter::value_type& init_value,
		const metrics::counter::time_point& timestamp
		);

std::shared_ptr<event_message> counter_increment_event(
		const std::string& counter_name,
		const metrics::counter::value_type& value,
		const metrics::counter::time_point& timestamp
		);

std::shared_ptr<event_message> counter_decrement_event(
		const std::string& counter_name,
		const metrics::counter::value_type& value,
		const metrics::counter::time_point& timestamp
		);


void delete_counter_event(event_message* message);

}} // namespace handystats::events


#endif // HANDYSTATS_COUNTER_EVENT_H_