#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the info UI.
/// </summary>
void Fractorium::InitInfoUI()
{
	const auto treeHeader = ui.SummaryTree->header();
	const auto tableHeader = ui.SummaryTable->horizontalHeader();
	treeHeader->setVisible(true);
	treeHeader->setSectionsClickable(true);
	treeHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
	connect(treeHeader, SIGNAL(sectionClicked(int)), this, SLOT(OnSummaryTreeHeaderSectionClicked(int)), Qt::QueuedConnection);
	connect(tableHeader, SIGNAL(sectionResized(int, int, int)), this, SLOT(OnSummaryTableHeaderResized(int, int, int)), Qt::QueuedConnection);
	SetFixedTableHeader(ui.SummaryTable->verticalHeader());
	ui.SummaryTable->setItem(0, 0, m_InfoNameItem = new QTableWidgetItem(""));
	ui.SummaryTable->setItem(1, 0, m_InfoPaletteItem = new QTableWidgetItem(""));
	ui.SummaryTable->setItem(2, 0, m_Info3dItem = new QTableWidgetItem(""));
	ui.SummaryTable->setItem(3, 0, m_InfoXaosItem = new QTableWidgetItem(""));
	ui.SummaryTable->setItem(4, 0, m_InfoXformCountItem = new QTableWidgetItem(""));
	ui.SummaryTable->setItem(5, 0, m_InfoFinalXformItem = new QTableWidgetItem(""));
	ui.InfoTabWidget->setCurrentIndex(0);//Make summary tab focused by default.
	ui.SummaryTree->SetMainWindow(this);
}

/// <summary>
/// Called when the palette cell of the summary table is resized in response
/// to a resizing of the Info dock.
/// </summary>
/// <param name="logicalIndex">Ignored</param>
/// <param name="oldSize">Ignored</param>
/// <param name="newSize">Ignored</param>
void Fractorium::OnSummaryTableHeaderResized(int logicalIndex, int oldSize, int newSize)
{
	QPixmap pixmap(QPixmap::fromImage(m_Controller->FinalPaletteImage()));//Create a QPixmap out of the QImage, will be empty on startup.
	SetPaletteTableItem(&pixmap, ui.SummaryTable, m_InfoPaletteItem, 1, 0);
}

/// <summary>
/// Expand or collapse the summary tree depending on the column index clicked.
/// 0: collapse, 1: expand.
/// </summary>
/// <param name="logicalIndex">The column which was clicked</param>
void Fractorium::OnSummaryTreeHeaderSectionClicked(int logicalIndex)
{
	if (const auto tree = ui.SummaryTree)
	{
		if (logicalIndex)
			tree->expandAll();
		else
			tree->collapseAll();
	}
}

