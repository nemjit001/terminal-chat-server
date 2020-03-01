#include "CircularLineBuffer.h"

int CircularLineBuffer::getShiftedIndex(int index)
{
    return (index + this->start) % this->buffer_size;
}

CircularLineBuffer::CircularLineBuffer()
{
    this->start = 0;
    this->count = 0;
    this->buffer_size = 4096;
    this->buffer = (char *)calloc(this->buffer_size, sizeof(char));
}

CircularLineBuffer::~CircularLineBuffer()
{
    free(this->buffer);
}

bool CircularLineBuffer::_write(const char *chars, int n_chars)
{
    if (n_chars > this->nFree())
    {
        return false;
    }

    int startIndex = this->nextFreeIndex();

    for (int i = 0; i < n_chars; i++)
    {
        int shiftedIndex = this->getShiftedIndex(i);
        this->buffer[shiftedIndex] = chars[i];

        this->count++;
    }

    return true;
}

std::string CircularLineBuffer::_read()
{
    std::string line;

    if (this->isEmpty()) return "";
    if (!this->hasLine()) return "";
    
    int endIndex = this->findNewline();

    for (int i = 0; i < endIndex + 1; i++)
    {
        int shiftedIndex = this->getShiftedIndex(i);
        line += this->buffer[shiftedIndex];
        this->buffer[shiftedIndex] = '\0';
        this->count--;
    }

    this->start = endIndex;

    return line;
}

bool CircularLineBuffer::write(const char *chars, int n_chars)
{
    buffer_mtx.lock();
    bool res = this->_write(chars, n_chars);
    buffer_mtx.unlock();
    return res;
}

std::string CircularLineBuffer::read()
{
    buffer_mtx.lock();
    std::string res = this->_read();
    buffer_mtx.unlock();
    return res;
}

int CircularLineBuffer::nFree()
{
    return this->buffer_size - this->count;
}

bool CircularLineBuffer::isFull()
{
    return this->count == this->buffer_size;
}

bool CircularLineBuffer::isEmpty()
{
    return this->count == 0;
}

int CircularLineBuffer::nextFreeIndex()
{
    for (int i = 0; i < this->buffer_size; i++)
    {
        if (buffer[this->getShiftedIndex(i)] == '\0') return i;
    }
    return -1;
}

int CircularLineBuffer::findNewline()
{
    for (int i = 0; i < this->buffer_size; i++)
    {
        if (buffer[this->getShiftedIndex(i)] == '\n') return i;
    }
    return -1;
}

bool CircularLineBuffer::hasLine()
{
    return this->findNewline();
}
