#include <algorithm>
#include <atomic>
#include <bit>
#include <new>

template<typename T, size_t Capacity>
class SPSCRingBuffer {
    static_assert(std::has_single_bit(Capacity), "Capacity must be a power of two");
    static_assert(Capacity > 1, "Capacity must be greater than 1");

    static constexpr size_t Alignment = std::hardware_destructive_interference_size;

    struct alignas(Alignment) PaddedAtomic {
        std::atomic<size_t> value = 0;
    };

    // 每个元素独占缓存行
    struct alignas(Alignment) CacheLineAlignedElement {
        T value;
    };

    PaddedAtomic head_;  // 消费者位置（单调递增）
    PaddedAtomic tail_;  // 生产者位置（单调递增）
    CacheLineAlignedElement buffer_[Capacity] = {};  // 环形存储

public:
    SPSCRingBuffer() = default;

    void push(T&& value) noexcept {
        const size_t current_tail = tail_.value.load(std::memory_order_relaxed);

        // 直接赋值
        buffer_[current_tail % Capacity].value = std::forward<T>(value);

        // 更新tail（保证写入可见性）
        tail_.value.store(current_tail + 1, std::memory_order_release);
    }

    template<typename F>
    void push(F&& f) noexcept {
        const size_t current_tail = tail_.value.load(std::memory_order_relaxed);

        // 通过外部函数赋值
        f(buffer_[current_tail % Capacity].value);

        // 更新tail（保证写入可见性）
        tail_.value.store(current_tail + 1, std::memory_order_release);
    }

    bool pop(T& out) noexcept {
        size_t current_head = head_.value.load(std::memory_order_relaxed);
        const size_t current_tail = tail_.value.load(std::memory_order_acquire);

        // 计算需要跳过的元素数量
        const size_t new_head = current_tail - Capacity;
        // 处理数据覆盖
        if (current_tail >= Capacity) {
            if (current_head < new_head || current_head > current_tail) {
                // 更新head（无需逐个析构，依赖自动管理）
                head_.value.store(new_head, std::memory_order_relaxed);
                current_head = new_head;
            }
        } else {
            if (current_head > current_tail && current_head < new_head) {
                // 更新head（无需逐个析构，依赖自动管理）
                head_.value.store(new_head, std::memory_order_relaxed);
                current_head = new_head;
            }
        }

        if (current_head == current_tail) {
            return false;
        }

        out = buffer_[current_head % Capacity].value;

        // 更新head
        head_.value.store(current_head + 1, std::memory_order_relaxed);
        return true;
    }

    template<typename F>
    bool pop(F&& f) noexcept {
        size_t current_head = head_.value.load(std::memory_order_relaxed);
        const size_t current_tail = tail_.value.load(std::memory_order_acquire);

        // 计算需要跳过的元素数量
        const size_t new_head = current_tail - Capacity;
        // 处理数据覆盖
        if (current_tail >= Capacity) {
            if (current_head < new_head || current_head > current_tail) {
                // 更新head（无需逐个析构，依赖自动管理）
                head_.value.store(new_head, std::memory_order_relaxed);
                current_head = new_head;
            }
        } else {
            if (current_head > current_tail && current_head < new_head) {
                // 更新head（无需逐个析构，依赖自动管理）
                head_.value.store(new_head, std::memory_order_relaxed);
                current_head = new_head;
            }
        }

        if (current_head == current_tail) {
            return false;
        }

        f(buffer_[current_head % Capacity].value);

        // 更新head
        head_.value.store(current_head + 1, std::memory_order_relaxed);
        return true;
    }

    // 元素数量，仅消费者线程调用
    size_t size() const noexcept {
        const size_t tail = tail_.value.load(std::memory_order_acquire);
        const size_t head = head_.value.load(std::memory_order_relaxed);
        return tail - head;
    }
};
