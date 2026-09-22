#include "FractoriumPch.h"
#include "GLWidget.h"
#include "Fractorium.h"

static const char* vertexShaderSource =
"attribute mediump vec4 posattr;\n"
"uniform mediump mat4 matrix;\n"
"uniform mediump float ps;\n"
"void main() {\n"
"   gl_Position = matrix * posattr;\n"
"	gl_PointSize = ps;\n"
"}\n";

static const char* fragmentShaderSource =
"uniform mediump vec4 mycolor;\n"
"void main() {\n"
"   gl_FragColor = mycolor;\n"
"}\n";

static const char* quadVertexShaderSource =
"attribute mediump vec4 posattr;\n"
"uniform mediump mat4 matrix;\n"
"varying mediump vec4 texcoord;\n"
"void main() {\n"
"	gl_Position = matrix * posattr;\n"
"	texcoord = posattr;\n"
"}\n";

static const char* quadFragmentShaderSource =
"uniform mediump sampler2D quadtex;\n"
"varying mediump vec4 texcoord;\n"
"void main() {\n"
"	gl_FragColor = texture2D(quadtex, texcoord.st);\n"
"}\n";

/// <summary>
/// Constructor which passes parent widget to the base and initializes OpenGL profile.
/// This will need to change in the future to implement all drawing as shader programs.
/// </summary>
/// <param name="p">The parent widget</param>
GLWidget::GLWidget(QWidget* p)
	: QOpenGLWidget(p)
{
}

/// <summary>
/// Empty destructor.
/// </summary>
GLWidget::~GLWidget()
{
}

/// <summary>
/// A manual initialization that must be called immediately after the main window is shown
/// and the virtual initializeGL() is called.
/// </summary>
void GLWidget::InitGL()
{
	if (!m_Init)
	{
		//auto qsf = this->format();
		//qDebug() << "InitGL*****************\nVersion: " << qsf.majorVersion() << ',' << qsf.minorVersion();
		//qDebug() << "Profile: " << qsf.profile();
		//qDebug() << "Depth buffer size: " << qsf.depthBufferSize();
		//qDebug() << "Swap behavior: " << qsf.swapBehavior();
		//qDebug() << "Swap interval: " << qsf.swapInterval();
		const auto w = std::ceil(m_Fractorium->ui.GLParentScrollArea->width() * devicePixelRatioF());
		const auto h = std::ceil(m_Fractorium->ui.GLParentScrollArea->height() * devicePixelRatioF());
		SetDimensions(w, h);
		//Start with either a flock of random embers, or the last flame from the previous run.
		//Can't do this until now because the window wasn't maximized yet, so the sizes would have been off.
		bool b = m_Fractorium->m_Settings->LoadLast();

		if (b)
		{
			auto path = GetDefaultUserPath();
			const QDir dir(path);
			const QString filename = path + "/lastonshutdown.flame";

			if (dir.exists(filename))
			{
				QStringList ql;
				ql << filename;
				m_Fractorium->m_Controller->OpenAndPrepFiles(ql, false);
			}
			else
				b = false;
		}

		if (!b)
		{
			m_Fractorium->m_WidthSpin->setValue(w);
			m_Fractorium->m_HeightSpin->setValue(h);
			m_Fractorium->OnActionNewFlock(false);//This must come after the previous two lines because it uses the values of the spinners.
		}

		m_Fractorium->m_Controller->DelayedStartRenderTimer();
		m_Init = true;
		/*
			auto clinfo = OpenCLInfo::DefInstance();
			auto& platforms = clinfo->Platforms();
			auto& alldevices = clinfo->Devices();
			std::vector<std::string> strs;
			auto cdc = wglGetCurrentDC();
			auto cc = wglGetCurrentContext();
			ostringstream os;
			strs.push_back(os.str()); os.str(""); os << "GLWidget::InitGL():";
			strs.push_back(os.str()); os.str(""); os << "\nCurrent DC: " << cdc;
			strs.push_back(os.str()); os.str(""); os << "\nCurrent Context: " << cc;

			for (int platform = 0; platform < platforms.size(); platform++)
			{
			cl_context_properties props[] =
			{
				CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
				CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
				CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>((platforms[platform])()),
				0
			};
			// Find CL capable devices in the current GL context
			//wglMakeCurrent(wglGetCurrentDC(), wglGetCurrentContext());
			::wglMakeCurrent(wglGetCurrentDC(), wglGetCurrentContext());
			size_t sizedev;
			cl_device_id devices[32];
			clGetGLContextInfoKHR_fn clGetGLContextInfo = (clGetGLContextInfoKHR_fn)clGetExtensionFunctionAddressForPlatform(platforms[platform](), "clGetGLContextInfoKHR");
			clGetGLContextInfo(props, CL_DEVICES_FOR_GL_CONTEXT_KHR, 32 * sizeof(cl_device_id), devices, &sizedev);
			sizedev = (cl_uint)(sizedev / sizeof(cl_device_id));

			for (int i = 0; i < sizedev; i++)
			{
				std::string s;
				size_t pi, di;
				auto dd = clinfo->DeviceFromId(devices[i], pi, di);

				if (dd)
				{
					auto& dev = *dd;
					auto& plat = platforms[pi];
					strs.push_back(os.str()); os.str(""); os << "\nPlatform[" << pi << "], device[" << di << "] is GL capable.";
					strs.push_back(os.str()); os.str(""); os << "\nPlatform profile: " << plat.getInfo<CL_PLATFORM_PROFILE>(nullptr).c_str() << endl;
					strs.push_back(os.str()); os.str(""); os << "\nPlatform version: " << plat.getInfo<CL_PLATFORM_VERSION>(nullptr).c_str() << endl;
					strs.push_back(os.str()); os.str(""); os << "\nPlatform name: " << plat.getInfo<CL_PLATFORM_NAME>(nullptr).c_str() << endl;
					strs.push_back(os.str()); os.str(""); os << "\nPlatform vendor: " << plat.getInfo<CL_PLATFORM_VENDOR>(nullptr).c_str() << endl;
					strs.push_back(os.str()); os.str(""); os << "\nPlatform extensions: " << plat.getInfo<CL_PLATFORM_EXTENSIONS>(nullptr).c_str() << endl;
					strs.push_back(os.str()); os.str(""); os << "\nVendor: " << dev.getInfo<CL_DEVICE_VENDOR>(nullptr).c_str() << endl;
					strs.push_back(os.str()); os.str(""); os << "\nDevice: " << dev.getInfo<CL_DEVICE_NAME>(nullptr).c_str() << endl;
					strs.push_back(os.str()); os.str(""); os << "\nDriver version: " << dev.getInfo<CL_DRIVER_VERSION>(nullptr).c_str() << endl;
					strs.push_back(os.str()); os.str(""); os << "\nDevice profile: " << dev.getInfo<CL_DEVICE_PROFILE>(nullptr).c_str() << endl;
					strs.push_back(os.str()); os.str(""); os << "\nDevice version: " << dev.getInfo<CL_DEVICE_VERSION>(nullptr).c_str() << endl;
					strs.push_back(os.str()); os.str(""); os << "\nDevice extensions: " << dev.getInfo<CL_DEVICE_EXTENSIONS>(nullptr).c_str() << endl;
					strs.push_back(os.str()); os.str(""); os << "\nDevice OpenCL C version: " << dev.getInfo<CL_DEVICE_OPENCL_C_VERSION>(nullptr).c_str() << endl;
				}
			}
			}

			m_Fractorium->ErrorReportToQTextEdit(strs, m_Fractorium->ui.InfoRenderingTextEdit);
		*/
	}
}

/// <summary>
/// Draw the final rendered image as a texture on a quad that is the same size as the window.
/// Different action is taken based on whether a CPU or OpenCL renderer is used.
/// For CPU, the output image buffer must be copied to OpenGL every time it's drawn.
/// For OpenCL, the output image and the texture are the same thing, so no copying is necessary
/// and all image memory remains on the card.
/// </summary>
void GLWidget::DrawQuad()
{
	this->glEnable(GL_TEXTURE_2D);
	this->glActiveTexture(GL_TEXTURE0);
	const auto renderer = m_Fractorium->m_Controller->Renderer();

	//Ensure all allocation has taken place first.
	if (m_OutputTexID != 0)
	{
		glBindTexture(GL_TEXTURE_2D, m_OutputTexID);//The texture to draw to.

		//Only draw if the dimensions match exactly.
		if (m_TexWidth == m_Fractorium->m_Controller->FinalRasW() && m_TexHeight == m_Fractorium->m_Controller->FinalRasH())
		{
			//Copy data from CPU to OpenGL if using a CPU renderer. This is not needed when using OpenCL.
			if (renderer->RendererType() == eRendererType::CPU_RENDERER || !renderer->Shared())
			{
				const auto finalImage = m_Fractorium->m_Controller->FinalImage();

				if (finalImage &&//Make absolutely sure all image dimensions match when copying host side buffer to GL texture.
						!finalImage->empty() &&
						((m_TexWidth * m_TexHeight) == static_cast<GLint>(finalImage->size())) &&
						(finalImage->size() == renderer->FinalDimensions()))
					this->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_TexWidth, m_TexHeight, GL_RGBA, GL_FLOAT, finalImage->data());
			}

			m_QuadProgram->bind();
			this->glVertexAttribPointer(m_TexturePosAttr, 2, GL_FLOAT, GL_FALSE, 0, m_TexVerts.data());
			this->glEnableVertexAttribArray(0);
			this->glDrawArrays(GL_TRIANGLE_STRIP, 0, 5);
			this->glDisableVertexAttribArray(0);
			m_QuadProgram->release();
		}
	}

	this->glBindTexture(GL_TEXTURE_2D, 0);//Stop using this texture.
	this->glDisable(GL_TEXTURE_2D);
}

/// <summary>
/// Set drag and drag modifier states to nothing.
/// </summary>
void GLEmberControllerBase::ClearDrag()
{
	m_DragModifier = 0;
	m_DragState = eDragState::DragNone;
}

/// <summary>
/// Wrapper around Allocate() call on the GL widget.
/// </summary>
bool GLEmberControllerBase::Allocate(bool force) { return m_GL->Allocate(force); }

