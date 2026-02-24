#include "circular_queue.h"

#include <concepts>
#include <string>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

template <typename Q, typename = void>
struct has_average : std::false_type {};

template <typename Q>
struct has_average<Q, std::void_t<decltype(std::declval<const Q&>().average())>> : std::true_type {};

static_assert(has_average<CircularQueue<int>>::value);
static_assert(!has_average<CircularQueue<std::string>>::value);

TEST(CircularQueueConstructorTest, ThrowsOnInvalidSize) {
    EXPECT_THROW((CircularQueue<int>(2)), std::invalid_argument);
    EXPECT_THROW((CircularQueue<int>(0)), std::invalid_argument);
}

TEST(CircularQueueIntTest, InitialState) {
    CircularQueue<int> q(4);
    EXPECT_EQ(q.size(), 4U);
    EXPECT_EQ(q.count(), 0U);
    EXPECT_FALSE(q.is_full());
}

TEST(CircularQueueIntTest, ReadFromEmptyQueueFails) {
    CircularQueue<int> q(4);
    int value = 0;
    EXPECT_FALSE(q.read(value));
}

TEST(CircularQueueIntTest, WriteReadFifoOrder) {
    CircularQueue<int> q(4);
    q.write(10);
    q.write(20);
    q.write(30);

    int value = 0;
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, 10);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, 20);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, 30);
    EXPECT_FALSE(q.read(value));
}

TEST(CircularQueueIntTest, OverwritesOldestWhenFull) {
    CircularQueue<int> q(3);
    q.write(1);
    q.write(2);
    q.write(3);

    EXPECT_TRUE(q.is_full());

    q.write(4);

    int value = 0;
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, 2);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, 3);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, 4);
}

TEST(CircularQueueIntTest, MakeEmptyClearsWithoutNodeChanges) {
    CircularQueue<int> q(5);
    q.write(1);
    q.write(2);
    q.write(3);

    q.make_empty();

    EXPECT_EQ(q.size(), 5U);
    EXPECT_EQ(q.count(), 0U);

    int value = 0;
    EXPECT_FALSE(q.read(value));

    q.write(42);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, 42);
}

TEST(CircularQueueIntTest, ResizeLargerKeepsDataOrder) {
    CircularQueue<int> q(3);
    q.write(1);
    q.write(2);

    q.resize(6);

    EXPECT_EQ(q.size(), 6U);
    EXPECT_EQ(q.count(), 2U);

    q.write(3);
    q.write(4);

    int value = 0;
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, 1);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, 2);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, 3);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, 4);
}

TEST(CircularQueueIntTest, ResizeSmallerDropsOldestData) {
    CircularQueue<int> q(6);
    q.write(1);
    q.write(2);
    q.write(3);
    q.write(4);
    q.write(5);

    q.resize(3);

    EXPECT_EQ(q.size(), 3U);
    EXPECT_EQ(q.count(), 3U);

    int value = 0;
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, 3);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, 4);
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, 5);
}

TEST(CircularQueueIntTest, ResizeThrowsOnInvalidSize) {
    CircularQueue<int> q(4);
    EXPECT_THROW(q.resize(2), std::invalid_argument);
}

TEST(CircularQueueMoveTest, MoveConstructorTransfersOwnership) {
    CircularQueue<int> source(4);
    source.write(7);
    source.write(8);

    CircularQueue<int> moved(std::move(source));

    EXPECT_EQ(moved.size(), 4U);
    EXPECT_EQ(moved.count(), 2U);

    int value = 0;
    ASSERT_TRUE(moved.read(value));
    EXPECT_EQ(value, 7);
    ASSERT_TRUE(moved.read(value));
    EXPECT_EQ(value, 8);

    EXPECT_EQ(source.size(), 0U);
    EXPECT_EQ(source.count(), 0U);
}

TEST(CircularQueueMoveTest, MoveAssignmentTransfersOwnership) {
    CircularQueue<int> source(4);
    source.write(11);
    source.write(12);

    CircularQueue<int> destination(3);
    destination.write(99);

    destination = std::move(source);

    EXPECT_EQ(destination.size(), 4U);
    EXPECT_EQ(destination.count(), 2U);

    int value = 0;
    ASSERT_TRUE(destination.read(value));
    EXPECT_EQ(value, 11);
    ASSERT_TRUE(destination.read(value));
    EXPECT_EQ(value, 12);

    EXPECT_EQ(source.size(), 0U);
    EXPECT_EQ(source.count(), 0U);
}

TEST(CircularQueueArithmeticTest, AverageForInt) {
    CircularQueue<int> q(5);
    q.write(2);
    q.write(4);
    q.write(6);
    EXPECT_DOUBLE_EQ(q.average(), 4.0);
}

TEST(CircularQueueArithmeticTest, AverageForFloat) {
    CircularQueue<float> q(5);
    q.write(1.5F);
    q.write(2.5F);
    q.write(4.0F);
    EXPECT_DOUBLE_EQ(q.average(), 8.0 / 3.0);
}

TEST(CircularQueueArithmeticTest, AverageThrowsForEmptyQueue) {
    CircularQueue<int> q(4);
    EXPECT_THROW(q.average(), std::underflow_error);
}

TEST(CircularQueueStringTest, WriteReadAndOverwrite) {
    CircularQueue<std::string> q(3);
    q.write("A");
    q.write("B");
    q.write("C");
    q.write("D");

    std::string value;
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, "B");
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, "C");
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, "D");
}

TEST(CircularQueueStringTest, ResizeSmallerDropsOldestStringData) {
    CircularQueue<std::string> q(5);
    q.write("one");
    q.write("two");
    q.write("three");
    q.write("four");

    q.resize(3);

    std::string value;
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, "two");
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, "three");
    ASSERT_TRUE(q.read(value));
    EXPECT_EQ(value, "four");
}
