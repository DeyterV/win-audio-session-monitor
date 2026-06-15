#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QTimer>

class AudioSessionDock : public QWidget {
	Q_OBJECT

public:
	explicit AudioSessionDock(QWidget *parent = nullptr);

private slots:
	void RefreshSessions();

private:
	QTableWidget *table;
	QPushButton *refreshBtn;
	QCheckBox *autoRefreshCb;
	QTimer *refreshTimer;

	static QString GetExeNameFromPid(DWORD pid);
	static QString StateString(AudioSessionState state);
};
