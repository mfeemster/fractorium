#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Initialize the menus UI.
/// </summary>
void Fractorium::InitMenusUI()
{
	//File menu.
	connect(ui.ActionNewFlock,					  SIGNAL(triggered(bool)), this, SLOT(OnActionNewFlock(bool)),					  Qt::QueuedConnection);
	connect(ui.ActionNewEmptyFlameInCurrentFile,  SIGNAL(triggered(bool)), this, SLOT(OnActionNewEmptyFlameInCurrentFile(bool)),  Qt::QueuedConnection);
	connect(ui.ActionNewRandomFlameInCurrentFile, SIGNAL(triggered(bool)), this, SLOT(OnActionNewRandomFlameInCurrentFile(bool)), Qt::QueuedConnection);
	connect(ui.ActionCopyFlameInCurrentFile,	  SIGNAL(triggered(bool)), this, SLOT(OnActionCopyFlameInCurrentFile(bool)),	  Qt::QueuedConnection);
	connect(ui.ActionCreateReferenceFile,         SIGNAL(triggered(bool)), this, SLOT(OnActionCreateReferenceFile(bool)),         Qt::QueuedConnection);
	connect(ui.ActionOpen,						  SIGNAL(triggered(bool)), this, SLOT(OnActionOpen(bool)),						  Qt::QueuedConnection);
	connect(ui.ActionOpenExamples,                SIGNAL(triggered(bool)), this, SLOT(OnActionOpenExamples(bool)),				  Qt::QueuedConnection);
	connect(ui.ActionSaveCurrentAsXml,			  SIGNAL(triggered(bool)), this, SLOT(OnActionSaveCurrentAsXml(bool)),			  Qt::QueuedConnection);
	connect(ui.ActionSaveEntireFileAsXml,		  SIGNAL(triggered(bool)), this, SLOT(OnActionSaveEntireFileAsXml(bool)),		  Qt::QueuedConnection);
	connect(ui.ActionSaveCurrentScreen,			  SIGNAL(triggered(bool)), this, SLOT(OnActionSaveCurrentScreen(bool)),			  Qt::QueuedConnection);
	connect(ui.ActionExit,						  SIGNAL(triggered(bool)), this, SLOT(OnActionExit(bool)),						  Qt::QueuedConnection);
	//Edit menu.
	connect(ui.ActionUndo,				  SIGNAL(triggered(bool)), this, SLOT(OnActionUndo(bool)),				  Qt::QueuedConnection);
	connect(ui.ActionRedo,				  SIGNAL(triggered(bool)), this, SLOT(OnActionRedo(bool)),				  Qt::QueuedConnection);
	connect(ui.ActionCopyXml,			  SIGNAL(triggered(bool)), this, SLOT(OnActionCopyXml(bool)),			  Qt::QueuedConnection);
	connect(ui.ActionCopyAllXml,		  SIGNAL(triggered(bool)), this, SLOT(OnActionCopyAllXml(bool)),		  Qt::QueuedConnection);
	connect(ui.ActionPasteXmlAppend,	  SIGNAL(triggered(bool)), this, SLOT(OnActionPasteXmlAppend(bool)),	  Qt::QueuedConnection);
	connect(ui.ActionPasteXmlOver,		  SIGNAL(triggered(bool)), this, SLOT(OnActionPasteXmlOver(bool)),		  Qt::QueuedConnection);
	connect(ui.ActionCopySelectedXforms,  SIGNAL(triggered(bool)), this, SLOT(OnActionCopySelectedXforms(bool)),  Qt::QueuedConnection);
	connect(ui.ActionPasteSelectedXforms, SIGNAL(triggered(bool)), this, SLOT(OnActionPasteSelectedXforms(bool)), Qt::QueuedConnection);
	connect(ui.ActionCopyKernel,          SIGNAL(triggered(bool)), this, SLOT(OnActionCopyKernel(bool)),          Qt::QueuedConnection);
	ui.ActionPasteSelectedXforms->setEnabled(false);
	//View menu.
	connect(ui.ActionResetWorkspace,       SIGNAL(triggered(bool)), this, SLOT(OnActionResetWorkspace(bool)),       Qt::QueuedConnection);
	connect(ui.ActionAlternateEditorImage, SIGNAL(triggered(bool)), this, SLOT(OnActionAlternateEditorImage(bool)), Qt::QueuedConnection);
	connect(ui.ActionResetScale,           SIGNAL(triggered(bool)), this, SLOT(OnActionResetScale(bool)),           Qt::QueuedConnection);
	//Tools menu.
	connect(ui.ActionAddReflectiveSymmetry, SIGNAL(triggered(bool)), this, SLOT(OnActionAddReflectiveSymmetry(bool)), Qt::QueuedConnection);
	connect(ui.ActionAddRotationalSymmetry, SIGNAL(triggered(bool)), this, SLOT(OnActionAddRotationalSymmetry(bool)), Qt::QueuedConnection);
	connect(ui.ActionAddBothSymmetry,		SIGNAL(triggered(bool)), this, SLOT(OnActionAddBothSymmetry(bool)),		  Qt::QueuedConnection);
	connect(ui.ActionClearFlame,			SIGNAL(triggered(bool)), this, SLOT(OnActionClearFlame(bool)),			  Qt::QueuedConnection);
	connect(ui.ActionFlatten,			    SIGNAL(triggered(bool)), this, SLOT(OnActionFlatten(bool)),			      Qt::QueuedConnection);
	connect(ui.ActionUnflatten,			    SIGNAL(triggered(bool)), this, SLOT(OnActionUnflatten(bool)),			  Qt::QueuedConnection);
	connect(ui.ActionStopRenderingPreviews,	SIGNAL(triggered(bool)), this, SLOT(OnActionStopRenderingPreviews(bool)), Qt::QueuedConnection);
	connect(ui.ActionRenderPreviews,		SIGNAL(triggered(bool)), this, SLOT(OnActionRenderPreviews(bool)),		  Qt::QueuedConnection);
	connect(ui.ActionFinalRender,			SIGNAL(triggered(bool)), this, SLOT(OnActionFinalRender(bool)),			  Qt::QueuedConnection);
	connect(ui.ActionOptions,				SIGNAL(triggered(bool)), this, SLOT(OnActionOptions(bool)),				  Qt::QueuedConnection);
	//Help menu.
	connect(ui.ActionAbout, SIGNAL(triggered(bool)), this, SLOT(OnActionAbout(bool)), Qt::QueuedConnection);
}

