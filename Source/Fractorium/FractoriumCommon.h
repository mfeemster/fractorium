#pragma once

#include "FractoriumPch.h"
#include "FractoriumSettings.h"
#include "PaletteTableWidgetItem.h"

/// <summary>
/// Fractorium global utility functions.
/// </summary>
#define PALETTE_CELL_HEIGHT 16

/// <summary>
/// Setup a spinner to be placed in a table cell.
/// Due to a serious compiler bug in MSVC, this must be declared as an outside function instead of a static member of Fractorium.
/// The reason is that the default arguments of type valType will not be interpreted correctly by the compiler.
/// If the bug is ever fixed, put it back as a static member function.
/// </summary>
/// <param name="table">The table the spinner belongs to</param>
/// <param name="receiver">The receiver object</param>
/// <param name="row">The row in the table where this spinner resides</param>
/// <param name="col">The col in the table where this spinner resides</param>
/// <param name="spinBox">Double pointer to spin box which will hold the spinner upon exit</param>
/// <param name="height">The height of the spinner</param>
/// <param name="min">The minimum value of the spinner</param>
/// <param name="max">The maximum value of the spinner</param>
/// <param name="step">The step of the spinner</param>
/// <param name="signal">The signal the spinner emits</param>
/// <param name="slot">The slot to receive the signal</param>
/// <param name="incRow">Whether to increment the row value</param>
/// <param name="val">The default value for the spinner</param>
/// <param name="doubleClickZero">When the spinner has a value of zero and is double clicked, assign this value</param>
/// <param name="doubleClickNonZero">When the spinner has a value of non-zero and is double clicked, assign this value</param>
template<typename spinType, typename valType>
static void SetupSpinner(QTableWidget* table, const QObject* receiver, int& row, int col, spinType*& spinBox, int height, valType min, valType max, valType step, const char* signal, const char* slot, bool incRow = true, valType val = 0, valType doubleClickZero = -999, valType doubleClickNonZero = -999)
{
	const auto settings = FractoriumSettings::DefInstance();
	spinBox = new spinType(table, height, step);
	spinBox->setRange(min, max);
	spinBox->setValue(val);

	if (col >= 0)
		table->setCellWidget(row, col, spinBox);

	if (string(signal) != "" && string(slot) != "")
		receiver->connect(spinBox, signal, receiver, slot, Qt::ConnectionType::QueuedConnection);

	if (doubleClickNonZero != -999 && doubleClickZero != -999)
	{
		spinBox->DoubleClick(true);
		spinBox->DoubleClickZero(static_cast<valType>(doubleClickZero));
		spinBox->DoubleClickNonZero(static_cast<valType>(doubleClickNonZero));
	}

	if (incRow)
		row++;
}

/// <summary>
/// Wrapper around QWidget::setTabOrder() to return the second widget.
/// This makes it easy to chain multiple calls without having to retype
/// all of them if the order changes or if a new widget is inserted.
/// </summary>
/// <param name="p">The parent widget that w1 and w2 belong to</param>
/// <param name="w1">The widget to come first in the tab order</param>
/// <param name="w2">The widget to come second in the tab order</param>
static QWidget* SetTabOrder(QWidget* p, QWidget* w1, QWidget* w2)
{
	p->setTabOrder(w1, w2);
	return w2;
}

/// <summary>
/// Truncates the precision of the value to the specified number of digits
/// after the decimal place.
/// </summary>
/// <param name="val">The value to truncate</param>
/// <param name="digits">The number of digits to leave after the decimal place</param>
/// <returns>The truncated value</returns>
static double TruncPrecision(double val, uint digits)
{
	const auto mult = std::pow(10, digits);
	return std::round(mult * val) / mult;
}

/// <summary>
/// Wrapper around QLocale::system().toDouble().
/// </summary>
/// <param name="s">The string to convert</param>
/// <param name="ok">Pointer to boolean which stores the success value of the conversion</param>
/// <returns>The converted value if successful, else 0.</returns>
static double ToDouble(const QString& s, bool* ok)
{
	return QLocale::system().toDouble(s, ok);
}

