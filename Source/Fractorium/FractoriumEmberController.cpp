#include "FractoriumPch.h"
#include "FractoriumEmberController.h"
#include "Fractorium.h"
#include "GLEmberController.h"

/// <summary>
/// Constructor which initializes the non-templated members contained in this class.
/// The renderer, other templated members and GUI setup will be done in the templated derived controller class.
/// </summary>
/// <param name="fractorium">Pointer to the main window.</param>
FractoriumEmberControllerBase::FractoriumEmberControllerBase(Fractorium* fractorium)
	: m_PaletteList(PaletteList<float>::Instance())
{
	Timing t;
	m_Fractorium = fractorium;
	m_Rand = QTIsaac<ISAAC_SIZE, ISAAC_INT>(ISAAC_INT(t.Tic()), ISAAC_INT(t.Tic() * 2), ISAAC_INT(t.Tic() * 3));//Ensure a different rand seed on each instance.
	m_RenderTimer = make_unique<QTimer>(m_Fractorium);
	m_RenderTimer->setInterval(0);
	m_Fractorium->connect(m_RenderTimer.get(), SIGNAL(timeout()), SLOT(IdleTimer()));
	m_RenderRestartTimer = make_unique<QTimer>(m_Fractorium);
	m_AnimateTimer = make_unique<QTimer>(m_Fractorium);
	m_AnimateTimer->stop();
	m_Fractorium->connect(m_RenderRestartTimer.get(), &QTimer::timeout, [&]() { m_Fractorium->StartRenderTimer(false); });//It's ok to pass false for the first shot because creating the controller will start the preview renders.
	m_Fractorium->connect(m_AnimateTimer.get(), &QTimer::timeout, [&]() { SequenceAnimateNextFrame(); });
}

/// <summary>
/// Destructor which stops rendering and deletes the timers.
/// All other memory is cleared automatically through the use of STL.
/// </summary>
FractoriumEmberControllerBase::~FractoriumEmberControllerBase()
{
	StopRenderTimer(true);
	m_RenderTimer->stop();
	m_RenderRestartTimer->stop();
	m_AnimateTimer->stop();
}

/// <summary>
/// Pause or resume the renderer.
/// </summary>
/// <param name="pause">True to pause, false to unpause.</param>
void FractoriumEmberControllerBase::Pause(bool pause)
{
	m_Renderer->Pause(pause);
}

/// <summary>
/// Retrieve the paused state of the renderer.
/// </summary>
/// <returns>True if the renderer is paused, else false.</returns>
bool FractoriumEmberControllerBase::Paused()
{
	return m_Renderer->Paused();
}

/// <summary>
/// Constructor which passes the main window parameter to the base, initializes the templated members contained in this class.
/// Then sets up the parts of the GUI that require templated Widgets, such as the variations tree and the palette table.
/// Note the renderer is not setup here automatically. Instead, it must be manually created by the caller later.
/// </summary>
/// <param name="fractorium">Pointer to the main window.</param>
template <typename T>
FractoriumEmberController<T>::FractoriumEmberController(Fractorium* fractorium)
	: FractoriumEmberControllerBase(fractorium),
	  m_VariationList(VariationList<T>::Instance())
{
	size_t b = 0;
	m_GLController = make_unique<GLEmberController<T>>(fractorium, fractorium->ui.GLDisplay, this);
	m_LibraryPreviewRenderer = make_unique<TreePreviewRenderer<T>>(this, m_Fractorium->ui.LibraryTree, m_EmberFile, nullptr);
	m_SequencePreviewRenderer = make_unique<TreePreviewRenderer<T>>(this, m_Fractorium->ui.SequenceTree, m_SequenceFile, m_Fractorium->ui.SequenceProgressBar);
	m_PaletteList->Clear();
	m_Fractorium->ui.PaletteFilenameCombo->clear();
	//Initial combo change event to fill the palette table will be called automatically later.
	//Look hard for a palette.
	const auto paths = GetDefaultPaths();

	for (auto& path : paths)
		b |= InitPaletteList(path);

	if (b)
	{
		m_SheepTools = make_unique<SheepTools<T, float>>(m_PaletteList->Name(0), new EmberNs::Renderer<T, float>());
	}
	else
	{
		QString allPaths;

		for (auto& path : paths)
			allPaths += path + "\r\n";

		allPaths = QString("No palettes found in paths:\r\n") + allPaths + "\r\nExiting.";
		std::runtime_error ex(allPaths.toStdString());
		throw ex;
	}

	if (m_PaletteList->Size() >= 1)//Only add the user palette if the folder already had a palette, which means we'll be using this folder.
		if (m_PaletteList->AddEmptyPaletteFile((GetDefaultUserPath() + "/user-palettes.xml").toStdString()))
			m_Fractorium->ui.PaletteFilenameCombo->addItem("user-palettes.xml");

	BackgroundChanged(QColor(0, 0, 0));//Default to black.
	ClearUndo();
}

