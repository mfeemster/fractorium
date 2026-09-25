#include "FractoriumPch.h"
#include "Fractorium.h"

#define XAOS_PREC 8

/// <summary>
/// Initialize the xforms xaos UI.
/// </summary>
void Fractorium::InitXaosUI()
{
	int spinHeight = 20;
	ui.XaosTableView->verticalHeader()->setSectionsClickable(true);
	ui.XaosTableView->horizontalHeader()->setSectionsClickable(true);
	auto ttstr = "<html><head/><body><p>Double click to toggle all table spinner values in one row/col.</p><p>If any cell in the row is non zero, set all cells to zero, else 1.</p><p>If shift is held down, reverse the logic.</p><p>If ctrl is held down, set each cell to a random 0 or 1.</p></body></html>";
	ui.XaosTableView->horizontalHeader()->setToolTip(ttstr);
	ui.XaosTableView->verticalHeader()->setToolTip(ttstr);
	m_XaosSpinBox = new DoubleSpinBox(nullptr, spinHeight, 0.1, false);
	m_XaosSpinBox->DoubleClick(true);
	m_XaosSpinBox->DoubleClickZero(1);
	m_XaosSpinBox->DoubleClickNonZero(0);
	m_XaosSpinBox->setDecimals(XAOS_PREC);
	m_XaosSpinBox->setObjectName("XaosSpinBox");
	m_XaosTableModel = nullptr;
	m_AppliedXaosTableModel = nullptr;
	m_XaosTableItemDelegate = new DoubleSpinBoxTableItemDelegate(m_XaosSpinBox, this);
	connect(m_XaosSpinBox,                           SIGNAL(valueChanged(double)),      this, SLOT(OnXaosChanged(double)),              Qt::ConnectionType::QueuedConnection);
	connect(ui.ClearXaosButton,                      SIGNAL(clicked(bool)),             this, SLOT(OnClearXaosButtonClicked(bool)),     Qt::ConnectionType::QueuedConnection);
	connect(ui.RandomXaosButton,                     SIGNAL(clicked(bool)),             this, SLOT(OnRandomXaosButtonClicked(bool)),    Qt::ConnectionType::QueuedConnection);
	connect(ui.TransposeXaosButton,                  SIGNAL(clicked(bool)),             this, SLOT(OnTransposeXaosButtonClicked(bool)), Qt::ConnectionType::QueuedConnection);
	connect(ui.ToggleXaosButton,                     SIGNAL(clicked(bool)),             this, SLOT(OnToggleXaosButtonClicked(bool)),    Qt::ConnectionType::QueuedConnection);
	connect(ui.AddLayerButton,                       SIGNAL(clicked(bool)),             this, SLOT(OnAddLayerButtonClicked(bool)),      Qt::ConnectionType::QueuedConnection);
	connect(ui.XaosTableView->verticalHeader(),      SIGNAL(sectionDoubleClicked(int)), this, SLOT(OnXaosRowDoubleClicked(int)),        Qt::ConnectionType::QueuedConnection);
	connect(ui.XaosTableView->horizontalHeader(),    SIGNAL(sectionDoubleClicked(int)), this, SLOT(OnXaosColDoubleClicked(int)),        Qt::ConnectionType::QueuedConnection);
	connect(ui.XaosTableView->horizontalScrollBar(), SIGNAL(valueChanged(int)),         this, SLOT(OnXaosHScrollValueChanged(int)),     Qt::ConnectionType::QueuedConnection);
	connect(ui.XaosTableView->verticalScrollBar(),   SIGNAL(valueChanged(int)),         this, SLOT(OnXaosVScrollValueChanged(int)),     Qt::ConnectionType::QueuedConnection);
}

/// <summary>
/// Fill the xaos table with the values from the ember.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::FillXaos()
{
	for (int i = 0, count = static_cast<int>(XformCount()); i < count; i++)//Column.
	{
		if (const auto xform = m_Ember.GetXform(i))
		{
			for (int j = 0; j < count; j++)//Row.
			{
				QModelIndex index = m_Fractorium->m_XaosTableModel->index(j, i, QModelIndex());//j and i are intentionally swapped here.
				m_Fractorium->m_XaosTableModel->setData(index, xform->Xaos(j));
			}
		}
	}

	m_Fractorium->ui.XaosTableView->resizeRowsToContents();
	m_Fractorium->ui.XaosTableView->resizeColumnsToContents();
}

