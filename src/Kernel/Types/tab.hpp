
/******************************************************************************
 * MODULE     : tab.hpp
 * DESCRIPTION: spacing
 * COPYRIGHT  : (C) 1999  David Allouche
 *******************************************************************************
 * This software falls under the GNU general public license version 3 or later.
 * It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
 * in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
 ******************************************************************************/

#ifndef TAB_H
#define TAB_H
#include "tree.hpp"
#include <memory>

enum tab_kind { tab_all, tab_first, tab_last };

class tab_rep {
public:
  int      pos;
  double   weight;
  tab_kind kind;

  inline tab_rep () {}
  tab_rep (int pos, tree t);

  friend class tab;
};

class tab {
  std::shared_ptr<tab_rep> rep;
public:
  inline tab () : rep (std::make_shared<tab_rep> ()) {}
  inline tab (int pos, tree t) : rep (std::make_shared<tab_rep> (pos, t)) {}
  
  inline tab_rep* operator->() { return rep.get (); }
  inline const tab_rep* operator->() const { return rep.get (); }
  inline tab_rep& operator*() { return *rep; }
  inline const tab_rep& operator*() const { return *rep; }
  
  inline bool operator== (const tab& other) const { return rep == other.rep; }
  inline bool operator!= (const tab& other) const { return rep != other.rep; }
};

#endif // defined TAB_H
