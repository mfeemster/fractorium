#include "FractoriumPch.h"
#include "Fractorium.h"

/// <summary>
/// Return whether the render timer is running.
/// </summary>
/// <returns>True if running, else false.</returns>
bool FractoriumEmberControllerBase::RenderTimerRunning()
{
	return m_RenderTimer->isActive();
}

/// <summary>
/// Start the render timer.
/// If a renderer has not been created yet, it will be created from the options.
/// </summary>
void FractoriumEmberControllerBase::StartRenderTimer()
{
	UpdateRender();
	m_RenderTimer->start();
	m_RenderElapsedTimer.Tic();
}

/// <summary>
/// Start the render timer after a short delay.
/// If the timer is already running, stop it first.
/// This is useful for stopping and restarting the render
/// process in response to things like a window resize.
/// </summary>
void FractoriumEmberControllerBase::DelayedStartRenderTimer()
{
	DeleteRenderer();
	m_RenderRestartTimer->setSingleShot(true);
	m_RenderRestartTimer->start(300);//Will stop the timer if it's already running, and start again.
}

/// <summary>
/// Stop the render timer and abort the rendering process.
/// Optionally block until stopping is complete.
/// </summary>
/// <param name="wait">True to block, else false.</param>
void FractoriumEmberControllerBase::StopRenderTimer(bool wait)
{
	m_RenderTimer->stop();

	if (m_Renderer.get())
		m_Renderer->Abort();

	if (wait)
	{
		while (m_Rendering || RenderTimerRunning() || (Renderer() && (!m_Renderer->Aborted() || m_Renderer->InRender())))
			QApplication::processEvents();
	}
}

/// <summary>
/// Stop all timers, rendering and drawing and block until they are done.
/// </summary>
void FractoriumEmberControllerBase::Shutdown()
{
	StopRenderTimer(true);
	ClearFinalImages();

	while (m_Fractorium->ui.GLDisplay->Drawing())
		QApplication::processEvents();
}

/// <summary>
/// Clear the output image buffers.
/// </summary>
void FractoriumEmberControllerBase::ClearFinalImages()
{
	Memset(m_FinalImage);
	//Unsure if we should also call RendererCL::ClearFinal() as well. At the moment it seems unnecessary.
}

/// <summary>
/// Update the state of the renderer.
/// Upon changing values, some intelligence is used to avoid blindly restarting the
/// entire iteration proceess every time a value changes. This is because some values don't affect the
/// iteration, and only affect filtering and final accumulation. They are broken into three categories:
/// 1) Restart the entire process.
/// 2) Log/density filter, then final accum.
/// 3) Final accum only.
/// 4) Continue iterating.
/// </summary>
/// <param name="action">The action to take</param>
void FractoriumEmberControllerBase::UpdateRender(eProcessAction action)
{
	AddProcessAction(action);
	m_RenderElapsedTimer.Tic();
}

/// <summary>
/// Call Shutdown() then delete the renderer and clear the textures in the output window if there is one.
/// Note the name is somewhat misleading because a new empty renderer is actually created as a placeholder.
/// This is that the program won't crash if the user adjusts any of the controls while the renderer is shut down.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::DeleteRenderer()
{
	Shutdown();
	m_Renderer = make_unique<EmberNs::Renderer<T, float>>();

	if (GLController())
		GLController()->ClearWindow();
}

