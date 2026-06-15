#include "audio-session-dock.hpp"

#include <obs-module.h>
#include <util/windows/ComPtr.hpp>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidgetItem>

AudioSessionDock::AudioSessionDock(QWidget *parent) : QWidget(parent)
{
	setMinimumWidth(400);

	auto *layout = new QVBoxLayout(this);
	layout->setContentsMargins(4, 4, 4, 4);
	layout->setSpacing(4);

	table = new QTableWidget(0, 4, this);
	table->setHorizontalHeaderLabels({"Executable", "PID", "Status", "Volume"});
	table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	table->verticalHeader()->setVisible(false);
	table->setAlternatingRowColors(true);
	layout->addWidget(table);

	auto *bottomBar = new QHBoxLayout();
	refreshBtn = new QPushButton("Refresh", this);
	autoRefreshCb = new QCheckBox("Auto-refresh", this);
	autoRefreshCb->setChecked(true);
	bottomBar->addWidget(refreshBtn);
	bottomBar->addWidget(autoRefreshCb);
	bottomBar->addStretch();
	layout->addLayout(bottomBar);

	refreshTimer = new QTimer(this);
	refreshTimer->setInterval(3000);

	connect(refreshBtn, &QPushButton::clicked, this, &AudioSessionDock::RefreshSessions);

	connect(autoRefreshCb, &QCheckBox::toggled, this, [this](bool checked) {
		if (checked)
			refreshTimer->start();
		else
			refreshTimer->stop();
	});

	connect(refreshTimer, &QTimer::timeout, this, &AudioSessionDock::RefreshSessions);

	refreshTimer->start();
	RefreshSessions();
}

QString AudioSessionDock::GetExeNameFromPid(DWORD pid)
{
	if (pid == 0)
		return QStringLiteral("System");

	HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (!h)
		return QString("(PID %1)").arg(pid);

	wchar_t path[MAX_PATH];
	DWORD size = MAX_PATH;
	QString name;

	if (QueryFullProcessImageNameW(h, 0, path, &size)) {
		wchar_t *fn = wcsrchr(path, L'\\');
		name = QString::fromWCharArray(fn ? fn + 1 : path);
	} else {
		name = QString("(PID %1)").arg(pid);
	}

	CloseHandle(h);
	return name;
}

QString AudioSessionDock::StateString(AudioSessionState state)
{
	switch (state) {
	case AudioSessionStateActive:
		return QStringLiteral("Active");
	case AudioSessionStateInactive:
		return QStringLiteral("Inactive");
	case AudioSessionStateExpired:
		return QStringLiteral("Expired");
	default:
		return QStringLiteral("Unknown");
	}
}

void AudioSessionDock::RefreshSessions()
{
	table->setRowCount(0);

	ComPtr<IMMDeviceEnumerator> enumerator;
	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
				      __uuidof(IMMDeviceEnumerator), (void **)enumerator.Assign());
	if (FAILED(hr)) {
		blog(LOG_WARNING, "[AudioSessionMonitor] CoCreateInstance IMMDeviceEnumerator failed: %lX",
		     hr);
		return;
	}

	ComPtr<IMMDevice> device;
	hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, device.Assign());
	if (FAILED(hr))
		return;

	ComPtr<IAudioSessionManager2> manager;
	hr = device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr,
			      (void **)manager.Assign());
	if (FAILED(hr))
		return;

	ComPtr<IAudioSessionEnumerator> sessionEnum;
	hr = manager->GetSessionEnumerator(sessionEnum.Assign());
	if (FAILED(hr))
		return;

	int count = 0;
	sessionEnum->GetCount(&count);

	for (int i = 0; i < count; i++) {
		ComPtr<IAudioSessionControl> ctrl;
		if (FAILED(sessionEnum->GetSession(i, ctrl.Assign())))
			continue;

		ComPtr<IAudioSessionControl2> ctrl2;
		if (FAILED(ctrl->QueryInterface(__uuidof(IAudioSessionControl2),
					        (void **)ctrl2.Assign())))
			continue;

		DWORD pid = 0;
		ctrl2->GetProcessId(&pid);

		AudioSessionState state = AudioSessionStateExpired;
		ctrl->GetState(&state);

		ComPtr<ISimpleAudioVolume> vol;
		float masterVolume = -1.0f;
		if (SUCCEEDED(ctrl->QueryInterface(__uuidof(ISimpleAudioVolume), (void **)vol.Assign())))
			vol->GetMasterVolume(&masterVolume);

		QString exeName = GetExeNameFromPid(pid);
		QString stateStr = StateString(state);
		QString volStr = masterVolume >= 0.0f ? QString("%1%").arg((int)(masterVolume * 100))
						      : QStringLiteral("—");

		int row = table->rowCount();
		table->insertRow(row);
		table->setItem(row, 0, new QTableWidgetItem(exeName));
		table->setItem(row, 1, new QTableWidgetItem(QString::number(pid)));
		table->setItem(row, 2, new QTableWidgetItem(stateStr));
		table->setItem(row, 3, new QTableWidgetItem(volStr));
	}
}
