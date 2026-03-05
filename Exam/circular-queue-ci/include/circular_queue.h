#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include <cstddef>
#include <concepts>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

template <typename T>
class CircularQueue {
public:
    explicit CircularQueue(std::size_t size) : size_(size) {
        if (size_ <= 2) {
            throw std::invalid_argument("Queue size must be greater than 2");
        }
        create_nodes(size_);
    }

    CircularQueue(const CircularQueue&) = delete;
    CircularQueue& operator=(const CircularQueue&) = delete;

    CircularQueue(CircularQueue&& other) noexcept
        : head_(other.head_), tail_(other.tail_), end_(other.end_), size_(other.size_), count_(other.count_) {
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.end_ = nullptr;
        other.size_ = 0;
        other.count_ = 0;
    }

    CircularQueue& operator=(CircularQueue&& other) noexcept {
        if (this != &other) {
            destroy_nodes();
            head_ = other.head_;
            tail_ = other.tail_;
            end_ = other.end_;
            size_ = other.size_;
            count_ = other.count_;

            other.head_ = nullptr;
            other.tail_ = nullptr;
            other.end_ = nullptr;
            other.size_ = 0;
            other.count_ = 0;
        }
        return *this;
    }

    ~CircularQueue() {
        destroy_nodes();
    }

    void resize(std::size_t new_size) {
        if (new_size <= 2) {
            throw std::invalid_argument("Queue size must be greater than 2");
        }
        if (new_size == size_) {
            return;
        }

        if (new_size < count_) {
            const std::size_t to_drop = count_ - new_size;
            for (std::size_t i = 0; i < to_drop; ++i) {
                discard_oldest();
            }
        }

        if (new_size > size_) {
            add_nodes(new_size - size_);
        } else {
            remove_nodes(size_ - new_size);
        }

        size_ = new_size;
    }

    void make_empty() {
        Node* current = head_;
        for (std::size_t i = 0; i < count_; ++i) {
            current->data.reset();
            current = current->next;
        }
        count_ = 0;
        tail_ = nullptr;
    }

    bool read(T& out) {
        if (count_ == 0) {
            return false;
        }

        out = std::move(*(head_->data));
        head_->data.reset();

        if (count_ == 1) {
            count_ = 0;
            tail_ = nullptr;
            return true;
        }

        head_ = head_->next;
        --count_;
        return true;
    }

    void write(const T& value) {
        write_impl(value);
    }

    void write(T&& value) {
        write_impl(std::move(value));
    }

    std::size_t size() const {
        return size_;
    }

    std::size_t count() const {
        return count_;
    }

    bool is_full() const {
        return count_ == size_;
    }

    template <typename U = T>
        requires std::is_arithmetic_v<U>
    double average() const {
        if (count_ == 0) {
            throw std::underflow_error("Cannot compute average of an empty queue");
        }

        long double sum = 0.0L;
        Node* current = head_;
        for (std::size_t i = 0; i < count_; ++i) {
            sum += static_cast<long double>(*(current->data));
            current = current->next;
        }

        return static_cast<double>(sum / static_cast<long double>(count_));
    }

private:
    struct Node {
        std::optional<T> data;
        Node* next = nullptr;
    };

    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    Node* end_ = nullptr;
    std::size_t size_ = 0;
    std::size_t count_ = 0;

    static void destroy_linear_nodes(Node* first) noexcept {
        while (first != nullptr) {
            Node* next = first->next;
            delete first;
            first = next;
        }
    }

    static std::pair<Node*, Node*> build_linear_nodes(std::size_t count) {
        Node* first = nullptr;
        Node* last = nullptr;
        try {
            for (std::size_t i = 0; i < count; ++i) {
                Node* node = new Node{};
                if (first == nullptr) {
                    first = node;
                } else {
                    last->next = node;
                }
                last = node;
            }
        } catch (...) {
            destroy_linear_nodes(first);
            throw;
        }
        return {first, last};
    }

    void create_nodes(std::size_t count) {
        auto [first, last] = build_linear_nodes(count);
        last->next = first;
        head_ = first;
        end_ = last;
    }

    void destroy_nodes() {
        if (head_ == nullptr || size_ == 0) {
            return;
        }

        Node* current = head_;
        for (std::size_t i = 0; i < size_; ++i) {
            Node* next = current->next;
            delete current;
            current = next;
        }

        head_ = nullptr;
        tail_ = nullptr;
        end_ = nullptr;
        size_ = 0;
        count_ = 0;
    }

    void discard_oldest() {
        if (count_ == 0) {
            return;
        }

        head_->data.reset();
        if (count_ == 1) {
            count_ = 0;
            tail_ = nullptr;
            return;
        }

        head_ = head_->next;
        --count_;
    }

    void add_nodes(std::size_t amount) {
        if (amount == 0) {
            return;
        }

        Node* anchor = (tail_ != nullptr) ? tail_ : end_;
        Node* after = anchor->next;

        auto [first_new, last_new] = build_linear_nodes(amount);

        anchor->next = first_new;
        last_new->next = after;
        if (anchor == end_) {
            end_ = last_new;
        }
    }

    void remove_nodes(std::size_t amount) {
        if (amount == 0) {
            return;
        }

        Node* anchor = (tail_ != nullptr) ? tail_ : end_;
        Node* current = anchor->next;

        for (std::size_t i = 0; i < amount; ++i) {
            Node* next = current->next;
            if (current == end_) {
                end_ = anchor;
            }
            if (current == head_) {
                head_ = next;
            }
            delete current;
            current = next;
        }

        anchor->next = current;
        if (count_ == 0) {
            tail_ = nullptr;
        }
    }

    template <typename U>
    void write_impl(U&& value) {
        if (count_ < size_) {
            Node* target = (count_ == 0) ? head_ : tail_->next;
            target->data = std::forward<U>(value);
            tail_ = target;
            ++count_;
            return;
        }

        head_ = head_->next;
        Node* target = tail_->next;
        target->data = std::forward<U>(value);
        tail_ = target;
    }
};

#endif
