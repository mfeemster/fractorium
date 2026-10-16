#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the library tree UI.
/// </summary>
void Fractorium::InitLibraryUI()
{
	ui.LibraryTree->SetMainWindow(this);
	//Making the TreeItemChanged() events use a direct connection is absolutely critical.
	connect(ui.LibraryTree,                 SIGNAL(itemChanged(QTreeWidgetItem*, int)),	      this, SLOT(OnEmberTreeItemChanged(QTreeWidgetItem*, int)),	   Qt::ConnectionType::DirectConnection);
	connect(ui.LibraryTree,                 SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnEmberTreeItemDoubleClicked(QTreeWidgetItem*, int)), Qt::ConnectionType::QueuedConnection);
	connect(ui.LibraryTree,                 SIGNAL(itemActivated(QTreeWidgetItem*, int)),     this, SLOT(OnEmberTreeItemDoubleClicked(QTreeWidgetItem*, int)), Qt::ConnectionType::QueuedConnection);
	connect(ui.SequenceTree,                SIGNAL(itemChanged(QTreeWidgetItem*, int)),	      this, SLOT(OnSequenceTreeItemChanged(QTreeWidgetItem*, int)),	   Qt::ConnectionType::DirectConnection);
	connect(ui.SequenceStartPreviewsButton, SIGNAL(clicked(bool)),                            this, SLOT(OnSequenceStartPreviewsButtonClicked(bool)),          Qt::ConnectionType::QueuedConnection);
	connect(ui.SequenceStopPreviewsButton,  SIGNAL(clicked(bool)),                            this, SLOT(OnSequenceStopPreviewsButtonClicked(bool)),           Qt::ConnectionType::QueuedConnection);
	connect(ui.SequenceAllButton,           SIGNAL(clicked(bool)),                            this, SLOT(OnSequenceAllButtonClicked(bool)),                    Qt::ConnectionType::QueuedConnection);
	connect(ui.SequenceGenerateButton,      SIGNAL(clicked(bool)),                            this, SLOT(OnSequenceGenerateButtonClicked(bool)),               Qt::ConnectionType::QueuedConnection);
	connect(ui.SequenceRenderButton,        SIGNAL(clicked(bool)),                            this, SLOT(OnSequenceRenderButtonClicked(bool)),                 Qt::ConnectionType::QueuedConnection);
	connect(ui.SequenceAnimateButton,       SIGNAL(clicked(bool)),                            this, SLOT(OnSequenceAnimateButtonClicked(bool)),                Qt::ConnectionType::QueuedConnection);
	connect(ui.SequenceClearButton,         SIGNAL(clicked(bool)),                            this, SLOT(OnSequenceClearButtonClicked(bool)),                  Qt::ConnectionType::QueuedConnection);
	connect(ui.SequenceSaveButton,          SIGNAL(clicked(bool)),                            this, SLOT(OnSequenceSaveButtonClicked(bool)),                   Qt::ConnectionType::QueuedConnection);
	connect(ui.SequenceOpenButton,          SIGNAL(clicked(bool)),                            this, SLOT(OnSequenceOpenButtonClicked(bool)),                   Qt::ConnectionType::QueuedConnection);
	connect(ui.SequenceStartFlameSpinBox,   SIGNAL(valueChanged(int)),                        this, SLOT(OnSequenceStartFlameSpinBoxChanged(int)),             Qt::ConnectionType::QueuedConnection);
	connect(ui.SequenceStopFlameSpinBox,    SIGNAL(valueChanged(int)),                        this, SLOT(OnSequenceStopFlameSpinBoxChanged(int)),              Qt::ConnectionType::QueuedConnection);
	//Animation FPS.
	ui.SequenceAnimationFpsSpinBox->setValue(m_Settings->AnimationFps());
}

