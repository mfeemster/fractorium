#include "FractoriumPch.h"
#include "SpinBox.h"
#include "FractoriumSettings.h"

QTimer SpinBox::s_Timer;

/// <summary>
/// Constructor that passes parent to the base and sets up height and step.
/// Specific focus policy is used to allow the user to hover over the control
/// and change its value using the mouse wheel without explicitly having to click
/// inside of it.
/// </summary>
/// <param name="p">The parent widget. Default: nullptr.</param>
/// <param name="h">The height of the spin box. Default: 16.</param>
/// <param name="step">The step used to increment/decrement the spin box when using the mouse wheel. Default: 1.</param>
SpinBox::SpinBox(QWidget* p, int h, int step)
	: QSpinBox(p)
{
	m_DoubleClick = false;
	m_DoubleClickLowVal = 0;
	m_DoubleClickNonZero = 0;
	m_DoubleClickZero = 1;
	m_Step = step;
	m_SmallStep = 1;
	m_Settings = FractoriumSettings::DefInstance();
	setSingleStep(step);
	setFrame(false);
	setButtonSymbols(QAbstractSpinBox::NoButtons);
	setFocusPolicy(Qt::FocusPolicy::StrongFocus);
	setMinimumHeight(h);//setGeometry() has no effect, so set both of these instead.
	setMaximumHeight(h);
	setContextMenuPolicy(Qt::ContextMenuPolicy::PreventContextMenu);
	lineEdit()->installEventFilter(this);
	lineEdit()->setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);
	connect(this, SIGNAL(valueChanged(int)), this, SLOT(onSpinBoxValueChanged(int)), Qt::ConnectionType::QueuedConnection);
}

/// <summary>
/// Set the value of the control without triggering signals.
/// </summary>
/// <param name="d">The value to set it to</param>
void SpinBox::SetValueStealth(int d)
{
	blockSignals(true);
	setValue(d);
	blockSignals(false);
}

void SpinBox::SetValueStealth(size_t d) { SetValueStealth(static_cast<int>(d)); }

/// <summary>
/// Set whether to respond to double click events.
/// </summary>
/// <param name="b">True if this should respond to double click events, else false.</param>
void SpinBox::DoubleClick(bool b)
{
	m_DoubleClick = b;
}

/// <summary>
/// Set the value to be used instead of zero to represent the lower value
/// used when responding to a double click.
/// </summary>
/// <param name="val">The value to be used for the lower value instead of zero</param>
void SpinBox::DoubleClickLowVal(int val)
{
	m_DoubleClickLowVal = val;
}

int SpinBox::DoubleClickLowVal()
{
	return m_DoubleClickLowVal;
}

/// <summary>
/// Set the value to be used when the user double clicks the spinner while
/// it contains zero.
/// </summary>
/// <param name="val">The value to be used</param>
void SpinBox::DoubleClickZero(int val)
{
	m_DoubleClickZero = val;
}

int SpinBox::DoubleClickZero()
{
	return m_DoubleClickZero;
}

/// <summary>
/// Set the value to be used when the user double clicks the spinner while
/// it contains a non-zero value.
/// </summary>
/// <param name="val">The value to be used</param>
void SpinBox::DoubleClickNonZero(int val)
{
	m_DoubleClickNonZero = val;
}

int SpinBox::DoubleClickNonZero()
{
	return m_DoubleClickNonZero;
}

/// <summary>
/// Set the small step to be used when the user holds down shift while scrolling.
/// The default is step / 10, so use this if something else is needed.
/// </summary>
/// <param name="step">The small step to use for scrolling while the shift key is down</param>
void SpinBox::SmallStep(int step)
{
	m_SmallStep = std::min(1, step);
}

/// <summary>
/// Expose the underlying QLineEdit control to the caller.
/// </summary>
/// <returns>A pointer to the QLineEdit</returns>
QLineEdit* SpinBox::lineEdit()
{
	return QSpinBox::lineEdit();
}

