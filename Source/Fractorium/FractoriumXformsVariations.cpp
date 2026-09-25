#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the xforms variations UI.
/// </summary>
void Fractorium::InitXformsVariationsUI()
{
	const auto tree = ui.VariationsTree;
	tree->clear();
	tree->header()->setSectionsClickable(true);
	connect(tree->header(),                 SIGNAL(sectionClicked(int)),         this, SLOT(OnTreeHeaderSectionClicked(int)));
	connect(ui.VariationsFilterLineEdit,    SIGNAL(textChanged(const QString&)), this, SLOT(OnVariationsFilterLineEditTextChanged(const QString&)));
	connect(ui.VariationsFilterClearButton, SIGNAL(clicked(bool)),               this, SLOT(OnVariationsFilterClearButtonClicked(bool)));
	connect(ui.ActionVariationsDialog,      SIGNAL(triggered(bool)),             this, SLOT(OnActionVariationsDialog(bool)), Qt::ConnectionType::QueuedConnection);
	//Setting dimensions in the designer with a layout is futile, so must hard code here.
	tree->setColumnWidth(0, 170);
	tree->setColumnWidth(1, 80);
	//tree->setColumnWidth(2, 20);
	//Set Default variation tree text and background colors for zero and non zero cases.
	m_VariationTreeColorNonZero = Qt::GlobalColor::black;
	m_VariationTreeColorZero = Qt::GlobalColor::black;
	m_VariationTreeBgColorNonZero = Qt::GlobalColor::lightGray;
	m_VariationTreeBgColorZero = Qt::GlobalColor::white;
}

/// <summary>
/// Show the variations filter dialog.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionVariationsDialog(bool checked)
{
	if (m_VarDialog->exec())
	{
		m_Controller->FilteredVariations();
		Filter();
	}
}

/// <summary>
/// Apply the text passed in, in conjuction with the selections from
/// the variations filter dialog to only show variations whose names
/// contain the substring and are selected.
/// Called when the user types in the variation filter text box and
/// when the variations dialog exits.
/// </summary>
/// <param name="text">The text to filter on</param>
template <typename T>
void FractoriumEmberController<T>::Filter(const QString& text)
{
	const auto& ids = m_Fractorium->m_VarDialog->Map();
	const auto tree = m_Fractorium->ui.VariationsTree;
	const auto xform = CurrentXform();
	tree->setUpdatesEnabled(false);

	for (int i = 0; i < tree->topLevelItemCount(); i++)
	{
		if (auto item = dynamic_cast<VariationTreeWidgetItem*>(tree->topLevelItem(i)))
		{
			auto varName = item->text(0);

			if (xform && xform->GetVariationById(item->Id()))//If it's present then show it no matter what the filter is.
			{
				item->setHidden(false);
			}
			else if (ids.contains(varName))//If the varation is the map of all variations, which is should always be, consider it as well as the filter text.
			{
				item->setHidden(!varName.contains(text, Qt::CaseSensitivity::CaseInsensitive) || !ids[varName].toBool());
			}
			else//Wasn't present, which should never happen, so just consider filter text.
			{
				item->setHidden(!varName.contains(text, Qt::CaseSensitivity::CaseInsensitive));
			}
		}
	}

	m_Fractorium->OnTreeHeaderSectionClicked(m_Fractorium->m_VarSortMode);//Must re-sort every time the filter changes.
	tree->setUpdatesEnabled(true);
}

void Fractorium::Filter()
{
	m_Controller->Filter(ui.VariationsFilterLineEdit->text());
}

template <typename T>
void FractoriumEmberController<T>::FilteredVariations()
{
	const auto& map = m_Fractorium->m_VarDialog->Map();
	m_FilteredVariations.clear();
	m_FilteredVariations.reserve(map.size());

	for (size_t i = 0; i < m_VariationList->Size(); i++)
		if (const auto var = m_VariationList->GetVariation(i))
			if (map.contains(var->Name().c_str()) && map[var->Name().c_str()].toBool())
				m_FilteredVariations.push_back(var->VariationId());
}