/// <summary>
/// Save the current render results to a file.
/// This could benefit from QImageWriter, however its compression capabilities are
/// severely lacking. A Png file comes out larger than a bitmap, so instead use the
/// Png and Jpg wrapper functions from the command line programs.
/// This will embed the id, url and nick fields from the options in the image comments.
/// </summary>
/// <param name="filename">The full path and filename</param>
/// <param name="comments">The comments to save in the png, jpg or exr</param>
/// <param name="pixels">The buffer containing the pixels</param>
/// <param name="width">The width in pixels of the image</param>
/// <param name="height">The height in pixels of the image</param>
/// <param name="png16Bit">Whether to use 16 bits per channel per pixel when saving as Png/32-bits per channel when saving as Exr.</param>
/// <param name="transparency">Whether to use alpha when saving as Png or Exr.</param>
bool FractoriumEmberControllerBase::SaveCurrentRender(const QString& filename, const EmberImageComments& comments, vector<v4F>& pixels, size_t width, size_t height, bool png16Bit, bool transparency)
{
	bool ret = false;

	if (filename != "")
	{
		const auto size = width * height;
		auto settings = m_Fractorium->m_Settings;
		QFileInfo fileInfo(filename);
		QString suffix = fileInfo.suffix();
		string s = filename.toStdString();
		string id = settings->Id().toStdString();
		string url = settings->Url().toStdString();
		string nick = settings->Nick().toStdString();

		//Ensure dimensions are valid.
		if (pixels.size() < size)
		{
			m_Fractorium->ShowCritical("Save Failed", "Dimensions didn't match, not saving.", true);
			return ret;
		}

		auto data = pixels.data();

		if (suffix.endsWith("bmp", Qt::CaseSensitivity::CaseInsensitive) || suffix.endsWith("jpg", Qt::CaseSensitivity::CaseInsensitive))
		{
			vector<unsigned char> rgb8Image(size * 3);
			Rgba32ToRgb8(data, rgb8Image.data(), width, height);

			if (suffix.endsWith("bmp", Qt::CaseSensitivity::CaseInsensitive))
				ret = WriteBmp(s.c_str(), rgb8Image.data(), width, height);
			else if (suffix.endsWith("jpg", Qt::CaseSensitivity::CaseInsensitive))
				ret = WriteJpeg(s.c_str(), rgb8Image.data(), width, height, 100, true, comments, id, url, nick);
		}
		else if (suffix.endsWith("png", Qt::CaseSensitivity::CaseInsensitive))
		{
			if (!png16Bit)
			{
				vector<unsigned char> rgba8Image(size * 4);
				Rgba32ToRgba8(data, rgba8Image.data(), width, height, transparency);
				ret = WritePng(s.c_str(), rgba8Image.data(), width, height, 1, true, comments, id, url, nick);
			}
			else
			{
				vector<glm::uint16> rgba16Image(size * 4);
				Rgba32ToRgba16(data, rgba16Image.data(), width, height, transparency);
                ret = WritePng(s.c_str(), reinterpret_cast<unsigned char*>(rgba16Image.data()), width, height, 2, true, comments, id, url, nick);
			}
		}
		else if (suffix.endsWith("exr", Qt::CaseSensitivity::CaseInsensitive))
		{
			if (!png16Bit)//Repurpose this for EXR 32-bit.
			{
				vector<Rgba> rgba32Image(size);
				Rgba32ToRgbaExr(data, rgba32Image.data(), width, height, transparency);
				ret = WriteExr16(s.c_str(), rgba32Image.data(), width, height, true, comments, id, url, nick);
			}
			else
			{
				vector<float> r(size);
				vector<float> g(size);
				vector<float> b(size);
				vector<float> a(size);
				Rgba32ToRgba32Exr(data, r.data(), g.data(), b.data(), a.data(), width, height, transparency);
				ret = WriteExr32(s.c_str(), r.data(), g.data(), b.data(), a.data(), width, height, true, comments, id, url, nick);
			}
		}
		else
		{
			m_Fractorium->ShowCritical("Save Failed", "Unrecognized format " + suffix + ", not saving.", true);
			return ret;
		}

		if (ret)
			settings->SaveFolder(fileInfo.canonicalPath());
		else
			m_Fractorium->ShowCritical("Save Failed", "Could not save file, try saving to a different folder.", true);
	}

	return ret;
}

/// <summary>
/// Add a process action to the list of actions to take.
/// Called in response to the user changing something on the GUI.
/// </summary>
/// <param name="action">The action for the renderer to take</param>
void FractoriumEmberControllerBase::AddProcessAction(eProcessAction action)
{
	rlg l(m_Cs);
	m_ProcessActions.push_back(action);

	if (m_Renderer.get())
		m_Renderer->Abort();
}

/// <summary>
/// Condense and clear the process actions into a single action and return.
/// Many actions may be specified, but only the one requiring the greatest amount
/// of processing matters. Extract and return the greatest and clear the vector.
/// </summary>
/// <returns>The most significant processing action desired</returns>
eProcessAction FractoriumEmberControllerBase::CondenseAndClearProcessActions()
{
	rlg l(m_Cs);
	auto action = eProcessAction::NOTHING;

	for (auto a : m_ProcessActions)
		if (a > action)
			action = a;

	m_ProcessActions.clear();
	return action;
}

/// <summary>
/// Render progress callback function to update progress bar.
/// </summary>
/// <param name="ember">The ember currently being rendered</param>
/// <param name="foo">An extra dummy parameter</param>
/// <param name="fraction">The progress fraction from 0-100</param>
/// <param name="stage">The stage of iteration. 1 is iterating, 2 is density filtering, 2 is final accumulation.</param>
/// <param name="etaMs">The estimated milliseconds to completion of the current stage</param>
/// <returns>0 if the user has changed anything on the GUI, else 1 to continue rendering.</returns>
template <typename T>
int FractoriumEmberController<T>::ProgressFunc(Ember<T>& ember, void* foo, double fraction, int stage, double etaMs)
{
	QString status;
	QMetaObject::invokeMethod(m_Fractorium->m_ProgressBar, "setValue", Qt::ConnectionType::QueuedConnection, Q_ARG(const int, int(fraction)));//Only really applies to iter and filter, because final accum only gives progress 0 and 100.

	if (stage == 0)
		status = "Iterating";
	else if (stage == 1)
		status = "Density Filtering";
	else if (stage == 2)
		status = "Spatial Filtering + Final Accumulation";

	QMetaObject::invokeMethod(m_Fractorium->m_RenderStatusLabel, "setText", Qt::ConnectionType::QueuedConnection, Q_ARG(const QString&, status));
	return m_ProcessActions.empty() ? 1 : 0;//If they've done anything, abort.
}

