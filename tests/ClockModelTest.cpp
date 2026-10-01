#include "ClockModel.h"
#include "FakeClock.h"

#include <QSignalSpy>
#include <QtTest>

class ClockModelTest : public QObject {
	Q_OBJECT

private slots:
	void FormatsTimeAndDate();
	void EmitsChangedOnlyWhenTextChanges();
};

void ClockModelTest::FormatsTimeAndDate() {
	FakeClock clock(QDateTime(QDate(2026, 9, 17), QTime(0, 24, 30)));
	ClockModel model(clock);

	QCOMPARE(model.TimeText(), QStringLiteral("00:24"));
	QCOMPARE(model.DateText(), QStringLiteral("Thursday sep 17"));
}

void ClockModelTest::EmitsChangedOnlyWhenTextChanges() {
	FakeClock clock(QDateTime(QDate(2026, 9, 17), QTime(0, 24, 0)));
	ClockModel model(clock);
	QSignalSpy spy(&model, &ClockModel::Changed);

	clock.SetNow(QDateTime(QDate(2026, 9, 17), QTime(0, 24, 30)));
	model.Refresh();
	QCOMPARE(spy.count(), 0);

	clock.SetNow(QDateTime(QDate(2026, 9, 17), QTime(0, 25, 0)));
	model.Refresh();
	QCOMPARE(spy.count(), 1);
	QCOMPARE(model.TimeText(), QStringLiteral("00:25"));
}

QTEST_GUILESS_MAIN(ClockModelTest)
#include "ClockModelTest.moc"