/// <summary>
/// Another workaround for the persistent text selection bug in Qt.
/// </summary>
void SpinBox::onSpinBoxValueChanged(int i)
{
	lineEdit()->deselect();//Gets rid of nasty "feature" that always has text selected.
}

/// <summary>
/// Called while the timer is activated due to the right mouse button being held down.
/// </summary>
void SpinBox::OnTimeout()
{
	int xdistance = m_MouseMovePoint.x() - m_MouseDownPoint.x();
	int ydistance = m_MouseMovePoint.y() - m_MouseDownPoint.y();
	int distance = abs(xdistance) > abs(ydistance) ? xdistance : ydistance;
	distance = Sqr(distance) * (distance < 0 ? -1 : 1);
	double scale, val;
	int d = value();
	bool shift = QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier);
	bool ctrl = QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier);
	double amount = (m_SmallStep + m_Step) * 0.5;

	if (shift)
		scale = 0.001;
	else if (ctrl)
		scale = 0.01;
	else
		scale = 0.01;

	val = d + (distance * amount * scale);
	setValue(int(val));
}

/// <summary>
/// Event filter for taking special action on double click events.
/// </summary>
/// <param name="o">The object</param>
/// <param name="e">The eevent</param>
/// <returns>false</returns>
bool SpinBox::eventFilter(QObject* o, QEvent* e)
{
	if (!isEnabled())
		return QSpinBox::eventFilter(o, e);

	const auto me = dynamic_cast<QMouseEvent*>(e);

	if (me)
	{
		if (!m_Settings->ToggleType() &&//Ensure double click toggles, not right click.
				me->type() == QMouseEvent::MouseButtonPress &&
				me->button() == Qt::MouseButton::RightButton)
		{
			m_MouseDownPoint = m_MouseMovePoint = me->pos();
			StartTimer();
			e->accept();
			return true;
		}
		else if (!m_Settings->ToggleType() &&
				 me->type() == QMouseEvent::MouseButtonRelease &&
				 me->button() == Qt::MouseButton::RightButton)
		{
			StopTimer();
			m_MouseDownPoint = m_MouseMovePoint = me->pos();
			e->accept();
			return true;
		}
		else if (!m_Settings->ToggleType() &&
				 me->type() == QMouseEvent::MouseMove &&
				 QGuiApplication::mouseButtons() & Qt::MouseButton::RightButton)
		{
			m_MouseMovePoint = me->pos();
			e->accept();
			return true;
		}
		else if (m_DoubleClick &&
				 ((!m_Settings->ToggleType() && e->type() == QMouseEvent::MouseButtonDblClick && me->button() == Qt::MouseButton::LeftButton) ||
				  (m_Settings->ToggleType() && me->type() == QMouseEvent::MouseButtonRelease && me->button() == Qt::MouseButton::RightButton)))
		{
			if (m_DoubleClickLowVal == value())
			{
				m_DoubleClickZeroEvent(this, m_DoubleClickZero);
				setValue(m_DoubleClickZero);
			}
			else
			{
				m_DoubleClickNonZeroEvent(this, m_DoubleClickNonZero);
				setValue(m_DoubleClickNonZero);
			}

			e->accept();
			return true;
		}
	}
	else if (isEnabled())
	{
		if (e->type() == QEvent::Wheel)
		{
			if (QWheelEvent* we = dynamic_cast<QWheelEvent*>(e))
			{
				const auto shift = QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier);
				const auto ctrl = QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier);

				if (we->angleDelta().ry() > 0)
				{
					if (shift)
					{
						setSingleStep(m_SmallStep);
						setValue(value() + m_SmallStep);
					}
					else
					{
						setSingleStep(m_Step);
						setValue(value() + (ctrl ? m_Step * 10 : m_Step));
					}
				}
				else
				{
					if (shift)
					{
						setSingleStep(m_SmallStep);
						setValue(value() - m_SmallStep);
					}
					else
					{
						setSingleStep(m_Step);
						setValue(value() - (ctrl ? m_Step * 10 : m_Step));
					}
				}

				e->accept();
				return true;
			}
		}
		else if (dynamic_cast<QKeyEvent*>(e))
		{
			e->accept();
			return true;
		}
	}

	return QSpinBox::eventFilter(o, e);
}

