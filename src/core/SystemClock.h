#pragma once

#include "IClock.h"

// Clock backed by the operating system time.
class SystemClock : public IClock {
public:
	QDateTime Now() const override;
};
