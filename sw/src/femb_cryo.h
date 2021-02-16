#ifndef femb_cryo_h
#define femb_cryo_h

#include <cstddef>
#include "io_reg.h"
#include "log.h"

class FEMB_CRYO {

public:
    // Initialize a FEMB index [0-3]
    FEMB_CRYO(int index);
    ~FEMB_CRYO();

protected:

    // The index'th FEMB connected to this WIB
    int index;
    
};

#endif