/// <summary>
/// Helpers to set/get/clear which keys are pressed while dragging.
/// </summary>
/// <returns>bool</returns>
bool GLEmberControllerBase::GetAlt()       { return (m_DragModifier & static_cast<et>(eDragModifier::DragModAlt)) == static_cast<et>(eDragModifier::DragModAlt); }
bool GLEmberControllerBase::GetShift()     { return (m_DragModifier & static_cast<et>(eDragModifier::DragModShift)) == static_cast<et>(eDragModifier::DragModShift); }
bool GLEmberControllerBase::GetControl()   { return (m_DragModifier & static_cast<et>(eDragModifier::DragModControl)) == static_cast<et>(eDragModifier::DragModControl); }
void GLEmberControllerBase::SetAlt()       { m_DragModifier |= static_cast<et>(eDragModifier::DragModAlt); }
void GLEmberControllerBase::SetShift()     { m_DragModifier |= static_cast<et>(eDragModifier::DragModShift); }
void GLEmberControllerBase::SetControl()   { m_DragModifier |= static_cast<et>(eDragModifier::DragModControl); }
void GLEmberControllerBase::ClearAlt()     { m_DragModifier &= ~static_cast<et>(eDragModifier::DragModAlt); }
void GLEmberControllerBase::ClearShift()   { m_DragModifier &= ~static_cast<et>(eDragModifier::DragModShift); }
void GLEmberControllerBase::ClearControl() { m_DragModifier &= ~static_cast<et>(eDragModifier::DragModControl); }

/// <summary>
/// Clear the OpenGL output window to be the background color of the current ember.
/// Both buffers must be cleared, else artifacts will show up.
/// </summary>
template <typename T>
void GLEmberController<T>::ClearWindow()
{
	const auto ember = m_FractoriumEmberController->CurrentEmber();
	m_GL->makeCurrent();
	m_GL->glClearColor(ember->m_Background.r, ember->m_Background.g, ember->m_Background.b, 1.0);
	m_GL->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/// <summary>
/// Set the currently selected xform.
/// The currently selected xform is drawn with a circle around it, with all others only showing their axes.
/// </summary>
/// <param name="xform">The xform.</param>
template <typename T>
void GLEmberController<T>::SetSelectedXform(Xform<T>* xform)
{
	//By doing this check, it prevents triggering unnecessary events when selecting an xform on this window with the mouse,
	//which will set the combo box on the main window, which will trigger this call. However, if the xform has been selected
	//here with the mouse, the window has already been repainted, so there's no need to do it again.
	if (m_SelectedXform != xform)
	{
		m_SelectedXform = xform;

		if (m_GL->m_Init)
			//m_GL->update();
			m_GL->repaint();//Force immediate redraw with repaint() instead of update().
	}
}

/// <summary>
/// Setters for main window pointers.
/// </summary>

void GLWidget::SetMainWindow(Fractorium* f) { m_Fractorium = f; }

/// <summary>
/// Getters for OpenGL state.
/// </summary>

bool GLWidget::Init() const { return m_Init; }
bool GLWidget::Drawing() const { return m_Drawing; }
GLint GLWidget::MaxTexSize() const { return m_MaxTexSize; }
GLuint GLWidget::OutputTexID() const { return m_OutputTexID; }
GLint GLWidget::TexWidth() const { return m_TexWidth; }
GLint GLWidget::TexHeight() const { return m_TexHeight; }

/// <summary>
/// Initialize OpenGL, called once at startup after the main window constructor finishes.
/// Although it seems an awkward place to put some of this setup code, the dimensions of the
/// main window and its widgets are not fully initialized before this is called.
/// Once this is done, the render timer is started after a short delay.
/// Rendering is then clear to begin.
/// </summary>
void GLWidget::initializeGL()
{
	//auto  qsf = this->format();
	//qDebug() << "initializeGL*****************\nVersion: " << qsf.majorVersion() << ',' << qsf.minorVersion();
	//qDebug() << "Profile: " << qsf.profile();
	//qDebug() << "Depth buffer size: " << qsf.depthBufferSize();
	//qDebug() << "Swap behavior: " << qsf.swapBehavior();
	//qDebug() << "Swap interval: " << qsf.swapInterval();

	if (!m_Init && m_Fractorium)
	{
		this->initializeOpenGLFunctions();

		if (!m_Program)
		{
			m_Program = new QOpenGLShaderProgram(this);

			if (!m_Program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource))
			{
				QMessageBox::critical(m_Fractorium, "Shader Error", "Error compiling affine vertex source: " + m_Program->log());
				QApplication::exit(1);
			}

			if (!m_Program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource))
			{
				QMessageBox::critical(m_Fractorium, "Shader Error", "Error compiling affine fragment source: " + m_Program->log());
				QApplication::exit(1);
			}

			if (!m_Program->link())
			{
				QMessageBox::critical(m_Fractorium, "Shader Error", "Error linking affine source: " + m_Program->log());
				QApplication::exit(1);
			}

			m_PosAttr = m_Program->attributeLocation("posattr");
			m_ColAttr = m_Program->uniformLocation("mycolor");
			m_PointSizeUniform = m_Program->uniformLocation("ps");
			m_MatrixUniform = m_Program->uniformLocation("matrix");
		}

		if (!m_QuadProgram)
		{
			m_QuadProgram = new QOpenGLShaderProgram(this);

			if (!m_QuadProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, quadVertexShaderSource))
			{
				QMessageBox::critical(m_Fractorium, "Shader Error", "Error compiling image texture vertex source: " + m_QuadProgram->log());
				QApplication::exit(1);
			}

			if (!m_QuadProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, quadFragmentShaderSource))
			{
				QMessageBox::critical(m_Fractorium, "Shader Error", "Error compiling image texture fragment source: " + m_QuadProgram->log());
				QApplication::exit(1);
			}

			if (!m_QuadProgram->link())
			{
				QMessageBox::critical(m_Fractorium, "Shader Error", "Error linking image texture source: " + m_QuadProgram->log());
				QApplication::exit(1);
			}

			m_TexturePosAttr = m_QuadProgram->attributeLocation("posattr");
			m_TextureUniform = m_QuadProgram->uniformLocation("quadtex");
			m_TextureMatrixUniform = m_QuadProgram->uniformLocation("matrix");
			m_TextureProjMatrix.ortho(0, 1, 1, 0, -1, 1);
			m_QuadProgram->bind();
			m_QuadProgram->setUniformValue(m_TextureUniform, 0);
			m_QuadProgram->setUniformValue(m_TextureMatrixUniform, m_TextureProjMatrix);
			m_QuadProgram->release();
		}

		//cout << "GL Version: " << (char *) glGetString(GL_VERSION) << endl;
		//cout << "GLSL version: " << (char *) glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
		this->glClearColor(0.0, 0.0, 0.0, 1.0);
		this->glDisable(GL_DEPTH_TEST);//This will remain disabled for the duration of the program.
		this->glEnable(GL_TEXTURE_2D);
		this->glEnable(GL_PROGRAM_POINT_SIZE);
		this->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_MaxTexSize);
		this->glDisable(GL_TEXTURE_2D);
		m_Fractorium->m_WidthSpin->setMaximum(m_MaxTexSize);
		m_Fractorium->m_HeightSpin->setMaximum(m_MaxTexSize);
	}
}

/// <summary>
/// The main drawing/update function.
/// First the quad will be drawn, then the remaining affine circles.
/// </summary>
void GLWidget::paintGL()
{
	/*
		auto  qsf = this->format();
		qDebug() << "paintGL*****************\nVersion: " << qsf.majorVersion() << ',' << qsf.minorVersion();
		qDebug() << "Profile: " << qsf.profile();
		qDebug() << "Depth buffer size: " << qsf.depthBufferSize();
		qDebug() << "Swap behavior: " << qsf.swapBehavior();
		qDebug() << "Swap interval: " << qsf.swapInterval();
	*/
	const auto controller = m_Fractorium->m_Controller.get();

	//Ensure there is a renderer and that it's supposed to be drawing, signified by the running timer.
	if (controller && controller->Renderer()/* && controller->ProcessState() != eProcessState::NONE*/)//Need a way to determine if at leat one successful render has happened.
	{
		const auto renderer = controller->Renderer();
		const auto unitX = std::abs(renderer->UpperRightX(false) - renderer->LowerLeftX(false)) / 2.0f;
		const auto unitY = std::abs(renderer->UpperRightY(false) - renderer->LowerLeftY(false)) / 2.0f;

		if (unitX > 100000 || unitY > 100000)//Need a better way to do this.//TODO
		{
			qDebug() << unitX << " " << unitY;
			//return;
		}

		m_Drawing = true;

		if (m_Fractorium->DrawImage())
		{
			GLController()->DrawImage();
		}
		else
		{
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		//Affine drawing.
		const auto pre = m_Fractorium->DrawPreAffines();
		const auto post = m_Fractorium->DrawPostAffines();
		this->glEnable(GL_BLEND);
		this->glEnable(GL_LINE_SMOOTH);
		this->glEnable(GL_POINT_SMOOTH);
#if defined (__APPLE__) || defined(MACOSX)
		this->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_COLOR);
#else
		this->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
#endif
		m_Program->bind();
		m_ProjMatrix.setToIdentity();
		m_ProjMatrix.ortho(-unitX, unitX, -unitY, unitY, -1, 1);//Projection matrix: OpenGL camera is always centered, just move the ember internally inside the renderer.
		m_ModelViewMatrix.setToIdentity();
		//this->DrawUnitSquare();
		controller->GLController()->DrawAffines(pre, post);
		m_Program->release();
		this->glDisable(GL_BLEND);
		this->glDisable(GL_LINE_SMOOTH);
		this->glDisable(GL_POINT_SMOOTH);
		m_Drawing = false;
	}
}

/// <summary>
/// Draw the image on the quad.
/// </summary>
template <typename T>
void GLEmberController<T>::DrawImage()
{
	const auto renderer = m_FractoriumEmberController->Renderer();
	const auto ember = m_FractoriumEmberController->CurrentEmber();
	m_GL->glClearColor(ember->m_Background.r, ember->m_Background.g, ember->m_Background.b, 1.0);
	m_GL->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderer->EnterFinalAccum();//Lock, may not be necessary, but just in case.
	renderer->EnterResize();

	if (SizesMatch())//Ensure all sizes are correct. If not, do nothing.
	{
		m_GL->DrawQuad();//Output image is drawn here.
	}

	renderer->LeaveResize();//Unlock, may not be necessary.
	renderer->LeaveFinalAccum();
}