/// <summary>
/// Clear the undo list as well as the undo/redo index and state.
/// </summary>
template <typename T>
void FractoriumEmberController<T>::ClearUndo()
{
	m_UndoIndex = 0;
	m_UndoList.clear();
	m_EditState = eEditUndoState::REGULAR_EDIT;
	m_LastEditWasUndoRedo = false;
	m_Fractorium->ui.ActionUndo->setEnabled(false);
	m_Fractorium->ui.ActionRedo->setEnabled(false);
}

/// <summary>
/// The hierarchy/order of sizes is like so:
/// Ember
///		GL Widget
///			Texture (passed to RendererCL)
///				Viewport
/// Since this uses m_GL->SizesMatch(), which uses the renderer's dimensions, this
/// must be called after the renderer has set the current ember.
/// </summary>
/// <returns>True if dimensions had to be resized due to a mismatch, else false.</returns>
template <typename T>
bool FractoriumEmberController<T>::SyncSizes()
{
	bool changed = false;
	auto gl = m_Fractorium->ui.GLDisplay;
	RendererCL<T, float>* rendererCL = nullptr;

	if (gl && !m_GLController->SizesMatch())
	{
		m_GLController->ClearWindow();
		gl->SetDimensions(static_cast<int>(m_Ember.m_FinalRasW), static_cast<int>(m_Ember.m_FinalRasH));
		gl->Allocate();
		gl->SetViewport();

		if (m_Renderer->RendererType() == eRendererType::OPENCL_RENDERER && (rendererCL = dynamic_cast<RendererCL<T, float>*>(m_Renderer.get())))
			rendererCL->SetOutputTexture(gl->OutputTexID());

		m_Fractorium->CenterScrollbars();
		changed = true;
	}

	return changed;
}