/// <summary>
/// Select the item in the library tree specified by the passed in index.
/// </summary>
/// <param name="index">The 0-based index of the item in the library tree to select</param>
void Fractorium::SelectLibraryItem(size_t index)
{
	EmberTreeWidgetItemBase* item = nullptr;

	if (const auto top = ui.LibraryTree->topLevelItem(0))
	{
		for (int i = 0; i < top->childCount(); i++)
		{
			if (auto emberItem = dynamic_cast<EmberTreeWidgetItemBase*>(top->child(i)))
			{
                auto b = i == static_cast<int>(index);

				if (b)
					item = emberItem;

				emberItem->setSelected(b);
				emberItem->setCheckState(NAME_COL, b ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
			}
		}

		if (item)
			ui.LibraryTree->scrollToItem(item, QAbstractItemView::EnsureVisible);
	}
}

/// <summary>
/// Get the index of the currently selected ember in the library tree.
/// </summary>
/// <param name="isChecked">Whether to search for items that are checked or items that are only selected</param>
/// <returns>A pair containing the index of the item clicked and a pointer to the item</param>
vector<pair<size_t, QTreeWidgetItem*>> Fractorium::GetCurrentEmberIndex(bool isChecked)
{
	int index = 0;
	QTreeWidgetItem* item = nullptr;
	const auto tree = ui.LibraryTree;
	vector<pair<size_t, QTreeWidgetItem*>> v;

	if (const auto top = tree->topLevelItem(0))
	{
		for (int i = 0; i < top->childCount(); i++)//Iterate through all of the children, which will represent the open embers.
		{
			item = top->child(index);

			if (item && item->isSelected())
			{
				if (isChecked)
				{
					if (item->checkState(NAME_COL) == Qt::CheckState::Checked)
						v.push_back(make_pair(index, item));
				}
				else
					v.push_back(make_pair(index, item));
			}

			index++;
		}
	}

	return v;
}

/// <summary>
/// Slot function to be called via QMetaObject::invokeMethod() to update preview images in the preview thread.
/// </summary>
/// <param name="item">The item double clicked on</param>
/// <param name="v">The vector holding the RGBA bitmap</param>
/// <param name="w">The width of the bitmap</param>
/// <param name="h">The height of the bitmap</param>
void Fractorium::SetTreeItemData(EmberTreeWidgetItemBase* item, vv4F& v, uint w, uint h)
{
	m_PreviewVec.resize(size_t(w) * size_t(h) * 4);
	Rgba32ToRgba8(v.data(), m_PreviewVec.data(), w, h, m_Settings->Transparency());
	item->SetImage(m_PreviewVec, w, h);
}

/// <summary>
/// Set all libary tree entries to the name of the corresponding ember they represent.
/// Set all libary tree entries to point to the underlying ember they represent.
/// </summary>
/// <param name="update">A bitfield representing the type of synchronizing to do. Update one or more of index, name or pointer.</param>
template <typename T>
void FractoriumEmberController<T>::SyncLibrary(uint update)
{
	auto it = m_EmberFile.m_Embers.begin();
	const auto tree = m_Fractorium->ui.LibraryTree;

	if (const auto top = tree->topLevelItem(0))
	{
		for (int i = 0; i < top->childCount() && it != m_EmberFile.m_Embers.end(); ++i, ++it)//Iterate through all of the children, which will represent the open embers.
		{
			if (auto emberItem = dynamic_cast<EmberTreeWidgetItem<T>*>(top->child(i)))//Cast the child widget to the EmberTreeWidgetItem type.
			{
				if (update & static_cast<uint>(eLibraryUpdate::INDEX))
					it->m_Index = i;

				if (update & static_cast<uint>(eLibraryUpdate::NAME))
					emberItem->setText(NAME_COL, QString::fromStdString(it->m_Name));

				if (update & static_cast<uint>(eLibraryUpdate::POINTER))
					emberItem->SetEmberPointer(&(*it));

				if (emberItem->checkState(NAME_COL) == Qt::CheckState::Checked)
					m_EmberFilePointer = emberItem->GetEmber();

				emberItem->setText(INDEX_COL, ToString(i));
			}
		}
	}
}

/// <summary>
/// Fill the library tree with the names of the embers in the
/// currently opened file.
/// Start preview render thread.
/// </summary>
/// <param name="selectIndex">After the tree is filled, select this index. Pass -1 to omit selecting an index.</param>
template <typename T>
void FractoriumEmberController<T>::FillLibraryTree(int selectIndex)
{
	StopAllPreviewRenderers();
	const uint size = PREVIEW_SIZE;
	vector<unsigned char> empy_preview(size * size * 4);
	const auto tree = m_Fractorium->ui.LibraryTree;
	tree->clear();
	auto fileItem = new QTreeWidgetItem(tree);
	QFileInfo info(m_EmberFile.m_Filename);
	fileItem->setText(NAME_COL, info.fileName());
	fileItem->setToolTip(NAME_COL, m_EmberFile.m_Filename);
	fileItem->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsDropEnabled);
	uint i = 0;

	for (auto& it : m_EmberFile.m_Embers)
	{
		it.m_Index = i;
		auto emberItem = new EmberTreeWidgetItem<T>(&it, fileItem);
		auto istr = ToString(i++);
		emberItem->setText(INDEX_COL, istr);

		if (it.m_Name.empty())
			emberItem->setText(NAME_COL, istr);
		else
			emberItem->setText(NAME_COL, it.m_Name.c_str());

		emberItem->setToolTip(NAME_COL, emberItem->text(NAME_COL));
		emberItem->SetImage(empy_preview, size, size);
		emberItem->SetEmberPointer(&it);
	}

	//tree->update();

	if (selectIndex != -1)
		m_Fractorium->SelectLibraryItem(selectIndex);

	m_Fractorium->SyncFileCountToSequenceCount();
	RenderLibraryPreviews(0, static_cast<uint>(m_EmberFile.Size()));
	tree->expandAll();
}

