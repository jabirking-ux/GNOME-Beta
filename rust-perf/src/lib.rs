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