/// <summary>
/// Empty destructor that does nothing.
/// </summary>
template <typename T>
FractoriumEmberController<T>::~FractoriumEmberController() { }

/// <summary>
/// Setters for embers, ember files and palettes which convert between float and double types.
/// These are used to preserve the current ember/file when switching between renderers.
/// Note that some precision will be lost when going from double to float.
/// </summary>
template <typename T> void FractoriumEmberController<T>::SetEmber(const Ember<float>& ember, bool verbatim, bool updatePointer, int xformIndex) { SetEmberPrivate<float>(ember, verbatim, updatePointer, xformIndex); }
template <typename T> void FractoriumEmberController<T>::CopyEmber(Ember<float>& ember, std::function<void(Ember<float>& ember)> perEmberOperation) { ember = m_Ember; perEmberOperation(ember); }
template <typename T> void FractoriumEmberController<T>::SetEmberFile(const EmberFile<float>& emberFile, bool move) { move ? m_EmberFile = std::move(emberFile) : m_EmberFile = emberFile; }
template <typename T> void FractoriumEmberController<T>::CopyEmberFile(EmberFile<float>& emberFile, bool sequence, std::function<void(Ember<float>& ember)> perEmberOperation)
{
	if (sequence)
	{
		emberFile.m_Filename = m_SequenceFile.m_Filename;
		CopyCont(emberFile.m_Embers, m_SequenceFile.m_Embers, perEmberOperation);
	}
	else
	{
		emberFile.m_Filename = m_EmberFile.m_Filename;
		CopyCont(emberFile.m_Embers, m_EmberFile.m_Embers, perEmberOperation);
	}
}

template <typename T> void FractoriumEmberController<T>::CopyXaosToggleEmber(Ember<float>& ember) { ember = m_XaosToggleEmber; }
template <typename T> void FractoriumEmberController<T>::SetXaosToggleEmber(const Ember<float>& ember) { m_XaosToggleEmber = ember; }
template <typename T> void FractoriumEmberController<T>::SetTempPalette(const Palette<float>& palette) { m_TempPalette = palette; }
template <typename T> void FractoriumEmberController<T>::CopyTempPalette(Palette<float>& palette) { palette = m_TempPalette; }
#ifdef DO_DOUBLE
template <typename T> void FractoriumEmberController<T>::SetEmber(const Ember<double>& ember, bool verbatim, bool updatePointer, int xformIndex) { SetEmberPrivate<double>(ember, verbatim, updatePointer, xformIndex); }
template <typename T> void FractoriumEmberController<T>::CopyEmber(Ember<double>& ember, std::function<void(Ember<double>& ember)> perEmberOperation) { ember = m_Ember; perEmberOperation(ember); }
template <typename T> void FractoriumEmberController<T>::SetEmberFile(const EmberFile<double>& emberFile, bool move) { move ? m_EmberFile = std::move(emberFile) : m_EmberFile = emberFile; }
template <typename T> void FractoriumEmberController<T>::CopyEmberFile(EmberFile<double>& emberFile, bool sequence, std::function<void(Ember<double>& ember)> perEmberOperation)
{
	if (sequence)
	{
		emberFile.m_Filename = m_SequenceFile.m_Filename;
		CopyCont(emberFile.m_Embers, m_SequenceFile.m_Embers, perEmberOperation);
	}
	else
	{
		emberFile.m_Filename = m_EmberFile.m_Filename;
		CopyCont(emberFile.m_Embers, m_EmberFile.m_Embers, perEmberOperation);
	}
}

