/******************************************************************************
 * MODULE     : QTMRadialMenu.cpp
 * DESCRIPTION: A visually appealing radial (pie) menu widget that docks
 *              in the bottom-right corner of its parent widget.
 * COPYRIGHT  : (C) 2025 Mogan Contributors
 *******************************************************************************
 * This software falls under the GNU general public license version 3 or later.
 * It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
 * in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
 ******************************************************************************/

#include "QTMRadialMenu.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

// ---------------------------------------------------------------------------
// Colour palette
// ---------------------------------------------------------------------------
static const QColor kBgOuter (30, 30, 36, 230);
static const QColor kSectorNormal (52, 52, 64, 200);
static const QColor kSectorHover (0, 122, 255, 180);
static const QColor kSectorPressed (0, 90, 210, 220);
static const QColor kBorderColor (80, 80, 100, 120);
static const QColor kTextColor (230, 230, 240);
static const QColor kTextDimColor (160, 160, 180);
static const QColor kCenterNormal (45, 45, 55, 240);
static const QColor kCenterHover (60, 60, 75, 250);
static const QColor kGlowColor (0, 122, 255, 60);

static const QColor kTriggerBg (0, 122, 255, 220);
static const QColor kTriggerHover (0, 140, 255, 240);
static const QColor kTriggerIcon (255, 255, 255, 230);

// ###########################################################################
//  QTMRadialMenu
// ###########################################################################

QTMRadialMenu::QTMRadialMenu (QWidget* parent)
    : QWidget (parent), m_innerRadius (50.0), m_outerRadius (130.0),
      m_centerRadius (35.0), m_openProgress (0.0), m_open (false),
      m_hoveredIndex (-1), m_pressedIndex (-1), m_anim (nullptr) {

  setAttribute (Qt::WA_NoSystemBackground);
  setAutoFillBackground (false);
  setMouseTracking (true);
  hide ();
}

QTMRadialMenu::~QTMRadialMenu () {}

int
QTMRadialMenu::addItem (const QString& label, const QIcon& icon,
                        const QString& tooltip, int id) {
  QTMRadialMenuItem item;
  item.label  = label;
  item.icon   = icon;
  item.tooltip= tooltip;
  item.id     = (id < 0) ? m_items.size () : id;
  m_items.append (item);
  return m_items.size () - 1;
}

void
QTMRadialMenu::clearItems () {
  m_items.clear ();
  m_hoveredIndex= -1;
  m_pressedIndex= -1;
}

void
QTMRadialMenu::setInnerRadius (qreal r) {
  m_innerRadius= r;
}
void
QTMRadialMenu::setOuterRadius (qreal r) {
  m_outerRadius= r;
}
void
QTMRadialMenu::setCenterRadius (qreal r) {
  m_centerRadius= r;
}

QSize
QTMRadialMenu::sizeForRadius () const {
  int side= qRound (m_outerRadius * 2) + 40;
  return QSize (side, side);
}

void
QTMRadialMenu::setOpenProgress (qreal p) {
  m_openProgress= p;
  update ();
}

void
QTMRadialMenu::animateOpen () {
  if (m_open) return;
  m_open= true;
  setFixedSize (sizeForRadius ());
  show ();
  raise ();

  if (m_anim) {
    m_anim->stop ();
    delete m_anim;
  }
  m_anim= new QPropertyAnimation (this, "openProgress", this);
  m_anim->setDuration (350);
  m_anim->setStartValue (0.0);
  m_anim->setEndValue (1.0);
  m_anim->setEasingCurve (QEasingCurve::OutBack);
  m_anim->start ();
}