/// <summary>
/// Update the library tree with the newly added embers (most likely from pasting) and
/// only render previews for the new ones, without clearing the entire tree.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::UpdateLibraryTree()
{
	const uint size = PREVIEW_SIZE;
	vector<unsigned char> empy_preview(size * size * 4);
	const auto tree = m_Fractorium->ui.LibraryTree;

	if (auto top = tree->topLevelItem(0))
	{
		const int origChildCount = top->childCount();
		int i = origChildCount;

		for (auto it = Advance(m_EmberFile.m_Embers.begin(), i); it != m_EmberFile.m_Embers.end(); ++it)
		{
			auto emberItem = new EmberTreeWidgetItem<T>(&(*it), top);
			auto istr = ToString(i++);
			emberItem->setText(INDEX_COL, istr);

			if (it->m_Name.empty())
				emberItem->setText(NAME_COL, istr);
			else
				emberItem->setText(NAME_COL, it->m_Name.c_str());

			emberItem->setToolTip(NAME_COL, emberItem->text(NAME_COL));
			emberItem->SetImage(empy_preview, size, size);
		}

		//When adding elements, ensure all indices are sequential.
		SyncLibrary(static_cast<uint>(eLibraryUpdate::INDEX));
		m_Fractorium->SyncFileCountToSequenceCount();
		RenderLibraryPreviews(origChildCount, static_cast<uint>(m_EmberFile.Size()));
	}
}

/// <summary>
/// Copy the text of the item which was changed to the name of the current ember.
/// Ensure all names are unique in the opened file.
/// This seems to be called spuriously, so we do a check inside to make sure
/// the text was actually changed.
/// We also have to wrap the dynamic_cast call in a try/catch block  because this can
/// be called on a widget that has already been deleted.
/// </summary>
/// <param name="item">The libary tree item changed</param>
/// <param name="col">The column clicked, ignored.</param>
template <typename T>
void FractoriumEmberController<T>::EmberTreeItemChanged(QTreeWidgetItem* item, int col)
{
	try
	{
		if (auto emberItem = dynamic_cast<EmberTreeWidgetItem<T>*>(item))
		{
			auto oldName = emberItem->GetEmber()->m_Name;//First preserve the previous name.
			auto newName = emberItem->text(NAME_COL).toStdString();

			//Checking/unchecking other items shouldn't perform the processing below.
			//If nothing changed, nothing to do.
			if (!emberItem->isSelected() && newName == oldName)
				return;

			if (newName.empty())//Prevent empty string.
			{
				emberItem->UpdateEditText();
				return;
			}

			emberItem->UpdateEmberName();//Copy edit text to the ember's name variable.
			m_EmberFile.MakeNamesUnique();//Ensure all names remain unique.
			SyncLibrary(static_cast<uint>(eLibraryUpdate::NAME));//Copy all ember names to the tree items since some might have changed to be made unique.
			newName = emberItem->GetEmber()->m_Name;//Get the new, final, unique name.

			if (m_EmberFilePointer && m_EmberFilePointer == emberItem->GetEmber() && oldName != newName)//If the ember edited was the current one, and the name was indeed changed, update the name of the current one.
			{
				m_Ember.m_Name = newName;
				m_LastSaveCurrent = "";//Reset will force the dialog to show on the next save current since the user probably wants a different name.
			}
		}
		else if (const auto parentItem = dynamic_cast<QTreeWidgetItem*>(item))
		{
			const auto text = parentItem->text(NAME_COL);

			if (text != "")
			{
				m_EmberFile.m_Filename = text;
				//m_LastSaveAll = "";//Reset will force the dialog to show on the next save all since the user probably wants a different name.
			}
		}
	}
	catch (const std::exception& e)
	{
		qDebug() << "FractoriumEmberController<T>::EmberTreeItemChanged() : Exception thrown: " << e.what();
	}
}

void Fractorium::OnEmberTreeItemChanged(QTreeWidgetItem* item, int col)
{
	if (item && ui.LibraryTree->topLevelItemCount())//This can sometimes be spurriously called even when the tree is empty.
		m_Controller->EmberTreeItemChanged(item, col);
}

/// <summary>
/// Set the current ember to the selected item.
/// Clears the undo state.
/// Resets the rendering process.
/// Called when the user double clicks on a library tree item.
/// This will get called twice for some reason, so the check state is checked to prevent duplicate processing.
/// </summary>
/// <param name="item">The item double clicked on</param>
/// <param name="col">The column clicked</param>
template <typename T>
void FractoriumEmberController<T>::EmberTreeItemDoubleClicked(QTreeWidgetItem* item, int col)
{
	if (item->checkState(col) == Qt::CheckState::Unchecked)
		SetEmber(m_Fractorium->ui.LibraryTree->currentIndex().row(), false);
}

