#ifndef _I_PERSISTOR_H_
#define _I_PERSISTOR_H_


class IPersistor
{
public:

    virtual bool Open () = 0;
    
    virtual bool Close () = 0;

    virtual bool Write (unsigned char *data, unsigned int size) = 0;

    virtual bool Read (unsigned char *buffer, unsigned int size) = 0;
    
};

#endif /* _I_PERSISTOR_H_ */