template <typename T> void FractoriumEmberController<T>::CopyXaosToggleEmber(Ember<double>& ember) { ember = m_XaosToggleEmber; }
template <typename T> void FractoriumEmberController<T>::SetXaosToggleEmber(const Ember<double>& ember) { m_XaosToggleEmber = ember; }
template <typename T> void FractoriumEmberController<T>::SetTempPalette(const Palette<double>& palette) { m_TempPalette = palette; }
template <typename T> void FractoriumEmberController<T>::CopyTempPalette(Palette<double>& palette) { palette = m_TempPalette; }
#endif
template <typename T> Ember<T>* FractoriumEmberController<T>::CurrentEmber() { return &m_Ember; }

template <typename T>
void FractoriumEmberController<T>::ConstrainDimensions(Ember<T>& ember)
{
	ember.m_FinalRasW = std::min<int>(m_Fractorium->ui.GLDisplay->MaxTexSize(), int(ember.m_FinalRasW));
	ember.m_FinalRasH = std::min<int>(m_Fractorium->ui.GLDisplay->MaxTexSize(), int(ember.m_FinalRasH));
}

/// <summary>
/// Set the ember at the specified index from the currently opened file as the current Ember.
/// Also attempt to keep the same xform inxed selected, which is useful when switching between many
/// similar embers when setting up animation keyframes.
/// Clears the undo state.
/// Resets the rendering process.
/// </summary>
/// <param name="index">The index in the file from which to retrieve the ember</param>
/// <param name="verbatim">If true, do not overwrite temporal samples, quality or supersample value, else overwrite.</param>
template <typename T>
void FractoriumEmberController<T>::SetEmber(size_t index, bool verbatim)
{
	if (index < m_EmberFile.Size())
	{
		auto currentXformIndex = m_Fractorium->ui.CurrentXformCombo->currentIndex();
		m_Fractorium->SelectLibraryItem(index);
		ClearUndo();
		SetEmber(*m_EmberFile.Get(index), verbatim, true, currentXformIndex);
	}
}

/// <summary>
/// Wrapper to call a function, then optionally add the requested action to the rendering queue.
/// </summary>
/// <param name="func">The function to call</param>
/// <param name="updateRender">True to update renderer, else false. Default: true.</param>
/// <param name="action">The action to add to the rendering queue. Default: eProcessAction::FULL_RENDER.</param>
template <typename T>
void FractoriumEmberController<T>::Update(std::function<void(void)> func, bool updateRender, eProcessAction action)
{
	func();

	if (updateRender)
		UpdateRender(action);
}

/// <summary>
/// Wrapper to call a function on the current ember and optionally all other embers in the file.
/// Then optionally add the requested action to the rendering queue.
/// </summary>
/// <param name="func">The function to call</param>
/// <param name="updateRender">True to update renderer, else false. Default: true.</param>
/// <param name="action">The action to add to the rendering queue. Default: eProcessAction::FULL_RENDER.</param>
/// <param name="applyAll">True to apply the action to all embers in the file in addition to the curent one, false to apply the action only to the current one.</param>
template <typename T>
void FractoriumEmberController<T>::UpdateAll(std::function<void(Ember<T>& ember, bool isMain)> func, bool updateRender, eProcessAction action, bool applyAll)
{
	func(m_Ember, true);

	if (applyAll)
		for (auto& it : m_EmberFile.m_Embers)
			func(it, false);

	if (updateRender)
		UpdateRender(action);
}