/// <summary>
/// Draw the affine circles.
/// </summary>
/// <param name="pre">True to draw pre affines, else don't.</param>
/// <param name="post">True to draw post affines, else don't.</param>
template <typename T>
void GLEmberController<T>::DrawAffines(bool pre, bool post)
{
	QueryVMP();//Resolves to float or double specialization function depending on T.

	if (!m_Fractorium->DrawXforms())
		return;

	const auto ember = m_FractoriumEmberController->CurrentEmber();
	const auto dragging = m_DragState == eDragState::DragDragging;
	const auto forceFinal = m_Fractorium->HaveFinal();

	if (m_DragState == eDragState::DragRotateScale)
	{
		const auto dprf = m_GL->devicePixelRatioF();
		const auto world = ScrolledCenter(true);
		m_GL->glLineWidth(1.0f * dprf);
		const GLfloat vertices[] =
		{
			static_cast<GLfloat>(m_MouseWorldPos.x), static_cast<GLfloat>(m_MouseWorldPos.y),//Mouse position while dragging with right button down...
			static_cast<GLfloat>(world.x), static_cast<GLfloat>(world.y)//...to center.
		};
		const QVector4D col(0.0f, 1.0f, 1.0f, 1.0f);
		m_GL->DrawPointOrLine(col, vertices, 2, GL_LINES);
	}

	//Draw grid if control key is pressed.
	if ((m_GL->hasFocus() && GetControl()) || m_Fractorium->DrawGrid())
		DrawGrid();

	//When dragging, only draw the selected xform's affine and hide all others.
	if (!m_Fractorium->m_Settings->ShowAllXforms() && dragging)
	{
		if (m_SelectedXform)
			DrawAffine(m_SelectedXform, m_AffineType == eAffineType::AffinePre, true, false);
	}
	else//Show all while dragging, or not dragging just hovering/mouse move.
	{
		if (pre && m_Fractorium->DrawAllPre())//Draw all pre affine if specified.
		{
			size_t i = 0;
			bool any = false;

			while (auto xform = ember->GetTotalXform(i, forceFinal))
				if (m_Fractorium->IsXformSelected(i++))
				{
					any = true;
					break;
				}

			i = 0;

			while (const auto xform = ember->GetTotalXform(i, forceFinal))
			{
				bool selected = m_Fractorium->IsXformSelected(i++) || (!any && m_SelectedXform == xform);
				DrawAffine(xform, true, selected, !dragging && (m_HoverXform == xform));
			}
		}
		else if (pre && m_Fractorium->DrawSelectedPre())//Only draw selected pre affine, and if none are selected, draw current. All are considered "selected", so circles are drawn around them.
		{
			size_t i = 0;
			bool any = false;

			while (const auto xform = ember->GetTotalXform(i, forceFinal))
			{
				if (m_Fractorium->IsXformSelected(i++))
				{
					DrawAffine(xform, true, true, !dragging && (m_HoverXform == xform));
					any = true;
				}
			}

			if (!any)
				DrawAffine(m_FractoriumEmberController->CurrentXform(), true, true, !dragging && (m_HoverXform == m_FractoriumEmberController->CurrentXform()));
		}

		if (post && m_Fractorium->DrawAllPost())//Draw all post affine if specified.
		{
			size_t i = 0;
			bool any = false;

			while (const auto xform = ember->GetTotalXform(i, forceFinal))
				if (m_Fractorium->IsXformSelected(i++))
				{
					any = true;
					break;
				}

			i = 0;

			while (const auto xform = ember->GetTotalXform(i, forceFinal))
			{
				bool selected = m_Fractorium->IsXformSelected(i++) || (!any && m_SelectedXform == xform);
				DrawAffine(xform, false, selected, !dragging && (m_HoverXform == xform));
			}
		}
		else if (post && m_Fractorium->DrawSelectedPost())//Only draw selected post, and if none are selected, draw current. All are considered "selected", so circles are drawn around them.
		{
			size_t i = 0;
			bool any = false;

			while (const auto xform = ember->GetTotalXform(i, forceFinal))
			{
				if (m_Fractorium->IsXformSelected(i++))
				{
					DrawAffine(xform, false, true, !dragging && (m_HoverXform == xform));
					any = true;
				}
			}

			if (!any)
				DrawAffine(m_FractoriumEmberController->CurrentXform(), false, true, !dragging && (m_HoverXform == m_FractoriumEmberController->CurrentXform()));
		}
	}

	if (dragging)//Draw large yellow dot on select or drag.
	{
		const GLfloat vertices[] =//Should these be of type T?//TODO
		{
			static_cast<GLfloat>(m_DragHandlePos.x), static_cast<GLfloat>(m_DragHandlePos.y)
		};
		const QVector4D col(1.0f, 1.0f, 0.5f, 1.0f);
		m_GL->DrawPointOrLine(col, vertices, 1, GL_POINTS, false, 6.0f);
	}
	else if (m_DragState == eDragState::DragSelect)
	{
		m_GL->glLineWidth(2.0f * m_GL->devicePixelRatioF());
		const GLfloat vertices[] =//Should these be of type T?//TODO
		{
			static_cast<GLfloat>(m_MouseDownWorldPos.x), static_cast<GLfloat>(m_MouseDownWorldPos.y),//UL->UR
			static_cast<GLfloat>(m_MouseWorldPos.x    ), static_cast<GLfloat>(m_MouseDownWorldPos.y),
			static_cast<GLfloat>(m_MouseDownWorldPos.x), static_cast<GLfloat>(m_MouseWorldPos.y),//LL->LR
			static_cast<GLfloat>(m_MouseWorldPos.x    ), static_cast<GLfloat>(m_MouseWorldPos.y),
			static_cast<GLfloat>(m_MouseDownWorldPos.x), static_cast<GLfloat>(m_MouseDownWorldPos.y),//UL->LL
			static_cast<GLfloat>(m_MouseDownWorldPos.x), static_cast<GLfloat>(m_MouseWorldPos.y),
			static_cast<GLfloat>(m_MouseWorldPos.x    ), static_cast<GLfloat>(m_MouseDownWorldPos.y),//UR->LR
			static_cast<GLfloat>(m_MouseWorldPos.x    ), static_cast<GLfloat>(m_MouseWorldPos.y)
		};
		const QVector4D col(0.0f, 0.0f, 1.0f, 1.0f);
		m_GL->DrawPointOrLine(col, vertices, 8, GL_LINES);
		m_GL->glLineWidth(1.0f * m_GL->devicePixelRatioF());
	}
	else if (m_HoverType != eHoverType::HoverNone && m_HoverXform == m_SelectedXform)//Draw large turquoise dot on hover if they are hovering over the selected xform.
	{
		const GLfloat vertices[] =//Should these be of type T?//TODO
		{
			static_cast<GLfloat>(m_HoverHandlePos.x), static_cast<GLfloat>(m_HoverHandlePos.y)
		};
		const QVector4D col(0.5f, 1.0f, 1.0f, 1.0f);
		m_GL->DrawPointOrLine(col, vertices, 1, GL_POINTS, false, 6.0f);
	}
}

/// <summary>
/// Set drag modifiers based on key press.
/// </summary>
/// <param name="e">The event</param>
bool GLEmberControllerBase::KeyPress_(QKeyEvent* e)
{
	if (e->key() == Qt::Key_Control)
	{
		SetControl();
		return true;
	}

	return false;
}

/// <summary>
/// Call controller KeyPress().
/// </summary>
/// <param name="e">The event</param>
void GLWidget::keyPressEvent(QKeyEvent* e)
{
	if (!GLController() || !GLController()->KeyPress_(e))
		QOpenGLWidget::keyPressEvent(e);

	update();
}

/// <summary>
/// Set drag modifiers based on key release.
/// </summary>
/// <param name="e">The event</param>
bool GLEmberControllerBase::KeyRelease_(QKeyEvent* e)
{
	if (e != nullptr && e->key() == Qt::Key_Control)
	{
		ClearControl();
		return true;
	}

	return false;
}

/// <summary>
/// Call controller KeyRelease_().
/// </summary>
/// <param name="e">The event</param>
void GLWidget::keyReleaseEvent(QKeyEvent* e)
{
	if (e)
	{
		if (!GLController() || !GLController()->KeyRelease_(e))
			QOpenGLWidget::keyReleaseEvent(e);

		update();
	}
}

/// <summary>
/// Determine if the mouse click was over an affine circle
/// and set the appropriate selection information to be used
/// on subsequent mouse move events.
/// If nothing was selected, then reset the selection and drag states.
/// </summary>
/// <param name="e">The event</param>
template <typename T>
void GLEmberController<T>::MousePress(QMouseEvent* e)
{
	if (!e)
		return;

	const auto x = e->position().x();
	const auto y = e->position().y();
	v3T const mouseFlipped(x * m_GL->devicePixelRatioF(), m_Viewport[3] - y * m_GL->devicePixelRatioF(), 0);//Must flip y because in OpenGL, 0,0 is bottom left, but in windows, it's top left.
	const auto ember = m_FractoriumEmberController->CurrentEmber();
	const auto renderer = m_FractoriumEmberController->Renderer();

	//Ensure everything has been initialized.
	if (!renderer)
		return;

	m_MouseDownPos = glm::ivec2(x * m_GL->devicePixelRatioF(), y * m_GL->devicePixelRatioF());//Capture the raster coordinates of where the mouse was clicked.
	m_MouseWorldPos = WindowToWorld(mouseFlipped, false);//Capture the world cartesian coordinates of where the mouse is.
	m_BoundsDown.w = renderer->LowerLeftX(false);//Need to capture these because they'll be changing if scaling.
	m_BoundsDown.x = renderer->LowerLeftY(false);
	m_BoundsDown.y = renderer->UpperRightX(false);
	m_BoundsDown.z = renderer->UpperRightY(false);
	const auto mod = e->modifiers();

	if (mod.testFlag(Qt::ShiftModifier))
		SetShift();

	if (mod.testFlag(Qt::AltModifier))
		SetAlt();

	if (m_DragState == eDragState::DragNone)//Only take action if the user wasn't already dragging.
	{
		m_MouseDownWorldPos = m_MouseWorldPos;//Set the mouse down position to the current position.

		if (e->button() & Qt::LeftButton)
		{
			const auto xformIndex = UpdateHover(mouseFlipped);//Determine if an affine circle was clicked.

			if (m_HoverXform && xformIndex != -1)
			{
				m_SelectedXform = m_HoverXform;
				m_DragSrcTransform = Affine2D<T>(m_AffineType == eAffineType::AffinePre ? m_SelectedXform->m_Affine : m_SelectedXform->m_Post);//Copy the affine of the xform that was selected.
				//The user has selected an xform by clicking on it, so update the main GUI by selecting this xform in the combo box.
				m_Fractorium->CurrentXform(xformIndex);//Must do this first so UpdateXform() below properly grabs the current plus any selected.
				m_DragSrcPreTransforms.clear();
				m_DragSrcPostTransforms.clear();
				m_FractoriumEmberController->UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex)
				{
					if (m_AffineType == eAffineType::AffinePre)
						m_DragSrcPreTransforms[xfindex] = xform->m_Affine;
					else
						m_DragSrcPostTransforms[xfindex] = xform->m_Post;
				}, eXformUpdate::UPDATE_SELECTED, false);//Don't update renderer here.
				m_DragHandlePos = m_HoverHandlePos;//The location in local coordinates of the point selected on the spinner, x, y or center.
				m_DragState = eDragState::DragDragging;
				m_GL->repaint();
			}
			else//Nothing was selected.
			{
				//m_SelectedXform = nullptr;
				m_DragSrcPreTransforms.clear();
				m_DragSrcPostTransforms.clear();
				m_DragState = eDragState::DragNone;
			}
		}
		else if (e->button() == Qt::MiddleButton || (e->button() == Qt::RightButton && e->modifiers() & Qt::ShiftModifier && !(e->modifiers() & Qt::AltModifier)))//Middle button or right button with shift key, do whole image translation.
		{
			m_CenterDownX = ember->m_CenterX;//Capture where the center of the image is because this value will change when panning.
			m_CenterDownY = ember->m_CenterY;
			m_DragState = eDragState::DragPanning;
		}
		else if (e->button() == Qt::RightButton)//Right button does whole image rotation and scaling.
		{
			if (m_Fractorium->DrawImage())
			{
				m_CenterDownX = ember->m_CenterX;//Capture these because they will change when rotating and scaling.
				m_CenterDownY = ember->m_CenterY;

				if (GetAlt() && GetShift())
				{
					m_PitchDown = ember->m_CamPitch * RAD_2_DEG_T;
					m_YawDown = ember->m_CamYaw * RAD_2_DEG_T;
					m_DragState = eDragState::DragPitchYaw;
				}
				else
				{
					m_RotationDown = ember->m_Rotate;
					m_ScaleDown = ember->m_PixelsPerUnit;
					m_DragState = eDragState::DragRotateScale;
				}
			}
		}
	}
}

