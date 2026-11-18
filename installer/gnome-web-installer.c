#define _WIN32_IE 0x0600
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <objbase.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <adwaita.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <glib/gstdio.h>
#include <string.h>

#ifndef APP_VERSION
#define APP_VERSION "51.2.0"
#endif

typedef struct {
  AdwApplication *app;
  AdwApplicationWindow *win;
  GtkStack *stack;

  /* Options */
  char *source_dir;
  char *install_dir;
  gboolean desktop_shortcut;
  gboolean startmenu_shortcut;
  gboolean remove_user_data;
  gboolean uninstall_mode;

  /* UI widgets */
  GtkLabel *install_dir_label;
  AdwSwitchRow *desktop_row;
  AdwSwitchRow *startmenu_row;
  AdwSwitchRow *userdata_row;

  GtkProgressBar *progress_bar;
  GtkLabel *progress_label;
  GtkTextBuffer *log_buffer;
  GtkLabel *done_desc;
} InstallerState;

typedef struct {
  InstallerState *st;
  char *message;
  double fraction;
  gboolean is_log;
} UiUpdate;

static gboolean ui_update_cb (gpointer data) {
  UiUpdate *u = data;
  InstallerState *st = u->st;
  if (u->is_log && st->log_buffer && u->message) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter (st->log_buffer, &end);
    gtk_text_buffer_insert (st->log_buffer, &end, u->message, -1);
    gtk_text_buffer_insert (st->log_buffer, &end, "\n", 1);
  } else {
    if (st->progress_bar && u->fraction >= 0.0)
      gtk_progress_bar_set_fraction (st->progress_bar, u->fraction);
    if (st->progress_label && u->message)
      gtk_label_set_text (st->progress_label, u->message);
  }
  g_free (u->message);
  g_free (u);
  return G_SOURCE_REMOVE;
}

static void post_progress (InstallerState *st, double frac, const char *msg) {
  UiUpdate *u = g_new0 (UiUpdate, 1);
  u->st = st;
  u->fraction = frac;
  u->message = g_strdup (msg);
  g_idle_add (ui_update_cb, u);
}

static void post_log (InstallerState *st, const char *msg) {
  UiUpdate *u = g_new0 (UiUpdate, 1);
  u->st = st;
  u->fraction = -1.0;
  u->is_log = TRUE;
  u->message = g_strdup (msg);
  g_idle_add (ui_update_cb, u);
}

static void create_shortcut_win32 (const char *target_exe, const char *args, const char *lnk_path, const char *desc, const char *icon_path) {
  CoInitialize (NULL);
  IShellLinkW *psl = NULL;
  if (SUCCEEDED (CoCreateInstance (&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkW, (void **)&psl))) {
    g_autofree gunichar2 *wtarget = g_utf8_to_utf16 (target_exe, -1, NULL, NULL, NULL);
    g_autofree gunichar2 *wdesc = g_utf8_to_utf16 (desc, -1, NULL, NULL, NULL);
    g_autofree gunichar2 *wicon = g_utf8_to_utf16 (icon_path ? icon_path : target_exe, -1, NULL, NULL, NULL);
    g_autofree char *workdir = g_path_get_dirname (target_exe);
    g_autofree gunichar2 *wwork = g_utf8_to_utf16 (workdir, -1, NULL, NULL, NULL);

    psl->lpVtbl->SetPath (psl, wtarget);
    if (args) {
      g_autofree gunichar2 *wargs = g_utf8_to_utf16 (args, -1, NULL, NULL, NULL);
      psl->lpVtbl->SetArguments (psl, wargs);
    }
    psl->lpVtbl->SetDescription (psl, wdesc);
    psl->lpVtbl->SetIconLocation (psl, wicon, 0);
    psl->lpVtbl->SetWorkingDirectory (psl, wwork);

    IPersistFile *ppf = NULL;
    if (SUCCEEDED (psl->lpVtbl->QueryInterface (psl, &IID_IPersistFile, (void **)&ppf))) {
      g_autofree gunichar2 *wlnk = g_utf8_to_utf16 (lnk_path, -1, NULL, NULL, NULL);
      ppf->lpVtbl->Save (ppf, wlnk, TRUE);
      ppf->lpVtbl->Release (ppf);
    }
    psl->lpVtbl->Release (psl);
  }
  CoUninitialize ();
}