/// <summary>
/// Wrapper around QLocale::system().toString().
/// </summary>
/// <param name="s">The value to convert</param>
/// <returns>The string value if successful, else "".</returns>
template <typename T>
static QString ToString(T val)
{
	return QLocale::system().toString(val);
}

/// <summary>
/// Force a QString to end with the specified value.
/// </summary>
/// <param name="s">The string to append a suffix to</param>
/// <param name="e">The suffix to append</param>
/// <returns>The original string value if it already ended in e, else the original value appended with e.</returns>
template <typename T>
static QString MakeEnd(const QString& s, T e)
{
	if (!s.endsWith(e))
		return s + e;
	else
		return s;
}

/// <summary>
/// Check if a path is not empty and exists on the file system.
/// </summary>
/// <param name="s">The path to check</param>
/// <returns>True if s was not empty and existed, else false.</returns>
static bool Exists(const QString& s)
{
	return s != "" && QDir(s).exists();
}

/// <summary>
/// Convert a color to one that is displayable on any background.
/// </summary>
/// <param name="color">The color to convert</param>
/// <returns>The converted color</returns>
static QColor VisibleColor(const QColor& color)
{
	constexpr int threshold = 105;
	const auto delta = (color.red()   * 0.299) + //Magic numbers gotten from a Stack Overflow post.
					   (color.green() * 0.587) +
					   (color.blue()  * 0.114);
	return (255 - delta < threshold) ? QColor(0, 0, 0) : QColor(255, 255, 255);
}

/// <summary>
/// Determine whether an xform in an ember is linked to any other xform
/// in the ember.
/// </summary>
/// <param name="ember">The ember which contains the xform</param>
/// <param name="xform">The xform to inspect</param>
/// <returns>The index of the xform that the xform argument is linked to, else -1</returns>
template <typename T>
static intmax_t IsXformLinked(Ember<T>& ember, Xform<T>* xform)
{
	const auto count = ember.XformCount();
	const auto index = ember.GetXformIndex(xform);
	intmax_t linked = -1;
	size_t toOneCount = 0;
	size_t toZeroCount = 0;
	size_t toOneIndex = 0;
	size_t fromOneCount = 0;
	size_t fromZeroCount = 0;

	if (index >= 0)
	{
		for (size_t i = 0; i < count; i++)
		{
			if (xform->Xaos(i) == 0)
				toZeroCount++;
			else if (xform->Xaos(i) == 1)
			{
				toOneIndex = i;
				toOneCount++;
			}
		}

		if ((toZeroCount == (count - 1)) && toOneCount == 1)
		{
			for (size_t i = 0; i < count; i++)
			{
				if (auto fromXform = ember.GetXform(i))
				{
					if (fromXform->Xaos(toOneIndex) == 0)
						fromZeroCount++;
					else if (fromXform->Xaos(toOneIndex) == 1)
						fromOneCount++;
				}
			}

			if ((fromZeroCount == (count - 1)) && fromOneCount == 1)
				linked = toOneIndex;
		}
	}

	return linked;
}

/// <summary>
/// Convert the passed in QList<QVariant> of absolute device indices to a vector<pair<size_t, size_t>> of platform,device
/// index pairs.
/// </summary>
/// <param name="selectedDevices">The absolute device indices</param>
/// <returns>The converted device vector of platform,device index pairs</returns>
static vector<pair<size_t, size_t>> Devices(const QList<QVariant>& selectedDevices)
{
	vector<pair<size_t, size_t>> vec;
	auto& devices = OpenCLInfo::Instance()->DeviceIndices();
	vec.reserve(selectedDevices.size());

	for (int i = 0; i < selectedDevices.size(); i++)
	{
		const auto index = selectedDevices[i].toUInt();

		if (index < devices.size())
			vec.push_back(devices[index]);
	}

	return vec;
}