/// <summary>
/// Create a new flock of random embers, with the specified length.
/// </summary>
/// <param name="count">The number of embers to include in the flock</param>
template <typename T>
void FractoriumEmberController<T>::NewFlock(size_t count)
{
	bool nv = false;
	Ember<T> ember;
	StopAllPreviewRenderers();
	m_EmberFile.Clear();
	m_EmberFile.m_Filename = EmberFile<T>::DefaultFilename();
	vector<eVariationId> filteredVariations;
	vector<eVariationId>* filteredVariationsRef = &m_FilteredVariations;
	auto& deviceNames = OpenCLInfo::Instance()->AllDeviceNames();

	for (auto& dev : deviceNames)
		if (Find(ToLower(dev), "nvidia"))
			nv = true;

	if (nv)//Nvidia cannot handle synth. It takes over a minute to compile and uses about 4GB of memory.
	{
		filteredVariations = m_FilteredVariations;
		filteredVariations.erase(std::remove_if(filteredVariations.begin(), filteredVariations.end(),
												[&](const eVariationId & id) -> bool
		{
			return id == eVariationId::VAR_SYNTH || id == eVariationId::VAR_PRE_SYNTH || id == eVariationId::VAR_POST_SYNTH;
		}
											   ), filteredVariations.end());
		filteredVariationsRef = &filteredVariations;
	}

	for (size_t i = 0; i < count; i++)
	{
		m_SheepTools->Random(ember, *filteredVariationsRef, static_cast<intmax_t>(QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedFrand<T>(-2, 2)), 0, 8);
		ParamsToEmber(ember);
		ember.m_Index = i;
		ember.m_Name = CleanPath(m_EmberFile.m_Filename.toStdString() + "_" + ToString(i + 1ULL).toStdString());
		m_EmberFile.m_Embers.push_back(ember);
	}

	m_LastSaveAll = "";
	FillLibraryTree();
}

/// <summary>
/// Create a new flock and assign the first ember as the current one.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionNewFlock(bool checked)
{
	m_Controller->NewFlock(m_Settings->RandomCount());
	m_Controller->SetEmber(0, false);
}

/// <summary>
/// Create and add a new empty ember in the currently opened file
/// and set it as the current one.
/// It will have one empty xform in it.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::NewEmptyFlameInCurrentFile()
{
	Ember<T> ember;
	Xform<T> xform;
	QDateTime local(QDateTime::currentDateTime());
	StopAllPreviewRenderers();
	ParamsToEmber(ember);
	xform.m_Weight = T(0.25);
	xform.m_ColorX = m_Rand.Frand01<T>();
	xform.AddVariation(m_VariationList->GetVariationCopy(eVariationId::VAR_LINEAR));
	ember.AddXform(xform);
	ember.m_Palette = *m_PaletteList->GetRandomPalette();
	ember.m_Name = EmberFile<T>::DefaultEmberName(m_EmberFile.Size() + 1).toStdString();
	ember.m_Index = m_EmberFile.Size();
	m_EmberFile.m_Embers.push_back(ember);//Will invalidate the pointers contained in the EmberTreeWidgetItems, UpdateLibraryTree() will resync.
	m_EmberFile.MakeNamesUnique();
	UpdateLibraryTree();
	SetEmber(m_EmberFile.Size() - 1, false);
}

void Fractorium::OnActionNewEmptyFlameInCurrentFile(bool checked) { m_Controller->NewEmptyFlameInCurrentFile(); }

/// <summary>
/// Create and add a new random ember in the currently opened file
/// and set it as the current one.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::NewRandomFlameInCurrentFile()
{
	Ember<T> ember;
	StopAllPreviewRenderers();
	m_SheepTools->Random(ember, m_FilteredVariations, static_cast<int>(QTIsaac<ISAAC_SIZE, ISAAC_INT>::LockedFrand<T>(-2, 2)), 0, 8);
	ParamsToEmber(ember);
	ember.m_Name = EmberFile<T>::DefaultEmberName(m_EmberFile.Size() + 1).toStdString();
	ember.m_Index = m_EmberFile.Size();
	m_EmberFile.m_Embers.push_back(ember);//Will invalidate the pointers contained in the EmberTreeWidgetItems, UpdateLibraryTree() will resync.
	m_EmberFile.MakeNamesUnique();
	UpdateLibraryTree();
	SetEmber(m_EmberFile.Size() - 1, false);
}

void Fractorium::OnActionNewRandomFlameInCurrentFile(bool checked) { m_Controller->NewRandomFlameInCurrentFile(); }