/// <summary>
/// Fill the xaos table with the xaos values applied to the xform weights from the ember.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::FillAppliedXaos()
{
	std::vector<T> weights;
	auto ct = int(XformCount());

	for (int i = 0, count = ct; i < count; i++)
		weights.push_back(m_Ember.GetXform(i)->m_Weight);

	for (int i = 0, count = ct; i < count; i++)//Column.
	{
		if (const auto xform = m_Ember.GetXform(i))
		{
			T norm = 0;
			double start = 0, offset = 0;
			auto tempweights = weights;

			for (int j = 0; j < count; j++)//Row.
				tempweights[j] *= xform->Xaos(j);

			QPixmap pixmap(m_Fractorium->ui.XaosAppliedTableView->columnWidth(i) - 8, m_Fractorium->ui.XaosTableView->rowHeight(0) * count);
			QPainter painter(&pixmap);
			auto twi = std::make_unique<QTableWidgetItem>();

			for (auto& w : tempweights) norm += w;

			for (auto& w : tempweights) w = norm == static_cast<T>(0) ? static_cast<T>(0) : w / norm;

			for (int j = 0; j < count; j++)//Row.
			{
				auto normXaosAppliedWeight = tempweights[j];
				QModelIndex index = m_Fractorium->m_AppliedXaosTableModel->index(j, i, QModelIndex());//j and i are intentionally swapped here.
				m_Fractorium->m_AppliedXaosTableModel->setData(index, TruncPrecision(normXaosAppliedWeight, 4));//Applied xaos is just a read only table for display purposes.
			}

			if (norm)
			{
				for (size_t wi = 0; wi < tempweights.size() && offset <= pixmap.height(); wi++)
				{
					offset = std::min<T>(offset + tempweights[wi] * pixmap.height(), pixmap.height());
					painter.fillRect(0, start, pixmap.width(), offset, m_Fractorium->m_XformComboColors[wi % XFORM_COLOR_COUNT]);
					start = offset;
				}
			}
			else
			{
				painter.fillRect(0, 0, pixmap.width(), pixmap.height(), m_Fractorium->m_XformComboColors[0]);
			}

			twi->setData(Qt::ItemDataRole::DecorationRole, pixmap);
			m_Fractorium->ui.XaosDistVizTableWidget->setItem(0, i, twi.release());
		}
	}

	m_Fractorium->ui.XaosDistVizTableWidget->resizeRowsToContents();
	m_Fractorium->ui.XaosDistVizTableWidget->resizeColumnsToContents();
	m_Fractorium->ui.XaosAppliedTableView->resizeRowsToContents();
	m_Fractorium->ui.XaosAppliedTableView->resizeColumnsToContents();
}

/// <summary>
/// Set the xaos value.
/// Called when any xaos spinner is changed.
/// It actually gets called multiple times as the user clicks around the
/// xaos table due to how QTableView passes events to and from its model.
/// To filter out spurrious events, the value is checked against the existing
/// xaos value.
/// Resets the rendering process.
/// </summary>
/// <param name="x">The index of the xform whose xaos value was changed (column)</param>
/// <param name="y">The index of the to value that was changed for the xform (row)</param>
/// <param name="val">The changed value of the xaos element</param>
template <typename T>
void FractoriumEmberController<T>::XaosChanged(int x, int y, double val)
{
	auto newVal = TruncPrecision(val, XAOS_PREC);//Sometimes 0 comes in as a very small number, so round.

	if (auto xform = m_Ember.GetXform(x))
		if (!IsClose<T>(newVal, xform->Xaos(y), T(1e-7)))
		{
			Update([&] { xform->SetXaos(y, newVal); });
			FillAppliedXaos();
		}
}

void Fractorium::OnXaosChanged(double d)
{
	if (auto senderSpinBox = qobject_cast<DoubleSpinBox*>(sender()))
	{
		auto p = senderSpinBox->property("tableindex").toPoint();
		m_Controller->XaosChanged(p.y(), p.x(), d);//Intentionally switched, column is the from xform, row is the to xform.
	}
}

void Fractorium::OnXaosTableModelDataChanged(const QModelIndex& indexA, const QModelIndex& indexB)
{
	m_Controller->XaosChanged(indexA.column(), indexA.row(), indexA.data().toDouble());//Intentionally switched, column is the from xform, row is the to xform.
}

