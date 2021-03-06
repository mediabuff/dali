#ifndef __DALI_TOOLKIT_TABLE_VIEW_H__
#define __DALI_TOOLKIT_TABLE_VIEW_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// EXTERNAL INCLUDES
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/actors/actor-enumerations.h>

// INTERNAL INCLUDES
#include <dali-toolkit/public-api/controls/control.h>

namespace Dali
{

namespace Toolkit
{

namespace Internal DALI_INTERNAL
{
class TableView;
}
/**
 * @addtogroup dali_toolkit_controls_table_view
 * @{
 */

/**
 * @brief TableView is a layout container for aligning child actors in a grid like layout.
 *
 * TableView constrains the x and y position and width and height of the child actors.
 * z position and depth are left intact so that 3D model actors can also be laid out
 * in a grid without loosing their depth scaling.
 *
 * @nosubgrouping
 * <h3>Per-child Custom properties for script supporting:</h3>
 *
 * When an actor is add to the tableView through Actor::Add() instead of TableView::AddChild,
 * the following custom properties of the actor are checked to decide the actor position inside the table.
 *
 * These properties are registered dynamically to the child and is non-animatable.
 *
 * | %Property Name            | Type        |
 * |---------------------------|-------------|
 * | cell-index                | Vector2     |
 * | row-span                  | float       |
 * | column-span               | float       |
 * | cell-horizontal-alignment | string      |
 * | cell-vertical-alignment   | string      |
 *
 * The row-span or column span has integer value, but its type is float here due to the limitation of the builder's ability to differentiate integer and float from Json string.
 * The available values for cell-horizontal-alignment are: left, center, right.
 * The available values for cell-vertical-alignment are: top, center, bottom.
 *
 * @code
 * "name":"gallery-1",
 * "type":"ImageActor",
 * "image": {
 *    "filename": "{DALI_IMAGE_DIR}gallery-small-1.jpg"
 *  },
 *  "custom-properties": {
 *     "cell-index":[1,1],  // property to specify the top-left cell this child occupies, if not set, the first available cell is used
 *     "row-span":3,        // property to specify how many rows this child occupies, if not set, default value is 1
 *     "column-span": 2,    // property to specify how many columns this child occupies, if nor set, default value is 1
 *     "cell-horizontal-alignment": "left", // property to specify how to align horizontally inside the cells, if not set, default value is 'left'
 *     "cell-vertical-alignment": "center"  // property to specify how to align vertically inside the cells, if not set, default value is 'top'
 *   }
 * @endcode
 */
class DALI_IMPORT_API TableView : public Control
{
public:

  /**
   * @brief The start and end property ranges for this control.
   */
  enum PropertyRange
  {
    PROPERTY_START_INDEX = Control::CONTROL_PROPERTY_END_INDEX + 1,
    PROPERTY_END_INDEX =   PROPERTY_START_INDEX + 1000              ///< Reserve property indices
  };

  /**
   * @brief An enumeration of properties belonging to the TableView class.
   *
   * LayoutRows: set the height of the rows.
   * It has the format as follows in script:
   * @code
   * "layout-rows":
      {
        "0": { "policy": "fixed", "value": 40 },       //@see SetFixedHight
        "2": { "policy": "relative", "value": 0.33 },  //@see SetRelativeHeight
        "3": { "policy": "fit", "value":0.0 }          //@see SetFitHeight, the value is not used, its height is decided by the children in this row
      }
   * @endcode
   *
   * LayoutColumns: set the height of the rows.
   * It has the format as follows in script:
   * @code
   * "layout-columns":
      {
        "0": { "policy": "fixed", "value": 40 },       //@see SetFixedWidth
        "1": { "policy": "fit", "value":0.0 }          //@see SetFitHeight, the value is not used, its width is decided by the children in this column
        "2": { "policy": "relative", "value": 0.33 }   //@see SetRelativeWidth
      }
   * @endcode
   */
  struct Property
  {
    enum
    {
      ROWS = PROPERTY_START_INDEX, ///< name "rows",           type unsigned int
      COLUMNS,                     ///< name "columns",        type unsigned int
      CELL_PADDING,                ///< name "cell-padding",   type Vector2
      LAYOUT_ROWS,                 ///< name "layout-rows",    type Map
      LAYOUT_COLUMNS,              ///< name "layout-columns", type Map
    };
  };

  /**
   * @brief Describes how the size of a row / column been set
   */
  enum LayoutPolicy
  {
    FIXED,      ///< Fixed with the given value.
    RELATIVE,   ///< Calculated as percentage of the remainder after subtracting Padding and Fixed height/width
    FILL,       ///< Default policy, get the remainder of the 100% (after subtracting Fixed, Fit and Relative height/ width) divided evenly between 'fill' rows/columns
    FIT         ///< Fit around its children.
  };