/// <summary>
/// Create and add a a copy of the current ember in the currently opened file
/// and set it as the current one.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::CopyFlameInCurrentFile()
{
	StopAllPreviewRenderers();
	auto ember = m_Ember;
	auto scale = m_LockedScale;
	auto model = m_Fractorium->ui.LibraryTree->selectionModel();
	auto sel = model->selectedIndexes();
	auto insertEmberIndex = sel.size() > 0 ? sel[0].row() + 1 : 0;
	ember.m_Name = EmberFile<T>::DefaultEmberName(m_EmberFile.Size() + 1).toStdString();
	ember.m_Index = insertEmberIndex;//Will be overwritten below in FillLibraryTree().
	m_EmberFile.m_Embers.insert(Advance(m_EmberFile.m_Embers.begin(), insertEmberIndex), ember);//Will invalidate the pointers contained in the EmberTreeWidgetItems, FillLibraryTree() has the effect of resyncing because index, name and pointer are assigned.
	m_EmberFile.MakeNamesUnique();
	FillLibraryTree(insertEmberIndex);
	SetEmber(insertEmberIndex, false);
	LockedScale(scale);//Need to call this to keep affine scales as they were because SetEmber() resets them.
}

void Fractorium::OnActionCopyFlameInCurrentFile(bool checked) { m_Controller->CopyFlameInCurrentFile(); }

/// <summary>
/// Create a reference file containing one ember for every possible regular variation, plus one for post_smartcrop
/// since it only exists in post form.
/// This will replace whatever file the user has open.
/// Clears the undo state.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::CreateReferenceFile()
{
	StopAllPreviewRenderers();
	auto temppal = m_Ember.m_Palette;
	m_EmberFile.Clear();
	m_EmberFile.m_Filename = QString("Reference_") + EMBER_VERSION;
	auto varList = VariationList<T>::Instance();
	auto& regVars = varList->RegVars();
	auto count = regVars.size();
	auto addsquaresfunc = [&](size_t i, const Variation<T>* var)
	{
		Ember<T> ember;
		Xform<T> xf0, xf1, xf2, xf3, xf4, xffinal;
		//
		xf0.AddVariation(varList->GetVariationCopy(eVariationId::VAR_SQUARE, T(0.5)));
		//
		xf1.m_Affine.C(T(0.5));
		xf1.m_Affine.F(T(0.5));
		xf1.AddVariation(varList->GetVariationCopy(eVariationId::VAR_LINEAR));
		//
		xf2.m_Affine.C(T(-0.5));
		xf2.m_Affine.F(T(0.5));
		xf2.AddVariation(varList->GetVariationCopy(eVariationId::VAR_LINEAR));
		//
		xf3.m_Affine.C(T(-0.5));
		xf3.m_Affine.F(T(-0.5));
		xf3.AddVariation(varList->GetVariationCopy(eVariationId::VAR_LINEAR));
		//
		xf4.m_Affine.C(T(0.5));
		xf4.m_Affine.F(T(-0.5));
		xf4.AddVariation(varList->GetVariationCopy(eVariationId::VAR_LINEAR));
		//
		xffinal.AddVariation(var->Copy());
		//
		ember.AddXform(xf0);
		ember.AddXform(xf1);
		ember.AddXform(xf2);
		ember.AddXform(xf3);
		ember.AddXform(xf4);
		ember.SetFinalXform(xffinal);
		ember.EqualizeWeights();
		ember.m_Index = i;
		ember.m_MaxRadDE = 0;
		ember.m_Name = var->Name() + " Squares";
		ember.m_Palette = temppal;
		m_EmberFile.m_Embers.push_back(ember);
	};
	auto addsbarsfunc = [&](size_t i, const Variation<T>* var)
	{
		Ember<T> ember;
		Xform<T> xf0, xf1, xf2, xffinal;
		//
		xf0.AddVariation(varList->GetVariationCopy(eVariationId::VAR_PRE_BLUR, T(100)));
		xf0.AddVariation(varList->GetVariationCopy(eVariationId::VAR_CYLINDER, T(0.1)));
		//
		xf1.m_Affine.C(T(0.4));
		xf1.AddVariation(varList->GetVariationCopy(eVariationId::VAR_LINEAR));
		//
		xf2.m_Affine.C(T(-0.4));
		xf2.AddVariation(varList->GetVariationCopy(eVariationId::VAR_LINEAR));
		//
		xffinal.AddVariation(var->Copy());
		ember.AddXform(xf0);
		ember.AddXform(xf1);
		ember.AddXform(xf2);
		ember.SetFinalXform(xffinal);
		ember.EqualizeWeights();
		ember.m_Index = i;
		ember.m_MaxRadDE = 0;
		ember.m_Name = var->Name() + " Bars";
		ember.m_Palette = temppal;
		m_EmberFile.m_Embers.push_back(ember);
	};
	size_t i;

	for (i = 0; i < count; i++)
	{
		addsquaresfunc(i, regVars[i]);
		addsbarsfunc(i, regVars[i]);
	}

	addsquaresfunc(i, varList->GetVariation(eVariationId::VAR_POST_SMARTCROP));//post_smartcrop is the only variation that exists only in post form, so it must be done manually here.
	addsbarsfunc(i, varList->GetVariation(eVariationId::VAR_POST_SMARTCROP));
	m_LastSaveAll = "";
	FillLibraryTree();
}

void Fractorium::OnActionCreateReferenceFile(bool checked) { m_Controller->CreateReferenceFile(); }

