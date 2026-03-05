#include "circular_queue.h"

#include <concepts>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

template <typename Q, typename = void>
struct has_average : std::false_type {};

template <typename Q>
struct has_average<Q, std::void_t<decltype(std::declval<const Q&>().average())>> : std::true_type {};

static_assert(has_average<CircularQueue<int>>::value);
static_assert(!has_average<CircularQueue<std::string>>::value);

template <typename T>
struct queue_test_data;

template <>
struct queue_test_data<int> {
    static std::vector<int> values() { return {1, 2, 3, 4}; }
};

template <>
struct queue_test_data<float> {
    static std::vector<float> values() { return {1.5F, 2.5F, 3.5F, 4.5F}; }
};

template <>
struct queue_test_data<std::string> {
    static std::vector<std::string> values() { return {"one", "two", "three", "four"}; }
};

template <typename T>
class CircularQueueTypedTest : public ::testing::Test {
protected:
    using value_type = T;
    using queue_type = CircularQueue<T>;

    static std::vector<T> data() { return queue_test_data<T>::values(); }
};

using QueueTypes = ::testing::Types<int, float, std::string>;
TYPED_TEST_SUITE(CircularQueueTypedTest, QueueTypes);

TEST(CircularQueueConstructorTest, ThrowsOnInvalidSize) {
    EXPECT_THROW((CircularQueue<int>(2)), std::invalid_argument);
    EXPECT_THROW((CircularQueue<int>(0)), std::invalid_argument);
}

TYPED_TEST(CircularQueueTypedTest, InitialState) {
    typename TestFixture::queue_type q(4);
    EXPECT_EQ(q.size(), 4U);
    EXPECT_EQ(q.count(), 0U);
    EXPECT_FALSE(q.is_full());
}

TYPED_TEST(CircularQueueTypedTest, ReadFromEmptyQueueFails) {
    typename TestFixture::queue_type q(4);
    typename TestFixture::value_type value{};
    EXPECT_FALSE(q.read(value));
}

TYPED_TEST(CircularQueueTypedTest, WriteReadFifoOrder) {
    typename TestFixture::queue_type q(4);
    const auto values = TestFixture::data();
    q.write(values[0]);
    q.write(values[1]);
    q.write(values[2]);

    typename TestFixture::value_type value{};
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, values[0]);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, values[1]);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, values[2]);
    EXPECT_FALSE(q.read(value));
}

TYPED_TEST(CircularQueueTypedTest, OverwritesOldestWhenFull) {
    typename TestFixture::queue_type q(3);
    const auto values = TestFixture::data();
    q.write(values[0]);
    q.write(values[1]);
    q.write(values[2]);
    q.write(values[3]);

    typename TestFixture::value_type value{};
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, values[1]);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, values[2]);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, values[3]);
}

TYPED_TEST(CircularQueueTypedTest, MakeEmptyClearsWithoutNodeChanges) {
    typename TestFixture::queue_type q(5);
    const auto values = TestFixture::data();
    q.write(values[0]);
    q.write(values[1]);
    q.write(values[2]);

    q.make_empty();
    EXPECT_EQ(q.size(), 5U);
    EXPECT_EQ(q.count(), 0U);

    typename TestFixture::value_type value{};
    EXPECT_FALSE(q.read(value));

    q.write(values[3]);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, values[3]);
}

TYPED_TEST(CircularQueueTypedTest, ResizeLargerKeepsDataOrder) {
    typename TestFixture::queue_type q(3);
    const auto values = TestFixture::data();
    q.write(values[0]);
    q.write(values[1]);
    q.resize(6);
    q.write(values[2]);
    q.write(values[3]);

    EXPECT_EQ(q.size(), 6U);
    EXPECT_EQ(q.count(), 4U);

    typename TestFixture::value_type value{};
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, values[0]);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, values[1]);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, values[2]);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, values[3]);
}

TYPED_TEST(CircularQueueTypedTest, ResizeSmallerDropsOldestData) {
    typename TestFixture::queue_type q(6);
    const auto values = TestFixture::data();
    q.write(values[0]);
    q.write(values[1]);
    q.write(values[2]);
    q.write(values[3]);
    q.resize(3);

    EXPECT_EQ(q.size(), 3U);
    EXPECT_EQ(q.count(), 3U);

    typename TestFixture::value_type value{};
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, values[1]);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, values[2]);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, values[3]);
}

TYPED_TEST(CircularQueueTypedTest, MoveConstructorTransfersOwnership) {
    typename TestFixture::queue_type source(4);
    const auto values = TestFixture::data();
    source.write(values[0]);
    source.write(values[1]);

    typename TestFixture::queue_type moved(std::move(source));
    EXPECT_EQ(moved.size(), 4U);
    EXPECT_EQ(moved.count(), 2U);

    typename TestFixture::value_type value{};
    ASSERT_TRUE(moved.read(value));
    EXPECT_EQ(value, values[0]);
    ASSERT_TRUE(moved.read(value));
    EXPECT_EQ(value, values[1]);

    EXPECT_EQ(source.size(), 0U);
    EXPECT_EQ(source.count(), 0U);
}

TYPED_TEST(CircularQueueTypedTest, MoveAssignmentTransfersOwnership) {
    typename TestFixture::queue_type source(4);
    const auto values = TestFixture::data();
    source.write(values[0]);
    source.write(values[1]);

    typename TestFixture::queue_type destination(3);
    destination.write(values[2]);
    destination = std::move(source);

    EXPECT_EQ(destination.size(), 4U);
    EXPECT_EQ(destination.count(), 2U);

    typename TestFixture::value_type value{};
    ASSERT_TRUE(destination.read(value));
    EXPECT_EQ(value, values[0]);
    ASSERT_TRUE(destination.read(value));
    EXPECT_EQ(value, values[1]);

    EXPECT_EQ(source.size(), 0U);
    EXPECT_EQ(source.count(), 0U);
}

TEST(CircularQueueResizeTest, ResizeThrowsOnInvalidSize) {
    CircularQueue<int> q(4);
    EXPECT_THROW(q.resize(2), std::invalid_argument);
}

TEST(CircularQueueArithmeticTest, AverageForArithmeticTypes) {
    CircularQueue<int> qi(5);
    qi.write(2);
    qi.write(4);
    qi.write(6);
    EXPECT_DOUBLE_EQ(qi.average(), 4.0);

    CircularQueue<float> qf(5);
    qf.write(1.5F);
    qf.write(2.5F);
    qf.write(4.0F);
    EXPECT_DOUBLE_EQ(qf.average(), 8.0 / 3.0);
}

TEST(CircularQueueArithmeticTest, AverageThrowsForEmptyQueue) {
    CircularQueue<int> q(4);
    EXPECT_THROW(q.average(), std::underflow_error);
}
