#!/usr/bin/env bash
set -e
export PATH="/ucrt64/bin:$PATH"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DIST="$ROOT/dist/app"
BUILD="$ROOT/build"
WEBKIT_SRC="${1:-}"

rm -rf "$DIST"
mkdir -p "$DIST/share/glib-2.0/schemas" "$DIST/share/icons" "$DIST/share/epiphany"

# 1. Copy Epiphany binaries and internal DLLs
cp "$BUILD/src/epiphany.exe" "$DIST/"
cp "$BUILD/src/libephymain.dll" "$DIST/"
cp "$BUILD/lib/libephymisc.dll" "$DIST/"
cp "$BUILD/lib/autofill/libephyautofill.dll" "$DIST/"
cp "$BUILD/lib/sync/libephysync.dll" "$DIST/"

# 1b. Build native Libadwaita Linux-style installer into staging
gcc -O2 -Wall -Wextra "$ROOT/installer/gnome-web-installer.c" -o "$DIST/gnome-web-installer.exe" \
  -DAPP_VERSION="\"${VERSION:-51.2.0}\"" \
  $(pkg-config --cflags --libs libadwaita-1 gtk4 gio-2.0) -lole32 -lshell32 -luuid -mwindows

# 2. Copy WebKit WinCairo runtime if provided
if [ -n "$WEBKIT_SRC" ] && [ -d "$WEBKIT_SRC" ]; then
  echo "Copying WebKit WinCairo from $WEBKIT_SRC..."
  cp -n "$WEBKIT_SRC"/*.dll "$DIST/" 2>/dev/null || true
  cp -n "$WEBKIT_SRC"/*.exe "$DIST/" 2>/dev/null || true
  [ -d "$WEBKIT_SRC/WebKit.resources" ] && cp -r "$WEBKIT_SRC/WebKit.resources" "$DIST/" || true
  [ -d "$WEBKIT_SRC/cacert.pem" ] || cp "$WEBKIT_SRC/cacert.pem" "$DIST/" 2>/dev/null || true
fi

# 3. Collect all UCRT64 DLL dependencies via ldd
echo "Collecting UCRT64 runtime DLLs..."
for bin in "$DIST/epiphany.exe" "$DIST/gnome-web-installer.exe" "$DIST/libephymain.dll" "$DIST/libephymisc.dll" "$DIST/libephyautofill.dll" "$DIST/libephysync.dll"; do
  ldd "$bin" | awk '/\/ucrt64\/bin\// {print $3}' | sort -u | while read -r dll; do
    [ -f "$dll" ] && cp -n "$dll" "$DIST/"
  done
done

# GDK-Pixbuf loaders & GIO modules
mkdir -p "$DIST/lib/gdk-pixbuf-2.0/2.10.0/loaders" "$DIST/lib/gio/modules"
cp -r /ucrt64/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.dll "$DIST/lib/gdk-pixbuf-2.0/2.10.0/loaders/" 2>/dev/null || true
cp /ucrt64/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache "$DIST/lib/gdk-pixbuf-2.0/2.10.0/" 2>/dev/null || true
cp /ucrt64/lib/gio/modules/*.dll "$DIST/lib/gio/modules/" 2>/dev/null || true
for mod in "$DIST"/lib/gio/modules/*.dll "$DIST"/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.dll; do
  [ -f "$mod" ] && ldd "$mod" | awk '/\/ucrt64\/bin\// {print $3}' | sort -u | while read -r dll; do
    [ -f "$dll" ] && cp -n "$dll" "$DIST/"
  done
done

# 4. GSettings schemas (Epiphany + GTK4 + gsettings-desktop-schemas)
cp "$ROOT/data/org.gnome.epiphany.gschema.xml" "$DIST/share/glib-2.0/schemas/"
cp "$BUILD/data/org.gnome.Epiphany.enums.xml" "$DIST/share/glib-2.0/schemas/" 2>/dev/null || true
cp /ucrt64/share/glib-2.0/schemas/*.xml "$DIST/share/glib-2.0/schemas/" 2>/dev/null || true
glib-compile-schemas "$DIST/share/glib-2.0/schemas"

# 5. Icons, GTK4 Windows Font Config, SSL CA bundle & Epiphany data
mkdir -p "$DIST/etc/gtk-4.0" "$DIST/share/gtk-4.0"
cat > "$DIST/etc/gtk-4.0/settings.ini" << 'EOF'
[Settings]
gtk-font-name=Segoe UI 10
gtk-hint-font-metrics=1
gtk-xft-antialias=1
gtk-xft-hinting=1
gtk-xft-hintstyle=hintslight
gtk-xft-rgba=rgb
EOF
cp "$DIST/etc/gtk-4.0/settings.ini" "$DIST/share/gtk-4.0/settings.ini"
[ -f /ucrt64/etc/ssl/certs/ca-bundle.crt ] && cp /ucrt64/etc/ssl/certs/ca-bundle.crt "$DIST/cacert.pem" || true
cp -r /ucrt64/share/icons/Adwaita "$DIST/share/icons/" 2>/dev/null || true
cp -r /ucrt64/share/icons/hicolor "$DIST/share/icons/" 2>/dev/null || true
mkdir -p "$DIST/share/icons/hicolor/scalable/apps" "$DIST/share/icons/hicolor/symbolic/apps"
cp "$ROOT/data/icons/hicolor/scalable/apps/org.gnome.Epiphany.svg" "$DIST/share/icons/hicolor/scalable/apps/" 2>/dev/null || true
cp "$ROOT/data/icons/hicolor/symbolic/apps/org.gnome.Epiphany-symbolic.svg" "$DIST/share/icons/hicolor/symbolic/apps/" 2>/dev/null || true
gtk4-update-icon-cache -f -t "$DIST/share/icons/Adwaita" 2>/dev/null || true
gtk4-update-icon-cache -f -t "$DIST/share/icons/hicolor" 2>/dev/null || true
cp "$BUILD/data/default-bookmarks.rdf" "$DIST/share/epiphany/" 2>/dev/null || true

echo "Staged GNOME Web (Epiphany) in $DIST"