/// <summary>
/// Fill the summary tree with values from the current ember.
/// This is meant to be a rough summary by containing only the most relevant
/// values from the ember.
/// It's also meant to be used in a fire-and-forget way. Once the tree is filled
/// individual nodes are never referenced again.
/// The entire tree is cleared and refilled whenever a render is completed.
/// This would seem inefficient, but it appears to update with no flicker.
/// If this ever presents a problem in the future, revisit with a more
/// intelligent design.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::FillSummary()
{
	const auto p = 3;
	const auto vp = 4;
	const auto vlen = 7;
	const auto pc = 'f';
	const auto iconSize = 20;
	const auto forceFinal = m_Fractorium->HaveFinal();
	const auto total = m_Ember.TotalXformCount(forceFinal);
	const auto table = m_Fractorium->ui.SummaryTable;
	const auto nondraggable = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	const auto draggable = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
	size_t x = 0;
	Xform<T>* xform = nullptr;
	QColor color;
	auto tree = m_Fractorium->ui.SummaryTree;
	tree->blockSignals(true);
	tree->clear();
	m_Fractorium->m_InfoNameItem->setText(m_Ember.m_Name.c_str());
	m_Fractorium->m_Info3dItem->setText(m_Ember.ProjBits() ? "Yes" : "No");
	m_Fractorium->m_InfoXaosItem->setText(m_Ember.XaosPresent() ? "Yes" : "No");
	m_Fractorium->m_InfoXformCountItem->setText(QString::number(m_Ember.XformCount()));
	m_Fractorium->m_InfoFinalXformItem->setText(m_Ember.UseFinalXform() ? "Yes" : "No");
	QPixmap pixmap(QPixmap::fromImage(m_FinalPaletteImage));//Create a QPixmap out of the QImage.
	QSize size(table->columnWidth(0), table->rowHeight(1) + 1);
	m_Fractorium->m_InfoPaletteItem->setData(Qt::DecorationRole, pixmap.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

	for (x = 0; x < total && (xform = m_Ember.GetTotalXform(x, forceFinal)); x++)
	{
		size_t i = 0;
		QString as = "Pre";
		auto item1 = new QTreeWidgetItem(tree);
		intmax_t linkedIndex = IsXformLinked(m_Ember, xform);
		QString linked = (linkedIndex != -1) ? (" Linked to " + QString::number(linkedIndex + 1)) : "";
		auto index = m_Ember.GetXformIndex(xform);
		m_Ember.CalcNormalizedWeights(m_NormalizedWeights);
		xform->SetPrecalcFlags();//Needed for HasPost() below.

		if (!m_Ember.IsFinalXform(xform) && index != -1)
		{
			item1->setText(0, "Xform " +
						   QString::number(x + 1) +
						   " (" + QLocale::system().toString(xform->m_Weight, pc, p) + ") (" +
						   QLocale::system().toString(double(m_NormalizedWeights[index]), pc, p) + ")" +
						   linked);
		}
		else
			item1->setText(0, "Final xform");

		item1->setText(1, xform->m_Name.c_str());
		item1->setFlags(nondraggable);
		auto affineItem = new QTreeWidgetItem(item1);
		affineItem->setText(0, "Affine");
		affineItem->setFlags(nondraggable);

		if (xform->m_Affine.IsZero())
			as += " Empty";
		else if (xform->m_Affine.IsID())
			as += " ID";

		if (xform->HasPost())
		{
			as += ", Post";

			if (xform->m_Post.IsZero())
				as += " Empty";//Don't need to check further for IsID() because post is not included if it's ID.
		}

		affineItem->setText(1, as);
		auto colorIndexItem = new QTreeWidgetItem(item1);
		colorIndexItem->setText(0, "Color index");
		colorIndexItem->setText(1, QLocale::system().toString(xform->m_ColorX, pc, p));
		colorIndexItem->setFlags(nondraggable | Qt::ItemNeverHasChildren);
		color = ColorIndexToQColor(xform->m_ColorX);
		color.setAlphaF(xform->m_Opacity);
		colorIndexItem->setBackground(1, color);
		colorIndexItem->setForeground(1, VisibleColor(color));
		auto colorSpeedItem = new QTreeWidgetItem(item1);
		colorSpeedItem->setText(0, "Color speed");
		colorSpeedItem->setText(1, QLocale::system().toString(xform->m_ColorSpeed, pc, p));
		colorSpeedItem->setFlags(nondraggable | Qt::ItemNeverHasChildren);
		auto opacityItem = new QTreeWidgetItem(item1);
		opacityItem->setText(0, "Opacity");
		opacityItem->setText(1, QLocale::system().toString(xform->m_Opacity, pc, p));
		opacityItem->setFlags(nondraggable | Qt::ItemNeverHasChildren);
		auto dcItem = new QTreeWidgetItem(item1);
		dcItem->setText(0, "Direct color");
		dcItem->setText(1, QLocale::system().toString(xform->m_DirectColor, pc, p));
		dcItem->setFlags(nondraggable | Qt::ItemNeverHasChildren);

		if (dcItem->text(0) != tree->LastNonVarField())
			throw "Last info tree non-variation index did not match expected value";

		while (auto var = xform->GetVariation(i++))
		{
			auto vitem = new VariationTreeWidgetItem(var->VariationId(), item1);
			vitem->setText(0, QString::fromStdString(var->Name()));
			vitem->setText(1, QLocale::system().toString(var->m_Weight, pc, vp).rightJustified(vlen, ' '));
			vitem->setFlags(draggable);
			auto qi = MakeVariationIcon(var, iconSize);
			vitem->setIcon(0, qi);

			if (const auto parVar = dynamic_cast<ParametricVariation<T>*>(var))
			{
				auto params = parVar->Params();

                for (size_t j = 0; j < parVar->ParamCount(); j++)
				{
					if (!params[j].IsPrecalc())
					{
						auto pitem = new QTreeWidgetItem(vitem);
						pitem->setText(0, params[j].Name().c_str());
						pitem->setText(1, QLocale::system().toString(params[j].ParamVal(), pc, vp).rightJustified(vlen, ' '));
						pitem->setFlags(nondraggable);
					}
				}
			}
		}

		const auto item2 = new QTreeWidgetItem(tree);//Empty item in between xforms.
	}

	tree->expandAll();
	tree->blockSignals(false);
}

void Fractorium::FillSummary()
{
	m_Controller->FillSummary();
}

/// <summary>
/// Reorder the variations of the xform for the passed in tree widget item.
/// Read the newly reordered variation items in order, removing each from the xform
/// corresponding to the passed in item, and storing them in a vector. Then re-add those variation
/// pointers back to the xform in the same order they were removed.
/// This will be called after the user performs a drag and drop operation on the variations in the
/// info tree. So the variations will be in the newly desired order.
/// </summary>
/// <param name="dme">Pointer to the parent (xform level) tree widget item which contains the variation item being dragged</param>
template <typename T>
void FractoriumEmberController<T>::ReorderVariations(QTreeWidgetItem* item)
{
	const auto tree = m_Fractorium->ui.SummaryTree;
	const auto xfindex = tree->indexOfTopLevelItem(item) / 2;//Blank lines each count as one.

	if (auto xform = m_Ember.GetTotalXform(xfindex))
	{
		vector<Variation<T>*> vars;
		vars.reserve(xform->TotalVariationCount());
		Update([&]
		{
			int i = 0;

			while (const auto ch = item->child(i))
			{
				if (ch->text(0) == tree->LastNonVarField())
				{
					i++;

					while (auto varch = dynamic_cast<VariationTreeWidgetItem*>(item->child(i++)))
						if (auto var = xform->RemoveVariationById(varch->Id()))
							vars.push_back(var);

					for (auto& var : vars)
						xform->AddVariation(var);

					break;
				}

				i++;
			}

		}, true, eProcessAction::FULL_RENDER);
	}
}

void Fractorium::ReorderVariations(QTreeWidgetItem* item)
{
	m_Controller->ReorderVariations(item);
}

/// <summary>
/// Update the histogram bounds display labels.
/// This shows the user the actual bounds of what's
/// being rendered. Mostly of engineering interest.
/// </summary>
void Fractorium::UpdateHistogramBounds()
{
	static QString ul, ur, lr, ll, wh, g, de;

	if (auto r = m_Controller->Renderer())
	{
		auto ulstr = ul.asprintf("UL: %3.3f, %3.3f", r->LowerLeftX(), r->UpperRightY());//These bounds include gutter padding.
		auto urstr = ur.asprintf("UR: %3.3f, %3.3f", r->UpperRightX(), r->UpperRightY());
		auto lrstr = lr.asprintf("LR: %3.3f, %3.3f", r->UpperRightX(), r->LowerLeftY());
		auto llstr = ll.asprintf("LL: %3.3f, %3.3f", r->LowerLeftX(), r->LowerLeftY());
        auto whstr = wh.asprintf("W x H: %4zu x %4zu", r->SuperRasW(), r->SuperRasH());
        auto gstr = g.asprintf("%zu", r->GutterWidth());
		ui.InfoBoundsLabelUL->setText(ulstr);
		ui.InfoBoundsLabelUR->setText(urstr);
		ui.InfoBoundsLabelLR->setText(lrstr);
		ui.InfoBoundsLabelLL->setText(llstr);
		ui.InfoBoundsLabelWH->setText(whstr);
		ui.InfoBoundsTable->item(0, 1)->setText(gstr);

		if (r->GetDensityFilter())
		{
			const auto deWidth = (r->GetDensityFilter()->FilterWidth() * 2) + 1;
            auto destr = de.asprintf("%jd x %jd", deWidth, deWidth);
			ui.InfoBoundsTable->item(1, 1)->setText(destr);
		}
		else
			ui.InfoBoundsTable->item(1, 1)->setText("N/A");
	}
}

/// <summary>
/// Fill the passed in QTextEdit with the vector of strings.
/// Optionally clear first.
/// Serves as a convenience function because the error reports coming
/// from Ember and EmberCL use vector<string>.
/// Use invokeMethod() in case this is called from a thread.
/// </summary>
/// <param name="errors">The vector of error strings</param>
/// <param name="textEdit">The QTextEdit to fill</param>
/// <param name="clear">Clear if true, else don't.</param>
void Fractorium::ErrorReportToQTextEdit(const vector<string>& errors, QTextEdit* textEdit, bool clear)
{
	if (clear)
		QMetaObject::invokeMethod(textEdit, "clear", Qt::QueuedConnection);

	for (auto& error : errors)
		QMetaObject::invokeMethod(textEdit, "append", Qt::QueuedConnection, Q_ARG(const QString&, QString::fromStdString(error) + "\n"));
}

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
