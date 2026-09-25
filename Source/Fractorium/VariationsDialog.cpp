#include "FractoriumPch.h"
#include "VariationsDialog.h"

/// <summary>
/// Constructor that takes a parent widget and passes it to the base, then
/// sets up the GUI.
/// </summary>
/// <param name="settings">Pointer to the global settings object to use</param>
/// <param name="p">The parent widget. Default: nullptr.</param>
/// <param name="f">The window flags. Default: 0.</param>
FractoriumVariationsDialog::FractoriumVariationsDialog(QWidget* p, Qt::WindowFlags f)
	: QDialog(p, f),
	  m_VariationList(VariationList<float>::Instance())
{
	ui.setupUi(this);
	const auto table = ui.VariationsTable;
	m_Settings = FractoriumSettings::DefInstance();
	m_Vars = m_Settings->Variations();
	Populate();
	OnSelectAllButtonClicked(true);
	m_CheckBoxes.push_back(ui.SumCheckBox);
	m_CheckBoxes.push_back(ui.AssignCheckBox);
	m_CheckBoxes.push_back(ui.PpSumCheckBox);
	m_CheckBoxes.push_back(ui.PpAssignCheckBox);
	m_CheckBoxes.push_back(ui.DcCheckBox);
	m_CheckBoxes.push_back(ui.StateCheckBox);
	m_CheckBoxes.push_back(ui.ParamCheckBox);
	m_CheckBoxes.push_back(ui.NonParamCheckBox);
	ui.SumCheckBox->setCheckState     (static_cast<Qt::CheckState>(m_Settings->VarFilterSum     ()));
	ui.AssignCheckBox->setCheckState  (static_cast<Qt::CheckState>(m_Settings->VarFilterAssign  ()));
	ui.PpSumCheckBox->setCheckState   (static_cast<Qt::CheckState>(m_Settings->VarFilterPpsum   ()));
	ui.PpAssignCheckBox->setCheckState(static_cast<Qt::CheckState>(m_Settings->VarFilterPpassign()));
	ui.DcCheckBox->setCheckState      (static_cast<Qt::CheckState>(m_Settings->VarFilterSdc     ()));
	ui.StateCheckBox->setCheckState   (static_cast<Qt::CheckState>(m_Settings->VarFilterState   ()));
	ui.ParamCheckBox->setCheckState   (static_cast<Qt::CheckState>(m_Settings->VarFilterParam   ()));
	ui.NonParamCheckBox->setCheckState(static_cast<Qt::CheckState>(m_Settings->VarFilterNonparam()));

	for (auto& cb : m_CheckBoxes)
	{
		if (cb->checkState() == Qt::CheckState::PartiallyChecked)
		{
			auto font = cb->font();
			font.setStrikeOut(true);
			cb->setFont(font);
		}
	}

	table->verticalHeader()->setSectionsClickable(true);
	table->horizontalHeader()->setSectionsClickable(true);
	table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	connect(table,                    SIGNAL(itemChanged(QTableWidgetItem*)),    this, SLOT(OnVariationsTableItemChanged(QTableWidgetItem*)), Qt::ConnectionType::QueuedConnection);
	connect(ui.SelectAllButton,       SIGNAL(clicked(bool)),                     this, SLOT(OnSelectAllButtonClicked(bool)),                  Qt::ConnectionType::QueuedConnection);
	connect(ui.InvertSelectionButton, SIGNAL(clicked(bool)),                     this, SLOT(OnInvertSelectionButtonClicked(bool)),            Qt::ConnectionType::QueuedConnection);
	connect(ui.SelectNoneButton,      SIGNAL(clicked(bool)),                     this, SLOT(OnSelectNoneButtonClicked(bool)),                 Qt::ConnectionType::QueuedConnection);
	connect(ui.SumCheckBox,           SIGNAL(checkStateChanged(Qt::CheckState)), this, SLOT(OnSelectionCheckBoxStateChanged(Qt::CheckState)), Qt::ConnectionType::QueuedConnection);
	connect(ui.AssignCheckBox,        SIGNAL(checkStateChanged(Qt::CheckState)), this, SLOT(OnSelectionCheckBoxStateChanged(Qt::CheckState)), Qt::ConnectionType::QueuedConnection);
	connect(ui.PpSumCheckBox,         SIGNAL(checkStateChanged(Qt::CheckState)), this, SLOT(OnSelectionCheckBoxStateChanged(Qt::CheckState)), Qt::ConnectionType::QueuedConnection);
	connect(ui.PpAssignCheckBox,      SIGNAL(checkStateChanged(Qt::CheckState)), this, SLOT(OnSelectionCheckBoxStateChanged(Qt::CheckState)), Qt::ConnectionType::QueuedConnection);
	connect(ui.DcCheckBox,            SIGNAL(checkStateChanged(Qt::CheckState)), this, SLOT(OnSelectionCheckBoxStateChanged(Qt::CheckState)), Qt::ConnectionType::QueuedConnection);
	connect(ui.StateCheckBox,         SIGNAL(checkStateChanged(Qt::CheckState)), this, SLOT(OnSelectionCheckBoxStateChanged(Qt::CheckState)), Qt::ConnectionType::QueuedConnection);
	connect(ui.ParamCheckBox,         SIGNAL(checkStateChanged(Qt::CheckState)), this, SLOT(OnSelectionCheckBoxStateChanged(Qt::CheckState)), Qt::ConnectionType::QueuedConnection);
	connect(ui.NonParamCheckBox,      SIGNAL(checkStateChanged(Qt::CheckState)), this, SLOT(OnSelectionCheckBoxStateChanged(Qt::CheckState)), Qt::ConnectionType::QueuedConnection);
}

