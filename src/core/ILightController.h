#pragma once

#include <QObject>

// Abstract light switch so that the real hardware backend can be plugged in later.
class ILightController : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool isOn READ IsOn NOTIFY StateChanged)

public:
	explicit ILightController(QObject* pParent = nullptr);
	~ILightController() override = default;

	virtual bool IsOn() const = 0;
	virtual void SetOn(bool isOn) = 0;

	Q_INVOKABLE void Toggle();

signals:
	void StateChanged(bool isOn);
};
