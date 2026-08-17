
/******************************************************************************
 * MODULE     : preferences.cpp
 * DESCRIPTION: User preferences for TeXmacs
 * COPYRIGHT  : (C) 2012  Joris van der Hoeven
 *******************************************************************************
 * This software falls under the GNU general public license version 3 or later.
 * It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
 * in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
 ******************************************************************************/

#include "preferences.hpp"
#include "analyze.hpp"
#include "file.hpp"
#include "iterator.hpp"
#include "merge_sort.hpp"
#include "scheme.hpp"
#include "sys_utils.hpp"
#include "tm_file.hpp"
#include "tree_helper.hpp"

#include <moebius/data/scheme.hpp>
#include <nlohmann/json.hpp>
#include <string>

using moebius::data::block_to_scheme_tree;
using moebius::data::scm_quote;
using moebius::data::scm_unquote;
using nlohmann::json;

tree texmacs_settings= tuple ();

/******************************************************************************
 * Old style settings files
 ******************************************************************************/

static string
line_read (string s, int& i) {
  int start= i, n= N (s);
  for (start= i; i < n; i++)
    if (s[i] == '\n') break;
  string r= s (start, i);
  if (i < n) i++;
  return r;
}

void
get_old_settings (string s) {
  int i= 0, j;
  while (i < N (s)) {
    string l= line_read (s, i);
    for (j= 0; j < N (l); j++)
      if (l[j] == '=') {
        string left= l (0, j);
        while ((j < N (l)) && ((l[j] == '=') || (l[j] == ' ')))
          j++;
        string right= l (j, N (l));
        set_setting (left, right);
      }
  }
}

/******************************************************************************
 * Subroutines for the TeXmacs settings
 ******************************************************************************/

string
get_setting (string var, string def) {
  int i, n= N (texmacs_settings);
  for (i= 0; i < n; i++)
    if (is_tuple (texmacs_settings[i], var, 1)) {
      return scm_unquote (as_string (texmacs_settings[i][1]));
    }
  return def;
}

void
set_setting (string var, string val) {
  int i, n= N (texmacs_settings);
  for (i= 0; i < n; i++)
    if (is_tuple (texmacs_settings[i], var, 1)) {
      texmacs_settings[i][1]= scm_quote (val);
      return;
    }
  texmacs_settings << tuple (var, scm_quote (val));
}

/******************************************************************************
 * Changing the user preferences
 ******************************************************************************/

bool                    user_prefs_modified= false;
hashmap<string, string> user_prefs ("");
void                    notify_preference (string var);

bool
has_user_preference (string var) {
  return user_prefs->contains (var);
}

void
set_user_preference (string var, string val) {
  if (val == "default") user_prefs->reset (var);
  else user_prefs (var)= val;
  user_prefs_modified= true;
  notify_preference (var);
}

void
set_user_preference_silent (string var, string val) {
  if (val == "default") user_prefs->reset (var);
  else user_prefs (var)= val;
  user_prefs_modified= true;
}

void
reset_user_preference (string var) {
  user_prefs->reset (var);
  user_prefs_modified= true;
  notify_preference (var);
}

string
get_user_preference (string var, string val) {
  if (user_prefs->contains (var)) return user_prefs[var];
  else return val;
}

/******************************************************************************
 * Loading and saving user preferences
 ******************************************************************************/

// lolly string 与 std::string 互转（nlohmann::json 的键/值用 std::string）
static string
lolly_string (const std::string& s) {
  return string (s.c_str ());
}

static std::string
std_string (const string& s) {
  std::string r;
  for (int i= 0; i < N (s); i++)
    r+= s[i];
  return r;
}

// 读取 JSON 首选项文件：仅导入键值均为字符串的原子项，
// 其余（数字/布尔/null）跳过以容错
static void
load_json_preferences (url prefs_file) {
  json j= json::parse (std_string (string_load (prefs_file)), nullptr, false);
  if (j.is_discarded () || !j.is_object ()) return;
  for (json::iterator it= j.begin (); it != j.end (); ++it)
    if (it.value ().is_string ())
      user_prefs (lolly_string (it.key ()))=
          lolly_string (it.value ().get<std::string> ());
}