/// <summary>
/// Call controller MousePress().
/// </summary>
/// <param name="e">The event</param>
void GLWidget::mousePressEvent(QMouseEvent* e)
{
	if (e)
	{
		setFocus();//Must do this so that this window gets keyboard events.

		if (const auto controller = GLController())
			controller->MousePress(e);

		QOpenGLWidget::mousePressEvent(e);
	}
}

/// <summary>
/// Reset the selection and dragging state, but re-calculate the
/// hovering state because the mouse might still be over an affine circle.
/// </summary>
/// <param name="e">The event</param>
template <typename T>
void GLEmberController<T>::MouseRelease(QMouseEvent* e)
{
	if (e)
	{
		const auto x = e->position().x();
		const auto y = e->position().y();
		v3T const mouseFlipped(x * m_GL->devicePixelRatioF(), m_Viewport[3] - y * m_GL->devicePixelRatioF(), 0);//Must flip y because in OpenGL, 0,0 is bottom left, but in windows, it's top left.
		m_MouseWorldPos = WindowToWorld(mouseFlipped, false);

		if (m_DragState == eDragState::DragDragging && (e->button() & Qt::LeftButton))
			UpdateHover(mouseFlipped);

		if (m_DragState == eDragState::DragNone)
			m_Fractorium->OnXformsSelectNoneButtonClicked(false);

		m_DragState = eDragState::DragNone;
		m_DragModifier = 0;
		m_GL->update();
	}
}

/// <summary>
/// Call controller MouseRelease().
/// </summary>
/// <param name="e">The event</param>
void GLWidget::mouseReleaseEvent(QMouseEvent* e)
{
	setFocus();//Must do this so that this window gets keyboard events.

	if (const auto controller = GLController())
		controller->MouseRelease(e);

	QOpenGLWidget::mouseReleaseEvent(e);
}

/// <summary>
/// If dragging, update relevant values and reset entire rendering process.
/// If hovering, update display.
/// </summary>
/// <param name="e">The event</param>
template <typename T>
void GLEmberController<T>::MouseMove(QMouseEvent* e)
{
	const auto draw = true;
	const auto x = e->position().x();
	const auto y = e->position().y();
	const glm::ivec2 mouse(x * m_GL->devicePixelRatioF(), y * m_GL->devicePixelRatioF());
	const v3T mouseFlipped(x * m_GL->devicePixelRatioF(), m_Viewport[3] - y * m_GL->devicePixelRatioF(), 0);//Must flip y because in OpenGL, 0,0 is bottom left, but in windows, it's top left.
	const auto ember = m_FractoriumEmberController->CurrentEmber();

	//First check to see if the mouse actually moved.
	if (mouse == m_MousePos)
		return;

	m_MousePos = mouse;
	m_MouseWorldPos = WindowToWorld(mouseFlipped, false);

	//Update status bar on main window, regardless of whether anything is being dragged.
	if (m_Fractorium->m_Controller->RenderTimerRunning())
		m_Fractorium->SetCoordinateStatus(x * m_GL->devicePixelRatioF(), y * m_GL->devicePixelRatioF(), m_MouseWorldPos.x, m_MouseWorldPos.y);

	if (m_SelectedXform && m_DragState == eDragState::DragDragging)//Dragging and affine.
	{
		const bool pre = m_AffineType == eAffineType::AffinePre;

		if (m_HoverType == eHoverType::HoverTranslation)
			CalcDragTranslation();
		else if (m_HoverType == eHoverType::HoverXAxis)
			CalcDragXAxis();
		else if (m_HoverType == eHoverType::HoverYAxis)
			CalcDragYAxis();

		m_FractoriumEmberController->FillAffineWithXform(m_SelectedXform, pre);//Update the spinners in the affine tab of the main window.
		m_FractoriumEmberController->UpdateRender();//Restart the rendering process.
	}
	else if ((m_DragState == eDragState::DragNone || m_DragState == eDragState::DragSelect) && (e->buttons() & Qt::LeftButton))
	{
		m_DragState = eDragState::DragSelect;//Only set drag state once the user starts moving the mouse with the left button down.
		//Iterate over each xform, seeing if it's in the bounding box.
		const QPointF tl(m_MouseDownWorldPos.x, m_MouseDownWorldPos.y);
		const QPointF br(m_MouseWorldPos.x, m_MouseWorldPos.y);
		const QRectF qrf(tl, br);
		const T scale = m_FractoriumEmberController->AffineScaleCurrentToLocked();

		m_FractoriumEmberController->UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex)
		{
			if (m_Fractorium->DrawAllPre() || xform == m_SelectedXform)//Draw all pre affine if specified.
			{
				const QPointF cd(xform->m_Affine.C() * scale, xform->m_Affine.F() * scale);
				bool b = qrf.contains(cd);
				m_FractoriumEmberController->XformCheckboxAt(static_cast<int>(xfindex), [&](QCheckBox * cb)
				{
					cb->setChecked(b);
				});
			}

			if (m_Fractorium->DrawAllPost() || xform == m_SelectedXform)
			{
				const QPointF cd(xform->m_Post.C() * scale, xform->m_Post.F() * scale);
				bool b = qrf.contains(cd);
				m_FractoriumEmberController->XformCheckboxAt(static_cast<int>(xfindex), [&](QCheckBox * cb)
				{
					if (!cb->isChecked() && b)
						cb->setChecked(b);
				});
			}
		}, eXformUpdate::UPDATE_ALL, false);
	}
	else if (m_DragState == eDragState::DragPanning)//Translating the whole image.
	{
		const auto x = -(m_MouseWorldPos.x - m_MouseDownWorldPos.x);
		const auto y = (m_MouseWorldPos.y - m_MouseDownWorldPos.y);
		Affine2D<T> rotMat;
		rotMat.C(m_CenterDownX);
		rotMat.F(m_CenterDownY);
		rotMat.Rotate(ember->m_Rotate * DEG_2_RAD_T);
		const v2T v1(x, y);
		const v2T v2 = rotMat.TransformVector(v1);
		ember->m_CenterX = v2.x;
		ember->m_CenterY = ember->m_RotCenterY = v2.y;
		m_FractoriumEmberController->SetCenter(ember->m_CenterX, ember->m_CenterY);//Will restart the rendering process.
	}
	else if (m_DragState == eDragState::DragRotateScale)//Rotating and scaling the whole image.
	{
		const T rot = CalcRotation();
		const T scale = CalcScale();
		ember->m_Rotate = NormalizeDeg360<T>(m_RotationDown + rot);
		m_Fractorium->SetRotation(ember->m_Rotate, true);
		m_Fractorium->SetScale(std::max(static_cast<T>(10), m_ScaleDown + scale));//Will restart the rendering process.
	}
	else if (m_DragState == eDragState::DragPitchYaw)//Pitching and yawing the whole image.
	{
		T pitch;
		T yaw;
		auto rotate = ember->m_Rotate;

		if ((rotate <= 45 && rotate >= -45) || (rotate >= 135) || (rotate <= -135))
		{
			pitch = m_PitchDown + (m_MouseWorldPos.y - m_MouseDownWorldPos.y) * 100;
			yaw   = m_YawDown + (m_MouseWorldPos.x - m_MouseDownWorldPos.x) * 100;
		}
		else
		{
			pitch = m_PitchDown + (m_MouseWorldPos.x - m_MouseDownWorldPos.x) * 100;
			yaw   = m_YawDown + (m_MouseWorldPos.y - m_MouseDownWorldPos.y) * 100;
		}

		m_Fractorium->SetPitch(pitch);
		m_Fractorium->SetYaw(yaw);
	}
	else
	{
		//If the user doesn't already have a key down, and they aren't dragging, clear the keys to be safe.
		//This is done because if they do an alt+tab between windows, it thinks the alt key is down.
		if (e->modifiers() == Qt::NoModifier)
			ClearDrag();

		//Check if they weren't dragging and weren't hovering over any affine.
		//In that case, nothing needs to be done.
		if (UpdateHover(mouseFlipped) == -1)
		{
			m_HoverXform = nullptr;
		}
	}

	//Only update if the user was dragging or hovered over a point.
	//Use repaint() to update immediately for a more responsive feel.
	if ((m_DragState != eDragState::DragNone) || draw)
		m_GL->update();
}

/// <summary>
/// Call controller MouseMove().
/// </summary>
/// <param name="e">The event</param>
void GLWidget::mouseMoveEvent(QMouseEvent* e)
{
	setFocus();//Must do this so that this window gets keyboard events.

	if (const auto controller = GLController())
		controller->MouseMove(e);

	QOpenGLWidget::mouseMoveEvent(e);
}

/// <summary>
/// Mouse wheel changes the scale (pixels per unit) which
/// will zoom in the image in our out, while sacrificing quality.
/// If the user needs to preserve quality, they can use the zoom spinner
/// on the main window.
/// If Alt is pressed, only the scale of the affines is changed, the scale of the image remains untouched.
/// </summary>
/// <param name="e">The event</param>
template <typename T>
void GLEmberController<T>::Wheel(QWheelEvent* e)
{
	if ((e->modifiers() & Qt::AltModifier) && m_Fractorium->DrawXforms())
	{
#ifdef __APPLE__
		m_FractoriumEmberController->ChangeLockedScale(e->angleDelta().y() >= 0 ? 1.0981 : 0.9);
#else
		m_FractoriumEmberController->ChangeLockedScale(e->angleDelta().x() >= 0 ? 1.0981 : 0.9);
#endif
		m_GL->update();
	}
	else
	{
		if (m_Fractorium->DrawImage() && !(e->buttons() & Qt::MiddleButton) && !(e->modifiers() & Qt::ShiftModifier))//Middle button does whole image translation, so ignore the mouse wheel while panning to avoid inadvertent zooming. ShiftModifier for sensitive mouse.
		{
			auto ember = m_FractoriumEmberController->CurrentEmber();
			m_Fractorium->SetScale(ember->m_PixelsPerUnit + (e->angleDelta().y() >= 0 ? 50 : -50));
		}
	}
}

