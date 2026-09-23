#include "FractoriumPch.h"
#include "GLEmberController.h"
#include "FractoriumEmberController.h"
#include "Fractorium.h"
#include "GLWidget.h"

/// <summary>
/// Constructor which assigns pointers to the main window and the GLWidget.
/// </summary>
/// <param name="fractorium">Pointer to the main window</param>
/// <param name="glWidget">Pointer to the GLWidget</param>
GLEmberControllerBase::GLEmberControllerBase(Fractorium* fractorium, GLWidget* glWidget)
{
	m_Fractorium = fractorium;
	m_GL = glWidget;
	m_AffineType = eAffineType::AffinePre;
	m_HoverType = eHoverType::HoverNone;
	m_DragState = eDragState::DragNone;
	m_DragModifier = 0;
}

/// <summary>
/// Empty destructor which does nothing.
/// </summary>
GLEmberControllerBase::~GLEmberControllerBase() { }

/// <summary>
/// Constructor which passes the pointers to the main window the GLWidget to the base,
/// then assigns the pointer to the parent controller.
/// </summary>
/// <param name="fractorium">Pointer to the main window</param>
/// <param name="glWidget">Pointer to the GLWidget</param>
/// <param name="controller">Pointer to the parent controller of the same template type</param>
template <typename T>
GLEmberController<T>::GLEmberController(Fractorium* fractorium, GLWidget* glWidget, FractoriumEmberController<T>* controller)
	: GLEmberControllerBase(fractorium, glWidget)
{
	GridStep = static_cast<T>(1.0 / 4.0); // michel, needs to insert on GUI to be flexible//TODO
	m_FractoriumEmberController = controller;
	m_HoverXform = nullptr;
	m_SelectedXform = nullptr;
	m_CenterDownX = 0;
	m_CenterDownY = 0;
}

/// <summary>
/// Empty destructor which does nothing.
/// </summary>
template <typename T>
GLEmberController<T>::~GLEmberController() { }

/// <summary>
/// Check that the final output size of the current ember matches the dimensions passed in.
/// </summary>
/// <param name="w">The width to compare to</param>
/// <param name="h">The height to compare to</param>
/// <returns>True if any don't match, else false if they are both equal.</returns>
template <typename T>
bool GLEmberController<T>::CheckForSizeMismatch(int w, int h)
{
    return m_FractoriumEmberController->FinalRasW() != (size_t)w || m_FractoriumEmberController->FinalRasH() != (size_t)h;
}

/// <summary>
/// Reset the drag and hover state. Called in response setting a new ember as the current one.
/// </summary>
template <typename T>
void GLEmberController<T>::ResetMouseState()
{
	m_HoverType = eHoverType::HoverNone;
	m_HoverXform = nullptr;
	m_SelectedXform = nullptr;
}

/// <summary>
/// Calculate the scale.
/// Used when dragging the right mouse button.
/// </summary>
/// <returns>The distance dragged in pixels</returns>
template <typename T>
T GLEmberController<T>::CalcScale()
{
	//Can't operate using world coords here because every time scale changes, the world bounds change.
	//So must instead calculate distance traveled based on window coords, which do not change outside of resize events.
	const auto windowCenter = ScrolledCenter(false);
	const v2T windowMousePosDistanceFromCenter(m_MousePos.x - windowCenter.x, m_MousePos.y - windowCenter.y);
	const v2T windowMouseDownDistanceFromCenter(m_MouseDownPos.x - windowCenter.x, m_MouseDownPos.y - windowCenter.y);
	const T lengthMousePosFromCenterInPixels = glm::length(windowMousePosDistanceFromCenter);
	const T lengthMouseDownFromCenterInPixels = glm::length(windowMouseDownDistanceFromCenter);
	return lengthMousePosFromCenterInPixels - lengthMouseDownFromCenterInPixels;
}