/// <summary>
/// Dynamically populate the variation tree widget with VariationTreeWidgetItem and VariationTreeDoubleSpinBox
/// templated with the correct type.
/// This will clear any previous contents.
/// Called upon initialization, or controller type change.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::SetupVariationsTree()
{
	T fMin = TLOW;
	T fMax = TMAX;
	const int iconSize = 20;
	const QSize hint0(170, 16);
	const QSize hint1(80, 16);
	//const QSize hint2(20, 16);
	QPixmap pixmap(iconSize * 3, iconSize);
	auto mask = pixmap.createMaskFromColor(QColor("transparent"), Qt::MaskMode::MaskOutColor);
	auto tree = m_Fractorium->ui.VariationsTree;
	tree->clear();
	tree->blockSignals(true);
	pixmap.setMask(mask);
	QPainter paint(&pixmap);
	paint.fillRect(QRect(0, 0, iconSize * 3, iconSize), QColor(0, 0, 0, 0));
	QIcon defIcon(pixmap);

	for (size_t i = 0; i < m_VariationList->Size(); i++)
	{
		auto var = m_VariationList->GetVariation(i);
		auto parVar = dynamic_cast<const ParametricVariation<T>*>(var);
		//First add the variation, with a spinner for its weight.
		auto item = new VariationTreeWidgetItem(var->VariationId(), tree);
		auto spinBox = new VariationTreeDoubleSpinBox(tree, item, var->VariationId(), "");
		item->setText(0, QString::fromStdString(var->Name()));
		item->setSizeHint(0, hint0);
		item->setSizeHint(1, hint1);
		//item->setSizeHint(2, hint2);
		auto qi = MakeVariationIcon(var, iconSize);
		item->setIcon(0, qi);
		spinBox->setRange(fMin, fMax);
		spinBox->DoubleClick(true);
		spinBox->DoubleClickZero(1);
		spinBox->DoubleClickNonZero(0);
		spinBox->SmallStep(0.001);
		tree->setItemWidget(item, 1, spinBox);
		m_Fractorium->connect(spinBox, SIGNAL(valueChanged(double)), SLOT(OnVariationSpinBoxValueChanged(double)), Qt::ConnectionType::QueuedConnection);

		//Check to see if the variation was parametric, and add a tree entry with a spinner for each parameter.
		if (parVar)
		{
			auto params = parVar->Params();

			for (size_t j = 0; j < parVar->ParamCount(); j++)
			{
				if (!params[j].IsPrecalc())
				{
					auto def = params[j].Def();
					auto paramWidget = new VariationTreeWidgetItem(var->VariationId(), item);
					auto varSpinBox = new VariationTreeDoubleSpinBox(tree, paramWidget, parVar->VariationId(), params[j].Name());
					paramWidget->setText(0, params[j].Name().c_str());
					paramWidget->setSizeHint(0, hint0);
					paramWidget->setSizeHint(1, hint1);
					paramWidget->setIcon(0, defIcon);
					varSpinBox->setRange(params[j].Min(), params[j].Max());
					varSpinBox->setValue(params[j].ParamVal());
					varSpinBox->DoubleClick(true);
					varSpinBox->DoubleClickZero(def != 0 ? 0 : 1);
					varSpinBox->DoubleClickLowVal(def);
					varSpinBox->DoubleClickNonZero(def);

					if (params[j].Type() == eParamType::INTEGER || params[j].Type() == eParamType::INTEGER_NONZERO)
					{
						varSpinBox->setSingleStep(1);
						varSpinBox->Step(1);
						varSpinBox->SmallStep(1);
					}

					tree->setItemWidget(paramWidget, 1, varSpinBox);
					m_Fractorium->connect(varSpinBox, SIGNAL(valueChanged(double)), SLOT(OnVariationSpinBoxValueChanged(double)), Qt::ConnectionType::QueuedConnection);
				}
			}
		}
	}

	Filter("");
	tree->blockSignals(false);
}

/// <summary>
/// Set every spinner in the variation tree, including params, to zero.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::ClearVariationsTree()
{
	const auto tree = m_Fractorium->ui.VariationsTree;

	for (int i = 0; i < tree->topLevelItemCount(); i++)
	{
		const auto item = tree->topLevelItem(i);

		if (auto spinBox = dynamic_cast<VariationTreeDoubleSpinBox*>(tree->itemWidget(item, 1)))
		{
			spinBox->SetValueStealth(0);

			for (int j = 0; j < item->childCount(); j++)//Iterate through all of the children, which will be the params.
			{
				if (const auto varSpinBox = dynamic_cast<VariationTreeDoubleSpinBox*>(tree->itemWidget(item->child(j), 1)))//Cast the child widget to the VariationTreeDoubleSpinBox type.
					varSpinBox->SetValueStealth(0);
			}
		}
	}
}