/// <summary>
/// Setup a table showing all available OpenCL devices on the system.
/// Create checkboxes and radio buttons which allow the user to specify
/// which devices to use, and which one to make the primary device.
/// Used in the options dialog and the final render dialog.
/// </summary>
/// <param name="table">The QTableWidget to setup</param>
/// <param name="settingsDevices">The absolute indices of the devices to use, with the first being the primary.</param>
static void SetupDeviceTable(QTableWidget* table, const QList<QVariant>& settingsDevices)
{
	bool primary = false;
	auto& deviceNames = OpenCLInfo::Instance()->AllDeviceNames();
	table->clearContents();
	table->setRowCount(static_cast<int>(deviceNames.size()));

    for (int i = 0; i < static_cast<int>(deviceNames.size()); i++)
	{
		const auto checkItem = new QTableWidgetItem();
		const auto radio = new QRadioButton();
		const auto deviceItem = new QTableWidgetItem(QString::fromStdString(deviceNames[i]));
		table->setItem(i, 0, checkItem);
		table->setCellWidget(i, 1, radio);
		table->setItem(i, 2, deviceItem);

		if (settingsDevices.contains(QVariant::fromValue(i)))
		{
			checkItem->setCheckState(Qt::CheckState::Checked);

			if (!primary)
			{
				radio->setChecked(true);
				primary = true;
			}
		}
		else
			checkItem->setCheckState(Qt::CheckState::Unchecked);
	}

	if (!primary && table->rowCount() > 0)//Primary was never set, so just default to the first device and hope it was the one detected as the main display.
	{
		table->item(0, 0)->setCheckState(Qt::CheckState::Checked);
		qobject_cast<QRadioButton*>(table->cellWidget(0, 1))->setChecked(true);
	}
}

/// <summary>
/// Copy the passed in selected absolute device indices to the controls on the passed in table.
/// Used in the options dialog and the final render dialog.
/// </summary>
/// <param name="table">The QTableWidget to copy values to</param>
/// <param name="settingsDevices">The absolute indices of the devices to use, with the first being the primary.</param>
static void SettingsToDeviceTable(QTableWidget* table, const QList<QVariant>& settingsDevices)
{
	if (settingsDevices.empty() && table->rowCount() > 0)
	{
		table->item(0, 0)->setCheckState(Qt::CheckState::Checked);
		qobject_cast<QRadioButton*>(table->cellWidget(0, 1))->setChecked(true);

		for (auto row = 1; row < table->rowCount(); row++)
			if (auto item = table->item(row, 0))
				item->setCheckState(Qt::CheckState::Unchecked);
	}
	else
	{
		for (auto row = 0; row < table->rowCount(); row++)
		{
			if (const auto item = table->item(row, 0))
			{
				if (settingsDevices.contains(row))
				{
					item->setCheckState(Qt::CheckState::Checked);

					if (!settingsDevices.indexOf(QVariant::fromValue(row)))
						if (const auto radio = qobject_cast<QRadioButton*>(table->cellWidget(row, 1)))
							radio->setChecked(true);
				}
				else
				{
					item->setCheckState(Qt::CheckState::Unchecked);
				}
			}
		}
	}
}

/// <summary>
/// Copy the values of the controls on the passed in table to a list of absolute device indices.
/// Used in the options dialog and the final render dialog.
/// </summary>
/// <param name="table">The QTableWidget to copy values from</param>
/// <returns>The list of absolute device indices</returns>
static QList<QVariant> DeviceTableToSettings(QTableWidget* table)
{
	QList<QVariant> devices;
	const auto rows = table->rowCount();

	for (int row = 0; row < rows; row++)
	{
		const auto checkItem = table->item(row, 0);
		const auto radio = qobject_cast<QRadioButton*>(table->cellWidget(row, 1));

		if (checkItem->checkState() == Qt::CheckState::Checked)
		{
			if (radio && radio->isChecked())
				devices.push_front(row);
			else
				devices.push_back(row);
		}
	}

	return devices;
}