/// <summary>
/// Open a list of ember Xml files, apply various values from the GUI widgets.
/// Either append these newly read embers to the existing open embers,
/// or clear the current ember file first.
/// When appending, add the new embers the the end of library tree.
/// When not appending, clear and populate the library tree with the new embers.
/// Set the current ember to the first one in the newly opened list.
/// Clears the undo state.
/// Resets the rendering process.
/// </summary>
/// <param name="filenames">A list of full paths and filenames</param>
/// <param name="append">True to append the embers in the new files to the end of the currently open embers, false to clear and replace them</param>
template <typename T>
void FractoriumEmberController<T>::OpenAndPrepFiles(const QStringList& filenames, bool append)
{
	if (!filenames.empty())
	{
		StopAllPreviewRenderers();
		size_t i;
		EmberFile<T> emberFile;
		XmlToEmber<T> parser;
		vector<Ember<T>> embers;
		vector<string> errors;
		uint previousSize = append ? uint(m_EmberFile.Size()) : 0u;
		emberFile.m_Filename = filenames[0];

		for (auto& filename : filenames)
		{
			embers.clear();

			if (parser.Parse(filename.toStdString().c_str(), embers, true) && !embers.empty())
			{
				for (i = 0; i < embers.size(); i++)
				{
					ConstrainDimensions(embers[i]);//Do not exceed the max texture size.

					//Also ensure it has a name.
					if (embers[i].m_Name == "" || embers[i].m_Name == "No name")
						embers[i].m_Name = ToString<qulonglong>(i).toStdString();
					else
						embers[i].m_Name = CleanPath(embers[i].m_Name);

					embers[i].m_Quality = m_Fractorium->m_QualitySpin->value();
					embers[i].m_Supersample = m_Fractorium->m_SupersampleSpin->value();
				}

				m_LastSaveAll = "";
				emberFile.m_Embers.insert(emberFile.m_Embers.end(), embers.begin(), embers.end());
				errors = parser.ErrorReport();
			}
			else
			{
				errors = parser.ErrorReport();
				m_Fractorium->ShowCritical("Open Failed", "Could not open file, see info tab for details.");
			}

			if (!errors.empty())
				m_Fractorium->ErrorReportToQTextEdit(errors, m_Fractorium->ui.InfoFileOpeningTextEdit, false);//Concat errors from all files.
		}

		if (append)
		{
			if (m_EmberFile.m_Filename == "")
				m_EmberFile.m_Filename = filenames[0];

			m_EmberFile.m_Embers.insert(m_EmberFile.m_Embers.end(), emberFile.m_Embers.begin(), emberFile.m_Embers.end());
		}
		else if (emberFile.Size() > 0)//Ensure at least something was read.
			m_EmberFile = std::move(emberFile);//Move the temp to avoid creating dupes because we no longer need it.
		else if (!m_EmberFile.Size())
		{
			//Loading failed, so fill it with a dummy.
			Ember<T> ember;
			m_EmberFile.m_Embers.push_back(ember);
		}

		//Resync indices and names.
		i = 0;

		for (auto& it : m_EmberFile.m_Embers)
			it.m_Index = i++;

		m_EmberFile.MakeNamesUnique();

		if (append)
			UpdateLibraryTree();
		else
			FillLibraryTree(append ? previousSize - 1 : 0);

		ClearUndo();
		m_GLController->ClearControl();
		SetEmber(previousSize, false);
	}
}

/// <summary>
/// Show a file open dialog to open ember Xml files.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionOpen(bool checked) { m_Controller->OpenAndPrepFiles(SetupOpenXmlDialog(), false); }

/// <summary>
/// Show a file open dialog to open examples Xml files.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionOpenExamples(bool checked) { m_Controller->OpenAndPrepFiles(SetupOpenXmlDialog(true), false); }

/// <summary>
/// Save current ember as Xml, using the Xml saving template values from the options.
/// This will first save the current ember back to the opened file in memory before
/// saving it to disk.
/// </summary>
/// <param name="filename">The filename to save the ember to. If empty, use internal variables to determine the filename.</param>
template <typename T>
void FractoriumEmberController<T>::SaveCurrentAsXml(QString filename)
{
	auto s = m_Fractorium->m_Settings;

	if (filename == "")
	{
		if (s->SaveAutoUnique() && m_LastSaveCurrent != "")
		{
			filename = EmberFile<T>::UniqueFilename(m_LastSaveCurrent);
		}
		else if (QFile::exists(m_LastSaveCurrent))
		{
			filename = m_LastSaveCurrent;
		}
		else
		{
			if (m_EmberFile.Size() == 1)
				filename = m_Fractorium->SetupSaveXmlDialog(m_EmberFile.m_Filename);//If only one ember present, just use parent filename.
			else
				filename = m_Fractorium->SetupSaveXmlDialog(QString::fromStdString(m_Ember.m_Name));//More than one ember present, use individual ember name.
		}
	}

	if (filename != "")
	{
		auto ember = m_Ember;
		EmberToXml<T> writer;
		QFileInfo fileInfo(filename);
		auto tempEdit = ember.m_Edits;
		SaveCurrentToOpenedFile();//Save the current ember back to the opened file before writing to disk.
		ApplyXmlSavingTemplate(ember);
		ember.m_Edits = writer.CreateNewEditdoc(&ember, nullptr, "edit", s->Nick().toStdString(), s->Url().toStdString(), s->Id().toStdString(), "", 0, 0);

		if (tempEdit)
			xmlFreeDoc(tempEdit);

		if (writer.Save(filename.toStdString().c_str(), ember, 0, true, true))
		{
			s->SaveFolder(fileInfo.canonicalPath());

			if (!s->SaveAutoUnique() || m_LastSaveCurrent == "")//Only save filename on first time through when doing auto unique names.
				m_LastSaveCurrent = filename;
		}
		else
			m_Fractorium->ShowCritical("Save Failed", "Could not save file, try saving to a different folder.");
	}

	m_GLController->ClearControl();
}