void Fractorium::OnEmberTreeItemDoubleClicked(QTreeWidgetItem* item, int col)
{
	if (ui.LibraryTree->topLevelItemCount())//This can sometimes be spurriously called even when the tree is empty.
		m_Controller->EmberTreeItemDoubleClicked(item, col);
}

/// <summary>
/// Move a possibly disjoint selection of library items from their indices,
/// to another index.
/// </summary>
/// <param name="items">The selected list of items to move</param>
/// <param name="destRow">The destination index to move the item to</param>
template <typename T>
void FractoriumEmberController<T>::MoveLibraryItems(const QModelIndexList& items, int destRow)
{
	const auto tree = m_Fractorium->ui.LibraryTree;
	list<string> names;

	for (auto& item : items)
		if (auto temp = m_EmberFile.Get(item.row()))
			names.push_back(temp->m_Name);

	auto b = m_EmberFile.m_Embers.begin();
	/*const auto result = */Gather(b, m_EmberFile.m_Embers.end(), Advance(b, destRow), [&](const Ember<T>& ember)
	{
		auto position = std::find(names.begin(), names.end(), ember.m_Name);

		if (position != names.end())
		{
			names.erase(position);
			return true;
		}

		return false;
	});
	tree->update();
	SyncLibrary(static_cast<uint>(eLibraryUpdate::INDEX) | static_cast<uint>(eLibraryUpdate::NAME) | static_cast<uint>(eLibraryUpdate::POINTER));
}

/// <summary>
/// Delete the currently selected items in the tree.
/// Note this is not necessarilly the current ember, it's just the items
/// in the tree that are selected.
/// </summary>
/// <param name="v">A vector of pairs, each containing the index of the item selected and a pointer to the item</param>
template <typename T>
void FractoriumEmberController<T>::Delete(const vector<pair<size_t, QTreeWidgetItem*>>& v)
{
	size_t offset = 0;
	uint last = 0;

	for (auto& p : v)
	{
		if (p.second && m_EmberFile.Delete(p.first - offset))
		{
			last = uint(p.first - offset);
			delete p.second;
			SyncLibrary(static_cast<uint>(eLibraryUpdate::INDEX) | static_cast<uint>(eLibraryUpdate::NAME) | static_cast<uint>(eLibraryUpdate::POINTER));
			m_Fractorium->SyncFileCountToSequenceCount();
		}

		offset++;
	}

	//Select the next item in the tree closest to the last one that was deleted.
	if (const auto top = m_Fractorium->ui.LibraryTree->topLevelItem(0))
	{
		last = std::min<uint>(top->childCount() - 1, last);

		if (auto item = dynamic_cast<EmberTreeWidgetItem<T>*>(top->child(last)))
			if (item->GetEmber()->m_Name != m_Ember.m_Name)
				EmberTreeItemDoubleClicked(item, 0);
	}
}

/// <summary>
/// Called when the user presses and releases the delete key while the library tree has the focus,
/// and an item is selected.
/// </summary>
/// <param name="v">A vector of pairs, each containing the index of the item selected and a pointer to the item</param>
void Fractorium::OnDelete(const vector<pair<size_t, QTreeWidgetItem*>>& v)
{
	m_Controller->Delete(v);
}

/// <summary>
/// Stop the preview renderer if it's already running.
/// Clear all of the existing preview images, then start the preview rendering thread.
/// Optionally only render previews for a subset of all open embers.
/// </summary>
/// <param name="start">The 0-based index to start rendering previews for</param>
/// <param name="end">The 0-based index which is one beyond the last ember to render a preview for</param>
template <typename T>
void FractoriumEmberController<T>::RenderPreviews(QTreeWidget* tree, TreePreviewRenderer<T>* renderer, EmberFile<T>& file, uint start, uint end)
{
	renderer->Stop();

	if (start == UINT_MAX && end == UINT_MAX)
	{
		// Animated item might be at index 0, previews go in last item.
		if (const auto top = tree->topLevelItem(tree->topLevelItemCount() - 1))
		{
			const auto childCount = top->childCount();
			vector<unsigned char> emptyPreview(PREVIEW_SIZE * PREVIEW_SIZE * 4);

			for (int i = 0; i < childCount; i++)
				if (auto treeItem = dynamic_cast<EmberTreeWidgetItemBase*>(top->child(i)))
					treeItem->SetImage(emptyPreview, PREVIEW_SIZE, PREVIEW_SIZE);
		}

		renderer->Render(0, uint(file.Size()));
	}
	else
		renderer->Render(start, end);
}