static void write_reg_str (HKEY root, const char *subkey, const char *name, const char *val) {
  g_autofree gunichar2 *wsub = g_utf8_to_utf16 (subkey, -1, NULL, NULL, NULL);
  g_autofree gunichar2 *wname = g_utf8_to_utf16 (name, -1, NULL, NULL, NULL);
  g_autofree gunichar2 *wval = g_utf8_to_utf16 (val, -1, NULL, NULL, NULL);
  HKEY hkey = NULL;
  if (RegCreateKeyExW (root, wsub, 0, NULL, 0, KEY_WRITE, NULL, &hkey, NULL) == ERROR_SUCCESS) {
    RegSetValueExW (hkey, wname, 0, REG_SZ, (const BYTE *)wval, (DWORD)((wcslen (wval) + 1) * sizeof (wchar_t)));
    RegCloseKey (hkey);
  }
}

static void write_reg_dword (HKEY root, const char *subkey, const char *name, DWORD val) {
  g_autofree gunichar2 *wsub = g_utf8_to_utf16 (subkey, -1, NULL, NULL, NULL);
  g_autofree gunichar2 *wname = g_utf8_to_utf16 (name, -1, NULL, NULL, NULL);
  HKEY hkey = NULL;
  if (RegCreateKeyExW (root, wsub, 0, NULL, 0, KEY_WRITE, NULL, &hkey, NULL) == ERROR_SUCCESS) {
    RegSetValueExW (hkey, wname, 0, REG_DWORD, (const BYTE *)&val, sizeof (DWORD));
    RegCloseKey (hkey);
  }
}

static void terminate_running_epiphany (void) {
  HANDLE snap = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);
  if (snap == INVALID_HANDLE_VALUE) return;
  PROCESSENTRY32W pe;
  pe.dwSize = sizeof (pe);
  if (Process32FirstW (snap, &pe)) {
    do {
      if (_wcsicmp (pe.szExeFile, L"epiphany.exe") == 0 ||
          _wcsicmp (pe.szExeFile, L"WebKitWebProcess.exe") == 0 ||
          _wcsicmp (pe.szExeFile, L"WebKitNetworkProcess.exe") == 0 ||
          _wcsicmp (pe.szExeFile, L"WebKitGPUProcess.exe") == 0) {
        HANDLE hp = OpenProcess (PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
        if (hp) { TerminateProcess (hp, 0); CloseHandle (hp); }
      }
    } while (Process32NextW (snap, &pe));
  }
  CloseHandle (snap);
}

static void collect_files_recursive (const char *base, const char *rel, GPtrArray *files) {
  g_autofree char *dir_path = rel ? g_build_filename (base, rel, NULL) : g_strdup (base);
  GDir *d = g_dir_open (dir_path, 0, NULL);
  if (!d) return;
  const char *name;
  while ((name = g_dir_read_name (d)) != NULL) {
    g_autofree char *sub_rel = rel ? g_build_filename (rel, name, NULL) : g_strdup (name);
    g_autofree char *full = g_build_filename (base, sub_rel, NULL);
    if (g_file_test (full, G_FILE_TEST_IS_DIR)) {
      collect_files_recursive (base, sub_rel, files);
    } else {
      g_ptr_array_add (files, g_strdup (sub_rel));
    }
  }
  g_dir_close (d);
}

static void delete_dir_recursive (const char *path) {
  GDir *d = g_dir_open (path, 0, NULL);
  if (!d) return;
  const char *name;
  while ((name = g_dir_read_name (d)) != NULL) {
    g_autofree char *full = g_build_filename (path, name, NULL);
    if (g_file_test (full, G_FILE_TEST_IS_DIR))
      delete_dir_recursive (full);
    else
      g_remove (full);
  }
  g_dir_close (d);
  g_rmdir (path);
}

