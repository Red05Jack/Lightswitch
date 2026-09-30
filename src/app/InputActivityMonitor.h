#pragma once

#include <QObject>

class NightModeController;

// Application wide event filter that reports every touch, click or key press to the night mode controller.
class InputActivityMonitor : public QObject {
	Q_OBJECT

public:
	explicit InputActivityMonitor(NightModeController& nightMode, QObject* pParent = nullptr);

protected:
	bool eventFilter(QObject* pWatched, QEvent* pEvent) override;

private:
	NightModeController& m_nightMode;
};