/// <summary>
/// Calculate the rotation.
/// Used when dragging the right mouse button.
/// </summary>
/// <returns>The angular distance rotated from -180-180</returns>
template <typename T>
T GLEmberController<T>::CalcRotation()
{
	const auto scrolledWorldCenter = ScrolledCenter(true);
	const T rotStart = NormalizeDeg360<T>((std::atan2(m_MouseDownWorldPos.y - scrolledWorldCenter.y, m_MouseDownWorldPos.x - scrolledWorldCenter.x) * RAD_2_DEG_T));
	const T rot = NormalizeDeg360<T>((std::atan2(m_MouseWorldPos.y - scrolledWorldCenter.y, m_MouseWorldPos.x - scrolledWorldCenter.x) * RAD_2_DEG_T));
	return rotStart - rot;
}

/// <summary>
/// Return the window coordinates of the center of the viewable area.
/// This is the middle of the parent scroll area plus the scroll bar offset, all scaled by the device pixel ratio.
/// </summary>
/// <param name="toWorld">True to return world coordinates, else return window coordinates.</param>
/// <returns>The coordinates of the center of the viewable area in either window space or world space.</returns>
template <typename T>
v3T GLEmberController<T>::ScrolledCenter(bool toWorld)
{
	const auto dprf = m_GL->devicePixelRatioF();
	const auto wpsa = m_Fractorium->ui.GLParentScrollArea->width();
	const auto hpsa = m_Fractorium->ui.GLParentScrollArea->height();
	const auto hpos = m_Fractorium->ui.GLParentScrollArea->horizontalScrollBar()->value();
	const auto vpos = m_Fractorium->ui.GLParentScrollArea->verticalScrollBar()->value();
	v3T v;

	if (!m_Fractorium->ui.GLParentScrollArea->horizontalScrollBar()->isVisible() && !m_Fractorium->ui.GLParentScrollArea->verticalScrollBar()->isVisible())
		v = v3T(((m_GL->width() / 2)) * dprf,
				((m_GL->height() / 2)) * dprf,
				0);
	else
		v = v3T((hpos + (wpsa / 2)) * dprf,
				(vpos + (hpsa / 2)) * dprf,
				0);

	if (toWorld)
		return WindowToWorld(v, true);

	return v;
}

/// <summary>
/// Snap the passed in world cartesian coordinate to the grid for rotation, scale or translation.
/// </summary>
/// <param name="vec">The world cartesian coordinate to be snapped</param>
/// <returns>The snapped world cartesian coordinate</returns>
template <typename T>
typename v2T GLEmberController<T>::SnapToGrid(const v2T& vec) const
{
	return v2T(glm::round(vec.x / GridStep) * GridStep, glm::round(vec.y / GridStep) * GridStep);
}

/// <summary>
/// Snap the passed in world cartesian coordinate to the grid for rotation, scale or translation.
/// </summary>
/// <param name="vec">The world cartesian coordinate to be snapped</param>
/// <returns>The snapped world cartesian coordinate</returns>
template <typename T>
typename v3T GLEmberController<T>::SnapToGrid(const v3T& vec) const
{
	return v3T(glm::round(vec.x / GridStep) * GridStep,
			   glm::round(vec.y / GridStep) * GridStep,
			   vec.z);
}

/// <summary>
/// Snap the passed in world cartesian coordinate to the grid for rotation only.
/// </summary>
/// <param name="vec">The world cartesian coordinate to be snapped</param>
/// <param name="divisions">The divisions of a circle to use for snapping</param>
/// <returns>The snapped world cartesian coordinate</returns>
template <typename T>
typename v3T GLEmberController<T>::SnapToNormalizedAngle(const v3T& vec, uint divisions) const
{
	T bestRsq = numeric_limits<T>::max();
	v3T c(0, 0, 0), best;
	best.x = 1;
	best.y = 0;

	for (uint i = 0; i < divisions; i++)
	{
		const auto theta = 2.0 * M_PI * static_cast<T>(i) / static_cast<T>(divisions);
		c.x = std::cos(theta);
		c.y = std::sin(theta);
		const auto rsq = glm::distance(vec, c);

		if (rsq < bestRsq)
		{
			best = c;
			bestRsq = rsq;
		}
	}

	return best;
}