/// <summary>
/// Ensure device selection on the passed in table make sense.
/// </summary>
/// <param name="table">The QTableWidget to setup</param>
/// <param name="row">The row of the cell</param>
/// <param name="col">The column of the cell</param>
static void HandleDeviceTableCheckChanged(QTableWidget* table, int row, int col)
{
	auto primaryRow = -1;

	for (auto i = 0; i < table->rowCount(); i++)
	{
		if (const auto radio = qobject_cast<QRadioButton*>(table->cellWidget(i, 1)))
		{
			if (radio->isChecked())
			{
				primaryRow = i;
				break;
			}
		}
	}

	if (primaryRow == -1)
		primaryRow = 0;

	if (const auto primaryItem = table->item(primaryRow, 0))
		if (primaryItem->checkState() == Qt::CheckState::Unchecked)
			primaryItem->setCheckState(Qt::CheckState::Checked);
}

/// <summary>
/// Set a row in a table to represent a palette.
/// This will place the palette name as a string value in the first column,
/// and a QPixmap representing the palette in the second column.
/// </summary>
/// <param name="paletteTable">The table write to the row to</param>
/// <param name="palette">A pointer to the palette to write to the row</param>
/// <param name="row">The row to write the palette to</param>
static void AddPaletteToTable(QTableWidget* paletteTable, Palette<float>* palette, int row)
{
	const auto v = palette->MakeRgbPaletteBlock(PALETTE_CELL_HEIGHT);
	auto nameCol = std::make_unique<QTableWidgetItem>(palette->m_Name.c_str());
	nameCol->setToolTip(palette->m_Name.c_str());
	nameCol->setFlags(palette->m_SourceColors.empty() ? (Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable)
					  : (Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsSelectable));
	paletteTable->setItem(row, 0, nameCol.release());
	const QImage image(v.data(), static_cast<int>(palette->Size()), PALETTE_CELL_HEIGHT, QImage::Format_RGB888);
	auto paletteItem = std::make_unique<PaletteTableWidgetItem>(palette);
	paletteItem->setData(Qt::ItemDataRole::DecorationRole, QPixmap::fromImage(image));
	paletteItem->setFlags(paletteItem->flags() & ~Qt::ItemFlag::ItemIsEditable);
	paletteTable->setItem(row, 1, paletteItem.release());
}

/// <summary>
/// Read a palette Xml file and populate the palette table with the contents.
/// This will clear any previous contents.
/// Called upon initialization, palette combo index change, and controller type change.
/// </summary>
/// <param name="s">The name of the palette file without the path</param>
/// <param name="paletteTable">The table to populate</param>
/// <param name="paletteList">The global PaletteList shared_ptr to retrieve the specified palette file from to populate the table with</param>
/// <returns>True if successful, else false.</returns>
static bool FillPaletteTable(const string& s, QTableWidget* paletteTable, shared_ptr<PaletteList<float>> paletteList)
{
	if (!s.empty())//This occasionally seems to get called with an empty string for reasons unknown.
	{
		if (const auto palettes = paletteList->GetPaletteListByFilename(s))
		{
			paletteTable->clear();
			paletteTable->blockSignals(true);
			paletteTable->setRowCount(static_cast<int>(palettes->size()));
			//Headers get removed when clearing, so must re-create here.
			auto nameHeader = std::make_unique<QTableWidgetItem>("Name");
			auto paletteHeader = std::make_unique<QTableWidgetItem>("Palette");
			nameHeader->setTextAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);
			paletteHeader->setTextAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);
			paletteTable->setHorizontalHeaderItem(0, nameHeader.release());
			paletteTable->setHorizontalHeaderItem(1, paletteHeader.release());

			//Palette list table.
            for (auto i = 0; i < static_cast<int>(palettes->size()); i++)
				if (const auto palette = &(*palettes)[i])
					AddPaletteToTable(paletteTable, palette, i);

			paletteTable->blockSignals(false);
			return true;
		}
	}

	return false;
}