  /**
   * Structure to specify layout position for child actor
   */
  struct CellPosition
  {
    /**
     * Constructor to initialise values to defaults for convenience
     */
    CellPosition( unsigned int rowIndex = 0, unsigned int columnIndex = 0,
                    unsigned int rowSpan = 1, unsigned int columnSpan = 1 )
    : rowIndex( rowIndex ), columnIndex( columnIndex ),
      rowSpan( rowSpan ), columnSpan( columnSpan )
    { }

    unsigned int rowIndex;
    unsigned int columnIndex;
    unsigned int rowSpan;
    unsigned int columnSpan;
  };

  /**
   * Create a TableView handle; this can be initialised with TableView::New()
   * Calling member functions with an uninitialised handle is not allowed.
   */
  TableView();

  /**
   * Copy constructor. Creates another handle that points to the same real object
   * @param handle to copy from
   */
  TableView( const TableView& handle );

  /**
   * Assignment operator. Changes this handle to point to another real object
   */
  TableView& operator=( const TableView& handle );

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~TableView();

  /**
   * Create the TableView control.
   * @param[in] initialRows for the table
   * @param[in] initialColumns for the table
   * @return A handle to the TableView control.
   */
  static TableView New( unsigned int initialRows, unsigned int initialColumns );

  /**
   * Downcast an Object handle to TableView. If handle points to a TableView the
   * downcast produces valid handle. If not the returned handle is left uninitialized.
   * @param[in] handle Handle to an object
   * @return handle to a TableView or an uninitialized handle
   */
  static TableView DownCast( BaseHandle handle );

  /**
   * Adds a child to the table
   * If the row or column index is outside the table, the table gets resized bigger
   * @pre The child actor has been initialized.
   * @param[in] child to add
   * @param[in] position for the child
   * @return true if the addition succeeded, false if the cell is already occupied
   */
  bool AddChild( Actor child, CellPosition position );

  /**
   * Returns a child from the given layout position
   * Note! if there is no child in this position this method returns an uninitialized
   * Actor handle
   * @param[in] position in the table
   * @return child that was in the cell or an uninitialized handle
   */
  Actor GetChildAt( CellPosition position );

  /**
   * Removes a child from the given layout position
   * Note! if there is no child in this position this method does nothing
   * @param[in] position for the child to remove
   * @return child that was removed or an uninitialized handle
   */
  Actor RemoveChildAt( CellPosition position );

  /**
   * Finds the childs layout position
   * @param[in] child to search for
   * @param[out] position for the child
   * @return true if the child was included in this TableView
   */
  bool FindChildPosition( Actor child, CellPosition& position );

  /**
   * Insert a new row to given index
   * @param [in] rowIndex of the new row
   */
  void InsertRow( unsigned int rowIndex );

  /**
   * Delete a row from given index
   * Removed elements are deleted
   * @param [in] rowIndex of the row to delete
   */
  void DeleteRow( unsigned int rowIndex );

  /**
   * Delete a row from given index
   * @param [in] rowIndex of the row to delete
   * @param [out] removed elements
   */
  void DeleteRow( unsigned int rowIndex, std::vector<Actor>& removed );

  /**
   * Insert a new column to given index
   * @param [in] columnIndex of the new column
   */
  void InsertColumn( unsigned int columnIndex );

  /**
   * Delete a column from given index.
   * Removed elements are deleted
   * @param [in] columnIndex of the column to delete
   */
  void DeleteColumn( unsigned int columnIndex );

  /**
   * Delete a column from given index
   * @param [in] columnIndex of the column to delete
   * @param [out] removed elements
   */
  void DeleteColumn( unsigned int columnIndex, std::vector<Actor>& removed );

  /**
   * Resize the TableView. Note! if the new size is smaller than old,
   * superfluous actors get removed. If you want to relayout removed children,
   * use the variant that returns the removed Actors and reinsert them into the table
   * If an actor spans to a removed row or column it gets removed from the table
   * @param[in] rows for the table
   * @param[in] columns for the table
   */
  void Resize( unsigned int rows, unsigned int columns );

  /**
   * Resize the TableView. Note! if the new size is smaller than old,
   * superfluous actors get removed.
   * If an actor spans to a removed row or column it gets removed from the table
   * @param[in] rows for the table
   * @param[in] columns for the table
   * @param[out] removed actor handles
   */
  void Resize( unsigned int rows, unsigned int columns, std::vector<Actor>& removed );