static gboolean install_done_idle (gpointer data) {
  InstallerState *st = data;
  g_autofree char *msg = g_strdup_printf ("Installed to %s", st->install_dir);
  gtk_label_set_text (st->done_desc, msg);
  gtk_stack_set_visible_child_name (st->stack, "done");
  return G_SOURCE_REMOVE;
}

static void install_worker (GTask *task, gpointer source, gpointer task_data, GCancellable *canc) {
  (void)source; (void)canc;
  InstallerState *st = task_data;

  post_progress (st, 0.02, "Preparing installation…");
  terminate_running_epiphany ();
  g_mkdir_with_parents (st->install_dir, 0755);

  GPtrArray *files = g_ptr_array_new_with_free_func (g_free);
  collect_files_recursive (st->source_dir, NULL, files);
  post_log (st, "Scanning package payload…");

  for (guint i = 0; i < files->len; i++) {
    const char *rel = g_ptr_array_index (files, i);
    g_autofree char *src = g_build_filename (st->source_dir, rel, NULL);
    g_autofree char *dst = g_build_filename (st->install_dir, rel, NULL);
    g_autofree char *dst_dir = g_path_get_dirname (dst);
    g_mkdir_with_parents (dst_dir, 0755);

    g_autoptr (GFile) fsrc = g_file_new_for_path (src);
    g_autoptr (GFile) fdst = g_file_new_for_path (dst);
    g_file_copy (fsrc, fdst, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, NULL);

    if ((i % 8) == 0 || i + 1 == files->len) {
      double frac = 0.05 + 0.85 * ((double)(i + 1) / (double)(files->len ? files->len : 1));
      g_autofree char *msg = g_strdup_printf ("Installing %s (%u / %u)…", rel, i + 1, files->len);
      post_progress (st, frac, msg);
      post_log (st, rel);
    }
  }
  g_ptr_array_unref (files);

  /* Copy installer itself as Uninstall.exe */
  post_progress (st, 0.92, "Configuring shortcuts and uninstaller…");
  wchar_t self_w[MAX_PATH];
  GetModuleFileNameW (NULL, self_w, MAX_PATH);
  g_autofree char *self_utf8 = g_utf16_to_utf8 (self_w, -1, NULL, NULL, NULL);
  g_autofree char *uninst_exe = g_build_filename (st->install_dir, "Uninstall.exe", NULL);
  if (self_utf8) {
    g_autoptr (GFile) fsrc = g_file_new_for_path (self_utf8);
    g_autoptr (GFile) fdst = g_file_new_for_path (uninst_exe);
    g_file_copy (fsrc, fdst, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, NULL);
  }

  g_autofree char *app_exe = g_build_filename (st->install_dir, "epiphany.exe", NULL);
  if (st->desktop_shortcut) {
    const char *desktop = g_get_user_special_dir (G_USER_DIRECTORY_DESKTOP);
    if (desktop) {
      g_autofree char *lnk = g_build_filename (desktop, "GNOME Web.lnk", NULL);
      create_shortcut_win32 (app_exe, NULL, lnk, "GNOME Web (Epiphany)", app_exe);
      post_log (st, "Created Desktop shortcut");
    }
  }
  if (st->startmenu_shortcut) {
    wchar_t sm_w[MAX_PATH];
    if (SUCCEEDED (SHGetFolderPathW (NULL, CSIDL_PROGRAMS, NULL, 0, sm_w))) {
      g_autofree char *sm = g_utf16_to_utf8 (sm_w, -1, NULL, NULL, NULL);
      g_autofree char *sm_dir = g_build_filename (sm, "GNOME Web", NULL);
      g_mkdir_with_parents (sm_dir, 0755);
      g_autofree char *lnk = g_build_filename (sm_dir, "GNOME Web.lnk", NULL);
      g_autofree char *ulnk = g_build_filename (sm_dir, "Uninstall GNOME Web.lnk", NULL);
      create_shortcut_win32 (app_exe, NULL, lnk, "GNOME Web (Epiphany)", app_exe);
      create_shortcut_win32 (uninst_exe, "--uninstall", ulnk, "Uninstall GNOME Web", uninst_exe);
      post_log (st, "Created Start Menu shortcuts");
    }
  }

  /* Add/Remove Programs Registry (1:1 NSIS parity) */
  const char *reg_uninst = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\GNOME Web";
  g_autofree char *uninst_cmd = g_strdup_printf ("\"%s\" --uninstall", uninst_exe);
  write_reg_str (HKEY_CURRENT_USER, "Software\\GNOME Web", "InstallDir", st->install_dir);
  write_reg_str (HKEY_CURRENT_USER, reg_uninst, "DisplayName", "GNOME Web");
  write_reg_str (HKEY_CURRENT_USER, reg_uninst, "DisplayVersion", APP_VERSION);
  write_reg_str (HKEY_CURRENT_USER, reg_uninst, "Publisher", "The GNOME Project");
  write_reg_str (HKEY_CURRENT_USER, reg_uninst, "DisplayIcon", app_exe);
  write_reg_str (HKEY_CURRENT_USER, reg_uninst, "UninstallString", uninst_cmd);
  write_reg_dword (HKEY_CURRENT_USER, reg_uninst, "NoModify", 1);
  write_reg_dword (HKEY_CURRENT_USER, reg_uninst, "NoRepair", 1);

  post_progress (st, 1.0, "Installation complete.");
  g_idle_add (install_done_idle, st);
  g_task_return_boolean (task, TRUE);
}