void Fractorium::OnActionSaveCurrentAsXml(bool checked) { m_Controller->SaveCurrentAsXml(); }

/// <summary>
/// Save entire opened file Xml, using the Xml saving template values from the options on each ember.
/// This will first save the current ember back to the opened file in memory before
/// saving all to disk.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::SaveEntireFileAsXml()
{
	QString filename;
	auto s = m_Fractorium->m_Settings;

	if (s->SaveAutoUnique() && m_LastSaveAll != "")
		filename = EmberFile<T>::UniqueFilename(m_LastSaveAll);
	else if (QFile::exists(m_LastSaveAll))
		filename = m_LastSaveAll;
	else
		filename = m_Fractorium->SetupSaveXmlDialog(m_EmberFile.m_Filename);

	if (filename != "")
	{
		EmberFile<T> emberFile;
		EmberToXml<T> writer;
		QFileInfo fileInfo(filename);
		SaveCurrentToOpenedFile();//Save the current ember back to the opened file before writing to disk.
		emberFile = m_EmberFile;

		for (auto& ember : emberFile.m_Embers)
			ApplyXmlSavingTemplate(ember);

		if (writer.Save(filename.toStdString().c_str(), emberFile.m_Embers, 0, true, true, false, false, false))
		{
			if (!s->SaveAutoUnique() || m_LastSaveAll == "")//Only save filename on first time through when doing auto unique names.
				m_LastSaveAll = filename;

			s->SaveFolder(fileInfo.canonicalPath());
		}
		else
			m_Fractorium->ShowCritical("Save Failed", "Could not save file, try saving to a different folder.");
	}

	m_GLController->ClearControl();
}

void Fractorium::OnActionSaveEntireFileAsXml(bool checked) { m_Controller->SaveEntireFileAsXml(); }

template <typename T>
void FractoriumEmberController<T>::SaveCurrentFileOnShutdown()
{
	EmberToXml<T> writer;
	auto path = GetDefaultUserPath();
	QDir dir(path);

	if (!dir.exists())
		dir.mkpath(".");

	string filename = path.toStdString() + "/lastonshutdown.flame";
	writer.Save(filename, m_EmberFile.m_Embers, 0, true, true, false, false, false);
}

/// <summary>
/// Show a file save dialog and save what is currently shown in the render window to disk as an image.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionSaveCurrentScreen(bool checked)
{
	auto filename = SetupSaveImageDialog(m_Controller->Name());
	auto renderer = m_Controller->Renderer();
	auto& pixels = *m_Controller->FinalImage();
	auto rendererCL = dynamic_cast<RendererCLBase*>(renderer);
	auto stats = m_Controller->Stats();
	auto comments = renderer->ImageComments(stats, 0, true);
	auto settings = FractoriumSettings::Instance();

	if (rendererCL && renderer->PrepFinalAccumVector(pixels))
	{
		if (!rendererCL->ReadFinal(pixels.data()))
		{
			ShowCritical("GPU Read Error", "Could not read image from the GPU, aborting image save.", false);
			return;
		}
	}

	m_Controller->SaveCurrentRender(filename, comments, pixels, renderer->FinalRasW(), renderer->FinalRasH(), settings->Png16Bit(), settings->Transparency());
}

/// <summary>
/// Save the current ember back to its position in the opened file.
/// This will not take any action if the previews are still rendering because
/// this writes to memory the preview renderer might be reading, and also stops the
/// preview renderer.
/// This does not save to disk.
/// </summary>
/// <param name="render">Whether to re-render the preview thumbnail after saving to the open file. Default: true.</param>
template <typename T>
uint FractoriumEmberController<T>::SaveCurrentToOpenedFile(bool render)
{
	uint i = 0;
	bool fileFound = false;

	if (!m_LibraryPreviewRenderer->Running())
	{
		for (auto& it : m_EmberFile.m_Embers)
		{
			if (&it == m_EmberFilePointer)//Just compare memory addresses.
			{
				it = m_Ember;//Save it to the opened file in memory.
				fileFound = true;
				break;
			}

			i++;
		}

		if (!fileFound)
		{
			StopAllPreviewRenderers();
			m_EmberFile.m_Embers.push_back(m_Ember);
			m_EmberFile.MakeNamesUnique();

			if (render)
				UpdateLibraryTree();
		}
		else if (render)
			RenderLibraryPreviews(i, i + 1);
	}

	return i;
}

/// <summary>
/// Exit the application.
/// </summary>
/// <param name="checked">Ignore.</param>
void Fractorium::OnActionExit(bool checked)
{
	closeEvent(nullptr);
	QApplication::exit();
}

/// <summary>
/// Undoes this instance.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::Undo()
{
	if (m_UndoList.size() > 1 && m_UndoIndex > 0)
	{
		bool forceFinal = m_Fractorium->HaveFinal();
		auto current = CurrentXform();
		int index = m_Ember.GetTotalXformIndex(current, forceFinal);
		m_LastEditWasUndoRedo = true;
		m_UndoIndex = std::max<size_t>(0u, m_UndoIndex - 1u);
		SetEmber(m_UndoList[m_UndoIndex], true, false, index);//Don't update pointer because it's coming from the undo list.
		m_EditState = eEditUndoState::UNDO_REDO;

		if (index >= 0 &&  index < m_Fractorium->ui.CurrentXformCombo->count())
			m_Fractorium->CurrentXform(index);

		m_Fractorium->ui.ActionUndo->setEnabled(m_UndoList.size() > 1 && (m_UndoIndex > 0));
		m_Fractorium->ui.ActionRedo->setEnabled(m_UndoList.size() > 1 && !(m_UndoIndex == m_UndoList.size() - 1));
	}
}