/// <summary>
/// Clear xaos table, recreate all spinners based on the xaos used in the current ember.
/// </summary>
void Fractorium::FillXaosTable()
{
	int count = static_cast<int>(m_Controller->XformCount());
	QStringList hl, vl, blanks;
	auto oldModel = std::make_unique<QStandardItemModel>(m_XaosTableModel);
	hl.reserve(count);
	vl.reserve(count);
	blanks.push_back("");
	m_XaosTableModel = new QStandardItemModel(count, count, this);
	m_AppliedXaosTableModel = new QStandardItemModel(count, count, this);
	connect(m_XaosTableModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)), SLOT(OnXaosTableModelDataChanged(QModelIndex, QModelIndex)));

	for (int i = 0; i < count; i++)
	{
		auto s = m_Controller->MakeXformCaption(i);
		hl.push_back("F" + s);
		vl.push_back("T" + s);
	}

	m_XaosTableModel->setHorizontalHeaderLabels(hl);
	m_XaosTableModel->setVerticalHeaderLabels(vl);
	m_AppliedXaosTableModel->setHorizontalHeaderLabels(hl);
	m_AppliedXaosTableModel->setVerticalHeaderLabels(vl);
	ui.XaosDistVizTableWidget->setRowCount(1);
	ui.XaosDistVizTableWidget->setColumnCount(count);
	ui.XaosDistVizTableWidget->verticalHeader()->setSectionsClickable(false);
	ui.XaosDistVizTableWidget->horizontalHeader()->setSectionsClickable(false);
	ui.XaosTableView->setModel(m_XaosTableModel);
	ui.XaosAppliedTableView->setModel(m_AppliedXaosTableModel);
	ui.XaosTableView->setItemDelegate(m_XaosTableItemDelegate);//No need for a delegate on the applied table because it's read-only.
	ui.XaosDistVizTableWidget->setRowHeight(0, ui.XaosTableView->rowHeight(0) * count);
	ui.XaosDistVizTableWidget->setHorizontalHeaderLabels(hl);
	ui.XaosDistVizTableWidget->setVerticalHeaderLabels(vl);
	SetTabOrder(this, ui.ClearXaosButton, ui.RandomXaosButton);
	ui.XaosDistVizTableWidget->verticalHeader()->setStyleSheet("QHeaderView::section::vertical { color: rgba(0, 0, 0, 0); }");
	m_Controller->FillXaos();
	m_Controller->FillAppliedXaos();
	//Needed to get the dark stylesheet to correctly color the top left corner button.
	auto widgetList = ui.XaosTableView->findChildren<QAbstractButton*>();

	for (auto& it : widgetList)
		it->setEnabled(true);

	widgetList = ui.XaosAppliedTableView->findChildren<QAbstractButton*>();

	for (auto& it : widgetList)
		it->setEnabled(true);
}

/// <summary>
/// Clear all xaos from the current ember.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::ClearXaos()
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.ClearXaos();
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
	FillXaos();
	FillAppliedXaos();
}

void Fractorium::OnClearXaosButtonClicked(bool checked) { m_Controller->ClearXaos(); }

/// <summary>
/// Set all xaos values to random numbers.
/// There is a 50% chance they're set to 0 or 1, and
/// 50% that they're 0-3.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::RandomXaos()
{
	bool ctrl = QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier);
	Update([&]
	{
		size_t i = 0;

		while (auto xform = m_Ember.GetXform(i++))
		{
			for (size_t j = 0; j < m_Ember.XformCount(); j++)
			{
				if (!ctrl)
					xform->SetXaos(j, static_cast<T>(m_Rand.RandBit()));
				else if (m_Rand.RandBit())
					xform->SetXaos(j, static_cast<T>(m_Rand.RandBit()));
				else
					xform->SetXaos(j, TruncPrecision(m_Rand.Frand<T>(0, 3), 3));
			}
		}
	});
	FillXaos();
	FillAppliedXaos();
}

void Fractorium::OnRandomXaosButtonClicked(bool checked) { m_Controller->RandomXaos(); }

/// <summary>
/// Add a layer using the specified number of xforms.
/// A layer is defined as a new set of xforms whose xaos values are the following:
/// From existing to existing: unchanged.
/// From existing to new: 0.
/// From new to existing: 0.
/// From new to new: 1.
/// Resets the rendering process.
/// </summary>
/// <param name="xforms">The number of new xforms to add to create the layer</param>
template <typename T>
void FractoriumEmberController<T>::AddLayer(int xforms)
{
	Update([&]
	{
		std::vector<std::pair<Xform<T>, size_t>> vec(xforms);
		AddXformsWithXaos(m_Ember, vec, eXaosPasteStyle::ZERO_TO_ONE);

	});
	FillXforms();
	FillSummary();
}

void Fractorium::OnAddLayerButtonClicked(bool checked) { m_Controller->AddLayer(ui.AddLayerSpinBox->value()); }

/// <summary>
/// Flip the row and column values of the xaos table.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::TransposeXaos()
{
	Update([&]
	{
		size_t i = 0, j = 0;
		vector<vector<double>> tempxaos;
		tempxaos.reserve(m_Ember.XformCount());

		while (const auto xform = m_Ember.GetXform(i++))
		{
			vector<double> tempvec;
			tempvec.reserve(m_Ember.XformCount());

			for (j = 0; j < m_Ember.XformCount(); j++)
				tempvec.push_back(xform->Xaos(j));

			tempxaos.push_back(std::move(tempvec));
		}

		for (j = 0; j < tempxaos.size(); j++)
			for (i = 0; i < tempxaos[j].size(); i++)
				if (auto xform = m_Ember.GetXform(i))
					xform->SetXaos(j, static_cast<T>(tempxaos[j][i]));
	});
	FillXaos();
	FillAppliedXaos();
}

