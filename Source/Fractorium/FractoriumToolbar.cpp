#include "FractoriumPch.h"
#include "Fractorium.h"
#include "QssDialog.h"

/// <summary>
/// Initialize the toolbar UI.
/// </summary>
void Fractorium::InitToolbarUI()
{
	auto clGroup = new QActionGroup(this);
	clGroup->addAction(ui.ActionCpu);
	clGroup->addAction(ui.ActionCL);
	auto spGroup = new QActionGroup(this);
	spGroup->addAction(ui.ActionSP);
	spGroup->addAction(ui.ActionDP);
	SyncOptionsToToolbar();
	ui.ActionDrawImage->setChecked(true);
	m_PreviousAffineState[int(eAffineState::PRE)     ] = true;
	m_PreviousAffineState[int(eAffineState::ALL_PRE) ] = true;
	m_PreviousAffineState[int(eAffineState::POST)    ] = false;
	m_PreviousAffineState[int(eAffineState::ALL_POST)] = false;
	connect(ui.ActionCpu,                SIGNAL(triggered(bool)), this, SLOT(OnActionCpu(bool)),               Qt::ConnectionType::QueuedConnection);
	connect(ui.ActionCL,                 SIGNAL(triggered(bool)), this, SLOT(OnActionCL(bool)),                Qt::ConnectionType::QueuedConnection);
	connect(ui.ActionSP,                 SIGNAL(triggered(bool)), this, SLOT(OnActionSP(bool)),                Qt::ConnectionType::QueuedConnection);
	connect(ui.ActionDP,                 SIGNAL(triggered(bool)), this, SLOT(OnActionDP(bool)),                Qt::ConnectionType::QueuedConnection);
	connect(ui.ActionCompat,             SIGNAL(triggered(bool)), this, SLOT(OnActionCompat(bool)),            Qt::ConnectionType::QueuedConnection);
	connect(ui.ActionStyle,              SIGNAL(triggered(bool)), this, SLOT(OnActionStyle(bool)),             Qt::ConnectionType::QueuedConnection);
	connect(ui.ActionStartStopRenderer,  SIGNAL(triggered(bool)), this, SLOT(OnActionStartStopRenderer(bool)), Qt::ConnectionType::QueuedConnection);
	connect(ui.ActionDrawImage,          SIGNAL(triggered(bool)), this, SLOT(OnActionDrawImage(bool)),         Qt::ConnectionType::QueuedConnection);
	connect(ui.ActionDrawPreAffines,     SIGNAL(triggered(bool)), this, SLOT(OnActionDrawAffines(bool)),       Qt::ConnectionType::QueuedConnection);
	connect(ui.ActionDrawPostAffines,    SIGNAL(triggered(bool)), this, SLOT(OnActionDrawAffines(bool)),       Qt::ConnectionType::QueuedConnection);
	connect(ui.ActionDrawAllPreAffines,  SIGNAL(triggered(bool)), this, SLOT(OnActionDrawAllAffines(bool)),    Qt::ConnectionType::QueuedConnection);
	connect(ui.ActionDrawAllPostAffines, SIGNAL(triggered(bool)), this, SLOT(OnActionDrawAllAffines(bool)),    Qt::ConnectionType::QueuedConnection);
	connect(ui.ActionDrawGrid,           SIGNAL(triggered(bool)), this, SLOT(OnActionDrawGrid(bool)),          Qt::ConnectionType::QueuedConnection);
}

/// <summary>
/// GUI wrapper functions, getters only.
/// </summary>

