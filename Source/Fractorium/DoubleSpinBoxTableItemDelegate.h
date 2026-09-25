#pragma once

#include "FractoriumPch.h"
#include "DoubleSpinBox.h"

/// <summary>
/// DoubleSpinBoxTableItemDelegate class.
/// </summary>

/// <summary>
/// Used for showing a DoubleSpinBox on the cell of a QTableView when the user enters the cell to edit it.
/// </summary>
class DoubleSpinBoxTableItemDelegate
	: public QItemDelegate
{
	Q_OBJECT
public:
	/// <summary>
	/// Constructor that assigns a DoubleSpinBox.
	/// </summary>
	/// <param name="title">The DoubleSpinBox to use throughought the life of the object</param>
	/// <param name="parent">The parent widget. Default: nullptr.</param>
	explicit DoubleSpinBoxTableItemDelegate(DoubleSpinBox* spinBox, QObject* parent = nullptr)
		: QItemDelegate(parent),
		  m_SpinBox(spinBox)
	{
	}

	/// <summary>
	/// Re-parent and return the DoubleSpinBox to display when the user clicks on a cell and it enters edit mode.
	/// The re-parenting is done so that the DoubleSpinBox appears directly on top of the cell.
	/// </summary>
	/// <param name="parent">The parent cell</param>
	/// <param name="option">Ignored</param>
	/// <param name="index">Ignored</param>
	/// <returns>The DoubleSpinBox member</returns>
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		m_SpinBox->setParent(parent);
		return m_SpinBox;
	}

	/// <summary>
	/// Prevent DoubleSpinBox control from being destroyed when the cell loses focus.
	/// </summary>
	/// <param name="editor">Ignored</param>
	/// <param name="index">Ignored</param>
	void destroyEditor(QWidget* editor, const QModelIndex& index) const override
	{
	}

	/// <summary>
	/// Set the value of the DoubleSpinBox as well as its tableindex property.
	/// </summary>
	/// <param name="editor">Ignored</param>
	/// <param name="index">Ignored</param>
	void setEditorData(QWidget* editor, const QModelIndex& index) const override
	{
		const QPoint p(index.row(), index.column());
		const auto value = index.model()->data(index, Qt::ItemDataRole::EditRole).toDouble();
		m_SpinBox->setProperty("tableindex", p);
		m_SpinBox->setValue(value);
	}

	/// <summary>
	/// Set the cell in the model to the value of the DoubleSpinBox.
	/// </summary>
	/// <param name="editor">Ignored</param>
	/// <param name="model">The model whose value will be set</param>
	/// <param name="index">The cell index of the model</param>
	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
	{
		model->setData(index, m_SpinBox->value(), Qt::ItemDataRole::EditRole);
	}

	/// <summary>
	/// Set the geometry of the DoubleSpinBox to match the cell being edited.
	/// </summary>
	/// <param name="editor">The DoubleSpinBox member</param>
	/// <param name="option">Contains the rectangle to be used for the geometry of the DoubleSpinBox</param>
	/// <param name="index">Ignored</param>
	void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		editor->setGeometry(option.rect);
	}

private:
	DoubleSpinBox* m_SpinBox;
};