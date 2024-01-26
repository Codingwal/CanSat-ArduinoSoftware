#pragma once

template <typename T>
class Vector
{
private:
    T *m_Data = nullptr;
    size_t m_Size = 0;
    size_t m_Capacity = 0;

private:
    void ReAlloc(size_t newCapacity)
    {
        T *newBlock = new T[newCapacity];

        for (size_t i = 0; i < m_Size; i++)
        {
            newBlock[i] = m_Data[i];
        }

        delete[] m_Data;
        m_Data = newBlock;
        m_Capacity = newCapacity;
    }

public:
    Vector(size_t capacity)
    {
        ReAlloc(capacity);
    }
    ~Vector()
    {
        delete[] m_Data;
    }

    void pushBack(const T &value)
    {
        if (m_Size >= m_Capacity)
        {
            ReAlloc(m_Capacity * 2);
        }
        m_Data[m_Size] = value;
        m_Size++;
    }

    size_t size() const
    {
        return m_Size;
    }

    const T &operator[](size_t index) const
    {
        return m_Data[index];
    }
    T &operator[](size_t index)
    {
        return m_Data[index];
    }
};