void Fractorium::OnActionUndo(bool checked) { m_Controller->Undo(); }

/// <summary>
/// Redoes this instance.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::Redo()
{
	if (m_UndoList.size() > 1 && m_UndoIndex < m_UndoList.size() - 1)
	{
		bool forceFinal = m_Fractorium->HaveFinal();
		auto current = CurrentXform();
		int index = m_Ember.GetTotalXformIndex(current, forceFinal);
		m_LastEditWasUndoRedo = true;
		m_UndoIndex = std::min<size_t>(m_UndoIndex + 1, m_UndoList.size() - 1);
		SetEmber(m_UndoList[m_UndoIndex], true, false, index);//Don't update pointer because it's coming from the undo list.
		m_EditState = eEditUndoState::UNDO_REDO;

		if (index >= 0 && index < m_Fractorium->ui.CurrentXformCombo->count())
			m_Fractorium->CurrentXform(index);

		m_Fractorium->ui.ActionUndo->setEnabled(m_UndoList.size() > 1 && (m_UndoIndex > 0));
		m_Fractorium->ui.ActionRedo->setEnabled(m_UndoList.size() > 1 && !(m_UndoIndex == m_UndoList.size() - 1));
	}
}

void Fractorium::OnActionRedo(bool checked) { m_Controller->Redo(); }

/// <summary>
/// Copy the current ember Xml to the clipboard.
/// Apply Xml saving settings from the options first.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::CopyXml()
{
	auto ember = m_Ember;
	EmberToXml<T> emberToXml;
	auto settings = m_Fractorium->m_Settings;
	ember.m_Quality         = settings->XmlQuality();
	ember.m_Supersample     = settings->XmlSupersample();
	ember.m_TemporalSamples = settings->XmlTemporalSamples();
	QApplication::clipboard()->setText(QString::fromStdString(emberToXml.ToString(ember, "", 0, false, true)));
}

void Fractorium::OnActionCopyXml(bool checked) { m_Controller->CopyXml(); }

/// <summary>
/// Copy the Xmls for all open embers as a single string to the clipboard, enclosed with the <flames> tag.
/// Apply Xml saving settings from the options first for each ember.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::CopyAllXml()
{
	ostringstream os;
	EmberToXml<T> emberToXml;
	os << "<flames>\n";

	for (auto& e : m_EmberFile.m_Embers)
	{
		Ember<T> ember = e;
		ApplyXmlSavingTemplate(ember);
		os << emberToXml.ToString(ember, "", 0, false, true);
	}

	os << "</flames>\n";
	QApplication::clipboard()->setText(QString::fromStdString(os.str()));
}

void Fractorium::OnActionCopyAllXml(bool checked) { m_Controller->CopyAllXml(); }

/// <summary>
/// Convert the Xml text from the clipboard to an ember, add it to the end
/// of the current file and set it as the current ember. If multiple Xmls were
/// copied to the clipboard and were enclosed in <flames> tags, then all of them will be added.
/// Clears the undo state.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::PasteXmlAppend()
{
	size_t previousSize = m_EmberFile.Size();
	string s, errors;
	XmlToEmber<T> parser;
	vector<Ember<T>> embers;
	auto b = QApplication::clipboard()->text().toUtf8();
	//auto codec = QTextCodec::codecForName("UTF-8");
	//auto b = codec->fromUnicode(QApplication::clipboard()->text());
	s.reserve(b.size());

	for (auto i = 0; i < b.size(); i++)
	{
		if (uint(b[i]) < 128u)
			s.push_back(b[i]);
	}

	b.clear();
	StopAllPreviewRenderers();
	parser.Parse(reinterpret_cast<byte*>(const_cast<char*>(s.c_str())), "", embers, true);
	errors = parser.ErrorReportString();

	if (errors != "")
	{
		m_Fractorium->ShowCritical("Paste Error", QString::fromStdString(errors));
	}

	if (!embers.empty())
	{
		for (auto i = 0; i < embers.size(); i++)
		{
			embers[i].m_Index = m_EmberFile.Size();
			ConstrainDimensions(embers[i]);//Do not exceed the max texture size.

			//Also ensure it has a name.
			if (embers[i].m_Name == "" || embers[i].m_Name == "No name")
				embers[i].m_Name = ToString<qulonglong>(embers[i].m_Index).toStdString();
			else
				embers[i].m_Name = CleanPath(embers[i].m_Name);

			m_EmberFile.m_Embers.push_back(embers[i]);//Will invalidate the pointers contained in the EmberTreeWidgetItems, UpdateLibraryTree() will resync.
		}

		m_EmberFile.MakeNamesUnique();
		UpdateLibraryTree();
		SetEmber(previousSize, false);
	}
}

void Fractorium::OnActionPasteXmlAppend(bool checked) { m_Controller->PasteXmlAppend(); }