/// <summary>
/// The main rendering function.
/// Called whenever the event loop is idle.
/// </summary>
/// <returns>True if nothing went wrong, else false.</returns>
template <typename T>
bool FractoriumEmberController<T>::Render()
{
	if (!m_Renderer.get())
		return false;

	m_Rendering = true;
	bool success = true;
	auto gl = m_Fractorium->ui.GLDisplay;
	RendererCL<T, float>* rendererCL = nullptr;
	eProcessAction qualityAction = eProcessAction::NOTHING, action = eProcessAction::NOTHING;
	//Quality is the only parameter we update inside the timer.
	//This is to allow the user to rapidly increase the quality spinner
	//without fully resetting the render. Instead, it will just keep iterating
	//where it last left off in response to an increase.
	T d = static_cast<T>(m_Fractorium->m_QualitySpin->value());

	if (d < m_Ember.m_Quality)//Full restart if quality decreased.
	{
		m_Ember.m_Quality = d;
		qualityAction = eProcessAction::FULL_RENDER;
	}
	else if (d > m_Ember.m_Quality && ProcessState() == eProcessState::ACCUM_DONE)//If quality increased, keep iterating after current render finishes.
	{
		m_Ember.m_Quality = d;
		qualityAction = eProcessAction::KEEP_ITERATING;
	}
	else if (IsClose(d, m_Ember.m_Quality))
		qualityAction = eProcessAction::NOTHING;

	if (qualityAction == eProcessAction::FULL_RENDER)
		Update([&] {});//Stop the current render, a full restart is needed.
	else if (qualityAction == eProcessAction::KEEP_ITERATING)
		m_ProcessActions.push_back(qualityAction);//Special, direct call to avoid resetting the render inside Update() because only KEEP_ITERATING is needed.

	action = CondenseAndClearProcessActions();//Combine with all other previously requested actions.

	if (m_Renderer->RendererType() == eRendererType::OPENCL_RENDERER)
		rendererCL = dynamic_cast<RendererCL<T, float>*>(m_Renderer.get());

	//Force temporal samples to always be 1. Perhaps change later when animation is implemented.
	m_Ember.m_TemporalSamples = 1;

	//Take care of solo xforms and set the current ember and action.
	if (action != eProcessAction::NOTHING)
	{
		intmax_t i = 0;
		const auto solo = m_Ember.m_Solo;
		const bool forceFinal = m_Fractorium->HaveFinal();

		if (solo != -1)
		{
			m_TempOpacities.resize(m_Ember.TotalXformCount(forceFinal));

			while (auto xform = m_Ember.GetTotalXform(i))
			{
				m_TempOpacities[i] = xform->m_Opacity;
				xform->m_Opacity = i++ == solo ? 1 : 0;
			}
		}

		i = 0;
		m_Renderer->SetEmber(m_Ember, action, true);

		if (solo != -1)
			while (auto xform = m_Ember.GetTotalXform(i, forceFinal))
				xform->m_Opacity = m_TempOpacities[i++];
	}

	//Ensure sizes are equal and if not, update dimensions.
	if (SyncSizes())
	{
		action = eProcessAction::FULL_RENDER;
		return true;
	}

	//Determining if a completely new rendering process is being started.
	const auto iterBegin = ProcessState() == eProcessState::NONE;

	if (iterBegin)
	{
		if (m_Renderer->RendererType() == eRendererType::CPU_RENDERER)
			m_SubBatchCount = m_Fractorium->m_Settings->CpuSubBatch();
		else if (m_Renderer->RendererType() == eRendererType::OPENCL_RENDERER)
			m_SubBatchCount = m_Fractorium->m_Settings->OpenCLSubBatch();

		m_Fractorium->m_ProgressBar->setValue(0);
		const QString status = "Starting";
		QMetaObject::invokeMethod(m_Fractorium->m_RenderStatusLabel, "setText", Qt::ConnectionType::QueuedConnection, Q_ARG(const QString&, status));
	}

	//If the rendering process hasn't finished, render with the current specified action.
	if (ProcessState() != eProcessState::ACCUM_DONE)
	{
		//if (m_Renderer->Run(m_FinalImage, 0) == RENDER_OK)//Full, non-incremental render for debugging.
		const bool update = iterBegin || m_Fractorium->m_Settings->ContinuousUpdate();
		eRenderStatus result;

		if ((result = m_Renderer->Run(m_FinalImage, 0, m_SubBatchCount, update)) == eRenderStatus::RENDER_OK)//Force output on iterBegin or if the settings specify to always do it.
		{
			//The amount to increment sub batch while rendering proceeds is purely empirical.
			//Change later if better values can be derived/observed.
			if (m_Renderer->RendererType() == eRendererType::OPENCL_RENDERER)
			{
				if (m_SubBatchCount < (4 * m_Devices.size()))//More than 4 with OpenCL gives a sluggish UI.
					m_SubBatchCount += m_Devices.size();
			}
			else
			{
				if (m_SubBatchCount < 5)
					m_SubBatchCount++;
				else if (m_SubBatchCount < 105)//More than 105 with CPU gives a sluggish UI.
					m_SubBatchCount += 25;
			}

			//Rendering has finished, update final stats.
			if (ProcessState() == eProcessState::ACCUM_DONE)
			{
				const auto stats = m_Renderer->Stats();
				auto iters = ToString<qulonglong>(stats.m_Iters);
				auto scaledQuality = ToString(static_cast<qulonglong>(m_Renderer->ScaledQuality()));
				//auto renderTime = m_RenderElapsedTimer.Format(m_RenderElapsedTimer.Toc());
				auto renderTime = m_RenderElapsedTimer.Format(stats.m_RenderMs);
				m_Fractorium->m_ProgressBar->setValue(100);

				//Only certain stats can be reported with OpenCL.
				if (m_Renderer->RendererType() == eRendererType::OPENCL_RENDERER)
				{
					const QString status = "Iters: " + iters + ". Scaled quality: " + scaledQuality + ". Total time: " + QString::fromStdString(renderTime) + ".";
					QMetaObject::invokeMethod(m_Fractorium->m_RenderStatusLabel, "setText", Qt::ConnectionType::QueuedConnection, Q_ARG(const QString&, status));
				}
				else
				{
					const auto percent = static_cast<double>(stats.m_Badvals) / static_cast<double>(stats.m_Iters);
					auto badVals = ToString<qulonglong>(stats.m_Badvals);
					auto badPercent = QLocale::system().toString(percent * 100, 'f', 2);
					const QString status = "Iters: " + iters + ". Scaled quality: " + scaledQuality + ". Bad values: " + badVals + " (" + badPercent + "%). Total time: " + QString::fromStdString(renderTime) + ".";
					QMetaObject::invokeMethod(m_Fractorium->m_RenderStatusLabel, "setText", Qt::ConnectionType::QueuedConnection, Q_ARG(const QString&, status));
				}

				if (m_LastEditWasUndoRedo && (m_UndoIndex == m_UndoList.size() - 1))//Traversing through undo list, reached the end, so put back in regular edit mode.
				{
					m_EditState = eEditUndoState::REGULAR_EDIT;
				}
				else if (m_EditState == eEditUndoState::REGULAR_EDIT)//Regular edit, just add to the end of the undo list.
				{
					const auto btn = QApplication::mouseButtons();

					if ((action == eProcessAction::ACCUM_ONLY || action == eProcessAction::FILTER_AND_ACCUM) ||
							(!btn.testFlag(Qt::MouseButton::LeftButton) && !btn.testFlag(Qt::MouseButton::RightButton) && !btn.testFlag(Qt::MouseButton::MiddleButton)))
					{
						m_UndoList.push_back(m_Ember);
						m_UndoIndex = m_UndoList.size() - 1;
						m_Fractorium->ui.ActionUndo->setEnabled(m_UndoList.size() > 1);
						m_Fractorium->ui.ActionRedo->setEnabled(false);

						if (m_UndoList.size() >= UNDO_SIZE)
							m_UndoList.pop_front();
					}

					//else
					//	qDebug() << "Mouse was down, not adding to undo list.";
				}
				else if (!m_LastEditWasUndoRedo &&
						 m_UndoList.size() &&
						 m_UndoIndex < m_UndoList.size() - 1)//They were anywhere but the end of the undo list, then did a manual edit, so clear the undo list.
				{
					Ember<T> ember(m_UndoList[m_UndoIndex]);
					ClearUndo();
					m_UndoList.push_back(ember);
					m_UndoList.push_back(m_Ember);
					m_UndoIndex = m_UndoList.size() - 1;
					m_Fractorium->ui.ActionUndo->setEnabled(true);
					m_Fractorium->ui.ActionRedo->setEnabled(false);
				}

				m_LastEditWasUndoRedo = false;
				m_Fractorium->UpdateHistogramBounds();//Mostly of engineering interest.
				FillSummary();//Only update summary on render completion since it's not the type of thing the user needs real-time updates on.
			}

			//Update the GL window on start or continuous rendering because the output will be forced.
			//Update it on finish because the rendering process is completely done.
			if (update || ProcessState() == eProcessState::ACCUM_DONE)
			{
				gl->update();//Queue update.

				if (ProcessState() == eProcessState::ACCUM_DONE)
					SaveCurrentToOpenedFile();//Will not save if the previews are still rendering.

				//Uncomment for debugging kernel build and execution errors.
				//m_Fractorium->ui.InfoRenderingTextEdit->setText(QString::fromStdString(m_Fractorium->m_Wrapper.DumpInfo()));
				//if (rendererCL)
				//{
				//	string s = "OpenCL Kernels: \r\n" + rendererCL->IterKernel() + "\r\n" + rendererCL->DEKernel() + "\r\n" + rendererCL->FinalAccumKernel();
				//
				//	QMetaObject::invokeMethod(m_Fractorium->ui.InfoRenderingTextEdit, "setText", Qt::ConnectionType::QueuedConnection, Q_ARG(const QString&, QString::fromStdString(s)));
				//}
			}
		}
		else if (result != eRenderStatus::RENDER_ABORT)//If error was returned, something went very wrong, show error report.
		{
			auto errors = m_Renderer->ErrorReport();
			success = false;
			m_FailedRenders++;
			const QString status = "Rendering failed, see info tab. Try changing parameters.";
			QMetaObject::invokeMethod(m_Fractorium->m_RenderStatusLabel, "setText", Qt::ConnectionType::QueuedConnection, Q_ARG(const QString&, status));
			m_Fractorium->ErrorReportToQTextEdit(errors, m_Fractorium->ui.InfoRenderingTextEdit);
			m_Renderer->ClearErrorReport();

			if (m_FailedRenders >= 3)
			{
				m_Rendering = false;
				StopRenderTimer(true);
				const QString status2 = "Rendering failed 3 or more times, stopping all rendering, see info tab. Try changing renderer types.";
				QMetaObject::invokeMethod(m_Fractorium->m_RenderStatusLabel, "setText", Qt::ConnectionType::QueuedConnection, Q_ARG(const QString&, status2));
				ClearFinalImages();
				m_GLController->ClearWindow();

				if (rendererCL)
				{
					//string s = "OpenCL Kernels: \r\n" + rendererCL->IterKernel() + "\r\n" + rendererCL->DEKernel() + "\r\n" + rendererCL->FinalAccumKernel();
					rendererCL->ClearFinal();
					//QMetaObject::invokeMethod(m_Fractorium->ui.InfoRenderingTextEdit, "setText", Qt::ConnectionType::QueuedConnection, Q_ARG(const QString&, QString::fromStdString(s)));
				}
			}
		}
	}

	//Upon finishing, or having nothing to do, rest.
	if (ProcessState() == eProcessState::ACCUM_DONE)
		QThread::msleep(1);

	m_Rendering = false;
	return success;
}

