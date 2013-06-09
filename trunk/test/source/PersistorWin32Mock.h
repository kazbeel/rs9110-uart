#ifndef _PERSISTOR_WIN32_MOCK_H_
#define _PERSISTOR_WIN32_MOCK_H_

#include "IPersistor.h"


class PersistorWin32Mock : public IPersistor
{
public:

    PersistorWin32Mock ();

    virtual ~PersistorWin32Mock ();

    virtual bool Open ();

    virtual bool Close ();

    virtual bool Write (unsigned char *data, unsigned int size);

    virtual bool Read (unsigned char *buffer, unsigned int size);

    char * GetBufferData () const;

    unsigned int GetBufferSize ();


private:

    char           *buffer;
    unsigned int    bufferSize;

};

#endif /* _PERSISTOR_WIN32_MOCK_H_ */