/// <summary>
/// Wrapper around calling RenderPreviews with the appropriate values passed in for the previews in the main library tree.
/// </summary>
/// <param name="start">The 0-based index to start rendering previews for</param>
/// <param name="end">The 0-based index which is one beyond the last ember to render a preview for</param>
template <typename T>
void FractoriumEmberController<T>::RenderLibraryPreviews(uint start, uint end)
{
	RenderPreviews(m_Fractorium->ui.LibraryTree, m_LibraryPreviewRenderer.get(), m_EmberFile, start, end);
}

template <typename T>
void FractoriumEmberController<T>::StopLibraryPreviewRender()
{
	m_LibraryPreviewRenderer->Stop();
	QApplication::processEvents();
}

/// <summary>
/// Thing wrapper around StopLibraryPreviewRender() and StopSequencePreviewRender() to stop both preview renderers.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::StopAllPreviewRenderers()
{
	StopLibraryPreviewRender();
	StopSequencePreviewRender();
}

template <typename T>
void FractoriumEmberController<T>::AddAnimationItem()
{
	auto fileItem = new QTreeWidgetItem(m_Fractorium->ui.SequenceTree);
	fileItem->setText(NAME_COL, "Rendered Animation");
	fileItem->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
	auto emberItem = new EmberTreeWidgetItemBase(fileItem);
	emberItem->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
	emberItem->setToolTip(INDEX_COL, "Animated Frame");
	const uint size = PREVIEW_SIZE;
	vector<unsigned char> empy_preview(size * size * 4);
	emberItem->SetImage(empy_preview, size, size);
}

/// <summary>
/// Fill the sequence tree with the names of the embers in the
/// currently generated sequence.
/// Start the sequence preview render thread.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::FillSequenceTree()
{
	StopAllPreviewRenderers();
	const uint size = PREVIEW_SIZE;
	vector<unsigned char> empy_preview(size * size * 4);
	const auto tree = m_Fractorium->ui.SequenceTree;
	tree->clear();
	//Add extra TreeWidget for animation at index 0.
	AddAnimationItem();
	m_AnimateTimer->stop();
	auto fileItem = new QTreeWidgetItem(tree);
	QFileInfo info(m_SequenceFile.m_Filename);
	fileItem->setText(NAME_COL, info.fileName());
	fileItem->setToolTip(NAME_COL, m_SequenceFile.m_Filename);
	fileItem->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsSelectable);
	uint i = 0;

	for (auto& it : m_SequenceFile.m_Embers)
	{
		auto emberItem = new EmberTreeWidgetItemBase(fileItem);
		auto istr = ToString(i++);

		if (it.m_Name.empty())
			emberItem->setText(NAME_COL, istr);
		else
			emberItem->setText(NAME_COL, it.m_Name.c_str());

		emberItem->setText(INDEX_COL, istr);
		emberItem->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
		emberItem->setToolTip(NAME_COL, emberItem->text(NAME_COL));
		emberItem->SetImage(empy_preview, size, size);
	}

	tree->expandAll();
	//Hide, then show the animation item.
	tree->collapseItem(tree->topLevelItem(0));
	RenderSequencePreviews(0, uint(m_SequenceFile.Size()));

	if (const auto animation = tree->topLevelItem(0))
	{
		animation->setExpanded(true);
		m_AnimateFrame = 0;
		m_AnimateTimer->start(1000 / m_Fractorium->ui.SequenceAnimationFpsSpinBox->value());
	}
}

/// <summary>
/// Copy the text of the root item to the name of the sequence file.
/// Called whenever the text of the root item is changed.
/// </summary>
/// <param name="item">The root sequence tree item which changed</param>
/// <param name="col">The column clicked, ignored.</param>
template <typename T>
void FractoriumEmberController<T>::SequenceTreeItemChanged(QTreeWidgetItem* item, int col)
{
	if (item == m_Fractorium->ui.SequenceTree->topLevelItem(1))
	{
		auto text = item->text(NAME_COL);

		if (text != "")
			m_SequenceFile.m_Filename = text;
	}
}

void Fractorium::OnSequenceTreeItemChanged(QTreeWidgetItem* item, int col)
{
	if (item && ui.SequenceTree->topLevelItemCount())
		m_Controller->SequenceTreeItemChanged(item, col);
}

/// <summary>
/// Wrapper around calling RenderPreviews with the appropriate values passed in for the previews in the sequence tree.
/// Called when Render Previews is clicked.
/// </summary>
/// <param name="start">Ignored, render all.</param>
/// <param name="end">Ignored, render all.</param>
template <typename T>
void FractoriumEmberController<T>::RenderSequencePreviews(uint start, uint end)
{
	RenderPreviews(m_Fractorium->ui.SequenceTree, m_SequencePreviewRenderer.get(), m_SequenceFile, start, end);
}
void Fractorium::OnSequenceStartPreviewsButtonClicked(bool checked) { m_Controller->RenderSequencePreviews(); }