/// <summary>
/// Copy the value of a variation or param spinner to its corresponding value
/// in the selected xforms.
/// Called when any spinner in the variations tree is changed.
/// Resets the rendering process.
/// </summary>
/// <param name="d">The spinner value</param>
template <typename T>
void FractoriumEmberController<T>::VariationSpinBoxValueChanged(double d)//Would be awesome to make this work for all.//TODO
{
	bool update = false;
	const auto objSender = m_Fractorium->sender();
	const auto tree = m_Fractorium->ui.VariationsTree;
	const auto sender = dynamic_cast<VariationTreeDoubleSpinBox*>(objSender);

	if (sender)
	{
		UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex)
		{
			const auto var = m_VariationList->GetVariation(sender->GetVariationId());//The variation attached to the sender, for reference only.
			const auto parVar = dynamic_cast<const ParametricVariation<T>*>(var);//The parametric cast of that variation.
			const auto xformVar = xform->GetVariationById(var->VariationId());//The corresponding variation in the currently selected xform.
			const auto widgetItem = sender->WidgetItem();
			const auto isParam = parVar && sender->IsParam();

			if (isParam)
			{
				//Do not take action if the xform doesn't contain the variation which this param is part of.
				if (const auto xformParVar = dynamic_cast<ParametricVariation<T>*>(xformVar))//The parametric cast of the xform's variation.
					if (xformParVar->SetParamVal(sender->ParamName().c_str(), d))
						update = true;
			}
			else
			{
				//If they spun down to zero, and it wasn't a parameter item,
				//and the current xform contained the variation, then remove the variation.
				if (IsNearZero(d))
				{
					if (xformVar)
						xform->DeleteVariationById(var->VariationId());

					widgetItem->setForeground(0, m_Fractorium->m_VariationTreeColorZero);
					widgetItem->setBackground(0, m_Fractorium->m_VariationTreeBgColorZero);
				}
				else
				{
					if (xformVar)//The xform already contained this variation, which means they just went from a non-zero weight to another non-zero weight (the simple case).
					{
						xformVar->m_Weight = d;
					}
					else
					{
						//If the item wasn't a param and the xform did not contain this variation,
						//it means they went from zero to a non-zero weight, so add a new copy of this xform.
						const auto newVar = var->Copy();//Create a new one with default values.
						newVar->m_Weight = d;
						xform->AddVariation(newVar);
						widgetItem->setForeground(0, m_Fractorium->m_VariationTreeColorNonZero);
						widgetItem->setBackground(0, m_Fractorium->m_VariationTreeBgColorNonZero);

						//If they've added a new parametric variation, then grab the values currently in the spinners
						//for the child parameters and assign them to the newly added variation.
						if (parVar)
						{
							const auto newParVar = dynamic_cast<ParametricVariation<T>*>(newVar);

							for (int i = 0; i < widgetItem->childCount(); i++)//Iterate through all of the children, which will be the params.
							{
								const auto childItem = widgetItem->child(i);//Get the child.
								const auto itemWidget = tree->itemWidget(childItem, 1);//Get the widget for the child.

								if (const auto spinBox = dynamic_cast<VariationTreeDoubleSpinBox*>(itemWidget))//Cast the widget to the VariationTreeDoubleSpinBox type.
								{
									string s = childItem->text(0).toStdString();//Use the name of the child, and the value of the spinner widget to assign the param.
									newParVar->SetParamVal(s.c_str(), spinBox->value());
								}
							}
						}
					}
				}

				update = true;
			}
		}, eXformUpdate::UPDATE_CURRENT_AND_SELECTED, false);

		if (update)
			UpdateRender();
	}
}

void Fractorium::OnVariationSpinBoxValueChanged(double d) { m_Controller->VariationSpinBoxValueChanged(d); }

/// <summary>
/// Fill in the variations tree with the values from the current xform.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::FillVariationTreeWithCurrentXform()
{
	FillVariationTreeWithXform(CurrentXform());
}

/// <summary>
/// Fill the variation tree values from passed in xform and apply the current sorting mode.
/// Called when the currently selected xform changes.
/// </summary>
/// <param name="xform">The xform whose variation values will be used to fill the tree</param>
template <typename T>
void FractoriumEmberController<T>::FillVariationTreeWithXform(Xform<T>* xform)
{
	const auto tree = m_Fractorium->ui.VariationsTree;
	tree->blockSignals(true);
	m_Fractorium->Filter();

	for (int i = 0; i < tree->topLevelItemCount(); i++)
	{
		const auto item = dynamic_cast<VariationTreeWidgetItem*>(tree->topLevelItem(i));
		const auto var = xform->GetVariationById(item->Id());//See if this variation in the tree was contained in the xform.
		const auto parVar = dynamic_cast<ParametricVariation<T>*>(var);//Attempt cast to parametric variation for later.
		const auto origParVar = dynamic_cast<const ParametricVariation<T>*>(m_VariationList->GetVariation(item->Id()));

		if (const auto spinBox = dynamic_cast<VariationTreeDoubleSpinBox*>(tree->itemWidget(item, 1)))//Get the widget for the item, and cast the widget to the VariationTreeDoubleSpinBox type.
		{
			if (var)//Ensure it's visible, even if it's supposed to be filtered.
				item->setHidden(false);

			spinBox->SetValueStealth(var ? var->m_Weight : 0);//If the variation was present, set the spin box to its weight, else zero.
			item->setForeground(0, var ? m_Fractorium->m_VariationTreeColorNonZero :  m_Fractorium->m_VariationTreeColorZero);
			item->setBackground(0, var ? m_Fractorium->m_VariationTreeBgColorNonZero :  m_Fractorium->m_VariationTreeBgColorZero);

			for (int j = 0; j < item->childCount(); j++)//Iterate through all of the children, which will be the params if it was a parametric variation.
			{
				T* param = nullptr;
				const auto childItem = item->child(j);//Get the child.
				const auto childItemWidget = tree->itemWidget(childItem, 1);//Get the widget for the child.

				if (const auto childSpinBox = dynamic_cast<VariationTreeDoubleSpinBox*>(childItemWidget))//Cast the widget to the VariationTreeDoubleSpinBox type.
				{
					string s = childItem->text(0).toStdString();//Get the name of the child.

					if (parVar)
					{
						if ((param = parVar->GetParam(s.c_str())))//Retrieve pointer to the param.
							childSpinBox->SetValueStealth(*param);
					}
					else if (origParVar)//Parametric variation was not present in this xform, so set child values to defaults.
					{
						if ((param = origParVar->GetParam(s.c_str())))
							childSpinBox->SetValueStealth(*param);
						else
							childSpinBox->SetValueStealth(0);//Will most likely never happen, but just to be safe.
					}
				}
			}
		}
	}

	tree->blockSignals(false);
	m_Fractorium->OnTreeHeaderSectionClicked(m_Fractorium->m_VarSortMode);
}