/// <summary>
/// Stop rendering and initialize a new renderer, using the specified type.
/// Rendering will be left in a stopped state. The caller is responsible for restarting the render loop again.
/// </summary>
/// <param name="renderType">The type of render to create</param>
/// <param name="devices">The platform,device index pairs of the devices to use</param>
/// <param name="updatePreviews">True to re-render the library previews, else false.</param>
/// <param name="shared">True if shared with OpenGL, else false. Default: true.</param>
/// <returns>True if nothing went wrong, else false.</returns>
template <typename T>
bool FractoriumEmberController<T>::CreateRenderer(eRendererType renderType, const vector<pair<size_t, size_t>>& devices, bool updatePreviews, bool shared)
{
	bool ok = true;
	auto s = m_Fractorium->m_Settings;
	auto gl = m_Fractorium->ui.GLDisplay;

	if (!m_Renderer.get() || (m_Renderer->RendererType() != renderType) || !Equal(m_Devices, devices) || m_Renderer->Shared() != shared)
	{
		EmberReport emberReport;
		vector<string> errorReport;
		DeleteRenderer();//Delete the renderer and refresh the textures.

		//Before starting, must take care of allocations.
		if (gl->Allocate(true))//Forcing a realloc of the texture is necessary on AMD, but not on nVidia.
		{
			m_Renderer = unique_ptr<EmberNs::RendererBase>(::CreateRenderer<T>(renderType, devices, shared, gl->OutputTexID(), emberReport));//Always make bucket type float.
			errorReport = emberReport.ErrorReport();

			if (errorReport.empty())
			{
				m_Devices = devices;
				m_OutputTexID = gl->OutputTexID();
			}
			else
			{
				ok = false;
				m_Fractorium->ShowCritical("Renderer Creation Error", "Could not create requested renderer, fallback CPU renderer created. See info tab for details.");
				m_Fractorium->ErrorReportToQTextEdit(errorReport, m_Fractorium->ui.InfoRenderingTextEdit);
			}
		}
		else
		{
			ok = false;
			m_Fractorium->ShowCritical("Renderer Creation Error", "Could not create OpenGL texture, interactive rendering will be disabled.");
			m_Fractorium->ErrorReportToQTextEdit(errorReport, m_Fractorium->ui.InfoRenderingTextEdit);
		}
	}

	if (m_Renderer.get())
	{
		m_RenderType = m_Renderer->RendererType();

		if (m_RenderType == eRendererType::OPENCL_RENDERER)
		{
			auto val = m_Fractorium->m_Settings->OpenClQuality() * m_Fractorium->m_Settings->Devices().size();
			m_Fractorium->m_QualitySpin->DoubleClickZero(val);
			m_Fractorium->m_QualitySpin->DoubleClickNonZero(val);

			if (m_Fractorium->m_QualitySpin->value() < val)
				m_Fractorium->m_QualitySpin->setValue(val);

			if (auto rendererCL = dynamic_cast<RendererCL<T, float>*>(m_Renderer.get()))
				rendererCL->SubBatchPercentPerThread(static_cast<float>(s->OpenCLSubBatchPct()));
		}
		else
		{
			const auto quality = m_Fractorium->m_Settings->CpuQuality();
			m_Fractorium->m_QualitySpin->DoubleClickZero(quality);
			m_Fractorium->m_QualitySpin->DoubleClickNonZero(quality);

			if (m_Fractorium->m_QualitySpin->value() > quality)
				m_Fractorium->m_QualitySpin->setValue(quality);
		}

		m_Renderer->Callback(this);
		m_Renderer->ReclaimOnResize(true);
		//Give it an initial ember, will be updated many times later.
		//Even though the bounds are computed when starting the next render. The OpenGL draw calls use these values, which might get called before the render starts.
		m_Renderer->SetEmber(m_Ember, eProcessAction::FULL_RENDER, true);
		m_Renderer->EarlyClip(s->EarlyClip());
		m_Renderer->YAxisUp(s->YAxisUp());
		m_Renderer->ThreadCount(s->ThreadCount());

		if (m_Renderer->RendererType() == eRendererType::CPU_RENDERER)
			m_Renderer->InteractiveFilter(s->CpuDEFilter() ? eInteractiveFilter::FILTER_DE : eInteractiveFilter::FILTER_LOG);
		else
			m_Renderer->InteractiveFilter(s->OpenCLDEFilter() ? eInteractiveFilter::FILTER_DE : eInteractiveFilter::FILTER_LOG);

		if (updatePreviews)
		{
			m_LibraryPreviewRenderer = make_unique<TreePreviewRenderer<T>>(this, m_Fractorium->ui.LibraryTree, m_EmberFile, nullptr);//Will take the same settings as the main renderer.
			RenderLibraryPreviews();
		}

		m_FailedRenders = 0;
		m_RenderElapsedTimer.Tic();
		//Leave rendering in a stopped state. The caller is responsible for restarting the render loop again.
	}
	else
	{
		ok = false;
		m_Fractorium->ShowCritical("Renderer Creation Error", "Creating a basic CPU renderer failed, something is catastrophically wrong. Exiting program.");
		QApplication::quit();
	}

	return ok;
}

