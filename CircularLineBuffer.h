#ifndef CPP_CIRCULAR_LINE_BUFFER_H
#define CPP_CIRCULAR_LINE_BUFFER_H

#include <mutex>
#include <string>

class CircularLineBuffer
{
private:
    std::mutex buffer_mtx;
    char *buffer;
    int buffer_size;
    int start, count;

    bool _write(const char *chars, int n_chars);
    std::string _read();
    int getShiftedIndex(int index);
public:
    CircularLineBuffer();
    ~CircularLineBuffer();

    bool write(const char *chars, int n_chars);
    std::string read();
    int nFree();
    bool isFull();
    bool isEmpty();
    int nextFreeIndex();
    int findNewline();
    bool hasLine();
};


#endif