/// <summary>
/// Call controller Wheel().
/// </summary>
/// <param name="e">The event</param>
void GLWidget::wheelEvent(QWheelEvent* e)
{
	if (e)
	{
		if (auto controller = GLController())
		{
			controller->Wheel(e);
			e->accept();//Prevents it from being sent to the main scroll bars. Scrolling should only affect the scale parameter and affine display zooming.
		}
	}

	//Do not call QOpenGLWidget::wheelEvent(e) because this should only affect the scale and not the position of the scroll bars.
}

/// <summary>
/// Wrapper around drawing a simple primitive, like a point or line, using a GLSL program.
/// </summary>
/// <param name="col">The color to draw with</param>
/// <param name="vertices">The vertices to use</param>
/// <param name="drawType">The type of primitive to draw, such as GL_POINT or GL_LINES</param>
/// <param name="dashed">True to draw dashed lines, else solid</param>
/// <param name="pointSize">The size in pixels of points, which is internally scaled by the device pixel ratio.</param>
void GLWidget::DrawPointOrLine(const QVector4D& col, const std::vector<float>& vertices, int drawType, bool dashed, GLfloat pointSize)
{
	DrawPointOrLine(col, vertices.data(), int(vertices.size() / 2), drawType, dashed, pointSize);
}

/// <summary>
/// Wrapper around drawing a simple primitive, like a point or line, using a GLSL program.
/// </summary>
/// <param name="col">The color to draw with</param>
/// <param name="vertices">The vertices to use</param>
/// <param name="size">The number of verticies. This is usually the size of vertices / 2.</param>
/// <param name="drawType">The type of primitive to draw, such as GL_POINT or GL_LINES</param>
/// <param name="dashed">True to draw dashed lines, else solid</param>
/// <param name="pointSize">The size in pixels of points, which is internally scaled by the device pixel ratio.</param>
void GLWidget::DrawPointOrLine(const QVector4D& col, const GLfloat* vertices, int size, int drawType, bool dashed, GLfloat pointSize)
{
	if (dashed && (drawType == GL_LINES || drawType == GL_LINE_LOOP))
	{
		glLineStipple(1, 0XFF00);
		glEnable(GL_LINE_STIPPLE);
	}

	m_ModelViewProjectionMatrix = m_ProjMatrix * m_ModelViewMatrix;
	m_Program->setUniformValue(m_ColAttr, col);
	m_Program->setUniformValue(m_PointSizeUniform, pointSize * GLfloat(devicePixelRatioF()));
	m_Program->setUniformValue(m_MatrixUniform, m_ModelViewProjectionMatrix);
	this->glVertexAttribPointer(m_PosAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	this->glEnableVertexAttribArray(0);
	this->glDrawArrays(drawType, 0, size);
	this->glDisableVertexAttribArray(0);

	if (dashed && (drawType == GL_LINES || drawType == GL_LINE_LOOP))
		glDisable(GL_LINE_STIPPLE);
}

/// <summary>
/// Set the dimensions of the drawing area.
/// This will be called from the main window's SyncSizes() function.
/// </summary>
/// <param name="w">Width in pixels</param>
/// <param name="h">Height in pixels</param>
void GLWidget::SetDimensions(int w, int h)
{
	const auto downscaledW = std::ceil(w / devicePixelRatioF());
	const auto downscaledH = std::ceil(h / devicePixelRatioF());
	setFixedSize(downscaledW, downscaledH);
}

/// <summary>
/// Set up texture memory to match the size of the window.
/// If first allocation, generate, bind and set parameters.
/// If subsequent call, only take action if dimensions don't match the window. In such case,
/// first deallocate, then reallocate.
/// </summary>
/// <returns>True if success, else false.</returns>
bool GLWidget::Allocate(bool force)
{
	bool alloc = false;
	//auto scaledW = std::ceil(width() * devicePixelRatioF());
	const auto w = m_Fractorium->m_Controller->FinalRasW();
	const auto h = m_Fractorium->m_Controller->FinalRasH();
	bool const doResize = force || m_TexWidth != w || m_TexHeight != h;
	bool const doIt = doResize || m_OutputTexID == 0;
	if (doIt)
	{
		m_TexWidth = static_cast<GLint>(w);
		m_TexHeight = static_cast<GLint>(h);
		this->glEnable(GL_TEXTURE_2D);

		if (doResize)
			Deallocate();

		this->glActiveTexture(GL_TEXTURE0);
		this->glGenTextures(1, &m_OutputTexID);
		this->glBindTexture(GL_TEXTURE_2D, m_OutputTexID);
		this->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Fractron had this as GL_LINEAR_MIPMAP_LINEAR for OpenCL and Cuda.
		this->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		this->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		this->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#if defined (__APPLE__) || defined(MACOSX)
		this->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_TexWidth, m_TexHeight, 0, GL_RGB, GL_FLOAT, nullptr);
#else
		this->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_TexWidth, m_TexHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
#endif
		alloc = true;
	}

	if (alloc)
	{
		this->glBindTexture(GL_TEXTURE_2D, 0);
		this->glDisable(GL_TEXTURE_2D);
	}

	this->glFinish();
	return m_OutputTexID != 0;
}

/// <summary>
/// Deallocate texture memory.
/// </summary>
/// <returns>True if anything deleted, else false.</returns>
bool GLWidget::Deallocate()
{
	bool deleted = false;

	if (m_OutputTexID != 0)
	{
		this->glBindTexture(GL_TEXTURE_2D, m_OutputTexID);
		this->glDeleteTextures(1, &m_OutputTexID);
		m_OutputTexID = 0;
		deleted = true;
	}

	return deleted;
}

/// <summary>
/// Set the viewport to match the window dimensions.
/// If the dimensions already match, no action is taken.
/// </summary>
void GLWidget::SetViewport()
{
	if (m_Init && (m_ViewWidth != m_TexWidth || m_ViewHeight != m_TexHeight))
	{
		this->glViewport(0, 0, GLint{ m_TexWidth }, GLint{ m_TexHeight });
		m_Viewport = glm::ivec4(0, 0, m_TexWidth, m_TexHeight);
		m_ViewWidth = m_TexWidth;
		m_ViewHeight = m_TexHeight;
	}
}

/// <summary>
/// Determine whether the dimensions of the renderer's current ember match
/// the dimensions of the widget, texture and viewport.
/// Since this uses the renderer's dimensions, this
/// must be called after the renderer has set the current ember.
/// </summary>
/// <returns>True if all sizes match, else false.</returns>
template <typename T>
bool GLEmberController<T>::SizesMatch()
{
	//auto scaledW = std::ceil(m_GL->width() * m_GL->devicePixelRatioF());
	//auto scaledH = std::ceil(m_GL->height() * m_GL->devicePixelRatioF());
	const auto ember = m_FractoriumEmberController->CurrentEmber();
	return (ember &&
			ember->m_FinalRasW == m_GL->m_TexWidth &&
			ember->m_FinalRasH == m_GL->m_TexHeight &&
			m_GL->m_TexWidth == m_GL->m_ViewWidth &&
			m_GL->m_TexHeight == m_GL->m_ViewHeight);
}

/// <summary>
/// Draw the unit square.
/// </summary>
void GLWidget::DrawUnitSquare()
{
	glLineWidth(1.0f * devicePixelRatioF());
	GLfloat vertices[] =//Should these be of type T?//TODO
	{
		-1, -1,
		1, -1,
		-1, 1,
		1, 1,
		-1, -1,
		-1, 1,
		1, -1,
		1, 1
	};
	QVector4D col(1.0f, 1.0f, 1.0f, 0.25f);
	DrawPointOrLine(col, vertices, 8, GL_LINES);
	const GLfloat vertices2[] =//Should these be of type T?//TODO
	{
		-1, 0,
		1, 0
	};
	QVector4D col2(1.0f, 0.0f, 0.0f, 0.5f);
	DrawPointOrLine(col2, vertices2, 2, GL_LINES);
	const GLfloat vertices3[] =//Should these be of type T?//TODO
	{
		0, -1,
		0, 1
	};
	const QVector4D col3(0.0f, 1.0f, 0.0f, 0.5f);
	DrawPointOrLine(col3, vertices3, 2, GL_LINES);
}

/// <summary>
/// Draw the grid
/// The frequency of the grid lines will change depending on the zoom (ALT+WHEEL).
/// Calculated with the frame always centered, the renderer just moves the camera.
/// </summary>
template <typename T>
void GLEmberController<T>::DrawGrid()
{
	const auto renderer = m_Fractorium->m_Controller->Renderer();
	const auto scale = m_FractoriumEmberController->AffineScaleCurrentToLocked();
	//qDebug() << renderer->UpperRightX(false) << " " << renderer->LowerLeftX(false) << " " << renderer->UpperRightY(false) << " " << renderer->LowerLeftY(false);
	const float unitX = (std::abs(renderer->UpperRightX(false) - renderer->LowerLeftX(false)) / 2.0f) / scale;
	const float unitY = (std::abs(renderer->UpperRightY(false) - renderer->LowerLeftY(false)) / 2.0f) / scale;

	if (unitX > 100000 || unitY > 100000)//Need a better way to do this.//TODO
	{
		qDebug() << unitX << " " << unitY;
		//return;
	}

	const float xLow = std::floor(-unitX);
	const float xHigh = std::ceil(unitX);
	const float yLow = std::floor(-unitY);
	const float yHigh = std::ceil(unitY);
	const float alpha = 0.25f;
	const int xsteps = std::ceil(std::abs(xHigh - xLow) / GridStep);//Need these because sometimes the float value never reaches the max and it gets stuck in an infinite loop.
	const int ysteps = std::ceil(std::abs(yHigh - yLow) / GridStep);
	Affine2D<T> temp;
	m_GL->glLineWidth(1.0f * m_GL->devicePixelRatioF());
	const m4T mat = (temp * scale).ToMat4ColMajor();
	glm::tmat4x4<float, glm::defaultp> tempmat4 = mat;
	m_GL->m_ModelViewMatrix = QMatrix4x4(glm::value_ptr(tempmat4));
	m_GL->glLineWidth(1.0f * m_GL->devicePixelRatioF());
	m_Verts.clear();

	for (float fx = xLow, i = 0; fx <= xHigh && i < xsteps; fx += GridStep, i++)
	{
		m_Verts.push_back(fx);
		m_Verts.push_back(yLow);
		m_Verts.push_back(fx);
		m_Verts.push_back(yHigh);
	}

	for (float fy = yLow, i = 0; fy < yHigh && i < ysteps; fy += GridStep, i++)
	{
		m_Verts.push_back(xLow);
		m_Verts.push_back(fy);
		m_Verts.push_back(xHigh);
		m_Verts.push_back(fy);
	}

	QVector4D col(0.5f, 0.5f, 0.5f, alpha);
	m_GL->DrawPointOrLine(col, m_Verts, GL_LINES);
	m_Verts.clear();
	m_Verts.push_back(0.0f);
	m_Verts.push_back(0.0f);
	m_Verts.push_back(xHigh);
	m_Verts.push_back(0.0f);
	col = QVector4D(1.0f, 0.0f, 0.0f, alpha);
	m_GL->DrawPointOrLine(col, m_Verts, GL_LINES);
	m_Verts.clear();
	m_Verts.push_back(0.0f);
	m_Verts.push_back(0.0f);
	m_Verts.push_back(xLow);
	m_Verts.push_back(0.0f);
	col = QVector4D(0.5f, 0.0f, 0.0f, alpha);
	m_GL->DrawPointOrLine(col, m_Verts, GL_LINES);
	m_Verts.clear();
	m_Verts.push_back(0.0f);
	m_Verts.push_back(0.0f);
	m_Verts.push_back(0.0f);
	m_Verts.push_back(yHigh);
	col = QVector4D(0.0f, 1.0f, 0.0f, alpha);
	m_GL->DrawPointOrLine(col, m_Verts, GL_LINES);
	m_Verts.clear();
	m_Verts.push_back(0.0f);
	m_Verts.push_back(0.0f);
	m_Verts.push_back(0.0f);
	m_Verts.push_back(yLow);
	col = QVector4D(0.0f, 0.5f, 0.0f, alpha);
	m_GL->DrawPointOrLine(col, m_Verts, GL_LINES);
}

