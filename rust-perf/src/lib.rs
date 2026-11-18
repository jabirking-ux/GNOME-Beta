use std::ffi::{CStr, CString};
use std::os::raw::c_char;

#[no_mangle]
pub extern "C" fn ephy_rust_uri_for_display(uri: *const c_char) -> *mut c_char {
    if uri.is_null() {
        return std::ptr::null_mut();
    }
    let c_str = unsafe { CStr::from_ptr(uri) };
    let s = match c_str.to_str() {
        Ok(v) => v.trim(),
        Err(_) => return std::ptr::null_mut(),
    };
    let display = s.strip_prefix("https://").or_else(|| s.strip_prefix("http://")).unwrap_or(s);
    let display = display.strip_suffix('/').unwrap_or(display);
    match CString::new(display) {
        Ok(cs) => cs.into_raw(),
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn ephy_rust_fuzzy_score(query: *const c_char, candidate: *const c_char) -> i32 {
    if query.is_null() || candidate.is_null() {
        return 0;
    }
    let q = unsafe { CStr::from_ptr(query) }.to_string_lossy().to_lowercase();
    let c = unsafe { CStr::from_ptr(candidate) }.to_string_lossy().to_lowercase();
    if q.is_empty() {
        return 1;
    }
    if c.starts_with(&q) {
        return 100;
    }
    if c.contains(&q) {
        return 50;
    }
    0
}

#[no_mangle]
pub extern "C" fn ephy_rust_free_str(ptr: *mut c_char) {
    if !ptr.is_null() {
        unsafe {
            drop(CString::from_raw(ptr));
        }
    }
}

#[no_mangle]
pub extern "C" fn ephy_rust_prewarm_env(exe_dir: *const c_char) {
    if exe_dir.is_null() {
        return;
    }
    let base = match unsafe { CStr::from_ptr(exe_dir) }.to_str() {
        Ok(s) => std::path::Path::new(s),
        Err(_) => return,
    };
    let schema_dir = base.join("share").join("glib-2.0").join("schemas");
    let share_dir = base.join("share");
    let gio_mods = base.join("lib").join("gio").join("modules");
    let ca_file = base.join("cacert.pem");
    let pixbuf_cache = base.join("lib").join("gdk-pixbuf-2.0").join("2.10.0").join("loaders.cache");
    let pixbuf_dir = base.join("lib").join("gdk-pixbuf-2.0").join("2.10.0").join("loaders");

    if schema_dir.is_dir() {
        if std::env::var_os("GSETTINGS_SCHEMA_DIR").is_none() {
            std::env::set_var("GSETTINGS_SCHEMA_DIR", &schema_dir);
        }
        if std::env::var_os("XDG_DATA_DIRS").is_none() {
            std::env::set_var("XDG_DATA_DIRS", &share_dir);
        }
    }
    if gio_mods.is_dir() && std::env::var_os("GIO_MODULE_DIR").is_none() {
        std::env::set_var("GIO_MODULE_DIR", &gio_mods);
    }
    if ca_file.is_file() && std::env::var_os("SSL_CERT_FILE").is_none() {
        std::env::set_var("SSL_CERT_FILE", &ca_file);
    }
    if pixbuf_cache.is_file() && std::env::var_os("GDK_PIXBUF_MODULE_FILE").is_none() {
        std::env::set_var("GDK_PIXBUF_MODULE_FILE", &pixbuf_cache);
    }
    if pixbuf_dir.is_dir() && std::env::var_os("GDK_PIXBUF_MODULEDIR").is_none() {
        std::env::set_var("GDK_PIXBUF_MODULEDIR", &pixbuf_dir);
    }
}

#[no_mangle]
pub extern "C" fn ephy_rust_check_auto_update(current_version: *const c_char) {
    let cur = if current_version.is_null() {
        "51.2".to_string()
    } else {
        unsafe { CStr::from_ptr(current_version) }.to_string_lossy().into_owned()
    };
    std::thread::spawn(move || {
        let out = match std::process::Command::new("curl.exe")
            .args(["-sL", "-H", "User-Agent: GNOME-Beta-Updater", "https://api.github.com/repos/jabirking-ux/GNOME-Beta/releases/latest"])
            .output()
        {
            Ok(o) if o.status.success() => String::from_utf8_lossy(&o.stdout).into_owned(),
            _ => return,
        };
        let tag = out.split("\"tag_name\"").nth(1)
            .and_then(|s| s.split('"').nth(1))
            .unwrap_or("");
        let tag_ver = tag.trim_start_matches('v');
        if tag_ver.is_empty() || tag_ver == cur {
            return;
        }
        let mut setup_url = String::new();
        for part in out.split("\"browser_download_url\"") {
            if let Some(url) = part.split('"').nth(1) {
                if url.contains("Setup") && url.ends_with(".exe") {
                    setup_url = url.to_string();
                    break;
                }
            }
        }
        if setup_url.is_empty() {
            return;
        }
        let dest = std::env::temp_dir().join("GNOME-Beta-Setup-latest.exe");
        let dl = std::process::Command::new("curl.exe")
            .args(["-sL", "-o", dest.to_string_lossy().as_ref(), &setup_url])
            .status();
        if matches!(dl, Ok(s) if s.success()) {
            let _ = std::process::Command::new(&dest).spawn();
        }
    });
}

#[no_mangle]
pub extern "C" fn ephy_rust_register_default_browser(exe_path: *const c_char) -> i32 {
    if exe_path.is_null() {
        return 0;
    }
    let exe = match unsafe { CStr::from_ptr(exe_path) }.to_str() {
        Ok(s) if !s.is_empty() => s,
        _ => return 0,
    };
    let cmd_val = format!("\"{}\" \"%1\"", exe);
    let icon_val = format!("{},0", exe);
    let reg_cmds: &[(&str, &str, &str)] = &[
        (r"HKCU\Software\Classes\GNOMEWebHTML", "", "GNOME Web HTML Document"),
        (r"HKCU\Software\Classes\GNOMEWebHTML\DefaultIcon", "", &icon_val),
        (r"HKCU\Software\Classes\GNOMEWebHTML\shell\open\command", "", &cmd_val),
        (r"HKCU\Software\Classes\Applications\epiphany.exe\shell\open\command", "", &cmd_val),
        (r"HKCU\Software\RegisteredApplications", "GNOME Web", r"Software\GNOME Web\Capabilities"),
        (r"HKCU\Software\GNOME Web\Capabilities", "ApplicationName", "GNOME Web"),
        (r"HKCU\Software\GNOME Web\Capabilities", "ApplicationDescription", "GNOME Web (Epiphany) Browser"),
        (r"HKCU\Software\GNOME Web\Capabilities\URLAssociations", "http", "GNOMEWebHTML"),
        (r"HKCU\Software\GNOME Web\Capabilities\URLAssociations", "https", "GNOMEWebHTML"),
        (r"HKCU\Software\GNOME Web\Capabilities\FileAssociations", ".htm", "GNOMEWebHTML"),
        (r"HKCU\Software\GNOME Web\Capabilities\FileAssociations", ".html", "GNOMEWebHTML"),
    ];
    for (key, name, val) in reg_cmds {
        let mut args = vec!["add", key];
        if name.is_empty() {
            args.push("/ve");
        } else {
            args.push("/v");
            args.push(name);
        }
        args.extend_from_slice(&["/t", "REG_SZ", "/d", val, "/f"]);
        let _ = std::process::Command::new("reg.exe").args(&args).status();
    }
    let _ = std::process::Command::new("cmd.exe")
        .args(["/c", "start", "", "ms-settings:defaultapps?registeredAppUser=GNOME%20Web"])
        .spawn();
    1
}

#[no_mangle]
pub extern "C" fn ephy_rust_cred_store(target: *const c_char, user: *const c_char, secret: *const c_char) -> i32 {
    if target.is_null() || user.is_null() || secret.is_null() {
        return 0;
    }
    let t = unsafe { CStr::from_ptr(target) }.to_string_lossy();
    let u = unsafe { CStr::from_ptr(user) }.to_string_lossy();
    let s = unsafe { CStr::from_ptr(secret) }.to_string_lossy();
    let ok = std::process::Command::new("cmdkey.exe")
        .args([&format!("/generic:{}", t), &format!("/user:{}", u), &format!("/pass:{}", s)])
        .status()
        .map(|st| st.success())
        .unwrap_or(false);
    if ok { 1 } else { 0 }
}

#[no_mangle]
pub extern "C" fn ephy_rust_cred_delete(target: *const c_char) -> i32 {
    if target.is_null() {
        return 0;
    }
    let t = unsafe { CStr::from_ptr(target) }.to_string_lossy();
    let ok = std::process::Command::new("cmdkey.exe")
        .args([&format!("/delete:{}", t)])
        .status()
        .map(|st| st.success())
        .unwrap_or(false);
    if ok { 1 } else { 0 }
}

#[no_mangle]
pub extern "C" fn ephy_rust_sanitize_filename(name: *const c_char) -> *mut c_char {
    if name.is_null() {
        return std::ptr::null_mut();
    }
    let s = unsafe { CStr::from_ptr(name) }.to_string_lossy();
    let clean: String = s.chars().map(|c| match c {
        '<' | '>' | ':' | '"' | '/' | '\\' | '|' | '?' | '*' => '_',
        _ if c.is_control() => '_',
        _ => c,
    }).collect();
    CString::new(clean.trim()).map(|cs| cs.into_raw()).unwrap_or(std::ptr::null_mut())
}

#[no_mangle]
pub extern "C" fn ephy_rust_adblock_match(url: *const c_char) -> i32 {
    if url.is_null() {
        return 0;
    }
    let u = unsafe { CStr::from_ptr(url) }.to_string_lossy().to_lowercase();
    const TRACKERS: &[&str] = &[
        "doubleclick.net", "googlesyndication.com", "google-analytics.com",
        "googletagmanager.com", "facebook.net/tr", "ads.yahoo.com", "adnxs.com",
    ];
    if TRACKERS.iter().any(|t| u.contains(t)) { 1 } else { 0 }
}

#[no_mangle]
pub extern "C" fn ephy_rust_search_firefox_addons(query: *const c_char) -> *mut c_char {
    if query.is_null() {
        return std::ptr::null_mut();
    }
    let q = unsafe { CStr::from_ptr(query) }.to_string_lossy();
    let encoded: String = q.bytes().map(|b| {
        if b.is_ascii_alphanumeric() || b == b'-' || b == b'_' || b == b'.' {
            (b as char).to_string()
        } else {
            format!("%{:02X}", b)
        }
    }).collect();
    let url = format!("https://addons.mozilla.org/api/v5/addons/search/?q={}&type=extension&page_size=10", encoded);
    let out = match std::process::Command::new("curl.exe")
        .args(["-sL", "-H", "User-Agent: GNOME-Beta-AMO", &url])
        .output()
    {
        Ok(o) if o.status.success() => String::from_utf8_lossy(&o.stdout).into_owned(),
        _ => return std::ptr::null_mut(),
    };
    CString::new(out).map(|cs| cs.into_raw()).unwrap_or(std::ptr::null_mut())
}

#[no_mangle]
pub extern "C" fn ephy_rust_install_firefox_xpi(xpi_path: *const c_char, dest_dir: *const c_char) -> i32 {
    if xpi_path.is_null() || dest_dir.is_null() {
        return 0;
    }
    let xpi = unsafe { CStr::from_ptr(xpi_path) }.to_string_lossy();
    let dest = unsafe { CStr::from_ptr(dest_dir) }.to_string_lossy();
    let _ = std::fs::create_dir_all(dest.as_ref());
    let ok = std::process::Command::new("tar.exe")
        .args(["-xf", xpi.as_ref(), "-C", dest.as_ref()])
        .status()
        .map(|st| st.success())
        .unwrap_or(false);
    if ok && std::path::Path::new(dest.as_ref()).join("manifest.json").is_file() {
        1
    } else {
        0
    }
}