void
QTMRadialMenu::animateClose () {
  if (!m_open) return;

  if (m_anim) {
    m_anim->stop ();
    delete m_anim;
  }
  m_anim= new QPropertyAnimation (this, "openProgress", this);
  m_anim->setDuration (200);
  m_anim->setStartValue (m_openProgress);
  m_anim->setEndValue (0.0);
  m_anim->setEasingCurve (QEasingCurve::InCubic);
  connect (m_anim, &QPropertyAnimation::finished, this, [this] () {
    m_open= false;
    hide ();
  });
  m_anim->start ();
}

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------
int
QTMRadialMenu::sectorAtPos (const QPoint& pos) const {
  if (m_items.isEmpty () || m_openProgress < 0.01) return -1;

  const QPointF center (width () / 2.0, height () / 2.0);
  const QPointF delta= QPointF (pos) - center;
  const qreal   dist = qSqrt (delta.x () * delta.x () + delta.y () * delta.y ());

  if (dist < m_centerRadius * m_openProgress) return -2;

  const qreal outerR= m_outerRadius * m_openProgress;
  const qreal innerR= m_innerRadius * m_openProgress;
  if (dist > outerR || dist < innerR) return -1;

  const int   n         = m_items.size ();
  const qreal sectorSpan= 360.0 / n;

  qreal angle= qRadiansToDegrees (qAtan2 (delta.x (), -delta.y ()));
  if (angle < 0) angle+= 360.0;
  angle+= sectorSpan / 2.0;
  if (angle >= 360.0) angle-= 360.0;

  return static_cast<int> (angle / sectorSpan) % n;
}

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------
void
QTMRadialMenu::paintEvent (QPaintEvent*) {
  QPainter p (this);
  p.setRenderHint (QPainter::Antialiasing, true);
  p.setRenderHint (QPainter::SmoothPixmapTransform, true);

  drawBackground (p);
  drawSectors (p);
  drawCenterCircle (p);
  drawLabelsAndIcons (p);
}

void
QTMRadialMenu::mouseMoveEvent (QMouseEvent* event) {
  int idx= sectorAtPos (event->pos ());
  if (idx != m_hoveredIndex) {
    m_hoveredIndex= idx;
    update ();
  }
}

void
QTMRadialMenu::mousePressEvent (QMouseEvent* event) {
  if (event->button () == Qt::LeftButton) {
    m_pressedIndex= sectorAtPos (event->pos ());
    update ();
  }
}

void
QTMRadialMenu::mouseReleaseEvent (QMouseEvent* event) {
  if (event->button () == Qt::LeftButton) {
    int idx= sectorAtPos (event->pos ());
    if (idx >= 0 && idx == m_pressedIndex && idx < m_items.size ()) {
      emit itemClicked (m_items[idx].id, idx);
      emit closeRequested ();
    }
    else if (idx == -2) {
      emit closeRequested ();
    }
    m_pressedIndex= -1;
    update ();
  }
}

// ---------------------------------------------------------------------------
// Painting
// ---------------------------------------------------------------------------
void
QTMRadialMenu::drawBackground (QPainter& p) {
  const QPointF center (width () / 2.0, height () / 2.0);
  const qreal   outerR= m_outerRadius * m_openProgress;

  QRadialGradient glow (center, outerR + 20);
  glow.setColorAt (0.0, QColor (0, 122, 255, qRound (25 * m_openProgress)));
  glow.setColorAt (0.7, QColor (0, 60, 140, qRound (10 * m_openProgress)));
  glow.setColorAt (1.0, Qt::transparent);
  p.setPen (Qt::NoPen);
  p.setBrush (glow);
  p.drawEllipse (center, outerR + 20, outerR + 20);
}