/// <summary>
/// A wrapper to iterate over every table widget item and perform the passed in function on it.
/// </summary>
/// <param name="func">Function to call on each object</param>
void FractoriumVariationsDialog::ForEachCell(std::function<void(QTableWidgetItem* cb)> func)
{
	const auto table = ui.VariationsTable;
	const auto rows = table->rowCount();
	const auto cols = table->columnCount();
	table->model()->blockSignals(true);

	for (auto row = 0; row < rows; row++)
		for (auto col = 0; col < cols; col++)
			if (const auto cb = table->item(row, col))
				func(cb);

	table->model()->blockSignals(false);
	table->model()->layoutChanged();
}

/// <summary>
/// A wrapper to iterate over every selected table widget item and perform the passed in function on it.
/// </summary>
/// <param name="func">Function to call on each object</param>
void FractoriumVariationsDialog::ForEachSelectedCell(std::function<void(QTableWidgetItem* cb)> func)
{
	const auto table = ui.VariationsTable;
	const auto selectedItems = table->selectedItems();
	table->model()->blockSignals(true);

	for (auto item : selectedItems)
		if (item)
			func(item);

	table->model()->blockSignals(false);
	table->model()->layoutChanged();
}

/// <summary>
/// Copy the values of the checkboxes to the map.
/// </summary>
void FractoriumVariationsDialog::SyncSettings()
{
	QMap<QString, QVariant> m;
	ForEachCell([&](QTableWidgetItem * cb)
	{
		if (!cb->text().isEmpty())
			m[cb->text()] = cb->checkState() == Qt::CheckState::Checked;
	});
	m_Settings->Variations(m);
	m_Settings->VarFilterSum     (static_cast<int>(ui.SumCheckBox->checkState()));
	m_Settings->VarFilterAssign  (static_cast<int>(ui.AssignCheckBox->checkState()));
	m_Settings->VarFilterPpsum   (static_cast<int>(ui.PpSumCheckBox->checkState()));
	m_Settings->VarFilterPpassign(static_cast<int>(ui.PpAssignCheckBox->checkState()));
	m_Settings->VarFilterSdc     (static_cast<int>(ui.DcCheckBox->checkState()));
	m_Settings->VarFilterState   (static_cast<int>(ui.StateCheckBox->checkState()));
	m_Settings->VarFilterParam   (static_cast<int>(ui.ParamCheckBox->checkState()));
	m_Settings->VarFilterNonparam(static_cast<int>(ui.NonParamCheckBox->checkState()));
}

/// <summary>
/// Return a const reference to the map.
/// This will contains the state of the checkboxes after
/// the user clicks ok.
/// </summary>
const QMap<QString, QVariant>& FractoriumVariationsDialog::Map()
{
	return m_Vars;
}