/// <summary>
/// Draw the pre or post affine circle for the passed in xform.
/// For drawing affine transforms, multiply the identity model view matrix by the
/// affine for each xform, so that all points are considered to be "1".
/// </summary>
/// <param name="xform">A pointer to the xform whose affine will be drawn</param>
/// <param name="pre">True for pre affine, else false for post.</param>
/// <param name="selected">True if selected (draw enclosing circle), else false (only draw axes).</param>
/// <param name="hovered">True if the xform is being hovered over (draw tansparent disc), else false (no disc).</param>
template <typename T>
void GLEmberController<T>::DrawAffine(const Xform<T>* xform, bool pre, bool selected, bool hovered)
{
	const auto ember = m_FractoriumEmberController->CurrentEmber();
	const auto final = ember->IsFinalXform(xform);
	const auto index = ember->GetXformIndex(xform);
	const auto size = ember->m_Palette.m_Entries.size();
	const auto color = ember->m_Palette.m_Entries[Clamp<T>(xform->m_ColorX * size, 0, size - 1)];
	const auto& affine = pre ? xform->m_Affine : xform->m_Post;
	const m4T mat = (affine * m_FractoriumEmberController->AffineScaleCurrentToLocked()).ToMat4ColMajor();
	glm::tmat4x4<float, glm::defaultp> tempmat4 = mat;
	m_GL->m_ModelViewMatrix = QMatrix4x4(glm::value_ptr(tempmat4));
	m_GL->DrawAffineHelper(index, 1.0, 3.0, selected, hovered, pre, final, true);//Circle line width is thinner than the line width for the axes, just to distinguish it.
	m_GL->DrawAffineHelper(index, 0.5, 2.0, selected, hovered, pre, final, false);
	QVector4D col(0.0f, 0.0f, 0.0f, selected ? 1.0f : 0.5f);
	m_Verts.clear();
	m_Verts.push_back(0.0f);
	m_Verts.push_back(0.0f);
	m_Verts.push_back(1.0f);
	m_Verts.push_back(0.0f);
	m_Verts.push_back(0.0f);
	m_Verts.push_back(1.0f);
	m_GL->DrawPointOrLine(col, m_Verts, GL_POINTS, !pre, 6.0f);//Three black points, one in the center and two on the circle. Drawn big 5px first to give a black outline.
	//Somewhat of a hack, since it's drawing over the Y axis line it just drew.
	m_GL->glLineWidth(2.0f * m_GL->devicePixelRatioF());//Draw lines again for y axis only, using the color of the selected xform.
	m_Verts.clear();
	m_Verts.push_back(0.0f);
	m_Verts.push_back(0.0f);
	m_Verts.push_back(0.0f);
	m_Verts.push_back(1.0f);
	col = QVector4D(color.r, color.g, color.b, 1.0f);
	m_GL->DrawPointOrLine(col, m_Verts, GL_LINES, !pre);
	//Line from x to y in the color of the xform combo, thinner and solid with no background to somewhat distinguish it.
	m_GL->glLineWidth(0.5f * m_GL->devicePixelRatioF());
	m_Verts.clear();
	m_Verts.push_back(0);
	m_Verts.push_back(1);
	m_Verts.push_back(1);
	m_Verts.push_back(0);
	auto qcol = final ? m_Fractorium->m_FinalXformComboColor : m_Fractorium->m_XformComboColors[index % XFORM_COLOR_COUNT];
	m_GL->DrawPointOrLine(QVector4D(qcol.redF(), qcol.greenF(), qcol.blueF(), 1.0f), m_Verts, GL_LINES, !pre);
	//
	m_GL->glLineWidth(2.0f * m_GL->devicePixelRatioF());
	m_Verts.clear();
	m_Verts.push_back(0.0f);//Center.
	m_Verts.push_back(0.0f);
	col = QVector4D(1.0f, 1.0f, 1.0f, selected ? 1.0f : 0.5f);
	m_GL->DrawPointOrLine(col, m_Verts, GL_POINTS, false, 5.0f);//Draw smaller white point, to give a black outline effect.
	m_Verts.clear();
	m_Verts.push_back(1.0f);//X axis.
	m_Verts.push_back(0.0f);
	col = QVector4D(0.0f, 1.0f, 0.0f, selected ? 1.0f : 0.5f);
	m_GL->DrawPointOrLine(col, m_Verts, GL_POINTS, false, 5.0f);//Draw smaller green point, to give a black outline effect.
	m_Verts.clear();
	m_Verts.push_back(0.0f);//Y axis.
	m_Verts.push_back(1.0f);
	col = QVector4D(1.0f, 0.0f, 1.0f, selected ? 1.0f : 0.5f);
	m_GL->DrawPointOrLine(col, m_Verts, GL_POINTS, false, 5.0f);//Draw smaller purple point, to give a black outline effect.
	m_GL->m_ModelViewMatrix.setToIdentity();
}

/// <summary>
/// Draw the axes, and optionally the surrounding circle
/// of an affine transform.
/// </summary>
/// <param name="index"></param>
/// <param name="circleWidth"></param>
/// <param name="lineWidth"></param>
/// <param name="selected">True if selected (draw enclosing circle), else false (only draw axes).</param>
/// <param name="hovered">True if the xform is being hovered over (draw tansparent disc), else false (no disc).</param>
/// <param name="pre"></param>
/// <param name="final"></param>
/// <param name="background"></param>
void GLWidget::DrawAffineHelper(int index, float circleWidth, float lineWidth, bool selected, bool hovered, bool pre, bool final, bool background)
{
	const auto col = final ? m_Fractorium->m_FinalXformComboColor : m_Fractorium->m_XformComboColors[index % XFORM_COLOR_COUNT];
	QVector4D color;

	//Circle part.
	if (!background)
	{
		color = QVector4D(col.redF(), col.greenF(), col.blueF(), hovered ? 0.25f : 1.0f);//Draw pre affine transform with white.
	}
	else
	{
		color = QVector4D(0.0f, 0.0f, 0.0f, hovered ? 0.25f : 1.0f);//Draw pre affine transform outline with white.
	}

	m_Verts.clear();
	glLineWidth(circleWidth * devicePixelRatioF());//One thinner, colored black, except green on x axis for post affine.

	if (selected || hovered)
	{
		for (size_t i = 1; i <= 64; i++)//The circle.
		{
			const float theta = static_cast<float>(M_PI) * 2.0f * static_cast<float>(i % 64) / 64.0f;
			const float fx = std::cos(theta);
			const float fy = std::sin(theta);
			m_Verts.push_back(fx);
			m_Verts.push_back(fy);
		}

		DrawPointOrLine(color, m_Verts, hovered ? GL_TRIANGLE_FAN : GL_LINE_LOOP, !pre);
	}

	glLineWidth(lineWidth * devicePixelRatioF());//One thicker, colored black, except green on x axis for post affine.

	//Lines from center to circle.
	if (!background)
	{
		color = QVector4D(col.redF(), col.greenF(), col.blueF(), 1.0f);
	}
	else
	{
		if (pre)
			color = QVector4D(0.0f, 0.0f, 0.0f, 1.0f);//Draw pre affine transform outline with white.
		else
			color = QVector4D(0.0f, 0.75f, 0.0f, 1.0f);//Draw post affine transform outline with green.
	}

	//The lines from the center to the circle.
	m_Verts.clear();
	m_Verts.push_back(0);//X axis.
	m_Verts.push_back(0);
	m_Verts.push_back(1);
	m_Verts.push_back(0);
	DrawPointOrLine(color, m_Verts, GL_LINES, !pre);

	if (background)
		color = QVector4D(0.0f, 0.0f, 0.0f, 1.0f);

	m_Verts.clear();
	m_Verts.push_back(0);//Y axis.
	m_Verts.push_back(0);
	m_Verts.push_back(0);
	m_Verts.push_back(1);
	DrawPointOrLine(color, m_Verts, GL_LINES, !pre);
}

