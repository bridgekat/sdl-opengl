#ifndef DEBUG_H_
#define DEBUG_H_

#include <cassert>
#include "logger.h"

inline void Assert(bool expr) {
	if (expr) return;
	// Add a breakpoint here?
	assert(false);
}

inline void Assert(bool expr, std::string errormsg) {
	if (expr) return;
	LogFatal(errormsg);
	// Add a breakpoint here?
	assert(false);
}

#undef assert

#endif // !DEBUG_H_