void
QTMRadialMenu::drawSectors (QPainter& p) {
  const int n= m_items.size ();
  if (n == 0) return;

  const QPointF center (width () / 2.0, height () / 2.0);
  const qreal   outerR   = m_outerRadius * m_openProgress;
  const qreal   innerR   = m_innerRadius * m_openProgress;
  const qreal   spanAngle= 360.0 / n;
  const qreal   gapAngle = 1.5;

  for (int i= 0; i < n; ++i) {
    qreal startDeg= 90.0 - (i * spanAngle) - spanAngle / 2.0 + gapAngle / 2.0;
    qreal sweepDeg= spanAngle - gapAngle;

    QPainterPath path;
    QRectF       outerRect (center.x () - outerR, center.y () - outerR,
                            outerR * 2, outerR * 2);
    QRectF       innerRect (center.x () - innerR, center.y () - innerR,
                            innerR * 2, innerR * 2);

    path.arcMoveTo (outerRect, startDeg);
    path.arcTo (outerRect, startDeg, sweepDeg);
    path.arcTo (innerRect, startDeg + sweepDeg, -sweepDeg);
    path.closeSubpath ();

    QColor fillColor= kSectorNormal;
    if (i == m_pressedIndex)
      fillColor= kSectorPressed;
    else if (i == m_hoveredIndex)
      fillColor= kSectorHover;

    qreal   midAngle= (90.0 - i * spanAngle) * M_PI / 180.0;
    QPointF gradStart (center.x () + innerR * qCos (midAngle),
                       center.y () - innerR * qSin (midAngle));
    QPointF gradEnd (center.x () + outerR * qCos (midAngle),
                     center.y () - outerR * qSin (midAngle));

    QLinearGradient grad (gradStart, gradEnd);
    grad.setColorAt (0.0, fillColor.darker (110));
    grad.setColorAt (1.0, fillColor.lighter (115));

    p.setPen (QPen (kBorderColor, 0.5));
    p.setBrush (grad);
    p.drawPath (path);

    if (i == m_hoveredIndex) {
      QRadialGradient hoverGlow (center, outerR);
      hoverGlow.setColorAt (0.3, Qt::transparent);
      hoverGlow.setColorAt (0.6, kGlowColor);
      hoverGlow.setColorAt (1.0, Qt::transparent);
      p.setPen (Qt::NoPen);
      p.setBrush (hoverGlow);
      p.drawPath (path);
    }
  }
}

void
QTMRadialMenu::drawCenterCircle (QPainter& p) {
  const QPointF center (width () / 2.0, height () / 2.0);
  const qreal   r= m_centerRadius * m_openProgress;

  QRadialGradient grad (center, r);
  bool            hovered= (m_hoveredIndex == -2);
  grad.setColorAt (0.0, hovered ? kCenterHover.lighter (120) : kCenterNormal);
  grad.setColorAt (1.0, hovered ? kCenterHover : kCenterNormal.darker (120));

  p.setPen (QPen (kBorderColor, 1.0));
  p.setBrush (grad);
  p.drawEllipse (center, r, r);

  // "X" icon in center
  if (m_openProgress > 0.5) {
    qreal  alpha= (m_openProgress - 0.5) * 2.0;
    QColor xColor (kTextDimColor);
    xColor.setAlphaF (alpha * xColor.alphaF ());
    p.setPen (QPen (xColor, 2.0, Qt::SolidLine, Qt::RoundCap));
    qreal s= r * 0.3;
    p.drawLine (QPointF (center.x () - s, center.y () - s),
                QPointF (center.x () + s, center.y () + s));
    p.drawLine (QPointF (center.x () + s, center.y () - s),
                QPointF (center.x () - s, center.y () + s));
  }
}

void
QTMRadialMenu::drawLabelsAndIcons (QPainter& p) {
  const int n= m_items.size ();
  if (n == 0) return;

  const QPointF center (width () / 2.0, height () / 2.0);
  const qreal   midR     = (m_innerRadius + m_outerRadius) / 2.0 * m_openProgress;
  const qreal   spanAngle= 360.0 / n;

  for (int i= 0; i < n; ++i) {
    qreal angleDeg= i * spanAngle;
    qreal angleRad= qDegreesToRadians (angleDeg - 90.0);

    QPointF itemCenter (center.x () + midR * qCos (angleRad),
                        center.y () + midR * qSin (angleRad));

    qreal alpha= qBound (0.0, (m_openProgress - 0.3) / 0.7, 1.0);

    if (!m_items[i].icon.isNull ()) {
      int    iconSize= 24;
      QRectF iconRect (itemCenter.x () - iconSize / 2.0,
                       itemCenter.y () - iconSize / 2.0 - 6, iconSize,
                       iconSize);
      p.setOpacity (alpha);
      m_items[i].icon.paint (&p, iconRect.toRect ());
      p.setOpacity (1.0);

      QFont font= p.font ();
      font.setPixelSize (11);
      font.setWeight (QFont::Medium);
      p.setFont (font);

      QColor textCol= (i == m_hoveredIndex) ? kTextColor : kTextDimColor;
      textCol.setAlphaF (alpha);
      p.setPen (textCol);

      QRectF textRect (itemCenter.x () - 40, itemCenter.y () + 8, 80, 20);
      p.drawText (textRect, Qt::AlignHCenter | Qt::AlignTop, m_items[i].label);
    }
    else {
      QFont font= p.font ();
      font.setPixelSize (13);
      font.setWeight (QFont::DemiBold);
      p.setFont (font);

      QColor textCol= (i == m_hoveredIndex) ? kTextColor : kTextDimColor;
      textCol.setAlphaF (alpha);
      p.setPen (textCol);

      QRectF textRect (itemCenter.x () - 45, itemCenter.y () - 10, 90, 20);
      p.drawText (textRect, Qt::AlignCenter, m_items[i].label);
    }
  }
}

