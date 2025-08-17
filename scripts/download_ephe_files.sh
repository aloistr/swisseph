#!/usr/bin/env bash
set -euo pipefail

# --- Config (override via env) ---
JPL_URL="${JPL_URL:-https://ssd.jpl.nasa.gov/ftp/eph/planets/Linux/de431.eph}"   # use HTTPS if possible
DOWNLOADED_FILENAME="${DOWNLOADED_FILENAME:-de431.eph}"
FINAL_FILENAME="${FINAL_FILENAME:-de431.eph}"
EXPECTED_CHECKSUM="${EXPECTED_CHECKSUM:-}"    # optional SHA256
OUT_DIR="${OUT_DIR:-data/ephemeris}"

# --- Helpers (macOS-friendly) ---
hash_cmd() {
  if command -v shasum >/dev/null 2>&1; then echo "shasum -a 256"; return; fi
  if command -v sha256sum >/dev/null 2>&1; then echo "sha256sum"; return; fi
  echo ""
}

verify_checksum() {
  local file="$1" expected="$2" cmd
  [[ -z "$expected" ]] && { echo "No EXPECTED_CHECKSUM; skipping checksum verification."; return; }
  cmd="$(hash_cmd)"
  [[ -z "$cmd" ]] && { echo "No SHA256 tool found; skipping checksum verification."; return; }
  local actual
  actual="$($cmd "$file" | awk '{print $1}')"
  if [[ "$actual" != "$expected" ]]; then
    echo "Checksum mismatch for $file"
    echo "Expected: $expected"
    echo "Actual:   $actual"
    exit 1
  fi
  echo "Checksum OK."
}

need_space() {
  # require ~4GB free
  local required_kb=$((4 * 1024 * 1024))
  local avail_kb
  avail_kb=$(df -Pk "$OUT_DIR" 2>/dev/null | awk 'NR==2{print $4}')
  if [[ -z "${avail_kb:-}" ]]; then
    mkdir -p "$OUT_DIR"
    avail_kb=$(df -Pk "$OUT_DIR" | awk 'NR==2{print $4}')
  fi
  if (( avail_kb < required_kb )); then
    echo "Not enough disk space in $OUT_DIR (need ~4GB free)."
    exit 1
  fi
}

file_size_bytes() {
  # macOS/BSD stat
  stat -f%z "$1" 2>/dev/null || echo 0
}

# --- Main ---
mkdir -p "$OUT_DIR"
need_space

tmp="${OUT_DIR}/${DOWNLOADED_FILENAME}.part"

echo "Downloading:"
echo "  URL:      $JPL_URL"
echo "  To:       $tmp"
echo "  Final as: ${OUT_DIR}/${FINAL_FILENAME}"

# curl (native on macOS); fallback to wget if installed
if command -v curl >/dev/null 2>&1; then
  # -f fail on HTTP errors, -L follow redirects, --continue-at - = resume
  curl -fL --retry 5 --retry-delay 3 --continue-at - --output "$tmp" "$JPL_URL"
elif command -v wget >/dev/null 2>&1; then
  wget --tries=5 --timeout=30 --continue -O "$tmp" "$JPL_URL"
else
  echo "Need curl or wget installed."
  exit 1
fi

size_bytes=$(file_size_bytes "$tmp")
if [[ "$size_bytes" -lt 1073741824 ]]; then
  echo "Downloaded file looks too small ($size_bytes bytes)."
  echo "You may have grabbed an HTML page instead of the raw file."
  exit 1
fi

verify_checksum "$tmp" "$EXPECTED_CHECKSUM"

mv -f "$tmp" "${OUT_DIR}/${FINAL_FILENAME}"
echo "Done: ${OUT_DIR}/${FINAL_FILENAME}"