// 按 semver2 优先级比较两个版本串（忽略 build 元数据；核心段逐数字比较，预发布
// 段按标识符比较，数字标识符小于字母标识符）。返回负数/0/正数。
// 版本目录名来自 XMACS_VERSION，如 "2026.3.0-rc13"（旧目录）或
// "2026.3.1-rc.1"。
static int
compare_versions (string a, string b) {
  // 剥离 build 元数据（+ 之后的部分）
  int a_plus= search_forwards ("+", 0, a);
  int b_plus= search_forwards ("+", 0, b);
  if (a_plus >= 0) a= a (0, a_plus);
  if (b_plus >= 0) b= b (0, b_plus);
  int           a_dash= search_forwards ("-", 0, a);
  int           b_dash= search_forwards ("-", 0, b);
  array<string> ac    = tokenize (a_dash >= 0 ? a (0, a_dash) : a, ".");
  array<string> bc    = tokenize (b_dash >= 0 ? b (0, b_dash) : b, ".");
  int           n     = N (ac) > N (bc) ? N (ac) : N (bc);
  for (int i= 0; i < n; i++) {
    long int an= i < N (ac) ? as_long_int (ac[i]) : 0;
    long int bn= i < N (bc) ? as_long_int (bc[i]) : 0;
    if (an != bn) return an < bn ? -1 : 1;
  }
  // 核心段相等：无预发布段 > 有预发布段；预发布段按标识符逐个比较
  bool a_pre= a_dash >= 0;
  bool b_pre= b_dash >= 0;
  if (a_pre != b_pre) return a_pre ? -1 : 1;
  if (!a_pre) return 0;
  array<string> ap= tokenize (a (a_dash + 1, N (a)), ".");
  array<string> bp= tokenize (b (b_dash + 1, N (b)), ".");
  n               = N (ap) > N (bp) ? N (ap) : N (bp);
  for (int i= 0; i < n; i++) {
    if (i >= N (ap)) return -1; // a 缺标识符 → a < b
    if (i >= N (bp)) return 1;
    bool a_num= is_int (ap[i]);
    bool b_num= is_int (bp[i]);
    if (a_num && b_num) {
      long int an= as_long_int (ap[i]);
      long int bn= as_long_int (bp[i]);
      if (an != bn) return an < bn ? -1 : 1;
    }
    else if (a_num) return -1; // 数字标识符 < 字母标识符
    else if (b_num) return 1;
    else if (ap[i] != bp[i]) return ap[i] < bp[i] ? -1 : 1;
  }
  return 0;
}

struct version_leq {
  static bool leq (string a, string b) { return compare_versions (a, b) <= 0; }
};

// 读取旧版 scheme 列表格式的 .scm 首选项文件
static void
load_legacy_preferences (url prefs_file) {
  tree p= block_to_scheme_tree (string_load (prefs_file));
  while (is_func (p, TUPLE, 1))
    p= p[0];
  for (int i= 0; i < N (p); i++)
    if (is_func (p[i], TUPLE, 2) && is_atomic (p[i][0]) &&
        is_atomic (p[i][1]) && is_quoted (p[i][0]->label) &&
        is_quoted (p[i][1]->label)) {
      string var      = scm_unquote (p[i][0]->label);
      string val      = scm_unquote (p[i][1]->label);
      user_prefs (var)= val;
    }
}

// 扫描 system/ 下含首选项文件的旧版本目录，按版本升序逐个导入并立即落盘。
// 仅在当前版本目录没有 preferences.json（即升级后的首次启动）时调用。
static void
migrate_legacy_preferences () {
  url           system_dir= get_texmacs_home_path () * "system";
  bool          error_flag= false;
  array<string> entries   = read_directory (system_dir, error_flag);
  if (error_flag) return;

  array<string> versions;
  for (int i= 0; i < N (entries); i++) {
    string name= entries[i];
    if (starts (name, ".")) continue;
    url dir= system_dir * name;
    if (!is_directory (dir)) continue;
    if (exists (dir * "preferences.scm") || exists (dir * "preferences.json"))
      versions << name;
  }
  merge_sort_leq<string, version_leq> (versions);

  bool imported= false;
  for (int i= 0; i < N (versions); i++) {
    url dir= system_dir * versions[i];
    // 每个目录先 .scm 后 .json（json 覆盖 scm），后写覆盖先写 → 高版本优先
    if (exists (dir * "preferences.scm")) {
      load_legacy_preferences (dir * "preferences.scm");
      imported= true;
    }
    if (exists (dir * "preferences.json")) {
      load_json_preferences (dir * "preferences.json");
      imported= true;
    }
  }
  if (imported) {
    user_prefs_modified= true;
    make_dir (head (get_tm_preference_path ()));
    save_user_preferences ();
  }
}

void
load_user_preferences () {
  url prefs_file= get_tm_preference_path ();
  user_prefs    = hashmap<string, string> ("");
  if (exists (prefs_file)) {
    load_json_preferences (prefs_file);
  }
  else {
    migrate_legacy_preferences ();
  }
  user_prefs_modified= false;
}

void
save_user_preferences () {
  if (!user_prefs_modified) return;
  url              prefs_file= get_tm_preference_path ();
  iterator<string> it        = iterate (user_prefs);
  array<string>    a;
  while (it->busy ())
    a << it->next ();
  merge_sort (a);
  json j= json::object ();
  for (int i= 0; i < N (a); i++)
    j[std_string (a[i])]= std_string (user_prefs[a[i]]);
  if (save_string (prefs_file, lolly_string (j.dump ())))
    std_warning << "The user preferences could not be saved\n";
  user_prefs_modified= false;
}

/******************************************************************************
 * User preferences
 ******************************************************************************/

static bool preferences_ok= false;

void
notify_preferences_booted () {
  preferences_ok= true;
}

void
set_preference (string var, string val) {
  if (!preferences_ok) set_user_preference (var, val);
  else (void) call ("set-preference", var, val);
}

void
notify_preference (string var) {
  if (preferences_ok) (void) call ("notify-preference", var);
}

string
get_preference (string var, string def) {
  if (!preferences_ok) return get_user_preference (var, def);
  else {
    string pref= as_string (call ("get-preference", var));
    if (pref == "default") return def;
    else return pref;
  }
}