  /**
   * Set horizontal and vertical padding between cells
   * @param[in] padding width and height
   */
  void SetCellPadding( Size padding );

  /**
   * @return the current padding as width and height
   */
  Size GetCellPadding();

  /**
   * @brief Specify this row as fitting its height to its children
   *
   * @param[in] rowIndex The row to set
   */
  void SetFitHeight( unsigned int rowIndex );

  /**
   * @brief Is the row a fit row
   *
   * @param[in] rowIndex The row to check
   * @return Return true if the row is fit
   */
  bool IsFitHeight( unsigned int rowIndex ) const;

  /**
   * @brief Specify this column as fitting its width to its children
   *
   * @param[in] columnIndex The column to set
   */
  void SetFitWidth( unsigned int columnIndex );

  /**
   * @brief Is the column a fit column
   *
   * @param[in] columnIndex The column to check
   * @return Return true if the column is fit
   */
  bool IsFitWidth( unsigned int columnIndex ) const;

  /**
   * Sets a row to have fixed height
   * Setting a fixed height of 0 has no effect
   * @pre The row rowIndex must exist.
   * @param rowIndex for row with fixed height
   * @param height in world coordinate units
   */
  void SetFixedHeight( unsigned int rowIndex, float height );

  /**
   * Gets a row's fixed height.
   * Note! The returned value is valid if it has been set before.
   * @pre The row rowIndex must exist.
   * @return height in world coordinate units.
   */
  float GetFixedHeight( unsigned int rowIndex ) const;

  /**
   * Sets a row to have relative height. Relative height means percentage of
   * the remainder of the table height after subtracting Padding and Fixed height rows
   * Setting a relative height of 0 has no effect
   * @pre The row rowIndex must exist.
   * @param rowIndex for row with relative height
   * @param heightPercentage between 0.0f and 1.0f
   */
  void SetRelativeHeight( unsigned int rowIndex, float heightPercentage );

  /**
   * Gets a row's relative height.
   * Note! The returned value is valid if it has been set before.
   * @pre The row rowIndex must exist.
   * @return height in percentage units, between 0.0f and 1.0f.
   */
  float GetRelativeHeight( unsigned int rowIndex ) const;

  /**
   * Sets a column to have fixed width
   * Setting a fixed width of 0 has no effect
   * @pre The column columnIndex must exist.
   * @param columnIndex for column with fixed width
   * @param width in world coordinate units
   */
  void SetFixedWidth( unsigned int columnIndex, float width );

  /**
   * Gets a column's fixed width.
   * Note! The returned value is valid if it has been set before.
   * @pre The column columnIndex must exist.
   * @return width in world coordinate units.
   */
  float GetFixedWidth( unsigned int columnIndex ) const;

  /**
   * Sets a column to have relative width. Relative width means percentage of
   * the remainder of table width after subtracting Padding and Fixed width columns
   * Setting a relative width of 0 has no effect
   * @pre The column columnIndex must exist.
   * @param columnIndex for column with fixed width
   * @param widthPercentage between 0.0f and 1.0f
   */
  void SetRelativeWidth( unsigned int columnIndex, float widthPercentage );

  /**
   * Gets a column's relative width.
   * Note! The returned value is valid if it has been set before.
   * @pre The column columnIndex must exist.
   * @return width in percentage units, between 0.0f and 1.0f.
   */
  float GetRelativeWidth( unsigned int columnIndex ) const;

  /**
   * @return the amount of rows in the table
   */
  unsigned int GetRows();

  /**
   * @return the amount of columns in the table
   */
  unsigned int GetColumns();

  /**
   * @brief Set the alignment on a cell.
   *
   * Cells without calling this function have the default values of LEFT and TOP respectively.
   *
   * @param[in] position The cell to set alignment on.
   * @param[in] horizontal The horizontal alignment.
   * @param[in] vertical The vertical alignment.
   */
  void SetCellAlignment( CellPosition position, HorizontalAlignment::Type horizontal, VerticalAlignment::Type vertical );

public: // Not intended for application developers

  /**
   * Creates a handle using the Toolkit::Internal implementation.
   * @param[in]  implementation  The Control implementation.
   */
  DALI_INTERNAL TableView(Internal::TableView& implementation);

  /**
   * Allows the creation of this Control from an Internal::CustomActor pointer.
   * @param[in]  internal  A pointer to the internal CustomActor.
   */
  explicit DALI_INTERNAL TableView( Dali::Internal::CustomActor* internal );
};

/**
 * @}
 */
} // namespace Toolkit

} // namespace Dali

#endif // __DALI_TOOLKIT_TABLE_VIEW_H__
