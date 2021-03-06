// Copyright (c) 2014 Yandex LLC. All rights reserved.

#ifndef HANDYSTATS_ATTRIBUTE_EVENT_IMPL_HPP_
#define HANDYSTATS_ATTRIBUTE_EVENT_IMPL_HPP_

#include <string>

#include <handystats/metrics/attribute.hpp>

#include "events/event_message_impl.hpp"


namespace handystats { namespace events { namespace attribute {

namespace event_type {
enum : char {
	SET = 0
};
} // namespace event_type

/*
 * Event creation functions
 */
event_message* create_set_event(
		std::string&& attribute_name,
		const metrics::attribute::value_type& value,
		const metrics::attribute::time_point& timestamp
	);

/*
 * Event destructor
 */
void delete_event(event_message* message);


/*
 * Event processing function
 */
void process_event(metrics::attribute& counter, const event_message& message);

}}} // namespace handystats::events::attribute


#endif // HANDYSTATS_ATTRIBUTE_EVENT_HPP_

