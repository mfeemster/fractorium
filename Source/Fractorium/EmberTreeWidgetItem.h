#pragma once

#include "FractoriumPch.h"

#define INDEX_COL 0
#define NAME_COL 1

template <typename T> class FractoriumEmberController;

/// <summary>
/// EmberTreeWidgetItem
/// </summary>

/// <summary>
/// A thin derivation of QTreeWidgetItem for a tree of embers in an open file.
/// The tree is intended to contain one open ember file at a time.
/// This is a non-templated base for casting purposes.
/// </summary>
class EmberTreeWidgetItemBase : public QTreeWidgetItem
{
public:
	friend FractoriumEmberController<float>;

#ifdef DO_DOUBLE
	friend FractoriumEmberController<double>;
#endif

	/// <summary>
	/// Constructor that takes a pointer to a QTreeWidget as a parent widget.
	/// This is meant to be a root level item.
	/// </summary>
	/// <param name="p">The parent widget of this item</param>
	explicit EmberTreeWidgetItemBase(QTreeWidget* p)
		: QTreeWidgetItem(p)
	{
	}

	/// <summary>
	/// Constructor that takes a pointer to a QTreeWidgetItem as a parent widget.
	/// This is meant to be the child of a root level item.
	/// </summary>
	/// <param name="p">The parent widget of this item</param>
	explicit EmberTreeWidgetItemBase(QTreeWidgetItem* p)
		: QTreeWidgetItem(p)
	{
	}

	~EmberTreeWidgetItemBase()
	{
		//qDebug() << "~EmberTreeWidgetItemBase()";
	}

	/// <summary>
	/// Set the preview image for the tree widget item.
	/// </summary>
	/// <param name="v">The vector containing the RGB pixels [0..255] which will make up the preview image</param>
	/// <param name="width">The width of the image in pixels</param>
	/// <param name="height">The height of the image in pixels</param>
	void SetImage(vector<unsigned char>& v, uint width, uint height)
	{
		constexpr auto size = PREVIEW_SIZE;
		m_Image = QImage(width, height, QImage::Format_RGBA8888);
		memcpy(m_Image.scanLine(0), v.data(), SizeOf(v));//Memcpy the data in.
		m_Pixmap = QPixmap::fromImage(m_Image).scaled(QSize(size, size), Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation);//Create a QPixmap out of the QImage, scaled to size.
		setData(NAME_COL, Qt::DecorationRole, m_Pixmap);
	}

	void SetRendered()
	{
		m_Rendered = true;
	}

protected:
	QImage m_Image;
	QPixmap m_Pixmap;
	bool m_Rendered = false;
};

/// <summary>
/// A thin derivation of QTreeWidgetItem for a tree of embers in an open file.
/// The tree is intended to contain one open ember file at a time.
/// </summary>
template <typename T>
class EmberTreeWidgetItem : public EmberTreeWidgetItemBase
{
public:
	/// <summary>
	/// Constructor that takes a pointer to an ember and a QTreeWidget as a parent widget.
	/// This is meant to be a root level item.
	/// </summary>
	/// <param name="ember">A pointer to the ember this item will represent</param>
	/// <param name="p">The parent widget of this item</param>
	explicit EmberTreeWidgetItem(Ember<T>* ember, QTreeWidget* p = nullptr)
		: EmberTreeWidgetItemBase(p),
		  m_Ember(ember)
	{
		setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsDragEnabled);
		setCheckState(NAME_COL, Qt::CheckState::Unchecked);
	}

	/// <summary>
	/// Constructor that takes a pointer to an ember and a QTreeWidgetItem as a parent widget.
	/// This is meant to be the child of a root level item.
	/// </summary>
	/// <param name="ember">A pointer to the ember this item will represent</param>
	/// <param name="p">The parent widget of this item</param>
	explicit EmberTreeWidgetItem(Ember<T>* ember, QTreeWidgetItem* p = nullptr)
		: EmberTreeWidgetItemBase(p),
		  m_Ember(ember)
	{
		setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsDragEnabled);
		setCheckState(NAME_COL, Qt::CheckState::Unchecked);
	}

	/// <summary>
	/// Copy the text of the tree item to the name of the ember.
	/// </summary>
	void UpdateEmberName() { m_Ember->m_Name = text(NAME_COL).toStdString(); }

	/// <summary>
	/// Set the text of the tree item.
	/// </summary>
	void UpdateEditText() { setText(NAME_COL, QString::fromStdString(m_Ember->m_Name)); }

	/// <summary>
	/// Get a pointer to the ember held by the tree item.
	/// </summary>
	Ember<T>* GetEmber() const { return m_Ember; }

	/// <summary>
	/// Perform a deep copy from the passed in ember to the dereferenced
	/// ember pointer of the tree item.
	/// </summary>
	/// <param name="ember">The ember to copy</param>
	void SetEmber(Ember<T>& ember) { *m_Ember = ember; }

	/// <summary>
	/// Set the ember pointer member to point to the passed in ember pointer.
	/// </summary>
	/// <param name="ember">The ember to point to</param>
	void SetEmberPointer(Ember<T>* ember) { m_Ember = ember; }

private:
	Ember<T>* m_Ember;
};
