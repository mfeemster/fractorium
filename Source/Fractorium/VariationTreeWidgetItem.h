#pragma once

#include "FractoriumPch.h"
#include "DoubleSpinBox.h"

/// <summary>
/// VariationTreeWidgetItem class.
/// </summary>

/// <summary>
/// A derivation of QTreeWidgetItem which helps us with sorting.
/// This is used when the user chooses to sort the variations tree
/// by index or by weight. It supports weights less than, equal to, or
/// greater than zero.
/// </summary>
class VariationTreeWidgetItem : public QTreeWidgetItem
{
public:
	/// <summary>
	/// Constructor that takes a pointer to a QTreeWidget as the parent
	/// and passes it to the base.
	/// </summary>
	/// <param name="id">The ID of the variation this widget will represent</param>
	/// <param name="p">The parent widget</param>
	VariationTreeWidgetItem(eVariationId id, QTreeWidget* p = nullptr)
		: QTreeWidgetItem(p)
	{
		m_Id = id;
	}

	/// <summary>
	/// Constructor that takes a pointer to a QTreeWidgetItem as the parent
	/// and passes it to the base.
	/// This is used for making sub items for parametric variation parameters.
	/// </summary>
	/// <param name="id">The ID of the variation this widget will represent</param>
	/// <param name="p">The parent widget</param>
    VariationTreeWidgetItem(eVariationId id, QTreeWidgetItem* p = nullptr)
		: QTreeWidgetItem(p)
	{
		m_Id = id;
	}

	//virtual ~VariationTreeWidgetItem() { }
	eVariationId Id() const { return m_Id; }

private:
	/// <summary>
	/// Less than operator used for sorting.
	/// </summary>
	/// <param name="other">The QTreeWidgetItem to compare against for sorting</param>
	/// <returns>True if this is less than other, else false.</returns>
	bool operator < (const QTreeWidgetItem& other) const override
	{
		const auto column = treeWidget()->sortColumn();
		auto itemWidget1 = treeWidget()->itemWidget(const_cast<VariationTreeWidgetItem*>(this), 1);//Get the widget for the second column.

		if (auto spinBox1 = dynamic_cast<VariationTreeDoubleSpinBox*>(itemWidget1))//Cast the widget to the VariationTreeDoubleSpinBox type.
		{
			auto itemWidget2 = treeWidget()->itemWidget(const_cast<QTreeWidgetItem*>(&other), 1);//Get the widget for the second column of the widget item passed in.

			if (auto spinBox2 = dynamic_cast<VariationTreeDoubleSpinBox*>(itemWidget2))//Cast the widget to the VariationTreeDoubleSpinBox type.
			{
				if (spinBox1->IsParam() || spinBox2->IsParam())//Do not sort params, their order will always remain the same.
					return false;

				const auto weight1 = spinBox1->value();
				const auto weight2 = spinBox2->value();
				const auto index1 = spinBox1->GetVariationId();
				const auto index2 = spinBox2->GetVariationId();

				if (column == 0)//First column clicked, sort by variation index.
				{
					return index1 < index2;
				}
				else if (column == 1)//Second column clicked, sort by weight.
				{
					if (IsNearZero(weight1) && IsNearZero(weight2))
						return index1 > index2;
					else
						return std::abs(weight1) < fabs(weight2);
				}
			}
		}

		return false;
	}

	eVariationId m_Id;
};
