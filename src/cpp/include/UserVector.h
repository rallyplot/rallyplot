
#ifndef UserVector_H
#define UserVector_H

#include <iostream>
#include <string>


template<typename T>
class StdPtrVector
/*
    Read-only wrapper around non-owning pointers to expose some
    functionality of std::vector for convenience and backward compatibility.
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

    T operator[](std::size_t index) const
    {

        if (index < 0 || index >= static_cast<int>(m_size))
        {
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