/// <summary>
/// Wrapper to call a function on the specified xforms, then optionally add the requested action to the rendering queue.
/// If no xforms are selected via the checkboxes, and the update type is UPDATE_SELECTED, then the function will be called only on the currently selected xform.
/// If the update type is UPDATE_CURRENT_AND_SELECTED, and the current is not among those selected, then the function will be called on the currently selected xform as well.
/// </summary>
/// <param name="func">The function to call which will pass the xform under consideration, the absolute xform index, and the index within the selected xforms</param>
/// <param name="updateType">Whether to apply this update operation on the current, all or selected xforms. Default: eXformUpdate::UPDATE_CURRENT.</param>
/// <param name="updateRender">True to update renderer, else false. Default: true.</param>
/// <param name="action">The action to add to the rendering queue. Default: eProcessAction::FULL_RENDER.</param>
/// <param name="index">The xform index to use when action is eXformUpdate::UPDATE_SPECIFIC. Default: 0.</param>
template <typename T>
void FractoriumEmberController<T>::UpdateXform(std::function<void(Xform<T>*, size_t, size_t)> func, eXformUpdate updateType, bool updateRender, eProcessAction action, size_t index)
{
	int i = 0;
	size_t selIndex = 0;
	const auto current = CurrentXform();
	const auto currentIndex = m_Fractorium->ui.CurrentXformCombo->currentIndex();
	const bool forceFinal = m_Fractorium->HaveFinal();
	const bool isCurrentFinal = m_Ember.IsFinalXform(current);
	const bool doFinal = updateType != eXformUpdate::UPDATE_SELECTED_EXCEPT_FINAL && updateType != eXformUpdate::UPDATE_ALL_EXCEPT_FINAL;

	switch (updateType)
	{
		case eXformUpdate::UPDATE_SPECIFIC:
		{
			if (auto xform = m_Ember.GetTotalXform(index, forceFinal))
				func(xform, index, 0);
		}
		break;

		case eXformUpdate::UPDATE_CURRENT:
		{
			if (current)
				func(current, currentIndex, 0);
		}
		break;

		case eXformUpdate::UPDATE_CURRENT_AND_SELECTED:
		{
			bool currentDone = false;

			while (const auto xform = m_Ember.GetTotalXform(i, forceFinal))
			{
				if (m_Fractorium->IsXformSelected(i))
				{
					func(xform, i, selIndex++);

					if (xform == current)
						currentDone = true;
				}

				i++;
			}

			if (!currentDone)//Current was not among those selected, so apply to it.
				func(current, currentIndex, selIndex);
		}
		break;

		case eXformUpdate::UPDATE_SELECTED:
		case eXformUpdate::UPDATE_SELECTED_EXCEPT_FINAL:
		{
			bool anyUpdated = false;

			while (const auto xform = (doFinal ? m_Ember.GetTotalXform(i, forceFinal) : m_Ember.GetXform(i)))
			{
				if (m_Fractorium->IsXformSelected(i))
				{
					func(xform, i, selIndex++);
					anyUpdated = true;
				}

				i++;
			}

			if (!anyUpdated)//None were selected, so just apply to the current.
				if (doFinal || !isCurrentFinal)//If do final, call func regardless. If not, only call if current is not final.
					if (current)
						func(current, currentIndex, selIndex);
		}
		break;

		case eXformUpdate::UPDATE_ALL:
		{
			while (const auto xform = m_Ember.GetTotalXform(i, forceFinal))
				func(xform, i++, selIndex++);
		}
		break;

		case eXformUpdate::UPDATE_ALL_EXCEPT_FINAL:
		default:
		{
			while (const auto xform = m_Ember.GetXform(i))
				func(xform, i++, selIndex++);
		}
		break;
	}

	if (updateRender)
		UpdateRender(action);
}

/// <summary>
/// Set the current ember, but use GUI values for the fields which make sense to
/// keep the same between ember selection changes.
/// Note the extra template parameter U allows for assigning ember of different types.
/// Resets the rendering process.
/// </summary>
/// <param name="ember">The ember to set as the current</param>
/// <param name="verbatim">If true, do not overwrite temporal samples, quality or supersample value, else overwrite.</param>
/// <param name="updatePointer">If true, update the current ember pointer to the address of the one passed in.</param>
/// <param name="xformIndex">The xform index to select as the current one</param>
template <typename T>
template <typename U>
void FractoriumEmberController<T>::SetEmberPrivate(const Ember<U>& ember, bool verbatim, bool updatePointer, int xformIndex)
{
	if (ember.m_Name != m_Ember.m_Name)
		m_LastSaveCurrent = "";

	const auto w = m_Ember.m_FinalRasW;//Cache values for use below.
	const auto h = m_Ember.m_FinalRasH;
	m_Ember = ember;
	m_XaosToggleEmber.Clear();

	if (updatePointer && (typeid(T) == typeid(U)))
        m_EmberFilePointer = reinterpret_cast<Ember<T>*>(const_cast<Ember<U>*>(&ember));

	if (!verbatim)
	{
		m_Ember.m_TemporalSamples = 1;//Change once animation is supported.
		m_Ember.m_Quality = m_Fractorium->m_QualitySpin->value();
		m_Ember.m_Supersample = m_Fractorium->m_SupersampleSpin->value();
	}

	static EmberToXml<T> writer;//Save parameters of last full render just in case there is a crash.
	const auto path = GetDefaultUserPath();
	const QDir dir(path);

	if (!dir.exists())
		dir.mkpath(".");

	const auto filename = path.toStdString() + "/last.flame";
	writer.Save(filename, m_Ember, 0, true, true, false, true, true);
	m_GLController->ResetMouseState();
	FillXforms(xformIndex);//Must do this first because the palette setup in FillParamTablesAndPalette() uses the xforms combo.
	FillParamTablesAndPalette();
	FillCurvesControl();
	FillSummary();

	//If a resize happened, this won't do anything because the new size is not reflected in the scroll area yet.
	//However, it will have been taken care of in SyncSizes() in that case, so it's ok.
	//This is for when a new ember with the same size was loaded. If it was larger than the scroll area, and was scrolled, re-center it.
	if (m_Ember.m_FinalRasW == w && m_Ember.m_FinalRasH == h)
		m_Fractorium->CenterScrollbars();
}

