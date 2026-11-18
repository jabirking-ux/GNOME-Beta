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