/// <summary>
/// Clears the type checkboxes without triggering any events.
/// </summary>
void FractoriumVariationsDialog::ClearTypesStealth()
{
	for (auto& cb : m_CheckBoxes)
	{
		cb->blockSignals(true);
		cb->setCheckState(Qt::CheckState::Unchecked);
		auto f = cb->font();
		f.setStrikeOut(cb->checkState() == Qt::CheckState::PartiallyChecked);
		cb->setFont(f);
		cb->blockSignals(false);
	}
}

/// <summary>
/// Check all of the checkboxes.
/// </summary>
/// <param name="checked">Ignored</param>
void FractoriumVariationsDialog::OnSelectAllButtonClicked(bool checked)
{
	ClearTypesStealth();
	ForEachCell([&](QTableWidgetItem * cb) { cb->setCheckState(Qt::CheckState::Checked); });
}

/// <summary>
/// Invert the selection state of the checkboxes.
/// </summary>
/// <param name="checked">Ignored</param>
void FractoriumVariationsDialog::OnInvertSelectionButtonClicked(bool checked)
{
	ClearTypesStealth();
	ForEachCell([&](QTableWidgetItem * cb)
	{
		if (cb->checkState() != Qt::CheckState::Checked)
			cb->setCheckState(Qt::CheckState::Checked);
		else
			cb->setCheckState(Qt::CheckState::Unchecked);
	});
}

/// <summary>
/// Uncheck all of the checkboxes.
/// </summary>
/// <param name="checked">Ignored</param>
void FractoriumVariationsDialog::OnSelectNoneButtonClicked(bool checked)
{
	ClearTypesStealth();
	ForEachCell([&](QTableWidgetItem * cb) { cb->setCheckState(Qt::CheckState::Unchecked); });
}