/// <summary>
/// Enable or disable the controls related to changing the renderer.
/// Used when pausing and restarting the renderer.
/// </summary>
/// <param name="enable">True to enable, else false.</param>
void Fractorium::EnableRenderControls(bool enable)
{
	ui.ActionCpu->setEnabled(enable);
	ui.ActionCL->setEnabled(enable);
	ui.ActionSP->setEnabled(enable);
	ui.ActionDP->setEnabled(enable);
	ui.ActionOptions->setEnabled(enable);
}

/// <summary>
/// Wrapper to stop the timer, shutdown the controller and recreate, then restart the controller and renderer from the options.
/// </summary>
/// <param name="updatePreviews">True to re-render the library previews, else false.</param>
void Fractorium::ShutdownAndRecreateFromOptions(bool updatePreviews)
{
	//First completely stop what the current rendering process is doing.
	m_Controller->Shutdown();
	StartRenderTimer(updatePreviews);//This will recreate the controller and/or the renderer from the options if necessary, then start the render timer.
	m_Settings->sync();
}

/// <summary>
/// Create a new renderer from the options.
/// </summary>
/// <param name="updatePreviews">True to re-render the library previews, else false.</param>
/// <returns>True if nothing went wrong, else false.</returns>
bool Fractorium::CreateRendererFromOptions(bool updatePreviews)
{
	bool ok = true;
	const auto useOpenCL = m_Info->Ok() && m_Settings->OpenCL();
	auto v = Devices(m_Settings->Devices());
	const auto doOpenCL = useOpenCL && !v.empty();
	ui.ActionCopyKernel->setEnabled(doOpenCL);

	//The most important option to process is what kind of renderer is desired, so do it first.
	if (!m_Controller->CreateRenderer(doOpenCL ? eRendererType::OPENCL_RENDERER : eRendererType::CPU_RENDERER, v, updatePreviews, doOpenCL && m_Settings->SharedTexture()))
	{
		//If using OpenCL, will only get here if creating RendererCL failed, but creating a backup CPU Renderer succeeded.
		ShowCritical("Renderer Creation Error", "Error creating renderer, most likely a GPU problem. Using CPU instead.");
		m_Settings->OpenCL(false);
		m_Settings->SharedTexture(false);
		ui.ActionCpu->setChecked(true);
		ui.ActionCL->setChecked(false);
		ui.ActionCopyKernel->setEnabled(false);
		m_OptionsDialog->ui.OpenCLCheckBox->setChecked(false);
		m_OptionsDialog->ui.SharedTextureCheckBox->setChecked(false);

		if (m_FinalRenderDialog)
			m_FinalRenderDialog->ui.FinalRenderOpenCLCheckBox->setChecked(false);

		ok = false;
	}

	if (auto rendererCL = dynamic_cast<RendererCLBase*>(m_Controller->Renderer()))
	{
		rendererCL->m_CompileBegun = [&]()
		{
			const QString status = "Compiling OpenCL kernel...";
			QMetaObject::invokeMethod(m_RenderStatusLabel, "setText", Qt::ConnectionType::QueuedConnection, Q_ARG(const QString&, status));
			//m_RenderStatusLabel->repaint();
			QApplication::processEvents();
		};
	}

	return ok;
}

