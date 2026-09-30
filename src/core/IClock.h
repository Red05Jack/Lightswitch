#pragma once

#include <QDateTime>

// Provides the current local date and time so that time dependent logic stays testable.
class IClock {
public:
	virtual ~IClock() = default;

	virtual QDateTime Now() const = 0;
};
