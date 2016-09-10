#include "token.h"

static const tk_type EOI_("END_OF_INPUT");
const tk_type_ptr tk_types::EOI = &EOI_;

static const tk_type UNKNOWN_;
const tk_type_ptr tk_types::UNKNOWN = &UNKNOWN_;

std::ostream &operator<<(std::ostream &o, const tk &b)
{
    return o << b.type()->name() << ":" << b.start_line() << "[" << b.value() << "]";
}