/// <summary>
/// Create a new controller from the options.
/// This does not create the internal renderer or start the timers.
/// </summary>
/// <returns>True if successful, else false.</returns>
bool Fractorium::CreateControllerFromOptions()
{
	const size_t elementSize =
#ifdef DO_DOUBLE
		m_Settings->Double() ? sizeof(double) :
#endif
		sizeof(float);

	if (!m_Controller.get() || (m_Controller->SizeOfT() != elementSize))
	{
		const auto hue = m_PaletteHueSpin->value();
		const auto sat = m_PaletteSaturationSpin->value();
		const auto bright = m_PaletteBrightnessSpin->value();
		const auto con = m_PaletteContrastSpin->value();
		const auto blur = m_PaletteBlurSpin->value();
		const auto freq = m_PaletteFrequencySpin->value();
		const auto rot = m_PreviewPaletteRotation;
		double scale = 0;
		uint current = 0;
#ifdef DO_DOUBLE
		EmberFile<double> efd;
		Palette<double> tempPalette;
		Ember<double> xaosToggle;
#else
		EmberFile<float> efd;
		Palette<float> tempPalette;
		Ember<float> xaosToggle;
#endif
		const QModelIndex index = ui.LibraryTree->currentIndex();
		ui.LibraryTree->clear();//This must be here before FillLibraryTree() is called below, else a spurious EmberTreeItemChanged event will be called on a deleted object.

		//First check if a controller has already been created, and if so, save its embers and gracefully shut it down.
		if (m_Controller.get())
		{
			scale = m_Controller->LockedScale();
			m_Controller->StopAllPreviewRenderers();//Must stop any previews first, else changing controllers will crash the program and SaveCurrentToOpenedFile() will return 0.
			m_Controller->CopyTempPalette(tempPalette);//Convert float to double or save double verbatim;
			current = m_Controller->SaveCurrentToOpenedFile(false);
			//Replace below with this once LLVM fixes a crash in their compiler with default lambda parameters.//TODO
			//m_Controller->CopyEmberFile(efd);
			m_Controller->CopyXaosToggleEmber(xaosToggle);
#ifdef DO_DOUBLE
			m_Controller->CopyEmberFile(efd, false, [&](Ember<double>& ember) { });
#else
			m_Controller->CopyEmberFile(efd, false, [&](Ember<float>& ember) { });
#endif
			m_Controller->Shutdown();
		}

#ifdef DO_DOUBLE

		if (m_Settings->Double())
			m_Controller = unique_ptr<FractoriumEmberControllerBase>(new FractoriumEmberController<double>(this));
		else
#endif
			m_Controller = unique_ptr<FractoriumEmberControllerBase>(new FractoriumEmberController<float>(this));

		//Restore the ember and ember file.
		if (m_Controller.get())
		{
			if (auto prev = efd.Get(current))//Restore base temp palette. Adjustments will be then be applied and stored back in in m_Ember.m_Palette below.
				prev->m_Palette = tempPalette;

			m_Controller->SetEmberFile(efd, true);
			//Template specific palette table and variations tree setup in controller constructor, but
			//must manually setup the library tree here because it's after the embers were assigned.
			//Passing row re-selects the item that was previously selected.
			//This will eventually call FillParamTablesAndPalette(), which in addition to filling in various fields,
			//will apply the palette adjustments.
			m_Controller->FillLibraryTree(index.row());
			m_Controller->SetEmber(current, true);
			m_Controller->SetXaosToggleEmber(xaosToggle);
			m_Controller->LockedScale(scale);
			//Setting these and updating the GUI overwrites the work of clearing them done in SetEmber() above.
			//It's a corner case, but doesn't seem to matter.
			m_PaletteHueSpin->SetValueStealth(hue);
			m_PaletteSaturationSpin->SetValueStealth(sat);
			m_PaletteBrightnessSpin->SetValueStealth(bright);
			m_PaletteContrastSpin->SetValueStealth(con);
			m_PaletteBlurSpin->SetValueStealth(blur);
			m_PaletteFrequencySpin->SetValueStealth(freq);
			m_PreviewPaletteRotation = rot;
			m_Controller->PaletteAdjust();//Applies the adjustments to temp and saves in m_Ember.m_Palette, then fills in the palette preview widget.
		}

		return m_Controller.get();
	}

	return false;
}