/// <summary>
/// Convert raster window coordinates to world cartesian coordinates.
/// </summary>
/// <param name="v">The window coordinates to convert</param>
/// <param name="flip">True to flip vertically, else don't.</param>
/// <returns>The converted world cartesian coordinates</returns>
template <typename T>
typename v3T GLEmberController<T>::WindowToWorld(const v3T& v, bool flip) const
{
	const v3T mouse(v.x, flip ? m_Viewport[3] - v.y : v.y, 0);//Must flip y because in OpenGL, 0,0 is bottom left, but in windows, it's top left.
	v3T newCoords = glm::unProject(mouse, m_Modelview, m_Projection, m_Viewport);//Perform the calculation.
	newCoords.z = 0;//For some reason, unProject() always comes back with the z coordinate as something other than 0. It should be 0 at all times.
	return newCoords;
}

/// <summary>
/// Template specialization for querying the viewport, modelview and projection
/// matrices as floats.
/// </summary>
template <>
void GLEmberController<float>::QueryVMP()
{
#ifndef USE_GLSL
	m_GL->glGetIntegerv(GL_VIEWPORT, glm::value_ptr(m_Viewport));
	m_GL->glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(m_Modelview));
	m_GL->glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(m_Projection));
#else
	m_Viewport = m_GL->m_Viewport;
	glm::tmat4x4<float, glm::defaultp> tempmat = glm::make_mat4(m_GL->m_ModelViewMatrix.data());
	m_Modelview = tempmat;
	tempmat = glm::make_mat4(m_GL->m_ProjMatrix.data());
	m_Projection = tempmat;
#endif
}

#ifdef DO_DOUBLE
/// <summary>
/// Template specialization for querying the viewport, modelview and projection
/// matrices as doubles.
/// </summary>
template <>
void GLEmberController<double>::QueryVMP()
{
#ifndef USE_GLSL
	m_GL->glGetIntegerv(GL_VIEWPORT, glm::value_ptr(m_Viewport));
	m_GL->glGetDoublev(GL_MODELVIEW_MATRIX, glm::value_ptr(m_Modelview));
	m_GL->glGetDoublev(GL_PROJECTION_MATRIX, glm::value_ptr(m_Projection));
#else
	m_Viewport = m_GL->m_Viewport;
	glm::tmat4x4<float, glm::defaultp> tempmat = glm::make_mat4(m_GL->m_ModelViewMatrix.data());
	m_Modelview = tempmat;
	tempmat = glm::make_mat4(m_GL->m_ProjMatrix.data());
	m_Projection = tempmat;
#endif
}
#endif

/// <summary>
/// Template specialization for multiplying the current matrix
/// by an m4<float>.
/// </summary>
#ifndef USE_GLSL
template <>
void GLEmberController<float>::MultMatrix(tmat4x4<float, glm::defaultp>& mat)
{
	m_GL->glMultMatrixf(glm::value_ptr(mat));
}
#endif

#ifdef DO_DOUBLE
/// <summary>
/// Template specialization for multiplying the current matrix
/// by an m4<double>.
/// </summary>
#ifndef USE_GLSL
template <>
void GLEmberController<double>::MultMatrix(tmat4x4<double, glm::defaultp>& mat)
{
	m_GL->glMultMatrixd(glm::value_ptr(mat));
}
#endif
#endif

/// <summary>
/// Query the matrices currently being used.
/// Debugging function, unused.
/// </summary>
/// <param name="print">True to print values, else false.</param>
template <typename T>
void GLEmberController<T>::QueryMatrices(bool print)
{
	if (const auto renderer = m_FractoriumEmberController->Renderer())
	{
		QueryVMP();

		if (print)
		{
			for (glm::length_t i = 0; i < 4; i++)
				qDebug() << "Viewport[" << i << "] = " << m_Viewport[i] << "\n";

			for (glm::length_t i = 0; i < 16; i++)
				qDebug() << "Modelview[" << i << "] = " << glm::value_ptr(m_Modelview)[i] << "\n";

			for (glm::length_t i = 0; i < 16; i++)
				qDebug() << "Projection[" << i << "] = " << glm::value_ptr(m_Projection)[i] << "\n";
		}
	}
}

template class GLEmberController<float>;

#ifdef DO_DOUBLE
	template class GLEmberController<double>;
#endif
