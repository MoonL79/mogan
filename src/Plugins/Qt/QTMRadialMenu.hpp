/******************************************************************************
 * MODULE     : QTMRadialMenu.hpp
 * DESCRIPTION: A visually appealing radial (pie) menu widget that docks
 *              in the bottom-right corner of its parent widget.
 * COPYRIGHT  : (C) 2025 Mogan Contributors
 *******************************************************************************
 * This software falls under the GNU general public license version 3 or later.
 * It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
 * in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
 ******************************************************************************/

#ifndef QTMRADIALMENU_HPP
#define QTMRADIALMENU_HPP

#include <QIcon>
#include <QPropertyAnimation>
#include <QWidget>

// ============================================================================
// QTMRadialMenuItem
// ============================================================================
struct QTMRadialMenuItem {
  QString label;
  QIcon   icon;
  QString tooltip;
  int     id;
};

// ============================================================================
// QTMRadialMenu  –  the radial ring itself (no window flags, pure child widget)
// ============================================================================
class QTMRadialMenu : public QWidget {
  Q_OBJECT
  Q_PROPERTY (qreal openProgress READ openProgress WRITE setOpenProgress)

public:
  explicit QTMRadialMenu (QWidget* parent= nullptr);
  ~QTMRadialMenu () override;

  int  addItem (const QString& label, const QIcon& icon= QIcon (),
                const QString& tooltip= QString (), int id= -1);
  void clearItems ();

  void setInnerRadius (qreal r);
  void setOuterRadius (qreal r);
  void setCenterRadius (qreal r);

  qreal innerRadius () const { return m_innerRadius; }
  qreal outerRadius () const { return m_outerRadius; }
  qreal centerRadius () const { return m_centerRadius; }

  qreal openProgress () const { return m_openProgress; }
  void  setOpenProgress (qreal p);

  /** Preferred widget size based on outer radius. */
  QSize sizeForRadius () const;

  void animateOpen ();
  void animateClose ();
  bool isOpen () const { return m_open; }

signals:
  void itemClicked (int id, int index);
  void closeRequested ();

protected:
  void paintEvent (QPaintEvent* event) override;
  void mouseMoveEvent (QMouseEvent* event) override;
  void mousePressEvent (QMouseEvent* event) override;
  void mouseReleaseEvent (QMouseEvent* event) override;

private:
  int sectorAtPos (const QPoint& pos) const;

  void drawBackground (QPainter& p);
  void drawSectors (QPainter& p);
  void drawCenterCircle (QPainter& p);
  void drawLabelsAndIcons (QPainter& p);

  QVector<QTMRadialMenuItem> m_items;

  qreal m_innerRadius;
  qreal m_outerRadius;
  qreal m_centerRadius;
  qreal m_openProgress;
  bool  m_open;

  int m_hoveredIndex;
  int m_pressedIndex;

  QPropertyAnimation* m_anim;
};

// ============================================================================
// QTMRadialMenuDock  –  floating overlay that lives in the parent's
//                        bottom-right corner.  Contains a trigger button
//                        and the radial menu.
// ============================================================================
class QTMRadialMenuDock : public QWidget {
  Q_OBJECT
  Q_PROPERTY (
      qreal triggerRotation READ triggerRotation WRITE setTriggerRotation)

public:
  explicit QTMRadialMenuDock (QWidget* parent);
  ~QTMRadialMenuDock () override;

  QTMRadialMenu* menu () const { return m_menu; }

  qreal triggerRotation () const { return m_triggerRotation; }
  void  setTriggerRotation (qreal r);

signals:
  void itemClicked (int id, int index);
  void aboutToExpand (); // Emitted before menu expands

protected:
  bool eventFilter (QObject* obj, QEvent* event) override;
  void paintEvent (QPaintEvent* event) override;
  void mousePressEvent (QMouseEvent* event) override;
  void mouseMoveEvent (QMouseEvent* event) override;
  void mouseReleaseEvent (QMouseEvent* event) override;

private:
  void reposition ();
  void toggle ();
  void updateMask ();

  QTMRadialMenu*      m_menu;
  QPropertyAnimation* m_triggerAnim;
  qreal               m_triggerRotation;
  int                 m_triggerSize;
  int                 m_margin;
  bool                m_expanded;
};

#endif // QTMRADIALMENU_HPP
