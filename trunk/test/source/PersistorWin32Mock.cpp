#include "PersistorWin32Mock.h"

#include <string.h>



PersistorWin32Mock::PersistorWin32Mock ()
{
    buffer = new char[2000];
}


PersistorWin32Mock::~PersistorWin32Mock ()
{
    if(buffer != 0)
    {
        delete buffer;
    }
}


bool PersistorWin32Mock::Open ()
{
    return true;
}


bool PersistorWin32Mock::Close ()
{
    return true;
}


bool PersistorWin32Mock::Write (unsigned char *data, unsigned int size)
{
    memcpy(buffer, data, size);
    buffer[size] = '\0';
    bufferSize = size;

    return true;
}


bool PersistorWin32Mock::Read (unsigned char *buffer, unsigned int size)
{
    return true;
}


char * PersistorWin32Mock::GetBufferData () const
{
    return buffer;
}

unsigned int PersistorWin32Mock::GetBufferSize ()
{
    return bufferSize;
}
