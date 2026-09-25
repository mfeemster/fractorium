#include "FractoriumPch.h"
#include "LibraryTreeWidget.h"
#include "Fractorium.h"

/// <summary>
/// Set a pointer to the main window.
/// </summary>
/// <param name="f">Pointer to the main Fractorium object</param>
void LibraryTreeWidget::SetMainWindow(Fractorium* f)
{
	m_Fractorium = f;
}

/// <summary>
/// Process the drop event to allow for moving items around inside of the tree.
/// </summary>
/// <param name="de">Pointer to the QDropEvent object</param>
void LibraryTreeWidget::dropEvent(QDropEvent* de)
{
	const auto droppedIndex = indexAt(de->position().toPoint());
	const auto items = selectionModel()->selectedRows();

	if (!droppedIndex.isValid())//Don't process drop because it's outside of the droppable area.
	{
		de->ignore();
		return;
	}
	else if (!items.empty())//Actually do the drop and move the item to a new location.
	{
		// get the list of the items that are about to be dragged
		int i, row = droppedIndex.row();
		DropIndicatorPosition dp = dropIndicatorPosition();
		QList<QTreeWidgetItem*> dragItems = selectedItems();

		if (dp == QAbstractItemView::BelowItem)
			row++;

		const auto itemat = this->itemFromIndex(droppedIndex);
		QTreeWidget::dropEvent(de);//This internally changes the order of the items.

		//Qt has a long standing major bug that rearranges the order of disjoint selections when
		//The drop location is in between the disjoint regions.
		//This is an attempt to correct for that bug by removing the dropped items, then re-inserting them
		//in the order they were selected.
		//This bug remains present as of Qt 5.8: https://bugreports.qt.io/browse/QTBUG-45320
		if (const auto top = topLevelItem(0))
		{
			if (itemat)
			{
				const auto offsetitem = this->indexFromItem(itemat);

				if (dp == QAbstractItemView::BelowItem)
				{
					const auto itemrow = offsetitem.row() + 1;

					for (i = 0; i < dragItems.size(); i++)
					{
						if (itemrow < top->childCount())
							top->takeChild(itemrow);
					}

					for (i = 0; i < dragItems.size(); i++)
					{
						const auto offset = i + itemrow;

						if (offset <= top->childCount())
							top->insertChild(offset, dragItems[i]);
					}
				}
				else
				{
					const auto itemrow = offsetitem.row();//Will be at least 1 if dropped above it.
					auto offset = itemrow;

					for (i = 0; i < dragItems.size() && offset > 0; i++)
					{
						offset--;

						if (offset < top->childCount())
							top->takeChild(offset);
					}

					for (i = 0; i < dragItems.size(); i++)
					{
						if (offset <= top->childCount())
							top->insertChild(offset, dragItems[i]);

						offset++;
					}
				}
			}
		}

		m_Fractorium->m_Controller->MoveLibraryItems(items, row);
	}
}

/// <summary>
/// Set a pointer to the main window.
/// </summary>
/// <param name="f">Pointer to the main Fractorium object</param>
void InfoTreeWidget::SetMainWindow(Fractorium* f)
{
	m_Fractorium = f;
}

/// <summary>
/// Called on each mouse movement while dragging, validate whether the area
/// being dragged over can be dropped on.
/// Can only drop on like (pre/reg/post) variation section of the same xform
/// of the variation being dragged.
/// </summary>
/// <param name="dme">Pointer to the drag move event</param>
void InfoTreeWidget::dragMoveEvent(QDragMoveEvent* dme)
{
	QModelIndex const index = indexAt(dme->position().toPoint());

	if (!index.isValid())//Don't process drop because it's outside of the droppable area.
	{
		dme->ignore();
		return;
	}

	QList<QTreeWidgetItem*> dragItems = selectedItems();

	if (dragItems.size())
	{
		if (auto drag0 = dragItems[0])
		{
			if (auto itemat = itemFromIndex(index))
			{
				const auto dragpre = drag0->text(0).startsWith("pre_", Qt::CaseSensitivity::CaseInsensitive);
				const auto droppre = itemat->text(0).startsWith("pre_", Qt::CaseSensitivity::CaseInsensitive);
				const auto dragpost = drag0->text(0).startsWith("post_", Qt::CaseSensitivity::CaseInsensitive);
				const auto droppost = itemat->text(0).startsWith("post_", Qt::CaseSensitivity::CaseInsensitive);

				if (const auto par = itemat->parent())
				{
					if (drag0->parent() == par &&
							(par->text(0).startsWith("xform ", Qt::CaseSensitivity::CaseInsensitive) ||
							 par->text(0).startsWith("final", Qt::CaseSensitivity::CaseInsensitive)))
					{
                        if (dynamic_cast<const VariationTreeWidgetItem*>(itemat) != nullptr)
						{
							bool const dopre = dragpre && droppre;
							bool const dopost = dragpost && droppost;
							bool const doreg = !dragpre && !droppre && !dragpost && !droppost;

							if (dopre || doreg || dopost)
							{
								QTreeWidget::dragMoveEvent(dme);
								return;
							}
						}
					}
				}
			}
		}
	}

	dme->ignore();
}

/// <summary>
/// Process the drop event to allow for moving items around inside of the tree.
/// This will only allow variations to be moved around within the variation section of the tree
/// and nowhere else.
/// </summary>
/// <param name="de">Pointer to the QDropEvent object</param>
void InfoTreeWidget::dropEvent(QDropEvent* de)
{
	QModelIndex const droppedIndex = indexAt(de->position().toPoint());
	auto items = selectionModel()->selectedRows();

	if (!droppedIndex.isValid())//Don't process drop because it's outside of the droppable area.
	{
		de->ignore();
		return;
	}
	else if (!items.empty())//Actually do the drop and move the item to a new location.
	{
		QList<QTreeWidgetItem*> dragItems = selectedItems();

		if (dragItems.size())
		{
			auto drag0 = dragItems[0];

			if (auto itemat = itemFromIndex(droppedIndex))
			{
				if (auto par = itemat->parent())
				{
                    if (dynamic_cast<const VariationTreeWidgetItem*>(itemat) != nullptr)
					{
                        if (dynamic_cast<const VariationTreeWidgetItem*>(drag0) != nullptr)
						{
							QTreeWidget::dropEvent(de);//This internally changes the order of the items.
							m_Fractorium->ReorderVariations(par);
							return;
						}
					}
				}
			}
		}

		de->ignore();
	}
}