static gboolean uninstall_done_idle (gpointer data) {
  InstallerState *st = data;
  gtk_stack_set_visible_child_name (st->stack, "uninstalled");
  return G_SOURCE_REMOVE;
}

static void uninstall_worker (GTask *task, gpointer source, gpointer task_data, GCancellable *canc) {
  (void)source; (void)canc;
  InstallerState *st = task_data;

  terminate_running_epiphany ();

  const char *desktop = g_get_user_special_dir (G_USER_DIRECTORY_DESKTOP);
  if (desktop) {
    g_autofree char *lnk = g_build_filename (desktop, "GNOME Web.lnk", NULL);
    g_remove (lnk);
  }
  wchar_t sm_w[MAX_PATH];
  if (SUCCEEDED (SHGetFolderPathW (NULL, CSIDL_PROGRAMS, NULL, 0, sm_w))) {
    g_autofree char *sm = g_utf16_to_utf8 (sm_w, -1, NULL, NULL, NULL);
    g_autofree char *sm_dir = g_build_filename (sm, "GNOME Web", NULL);
    delete_dir_recursive (sm_dir);
  }

  RegDeleteKeyW (HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\GNOME Web");
  RegDeleteKeyW (HKEY_CURRENT_USER, L"Software\\GNOME Web");

  if (st->remove_user_data) {
    g_autofree char *ephy_data = g_build_filename (g_get_user_data_dir (), "epiphany", NULL);
    g_autofree char *ephy_cfg = g_build_filename (g_get_user_config_dir (), "epiphany", NULL);
    delete_dir_recursive (ephy_data);
    delete_dir_recursive (ephy_cfg);
  }

  /* Delete install_dir contents (except running Uninstall.exe, which is cleaned via cmd) */
  delete_dir_recursive (st->install_dir);
  g_autofree char *cmd = g_strdup_printf ("/c ping 127.0.0.1 -n 3 > nul & rmdir /s /q \"%s\"", st->install_dir);
  g_autofree gunichar2 *wcmd = g_utf8_to_utf16 (cmd, -1, NULL, NULL, NULL);
  ShellExecuteW (NULL, L"open", L"cmd.exe", wcmd, NULL, SW_HIDE);

  g_idle_add (uninstall_done_idle, st);
  g_task_return_boolean (task, TRUE);
}

static void on_folder_picked (GObject *src, GAsyncResult *res, gpointer data) {
  InstallerState *st = data;
  g_autoptr (GFile) f = gtk_file_dialog_select_folder_finish (GTK_FILE_DIALOG (src), res, NULL);
  if (f) {
    g_autofree char *p = g_file_get_path (f);
    if (p) {
      g_free (st->install_dir);
      st->install_dir = g_build_filename (p, "GNOME Web", NULL);
      gtk_label_set_text (st->install_dir_label, st->install_dir);
    }
  }
}

static void on_change_dir_clicked (GtkButton *btn, gpointer data) {
  (void)btn;
  InstallerState *st = data;
  g_autoptr (GtkFileDialog) dlg = gtk_file_dialog_new ();
  gtk_file_dialog_set_title (dlg, "Choose Installation Folder");
  gtk_file_dialog_select_folder (dlg, GTK_WINDOW (st->win), NULL, on_folder_picked, st);
}

static void on_install_clicked (GtkButton *btn, gpointer data) {
  (void)btn;
  InstallerState *st = data;
  st->desktop_shortcut = adw_switch_row_get_active (st->desktop_row);
  st->startmenu_shortcut = adw_switch_row_get_active (st->startmenu_row);
  gtk_stack_set_visible_child_name (st->stack, "installing");
  g_autoptr (GTask) t = g_task_new (NULL, NULL, NULL, st);
  g_task_set_task_data (t, st, NULL);
  g_task_run_in_thread (t, install_worker);
}

static void on_uninstall_clicked (GtkButton *btn, gpointer data) {
  (void)btn;
  InstallerState *st = data;
  st->remove_user_data = adw_switch_row_get_active (st->userdata_row);
  g_autoptr (GTask) t = g_task_new (NULL, NULL, NULL, st);
  g_task_set_task_data (t, st, NULL);
  g_task_run_in_thread (t, uninstall_worker);
}

static void on_launch_clicked (GtkButton *btn, gpointer data) {
  (void)btn;
  InstallerState *st = data;
  g_autofree char *exe = g_build_filename (st->install_dir, "epiphany.exe", NULL);
  g_autofree gunichar2 *wexe = g_utf8_to_utf16 (exe, -1, NULL, NULL, NULL);
  ShellExecuteW (NULL, L"open", wexe, NULL, NULL, SW_SHOWNORMAL);
  g_application_quit (G_APPLICATION (st->app));
}

static void on_close_clicked (GtkButton *btn, gpointer data) {
  (void)btn;
  InstallerState *st = data;
  g_application_quit (G_APPLICATION (st->app));
}

static void on_activate (GtkApplication *app, gpointer data) {
  InstallerState *st = data;
  st->win = ADW_APPLICATION_WINDOW (adw_application_window_new (app));
  gtk_window_set_title (GTK_WINDOW (st->win), "GNOME Web Installer");
  gtk_window_set_default_size (GTK_WINDOW (st->win), 520, 560);
  gtk_window_set_resizable (GTK_WINDOW (st->win), FALSE);

  GtkWidget *toolbar_view = adw_toolbar_view_new ();
  GtkWidget *header = adw_header_bar_new ();
  adw_header_bar_set_title_widget (ADW_HEADER_BAR (header), adw_window_title_new ("GNOME Web", "Version " APP_VERSION));
  adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (toolbar_view), header);

  st->stack = GTK_STACK (gtk_stack_new ());
  gtk_stack_set_transition_type (st->stack, GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
  adw_toolbar_view_set_content (ADW_TOOLBAR_VIEW (toolbar_view), GTK_WIDGET (st->stack));

  /* Page 1: Welcome / Options */
  GtkWidget *welcome_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
  gtk_widget_set_margin_top (welcome_box, 16);
  gtk_widget_set_margin_bottom (welcome_box, 24);
  gtk_widget_set_margin_start (welcome_box, 24);
  gtk_widget_set_margin_end (welcome_box, 24);

  GtkWidget *status = adw_status_page_new ();
  adw_status_page_set_icon_name (ADW_STATUS_PAGE (status), "org.gnome.Epiphany");
  adw_status_page_set_title (ADW_STATUS_PAGE (status), "Install GNOME Web");
  adw_status_page_set_description (ADW_STATUS_PAGE (status),
    "Simple, clean, and privacy-respecting web browser built with GTK4, Libadwaita, and WebKit.");
  gtk_box_append (GTK_BOX (welcome_box), status);

  GtkWidget *grp = adw_preferences_group_new ();
  adw_preferences_group_set_title (ADW_PREFERENCES_GROUP (grp), "Installation Options");

  GtkWidget *dir_row = adw_action_row_new ();
  adw_preferences_row_set_title (ADW_PREFERENCES_ROW (dir_row), "Destination Folder");
  st->install_dir_label = GTK_LABEL (gtk_label_new (st->install_dir));
  gtk_label_set_ellipsize (st->install_dir_label, PANGO_ELLIPSIZE_START);
  gtk_widget_add_css_class (GTK_WIDGET (st->install_dir_label), "dim-label");
  adw_action_row_add_suffix (ADW_ACTION_ROW (dir_row), GTK_WIDGET (st->install_dir_label));
  GtkWidget *chg_btn = gtk_button_new_with_label ("Change…");
  gtk_widget_set_valign (chg_btn, GTK_ALIGN_CENTER);
  g_signal_connect (chg_btn, "clicked", G_CALLBACK (on_change_dir_clicked), st);
  adw_action_row_add_suffix (ADW_ACTION_ROW (dir_row), chg_btn);
  adw_preferences_group_add (ADW_PREFERENCES_GROUP (grp), dir_row);

  st->desktop_row = ADW_SWITCH_ROW (adw_switch_row_new ());
  adw_preferences_row_set_title (ADW_PREFERENCES_ROW (st->desktop_row), "Desktop Shortcut");
  adw_switch_row_set_active (st->desktop_row, TRUE);
  adw_preferences_group_add (ADW_PREFERENCES_GROUP (grp), GTK_WIDGET (st->desktop_row));

  st->startmenu_row = ADW_SWITCH_ROW (adw_switch_row_new ());
  adw_preferences_row_set_title (ADW_PREFERENCES_ROW (st->startmenu_row), "Start Menu Entry");
  adw_switch_row_set_active (st->startmenu_row, TRUE);
  adw_preferences_group_add (ADW_PREFERENCES_GROUP (grp), GTK_WIDGET (st->startmenu_row));

  gtk_box_append (GTK_BOX (welcome_box), grp);

  GtkWidget *install_btn = gtk_button_new_with_label ("Install GNOME Web");
  gtk_widget_add_css_class (install_btn, "suggested-action");
  gtk_widget_add_css_class (install_btn, "pill");
  gtk_widget_set_halign (install_btn, GTK_ALIGN_CENTER);
  g_signal_connect (install_btn, "clicked", G_CALLBACK (on_install_clicked), st);
  gtk_box_append (GTK_BOX (welcome_box), install_btn);

  gtk_stack_add_named (st->stack, welcome_box, "welcome");

  /* Page 2: Installing */
  GtkWidget *inst_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 16);
  gtk_widget_set_margin_top (inst_box, 32);
  gtk_widget_set_margin_bottom (inst_box, 24);
  gtk_widget_set_margin_start (inst_box, 28);
  gtk_widget_set_margin_end (inst_box, 28);

  GtkWidget *inst_status = adw_status_page_new ();
  adw_status_page_set_icon_name (ADW_STATUS_PAGE (inst_status), "org.gnome.Epiphany");
  adw_status_page_set_title (ADW_STATUS_PAGE (inst_status), "Installing GNOME Web…");
  gtk_box_append (GTK_BOX (inst_box), inst_status);

  st->progress_bar = GTK_PROGRESS_BAR (gtk_progress_bar_new ());
  gtk_box_append (GTK_BOX (inst_box), GTK_WIDGET (st->progress_bar));

  st->progress_label = GTK_LABEL (gtk_label_new ("Starting installation…"));
  gtk_widget_add_css_class (GTK_WIDGET (st->progress_label), "dim-label");
  gtk_label_set_ellipsize (st->progress_label, PANGO_ELLIPSIZE_MIDDLE);
  gtk_box_append (GTK_BOX (inst_box), GTK_WIDGET (st->progress_label));

  GtkWidget *exp = gtk_expander_new ("Details");
  GtkWidget *scr = gtk_scrolled_window_new ();
  gtk_widget_set_size_request (scr, -1, 140);
  GtkWidget *tv = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (tv), FALSE);
  gtk_text_view_set_monospace (GTK_TEXT_VIEW (tv), TRUE);
  st->log_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (tv));
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scr), tv);
  gtk_expander_set_child (GTK_EXPANDER (exp), scr);
  gtk_box_append (GTK_BOX (inst_box), exp);

  gtk_stack_add_named (st->stack, inst_box, "installing");

  /* Page 3: Done */
  GtkWidget *done_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
  gtk_widget_set_valign (done_box, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_start (done_box, 28);
  gtk_widget_set_margin_end (done_box, 28);

  GtkWidget *done_status = adw_status_page_new ();
  adw_status_page_set_icon_name (ADW_STATUS_PAGE (done_status), "emblem-ok-symbolic");
  adw_status_page_set_title (ADW_STATUS_PAGE (done_status), "GNOME Web is Ready");
  gtk_box_append (GTK_BOX (done_box), done_status);

  st->done_desc = GTK_LABEL (gtk_label_new (""));
  gtk_widget_add_css_class (GTK_WIDGET (st->done_desc), "dim-label");
  gtk_box_append (GTK_BOX (done_box), GTK_WIDGET (st->done_desc));

  GtkWidget *btn_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_set_halign (btn_box, GTK_ALIGN_CENTER);
  GtkWidget *launch_btn = gtk_button_new_with_label ("Launch GNOME Web");
  gtk_widget_add_css_class (launch_btn, "suggested-action");
  gtk_widget_add_css_class (launch_btn, "pill");
  g_signal_connect (launch_btn, "clicked", G_CALLBACK (on_launch_clicked), st);
  GtkWidget *close_btn = gtk_button_new_with_label ("Close");
  gtk_widget_add_css_class (close_btn, "pill");
  g_signal_connect (close_btn, "clicked", G_CALLBACK (on_close_clicked), st);
  gtk_box_append (GTK_BOX (btn_box), close_btn);
  gtk_box_append (GTK_BOX (btn_box), launch_btn);
  gtk_box_append (GTK_BOX (done_box), btn_box);

  gtk_stack_add_named (st->stack, done_box, "done");

  /* Page 4: Uninstall */
  GtkWidget *un_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
  gtk_widget_set_valign (un_box, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_start (un_box, 28);
  gtk_widget_set_margin_end (un_box, 28);

  GtkWidget *un_status = adw_status_page_new ();
  adw_status_page_set_icon_name (ADW_STATUS_PAGE (un_status), "user-trash-symbolic");
  adw_status_page_set_title (ADW_STATUS_PAGE (un_status), "Uninstall GNOME Web?");
  adw_status_page_set_description (ADW_STATUS_PAGE (un_status),
    "This will remove GNOME Web and its shortcuts from your system.");
  gtk_box_append (GTK_BOX (un_box), un_status);

  GtkWidget *ugrp = adw_preferences_group_new ();
  st->userdata_row = ADW_SWITCH_ROW (adw_switch_row_new ());
  adw_preferences_row_set_title (ADW_PREFERENCES_ROW (st->userdata_row), "Also remove bookmarks and browsing data");
  adw_preferences_group_add (ADW_PREFERENCES_GROUP (ugrp), GTK_WIDGET (st->userdata_row));
  gtk_box_append (GTK_BOX (un_box), ugrp);

  GtkWidget *ubtn_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_set_halign (ubtn_box, GTK_ALIGN_CENTER);
  GtkWidget *ucancel = gtk_button_new_with_label ("Cancel");
  gtk_widget_add_css_class (ucancel, "pill");
  g_signal_connect (ucancel, "clicked", G_CALLBACK (on_close_clicked), st);
  GtkWidget *uremove = gtk_button_new_with_label ("Uninstall");
  gtk_widget_add_css_class (uremove, "destructive-action");
  gtk_widget_add_css_class (uremove, "pill");
  g_signal_connect (uremove, "clicked", G_CALLBACK (on_uninstall_clicked), st);
  gtk_box_append (GTK_BOX (ubtn_box), ucancel);
  gtk_box_append (GTK_BOX (ubtn_box), uremove);
  gtk_box_append (GTK_BOX (un_box), ubtn_box);

  gtk_stack_add_named (st->stack, un_box, "uninstall");

  /* Page 5: Uninstalled */
  GtkWidget *un_done = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
  gtk_widget_set_valign (un_done, GTK_ALIGN_CENTER);
  GtkWidget *un_done_st = adw_status_page_new ();
  adw_status_page_set_icon_name (ADW_STATUS_PAGE (un_done_st), "emblem-ok-symbolic");
  adw_status_page_set_title (ADW_STATUS_PAGE (un_done_st), "GNOME Web Uninstalled");
  gtk_box_append (GTK_BOX (un_done), un_done_st);
  GtkWidget *uclose = gtk_button_new_with_label ("Close");
  gtk_widget_add_css_class (uclose, "pill");
  gtk_widget_set_halign (uclose, GTK_ALIGN_CENTER);
  g_signal_connect (uclose, "clicked", G_CALLBACK (on_close_clicked), st);
  gtk_box_append (GTK_BOX (un_done), uclose);
  gtk_stack_add_named (st->stack, un_done, "uninstalled");

  gtk_stack_set_visible_child_name (st->stack, st->uninstall_mode ? "uninstall" : "welcome");

  adw_application_window_set_content (st->win, toolbar_view);
  gtk_window_present (GTK_WINDOW (st->win));
}

int main (int argc, char **argv) {
  g_autofree char *exe_dir = g_win32_get_package_installation_directory_of_module (NULL);
  if (exe_dir) {
    g_autofree char *share_dir = g_build_filename (exe_dir, "share", NULL);
    g_autofree char *schema_dir = g_build_filename (share_dir, "glib-2.0", "schemas", NULL);
    if (g_file_test (schema_dir, G_FILE_TEST_IS_DIR)) {
      g_setenv ("GSETTINGS_SCHEMA_DIR", schema_dir, FALSE);
      g_setenv ("XDG_DATA_DIRS", share_dir, FALSE);
    }
  }

  InstallerState *st = g_new0 (InstallerState, 1);
  st->source_dir = exe_dir ? g_strdup (exe_dir) : g_get_current_dir ();
  const char *local_app = g_get_user_data_dir ();
  st->install_dir = g_build_filename (local_app, "Programs", "GNOME Web", NULL);

  for (int i = 1; i < argc; i++) {
    if (g_strcmp0 (argv[i], "--uninstall") == 0) {
      st->uninstall_mode = TRUE;
      if (exe_dir) { g_free (st->install_dir); st->install_dir = g_strdup (exe_dir); }
    } else if (g_strcmp0 (argv[i], "--source") == 0 && i + 1 < argc) {
      g_free (st->source_dir);
      st->source_dir = g_strdup (argv[++i]);
    }
  }

  st->app = adw_application_new ("org.gnome.Epiphany.Installer", G_APPLICATION_NON_UNIQUE);
  g_signal_connect (st->app, "activate", G_CALLBACK (on_activate), st);
  int ret = g_application_run (G_APPLICATION (st->app), 1, argv);
  g_object_unref (st->app);
  return ret;
}