bool Fractorium::DrawPreAffines()   { return ui.ActionDrawPreAffines->isChecked();  }
bool Fractorium::DrawPostAffines()  { return ui.ActionDrawPostAffines->isChecked();  }
bool Fractorium::DrawSelectedPre()  { return !ui.ActionDrawAllPreAffines->isChecked();  }
bool Fractorium::DrawAllPre()       { return ui.ActionDrawAllPreAffines->isChecked();  }
bool Fractorium::DrawSelectedPost() { return !ui.ActionDrawAllPostAffines->isChecked();  }
bool Fractorium::DrawAllPost()      { return ui.ActionDrawAllPostAffines->isChecked(); }
bool Fractorium::DrawXforms()       { return ui.ActionDrawPreAffines->isChecked() || ui.ActionDrawPostAffines->isChecked(); }
bool Fractorium::DrawImage()        { return ui.ActionDrawImage->isChecked();  }
bool Fractorium::DrawGrid()         { return ui.ActionDrawGrid->isChecked();   }

/// <summary>
/// Called when the CPU render option on the toolbar is clicked.
/// </summary>
/// <param name="checked">Check state, action only taken if true.</param>
void Fractorium::OnActionCpu(bool checked)
{
	if (checked && m_Settings->OpenCL())
	{
		m_Settings->OpenCL(false);
		ShutdownAndRecreateFromOptions(false);
	}
}

/// <summary>
/// Called when the OpenCL render option on the toolbar is clicked.
/// </summary>
/// <param name="checked">Check state, action only taken if true.</param>
void Fractorium::OnActionCL(bool checked)
{
	if (checked && !m_Settings->OpenCL())
	{
		m_Settings->OpenCL(true);
		ShutdownAndRecreateFromOptions(false);
	}
}

/// <summary>
/// Called when the single precision render option on the toolbar is clicked.
/// </summary>
/// <param name="checked">Check state, action only taken if true.</param>
void Fractorium::OnActionSP(bool checked)
{
	if (checked && m_Settings->Double())
	{
		m_Settings->Double(false);
		ShutdownAndRecreateFromOptions(true);//Pass true, but it's not needed because creating a new controller will force a library tree re-render.
	}
}

/// <summary>
/// Called when the double precision render option on the toolbar is clicked.
/// </summary>
/// <param name="checked">Check state, action only taken if true.</param>
void Fractorium::OnActionDP(bool checked)
{
	if (checked && !m_Settings->Double())
	{
		m_Settings->Double(true);
		ShutdownAndRecreateFromOptions(true);//Pass true, but it's not needed because creating a new controller will force a library tree re-render.
	}
}

/// <summary>
/// Called when the flam3 compatibility option on the toolbar is clicked.
/// </summary>
/// <param name="checked">Check state, action only taken if true.</param>
void Fractorium::OnActionCompat(bool checked)
{
	m_Settings->Flam3Compat(checked);
	Compat::m_Compat = checked;
	ShutdownAndRecreateFromOptions(true);
}

/// <summary>
/// Called when the show style button is clicked.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionStyle(bool checked)
{
#ifdef __APPLE__
	m_QssDialog->exec();
#else
	m_QssDialog->show();
#endif
}

/// <summary>
/// Called when the start/stop renderer button is clicked.
/// </summary>
/// <param name="checked">Check state, stop renderer if true, else start.</param>
void Fractorium::OnActionStartStopRenderer(bool checked)
{
	EnableRenderControls(!checked);

	if (checked)
	{
		m_Controller->StopRenderTimer(true);
		ui.ActionStartStopRenderer->setToolTip("Start Renderer");
		ui.ActionStartStopRenderer->setIcon(QIcon(":/Fractorium/Icons/control.png"));
	}
	else
	{
		m_Controller->StartRenderTimer();
		ui.ActionStartStopRenderer->setToolTip("Stop Renderer");
		ui.ActionStartStopRenderer->setIcon(QIcon(":/Fractorium/Icons/control-stop-square.png"));
	}
}

/// <summary>
/// Called when the pre affine button is clicked.
/// </summary>
/// <param name="checked">Check state, show pre affines if true, else hide.</param>
void Fractorium::OnActionDrawAffines(bool checked)
{
	m_Settings->ShowXforms(checked);

	if (!ui.ActionDrawImage->isChecked() && !(ui.ActionDrawPreAffines->isChecked() || ui.ActionDrawPostAffines->isChecked()))
		ui.ActionDrawImage->setChecked(true);

	SaveAffineState();
	ui.GLDisplay->update();
}