// ###########################################################################
//  QTMRadialMenuDock
// ###########################################################################

QTMRadialMenuDock::QTMRadialMenuDock (QWidget* parent)
    : QWidget (parent), m_menu (nullptr), m_triggerAnim (nullptr),
      m_triggerRotation (0.0), m_triggerSize (48), m_margin (18),
      m_expanded (false) {

  // Transparent background: no auto-fill, we paint everything ourselves
  setAutoFillBackground (false);
  setAttribute (Qt::WA_OpaquePaintEvent, false);
  setMouseTracking (true);

  // Start as just the trigger button size
  int side= m_triggerSize + m_margin;
  resize (side, side);
  updateMask ();

  // Create the radial menu as a child
  m_menu= new QTMRadialMenu (this);
  m_menu->hide ();

  connect (m_menu, &QTMRadialMenu::closeRequested, this, [this] () {
    toggle ();
  });
  connect (m_menu, &QTMRadialMenu::itemClicked, this,
           &QTMRadialMenuDock::itemClicked);

  // Watch parent for resize and show events to reposition ourselves
  if (parent) {
    parent->installEventFilter (this);
  }

  show ();
  raise ();
}

QTMRadialMenuDock::~QTMRadialMenuDock () {}

void
QTMRadialMenuDock::setTriggerRotation (qreal r) {
  m_triggerRotation= r;
  update ();
}

bool
QTMRadialMenuDock::eventFilter (QObject* obj, QEvent* event) {
  if (obj == parent ()) {
    if (event->type () == QEvent::Resize || event->type () == QEvent::Show ||
        event->type () == QEvent::LayoutRequest) {
      reposition ();
    }
  }
  return QWidget::eventFilter (obj, event);
}

void
QTMRadialMenuDock::reposition () {
  QWidget* p= parentWidget ();
  if (!p || p->width () < 1 || p->height () < 1) return;

  if (m_expanded) {
    QSize menuSz= m_menu->sizeForRadius ();
    int   w     = menuSz.width () + m_margin;
    int   h     = menuSz.height () + m_margin;
    resize (w, h);
    move (p->width () - w, p->height () - h);

    m_menu->setFixedSize (menuSz);
    m_menu->move ((w - menuSz.width ()) / 2, (h - menuSz.height ()) / 2);
  }
  else {
    int side= m_triggerSize + m_margin;
    resize (side, side);
    move (p->width () - side, p->height () - side);
  }
  raise ();
  show ();
  updateMask ();
}

void
QTMRadialMenuDock::toggle () {
  m_expanded= !m_expanded;
  
  // Emit signal before expanding so items can be updated
  if (m_expanded) {
    emit aboutToExpand ();
  }
  
  reposition ();

  if (m_expanded) {
    m_menu->animateOpen ();
  }
  else {
    m_menu->animateClose ();
  }

  // Rotate the trigger "+" icon to "x"
  if (m_triggerAnim) {
    m_triggerAnim->stop ();
    delete m_triggerAnim;
  }
  m_triggerAnim= new QPropertyAnimation (this, "triggerRotation", this);
  m_triggerAnim->setDuration (300);
  m_triggerAnim->setStartValue (m_triggerRotation);
  m_triggerAnim->setEndValue (m_expanded ? 45.0 : 0.0);
  m_triggerAnim->setEasingCurve (QEasingCurve::OutCubic);
  m_triggerAnim->start ();
}