/// <summary>
/// Override which is for handling specific key presses while this control is focused.
/// In particular, + = and up arrow increase the value, equivalent to scrolling the mouse wheel up, while also observing shift/ctrl modifiers.
/// Values are decreased in the same way by pressing - _ or down arrow.
/// </summary>
/// <param name="ke">The key event</param>
void SpinBox::keyPressEvent(QKeyEvent* ke)
{
	bool shift = QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier);
	bool ctrl = QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier);

	if (ke->key() == Qt::Key::Key_Up)
	{
		if (shift)
		{
			setSingleStep(m_SmallStep);
			setValue(value() + m_SmallStep);
		}
		else
		{
			setSingleStep(m_Step);
			setValue(value() + (ctrl ? m_Step * 10 : m_Step));
		}

		ke->accept();
	}
	else if (ke->key() == Qt::Key::Key_Down)
	{
		if (shift)
		{
			setSingleStep(m_SmallStep);
			setValue(value() - m_SmallStep);
		}
		else
		{
			setSingleStep(m_Step);
			setValue(value() - (ctrl ? m_Step * 10 : m_Step));
		}

		ke->accept();
	}
	else if (ke->key() == Qt::Key::Key_Space)
	{
		if (m_DoubleClickLowVal == value())
		{
			m_DoubleClickZeroEvent(this, m_DoubleClickZero);
			setValue(m_DoubleClickZero);
		}
		else
		{
			m_DoubleClickNonZeroEvent(this, m_DoubleClickNonZero);
			setValue(m_DoubleClickNonZero);
		}
	}
	else
		QSpinBox::keyPressEvent(ke);
}

/// <summary>
/// Called when focus enters the spinner.
/// </summary>
/// <param name="e">The event</param>
void SpinBox::focusInEvent(QFocusEvent* e)
{
	StopTimer();
	QSpinBox::focusInEvent(e);
}

/// <summary>
/// Called when focus leaves the spinner.
/// Qt has a nasty "feature" that leaves the text in a spinner selected
/// and the cursor visible, regardless of whether it has the focus.
/// Manually clear both here.
/// </summary>
/// <param name="e">The event</param>
void SpinBox::focusOutEvent(QFocusEvent* e)
{
	StopTimer();
	QSpinBox::focusOutEvent(e);
}

/// <summary>
/// Called when focus enters the spinner.
/// Must set the focus to make sure key down messages don't erroneously go to the GLWidget.
/// </summary>
/// <param name="e">The event</param>
void SpinBox::enterEvent(QEnterEvent* e)
{
	StopTimer();
	QSpinBox::enterEvent(e);
}

/// <summary>
/// Called when focus leaves the spinner.
/// Must clear the focus to make sure key down messages don't erroneously go to the GLWidget.
/// </summary>
/// <param name="e">The event</param>
void SpinBox::leaveEvent(QEvent* e)
{
	StopTimer();
	QSpinBox::leaveEvent(e);
}

/// <summary>
/// Start the timer in response to the right mouse button being pressed.
/// </summary>
void SpinBox::StartTimer()
{
	s_Timer.stop();
	connect(&s_Timer, SIGNAL(timeout()), this, SLOT(OnTimeout()));
	s_Timer.start(300);
}

/// <summary>
/// Stop the timer in response to the left mouse button being pressed.
/// </summary>
void SpinBox::StopTimer()
{
	s_Timer.stop();
	disconnect(&s_Timer, SIGNAL(timeout()), this, SLOT(OnTimeout()));
}