/// <summary>
/// Stop rendering the sequence previews.
/// Called when Stop Previews is clicked.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::StopSequencePreviewRender()
{
	m_SequencePreviewRenderer->Stop();
	QApplication::processEvents();
}
void Fractorium::OnSequenceStopPreviewsButtonClicked(bool checked) { m_Controller->StopSequencePreviewRender(); }

/// <summary>
/// Set the start and stop spin boxes to 0 and the length of the ember file minus 1, respectively.
/// Called whenever the count of the current file changes or when All is clicked.
/// </summary>
void Fractorium::SyncFileCountToSequenceCount()
{
	if (const auto top = ui.LibraryTree->topLevelItem(0))
	{
		const int count = top->childCount() - 1;
		ui.LibraryTree->headerItem()->setText(NAME_COL, "Current Flame File (" + QString::number(top->childCount()) + ")");
		ui.SequenceStartFlameSpinBox->setMinimum(0);
		ui.SequenceStartFlameSpinBox->setMaximum(count);
		ui.SequenceStartFlameSpinBox->setValue(0);
		ui.SequenceStopFlameSpinBox->setMinimum(0);
		ui.SequenceStopFlameSpinBox->setMaximum(count);
		ui.SequenceStopFlameSpinBox->setValue(count);
	}
}
void Fractorium::OnSequenceAllButtonClicked(bool checked) { SyncFileCountToSequenceCount(); }

/// <summary>
/// Generate an animation sequence and place it in the sequence tree. This code is
/// mostly similar to that of EmberGenome.
/// It differs in a few ways:
///		The number of frames used in a rotation and in blending can differ. In EmberGenome, they are the same.
///		The number of rotations, frames used in rotations and frames used in blending can all be randomized.
/// Called when the Generate button is clicked.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::SequenceGenerateButtonClicked()
{
	StopAllPreviewRenderers();
	SaveCurrentToOpenedFile(false);
	Ember<T> result;
	auto& ui = m_Fractorium->ui;
	auto s = m_Fractorium->m_Settings;
	//Number of rotations performed during interpolation.
	const size_t start = ui.SequenceStartFlameSpinBox->value();
	const size_t stop = ui.SequenceStopFlameSpinBox->value();
	const size_t startCount = ui.SequenceStartCountSpinBox->value();
	//const size_t keyFrames = (stop - start) + 1;
	size_t frameCount = 0;
	size_t fps = ui.SequenceAnimationFpsSpinBox->value();
	double frames = 0;
	vector<pair<size_t, size_t>> devices;//Dummy.
	EmberReport emberReport;
	ostringstream os;
	string palettePath = FindFirstDefaultPalette().toStdString();

	if (palettePath.empty())//This should never happen because the program requires a palette file to run this far.
	{
		QMessageBox::warning(nullptr, "Sequence", "No flam3-palettes.xml file found, sequence will not be generated.");
		return;
	}

	SheepTools<T, float> tools(palettePath, EmberCommon::CreateRenderer<T>(eRendererType::CPU_RENDERER, devices, false, 0, emberReport));
	auto it = Advance(m_EmberFile.m_Embers.begin(), start);

	for (size_t i = start; i <= stop && it != m_EmberFile.m_Embers.end(); i++, ++it)
	{
		if ((!it->m_Rotations || !it->m_SecondsPerRotation) && !it->m_BlendSeconds)
		{
			QMessageBox::critical(m_Fractorium, "Animation sequence parameters error",
								  "Rotations and seconds per rotation, or blend seconds must be positive and non-zero");
			return;
		}

		frames += fps * it->m_SecondsPerRotation * it->m_Rotations;

		if (i < stop)
			frames += fps * it->m_BlendSeconds;
	}

	frames += startCount;
	os << setfill('0') << setprecision(0) << fixed;
	m_SequenceFile.Clear();
	m_SequenceFile.m_Filename = EmberFile<T>::DefaultFilename("Sequence_");
	double blend;
	size_t frame;
	Ember<T> embers[2];//Spin needs contiguous array below, and this will also get modified, so a copy is needed to avoid modifying the embers in the original file.
	const auto padding = streamsize(std::log10(frames)) + 1;
	auto lastLoopsBlendCw = false;
	it = Advance(m_EmberFile.m_Embers.begin(), start);

	for (size_t i = start; i <= stop && it != m_EmberFile.m_Embers.end(); i++, ++it)
	{
		//The direction to rotate the loops.
		const auto loopsCw = it->m_RotateXformsCw;//ui.SequenceRotationsCWCheckBox->isChecked();
		const auto loopsBlendCw = it->m_BlendRotateXformsCw;//ui.SequenceRotationsPerBlendCWCheckBox->isChecked();
		//Whether to stagger, default is 1 which means no stagger.
		const auto stagger = it->m_Stagger;//ui.SequenceStaggerSpinBox->value();
		//Rotations on keyframes.
		const auto rotations = it->m_Rotations;
		//Number of frames it takes to rotate a keyframe.
        const auto rotFrames = fps * static_cast<double>(it->m_SecondsPerRotation);
		//Number of frames it takes to interpolate.
		const auto framesBlend = fps * it->m_BlendSeconds;
		const auto rotsPerBlend = it->m_RotationsPerBlend;
		const auto linear = it->m_Linear;
		tools.SetSpinParams(!linear,
							stagger,
							0,
							0,
							s->Nick().toStdString(),
							s->Url().toStdString(),
							s->Id().toStdString(),
							"",
							0,
							0);
		embers[0] = *it;

		if (rotations > 0)
		{
			const auto roundFrames = size_t(std::round(rotFrames * rotations));

			for (frame = 0; frame < roundFrames; frame++)
			{
				blend = frame / rotFrames;
				tools.Spin(embers[0], nullptr, result, startCount + frameCount++, blend, loopsCw);//Result is cleared and reassigned each time inside of Spin().
				FormatName(result, os, padding);
				m_SequenceFile.m_Embers.push_back(result);
			}

			//The loop above will have rotated just shy of a complete rotation.
			//Rotate the next step and save in result, but do not print.
			//result will be the starting point for the interp phase below.
			frame = roundFrames;
			blend = frame / rotFrames;
			tools.Spin(embers[0], nullptr, result, startCount + frameCount, blend, loopsCw);//Do not increment frameCount here.
			FormatName(result, os, padding);
		}

		if (i < stop)
		{
			if (rotations > 0)//Store the last result as the flame to interpolate from. This applies for whole or fractional values of opt.Loops().
				embers[0] = result;

			auto it2 = it;//Need a quick temporary to avoid modifying it, which is used in the loop.
			embers[1] = *(++it2);//Get the next ember to be used with blending below.
			const auto d = double(rotsPerBlend);
			const auto rpb = size_t(std::round(d));

			for (frame = 0; frame < framesBlend; frame++)
			{
				const auto seqFlag = frame == 0 || (frame == framesBlend - 1);
				blend = frame / framesBlend;
				result.Clear();
				tools.SpinInter(&embers[0], nullptr, result, startCount + frameCount++, seqFlag, blend, rpb, loopsBlendCw);
				FormatName(result, os, padding);
				m_SequenceFile.m_Embers.push_back(result);
			}
		}

		lastLoopsBlendCw = loopsBlendCw;
	}

	it = Advance(m_EmberFile.m_Embers.begin(), stop);
	tools.Spin(*it, nullptr, result, startCount + frameCount, 0, lastLoopsBlendCw);
	FormatName(result, os, padding);
	m_SequenceFile.m_Embers.push_back(result);
	FillSequenceTree();//The sequence has been generated, now create preview thumbnails.
}
void Fractorium::OnSequenceGenerateButtonClicked(bool checked) { m_Controller->SequenceGenerateButtonClicked(); }