/// <summary>
/// Get the default search paths for config and palette files.
/// </summary>
/// <returns>vector<QString> of paths</returns>
static vector<QString> GetDefaultPaths()
{
	static vector<QString> paths =
	{
		(QDir::homePath() + "/AppData/Roaming/Fractorium").toLocal8Bit().data(),
#ifndef _WIN32
		QString("/usr/share/fractorium").toLocal8Bit().data(),
		QString("/usr/local/share/fractorium").toLocal8Bit().data(),
		(QDir::homePath() + "/.config/fractorium").toLocal8Bit().data(),
#endif
		QDir::currentPath().toLocal8Bit().data(),
		QCoreApplication::applicationDirPath().toLocal8Bit().data()
	};
	return paths;
}

/// <summary>
/// Get the default user path for config and palette files.
/// </summary>
/// <returns>vector<QString> of paths</returns>
static QString GetDefaultUserPath()
{
#ifdef _WIN32
	return (QDir::homePath() + "/AppData/Roaming/Fractorium").toLocal8Bit().data();
#else
	return (QDir::homePath() + "/.config/fractorium").toLocal8Bit().data();
#endif
}

/// <summary>
/// Get the first flam3-palettes.xml file in the default search paths.
/// </summary>
/// <returns>The full path and filename if found, else empty string.</returns>
static QString FindFirstDefaultPalette()
{
	const auto paths = GetDefaultPaths();

	for (auto& path : paths)
	{
		const auto full = path + "/flam3-palettes.xml";

		if (QFile::exists(full))
			return full;
	}

	return "";
}

/// <summary>
/// The basic style that is needed for things to look right, this varies by OS.
/// </summary>
/// <returns>The base style</returns>
static QString BaseStyle()
{
	return "/*---Base Style---\n"
		   "This may be needed to deal with the large tabs.\n"
		   "You should keep this at the top of whatever custom style you make to ensure the tabs aren't unusually large.*/\n"
		   "/*\n"
#ifndef _WIN32
		   "QTabBar::tab { height: 4ex; }\n"
#else
		   "QTabBar::tab { height: 4ex; }\n"
#endif
		   "*/\n"
		   "*\n"
		   "{\n"
		   "\tfont: 8.5pt \"MS Shell Dlg 2\";/*For some reason the font changes if you set any style. Set this to whatever font is the default on your system*/\n"
		   "}\n"
		   "\n"
		   "/*This is needed to give the labels on the status bar some padding.*/\n"
		   "QStatusBar QLabel { padding-left: 2px; padding-right: 2px; }\n"
		   "\n"
		   "Fractorium\n"
		   "{\n"
		   "\tqproperty-VariationTreeColorNonZero:black;\n"
		   "\tqproperty-VariationTreeColorZero:black;\n"
		   "\tqproperty-VariationTreeBgColorNonZero:lightgray;\n"
		   "\tqproperty-VariationTreeBgColorZero:white;\n"
		   "}\n"
		   "\n"
		   "/*For some reason, Qt does not draw table headers correctly, so the style must always be manually specified.*/\n"
		   "QHeaderView::section::vertical:enabled\n"
		   "{\n"
		   "\tcolor: black;\n"
		   "\tbackground-color: lightgray;\n"
		   "\tborder: none;\n"
		   "\tborder-bottom: 1px solid gray;\n"
		   "\tpadding-left: 5px;\n"
		   "\tpadding-right: 0px;\n"
		   "\tpadding-top: 0px;\n"
		   "\tpadding-bottom: 0px;\n"
		   "}\n"
		   "\n"
		   "QHeaderView::section::horizontal:enabled\n"
		   "{\n"
		   "\tcolor: black;\n"
		   "\tbackground-color: lightgray;\n"
		   "\tborder: 1px solid darkgray;\n"
		   "\tborder-right: 1px solid gray;\n"
		   "\tpadding-top: 0px;\n"
		   "\tpadding-bottom: 0px;\n"
		   "\tpadding-left: 3px;\n"
		   "}\n"
		   "\n"
		   "QHeaderView::section::vertical:disabled\n"
		   "{\n"
		   "\tcolor: rgb(35, 35, 35);\n"
		   "\tbackground-color: rgb(53, 53, 53);\n"
		   "\tborder: 0px none darkgray;\n"
		   "\tborder-bottom: 1px solid rgb(53, 53, 53);\n"
		   "\tpadding-left: 5px;\n"
		   "\tpadding-right: 0px;\n"
		   "\tpadding-top: 0px;\n"
		   "\tpadding-bottom: 0px;\n"
		   "}\n"
		   "\n"
		   "QHeaderView::section::horizontal:disabled\n"
		   "{\n"
		   "\tcolor:rgb(35, 35, 35);\n"
		   "\tbackground-color: rgb(53, 53, 53);\n"
		   "\tborder: 0px none darkgray;\n"
		   "\tborder-right: 0px solid rgb(53, 53, 53);\n"
		   "\tpadding-top: 0px;\n"
		   "\tpadding-bottom: 0px;\n"
		   "\tpadding-left: 3px;\n"
		   "}\n"
		   ;
}

