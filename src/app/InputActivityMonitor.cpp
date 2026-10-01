#include "InputActivityMonitor.h"

#include "NightModeController.h"

#include <QEvent>

InputActivityMonitor::InputActivityMonitor(NightModeController& nightMode, QObject* pParent)
	: QObject(pParent)
	, m_nightMode(nightMode) {
}

// Registers input events and never consumes them.
bool InputActivityMonitor::eventFilter(QObject* pWatched, QEvent* pEvent) {
	switch (pEvent->type()) {
	case QEvent::MouseButtonPress:
	case QEvent::TouchBegin:
	case QEvent::KeyPress:
		m_nightMode.RegisterInput();
		break;
	default:
		break;
	}
	return QObject::eventFilter(pWatched, pEvent);
}
