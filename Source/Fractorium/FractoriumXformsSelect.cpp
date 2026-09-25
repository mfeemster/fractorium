#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the xforms selection UI.
/// </summary>
void Fractorium::InitXformsSelectUI()
{
	m_XformsSelectionLayout = dynamic_cast<QFormLayout*>(ui.XformsSelectGroupBoxScrollAreaWidget->layout());
	connect(ui.XformsSelectAllButton,  SIGNAL(clicked(bool)), this, SLOT(OnXformsSelectAllButtonClicked(bool)),  Qt::ConnectionType::QueuedConnection);
	connect(ui.XformsSelectNoneButton, SIGNAL(clicked(bool)), this, SLOT(OnXformsSelectNoneButtonClicked(bool)), Qt::ConnectionType::QueuedConnection);
	ClearXformsSelections();
}

/// <summary>
/// Check all of the xform selection checkboxes.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnXformsSelectAllButtonClicked(bool checked) { ForEachXformCheckbox([&](int i, QCheckBox * w, bool isFinal) { w->setChecked(true); }); }

/// <summary>
/// Uncheck all of the xform selection checkboxes.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnXformsSelectNoneButtonClicked(bool checked) { ForEachXformCheckbox([&](int i, QCheckBox * w, bool isFinal) { w->setChecked(false); }); }

/// <summary>
/// Return whether the checkbox at the specified index is checked.
/// </summary>
/// <param name="i">The index of the xform to check for selection</param>
/// <param name="checked">True if checked, else false.</param>
bool Fractorium::IsXformSelected(size_t i)
{
	if (i < m_XformSelections.size())
		if (const auto w = m_XformSelections[i])
			return w->isChecked();

	return false;
}

/// <summary>
/// Return the number of xforms selected, optionally counting the final xform.
/// </summary>
/// <param name="includeFinal">Whether to include the final in the count if selected.</param>
/// <returns>The caption string</returns>
int Fractorium::SelectedXformCount(bool includeFinal)
{
	auto selCount = 0;
	ForEachXformCheckbox([&](int index, QCheckBox * cb, bool isFinal)
	{
		if (cb->isChecked())
		{
			if (!isFinal || includeFinal)
				selCount++;
		}
	});
	return selCount;
}

/// <summary>
/// Clear all of the dynamically created xform checkboxes.
/// </summary>
void Fractorium::ClearXformsSelections()
{
	QLayoutItem* child = nullptr;
	m_XformSelections.clear();
	m_XformsSelectionLayout->blockSignals(true);

	while (m_XformsSelectionLayout->count() && (child = m_XformsSelectionLayout->takeAt(0)))
	{
		auto w = child->widget();
		delete child;
		delete w;
	}

	m_XformsSelectionLayout->blockSignals(false);
}

/// <summary>
/// Make a caption from an xform.
/// The caption will be the xform count + 1, optionally followed by the xform's name.
/// For final xforms, the string "Final" will be used in place of the count.
/// </summary>
/// <param name="i">The index of the xform to make a caption for</param>
/// <returns>The caption string</returns>
template <typename T>
QString FractoriumEmberController<T>::MakeXformCaption(size_t i)
{
	const auto forceFinal = m_Fractorium->HaveFinal();
	const auto isFinal = m_Ember.FinalXform() == m_Ember.GetTotalXform(i, forceFinal);
	QString caption = isFinal ? "Final" : QString::number(i + 1);

	if (const auto xform = m_Ember.GetTotalXform(i, forceFinal))
		if (!xform->m_Name.empty())
			caption += " (" + QString::fromStdString(xform->m_Name) + ")";

	return caption;
}

/// <summary>
/// Function to perform the specified operation on every dynamically created xform selection checkbox.
/// </summary>
/// <param name="func">The operation to perform which is a function taking the index of the xform, its checkbox, and a bool specifying whether it's final.</param>
void Fractorium::ForEachXformCheckbox(std::function<void(int, QCheckBox*, bool)> func)
{
	int i = 0;
	const auto haveFinal = HaveFinal();

	for (auto& cb : m_XformSelections)
		func(i++, cb, haveFinal && cb == m_XformSelections.back());
}

/// <summary>
/// Function to perform the specified operation on one dynamically created xform selection checkbox.
/// </summary>
/// <param name="i">The index of the checkbox</param>
/// <param name="func">The operation to perform</param>
/// <returns>True if the checkbox was found, else false.</returns>
template <typename T>
bool FractoriumEmberController<T>::XformCheckboxAt(size_t i, std::function<void(QCheckBox*)> func)
{
	if (i < m_Fractorium->m_XformSelections.size())
	{
		if (const auto w = m_Fractorium->m_XformSelections[i])
		{
			func(w);
			return true;
		}
	}

	return false;
}

/// <summary>
/// Function to perform the specified operation on one dynamically created xform selection checkbox.
/// The checkbox is specified by the xform it corresponds to, rather than its index.
/// </summary>
/// <param name="xform">The xform that corresponds to the checkbox</param>
/// <param name="func">The operation to perform</param>
/// <returns>True if the checkbox was found, else false.</returns>
template <typename T>
bool FractoriumEmberController<T>::XformCheckboxAt(Xform<T>* xform, std::function<void(QCheckBox*)> func)
{
	const auto forceFinal = m_Fractorium->HaveFinal();
	return XformCheckboxAt(m_Ember.GetTotalXformIndex(xform, forceFinal), func);
}

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