void Fractorium::OnTransposeXaosButtonClicked(bool checked) { m_Controller->TransposeXaos(); }

/// <summary>
/// Toggle whether to use xaos or not by saving/restoring a backup flame.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::ToggleXaos()
{
	Update([&]
	{
		if (m_Ember.XaosPresent())
		{
			m_XaosToggleEmber = m_Ember;
			m_Ember.ClearXaos();
		}
		else if (m_XaosToggleEmber.XaosPresent())
		{
			size_t i = 0;

			while (auto xform = m_Ember.GetXform(i))
			{
				auto backupXform = m_XaosToggleEmber.GetXform(i);

				if (backupXform)
				{
					for (size_t j = 0; j < m_Ember.XformCount(); j++)
						xform->SetXaos(j, TruncPrecision(backupXform->Xaos(j), XAOS_PREC));
				}
				else
					break;

				i++;
			}

			m_XaosToggleEmber.Clear();
		}
	});
	FillXaos();
	FillAppliedXaos();
}

void Fractorium::OnToggleXaosButtonClicked(bool checked) { m_Controller->ToggleXaos(); }

/// <summary>
/// Toggle all xaos values in one row on left mouse button double click and resize all cells to fit their data.
/// Skip toggling and only refit on right mouse button double click.
/// Resets the rendering process.
/// </summary>
/// <param name="logicalIndex">The index of the row that was double clicked</param>
void Fractorium::OnXaosRowDoubleClicked(int logicalIndex)
{
	const auto btn = QApplication::mouseButtons();

	if (!btn.testFlag(Qt::MouseButton::RightButton))
		ToggleTableRow(ui.XaosTableView, logicalIndex);

	ui.XaosTableView->resizeRowsToContents();
	ui.XaosTableView->resizeColumnsToContents();
	m_Controller->FillAppliedXaos();
}

/// <summary>
/// Toggle all xaos values in one column on left mouse button double click and resize all cells to fit their data.
/// Skip toggling and only refit on right mouse button double click.
/// Resets the rendering process.
/// </summary>
/// <param name="logicalIndex">The index of the column that was double clicked</param>
void Fractorium::OnXaosColDoubleClicked(int logicalIndex)
{
	const auto btn = QApplication::mouseButtons();

	if (!btn.testFlag(Qt::MouseButton::RightButton))
		ToggleTableCol(ui.XaosTableView, logicalIndex);

	ui.XaosTableView->resizeRowsToContents();
	ui.XaosTableView->resizeColumnsToContents();
	m_Controller->FillAppliedXaos();
}

/// <summary>
/// Take the value of the horizontal scrollbar on the xaos table and set the same
/// horizontal scroll bar position on XaosDistVizTableWidget and XaosAppliedTableView.
/// This allows them to easily see the same part of all three tables at the same time
/// when there are more xforms than can fit on the screen at once.
/// </summary>
/// <param name="value">The value of the xaos table horizontal scroll bar</param>
void Fractorium::OnXaosHScrollValueChanged(int value)
{
	ui.XaosDistVizTableWidget->horizontalScrollBar()->setValue(value);
	ui.XaosAppliedTableView->horizontalScrollBar()->setValue(value);
}

/// <summary>
/// Take the value of the vertical scrollbar on the xaos table and set the same
/// vertical scroll bar position on XaosDistVizTableWidget and XaosAppliedTableView.
/// This allows them to easily see the same part of all three tables at the same time
/// when there are more xforms than can fit on the screen at once.
/// </summary>
/// <param name="value">The value of the xaos table vertical scroll bar</param>
void Fractorium::OnXaosVScrollValueChanged(int value)
{
	ui.XaosDistVizTableWidget->verticalScrollBar()->setValue(value);
	ui.XaosAppliedTableView->verticalScrollBar()->setValue(value);
}

/// <summary>
/// Get an enum value corresponding to the currently selected xaos pasting mode.
/// </summary>
/// <returns>The xaos pasting mode enum</returns>
eXaosPasteStyle Fractorium::GetXaosPasteStyleType()
{
	if (ui.XaosPasteNoneRadio->isChecked())
		return eXaosPasteStyle::NONE;
	else if (ui.XaosPaste0to1Radio->isChecked())
		return eXaosPasteStyle::ZERO_TO_ONE;
	else if (ui.XaosPaste0toValsRadio->isChecked())
		return eXaosPasteStyle::ZERO_TO_VALS;
	else if (ui.XaosPaste1toValsRadio->isChecked())
		return eXaosPasteStyle::ONE_TO_VALS;
	else if (ui.XaosPasteValsTo1Radio->isChecked())
		return eXaosPasteStyle::VALS_TO_ONE;
	else
		return eXaosPasteStyle::NONE;
}

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