/// <summary>
/// Convert the Xml text from the clipboard to an ember, overwrite the
/// current file and set the first as the current ember. If multiple Xmls were
/// copied to the clipboard and were enclosed in <flames> tags, then the current file will contain all of them.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::PasteXmlOver()
{
	size_t i = 0;
	string s, errors;
	XmlToEmber<T> parser;
	list<Ember<T>> embers;
	auto backupEmber = *m_EmberFile.m_Embers.begin();
	auto b = QApplication::clipboard()->text().toUtf8();
	//auto codec = QTextCodec::codecForName("UTF-8");
	//auto b = codec->fromUnicode(QApplication::clipboard()->text());
	s.reserve(b.size());

	for (auto i = 0; i < b.size(); i++)
	{
		if (uint(b[i]) < 128u)
			s.push_back(b[i]);
	}

	b.clear();
	StopAllPreviewRenderers();
	parser.Parse(reinterpret_cast<byte*>(const_cast<char*>(s.c_str())), "", embers, true);
	errors = parser.ErrorReportString();

	if (errors != "")
	{
		m_Fractorium->ShowCritical("Paste Error", QString::fromStdString(errors));
	}

	if (embers.size())
	{
		m_EmberFile.m_Embers = std::move(embers);//Will invalidate the pointers contained in the EmberTreeWidgetItems, UpdateLibraryTree() will resync.

		for (auto it : m_EmberFile.m_Embers)
		{
			it.m_Index = i++;
			ConstrainDimensions(it);//Do not exceed the max texture size.

			//Also ensure it has a name.
			if (it.m_Name == "" || it.m_Name == "No name")
				it.m_Name = ToString<qulonglong>(it.m_Index).toStdString();
			else
				it.m_Name = CleanPath(it.m_Name);
		}

		m_EmberFile.MakeNamesUnique();
		FillLibraryTree();
		SetEmber(0, false);
	}
}

void Fractorium::OnActionPasteXmlOver(bool checked) { m_Controller->PasteXmlOver(); }

/// <summary>
/// Copy the selected xforms.
/// Note this will also copy final if selected.
/// If none selected, just copy current.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::CopySelectedXforms()
{
	m_CopiedXforms.clear();
	m_CopiedFinalXform.Clear();
	UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex)
	{
		if (m_Ember.IsFinalXform(xform))
			m_CopiedFinalXform = *xform;
		else
			m_CopiedXforms.emplace_back(*xform, xfindex);
	}, eXformUpdate::UPDATE_SELECTED, false);
	m_Fractorium->ui.ActionPasteSelectedXforms->setEnabled(true);
}

void Fractorium::OnActionCopySelectedXforms(bool checked)
{
	m_Controller->CopySelectedXforms();
}

/// <summary>
/// Paste the selected xforms.
/// Note this will also paste/overwrite final if previously copied.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::PasteSelectedXforms()
{
	Update([&]()
	{
		AddXformsWithXaos(m_Ember, m_CopiedXforms, m_Fractorium->GetXaosPasteStyleType());

		if (!m_CopiedFinalXform.Empty())
			m_Ember.SetFinalXform(m_CopiedFinalXform);

		FillXforms();
	});
}

void Fractorium::OnActionPasteSelectedXforms(bool checked)
{
	m_Controller->PasteSelectedXforms();
}

/// <summary>
/// Copy the text of the OpenCL iteration kernel to the clipboard.
/// This performs no action if the renderer is not of type RendererCL.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::CopyKernel()
{
	if (auto rendererCL = dynamic_cast<RendererCL<T, float>*>(m_Renderer.get()))
		QApplication::clipboard()->setText(QString::fromStdString(rendererCL->IterKernel()));
}

void Fractorium::OnActionCopyKernel(bool checked) {	m_Controller->CopyKernel(); }

/// <summary>
/// Reset dock widgets and tabs to their default position.
/// Note that there is a bug in Qt, where it will only move them all to the left side if at least
/// one is on the left side, or they are all floating. If one or more are docked right, and none are docked
/// left, then it will put them all on the right side. Hopefully this isn't too much of a problem.
/// </summary>
void Fractorium::OnActionResetWorkspace(bool checked)
{
	QDockWidget* firstDock = nullptr;

	for (auto dock : m_Docks)
	{
		dock->setFloating(true);
		dock->setGeometry(QRect(100, 100, dock->width(), dock->height()));//Doesn't seem to have an effect.
		dock->setFloating(false);
		dock->show();

		if (firstDock)
			tabifyDockWidget(firstDock, dock);

		firstDock = dock;
	}

	ui.LibraryDockWidget->raise();
	ui.LibraryDockWidget->show();
}

/// <summary>
/// Alternate between Editor/Image.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionAlternateEditorImage(bool checked)
{
	if (DrawPreAffines() || DrawPostAffines())
	{
		SaveAffineState();
		ui.ActionDrawPreAffines->setChecked(false);
		ui.ActionDrawAllPreAffines->setChecked(false);
		ui.ActionDrawPostAffines->setChecked(false);
		ui.ActionDrawAllPostAffines->setChecked(false);
		ui.ActionDrawGrid->setChecked(false);
		ui.ActionDrawImage->setChecked(true);
	}
	else
	{
		ui.ActionDrawImage->setChecked(false);
		ui.ActionDrawGrid->setChecked(true);
		SyncAffineStateToToolbar();
	}

	ui.GLDisplay->update();
}

/// <summary>
/// Reset the scale used to draw affines, which was adjusted by zooming with the Alt key pressed.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionResetScale(bool checked)
{
	m_Controller->InitLockedScale();
	ui.GLDisplay->update();
}

/// <summary>
/// Add reflective symmetry to the current ember.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::AddReflectiveSymmetry()
{
	bool forceFinal = m_Fractorium->HaveFinal();
	Update([&]()
	{
		m_Ember.AddSymmetry(-1, m_Rand);
		auto index = m_Ember.TotalXformCount(forceFinal) - (forceFinal ? 2 : 1);//Set index to the last item before final.
		FillXforms(int(index));
	});
}

void Fractorium::OnActionAddReflectiveSymmetry(bool checked) { m_Controller->AddReflectiveSymmetry(); }

/// <summary>
/// Add rotational symmetry to the current ember.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::AddRotationalSymmetry()
{
	bool forceFinal = m_Fractorium->HaveFinal();
	Update([&]()
	{
		m_Ember.AddSymmetry(2, m_Rand);
		auto index = m_Ember.TotalXformCount(forceFinal) - (forceFinal ? 2 : 1);//Set index to the last item before final.
		FillXforms(int(index));
	});
}