/// <summary>
/// Called when any of the selection checkboxes change state.
/// This will re-evaluate the entire grid of checkboxes.
/// </summary>
/// <param name="state">Ignored</param>
void FractoriumVariationsDialog::OnSelectionCheckBoxStateChanged(Qt::CheckState state)
{
	static vector<string> dc{ "m_ColorX" };
	static vector<string> assign{ "outPoint->m_X =", "outPoint->m_Y =", "outPoint->m_Z =",
								  "outPoint->m_X=", "outPoint->m_Y=", "outPoint->m_Z=" };
	static std::set<eVariationId> excluded;
	excluded.clear();

	if (const auto s = dynamic_cast<QCheckBox*>(sender()))
	{
		auto f = s->font();
		f.setStrikeOut(s->checkState() == Qt::CheckState::PartiallyChecked);
		s->setFont(f);
	}

	ForEachCell([&](QTableWidgetItem * cb) { cb->setCheckState(Qt::CheckState::Unchecked); });
	ForEachCell([&](QTableWidgetItem * cb)
	{
		if (const auto var = m_VariationList->GetVariation(cb->text().toStdString()))
		{
			if (ui.StateCheckBox->checkState() != Qt::CheckState::Unchecked)
			{
				if (!var->StateOpenCLString().empty())
				{
					if (ui.StateCheckBox->checkState() == Qt::CheckState::PartiallyChecked)
					{
						cb->setCheckState(Qt::CheckState::Unchecked);
						excluded.insert(var->VariationId());
					}
					else if (!Contains(excluded, var->VariationId()))
						cb->setCheckState(Qt::CheckState::Checked);
				}
			}

			if (ui.SumCheckBox->isChecked() != Qt::CheckState::Unchecked)
			{
				if (var->VarType() == eVariationType::VARTYPE_REG && !SearchVar(var, assign, false))
				{
					if (ui.SumCheckBox->checkState() == Qt::CheckState::PartiallyChecked)
					{
						cb->setCheckState(Qt::CheckState::Unchecked);
						excluded.insert(var->VariationId());
					}
					else if (!Contains(excluded, var->VariationId()))
						cb->setCheckState(Qt::CheckState::Checked);
				}
			}

			if (ui.AssignCheckBox->isChecked() != Qt::CheckState::Unchecked)
			{
				if (var->VarType() == eVariationType::VARTYPE_REG && SearchVar(var, assign, false))
				{
					if (ui.AssignCheckBox->checkState() == Qt::CheckState::PartiallyChecked)
					{
						cb->setCheckState(Qt::CheckState::Unchecked);
						excluded.insert(var->VariationId());
					}
					else if (!Contains(excluded, var->VariationId()))
						cb->setCheckState(Qt::CheckState::Checked);
				}
			}

			if (ui.DcCheckBox->isChecked() != Qt::CheckState::Unchecked)
			{
				if (SearchVar(var, dc, false))
				{
					if (ui.DcCheckBox->checkState() == Qt::CheckState::PartiallyChecked)
					{
						cb->setCheckState(Qt::CheckState::Unchecked);
						excluded.insert(var->VariationId());
					}
					else if (!Contains(excluded, var->VariationId()))
						cb->setCheckState(Qt::CheckState::Checked);
				}
			}

			if (ui.NonParamCheckBox->isChecked() != Qt::CheckState::Unchecked)
			{
				if (!m_VariationList->GetParametricVariation(cb->text().toStdString()))
				{
					if (ui.NonParamCheckBox->checkState() == Qt::CheckState::PartiallyChecked)
					{
						cb->setCheckState(Qt::CheckState::Unchecked);
						excluded.insert(var->VariationId());
					}
					else if (!Contains(excluded, var->VariationId()))
						cb->setCheckState(Qt::CheckState::Checked);
				}
			}
		}

		if (ui.PpSumCheckBox->isChecked() != Qt::CheckState::Unchecked)
		{
			if (const auto pre = m_VariationList->GetPreVariation(cb->text().toStdString()))
			{
				if (pre->AssignType() == eVariationAssignType::ASSIGNTYPE_SUM)
				{
					if (ui.PpSumCheckBox->checkState() == Qt::CheckState::PartiallyChecked)
					{
						cb->setCheckState(Qt::CheckState::Unchecked);
						excluded.insert(pre->VariationId());
					}
					else if (!Contains(excluded, pre->VariationId()))
						cb->setCheckState(Qt::CheckState::Checked);
				}
			}

			if (const auto post = m_VariationList->GetPostVariation(cb->text().toStdString()))
			{
				if (post->AssignType() == eVariationAssignType::ASSIGNTYPE_SUM)
				{
					if (ui.PpSumCheckBox->checkState() == Qt::CheckState::PartiallyChecked)
					{
						cb->setCheckState(Qt::CheckState::Unchecked);
						excluded.insert(post->VariationId());
					}
					else if (!Contains(excluded, post->VariationId()))
						cb->setCheckState(Qt::CheckState::Checked);
				}
			}
		}

		if (ui.PpAssignCheckBox->isChecked() != Qt::CheckState::Unchecked)
		{
			if (const auto pre = m_VariationList->GetPreVariation(cb->text().toStdString()))
			{
				if (pre->AssignType() == eVariationAssignType::ASSIGNTYPE_SET)
				{
					if (ui.PpAssignCheckBox->checkState() == Qt::CheckState::PartiallyChecked)
					{
						cb->setCheckState(Qt::CheckState::Unchecked);
						excluded.insert(pre->VariationId());
					}
					else if (!Contains(excluded, pre->VariationId()))
						cb->setCheckState(Qt::CheckState::Checked);
				}
			}

			if (const auto post = m_VariationList->GetPostVariation(cb->text().toStdString()))
			{
				if (post->AssignType() == eVariationAssignType::ASSIGNTYPE_SET)
				{
					if (ui.PpAssignCheckBox->checkState() == Qt::CheckState::PartiallyChecked)
					{
						cb->setCheckState(Qt::CheckState::Unchecked);
						excluded.insert(post->VariationId());
					}
					else if (!Contains(excluded, post->VariationId()))
						cb->setCheckState(Qt::CheckState::Checked);
				}
			}
		}

		if (ui.ParamCheckBox->isChecked() != Qt::CheckState::Unchecked)
		{
			if (const auto parVar = m_VariationList->GetParametricVariation(cb->text().toStdString()))
			{
				if (ui.ParamCheckBox->checkState() == Qt::CheckState::PartiallyChecked)
				{
					cb->setCheckState(Qt::CheckState::Unchecked);
					excluded.insert(parVar->VariationId());
				}
				else if (!Contains(excluded, parVar->VariationId()))
					cb->setCheckState(Qt::CheckState::Checked);
			}
		}
	});
}

