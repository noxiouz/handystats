#include <vector>
#include <utility>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <queue>
#include <string>
#include <map>
#include <memory>

#include <gtest/gtest.h>

#include <handystats/operation.hpp>
#include <handystats/measuring_points.hpp>
#include <handystats/json_dump.hpp>

#include "events/event_message_impl.hpp"
#include "message_queue_impl.hpp"
#include "internal_metrics_impl.hpp"
#include "internal_metrics/internal_counter_impl.hpp"

#include "message_queue_helper.hpp"

namespace handystats { namespace internal {

extern std::map<std::string, internal_metric> internal_metrics;

}} // namespace handystats::internal


class HandyScopedCounterTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		HANDY_INIT();
	}
	virtual void TearDown() {
		HANDY_FINALIZE();
	}
};

TEST_F(HandyScopedCounterTest, TestSingleScope) {
	const int COUNT = 10;

	for (int step = 0; step < COUNT; ++step) {
		HANDY_COUNTER_SCOPE("test.counter", 1);
	}

	handystats::message_queue::wait_until_empty();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	auto agg_stats =
		boost::get<handystats::internal::internal_counter*>(handystats::internal::internal_metrics["test.counter"])
		->base_counter
		->values;

	ASSERT_EQ(agg_stats.count(), 2 * COUNT + 1);
	ASSERT_EQ(agg_stats.min(), 0);
	ASSERT_EQ(agg_stats.max(), 1);

	std::cout << *HANDY_JSON_DUMP() << std::endl;
}

TEST_F(HandyScopedCounterTest, TestDoubleNestedScope) {
	const int COUNT = 10;

	for (int step = 0; step < COUNT; ++step) {
		HANDY_COUNTER_SCOPE("test.counter", 1);
		{
			HANDY_COUNTER_SCOPE("test.counter", -1);
		}
	}

	handystats::message_queue::wait_until_empty();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	auto agg_stats =
		boost::get<handystats::internal::internal_counter*>(handystats::internal::internal_metrics["test.counter"])
		->base_counter
		->values;

	ASSERT_EQ(agg_stats.count(), 4 * COUNT + 1);
	ASSERT_EQ(agg_stats.min(), 0);
	ASSERT_EQ(agg_stats.max(), 1);

	std::cout << *HANDY_JSON_DUMP() << std::endl;
}