/// <summary>
/// Thin derivation to handle preview rendering multiple embers previews to a tree.
/// </summary>
/// <param name="start">The 0-based index to start rendering previews for</param>
/// <param name="end">The 0-based index which is one beyond the last ember to render a preview for</param>
template <typename T>
void TreePreviewRenderer<T>::PreviewRenderFunc(uint start, uint end)
{
	const auto f = m_Controller->m_Fractorium;
	m_PreviewRenderer.EarlyClip(f->m_Settings->EarlyClip());
	m_PreviewRenderer.YAxisUp(f->m_Settings->YAxisUp());
	m_PreviewRenderer.ThreadCount(std::max(1u, Timing::ProcessorCount() - 1));//Leave one processor free so the GUI can breathe.

	// Possible animate item at index 0
	if (const auto top = m_Tree->topLevelItem(m_Tree->topLevelItemCount() - 1))
	{
		size_t i = start;
		double prc = 0;
		double ct = 0;
		double total = end - start;

		if (m_Pb)
			QMetaObject::invokeMethod(m_Pb, "setValue", Qt::ConnectionType::QueuedConnection, Q_ARG(const int, 0));

		for (auto b = Advance(m_EmberFile.m_Embers.begin(), start); m_PreviewRun && i < end && b != m_EmberFile.m_Embers.end(); ++b, ++i)
		{
			m_PreviewEmber = *b;
			m_PreviewEmber.SyncSize();
			m_PreviewEmber.SetSizeAndAdjustScale(PREVIEW_SIZE, PREVIEW_SIZE, false, eScaleType::SCALE_WIDTH);
			m_PreviewEmber.m_TemporalSamples = 1;
			m_PreviewEmber.m_Quality = 25;
			m_PreviewEmber.m_Supersample = 1;
			m_PreviewRenderer.SetEmber(m_PreviewEmber, eProcessAction::FULL_RENDER, true);

			if (m_PreviewRenderer.Run(m_PreviewFinalImage) == eRenderStatus::RENDER_OK)
			{
				if (const auto treeItem = dynamic_cast<EmberTreeWidgetItemBase*>(top->child(int(i))))
				{
					//It is critical that Qt::DirectConnection is passed because this is running on a different thread than the UI.
					//This ensures the events are processed in order as each preview is updated, and that control does not return here
					//until the update is complete.
					if (m_PreviewRun)
					{
						QMetaObject::invokeMethod(f, "SetTreeItemData", Qt::ConnectionType::DirectConnection,
												  Q_ARG(EmberTreeWidgetItemBase*, treeItem),
												  Q_ARG(vv4F&, m_PreviewFinalImage),
												  Q_ARG(uint, PREVIEW_SIZE),
												  Q_ARG(uint, PREVIEW_SIZE));
						prc = 100.0 * (++ct / total);

						if (m_Pb)
							QMetaObject::invokeMethod(m_Pb, "setValue", Qt::ConnectionType::QueuedConnection, Q_ARG(const int, int(prc)));

						treeItem->SetRendered();
					}
				}
			}
		}

		if (m_Pb)
			QMetaObject::invokeMethod(m_Pb, "setValue", Qt::ConnectionType::QueuedConnection, Q_ARG(const int, 100));
	}
}

template class FractoriumEmberController<float>;
template class PreviewRenderer<float>;
template class TreePreviewRenderer<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
	template class PreviewRenderer<double>;
	template class TreePreviewRenderer<double>;
#endif