/// <summary>
/// Start the render timer.
/// If a renderer has not been created yet, or differs form the options, it will first be created from the options.
/// </summary>
/// <param name="updatePreviews">True to re-render the library previews, else false.</param>
void Fractorium::StartRenderTimer(bool updatePreviews)
{
	//Starting the render timer, either for the first time
	//or from a paused state, such as resizing or applying new options.
	const auto newController = CreateControllerFromOptions();

	if (m_Controller.get())
	{
		//On program startup, the renderer does not get initialized until now.
		//If a new controller was created, then previews will have started, so only start the previews if a new controller
		//was *not* created and updatePreviews is true.
		CreateRendererFromOptions(updatePreviews && !newController);

		if (m_Controller->Renderer())
			m_Controller->StartRenderTimer();
	}
}

/// <summary>
/// Idle timer event which calls the controller's Render() function.
/// </summary>
void Fractorium::IdleTimer() { m_Controller->Render(); }

/// <summary>
/// Thin wrapper to determine if the controllers have been properly initialized.
/// </summary>
/// <returns>True if the ember controller and GL controllers are both not nullptr, else false.</returns>
bool Fractorium::ControllersOk() { return m_Controller.get() && m_Controller->GLController(); }

template class FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	template class FractoriumEmberController<double>;
#endif
