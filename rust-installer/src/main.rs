#![windows_subsystem = "windows"]

use std::env;
use std::fs::{self, File};
use std::io;
use std::path::{Path, PathBuf};
use std::process::Command;

static PAYLOAD_ZIP: &[u8] = include_bytes!("../../dist/payload.zip");

fn extract_zip(data: &[u8], dest: &Path) -> io::Result<()> {
    let reader = io::Cursor::new(data);
    let mut archive = zip::ZipArchive::new(reader).map_err(|e| io::Error::new(io::ErrorKind::Other, e))?;
    for i in 0..archive.len() {
        let mut file = archive.by_index(i).map_err(|e| io::Error::new(io::ErrorKind::Other, e))?;
        let outpath = match file.enclosed_name() {
            Some(p) => dest.join(p),
            None => continue,
        };
        if file.is_dir() {
            fs::create_dir_all(&outpath)?;
        } else {
            if let Some(p) = outpath.parent() {
                if !p.exists() {
                    fs::create_dir_all(p)?;
                }
            }
            let mut outfile = File::create(&outpath)?;
            io::copy(&mut file, &mut outfile)?;
        }
    }
    Ok(())
}

fn main() {
    let temp_dir: PathBuf = env::temp_dir().join(format!("gnome-web-setup-{}", std::process::id()));
    let _ = fs::remove_dir_all(&temp_dir);
    if fs::create_dir_all(&temp_dir).is_err() {
        return;
    }

    if extract_zip(PAYLOAD_ZIP, &temp_dir).is_ok() {
        let installer_exe = temp_dir.join("gnome-web-installer.exe");
        let args: Vec<String> = env::args().skip(1).collect();
        let mut cmd = Command::new(&installer_exe);
        cmd.arg("--source").arg(&temp_dir);
        for a in args {
            cmd.arg(a);
        }
        if let Ok(mut child) = cmd.spawn() {
            let _ = child.wait();
        }
    }

    let _ = fs::remove_dir_all(&temp_dir);
}