/// <summary>
/// Create all checkboxes and check them according to the map.
/// </summary>
void FractoriumVariationsDialog::Populate()
{
	const auto table = ui.VariationsTable;
	const auto size = static_cast<int>(std::max<size_t>(std::max<size_t>(m_VariationList->RegSize(), m_VariationList->PreSize()), m_VariationList->PostSize()));
	table->setRowCount(size);

	for (auto i = 0; i < size; i++)
	{
		if (const auto pre = m_VariationList->GetVariation(i, eVariationType::VARTYPE_PRE))
		{
			const auto cb = new QTableWidgetItem(pre->Name().c_str());
			table->setItem(i, 0, cb);
			SetCheckFromMap(cb, pre);
		}

		if (const auto reg = m_VariationList->GetVariation(i, eVariationType::VARTYPE_REG))
		{
			const auto cb = new QTableWidgetItem(reg->Name().c_str());
			table->setItem(i, 1, cb);
			SetCheckFromMap(cb, reg);
		}

		if (const auto post = m_VariationList->GetVariation(i, eVariationType::VARTYPE_POST))
		{
			const auto cb = new QTableWidgetItem(post->Name().c_str());
			table->setItem(i, 2, cb);
			SetCheckFromMap(cb, post);
		}
	}
}

/// <summary>
/// Called when a checkbox changes state.
/// There is a slight hack here to apply the change to all selected checkboxes
/// if ctrl is pressed.
/// Otherwise it will only apply to the checkbox that was clicked.
/// </summary>
/// <param name="item"></param>
void FractoriumVariationsDialog::OnVariationsTableItemChanged(QTableWidgetItem* item)
{
	const auto ctrl = QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier);

	if (ctrl)
		ForEachSelectedCell([&](QTableWidgetItem * cb) { cb->setCheckState(item->checkState()); });
}

/// <summary>
/// Called when the user clicks ok.
/// Copy the state of the checkboxes to the map.
/// </summary>
void FractoriumVariationsDialog::accept()
{
	GuiToData();
	QDialog::accept();
}

/// <summary>
/// Called when the user clicks cancel.
/// Reset the state of the the checkboxes to what the map previously was
/// when the dialog was shown.
/// </summary>
void FractoriumVariationsDialog::reject()
{
	DataToGui();
	QDialog::reject();
}

/// <summary>
/// Copy the state of the map to the checkboxes and show the dialog.
/// </summary>
/// <param name="e">Event, passed to base.</param>
void FractoriumVariationsDialog::showEvent(QShowEvent* e)
{
	DataToGui();
	QDialog::showEvent(e);
}

/// <summary>
/// Copy the values in the map to the state of the checkboxes.
/// </summary>
void FractoriumVariationsDialog::DataToGui()
{
	ForEachCell([&](QTableWidgetItem * cb)
	{
		if (const auto var = m_VariationList->GetVariation(cb->text().toStdString()))
			SetCheckFromMap(cb, var);
	});
}

/// <summary>
/// Copy the state of the checkboxes to the map.
/// </summary>
void FractoriumVariationsDialog::GuiToData()
{
	ForEachCell([&](QTableWidgetItem * cb)
	{
		if (const auto var = m_VariationList->GetVariation(cb->text().toStdString()))
			m_Vars[cb->text()] = (cb->checkState() == Qt::CheckState::Checked);
	});
}

/// <summary>
/// Set the state of the passed in table item checkbox based on the boolean contained
/// in the map for the passed in variation.
/// </summary>
/// <param name="cb">The checkbox to check</param>
/// <param name="var">That variation to be looked up in the map</param>
void FractoriumVariationsDialog::SetCheckFromMap(QTableWidgetItem* cb, const Variation<float>* var)
{
	if (!m_Vars.contains(var->Name().c_str()))
	{
		cb->setCheckState(Qt::CheckState::Checked);
	}
	else
	{
		const auto chk = m_Vars[var->Name().c_str()].toBool();
		cb->setCheckState(chk ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
	}
}
