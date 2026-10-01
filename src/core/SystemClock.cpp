#include "SystemClock.h"

// Returns the current local time of the operating system.
QDateTime SystemClock::Now() const {
	return QDateTime::currentDateTime();
}