// ---------------------------------------------------------------------------
// Painting – trigger button
// ---------------------------------------------------------------------------
void
QTMRadialMenuDock::paintEvent (QPaintEvent*) {
  QPainter p (this);
  p.setRenderHint (QPainter::Antialiasing, true);

  // The trigger button is always at the bottom-right of this widget
  qreal cx= width () - m_margin / 2.0 - m_triggerSize / 2.0;
  qreal cy= height () - m_margin / 2.0 - m_triggerSize / 2.0;
  qreal r = m_triggerSize / 2.0;

  // Shadow
  QRadialGradient shadow (QPointF (cx, cy + 2), r + 8);
  shadow.setColorAt (0.0, QColor (0, 0, 0, 50));
  shadow.setColorAt (1.0, Qt::transparent);
  p.setPen (Qt::NoPen);
  p.setBrush (shadow);
  p.drawEllipse (QPointF (cx, cy + 2), r + 8, r + 8);

  // Button body
  bool underMouse= rect ().contains (mapFromGlobal (QCursor::pos ()));
  QRadialGradient bg (QPointF (cx - r * 0.3, cy - r * 0.3), r * 1.8);
  QColor          base= (underMouse && !m_expanded) ? kTriggerHover : kTriggerBg;
  bg.setColorAt (0.0, base.lighter (130));
  bg.setColorAt (1.0, base);
  p.setBrush (bg);
  p.setPen (Qt::NoPen);
  p.drawEllipse (QPointF (cx, cy), r, r);

  // "+" / "x" icon
  p.save ();
  p.translate (cx, cy);
  p.rotate (m_triggerRotation);
  QPen iconPen (kTriggerIcon, 2.5, Qt::SolidLine, Qt::RoundCap);
  p.setPen (iconPen);
  qreal s= r * 0.35;
  p.drawLine (QPointF (-s, 0), QPointF (s, 0));
  p.drawLine (QPointF (0, -s), QPointF (0, s));
  p.restore ();
}

// ---------------------------------------------------------------------------
// Mouse – only the trigger button area responds when collapsed
// ---------------------------------------------------------------------------
void
QTMRadialMenuDock::mousePressEvent (QMouseEvent* event) {
  if (event->button () == Qt::LeftButton) {
    // Check if click is on the trigger button
    qreal cx   = width () - m_margin / 2.0 - m_triggerSize / 2.0;
    qreal cy   = height () - m_margin / 2.0 - m_triggerSize / 2.0;
    qreal dx   = event->pos ().x () - cx;
    qreal dy   = event->pos ().y () - cy;
    qreal dist2= dx * dx + dy * dy;
    qreal r    = m_triggerSize / 2.0;
    if (dist2 <= r * r) {
      toggle ();
      return;
    }
  }
  // If expanded and click is outside the menu ring, close
  if (m_expanded) {
    toggle ();
    return;
  }
  QWidget::mousePressEvent (event);
}

void
QTMRadialMenuDock::mouseMoveEvent (QMouseEvent* event) {
  update (); // repaint trigger hover state
  QWidget::mouseMoveEvent (event);
}

void
QTMRadialMenuDock::mouseReleaseEvent (QMouseEvent* event) {
  QWidget::mouseReleaseEvent (event);
}

void
QTMRadialMenuDock::updateMask () {
  // Create a circular mask for the trigger button area
  QRegion region;
  
  if (m_expanded) {
    // When expanded, the entire widget is interactive (menu + trigger)
    region= QRegion (rect ());
  }
  else {
    // When collapsed, only the circular trigger button should be interactive
    qreal cx= width () - m_margin / 2.0 - m_triggerSize / 2.0;
    qreal cy= height () - m_margin / 2.0 - m_triggerSize / 2.0;
    qreal r = m_triggerSize / 2.0;
    
    // Create circular region for the trigger button
    region= QRegion (static_cast<int> (cx - r), static_cast<int> (cy - r),
                     static_cast<int> (r * 2), static_cast<int> (r * 2),
                     QRegion::Ellipse);
  }
  
  setMask (region);
}