void Fractorium::OnActionAddRotationalSymmetry(bool checked) { m_Controller->AddRotationalSymmetry(); }

/// <summary>
/// Add both reflective and rotational symmetry to the current ember.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::AddBothSymmetry()
{
	bool forceFinal = m_Fractorium->HaveFinal();
	Update([&]()
	{
		m_Ember.AddSymmetry(-2, m_Rand);
		auto index = m_Ember.TotalXformCount(forceFinal) - (forceFinal ? 2 : 1);//Set index to the last item before final.
		FillXforms(int(index));
	});
}

void Fractorium::OnActionAddBothSymmetry(bool checked) { m_Controller->AddBothSymmetry(); }

/// <summary>
/// Adds a FlattenVariation to every xform in the current ember.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::Flatten()
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.Flatten(XmlToEmber<T>::m_FlattenNames);
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
	FillVariationTreeWithCurrentXform();
}
void Fractorium::OnActionFlatten(bool checked) { m_Controller->Flatten(); }

/// <summary>
/// Removes pre/reg/post FlattenVariation from every xform in the current ember.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::Unflatten()
{
	UpdateAll([&](Ember<T>& ember, bool isMain)
	{
		ember.Unflatten();
	}, true, eProcessAction::FULL_RENDER, m_Fractorium->ApplyAll());
	FillVariationTreeWithCurrentXform();
}
void Fractorium::OnActionUnflatten(bool checked) { m_Controller->Unflatten(); }

/// <summary>
/// Delete all but one xform in the current ember.
/// Clear that xform's variations.
/// Resets the rendering process.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::ClearFlame()
{
	Update([&]()
	{
		Xform<T> newXform;
		newXform.m_Weight = 0.25;
		newXform.m_ColorX = m_Rand.Frand01<T>();
		newXform.AddVariation(m_VariationList->GetVariationCopy(eVariationId::VAR_LINEAR));
		m_Ember.Clear();
		m_Ember.m_FinalRasW = m_Fractorium->m_WidthSpin->value();
		m_Ember.m_FinalRasH = m_Fractorium->m_HeightSpin->value();
		m_Ember.m_Quality = m_Fractorium->m_QualitySpin->value();
		m_Ember.AddXform(newXform);
		FillXforms();//Must do this first because the palette setup in FillParamTablesAndPalette() uses the xforms combo.
		FillParamTablesAndPalette();
		FillCurvesControl();
		FillSummary();
	});
}

void Fractorium::OnActionClearFlame(bool checked) { m_Controller->ClearFlame(); }

/// <summary>
/// Re-render all previews.
/// </summary>
void Fractorium::OnActionRenderPreviews(bool checked)
{
	m_Controller->RenderLibraryPreviews();
}

/// <summary>
/// Stop all previews from being rendered. This is handy if the user
/// opens a large file with many embers in it, such as an animation sequence.
/// </summary>
void Fractorium::OnActionStopRenderingPreviews(bool checked) { m_Controller->StopLibraryPreviewRender(); }

/// <summary>
/// Show the final render dialog as a modeless dialog to allow
/// the user to minimize the main window while doing a lengthy final render.
/// Note: The user probably should not be otherwise interacting with the main GUI
/// while the final render is taking place.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionFinalRender(bool checked)
{
	//First completely stop what the current rendering process is doing.
	m_Controller->DeleteRenderer();//Delete the renderer, but not the controller.
	m_Controller->StopAllPreviewRenderers();
	m_Controller->SaveCurrentToOpenedFile(false);//Save whatever was edited back to the current open file.
	m_RenderStatusLabel->setText("Renderer stopped.");
	SetupFinalRenderDialog();

	if (m_FinalRenderDialog.get())
		m_FinalRenderDialog->Show(false);
}

/// <summary>
/// Called when the final render dialog has been closed.
/// </summary>
/// <param name="result">Ignored</param>
void Fractorium::OnFinalRenderClose(int result)
{
	m_RenderStatusLabel->setText("Renderer starting...");
	StartRenderTimer(false);//Re-create the renderer and start rendering again.
	ui.ActionStartStopRenderer->setChecked(false);//Re-enable any controls that might have been disabled.
	OnActionStartStopRenderer(false);
	m_FinalRenderDialog.reset();
}

/// <summary>
/// Show the final options dialog.
/// Restart rendering and sync options after the options dialog is dismissed with Ok.
/// Called when the options dialog is finished with ok.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionOptions(bool checked)
{
	bool ec = m_Settings->EarlyClip();
	bool yup = m_Settings->YAxisUp();
	bool trans = m_Settings->Transparency();
	bool compat = m_Settings->Flam3Compat();

	if (m_OptionsDialog->exec())
	{
		bool updatePreviews = ec != m_Settings->EarlyClip() ||
							  yup != m_Settings->YAxisUp() ||
							  trans != m_Settings->Transparency() ||
							  compat != m_Settings->Flam3Compat();
		Compat::m_Compat = m_Settings->Flam3Compat();
		SyncOptionsToToolbar();//This won't trigger a recreate, the call below handles it.
		ShutdownAndRecreateFromOptions(updatePreviews);//This will recreate the controller and/or the renderer from the options if necessary, then start the render timer.
	}
}

/// <summary>
/// Show the about dialog.
/// </summary>
/// <param name="checked">Ignored</param>
void Fractorium::OnActionAbout(bool checked)
{
	m_AboutDialog->exec();
}

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