/// <summary>
/// Show the final render dialog and load the sequence into it.
/// This will automatically check the Render All and Render as Animation sequence checkboxes.
/// Called when the Render Sequence button is clicked.
/// </summary>
/// <param name="checked">Ignored.</param>
void Fractorium::OnSequenceRenderButtonClicked(bool checked)
{
	if (ui.SequenceTree->topLevelItemCount() > 0)
	{
		//First completely stop what the current rendering process is doing.
		m_Controller->DeleteRenderer();//Delete the renderer, but not the controller.
		m_Controller->StopAllPreviewRenderers();
		m_Controller->SaveCurrentToOpenedFile(false);//Save whatever was edited back to the current open file.
		m_RenderStatusLabel->setText("Renderer stopped.");
		SetupFinalRenderDialog();

		if (m_FinalRenderDialog)
			m_FinalRenderDialog->Show(true);//Show with a bool specifying that it came from the sequence generator.
	}
}

/// <summary>
/// Animate the sequence
/// </summary>
template <typename T>
void FractoriumEmberController<T>::SequenceAnimateNextFrame()
{
	const auto tree = m_Fractorium->ui.SequenceTree;

	if (const auto renders = tree->topLevelItem(1))
	{
		if (renders->childCount())
		{
			const auto animate = dynamic_cast<EmberTreeWidgetItemBase*>(tree->topLevelItem(0)->child(0));
			const auto frame = m_AnimateFrame++ % renders->childCount();
			const auto nth = dynamic_cast<EmberTreeWidgetItemBase*>(renders->child(frame));

			if (animate && nth)
			{
				if (!nth->m_Rendered)
				{
					m_AnimateFrame = 0;
				}
				else
				{
					animate->m_Pixmap = QPixmap(nth->m_Pixmap);
					animate->setData(NAME_COL, Qt::ItemDataRole::DecorationRole, animate->m_Pixmap);
				}
			}
		}
	}
}

