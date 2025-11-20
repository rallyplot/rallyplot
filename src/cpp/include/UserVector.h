
#ifndef UserVector_H
#define UserVector_H

#include <iostream>
#include <string>


template<typename T>
class StdPtrVector
/*
    Basic wrapper around pointers (non-owning) to expose some
    functionality of std::vector for convenience and backward compatability.

// TODO: EXPLAIN
// Yes, std::vector<float>(ptr, ptr + size) does perform a copy of the data.

Copies are by value
 */
{

public:

    StdPtrVector(const T* ptr, std::size_t size) : m_ptr(ptr), m_size(size) {};
    StdPtrVector() : m_ptr(nullptr), m_size(0) {};
    ~StdPtrVector() {};

    StdPtrVector(const StdPtrVector&) = default;
    StdPtrVector& operator=(const StdPtrVector&) = default;
    StdPtrVector(StdPtrVector&&) = default;
    StdPtrVector& operator=(StdPtrVector&&) = default;

    T operator[](int index) const
    {

        if (index < 0 || index >= static_cast<int>(m_size))
        {
            // This is an internal error
            std::cerr << "CRITICAL ERROR: StdPtrVector called with bad index. "
                      << "Index must be positive and less than "  << std::to_string(m_size) << std::endl;
            std::exit(EXIT_FAILURE);
        }
        return m_ptr[index];
    }


    const T* data() const
    {
        return m_ptr;
    }

    const T* begin() const
    {
        return m_ptr;
    }

    const T* end() const
    {
        return m_ptr + m_size;
    }

    const std::size_t size() const
    {
        return m_size;
    }

private:

    const T* m_ptr;
    std::size_t m_size;

};


#endif // UserVector_H