/// <summary>
/// Determine the index of the xform being hovered over if any.
/// Give precedence to the currently selected xform, if any.
/// </summary>
/// <param name="glCoords">The mouse raster coordinates to check</param>
/// <returns>The index of the xform being hovered over, else -1 if no hover.</returns>
template <typename T>
int GLEmberController<T>::UpdateHover(const v3T& glCoords)
{
	const bool pre = m_Fractorium->DrawPreAffines();
	const bool post = m_Fractorium->DrawPostAffines();
	int i = 0, bestIndex = -1;
	T bestDist = 10;
	const auto ember = m_FractoriumEmberController->CurrentEmber();
	m_HoverType = eHoverType::HoverNone;

	if (m_Fractorium->DrawXforms())//Don't bother checking anything if the user wants to see no xforms.
	{
		const bool forceFinal = m_Fractorium->HaveFinal();

		//If there's a selected/current xform, check it first so it gets precedence over the others.
		if (m_SelectedXform)
		{
			//These checks prevent highlighting the pre/post selected xform circle, when one is set to show all, and the other
			//is set to show current, and the user hovers over another xform, but doesn't select it, then moves the mouse
			//back over the hidden circle for the pre/post that was set to only show current.
			const bool isSel = m_Fractorium->IsXformSelected(ember->GetTotalXformIndex(m_SelectedXform));
			const bool checkPre = pre && (m_Fractorium->DrawAllPre() || (m_Fractorium->DrawSelectedPre() && isSel));
			const bool checkPost = post && (m_Fractorium->DrawAllPost() || (m_Fractorium->DrawSelectedPost() && isSel));

			if (CheckXformHover(m_SelectedXform, glCoords, bestDist, checkPre, checkPost))
			{
				m_HoverXform = m_SelectedXform;
				bestIndex = static_cast<int>(ember->GetTotalXformIndex(m_SelectedXform, forceFinal));
			}
		}

		//Check all xforms.
		while (const auto xform = ember->GetTotalXform(i, forceFinal))
		{
			const bool isSel = m_Fractorium->IsXformSelected(i);

			if (pre)
			{
				const bool checkPre = m_Fractorium->DrawAllPre() || (m_Fractorium->DrawSelectedPre() && isSel) || (m_SelectedXform == xform);

				if (checkPre)//Only check pre affine if they are shown.
				{
					if (CheckXformHover(xform, glCoords, bestDist, true, false))
					{
						m_HoverXform = xform;
						bestIndex = i;
					}
				}
			}

			if (post)
			{
				const bool checkPost = m_Fractorium->DrawAllPost() || (m_Fractorium->DrawSelectedPost() && isSel) || (m_SelectedXform == xform);

				if (checkPost)
				{
					if (CheckXformHover(xform, glCoords, bestDist, false, true))
					{
						m_HoverXform = xform;
						bestIndex = i;
					}
				}
			}

			i++;
		}
	}

	return bestIndex;
}

/// <summary>
/// Determine the passed in xform's pre/post affine transforms are being hovered over.
/// Meant to be called in succession when checking all xforms for hover, and the best
/// hover distance is recorded in the bestDist reference parameter.
/// Mouse coordinates will be converted internally to world cartesian coordinates for checking.
/// </summary>
/// <param name="xform">A pointer to the xform to check for hover</param>
/// <param name="glCoords">The mouse raster coordinates to check</param>
/// <param name="bestDist">Reference to hold the best distance found so far</param>
/// <param name="pre">True to check pre affine, else don't.</param>
/// <param name="post">True to check post affine, else don't.</param>
/// <returns>True if hovering and the distance is smaller than the bestDist parameter</returns>
template <typename T>
bool GLEmberController<T>::CheckXformHover(const Xform<T>* xform, const v3T& glCoords, T& bestDist, bool pre, bool post)
{
	bool preFound = false, postFound = false;
	T dist = 0;
	const auto scale = m_FractoriumEmberController->AffineScaleCurrentToLocked();
	v3T pos;

	if (pre)
	{
		const auto affineScaled = xform->m_Affine * scale;
		const v3T translation(affineScaled.C(), affineScaled.F(), 0);
		const v3T transScreen = glm::project(translation, m_Modelview, m_Projection, m_Viewport);
		const v3T xAxis(affineScaled.A(), affineScaled.D(), 0);
		const v3T xAxisScreen = glm::project(translation + xAxis, m_Modelview, m_Projection, m_Viewport);
		const v3T yAxis(affineScaled.B(), affineScaled.E(), 0);
		const v3T yAxisScreen = glm::project(translation + yAxis, m_Modelview, m_Projection, m_Viewport);
		pos = translation;
		dist = glm::distance(glCoords, transScreen);

		if (dist < bestDist)
		{
			bestDist = dist;
			m_HoverType = eHoverType::HoverTranslation;
			m_HoverHandlePos = pos;
			preFound = true;
		}

		pos = translation + xAxis;
		dist = glm::distance(glCoords, xAxisScreen);

		if (dist < bestDist)
		{
			bestDist = dist;
			m_HoverType = eHoverType::HoverXAxis;
			m_HoverHandlePos = pos;
			preFound = true;
		}

		pos = translation + yAxis;
		dist = glm::distance(glCoords, yAxisScreen);

		if (dist < bestDist)
		{
			bestDist = dist;
			m_HoverType = eHoverType::HoverYAxis;
			m_HoverHandlePos = pos;
			preFound = true;
		}

		if (preFound)
			m_AffineType = eAffineType::AffinePre;
	}

	if (post)
	{
		const auto affineScaled = xform->m_Post * scale;
		const v3T translation(affineScaled.C(), affineScaled.F(), 0);
		const v3T transScreen = glm::project(translation, m_Modelview, m_Projection, m_Viewport);
		const v3T xAxis(affineScaled.A(), affineScaled.D(), 0);
		const v3T xAxisScreen = glm::project(translation + xAxis, m_Modelview, m_Projection, m_Viewport);
		const v3T yAxis(affineScaled.B(), affineScaled.E(), 0);
		const v3T yAxisScreen = glm::project(translation + yAxis, m_Modelview, m_Projection, m_Viewport);
		pos = translation;
		dist = glm::distance(glCoords, transScreen);

		if (dist < bestDist)
		{
			bestDist = dist;
			m_HoverType = eHoverType::HoverTranslation;
			m_HoverHandlePos = pos;
			postFound = true;
		}

		pos = translation + xAxis;
		dist = glm::distance(glCoords, xAxisScreen);

		if (dist < bestDist)
		{
			bestDist = dist;
			m_HoverType = eHoverType::HoverXAxis;
			m_HoverHandlePos = pos;
			postFound = true;
		}

		pos = translation + yAxis;
		dist = glm::distance(glCoords, yAxisScreen);

		if (dist < bestDist)
		{
			bestDist = dist;
			m_HoverType = eHoverType::HoverYAxis;
			m_HoverHandlePos = pos;
			postFound = true;
		}

		if (postFound)
			m_AffineType = eAffineType::AffinePost;
	}

	return preFound || postFound;
}

/// <summary>
/// Calculate the new affine transform when dragging with the x axis with the left mouse button.
/// The value returned will depend on whether any modifier keys were held down.
/// None: Rotate only.
/// Local Pivot:
///		Shift: Scale and optionally Rotate about affine center.
///		Alt: Rotate single axis about affine center.
///		Shift + Alt: Free transform.
///		Control: Rotate, snapping to grid.
///		Control + Shift: Scale and optionally Rotate, snapping to grid.
///		Control + Alt: Rotate single axis about affine center, snapping to grid.
///		Control + Shift + Alt: Free transform, snapping to grid.
/// World Pivot:
///		Shift + Alt: Rotate single axis about world center.
///		Control + Shift + Alt: Rotate single axis about world center, snapping to grid.
///		All others are the same as local pivot.
/// </summary>
/// <returns>The new affine transform to be assigned to the selected xform</returns>
template <typename T>
void GLEmberController<T>::CalcDragXAxis()
{
	const T affineToWorldScale = static_cast<T>(m_FractoriumEmberController->AffineScaleLockedToCurrent());
	const T worldToAffineScale = static_cast<T>(m_FractoriumEmberController->AffineScaleCurrentToLocked());
	const bool pre = m_AffineType == eAffineType::AffinePre;
	const bool worldPivotShiftAlt = !m_Fractorium->LocalPivot() && GetShift() && GetAlt();
	const auto worldRelAxisStartScaled = (v2T(m_HoverHandlePos) * affineToWorldScale) - m_DragSrcTransform.O();//World axis start position, relative, scaled by the zoom.
	const T startAngle = std::atan2(worldRelAxisStartScaled.y, worldRelAxisStartScaled.x);
	v3T relScaled = (m_MouseWorldPos * affineToWorldScale) - v3T(m_DragSrcTransform.O(), 0);

	if (!GetShift())
	{
		if (GetControl())
		{
			relScaled = SnapToNormalizedAngle(relScaled, 24u);//relScaled is using the relative scaled position of the axis.
		}

		const T endAngle = std::atan2(relScaled.y, relScaled.x);
		const T angle = startAngle - endAngle;
		m_FractoriumEmberController->UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex)
		{
			auto it = m_DragSrcPreTransforms.find(xfindex);

			if (it != m_DragSrcPreTransforms.end())
			{
				auto src = it->second;
				auto& affine = xform->m_Affine;

				if (GetAlt())
				{
					src.Rotate(angle);
					affine.X(src.X());
				}
				else
				{
					src.Rotate(angle);
					affine = src;
				}
			}

			it = m_DragSrcPostTransforms.find(xfindex);

			if (it != m_DragSrcPostTransforms.end())
			{
				auto src = it->second;
				auto& affine = xform->m_Post;

				if (GetAlt())
				{
					src.Rotate(angle);
					affine.X(src.X());
				}
				else
				{
					src.Rotate(angle);
					affine = src;
				}
			}

			const auto& affine = pre ? xform->m_Affine : xform->m_Post;

			if (xform == m_FractoriumEmberController->CurrentXform())
				m_DragHandlePos = v3T((affine.O() + affine.X()) * worldToAffineScale, 0);
		}, eXformUpdate::UPDATE_SELECTED, false);//Calling code will update renderer.
	}
	else
	{
		auto origmag = Zeps(glm::length(m_DragSrcTransform.X()));//Magnitude of original dragged axis before it was dragged.

		if (GetControl())
		{
			relScaled = SnapToGrid(relScaled);
		}

		const auto newmag = glm::length(relScaled);
		const auto newprc = newmag / origmag;
		const T endAngle = std::atan2(relScaled.y, relScaled.x);
		const T angle = startAngle - endAngle;
		m_FractoriumEmberController->UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex)
		{
			auto it = m_DragSrcPreTransforms.find(xfindex);

			if (it != m_DragSrcPreTransforms.end())
			{
				auto src = it->second;
				auto& affine = xform->m_Affine;

				if (worldPivotShiftAlt)
				{
					src.X(src.O() + src.X());
					src.O(v2T(0));
					src.Rotate(angle);
					affine.X(src.X() - affine.O());
				}
				else if (GetAlt())
				{
					affine.X(v2T(relScaled));//Absolute, not ratio.
				}
				else
				{
					src.ScaleXY(newprc);

					if (m_Fractorium->m_Settings->RotateAndScale())
						src.Rotate(angle);

					affine = src;
				}
			}

			it = m_DragSrcPostTransforms.find(xfindex);

			if (it != m_DragSrcPostTransforms.end())
			{
				auto src = it->second;
				auto& affine = xform->m_Post;

				if (worldPivotShiftAlt)
				{
					src.X(src.O() + src.X());
					src.O(v2T(0));
					src.Rotate(angle);
					affine.X(src.X() - affine.O());
				}
				else if (GetAlt())
				{
					affine.X(v2T(relScaled));//Absolute, not ratio.
				}
				else
				{
					src.ScaleXY(newprc);

					if (m_Fractorium->m_Settings->RotateAndScale())
						src.Rotate(angle);

					affine = src;
				}
			}

			const auto& affine = pre ? xform->m_Affine : xform->m_Post;

			if (xform == m_FractoriumEmberController->CurrentXform())
				m_DragHandlePos = v3T((affine.O() + affine.X()) * worldToAffineScale, 0);
		}, eXformUpdate::UPDATE_SELECTED, false);
	}
}