/// <summary>
/// Toggle whether to show selected/all post affines.
/// Called when the show all post affine button is clicked.
/// </summary>
/// <param name="checked">Check state, show all pre affines if true, else show selected.</param>
void Fractorium::OnActionDrawAllAffines(bool checked)
{
	SaveAffineState();
	ui.GLDisplay->update();
}

/// <summary>
/// Toggle whether to show the image.
/// Called when the image button is clicked.
/// </summary>
/// <param name="checked">Check state, show image if true, else hide.</param>
void Fractorium::OnActionDrawImage(bool checked)
{
	if (!ui.ActionDrawImage->isChecked())
		SyncAffineStateToToolbar();

	ui.GLDisplay->update();
}

/// <summary>
/// Toggle whether to show the grid.
/// Called when the grid image button is clicked.
/// </summary>
/// <param name="checked">Check state, show grid if true, else hide.</param>
void Fractorium::OnActionDrawGrid(bool checked)
{
	m_Settings->ShowGrid(checked);
	ui.GLDisplay->update();
}

/// <summary>
/// Keep previous state of the affines to switch between Editor and Image
/// </summary>
void Fractorium::SaveAffineState()
{
	if (!ui.ActionDrawPreAffines->isChecked() && !ui.ActionDrawPostAffines->isChecked())
		return;

	m_PreviousAffineState[int(eAffineState::PRE)     ] = ui.ActionDrawPreAffines->isChecked();
	m_PreviousAffineState[int(eAffineState::ALL_PRE) ] = ui.ActionDrawAllPreAffines->isChecked();
	m_PreviousAffineState[int(eAffineState::POST)    ] = ui.ActionDrawPostAffines->isChecked();
	m_PreviousAffineState[int(eAffineState::ALL_POST)] = ui.ActionDrawAllPostAffines->isChecked();
}

/// <summary>
/// Sync affine state data to the check state of the toolbar buttons.
/// This does not trigger a clicked() event.
/// </summary>
void Fractorium::SyncAffineStateToToolbar()
{
	ui.ActionDrawPreAffines->setChecked(m_PreviousAffineState    [int(eAffineState::PRE)     ]);
	ui.ActionDrawAllPreAffines->setChecked(m_PreviousAffineState [int(eAffineState::ALL_PRE) ]);
	ui.ActionDrawPostAffines->setChecked(m_PreviousAffineState   [int(eAffineState::POST)    ]);
	ui.ActionDrawAllPostAffines->setChecked(m_PreviousAffineState[int(eAffineState::ALL_POST)]);
}

/// <summary>
/// Sync options data to the check state of the toolbar buttons.
/// This does not trigger a clicked() event.
/// </summary>
void Fractorium::SyncOptionsToToolbar()
{
	static bool openCL = !m_Info->Devices().empty();

	if (!openCL)
	{
		ui.ActionCL->setEnabled(false);
	}

	if (openCL && m_Settings->OpenCL())
	{
		ui.ActionCpu->setChecked(false);
		ui.ActionCL->setChecked(true);
	}
	else
	{
		ui.ActionCpu->setChecked(true);
		ui.ActionCL->setChecked(false);
	}

	if (m_Settings->Double())
	{
		ui.ActionSP->setChecked(false);
		ui.ActionDP->setChecked(true);
	}
	else
	{
		ui.ActionSP->setChecked(true);
		ui.ActionDP->setChecked(false);
	}

	ui.ActionCompat->setChecked(m_Settings->Flam3Compat());
	ui.ActionDrawGrid->setChecked(m_Settings->ShowGrid());
	ui.ActionDrawPreAffines->setChecked(m_Settings->ShowXforms());
	ui.ActionDrawAllPreAffines->setChecked(m_Settings->ShowXforms());
	SaveAffineState();
}