/// <summary>
/// Get all parent objects of the passed in widget.
/// </summary>
/// <param name="widget">The widget whose parents will be retrieved</param>
/// <returns>The entire parent object chain in a QList</returns>
template <typename T>
static QList<T> GetAllParents(QWidget* widget)
{
	QList<T> parents;

	if (widget != nullptr)
	{
		while (const auto parent = qobject_cast<QWidget*>(widget->parent()))
		{
			if (const auto parentT = qobject_cast<T>(parent))
				parents.push_back(parentT);

			widget = parent;
		}
	}

	return parents;
}

/// <summary>
/// Constrain the value in low to be less than or equal to the value in high.
/// Template expected to be any control which has member functions value() and setValue().
/// Most likely QSpinBox or QDoubleSpinbox.
/// </summary>
/// <param name="low">The control which must contain the lower value</param>
/// <param name="high">The control which must contain the higher value</param>
/// <returns>True if the value of low had to be changed, else false.</returns>
template <typename T>
bool ConstrainLow(T* low, T* high)
{
	if (low->value() > high->value())
	{
		low->blockSignals(true);
		low->setValue(high->value());
		low->blockSignals(false);
		return true;
	}

	return false;
}

/// <summary>
/// Constrain the value in high to be greater than or equal to the value in low.
/// Template expected to be any control which has member functions value() and setValue().
/// Most likely QSpinBox or QDoubleSpinbox.
/// </summary>
/// <param name="low">The control which must contain the lower value</param>
/// <param name="high">The control which must contain the higher value</param>
/// <returns>True if the value of high had to be changed, else false.</returns>
template <typename T>
bool ConstrainHigh(T* low, T* high)
{
	if (high->value() < low->value())
	{
		high->blockSignals(true);
		high->setValue(low->value());
		high->blockSignals(false);
		return true;
	}

	return false;
}

/// <summary>
/// Move all, possibly disjointly, selected items in a range to
/// a new location and update all existing locations.
/// Atribution: Sean Parent, Going Native 2013.
/// </summary>
/// <param name="f">The location of the first selected item, or the start of the collection.</param>
/// <param name="l">The location of the last selected item, or the end of the collection.</param>
/// <returns>A pair of iterators representing the start and end locations of the list of newly moved items</returns>
template <typename I, typename S>
pair<I, I> Gather(I f, I l, I p, S s)
{
	return
	{
		stable_partition(f, p, [&](const typename iterator_traits<I>::value_type & x) { return !s(x); }),
		stable_partition(p, l, s)
	};
}