/// <summary>
/// Animate the sequence
/// </summary>
template <typename T>
void FractoriumEmberController<T>::SequenceAnimateButtonClicked()
{
	if (const auto animation = m_Fractorium->ui.SequenceTree->topLevelItem(0))
	{
		if (animation->isExpanded() && m_AnimateTimer->isActive())
		{
			animation->setExpanded(false);
			m_AnimateTimer->stop();
		}
		else
		{
			animation->setExpanded(true);
			m_AnimateFrame = 0;
			m_AnimateTimer->start(1000 / m_Fractorium->ui.SequenceAnimationFpsSpinBox->value());
		}
	}
}
void Fractorium::OnSequenceAnimateButtonClicked(bool checked) { m_Controller->SequenceAnimateButtonClicked(); }

/// <summary>
/// Clear the sequence.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::SequenceClearButtonClicked()
{
	m_SequencePreviewRenderer->Stop();
	m_Fractorium->ui.SequenceTree->clear();
}
void Fractorium::OnSequenceClearButtonClicked(bool checked) { m_Controller->SequenceClearButtonClicked(); }

/// <summary>
/// Save the sequence to a file.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::SequenceSaveButtonClicked()
{
	auto s = m_Fractorium->m_Settings;
	QString filename = m_Fractorium->SetupSaveXmlDialog(m_SequenceFile.m_Filename);

	if (filename != "")
	{
		EmberToXml<T> writer;
		QFileInfo fileInfo(filename);

		for (auto& ember : m_SequenceFile.m_Embers)
			ApplyXmlSavingTemplate(ember);

		if (writer.Save(filename.toStdString().c_str(), m_SequenceFile.m_Embers, 0, true, true, false, false, false))
			s->SaveFolder(fileInfo.canonicalPath());
		else
			m_Fractorium->ShowCritical("Save Failed", "Could not save sequence file, try saving to a different folder.");
	}
}
void Fractorium::OnSequenceSaveButtonClicked(bool checked) { m_Controller->SequenceSaveButtonClicked(); }

/// <summary>
/// Open one or more sequence file, concatenate them all, place them in the sequence
/// tree and begin rendering the sequence previews.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::SequenceOpenButtonClicked()
{
	m_SequencePreviewRenderer->Stop();
	auto filenames = m_Fractorium->SetupOpenXmlDialog();

	if (!filenames.empty())
	{
		size_t i;
		EmberFile<T> emberFile;
		XmlToEmber<T> parser;
		vector<Ember<T>> embers;
		vector<string> errors;
		emberFile.m_Filename = filenames[0];

		for (auto& filename : filenames)
		{
			embers.clear();

			if (parser.Parse(filename.toStdString().c_str(), embers, true) && !embers.empty())
			{
				for (i = 0; i < embers.size(); i++)
					if (embers[i].m_Name == "" || embers[i].m_Name == "No name")//Ensure it has a name.
						embers[i].m_Name = ToString<qulonglong>(i).toStdString();

				emberFile.m_Embers.insert(emberFile.m_Embers.end(), embers.begin(), embers.end());
				errors = parser.ErrorReport();
			}
			else
			{
				errors = parser.ErrorReport();
				m_Fractorium->ShowCritical("Open Failed", "Could not open sequence file, see info tab for details.");
			}

			if (!errors.empty())
				m_Fractorium->ErrorReportToQTextEdit(errors, m_Fractorium->ui.InfoFileOpeningTextEdit, false);//Concat errors from all files.
		}

		if (emberFile.Size() > 0)//Ensure at least something was read.
		{
			emberFile.MakeNamesUnique();
			m_SequenceFile = std::move(emberFile);//Move the temp to avoid creating dupes because we no longer need it.
			FillSequenceTree();
		}
	}
}
void Fractorium::OnSequenceOpenButtonClicked(bool checked) { m_Controller->SequenceOpenButtonClicked(); }

/// <summary>
/// Constrain all min/max spinboxes.
/// </summary>
void Fractorium::OnSequenceStartFlameSpinBoxChanged(int d) { ui.SequenceStopFlameSpinBox->setMinimum(d);                                               }
void Fractorium::OnSequenceStopFlameSpinBoxChanged(int d)  { if (ui.SequenceStopFlameSpinBox->hasFocus()) ui.SequenceStartFlameSpinBox->setMaximum(d); }

/// <summary>
/// Save all sequence settings to match the values in the controls.
/// </summary>
void Fractorium::SyncSequenceSettings()
{
	m_Settings->AnimationFps(ui.SequenceAnimationFpsSpinBox->value());
}

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
