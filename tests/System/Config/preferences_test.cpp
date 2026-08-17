
/******************************************************************************
 * MODULE     : preferences_test.cpp
 * DESCRIPTION: Unit tests for user preferences (JSON 读写)
 * COPYRIGHT  : (C) 2026  MoonLL
 *******************************************************************************
 * This software falls under the GNU general public license version 3 or later.
 * It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
 * in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
 ******************************************************************************/

#include "analyze.hpp"
#include "base.hpp"
#include "file.hpp"
#include "preferences.hpp"
#include "sys_utils.hpp"
#include "tm_sys_utils.hpp"
#include "tm_url.hpp"
#include "url.hpp"

#include <QTemporaryDir>
#include <QtTest/QtTest>

class TestPreferences : public QObject {
  Q_OBJECT

  string orig_home;

  static void restore_env (const char* name, string val) {
    if (is_empty (val)) qunsetenv (name);
    else set_env (string (name), val);
  }

private slots:
  void init () {
    init_lolly ();
    orig_home= get_env ("TEXMACS_HOME_PATH");
  }
  void cleanup () { restore_env ("TEXMACS_HOME_PATH", orig_home); }
  void test_save_load_roundtrip ();
  void test_load_missing_file ();
  void test_migrate_legacy_preferences ();
  void test_migrate_no_legacy ();
};

// 写旧版 scheme 列表格式首选项文件（每行一个 ("key" "value") 对）
static void
write_legacy_scm (url dir, string content) {
  make_dir (dir);
  QVERIFY2 (!save_string (dir * "preferences.scm", content),
            as_charp ("failed to write legacy scm: " * as_string (dir)));
}

void
TestPreferences::test_save_load_roundtrip () {
  QTemporaryDir home;
  QVERIFY (home.isValid ());
  set_env ("TEXMACS_HOME_PATH", string (home.path ().toUtf8 ().constData ()));
  load_user_preferences (); // 复位全局首选项状态

  // save_string 不建父目录，先建 system/<版本>/
  url prefs_file= get_tm_preference_path ();
  make_dir (head (prefs_file));

  // 写入含特殊字符（引号/反斜杠/换行）的值并保存
  set_user_preference ("test.str", "a\"b\\c\n");
  set_user_preference ("test.empty", "");
  set_user_preference ("test.normal", "hello");
  save_user_preferences ();

  // 磁盘上是合法 JSON，且特殊字符被正确转义
  string content;
  QVERIFY2 (!load_string (prefs_file, content, false),
            as_charp ("failed to read preferences.json"));
  QVERIFY2 (starts (content, "{") && ends (content, "}"),
            as_charp ("expected a JSON object: " * content));
  QVERIFY2 (occurs ("\\\"", content) && occurs ("\\\\", content) &&
                occurs ("\\n", content),
            as_charp ("expected escaped value: " * content));

  // 重新加载，取回原值
  load_user_preferences ();
  QVERIFY2 (
      get_user_preference ("test.str", "") == "a\"b\\c\n",
      as_charp ("roundtrip broken: " * get_user_preference ("test.str", "")));
  QVERIFY2 (get_user_preference ("test.empty", "") == "",
            as_charp ("empty value roundtrip broken"));
  QVERIFY2 (get_user_preference ("test.normal", "") == "hello",
            as_charp ("normal value roundtrip broken"));
}

void
TestPreferences::test_load_missing_file () {
  QTemporaryDir home;
  QVERIFY (home.isValid ());
  set_env ("TEXMACS_HOME_PATH", string (home.path ().toUtf8 ().constData ()));
  load_user_preferences ();

  url prefs_file= get_tm_preference_path ();
  QVERIFY (!exists (prefs_file));
  QVERIFY2 (get_user_preference ("some.key", "def") == "def",
            as_charp ("missing file should yield defaults"));
}

void
TestPreferences::test_migrate_legacy_preferences () {
  QTemporaryDir home;
  QVERIFY (home.isValid ());
  string home_path (home.path ().toUtf8 ().constData ());
  set_env ("TEXMACS_HOME_PATH", home_path);

  // 两个旧版本目录，v2 新于 v1，含冲突键 key.shared（高版本应胜出）
  write_legacy_scm (url (home_path) * "system" * "2026.2.3",
                    "(\"key.a\" \"v1\")\n(\"key.shared\" \"old\")\n");
  write_legacy_scm (url (home_path) * "system" * "2026.3.0-rc13",
                    "(\"key.b\" \"v2\")\n(\"key.shared\" \"new\")\n");

  load_user_preferences (); // 当前版本无 JSON → 触发迁移

  QVERIFY2 (get_user_preference ("key.a", "") == "v1",
            as_charp ("v1 key missing after migration"));
  QVERIFY2 (get_user_preference ("key.b", "") == "v2",
            as_charp ("v2 key missing after migration"));
  QVERIFY2 (get_user_preference ("key.shared", "") == "new",
            as_charp ("higher version should win the conflict"));

  // 迁移落盘：当前版本 preferences.json 生成且包含合并结果
  url prefs_file= get_tm_preference_path ();
  QVERIFY2 (exists (prefs_file), as_charp ("migrated json not created"));
  string content;
  QVERIFY2 (!load_string (prefs_file, content, false),
            as_charp ("failed to read migrated json"));
  QVERIFY2 (occurs ("key.shared", content) && occurs ("\"new\"", content),
            as_charp ("migrated content wrong: " * content));

  // 再次加载直接读 JSON，不再迁移，键值稳定
  load_user_preferences ();
  QVERIFY2 (get_user_preference ("key.shared", "") == "new",
            as_charp ("second load lost migrated values"));
}

void
TestPreferences::test_migrate_no_legacy () {
  QTemporaryDir home;
  QVERIFY (home.isValid ());
  set_env ("TEXMACS_HOME_PATH", string (home.path ().toUtf8 ().constData ()));

  // 空 system/ 目录（首次安装）：不生成任何首选项文件
  make_dir (url (home.path ().toUtf8 ().constData ()) * "system");
  load_user_preferences ();
  QVERIFY (!exists (get_tm_preference_path ()));
  QVERIFY2 (get_user_preference ("some.key", "def") == "def",
            as_charp ("missing file should yield defaults"));
}

QTEST_MAIN (TestPreferences)
#include "preferences_test.moc"