/// <summary>
/// Calculate the new affine transform when dragging with the y axis with the left mouse button.
/// The value returned will depend on whether any modifier keys were held down.
/// None: Rotate only.
/// Local Pivot:
///		Shift: Scale and optionally Rotate about affine center.
///		Alt: Rotate single axis about affine center.
///		Shift + Alt: Free transform.
///		Control: Rotate, snapping to grid.
///		Control + Shift: Scale and optionally Rotate, snapping to grid.
///		Control + Alt: Rotate single axis about affine center, snapping to grid.
///		Control + Shift + Alt: Free transform, snapping to grid.
/// World Pivot:
///		Shift + Alt: Rotate single axis about world center.
///		Control + Shift + Alt: Rotate single axis about world center, snapping to grid.
///		All others are the same as local pivot.
/// </summary>
/// <returns>The new affine transform to be assigned to the selected xform</returns>
template <typename T>
void GLEmberController<T>::CalcDragYAxis()
{
	const T affineToWorldScale = static_cast<T>(m_FractoriumEmberController->AffineScaleLockedToCurrent());
	const T worldToAffineScale = static_cast<T>(m_FractoriumEmberController->AffineScaleCurrentToLocked());
	const bool pre = m_AffineType == eAffineType::AffinePre;
	const bool worldPivotShiftAlt = !m_Fractorium->LocalPivot() && GetShift() && GetAlt();
	const auto worldRelAxisStartScaled = (v2T(m_HoverHandlePos) * affineToWorldScale) - m_DragSrcTransform.O();//World axis start position, relative, scaled by the zoom.
	const T startAngle = std::atan2(worldRelAxisStartScaled.y, worldRelAxisStartScaled.x);
	v3T relScaled = (m_MouseWorldPos * affineToWorldScale) - v3T(m_DragSrcTransform.O(), 0);

	if (!GetShift())
	{
		if (GetControl())
		{
			relScaled = SnapToNormalizedAngle(relScaled, 24u);//relScaled is using the relative scaled position of the axis.
		}

		const T endAngle = std::atan2(relScaled.y, relScaled.x);
		const T angle = startAngle - endAngle;
		m_FractoriumEmberController->UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex)
		{
			auto it = m_DragSrcPreTransforms.find(xfindex);

			if (it != m_DragSrcPreTransforms.end())
			{
				auto src = it->second;
				auto& affine = xform->m_Affine;

				if (GetAlt())
				{
					src.Rotate(angle);
					affine.Y(src.Y());
				}
				else
				{
					src.Rotate(angle);
					affine = src;
				}
			}

			it = m_DragSrcPostTransforms.find(xfindex);

			if (it != m_DragSrcPostTransforms.end())
			{
				auto src = it->second;
				auto& affine = xform->m_Post;

				if (GetAlt())
				{
					src.Rotate(angle);
					affine.Y(src.Y());
				}
				else
				{
					src.Rotate(angle);
					affine = src;
				}
			}

			const auto& affine = pre ? xform->m_Affine : xform->m_Post;

			if (xform == m_FractoriumEmberController->CurrentXform())
				m_DragHandlePos = v3T((affine.O() + affine.Y()) * worldToAffineScale, 0);
		}, eXformUpdate::UPDATE_SELECTED, false);//Calling code will update renderer.
	}
	else
	{
		const auto origmag = Zeps(glm::length(m_DragSrcTransform.Y()));//Magnitude of original dragged axis before it was dragged.

		if (GetControl())
		{
			relScaled = SnapToGrid(relScaled);
		}

		const auto newmag = glm::length(relScaled);
		const auto newprc = newmag / origmag;
		const T endAngle = std::atan2(relScaled.y, relScaled.x);
		const T angle = startAngle - endAngle;
		m_FractoriumEmberController->UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex)
		{
			auto it = m_DragSrcPreTransforms.find(xfindex);

			if (it != m_DragSrcPreTransforms.end())
			{
				auto src = it->second;
				auto& affine = xform->m_Affine;

				if (worldPivotShiftAlt)
				{
					src.Y(src.O() + src.Y());
					src.O(v2T(0));
					src.Rotate(angle);
					affine.Y(src.Y() - affine.O());
				}
				else if (GetAlt())
				{
					affine.Y(v2T(relScaled));//Absolute, not ratio.
				}
				else
				{
					src.ScaleXY(newprc);

					if (m_Fractorium->m_Settings->RotateAndScale())
						src.Rotate(angle);

					affine = src;
				}
			}

			it = m_DragSrcPostTransforms.find(xfindex);

			if (it != m_DragSrcPostTransforms.end())
			{
				auto src = it->second;
				auto& affine = xform->m_Post;

				if (worldPivotShiftAlt)
				{
					src.Y(src.O() + src.Y());
					src.O(v2T(0));
					src.Rotate(angle);
					affine.Y(src.Y() - affine.O());
				}
				else if (GetAlt())
				{
					affine.Y(v2T(relScaled));//Absolute, not ratio.
				}
				else
				{
					src.ScaleXY(newprc);

					if (m_Fractorium->m_Settings->RotateAndScale())
						src.Rotate(angle);

					affine = src;
				}
			}

			const auto& affine = pre ? xform->m_Affine : xform->m_Post;

			if (xform == m_FractoriumEmberController->CurrentXform())
				m_DragHandlePos = v3T((affine.O() + affine.Y()) * worldToAffineScale, 0);
		}, eXformUpdate::UPDATE_SELECTED, false);
	}
}

/// <summary>
/// Calculate the new affine transform when dragging the center with the left mouse button.
/// The value returned will depend on whether any modifier keys were held down.
/// None: Free transform.
/// Local Pivot:
///		Shift: Rotate about world center, keeping orientation the same.
///		Control: Free transform, snapping to grid.
///		Control + Shift: Rotate about world center, keeping orientation the same, snapping to grid.
/// World Pivot:
///		Shift: Rotate about world center, rotating orientation.
///		Control + Shift: Rotate about world center, rotating orientation, snapping to grid.
///		All others are the same as local pivot.
/// </summary>
template <typename T>
void GLEmberController<T>::CalcDragTranslation()
{
	const T affineToWorldScale = static_cast<T>(m_FractoriumEmberController->AffineScaleLockedToCurrent());
	const T worldToAffineScale = static_cast<T>(m_FractoriumEmberController->AffineScaleCurrentToLocked());
	const bool worldPivotShift = !m_Fractorium->LocalPivot() && GetShift();
	const bool pre = m_AffineType == eAffineType::AffinePre;

	if (GetShift())
	{
		const v3T snapped = GetControl() ? SnapToNormalizedAngle(m_MouseWorldPos, 24) : m_MouseWorldPos;
		const T startAngle = std::atan2(m_DragSrcTransform.O().y, m_DragSrcTransform.O().x);
		const T endAngle = std::atan2(snapped.y, snapped.x);
		const T angle = startAngle - endAngle;
		m_FractoriumEmberController->UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex)
		{
			auto it = m_DragSrcPreTransforms.find(xfindex);

			if (it != m_DragSrcPreTransforms.end())
			{
				auto src = it->second;
				auto& affine = xform->m_Affine;
				src.RotateTrans(angle);

				if (worldPivotShift)
				{
					src.Rotate(angle);
					affine.X(src.X());
					affine.Y(src.Y());
				}

				affine.O(src.O());
			}

			it = m_DragSrcPostTransforms.find(xfindex);

			if (it != m_DragSrcPostTransforms.end())
			{
				auto src = it->second;
				auto& affine = xform->m_Post;
				src.RotateTrans(angle);

				if (worldPivotShift)
				{
					src.Rotate(angle);
					affine.X(src.X());
					affine.Y(src.Y());
				}

				affine.O(src.O());
			}

			const auto& affine = pre ? xform->m_Affine : xform->m_Post;

			if (xform == m_FractoriumEmberController->CurrentXform())
				m_DragHandlePos = v3T(affine.O(), 0) * worldToAffineScale;
		}, eXformUpdate::UPDATE_SELECTED, false);//Calling code will update renderer.
	}
	else
	{
		const auto diff = m_MouseWorldPos - m_MouseDownWorldPos;

		if (GetControl())
		{
			m_FractoriumEmberController->UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex)
			{
				auto it = m_DragSrcPreTransforms.find(xfindex);

				if (it != m_DragSrcPreTransforms.end())
				{
					const auto& src = it->second;
					auto& affine = xform->m_Affine;
					const auto offset = src.O() + (affineToWorldScale * v2T(diff));
					const auto snapped = SnapToGrid(offset);
					affine.O(v2T(snapped.x, snapped.y));
				}

				it = m_DragSrcPostTransforms.find(xfindex);

				if (it != m_DragSrcPostTransforms.end())
				{
					auto& src = it->second;
					auto& affine = xform->m_Post;
					auto offset = src.O() + (affineToWorldScale * v2T(diff));
					auto snapped = SnapToGrid(offset);
					affine.O(v2T(snapped.x, snapped.y));
				}

				auto& affine = pre ? xform->m_Affine : xform->m_Post;

				if (xform == m_FractoriumEmberController->CurrentXform())
					m_DragHandlePos = v3T(affine.O(), 0) * worldToAffineScale;
			}, eXformUpdate::UPDATE_CURRENT_AND_SELECTED, false);
		}
		else
		{
			m_FractoriumEmberController->UpdateXform([&](Xform<T>* xform, size_t xfindex, size_t selIndex)
			{
				auto it = m_DragSrcPreTransforms.find(xfindex);

				if (it != m_DragSrcPreTransforms.end())
				{
					auto& src = it->second;
					auto& affine = xform->m_Affine;
					affine.O(src.O() + (affineToWorldScale * v2T(diff)));
				}

				it = m_DragSrcPostTransforms.find(xfindex);

				if (it != m_DragSrcPostTransforms.end())
				{
					auto& src = it->second;
					auto& affine = xform->m_Post;
					affine.O(src.O() + (affineToWorldScale * v2T(diff)));
				}

				auto& affine = pre ? xform->m_Affine : xform->m_Post;

				if (xform == m_FractoriumEmberController->CurrentXform())
					m_DragHandlePos = v3T(affine.O(), 0) * worldToAffineScale;
			}, eXformUpdate::UPDATE_SELECTED, false);
		}
	}
}

/// <summary>
/// Thin wrapper to check if all controllers are ok and return a pointer to the GLController.
/// </summary>
/// <returns>A pointer to the GLController if everything is ok, else false.</returns>
GLEmberControllerBase* GLWidget::GLController()
{
	if (m_Fractorium && m_Fractorium->ControllersOk())
		return m_Fractorium->m_Controller->GLController();

	return nullptr;
}

template class GLEmberController<float>;

#ifdef DO_DOUBLE
	template class GLEmberController<double>;
#endif