/// <summary>
/// Create an icon for the passed in variation which indicates the following:
///		Red: position matters because it uses non-standard assigning vs. summing.
///		Green: uses direct color.
///		Blue: maintains internal state, mostly of engineering interest.
/// </summary>
/// <param name="text">The variation to create the icon for</param>
/// <param name="size">The size of the icon in pixels</param>
/// <returns>The newly created icon</returns>
template <typename T>
QIcon FractoriumEmberController<T>::MakeVariationIcon(const Variation<T>* var, int iconSize)
{
	static vector<string> dc{ "m_ColorX" };
	static vector<string> assign{ "outPoint->m_X =", "outPoint->m_Y =", "outPoint->m_Z =",
								  "outPoint->m_X=", "outPoint->m_Y=", "outPoint->m_Z=" };
	QPixmap pixmap(iconSize * 3, iconSize);
	auto mask = pixmap.createMaskFromColor(QColor("transparent"), Qt::MaskMode::MaskOutColor);
	pixmap.setMask(mask);
	QPainter paint(&pixmap);
	paint.fillRect(QRect(0, 0, iconSize * 3, iconSize), QColor(0, 0, 0, 0));

	if (var->VarType() == eVariationType::VARTYPE_REG)
	{
		if (SearchVar(var, assign, false))
			paint.fillRect(QRect(0, 0, iconSize, iconSize), QColor(255, 0, 0));
	}
	else if (var->VarType() == eVariationType::VARTYPE_PRE || var->VarType() == eVariationType::VARTYPE_POST)
	{
		if (var->AssignType() == eVariationAssignType::ASSIGNTYPE_SUM)
			paint.fillRect(QRect(0, 0, iconSize, iconSize), QColor(255, 0, 0));
	}

	bool isDc = SearchVar(var, dc, false);

	if (isDc)
		paint.fillRect(QRect(iconSize, 0, iconSize, iconSize), QColor(0, 255, 0));

	if (!var->StateOpenCLString().empty())
		paint.fillRect(QRect(iconSize * 2, 0, iconSize, iconSize), QColor(0, 0, 255));

	QIcon qi(pixmap);
	return qi;
}

/// <summary>
/// Change the sorting to be either by variation ID, or by weight.
/// If sorting by variation ID, repeated clicks will alternate ascending or descending.
/// Called when the user clicks the tree headers.
/// </summary>
/// <param name="logicalIndex">Column index of the header clicked. Sort by name if 0, sort by weight if 1.</param>
void Fractorium::OnTreeHeaderSectionClicked(int logicalIndex)
{
	if (logicalIndex <= 1)
	{
		m_VarSortMode = logicalIndex;
		ui.VariationsTree->sortItems(m_VarSortMode, m_VarSortMode == 0 ? Qt::SortOrder::AscendingOrder : Qt::SortOrder::DescendingOrder);

		if (m_VarSortMode == 1)
			ui.VariationsTree->scrollToTop();
	}
}

/// <summary>
/// Apply the text in the variation filter text box to only show variations whose names
/// contain the substring.
/// Called when the user types in the variation filter text box.
/// </summary>
/// <param name="text">The text to filter on</param>
void Fractorium::OnVariationsFilterLineEditTextChanged(const QString& text)
{
	Filter();
}

/// <summary>
/// Clear the variation name filter, which will display all variations.
/// Called when the clear variations filter button is clicked.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnVariationsFilterClearButtonClicked(bool checked)
{
	ui.VariationsFilterLineEdit->clear();
}

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
